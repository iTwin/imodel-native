/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformAdmins.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
//Will ignore non local file format (WMS; GeoRaster, etc...)
#define PREVIEWHANDLER_FILE_FORMATS       

#include <ImagePP/all/h/HRFFileFormats.h>
#include "RealityPlatformUtil.h"
#include "PointCloudVortex.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void callHostOnAssert(WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType)
    {
    //&&JFC  We want to assert, not silently ignore it. Look for example elsewhere.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool InitBaseGCS()
    {
    //WIP
    //BeFileName dllFileName;
    //Bentley::BeGetModuleFileName(dllFileName, NULL);

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString geoCoordDir = exePath;
    size_t pos = geoCoordDir.find_last_of(L"/\\");
    geoCoordDir = geoCoordDir.substr(0, pos + 1);
    geoCoordDir.append(L"GeoCoordinateData");

    // Make sure directory exist.
    BeFileName dir(geoCoordDir);
    if (!dir.IsDirectory())
        return false;

    GeoCoordinates::BaseGCS::Initialize(geoCoordDir.c_str());

    return true;
    }

struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    virtual ImagePP::IRasterGeoCoordinateServices* _GetIRasterGeoCoordinateServicesImpl() const override
        {
        //WIP
        //if (GeoCoordinationManager::GetServices() != NULL)
        return ImagePP::ImageppLib::GetDefaultIRasterGeoCoordinateServicesImpl();

        //return NULL;
        }

    virtual ~MyImageppLibAdmin()
        {
        }
    };

struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost()
        {
        BeAssertFunctions::SetBeAssertHandler(callHostOnAssert);
        }


    virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override
        {
        return *new MyImageppLibAdmin();
        }


    virtual void _RegisterFileFormat() override
        {
        REGISTER_SUPPORTED_FILEFORMAT
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertiesProvider> PropertiesProvider::Create(WCharCP inFilename, PointCloudView view)
    {
    WString filename = inFilename;
    size_t pos = filename.find_last_of(L".");

    WString ext = filename.substr(pos, filename.length()); 
    ext.ToUpper();  //&&JFC use case insensitive(EqualsI) compare not ToUpper.

    if (ext.Equals(L".POD"))
        {
        //&&JFC We need a new instance per create. Using static here will have the effect of
        // always returning the same instance which was created using the first filename.
        // A refcounted object is always allocated with new and never allocated on the stack because the 
        // RefcountedPtr container will always call delete.
//         static PointCloudProperties s_pointCloudProperties(inFilename, view);
//         return &s_pointCloudProperties;
        }
    else
        {
//         static RasterProperties s_rasterProperties(inFilename);
//         return &s_rasterProperties;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RasterProperties::RasterProperties(WCharCP inFilename)
: m_filename(inFilename)
    {
    //&&JFC - need to this only once per session and out of the properties object.
    //      - Move *Properties methods in RealityDataProvider.cpp and keep only init/admin stuff here.
    Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RasterProperties::~RasterProperties()
    {
    Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterProperties::Initialize()
    {
    CriticalSectionHelper::Init();

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    if (!InitBaseGCS())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterProperties::Terminate()
    {
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudProperties::PointCloudProperties(WCharCP inFilename, PointCloudView view)
    {
    Initialize();   //&&JFC need to this only once per session.

    m_view = view;

    GetFile(inFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudProperties::~PointCloudProperties()
    {
    //CloseFile();
    Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudProperties::Initialize()
    {
    CriticalSectionHelper::Init();

    PointCloudVortex::Initialize();

    if (!InitBaseGCS())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudProperties::Terminate()
    {

    }

