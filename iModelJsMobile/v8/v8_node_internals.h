#include <limits.h>  // INT_MAX
#include <string.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <node-addon-api/node_api.h>
#include "util-inl.h"
#include "../node_internals.h"
#include <Bentley/BeAssert.h>
#undef min

// ***
// ***  This was extracted from node_internals.h -- it is v8-specific
// ***

#if NODE_MAJOR_VERSION < 8 || NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION < 6
class CallbackScope {
  public:
    CallbackScope(void *work);
};
#endif // NODE_MAJOR_VERSION < 8

namespace node {

#if NODE_MAJOR_VERSION < 8 || NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION < 6
typedef int async_id;

typedef struct async_context {
  node::async_id async_id;
  node::async_id trigger_async_id;
} async_context;

#define NODE_EXTERN
NODE_EXTERN async_context EmitAsyncInit(v8::Isolate* isolate,
                                        v8::Local<v8::Object> resource,
                                        v8::Local<v8::String> name,
                                        async_id trigger_async_id = -1);

NODE_EXTERN void EmitAsyncDestroy(v8::Isolate* isolate,
                                  async_context asyncContext);

v8::MaybeLocal<v8::Value> MakeCallback(v8::Isolate* isolate,
                                       v8::Local<v8::Object> recv,
                                       v8::Local<v8::Function> callback,
                                       int argc,
                                       v8::Local<v8::Value>* argv,
                                       async_context asyncContext);

#if NODE_MAJOR_VERSION < 8
class AsyncResource {
  public:
    AsyncResource(v8::Isolate* isolate,
                  v8::Local<v8::Object> object,
                  const char *name);
};
#endif // node version below 8

#endif // node version below 8.6



}  // namespace node


