/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/GTests/ScalableMeshUnitTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "SMUnitTestUtil.h"
#include <ScalableMesh/IScalableMeshProgress.h>

class ScalableMeshEnvironment : public ::testing::Environment
    {
    virtual void SetUp() 
        { 
        // Check that dataset path is valid
        BeFileName dataPath(SM_DATA_PATH);

        ASSERT_TRUE(ScalableMeshGTestUtil::GetDataPath(dataPath));

        ASSERT_TRUE(ScalableMeshGTestUtil::InitScalableMesh());

        BeFileName tempPath = ScalableMeshGTestUtil::GetUserSMTempDir();

        //if folder exists delete what we have inside
        if (BeFileName::DoesPathExist(tempPath.c_str()))
            {
            BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(tempPath.c_str());
            EXPECT_EQ(status == BeFileNameStatus::Success, true);
            }
        }
    virtual void TearDown() 
        {
        BeFileName test;
#ifdef VANCOUVER_API
        BeFileName::BeGetTempPath(test);
#else
        Desktop::FileSystem::BeGetTempPath(test);
#endif
        BeFileName tempPath = ScalableMeshGTestUtil::GetUserSMTempDir();
        if (test != tempPath && BeFileName::DoesPathExist(tempPath.c_str()))
            {
            BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(tempPath.c_str());

            if (status != BeFileNameStatus::Success)
                {
                assert(!"Error while removing 3dtiles in temp folder");
                }
            }
        }
    };

::testing::Environment* const sm_env = ::testing::AddGlobalTestEnvironment(new ScalableMeshEnvironment);

class ScalableMeshTest : public ::testing::TestWithParam<BeFileName>
    {
    protected:
        BeFileName m_filename;

    public:
        virtual void SetUp() { m_filename = GetParam(); }
        virtual void TearDown() { }
        BeFileName GetFileName() { return m_filename; }
        ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }

        ScalableMesh::IScalableMeshPtr OpenMesh()
            {
            StatusInt status;
            ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(m_filename, true, true, status);
            BeAssert(status == SUCCESS);
            return myScalableMesh;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTest, CanOpen)
    {
    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";

    EXPECT_EQ(OpenMesh().IsValid(), true ) << "\n Error opening "<< typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTest, CanGetRootNode)
    {
    auto myScalableMesh = OpenMesh();
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto rootNode = myScalableMesh->GetRootNode();

    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";

    EXPECT_EQ(rootNode.IsValid(), true) << "\n Error getting root node for " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTest, HasCoherentMeshFormat)
    {
    auto myScalableMesh = OpenMesh();
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    EXPECT_EQ((myScalableMesh->IsCesium3DTiles() && ScalableMeshGTestUtil::SMMeshType::TYPE_3DTILES == GetType())
        || ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType(), true) << "\n Incoherence found for " << GetFileName().c_str() << std::endl << std::endl;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTest, CanGenerate3DTiles)
    {
    auto myScalableMesh = OpenMesh();
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Skip 3dtiles
    if ((myScalableMesh->IsCesium3DTiles() && ScalableMeshGTestUtil::SMMeshType::TYPE_3DTILES == GetType()))
        return;

    auto filename = BeFileName::GetFileNameWithoutExtension(GetFileName());

    BeFileName tempPath = ScalableMeshGTestUtil::GetUserSMTempDir();
    tempPath.AppendToPath(L"3dtiles");
    tempPath.AppendToPath(filename.c_str());
    tempPath.AppendSeparator();

    BeFileNameStatus statusFile = BeFileName::CreateNewDirectory(tempPath.c_str());
    ASSERT_EQ(statusFile == BeFileNameStatus::Success || statusFile == BeFileNameStatus::AlreadyExists, true);

    auto ret = myScalableMesh->Generate3DTiles(tempPath);

    EXPECT_EQ(SUCCESS == ret, true) << "\n Could not convert 3sm to 3dtiles: " << GetFileName().c_str() << std::endl << std::endl;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTest, ::testing::ValuesIn(ScalableMeshGTestUtil::GetFiles(BeFileName(SM_DATA_PATH))));

// Wrap Google's ASSERT_TRUE macro into a lambda because it returns a "success" error code.
// When calling from main function, we actually want to return an error.
#define FAIL_IF_FALSE(condition) \
    { \
    bool isTrue = condition; \
    [&]() { ASSERT_TRUE(isTrue); }(); \
    if (!(isTrue)) return 1; \
    }


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Richard.Bois      10/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
//int main(int argc, char **argv) 
//    {
//    ::testing::InitGoogleTest(&argc, argv);
//
//    // Check that dataset path is valid
//    BeFileName dataPath(SM_DATA_PATH);
//
//    FAIL_IF_FALSE((ScalableMeshGTestUtil::GetDataPath(dataPath)));
//
//    FAIL_IF_FALSE((ScalableMeshGTestUtil::InitScalableMesh()));
//
//    BeFileName tempPath = ScalableMeshGTestUtil::GetUserSMTempDir();
//
//    //if folder exists delete what we have inside
//    if (BeFileName::DoesPathExist(tempPath.c_str()))
//        {
//        BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(tempPath.c_str());
//        EXPECT_EQ(status == BeFileNameStatus::Success, true);
//        }
//
//    auto retCode = RUN_ALL_TESTS();
//
//    // clean up
//    BeFileName test;
//    Desktop::FileSystem::BeGetTempPath(test);
//    //BeFileName::BeGetTempPath(test);
//    if (test != tempPath && BeFileName::DoesPathExist(tempPath.c_str()))
//        {
//        BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(tempPath.c_str());
//
//        if (status != BeFileNameStatus::Success)
//            {
//            assert(!"Error while removing 3dtiles in temp folder");
//            }
//        }
//
//    return retCode;
//    }