// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
import { ConnectionState } from './panel_browser_api'
import { Region } from './region_interface'

export default class APIProxy {
  static getInstance: () => APIProxy
  showUI: () => {}
  closeUI: () => {}
  getConnectionState: () => Promise<StateResponse>
  createVPNConnection: () => {}
  connect: () => {}
  disconnect: () => {}
  addVPNObserver: (obj: ServiceObserver) => {}
  getAllRegions: () => Promise<RegionResponse>
}
interface ServiceObserver {
  onConnectionCreated?: Function
  onConnectionRemoved?: Function
  onConnectionStateChanged?: Function
}
interface StateResponse {
  state: ConnectionState
}
interface RegionResponse {
  regions: Region
}
