/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
/* global window */

const { EventEmitter } = require('events')

const TrezorConnect = require('trezor-connect').default;

import {
  HardwareWallet,
  TrezorDerivationPaths
} from '../../components/desktop/popup-modals/add-account-modal/hardware-wallet-connect/types'

const TREZOR_CONNECT_MANIFEST = {
  email: 'support@metamask.io',
  appUrl: 'https://metamask.io',
}

export default class TrezorBridgeKeyring extends EventEmitter {
  constructor () {
    super()
    TrezorConnect.manifest(TREZOR_CONNECT_MANIFEST);
  }

  type = () => {
    return HardwareWallet.Trezor
  }

  getAccounts = ({from, to, scheme = TrezorDerivationPaths.Default}:{from: number, to: number, scheme?: string}) => {
    return new Promise(async (resolve, reject) => {
      if (scheme !== TrezorDerivationPaths.Default) {
        reject(new Error('Unsupported derivation paths'))
        return
      }
      if (from < 0) {
        from = 0
      }
      this._getAccounts(from, to).then(resolve).catch(reject)
    })
  }

  /* PRIVATE METHODS */
  _getPathForIndex = (index: number) => {
      return `m/44'/60'/${index}'/0/0`
  }

  _getAccounts = async (from: number, to: number) => {
    let request = []

    for (let i = from; i <= to; i++) {
      const path = this._getPathForIndex(i)
      request.push({ path: path, showOnTrezor: false})
    }
    console.log(JSON.stringify(request))
    return TrezorConnect.ethereumGetAddress({
      bundle: request})
  }
}
