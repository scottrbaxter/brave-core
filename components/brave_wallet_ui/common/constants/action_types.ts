// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import {
  AppObjectType,
  AccountInfo,
  TransactionInfo,
  TokenInfo
} from '../../constants/types'

export type InitializedPayloadType = {
  isWalletCreated: boolean
  isWalletLocked: boolean
  favoriteApps: AppObjectType[]
  isWalletBackedUp: boolean
  visibleTokens: string[]
  accountInfos: AccountInfo[]
}

export type UnlockWalletPayloadType = {
  password: string
}

export type ChainChangedEventPayloadType = {
  chainId: string
}

export type NewUnapprovedTxAdded = {
  txInfo: TransactionInfo
}

export type UnapprovedTxUpdated = {
  txInfo: TransactionInfo
}

export type TransactionStatusChanged = {
  txInfo: TransactionInfo
}

export type AddUserAssetPayloadType = {
  token: TokenInfo
  chainId: string
}

export type RemoveUserAssetPayloadType = {
  contractAddress: string
  chainId: string
}

export type SetUserAssetVisiblePayloadType = {
  contractAddress: string
  chainId: string
  isVisible: boolean
}
