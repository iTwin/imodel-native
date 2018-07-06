/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshAnalysis.h $
|     $Date: 2016/08/23 10:24:32 $
|     $Author:Stephane.Nullans $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Geom/Polyface.h>
#include <Bentley/RefCounted.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <ScalableMesh/IScalableMeshQuery.h>

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMVolumeSegment {
    SMVolumeSegment() { volume = 0; }
    bvector<double> VolumeRanges; // vector of abcissa couples (min-max) 
    double volume; // the integrated volume (sum of volume ranges)
    };

struct ISMProgressReport
    {
    double  m_workDone;     // work advancement between 0-1
    float   m_timeDelay;    // delay in sec for reports
    bool    m_processCanceled;
    };

// For cancellation and progress report
struct ISMAnalysisProgressListener
    {
    ISMAnalysisProgressListener() {}
    ~ISMAnalysisProgressListener() {}
    public:
        //Return true to continue process, false to abort
        virtual bool _CheckContinueOnProgress(ISMProgressReport const& report) { return true; }
    };

class ISMGridVolume
    {
    public:
        BENTLEY_SM_EXPORT ISMGridVolume();
        
        BENTLEY_SM_EXPORT virtual ~ISMGridVolume();

        // Get effective grid size in both direction
        BENTLEY_SM_EXPORT bool GetGridSize(int &_xSize, int &_ySize);

        // Initialize a Grid of Volume Segments _xSize*_ySize and allocates memory for it
        BENTLEY_SM_EXPORT bool InitGrid(int _xSize, int _ySize);

        // input data
        DVec3d m_direction;     // the projection direction
        double m_resolution;    // the grid resolution wanted
        DPoint3d m_center;      // the 3SM center
        int m_gridSizeLimit;    // size limit used to clamp grid size

        // output data ------------------------------------------------
        double m_totalVolume;
        double m_cutVolume;
        double m_fillVolume;
        SMVolumeSegment* m_VolSegments; // table of segments ; one per grid element
        DRange3d m_range;               // range of the 3D grid volume
        DPoint3d m_gridOrigin;     // the grid origin (lower left corner) in world

        bool m_isWorld;     // The grid values are in World coordinates (not in 3SM coords)
        bool m_isEcef;      // The grid values are in Ecef coordinates

    protected:
        int m_xSize;        // Size of the Grid in X
        int m_ySize;        // Size of the Grid in Y

        bool m_bInitialised;
    };

class IScalableMeshAnalysis  : public RefCountedBase
    {
    protected:
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) = 0;
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMesh* anotherMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) = 0;
        virtual void _SetMaxThreadNumber(int num) =0;
        virtual void _SetUnitToMeter(double val) =0;

    public:
        // Compute Volume between the 3SM and a given polygon
        // returns different values (fill, cut, per grid node values) in the ISMGridVolume object
        BENTLEY_SM_EXPORT DTMStatusInt ComputeDiscreteVolume(const bvector<DPoint3d>& polygon,
            double resolution,
            ISMGridVolume& grid,
            ISMAnalysisProgressListener* pProgressListener = NULL);

        // Compute Volume difference with another 3SM in a polygon restriction
        // returns different values (fill, cut, per grid node values) in the ISMGridVolume object
        BENTLEY_SM_EXPORT DTMStatusInt ComputeDiscreteVolume(const bvector<DPoint3d>& polygon,
            IScalableMesh* anotherMesh,
            double resolution, ISMGridVolume& grid,
            ISMAnalysisProgressListener* pProgressListener = NULL);

        BENTLEY_SM_EXPORT void SetMaxThreadNumber(int num);

        BENTLEY_SM_EXPORT void SetUnitToMeter(double val);
    };

typedef RefCountedPtr<IScalableMeshAnalysis>                          IScalableMeshAnalysisPtr;

END_BENTLEY_SCALABLEMESH_NAMESPACE
