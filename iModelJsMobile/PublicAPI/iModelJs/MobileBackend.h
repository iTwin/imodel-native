/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <functional>

typedef struct napi_env__* napi_env;

//=======================================================================================
// @bsiclass                                                  Steve.Wilson 11/19
//=======================================================================================
struct MobileBackend {
public:
  typedef std::function<void(napi_env)> Callback_T;

  static void Start(int argc, const char** argv);
  static void SetPort(unsigned int value);
  static unsigned int GetPort();
  static void RunOnEventLoop(const Callback_T& callback);
};
