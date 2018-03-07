/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/DgnDbToBimConverter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/Bentley.h>

#define BEGIN_DGNDB_TO_BIM_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace DgnDbToBim {
#define END_DGNDB_TO_BIM_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_DGNDB_TO_BIM_NAMESPACE using namespace BentleyApi::DgnDbToBim;

BEGIN_DGNDB_TO_BIM_NAMESPACE

#ifdef __BIM_TELEPORTER_BUILD__
#define DGNDB_TO_BIM_CONVERTER_EXPORT __declspec(dllexport)
#else
#define DGNDB_TO_BIM_CONVERTER_EXPORT __declspec(dllimport)
#endif

struct DgnDbToBimConverter
    {
    public:
        __declspec(dllexport) static bool Convert(WCharCP inputPath, WCharCP outputPath);
    };
END_DGNDB_TO_BIM_NAMESPACE