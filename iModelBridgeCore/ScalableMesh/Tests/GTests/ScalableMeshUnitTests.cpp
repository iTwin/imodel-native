/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/GTests/ScalableMeshUnitTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "SMUnitTestUtil.h"
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshSaveAs.h>
#include <TerrainModel/Core/IDTM.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/ClipVector.h>
#include "SMUnitTestDisplayQuery.h"

class ScalableMeshEnvironment : public ::testing::Environment
    {
    private:
        Json::Value m_groundTruth;

    public:

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

        // Setup test data ground truths
        dataPath.AppendToPath(L"ground_truths.json");
        m_groundTruth = ScalableMeshGTestUtil::GetGroundTruthJsonFile(dataPath);

        ASSERT_FALSE(m_groundTruth.isNull());
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

    const Json::Value& GetGroundTruthJsonFile()
        {
        return m_groundTruth;
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
        ScalableMesh::IScalableMeshPtr OpenMesh()
            {
            StatusInt status;
            ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(m_filename, true, true, status);
            BeAssert(status == SUCCESS);
            return myScalableMesh;
            }
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
    EXPECT_EQ(OpenMesh().IsValid(), true) << "\n Error opening " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
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

    EXPECT_EQ((myScalableMesh->IsCesium3DTiles() && ScalableMeshGTestUtil::SMMeshType::TYPE_3DTILES_TILESET == GetType())
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
    if ((myScalableMesh->IsCesium3DTiles() && ScalableMeshGTestUtil::SMMeshType::TYPE_3DTILES_TILESET == GetType()))
        return;

    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));
    auto const& smDataInfo = static_cast<ScalableMeshEnvironment*>(sm_env)->GetGroundTruthJsonFile();
    ASSERT_TRUE(smDataInfo.isMember(datasetName.c_str()));
    auto const& groundTruthInfo = smDataInfo[datasetName.c_str()];

    ASSERT_TRUE(groundTruthInfo.isMember("Converted3DTilesFileCount"));

    auto filename = BeFileName::GetFileNameWithoutExtension(GetFileName());

    BeFileName tempPath = ScalableMeshGTestUtil::GetUserSMTempDir();
    tempPath.AppendToPath(L"3dtiles");
    tempPath.AppendToPath(filename.c_str());
    tempPath.AppendSeparator();

    BeFileNameStatus statusFile = BeFileName::CreateNewDirectory(tempPath.c_str());
    ASSERT_EQ(statusFile == BeFileNameStatus::Success || statusFile == BeFileNameStatus::AlreadyExists, true);

    auto ret = IScalableMeshSaveAs::Generate3DTiles(myScalableMesh, tempPath, L"", SMCloudServerType::LocalDisk, nullptr, nullptr, (uint64_t)-1);
    //auto ret = myScalableMesh->Generate3DTiles(tempPath);

    ASSERT_TRUE(SUCCESS == ret);
    
    EXPECT_EQ(ScalableMeshGTestUtil::GetFileCount(tempPath), groundTruthInfo["Converted3DTilesFileCount"].asUInt64()) << "\n Could not convert 3sm to 3dtiles: " << GetFileName().c_str() << std::endl << std::endl;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, VerifyMeshInfo)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));
    auto const& smDataInfo = static_cast<ScalableMeshEnvironment*>(sm_env)->GetGroundTruthJsonFile();
    ASSERT_TRUE(smDataInfo.isMember(datasetName.c_str()));
    auto const& groundTruthInfo = smDataInfo[datasetName.c_str()];

    ASSERT_TRUE(groundTruthInfo.isMember("PointCount"));
    ASSERT_TRUE(groundTruthInfo.isMember("IsTerrain"));
    ASSERT_TRUE(groundTruthInfo.isMember("NbResolutions"));
    ASSERT_TRUE(groundTruthInfo.isMember("Range"));
    ASSERT_TRUE(groundTruthInfo["Range"].isMember("min"));
    ASSERT_TRUE(groundTruthInfo["Range"]["min"].isArray());
    ASSERT_EQ(groundTruthInfo["Range"]["min"].size(), 3);
    ASSERT_TRUE(groundTruthInfo["Range"].isMember("max"));
    ASSERT_TRUE(groundTruthInfo["Range"]["max"].isArray());
    ASSERT_EQ(groundTruthInfo["Range"]["max"].size(), 3);

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    ASSERT_TRUE(SUCCESS == myScalableMesh->GetRange(range));

    auto const& groundTruthRange = groundTruthInfo["Range"];

    DPoint3d groundTruthRangeMin = DPoint3d::From(groundTruthRange["min"][0].asDouble(), groundTruthRange["min"][1].asDouble(), groundTruthRange["min"][2].asDouble());
    DPoint3d groundTruthRangeMax = DPoint3d::From(groundTruthRange["max"][0].asDouble(), groundTruthRange["max"][1].asDouble(), groundTruthRange["max"][2].asDouble());

    EXPECT_EQ(myScalableMesh->GetPointCount(), groundTruthInfo["PointCount"].asUInt64());
    EXPECT_EQ(myScalableMesh->IsTerrain(), groundTruthInfo["IsTerrain"].asBool()); 
    EXPECT_EQ(myScalableMesh->GetNbResolutions(), groundTruthInfo["NbResolutions"].asUInt64());
    EXPECT_DOUBLE_EQ(range.low.x, groundTruthRangeMin.x);
    EXPECT_DOUBLE_EQ(range.low.y, groundTruthRangeMin.y);
    EXPECT_DOUBLE_EQ(range.low.z, groundTruthRangeMin.z);
    EXPECT_DOUBLE_EQ(range.high.x, groundTruthRangeMax.x);
    EXPECT_DOUBLE_EQ(range.high.y, groundTruthRangeMax.y);
    EXPECT_DOUBLE_EQ(range.high.z, groundTruthRangeMax.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetBoundary)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));
    auto const& smDataInfo = static_cast<ScalableMeshEnvironment*>(sm_env)->GetGroundTruthJsonFile();
    ASSERT_TRUE(smDataInfo.isMember(datasetName.c_str()));
    auto const& groundTruthInfo = smDataInfo[datasetName.c_str()];

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    if (myScalableMesh->IsCesium3DTiles())
        {
        ASSERT_FALSE(groundTruthInfo.isMember("Boundary"));

        // Skip, they do not contain the required mesh graph
        return;
        }
    ASSERT_TRUE(groundTruthInfo.isMember("Boundary"));

    auto const& groundTruthBoundary = groundTruthInfo["Boundary"];
    ASSERT_TRUE(groundTruthBoundary.isArray());

    bvector<DPoint3d> boundary;
    ASSERT_TRUE(SUCCESS == myScalableMesh->GetBoundary(boundary));

    EXPECT_FALSE(boundary.empty());
    ASSERT_EQ(boundary.size(), groundTruthBoundary.size() / 3);

    for (int i = 0; i < boundary.size(); i++)
        {
        EXPECT_DOUBLE_EQ(boundary[i].x, groundTruthBoundary[3 * i].asDouble());
        EXPECT_DOUBLE_EQ(boundary[i].y, groundTruthBoundary[3 * i + 1].asDouble());
        EXPECT_DOUBLE_EQ(boundary[i].z, groundTruthBoundary[3 * i + 2].asDouble());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetBaseGCS)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));
    auto const& smDataInfo = static_cast<ScalableMeshEnvironment*>(sm_env)->GetGroundTruthJsonFile();
    ASSERT_TRUE(smDataInfo.isMember(datasetName.c_str()));
    auto const& groundTruthInfo = smDataInfo[datasetName.c_str()];

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto const& baseGCS = myScalableMesh->GetBaseGCS();

    auto groundTruthGCS = GeoCoordinates::BaseGCS::CreateGCS();

    StatusInt warningStat;
    WString warningMessageString;

    groundTruthGCS->InitFromWellKnownText(&warningStat,
                                          &warningMessageString,
                                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown,
                                          WString(groundTruthInfo["WellKnownText"].asCString()).c_str());

    EXPECT_EQ(warningStat, SUCCESS);
    EXPECT_EQ(warningMessageString.size(), 0);

    if (groundTruthGCS->IsValid())
        {
        EXPECT_TRUE(baseGCS->IsValid() && baseGCS->IsEquivalent(*groundTruthGCS));
        }
    else
        EXPECT_FALSE(baseGCS.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, VerifyMeshNodeInfo)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));
    auto const& smDataInfo = static_cast<ScalableMeshEnvironment*>(sm_env)->GetGroundTruthJsonFile();
    ASSERT_TRUE(smDataInfo.isMember(datasetName.c_str()));
    auto const& groundTruthInfo = smDataInfo[datasetName.c_str()];

    ASSERT_TRUE(groundTruthInfo.isMember("RootNode"));
    auto const& groundTruthRoot = groundTruthInfo["RootNode"];

    ASSERT_TRUE(groundTruthRoot.isMember("IsTextured"));
    ASSERT_TRUE(groundTruthRoot.isMember("ArePointsFullResolution"));
    ASSERT_TRUE(groundTruthRoot.isMember("ArePoints3d"));
    ASSERT_TRUE(groundTruthRoot.isMember("NodeId"));
    ASSERT_TRUE(groundTruthRoot.isMember("Level"));
    ASSERT_TRUE(groundTruthRoot.isMember("PointCount"));

    ASSERT_TRUE(groundTruthRoot.isMember("Resolutions"));
    ASSERT_TRUE(groundTruthRoot["Resolutions"].isArray());
    ASSERT_EQ(groundTruthRoot["Resolutions"].size(), 2);

    ASSERT_TRUE(groundTruthRoot.isMember("NodeExtent"));
    ASSERT_TRUE(groundTruthRoot["NodeExtent"].isMember("min"));
    ASSERT_TRUE(groundTruthRoot["NodeExtent"]["min"].isArray());
    ASSERT_EQ(groundTruthRoot["NodeExtent"]["min"].size(), 3);
    ASSERT_TRUE(groundTruthRoot["NodeExtent"].isMember("max"));
    ASSERT_TRUE(groundTruthRoot["NodeExtent"]["max"].isArray());
    ASSERT_EQ(groundTruthRoot["NodeExtent"]["max"].size(), 3);

    ASSERT_TRUE(groundTruthRoot.isMember("ContentExtent"));
    ASSERT_TRUE(groundTruthRoot["ContentExtent"].isMember("min"));
    ASSERT_TRUE(groundTruthRoot["ContentExtent"]["min"].isArray());
    ASSERT_EQ(groundTruthRoot["ContentExtent"]["min"].size(), 3);
    ASSERT_TRUE(groundTruthRoot["ContentExtent"].isMember("max"));
    ASSERT_TRUE(groundTruthRoot["ContentExtent"]["max"].isArray());
    ASSERT_EQ(groundTruthRoot["ContentExtent"]["max"].size(), 3);

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto rootNode = myScalableMesh->GetRootNode();
    ASSERT_TRUE(rootNode.IsValid());

    // Geometric and texture resolutions
    float geoRes, texRes;
    rootNode->GetResolutions(geoRes, texRes);

    auto const& groundTruthResolutions = groundTruthRoot["Resolutions"];
    
    // Node extent
    DRange3d nodeExtent = rootNode->GetNodeExtent();

    auto const& groundTruthNodeExtent = groundTruthRoot["NodeExtent"];

    DPoint3d groundTruthNodeExtentMin = DPoint3d::From(groundTruthNodeExtent["min"][0].asDouble(), groundTruthNodeExtent["min"][1].asDouble(), groundTruthNodeExtent["min"][2].asDouble());
    DPoint3d groundTruthNodeExtentMax = DPoint3d::From(groundTruthNodeExtent["max"][0].asDouble(), groundTruthNodeExtent["max"][1].asDouble(), groundTruthNodeExtent["max"][2].asDouble());

    // Node content extent
    DRange3d nodeContentExtent = rootNode->GetContentExtent();

    auto const& groundTruthNodeContentExtent = groundTruthRoot["ContentExtent"];

    DPoint3d groundTruthNodeContentExtentMin = DPoint3d::From(groundTruthNodeContentExtent["min"][0].asDouble(), groundTruthNodeContentExtent["min"][1].asDouble(), groundTruthNodeContentExtent["min"][2].asDouble());
    DPoint3d groundTruthNodeContentExtentMax = DPoint3d::From(groundTruthNodeContentExtent["max"][0].asDouble(), groundTruthNodeContentExtent["max"][1].asDouble(), groundTruthNodeContentExtent["max"][2].asDouble());

    // Validation
    EXPECT_EQ(rootNode->IsTextured(), groundTruthRoot["IsTextured"].asBool());
    EXPECT_EQ(rootNode->ArePointsFullResolution(), groundTruthRoot["ArePointsFullResolution"].asBool());
    EXPECT_EQ(rootNode->ArePoints3d(), groundTruthRoot["ArePoints3d"].asBool());
    EXPECT_EQ(rootNode->GetNodeId(), groundTruthRoot["NodeId"].asUInt64());
    EXPECT_EQ(rootNode->GetLevel(), groundTruthRoot["Level"].asUInt64());
    EXPECT_EQ(rootNode->GetPointCount(), groundTruthRoot["PointCount"].asUInt64());
    EXPECT_FLOAT_EQ(geoRes, groundTruthResolutions[0].asFloat());
    EXPECT_FLOAT_EQ(texRes, groundTruthResolutions[1].asFloat());
    EXPECT_DOUBLE_EQ(nodeExtent.low.x, groundTruthNodeExtentMin.x);
    EXPECT_DOUBLE_EQ(nodeExtent.low.y, groundTruthNodeExtentMin.y);
    EXPECT_DOUBLE_EQ(nodeExtent.low.z, groundTruthNodeExtentMin.z);
    EXPECT_DOUBLE_EQ(nodeExtent.high.x, groundTruthNodeExtentMax.x);
    EXPECT_DOUBLE_EQ(nodeExtent.high.y, groundTruthNodeExtentMax.y);
    EXPECT_DOUBLE_EQ(nodeExtent.high.z, groundTruthNodeExtentMax.z);
    EXPECT_DOUBLE_EQ(nodeContentExtent.low.x, groundTruthNodeContentExtentMin.x);
    EXPECT_DOUBLE_EQ(nodeContentExtent.low.y, groundTruthNodeContentExtentMin.y);
    EXPECT_DOUBLE_EQ(nodeContentExtent.low.z, groundTruthNodeContentExtentMin.z);
    EXPECT_DOUBLE_EQ(nodeContentExtent.high.x, groundTruthNodeContentExtentMax.x);
    EXPECT_DOUBLE_EQ(nodeContentExtent.high.y, groundTruthNodeContentExtentMax.y);
    EXPECT_DOUBLE_EQ(nodeContentExtent.high.z, groundTruthNodeContentExtentMax.z);
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
    destination.AppendToPath(testFile.c_str());

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

    auto ret = IScalableMeshSaveAs::DoSaveAs(myScalableMesh, destination, clip, nullptr/*progressSM*/);
    //auto ret = myScalableMesh->SaveAs(destination, clip, nullptr/*progressSM*/);

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
    destination.AppendToPath(testFile.c_str());

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

    auto ret = IScalableMeshSaveAs::DoSaveAs(myScalableMesh, destination, clip, nullptr/*progressSM*/);
    //auto ret = myScalableMesh->SaveAs(destination, clip, nullptr/*progressSM*/);

    EXPECT_EQ(SUCCESS == ret && BeFileName::DoesPathExist(destination.c_str()), true) << "\n Error saving to new 3sm" << std::endl << std::endl;

    // Cleanup
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));
    }

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDrapePoints, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfValues(BeFileName(SM_LISTING_FILE_NAME))));

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDisplayQuery, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfDisplayQueryValues(BeFileName(SM_DISPLAY_QUERY_TEST_CASES))));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
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
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePoints, IntersectRay)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePoints, DrapeAlongVector)
{
}

#if 0
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestProjectPoints, ProjectPoint)
{
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePointsFast, DrapeSinglePoint)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePointsFast, DrapeLinear)
{
}
#endif



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



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetNodeQueryInterface)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    ASSERT_EQ(ptr.IsValid(), true);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQuerySingleNode)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::From(0, 0, -1);
    params->SetDirection(direction);

    IScalableMeshNodePtr node = nullptr;

    DRange3d range;
    myScalableMesh->GetRange(range);
    DPoint3d testPt = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.high.z);
    ptr->Query(node, &testPt, NULL, 0, params);

    ASSERT_TRUE(node != nullptr);
    ASSERT_TRUE(node->GetContentExtent().low.x <= testPt.x);
    ASSERT_TRUE(node->GetContentExtent().low.y <= testPt.y);
    ASSERT_TRUE(node->GetContentExtent().high.x >= testPt.x);
    ASSERT_TRUE(node->GetContentExtent().high.y >= testPt.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQueryMultipleNode)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::From(0, 0, -1);
    params->SetDirection(direction);

    DVec3d testDirection = params->GetDirection();
    ASSERT_TRUE(testDirection.IsEqual(direction));

    bvector<IScalableMeshNodePtr> nodes;
    IScalableMeshNodePtr node = nullptr;

    DRange3d range;
    myScalableMesh->GetRange(range);
    DPoint3d testPt = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.high.z);
    ptr->Query(nodes, &testPt, NULL, 0, params);

    ASSERT_TRUE(!nodes.empty());

    ptr->Query(node, &testPt, NULL, 0, params);
    ASSERT_TRUE(std::find_if(nodes.begin(), nodes.end(), [&node](const IScalableMeshNodePtr& val)
    {
        return val->GetNodeId() == node->GetNodeId();
    }) != nodes.end());

    nodes.clear();
    testPt = DPoint3d::From(range.high.x + range.XLength() / 2, range.high.y + range.YLength() / 2, range.high.z);
    ptr->Query(nodes, &testPt, NULL, 0, params);
    ASSERT_TRUE(nodes.empty());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQueryByLevel)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::From(0, 0, -1);
    params->SetDirection(direction);

    IScalableMeshNodePtr node = nullptr;

    DRange3d range;
    myScalableMesh->GetRange(range);
    DPoint3d testPt = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.high.z);

    for (size_t i = 0; i < myScalableMesh->GetNbResolutions(); ++i)
    {
        params->SetLevel(i);
        ptr->Query(node, &testPt, NULL, 0, params);
        EXPECT_TRUE(node->GetLevel() == i || (node->GetLevel() <= i && node->GetChildrenNodes().empty()));
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQueryUsing2dProjectedRays)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, LoadMeshWithTexture)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);
    IScalableMeshNodePtr nodeP = myScalableMesh->GetRootNode();

    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);

    auto mesh = nodeP->GetMesh(flags);
    if (!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
        ASSERT_TRUE(mesh.IsValid());
    if (mesh.IsValid())
    {
        if (nodeP->IsTextured())
            ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() != nullptr);
        else
            ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() == nullptr);

        ASSERT_EQ(mesh->GetNbPoints(), mesh->GetPolyfaceQuery()->GetPointCount());
        ASSERT_EQ(mesh->GetNbFaces(), mesh->GetPolyfaceQuery()->GetPointIndexCount() / 3);
        ASSERT_EQ(nodeP->GetPointCount(), mesh->GetNbPoints());

        flags = IScalableMeshMeshFlags::Create(false, false);
        mesh = nodeP->GetMesh(flags);
        ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() == nullptr);

        ASSERT_EQ(mesh->GetNbPoints(), mesh->GetPolyfaceQuery()->GetPointCount());
        ASSERT_EQ(mesh->GetNbFaces(), mesh->GetPolyfaceQuery()->GetPointIndexCount() / 3);
        ASSERT_EQ(nodeP->GetPointCount(), mesh->GetNbPoints());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, AddSkirt)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, ModifySkirt)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, RemoveSkirt)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetSkirtMeshes)
{
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

