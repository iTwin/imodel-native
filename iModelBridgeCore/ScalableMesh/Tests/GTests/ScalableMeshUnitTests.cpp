/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include "SMUnitTestUtil.h"
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshSaveAs.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <random>
#include <TerrainModel/Core/IDTM.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/ClipVector.h>



#include "SMUnitTestDisplayQuery.h"
#include "../../STM/Edits/ClipUtilities.h"
#ifndef VANCOUVER_API
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#else
#include <DgnPlatform/Tools/ConfigurationManager.h>
#endif
#include <queue>

#ifdef VANCOUVER_API
#define DELETE_FILE(filenameP, removeReadOnlyAttribute) BeFileName::BeDeleteFile(filenameP, removeReadOnlyAttribute)
#else
#define DELETE_FILE(filenameP, removeReadOnlyAttribute) BeFileName::BeDeleteFile(filenameP)
#endif

USING_NAMESPACE_BENTLEY_TERRAINMODEL


#define BINGWKT L"PROJCS[\"Google Maps Global Mercator\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_2SP\"],PARAMETER[\"standard_parallel_1\",0],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1],EXTENSION[\"PROJ4\",\"+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs\"],AUTHORITY[\"EPSG\",\"900913\"]]"
#define NUM_CLIP_POINTS 21

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
    static ScalableMesh::IScalableMeshPtr OpenMesh(BeFileName filename, bool readOnly = true, bool shareable = true)
        {
        StatusInt status;
        ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(filename, readOnly, shareable, status);
        BeAssert(status == SUCCESS);
        return myScalableMesh;
        }

    static ScalableMesh::IScalableMeshNodePtr GetNonEmptyNode(ScalableMesh::IScalableMeshPtr sm, std::function<bool(IScalableMeshNodePtr)> predicate = [](IScalableMeshNodePtr) { return true; }, bool returnFirstNonEmptyNode = false)
        {
        bvector<IScalableMeshNodePtr> nodesList;
        std::function<bool(IScalableMeshNodePtr)> getNonEmptyNodes;
        getNonEmptyNodes = [&](IScalableMeshNodePtr node) -> bool
            {
            if (node->GetPointCount() > 0 && predicate(node))
                {
                nodesList.push_back(node);
                if (returnFirstNonEmptyNode)
                    return false;
                }
            for (auto child : node->GetChildrenNodes())
                if (!getNonEmptyNodes(child)) return false;
            return true;
            };
        getNonEmptyNodes(sm->GetRootNode());
        if(nodesList.empty()) return nullptr;
        if (returnFirstNonEmptyNode) return nodesList[0];
        std::random_device rd;
        std::default_random_engine e1(rd());
        std::uniform_int_distribution<uint64_t> index(0, nodesList.size() - 1);

        return nodesList[index(e1)];
        }
    };

class ScalableMeshTestWithParams : public ::testing::TestWithParam<BeFileName>
    {
    protected:
        BeFileName m_filename;

        void RemoveTempFiles()
            {
            BeFileName tempClipFile = ScalableMeshGTestUtil::GetTempPathFromProjectPath(m_filename);
            tempClipFile.append(L"_clips");


            BeFileName tempClipDefFile = ScalableMeshGTestUtil::GetTempPathFromProjectPath(m_filename);
            tempClipDefFile.append(L"_clipDefinitions");

            if(BeFileName::DoesPathExist(tempClipFile.c_str()))
                {
                ASSERT_TRUE(BeFileNameStatus::Success == DELETE_FILE(tempClipFile.c_str(), true));
                }
            if(BeFileName::DoesPathExist(tempClipDefFile.c_str()))
                {
                ASSERT_TRUE(BeFileNameStatus::Success == DELETE_FILE(tempClipDefFile.c_str(), true));
                }
            }
    public:
        virtual void SetUp()
            {
            m_filename = GetParam();

            //remove any existing 3sm clip files for this filename from the temp folder
            RemoveTempFiles();
            }

        virtual void TearDown()
            {
            //remove the 3sm clip files for this filename from the temp folder
            RemoveTempFiles();
            }
        BeFileName GetFileName() { return m_filename; }
        ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }

        void CreateClipFromRangeHelper(bvector<DPoint3d>& vec, const DRange3d& range)
            {
            std::random_device rd;

            std::default_random_engine e1(rd());
            std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
            std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

            for (size_t i = 0; i < NUM_CLIP_POINTS; ++i)
                {
                DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
                vec.push_back(pt);
                }
            };

        void CreateSimpleClipFromRangeHelper(bvector<DPoint3d>& vec, const DRange3d& range)
            {
            std::random_device rd;

            std::default_random_engine e1(rd());
            std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
            std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

            for (size_t i = 0; i < 2; ++i)
                {
                DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
                vec.push_back(pt);
                }
            DRange3d clipRange = DRange3d::From(vec.data(), (int)vec.size());

            vec.clear();
            vec.push_back(DPoint3d::From(clipRange.low.x, clipRange.low.y, clipRange.high.z));
            vec.push_back(DPoint3d::From(clipRange.low.x, clipRange.high.y, clipRange.high.z));
            vec.push_back(clipRange.high);
            vec.push_back(DPoint3d::From(clipRange.high.x, clipRange.low.y, clipRange.high.z)); 
            vec.push_back(DPoint3d::From(clipRange.low.x, clipRange.low.y, clipRange.high.z));
            }

        void SetReprojection(IScalableMeshPtr myScalableMesh, bool& status)
            {
            status = false;
            Transform tr = myScalableMesh->GetReprojectionTransform();
#ifdef VANCOUVER_API
            if (tr.IsIdentity())
                {
                // Reproject ECEF mesh using Bing maps GCS because they can't be clipped without being reprojected
                WString bingWKT(BINGWKT);
                auto bingGCS = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS();

                StatusInt warningStat;
                WString warningMessageString;

                bingGCS->InitFromWellKnownText(&warningStat,
                    &warningMessageString,
                    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown,
                    bingWKT.c_str());

                ASSERT_EQ(warningStat, SUCCESS);
                ASSERT_EQ(warningMessageString.size(), 0);

                DRange3d range;
                ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

                DPoint3d extent;
                extent.DifferenceOf(range.high, range.low);

                DgnGCSPtr  smGCS = GeoCoordinates::DgnGCS::CreateGCS(myScalableMesh->GetBaseGCS().get(), nullptr);
                DgnGCSPtr  targetGCS = GeoCoordinates::DgnGCS::CreateGCS(bingGCS.get(), nullptr);

                Transform       approxTransform;
                auto localTransformStatus = smGCS->GetLocalTransform(&approxTransform,
                    range.low,
                    &extent,
                    true/*doRotate*/, true/*doScale*/,
                    GeoCoordinates::GeoCoordInterpretation::XYZ,
                    *targetGCS);

                if (0 == localTransformStatus || 1 == localTransformStatus)
                    {
                    status = true;
                    myScalableMesh->SetReprojection(*bingGCS, approxTransform);
                    }
                }
#endif
            }
    };


class ScalableMeshGenerationTestWithParams : public ::testing::TestWithParam<BeFileName>
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
            BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr gcs = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS();
            Transform tr;
            tr.InitFrom(m_transform);
            myScalableMesh->SetReprojection(*gcs, tr);
        }
        return myScalableMesh;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanOpen)
    {
    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";
    EXPECT_EQ(ScalableMeshTest::OpenMesh(m_filename).IsValid(), true) << "\n Error opening " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanOpenReadOnly)
    {
    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";
    auto scalablemesh = ScalableMeshTest::OpenMesh(m_filename, true);
    ASSERT_EQ(scalablemesh.IsValid(), true);
    EXPECT_EQ(scalablemesh->IsReadOnly(), true) << "\n Error opening (read only) " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanOpenWrite)
    {
    auto scalablemesh = ScalableMeshTest::OpenMesh(m_filename, false);
    ASSERT_EQ(scalablemesh.IsValid(), true);
    if (ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType())
        EXPECT_EQ(scalablemesh->IsReadOnly(), false) << "\n Error opening (read only) 3sm: " << GetFileName().c_str() << std::endl << std::endl;
    else
        {
        // Cesium 3dtiles are always read only
        EXPECT_EQ(scalablemesh->IsReadOnly(), true) << "\n Error opening (read only) 3dTiles: " << GetFileName().c_str() << std::endl << std::endl;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, CanOpenShareable)
    {
    auto typeStr = ScalableMeshGTestUtil::SMMeshType::TYPE_3SM == GetType() ? L"3sm" : L"3dTiles";
    auto scalablemesh = ScalableMeshTest::OpenMesh(m_filename, true, true);
    ASSERT_EQ(scalablemesh.IsValid(), true);
    EXPECT_EQ(scalablemesh->IsShareable(), true) << "\n Error opening (read only) " << typeStr << ": " << GetFileName().c_str() << std::endl << std::endl;
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

    auto groundTruthGCS = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS();

    StatusInt warningStat;
    WString warningMessageString;

    groundTruthGCS->InitFromWellKnownText(&warningStat,
                                          &warningMessageString,
                                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown,
                                          WString(groundTruthInfo["WellKnownText"].asCString(), true).c_str());

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
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetQueryInterface)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    EXPECT_FALSE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_FULL_RESOLUTION).IsValid()); // Not implemented...
    EXPECT_FALSE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_EXTENTS_BY_PARTS).IsValid()); // Not implemented...
    EXPECT_FALSE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_ALL_POINTS_IN_EXTENT).IsValid()); // Not implemented...
    EXPECT_FALSE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_ALL_LINEARS_IN_EXTENT).IsValid()); // Not implemented...
    EXPECT_FALSE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_QTY).IsValid()); // Not implemented...

    EXPECT_TRUE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_VIEW_DEPENDENT).IsValid());
    EXPECT_TRUE(myScalableMesh->GetQueryInterface(ScalableMeshQueryType::SCM_QUERY_FIX_RESOLUTION_VIEW).IsValid());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetMeshQueryInterface)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);    

    EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION).IsValid());
    EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_VIEW_DEPENDENT).IsValid());
    EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_PLANE_INTERSECT).IsValid());
    EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_CONTEXT).IsValid());

    
    
    const GeoCoords::GCS smGcs(myScalableMesh->GetGCS());

    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr baseGcs(smGcs.GetGeoRef().GetBasePtr());

    if (baseGcs.IsValid())
        {
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr targetBaseGCSPtr(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"EPSG:900913"));
        ASSERT_EQ(targetBaseGCSPtr.IsValid(), true);

        DRange3d range;
        myScalableMesh->GetRange(range);

        GeoPoint latLong[2];                

        baseGcs->LatLongFromCartesian(latLong[0], range.low);
        baseGcs->LatLongFromCartesian(latLong[1], range.high);

        DRange3d rangeInTargetGcs;

        targetBaseGCSPtr->CartesianFromLatLong(rangeInTargetGcs.low, latLong[0]);
        targetBaseGCSPtr->CartesianFromLatLong(rangeInTargetGcs.high, latLong[1]);

        EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION, targetBaseGCSPtr, rangeInTargetGcs).IsValid());
        EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_VIEW_DEPENDENT, targetBaseGCSPtr, rangeInTargetGcs).IsValid());
        EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_PLANE_INTERSECT, targetBaseGCSPtr, rangeInTargetGcs).IsValid());
        EXPECT_TRUE(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_CONTEXT, targetBaseGCSPtr, rangeInTargetGcs).IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, MeshQuery)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshMeshQueryPtr meshQueryPtr(myScalableMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION));
   
    EXPECT_TRUE(meshQueryPtr.IsValid());

    IScalableMeshMeshQueryParamsPtr meshQueryParamPtr(IScalableMeshMeshQueryParams::CreateParams());

    EXPECT_TRUE(meshQueryParamPtr.IsValid());

    size_t levelInd = myScalableMesh->GetNbResolutions() - 1;
    meshQueryParamPtr->SetLevel(levelInd);

    EXPECT_TRUE(meshQueryParamPtr->GetLevel() == levelInd);

    const GeoCoords::GCS& gcs(myScalableMesh->GetGCS());
    
    if (gcs.GetGeoRef().GetBasePtr().IsValid())
        { 
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr sourceGcsPtr(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(*gcs.GetGeoRef().GetBasePtr().get()));
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr targetGcsPtr(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"EPSG:900913"));

        meshQueryParamPtr->SetGCS(sourceGcsPtr, targetGcsPtr);
                
        EXPECT_TRUE(meshQueryParamPtr->GetLevel() == levelInd);

        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr querySourceGcsPtr(meshQueryParamPtr->GetSourceGCS());
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr queryTargetGcsPtr(meshQueryParamPtr->GetTargetGCS());

        EXPECT_TRUE(sourceGcsPtr->IsEquivalent(*querySourceGcsPtr.get()));
        EXPECT_TRUE(targetGcsPtr->IsEquivalent(*queryTargetGcsPtr.get()));
        }
    
    bool useAllRes = meshQueryParamPtr->GetUseAllResolutions();
    meshQueryParamPtr->SetUseAllResolutions(!useAllRes);

    EXPECT_TRUE(useAllRes != meshQueryParamPtr->GetUseAllResolutions());        


    IScalableMeshMeshQueryParamsPtr meshQueryParamLowResPtr(IScalableMeshMeshQueryParams::CreateParams());

    meshQueryParamLowResPtr->SetLevel(0);

    DRange3d smRange; 
    myScalableMesh->GetRange(smRange);

    DPoint3d queryExtent[8];        
    smRange.Get8Corners(queryExtent);
    //queryExtent
    //const DPoint3d* pQueryExtentPts

    DPoint3d temp = queryExtent[3];
    queryExtent[3] = queryExtent[2];
    queryExtent[2] = temp;
    queryExtent[4] = queryExtent[0];

    IScalableMeshMeshPtr          meshPtr;
    bvector<IScalableMeshNodePtr> meshNodes;
        
    int status = meshQueryPtr->Query(meshPtr, queryExtent, 5, meshQueryParamLowResPtr);
    EXPECT_TRUE(status == SUCCESS);

    EXPECT_TRUE(meshPtr->GetNbPoints() > 0);
    EXPECT_TRUE(meshPtr->GetNbFaces() > 0);

    status = meshQueryPtr->Query(meshNodes, queryExtent, 5, meshQueryParamLowResPtr);
    EXPECT_TRUE(status == SUCCESS && meshNodes.size() > 0 || status != SUCCESS && meshNodes.size() == 0);


    bvector<IScalableMeshNodePtr> meshNodesPtr;
    
    ClipPrimitivePtr clipPrimitive(ClipPrimitive::CreateFromBlock(smRange.low, smRange.high, false, ClipMask::All, nullptr));
    ClipVectorPtr queryExtent3dClipPtr(ClipVector::CreateFromPrimitive(clipPrimitive.get()));

    status = meshQueryPtr->Query(meshNodesPtr, queryExtent3dClipPtr.get(), meshQueryParamLowResPtr);
    EXPECT_TRUE(status == SUCCESS && meshNodesPtr.size() > 0 || status != SUCCESS && meshNodesPtr.size() == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, SetGetReprojectionTransform)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr gcs = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS();
    Transform tr = Transform::FromRowValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    myScalableMesh->SetReprojection(*gcs, tr);

    Transform reprojectionTr = myScalableMesh->GetReprojectionTransform();

    EXPECT_TRUE(reprojectionTr.IsEqual(tr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, AddClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    uint64_t clipId = 0;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

    // Verify clip is added
    clipPoints.clear();
    myScalableMesh->GetClip(clipId, clipPoints);
    EXPECT_FALSE(clipPoints.empty());
    EXPECT_EQ(clipPoints.size(), NUM_CLIP_POINTS);

    //can't add twice the same ID
    ASSERT_FALSE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

    //can add twice the same data
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId + 1));

    //bvector<DPoint3d> clipDataEmpty;
    ASSERT_FALSE(myScalableMesh->AddClip(nullptr, 0, clipId)); //can't add empty data
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, RemoveClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    uint64_t clipId = 0;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

    myScalableMesh->RemoveClip(clipId);

    // Verify clip is removed
    clipPoints.clear();
    myScalableMesh->GetClip(clipId, clipPoints);
    EXPECT_TRUE(clipPoints.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, ModifyClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> originalClipPoints;
    CreateClipFromRangeHelper(originalClipPoints, range);

    uint64_t clipId = 0;
    ASSERT_TRUE(myScalableMesh->AddClip(originalClipPoints.data(), originalClipPoints.size(), clipId));

    // Change clip values
    bvector<DPoint3d> modifiedClipPoints;
    CreateClipFromRangeHelper(modifiedClipPoints, range);

    myScalableMesh->ModifyClip(modifiedClipPoints.data(), modifiedClipPoints.size(), clipId);

    // Verify change
    bvector<DPoint3d> newClipPoints;
    myScalableMesh->GetClip(clipId, newClipPoints);
    ASSERT_EQ(newClipPoints.size(), originalClipPoints.size());
    for (int i = 0; i < newClipPoints.size(); i++)
        {
        EXPECT_FALSE(newClipPoints[i].AlmostEqual(originalClipPoints[i]));
        EXPECT_TRUE(newClipPoints[i].AlmostEqual(modifiedClipPoints[i]));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, SynchronizeClipData)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    // We will create 2 clips and 1 skirt (associated to the first clip)
    uint64_t clipId1 = 1, clipId2 = 2, skirtId = clipId1;

    bvector<bvector<DPoint3d>> clips = { bvector<DPoint3d>(), bvector<DPoint3d>() };
    CreateClipFromRangeHelper(clips[0], range);
    CreateClipFromRangeHelper(clips[1], range);

    bvector<bpair<uint64_t, bvector<DPoint3d>>> listOfClips;
    listOfClips.push_back(make_bpair(clipId1, clips[0]));
    listOfClips.push_back(make_bpair(clipId2, clips[1]));
    
    bvector<bvector<DPoint3d>> skirt;
    skirt.push_back(clips[0]);
    skirt.push_back(clips[1]);
    bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>> listOfSkirts;
    listOfSkirts.push_back(make_bpair(skirtId, skirt));

    myScalableMesh->SynchronizeClipData(listOfClips, listOfSkirts);

    bvector<uint64_t> clipIds;
    myScalableMesh->GetAllClipIds(clipIds);

    EXPECT_FALSE(clipIds.empty());
    EXPECT_EQ(clipIds.size(), 2);
    EXPECT_EQ(clipIds[0], 1);
    EXPECT_EQ(clipIds[1], 2);

    skirt.clear();
    EXPECT_TRUE(myScalableMesh->GetSkirt(clipIds[0], skirt));

    for (int clip = 0; clip < clips.size(); clip++)
        {
        EXPECT_EQ(skirt[clip].size(), clips[clip].size());
        for (int i = 0; i < skirt[clip].size(); i++)
            {
            EXPECT_TRUE(skirt[clip][i].AlmostEqual(clips[clip][i]));
            }
        }

    skirt.clear();
    EXPECT_FALSE(myScalableMesh->GetSkirt(clipIds[1], skirt));
    EXPECT_TRUE(skirt.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, ShouldInvertClips)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    EXPECT_FALSE(myScalableMesh->ShouldInvertClips());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, SetInvertClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    ASSERT_FALSE(myScalableMesh->ShouldInvertClips());

    myScalableMesh->SetInvertClip(true);
    EXPECT_TRUE(myScalableMesh->ShouldInvertClips());

    myScalableMesh->SetInvertClip(false);
    EXPECT_FALSE(myScalableMesh->ShouldInvertClips());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, IsInsertingClips)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    EXPECT_FALSE(myScalableMesh->IsInsertingClips());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, SetIsInsertingClips)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    myScalableMesh->SetIsInsertingClips(true);

    EXPECT_TRUE(myScalableMesh->IsInsertingClips());

    myScalableMesh->SetIsInsertingClips(false);
    EXPECT_FALSE(myScalableMesh->IsInsertingClips());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, Node_HasClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto nodeToCheck = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

    auto range = nodeToCheck->GetContentExtent();
#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    uint64_t clipId = 0;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId)) << "Fails for Node ID: " << nodeToCheck->GetNodeId();

    DRange3d extent = DRange3d::From(&clipPoints[0], (int)clipPoints.size());

    ASSERT_TRUE((myScalableMesh->IsCesium3DTiles() && extent.IntersectsWith(nodeToCheck->GetContentExtent())) || extent.IntersectsWith(nodeToCheck->GetContentExtent(), 2)) << "Fails for Node ID: " << nodeToCheck->GetNodeId();
    EXPECT_TRUE(nodeToCheck->HasClip(clipId)) << "Fails for Node ID: " << nodeToCheck->GetNodeId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, Node_IsClippingUpToDate)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    std::function<void(IScalableMeshNodePtr)> checkNodesRecursive;
    checkNodesRecursive = [&](IScalableMeshNodePtr node)
        {
        ASSERT_TRUE(node->IsClippingUpToDate()) << "Fails for Node ID: " << node->GetNodeId();
        for (auto child : node->GetChildrenNodes()) checkNodesRecursive(child);
        };
    checkNodesRecursive(myScalableMesh->GetRootNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, Node_IsClippingUpToDateAfterAddClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto nodeToCheck = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

    auto range = nodeToCheck->GetContentExtent();

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    DRange3d extent = DRange3d::From(&clipPoints[0], (int)clipPoints.size());

    uint64_t clipId = 0;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId)) << "Fails for Node ID: "<< nodeToCheck->GetNodeId();

    ASSERT_TRUE((myScalableMesh->IsCesium3DTiles() && extent.IntersectsWith(nodeToCheck->GetContentExtent())) || extent.IntersectsWith(nodeToCheck->GetContentExtent(), 2)) << "Fails for Node ID: " << nodeToCheck->GetNodeId();
    EXPECT_FALSE(nodeToCheck->IsClippingUpToDate()) << "Fails for Node ID: " << nodeToCheck->GetNodeId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, Node_RefreshMergedClip)
    {
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    if (myScalableMesh->IsCesium3DTiles())
        {
        bool status;
        SetReprojection(myScalableMesh, status);
        if (status == false)
            {
            // Skip this dataset, it is not ECEF
            return;
            }
        }

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    uint64_t clipId = 1;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

    Transform reprojectionTr = myScalableMesh->GetReprojectionTransform();
    std::function<void(IScalableMeshNodePtr)> refreshMergedClipRecursive;
    refreshMergedClipRecursive = [&](IScalableMeshNodePtr node)
        {
        node->RefreshMergedClip(reprojectionTr);
        ASSERT_TRUE(node->IsClippingUpToDate()) << "Fails for Node ID: " << node->GetNodeId();
        for (auto child : node->GetChildrenNodes()) refreshMergedClipRecursive(child);
        };
    refreshMergedClipRecursive(myScalableMesh->GetRootNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestWithParams, ::testing::ValuesIn(ScalableMeshGTestUtil::GetFiles(BeFileName(SM_DATA_PATH))));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshGenerationTestWithParams, ::testing::ValuesIn(ScalableMeshGTestUtil::GetFiles(BeFileName(SM_DATA_SOURCE_PATH), true)));

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

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    Transform tr = Transform::FromIdentity();

    // Create clip
    auto clipPrimitive = ClipPrimitive::CreateFromBlock(range.low, range.high, false /*isMask*/, ClipMask::None, &tr, false /*invisible*/);

    auto clip = ClipVector::CreateFromPrimitive(clipPrimitive.get());

    auto ret = IScalableMeshSaveAs::DoSaveAs(myScalableMesh, destination, clip, nullptr/*progressSM*/, tr);
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

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    Transform tr = Transform::FromIdentity();

    // Create clip
    auto clipPrimitive = ClipPrimitive::CreateFromBlock(range.low, range.high, true /*isMask*/, ClipMask::None, &tr, false /*invisible*/);

    auto clip = ClipVector::CreateFromPrimitive(clipPrimitive.get());

    auto ret = IScalableMeshSaveAs::DoSaveAs(myScalableMesh, destination, clip, nullptr/*progressSM*/, tr);
    //auto ret = myScalableMesh->SaveAs(destination, clip, nullptr/*progressSM*/);

    EXPECT_EQ(SUCCESS == ret && BeFileName::DoesPathExist(destination.c_str()), true) << "\n Error saving to new 3sm" << std::endl << std::endl;

    // Cleanup
    ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(destination.c_str()));
    }

INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestDrapePoints, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfValues(BeFileName(SM_LISTING_FILE_NAME))));

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
    auto myScalableMesh = OpenMesh();
    for (size_t i = 0; i < GetData().size(); ++i)
    {
        DPoint3d sourcePt = GetData()[i];
        DPoint3d result = sourcePt;

        DPoint3d expectedResult = GetResult().front();
        for (auto&pt : GetResult())
            if (fabs(pt.x - sourcePt.x) < 1e-6 && fabs(pt.y - sourcePt.y)< 1e-6)
                expectedResult = pt;
        DVec3d direction = DVec3d::From(0, 0, -1);
        ASSERT_EQ(true, myScalableMesh->GetDTMInterface()->GetDTMDraping()->IntersectRay(result, direction, sourcePt));
        EXPECT_EQ(fabs(result.z - expectedResult.z) < 1e-6, true);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePoints, IntersectRayMultipleHits)
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
        DVec3d direction = DVec3d::From(0, 0, -1);
        bvector<TerrainModel::DTMRayIntersection> results;
        ASSERT_EQ(true, myScalableMesh->GetDTMInterface()->GetDTMDraping()->IntersectRay(results, direction, sourcePt));
        ASSERT_EQ(results.size(),1);
        ASSERT_TRUE(results[0].isOnMesh);
        EXPECT_EQ(fabs(results[0].point.z - expectedResult.z) < 1e-6, true);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Elenie.Godzaridis  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestDrapePoints, DrapeAlongVector)
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
        ASSERT_EQ(true, myScalableMesh->GetDTMInterface()->GetDTMDraping()->DrapeAlongVector(&result, nullptr, nullptr, nullptr, &drapedType, sourcePt, 0, 1e6));
        EXPECT_EQ(fabs(result.z - expectedResult.z) < 0.01, true);
        EXPECT_EQ(drapedType == 1 || drapedType == 3, true);
    }
}

#if 0

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
* @bsimethod                                                  Mathieu.St-Pierre   03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshGenerationTestWithParams, FromSourceCreation)
    {
    //m_filename.c_str()
    BeFileName newSmFileName = ScalableMeshGTestUtil::GetUserSMTempDir();

    BeFileNameStatus statusFile = BeFileName::CreateNewDirectory(newSmFileName.c_str());
    ASSERT_EQ(statusFile == BeFileNameStatus::Success || statusFile == BeFileNameStatus::AlreadyExists, true);

    newSmFileName = BeFileName((newSmFileName + WString(L"\\fromSourceCreation.3sm")).c_str());

    if (BeFileName::DoesPathExist(newSmFileName.c_str()))
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeDeleteFile(newSmFileName.c_str()));
    
    StatusInt status;
    IScalableMeshSourceCreatorPtr smCreatorPtr(IScalableMeshSourceCreator::GetFor(newSmFileName.c_str(), status));

    EXPECT_EQ(status == SUCCESS, true);    

    ScalableMesh::IDTMSourcePtr sourceP = ScalableMesh::IDTMLocalFileSource::Create(ScalableMesh::DTMSourceDataType::DTM_SOURCE_DATA_POINT, m_filename.c_str());        

    //Ensure that the source data is always meshed using 2.5D mesher.
    ScalableMesh::SourceImportConfig& sourceImportConfig = sourceP->EditConfig();
    ScalableMesh::Import::ScalableMeshData data = sourceImportConfig.GetReplacementSMData();
    data.SetRepresenting3dData(false);
    sourceImportConfig.SetReplacementSMData(data);

    smCreatorPtr->EditSources().Add(sourceP);
    smCreatorPtr->SaveToFile();
    SMStatus smStatus = smCreatorPtr->Create();

    EXPECT_EQ(smStatus == S_SUCCESS, true);

    smCreatorPtr = nullptr;

    IScalableMeshPtr smPtr(IScalableMesh::GetFor(newSmFileName.c_str(), false, true, status));    

    EXPECT_EQ(status == SUCCESS && smPtr.IsValid(), true);
    
    EXPECT_EQ(smPtr->InSynchWithSources(), true);
    EXPECT_EQ(smPtr->InSynchWithDataSources(), true);

    time_t lastSynchTime;

    smPtr->LastSynchronizationCheck(lastSynchTime);

    time_t currentTime;
    time(&currentTime);

    EXPECT_EQ(lastSynchTime <= currentTime, true);    
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
TEST_P(ScalableMeshTestWithParams, NodeRayQueryByDepth)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::From(0, 0, -1);
    params->SetDirection(direction);

    bvector<IScalableMeshNodePtr> nodes1;

    DRange3d range;
    myScalableMesh->GetRange(range);
    DPoint3d testPt = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.high.z+100);
    ptr->Query(nodes1, &testPt, NULL, 0, params);

    DRay3d toRange = DRay3d::FromOriginAndVector(testPt, direction);
    DRange1d fraction;
    DSegment3d segment;
    toRange.ClipToRange(nodes1.front()->GetContentExtent(), segment, fraction);

    params->SetDepth(fraction.low * 0.75);
    ASSERT_EQ(params->GetDepth(), fraction.low * 0.75);
    bvector<IScalableMeshNodePtr> nodes2;
    ptr->Query(nodes2, &testPt, NULL, 0, params);

    ASSERT_TRUE(nodes2.empty() || nodes2.front()->GetNodeId() != nodes1.front()->GetNodeId());
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
* @bsimethod                                               Elenie.Godzaridis     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQueryUnbounded)
    {
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::From(0, 0, -1);
    params->SetDirection(direction);

    bvector<IScalableMeshNodePtr> nodes;

    DRange3d range;
    myScalableMesh->GetRange(range);
    params->SetUseUnboundedRay(false);
    DPoint3d testPt = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.low.z -10);
    ptr->Query(nodes, &testPt, NULL, 0, params);

    ASSERT_TRUE(nodes.empty());
    params->SetUseUnboundedRay(true);

    ASSERT_TRUE(params->GetUseUnboundedRay());
    ptr->Query(nodes, &testPt, NULL, 0, params);

    ASSERT_TRUE(!nodes.empty());
    ASSERT_TRUE(nodes[0]->GetContentExtent().low.x <= testPt.x);
    ASSERT_TRUE(nodes[0]->GetContentExtent().low.y <= testPt.y);
    ASSERT_TRUE(nodes[0]->GetContentExtent().high.x >= testPt.x);
    ASSERT_TRUE(nodes[0]->GetContentExtent().high.y >= testPt.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, NodeRayQueryUsing2dProjectedRays)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    myScalableMesh->GetRange(range);

    DPoint3d pt1 = DPoint3d::From(range.low.x, range.low.y, range.high.z + 200);

    DPoint3d pt2 = DPoint3d::From(range.high.x, range.high.y, range.high.z + 200);

    IScalableMeshNodeRayQueryPtr ptr = myScalableMesh->GetNodeQueryInterface();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    DVec3d direction = DVec3d::FromStartEnd(pt1, pt2);
    params->SetDirection(direction);

    DVec3d testDirection = params->GetDirection();
    ASSERT_TRUE(testDirection.IsEqual(direction));

    bvector<IScalableMeshNodePtr> nodes;
    IScalableMeshNodePtr node = nullptr;

    ptr->Query(nodes, &pt1, NULL, 0, params);

    ASSERT_TRUE(nodes.empty()); //the whole 3d ray is outside the range...
    ASSERT_FALSE(params->Get2d());

    params->Set2d(true);
    ASSERT_TRUE(params->Get2d());

    ptr->Query(nodes, &pt1, NULL, 0, params);

    ASSERT_FALSE(nodes.empty()); //with the 2d query, we get some results
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, LoadMeshWithTexture)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);
    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

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
        ASSERT_GE(nodeP->GetPointCount(), mesh->GetNbPoints());

        flags = IScalableMeshMeshFlags::Create(false, false);
        mesh = nodeP->GetMesh(flags);
        ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() == nullptr);

        ASSERT_EQ(mesh->GetNbPoints(), mesh->GetPolyfaceQuery()->GetPointCount());
        ASSERT_EQ(mesh->GetNbFaces(), mesh->GetPolyfaceQuery()->GetPointIndexCount() / 3);
        ASSERT_GE(nodeP->GetPointCount(), mesh->GetNbPoints());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, LoadMeshWithGraph)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    //not supported for the cesium data
    if (myScalableMesh->IsCesium3DTiles())
        return;

    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(false, true);

    auto mesh = nodeP->GetMesh(flags);
    if (!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
        ASSERT_TRUE(mesh.IsValid());
    if (mesh.IsValid())
    {
        if (!myScalableMesh->IsTerrain())
            return;

        ASSERT_TRUE(flags->ShouldLoadGraph());
        bvector<DPoint3d> pts;
        ASSERT_EQ(mesh->GetBoundary(pts), DTM_SUCCESS);

        flags->SetLoadGraph(false);
        ASSERT_FALSE(flags->ShouldLoadGraph());
        mesh = nodeP->GetMesh(flags);
        ASSERT_EQ(mesh->GetBoundary(pts), DTM_ERROR);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Richard.Bois     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, LoadMeshWithClip)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    if (myScalableMesh->IsCesium3DTiles())
        {
        bool status;
        SetReprojection(myScalableMesh, status);
        if (status == false)
            {
            // Skip this dataset, it is not ECEF
            return;
            }
        }

    // Compute clip object from mesh range
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateSimpleClipFromRangeHelper(clipPoints, range);

    DRange3d clipRange = DRange3d::From(clipPoints);


    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh, [&] (IScalableMeshNodePtr nodeP)
                                                   {
                                                   return clipRange.IntersectsWith(nodeP->GetContentExtent());
                                                   });

    if(nodeP->GetPointCount() > 4)
        {

        Transform tr = Transform::FromIdentity();

        uint64_t clipId = 1;
        ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false, true);

        auto mesh = nodeP->GetMesh(flags);
        if(!nodeP->GetContentExtent().IsNull())
            ASSERT_TRUE(mesh.IsValid());
        if(mesh.IsValid())
            {
            if(nodeP->IsTextured())
                ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() != nullptr);
            else
                ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() == nullptr);

            ASSERT_EQ(mesh->GetNbPoints(), mesh->GetPolyfaceQuery()->GetPointCount());
            ASSERT_EQ(mesh->GetNbFaces(), mesh->GetPolyfaceQuery()->GetPointIndexCount() / 3);
            ASSERT_NE(nodeP->GetPointCount(), mesh->GetNbPoints());

            flags = IScalableMeshMeshFlags::Create(false, false);
            mesh = nodeP->GetMesh(flags);
            ASSERT_TRUE(mesh->GetPolyfaceQuery()->GetParamCP() == nullptr);

            ASSERT_EQ(mesh->GetNbPoints(), mesh->GetPolyfaceQuery()->GetPointCount());
            ASSERT_EQ(mesh->GetNbFaces(), mesh->GetPolyfaceQuery()->GetPointIndexCount() / 3);
            ASSERT_GE(nodeP->GetPointCount(), mesh->GetNbPoints());
            }
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Richard.Bois     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetAsBcDTM)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    if (myScalableMesh->IsTerrain())
        {

        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);
        auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);
        auto mesh = nodeP->GetMesh(flags);
        if (!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
            ASSERT_TRUE(mesh.IsValid());
        if (mesh.IsValid())
            {

            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr bcdtmP = nullptr;
            ASSERT_EQ(SUCCESS, mesh->GetAsBcDTM(bcdtmP));
            ASSERT_EQ(bcdtmP->GetTrianglesCount(), mesh->GetNbFaces());
            }
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Richard.Bois     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetTexture)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);
    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

    IScalableMeshTexturePtr textureP = nodeP->GetTexture();
    if (textureP.IsValid())
    {
        ASSERT_TRUE(textureP->GetSize() != 0);
        ASSERT_TRUE(textureP->GetData() != nullptr);
        ASSERT_EQ  (textureP->GetNOfChannels(), 3);  // Only support RGB for now...
        ASSERT_GT  (textureP->GetDimension().x, 0);  // width > 0
        ASSERT_GT  (textureP->GetDimension().y, 0);  // height > 0
    }
    else
    {
        ASSERT_FALSE(nodeP->IsTextured());
    }
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, AddSkirt)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    myScalableMesh->GetRange(range);
    range.ScaleAboutCenter(range,0.75);

    bvector<bvector<DPoint3d>> skirtData; 
    bvector<DPoint3d> vec;
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    for(size_t i =0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);

    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 242));
    //can't add twice the same ID
    ASSERT_FALSE(myScalableMesh->AddSkirt(skirtData, 242));

    //can add twice the same data
    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 245));

    bvector<bvector<DPoint3d>> skirtDataEmpty;
    ASSERT_FALSE(myScalableMesh->AddSkirt(skirtDataEmpty, 1)); //can't add empty data

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, ModifySkirt)
{

    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    myScalableMesh->GetRange(range);
    range.ScaleAboutCenter(range,0.75);

    bvector<bvector<DPoint3d>> skirtData;
    bvector<DPoint3d> vec;
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    for (size_t i = 0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);

    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 242));

    vec.clear();
    for (size_t i = 0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);
    ASSERT_TRUE(myScalableMesh->ModifySkirt(skirtData, 242)); //can modify skirts with existing IDs

    bvector<bvector<DPoint3d>> skirtDataEmpty;
    ASSERT_FALSE(myScalableMesh->ModifySkirt(skirtDataEmpty, 242)); //can't do an empty skirt, use delete for this

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, RemoveSkirt)
{

    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    myScalableMesh->GetRange(range);
    range.ScaleAboutCenter(range,0.75);

    bvector<bvector<DPoint3d>> skirtData;
    bvector<DPoint3d> vec;
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    for (size_t i = 0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);

    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 242));
    ASSERT_TRUE(myScalableMesh->RemoveSkirt(242));

    vec.clear();
    for (size_t i = 0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);
    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 242)); //after removing, can add again with the same id
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetAllClipIds)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    DRange3d range;
    myScalableMesh->GetRange(range);
    range.ScaleAboutCenter(range, 0.75);

    bvector<DPoint3d> clipData;
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    for (size_t i = 0; i < 15; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        clipData.push_back(pt);
    }

    clipData.push_back(clipData.front());
    bvector<uint64_t> ids;
    myScalableMesh->GetAllClipIds(ids);

    ASSERT_TRUE(ids.empty());
    myScalableMesh->AddClip(clipData.data(), clipData.size(), 222);
    myScalableMesh->GetAllClipIds(ids);

    ASSERT_TRUE(!ids.empty());
    ASSERT_TRUE(ids.front() == 222);
    ids.clear();

    myScalableMesh->AddClip(clipData.data(), clipData.size(), 245);
    myScalableMesh->GetAllClipIds(ids);
    ASSERT_TRUE(ids.size() == 2);

    ids.clear();
    myScalableMesh->RemoveClip(222);
    myScalableMesh->RemoveClip(245);
    myScalableMesh->GetAllClipIds(ids);
    ASSERT_TRUE(ids.empty());

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetSkirtMeshes)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    //Don't test clipping on un-reprojected Cesium datasets
    if (myScalableMesh->IsCesium3DTiles())
        {
        bool status;
        SetReprojection(myScalableMesh, status);
        if (status == false)
            {
            // Skip this dataset, it is not ECEF
            return;
            }
        }

    DRange3d range;
    myScalableMesh->GetRange(range);
    range.ScaleAboutCenter(range, 0.75);

    bvector<bvector<DPoint3d>> skirtData;
    bvector<DPoint3d> vec;
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    for (size_t i = 0; i < 21; ++i)
    {
        DPoint3d pt = DPoint3d::From(val_x(e1), val_y(e1), range.high.z);
        vec.push_back(pt);
    }

    skirtData.push_back(vec);
    myScalableMesh->AddClip(&vec[0], vec.size(), 242);
    ASSERT_TRUE(myScalableMesh->AddSkirt(skirtData, 242));
    bset<uint64_t> passedClips;
    passedClips.insert(242);

    bset<uint64_t> fakeClips;
    fakeClips.insert(1);

    std::queue<IScalableMeshNodePtr> allNodes;
    allNodes.push(myScalableMesh->GetRootNode());

    while(!allNodes.empty())
    {
        auto current = allNodes.front();
        allNodes.pop();
        bvector<PolyfaceHeaderPtr> meshes;

        for (auto& node : current->GetChildrenNodes())
            allNodes.push(node);
        if (current->GetPointCount() < 4) continue;
        if (!current->HasClip(242)) continue;

        current->GetSkirtMeshes(meshes, passedClips);
        //need to refresh clips
        ASSERT_TRUE(meshes.empty() || meshes.front()->GetPointIndexCount() == 0);

        Transform tr = myScalableMesh->GetReprojectionTransform();
        current->RefreshMergedClip(tr);

        meshes.clear();
        current->GetSkirtMeshes(meshes, passedClips);
        ASSERT_TRUE(!meshes.empty());
        for(auto& mesh: meshes)
        {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*mesh);
            bvector<int> &pointIndex = visitor->ClientPointIndex();
            for (visitor->Reset(); visitor->AdvanceToNextFace();)
            {
                DPoint3d tri[3] = { mesh->GetPointCP()[pointIndex[0]], mesh->GetPointCP()[pointIndex[1]], mesh->GetPointCP()[pointIndex[2]] };
                
                bool foundLine[3] = { false, false, false };
                //this checks that all points presents in the meshes project on the skirt lines in the XY plane
                for(size_t i =0; i < vec.size()-1; ++i)
                {
                    DSegment3d seg = DSegment3d::From(vec[i], vec[i + 1]);
                    for(size_t j =0; j < 3; ++j)
                    {
                        DPoint3d project;
                        double param;
                        if (!foundLine[j])
                        {
                            foundLine[j] = seg.ProjectPointXY(project, param, tri[j]) && param > -1e-4 &&
                                param < 1+1e-4 && abs(project.x - tri[j].x) < 1e-4 && abs(project.y - tri[j].y) < 1e-4;
                        }
                    }
                }
                ASSERT_TRUE(foundLine[0] && foundLine[1] && foundLine[2]);
            }
        }

        meshes.clear();
        current->GetSkirtMeshes(meshes, fakeClips);

        ASSERT_TRUE(meshes.empty());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetChildrenNodes)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodePtr nodeP = myScalableMesh->GetRootNode();

    if (myScalableMesh->GetNbResolutions() > 1)
    {
        auto nodes = nodeP->GetChildrenNodes();
        ASSERT_EQ(nodes.empty(), false);

        for (auto& nodeChild : nodes)
        {
            ASSERT_EQ(nodeChild->GetParentNode()->GetNodeId(), nodeP->GetNodeId());
            ASSERT_EQ(nodeChild->GetLevel(), 1);
        }

        std::queue<IScalableMeshNodePtr> nodeQueue;
        for (auto& nodeChild : nodes)
            nodeQueue.push(nodeChild);

        while (!nodeQueue.empty())
        {
            IScalableMeshNodePtr current = nodeQueue.front();
            nodeQueue.pop();
            bvector<IScalableMeshNodePtr> nodeNext = current->GetChildrenNodes();
            for (auto& n : nodeNext)
            {
                nodeQueue.push(n);
                ASSERT_EQ(n->GetLevel(), current->GetLevel() + 1);
            }
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetBcDTM)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    if (myScalableMesh->IsTerrain())
        {
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);
        auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);
        auto mesh = nodeP->GetMesh(flags);
        if (!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
            ASSERT_TRUE(mesh.IsValid());
        if (mesh.IsValid())
            {

            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr bcdtmP = nullptr;
            ASSERT_EQ(SUCCESS, mesh->GetAsBcDTM(bcdtmP));
            ASSERT_EQ(bcdtmP->GetTrianglesCount(), mesh->GetNbFaces());

            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr bcdtmP2 = nodeP->GetBcDTM();
            ASSERT_EQ(bcdtmP->GetTrianglesCount(), bcdtmP2->GetTrianglesCount());
            }
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetNeighborAt)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    IScalableMeshNodePtr nodeP = myScalableMesh->GetRootNode();
    if (myScalableMesh->GetNbResolutions() > 1)
    {
        auto nodes = nodeP->GetChildrenNodes();
        ASSERT_EQ(nodes.empty(), false);

        std::queue<IScalableMeshNodePtr> nodeQueue;
        for (auto& nodeChild : nodes)
            nodeQueue.push(nodeChild);

        while (!nodeQueue.empty())
        {
            IScalableMeshNodePtr current = nodeQueue.front();
            nodeQueue.pop();
            bvector<IScalableMeshNodePtr> nodeNext = current->GetChildrenNodes();
            for (auto& n : nodeNext)
            {
                nodeQueue.push(n);
            }
            bvector<IScalableMeshNodePtr> neighbor = current->GetNeighborAt(-1, 0, 0);

            DRange3d myExt = current->GetNodeExtent();
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.high.x <= myExt.low.x, true);
                ASSERT_EQ(range.high.y >= myExt.low.y && range.low.y <= myExt.high.y, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(1, 0, 0);
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.low.x >= myExt.high.x, true);
                ASSERT_EQ(range.high.y >= myExt.low.y && range.low.y <= myExt.high.y, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(0, -1, 0);
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.high.y <= myExt.low.y, true);
                ASSERT_EQ(range.high.x >= myExt.low.x && range.low.x <= myExt.high.x, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(0, 1, 0);
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.low.y >= myExt.high.y, true);
                ASSERT_EQ(range.high.x >= myExt.low.x && range.low.x <= myExt.high.x, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(1, 1, 0);
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.low.x >= myExt.high.x, true);
                ASSERT_EQ(range.low.y >= myExt.high.y, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(1, -1, 0);
            for (auto& n : neighbor)
            {
                DRange3d range = n->GetNodeExtent();
                ASSERT_EQ(range.low.x >= myExt.high.x, true);
                ASSERT_EQ(range.high.y <= myExt.low.y, true);
                ASSERT_EQ(range.high.z >= myExt.low.z && range.low.z <= myExt.high.z, true);
            }

            neighbor = current->GetNeighborAt(0, 0, 0);
            ASSERT_EQ(neighbor.empty(), true);

            neighbor = current->GetNeighborAt(127, 0, 127);
            ASSERT_EQ(neighbor.empty(), true);

            neighbor = current->GetNeighborAt(2, 0, 0);
            ASSERT_EQ(neighbor.empty(), true);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetTextureCompressed)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);
    IScalableMeshTexturePtr textureP = nodeP->GetTextureCompressed();
    if (textureP.IsValid())
    {
        ASSERT_TRUE(textureP->GetSize() != 0);
        ASSERT_TRUE(textureP->GetData() != nullptr);
    }
    else
    {
        ASSERT_FALSE(nodeP->IsTextured());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Elenie.Godzaridis     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestWithParams, GetMeshUnderClip)
{
    auto datasetName = Utf8String(BeFileName::GetFileNameWithoutExtension(m_filename.c_str()));

    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    //if (myScalableMesh->IsCesium3DTiles())
    //    {
    //    bool status;
    //    SetReprojection(myScalableMesh, status);
    //    if (status == false)
    //        {
    //        // Skip this dataset, it is not ECEF
    //        return;
    //        }
    //    }

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

    double maxZ = range.high.z;
#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    range.high.z = maxZ;
    bvector<DPoint3d> clipPoints;
    CreateSimpleClipFromRangeHelper(clipPoints, range);

    uint64_t clipId = 20;
    ASSERT_TRUE(myScalableMesh->AddClip(clipPoints.data(), clipPoints.size(), clipId));

    //load random node
    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh, [&](IScalableMeshNodePtr nodeP)
        {
        return nodeP->HasAnyClip();
        });
    if(nodeP != nullptr)
        {
        ASSERT_TRUE(nodeP != nullptr);
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(false, false);

        Transform tr = myScalableMesh->GetReprojectionTransform();
        nodeP->RefreshMergedClip(tr);
        auto mesh = nodeP->GetMeshUnderClip(flags, clipId);
        if(!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
            {
            ASSERT_TRUE(mesh.IsValid());
            for(size_t pt = 0; pt < mesh->GetPolyfaceQuery()->GetPointIndexCount(); ++pt)
                ASSERT_TRUE(range.IsContainedXY(mesh->GetPolyfaceQuery()->GetPointCP()[mesh->GetPolyfaceQuery()->GetPointIndexCP()[pt] - 1]));
            }
        }
    else
        {
        std::cerr << "[          ] Skipping data where clips has no effect on the mesh..." << std::endl;
        }
}

TEST_P(ScalableMeshTestWithParams, MeshClipperClipFilterModes3D)
{
    auto myScalableMesh = ScalableMeshTest::OpenMesh(m_filename);
    ASSERT_EQ(myScalableMesh.IsValid(), true);

    // Create clip from scalable mesh extent
    DRange3d range;
    ASSERT_EQ(DTM_SUCCESS, myScalableMesh->GetRange(range));

#ifdef VANCOUVER_API
    range.scaleAboutCenter(&range, 0.75);
#else
    range.ScaleAboutCenter(range, 0.75);
#endif

    bvector<DPoint3d> clipPoints;
    CreateClipFromRangeHelper(clipPoints, range);

    bvector<uint64_t> ids;
    ids.push_back(1);
    bvector<bvector<DPoint3d>> polygons;
    polygons.push_back(clipPoints);

    //load random node
    auto nodeP = ScalableMeshTest::GetNonEmptyNode(myScalableMesh);

    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);

    auto mesh = nodeP->GetMesh(flags);
    if (!nodeP->GetContentExtent().IsNull() && nodeP->GetPointCount() > 4)
        ASSERT_TRUE(mesh.IsValid());
    if (mesh.IsValid())
    {
        MeshClipper m;
        m.SetSourceMesh(mesh->GetPolyfaceQuery());
        m.SetClipGeometry(ids, polygons);

        EXPECT_TRUE(m.IsClipMask(1));
        EXPECT_FALSE(m.WasClipped());
    }
}

//// Wrap Google's ASSERT_TRUE macro into a lambda because it returns a "success" error code.
//// When calling from main function, we actually want to return an error.
//#define FAIL_IF_FALSE(condition) \
//    { \
//    bool isTrue = condition; \
//    [&]() { ASSERT_TRUE(isTrue); }(); \
//    if (!(isTrue)) return 1; \
//    }


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

