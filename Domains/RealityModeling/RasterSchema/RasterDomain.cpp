/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

DOMAIN_DEFINE_MEMBERS(RasterDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
RasterDomain::RasterDomain() : DgnDomain(BENTLEY_RASTER_SCHEMA_NAME, "Bentley Raster Domain", 1) 
    {
    RegisterHandler(RasterModelHandler::GetHandler());
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void RasterDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

