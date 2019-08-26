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

#include <atomic>
#include <chrono>

#include <folly/portability/GFlags.h>

DECLARE_int32(codel_interval);
DECLARE_int32(codel_target_delay);

namespace wangle {

/// CoDel (controlled delay) is an active queue management algorithm from
/// networking for battling bufferbloat.
///
/// Services also have queues (of requests, not packets) and suffer from
/// queueing delay when overloaded. This class adapts the codel algorithm for
/// services.
///
/// Codel is discussed in depth on the web [1,2], but a basic sketch of the
/// algorithm is this: if every request has experienced queueing delay greater
/// than the target (5ms) during the past interval (100ms), then we shed load.
///
/// We have adapted the codel algorithm. TCP sheds load by changing windows in
/// reaction to dropped packets. Codel in a network setting drops packets at
/// increasingly shorter intervals (100 / sqrt(n)) to achieve a linear change
/// in throughput. In our experience a different scheme works better for
/// services: when overloaded slough off requests that we dequeue which have
/// exceeded an alternate timeout (2 * target_delay).
///
/// So in summary, to use this class, calculate the time each request spent in
/// the queue and feed that delay to overloaded(), which will tell you whether
/// to expire this request.
///
/// You can also ask for an instantaneous load estimate and the minimum delay
/// observed during this interval.
///
///
/// 1. http://queue.acm.org/detail.cfm?id=2209336
/// 2. https://en.wikipedia.org/wiki/CoDel
class Codel {

 public:
  Codel();

  /// Returns true if this request should be expired to reduce overload.
  /// In detail, this returns true if min_delay > target_delay for the
  /// interval, and this delay > 2 * target_delay.
  ///
  /// As you may guess, we observe the clock so this is time sensitive. Call
  /// it promptly after calculating queueing delay.
  bool overloaded(std::chrono::nanoseconds delay);

  /// Get the queue load, as seen by the codel algorithm
  /// Gives a rough guess at how bad the queue delay is.
  ///
  ///   min(100%, min_delay / (2 * target_delay))
  ///
  /// Return:  0 = no delay, 100 = At the queueing limit
  int getLoad();

  std::chrono::nanoseconds getMinDelay();
  std::chrono::milliseconds getInterval();
  std::chrono::milliseconds getTargetDelay();
  std::chrono::milliseconds getSloughTimeout();

 private:
  std::chrono::nanoseconds codelMinDelay_;
  std::chrono::time_point<std::chrono::steady_clock> codelIntervalTime_;

  // flag to make overloaded() thread-safe, since we only want
  // to reset the delay once per time period
  std::atomic<bool> codelResetDelay_;

  bool overloaded_;
};

} // namespace wangle
