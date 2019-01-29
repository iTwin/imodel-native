/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <Bentley/Desktop/FileSystem.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define UNIX_MAIN_CALLS_WMAIN(ARGV_TYPE)                                            \
extern "C" int main(int argc, char** argv)                                          \
    {                                                                               \
    BentleyApi::bvector<wchar_t*> argv_w_ptrs;                                      \
    for(int i=0; i<argc; i++)                                                       \
        {                                                                           \
        BentleyApi::WString argw(argv[i], BentleyApi::BentleyCharEncoding::Utf8);   \
        auto argp = new wchar_t[argw.size() + 1];                                   \
        wcscpy(argp, argw.data());                                                  \
        argv_w_ptrs.push_back(argp);                                                \
        }                                                                           \
                                                                                    \
    return wmain(argc, (ARGV_TYPE)argv_w_ptrs.data());                              \
    }

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* An implementation of IKnownLocationsAdmin that is useful for desktop applications.
* This implementation works for Windows, Linux, and MacOS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct KnownDesktopLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownDesktopLocationsAdmin()
        {
        Desktop::FileSystem::BeGetTempPath(m_tempDirectory);
        m_executableDirectory = Desktop::FileSystem::GetExecutableDir();
        m_assetsDirectory = m_executableDirectory;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

END_BENTLEY_DGN_NAMESPACE
