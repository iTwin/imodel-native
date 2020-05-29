/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <iModelJs/MobileBackend.h>
#include <libuv/uv.h>
#include <google_v8/v8.h>
#include <vector>

//=======================================================================================
// @bsiclass                                                  Steve.Wilson 11/19
//=======================================================================================
struct BackendEventLoop {
private:
  std::vector<char> m_args;
  std::vector<char*> m_argv;
  uv_thread_t m_thread;
  v8::Isolate* m_isolate;

  static void ThreadEntry(void* arg);
  void ThreadMain();

public:
  BackendEventLoop(int argc, const char** argv);
  ~BackendEventLoop();

  v8::Isolate* GetIsolate() const { return m_isolate; }
};
