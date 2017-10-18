#include "FakeServer.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
#ifdef _WIP_
//---------------------------------------------------------------------------------------
// @bsiclass                                   Farhad.Kabir                      10/17
//+---------------+---------------+---------------+---------------+---------------+------
class FakeServerFixture : public DgnDbTestFixture
    {
    public:
        BeFileName outPath;
        BeFileName m_seed;
        BeFileName GetOutputDirectory()
            {
            BeFileName outputDir;
            BeTest::GetHost().GetOutputRoot(outputDir);
            outputDir.AppendToPath(L"iModelHub");
            return outputDir;
            }
        BeFileName BuildDbFileNameCheck(Utf8StringCR baseName)
            {
            WString wFileName;
            BeStringUtilities::Utf8ToWChar(wFileName, (baseName + ".bim").c_str());
            BeFileName projectFileName = GetOutputDirectory();
            projectFileName.AppendToPath(wFileName.c_str());
            return projectFileName;
            }
        BeFileName BuildDbFileName(Utf8StringCR baseName)
            {
            WString wFileName;
            BeStringUtilities::Utf8ToWChar(wFileName, (baseName + ".bim").c_str());
            BeFileName projectFileName = GetOutputDirectory();
            BeFileName::CreateNewDirectory(projectFileName.c_str());
            EXPECT_TRUE(projectFileName.IsDirectory());
            projectFileName.AppendToPath(wFileName.c_str());
            return projectFileName;
            }
        virtual void SetUp()
            {
            BeTest::GetHost().GetOutputRoot(outPath);
            BeFileName seedFilePath = outPath;
            outPath.AppendToPath(L"Server");
            WCharCP serverPath = outPath.GetWCharCP();
            BeFileNameStatus stat = FakeServer::CreateFakeServer(serverPath);
            EXPECT_EQ(stat, BeFileNameStatus::Success);
            WCharP seedFile = L"DgnPlatformSeedManager_OneSpatialModel10.bim";
            EXPECT_EQ(BeFileNameStatus::Success, FakeServer::CreateiModelFromSeed(seedFilePath.c_str(), serverPath, seedFile));
            }
        static void SetUpTestCase()
            {
            DgnDbTestFixture::SetUpTestCase();
            }
        virtual void TearDown()
            {
            WCharCP serverPath = outPath.GetWCharCP();
            EXPECT_EQ(BeFileNameStatus::Success, FakeServer::DeleteAlliModels(serverPath));
            }
    };

//---------------------------------------------------------------------------------------
// @bsitest                                   Farhad.Kabir                      10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FakeServerFixture, CreateFakeServer) 
    {
    WCharCP serverPath = outPath.GetWCharCP();
    WCharP downloadPath = L"E:\\out2";
    WCharP seedFile = L"DgnPlatformSeedManager_OneSpatialModel10.bim";
    EXPECT_EQ(BeFileNameStatus::Success, FakeServer::DownloadiModel(downloadPath, serverPath, seedFile));
    DbResult res = DbResult::BE_SQLITE_ABORT;
    DgnDbPtr m_db = FakeServer::AcquireBriefcase(res, downloadPath, seedFile);
    EXPECT_EQ(DbResult::BE_SQLITE_OK, res);
    EXPECT_TRUE(m_db.IsValid());
    
    }

//---------------------------------------------------------------------------------------
// @bsitest                                   Farhad.Kabir                      10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FakeServerFixture, CreateFakeServer2)
    {
    FakeServer::CreateiModel(outPath, L"Briefcase0295.bim");
    }
#endif // _WIP_