/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/UnitTests/ScopedDgnHost.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// __BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnPlatformLib.h>

// A published unit test can include only published API header files ... with the exception of a few utilities like this.
// This header file is snuck in through a back door. This header file can only include and use published API header files 
// (or the few headers that are brought in through the back door).

BEGIN_BENTLEY_DGN_NAMESPACE

struct ScopedDgnHostImpl;

//! Registers a host within a scope
struct ScopedDgnHost
{
    ScopedDgnHostImpl* m_pimpl;

    struct FetchScriptCallback
        {
        virtual DgnDbStatus _FetchScript(Utf8StringR, DgnScriptType&, DateTime& lmt, DgnDbR, Utf8CP, DgnScriptType) = 0;
        };

    DGNPLATFORM_EXPORT explicit ScopedDgnHost();
    DGNPLATFORM_EXPORT ~ScopedDgnHost();

    DGNPLATFORM_EXPORT void SetFetchScriptCallback(FetchScriptCallback* cb);
    DGNPLATFORM_EXPORT void SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin);
    DGNPLATFORM_EXPORT DgnPlatformLib::Host::RepositoryAdmin* GetRepositoryAdmin();

    //! Temporarily override the host's RepositoryAdmin
    struct ScopedRepositoryAdminOverride
    {
    private:
        ScopedDgnHost&                          m_host;
        DgnPlatformLib::Host::RepositoryAdmin*  m_prevAdmin;

        ScopedRepositoryAdminOverride(ScopedRepositoryAdminOverride const&);
        ScopedRepositoryAdminOverride& operator=(ScopedRepositoryAdminOverride const&);
    public:
        ScopedRepositoryAdminOverride(ScopedDgnHost& host, DgnPlatformLib::Host::RepositoryAdmin* admin)
            : m_host(host), m_prevAdmin(host.GetRepositoryAdmin())
            {
            m_host.SetRepositoryAdmin(admin);
            }
        ~ScopedRepositoryAdminOverride()
            {
            m_host.SetRepositoryAdmin(m_prevAdmin);
            }
    };
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

    // Ensures a transactable, standalone briefcase.
    DGNPLATFORM_EXPORT static void MustBeBriefcase(DgnDbPtr& db, DgnDb::OpenMode mode);
};

struct TestDgnManager : TestDataManager
{
enum DgnInitializeMode {DGNINITIALIZEMODE_None, DGNINITIALIZEMODE_FillModel}; // DGNINITIALIZEMODE_FillModel will load the default root model.

TestDgnManager (WCharCP fullFileName, BeSQLite::Db::OpenMode mode, bool needBriefcase, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel) : TestDataManager(fullFileName,mode,needBriefcase,(DGNINITIALIZEMODE_FillModel==imode)) {}
};

END_BENTLEY_DGN_NAMESPACE

