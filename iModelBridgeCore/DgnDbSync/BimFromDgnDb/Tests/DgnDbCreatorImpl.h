/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <DgnDb06Api/Bentley/Bentley.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformLib.h>

#include "Tests.h"
BEGIN_DGNDB0601_TO_JSON_NAMESPACE

struct DgnDbCreatorImpl
    {
    private:
        BeFileName m_dgndbFilename;
        DgnDbPtr            m_dgndb;

    public:
        DgnDbCreatorImpl(const char* fileName);
        bool CreateDgnDb();
        bool AddElement(const char* schemaName, const char* instanceXml);
        bool ImportSchema(const char* schemaXml);
    };

END_DGNDB0601_TO_JSON_NAMESPACE
