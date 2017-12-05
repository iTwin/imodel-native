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
#include <ScalableMesh/ScalableMeshDefs.h>
#include <TerrainModel/Core/IDTM.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/ClipVector.h>
#include "SMUnitTestDisplayQuery.h"

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

class ScalableMeshTest : public ::testing::Test
    {
    virtual void SetUp() 
        {

        ScalableMesh::ScalableMeshFilterType filterType = ScalableMesh::SCM_FILTER_DUMB_MESH;
        WChar filterTypeChar[10];
        
        swprintf(filterTypeChar, L"%i", filterType);
        
        BentleyStatus defineStatus = ConfigurationManager::DefineVariable(L"SM_FILTER_TYPE", filterTypeChar);
        
        BeAssert(defineStatus == SUCCESS);

        WString filterTypeStr;
        ASSERT_TRUE(BSISUCCESS == ConfigurationManager::GetVariable(filterTypeStr, L"SM_FILTER_TYPE"));
        ASSERT_TRUE(filterTypeStr == L"2");
        }
    virtual void TearDown() 
        {
        //auto err = GetLastError();
        //std::cout << "LastError = " << err << std::endl;
        }

    public:
    static ScalableMesh::IScalableMeshPtr OpenMesh(BeFileName filename)
        {
        StatusInt status;
        ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(filename, true, true, status);
        BeAssert(status == SUCCESS);
        return myScalableMesh;
        }
    };

class ScalableMeshTestWithParams : public ::testing::TestWithParam<BeFileName>
    {
    protected:
        BeFileName m_filename;

    public:
        virtual void SetUp() 
            {
            m_filename = GetParam(); 
            }
        virtual void TearDown() { }
        BeFileName GetFileName() { return m_filename; }
        ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }

    };


class ScalableMeshTestDrapePoints : public ::testing::TestWithParam<std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>>>
{
protected:
	BeFileName m_filename;
	DMatrix4d m_transform;
	bvector<DPoint3d> m_sourcePtOrLinearData;
	bvector<DPoint3d> m_expectedResult;

public:
	virtual void SetUp() { 
		auto paramList = GetParam(); 
		m_filename = std::get<0>(paramList);
		m_transform = std::get<1>(paramList);
		m_sourcePtOrLinearData = std::get<2>(paramList);
		m_expectedResult = std::get<3>(paramList);
	}
	virtual void TearDown() { }
	BeFileName GetFileName() { return m_filename; }
	bvector<DPoint3d>& GetData() { return m_sourcePtOrLinearData;  }
	bvector<DPoint3d>& GetResult() { return m_expectedResult; }
	ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }

	ScalableMesh::IScalableMeshPtr OpenMesh()
	{
		StatusInt status;
		ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(m_filename, true, true, status);
		BeAssert(status == SUCCESS);
		if (myScalableMesh != nullptr)
		{
			GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS();
			Transform tr;
			tr.InitFrom(m_transform);
			myScalableMesh->SetReprojection(*gcs, tr);
		}
		return myScalableMesh;
	}

};


class ScalableMeshTestDisplayQuery : public ::testing::TestWithParam<std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>>>
{
protected:
    BeFileName        m_filename;
    DMatrix4d         m_rootToViewMatrix;    
    bvector<DPoint4d> m_clipPlanes;
    bvector<double>   m_expectedResults;

public:

    virtual void SetUp() {
        auto paramList = GetParam();
        m_filename = std::get<0>(paramList);
        m_rootToViewMatrix = std::get<1>(paramList);
        m_clipPlanes = std::get<2>(paramList);
        m_expectedResults = std::get<3>(paramList);
    }

    virtual void TearDown() { }
    BeFileName GetFileName() { return m_filename; }
    const DMatrix4d& GetRootToViewMatrix() { return m_rootToViewMatrix; }
    const bvector<DPoint4d>& GetClipPlanes() { return m_clipPlanes; }
    bvector<double>& GetExpectedResults() { return m_expectedResults; }

    ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }


};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanOpen)
    {
    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";

    EXPECT_EQ(ScalableMeshTest::OpenMesh(m_filename).IsValid(), true ) << "\n Error opening "<< typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanGetRootNode)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto rootNode = myScalableMesh->GetRootNode();

    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";

    EXPECT_EQ(rootNode.IsValid(), true) << "\n Error getting root node for " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, HasCoherentMeshFormat)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    EXPECT_EQ((myScalableMesh->IsCesium3DTiles() && ScalableMeshGTestUtil::SMMeshType::TYPE_3DTILES == GetType())
        || ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType(), true) << "\n Incoherence found for " << GetFileName().c_str() << std::endl << std::endl;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanGenerate3DTiles)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
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
INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestWithParams, ::testing::ValuesIn(ScalableMeshGTestUtil::GetFiles(BeFileName(SM_DATA_PATH))));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshTest, SaveAsWithClipBoundary)
    {
    WString filterTypeStr;

    ASSERT_TRUE(BSISUCCESS == ConfigurationManager::GetVariable(filterTypeStr, L"SM_FILTER_TYPE"));

    WString testFile = L"SaltLakeCity_small.3sm";
    BeFileName filename(SM_DATA_PATH);
    ASSERT_TRUE(ScalableMeshGTestUtil::GetDataPath(filename));

    filename.AppendToPath(testFile.c_str());

    auto myScalableMesh = ScalableMeshTest::OpenMesh(filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    BeFileName destination = ScalableMeshGTestUtil::GetUserSMTempDir();
    destination.append(testFile.c_str());

    if (BeFileName::DoesPathExist(destination.c_str()))
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));

    //ScalableMeshGTestUtil::TestProgressListener progressListener;
    //auto progressSM = ScalableMesh::IScalableMeshProgress::Create(ScalableMesh::ScalableMeshProcessType::SAVEAS_3SM, myScalableMesh);
    //ASSERT_TRUE(progressSM->AddListener(progressListener));

    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

    range.scaleAboutCenter(&range, 0.75);

    Transform tr = Transform::FromIdentity();

    // Create clip
    auto clipPrimitive = DgnPlatform::ClipPrimitive::CreateFromBlock(range.low, range.high, false /*isMask*/, DgnPlatform::ClipMask::None, &tr, false /*invisible*/);

    auto clip = DgnPlatform::ClipVector::CreateFromPrimitive(clipPrimitive);

    auto ret = myScalableMesh->SaveAs(destination, clip, nullptr/*progressSM*/);

    EXPECT_EQ(SUCCESS == ret && BeFileName::DoesPathExist(destination.c_str()), true) << "\n Error saving to new 3sm" << std::endl << std::endl;

    // Cleanup
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshTest, SaveAsWithClipMask)
    {
    WString filterTypeStr;

    ASSERT_TRUE(BSISUCCESS == ConfigurationManager::GetVariable(filterTypeStr, L"SM_FILTER_TYPE"));

    WString testFile = L"SaltLakeCity_small.3sm";
    BeFileName filename(SM_DATA_PATH);
    ASSERT_TRUE(ScalableMeshGTestUtil::GetDataPath(filename));

    filename.AppendToPath(testFile.c_str());

    auto myScalableMesh = ScalableMeshTest::OpenMesh(filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    BeFileName destination = ScalableMeshGTestUtil::GetUserSMTempDir();
    destination.append(testFile.c_str());

    if (BeFileName::DoesPathExist(destination.c_str()))
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));

    //ScalableMeshGTestUtil::TestProgressListener progressListener;
    //auto progressSM = ScalableMesh::IScalableMeshProgress::Create(ScalableMesh::ScalableMeshProcessType::SAVEAS_3SM, myScalableMesh);
    //ASSERT_TRUE(progressSM->AddListener(progressListener));

    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

    range.scaleAboutCenter(&range, 0.75);

    Transform tr = Transform::FromIdentity();

    // Create clip
    auto clipPrimitive = DgnPlatform::ClipPrimitive::CreateFromBlock(range.low, range.high, true /*isMask*/, DgnPlatform::ClipMask::None, &tr, false /*invisible*/);

    auto clip = DgnPlatform::ClipVector::CreateFromPrimitive(clipPrimitive);

    auto ret = myScalableMesh->SaveAs(destination, clip, nullptr/*progressSM*/);

    EXPECT_EQ(SUCCESS == ret && BeFileName::DoesPathExist(destination.c_str()), true) << "\n Error saving to new 3sm" << std::endl << std::endl;

    // Cleanup
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));
    }

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDrapePoints, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfValues(BeFileName(SM_LISTING_FILE_NAME))));

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDisplayQuery, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfDisplayQueryValues(BeFileName(SM_DISPLAY_QUERY_TEST_CASES))));

TEST_P(ScalableMeshTestDrapePoints, DrapeSinglePoint)
{
	auto myScalableMesh = OpenMesh();
	for (size_t i = 0; i < GetData().size(); ++i)
	{
		DPoint3d sourcePt = GetData()[i];
		DPoint3d result = sourcePt;

		DPoint3d expectedResult = GetResult().front();
		for (auto&pt : GetResult())
			if (fabs(pt.x - sourcePt.x) < 1e-6 && fabs(pt.y - sourcePt.y)< 1e-6)
				expectedResult = pt;
		int drapedType = 0;
		ASSERT_EQ(DTM_SUCCESS,myScalableMesh->GetDTMInterface()->GetDTMDraping()->DrapePoint(&result.z, nullptr, nullptr, nullptr, drapedType, sourcePt));
		EXPECT_EQ(fabs(result.z - expectedResult.z) < 1e-6, true);
		EXPECT_EQ(drapedType == 1 || drapedType == 3, true);
	}
}

TEST_P(ScalableMeshTestDrapePoints, DrapeLinear)
{   
	auto myScalableMesh = OpenMesh();
	bvector<DPoint3d> sourcePts = GetData();
	TerrainModel::DTMDrapedLinePtr result;
	ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetDTMInterface()->GetDTMDraping()->DrapeLinear(result, sourcePts.data(), (int)sourcePts.size()));
	ASSERT_EQ(result.IsValid(), true);
	for (size_t i = 0; i < GetResult().size(); ++i)
	{
		DPoint3d pt;
		result->GetPointByIndex(pt, nullptr, nullptr, (unsigned int)i);
		EXPECT_EQ(fabs(pt.z - GetResult()[i].z) < 1e-6, true);
		EXPECT_EQ(fabs(pt.x - GetResult()[i].x) < 1e-6, true);
		EXPECT_EQ(fabs(pt.y - GetResult()[i].y) < 1e-6, true);
	}
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mathieu.St-Pierre   11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDisplayQuery, ProgressiveQuery)
    {

    /*
    BeFileName GetFileName() { return m_filename; }
    const DMatrix4d& GetRootToViewMatrix() { return m_rootToViewMatrix; }
    const bvector<DPoint4d>& GetClipPlanes() { return m_clipPlanes; }
    bvector<double>& GetExpectedResults() { return m_expectedResults; }
    */


    //auto myScalableMesh = OpenMesh();

    /*
    bvector<DPoint3d> sourcePts = GetData();
    TerrainModel::DTMDrapedLinePtr result;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetDTMInterface()->GetDTMDraping()->DrapeLinear(result, sourcePts.data(), (int)sourcePts.size()));
    ASSERT_EQ(result.IsValid(), true);
    for (size_t i = 0; i < GetResult().size(); ++i)
    {
    DPoint3d pt;
    result->GetPointByIndex(pt, nullptr, nullptr, (unsigned int)i);
    EXPECT_EQ(fabs(pt.z - GetResult()[i].z) < 1e-6, true);
    EXPECT_EQ(fabs(pt.x - GetResult()[i].x) < 1e-6, true);
    EXPECT_EQ(fabs(pt.y - GetResult()[i].y) < 1e-6, true);
    }*/
    

    DisplayQueryTester queryTester;

    bool result = queryTester.SetQueryParams(GetFileName(), GetRootToViewMatrix(), GetClipPlanes(), GetExpectedResults());

    EXPECT_EQ(result == true, true);

    if (result)
        queryTester.DoQuery();
    }


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

