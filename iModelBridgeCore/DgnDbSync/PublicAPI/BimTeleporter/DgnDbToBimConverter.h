/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BimTeleporter/DgnDbToBimConverter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>

#define BEGIN_DGNDB_TO_BIM_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace DgnDbToBim {
#define END_DGNDB_TO_BIM_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_DGNDB_TO_BIM_NAMESPACE using namespace BentleyApi::DgnDbToBim;

BEGIN_DGNDB_TO_BIM_NAMESPACE

#ifdef __BIM_TELEPORTER_BUILD__
#define DGNDB_TO_BIM_CONVERTER_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNDB_TO_BIM_CONVERTER_EXPORT IMPORT_ATTRIBUTE
#endif

struct DgnDbToBimConverter
    {
    public:
        //! Converts a 1.6 dgndb/imodel to 2.0.  Calling application must have already initialized the host
        //! @param[in] inputPath    Full filename of the input 1.6 file
        //! @param[in] outputPath   Path to the directory where the 2.0 bim will be created
        DGNDB_TO_BIM_CONVERTER_EXPORT static bool Convert(WCharCP inputPath, WCharCP outputPath);
    };

END_DGNDB_TO_BIM_NAMESPACE
