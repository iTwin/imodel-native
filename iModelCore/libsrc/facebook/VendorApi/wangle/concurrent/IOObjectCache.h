/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include <folly/ThreadLocal.h>
#include <folly/io/async/EventBase.h>
#include <wangle/concurrent/GlobalExecutor.h>

namespace wangle {

/*
 * IOObjectCache manages objects of type T that are dependent on an EventBase
 * provided by the global IOExecutor.
 *
 * Provide a factory that creates T objects given an EventBase, and get() will
 * lazily create T objects based on an EventBase from the global IOExecutor.
 * These are stored thread locally - for a given pair of event base and calling
 * thread there will only be one T object created.
 *
 * The primary use case is for managing objects that need to do async IO on an
 * event base (e.g. thrift clients) that can be used outside the IO thread
 * without much hassle. For instance, you could use this to manage Thrift
 * clients that are only ever called from within other threads without the
 * calling thread needing to know anything about the IO threads that the clients
 * will do their work on.
 */
template <class T>
class IOObjectCache {
 public:
  typedef std::function<std::shared_ptr<T>(folly::EventBase*)> TFactory;

  IOObjectCache() = default;
  explicit IOObjectCache(TFactory factory)
    : factory_(std::move(factory)) {}

  std::shared_ptr<T> get() {
    CHECK(factory_);
    auto eb = getIOExecutor()->getEventBase();
    CHECK(eb);
    auto it = cache_->find(eb);
    if (it == cache_->end()) {
      auto p = cache_->insert(std::make_pair(eb, factory_(eb)));
      it = p.first;
    }
    return it->second;
  };

  void setFactory(TFactory factory) {
    factory_ = std::move(factory);
  }

 private:
  folly::ThreadLocal<std::map<folly::EventBase*, std::shared_ptr<T>>> cache_;
  TFactory factory_;
};

} // namespace wangle
