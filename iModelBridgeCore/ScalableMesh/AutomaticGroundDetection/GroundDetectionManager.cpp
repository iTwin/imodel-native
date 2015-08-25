/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/GroundDetectionManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"

#include <DgnPlatform\Tools\MdlCnv.h>
#include <ScalableMesh\AutomaticGroundDetection\GroundDetectionManager.h>

#include "PointCloudEditGroundDetection.h"
#include "PointCloudQuadTree.h"
#include "PointCloudClassification.h"
#include <ScalableMesh\ScalableMeshLib.h>
#include <ScalableMesh\ScalableMeshAdmin.h>
#include "AutomaticGroundDetectionInternalConfig.h"
#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
#endif
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_LOGGING


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

const double GroundDetectionParameters::AUTODETECT_PARAMS = -1;

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard        11/2014
+===============+===============+===============+===============+===============+======*/
struct  DummyProgressListener : public IGroundDetectionProgressListener
    {
    DummyProgressListener()  {}
    ~DummyProgressListener() {}
    }; // DummyProgressListener
static DummyProgressListener  s_dummyProgressListener;//A do nothing progress listener


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReport::ProgressReport(IGroundDetectionProgressListenerP pProgressListener)
:m_currentStep(STATE_QTY),
m_workDone(0.0),
m_hasEstimatedTime(false),
m_estimatedTime(0.0),
m_pProgressListener(pProgressListener),
m_currentPhase(0),
m_totalNumberOfPhases(0)
    {
    if (NULL == m_pProgressListener)
        m_pProgressListener = &s_dummyProgressListener;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReport::ProgressReport() 
:m_currentStep(STATE_QTY), 
m_workDone(0.0),
m_hasEstimatedTime(false),
m_estimatedTime(0.0),
m_currentPhase(0),
m_totalNumberOfPhases(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReport::~ProgressReport()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProgressReport::SetCurrentPhase(int phaseNumber)                { m_currentPhase = phaseNumber; }
int     ProgressReport::GetCurrentPhase() const                         { return m_currentPhase; }
void    ProgressReport::SetTotalNumberOfPhases(int totalNumberOfPhases) { m_totalNumberOfPhases = totalNumberOfPhases; }
int     ProgressReport::GetTotalNumberOfPhases() const                  { return m_totalNumberOfPhases; }
void    ProgressReport::SetCurrentStep(ProgressReport::ProgressStateId step){ m_currentStep = step; }
ProgressReport::ProgressStateId ProgressReport::GetCurrentStep() const  { return m_currentStep; }
void    ProgressReport::SetWorkDone(double workDone)                    { m_workDone = workDone; }
double  ProgressReport::GetWorkDone() const                             { return m_workDone; }
double  ProgressReport::GetEstimatedRemainingTime() const               { return m_estimatedTime; }
bool    ProgressReport::HasEstimatedRemainingTime() const               { return m_hasEstimatedTime; }
void    ProgressReport::SetEstimatedRemainingTime(bool hasEstimatedTime, double estimatedTime)
    { 
    m_hasEstimatedTime = hasEstimatedTime; 
    m_estimatedTime = estimatedTime; 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ProgressReport::CheckContinueOnProgress()    { return _CheckContinueOnProgress (*this);}
bool    ProgressReport::CheckContinueOnLifeSignal()  { return _CheckContinueOnLifeSignal(); }
void    ProgressReport::OnSignalError()              { return _OnSignalError(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProgressReport::_CheckContinueOnProgress(ProgressReportCR report) 
    {
    return m_pProgressListener->_CheckContinueOnProgress(report);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProgressReport::_CheckContinueOnLifeSignal() 
    {
    return m_pProgressListener->_CheckContinueOnLifeSignal();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressReport::_OnSignalError() 
    {
    m_pProgressListener->_OnSignalError();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::NativeLogging::ILogger* GroundDetectionLogger::Get()
    {
    return Bentley::NativeLogging::LoggingManager::GetLogger(L"ScalableMesh.GroundDetection");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetectionLogger::OutputTimerToLogger(StopWatch& timer)
    {
    int days = (int) timer.GetElapsedSeconds() / 60 / 60 / 24;
    int hours = (int) (timer.GetElapsedSeconds() / 60 / 60) % 24;
    int minutes = (int) (timer.GetElapsedSeconds() / 60) % 60;
    int seconds = (int) timer.GetElapsedSeconds() % 60;
    GROUNDDLOG->tracev(L"%.2lf seconds (%ld days %ld h %ld min %ld s)", timer.GetElapsedSeconds(), days, hours, minutes, seconds);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionParametersPtr GroundDetectionParameters::Create()
    {
    return new GroundDetectionParameters();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionParametersPtr GroundDetectionParameters::Clone(GroundDetectionParametersCR input)
    {
    return new GroundDetectionParameters(input);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionParameters::GroundDetectionParameters()
:m_slopeThreshold(AUTODETECT_PARAMS),
m_heightThreshold(AUTODETECT_PARAMS),
m_largestStructSize(AUTODETECT_PARAMS),
m_useGPU(USE_NON_DISPLAY_GPU),
m_useMultiThread(false),
m_density(1.0),
m_sensitivityFactor(1.0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionParameters::GroundDetectionParameters(const GroundDetectionParameters& input)
:m_slopeThreshold(input.m_slopeThreshold),
m_heightThreshold(input.m_heightThreshold),
m_largestStructSize(input.m_largestStructSize),
m_useGPU(input.m_useGPU),
m_useMultiThread(input.m_useMultiThread),
m_density(input.m_density),
m_sensitivityFactor(input.m_sensitivityFactor)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionParameters::~GroundDetectionParameters()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool      GroundDetectionParameters::IsAutoDetect(double value)         { return value == AUTODETECT_PARAMS ;}
double    GroundDetectionParameters::GetSlopeThreshold() const          { return m_slopeThreshold ;}
void      GroundDetectionParameters::SetSlopeThreshold(double value)    { m_slopeThreshold = value;}
double    GroundDetectionParameters::GetHeightThreshold() const         { return m_heightThreshold; }
void      GroundDetectionParameters::SetHeightThreshold(double value)   { m_heightThreshold = value; }
double    GroundDetectionParameters::GetLargestStructureSize() const    { return m_largestStructSize; }
void      GroundDetectionParameters::SetLargestStructureSize(double value){ m_largestStructSize = value; }
float     GroundDetectionParameters::GetDensity() const                 { return m_density; }
void      GroundDetectionParameters::SetDensity(float value)            { m_density = value; }
double    GroundDetectionParameters::GetSensitivityFactor() const { return m_sensitivityFactor; }
void      GroundDetectionParameters::SetSensitivityFactor(double value) { m_sensitivityFactor = value; }

GroundDetectionParameters::ProcessingStrategy GroundDetectionParameters::GetProcessingStrategy() const { return m_useGPU; }
void      GroundDetectionParameters::SetProcessingStrategy(GroundDetectionParameters::ProcessingStrategy value) { m_useGPU = value; }

bool      GroundDetectionParameters::GetUseMultiThread() const          { return m_useMultiThread; }
void      GroundDetectionParameters::SetUseMultiThread(bool value)      { m_useMultiThread = true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetectionManager::DoGroundDetection(EditElementHandleR elHandle, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener)
    {
    StatusInt status(ERROR);
    PointCloudQuadTreePtr quadTree;

#ifndef DISABLELOGGER //Don't want logger during ATP
    if (GROUNDDLOG->isSeverityEnabled(LOG_TRACE))
        {
        IPointCloudFileQueryPtr pPCFile = IPointCloudFileQuery::CreateFileQuery(elHandle);
        if (NULL != pPCFile.get())
            {
            WString name(pPCFile->GetFileName());
            UInt32 num_clouds(pPCFile->GetNumberOfClouds());
            UInt64 num_points(pPCFile->GetNumberOfPoints());

            GROUNDDLOG->tracev(L"%ls", name.c_str());
            GROUNDDLOG->tracev(L"%ld points in %ld clouds", num_points, num_clouds);
            }
        }

    GROUNDDLOG->trace(L"START - DoGroundDetection");
    StopWatch t;
    t.Start();
#endif


    // Create channel for saving ground channel
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(elHandle);
    IPointCloudChannelPtr channel = pData->GetChannel();

    // Create default informations for quadtree

    MSElementCP element = elHandle.GetElementCP();
    DRange3d   rangeVecPtsCloud;    
    DataConvert::ScanRangeToDRange3d (rangeVecPtsCloud, element->hdr.dhdr.range);


    DPoint3d origin;
    DPoint3d corner;

    origin.x = rangeVecPtsCloud.low.x;
    origin.y = rangeVecPtsCloud.low.y;
    origin.z = rangeVecPtsCloud.low.z;

    corner.x = rangeVecPtsCloud.high.x;
    corner.y = rangeVecPtsCloud.high.y;
    corner.z = rangeVecPtsCloud.high.z;

    PointCloudQuadTreeData quadTreeData;
    DPoint3d boundingBox[2] = { origin, corner };
    quadTreeData.m_bb = boundingBox;
    quadTreeData.m_elHandle = &elHandle;    // We need an handle for query on points cloud.
    quadTreeData.m_sizeBound = (unsigned int)TreeBuildingParams::numberOfPointsPerQuery;       // Max points in tile
    quadTreeData.m_maxDataSize = (unsigned int)TreeBuildingParams::numberOfPointsInVortexBuffer;    // Max dataBuffer for vortex
    quadTreeData.m_maxDepth = 0;

    ProgressReport report(pProgressListener);
    report.SetCurrentStep(ProgressReport::STATE_CREATE_TREE);
    report.SetTotalNumberOfPhases(3);
    report.SetCurrentPhase(1);
    report.SetWorkDone(0.0); //starting
    if (!report.CheckContinueOnProgress())
        return ERROR;//User abort

    // Create quadTree with some informations
    // Create seeds Seeds
    std::vector<QuadSeedPtr> seeds;
#ifdef SCALABLE_MESH_ATP
    double nTimeToCreateSeeds = 0.0;
    double timer = clock();
#endif
    quadTree = PointCloudQuadTree::Create(&quadTreeData, seeds, report, params.GetDensity()); //create trees and fetches preliminary seeds
    report.SetWorkDone(1.0);
    if (!report.CheckContinueOnProgress())
        return ERROR;//User abort

    double minAllowedTile = 0;

    if (!quadTree->createSeeds(seeds, 0.02, minAllowedTile, report)) //filters the seeds
        return ERROR;//process was canceled or failed

#ifdef SCALABLE_MESH_ATP
    nTimeToCreateSeeds = (clock() - timer) / CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(L"nTimeToCreateSeeds", nTimeToCreateSeeds);
#endif
    std::vector<std::vector<bool>> invalidSeedIndexes;

    invalidSeedIndexes.resize(seeds.size());
    vector<QuadSeedPtr>::iterator pSeedItr = seeds.begin();
    for (size_t k = 0; k < seeds.size(); k++, pSeedItr++)
        invalidSeedIndexes[k].resize((*pSeedItr)->seedPoints.size());



    // Data structure is ready, now, we can process
   // float tinpercentage = 100;
    GroundDetection groundDetection(elHandle, quadTree.get(), params);
    TINGrowingParams::acceptableHeightOfGroundPointsInPoints *= params.GetSensitivityFactor();
    groundDetection.setDebugInfo(false);
    try
        {
        // ground filtering                    
      //  if(params.heightThreshold == params.AUTODETECT_PARAMS) params.heightThreshold = 20.0 * 0.01 * ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP()); 
      //  if (params.slopeThreshold == params.AUTODETECT_PARAMS) params.slopeThreshold = 20;
        status = groundDetection.filterGround(seeds, elHandle, report);
        }
    catch (...)
        {
        status = ERROR;
        }

    // Save classification channel
    SisterFileManager::GetInstance().SaveChannelToFile(elHandle, channel, ClassificationChannelManager::Get()._GetExtension());

#ifndef DISABLELOGGER //Don't want logger during ATP
    t.Stop();
    GroundDetectionLogger::OutputTimerToLogger(t);
    GROUNDDLOG->trace(L"END - DoGroundDetection\n");
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetectionManager::DoGroundDetectionFromDTM(Bentley::DgnPlatform::EditElementHandle& elHandle, DTMPtr& terrainModel, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener)
    {
    StatusInt status(ERROR);
    PointCloudQuadTreePtr quadTree;

#ifndef DISABLELOGGER //Don't want logger during ATP
    if (GROUNDDLOG->isSeverityEnabled(LOG_TRACE))
        {
        IPointCloudFileQueryPtr pPCFile = IPointCloudFileQuery::CreateFileQuery(elHandle);
        if (NULL != pPCFile.get())
            {
            WString name(pPCFile->GetFileName());
            UInt32 num_clouds(pPCFile->GetNumberOfClouds());
            UInt64 num_points(pPCFile->GetNumberOfPoints());

            GROUNDDLOG->tracev(L"%ls", name.c_str());
            GROUNDDLOG->tracev(L"%ld points in %ld clouds", num_points, num_clouds);
            }
        }

    GROUNDDLOG->trace(L"START - DoGroundDetectionFromDTM");
    StopWatch t;
    t.Start();
#endif


    // Create channel for saving ground channel
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(elHandle);
    IPointCloudChannelPtr channel = pData->GetChannel();

    // Create default informations for quadtree

    MSElementCP element = elHandle.GetElementCP();
    DRange3d   rangeVecPtsCloud;
    DataConvert::ScanRangeToDRange3d(rangeVecPtsCloud, element->hdr.dhdr.range);


    DPoint3d origin;
    DPoint3d corner;

    origin.x = rangeVecPtsCloud.low.x;
    origin.y = rangeVecPtsCloud.low.y;
    origin.z = rangeVecPtsCloud.low.z;

    corner.x = rangeVecPtsCloud.high.x;
    corner.y = rangeVecPtsCloud.high.y;
    corner.z = rangeVecPtsCloud.high.z;

    PointCloudQuadTreeData quadTreeData;
    DPoint3d boundingBox[2] = { origin, corner };
    quadTreeData.m_bb = boundingBox;
    quadTreeData.m_elHandle = &elHandle;    // We need an handle for query on points cloud.
    quadTreeData.m_sizeBound = (unsigned int)TreeBuildingParams::numberOfPointsPerQuery;       // Max points in tile
    quadTreeData.m_maxDataSize = (unsigned int)TreeBuildingParams::numberOfPointsInVortexBuffer;    // Max dataBuffer for vortex
    quadTreeData.m_maxDepth = 0;
    ProgressReport report(pProgressListener);
    report.SetCurrentStep(ProgressReport::STATE_CREATE_TREE);
    report.SetTotalNumberOfPhases(3);
    report.SetCurrentPhase(1);
    if (!report.CheckContinueOnProgress())
        return ERROR;//User abort
    // Create quadTree with some informations
    // Create seeds Seeds
    std::vector<QuadSeedPtr> seeds;


    quadTree = PointCloudQuadTree::Create(&quadTreeData, report, params.GetDensity()); //create trees
    report.SetWorkDone(1.0);
    if (!report.CheckContinueOnProgress())
        return ERROR;//User abort


    BcDTM* dtmObject = terrainModel->GetBcDTM();

    std::vector<DPoint3d> seedsPts;
    size_t nbrPoint = dtmObject->GetPointCount();
    for (size_t i = 0; i < nbrPoint; i++)
        {
        DPoint3d pt;
        dtmObject->GetPoint((long) i, pt);
        pt.x = pt.x * ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());
        pt.y = pt.y * ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());
        pt.z = pt.z * ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());
        seedsPts.push_back(pt);
    }
    quadTree->setSeedsPoints(seedsPts, seeds);
    std::vector<std::vector<bool>> invalidSeedIndexes;

    invalidSeedIndexes.resize(seeds.size());
    vector<QuadSeedPtr>::iterator pSeedItr = seeds.begin();
    for (size_t k = 0; k < seeds.size(); k++, pSeedItr++)
        invalidSeedIndexes[k].resize((*pSeedItr)->seedPoints.size());

    // Data structure is ready, now, we can process
    // float tinpercentage = 100;
    GroundDetection groundDetection(elHandle, quadTree.get(), params);
    groundDetection.setDebugInfo(false);
    TINGrowingParams::acceptableHeightOfGroundPointsInPoints *= params.GetSensitivityFactor();
    try
        {
        // ground filtering                    
        // if (IsAutoDetect(params.GetHeightThreshold())) params.heightThreshold = 20.0 * 0.01 * ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());
        // if (IsAutoDetect(params.GetSlopeThreshold())) params.slopeThreshold = 20;
        status = groundDetection.filterGround(seeds, elHandle, report);
        }
    catch (...)
        {
        status = ERROR;
        }

    // Save classification channel
    SisterFileManager::GetInstance().SaveChannelToFile(elHandle, channel, ClassificationChannelManager::Get()._GetExtension());

#ifndef DISABLELOGGER //Don't want logger during ATP
    t.Stop();
    GroundDetectionLogger::OutputTimerToLogger(t);
    GROUNDDLOG->trace(L"END - DoGroundDetectionFromDTM\n");
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudChannelP GroundDetectionManager::GetChannelFromPODElement(ElementHandle& elHandle)
    {
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(elHandle);
    IPointCloudChannelPtr channel = pData->GetChannel();
    return channel.get();
    }


void GroundDetectionManager::SetConfigFromString(const char* allParameters)
    {
    std::string paramString(allParameters);
    std::istringstream str(paramString);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(str, token, ','))
        tokens.push_back(token);
    if (tokens.size() < 24) return;
    TINGrowingParams::acceptableHeightOfGroundPointsInPoints = std::atof(tokens[0].c_str());
    TINGrowingParams::maxTrustworthinessFactor = std::atof(tokens[1].c_str());
    TINGrowingParams::minTrustworthinessFactor = std::atof(tokens[2].c_str());
    SeedExclusionParams::thresholdOfRansacInliersForExclusion = std::atof(tokens[3].c_str());
    SeedExclusionParams::trustworthinessCorrectionFactorForInlierThreshold = std::atof(tokens[4].c_str());
    SeedExclusionParams::dontExcludeSeedsIfHighlyTrustworthy = std::atoi(tokens[5].c_str()) != 0;
    SeedExclusionParams::trustworthinessThresholdForExclusion = std::atof(tokens[6].c_str());
    SeedSelectionParams::deeperTilesHaveFewerSeedPoints = std::atoi(tokens[7].c_str()) != 0;
    SeedSelectionParams::relativeDepthThresholdToPickFewerSeedPoints = std::atof(tokens[8].c_str());
    SeedSelectionParams::correctiveFactorForNumberOfSeedPoints = std::atof(tokens[9].c_str());
    SeedSelectionParams::numberOfPointsInTilePerSeedPoint = (size_t)std::atoi(tokens[10].c_str());
    SeedSelectionParams::trustworthinessThresholdForExtraSeeds = std::atof(tokens[11].c_str());
    SeedSelectionParams::extraSeedSelectionCondition = (SeedSelectionConditions)std::atoi(tokens[12].c_str());
    SeedSelectionParams::trustworthinessThresholdForWaivingExtraSeedCondition = std::atof(tokens[13].c_str());
    SeedSelectionParams::acceptableRatioBetweenMedianAndMax = std::atof(tokens[14].c_str());
    SeedSelectionParams::acceptableRatioBetweenMaxAndMin = std::atof(tokens[15].c_str());
    TileProfilingParams::numberOfBinsPerAxisInTileHistogram = (size_t)std::atoi(tokens[16].c_str());
    TileProfilingParams::maxPercentileForOutlierDetectionInVariation = std::atof(tokens[17].c_str());
    TileProfilingParams::minPercentileForOutlierDetectionInVariation = std::atof(tokens[18].c_str());
    TileProfilingParams::minPercentileForOutlierDetectionInPointSet = std::atof(tokens[19].c_str());
    TileProfilingParams::maxPercentileForOutlierDetectionInPointSet = std::atof(tokens[20].c_str());
    TileProfilingParams::acceptableRatioBetweenNonOutlierVariations = std::atof(tokens[21].c_str());
    TileProfilingParams::assumeNoLowOutliers = std::atoi(tokens[22].c_str()) != 0;
    TileProfilingParams::adjustToleranceValue = std::atoi(tokens[23].c_str()) != 0;
    }
END_BENTLEY_SCALABLEMESH_NAMESPACE
