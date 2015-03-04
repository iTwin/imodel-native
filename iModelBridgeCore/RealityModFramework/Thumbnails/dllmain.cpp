/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/dllmain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
//Will ignore non local file format (WMS; GeoRaster, etc...)
#define PREVIEWHANDLER_FILE_FORMATS       

#include <ImagePP/all/h/HRFFileFormats.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/basegeocoordapi.h>

static HINSTANCE                                s_moduleHandle = 0;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HINSTANCE GetDllHandle()
    {
    return s_moduleHandle;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void callHostOnAssert (WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool InitBaseGCS()
{
    //WIP
    //BeFileName dllFileName;
    //Bentley::BeGetModuleFileName(dllFileName, NULL);

    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

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
IPropertiesProvider* IPropertiesProvider::Create(WCharCP inFilename, WCharCP view)
{
    WString filename = inFilename;
    size_t pos = filename.find_last_of(L".");

    WString ext = filename.substr(pos, filename.length());
    ext.ToUpper();

    if (ext.Equals(L".POD"))
    {
        static PointCloudProperties s_pointCloudProperties(inFilename, view);
        return &s_pointCloudProperties;
    }
    else
    {
        static RasterProperties s_rasterProperties(inFilename);
        return &s_rasterProperties;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RasterProperties::RasterProperties(WCharCP inFilename)
    {
    Initialize();

    mRasterFile = GetFile(inFilename);
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
    CriticalSectionHelper::Init ();

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
PointCloudProperties::PointCloudProperties(WCharCP inFilename, WCharCP view)
{
    Initialize();

    WString viewTmp = view;
    viewTmp.ToUpper();
    if (viewTmp.Equals(L"RIGHT"))
    {
        mView = PointCloudView::Right;
    }
    else if (viewTmp.Equals(L"FRONT"))
    {
        mView = PointCloudView::Front;
    }
    else if (viewTmp.Equals(L"ISO"))
    {
        mView = PointCloudView::Iso;
    }
    else
    {
        mView = PointCloudView::Top;
    }

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


/*----------------------------------------------------------------------------+
|
| name      DllMain
|
| This function is called everytime a new process or thread is attached or detached
| to the dll
|
| author    Marc Bedard 06/2001
|
+----------------------------------------------------------------------------*/
bool APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
    {
    static long nProcess = 0;

    switch (ul_reason_for_call)
        {
            case DLL_PROCESS_ATTACH:
                {
                nProcess++;

                if ((1 == nProcess))
                    {
                    s_moduleHandle = (HINSTANCE) hModule;
                    }
                }
                break;
            case DLL_THREAD_ATTACH:
                {
                }
                break;
            case DLL_THREAD_DETACH:
                {
                }
                break;
            case DLL_PROCESS_DETACH:
                {
                nProcess--;
                if ((0 == nProcess))
                    {
                    s_moduleHandle = 0;
                    }
                }
                break;
        }
    return true;
    }
