/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/PlatformLib.h>

// A published unit test can include only published API header files ... with the exception of a few utilities like this.
// This header file is snuck in through a back door. This header file can only include and use published API header files
// (or the few headers that are brought in through the back door).

BEGIN_BENTLEY_DGN_NAMESPACE

struct ScopedDgnHostImpl;

struct ScopedDgnHostConfig
{
    virtual ~ScopedDgnHostConfig() { }
    virtual PlatformLib::Host::VisualizationAdmin* SupplyVisualizationAdmin() { return nullptr; }
    virtual PlatformLib::Host::BRepGeometryAdmin* SupplyBRepGeometryAdmin() { return nullptr; }
};

//! Registers a host within a scope
struct ScopedDgnHost
{
    ScopedDgnHostImpl* m_pimpl;

    DGNPLATFORM_EXPORT explicit ScopedDgnHost(ScopedDgnHostConfig* config = nullptr);
    DGNPLATFORM_EXPORT ~ScopedDgnHost();
};

struct TestDataManager
{
private:
    WString                 m_fileName;
    BeSQLite::Db::OpenMode  m_openMode;
    bool                    m_fill;

    DgnDbPtr                m_dgndb;
    DgnModelP               m_model;

public:
    DGNPLATFORM_EXPORT TestDataManager (WCharCP fullFileName, BeSQLite::Db::OpenMode mode, bool needBriefcase, bool fill=true);
    DGNPLATFORM_EXPORT ~TestDataManager ();
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP() const;
                       DgnModelP GetAndFillDgnModelP() { return GetDgnModelP(); }
                       WString GetPath() const { DgnDbP file = GetDgnProjectP(); return file? file->GetFileName(): L""; }
                       DgnDbP GetDgnProjectP() const { return m_dgndb.get(); }
    DGNPLATFORM_EXPORT static StatusInt FindTestData (BeFileName& fullFileName, WCharCP fileName, BeFileName const& searchLeafDir);
    DGNPLATFORM_EXPORT BentleyStatus OpenTestFile (bool needBriefcase);
    DGNPLATFORM_EXPORT void CloseTestFile ();

    // Ensures a transactable, standalone iModel.
    DGNPLATFORM_EXPORT static void SetAsStandaloneDb(DgnDbPtr& db, DgnDb::OpenMode mode);
};

struct TestDgnManager : TestDataManager
{
enum DgnInitializeMode {DGNINITIALIZEMODE_None, DGNINITIALIZEMODE_FillModel}; // DGNINITIALIZEMODE_FillModel will load the default root model.

TestDgnManager (WCharCP fullFileName, BeSQLite::Db::OpenMode mode, bool needBriefcase, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel) : TestDataManager(fullFileName,mode,needBriefcase,(DGNINITIALIZEMODE_FillModel==imode)) {}
};

END_BENTLEY_DGN_NAMESPACE

