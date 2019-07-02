/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <json/json.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshModuleInfo
    {
    ScalableMeshModuleInfo();
    bool ToJson(Json::Value& json) const;

#if _WIN32
    HMODULE m_handle = NULL;
#endif
    WString m_productName;
    WString m_productVersion;
    //WString m_dllCreationTime;
    WString m_publisherName;
    WString m_publisherVersion;
    };

StatusInt InitializeModuleHandle(ScalableMeshModuleInfo* moduleInfo);

END_BENTLEY_SCALABLEMESH_NAMESPACE