/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/UnitTests/ScopedDgnHost.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// __BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnCore/DgnProject.h>

// A published unit test can include only published API header files ... with the exception of a few utilities like this.
// This header file is snuck in through a back door. This header file can only include and use published API header files 
// (or the few headers that are brought in through the back door).

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ScopedDgnHostImpl;

//! Registers a host within a scope
struct ScopedDgnHost
{
    ScopedDgnHostImpl* m_pimpl;

    DGNPLATFORM_EXPORT ScopedDgnHost();
    DGNPLATFORM_EXPORT ~ScopedDgnHost();
};

enum FileOpenMode
    {
    OPENMODE_READONLY               = (UInt32) DgnPlatform::DgnFileOpenMode::ReadOnly,
    OPENMODE_READWRITE              = (UInt32) DgnPlatform::DgnFileOpenMode::ReadWrite,
    OPENMODE_PREFERABLY_READWRITE   = (UInt32) DgnPlatform::DgnFileOpenMode::PreferablyReadWrite
    };

struct TestDataManager
{
    DgnProjectPtr m_project;
    DgnModelP     m_model;

    DGNPLATFORM_EXPORT TestDataManager (WCharCP fullFileName, FileOpenMode mode=OPENMODE_READWRITE, bool fill=true);
    DGNPLATFORM_EXPORT ~TestDataManager ();
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP() const;
                       DgnModelP GetAndFillDgnModelP() { return GetDgnModelP(); }
                       WString GetPath() const { DgnProjectP file = GetDgnProjectP(); return file? file->GetFileName(): L""; }
                       DgnProjectP GetDgnProjectP() const { return m_project.get(); }
    DGNPLATFORM_EXPORT static StatusInt FindTestData (BeFileName& fullFileName, WCharCP fileName, BeFileName const& searchLeafDir);
};

struct TestDgnManager : TestDataManager
{
enum DgnInitializeMode {DGNINITIALIZEMODE_None, DGNINITIALIZEMODE_FillModel}; // DGNINITIALIZEMODE_FillModel will load the default root model.

TestDgnManager (WCharCP fullFileName, FileOpenMode omode=OPENMODE_READWRITE, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel) : TestDataManager(fullFileName,omode,(DGNINITIALIZEMODE_FillModel==imode)) {;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

