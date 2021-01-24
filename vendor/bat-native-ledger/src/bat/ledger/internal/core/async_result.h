/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_LEDGER_CORE_ASYNC_RESULT_H_
#define BAT_LEDGER_CORE_ASYNC_RESULT_H_

#include <list>
#include <memory>
#include <utility>

#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/optional.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace ledger {

struct AsyncNullError {};

template <typename V, typename E = AsyncNullError>
class AsyncResult {
 public:
  using CompleteType = V;
  using ErrorType = E;

  using CompleteCallback = base::OnceCallback<void(const V&)>;
  using ErrorCallback = base::OnceCallback<void(const E&)>;

  enum class State { kPending, kComplete, kError };

  State state() { return store_->state; }

  void Listen(CompleteCallback on_complete, ErrorCallback on_error) {
    switch (store_->state) {
      case State::kComplete:
        base::SequencedTaskRunnerHandle::Get()->PostTask(FROM_HERE,
            base::BindOnce(AsyncResult::RunCompleteCallback, *this,
                           std::move(on_complete)));
        break;
      case State::kError:
        base::SequencedTaskRunnerHandle::Get()->PostTask(FROM_HERE,
            base::BindOnce(AsyncResult::RunErrorCallback, *this,
                           std::move(on_error)));
        break;
      case State::kPending:
        store_->complete_callbacks.emplace_back(std::move(on_complete));
        store_->error_callbacks.emplace_back(std::move(on_error));
        break;
    }
  }

  void Listen(CompleteCallback on_complete) {
    static_assert(
        std::is_same<E, AsyncNullError>::value,
        "An error callback is required when an error type is specified");
    Listen(std::move(on_complete), base::DoNothing());
  }

  class Resolver {
   public:
    Resolver() {}
    void Complete(V&& value) { result_.Complete(std::move(value)); }
    void Error(E&& error) { result_.Error(std::move(error)); }
    AsyncResult result() const { return result_; }

   private:
    AsyncResult result_;
  };

 private:
  AsyncResult() : store_(new Store()) {}

  struct Store {
    Store() {}
    State state = State::kPending;
    base::Optional<V> value;
    base::Optional<E> error;
    std::list<CompleteCallback> complete_callbacks;
    std::list<ErrorCallback> error_callbacks;
  };

  void Complete(V&& value) {
    if (store_->state != State::kPending) {
      return;
    }
    store_->state = State::kComplete;
    store_->value = std::move(value);
    for (auto& callback : store_->complete_callbacks) {
      std::move(callback).Run(*store_->value);
    }
    store_->complete_callbacks.clear();
    store_->error_callbacks.clear();
  }

  void Error(E&& error) {
    if (store_->state != State::kPending) {
      return;
    }
    store_->state = State::kError;
    store_->error = std::move(error);
    for (auto& callback : store_->error_callbacks) {
      std::move(callback).Run(*store_->error);
    }
    store_->complete_callbacks.clear();
    store_->error_callbacks.clear();
  }

  static void RunCompleteCallback(AsyncResult result,
                                  CompleteCallback on_complete) {
    DCHECK(result.store_->value);
    std::move(on_complete).Run(*result.store_->value);
  }

  static void RunErrorCallback(AsyncResult result, ErrorCallback on_error) {
    DCHECK(result.store_->error);
    std::move(on_error).Run(*result.store_->error);
  }

  std::shared_ptr<Store> store_;
};

}  // namespace ledger

#endif  // BAT_LEDGER_CORE_ASYNC_RESULT_H_
