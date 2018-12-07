#pragma once
#include <TerrainModel/Drainage/drainage.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include "Bentley/bvector.h"
#include "Bentley/bmap.h"

//#define ADDDRAINAGETYPES(t) BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE struct t; END_BENTLEY_TERRAINMODEL_NAMESPACE ADD_BENTLEY_TYPEDEFS1(BENTLEYTERRAINMODEL_NAMESPACE_NAME ,t, t, struct); typedef RefCountedPtr<BENTLEYTERRAINMODEL_NAMESPACE_NAME::t> t ## Ptr;

#if _WIN32
#define ADDDRAINAGETYPES(t)  TERRAINMODEL_TYPEDEFS(t); typedef RefCountedPtr<BENTLEY_NAMESPACE_NAME::TerrainModel::##t> t ## Ptr;
#define ABSTRACT_KEYWORD abstract
#else
#define ADDDRAINAGETYPES(t)  TERRAINMODEL_TYPEDEFS(t); typedef RefCountedPtr<BENTLEY_NAMESPACE_NAME::TerrainModel::t> t ## Ptr;
#define ABSTRACT_KEYWORD =0;
#endif

ADDDRAINAGETYPES(WaterAnalysis);
ADDDRAINAGETYPES(TraceFeature);
ADDDRAINAGETYPES(TraceOnPoint);
ADDDRAINAGETYPES(TraceOnPointFromExit);
ADDDRAINAGETYPES(TraceOnEdge);
ADDDRAINAGETYPES(TraceInTriangle);
ADDDRAINAGETYPES(TraceStartPoint);
ADDDRAINAGETYPES(TracePond);
ADDDRAINAGETYPES(TracePondLowPoint);
ADDDRAINAGETYPES(TracePondEdge);
ADDDRAINAGETYPES(TracePondTriangle);
ADDDRAINAGETYPES(TracePondFromPondExit);
ADDDRAINAGETYPES(TracePondExit);
ADDDRAINAGETYPES(TraceZSlopeLine);
ADDDRAINAGETYPES(PondAnalysis);

ADDDRAINAGETYPES(WaterAnalysisResult);

ADDDRAINAGETYPES(WaterAnalysisResultItem);
ADDDRAINAGETYPES(WaterAnalysisResultPoint);
ADDDRAINAGETYPES(WaterAnalysisResultGeometry);
ADDDRAINAGETYPES(WaterAnalysisResultStream);
ADDDRAINAGETYPES(WaterAnalysisResultPond);

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

struct Flow;

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResultItem : RefCountedBase
    {
    private:
        double m_waterVolume;

    protected:
        WaterAnalysisResultItem(double volume) : m_waterVolume(volume)
            {
            }
        virtual WaterAnalysisResultPointP _AsPoint() { return nullptr; }
        virtual WaterAnalysisResultStreamP _AsStream() { return nullptr; }
        virtual WaterAnalysisResultPondP _AsPond() { return nullptr; }
    public:

        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisResultPointP AsPoint();
        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisResultStreamP AsStream();
        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisResultPondP AsPond();
        BENTLEYDTMDRAINAGE_EXPORT double GetWaterVolume() const;
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResult : RefCountedBase, bvector<WaterAnalysisResultItemPtr>
    {
    public:
        enum class PointType
            {
            Start,
            Low,
            Exit
            };
    protected:
        virtual void _AddPoint(DPoint3dCR point, PointType type, double volume) ABSTRACT_KEYWORD;
        virtual void _AddStream(CurveVector& geometry, double volume) ABSTRACT_KEYWORD;
        virtual void _AddStream(const bvector<DPoint3d>& points, double volume) ABSTRACT_KEYWORD;
        virtual void _AddPond(CurveVector& geometry, bool isFull, double volume, double depth) ABSTRACT_KEYWORD;
        virtual bool _IsWaterVolumeResult() const ABSTRACT_KEYWORD;
    public:
        BENTLEYDTMDRAINAGE_EXPORT void AddPoint(DPoint3dCR point, PointType type, double volume);
        BENTLEYDTMDRAINAGE_EXPORT void AddStream(CurveVector& geometry, double volume);
        BENTLEYDTMDRAINAGE_EXPORT void AddStream(const bvector<DPoint3d>& points, double volume);
        BENTLEYDTMDRAINAGE_EXPORT void AddPond(CurveVector& geometry, bool isFull, double volume, double depth);
        BENTLEYDTMDRAINAGE_EXPORT bool IsWaterVolumeResult() const;
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResultPoint : WaterAnalysisResultItem
    {
    private:
        DPoint3d m_pt;
        WaterAnalysisResult::PointType m_type;
    protected:
        WaterAnalysisResultPoint(DPoint3dCR pt, WaterAnalysisResult::PointType type, double volume) : WaterAnalysisResultItem(volume), m_pt(pt), m_type(type)
            {
            }
        virtual WaterAnalysisResultPointP _AsPoint() override
            {
            return this;
            }
    public:
        BENTLEYDTMDRAINAGE_EXPORT DPoint3dCR GetPoint() const;
        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisResult::PointType GetType();

        static WaterAnalysisResultPointPtr Create(DPoint3dCR pt, WaterAnalysisResult::PointType type, double volume)
            {
            return new WaterAnalysisResultPoint(pt, type, volume);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResultGeometry : WaterAnalysisResultItem
    {
    private:
        CurveVectorPtr m_geometry;
    protected:
        WaterAnalysisResultGeometry(CurveVectorR geometry, double volume) : WaterAnalysisResultItem(volume), m_geometry(&geometry)
            {
            }
    public:
        BENTLEYDTMDRAINAGE_EXPORT CurveVectorCR GetGeometry() const;
        void AddPrimitives(CurveVectorCR source)
            {
            m_geometry->AddPrimitives(source);
            }
        void ConsolidateAdjacentPrimitives()
            {
            m_geometry->ConsolidateAdjacentPrimitives();
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResultStream : WaterAnalysisResultGeometry
    {
    protected:
        WaterAnalysisResultStream(CurveVectorR geometry, double volume) : WaterAnalysisResultGeometry(geometry, volume)
            {
            }
        virtual WaterAnalysisResultStreamP _AsStream() override
            {
            return this;
            }
    public:
        static WaterAnalysisResultStreamPtr Create(CurveVectorR geometry, double volume)
            {
            return new WaterAnalysisResultStream(geometry, volume);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysisResultPond : WaterAnalysisResultGeometry
    {
    private:
        bool m_isFull;
        double m_depth;
    protected:
        WaterAnalysisResultPond(CurveVectorR geometry, bool isFull, double volume, double depth) : WaterAnalysisResultGeometry(geometry, volume), m_isFull(isFull), m_depth(depth)
            {
            }
        virtual WaterAnalysisResultPondP _AsPond() override { return this; }
    public:
        BENTLEYDTMDRAINAGE_EXPORT bool IsFull() const;
        BENTLEYDTMDRAINAGE_EXPORT double Depth() const;
        static WaterAnalysisResultPondPtr Create(CurveVectorR geometry, bool isFull, double volume, double depth)
            {
            return new WaterAnalysisResultPond(geometry, isFull, volume, depth);
            }

    };


//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceFeature : RefCountedBase
    {
    public:
    struct WaterVolumeInfo
        {
        TraceFeature* feature;
        double vol;

        WaterVolumeInfo(TraceFeature* feature, double vol) : feature(feature), vol(vol)
            {
            }
        };
    protected:
        WaterAnalysis& m_tracer;
        bool m_finished = false;
        DTMStatusInt m_errorStatus = DTM_SUCCESS;
        WString m_errorMessage;

        TraceFeature* m_parent;
        bvector<TraceFeaturePtr> m_children;
        bool m_isHidden = false;
        bool m_isEnclosed = false;
        double m_currentVolume = 0;
        double m_volumeToProcess = 0;

        TraceFeature(WaterAnalysis& tracer, TraceFeature* parent) : m_tracer(tracer), m_parent(parent)
            {
            }

        TraceFeature(TraceFeatureCR from, WaterAnalysis& newTracer) : m_tracer(newTracer), m_parent(from.m_parent)
            {
            m_finished = from.m_finished;
            m_errorStatus = from.m_errorStatus;
            m_errorMessage = from.m_errorMessage;
            m_children = from.m_children;
            m_isHidden = from.m_isHidden;
            m_currentVolume = from.m_currentVolume;
            m_volumeToProcess = from.m_volumeToProcess;
            m_isEnclosed = from.m_isEnclosed;
            }

        void SetError(WCharCP message = L"")
            {
            m_errorMessage = message;
            m_errorStatus = DTM_ERROR;
            }

        TraceFeatureP CreateAndAddEdge(bvector<TraceFeaturePtr>& newFeatures, long pnt1, long pnt2, DPoint3dCR pt, double m_lastAngle);

        void FindFlows(bvector<Flow>& flows, long pnt, long priorPnt = -1, long nextPnt = -1, bool isFlatPond = false);
    public:

        void AddVolumeToProcess(double vol)
            {
            m_volumeToProcess += vol;
            }

        void ClearVolumeToProcess()
            {
            m_volumeToProcess = 0;
            }

        double GetVolumeToProcess() const
            {
            return m_volumeToProcess;
            }

        bool IsFinished() const
            {
            return m_finished || m_errorStatus != DTM_SUCCESS;
            }
        bool IsHidden() const
            {
            return m_isHidden;
            }

        double CurrentVolume() const
            {
            return m_currentVolume;
            }

        void SetEnclosed()
            {
            m_isEnclosed = true;
            }

        bool GetEnclosed() const
            {
            return m_isEnclosed;
            }

        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) ABSTRACT_KEYWORD;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) ABSTRACT_KEYWORD;
        virtual void AddResult(WaterAnalysisResultR result) const ABSTRACT_KEYWORD;

        void ClearIsHidden()
            {
            m_isHidden = false;
            }

        void HideChildren()
            {
            m_isHidden = true;
            for (auto& child : m_children)
                {
                if (child->m_parent == this)
                    child->HideChildren();
                }
            }

        const TraceFeatureP GetParent() const
            {
            return m_parent;
            }

        const bvector<TraceFeaturePtr>& GetChildren() const
            {
            return m_children;
            }

        virtual void ReplaceChildren(TraceFeature& oldChild, TraceFeature& newChild)
            {
            for (auto& child : m_children)
                {
                if (&oldChild == child.get())
                    child = &newChild;
                }
            }

        void ProcessWaterVolume(double vol, bvector<WaterVolumeInfo>& newWaterVolume)
            {
            if (vol <= 0.0001)
                return;

            m_currentVolume += vol;

            if (DTM_SUCCESS == m_errorStatus)
                GetNewWaterVolumes(vol, newWaterVolume);
            }

        virtual void GetNewWaterVolumes(double totalVolume, bvector<WaterVolumeInfo>& newWaterVolume)
            {
            double vol = totalVolume / m_children.size();
            for (auto&& child : m_children)
                {
                newWaterVolume.push_back(WaterVolumeInfo(child.get(), vol));
                child->AddVolumeToProcess(vol);
                }
            }

        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const ABSTRACT_KEYWORD;

        virtual bvector<TraceFeatureP> GetReferences() const
            {
            bvector<TraceFeatureP> ret;
            ret.push_back(m_parent);
            for (auto& child : m_children)
                ret.push_back(child.get());
            return ret;
            }

        virtual void RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable)
            {
            m_parent = featureRemapTable[m_parent];
            for (auto& child : m_children)
                child = featureRemapTable[child.get()];
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceStartPoint: public TraceFeature
    {
    private:
        DPoint3d m_startPoint;
        TraceStartPoint(WaterAnalysis& tracer, DPoint3dCR startPoint) : TraceFeature(tracer, nullptr), m_startPoint(startPoint)
            {
            }
        TraceStartPoint(TraceStartPointCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
            {
            m_startPoint = from.m_startPoint;
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;
        static TraceStartPointPtr Create(WaterAnalysis& tracer, DPoint3dCR startPoint)
            {
            return new TraceStartPoint(tracer, startPoint);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TraceStartPoint(*this, tracer);
            }

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceInTriangle : public TraceFeature
    {
    private:
        bvector<DPoint3d> m_points;
        DPoint3d m_startPt;
        long pnt1, pnt2, pnt3;

        TraceInTriangle(WaterAnalysis& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2), pnt3(P3), m_startPt(startPt)
            {
            m_points.push_back(startPt);
            }
        TraceInTriangle(TraceInTriangleCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
            {
            m_points = from.m_points;
            m_startPt = from.m_startPt;
            pnt1 = from.pnt1;
            pnt2 = from.pnt2;
            pnt3 = from.pnt3;
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;

        static TraceInTrianglePtr Create(WaterAnalysis& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt)
            {
            return new TraceInTriangle(tracer, parent, P1, P2, P3, startPt);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TraceInTriangle(*this, tracer);
            }

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceOnEdge: public TraceFeature
    {
    private:
        bvector<DPoint3d> m_points;

        long pnt1, pnt2, pnt3;
        DPoint3d m_pt;
        DPoint3d m_startPt;
        double lastAngle;

        TraceOnEdge(WaterAnalysis& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle);
        TraceOnEdge(TraceOnEdgeCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
            {
            m_points = from.m_points;
            pnt1 = from.pnt1;
            pnt2 = from.pnt2;
            pnt3 = from.pnt3;
            m_pt = from.m_pt;
            m_startPt = from.m_startPt;
            lastAngle = from.lastAngle;
            }
    public:
        const bvector<DPoint3d>& GetPoints() const
            {
            return m_points;
            }
        long GetPointNum() const
            {
            return pnt1;
            }
        long GetPointNum2() const
            {
            return pnt2;
            }
        void ProcessZSlopeTriangle(bvector<TraceFeaturePtr>& newFeatures);
        void ProcessZSlopeLine(bvector<TraceFeaturePtr>& newFeatures);
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;

        static TraceOnEdgePtr Create(WaterAnalysis& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle = -99)
            {
            return new TraceOnEdge(tracer, parent, P1, P2, P3, startPt, lastAngle);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TraceOnEdge(*this, tracer);
            }

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceOnPoint : public TraceFeature
    {
    private:
        bvector<DPoint3d> m_points;
        DPoint3d m_pt;
        long m_ptNum;
        long m_prevPtNum = -1;
        double m_lastAngle;
        bool m_onHullPoint = false;

        TraceOnPoint(WaterAnalysis& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle, long prevPtNum);
        TraceOnPoint(TraceOnPointCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
            {
            m_points = from.m_points;
            m_pt = from.m_pt;
            m_ptNum = from.m_ptNum;
            m_prevPtNum = from.m_prevPtNum;
            m_lastAngle = from.m_lastAngle;
            m_onHullPoint = from.m_onHullPoint;
            }
        void ProcessZSlope(bvector<TraceFeaturePtr>& newFeatures, long descentPnt1, long descentPnt2);

    public:
        long GetPointNum() const
            {
            return m_ptNum;
            }
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;
        static TraceOnPoint* GetOrCreate(bvector<TraceFeaturePtr>& newFeatures, WaterAnalysis& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle = -99, long m_prevPnt = -1);
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TraceOnPoint(*this, tracer);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct PondExitInfo
    {
    long exitPoint;
    long priorPoint;
    long nextPoint;

    PondExitInfo(long exitPoint, long priorPoint, long nextPoint) : exitPoint(exitPoint), priorPoint(priorPoint), nextPoint(nextPoint)
        { }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePond : public TraceFeature
    {
    protected:
        struct ExitPointInfo
            {
            long ptNum;
            TracePondExitP exit;
            ExitPointInfo(long ptNum, TracePondExitP exit) : ptNum(ptNum), exit(exit)
                {
                }
            };
        bvector<bvector<DPoint3d>> m_points;
        bvector<ExitPointInfo> m_exitPoints;
        double m_depth;
        DPoint3d m_pt;
        long m_ptNum;
        double m_maxVolume = -1;
        PondAnalysisPtr m_pondAnalysis;
        bool m_allFull = false;
        TracePondP m_topLevelPond = nullptr;

        TracePond(WaterAnalysis& tracer, TraceFeature& parent, DPoint3dCR pt, long ptNum) : TraceFeature(tracer, &parent), m_pt(pt), m_ptNum(ptNum)
            { }
        TracePond(TracePondCR from, WaterAnalysis& newTracer);

        virtual void GetExitInfo(bvector<PondExitInfo>& exits) ABSTRACT_KEYWORD;
        double GetVolumeAtElevation(double z);
        void ProcessPondExits(bvector<TraceFeaturePtr>& newFeatures, bool ignorePondDepth = false);

        virtual void RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable) override;
        virtual bvector<TraceFeatureP> GetReferences() const override;
    public:
        DPoint3dCR GetLowPoint() const
            {
            return m_pt;
            }
        void SetTopLevelPond(TracePondR pond)
            {
            m_topLevelPond = &pond;
            }
        long GetLowPointNumber() const
            {
            return m_ptNum;
            }
        const bvector<bvector<DPoint3d>>& GetPoints() const
            {
            return m_points;
            }
        double GetDepth() const
            {
            return m_depth;
            }
        PondAnalysis& GetPondAnalysis() const;
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;

        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;

        bool HasExitPoint(long pnt) const
            {
            for (auto&& exitPnt : m_exitPoints)
                {
                if (exitPnt.ptNum == pnt)
                    return true;
                }
            return false;
            }

        bvector<ExitPointInfo>&  GetPondExits()
            {
            return m_exitPoints;
            }


        bool IsFull() const
            {
            if (m_depth == 0)
                return true;
            if (m_maxVolume == -1)
                return false;
            return CurrentVolume() >= m_maxVolume;
            }

        double GetRealVolume() const
            {
            if (IsFull())
                return m_maxVolume;
            return CurrentVolume();
            }
        virtual void GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume) override;

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondExit : public TraceFeature
    {
    private:
        struct ExitInfo
            {
            long priorPnt;
            long nextPnt;
            TracePond* pond;

            ExitInfo(long priorPnt, long nextPnt, TracePondP pond) : priorPnt(priorPnt), nextPnt(nextPnt), pond(pond)
                {
                }
            };

        bvector<DPoint3d> m_points;
        DPoint3d m_exitPt;
        long m_exitPnt;
        bvector<ExitInfo> m_exits;
        bvector<TracePondP> m_ponds;
        long m_prevPtNum = -1;
        double m_lastAngle;
        bool m_calculated = false;
        long m_numExitFlows = 1; // This will always start with 1 the flow which created it.
        bool m_allEnclosed = false;
        bool m_hasProcessedDeadPond = false;
        bool m_onHullPoint = false;
        bool m_allPondsFull = false;
        bool m_processedInitialPondFlows = false;
        TracePondP m_enclosingPond = nullptr;
        struct FlowInfo
            {
            long pt;
            long pt2;
            TraceFeatureP child;
            TraceFeatureP thePond = nullptr;
            bool isNowPond = false;

            FlowInfo(long pt, long pt2, TraceFeatureP child) : pt(pt), pt2(pt2), child(child)
                {
                }
            };
        bvector<FlowInfo> m_flows;

        TracePondExit(WaterAnalysis& tracer, TraceFeature& parent, long exitPnt,  DPoint3dCR exitPoint) : TraceFeature(tracer, &parent), m_exitPt(exitPoint), m_exitPnt(exitPnt)
            {
            }
        TracePondExit(TracePondExitCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
            {
            m_points = from.m_points;
            m_exitPt = from.m_exitPt;
            m_exitPnt = from.m_exitPnt;
            m_prevPtNum = from.m_prevPtNum;
            m_lastAngle = from.m_lastAngle;
            m_calculated = from.m_calculated;
            m_numExitFlows = from.m_numExitFlows;
            m_ponds = from.m_ponds;
            m_exits = from.m_exits;
            m_hasProcessedDeadPond = from.m_hasProcessedDeadPond;
            m_onHullPoint = from.m_onHullPoint;
            m_allPondsFull = from.m_allPondsFull;
            m_enclosingPond = from.m_enclosingPond;
            m_allEnclosed = from.m_allEnclosed;
            m_flows = from.m_flows;
            m_processedInitialPondFlows = from.m_processedInitialPondFlows;
            }
        virtual void GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume) override;
        void GetExitFlows(bvector<TraceFeaturePtr>& newFeatures, long priorPnt, long nextPnt);

    public:
        virtual void ReplaceChildren(TraceFeature& oldChild, TraceFeature& newChild) override;

        void Process(bvector<TraceFeaturePtr>& newFeatures, bool ignoreFalseLow);
        TracePondP ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures);
        void CheckIsCalculated();

        void RecoverInnerPonds();
        void HideInnerPonds();
        void SetHasProcessedDeadPond(TracePondR enclosingPond)
            {
            SetEnclosed();
            m_enclosingPond = &enclosingPond;
            m_hasProcessedDeadPond = true;
            }
        bool HasProcessedDeadPond() const
            {
            return m_hasProcessedDeadPond;
            }
        TracePondP GetEnclosedPond()
            {
            return m_enclosingPond;
            }


        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;


        long GetExitPoint() const
            {
            return m_exitPnt;
            }

        void CheckFullPond(TracePond& pond);

        void AddPond(TracePond& pond, const PondExitInfo& pei);

        bool IsDeadPond() const
            {
            return m_allEnclosed;// m_numExitFlows == m_ponds.size();
            }

        bool IsCalculated() const
            {
            return m_calculated;
            }

        bvector<TracePondP>& GetPonds()
            {
            return m_ponds;
            }

        static TracePondExitPtr Create(WaterAnalysis& tracer, TraceFeature& parent, long exitPnt, DPoint3dCR exitPoint)
            {
            return new TracePondExit(tracer, parent, exitPnt, exitPoint);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TracePondExit(*this, tracer);
            }

        virtual void RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable) override;
        virtual bvector<TraceFeatureP> GetReferences() const override;


    };
#ifdef __BENTLEYTMDRAINAGE_BUILD__
//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondLowPoint : public TracePond
    {
    private:
        TracePondLowPoint(WaterAnalysis& tracer, TraceFeature& parent, long ptNum, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum)
            {

            }
        TracePondLowPoint(TracePondLowPointCR from, WaterAnalysis& newTracer) : TracePond(from, newTracer)
            {
            }

    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(WaterAnalysis& tracer, TraceFeature& parent, long ptNum, DPoint3dCR pt)
            {
            return new TracePondLowPoint(tracer, parent, ptNum, pt);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TracePondLowPoint(*this, tracer);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondEdge: public TracePond
    {
    private:
        long m_ptNum1, m_ptNum2;
        bvector<bvector<DPoint3d>> m_sumpLines;

        TracePondEdge(WaterAnalysis& tracer, TraceFeature& parent, long ptNum1, long ptNum2, DPoint3dCR pt);
        TracePondEdge(TracePondEdgeCR from, WaterAnalysis& newTracer);

    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;

    public:
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;
        virtual void AddResult(WaterAnalysisResultR result) const override;

        void GetSumpLines();

        static TracePondEdgePtr Create(WaterAnalysis& tracer, TraceFeature& parent, long ptNum, long ptNum2, DPoint3dCR pt)
            {
            return new TracePondEdge(tracer, parent, ptNum, ptNum2, pt);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TracePondEdge(*this, tracer);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondTriangle : public TracePond
    {
    private:
        long m_ptNum, m_ptNum2, m_ptNum3;

        TracePondTriangle(WaterAnalysis& tracer, TraceFeature& parent, long ptNum1, long ptNum2, long ptNum3, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum1), m_ptNum(ptNum1), m_ptNum2(ptNum2), m_ptNum3(ptNum3)
            {
            }
        TracePondTriangle(TracePondTriangleCR from, WaterAnalysis& newTracer) : TracePond(from, newTracer)
            {
            m_ptNum = from.m_ptNum;
            m_ptNum2 = from.m_ptNum2;
            m_ptNum3 = from.m_ptNum3;
            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(WaterAnalysis& tracer, TraceFeature& parent, long ptNum, long ptNum2, long ptNum3, DPoint3dCR pt)
            {
            return new TracePondTriangle(tracer, parent, ptNum, ptNum2, ptNum3, pt);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TracePondTriangle(*this, tracer);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondFromPondExit: public TracePond
    {
    private:
        TracePondExitP m_pondExit;
        DPoint3d m_exitPt;

        TracePondFromPondExit(WaterAnalysis& tracer, TraceFeature& parent, TracePondExit& pondExit, DPoint3dCR lowPt, long lowPtNum, DPoint3dCR exitPt) : TracePond(tracer, parent, lowPt, lowPtNum), m_pondExit(&pondExit), m_exitPt(exitPt)
            { }
        TracePondFromPondExit(TracePondFromPondExitCR from, WaterAnalysis& newTracer) : TracePond(from, newTracer)
            {
            m_pondExit = from.m_pondExit;
            m_exitPt = from.m_exitPt;
            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondFromPondExitPtr Create(WaterAnalysis& tracer, TraceFeature& parent, TracePondExit& pondExit, DPoint3dCR lowPt, long lowPtNum, DPoint3dCR exitPt)
            {
            return new TracePondFromPondExit(tracer, parent, pondExit, lowPt, lowPtNum, exitPt);
            }
        virtual TraceFeaturePtr Clone(WaterAnalysisR tracer) const override
            {
            return new TracePondFromPondExit(*this, tracer);
            }
        virtual bvector<TraceFeatureP> GetReferences() const override
            {
            auto ret = TracePond::GetReferences();
            ret.push_back(m_pondExit);
            return ret;
            }
        virtual void RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable) override
            {
            TracePond::RemapFeatures(featureRemapTable);
            m_pondExit = dynamic_cast<TracePondExitP>(featureRemapTable[m_pondExit]);
            }

    };
#endif


//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct WaterAnalysis : RefCounted<IRefCounted>
    {
    private:
        bool m_forWater = false;
        bmap<long, TraceOnPoint*> m_onPointFeatures;
        bmap<long, TracePond*> m_pondlowPts;
        bvector<TraceFeaturePtr> m_features;
        //bvector<TracePond*> m_ponds;
        bmap<long, TracePondExit*> m_pondExits;
        bvector<char> m_pointList;
        BcDTMR m_dtm;
        BC_DTM_OBJ* m_dtmObj;
        bool m_ascentTrace = false;
        double m_falseLowDepth = 0;
        double m_pondElevationTolerance = 0.01;
        double m_pondVolumeTolerance = 0.3;
        ZeroSlopeTraceOption m_zeroSlopeOption = ZeroSlopeTraceOption::TraceLastAngle;
        bool m_dtmHasVoids;

    private:
        WaterAnalysis(BcDTMR dtm);
        WaterAnalysis(WaterAnalysisCR from);
        BENTLEYDTMDRAINAGE_EXPORT virtual ~WaterAnalysis();
    public:
        const bvector<TraceFeaturePtr>& GetFeatures() const
            {
            return m_features;
            }
        BENTLEYDTMDRAINAGE_EXPORT double GetPondElevationTolerance() const;
        BENTLEYDTMDRAINAGE_EXPORT void SetPondElevationTolerance(double value);
        BENTLEYDTMDRAINAGE_EXPORT double GetPondVolumeTolerance() const;
        BENTLEYDTMDRAINAGE_EXPORT void SetPondVolumeTolerance(double value);
        BENTLEYDTMDRAINAGE_EXPORT double GetMinimumDepth() const;
        BENTLEYDTMDRAINAGE_EXPORT void SetMinimumDepth(double value);

        ZeroSlopeTraceOption GetZeroSlopeOption() const
            {
            return m_zeroSlopeOption;
            }
        void SetZeroSlopeOption(ZeroSlopeTraceOption value)
            {
            m_zeroSlopeOption = value;
            }

        bool ForWater() const
            {
            return m_forWater;
            }
        BcDTMR GetDTM() const
            {
            return m_dtm;
            }

        bool AscentTrace() const
            {
            return m_ascentTrace;
            }

        char* GetPointList()
            {
            return m_pointList.data();
            }

        bool DTMHasVoids() const
            {
            return m_dtmHasVoids;
            }

        BENTLEYDTMDRAINAGE_EXPORT int DoTrace(DPoint3dCR startPt);
        BENTLEYDTMDRAINAGE_EXPORT int AddWaterVolume(DPoint3dCR startPt, double volume);
        BENTLEYDTMDRAINAGE_EXPORT void DoTraceCallback(DTMFeatureCallback loadFunction, void* userArg);
        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisResultPtr GetResult() const;


        void AddAndProcessFeatures(bvector<TraceFeaturePtr>& featuresToAdd);
        TracePondExit* FindPondExit(long exitPoint);

        void AddPondExit(TracePondExit& pondexit);
        void AddOnPoint(long pointNum, TraceOnPoint& point);
        TraceOnPoint* FindExistingOnPoint(long pointNum);

        void AddPondLowPond(long pointNum, TracePond& pond);
        TracePond* FindPondLowPt(long pointNum);

    public:
        BENTLEYDTMDRAINAGE_EXPORT WaterAnalysisPtr Clone();
        BENTLEYDTMDRAINAGE_EXPORT static WaterAnalysisPtr Create(BcDTMR dtm);
        
    };

struct PondAnalysis;

struct TPtrList : bvector<long>
    {
    private:
    char* m_pointList = nullptr;
        TPtrList()
            {
            }
#ifdef DEBUGCHK
    public:
        void Validate(BC_DTM_OBJ* dtmP)
            {
            if (size() <= 1)
                return;

            for (auto sp = begin(); sp != end(); sp++)
                {
                auto np = GetNext(sp);

                if (!bcdtmList_testLineDtmObject(dtmP, *sp, *np))
                    BeAssert(false);
                }
            }

        bool CheckForDuplicate()
            {
            if (size() == 1)
                return true;

            for (auto it = begin(); it != end(); it++)
                {
                if (*it == *GetNext(it))
                    return false;
                }
            return true;

            }
#endif
    public:
        TPtrList(char* pointList, long startPnt, BC_DTM_OBJ* dtmP) : m_pointList(pointList)
            {
            clear();
            long sp = startPnt;
            BeAssert(sp != dtmP->nullPnt);
            do
                {
                push_back(sp);
                m_pointList[sp]++;
                long np = nodeAddrP(dtmP, sp)->tPtr;
                nodeAddrP(dtmP, sp)->tPtr = dtmP->nullPnt;
                sp = np;
                BeAssert(sp != dtmP->nullPnt);
                } while (sp != startPnt && sp != dtmP->nullPnt);
            }

        TPtrList(char* pointList, bvector<long>::iterator first, bvector<long>::iterator second) : m_pointList(pointList)
            {
            insert(begin(), first, second);
            for (auto p : *this)
                {
                m_pointList[p]++;
                }
            }
        const char* GetPointList() const
            {
            return m_pointList;
            }
        void SetPointList(char* value)
            {
            m_pointList = value;
            }
    public:
        void Save()
            {
            for (auto&& p : *this)
                {
                m_pointList[p]--;
                }
            }

        void Restore()
            {
            for (auto&& p : *this)
                {
                m_pointList[p]++;
                }
            }

        bvector<long>::iterator AddPoint(bvector<long>::iterator sp, long pnt)
            {
            m_pointList[pnt]++;
#ifdef DEBUGCHK
            CheckForDuplicate();
#endif
            //BeAssert(m_pointList[pnt] <= 2);
            ptrdiff_t i = sp - begin();
            sp++;
            this->insert(sp, pnt);
            return begin() + i;
            }

        bool NeedsSplit(long pnt)
            {
            return m_pointList[pnt] >= 2;   // ToDo Validate
            }

        bvector<long>::iterator RemovePoint(bvector<long>::iterator sp, bvector<long>::iterator fix)
            {
            ptrdiff_t i = fix - begin();
            if (sp < fix)
                i--;
            BeAssert(m_pointList[*sp] != 0);

            m_pointList[*sp]--;
            // Check if they are two of the same value in this list if so we need to split.
            this->erase(sp);
#ifdef DEBUGCHK
            CheckForDuplicate();
#endif
            return begin() + i;
            }

        TPtrList Split(bvector<long>::iterator& sp, bvector<long>::iterator& np, bool& success)
            {
            //if (*GetNext(np) == *sp)
            //    {
            //    success = true;
            //    RemovePoint(np, sp);
            //    return TPtrList();
            //    }
            bvector<long>::iterator nextSp;
            if (*GetPrevious(sp) == *np)
                {
                success = true;
                nextSp = sp;
                bvector<long> newList;
                newList.push_back(*sp);
                sp = RemovePoint(np, sp);
                if (sp == end())
                    sp = begin();

                //if (*GetNext(sp) == *GetPrevious(sp))
                //    {
                //    sp = RemovePoint(GetNext(sp), sp);
                //    if (sp == end())
                //        sp = begin();
                //    }
                sp = RemovePoint(sp, sp);
                if (sp == end())
                    sp--;
                //np = sp;
                return TPtrList(m_pointList, newList.begin(), newList.end());
                }
            else
                {
                nextSp = std::find(np, end(), *sp);
                if (nextSp == end())
                    nextSp = std::find(begin(), np, *sp);
                BeAssert(nextSp != end());
                while (nextSp != sp && *GetPrevious(nextSp) != *np)
                    {
                    nextSp = std::find(GetNext(nextSp), end(), *sp);
                    if (nextSp == end())
                        nextSp = std::find(begin(), end(), *sp);
                    }
                }

            if (nextSp == sp)   // NotFound is it a single point.
                {
                auto nextNp = std::find(GetNext(np), end(), *np);
                if (nextNp == end())
                    nextNp = std::find(begin(), end(), *np);

                if (nextNp == np)
                    {
                    success = false;
                    return TPtrList();
                    }

                auto first = np;
                auto last = nextNp;

                if (first > last)
                    std::swap(first, last);

                TPtrList newList(m_pointList, first, last);
                for (auto p : newList)
                    m_pointList[p]--;
                erase(first, last);

#ifdef DEBUGCHK
                CheckForDuplicate();
#endif

                sp = begin();
                np = GetNext(sp);
                success = true;
                return newList;
                }


            auto first = nextSp;
            auto last = sp;

            if (first > last)
                {
                first = np;
                last = GetPrevious(nextSp);
                }

            TPtrList newList(m_pointList, first, last);
#ifdef DEBUGCHK
            newList.CheckForDuplicate();
#endif

            for (auto p : newList)
                m_pointList[p]--;

            m_pointList[*sp]--;
            m_pointList[*np]--;
            erase(first, last + 2);

#ifdef DEBUGCHK
            CheckForDuplicate();
#endif

            sp = begin();
            np = GetNext(sp);
            success = true;
            return newList;
            }

        bool Has(long pnt) const
            {
            return m_pointList[pnt] != 0;
            }

        bvector<long>::iterator GetNext(bvector<long>::iterator it)
            {
            it++;
            if (it == end())
                return begin();
            return it;
            }

        bvector<long>::iterator GetPrevious(bvector<long>::iterator it)
            {
            if (it == begin())
                it = end();
            it--;
            return it;
            }

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct PondAnalysis : RefCountedBase
    {
    public:
        enum class LowPointType
            {
            Point = 0,
            Edge = 1,
            Triangle = 2,
            PondExit = 3
            };
    private:
        struct ExpandingPondInfo
            {
            enum class Location
                {
                Unknown,
                Outer,
                Inner
                };
            Location m_location = Location::Unknown;
            TPtrList tPtr;
            int index;
            bvector<long> m_savedBoundaryPoints;
            bvector<PondExitInfo> m_exitPoints;
            bool m_hasFinished = false;
            bool needsExpanding = true;
            // possible Speedups..
            double lowZ = -1e90;
            long lowPtNum = -1;

            ExpandingPondInfo(TPtrList tPtr, int index) : tPtr(tPtr), index(index)
                {
                }
            };

    private:
#ifdef VOLUME_DEBUG
        struct TriangleP
            {
            long P1, P2, P3;

            TriangleP(long P1, long P2, long P3) : P1(P1), P2(P2), P3(P3)
                {
                }
            TriangleP()
                {
                }
            bool operator<(const TriangleP& o) const
                {
                if (P1 < o.P1)
                    return true;
                if (P1 > o.P1)
                    return false;
                if (P2 < o.P2)
                    return true;
                if (P2 > o.P2)
                    return false;
                return (P3 < o.P3);
                }
            };

        bmap<TriangleP, bool> _triangles;

        double GetVolume(double z, bool a)
            {
            double totVol = 0;
            for (auto& l : _triangles)
                {
                if (l.second == a)
                    {
                    double cutVol = 0, fillVol = 0, cutArea = 0, fillArea = 0;
                    bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, l.first.P1, l.first.P2, l.first.P3, z, cutVol, fillVol, cutArea, fillArea);
                    totVol += fillVol;
                    }
                }
            return totVol;
            }
        void AddTriangle(long P1, long P2, long P3, bool v)
            {
            if (P1 < P2)
                std::swap(P1, P2);
            if (P1 < P3)
                std::swap(P1, P3);
            if (P2 < P3)
                std::swap(P2, P3);
            TriangleP l(P1, P2, P3);

            _triangles[l] = v;
            }
#endif
        WaterAnalysis& m_tracer;
        LowPointType m_type;
        bool m_needsVolume;
        long m_lowPnt;
        long m_pntNum2;
        long m_pntNum3;
        double m_lowZ;
        bool m_outerUnknown = true;
        BC_DTM_OBJ* dtmP;
        mutable bvector<bvector<DPoint3d>> m_currentVolumePoints;
        DTMStatusInt m_errorStatus = DTM_SUCCESS;
        WString m_errorMessage;
        char* m_pointList;
        bvector<long> m_lowPoints;
        bool m_isFlatPond = false;

        long m_nextIndex = 0;
        long m_currentIndex = 0;

        bvector<ExpandingPondInfo> m_startPoints;
        bvector<ExpandingPondInfo> m_newStartPoints;
        double m_totalVol = -1;
        double m_currentVol = 0;
        double m_currentArea = 0;
        double m_currentZ = -1e99;
        double m_exitZ = 0;

        bool m_targetVolumeNeedsRefining = false;
        double m_targetRefine_minZ;
        double m_targetRefine_maxZ;
        double m_targetRefine_minVolume;
        double m_targetRefine_maxVolume;

        ExpandingPondInfo* GetCurrentStartInfo()
            {
            for (auto&& i : m_startPoints)
                {
                if (i.index == m_currentIndex)
                    return &i;
                }
            return nullptr;
            }

        void ClearCurrentIndex()
            {
            SetCurrentIndex(-1);
            }

        void SetCurrentIndex(int index);
    private:
        void RefineTargetElevation();
        bool AscentTrace() const
            {
            return m_tracer.AscentTrace();
            }
    protected:
        PondAnalysis(WaterAnalysis& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3);
        PondAnalysis(PondAnalysisCR from, WaterAnalysis& tracer);
    public:
        static PondAnalysisPtr Create(WaterAnalysis& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2 = -1, long ptNum3 = -1);

        DTMStatusInt GetError() const
            {
            return m_errorStatus;
            }

        const bvector<bvector<DPoint3d>>& GetCurrentVolumePoints();

    private:

        void SetError()
            {
            m_errorStatus = DTM_ERROR;
            }

        void FindOuterBoundary();
        double GetVolumeOfTrianglesOnSide(double elevation);

        bvector<DPoint3d > GetPolygonAtElevation(double elevation, TPtrList& list);
        bvector<bvector<DPoint3d>> GetPolygonAtElevation();
        void SwapExit(long ptNum, ExpandingPondInfo& epi);
        void AddTriangleToValues(long p1, long p2, long p3, double elevation);
        void AddTrianglesToValues(long pnt, double elevation);
        void AddTrianglesToValues(long np, long sp, long endPnt, double elevation);
        bool AddTriangleToTPtr(bvector<long>::iterator& sp, bvector<long>::iterator& np, TPtrList& list, double elevation);

        template<class T> bool ExpandPolygon(T& helper, double testZ);
        bool ExpandPolygon(double testZ);
public:
        void IsSumpOrExit(bool& isSump, bool& isExit, bvector<long>::iterator sp, bvector<long>::iterator np, TPtrList& list);
        void IsSump(long prevPnt, long pnt, long nextPnt, bool& isSump, bool& isExit);
        bool IsSump(bmap<long, std::pair<bool, bool>>& cache, long prevPnt, long pnt, long nextPnt);
        bool IsExit(bmap<long, std::pair<bool, bool>>& cache, long prevPnt, long pnt, long nextPnt);
        bool IsRidge(long sp, long np, double testZ);
        void SaveBoundaryList();
        void RestoreBoundaryPoints();
private:
    bool GetExitPoints(double testZ);

        struct BoundaryListHelper
            {
            PondAnalysisR m_analysis;
            BoundaryListHelper(PondAnalysisR analysis) : m_analysis(analysis)
                {
                m_analysis.RestoreBoundaryPoints();
                }
            ~BoundaryListHelper()
                {
                m_analysis.SaveBoundaryList();
                }
            };

        void RemoveEmptyStartPoints();

        bool FixLowPts(long& startPoint, double z);
        void GetLowestPoint(long& outLowPtNum, double& outLowZ);
        void TestLowPoint(long pnt);
        void AddInitialLowPts();
        void InsertTPtrPolygonAtElevation();
        void Clear();

        void CreateInitialTPtr();
        void GetType();

    public:
        void FindBoundaryForVolume(bool& gotTargetVolume, double targetVolume = -1e90, double lowestExitPntZ = -1e90);

        bvector<DPoint3d> GetOuterBoundary();
        bvector<bvector<DPoint3d>> GetBoundary();
        bvector<PondExitInfo> GetOuterExits();
        void FindPond();
        const bvector<long>& GetLowPoints() const
            {
            return m_lowPoints;
            }

        void DebugSaveTPtr()
            {
            DebugSaveTPtr(-1);
            }

        void DebugSaveTPtr(int n);

        double GetCurrentVolume()
            {
            return m_totalVol;
            }

        bool DTMHasVoids() const
            {
            return m_tracer.DTMHasVoids();
            }

        PondAnalysisPtr Clone(WaterAnalysisR newTracer) const
            {
            return new PondAnalysis(*this, newTracer);
            }
    };
END_BENTLEY_TERRAINMODEL_NAMESPACE

