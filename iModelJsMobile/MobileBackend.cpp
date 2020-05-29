/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "MobileBackendInternal.h"
#include <Bentley/BeAssert.h>
#include "node/src/node.h"
#include <atomic>

std::atomic<BackendEventLoop*> s_backend(nullptr);
std::atomic<bool> s_ready(false);
std::atomic<unsigned int> s_port(0);
std::atomic<uv_loop_t*> s_loop(nullptr);

namespace {

class Async {
  private:
    MobileBackend::Callback_T m_callback;
    uv_async_t m_handle;

    Async(const MobileBackend::Callback_T& callback)
      : m_callback(callback) {
        auto s = uv_async_init(s_loop.load(), &m_handle, Handler);
        BeAssert(s >= 0);

        m_handle.data = this;

        s = uv_async_send(&m_handle);
        BeAssert(s >= 0);
    }

    static void Handler(uv_async_t* handle) {
      auto instance = static_cast<Async*>(handle->data);
      auto isolate = s_backend.load()->GetIsolate();
      v8::HandleScope handles(isolate);
      auto context = isolate->GetCurrentContext();
      auto env = node::GetNapiEnv(context);
      instance->m_callback(env);
      uv_close((uv_handle_t*)handle, Free);
    }

    static void Free(uv_handle_t* handle) {
      auto instance = static_cast<Async*>(handle->data);
      delete instance;
    }

  public:
    static void Create(const MobileBackend::Callback_T& callback) {
      new Async(callback);
    }
};

}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
void MobileBackend::Start(int argc, const char **argv) {
  BeAssert(s_backend == nullptr);
  s_backend = new BackendEventLoop(argc, argv);

  while (!s_ready) {
    ;
  }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
void MobileBackend::SetPort(unsigned int value) {
  s_port = value;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
unsigned int MobileBackend::GetPort() {
  return s_port.load();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
void MobileBackend::RunOnEventLoop(const Callback_T& callback) {
  Async::Create(callback);  
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
BackendEventLoop::BackendEventLoop(int argc, const char **argv) {
  for (auto a = 0; a != argc; ++a) {
    size_t c = 0;
    for (;;) {
      auto v = *(argv[a] + c);
      m_args.push_back(v);
      ++c;

      if (v == '\0') {
        break;
      }
    }
  }

  char* marker = nullptr;
  for (auto& c : m_args) {
    if (marker == nullptr) {
      marker = &c;
    }

    if (c == '\0') {
      m_argv.push_back(marker);
      marker = nullptr;
    }
  }

  auto s = uv_thread_create(&m_thread, &ThreadEntry, this);
  BeAssert(s >= 0);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
BackendEventLoop::~BackendEventLoop() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
void BackendEventLoop::ThreadEntry(void *arg) {
  reinterpret_cast<BackendEventLoop *>(arg)->ThreadMain();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Steve.Wilson 11/2019
//---------------------------------------------------------------------------------------
void BackendEventLoop::ThreadMain() {
  const auto flags = "--jitless";
  v8::V8::SetFlagsFromString(flags, sizeof(flags) + 1);

  node::SetReadyCallback([this](v8::Isolate* isolate) {
    m_isolate = isolate;

    auto prop = v8::String::NewFromUtf8(isolate, "__imodeljsRpcPort").ToLocalChecked();
    auto context = isolate->GetCurrentContext();

    auto port = context->Global()->Get(context, prop).ToLocalChecked();
    if (port->IsNumber()) {
      s_port = port->ToUint32(context).ToLocalChecked()->Value();
      s_loop = node::GetCurrentEventLoop(isolate);
      return s_ready = true;
    }

    return false;
  });

  auto argc = static_cast<int>(m_argv.size());
  auto argv = &m_argv.front();
  node::Start(argc, argv);
}
