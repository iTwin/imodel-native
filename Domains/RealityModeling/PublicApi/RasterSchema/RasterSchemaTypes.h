/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterSchemaTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/ExportMacros.h>

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct  RasterModel;

typedef RefCountedPtr<RasterModel>                          RasterModelPtr;

END_BENTLEY_RASTERSCHEMA_NAMESPACE

