/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             12/2017
//---------------------------------------------------------------------------------------
struct iModelHubWebsiteHost : DgnPlatformLib::Host
    {
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    void _SupplyProductName(Utf8StringR name) override { name.assign("iModel Hub Website"); }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(); }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }
    };

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             12/2017
//---------------------------------------------------------------------------------------
struct WebsiteSeedFileTests : public IntegrationTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    virtual void SetUp() override {}

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    virtual void TearDown() override {}
    };

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             12/2017
// This tests creates empty seed file for iModelHubWebsite
//---------------------------------------------------------------------------------------
TEST_F(WebsiteSeedFileTests, CreateSeedFile)
    {
    DgnPlatformLib::Initialize(*new iModelHubWebsiteHost());

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("iModel Hub Website seed file");
    createProjectParams.SetRootSubjectDescription("Seed file for iModel Hub Website.");
    createProjectParams.SetOverwriteExisting(true);
    BeSQLite::DbResult createStatus;

    BeFileName newfilePath;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(newfilePath, L"seedFile");
    BeAssert(SUCCESS == status && "Cannot get local directory");
    newfilePath.AppendToPath(L"iModelHubWebsiteSeed.bim");

    auto db = DgnDb::CreateDgnDb(&createStatus, newfilePath, createProjectParams);
    db->SaveChanges();

    std::cout << "New file created in " << newfilePath.GetNameUtf8() << std::endl;
    }