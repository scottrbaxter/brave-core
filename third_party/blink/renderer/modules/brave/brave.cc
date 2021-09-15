/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/renderer/modules/brave/brave.h"

#include "brave/third_party/blink/renderer/modules/brave/skus/skus.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"

namespace blink {

ScriptPromise Brave::isBrave(ScriptState* script_state) {
  auto* resolver = MakeGarbageCollected<ScriptPromiseResolver>(script_state);
  ScriptPromise promise = resolver->Promise();
  resolver->Resolve(true);
  return promise;
}

Skus* Brave::skus() {
  if (!skus_) {
    skus_ = MakeGarbageCollected<Skus>();
  }
  return skus_;
}

void Brave::Trace(blink::Visitor* visitor) const {
  visitor->Trace(skus_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
