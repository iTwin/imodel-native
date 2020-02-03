/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifdef WIN32

#include "SMUnitTestGroundDetection.h"
#include <Bentley/BeThread.h>
#include <ScalableMesh/IScalableMeshGroundExtractor.h>
#include <ScalableMesh/IScalableMeshDetectGround.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertUorPointsToDestUnit(bvector<DPoint3d>& regionPointsMeter, const bvector<DPoint3d>& regionPoints, Transform trans)
{
    for (int i = 0; i< regionPoints.size(); i++)
    {
        DPoint3d temp;
        trans.Multiply(temp, regionPoints[i]);
        regionPointsMeter.push_back(temp);
    }
}



void CreateBreaklines(BeFileNameCR extraLinearFeatureAbsFileName, bvector<DPoint3d> const& closedPolygonPoints, ScalableMesh::IScalableMeshPtr& scalableMeshModel, Transform& uorToDestUnits)
    {
    TerrainModel::DTMPtr dtm(scalableMeshModel->GetDTMInterface(DTMAnalysisType::RawDataOnly));

    TerrainModel::BcDTMPtr bcDtmPtr(TerrainModel::BcDTM::Create());
    TerrainModel::DTMDrapedLinePtr drapedLine;
    TerrainModel::IDTMDraping* draping = dtm->GetDTMDraping();
    bool hasAddedBreaklines = false;

    for (size_t segmentInd = 0; segmentInd < closedPolygonPoints.size() - 1; segmentInd++)
        {
        DTMStatusInt status = draping->DrapeLinear(drapedLine, &closedPolygonPoints[segmentInd], 2);
        ASSERT_TRUE(status == DTMStatusInt::DTM_SUCCESS);

        bvector<DPoint3d> breaklinePts;

        for (size_t ptInd = 0; ptInd < drapedLine->GetPointCount(); ptInd++)
            {
            DPoint3d pt;
            double distance;
            DTMDrapedLineCode code;

            status = drapedLine->GetPointByIndex(pt, &distance, &code, (int)ptInd);
            ASSERT_TRUE(status == SUCCESS);
            breaklinePts.push_back(pt);
            }

        if (breaklinePts.empty())
            continue;

        DTMFeatureId featureId;

        bvector<DPoint3d> breaklinePtsMeter;
        ConvertUorPointsToDestUnit(breaklinePtsMeter, breaklinePts, uorToDestUnits);

        status = bcDtmPtr->AddLinearFeature(DTMFeatureType::Breakline, &breaklinePtsMeter[0], (int)breaklinePtsMeter.size(), &featureId);
        ASSERT_TRUE(status == DTMStatusInt::DTM_SUCCESS);
        hasAddedBreaklines = true;
        }

    if (hasAddedBreaklines)
        {
        DTMStatusInt status = bcDtmPtr->SaveAsGeopakDat(extraLinearFeatureAbsFileName.c_str());
        ASSERT_TRUE(status == DTMStatusInt::DTM_SUCCESS);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionTester::GroundDetectionTester()
    {    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionTester::~GroundDetectionTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetectionTester::DoDetection()
    {    
    BeFileName outputDir;
#ifdef VANCOUVER_API
    BeFileNameStatus fileNameStatus = BeFileName::BeGetTempPath(outputDir);
#else
        BeFileNameStatus fileNameStatus = Desktop::FileSystem::BeGetTempPath(outputDir);
#endif

    ASSERT_TRUE(BeFileNameStatus::Success == fileNameStatus);

    m_smPtr->SetEditFilesBasePath(Utf8String(outputDir.c_str()));

    Utf8String editBasePath(m_smPtr->GetEditFilesBasePath());    
    EXPECT_EQ(editBasePath.CompareTo(Utf8String(outputDir.c_str())) == 0, true);
            
    uint64_t groundId = 21;

    WPrintfString groundOutputName(L"%s\\groundArea%i.3sm", outputDir.c_str(), groundId);

    if (BeFileName::DoesPathExist(groundOutputName.c_str()))
        {
#ifdef VANCOUVER_API
        fileNameStatus = BeFileName::BeDeleteFile(groundOutputName.c_str(), true);
#else
        fileNameStatus = BeFileName::BeDeleteFile(groundOutputName.c_str());
#endif
        ASSERT_TRUE(BeFileNameStatus::Success == fileNameStatus);
        }
    
    BeFileName featureFilePath(outputDir.c_str());

    BeFileName textureSubFolderName;
    BeFileName extraLinearFeatureFileName;

    IScalableMeshGroundExtractor::GetTempDataLocation(textureSubFolderName, extraLinearFeatureFileName);
    featureFilePath.AppendString(extraLinearFeatureFileName.c_str());

    double ratioToGcsUnit = 1.0 / m_smPtr->GetGCS().GetUnit().GetRatioToBase();

    Transform computedTransform = Transform::FromRowValues(ratioToGcsUnit, 0, 0, 0,
                                                           0, ratioToGcsUnit, 0, 0,
                                                           0, 0, ratioToGcsUnit, 0);


    CreateBreaklines(featureFilePath, m_groundArea, m_smPtr, computedTransform);

    IScalableMeshGroundPreviewerPtr pSmQuickGroundPreviewer;
    GeoCoordinates::BaseGCSCPtr destinationGcs;
    BeFileName groundOutputFileName(BeFileName::GetFileNameWithoutExtension(groundOutputName.c_str()).c_str());
    BeFileName outputDirName(outputDir.c_str());
    
    SMStatus status = IScalableMeshDetectGround::DetectGroundForRegion(m_smPtr, groundOutputFileName, outputDirName, m_groundArea, groundId, pSmQuickGroundPreviewer, destinationGcs, true, true, featureFilePath);
    EXPECT_EQ(status == S_SUCCESS, true);    
    
    Utf8String coverageName(groundOutputName.c_str());

    BentleyStatus createStatus = m_smPtr->CreateCoverage(m_groundArea, groundId, coverageName);
    EXPECT_EQ(createStatus == SUCCESS, true);
    
    StatusInt groundOpenStatus;
    IScalableMeshPtr smGroundPtr(IScalableMesh::GetFor(groundOutputFileName.c_str(), true, true, groundOpenStatus));

    EXPECT_EQ(smGroundPtr.IsValid(), true);
    EXPECT_EQ(smGroundPtr->GetPointCount() == m_expectedGroundPts, true);
    
    bvector<bvector<DPoint3d>> coverageData;
    
    m_smPtr->GetAllCoverages(coverageData);
    
    EXPECT_EQ(coverageData.size() == 1, true);
    EXPECT_EQ(coverageData[0].size() == m_groundArea.size(), true);

    bvector<uint64_t> ids;

    m_smPtr->GetCoverageIds(ids);

    EXPECT_EQ(ids.size() == 1, true);
    EXPECT_EQ(ids[0] == groundId, true);   
            
    IScalableMeshPtr smGroupPtr(m_smPtr->GetGroup());

    EXPECT_EQ(smGroupPtr.IsValid(), false);

    m_smPtr->AddToGroup(smGroundPtr);

    smGroupPtr = m_smPtr->GetGroup();

    EXPECT_EQ(smGroupPtr.IsValid(), true);

    m_smPtr->RemoveFromGroup(smGroundPtr);

    smGroupPtr = m_smPtr->GetGroup();

    EXPECT_EQ(smGroupPtr.IsValid(), true);

    BentleyStatus deleteCoverageStatus = m_smPtr->DeleteCoverage(groundId);
    EXPECT_EQ(deleteCoverageStatus == SUCCESS, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroundDetectionTester::SetQueryParams(const BeFileName& smFileName, const bvector<DPoint3d>& groundArea, uint64_t expectedGroundPts)
    {
    StatusInt status;
    m_smPtr = ScalableMesh::IScalableMesh::GetFor(smFileName, true, true, status);
            
    if (m_smPtr == nullptr)
        return false;

    m_groundArea = groundArea;
    m_expectedGroundPts = expectedGroundPts;
         
    return true;        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mathieu.St-Pierre   11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ScalableMeshTestGroundDetection, ExtractGround)
    {
    GroundDetectionTester groundDetectionTester;

    bool result = groundDetectionTester.SetQueryParams(GetFileName(), GetGroundArea(), GetExpectedGroundPts());

    EXPECT_EQ(result == true, true);

    if (result)
        groundDetectionTester.DoDetection();
    }


INSTANTIATE_TEST_CASE_P(ScalableMesh, ScalableMeshTestGroundDetection, ::testing::ValuesIn(ScalableMeshGTestUtil::GetListOfGroundDetectionValues(BeFileName(SM_GROUND_DETECTION_TEST_CASES))));

#endif
