/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECSchemaOps.h"
#include <string>

Napi::FunctionReference ECSchemaOps::constructor;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chris.Lawson     05/2020
//---------------------------------------------------------------------------------------
Napi::Object ECSchemaOps::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "ECSchemaOps", {
    InstanceMethod("computeChecksum", &ECSchemaOps::ComputeChecksum),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("ECSchemaOps", func);

  return exports;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chris.Lawson     05/2020
//---------------------------------------------------------------------------------------
ECSchemaOps::ECSchemaOps(const Napi::CallbackInfo& info) : Napi::ObjectWrap<ECSchemaOps>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  this->schemaUtil_ = new SchemaUtil();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chris.Lawson     05/2020
//---------------------------------------------------------------------------------------
Napi::Value ECSchemaOps::ComputeChecksum(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 2 || !info[0].IsString() || !info[1].IsArray()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  std::unordered_set<Utf8String> paths;
  Napi::Array jsPaths = info[1].As<Napi::Array>();
  unsigned int num_locations = jsPaths.Length();

  for (unsigned int i = 0; i < num_locations; i++) {
    Napi::String value = jsPaths.Get(i).As<Napi::String>();
    paths.emplace(value.Utf8Value());
  }

  Napi::String schemaPath = info[0].As<Napi::String>();
  std::string message;
  std::string sha1 = this->schemaUtil_->ComputeChecksum(message, schemaPath, paths);
  if ("" == sha1)
    Napi::TypeError::New(env, message).ThrowAsJavaScriptException();
  return Napi::String::New(env, sha1);
}