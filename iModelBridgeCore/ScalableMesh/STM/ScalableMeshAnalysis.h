/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshAnalysis.h $
|       $Date: 2016/08/23 10:33:32 $
|     $Author:Stephane.Nullans $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

class ScalableMeshAnalysis : public IScalableMeshAnalysis
    {
    private:
        IScalableMesh* m_scmPtr;
        int m_ThreadNumber;

        void _CreateCutVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);
        void _CreateFillVolumeRanges(SMVolumeSegment& segment, bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& _IPoints, DPoint3d& median, DVec3d& direction);

        bool _InitGridFrom(ISMGridVolume& grid, double _resolution, const DRange3d& _rangeMesh, const DRange3d& _rangeRegion);

        bool _convertTo3SMSpace(const bvector<DPoint3d>& polygon, bvector<DPoint3d>& area);
        bool _convert3SMToWorld(DPoint3d& pt);

    protected:
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) override;
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMesh* anotherMesh, double resolution, ISMGridVolume& grid, ISMAnalysisProgressListener* pProgressListener) override;
        virtual void _SetMaxThreadNumber(int num) override;

    public:
        ScalableMeshAnalysis(IScalableMesh* scmPtr);
        ~ScalableMeshAnalysis();

        static ScalableMeshAnalysis* Create(IScalableMesh* scmPtr);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE