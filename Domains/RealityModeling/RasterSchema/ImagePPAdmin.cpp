/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/ImagePPAdmin.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>

//Will ignore non local file format (WMS; GeoRaster, etc...)
#define PREVIEWHANDLER_FILE_FORMATS       

#include <ImagePP/all/h/HRFFileFormats.h>

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
static void callHostOnAssert(WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType)
    {
    BeAssertFunctions::DefaultAssertionFailureHandler(_Message, _File, _Line);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
MyImageppLibHost::MyImageppLibHost()
    {
    BeAssertFunctions::SetBeAssertHandler(callHostOnAssert);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
ImagePP::ImageppLibAdmin& MyImageppLibHost::_SupplyImageppLibAdmin()
    {
    return *new MyImageppLibAdmin();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void MyImageppLibHost::_RegisterFileFormat()
    {
    REGISTER_SUPPORTED_FILEFORMAT
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
ImagePP::IRasterGeoCoordinateServices* MyImageppLibAdmin::_GetIRasterGeoCoordinateServicesImpl() const
    {
    //WIP
    //if (GeoCoordinationManager::GetServices() != NULL)
    return ImagePP::ImageppLib::GetDefaultIRasterGeoCoordinateServicesImpl();

    //return NULL;
    }
