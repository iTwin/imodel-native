/*
 * Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <folly/ExceptionWrapper.h>

#include <exception>
#include <iostream>

namespace folly {

[[noreturn]] void exception_wrapper::throwException() const {
  if (throwfn_) {
    throwfn_(item_.get());
  } else if (eptr_) {
    std::rethrow_exception(eptr_);
  }
  std::cerr
      << "Cannot use `throwException` with an empty folly::exception_wrapper"
      << std::endl;
  std::terminate();
}

fbstring exception_wrapper::class_name() const {
  if (item_) {
    auto& i = *item_;
    return demangle(typeid(i));
  } else if (eptr_) {
    return ename_;
  } else {
    return fbstring();
  }
}

fbstring exception_wrapper::what() const {
  if (item_) {
    return exceptionStr(*item_);
  } else if (eptr_) {
    return estr_;
  } else {
    return fbstring();
  }
}

fbstring exceptionStr(const exception_wrapper& ew) {
  return ew.what();
}

} // folly
