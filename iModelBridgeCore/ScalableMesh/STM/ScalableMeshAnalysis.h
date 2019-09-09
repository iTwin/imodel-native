/*--------------------------------------------------------------------------------------+
|       $Date: 2016/08/23 10:33:32 $
|     $Author:Stephane.Nullans $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <ScalableMesh/IScalableMeshAnalysis.h>
#include <ScalableMesh/IScalableMesh.h>
#include "ImagePPHeaders.h"
#include "SMMeshIndex.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMProgressReport : public ISMAnalysisProgressListener, public ISMProgressReport
    {
    SMProgressReport(ISMAnalysisProgressListener* pProgressListener);
    SMProgressReport();
    ~SMProgressReport();

    bool CheckContinueOnProgress() { return _CheckContinueOnProgress(*this); }

    private:
        virtual bool    _CheckContinueOnProgress(ISMProgressReport const& report) override;

    public:
        ISMAnalysisProgressListener*   m_pProgressListener;
    };

// fwd declaration
struct Ellipsoid;

class ScalableMeshAnalysis : public IScalableMeshAnalysis
    {
    typedef BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection RayIntersection;
    private:
        IScalableMesh* m_scmPtr;
        int m_ThreadNumber;
        double m_unit2meter;

        void _CreateCutVolumeRanges(SMVolumeSegment& segment, bvector<RayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);
        void _CreateFillVolumeRanges(SMVolumeSegment& segment, bvector<RayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);

        //bool _InitGridFrom(ISMGridVolume& grid, double _resolutionMeter, const DRange3d& _rangeMesh, const DRange3d& _rangeRegion, double convertFactor = 1.0);
        bool _InitGridFrom(ISMGridVolume& grid, double _resolutionMeter, const DRange3d& _rangeMesh);
        void _FillGridVolumes(ISMGridVolume& grid, bool *intersected, double u2m=1.0);

        bool _convertTo3SMSpace(const bvector<DPoint3d>& polygon, bvector<DPoint3d>& area);
        bool _convert3SMToWorld(IScalableMesh* _3sm, DPoint3d& pt);
        bool _convert3SMToWorldDir(IScalableMesh* _3sm, const DPoint3d& pt, DVec3d& dir);
        bool _convertWorldTo3SM(IScalableMesh* _3sm, DPoint3d& pt);
        bool _convertWorldToEnu(IScalableMesh *scmPtr, Ellipsoid* ewgs84, DPoint3d& pt);
        bool _convertWorldToEnu(IScalableMesh *scmPtr, Ellipsoid* ewgs84, const bvector<RayIntersection>& Hits,
                                bvector<RayIntersection>& Hits_enu);

        bool _GetWorldRange(IScalableMesh *scm, DRange3d& _range);
        DRange3d _ConvertToWorldRange(IScalableMesh *scmPtr, DRange3d& range3sm);
        bool _GetComputationRange(DRange3d& rangeInter, bool inWorld, const bvector<DPoint3d>& polygon, IScalableMesh *mesh1, IScalableMesh *mesh2, bool bAlignZ);

        DTMStatusInt _ComputeDiscreteVolumeWorld(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener);
        DTMStatusInt _ComputeDiscreteVolumeWorld(const bvector<DPoint3d>& polygon, IScalableMesh* diffMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener);
        DTMStatusInt _ComputeDiscreteVolume3SM(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener);

        DTMStatusInt _ComputeDiscreteVolumeEcef(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener);
        DTMStatusInt _ComputeDiscreteVolumeEcef(const bvector<DPoint3d>& polygon, IScalableMesh* anotherMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener);

        bool         _GetComputationParamsInEnu(DRange3d& rangeEnu, bvector<DPoint3d>& polygonEnu,
                    Ellipsoid *wgs84, const bvector<DPoint3d>& polygonWorld, IScalableMesh* diffMesh);
    protected:
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) override;
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMesh* anotherMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) override;
        virtual void _SetMaxThreadNumber(int num) override;
        virtual void _SetUnitToMeter(double val) override;

    public:
        ScalableMeshAnalysis(IScalableMesh* scmPtr);
        ~ScalableMeshAnalysis();

        static ScalableMeshAnalysis* Create(IScalableMesh* scmPtr);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE