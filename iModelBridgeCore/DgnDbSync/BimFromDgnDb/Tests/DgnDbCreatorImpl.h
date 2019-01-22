/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/Tests/DgnDbCreatorImpl.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <DgnDb06Api/Bentley/Bentley.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformLib.h>

#define DGNDB0601_TO_JSON_NAMESPACE_NAME BimFromDgnDbTest
#define BEGIN_DGNDB0601_TO_JSON_NAMESPACE namespace BentleyG0601 { namespace Dgn { namespace DGNDB0601_TO_JSON_NAMESPACE_NAME {
#define END_DGNDB0601_TO_JSON_NAMESPACE   } } }

BEGIN_DGNDB0601_TO_JSON_NAMESPACE

struct DgnDbCreatorImpl
    {
    private:
        const char* m_dgndbFilename;

    public:
        DgnDbCreatorImpl(const char* fileName);

        void ImportSchema(const char* schemaXml);
    };

END_DGNDB0601_TO_JSON_NAMESPACE
