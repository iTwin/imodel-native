/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Napi/napi.h>
#include <string>
#include "SchemaUtil.h"

//=======================================================================================
//! @bsiclass                                   Chris.Lawson                   05/2020
//=======================================================================================
class ECSchemaOps : public Napi::ObjectWrap<ECSchemaOps> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ECSchemaOps(const Napi::CallbackInfo& info);

  private:
    static Napi::FunctionReference constructor;
    Napi::Value ComputeChecksum(const Napi::CallbackInfo& info);
    SchemaUtil *schemaUtil_;
};