#include "v8_node_internals.h"

// ***
// ***  This was extracted from node_internals.cc -- it is v8-specific
// ***

#if NODE_MAJOR_VERSION < 8 || NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION < 6
CallbackScope::CallbackScope(void *work) {
}
#endif // NODE_MAJOR_VERSION < 8

namespace node {

#if NODE_MAJOR_VERSION < 8

async_context EmitAsyncInit(v8::Isolate* isolate,
                            v8::Local<v8::Object> resource,
                            v8::Local<v8::String> name,
                            async_id trigger_async_id) {
  return async_context();
}

void EmitAsyncDestroy(v8::Isolate* isolate,
                      async_context asyncContext) {
}

AsyncResource::AsyncResource(v8::Isolate* isolate,
                             v8::Local<v8::Object> object,
                             const char *name) {
}

#endif // NODE_MAJOR_VERSION < 8

#if NODE_MAJOR_VERSION < 8 || NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION < 6

v8::MaybeLocal<v8::Value> MakeCallback(v8::Isolate* isolate,
                                       v8::Local<v8::Object> recv,
                                       v8::Local<v8::Function> callback,
                                       int argc,
                                       v8::Local<v8::Value>* argv,
                                       async_context asyncContext) {
//  return node::MakeCallback(isolate, recv, callback, argc, argv);

    // TODO: Node's "MakeCallback" function does serveral special things that we should probably 
    //          emulate, including processing v8's "microtask" queue (promise callbacks). 
    //          It also processes its own queues. 
    //  Study InternalCallbackScope and InternalMakeCallback in node.cc!!

    return callback->Call(recv, argc, argv);

}

#endif // NODE_MAJOR_VERSION < 8 || NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION < 6


}  // namespace node

