/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <windows.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* A Windows-specific implementation of IKnownLocationsAdmin that uses standard
* Windows locations for the temporary directory and the executable directory for the
* DgnPlatform assets directory.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct WindowsKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
WindowsKnownLocationsAdmin()
    {
    // use the standard Windows temporary directory
    wchar_t tempPathW[MAX_PATH];
    ::GetTempPathW (_countof(tempPathW), tempPathW);
    m_tempDirectory.SetName(tempPathW);
    m_tempDirectory.AppendSeparator();

    // the application directory is where the executable is located
    wchar_t moduleFileName[MAX_PATH];
    ::GetModuleFileNameW (NULL, moduleFileName, _countof(moduleFileName));
    BeFileName moduleDirectory(BeFileName::DevAndDir, moduleFileName);
    m_executableDirectory = moduleDirectory;
    m_executableDirectory.AppendSeparator();

    m_assetsDirectory = m_executableDirectory;
    m_assetsDirectory.AppendToPath(L"Assets");
    }
};

END_BENTLEY_DGN_NAMESPACE
