/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/dllmain.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
//Will ignore non local file format (WMS; GeoRaster, etc...)
#define PREVIEWHANDLER_FILE_FORMATS       
#define RASTERLIB_FILE_FORMATS
#include <ImagePP/all/h/HRFFileFormats.h>

#include <Imagepp/all/h/ImageppLib.h>
#include <PdfLibInitializer/PdfLibInitializer.h>

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


struct MyImageppLibHost : ImagePP::ImageppLib::Host     
    {   
    MyImageppLibHost()
        {
        BeAssertFunctions::SetBeAssertHandler (callHostOnAssert);
        }

    virtual void _RegisterFileFormat() override                  
        {            
        REGISTER_SUPPORTED_FILEFORMAT                                
        }                                                            
    };   

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MyPdfLibInitializerAdmin : Bentley::PdfLibInitializerAdmin
    {
protected:
    virtual BentleyStatus _GetPDFDataPath
    (
    WStringR cMapDir,
    WStringR unicodeDir,
    WStringR fontDir,
    WStringR colorDir,
    WStringR pluginDir
    ) const override
        {
        // Setup location to find PDF resource files (was created under exe location)...
        WString baseDir;
        GetBaseDirOfExecutingModule (baseDir);

        cMapDir = baseDir + WString(L"System\\Data\\PDFL\\CMap");
        fontDir = baseDir + WString(L"System\\Data\\PDFL\\Font");
        unicodeDir = baseDir + WString(L"System\\Data\\PDFL\\Unicode");

        return BSISUCCESS;
        }
    virtual bool   _IsPDFLibAvailable() const override  {return false;}

    };// MyPdfLibInitializerAdmin

/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct MyPdfLibInitializerHost : Bentley::PdfLibInitializer::Host 
    {
    virtual PdfLibInitializerAdmin& _SupplyPdfLibInitializerAdmin() override { return *new MyPdfLibInitializerAdmin(); }
    }; // MyPdfLibInitializerHost


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IThumbnailsProvider* ThumbnailsProvider::Get()
    {
    static ThumbnailsProvider s_thumbnailsProvider;
    return &s_thumbnailsProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ThumbnailsProvider::ThumbnailsProvider()
    {
    Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ThumbnailsProvider::~ThumbnailsProvider()
    {
    Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ThumbnailsProvider::Initialize()
    {
    CriticalSectionHelper::Init ();

    //Application needs to initialize PdfLibInitializer dll if it wants support for PDF raster attachment.
    Bentley::PdfLibInitializer::Initialize(*new MyPdfLibInitializerHost());

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    PointCloudVortex::Initialize();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ThumbnailsProvider::Terminate()
    {
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);
    Bentley::PdfLibInitializer::GetHost().Terminate(true);
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
