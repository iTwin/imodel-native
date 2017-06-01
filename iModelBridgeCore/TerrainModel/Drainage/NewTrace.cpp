/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/NewTrace.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma inline_depth(255)
#pragma inline_recursion(on)
#pragma push_macro("DEBUG")
#pragma push_macro("NDEBUG")
//#undef DEBUG
//#define NDEBUG
#include "Bentley\bvector.h"
#include "Bentley\bmap.h"
#pragma pop_macro("DEBUG")
#pragma pop_macro("NDEBUG")

#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>
int bcdtmDrainage_traceMaximumDescentDtmObjectOld(BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP, DTMFeatureCallback loadFunctionP, double falseLowDepth, double startX, double startY, void *userP );

#ifdef DEBUG
#define TPTRVALIDATEDEBUG
#define DEBUGCHK
#endif

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct ClearTPointerList
    {
    long& m_startPoint;
    BC_DTM_OBJ*& m_dtm;

    ClearTPointerList(long& startPoint, BC_DTM_OBJ*& dtm) : m_startPoint(startPoint), m_dtm(dtm)
        { }
    ~ClearTPointerList()
        {
        if (m_dtm && m_startPoint != m_dtm->nullPnt)
            bcdtmList_nullTptrListDtmObject(m_dtm, m_startPoint);
        }
    };

// DEBUG checking bools.
const bool checkVolumes = false;

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct DtmPolygonPtr
    {
    DTM_POLYGON_OBJ *ptr = nullptr;

    ~DtmPolygonPtr()
        {
        if (ptr!= nullptr) bcdtmPolygon_deletePolygonObject(&ptr);
        }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct DtmSumpLinesPtr
    {
    DTM_SUMP_LINES* ptr = nullptr;
    long num;

    ~DtmSumpLinesPtr()
        {
        if (ptr != nullptr) free(ptr);
        }
    };

#define ADDDRAINAGETYPES(t) struct t; ADD_BENTLEY_TYPEDEFS1( ,t, t, struct); typedef RefCountedPtr<t> t ## Ptr;

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

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TraceFeature : RefCountedBase
    {
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

        struct WaterVolumeInfo
            {
            TraceFeature* feature;
            double vol;
            WaterVolumeInfo(TraceFeature* feature, double vol) : feature(feature), vol(vol)
                { }
            };
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
        bvector<DPoint3d> m_points; // TracePoint.

        long pnt1, pnt2, pnt3;
        DPoint3d m_pt;
        DPoint3d m_startPt;

        //long nextPnt1, nextPnt2, nextPnt3;
        //DPoint3d nextPt;
        double lastAngle;
        //double saveLastAngle;
        //long startPointType;  //
        //long lastPoint;

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
        bvector<DPoint3d> m_points; // TracePoint.
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


        double m_maxVolume = -1;

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

        int DoTrace(DPoint3dCR startPt)
            {
            auto startFeature = TraceStartPoint::Create(*this, startPt);
            m_features.push_back(startFeature);
            bvector<TraceFeaturePtr> featuresToProcess;
            featuresToProcess.push_back(startFeature);
            AddAndProcessFeatures(featuresToProcess);
            return DTM_SUCCESS;
            }

        int AddWaterVolume(DPoint3dCR startPt, double volume)
            {
            auto startFeature = TraceStartPoint::Create(*this, startPt);
            m_features.push_back(startFeature);
            bvector<TraceFeaturePtr> featuresToProcess;
            featuresToProcess.push_back(startFeature);
            AddAndProcessFeatures(featuresToProcess);

            bvector<TraceFeature::WaterVolumeInfo> features;
            bvector<TraceFeature::WaterVolumeInfo> newFeatures;

            newFeatures.push_back(TraceFeature::WaterVolumeInfo(startFeature.get(), volume));
            //bmap<TraceFeature*, bool> processedFeatureMap;  // Temporary to help with debug.

            while (!newFeatures.empty())
                {
                std::swap(features, newFeatures);
                //for (auto&& feature : features)
                //    {
                //    if (processedFeatureMap.find(feature.feature) != processedFeatureMap.end())
                //        break;
                //    processedFeatureMap[feature.feature] = true;
                //    }

                newFeatures.clear();
                for (auto&& feature : features)
                    {
                    // TODO - remove
                    if (feature.feature->CurrentVolume() > volume)
                        return DTM_SUCCESS;
                    feature.feature->ProcessWaterVolume(feature.vol, newFeatures);
                    }
                }
            return DTM_SUCCESS;
            }

        void DoTraceCallback(DTMFeatureCallback loadFunction, void* userArg)
            {
            static bool showHidden = false;
            for (const auto& feature : m_features)
                {
                bool hasWater = feature->CurrentVolume() != 0;

                if (showHidden || !feature->IsHidden() || hasWater)
                    feature->DoTraceCallback(hasWater, loadFunction, userArg);
                }
            }
        void AddAndProcessFeatures(bvector<TraceFeaturePtr>& featuresToAdd)
            {
            bool haveProcessedAFeature = false;
            bvector<TraceFeature*> featuresToProcess;
            bvector<TraceFeaturePtr> newFeatures;
            bvector<TraceFeature*> newFeaturesToProcess;

            for (auto&& feature : featuresToAdd)
                {
                m_features.push_back(feature.get());
                featuresToProcess.push_back(feature.get());
                }

            do
                {
                haveProcessedAFeature = false;

                newFeatures.clear();
                newFeaturesToProcess.clear();

                for (auto&& feature : featuresToProcess)
                    {
                    if (feature->IsFinished())
                        continue;
                    haveProcessedAFeature = true;

                    feature->Process(newFeatures);
                    if (!feature->IsFinished())
                        newFeaturesToProcess.push_back(feature);
                    }
                for (auto&& feature : newFeatures)
                    newFeaturesToProcess.push_back(feature.get());

                featuresToProcess.swap(newFeaturesToProcess);
                m_features.insert(m_features.end(), newFeatures.begin(), newFeatures.end());
                if (m_features.size() > 1000)
                    break;
                } while (haveProcessedAFeature);
            }
        TracePondExit* FindPondExit(long exitPoint)
            {
            for (auto pondExit : m_pondExits)
                {
                if (pondExit->GetExitPoint() == exitPoint)
                    return pondExit;
                }
            return nullptr;
            }
        void AddPondExit(TracePondExit& pondexit)
            {
            m_pondExits.push_back(&pondexit);
            }
        void AddPond(TracePond& pond)
            {
            m_ponds.push_back(&pond);
            }

        bmap<long, TraceOnPoint*> m_onPointFeatures;
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


//#define VOLUME_DEBUG

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
        DrainageTracer& m_tracer;
        LowPointType m_type;
        bool m_needsVolume;
        long m_lowPnt;
        long m_pntNum2;
        long m_pntNum3;
        double m_lowZ;
        bool m_outerUnknown = true;
        BC_DTM_OBJ* dtmP;
        bvector<bvector<DPoint3d>> m_currentVolumePoints;
        DTMStatusInt m_errorStatus = DTM_SUCCESS;
        WString m_errorMessage;
        void ValidateTPtr(long startPnt)
            {
            if (startPnt == -1 || m_errorStatus != DTM_SUCCESS)
                return;
            long sp = startPnt;
            do
                {
                long np = nodeAddrP(dtmP, sp)->tPtr;
                if (sp != np && !bcdtmList_testLineDtmObject(dtmP, sp, np))
                    sp = sp;
                sp = np;
                } while (sp != startPnt);
            }

        void ValidateTPtrs()
            {

            for (auto&& startInfo : m_startPoints)
                {
                if (startInfo.m_hasFinished)
                    continue;
                long& startPoint = startInfo.startPnt;
                SetCurrentIndex(startInfo.index);
                ValidateTPtr(startPoint);
                }
            }

        struct ExpandingPondInfo
            {
            enum class Location
                {
                Unknown,
                Outer,
                Inner
                };
            Location m_location = Location::Unknown;
            long startPnt;
            int index;
            bvector<long> m_savedBoundaryPoints;
            bvector<PondExitInfo> m_exitPoints;
            bool m_hasFinished = false;
            // possible Speedups..
            double lowZ = -1e90;
            long lowPtNum = -1;

            ExpandingPondInfo(long startPnt, int index) : startPnt(startPnt), index(index)
                {
                }

            void SaveList(PondAnalysisR analysis)
                {
                auto dtmP = analysis.dtmP;
                m_savedBoundaryPoints.clear();
                long sp = startPnt;
                BeAssert(sp != analysis.dtmP->nullPnt);
                do
                    {
                    m_savedBoundaryPoints.push_back(sp);
                    long np = nodeAddrP(dtmP, sp)->tPtr;
                    nodeAddrP(dtmP, sp)->tPtr = dtmP->nullPnt;
                    sp = np;
                    BeAssert(sp != analysis.dtmP->nullPnt);
                    } while (sp != startPnt && sp != analysis.dtmP->nullPnt);
                }

            void RestoreList(PondAnalysisR analysis)
                {
                auto dtmP = analysis.dtmP;
                long prevPoint = -1;
                long startPoint = -1;
                for (auto pnt : m_savedBoundaryPoints)
                    {
                    if (prevPoint != -1)
                        {
                        BeAssert(nodeAddrP(dtmP, prevPoint)->tPtr == dtmP->nullPnt);
                        nodeAddrP(dtmP, prevPoint)->tPtr = pnt;
                        }
                    else
                        startPoint = pnt;
                    prevPoint = pnt;
                    }
                BeAssert(nodeAddrP(dtmP, prevPoint)->tPtr == dtmP->nullPnt);
                nodeAddrP(dtmP, prevPoint)->tPtr = startPoint;
                m_savedBoundaryPoints.clear();
                }
            };
        long m_nextIndex = 0;
        long m_currentIndex = 0;
        bool m_hasCurrentGotDuplicates = false;
        bvector<ExpandingPondInfo> m_startPoints;
        bvector<ExpandingPondInfo> m_newStartPoints;

        bmap<long, bvector<int>> m_indexPtInfo;
//        bmap<int, bvector<std::pair<long, long>>> m_duplicateForIndex;
        bvector<std::pair<int, bvector<std::pair<long, long>>>> m_duplicateForIndex;

        bvector<std::pair<long, long>>& GetDuplicateForIndex(int index)
            {
            for (auto&& i : m_duplicateForIndex)
                {
                if (i.first == index)
                    return i.second;
                }
            static bvector<std::pair<long, long>> l;
            return l;
            }
        bvector<std::pair<long, long>>& GetOrAddDuplicateForIndex(int index)
            {
            for (auto&& i : m_duplicateForIndex)
                {
                if (i.first == index)
                    return i.second;
                }
            m_duplicateForIndex.push_back(std::pair<int, bvector<std::pair<long, long>>>(index, bvector<std::pair<long, long>>()));
            return m_duplicateForIndex.back().second;
            }

        double m_currentVol = 0;
        double m_currentArea = 0;
        double m_currentZ = 0;

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
        void SetCurrentIndex(int index)
            {
#ifdef TPTRVALIDATEDEBUG
            for (auto&& a : m_startPoints)
                {
                if (a.index == m_currentIndex && a.m_savedBoundaryPoints.empty())
                    ValidateTPtr(a.startPnt);
                }
#endif
            if (m_currentIndex == index)
                return;

            if (m_hasCurrentGotDuplicates)
                {
                for (auto&& i : GetDuplicateForIndex(m_currentIndex))
                    i.second = nodeAddrP(dtmP, i.first)->tPtr;

                m_hasCurrentGotDuplicates = false;
                }
            m_currentIndex = index;
            if (index != -1)
                {
                for (auto&& i : GetDuplicateForIndex(m_currentIndex))
                    {
                    m_hasCurrentGotDuplicates = true;
                    nodeAddrP(dtmP, i.first)->tPtr = i.second;
                    }
                }
#ifdef TPTRVALIDATEDEBUG
            for (auto&& a : m_startPoints)
                {
                if (a.index == m_currentIndex && a.m_savedBoundaryPoints.empty())
                    ValidateTPtr(a.startPnt);
                }
#endif
            }
        void AddDuplicateTPtr(long ptNum, int index, long value)
            {
            if (index == m_currentIndex)
                m_hasCurrentGotDuplicates = true;

            m_indexPtInfo[ptNum].push_back(index);
            GetOrAddDuplicateForIndex(index).push_back(std::pair<long, long>(ptNum, value));
            }

        void RemoveDuplicatePoint(long ptNum)
            {
            if (!m_hasCurrentGotDuplicates)
                return;

            auto& dfi = GetDuplicateForIndex(m_currentIndex);
            bool found = false;
            for (auto&& i : dfi)
                {
                if (i.first == ptNum)
                    {
                    found = true;
                    dfi.erase(&i);
                    break;
                    }
                }
            if (!found)
                return;

            auto&& iPit = m_indexPtInfo.find(ptNum);
            auto& iPi = iPit->second;

            iPi.erase(std::find(iPi.begin(), iPi.end(), m_currentIndex));
            if (iPi.size() == 1)
                {
                auto& dfi2 = GetDuplicateForIndex(iPi[0]);
                for (auto&& i : dfi2)
                    {
                    if (i.first == ptNum)
                        {
                        nodeAddrP(dtmP, ptNum)->tPtr = i.second;
                        dfi2.erase(&i);
                        break;
                        }
                    }
                m_indexPtInfo.erase(iPit);
                }
            }
        void SwapDuplicate(long ptNum, int newIndex)
            {
            if (!m_hasCurrentGotDuplicates)
                return;

            bool found = false;
            auto& dfi = GetDuplicateForIndex(m_currentIndex);
            for (auto&& i : dfi)
                {
                if (i.first == ptNum)
                    {
                    found = true;
                    dfi.erase(&i);
                    break;
                    }
                }
            if (!found)
                return;

            GetOrAddDuplicateForIndex(newIndex).push_back(std::pair<long, long>(ptNum, nodeAddrP(dtmP, ptNum)->tPtr));

            auto&& iPi = m_indexPtInfo[ptNum];
            for (auto&& j : iPi)
                {
                if (j == m_currentIndex)
                    {
                    j = newIndex;
                    break;
                    }
                }
            }

    private:
        bool AscentTrace() const
            {
            return m_tracer.AscentTrace();
            }
    protected:
        PondAnalysis(DrainageTracer& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3) : m_tracer(tracer), m_lowPnt(lowPnt), m_lowZ(lowZ), m_type(type), m_pntNum2(ptNum2), m_pntNum3(ptNum3)
            {
            dtmP = m_tracer.GetDTM().GetTinHandle();
            }
    public:
        static PondAnalysisPtr Create(DrainageTracer& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2 = -1, long ptNum3 = -1)
            {
            return new PondAnalysis(tracer, lowPnt, lowZ, type, ptNum2, ptNum3);
            }

        DTMStatusInt GetError() const
            {
            return m_errorStatus;
            }

        const bvector<bvector<DPoint3d>>& GetCurrentVolumePoints()
            {
            return m_currentVolumePoints;
            }

    private:

        void SetError()
            {
            m_errorStatus = DTM_ERROR;
            }

        void FindOuterBoundary()
            {
            if (!m_outerUnknown)
                return;

            m_outerUnknown = false;
            ExpandingPondInfo* outerPond = nullptr;
            long lowPtNum = dtmP->numPoints;

            if (m_startPoints.empty())
                return;
            if (m_startPoints.size() == 1)
                {
                m_startPoints.front().m_location = ExpandingPondInfo::Location::Outer;
                return;
                }
            // OK This works for two point island boundary but not 1.
            for (auto&& startInfo : m_startPoints)
                {
                if (startInfo.m_location == ExpandingPondInfo::Location::Inner)
                    continue;
                long& startPnt = startInfo.startPnt;
                SetCurrentIndex(startInfo.index);
                startInfo.m_location = ExpandingPondInfo::Location::Inner;

                for (long sp = -1; sp != startPnt;)
                    {
                    if (sp == -1)
                        sp = startPnt;

                    long np = nodeAddrP(dtmP, sp)->tPtr;

                    if (np < lowPtNum)
                        {
                        lowPtNum = np;
                        outerPond = &startInfo;
                        }

                    sp = np;

                    }
                }
            BeAssert(outerPond != nullptr);
            if (nullptr != outerPond)
                outerPond->m_location = ExpandingPondInfo::Location::Outer;

#ifdef DEBUG_CODE
            for (auto&& startInfo : m_startPoints)
                {
                long& startPnt = startInfo.startPnt;
                SetCurrentIndex(startInfo.index);
                double area;
                DTMDirection direction;
                bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP, startPnt, &area, &direction);
                DTMDirection testDirection = startInfo.m_location== ExpandingPondInfo::Location::Outer ? DTMDirection::AntiClockwise : DTMDirection::Clockwise;

                if (testDirection == direction)
                    direction = direction;
                else
                    direction = direction;
                }
#endif
            }

        double GetVolumeOfTrianglesOnSide(double elevation)
        {
        ExpandingPondInfo* outerPond = nullptr;
        double lowPtNum = dtmP->numPoints;
        double totalVol = 0;

            m_outerUnknown = false;

        // OK This works for two point island boundary but not 1.
        for (auto&& startInfo : m_startPoints)
            {
            long& startPnt = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
                startInfo.m_location = ExpandingPondInfo::Location::Inner;

            for (long sp = -1; sp != startPnt;)
                {
                if (sp == -1)
                    sp = startPnt;

                long rotPnt = nodeAddrP(dtmP, sp)->tPtr;
                long endPnt;
                long thisPnt = sp;

                if (rotPnt == sp)
                    {
                    long clc = nodeAddrP(dtmP,sp)->cPtr ;
                    endPnt = clistAddrP(dtmP,clc)->pntNum ;
                    thisPnt = endPnt;
                    }
                else
                    {
                    endPnt = nodeAddrP(dtmP, rotPnt)->tPtr;
                    }

                DPoint3dCP rotPt = pointAddrP(dtmP, rotPnt);

                if (rotPnt < lowPtNum)
                    {
                    lowPtNum = rotPnt;
                    outerPond = &startInfo;
                    }

                do
                    {
                    long anp = bcdtmList_nextClkDtmObject(dtmP, rotPnt, thisPnt);

                    DPoint3dCP thisPt = pointAddrP(dtmP, thisPnt);

#ifdef VOLUME_DEBUG
                    AddTriangle(thisPnt, rotPnt, anp, false);
#endif
                    double cutVol = 0, fillVol = 0, cutArea = 0, fillArea = 0;
                    bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, thisPnt, rotPnt, anp, elevation, cutVol, fillVol, cutArea, fillArea);
                    totalVol += fillVol;
                        thisPnt = anp;
                        } while (thisPnt != endPnt);
                sp = rotPnt;
                }
            }
        BeAssert(outerPond != nullptr);
        if (nullptr != outerPond)
                outerPond->m_location = ExpandingPondInfo::Location::Outer;
        return totalVol;
        }

        bvector<DPoint3d > GetPolygonAtElevation(double elevation, long startPnt)
            {
            bvector<DPoint3d> points;

            for (long sp = -1; sp != startPnt;)
                {
                if (sp == -1)
                    sp = startPnt;

                long rotPnt = nodeAddrP(dtmP, sp)->tPtr;
                long endPnt;
                long thisPnt = sp;

                if (rotPnt == sp)
                    {
                    long clc = nodeAddrP(dtmP, sp)->cPtr;
                    endPnt = clistAddrP(dtmP, clc)->pntNum;
                    thisPnt = endPnt;
                    }
                else
                    {
                    endPnt = nodeAddrP(dtmP, rotPnt)->tPtr;
                    }

                DPoint3dCP rotPt = pointAddrP(dtmP, rotPnt);

                do
                    {
                    long anp = bcdtmList_nextClkDtmObject(dtmP, rotPnt, thisPnt);

                    // If
                    //if (nodeAddrP(dtmP, anp)->hPtr != sp)
                        {
                        DPoint3dCP thisPt = pointAddrP(dtmP, thisPnt);

                        if (thisPt->z <= elevation)
                            {
                            double dz = (thisPt->z - rotPt->z);

                            double p = dz == 0 ? 0 : (elevation - rotPt->z) / dz;
                            DPoint3d newPt = DPoint3d::FromInterpolate(*rotPt, p, *thisPt);

                            if (points.empty() || !points.back().IsEqual(newPt))
                                points.push_back(newPt);
                            }
                        thisPnt = anp;
                        }
                    } while (thisPnt != endPnt);
                    sp = rotPnt;
                }
            // Close Polygon
            BeAssert(!points.empty());
            if (!points.empty())
                points.push_back(points.front());
            return points;
            }

    bvector<bvector<DPoint3d>> GetPolygonAtElevation(double elevation)
        {
        bvector<bvector<DPoint3d>> boundaries;
        // OK This works for two point island boundary but not 1.
        for (auto&& startInfo : m_startPoints)
            {
            long& startPnt = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
            auto points = GetPolygonAtElevation(elevation, startPnt);
            if (!points.empty())
                boundaries.push_back(points);
            }
        return boundaries;
        }

    bvector<bvector<DPoint3d>> GetPolygonAtElevation()
        {
        bvector<bvector<DPoint3d>> boundaries;
        // OK This works for two point island boundary but not 1.
        for (auto&& startInfo : m_startPoints)
            {
            if (!startInfo.m_hasFinished)
                continue;
            long& startPnt = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
            double elevation = pointAddrP(dtmP, startInfo.m_exitPoints.front().exitPoint)->z;
            auto points = GetPolygonAtElevation(elevation, startPnt);
            if (!points.empty())
                boundaries.push_back(points);
            }
        return boundaries;
        }

    void SwapExit(long ptNum, ExpandingPondInfo& epi)
        {
        ExpandingPondInfo* currentEpi = GetCurrentStartInfo();

        BeAssert(nullptr != currentEpi);
        if (nullptr == currentEpi)
            return;

        for (auto it = currentEpi->m_exitPoints.begin(); it != currentEpi->m_exitPoints.end(); it++)
            {
            if (it->exitPoint == ptNum)
                {
                epi.m_exitPoints.push_back(*it);
                it = currentEpi->m_exitPoints.erase(it);
                }
            }

        }

    bool AddTriangleToTPtr(long sp, long& np, long& startPnt, double elevation)
        {
        // If we are removing the last point.
        if (sp == np)
            {
            nodeAddrP(dtmP, sp)->tPtr = dtmP->nullPnt;
            RemoveDuplicatePoint(sp);
            startPnt = -1;
            np = dtmP->nullPnt;
            return true;
            }
        // Is is a 2 line island boundary.
        if (nodeAddrP(dtmP, np)->tPtr == sp)
            {
            // Add triangles to current Area and Vol.
            nodeAddrP(dtmP, np)->tPtr = dtmP->nullPnt;
            RemoveDuplicatePoint(np);

            nodeAddrP(dtmP, sp)->tPtr = sp;
            if (np == startPnt)
                startPnt = sp;
            np = sp;
            return true;
            }
        long outerPt = bcdtmList_nextClkDtmObject(dtmP, sp, np);
        if (outerPt == -99)
            {
            return false;
            }

        if (pointAddrP(dtmP, outerPt)->z < elevation)
            {
            outerPt = outerPt;
            //BeAssert(false);
            }
        if (nodeAddrP(dtmP, outerPt)->tPtr == dtmP->nullPnt)
            {
            nodeAddrP(dtmP, sp)->tPtr = outerPt;
            nodeAddrP(dtmP, outerPt)->tPtr = np;
            np = outerPt;
            //continue;
            return true;
            }
        else if (outerPt == nodeAddrP(dtmP, np)->tPtr)
            {
            long rotPnt = np;
            long endPnt = nodeAddrP(dtmP, rotPnt)->tPtr;
            long thisPnt = bcdtmList_nextClkDtmObject(dtmP, rotPnt, sp);
            if (m_needsVolume)
                {
                DPoint3dCP rotPt = pointAddrP(dtmP, rotPnt);
                while (thisPnt != endPnt)
                    {
                    long anp = bcdtmList_nextClkDtmObject(dtmP, rotPnt, thisPnt);

                    if (nodeAddrP(dtmP, thisPnt)->tPtr == dtmP->nullPnt && nodeAddrP(dtmP, anp)->tPtr == dtmP->nullPnt)
                        {
#ifdef VOLUME_DEBUG
                        AddTriangle(np, thisPnt, anp, true);
#endif
                        AddTriangleToValues(np, thisPnt, anp, elevation);
                        }

                    thisPnt = anp;
                    }
                }
            nodeAddrP(dtmP, sp)->tPtr = outerPt;
            nodeAddrP(dtmP, np)->tPtr = dtmP->nullPnt;
            RemoveDuplicatePoint(np);

            if (np == startPnt)
                startPnt = outerPt;
            np = outerPt;
            return true;
            }
        else
            {
            //DebugSaveTPtr(np);
            // Need to create a new polygon.
            ExpandingPondInfo epi(np, m_nextIndex++);
                ExpandingPondInfo* currentInfo = GetCurrentStartInfo();

                if (currentInfo->m_location != ExpandingPondInfo::Location::Inner)
                    {
                    m_outerUnknown = true;
                    currentInfo->m_location = ExpandingPondInfo::Location::Unknown;
                    }
                else
                    epi.m_location = ExpandingPondInfo::Location::Inner;

                AddDuplicateTPtr(outerPt, m_currentIndex, nodeAddrP(dtmP, outerPt)->tPtr);
            AddDuplicateTPtr(outerPt, epi.index, np);
            SwapExit(np, epi);
            SwapExit(outerPt, epi);
            long prevPnt = np;
            while(true)
                {
                long anp = nodeAddrP(dtmP, prevPnt)->tPtr;
                SwapDuplicate(prevPnt, epi.index);
                SwapExit(prevPnt, epi);
                if (anp == outerPt)
                    break;
                BeAssert(np != anp);
                if (np == anp)
                    return false;

                BeAssert(anp != dtmP->nullPnt);
                prevPnt = anp;
                }
            startPnt = sp;
            nodeAddrP(dtmP, sp)->tPtr = outerPt;

            m_newStartPoints.push_back(epi);
            np = outerPt;
            return true;
            }
        return false;
        }

    void AddTriangleToValues(long p1, long p2, long p3, double elevation)
        {
        double cutVol = 0, fillVol = 0, cutArea = 0, fillArea = 0;
        bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, p1, p2, p3, elevation, cutVol, fillVol, cutArea, fillArea);
        if (AscentTrace())
            {
            //BeAssert(fillArea == 0);
            m_currentVol += cutVol;
            m_currentArea += cutArea;
            }
        else
            {
            //BeAssert(cutArea == 0);
            m_currentVol += fillVol;
            m_currentArea += fillArea;
            }
        }

    int m_i = 0;
    template<class T> bool ExpandPolygon(T& helper, double testZ)
        {
        bool hasExpanded = false;
        bool removedIsland = false;
        bool hasSplit = false;
        while (true)
            {
            for (auto&& startInfo : m_startPoints)
                {
                long& startPoint = startInfo.startPnt;
                if (startInfo.lowZ > testZ || startPoint == -1 || startInfo.m_hasFinished)
                    continue;

                SetCurrentIndex(startInfo.index);

                bool expanded = true;
                while (expanded)
                    {
                    expanded = false;
                    for (long sp = -1; sp != startPoint && sp != dtmP->nullPnt;)
                        {
                        if (sp == -1)
                            sp = startPoint;

                        long np = nodeAddrP(dtmP, sp)->tPtr;

                        // If this is not on the hull.
                        //if (nodeAddrP(dtmP, sp)->hPtr == dtmP->nullPnt)
                            {
                            bool removePoint = helper.func(dtmP, sp, np, testZ);

                            if (removePoint)
                                {
                                if (!AddTriangleToTPtr(sp, np, startPoint, testZ))
                                    {
                                    SetError();
                                    return false;
                                    }

                                //DebugSaveTPtr(m_i++);
                                hasExpanded = true;
                                expanded = true;

                                if (np == dtmP->nullPnt)
                                    {
                                    removedIsland = true;
                                    break;
                                    }
                                continue;
                                }
                            }
                        sp = np;
                        }
                    }
                }

            if (m_newStartPoints.empty())
                break;
            hasSplit = true;
            m_startPoints.insert(m_startPoints.end(), m_newStartPoints.begin(), m_newStartPoints.end());
            m_newStartPoints.clear();
            }

        if (removedIsland)
            {
            bvector<ExpandingPondInfo> newSP;

            for (auto&& sp : m_startPoints)
                {
                if (sp.startPnt != -1)
                    newSP.push_back(sp);
                }
            std::swap(newSP, m_startPoints);
            }
        if (hasSplit)
        FindOuterBoundary();
        return hasExpanded;
        }

    struct RemoveAtElevation
        {
        static bool func(BC_DTM_OBJ* dtmP, long sp, long np, double testZ)
            {
            bool removePoint = false;
            if (nodeAddrP(dtmP, sp)->hPtr == dtmP->nullPnt)
                {
                const double npZ = pointAddrP(dtmP, np)->z;


                if (npZ == testZ)
                    removePoint = true;
                }
            return removePoint;
            }
        };
    bool ExpandPolygon(double testZ)
        {
        RemoveAtElevation helper;
        return ExpandPolygon(helper, testZ);
        }

    // Use the sPtr to store flags.
    //enum class flag
    //    {
    //    isFalse = 0,
    //    isTrue = 1,
    //    initialized = 2,
    //    };

    //union sPtrFlags
    //    {
    //    flag isSumpLine : 2;
    //    flag
    //    }
    void IsSump(long prevPnt, long pnt, long nextPnt, bool& isSump, bool& isExit)
        {
        // Todo Ascent.
        if (nodeAddrP(dtmP, pnt)->hPtr != dtmP->nullPnt)    // hullPtr.
            {
            isExit = true;
            isSump = false;
            return;
            }

        double z = pointAddrP(dtmP, pnt)->z;
        isSump = false;
        isExit = false;

        // This is a single line.
        if (prevPnt == nextPnt)
            {
            if (pointAddrP(dtmP, prevPnt)->z == z)
                isSump = true;
            return;
            }

        while (true)
            {
            long np = bcdtmList_nextAntDtmObject(dtmP, pnt, prevPnt);
            if (np == nextPnt)
                break;

            double npZ = pointAddrP(dtmP, np)->z;
            if (npZ == z)
                isSump = true;
            else if (npZ < z)
                {
                isSump = false;
                isExit = true;
                return;
                }

            prevPnt = np;
            }
        }

    bool IsSump(bmap<long, std::pair<bool,bool>>& cache, long prevPnt, long pnt, long nextPnt)
        {
        // Todo Ascent.

        auto it = cache.find(pnt);
        if (it != cache.end())
            return it->second.first;
        bool isSump, isExit;
        IsSump(prevPnt, pnt, nextPnt, isSump, isExit);
        cache[pnt] = std::pair<bool, bool>(isSump, isExit);
        return isSump;
        }

    bool IsExit(bmap<long, std::pair<bool,bool>>& cache, long prevPnt, long pnt, long nextPnt)
        {
        // Todo Ascent.
        auto it = cache.find(pnt);
        if (it != cache.end())
            return it->second.second;
        bool isSump, isExit;
        IsSump(prevPnt, pnt, nextPnt, isSump, isExit);
        cache[pnt] = std::pair<bool, bool>(isSump, isExit);
        return isExit;
        }

    bool IsRidge(long sp, long np, double testZ)
        {
        long anp = bcdtmList_nextClkDtmObject(dtmP, sp, np);
        return (pointAddrP(dtmP, anp)->z < testZ);
        }

    struct RemoveZSlope
        {
        bmap<long, std::pair<bool, bool>> isSump;
        PondAnalysisR m_analysis;
        bool m_doSumps = false;

        RemoveZSlope(PondAnalysisR analysis) : m_analysis(analysis)
            {
            }

        bool func(BC_DTM_OBJ* dtmP, long sp, long np, double testZ)
            {
            static long testPt = 14849;

            if (np == testPt)
                np = testPt;

            // If this is not on the hull.
            const double npZ = pointAddrP(dtmP, np)->z;
            bool removePoint = false;
            if (npZ == testZ)
                {
                if (sp == np)
                    return true;
                bool isOnHull = nodeAddrP(dtmP, np)->hPtr != dtmP->nullPnt;
                const double spZ = pointAddrP(dtmP, sp)->z;

                if (!isOnHull)
                    {
                    // Is it a flat triangle @ testZ
                    if (spZ == testZ)
                        {
                        long outerPt = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                        BeAssert(outerPt != -99);
                        if (outerPt == -99)
                            {
                            return false;
                            }
                        double outerZ = pointAddrP(dtmP, outerPt)->z;
                        if (outerZ == testZ)
                            removePoint = true;
                        }
                    if (!removePoint)
                        {
                        bool isExit = false;
                        if (isOnHull || m_analysis.IsExit(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr))
                            {
                            isExit = true;
                            }
                        else
                            {
                            if (sp == np)
                                removePoint = true;
                            else
                                {
                                long outerPt = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                                double outerZ = pointAddrP(dtmP, outerPt)->z;
                                if (outerZ == testZ)
                                    removePoint = true;
                                else
                                    {
                                    isExit = m_analysis.IsSump(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr);
                                    }

                                }
                            //    if (npZ == pointAddrP(dtmP, sp)->z)  // Flat triangle on edge remove it.
                            //        {
                            //        long anp = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                            //        removePoint = npZ == pointAddrP(dtmP, anp)->z;
                            //        }
                            //if (m_doSumps && !removePoint)
                            //    {
                            //    // Check if there are sump lines.
                            //    long anp = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                            //    if (pointAddrP(dtmP, anp)->z >= testZ) // ToDo Ascent.
                            //        removePoint = m_analysis.IsSump(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr);
                            //    }
                            //}
                            }
                        }
                    }
                //if (isExit)
                //    m_analysis.GetCurrentStartInfo()->m_exitPoints.push_back(PondExitInfo(np, sp, nodeAddrP(dtmP, np)->tPtr));

                //if (!removePoint)
                //    {
                //    if (pointAddrP(dtmP, np)->z != testZ || !m_analysis.IsRidge(sp, np, testZ))
                //        {
                //        if (m_analysis.IsExit(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr))
                //            {
                //            m_analysis.GetCurrentStartInfo()->m_exitPoints.push_back(PondExitInfo(np, sp, nodeAddrP(dtmP, np)->tPtr));
                //            }
                //        }
                //    }

                }
            return removePoint;
            }
        };

    struct ExpandOverFlatTrianglesHelper
        {
        bmap<long, std::pair<bool, bool>> isSump;
        PondAnalysisR m_analysis;

        ExpandOverFlatTrianglesHelper (PondAnalysisR analysis) : m_analysis(analysis)
            {
            }

        bool func(BC_DTM_OBJ* dtmP, long sp, long np, double testZ)
            {
            // If this is not on the hull.
            const double npZ = pointAddrP(dtmP, np)->z;
            bool removePoint = false;
            bool isOnHull = nodeAddrP(dtmP, np)->hPtr != dtmP->nullPnt;
            if (npZ == testZ)
                {
                if (isOnHull || m_analysis.IsExit(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr))
                    {
                    //if (pointAddrP(dtmP, np)->z != testZ) // || !IsRidge(sp, np, testZ))
                    //    m_analysis.GetCurrentStartInfo()->m_exitPoints.push_back(PondExitInfo(np, sp, nodeAddrP(dtmP, np)->tPtr));
                    }
                else
                    {
                    if (sp == np)
                        removePoint = true;
                    else if (npZ == pointAddrP(dtmP, sp)->z)  // Flat triangle on edge remove it.
                        {
                        long anp = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                        removePoint = npZ == pointAddrP(dtmP, anp)->z;
                        }
                    //if (!removePoint)
                    //    {
                    //    // Check if there are sump lines.
                    //    long anp = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                    //    if (pointAddrP(dtmP, anp)->z >= testZ) // ToDo Ascent.
                    //        removePoint = m_analysis.IsSump(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr);
                    //    }
                    }

                //if (!removePoint)
                //    {
                //    if (pointAddrP(dtmP, np)->z != testZ || !m_analysis.IsRidge(sp, np, testZ))
                //        {
                //        if (m_analysis.IsExit(isSump, sp, np, nodeAddrP(dtmP, np)->tPtr))
                //            {
                //            m_analysis.GetCurrentStartInfo()->m_exitPoints.push_back(PondExitInfo(np, sp, nodeAddrP(dtmP, np)->tPtr));
                //            }
                //        }
                //    }

                }
            return removePoint;
            }
        };

    bool ExpandOverFlatTriangles(double testZ)
        {
        ExpandOverFlatTrianglesHelper helper(*this);
        return ExpandPolygon(helper, testZ);
        }
    bool GetExitPoints(double testZ)
        {
        RemoveZSlope helper(*this);

        bool ret = ExpandPolygon(helper, testZ);

        //if (ret)
            {
            helper.isSump.clear();
            for (auto&& startInfo : m_startPoints)
                {
                if (startInfo.m_hasFinished)
                    continue;

                long& startPoint = startInfo.startPnt;
                SetCurrentIndex(startInfo.index);
                for (long sp = -1; sp != startPoint && sp != dtmP->nullPnt;)
                    {
                    if (sp == -1)
                        sp = startPoint;

                    long np = nodeAddrP(dtmP, sp)->tPtr;

                    bool isOnHull = nodeAddrP(dtmP, np)->hPtr != dtmP->nullPnt;
                    if (pointAddrP(dtmP, np)->z == testZ)
                        {
                        bool isExit = false;
                        if (isOnHull || IsExit(helper.isSump, sp, np, nodeAddrP(dtmP, np)->tPtr))
                            {
                            isExit = true;
                            }
                        else
                            {
                            if (sp == np)
                                {
                                }
                            else
                                {
                                long outerPt = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                                double outerZ = pointAddrP(dtmP, outerPt)->z;
                                if (outerZ == testZ)
                                    {
                                    }
                                else
                                    {
                                    isExit = IsSump(helper.isSump, sp, np, nodeAddrP(dtmP, np)->tPtr);
                                    }

                                }

                            }
                        if (isExit)
                            startInfo.m_exitPoints.push_back(PondExitInfo(np, sp, nodeAddrP(dtmP, np)->tPtr));
                        }
                    sp = np;
                    }

                if (!startInfo.m_exitPoints.empty())
                    startInfo.m_hasFinished = true;
                }
            }
        return ret;
        }

    void SaveBoundaryList()
        {
        for (auto&& startInfo : m_startPoints)
            {
            long& startPnt = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
            startInfo.SaveList(*this);
            }
        ClearCurrentIndex();
        }
    void RestoreBoundaryPoints()
        {
        for (auto&& startInfo : m_startPoints)
            {
            long& startPnt = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
            startInfo.RestoreList(*this);
            }
        ClearCurrentIndex();
        }
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
public:
    void SetToMax(const bvector<bvector<DPoint3d>>& points)
        {
        m_currentVolumePoints = points;
        }

    bool m_isZSlope = false;
    bool FixLowPts(long& startPoint, double z)
        {
        if (pointAddrP(dtmP, startPoint)->z < z)
            startPoint = nodeAddrP(dtmP, startPoint)->tPtr;
        long sp = startPoint;

        do
            {
            long np = nodeAddrP(dtmP, sp)->tPtr;
            if (pointAddrP(dtmP, np)->z < z)
                {
                long nnp = nodeAddrP(dtmP, np)->tPtr;

                nodeAddrP(dtmP, m_lowPnt)->tPtr = nnp;
                nodeAddrP(dtmP, sp)->tPtr = m_lowPnt;
                nodeAddrP(dtmP, np)->tPtr = dtmP->nullPnt;
                return true;
                }
            sp = np;
            } while (sp != startPoint);
            return false;
        }

    void GetLowestPoint(long& outLowPtNum, double& outLowZ)
        {
        outLowPtNum = -1;
        for (auto&& startInfo : m_startPoints)
            {
            if (startInfo.m_hasFinished)
                continue;
            long& startPoint = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);
            long lowPtNum = -1;
            double lowZ = 0;
            long pnt = startPoint;
            // find lowest point.
            do
                {
                if (pnt == dtmP->nullPnt)
                    {
                    SetError();
                    return;
                    }

                double z = pointAddrP(dtmP, pnt)->z;
                if (AscentTrace())
                    {
                    if (z < m_currentZ && (lowPtNum == -1 || z >= lowZ))
                        {
                        lowZ = z;
                        lowPtNum = pnt;
                        }
                    }
                else
                    {
                    if (z > m_currentZ && (lowPtNum == -1 || z <= lowZ))
                        {
                        lowZ = z;
                        lowPtNum = pnt;
                        }
                    }
                pnt = nodeAddrP(dtmP, pnt)->tPtr;
                } while (pnt != startPoint);
            startInfo.lowZ = lowZ;
            startInfo.lowPtNum = lowPtNum;

            if (outLowPtNum == -1 || outLowZ > lowZ)
                {
                outLowPtNum = lowPtNum;
                outLowZ = lowZ;
                }
            }
        }

    void InsertTPtrPolygonAtElevation()
        {
        long startPnt;
        // ToDo Ascent
        // ToDo Hull tests and void tests!
        double testZ = pointAddrP(dtmP, m_lowPnt)->z;
        long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr ;
        long sp = -1;
        while( clc != dtmP->nullPtr )
            {
            sp  = clistAddrP(dtmP,clc)->pntNum ;
            clc = clistAddrP(dtmP,clc)->nextPtr ;
            if (pointAddrP(dtmP, sp)->z > testZ)
                break;
            }

        if (sp == -1)
            {
            SetError();
            return;
            }

        m_outerUnknown = true;
        SetCurrentIndex(m_nextIndex);
        m_startPoints.push_back(ExpandingPondInfo(sp, m_nextIndex++));
        long firstPnt = sp;
        startPnt = sp;
        long rotPnt = m_lowPnt;
        long firstRotPnt = rotPnt;

        while (true)
            {
            long np;
            if (nodeAddrP(dtmP, sp)->hPtr != dtmP->nullPnt)
                {
                m_startPoints.front().startPnt = sp;
                SetError();
                return;
                /*np = nodeAddrP(dtmP, sp)->hPtr;*/
                }
            else
                np = bcdtmList_nextClkDtmObject(dtmP, rotPnt, sp);

            if (pointAddrP(dtmP, np)->z <= testZ)
                {
                rotPnt = np;
                }
            else
                {
                if (nodeAddrP(dtmP, sp)->tPtr == np)
                    {
                    if (np == firstPnt && nodeAddrP(dtmP, firstPnt)->tPtr == dtmP->nullPnt)
                        {
                        long tp = sp;
                        while (nodeAddrP(dtmP, tp)->tPtr != dtmP->nullPnt)
                            tp = nodeAddrP(dtmP, tp)->tPtr;
                        nodeAddrP(dtmP, np)->tPtr = tp;
                        }
                    if (startPnt == sp)
                        startPnt = np;
                    nodeAddrP(dtmP, sp)->tPtr = dtmP->nullPnt; // sp;
                    //ExpandingPondInfo epi(sp, m_nextIndex++);
                    //m_startPoints.push_back(epi);
                    }
                else  if (nodeAddrP(dtmP, np)->tPtr != dtmP->nullPnt)
                    {
                    //if (np == firstPnt)
                    //    {
                    //    long tp = sp;
                    //    while (nodeAddrP(dtmP, tp)->tPtr != dtmP->nullPnt)
                    //        tp = nodeAddrP(dtmP, tp)->tPtr;
                    //    nodeAddrP(dtmP, np)->tPtr = tp;
                    //    }
                    ExpandingPondInfo epi(sp, m_nextIndex++);
                    AddDuplicateTPtr(np, m_currentIndex, nodeAddrP(dtmP, np)->tPtr);
                    AddDuplicateTPtr(np, epi.index, sp);

                    long prevPnt = sp;
                    while(true)
                        {
                        long anp = nodeAddrP(dtmP, prevPnt)->tPtr;
                        SwapDuplicate(prevPnt, epi.index);
                        if (anp == np)
                            break;
                        BeAssert(sp != anp);
                        if (sp == anp)
                            {
                            SetError();
                            return;
                            }

                        BeAssert(anp != dtmP->nullPnt);
                        prevPnt = anp;
                        }


                    m_startPoints.push_back(epi);
                    startPnt = np;
                    }
                else
                    {
                    nodeAddrP(dtmP, np)->tPtr = sp;
                    startPnt = np;
                    }
                sp = np;
                }
            if (sp == firstPnt && firstRotPnt == rotPnt)
                break;
            }
        if (nodeAddrP(dtmP, startPnt)->tPtr == dtmP->nullPnt)
            {
            m_startPoints.front().startPnt = startPnt;

            SetError();
            return;
            }
        m_startPoints.front().startPnt = startPnt;

        FindOuterBoundary();

        for (auto&& si : m_startPoints)
            {
            if (si.m_location == ExpandingPondInfo::Location::Outer)
                {
                DTMDirection direction;
                double area;
                SetCurrentIndex(si.index);
                bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP, si.startPnt, &area, &direction);
                if (direction != DTMDirection::AntiClockwise)
                    {
                    //SetError();
                    bcdtmList_reverseTptrPolygonDtmObject(dtmP, si.startPnt);
                    }
                break;
                }
            }
        }

    void Clear()
        {
        m_duplicateForIndex.clear();
        m_startPoints.clear();
        m_indexPtInfo.clear();
        m_currentIndex = 0;
        m_currentVolumePoints.clear();
        m_hasCurrentGotDuplicates = false;
        m_newStartPoints.clear();
        m_nextIndex = 0;
        m_currentZ = 0;
        m_currentArea = 0;
        m_currentVol = 0;
        }

    void CreateInitialTPtr()
        {
        //LowPointType oldType = m_type;
        //GetType();
        //if (m_type != oldType)
        //    {
        //    if (!(m_type == LowPointType::Triangle && oldType == LowPointType::Edge))
        //        m_type = oldType;
        //    }

        // This shouldn't be needed.
#ifdef DEBUGCHK
        long hasValues = 0;
        bcdtmList_checkForNoneNullTptrValuesDtmObject(dtmP, &hasValues);
        BeAssert(hasValues == 0);
        if (hasValues)
#endif
//        bcdtmList_nullTptrValuesDtmObject(dtmP);

        switch (m_type)
            {
            case LowPointType::Point:
            case LowPointType::Edge:
            //case LowPointType::Triangle:
                {
                long firstPoint;
                if (bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP, m_lowPnt, &firstPoint)) return;
                m_isZSlope = FixLowPts(firstPoint, m_lowZ);
                SetCurrentIndex(m_nextIndex);
                m_startPoints.push_back(ExpandingPondInfo(firstPoint, m_nextIndex++));
                break;
                }
            //case LowPointType::Edge:
                {
                long antPnt = bcdtmList_nextAntDtmObject(dtmP, m_lowPnt, m_pntNum2);
                long clkPnt = bcdtmList_nextClkDtmObject(dtmP, m_lowPnt, m_pntNum2);
                nodeAddrP(dtmP, m_lowPnt)->tPtr = antPnt;
                nodeAddrP(dtmP, antPnt)->tPtr = m_pntNum2;
                nodeAddrP(dtmP, m_pntNum2)->tPtr = clkPnt;
                nodeAddrP(dtmP, clkPnt)->tPtr = m_lowPnt;
                AddTriangleToValues(m_lowPnt, m_pntNum2, antPnt, m_lowZ);
                AddTriangleToValues(m_lowPnt, m_pntNum2, clkPnt, m_lowZ);

                long firstPoint = m_lowPnt;
                m_isZSlope = FixLowPts(firstPoint, m_lowZ);
                SetCurrentIndex(m_nextIndex);
                ExpandingPondInfo epi(m_lowPnt, m_nextIndex++);
                epi.m_location = ExpandingPondInfo::Location::Outer;
                m_outerUnknown = false;
                m_startPoints.push_back(epi);
                break;
                }
            case LowPointType::Triangle:
                {
                nodeAddrP(dtmP, m_lowPnt)->tPtr = m_pntNum2;
                nodeAddrP(dtmP, m_pntNum2)->tPtr = m_pntNum3;
                nodeAddrP(dtmP, m_pntNum3)->tPtr = m_lowPnt;
                AddTriangleToValues(m_lowPnt, m_pntNum2, m_pntNum3, m_lowZ);

                long firstPoint = m_lowPnt;
                m_isZSlope = FixLowPts(firstPoint, m_lowZ);
                SetCurrentIndex(m_nextIndex);
                ExpandingPondInfo epi(firstPoint, m_nextIndex++);
                epi.m_location = ExpandingPondInfo::Location::Outer;
                m_outerUnknown = false;
                m_startPoints.push_back(epi);
                break;
                }

            case LowPointType::PondExit:
                InsertTPtrPolygonAtElevation();
                m_isZSlope = false;
                break;
            }

        ValidateTPtrs();
        }

    void GetType()
        {
        double testZ = pointAddrP(dtmP, m_lowPnt)->z;
        long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr ;
        long sp = -1;
        long np;
        bool hasZSlope = false;
        while( clc != dtmP->nullPtr )
            {
            sp  = clistAddrP(dtmP,clc)->pntNum ;
            clc = clistAddrP(dtmP,clc)->nextPtr ;
            const double z = pointAddrP(dtmP, sp)->z;
            if (z < testZ)
                {
                m_type = LowPointType::PondExit;
                return;
                }
            if (pointAddrP(dtmP, sp)->z == testZ)
                {
                m_pntNum2 = sp;
                hasZSlope = true;
                if (clc == dtmP->nullPtr)
                    np = clistAddrP(dtmP,nodeAddrP(dtmP, m_lowPnt)->cPtr)->pntNum ;
                else
                    np  = clistAddrP(dtmP,clc)->pntNum ;
                if (pointAddrP(dtmP, np)->z == testZ)
                    {
                    m_pntNum2 = np;
                    m_pntNum3 = sp;
                    m_type = LowPointType::Triangle;
                    return;
                    }
                }
            }
        if (hasZSlope)
            {
            long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr ;
            sp  = clistAddrP(dtmP,clc)->pntNum ;
            if (pointAddrP(dtmP, sp)->z == testZ && pointAddrP(dtmP, np)->z == testZ)
                {
                m_pntNum2 = sp;
                m_pntNum3 = np;
                m_type = LowPointType::Triangle;
                return;
                }
            m_type = LowPointType::Edge;
            return;
            }
        m_type = LowPointType::Point;
        }

    void FindBoundaryForVolume(double targetVolume = -1e90, double lowestExitPntZ = -1e90)
        {
        bool hasTargetVol = targetVolume > -1e89;
        bool haslowestExitPntZ = lowestExitPntZ > -1e89;
        m_needsVolume = hasTargetVol;
        double ascentFix = AscentTrace() ? -1 : 1;
        double ptZ = m_currentZ;
        if (targetVolume > m_currentVol && !m_isZSlope)
            Clear();

        BoundaryListHelper __helper(*this);
        if (m_startPoints.empty())
            {
            CreateInitialTPtr();

            if (m_errorStatus)
                return;
            ptZ = m_lowZ;

            long lowPtNum;
            double lowZ;

            GetLowestPoint(lowPtNum, lowZ);

            GetExitPoints(ptZ);
            FindOuterBoundary();

            }
        // Find all points on the polygon that are lower than the exitPoint and add the points around it.
        bool expanded = true;

        double previousVolume = 0;
        long previousNextIndex = m_nextIndex;
        bool firstTime = true;

        while (expanded)
            {
            bool hasOuterExited = false;
            for (auto&& startInfo : m_startPoints)
                {
                    if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
                    {
                    hasOuterExited = startInfo.m_hasFinished;
                    break;
                    }
                else if (haslowestExitPntZ && pointAddrP(dtmP, startInfo.m_exitPoints[0].exitPoint)->z < lowestExitPntZ)
                    hasOuterExited = true;
                }

            if (hasOuterExited)
                {
                m_currentZ = ptZ;
                break;
                }

            // This also needs to scan around the point
            if (!firstTime && !ExpandPolygon(m_currentZ))
                {
                //SetError(); // Didn't expand, can't find exit point.
                //return;
                }
            firstTime = false;
            if (m_errorStatus)
                return;
            expanded = true;

            long lowPtNum;
            double lowZ;

            GetLowestPoint(lowPtNum, lowZ);

            if (m_currentZ == lowZ)
                {
                // This shouldn't happen
                SetError();
                return;
                }
#ifdef VOLUME_DEBUG
            if (checkVolumes)
                {
                const double aV = GetVolume(m_currentZ, true);
                const double bV = GetVolume(m_currentZ, false);
                const double tstVol = aV + bV;

                if (fabs(tstVol - previousVolume) > 0.01)
                    previousVolume = tstVol;
                }
#endif

            // Test volume.
            if (hasTargetVol)
                {
                const double newVolFromArea = m_currentArea * (ascentFix * (lowZ - m_currentZ));
                const double newVolFromPartials = GetVolumeOfTrianglesOnSide(lowZ);
                double newVol = m_currentVol + newVolFromArea + newVolFromPartials;

#ifdef VOLUME_DEBUG
                if (checkVolumes)
                    {
                    const double aV = GetVolume(lowZ, true);
                    const double bV = GetVolume(lowZ, false);
                    double chkVol = aV + bV;

                    if (fabs(chkVol - newVol) > 0.3)
                        newVol = chkVol;
                    }
#endif
                if (newVol > targetVolume)
                    {
                    double minZ = m_currentZ;
                    double maxZ = lowZ;
                    double minVol = previousVolume;
                    double maxVol = newVol;

                    // Calculate new Volume.
                    const double tol = 0.1;
                    const double ptol = 0.001;
                    double z = minZ;
                    while (true)
                        {

                        if (minZ >= maxZ || minVol >= maxVol)
                            {
                            SetError();
                            return;
                            }
                        double percent = (targetVolume - minVol) / (maxVol - minVol);
                        z = minZ + (maxZ - minZ) * percent;

                        const double newVolFromArea2 = m_currentArea * (ascentFix * (z - m_currentZ));
                        const double newVolFromPartials2 = GetVolumeOfTrianglesOnSide(z);
                        newVol = m_currentVol + newVolFromArea2 + newVolFromPartials2;

#ifdef VOLUME_DEBUG
                        if (checkVolumes)
                            {
                            const double aV = GetVolume(dtmP, z, true);
                            const double bV = GetVolume(dtmP, z, false);
                            double chkVol = aV + bV;

                            if (fabs(chkVol - newVol) > 0.3)
                                newVol = chkVol;
                            }
#endif

                        if (fabs(targetVolume - newVol) < tol)
                            break;
                        if (newVol > targetVolume)
                            {
                            maxZ = z;
                            maxVol = newVol;
                            }
                        else
                            {
                            minZ = z;
                            minVol = newVol;
                            }
                        }
                    m_currentVolumePoints = GetPolygonAtElevation(z);
                    return;
                    }
                previousVolume = newVol;
                }
            else
                {
                // Mark outer boundary.
                if (m_nextIndex != previousNextIndex)
                    FindOuterBoundary();
                }

            previousNextIndex = m_nextIndex;
            ptZ = lowZ;
            m_currentVol += m_currentArea * (ascentFix * (ptZ - m_currentZ));
            m_currentZ = ptZ;

            GetExitPoints(m_currentZ);

            // ToDo - Check for exit point.
            // should do something with all triangle points at that elevation.
            //GetExitPointsOld(lowZ);
            if (false)
                DebugSaveTPtr();
            }

        for (auto&& startInfo : m_startPoints)
            {
            if (startInfo.m_hasFinished)
                {
                GetPolygonAtElevation(m_currentZ);
                return;
                }
            }
        return;
        }

    bvector<DPoint3d> GetOuterBoundary()
        {
        for (auto&& startInfo : m_startPoints)
            {
                if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
                {
                if (!startInfo.m_hasFinished)
                    break;
                BoundaryListHelper helper(*this);
                long& startPnt = startInfo.startPnt;
                SetCurrentIndex(startInfo.index);
                return GetPolygonAtElevation(pointAddrP(dtmP, startInfo.m_exitPoints[0].exitPoint)->z, startPnt);
                }
            }
        return bvector<DPoint3d>();
        }

    bvector<bvector<DPoint3d>> GetBoundary()
        {
        bvector<bvector<DPoint3d>> b;
        b.push_back(GetOuterBoundary());
        return b;
        //BoundaryListHelper __helper(*this);

        //return GetPolygonAtElevation();

        }
    bvector<PondExitInfo> GetOuterExits()
        {
        for (auto&& startInfo : m_startPoints)
            {
                if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
                {
                //if (startInfo.m_exitPoints.size() >= 2)
                //    {
                //    // ToDo More than 1 exit.
                //    bvector<PondExitInfo> pts;
                //    pts.push_back(startInfo.m_exitPoints[0]);
                //    return pts;
                //    }

                return startInfo.m_exitPoints;
                }
            }
        return bvector<PondExitInfo>();
        }

    void FindPond()
        {
        FindBoundaryForVolume();
        }

    void DebugSaveTPtr()
        {
        DebugSaveTPtr(-1);
        }
    void DebugSaveTPtr(int n)
        {
        int curIndex = m_currentIndex;
        TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::Create();
        for (auto&& startInfo : m_startPoints)
            {
            if (startInfo.startPnt == -1)
                continue;

            long& startPoint = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);

            bvector<DPoint3d> pts;
            long sp = startPoint;

            pts.push_back(*pointAddrP(dtmP, sp));
            do
                {
                long np = nodeAddrP(dtmP, sp)->tPtr;
                pts.push_back(*pointAddrP(dtmP, np));
                sp = np;
                } while (sp != startPoint);
            DTMFeatureId fId;
            dtm->AddLinearFeature(DTMFeatureType::Breakline, pts.data(), (int)pts.size(), &fId);
            }

        for (auto&& startInfo : m_newStartPoints)
            {
            if (startInfo.startPnt == -1)
                continue;

            long& startPoint = startInfo.startPnt;
            SetCurrentIndex(startInfo.index);

            bvector<DPoint3d> pts;
            long sp = startPoint;

            pts.push_back(*pointAddrP(dtmP, sp));
            do
                {
                long np = nodeAddrP(dtmP, sp)->tPtr;
                pts.push_back(*pointAddrP(dtmP, np));
                sp = np;
                } while (sp != startPoint);
                DTMFeatureId fId;
                dtm->AddLinearFeature(DTMFeatureType::Breakline, pts.data(), (int)pts.size(), &fId);
            }

        if (n == -1)
            dtm->Save(L"d:\\tin.bcdtm");
        else
        {
            WString l;
            l.Sprintf(L"d:\\tin%d.bcdtm", n);
            dtm->Save(l.GetWCharCP());
        }
        SetCurrentIndex(curIndex);
        }
    };


//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceStartPoint::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    double z;
    long pointType, trgPnt1, trgPnt2, trgPnt3;
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    m_finished = true;

    if (bcdtmFind_triangleForPointDtmObject(dtmP, m_startPoint.x, m_startPoint.y, &z, &pointType, &trgPnt1, &trgPnt2, &trgPnt3))
        return;

    if (pointType == 0) // Point off triangulation.
        return;

    // Check Point To Point Tolerance
    if (bcdtmMath_distance(m_startPoint.x, m_startPoint.y, pointAddrP(dtmP, trgPnt1)->x, pointAddrP(dtmP, trgPnt1)->y) <= dtmP->ppTol)
        {
        pointType = 1; trgPnt2 = trgPnt3 = dtmP->nullPnt;
        }
    if (trgPnt2 != dtmP->nullPnt && bcdtmMath_distance(m_startPoint.x, m_startPoint.y, pointAddrP(dtmP, trgPnt2)->x, pointAddrP(dtmP, trgPnt2)->y) <= dtmP->ppTol)
        {
        pointType = 1; trgPnt1 = trgPnt2; trgPnt2 = trgPnt3 = dtmP->nullPnt;
        }
    if (trgPnt3 != dtmP->nullPnt && bcdtmMath_distance(m_startPoint.x, m_startPoint.y, pointAddrP(dtmP, trgPnt3)->x, pointAddrP(dtmP, trgPnt3)->y) <= dtmP->ppTol)
        {
        pointType = 1; trgPnt1 = trgPnt3; trgPnt2 = trgPnt3 = dtmP->nullPnt;
        }

    bool pntInVoid = false;
    if (pointType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP, trgPnt1)->PCWD)) pntInVoid = true;

    if (pointType == 2 || pointType == 3)
        {
        if (bcdtmList_testForVoidLineDtmObject(dtmP, trgPnt1, trgPnt2, pntInVoid))
            {
            SetError();
            return;
            }
        }
    if (pointType == 4)
        {
        if (bcdtmList_testForVoidTriangleDtmObject(dtmP, trgPnt1, trgPnt2, trgPnt3, pntInVoid))
            {
            SetError();
            return;
            }
        }
    if (pntInVoid)
        {
        SetError(L"Maximum Descent Start Point In Void");
        return;
        }

    if (pointType == 1) /// OnPoint
        {
        auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, trgPnt1, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
        m_children.push_back(child);
        return;
        }

    /*
    ** Test For Zero Slope Triangle
    */
    if (trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt)
        {
        if (pointAddrP(dtmP, trgPnt1)->z == pointAddrP(dtmP, trgPnt2)->z && pointAddrP(dtmP, trgPnt1)->z == pointAddrP(dtmP, trgPnt3)->z)
            {
            auto child = TracePondTriangle::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
            newFeatures.push_back(child);
            return;
            }
        }

    //if (trgPnt3 != dtmP->nullPnt && bcdtmMath_distance(m_startPoint.x, m_startPoint.y, pointAddrP(dtmP, trgPnt3)->x, pointAddrP(dtmP, trgPnt3)->y) <= dtmP->ppTol)
    //    {
    //    trgPnt1 = trgPnt3; trgPnt2 = trgPnt3 = dtmP->nullPnt;
    //    }

    /*
    ** Set Triangle Anti Clockwise
    */
    if (trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt)
        {
        if (bcdtmMath_pointSideOfDtmObject(dtmP, trgPnt1, trgPnt2, trgPnt3) < 0)
            std::swap(trgPnt2, trgPnt3);
        auto child = TraceInTriangle::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
        newFeatures.push_back(child);
        m_children.push_back(child);

        }
    else if (trgPnt2 != dtmP->nullPnt && trgPnt3 == dtmP->nullPnt)
        {
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 < 0) { SetError(); return; }
        if (pointAddrP(dtmP, trgPnt3)->z < z && bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
            {
            auto child = TraceOnEdge::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }

        std::swap(trgPnt1, trgPnt2);
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 < 0) { SetError(); return; }
        if (pointAddrP(dtmP, trgPnt3)->z < z && bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
            {
            auto child = TraceOnEdge::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceInTriangle::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    double descentAngle, ascentAngle, slope, angle;

    //if (dbg) bcdtmWrite_message(0, 0, 0, "Start Point Internal To Triangle");
    bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP, pnt1, pnt2, pnt3, &descentAngle, &ascentAngle, &slope);
    //if (dbg) bcdtmWrite_message(0, 0, 0, "Slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf", slope, descentAngle, ascentAngle);

    if (m_tracer.AscentTrace()) angle = ascentAngle;
    else angle = descentAngle;

    long nextPnt1, nextPnt2, nextPnt3;
    DPoint3d nextPt;
    if (bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(dtmP, m_tracer.AscentTrace() ? 1 : 2, pnt1, pnt2, pnt3, m_startPt.x, m_startPt.y, &nextPnt1, &nextPnt2, &nextPnt3, &nextPt.x, &nextPt.y, &nextPt.z))
        {
        SetError();
        return;
        }
    m_points.push_back(nextPt);
    m_finished = true;
    if (nextPnt3 != dtmP->nullPnt)  // Check this doesn't flow off the edge of triangulation.
        {
        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, angle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::ProcessZSlopeTriangle(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    //if (dbg) bcdtmWrite_message(0, 0, 0, "Zero Slope Triangle From Triangle Edge");
    if (m_tracer.m_zeroSlopeOption == ZeroSlopeTraceOption::TraceLastAngle)
        {
        //if (dbg) bcdtmWrite_message(0, 0, 0, "** Tracing At Last Angle");

        // Check Last Angle has Been Initialised

        BeAssert (lastAngle != -99.99);
        if (lastAngle == -99.99)
            {
            SetErrorNotImplemented(L"No LastAngle set.");
            return;
            //lastAngle = bcdtmMath_getPointAngleDtmObject(dtmP, startPnt1, startPnt2) + DTM_2PYE / 4.0;
            //while (lastAngle > DTM_2PYE) lastAngle = lastAngle - DTM_2PYE;
            }
        DPoint3d nextPt;
        long nextPnt1, nextPnt2, nextPnt3;
        if (bcdtmDrainage_calculateAngleIntersectOfRadialFromTriangleEdgeWithTriangleDtmObject(dtmP, pnt1, pnt2, pnt3, m_pt.x, m_pt.y, lastAngle, &nextPt.x, &nextPt.y, &nextPt.z, &nextPnt1, &nextPnt2, &nextPnt3)) { SetError(); return; }
        m_points.push_back(nextPt);

        if (nextPnt3 == dtmP->nullPnt)  // This flows of the triangulation?
            {
            m_finished = true;
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, pnt1, nextPt, lastAngle);
            m_children.push_back(child);
            return;
            }
        long hullPoint = 0;
        if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, nextPnt3, &hullPoint)) { SetError(); return; }

        // Flows of the edge of the triangulation.
        if (hullPoint)
            {
            m_finished = true;
            return;
            }

        if (!bcdtmList_testLineDtmObject(dtmP, nextPnt1, nextPnt2) || !bcdtmList_testLineDtmObject(dtmP, nextPnt1, nextPnt3) || !bcdtmList_testLineDtmObject(dtmP, nextPnt2, nextPnt3))
            pnt3 = pnt3;
        m_pt = nextPt;
        pnt1 = nextPnt1;
        pnt2 = nextPnt2;
        pnt3 = nextPnt3;

        }
    else
        {
        //if (dbg) bcdtmWrite_message(0, 0, 0, "** Placing Pond Over Zero Slope Triangle");
        //if (bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP, pnt1, pnt3, pnt2, nullptr, 0, 0, exitPointP, priorPointP, nextPointP, nullptr)) goto errexit;
        auto child = TracePondTriangle::Create(m_tracer, *this, pnt1, pnt2, pnt3, m_pt);
        newFeatures.push_back(child);
        m_children.push_back(child);
        m_finished = true;
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::ProcessZSlopeLine(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    //newFeatures.push_back(TraceZSlopeLine::Create(m_tracer, *this, pnt1, pnt2, m_pt));

    long prevPnt = bcdtmList_nextClkDtmObject(dtmP, pnt1, pnt2);

    if (pointAddrP(dtmP, prevPnt)->z == m_pt.z)
        {
        auto child = TracePondTriangle::Create(m_tracer, *this, pnt1, pnt2, pnt3, *pointAddrP(dtmP, pnt1));
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    else
        {
        auto child = TracePondEdge::Create(m_tracer, *this, pnt1, pnt2, *pointAddrP(dtmP, pnt1));
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    m_finished = true;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long nextPnt1, nextPnt2 = dtmP->nullPnt, nextPnt3 = dtmP->nullPnt;
    DPoint3d nextPt;
    if (pnt3 == dtmP->nullPnt)
        {
        // This flows of the edge of the triangulation.
        m_finished = true;
        return;
        }

    if (nodeAddrP(dtmP, pnt1)->hPtr == pnt2)
        {
        // This is on the edge
        m_finished = true;
        return;
        }

    if (nodeAddrP(dtmP, pnt2)->hPtr == pnt1)
        {
        // This is on the edge
        m_finished = true;
        return;
        }

    if (pointAddrP(dtmP, pnt1)->z == pointAddrP(dtmP, pnt2)->z  && pointAddrP(dtmP, pnt1)->z == pointAddrP(dtmP, pnt3)->z)
        {
        ProcessZSlopeTriangle(newFeatures);
        return;
        }

    bool voidTriangle;
    int flowDirection;
    if (bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP, m_tracer.drainageTablesP, pnt1, pnt2, pnt3, voidTriangle, flowDirection)) { SetError(); return; }

    if (m_tracer.AscentTrace()) flowDirection -= flowDirection;

    if (flowDirection >= 0)
        {
        double z1 = pointAddrP(dtmP, pnt1)->z;
        double z2 = pointAddrP(dtmP, pnt2)->z;

        if (z1 == z2)
            {
            ProcessZSlopeLine(newFeatures);
            return;
            }
        double dz = z2 - z1;
        if (m_tracer.AscentTrace()) dz = -dz;
        if (dz > 0)
            std::swap(pnt1, pnt2);

        nextPt = *pointAddrP(dtmP, pnt2);
        m_points.push_back(nextPt);
        m_finished = true;

        lastAngle = bcdtmMath_getAngle(pointAddrP(dtmP, pnt1)->x, pointAddrP(dtmP, pnt1)->y, nextPt.x, nextPt.y);

        auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, pnt2, nextPt, lastAngle);
        m_children.push_back(child);
        return;
        }

    double slope, descentAngle, ascentAngle;
    if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, m_tracer.drainageTablesP, pnt1, pnt3, pnt2, voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS) { SetError(); return; }
    //    if (dbg) bcdtmWrite_message(0, 0, 0, "slope = %8.4lf ascentAngle = %12.10lf descentAngle = %12.10lf", slope, ascentAngle, descentAngle);
    /*
    **        Calculate Radial Out From Start X And Start Y At Descent Angle
    */
    if (m_tracer.AscentTrace()) descentAngle = ascentAngle;

    lastAngle = descentAngle;
    const double dx = dtmP->xMax - dtmP->xMin;
    const double dy = dtmP->yMax - dtmP->yMin;

    const double radius = sqrt(dx * dx + dy * dy);
    const double xRad = m_pt.x+ radius * cos(descentAngle);
    const double yRad = m_pt.y + radius * sin(descentAngle);
    /*
    **        Determine Triangle Flow Out Edge
    */
    bool tracePointFoundP = true;
    int sdof = bcdtmMath_sideOf(m_pt.x, m_pt.y, xRad, yRad, pointAddrP(dtmP, pnt3)->x, pointAddrP(dtmP, pnt3)->y);
    //if (dbg) bcdtmWrite_message(0, 0, 0, "Flow Out Edge Side Of = %2d", sdof);
    /*
    **        Flow Passes Through P3
    */
    if (sdof == 0)
        {
        nextPnt1 = pnt3;
        nextPt = *pointAddrP(dtmP, pnt3);
        }
    /*
    **        Flow Intersects Edge P2-P3
    */
    else if (sdof > 0)
        {
        long intPnt;
        if (bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP, m_pt.x, m_pt.y, xRad, yRad, pnt3, pnt2, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt)) { SetError(); return; }
        //if (dbg) bcdtmWrite_message(0, 0, 0, "intPnt = %10ld", intPnt);
        if (intPnt != dtmP->nullPnt)
            {
            nextPnt1 = intPnt;
            nextPt = *pointAddrP(dtmP, nextPnt1);
            }
        else
            {
            nextPnt1 = pnt3;
            nextPnt2 = pnt2;
            if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0) { SetError(); return; }
            }
        }
    /*
    **        Flow Intersects Edge P1-P3
    */
    else if (sdof < 0)
        {
        long intPnt;
        if (bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP, m_pt.x, m_pt.y, xRad, yRad, pnt1, pnt3, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt)) { SetError(); return; }
        if (intPnt != dtmP->nullPnt)
            {
            nextPnt1 = intPnt;
            nextPt = *pointAddrP(dtmP, nextPnt1);
            }
        else
            {
            nextPnt1 = pnt1;
            nextPnt2 = pnt3;
            if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0) { SetError(); return; }
            }
        }
    m_points.push_back(nextPt);

    if (pnt2 == dtmP->nullPnt)
        {
        m_finished = true;
        auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, nextPnt1, nextPt, lastAngle);
        m_children.push_back(child);
        return;
        }
    if (pnt3 == dtmP->nullPnt)
        {
        m_finished = true;
        if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0) { SetError(); return; }
        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        return;
        }
    m_pt = nextPt;
    pnt1 = nextPnt1;
    pnt2 = nextPnt2;
    pnt3 = nextPnt3;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceZSlopeLine::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
/*
BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long exitPoint, priorPoint, nextPoint;

    //if (dbg) bcdtmWrite_message(0, 0, 0, "Zero Slope Sump Line Detected");
    if (m_tracer.m_falseLowDepth != 0)
    {
    if (m_tracer.drainageTablesP != nullptr && m_tracer.drainageTablesP->SizeOfZeroSlopeLinePondTable() > 0)
    {
    int exitPnt, priorPnt, nextPnt;
    m_tracer.drainageTablesP->FindZeroSlopeLinePond(pnt1, pnt2, exitPnt, priorPnt, nextPnt);
    *exitPointP = exitPnt;
    *priorPointP = priorPnt;
    *nextPointP = nextPnt;
    }
    else
    {
    if (bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP, nullptr, nullptr, nullptr, pnt1, pnt2, false, false, exitPointP, priorPointP, nextPointP, &sumpLinesP, &numSumpLines, &polygonP, nullptr, &area)) goto errexit;
    if (sumpLinesP != nullptr)
    {
    free(sumpLinesP); sumpLinesP = nullptr;
    }
    if (polygonP != nullptr) bcdtmPolygon_deletePolygonObject(&polygonP);
    }
    }
    //          Get Exit Point From Zero Sump Lines ** Added 8/1/2007
    else
    {
    if (dbg) bcdtmWrite_message(0, 0, 0, "Determining Pond About Zero Slope Sump Line");
    //                if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,startPnt1,startPnt2,nullptr,0,0,exitPointP,priorPointP,nextPointP,&sumpLinesP,&numSumpLines,&polygonP, nullptr)) goto errexit ;
    if (sumpLinesP != nullptr)
    {
    free(sumpLinesP); sumpLinesP = nullptr;
    }
    if (polygonP != nullptr) bcdtmPolygon_deletePolygonObject(&polygonP);
    if (*exitPointP != dtmP->nullPnt)
    {
    if (pointAddrP(dtmP, *exitPointP)->z == pointAddrP(dtmP, startPnt1)->z)
    {
    *nextPnt1P = *exitPointP;
    *nextXP = pointAddrP(dtmP, *exitPointP)->x;
    *nextYP = pointAddrP(dtmP, *exitPointP)->y;
    *nextZP = pointAddrP(dtmP, *exitPointP)->z;
    *tracePointFoundP = true;
    }
    }
    }
    */
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnPoint* TraceOnPoint::GetOrCreate(bvector<TraceFeaturePtr>& newFeatures, DrainageTracer& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle)
    {
    TraceOnPoint* existingPoint = tracer.FindExistingOnPoint(startPtNum);

    if (nullptr != existingPoint)
        {
        return existingPoint;
        }
    TraceOnPointPtr child = new TraceOnPoint(tracer, parent, startPtNum, startPoint, lastAngle);
    newFeatures.push_back(child);
    tracer.AddOnPoint(startPtNum, *child);
    return child.get();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnPoint::ProcessZSlope(bvector<TraceFeaturePtr>& newFeatures, long descentPnt1, long descentPnt2)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    if (m_tracer.m_zeroSlopeOption == ZeroSlopeTraceOption::TraceLastAngle)
        {
        //if (dbg)bcdtmWrite_message(0, 0, 0, "** Tracing At Last Angle");

        // Check Last Angle has Been Initialised
        long oldDescentPnt1 = descentPnt1;
        long oldDescentPnt2 = descentPnt2;
        BeAssert(m_lastAngle != -99.99);
        if (m_lastAngle == -99.99)
            {
            m_lastAngle = bcdtmMath_getAngle(m_pt.x, m_pt.y, (pointAddrP(dtmP, descentPnt1)->x + pointAddrP(dtmP, descentPnt2)->x) / 2.0, (pointAddrP(dtmP, descentPnt1)->y + pointAddrP(dtmP, descentPnt2)->y) / 2.0);
            }
        else
            bcdtmDrainage_findEdgeFromPointAndAngleDtmObject(dtmP, m_ptNum, m_lastAngle, descentPnt1, descentPnt2);

        DPoint3d nextPt;
        long intPnt, nextPnt1, nextPnt2, nextPnt3;
        if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, m_ptNum, descentPnt1, descentPnt2, m_lastAngle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt)) { SetError(); return; }
        if (m_pt.z != nextPt.z)
            {
            //*processP = false;

            descentPnt1 = oldDescentPnt1;
            descentPnt2 = oldDescentPnt2;
            //// We can't flow out of the Z slope area using last direction use the other routine.
            //if (dbg) bcdtmWrite_message(0, 0, 0, "** Placing Pond About Zero Slope Triangle");
            //long exitPoint, priorPoint, nextPoint;
            //if (bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP, m_ptNum, descentPnt1, descentPnt2, nullptr, false, false, &exitPoint, &priorPoint, &nextPoint, nullptr)) goto errexit;
            //*nextPnt1P = startPnt;
            //*nextPnt2P = descentPnt1;
            //*nextPnt3P = descentPnt2;
            }
        else
            {
            if (intPnt != dtmP->nullPnt) nextPnt1 = intPnt;
            else
                {
                m_points.push_back(nextPt);
                nextPnt1 = descentPnt1;
                nextPnt2 = descentPnt2;
                if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, descentPnt1, descentPnt2)) < 0) { SetError(); return; }
                auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, m_lastAngle);
                newFeatures.push_back(child);
                m_children.push_back(child);
                return;
                }
            }
        }

    //if (dbg) bcdtmWrite_message(0, 0, 0, "** Placing Pond About Zero Slope Triangle");
    //long exitPoint, priorPoint, nextPoint;
    //if (bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP, m_ptNum, descentPnt1, descentPnt2, nullptr, false, false, &exitPoint, &priorPoint, &nextPoint, nullptr, nullptr)) { SetError(); return; }
    auto pond = TracePondEdge::Create(m_tracer, *this, descentPnt1, descentPnt2, *pointAddrP(dtmP, descentPnt1));
    m_tracer.AddPond(*pond);
    newFeatures.push_back(pond);
    m_children.push_back(pond);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnPoint::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long pnt1, pnt2;
    long type;
    double slope, angle;

    long hullPoint = 0;
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_ptNum, &hullPoint)) { SetError(); return; }

    if (hullPoint)
        {
        m_onHullPoint = true;
        m_finished = true;
        return;
        }

    if (m_tracer.AscentTrace())
        {
        if (bcdtmDrainage_scanPointForMaximumAscentDtmObject(dtmP, m_tracer.drainageTablesP, m_ptNum, m_prevPtNum, &type, &pnt1, &pnt2, &slope, &angle)) { SetError(); return; }
        }
    else
        {
        if (bcdtmDrainage_scanPointForMaximumDescentDtmObject(dtmP, m_tracer.drainageTablesP, m_ptNum, m_prevPtNum, &type, &pnt1, &pnt2, &slope, &angle)) { SetError(); return; }
        }

    // type : 0 = Low/HighPoint
    //        1 = To a Point
    //        2 = Down a Triangle

    m_finished = true;

    if (type == 0)
        {
        auto pond = TracePondLowPoint::Create(m_tracer, *this, m_ptNum, m_pt);
        m_tracer.AddPond(*pond);
        newFeatures.push_back(pond);
        m_children.push_back(pond);
        return;

        }
    else if (type == 1)
        {
        if (slope == 0) // ZeroSlope
            {
            auto child = TracePondEdge::Create(m_tracer, *this, m_ptNum, pnt1, m_pt);
            newFeatures.push_back(child);
            m_children.push_back(child);
            //long pnt3 = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
            //newFeatures.push_back(TraceOnEdge::Create(m_tracer, *this, pnt1, pnt2, pnt3, *pointAddrP(dtmP, pnt1)));
            m_finished = true;
            return;
            }
        else
            {
            // ToDo check for SumpLine.
            DPoint3d nextPt = *pointAddrP(dtmP, pnt1);
            m_lastAngle = bcdtmMath_getAngle(m_pt.x, m_pt.y, nextPt.x, nextPt.y);

            m_prevPtNum = m_ptNum;
            m_ptNum = pnt1;
            m_pt = nextPt;
            m_points.push_back(m_pt);
            m_finished = false;
            return;
            }
        }
    else if (type == 2)
        {
        if (slope == 0.0)
            {
            ProcessZSlope(newFeatures, pnt1, pnt2);
            return;
            }
        long pnt3 = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
        auto child = TraceOnEdge::Create(m_tracer, *this, pnt1, pnt2, pnt3, m_pt, m_lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        return;
        }
    else
        SetError(L"Unknown type");
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
int bcdtmDrainage_determinePondAboutExitPointDtmObject
(
    BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
    DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
    DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
    int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
    DTMFeatureCallback        loadFunctionP,              // ==> Pointer To Call Back Function
    long                      lowPoint,                   // ==> Low Point To Create Pond About
    bool                      loadFlag,                   // ==> Pass The Pond Boundaries Back
    bool                      boundaryFlag,               // ==> ???
    long                      *exitPointP,                // <== Pond Exit Point On Tptr Polygon
    long                      *priorPointP,               // <== Prior Point To Exit Point On Tptr Polygon
    long                      *nextPointP,                // <== Next Point After Exit Point On Tptr Polygon
    DTM_POLYGON_OBJ           **polygonPP,                // <== Polygon Object To Store Pond Boundary
    void                      *userP                      // ==> User Pointer Passed Back To Call back Function
)
/*
** This Function Determines The Pond About A Low Point
*/
    {
    int     ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0), cdbg = DTM_CHECK_VALUE(0);
    long    startPoint, ofs, node;
    bool    pondValid = true;
    DTM_TIN_NODE   *nodeP;

    long numPondPts = 0;
    DPoint3d *pondPtsP = nullptr;
    BC_DTM_OBJ  *tempDtmP = nullptr;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Determing Pond About Low Point");
        bcdtmWrite_message(0, 0, 0, "dtmP                  = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "drainageTablesP       = %p", drainageTablesP);
        bcdtmWrite_message(0, 0, 0, "zeroSlopePolygonsP    = %p", zeroSlopePolygonsP);
        bcdtmWrite_message(0, 0, 0, "zeroSlopePointsIndexP = %p", zeroSlopePointsIndexP);
        bcdtmWrite_message(0, 0, 0, "LoadFunctionP         = %p", loadFunctionP);
        bcdtmWrite_message(0, 0, 0, "Low Point             = %8ld ** %10.4lf %10.4lf %10.4lf", lowPoint, pointAddrP(dtmP, lowPoint)->x, pointAddrP(dtmP, lowPoint)->y, pointAddrP(dtmP, lowPoint)->z);
        bcdtmWrite_message(0, 0, 0, "loadFlag              = %8ld", loadFlag);
        bcdtmWrite_message(0, 0, 0, "boundaryFlag          = %8ld", boundaryFlag);
        if (dbg == 2) bcdtmList_writeCircularListForPointDtmObject(dtmP, lowPoint);
        }
    /*
    ** Initialise
    */
    *exitPointP = dtmP->nullPnt;
    *priorPointP = dtmP->nullPnt;
    *nextPointP = dtmP->nullPnt;

    if ((loadFlag || boundaryFlag) && polygonPP == nullptr) goto errexit;
    if (polygonPP != nullptr && *polygonPP != nullptr) bcdtmPolygon_deletePolygonObject(polygonPP);
    /*
    ** Check For None Null Tptr Or Sptr Values
    */
    if (cdbg)
        {
        bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP, 1);
        bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP, 1);
        }
    /*
    ** Place Pond Around Low Point
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Placing Tptr Polygon About Low Point");
    if (bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP, lowPoint, &startPoint)) goto errexit;

    // Find all points on the polygon that are lower than the exitPoint and add the points around it.
    double ptZ = pointAddrP(dtmP, lowPoint)->z;
    bool expanded = true;
    while (expanded)
        {
        expanded = false;
        for(long sp = -1; sp != startPoint;)
            {
            if (sp == -1)
                sp = startPoint;
            long np = nodeAddrP(dtmP, sp)->tPtr;

            if (nodeAddrP(dtmP, sp)->hPtr == dtmP->nullPnt && nodeAddrP(dtmP, np)->hPtr == dtmP->nullPnt)
                {
                if (pointAddrP(dtmP, np)->z <= ptZ)
                    {
                    long outerPt = bcdtmList_nextClkDtmObject(dtmP, sp, np);
                    if (outerPt == -99)
                        goto errexit;

                    if (nodeAddrP(dtmP, outerPt)->tPtr == dtmP->nullPnt)
                        {
                        nodeAddrP(dtmP, sp)->tPtr = outerPt;
                        nodeAddrP(dtmP, outerPt)->tPtr = np;
                        np = outerPt;
                        expanded = true;
                        //continue;
                        }
                    else if (nodeAddrP(dtmP, outerPt)->tPtr == nodeAddrP(dtmP, np)->tPtr)
                        {
                        nodeAddrP(dtmP, sp)->tPtr = outerPt;
                        nodeAddrP(dtmP, np)->tPtr = dtmP->nullPnt;
                        if (np == startPoint)
                            startPoint = outerPt;
                        np = outerPt;
                        expanded = true;
                        }
                    }
                }
            sp = np;
            }
        }

    /*
    ** Expand Pond Boundary To Exit Point
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Expanding Pond To Exit Point");
    if (bcdtmDrainage_expandPondToExitPointDtmObject(dtmP, drainageTablesP, zeroSlopePolygonsP, zeroSlopePointsIndexP, startPoint, exitPointP, priorPointP, nextPointP)) goto errexit;
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "priorPoint = %8ld exitPoint = %8ld nextPoint = %8ld", *priorPointP, *exitPointP, *nextPointP);
        if( *exitPointP != NULL )
            {
            bcdtmWrite_message(0, 0, 0, "ExitPoint = %8ld ** %12.5lf %12.5lf %10.4lf", *exitPointP, pointAddrP(dtmP, *exitPointP)->x, pointAddrP(dtmP, *exitPointP)->y, pointAddrP(dtmP, *exitPointP)->z);
            }
        }
    if (*exitPointP == dtmP->nullPnt)
        {
        bcdtmWrite_message(2, 0, 0, "Pond Exit Point About About Low Point Not Determined");
        goto errexit;
        }

    /*
    ** Copy Pond Boundary To DTM
    */
    if (cdbg)
        {
        if (bcdtmList_copyTptrListToPointArrayDtmObject(dtmP, *exitPointP, &pondPtsP, &numPondPts)) goto errexit;
        if (bcdtmObject_createDtmObject(&tempDtmP)) goto errexit;
        if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::Breakline, tempDtmP->nullUserTag, 1, &tempDtmP->nullFeatureId, pondPtsP, numPondPts)) goto errexit;
        if (bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP, L"pondBoundary.dat")) goto errexit;
        if( pondPtsP != nullptr ) { free(pondPtsP) ; pondPtsP = nullptr ; }
        if (tempDtmP != nullptr) bcdtmObject_destroyDtmObject(&tempDtmP);
        }
    /*
    ** Validate Pond
    */
    if (cdbg == 2)
        {
        if (bcdtmDrainage_validatePondDtmObject(dtmP, *exitPointP, pondValid))
            {
            bcdtmWrite_message(0, 0, 0, "Pond Validation Error ** LowPoint = %8ld", lowPoint);
            goto errexit;
            }
        }
    /*
    ** Draw Pond Boundaries
    */
    if (loadFlag || boundaryFlag)
        {
        if (dbg)
            {
            bcdtmWrite_message(0, 0, 0, "Drawing Pond Boundary");
            bcdtmWrite_message(0, 0, 0, "lowPoint   = %9ld ** %10.4lf %10.4lf %10.4lf", lowPoint, pointAddrP(dtmP, lowPoint)->x, pointAddrP(dtmP, lowPoint)->y, pointAddrP(dtmP, lowPoint)->z);
            bcdtmWrite_message(0, 0, 0, "priorPoint = %9ld ** %10.4lf %10.4lf %10.4lf", *priorPointP, pointAddrP(dtmP, *priorPointP)->x, pointAddrP(dtmP, *priorPointP)->y, pointAddrP(dtmP, *priorPointP)->z);
            bcdtmWrite_message(0, 0, 0, "exitPoint  = %9ld ** %10.4lf %10.4lf %10.4lf", *exitPointP, pointAddrP(dtmP, *exitPointP)->x, pointAddrP(dtmP, *exitPointP)->y, pointAddrP(dtmP, *exitPointP)->z);
            bcdtmWrite_message(0, 0, 0, "nextPoint  = %9ld ** %10.4lf %10.4lf %10.4lf", *nextPointP, pointAddrP(dtmP, *nextPointP)->x, pointAddrP(dtmP, *nextPointP)->y, pointAddrP(dtmP, *nextPointP)->z);
            if (dbg == 2)bcdtmList_writeTptrListDtmObject(dtmP, *exitPointP);
            }
        if (bcdtmDrainage_extractPondBoundaryDtmObject(dtmP, pointAddrP(dtmP, *exitPointP)->z, *exitPointP, *nextPointP, loadFunctionP, loadFlag, boundaryFlag, polygonPP, userP)) goto errexit;
        }
    /*
    ** Check For None Null Pointer Values
    */
    if (cdbg)
        {
        for (node = 0; node < dtmP->numPoints; ++node)
            {
            nodeP = nodeAddrP(dtmP, node);
            if (nodeP->tPtr != dtmP->nullPnt || nodeP->sPtr != dtmP->nullPnt)
                {
                ofs = node;
                bcdtmWrite_message(0, 0, 0, "[%6ld]->tPtr = %9ld [%6ld]->sPtr = %9ld", ofs, nodeP->tPtr, ofs, nodeP->sPtr);
                ret = DTM_ERROR;
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Null Out Tptr Polygon
    */
    bcdtmList_nullTptrListDtmObject(dtmP, *exitPointP);
    if( pondPtsP != nullptr ) { free(pondPtsP) ; pondPtsP = nullptr ; }
    if (tempDtmP != nullptr) bcdtmObject_destroyDtmObject(&tempDtmP);
    /*
    ** Non Error Exit
    */
    if (dbg && ret == DTM_SUCCESS)  bcdtmWrite_message(0, 0, 0, "Determing Pond About Low Point Completed");
    if (dbg && ret != DTM_SUCCESS)  bcdtmWrite_message(0, 0, 0, "Determing Pond About Low Point Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondLowPoint::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    m_pondAnalysis = PondAnalysis::Create(m_tracer, m_ptNum, m_pt.z, PondAnalysis::LowPointType::Point);

    m_pondAnalysis->FindPond();
    exits = m_pondAnalysis->GetOuterExits();

    m_points = m_pondAnalysis->GetBoundary();
    return;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondEdge::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    m_pondAnalysis = PondAnalysis::Create(m_tracer, m_ptNum, m_pt.z, PondAnalysis::LowPointType::Edge, m_ptNum2);

    m_pondAnalysis->FindPond();
    exits = m_pondAnalysis->GetOuterExits();

    m_points = m_pondAnalysis->GetBoundary();
    return;
    }

void TracePondEdge::GetSumpLines()
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    DtmSumpLinesPtr sumpLines;
    if (bcdtmDrainage_concatenateZeroSlopeSumpLinesDtmObject(dtmP, m_ptNum, m_ptNum2, &sumpLines.ptr, &sumpLines.num))
        return;
    else if (sumpLines.ptr != nullptr)
        {
        // Add Sumplines.
        bvector<DPoint3d> pts;
        long prevPtNum = -1;
        DTM_SUMP_LINES* sumpLine = sumpLines.ptr;
        for (int i = 0; i < sumpLines.num; i++, sumpLine++)
            {
            if (prevPtNum == sumpLine->sP2)
                std::swap(sumpLine->sP1, sumpLine->sP2);

            if (prevPtNum != sumpLine->sP1)
                {
                if (!pts.empty())
                    {
                    m_sumpLines.push_back(pts);
                    pts.clear();
                    }
                pts.push_back(*pointAddrP(dtmP, sumpLine->sP1));
                }
            prevPtNum = sumpLine->sP2;
            pts.push_back(*pointAddrP(dtmP, prevPtNum));
            }
        if (!pts.empty())
            m_sumpLines.push_back(pts);
        }

    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondTriangle::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    m_pondAnalysis = PondAnalysis::Create(m_tracer, m_ptNum, m_pt.z, PondAnalysis::LowPointType::Triangle, m_ptNum2, m_ptNum3);

    m_pondAnalysis->FindPond();
    exits = m_pondAnalysis->GetOuterExits();

    m_points = m_pondAnalysis->GetBoundary();
    return;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondFromPondExit::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long oldPondExitPt = m_pondExit.GetExitPoint();
    m_pondAnalysis = PondAnalysis::Create(m_tracer, oldPondExitPt, pointAddrP(dtmP, oldPondExitPt)->z, PondAnalysis::LowPointType::PondExit);

    m_pondAnalysis->FindPond();
    exits = m_pondAnalysis->GetOuterExits();

    m_points = m_pondAnalysis->GetBoundary();

    if (exits.empty())
        {
        // Recover hidden ponds.
        m_pondExit.RecoverInnerPonds();
        }
    return;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysis& TracePond::GetPondAnalysis()
    {
    // ToDo
    //if (m_pondAnalysis.IsNull())
    //    m_pondAnalysis = PondAnalysis::Create(m_tracer, m_ptNum, m_pt.z);
    return *m_pondAnalysis;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_pt, 1, args);
    if (waterCallback)
        {
        if (m_pondAnalysis.IsValid())
            {
            bvector<DPoint3d> points;
            for (auto&& Tpoints : GetPondAnalysis().GetCurrentVolumePoints())
                {
                if (points.empty())
                    {
                    points = Tpoints;
                    }
                else
                    {
                    points.insert(points.end(), Tpoints.begin(), Tpoints.end());
                    points.push_back(points.front());
                    }
                }
                loadFunction(DTMFeatureType::LowPointPond, 0, 0, const_cast<DPoint3dP>(points.data()), points.size(), args);
            }
        for(auto& p : m_points)
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, p.data(), p.size(), args);   //ToDO Remove.
        }
    else
        {
        bvector<DPoint3d> points;
        for (auto&& Tpoints : m_points)
            {
            if (points.empty())
                {
                points = Tpoints;
                }
            else
                {
                points.insert(points.end(), Tpoints.begin(), Tpoints.end());
                points.push_back(points.front());
                }
            }
        loadFunction(DTMFeatureType::LowPointPond, 0, 0, const_cast<DPoint3dP>(points.data()), points.size(), args);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    bvector<PondExitInfo> exits;
    m_finished = true;

    GetExitInfo(exits);
    if (exits.empty())
        {
        SetError(L"Pond Exit Not found.");
        return;
        }

    // Get the exit Elevation, (from the first exit)
    DPoint3d exitPt = *pointAddrP(dtmP, exits[0].exitPoint);

    //              Check If Exit Point Pond Can Be Processed
    m_depth = fabs(exitPt.z - m_pt.z);

    if (m_depth < 0)
        {
        SetError(L"Pond Exit lower than lowPoint");
        return;
        }
    if (m_depth == 0)
        {
        TracePondEdge* pondEdge = dynamic_cast<TracePondEdge*>(this);
        if (nullptr != pondEdge)
            {
            pondEdge->GetSumpLines();
            }
        }
    for (auto& exitInfo : exits)
        {
        m_exitPoints.push_back(exitInfo.exitPoint);
        }

    bool addedNewPond = false;
    bvector<TracePondExit*> existingPondsExits;
    for (auto& exitInfo : exits)
        {
        TracePondExit* existingPondExit = m_tracer.FindPondExit(exitInfo.exitPoint);

        if (existingPondExit != nullptr)
            {
            existingPondsExits.push_back(existingPondExit);
            existingPondExit->AddPond(*this);
            }
        else
            {

            // Check if Pond exit is back to the original pond exit.

            auto pondExit = TracePondExit::Create(m_tracer, *this, exitInfo.exitPoint, exitInfo.priorPoint, exitInfo.nextPoint, *pointAddrP(dtmP, exitInfo.exitPoint));
            // Finished in a real pond/hill.
            newFeatures.push_back(pondExit);

            pondExit->AddPond(*this);
            m_tracer.AddPondExit(*pondExit);
            m_children.push_back(pondExit);
            addedNewPond = true;
            }
        }

    if (!existingPondsExits.empty() && !addedNewPond)
        {
        // Need to check if all connected ponds have exists.
        bool hasNonDeadPond = false;

        for (size_t i = 0; i < existingPondsExits.size(); i++)
            {
            auto pond = existingPondsExits[i];
            if (!pond->IsDeadPond())
                {
                hasNonDeadPond = true;
                break;
                }
            for (auto newPond : pond->GetPonds())
                {
                for (auto&& child : newPond->GetChildren())
                    {
                    auto pondExit = dynamic_cast<TracePondExit*>(child.get());
                    if (nullptr != pondExit)
                        {
                        if (std::find(existingPondsExits.begin(), existingPondsExits.end(), pondExit) == existingPondsExits.end())
                            {
                            existingPondsExits.push_back(pondExit);
                            }
                        }
                    }
                }
            }

        if (!hasNonDeadPond)
            {
            // Create new Pond from PondExit.
            existingPondsExits[0]->ProcessDeadPond(newFeatures);
            for (auto&& a : existingPondsExits)
                a->m_hasProcessedDeadPond = true;
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
double TracePond::GetVolumeAtElevation(double z)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::CreateFromDtmHandle(*dtmP);
    TerrainModel::BcDTMPtr pondDtm;
    dtm->ClipByPointString(pondDtm, m_points.front().data(), (int)m_points.front().size(), DTMClipOption::External);
    if (pondDtm.IsValid())
        {
        double cutVolume, fillVolume, balanceVolume, cutArea, fillArea;
        bcdtmTinVolume_surfaceToElevationDtmObject(pondDtm->GetTinHandle(), nullptr, 0, nullptr, 0, z, nullptr, nullptr, cutVolume, fillVolume, balanceVolume, cutArea, fillArea);
        return fillVolume;
        }
    return 0;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::CalculateMaxVolume()
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    if (!m_points.empty())
        m_maxVolume = GetVolumeAtElevation(m_points.front()[0].z);
    else
        m_maxVolume = 0;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume)
    {
    // ToDo change to handle inner ponds.
    if (m_maxVolume < 0)
        CalculateMaxVolume();

    double prevVolume = CurrentVolume() - totalVol;
    if (CurrentVolume() > m_maxVolume)
        {
        GetPondAnalysis().SetToMax(m_points);

        double vol = CurrentVolume() - m_maxVolume;
        if (prevVolume > m_maxVolume)
            vol = totalVol;

        vol /= m_exitPoints.size();
        for (int exitIndex : m_exitPoints)
            {
            auto exitPoint = m_tracer.FindPondExit(exitIndex);
            BeAssert(nullptr != exitPoint);
            if (nullptr != exitPoint)
                newWaterVolume.push_back(WaterVolumeInfo(exitPoint, vol));

            }
        return;
        }
    GetPondAnalysis().FindBoundaryForVolume(CurrentVolume());

    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume)
    {
    if (!m_calculated)
        {
        bvector<TraceFeaturePtr> newFeatures;
        Process(newFeatures, true);
        m_tracer.AddAndProcessFeatures(newFeatures);
        }
    bvector<TracePond*> nonFullPonds;
    for (auto pond : m_ponds)
        {
        if (!pond->IsFull())
            nonFullPonds.push_back(pond);
        }

    if (!nonFullPonds.empty())
        {
        double vol = totalVol / nonFullPonds.size();
        bool stillNotFull = true;
        for (auto pond : nonFullPonds)
            {
            newWaterVolume.push_back(WaterVolumeInfo(pond, vol));
            if (!pond->IsFull())
                stillNotFull = false;
            }
        if (stillNotFull)
            return;
        return;
        }
    for (auto&& child : m_children)
        {
        if (nullptr != dynamic_cast<TracePondFromPondExit*>(child.get()))
            {
            newWaterVolume.push_back(WaterVolumeInfo(child.get(), totalVol));
            return;
            }
        }
    __super::GetNewWaterVolumes(totalVol, newWaterVolume);
    }

void TracePondExit::RecoverInnerPonds()
    {
    for (auto&& pond : m_ponds)
        {
        pond->ClearIsHidden();
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::HideInnerPonds()
    {
    // find all connecting ponds.
    bvector<TracePond*> connectingPonds;
    for (auto&& pond : m_ponds)
        {
        connectingPonds.push_back(pond);
        }

    for (size_t i = 0; i < connectingPonds.size(); i++)
        {
        auto pond = connectingPonds[i];

        for (auto&& child : pond->GetChildren())
            {
            auto pondExit = dynamic_cast<TracePondExit*>(child.get());
            if (nullptr != pondExit)
                {
                for (auto&& newPond : pondExit->GetPonds())
                    {
                    if (std::find(connectingPonds.begin(), connectingPonds.end(), newPond) == connectingPonds.end())
                        {
                        connectingPonds.push_back(newPond);
                        }
                    }
                }
            }
        }
    for (auto&& pond : connectingPonds)
        pond->HideChildren();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures)
    {
    if (m_hasProcessedDeadPond)
        return;

    m_hasProcessedDeadPond = true;

    DPoint3d lowPt = m_exitPt;
    long lowPtNum = 0;
    for (auto&& pond : m_ponds)
        {
        if (pond->GetLowPoint().z < lowPt.z)
            {
            lowPt = pond->GetLowPoint();
            lowPtNum = pond->GetLowPointNumber();
            }
        }

    HideInnerPonds();
    auto child = TracePondFromPondExit::Create(m_tracer, *this, *this, lowPt, lowPtNum, m_exitPt);
    newFeatures.push_back(child);
    m_children.push_back(child);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    Process(newFeatures, false);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::Process(bvector<TraceFeaturePtr>& newFeatures, bool ignoreFalseLow)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    if (m_calculated)
        return;

    m_finished = true;
    if (!ignoreFalseLow)
        {
        for (auto&& pond : m_ponds)
            {
            if (pond->GetDepth() > m_tracer.m_falseLowDepth)
                return;
            }
        }

    m_calculated = true;


    long hullPoint = 0;
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_exitPnt, &hullPoint)) { SetError(); return; }

    if (hullPoint)
        {
        m_onHullPoint = true;
        return;
        }

    if (m_priorPnt == dtmP->nullPnt || m_nextPnt == dtmP->nullPnt)
        {
        SetError();
        return;
        }
    long nextPnt1, nextPnt2, nextPnt3;
    DPoint3d nextPt;
    bool process = true;
    if (bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject(dtmP, m_tracer.drainageTablesP, m_priorPnt, m_exitPnt, m_nextPnt, m_exitPt.x, m_exitPt.y, &nextPnt1, &nextPnt2, &nextPnt3, &nextPt.x, &nextPt.y, &nextPt.z, &process)) { SetError(); return; }

    if (!process)
        return;

    m_points.push_back(nextPt);

    // Set Parameters For Next Maximum Descent Trace
    double lastAngle = bcdtmMath_getAngle(m_exitPt.x, m_exitPt.y, nextPt.x, nextPt.y);
    if (nextPnt2 == dtmP->nullPnt)  // Point
        {
        auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, nextPnt1, nextPt, lastAngle);
        m_children.push_back(child);
        }
    else
        {
        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        }

    m_numExitFlows++;
    }

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct QuickFeatureJoiner
    {
    bvector<DPoint3d> m_points;
    DTMFeatureType m_fetaureType = DTMFeatureType::None;
    DTMFeatureCallback m_loadFunctionP;
    void* m_userP;

    QuickFeatureJoiner(DTMFeatureCallback loadFunctionP, void* userP) : m_loadFunctionP(loadFunctionP), m_userP(userP)
        {
        }
    ~QuickFeatureJoiner()
        {
        if (!m_points.empty())
            m_loadFunctionP(m_fetaureType, 0, 0, m_points.data(), m_points.size(), m_userP);
        }
    static int callback(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
        {
        QuickFeatureJoiner* instance = (QuickFeatureJoiner*)userArg;
        return instance->Callback(dtmFeatureType, points, numPoints);
        }
    int Callback(DTMFeatureType dtmFeatureType, DPoint3d *points, size_t numPoints)
        {
        if (numPoints == 2 && points[0].DistanceXY(points[1]) < 0.0001)
            numPoints = numPoints;
        if (dtmFeatureType == DTMFeatureType::AscentTrace || dtmFeatureType == DTMFeatureType::DescentTrace || dtmFeatureType == DTMFeatureType::SumpLine)
            {
            if (dtmFeatureType == m_fetaureType && !m_points.empty())
                {
                if (points[0].IsEqual(m_points.back()))
                    {
                    m_points.insert(m_points.end(), &points[1], &points[numPoints]);
                    return DTM_SUCCESS;
                    }
                }
            if (!m_points.empty())
                {
                int ret = m_loadFunctionP(m_fetaureType, 0, 0, m_points.data(), m_points.size(), m_userP);
                if (ret != DTM_SUCCESS)
                    return ret;
                m_points.clear();
                }
            m_fetaureType = dtmFeatureType;
            m_points.insert(m_points.end(), &points[0], &points[numPoints]);
            return DTM_SUCCESS;
            }
        if (!m_points.empty())
            {
            int ret = m_loadFunctionP(m_fetaureType, 0, 0, m_points.data(), m_points.size(), m_userP);
            if (ret != DTM_SUCCESS)
                return ret;
            m_points.clear();
            m_fetaureType = DTMFeatureType::None;
            }
        return m_loadFunctionP(dtmFeatureType, 0, 0, points, numPoints, m_userP);
        }

    };

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
int bcdtmDrainage_traceMaximumDescentDtmObject
(
    BC_DTM_OBJ         *dtmP,                  // ==> Pointer To Tin Object
    DTMDrainageTables  *drainageTablesP,       // ==> Pointer To Drainage Tables
    DTMFeatureCallback loadFunctionP,          // ==> Pointer To Load Function
    double             falseLowDepth,          // ==> False Low Depth
    double             startX,                 // ==> Start X Coordinate
    double             startY,                 // ==> Start Y Coordinate
    void               *userP                  // ==> User Pointer Passed Back To User
)
    {
    //startX = 290580.30906499998;
    //startY = 6242134.7627180004;
    //startX = 290580.30906499998;
    //startY = 6242134.7627180004;
    //startX = 773373.93307517446;
    //startY = 1785709.0749221439;
    //startX = 773230; startY = 1784950;
    //startX = 290513.48747999995;
    //startY = 6242144.7302313335;
    if (falseLowDepth < 0)
        bcdtmDrainage_traceMaximumDescentDtmObjectOld(dtmP, drainageTablesP, loadFunctionP, -falseLowDepth, startX, startY, userP);
    else
        {
        //bcdtmDrainage_traceMaximumDescentDtmObjectOld(dtmP, drainageTablesP, loadFunctionP, -falseLowDepth, startX, startY, userP);
        TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::CreateFromDtmHandle(*dtmP);
        DrainageTracer tracer(*dtm);

        //tracer.m_falseLowDepth = 0;
        //tracer.AddWaterVolume(DPoint3d::From(startX, startY, 0), falseLowDepth);

        tracer.m_falseLowDepth = falseLowDepth;
        tracer.DoTrace(DPoint3d::From(startX, startY, 0));
        QuickFeatureJoiner joiner(loadFunctionP, userP);
        tracer.DoTraceCallback(&QuickFeatureJoiner::callback, &joiner);

        }
    return DTM_SUCCESS;
    }



// ToDo

// 1 Create class to handle depth analysis,
//   * First find max Volume, and find ponds, add low points to list. Max Volume incs ponds.
//   * Change to use an array of startPoints. (and volume creation)
//   * When adding Triangles, if it attaches to another part, create an island. (Add case to remove island.)
//   * Cope with two/single point tPtr.
//   * Check exit points. When a point on a pond is added.  If that pond is full then add the pond volume to current and continue.
//   * Change to add zero slope lines to expansion routine.
//   *
//
// 2 Change create pond to use new code, and return multiple exit points.
//
// 3 Get the code to work for upstream as well as downstream.
// 4 Create new API.
// 5 Make it work with voids.
// 6 Find sump lines on the edges.
// Make GetPond work with ponds from pondExit.
