/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/WmsHandler.h>        

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

DOMAIN_DEFINE_MEMBERS(RasterDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
RasterDomain::RasterDomain() : DgnDomain(BENTLEY_RASTER_SCHEMA_NAME, "Bentley Raster Domain", 1) 
    {
    RegisterHandler(RasterModelHandler::GetHandler()); 
    RegisterHandler(WmsModelHandler::GetHandler());
    RegisterHandler(RasterFileModelHandler::GetHandler());
//&&ep1
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    // Make sure GCS is initialized. It is required before we start using BaseGCS object.
    T_HOST.GetGeoCoordinationAdmin()._GetServices();
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void RasterDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

