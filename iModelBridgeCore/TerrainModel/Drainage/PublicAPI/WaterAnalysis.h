#pragma once
#include <TerrainModel/Drainage/Drainage.h>
#include <TerrainModel/TerrainModel.h>
#include "Bentley\bvector.h"
#include "Bentley\bmap.h"

//#define ADDDRAINAGETYPES(t) BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE struct t; END_BENTLEY_TERRAINMODEL_NAMESPACE ADD_BENTLEY_TYPEDEFS1(BENTLEYTERRAINMODEL_NAMESPACE_NAME ,t, t, struct); typedef RefCountedPtr<BENTLEYTERRAINMODEL_NAMESPACE_NAME::t> t ## Ptr;

#define ADDDRAINAGETYPES(t)  TERRAINMODEL_TYPEDEFS(t); typedef RefCountedPtr<BENTLEY_NAMESPACE_NAME::TerrainModel::##t> t ## Ptr;

ADDDRAINAGETYPES(DrainageTracer);
ADDDRAINAGETYPES(TraceFeature);
ADDDRAINAGETYPES(TraceOnPoint);
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

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

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
        DrainageTracer& m_tracer;
        bool m_finished = false;
        DTMStatusInt m_errorStatus = DTM_SUCCESS;
        WString m_errorMessage;

        TraceFeature* m_parent;
        bvector<TraceFeaturePtr> m_children;
        bool m_isHidden = false;

        double m_currentVolume = 0;

        TraceFeature(DrainageTracer& tracer, TraceFeature* parent) : m_tracer(tracer), m_parent(parent)
            {
            }

        void SetError(WCharCP message = L"")
            {
            m_errorMessage = message;
            m_errorStatus = DTM_ERROR;
            }
        void SetErrorNotImplemented(WCharCP message = L"")
            {
            m_errorMessage = message;
            m_errorStatus = DTM_ERROR;
            }
        void SetErrorNotImplemented2(WCharCP message = L"")
            {
            m_errorMessage = message;
            m_errorStatus = DTM_ERROR;
            }
    public:
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


        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) abstract;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) abstract;

        void ClearIsHidden()
            {
            m_isHidden = false;
            }

        void HideChildren()
            {
            m_isHidden = true;
            for (auto& child : m_children)
                {
                child->HideChildren();
                }
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
                    child = &oldChild;
                }
            }

        void ProcessWaterVolume(double vol, bvector<WaterVolumeInfo>& newWaterVolume)
            {
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
                }
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceStartPoint: public TraceFeature
    {
    private:
        DPoint3d m_startPoint;
        TraceStartPoint(DrainageTracer& tracer, DPoint3dCR startPoint) : TraceFeature(tracer, nullptr), m_startPoint(startPoint)
            {
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_startPoint, 1, args);
            }
        static TraceStartPointPtr Create(DrainageTracer& tracer, DPoint3dCR startPoint)
            {
            return new TraceStartPoint(tracer, startPoint);
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

        TraceInTriangle(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2), pnt3(P3), m_startPt(startPt)
            {
            m_points.push_back(startPt);
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceInTrianglePtr Create(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt)
            {
            return new TraceInTriangle(tracer, parent, P1, P2, P3, startPt);
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

        TraceOnEdge(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2), pnt3(P3), m_pt(startPt), m_startPt(startPt), lastAngle(lastAngle)
            {
            m_points.push_back(startPt);
            }
    public:
        const bvector<DPoint3d>& GetPoints() const
            {
            return m_points;
            }
        void ProcessZSlopeTriangle(bvector<TraceFeaturePtr>& newFeatures);
        void ProcessZSlopeLine(bvector<TraceFeaturePtr>& newFeatures);
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceOnEdgePtr Create(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle = -99)
            {
            return new TraceOnEdge(tracer, parent, P1, P2, P3, startPt, lastAngle);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceZSlopeLine: public TraceFeature
    {
    private:
        bvector<DPoint3d> m_points;
        long pnt1, pnt2;

        TraceZSlopeLine(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2)
            {
            }
    public:
        const bvector<DPoint3d>& GetPoints() const
            {
            return m_points;
            }
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceZSlopeLinePtr Create(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2)
            {
            return new TraceZSlopeLine(tracer, parent, P1, P2);
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
        TraceOnPoint(DrainageTracer& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle) : TraceFeature(tracer, &parent), m_pt(startPoint), m_lastAngle(lastAngle), m_ptNum(startPtNum)
            {
            m_points.push_back(startPoint);
            }

        void ProcessZSlope(bvector<TraceFeaturePtr>& newFeatures, long descentPnt1, long descentPnt2);
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            if (m_points.size() != 1 && !m_onHullPoint)   // One point we can ignore.
                loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceOnPoint* GetOrCreate(bvector<TraceFeaturePtr>& newFeatures, DrainageTracer& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle = -99);
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
        bvector<bvector<DPoint3d>> m_points;
        bvector<long> m_exitPoints;
        double m_depth;
        DPoint3d m_pt;
        long m_ptNum;
        double m_maxVolume = -1;
        PondAnalysisPtr m_pondAnalysis;

        TracePond(DrainageTracer& tracer, TraceFeature& parent, DPoint3dCR pt, long ptNum) : TraceFeature(tracer, &parent), m_pt(pt), m_ptNum(ptNum)
            { }
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) abstract;
        void ProcessPondExits(bvector<TraceFeaturePtr>& newFeatures, bool ignorePondDepth = false);
        void CalculateMaxVolume();
        double GetVolumeAtElevation(double z);

    public:
        DPoint3dCR GetLowPoint() const
            {
            return m_pt;
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
        PondAnalysis& GetPondAnalysis();
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;

        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override;

        bool HasExitPoint(long pnt) const
            {
            for (auto exitPnt : m_exitPoints)
                {
                if (exitPnt == pnt)
                    return true;
                }
            return false;
            }


        bool IsFull()
            {
            if (m_maxVolume == -1)
                CalculateMaxVolume();
            return CurrentVolume() >= m_maxVolume;
            }

        double GetRealVolume()
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
struct TracePondLowPoint : public TracePond
    {
    private:
        TracePondLowPoint(DrainageTracer& tracer, TraceFeature& parent, long ptNum, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum)
            {

            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(DrainageTracer& tracer, TraceFeature& parent, long ptNum, DPoint3dCR pt)
            {
            return new TracePondLowPoint(tracer, parent, ptNum, pt);
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

        TracePondEdge(DrainageTracer& tracer, TraceFeature& parent, long ptNum1, long ptNum2, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum1), m_ptNum1(ptNum1), m_ptNum2(ptNum2)
            {}
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;

    public:
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            if (!m_points.empty())
                __super::DoTraceCallback(waterCallback, loadFunction, args);

            for (auto&& sump : m_sumpLines)
                loadFunction(DTMFeatureType::SumpLine, 0, 0, sump.data(), (long)sump.size(), args);
            }

        void GetSumpLines();

        static TracePondEdgePtr Create(DrainageTracer& tracer, TraceFeature& parent, long ptNum, long ptNum2, DPoint3dCR pt)
            {
            return new TracePondEdge(tracer, parent, ptNum, ptNum2, pt);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondTriangle : public TracePond
    {
    private:
        long m_ptNum, m_ptNum2, m_ptNum3;

        TracePondTriangle(DrainageTracer& tracer, TraceFeature& parent, long ptNum1, long ptNum2, long ptNum3, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum1), m_ptNum(ptNum1), m_ptNum2(ptNum2), m_ptNum3(ptNum3)
            {
            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(DrainageTracer& tracer, TraceFeature& parent, long ptNum, long ptNum2, long ptNum3, DPoint3dCR pt)
            {
            return new TracePondTriangle(tracer, parent, ptNum, ptNum2, ptNum3, pt);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondFromPondExit: public TracePond
    {
    private:
        TracePondExit& m_pondExit;
        DPoint3d m_exitPt;

        TracePondFromPondExit(DrainageTracer& tracer, TraceFeature& parent, TracePondExit& pondExit, DPoint3dCR lowPt, long lowPtNum, DPoint3dCR exitPt) : TracePond(tracer, parent, lowPt, lowPtNum), m_pondExit(pondExit), m_exitPt(exitPt)
            { }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondFromPondExitPtr Create(DrainageTracer& tracer, TraceFeature& parent, TracePondExit& pondExit, DPoint3dCR lowPt, long lowPtNum, DPoint3dCR exitPt)
            {
            return new TracePondFromPondExit(tracer, parent, pondExit, lowPt, lowPtNum, exitPt);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondExit : public TraceFeature
    {
    private:
        bvector<DPoint3d> m_points;
        DPoint3d m_exitPt;
        long m_exitPnt;
        long m_priorPnt;
        long m_nextPnt;
        long m_prevPtNum = -1;
        double m_lastAngle;
        bool m_calculated = false;
        long m_numExitFlows = 1; // This will always start with 1 the flow which created it.
        bvector<TracePond*> m_ponds;
    public:bool m_hasProcessedDeadPond = false;
    private:bool m_onHullPoint = false;

            TracePondExit(DrainageTracer& tracer, TraceFeature& parent, long exitPnt, long priorPnt, long nextPnt, DPoint3dCR exitPoint) : TraceFeature(tracer, &parent), m_exitPnt(exitPnt), m_priorPnt(priorPnt), m_nextPnt(nextPnt), m_exitPt(exitPoint)
                {
                m_points.push_back(exitPoint);
                }
            virtual void GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume) override;
            void Process(bvector<TraceFeaturePtr>& newFeatures, bool ignoreFalseLow);

    public:
        void ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures);
        void RecoverInnerPonds();
        void HideInnerPonds();


        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_exitPt, 1, args);

            if (!m_onHullPoint)
                loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }

        long GetExitPoint() const
            {
            return m_exitPnt;
            }

        void AddPond(TracePond& pond)
            {
            m_ponds.push_back(&pond);
            }

        bool IsDeadPond() const
            {
            return m_numExitFlows == m_ponds.size();
            }

        bvector<TracePond*>& GetPonds()
            {
            return m_ponds;
            }

        static TracePondExitPtr Create(DrainageTracer& tracer, TraceFeature& parent, long exitPnt, long priorPnt, long nextPnt, DPoint3dCR exitPoint)
            {
            return new TracePondExit(tracer, parent, exitPnt, priorPnt, nextPnt, exitPoint);
            }

    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct DrainageTracer
    {
    private:
        bmap<long, TraceOnPoint*> m_onPointFeatures;
        bvector<TraceFeaturePtr> m_features;
        bvector<TracePond*> m_ponds;
        bvector<TracePondExit*> m_pondExits;
        BcDTMR m_dtm;
        BC_DTM_OBJ* m_dtmObj;
    public:
        DTMDrainageTables* drainageTablesP = nullptr;
        ZeroSlopeTraceOption m_zeroSlopeOption = ZeroSlopeTraceOption::TraceLastAngle;
        double m_falseLowDepth;
        bool m_ascentTrace = false;
    public:
        DrainageTracer(BcDTMR dtm) : m_dtm(dtm), m_dtmObj(dtm.GetTinHandle())
            {

            }

        BcDTMR GetDTM()
            {
            return m_dtm;
            }

        bool AscentTrace() const
            {
            return m_ascentTrace;
            }

        int DoTrace(DPoint3dCR startPt);
        int AddWaterVolume(DPoint3dCR startPt, double volume);
        void DoTraceCallback(DTMFeatureCallback loadFunction, void* userArg);
        void AddAndProcessFeatures(bvector<TraceFeaturePtr>& featuresToAdd);
        TracePondExit* FindPondExit(long exitPoint);

        void AddPondExit(TracePondExit& pondexit)
            {
            m_pondExits.push_back(&pondexit);
            }
        void AddPond(TracePond& pond)
            {
            m_ponds.push_back(&pond);
            }

        void AddOnPoint(long pointNum, TraceOnPoint& point)
            {
            m_onPointFeatures[pointNum] = &point;
            }

        TraceOnPoint* FindExistingOnPoint(long pointNum)
            {
            auto it = m_onPointFeatures.find(pointNum);
            if (it != m_onPointFeatures.end())
                return it->second;
            return nullptr;
            }
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE