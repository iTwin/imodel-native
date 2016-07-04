/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/AutomaticGroundDetection/GroundDetectionManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform\ElementHandle.h>

#include <DgnPlatform\DgnPlatform.r.h>
#include <DgnPlatform\ElementHandle.h>
#include <RmgrTools\Tools\DataExternalizer.h>
#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\PointCloudClipHandler.h>
#include <PointCloud\PointCloudDataQuery.h>
#include <PointCloud\PointCloudChannel.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//NEEDWORK_RC_API - We must use define for these below (maybe in terrainModel.h?)
struct GroundDetectionParameters;
typedef RefCountedPtr<GroundDetectionParameters> GroundDetectionParametersPtr;
typedef struct GroundDetectionParameters* GroundDetectionParametersP;
typedef struct GroundDetectionParameters const* GroundDetectionParametersCP;
typedef struct GroundDetectionParameters & GroundDetectionParametersR;
typedef struct GroundDetectionParameters const& GroundDetectionParametersCR;


/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* To see log message in Microstaton:
*MS_BSILOG_ENABLE=1
*MS_BSILOG_CONFIG_FILE=YourConfigLogFile (normally $(MS_DATA)bsilog.config.xml )
* Add this entry below to YourConfigLogFile:
*<category name="ScalableMesh.GroundDetection">
*<priority value="all"/>
*</category>
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroundDetectionLogger
    {
    public:
        BENTLEY_SM_EXPORT static void GroundDetectionLogger::OutputTimerToLogger(StopWatch& timer);
        BENTLEY_SM_EXPORT static  BENTLEY_NAMESPACE_NAME::NativeLogging::ILogger* Get();
    };

#define GROUNDDLOG GroundDetectionLogger::Get()

//Uncomment line below to prevent running logger code (disable completely the logger) 
#define DISABLELOGGER 1

/*__PUBLISH_SECTION_START__*/

//NEEDWORK_RC_API - We must use define for these below (maybe in terrainModel.h?)
struct ProgressReport;
typedef struct ProgressReport* ProgressReportP;
typedef struct ProgressReport const* ProgressReportCP;
typedef struct ProgressReport & ProgressReportR;
typedef struct ProgressReport const& ProgressReportCR;

struct IGroundDetectionProgressListener;
typedef struct IGroundDetectionProgressListener* IGroundDetectionProgressListenerP;
typedef struct IGroundDetectionProgressListener const* IGroundDetectionProgressListenerCP;
typedef struct IGroundDetectionProgressListener & IGroundDetectionProgressListenerR;
typedef struct IGroundDetectionProgressListener const& IGroundDetectionProgressListenerCR;


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct    IGroundDetectionProgressListener
{
public:
    //Return true to continue process, false to abort
    virtual bool    _CheckContinueOnProgress(ProgressReportCR report)     { return true; }
    //Return true to continue process, false to abort
    virtual bool    _CheckContinueOnLifeSignal()                          { return true; }
    //Return true to continue process, false to abort
    virtual void    _OnSignalError()                                      {}
}; // IGroundDetectionProgressListener


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ProgressReport : public IGroundDetectionProgressListener
    {
public:
    typedef enum
        {
        STATE_CREATE_TREE = 0,
        STATE_CREATE_SEEDS,
        STATE_FILTER_SEEDS,
        STATE_FILTER_GROUND,
        STATE_QTY
        } ProgressStateId;
    ProgressReport(IGroundDetectionProgressListenerP pProgressListener);
    ProgressReport();
    ~ProgressReport();

    BENTLEY_SM_EXPORT void                SetCurrentPhase(int phaseNumber);
    BENTLEY_SM_EXPORT int                 GetCurrentPhase() const;
    BENTLEY_SM_EXPORT void                SetTotalNumberOfPhases(int totalNumberOfPhases);
    BENTLEY_SM_EXPORT int                 GetTotalNumberOfPhases() const;
    BENTLEY_SM_EXPORT void                SetCurrentStep(ProgressStateId step);
    BENTLEY_SM_EXPORT ProgressStateId     GetCurrentStep() const;
    BENTLEY_SM_EXPORT void                SetWorkDone(double workDone);
    BENTLEY_SM_EXPORT double              GetWorkDone() const;
    BENTLEY_SM_EXPORT void                SetEstimatedRemainingTime(bool hasEstimatedTime, double estimatedTime);
    BENTLEY_SM_EXPORT double              GetEstimatedRemainingTime() const;
    BENTLEY_SM_EXPORT bool                HasEstimatedRemainingTime() const;

    bool    CheckContinueOnProgress();
    bool    CheckContinueOnLifeSignal();
    void    OnSignalError();

private:
    IGroundDetectionProgressListenerP   m_pProgressListener;
    int                                 m_currentPhase;
    int                                 m_totalNumberOfPhases;
    ProgressStateId                     m_currentStep;
    double                              m_workDone; //between 0-1
    double                              m_estimatedTime;
    bool                                m_hasEstimatedTime;
    
    // IGroundDetectionProgressListener implementation
    virtual bool    _CheckContinueOnProgress(ProgressReportCR report) override;
    virtual bool    _CheckContinueOnLifeSignal() override;
    virtual void    _OnSignalError() override;
    };



/*=================================================================================**//**
* @bsiclass                                     		                    06/2015
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionParameters : public RefCountedBase
{
public:
    typedef enum
        {
        USE_ANY_GPU = 0,
        USE_NON_DISPLAY_GPU,
        USE_CPU_ONLY
        } ProcessingStrategy;

    BENTLEY_SM_EXPORT static GroundDetectionParametersPtr Create();
    BENTLEY_SM_EXPORT static GroundDetectionParametersPtr Clone(GroundDetectionParametersCR input);

    BENTLEY_SM_EXPORT double    GetSlopeThreshold() const;
    BENTLEY_SM_EXPORT void      SetSlopeThreshold(double value);
    BENTLEY_SM_EXPORT double    GetHeightThreshold() const;
    BENTLEY_SM_EXPORT void      SetHeightThreshold(double value);
    BENTLEY_SM_EXPORT double    GetLargestStructureSize() const;
    BENTLEY_SM_EXPORT void      SetLargestStructureSize(double value);
    BENTLEY_SM_EXPORT float     GetDensity() const;
    BENTLEY_SM_EXPORT void      SetDensity(float value);
    BENTLEY_SM_EXPORT double    GetSensitivityFactor() const;
    BENTLEY_SM_EXPORT void      SetSensitivityFactor(double value);

    BENTLEY_SM_EXPORT ProcessingStrategy GetProcessingStrategy() const;
    BENTLEY_SM_EXPORT void               SetProcessingStrategy(ProcessingStrategy value);

    BENTLEY_SM_EXPORT bool      GetUseMultiThread() const;
    BENTLEY_SM_EXPORT void      SetUseMultiThread(bool value);

    static            bool      IsAutoDetect(double value);

private:
    GroundDetectionParameters();
    GroundDetectionParameters(const GroundDetectionParameters& input);
    ~GroundDetectionParameters();

    static const double AUTODETECT_PARAMS;

    //Parameters
    double m_slopeThreshold;
    double m_heightThreshold;
    double m_largestStructSize;
    double m_sensitivityFactor; //larger grows ground more aggressively, to be used on the easier datasets

    //Processing strategy
    ProcessingStrategy  m_useGPU;
    bool                m_useMultiThread;
    float               m_density;
};


struct GroundDetectionManager
    {
    BENTLEY_SM_EXPORT static StatusInt DoGroundDetection(BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener = NULL);
    BENTLEY_SM_EXPORT static StatusInt DoGroundDetectionFromDTM(BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& terrainModel, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener = NULL);
    BENTLEY_SM_EXPORT static IPointCloudChannelP GetChannelFromPODElement(BENTLEY_NAMESPACE_NAME::DgnPlatform::ElementHandle& elHandle);
    BENTLEY_SM_EXPORT static void SetConfigFromString(const char* allParameters);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

/*__PUBLISH_SECTION_END__*/