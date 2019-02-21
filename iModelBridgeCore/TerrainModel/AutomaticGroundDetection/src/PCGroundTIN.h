/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/PCGroundTIN.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>
#include "DiscreetHistogram.h"
#include "GroundDetectionTypes.h"
#include "GroundDetectionGrid.h"
#include "IDtmProvider.h"
#include "PCThreadUtilities.h"
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>
#include <Bentley/bset.h>

#include "GroundDetectionGrid.h"


GROUND_DETECTION_TYPEDEF(PCGroundTINMT)
GROUND_DETECTION_TYPEDEF(QueryAllPointsForFirstSeedPointWork)
GROUND_DETECTION_TYPEDEF(FindFirstSeedPointWork)
GROUND_DETECTION_TYPEDEF(QueryAllPointsForTriangleWork)
GROUND_DETECTION_TYPEDEF(DensifyTriangleWork)




BEGIN_GROUND_DETECTION_NAMESPACE

bool CheckProcessNotAborted();

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct    DrawingFacility
    {
    static void  DrawTriangle(DPoint3d const& pt1, DPoint3d const& pt2, DPoint3d const& pt3, Transform const& metersToUors);
    static void  DrawPoint(DPoint3d const& pt, Transform const& metersToUors);
    static void  DrawBoundingBox(DRange3d const& boundingBox, Transform const& metersToUors);
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct DPoint3dValueEntryCompare
    {
    bool operator() (const DPoint3d& a, const DPoint3d& b) const
        {
        //Sort by z, then by x and finally by y
        if (a.z < b.z)
            {
            return true;
            }
        else if (a.z > b.z)
            {
            return false;
            }

        //z are equal, compare x
        if (a.x < b.x)
            {
            return true;
            }
        else if (a.x > b.x)
            {
            return false;
            }

        //z and x are equal, compare y
        return a.y < b.y;
        }
    };


typedef BENTLEY_NAMESPACE_NAME::bset<DPoint3d, DPoint3dValueEntryCompare> T_DPoint3dPointContainer;


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct TINPointContainer : public PointCollection, RefCountedBase
    {
    public:
        static TINPointContainerPtr Create();
        void        AddPoint(DPoint3d& ptIndex, PCGroundTriangle& pcGroundTriangle);

    private:

        BeMutex  m_pointContainerMutex;

        explicit TINPointContainer();
        ~TINPointContainer();
    }; // SeedPointContainer



/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
class PCGroundTrianglePointSorterPredicat
    {
    public:
        PCGroundTrianglePointSorterPredicat(PCGroundTriangle& pPCGroundTriangle) :m_PCGroundTriangle(pPCGroundTriangle) {}
        ~PCGroundTrianglePointSorterPredicat(){}
        bool operator()(const DPoint3d& a, const DPoint3d& b);
    private:
        PCGroundTriangle& m_PCGroundTriangle;
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
class PCGroundTrianglePointAcceptedPredicat
    {
    public:
        PCGroundTrianglePointAcceptedPredicat(PCGroundTriangle& pPCGroundTriangle) :m_PCGroundTriangle(pPCGroundTriangle) {}
        ~PCGroundTrianglePointAcceptedPredicat(){}
        bool operator()(const DPoint3d& a);
    private:
        PCGroundTriangle& m_PCGroundTriangle;
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct    PCGroundTriangle : public RefCountedBase
    {
public:

    static PCGroundTrianglePtr Create(PCGroundTIN& pcGroundTIN, DPoint3d const& pt1, DPoint3d const& pt2, DPoint3d const& pt3);
    static PCGroundTrianglePtr Create(PCGroundTIN& pcGroundTIN, Triangle const& triangle);

    bool PointAcceptedLowerZValuePredicate(const DPoint3d& a, const DPoint3d& b);
    bool IsAcceptedForTINDensification(DPoint3d const& point) const;
    bool IsAcceptedForClassification(DPoint3d const& point) const;
    bool IsAccepted(DPoint3d const& point, bool isClassification) const;
    bool IsAcceptedFromDistanceCriteria(double& distanceFound, DPoint3d const& point, DPlane3d& planeFromTriangle, double distanceThreshold) const;

    void             DrawBoundingBox() const;
    void             DrawTriangle() const;


    bool IsEqualToOneCoordinate(DPoint3d const& point) const;
    bool IsPointOnPlaneInside(DPoint3d pointOnPlane, bool strictlyInside) const;
    bool IsDensificationRequired() const;
    DPoint3d const& GetCentroid() const        { return m_triangle.GetCentroid(); }
    DPlane3d   GetPlane() const           { return m_triangle.GetPlane(); }
    DPoint3d   GetPoint(short index) const{ return m_triangle.GetPoint(index); }

    //Returns true if point found
    bool        QueryPointToAddToTin();
    bool        TryPointToAddToTin(const DPoint3d& pt);

    const PointCollection&  GetPointToAdd() const { return *m_pAcceptedPointCollection; }
    void        PrefetchPoints();
    size_t      GetMemorySize() const;


private:
    PCGroundTriangle(PCGroundTIN& pcGroundTIN, Triangle const& triangle);
    ~PCGroundTriangle();

    bool IsAcceptedFromCuttingOffEdgeCriteria(DPoint3d const& point, double distanceThreshold) const;

    DRange3d ComputeBoundingBox(GroundDetectionParameters const& params) const;

    Triangle                                m_triangle;
    DRange3d                                m_boundingBoxUors;
    PCGroundTIN&                            m_PCGroundTin;
    TINPointContainerPtr                    m_pAcceptedPointCollection;
    size_t                                  m_memorySize;
    IPointsProviderPtr                      m_pPointsProvider;
    BeMutex                                 m_queryPointMutex;
    
    };


typedef std::vector<PCGroundTrianglePtr> PCGroundTriangleCollection;

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct QueryAllPointsForFirstSeedPointWork : RefCounted<GroundDetectionWork> //RefCounted<PointCloudWork>
    {
    protected:
        GridCellEntryPtr m_pGridCellEntry;
        PCGroundTINMT& m_PCGroundTin;


        QueryAllPointsForFirstSeedPointWork(PCGroundTINMT& pcGroundTIN, GridCellEntry& gridCell) :m_PCGroundTin(pcGroundTIN), m_pGridCellEntry(&gridCell) {}
        ~QueryAllPointsForFirstSeedPointWork() {};
        virtual void    _DoWork() override;
        virtual size_t  _GetMemorySize(); //override;

    public:
        static QueryAllPointsForFirstSeedPointWorkPtr Create(PCGroundTINMT& pcGroundTIN, GridCellEntry& gridCell)
            {
            return new QueryAllPointsForFirstSeedPointWork(pcGroundTIN, gridCell);
            }
    };
/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct FindFirstSeedPointWork : RefCounted<GroundDetectionWork> //RefCounted<PointCloudWork>
    {
    protected:
        GridCellEntryPtr m_pGridCellEntry;
        PCGroundTINMT& m_PCGroundTin;


        FindFirstSeedPointWork(PCGroundTINMT& pcGroundTIN, GridCellEntry& gridCell) :m_PCGroundTin(pcGroundTIN), m_pGridCellEntry(&gridCell)   {}
        ~FindFirstSeedPointWork()  {};
        virtual void    _DoWork(); //override;
        virtual size_t  _GetMemorySize() /*override*/ {return 0;}

    public:
        static FindFirstSeedPointWorkPtr Create(PCGroundTINMT& pcGroundTIN, GridCellEntry& gridCell)
            {
            return new FindFirstSeedPointWork(pcGroundTIN,gridCell);
            }
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct QueryAllPointsForTriangleWork : RefCounted<GroundDetectionWork> //RefCounted<PointCloudWork>
    {
    protected:
        PCGroundTrianglePtr                 m_PCGroundTriangle;
        PCGroundTINMT&                      m_PCGroundTin;

        QueryAllPointsForTriangleWork(PCGroundTINMT& pcGroundTIN, PCGroundTriangle& triangle) :m_PCGroundTin(pcGroundTIN), m_PCGroundTriangle(&triangle) {}
        ~QueryAllPointsForTriangleWork() {};
        virtual void    _DoWork() ;
        virtual size_t  _GetMemorySize() ;

    public:
        static QueryAllPointsForTriangleWorkPtr Create(PCGroundTINMT& pcGroundTIN, PCGroundTriangle& triangle)
            {
            return new QueryAllPointsForTriangleWork(pcGroundTIN, triangle);
            }
    };


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct DensifyTriangleWork : RefCounted<GroundDetectionWork> //RefCounted<PointCloudWork>
    {
    protected:
        PCGroundTrianglePtr m_PCGroundTriangle;
        PCGroundTINMT&      m_PCGroundTin;

        DensifyTriangleWork(PCGroundTINMT& pcGroundTIN, PCGroundTriangle& triangle) :m_PCGroundTin(pcGroundTIN), m_PCGroundTriangle(&triangle)   {}
        ~DensifyTriangleWork()  {};
        virtual void _DoWork() ;
        virtual size_t  _GetMemorySize() ;

    public:
        static DensifyTriangleWorkPtr Create(PCGroundTINMT& pcGroundTIN,PCGroundTriangle& triangle)
            {
            return new DensifyTriangleWork(pcGroundTIN,triangle);
            }
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
#if 0  //GDZERO
struct ClassifyPointCloudWork /*: RefCounted<PointCloudWork>*/
    {
    protected:
        GridCellEntryPtr     m_pGridCellEntry;
        PCGroundTINMT&       m_PCGroundTin;

        ClassifyPointCloudWork(GridCellEntry& gridCell, PCGroundTINMT& pcGroundTIN) :m_pGridCellEntry(&gridCell), m_PCGroundTin(pcGroundTIN) {}
        ~ClassifyPointCloudWork()  {};
        virtual void _DoWork() ;
        virtual size_t  _GetMemorySize() {return 0;}

    public:
        static ClassifyPointCloudWorkPtr Create(GridCellEntry& gridCell, PCGroundTINMT& pcGroundTIN)
            {
            return new ClassifyPointCloudWork(gridCell, pcGroundTIN);
            }
    };
#endif
/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct    PCGroundTIN : public RefCountedBase,
                      public IDPoint3dCriteria
{
static const size_t CONTAINER_MAX_SIZE;
static const double HISTO_STEP_PRECISION_FACTOR;                    
static const size_t MAX_HISTO_STEP;
static const size_t MAX_NB_SEEDPOINTS_TO_ADD;
static const double SEED_BORDER_FACTOR;


public:
    enum FindTriangleResult
        {
        PointExternalToDtm      = 0,
        PointCoincidentToPoint1 = 1,
        PointOnLine             = 2,
        PointOnHull             = 3,
        PointInTriangle         = 4,
        };

    static PCGroundTINPtr Create(GroundDetectionParameters& params, ProgressReport& report);

    bool        PrepareFirstIteration();
    bool        PrepareNextIteration();
    void        StopIteration()         { m_shouldStopIteration = true; }

    StatusInt   CreateInitialTIN();
    StatusInt   DensifyTIN();
    StatusInt   Classify();
    StatusInt   SaveDtmFile();
    void        IncrementWorkDone();
    void        GetDTMPoints(bvector<DPoint3d>& points);


    DiscreetHistogram& GetAngleDiscreetHistogram() { return *m_pAnglesHisto; }
    DiscreetHistogram& GetHeightDeltaDiscreetHistogram() { return *m_pHeightDeltaHisto; }

    DRange3d const& GetBoundingBox() const       { return m_boundingBoxMeter; }
    
    GroundDetectionParameters& GetParamR()        { return *m_pParams; }
    GroundDetectionParameters const& GetParam() const  { return *m_pParams; }

    void        AddPoint(DPoint3d const& point);   
    void        AddTriangleToProcess(PCGroundTriangle& triangle1);
    void        SetNewSeedPoints(const bvector<DPoint3d>& newPoints);

    GridCellEntryPtr CreateGridCellEntry(DRange3d const& boundingBox);

    const T_DPoint3dPointContainer& GetNewPointToAdd() const {return m_newPointToAdd;}
    bool FindMirroredPointAndTriangle(double& distance, DPoint3d& mirroredPoint, DPoint3d const& point, DPoint3d const& nodePt) const;


    //DEBUGGING helper method
    void        DrawTriangles() const;
    void        DrawSeedPoints() const;
    void        DrawGrid() const;

protected:
    ProgressReportPtr                   m_pReport;//Shared pointer
    GroundDetectionParametersPtr        m_pParams;
    GroundDetectionGridPtr              m_pGDGrid;
    IDtmProviderPtr                     m_pBcDtm;
    PCGroundTriangleCollection          m_trianglesToProcess;
    DRange3d                            m_triangleToProcessExtent;
    T_DPoint3dPointContainer            m_newPointToAdd;
    bool                                m_isFirstIteration;

    PCGroundTIN(GroundDetectionParameters& params, ProgressReport& report);
    virtual ~PCGroundTIN();

    void ComputeParameterFromTINPoints();

    virtual void OutputDtmPreview(bool noDelay = false, BeMutex* newPointToAddMutex = nullptr);
    
    virtual GridCellEntryPtr _CreateGridCellEntry(DRange3d const& boundingBox);
    virtual bool _GetDistanceToTriangleFromPoint(double& distance, DPoint3d const& point) const;
    virtual void _ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats);
    virtual size_t _ComputeTriangulation();
    virtual void _ComputeTrianglesToProcessFromPointToAdd();
    virtual void _ComputeTrianglesToProcess();
    virtual void _AddTriangleToProcess(PCGroundTriangle& triangle1);
    virtual void _AddPoint(DPoint3d const& point);
    virtual StatusInt _CreateInitialTIN();
    virtual StatusInt _DensifyTIN();
    virtual StatusInt _Classify();
    virtual void      _IncrementWorkDone();
    virtual void      _GetDTMPoints(bvector<DPoint3d>& points);
    virtual void      _SetNewSeedPoints(const bvector<DPoint3d>& newPoints);
private:
    // IDPoint3dCriteria implementation
    virtual bool _IsAccepted(DPoint3d const& point) const override;

    void CreatePolyfaceQuery();
    
    DRange3d                            m_boundingBoxMeter;
    bool                                m_shouldStopIteration;
    DiscreetHistogramPtr                m_pAnglesHisto;
    DiscreetHistogramPtr                m_pHeightDeltaHisto;
    short                               m_strikeToStopComputeParameters;
    double                              m_oldAllowedAngle;
    double                              m_oldAllowedHeight;
    clock_t                             m_lastOutputPreviewTime; 

}; // PCGroundTIN

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     06/2015
+===============+===============+===============+===============+===============+======*/
struct    PCGroundTINMT : public PCGroundTIN
{
DEFINE_T_SUPER(PCGroundTIN)

public:
    
    static const unsigned int MAX_NUMBER_THREAD;

    static PCGroundTINPtr Create(GroundDetectionParameters& params, ProgressReport& report);

   PointCloudThreadPool& GetThreadPool();

protected : 

   virtual void OutputDtmPreview(bool noDelay = false, BeMutex* newPointToAddMutex = nullptr) override;

private:
        
    static BeMutex        s_newPointToAddCS;    
    intptr_t              m_mainThreadID;

    GroundDetectionThreadPoolPtr m_newThreadPool;
    GroundDetectionThreadPoolPtr GetWorkThreadPool();

    PointCloudThreadPoolPtr m_pQueryThreadPool;
    PointCloudThreadPoolPtr m_pThreadPool;    

    PCGroundTINMT(GroundDetectionParameters& params, ProgressReport& report);
    ~PCGroundTINMT();
    


    PointCloudThreadPool& GetQueryThreadPool();
    void FlushThreadPoolWork();

    virtual GridCellEntryPtr _CreateGridCellEntry(DRange3d const& boundingBox) override;
    virtual bool _GetDistanceToTriangleFromPoint(double& distance, DPoint3d const& point) const override;
    virtual size_t _ComputeTriangulation() override;
    virtual void _ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats) override;
    virtual void _ComputeTrianglesToProcessFromPointToAdd() override;
    virtual void _AddTriangleToProcess(PCGroundTriangle& triangle1);
    virtual void _AddPoint(DPoint3d const& point) override;
    virtual void _SetNewSeedPoints(const bvector<DPoint3d>& newpoints) override;
    virtual StatusInt _CreateInitialTIN() override;
    virtual StatusInt _DensifyTIN() override;
    virtual StatusInt _Classify() override;
    virtual void      _IncrementWorkDone() override;

}; // PCGroundTINMT

END_GROUND_DETECTION_NAMESPACE
