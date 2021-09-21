// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

const path = require('path')
const GenerateDepfilePlugin = require('./webpack-plugin-depfile')
const fs = require('fs-extra')
const webpack = require('webpack')

const srcPath = path.resolve(__dirname, '../../../')
const braveSrcPath = path.join(srcPath, 'brave')

const pathMap = {
  // Find files in the current build configurations /gen directory
  'gen': process.env.ROOT_GEN_DIR,
  // Generated resources at this path are available at chrome://resources and
  // whilst webpack will still bundle, we keep the alias to the served path
  // to minimize knowledge of specific gen/ paths and easily allow us to not bundle
  // them in the future for certain build configurationa, just like chromium.
  'chrome://resources': path.join(process.env.ROOT_GEN_DIR, 'ui/webui/resources/preprocessed'),
  // We import brave-ui direct from source and not from package repo, so we need
  // direct path to the src/ directory.
  'brave-ui': path.resolve(__dirname, '../../node_modules/brave-ui/src'),
  // Force same styled-components module for brave-core and brave-ui
  // which ensure both repos code use the same singletons, e.g. ThemeContext.
  'styled-components': path.resolve(__dirname, '../../node_modules/styled-components'),
}

/**
 * Generates a tsconfig.json file in the gen/ directoryc
 * so that typescript can import files from cthe current build's
 * gen/ directory (e.g. mojom-generated JS).
 *
 * @param {*} [atPath=process.env.ROOT_GEN_DIR]
 * @returns void
 */
async function createGenTsConfig (atPath = process.env.ROOT_GEN_DIR) {
  const configExtendsFrom = path.relative(
    atPath,
    path.join(braveSrcPath, 'tsconfig-webpack.json')
  )
  const tsConfigPath = path.join(atPath, 'tsconfig.json')
  // Even though ts-loader will get the paths from webpack for module resolution
  // that does not help some issues where chromium both generates ts definitions
  // and has JSDoc comments for the .m.js file. Sometimes the JSDoc is incorrect
  // whilst the associated .d.ts file has the correct definition. Without specifying
  // the path mapping in the tsconfig.json, Typescript (via ts-loader) will use
  // the JSDoc, and fail with an error. The example that prompted this is cr.sendWithPromise
  // where Typescript will not see that the second parameter is an optional spread param
  // and will fail with an error. Whilst this should be fixed in the chromium source,
  // it's better to be explicit here so that developers get the same experience at
  // both compile and design time.
  const paths = {}
  for (const path in pathMap) {
    paths[`${path}/*`] = [`${pathMap[path]}/*`]
  }
  const config = {
    extends: configExtendsFrom,
    compilerOptions: {
      paths
    }
  }
  await fs.writeFile(tsConfigPath, JSON.stringify(config))
  return tsConfigPath
}

module.exports = async function (env, argv) {
  // TODO(petemill): only do this once per build, in a separate target
  // which the webpack targets depend on.
  const tsConfigPath = await createGenTsConfig()
  // Webpack config object
  return {
    devtool: argv.mode === 'development' ? '#inline-source-map' : false,
    output: {
      path: process.env.TARGET_GEN_DIR,
      filename: '[name].bundle.js',
      chunkFilename: '[id].chunk.js'
    },
    resolve: {
      extensions: ['.js', '.tsx', '.ts', '.json'],
      alias: pathMap,
      // For explanation of "chromeapp", see:
      // https://github.com/brave/brave-browser/issues/5587
      aliasFields: ['chromeapp']
    },
    optimization: {
      // Define NO_CONCATENATE for analyzing module size.
      concatenateModules: !process.env.NO_CONCATENATE
    },
    plugins: process.env.DEPFILE_SOURCE_NAME ? [
      new GenerateDepfilePlugin({
        depfilePath: process.env.DEPFILE_PATH,
        depfileSourceName: process.env.DEPFILE_SOURCE_NAME
      })
    ] : [],
    module: {
      rules: [
        {
          test: /\.tsx?$/,
          loader: 'ts-loader',
          exclude: /node_modules\/(?!brave-ui)/,
          options: {
            getCustomTransformers: path.join(__dirname, './webpack-ts-transformers.js'),
            allowTsInNodeModules: true,
            // TODO(petemill): generate in gen/ directory with baseUrl back to src/brave
            // - that would remove any problems with TS analyzing types in an incorrect
            // output directory (e.g. Static/ instead of Component/)
            configFile: tsConfigPath
          }
        },
        {
          test: /\.css$/,
          loader: ['style-loader', 'css-loader']
        },
        // Loads font files for Font Awesome
        {
          test: /\.woff(2)?(\?v=[0-9]\.[0-9]\.[0-9])?$/,
          loader: 'url-loader?limit=13000&minetype=application/font-woff'
        },
        {
          test: /\.(ttf|eot|ico|svg|png|jpg|jpeg|gif)(\?v=[0-9]\.[0-9]\.[0-9])?$/,
          loader: 'file-loader'
        }]
    },
    node: {
      fs: 'empty'
    }
  }
}
