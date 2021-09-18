/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CRYPTO_AEAD_H_
#define BRAVE_CHROMIUM_SRC_CRYPTO_AEAD_H_

// DO NOT use this function, this can only be called from
// PasswordEncryptor::DecryptForImporter
#define BRAVE_AEAD_H_                                                 \
  void OverrideNonceLength(size_t length) { nonce_length_ = length; } \
                                                                      \
 private:                                                             \
  size_t nonce_length_ = 0;

#include "../../../crypto/aead.h"

#undef BRAVE_AEAD_H_

#endif  // BRAVE_CHROMIUM_SRC_CRYPTO_AEAD_H_
