/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/WmsHandler.h>        

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

DOMAIN_DEFINE_MEMBERS(RasterDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
RasterDomain::RasterDomain() : DgnDomain(RASTER_SCHEMA_NAME, "Raster Domain", 1) 
    {
    RegisterHandler(RasterModelHandler::GetHandler()); 
    RegisterHandler(WmsModelHandler::GetHandler());
    RegisterHandler(RasterFileModelHandler::GetHandler());

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

