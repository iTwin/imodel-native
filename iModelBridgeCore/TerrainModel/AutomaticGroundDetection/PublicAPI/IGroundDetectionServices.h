/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsAccumulator.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>


GROUND_DETECTION_TYPEDEF(ProgressReport)
GROUND_DETECTION_TYPEDEF(GroundDetectionParameters)


BEGIN_GROUND_DETECTION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct    IGroundDetectionProgressListener
{
public:
    //Return true to continue process, false to abort
    virtual bool    _CheckContinueOnProgress(ProgressReport const& report)     { return true; }
    //Return true to continue process, false to abort
    virtual bool    _CheckContinueOnLifeSignal()                          { return true; }
    //Return true to continue process, false to abort
    virtual void    _OnSignalError()                                      {}
    virtual void    _RefreshMSView(bool incremental=true)                                      {}
    virtual void    _OutputMessage(WChar* message) {}

}; // IGroundDetectionProgressListener


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ProgressReport : public RefCountedBase, IGroundDetectionProgressListener
    {
public:

     static ProgressReportPtr Create(IGroundDetectionProgressListener* pProgressListener);

     void                StartPhase(int phaseNumber, WChar const* traceText=L"Start Phase");
     void                EndPhase(WChar const* traceText = L"End Phase");
     int                 GetCurrentPhase() const;
     void                SetTotalNumberOfPhases(int totalNumberOfPhases);
     int                 GetTotalNumberOfPhases() const;
     void                StartCurrentIteration(int iteration);
     void                EndCurrentIteration();
     int                 GetCurrentIteration() const;
     void                SetWorkDone(double workDone);
     double              GetWorkDone() const;
     void                SetEstimatedRemainingTime(bool hasEstimatedTime, double estimatedTime);
     double              GetEstimatedRemainingTime() const;
     bool                HasEstimatedRemainingTime() const;
     StopWatch&          GetIterationTimerR();
     StopWatch&          GetTimerR();


    bool    CheckContinueOnProgress();
    bool    CheckContinueOnLifeSignal();
    void    OnSignalError();
    void    RefreshMSView(bool incremental=true);
    void    OutputMessage(WChar* message);


private:
    IGroundDetectionProgressListener*   m_pProgressListener;
    int                                 m_currentPhase;
    int                                 m_currentIteration;
    int                                 m_totalNumberOfPhases;
    double                              m_workDone; //between 0-1
    double                              m_estimatedTime;
    bool                                m_hasEstimatedTime;
    StopWatch                           m_timer;
    StopWatch                           m_timerIter;

    
    ProgressReport(IGroundDetectionProgressListener* pProgressListener);
    ProgressReport();
    ~ProgressReport();

    // IGroundDetectionProgressListener implementation
    virtual bool    _CheckContinueOnProgress(ProgressReport const& report) override;
    virtual bool    _CheckContinueOnLifeSignal() override;
    virtual void    _OnSignalError() override;
    virtual void    _RefreshMSView(bool incremental) override;
    virtual void    _OutputMessage(WChar* message) override;
    };


/*=================================================================================**//**
* @bsiclass                                                                 06/2015
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionParameters : public RefCountedBase
{
public:
  
    typedef enum
        {
        NO_DTM_REQUIRED=0,
        CREATE_NEW_DTM,
        USE_EXISTING_DTM
        } DTMFileOptions;

    GROUND_DETECTION_EXPORT static GroundDetectionParametersPtr Create();     


    //General options     
     GROUND_DETECTION_EXPORT Transform const& GetMetersToUors() const;
     GROUND_DETECTION_EXPORT void              SetMetersToUors(Transform const& metersToUors);
     GROUND_DETECTION_EXPORT bool             GetUseMultiThread() const;
     GROUND_DETECTION_EXPORT void             SetUseMultiThread(bool value);        

     GROUND_DETECTION_EXPORT IPointsProviderCreatorPtr GetPointsProviderCreator() const;        
     GROUND_DETECTION_EXPORT void                      SetPointsProviderCreator(IPointsProviderCreatorPtr& creator);        

     GROUND_DETECTION_EXPORT IGroundPointsAccumulatorPtr GetGroundPointsAccumulator() const;        
     GROUND_DETECTION_EXPORT void                        SetGroundPointsAccumulator(IGroundPointsAccumulatorPtr& pointsAccumulator);        

    //Seed options     
     GROUND_DETECTION_EXPORT void AddAdditionalSeedPoints(const bvector<DPoint3d>& additionalSeedPoints);
     GROUND_DETECTION_EXPORT void GetAdditionalSeedPoints(bvector<DPoint3d>& additionalSeedPoints) const;

    //DTM file options                 
     GROUND_DETECTION_EXPORT DTMFileOptions  GetCreateDtmFile() const;
     GROUND_DETECTION_EXPORT void            SetCreateDtmFile(DTMFileOptions createDtmFile);
     GROUND_DETECTION_EXPORT double          GetLargestStructureSize() const;
     GROUND_DETECTION_EXPORT void            SetLargestStructureSize(double value);
     GROUND_DETECTION_EXPORT bool            GetExpandTinToRange() const;
     GROUND_DETECTION_EXPORT void            SetExpandTinToRange(bool value);     
          
    //Dtm TIN densification options
     GROUND_DETECTION_EXPORT bool            GetDensifyTin() const;
     GROUND_DETECTION_EXPORT void            SetDensifyTin(bool value);
     GROUND_DETECTION_EXPORT double          GetTriangleEdgeThreshold() const;
     GROUND_DETECTION_EXPORT void            SetTriangleEdgeThreshold(double value);
     GROUND_DETECTION_EXPORT double          GetAnglePercentileFactor() const;         //We will use this histogram percentile angle threshold for densification
     GROUND_DETECTION_EXPORT void            SetAnglePercentileFactor(double value);
     GROUND_DETECTION_EXPORT double          GetHeightPercentileFactor() const;        //We will use this histogram percentile height threshold for densification
     GROUND_DETECTION_EXPORT void            SetHeightPercentileFactor(double value);    

     //Classification options    
    GROUND_DETECTION_EXPORT double          GetClassificationTolerance() const;
    GROUND_DETECTION_EXPORT void            SetClassificationTolerance(double value);
    GROUND_DETECTION_EXPORT bool            GetClassificationTolEstimateState() const;
    GROUND_DETECTION_EXPORT void            SetClassificationTolEstimateState(bool value);

    //solution parameters
     GROUND_DETECTION_EXPORT Angle     GetAngleThreshold() const;
     GROUND_DETECTION_EXPORT void      SetAngleThreshold(const Angle& value);
     GROUND_DETECTION_EXPORT double    GetHeightThreshold() const;
     GROUND_DETECTION_EXPORT void      SetHeightThreshold(double value);
     GROUND_DETECTION_EXPORT float     GetDensity() const;
     GROUND_DETECTION_EXPORT void      SetDensity(float value);
     GROUND_DETECTION_EXPORT double    GetSensitivityFactor() const;
     GROUND_DETECTION_EXPORT void      SetSensitivityFactor(double value);

    //DEBUG options
     /*
     bool      GetDrawSeeds() const;
     void      SetDrawSeeds(bool value);
     bool      GetDrawTriangles() const;
     void      SetDrawTriangles(bool value);
     bool      GetDrawGrid() const;
     void      SetDrawGrid(bool value);
     */

private:

    GroundDetectionParameters();
    GroundDetectionParameters(const GroundDetectionParameters& input);  //disable
    ~GroundDetectionParameters();

    //General options               
    bool                                m_useViewFilters;
    Transform                           m_metersToUors;
    IPointsProviderCreatorPtr           m_pointProviderCreator; 
    IGroundPointsAccumulatorPtr         m_groundPointsAccumulator; 

    //Seed options
    bvector<DPoint3d> m_additionalSeedPoints;
   
    //Dtm file options                
    DTMFileOptions  m_createDtmFile;
    double          m_largestStructSize;
    bool            m_expandTinToRange;       //If true, will create TIN points on range border     

    //Dtm TIN densification options
    bool        m_densifyTin;
    double      m_triangleEdgeThreshold;
    double      m_anglePercentileFactor;
    double      m_heightPercentileFactor;
    

    //Classification options    
    bool        m_classificationTolEstimateState;
    double      m_classificationTolerance; //Height above ground TIN that is considered to be ground point 
    
    //Processing strategy
    bool                m_useMultiThread;
    float               m_density;
    Angle               m_angleThreshold;
    double              m_heightThreshold;
    double              m_sensitivityFactor; //larger grows ground more aggressively, to be used on the easier datasets

    //Debug stuff
    /*
    bool     m_drawSeeds;
    bool     m_drawTriangles;
    bool     m_drawGrid;
    */
};


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     05/2015
+===============+===============+===============+===============+===============+======*/
struct IGroundDetectionServices 
{
//__PUBLISH_CLASS_VIRTUAL__
public:
    virtual StatusInt _DoGroundDetection(GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) = 0;
    virtual StatusInt _GetSeedPointsFromTIN(bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) = 0;
    virtual StatusInt _UpdateTINFileFromPoints(const bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) = 0;
}; // IGroundDetectionServices

END_GROUND_DETECTION_NAMESPACE
