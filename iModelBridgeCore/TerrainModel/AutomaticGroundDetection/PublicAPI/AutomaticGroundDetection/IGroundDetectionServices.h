/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PublicAPI/AutomaticGroundDetection/IGroundDetectionServices.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

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

     void                StartPhase(int phaseNumber, WChar* traceText=L"Start Phase");
     void                EndPhase(WChar* traceText = L"End Phase");
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
* @bsiclass                                     		                    06/2015
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionParameters : public RefCountedBase
{
public:
  
     static GroundDetectionParametersPtr Create();

     static WString DtmExtensionFromFileType(DTMFileTypeOptions fileType);
     static DTMFileTypeOptions DtmFileTypeFromExtension(WChar* extension);


    //General options     
     Transform const&     GetMetersToUors() const;
     void            SetMetersToUors(Transform const& metersToUors);
     bool            GetUseMultiThread() const;
     void            SetUseMultiThread(bool value);     
     bool            GetUseViewFilters() const;
     void            SetUseViewFilters(bool useFilters);

    //DTM file options          
     double          GetLargestStructureSize() const;
     void            SetLargestStructureSize(double value);
     bool            GetExpandTinToRange() const;
     void            SetExpandTinToRange(bool value);     
          
    //Dtm TIN densification options
     bool            GetDensifyTin() const;
     void            SetDensifyTin(bool value);
     double          GetTriangleEdgeThreshold() const;
     void            SetTriangleEdgeThreshold(double value);
     double          GetAnglePercentileFactor() const;         //We will use this histogram percentile angle threshold for densification
     void            SetAnglePercentileFactor(double value);
     double          GetHeightPercentileFactor() const;        //We will use this histogram percentile height threshold for densification
     void            SetHeightPercentileFactor(double value);    

    //solution parameters
     Angle     GetAngleThreshold() const;
     void      SetAngleThreshold(const Angle& value);
     double    GetHeightThreshold() const;
     void      SetHeightThreshold(double value);
     float     GetDensity() const;
     void      SetDensity(float value);
     double    GetSensitivityFactor() const;
     void      SetSensitivityFactor(double value);

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
   
    //Dtm file options                
    double          m_largestStructSize;
    bool            m_expandTinToRange;       //If true, will create TIN points on range border     

    //Dtm TIN densification options
    bool        m_densifyTin;
    double      m_triangleEdgeThreshold;
    double      m_anglePercentileFactor;
    double      m_heightPercentileFactor;
    
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
* @bsiclass                                     		Marc.Bedard     05/2015
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
