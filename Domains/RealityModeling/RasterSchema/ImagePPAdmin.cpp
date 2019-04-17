/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <RasterInternal.h>

//Will ignore non local file format (WMS; GeoRaster, etc...)
#define IPP_NO_REMOTE_FILE_FORMAT
#include <ImagePP/all/h/HRFFileFormats.h>

USING_NAMESPACE_BENTLEY_RASTER

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
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetDefaultTempDirectory(BeFileName& tempFileName) const
    {
    // Return the temp directory name. The directory is created if it does not exist.
    T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFileName, L"Raster");
    return BSISUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange) const
    {
    //If not empty and it exist, return it 
    if (!checkForChange && !m_localDirPath.IsEmpty() && BeFileName::IsDirectory(m_localDirPath.GetName()))
        {
        tempPath = m_localDirPath;
        return BSISUCCESS;
        }

    //Use default cache folder,m_localDirPath will be set by this call
    return T_Super::_GetLocalCacheDirPath(tempPath); 
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetGDalDataPath(BeFileNameR gdalDataPath) const
    {
    gdalDataPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    gdalDataPath.AppendToPath (L"GDalData");
    gdalDataPath.AppendSeparator();

    return BSISUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetECWDataPath(BeFileNameR ecwDataPath) const
    {
    ecwDataPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    ecwDataPath.AppendSeparator ();

    return BSISUCCESS;
    }

