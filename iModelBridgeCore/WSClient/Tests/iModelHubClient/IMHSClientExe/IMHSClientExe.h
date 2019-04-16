/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <DgnPlatform/DgnPlatform.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct KnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDir;
    BeFileName m_assetsDir;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDir; }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDir; }

    KnownLocationsAdmin(Utf8String exePath)
        {
        BeFileName exeFileName(exePath);
        BeFileName exeDirectoryName = exeFileName.GetDirectoryName();

        m_tempDir = BeFileName(exeDirectoryName);
        m_tempDir.AppendToPath(L"run\\temp\\");

        std::thread::id this_id = std::this_thread::get_id();
        Utf8String append;
        append.Sprintf("FoldThread%i", this_id);
        m_tempDir.AppendToPath(BeFileName(append));
        m_tempDir.AppendSeparator();
        BeFileName::CreateNewDirectory(m_tempDir);

        m_assetsDir = BeFileName(exeDirectoryName);
        m_assetsDir.AppendToPath(L"Assets");
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Algirdas.Mikoliunas                12/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMHSClientExe : DgnPlatformLib::Host
    {
    iModel::Hub::ClientPtr m_client;
    BriefcasePtr m_briefcase;
    Utf8String m_exePath;

    virtual void _SupplyProductName(BentleyApi::Utf8StringR name) override { name.assign("IMHSClientExe"); }
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownLocationsAdmin(m_exePath); }
    RepositoryAdmin& _SupplyRepositoryAdmin() override { return *(m_client->GetiModelAdmin()); }
    virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override { return BeSQLite::L10N::SqlangFiles(BeFileName()); }

    IMHSClientExe();
    BentleyStatus Initialize(Utf8String exePath);
    bool IsValid() const { return m_client.IsValid(); }
    BriefcasePtr AcquireBriefcase(iModelConnectionPtr connection, Utf8String guid);
    int CreateNewModelAndPush(Utf8String projectNr, Utf8String imodelId);
    };

END_BENTLEY_DGN_NAMESPACE
