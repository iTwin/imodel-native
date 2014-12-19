/*--------------------------------------------------------------------------------------+
|
|     $Source: all/utl/hfc/src/HFCResourceLoader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <ImagePP/all/h/HFCResourceLoader.h>
#include <ImagePP/all/h/ImagePPMessages.xliff.h>

#include <RmgrTools/ExportMacros.h>
#include <RmgrTools/Tools/RscFileManager.h>

#include <BeSQLite/L10N.h>

RscFileManager::DllRsc*  g_localResources = 0;

// Singleton
HFC_IMPLEMENT_SINGLETON(HFCResourceLoader)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static void dummy () {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuMarchand  05/2004
+---------------+---------------+---------------+---------------+---------------+------*/
HFCResourceLoader::HFCResourceLoader()
    {
#if 0 // WIP_BEFILENAME_PORTABILITY
    BeFileName dllFileName;
    Bentley::BeGetModuleFileName (dllFileName, &dummy);
    RscFileManager::StaticInitialize (L"");     // Usually done by Dgnplatform, Mstn or whatever but if not, it provides a default. ex. Active Image
    g_localResources = new RscFileManager::DllRsc(dllFileName);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuMarchand  05/2004
+---------------+---------------+---------------+---------------+---------------+------*/
HFCResourceLoader::~HFCResourceLoader()
    {
    }

#if defined (OLD_RSC_WAY)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString     HFCResourceLoader::GetString(unsigned int stringID) const
    {
    if (g_localResources==0)
        return WString(L"");

    return g_localResources->GetString(stringID, IMAGEPP_MessageList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString HFCResourceLoader::GetString(unsigned int stringID, unsigned int tableID) const
    {    
    if (g_localResources==0)
        return WString(L"");

    return g_localResources->GetString(stringID, tableID);
    }
#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString     HFCResourceLoader::GetString(ImagePPMessages stringID) const
    {
    return ImagePPMessage::GetStringW(stringID);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString HFCResourceLoader::GetExceptionString(ExceptionID stringID) const
    {    
    return ImagePPExceptionMessage::GetStringW(stringID);
    }
#endif

