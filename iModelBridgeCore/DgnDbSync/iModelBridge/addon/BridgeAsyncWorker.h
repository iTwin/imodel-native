/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Napi/napi.h>
#include <iModelBridge/iModelBridgeFwk.h>

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct RunBridgeAsyncWorker: Napi::AsyncWorker
    {
    Utf8String  m_jobInfo;
    int         m_status;
    void Execute() override;
    void OnOK() override;
    RunBridgeAsyncWorker(const Napi::Function& callback, Utf8StringCR jobInfo);
    };