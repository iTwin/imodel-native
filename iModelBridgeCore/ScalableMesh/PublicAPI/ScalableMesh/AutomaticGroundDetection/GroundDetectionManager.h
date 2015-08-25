/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/AutomaticGroundDetection/GroundDetectionManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

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
        BENTLEYSTM_EXPORT static void GroundDetectionLogger::OutputTimerToLogger(StopWatch& timer);
        BENTLEYSTM_EXPORT static  Bentley::NativeLogging::ILogger* Get();
    };

#define GROUNDDLOG GroundDetectionLogger::Get()

//Uncomment line below to prevent running logger code (disable completely the logger) 
//#define DISABLELOGGER 1

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

    BENTLEYSTM_EXPORT void                SetCurrentPhase(int phaseNumber);
    BENTLEYSTM_EXPORT int                 GetCurrentPhase() const;
    BENTLEYSTM_EXPORT void                SetTotalNumberOfPhases(int totalNumberOfPhases);
    BENTLEYSTM_EXPORT int                 GetTotalNumberOfPhases() const;
    BENTLEYSTM_EXPORT void                SetCurrentStep(ProgressStateId step);
    BENTLEYSTM_EXPORT ProgressStateId     GetCurrentStep() const;
    BENTLEYSTM_EXPORT void                SetWorkDone(double workDone);
    BENTLEYSTM_EXPORT double              GetWorkDone() const;
    BENTLEYSTM_EXPORT void                SetEstimatedRemainingTime(bool hasEstimatedTime, double estimatedTime);
    BENTLEYSTM_EXPORT double              GetEstimatedRemainingTime() const;
    BENTLEYSTM_EXPORT bool                HasEstimatedRemainingTime() const;

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

    BENTLEYSTM_EXPORT static GroundDetectionParametersPtr Create();
    BENTLEYSTM_EXPORT static GroundDetectionParametersPtr Clone(GroundDetectionParametersCR input);

    BENTLEYSTM_EXPORT double    GetSlopeThreshold() const;
    BENTLEYSTM_EXPORT void      SetSlopeThreshold(double value);
    BENTLEYSTM_EXPORT double    GetHeightThreshold() const;
    BENTLEYSTM_EXPORT void      SetHeightThreshold(double value);
    BENTLEYSTM_EXPORT double    GetLargestStructureSize() const;
    BENTLEYSTM_EXPORT void      SetLargestStructureSize(double value);
    BENTLEYSTM_EXPORT float     GetDensity() const;
    BENTLEYSTM_EXPORT void      SetDensity(float value);
    BENTLEYSTM_EXPORT double    GetSensitivityFactor() const;
    BENTLEYSTM_EXPORT void      SetSensitivityFactor(double value);

    BENTLEYSTM_EXPORT ProcessingStrategy GetProcessingStrategy() const;
    BENTLEYSTM_EXPORT void               SetProcessingStrategy(ProcessingStrategy value);

    BENTLEYSTM_EXPORT bool      GetUseMultiThread() const;
    BENTLEYSTM_EXPORT void      SetUseMultiThread(bool value);

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
    BENTLEYSTM_EXPORT static StatusInt DoGroundDetection(Bentley::DgnPlatform::EditElementHandle& elHandle, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener = NULL);
    BENTLEYSTM_EXPORT static StatusInt DoGroundDetectionFromDTM(Bentley::DgnPlatform::EditElementHandle& elHandle, Bentley::TerrainModel::DTMPtr& terrainModel, GroundDetectionParametersCR params, IGroundDetectionProgressListenerP pProgressListener = NULL);
    BENTLEYSTM_EXPORT static IPointCloudChannelP GetChannelFromPODElement(Bentley::DgnPlatform::ElementHandle& elHandle);
    BENTLEYSTM_EXPORT static void SetConfigFromString(const char* allParameters);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

/*__PUBLISH_SECTION_END__*/