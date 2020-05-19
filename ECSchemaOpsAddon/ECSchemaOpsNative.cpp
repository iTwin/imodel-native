
/* cppsrc/main.cpp */
#include <Napi/napi.h>
#include "ECSchemaOps.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chris.Lawson     05/2020
//---------------------------------------------------------------------------------------
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return ECSchemaOps::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)