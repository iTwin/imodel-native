/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/GroundDetectionParameters.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>
#include <TerrainModel/AutomaticGroundDetection/IGroundDetectionServices.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING


BEGIN_GROUND_DETECTION_NAMESPACE

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
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReportPtr ProgressReport::Create(IGroundDetectionProgressListener* pProgressListener)
    {
    return new ProgressReport(pProgressListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReport::ProgressReport(IGroundDetectionProgressListener* pProgressListener)
:m_workDone(0.0),
m_hasEstimatedTime(false),
m_estimatedTime(0.0),
m_pProgressListener(pProgressListener),
m_currentPhase(0),
m_totalNumberOfPhases(0),
m_currentIteration(0)
    {
    if (NULL == m_pProgressListener)
        m_pProgressListener = &s_dummyProgressListener;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressReport::ProgressReport() 
:m_workDone(0.0),
m_hasEstimatedTime(false),
m_estimatedTime(0.0),
m_currentPhase(0),
m_totalNumberOfPhases(0),
m_currentIteration(0)
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
int     ProgressReport::GetCurrentPhase() const                         { return m_currentPhase; }
void    ProgressReport::SetTotalNumberOfPhases(int totalNumberOfPhases) { m_totalNumberOfPhases = totalNumberOfPhases; }
int     ProgressReport::GetTotalNumberOfPhases() const                  { return m_totalNumberOfPhases; }
double  ProgressReport::GetWorkDone() const                             { return m_workDone; }
double  ProgressReport::GetEstimatedRemainingTime() const               { return m_estimatedTime; }
bool    ProgressReport::HasEstimatedRemainingTime() const               { return m_hasEstimatedTime; }
void    ProgressReport::SetEstimatedRemainingTime(bool hasEstimatedTime, double estimatedTime)
    { 
    m_hasEstimatedTime = hasEstimatedTime; 
    m_estimatedTime = estimatedTime; 
    }
StopWatch& ProgressReport::GetIterationTimerR() { return m_timerIter; }
StopWatch& ProgressReport::GetTimerR()  { return m_timer; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProgressReport::SetWorkDone(double workDone)                    
    {
    m_workDone = max(0.0,min(workDone,1.0)); 
    double currentNbSeconds;
    if (m_currentIteration > 0)
        {
        currentNbSeconds = m_timerIter.GetCurrentSeconds();
        }
    else
        {
        currentNbSeconds = m_timer.GetCurrentSeconds();
        }
    double remainingSeconds(0);
    if (m_workDone > 0.01 && m_workDone < 1.0 && currentNbSeconds!=0.0)
        {
        remainingSeconds = (currentNbSeconds / m_workDone ) - currentNbSeconds;
        SetEstimatedRemainingTime(true, remainingSeconds);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProgressReport::StartPhase(int phaseNumber, WChar const* traceText)
    {
    //GROUNDDLOG->trace(traceText);

    m_currentPhase = phaseNumber;
    m_currentIteration=0;
    m_timer.Start();
    SetWorkDone(0.0);
    SetEstimatedRemainingTime(false,0.0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProgressReport::EndPhase(WChar const* traceText)
    {
    m_timer.Stop();
    SetEstimatedRemainingTime(false, 0.0);
    /*
    GroundDetectionLogger::OutputTimerToLogger(m_timer);
    GROUNDDLOG->trace(traceText);
    */
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressReport::StartCurrentIteration(int iteration)
    {
    m_currentIteration = iteration;
    //GROUNDDLOG->tracev(L"START - Iteration=%d", m_currentIteration);
    m_timerIter.Start();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int  ProgressReport::GetCurrentIteration() const
    {
    return m_currentIteration;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressReport::EndCurrentIteration()
    {
    m_timerIter.Stop();
    /*
    GroundDetectionLogger::OutputTimerToLogger(m_timerIter);
    GROUNDDLOG->tracev(L"END - Iteration=%d", m_currentIteration);
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ProgressReport::CheckContinueOnProgress()    { return _CheckContinueOnProgress (*this);}
bool    ProgressReport::CheckContinueOnLifeSignal()  { return _CheckContinueOnLifeSignal(); }
void    ProgressReport::OnSignalError()              { return _OnSignalError(); }
void    ProgressReport::RefreshMSView(bool incremental) { return _RefreshMSView(incremental); }
void    ProgressReport::OutputMessage(WChar* message) { return _OutputMessage(message); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProgressReport::_CheckContinueOnProgress(ProgressReport const& report) 
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
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressReport::_RefreshMSView(bool incremental)
    {
    m_pProgressListener->_RefreshMSView(incremental);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressReport::_OutputMessage(WChar* message)
    {
    m_pProgressListener->_OutputMessage(message);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
/*
BENTLEY_NAMESPACE_NAME::NativeLogging::ILogger* GroundDetectionLogger::Get()
    {
    return Bentley::NativeLogging::LoggingManager::GetLogger(L"Descartes.GroundDetection");
    }
    */

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
/*
void GroundDetectionLogger::OutputTimerToLogger(StopWatch& timer)
    {
    int days = (int) timer.GetElapsedSeconds() / 60 / 60 / 24;
    int hours = (int) (timer.GetElapsedSeconds() / 60 / 60) % 24;
    int minutes = (int) (timer.GetElapsedSeconds() / 60) % 60;
    int seconds = (int) timer.GetElapsedSeconds() % 60;
   // GROUNDDLOG->tracev(L"%.2lf seconds (%ld days %ld h %ld min %ld s)", timer.GetElapsedSeconds(), days, hours, minutes, seconds);
    }
    */

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
GroundDetectionParameters::GroundDetectionParameters()
:m_angleThreshold(),
m_heightThreshold(1.0),
m_largestStructSize(25.0),
m_triangleEdgeThreshold(1.0),
m_anglePercentileFactor(50.0),
m_heightPercentileFactor(50.0),
m_classificationTolerance(10),
m_expandTinToRange(false),
m_useMultiThread(false),
m_density(1.0),
m_sensitivityFactor(1.0),
m_metersToUors(Transform::FromIdentity()),
m_createDtmFile(NO_DTM_REQUIRED),
m_densifyTin(true)
    {
    /*GDZERO
    Transform uorToMeter(IPointsProvider::GetUorToMeterTransform(mdlModelRef_getActive(),true));
    m_metersToUors.InverseOf(uorToMeter);
    */
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
Angle     GroundDetectionParameters::GetAngleThreshold() const          { return m_angleThreshold ;}
void      GroundDetectionParameters::SetAngleThreshold(const Angle& value)     { m_angleThreshold = value;}
double    GroundDetectionParameters::GetHeightThreshold() const         { return m_heightThreshold; }
void      GroundDetectionParameters::SetHeightThreshold(double value)   { m_heightThreshold = value; }
double    GroundDetectionParameters::GetTriangleEdgeThreshold() const   { return m_triangleEdgeThreshold; }
void      GroundDetectionParameters::SetTriangleEdgeThreshold(double value) { m_triangleEdgeThreshold = value; }
double    GroundDetectionParameters::GetAnglePercentileFactor() const { return m_anglePercentileFactor; }
void      GroundDetectionParameters::SetAnglePercentileFactor(double value) { m_anglePercentileFactor = value; }
double    GroundDetectionParameters::GetHeightPercentileFactor() const { return m_heightPercentileFactor; }
void      GroundDetectionParameters::SetHeightPercentileFactor(double value) { m_heightPercentileFactor = value; }

          GroundDetectionParameters::DTMFileOptions GroundDetectionParameters::GetCreateDtmFile() const                { return m_createDtmFile; }
void      GroundDetectionParameters::SetCreateDtmFile(GroundDetectionParameters::DTMFileOptions createDtmFile)    { m_createDtmFile = createDtmFile; }

double    GroundDetectionParameters::GetLargestStructureSize() const    { return m_largestStructSize; }
void      GroundDetectionParameters::SetLargestStructureSize(double value){ m_largestStructSize = value; }
float     GroundDetectionParameters::GetDensity() const                 { return m_density; }
void      GroundDetectionParameters::SetDensity(float value)            { m_density = value; }
double    GroundDetectionParameters::GetSensitivityFactor() const       { return m_sensitivityFactor; }
void      GroundDetectionParameters::SetSensitivityFactor(double value) { m_sensitivityFactor = value; }
double    GroundDetectionParameters::GetClassificationTolerance() const { return m_classificationTolerance; }
void      GroundDetectionParameters::SetClassificationTolerance(double value) {m_classificationTolerance = value; }
bool      GroundDetectionParameters::GetExpandTinToRange() const            { return m_expandTinToRange; }
void      GroundDetectionParameters::SetExpandTinToRange(bool value)        { m_expandTinToRange = value; }
bool      GroundDetectionParameters::GetDensifyTin() const                  { return m_densifyTin; }
void      GroundDetectionParameters::SetDensifyTin(bool value)              { m_densifyTin = value; }
bool      GroundDetectionParameters::GetClassificationTolEstimateState() const     { return m_classificationTolEstimateState; }
void      GroundDetectionParameters::SetClassificationTolEstimateState(bool value) { m_classificationTolEstimateState = value; }

bool      GroundDetectionParameters::GetUseMultiThread() const          { return m_useMultiThread; }
void      GroundDetectionParameters::SetUseMultiThread(bool value)      { m_useMultiThread = value; }

Transform const&  GroundDetectionParameters::GetMetersToUors() const     { return m_metersToUors; }
void        GroundDetectionParameters::SetMetersToUors(Transform const& metersToUors)      { m_metersToUors.Copy(metersToUors); }

IPointsProviderCreatorPtr GroundDetectionParameters::GetPointsProviderCreator() const     { return m_pointProviderCreator; }
void                     GroundDetectionParameters::SetPointsProviderCreator(IPointsProviderCreatorPtr& creator)      { m_pointProviderCreator = creator; }

IGroundPointsAccumulatorPtr GroundDetectionParameters::GetGroundPointsAccumulator() const     { return m_groundPointsAccumulator; }
void                        GroundDetectionParameters::SetGroundPointsAccumulator(IGroundPointsAccumulatorPtr& pointsAccumulator)      { m_groundPointsAccumulator = pointsAccumulator; }

void GroundDetectionParameters::AddAdditionalSeedPoints(const bvector<DPoint3d>& additionalSeedPoints)
    {    
    m_additionalSeedPoints.insert(m_additionalSeedPoints.end(), additionalSeedPoints.begin(), additionalSeedPoints.end());    
    }

void GroundDetectionParameters::GetAdditionalSeedPoints(bvector<DPoint3d>& additionalSeedPoints) const
    {
    additionalSeedPoints.insert(additionalSeedPoints.end(), m_additionalSeedPoints.begin(), m_additionalSeedPoints.end());    
    }

/*
BeFileName GroundDetectionParameters::GetDtmFilename() const            { return m_dtmFileName; }
void GroundDetectionParameters::SetDtmFilename(BeFileName const& dtmFileName){ m_dtmFileName = dtmFileName; }
*/





END_GROUND_DETECTION_NAMESPACE
