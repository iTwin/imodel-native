/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/NewTrace.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>


struct DtmPolygonPtr
    {
    DTM_POLYGON_OBJ *ptr = nullptr;

    ~DtmPolygonPtr()
        {
        if (ptr!= nullptr) bcdtmPolygon_deletePolygonObject(&ptr);
        }
    };

struct DtmSumpLinesPtr
    {
    DTM_SUMP_LINES* ptr = nullptr;
    long num;

    ~DtmSumpLinesPtr()
        {
        if (ptr != nullptr) free(ptr);
        }
    };
int bcdtmDrainage_traceMaximumDescentDtmObjectOld
(
    BC_DTM_OBJ         *dtmP,                  // ==> Pointer To Tin Object
    DTMDrainageTables  *drainageTablesP,       // ==> Pointer To Drainage Tables
    DTMFeatureCallback loadFunctionP,          // ==> Pointer To Load Function
    double             falseLowDepth,          // ==> False Low Depth
    double             startX,                 // ==> Start X Coordinate
    double             startY,                 // ==> Start Y Coordinate
    void               *userP                  // ==> User Pointer Passed Back To User
);

struct DrainageTracer;
struct TraceFeature;
struct TraceOnPoint;
struct TraceOnEdge;
struct TraceInTriangle;
struct TraceStartPoint;
struct TracePond;
struct TracePondLowPoint;
struct TracePondEdge;
struct TracePondTriangle;
struct TracePondFromPondExit;
struct TracePondExit;
struct TraceZSlopeLine;
typedef RefCountedPtr<TraceFeature> TraceFeaturePtr;
typedef RefCountedPtr<TraceStartPoint> TraceStartPointPtr;
typedef RefCountedPtr<TraceInTriangle> TraceInTrianglePtr;
typedef RefCountedPtr<TraceOnPoint> TraceOnPointPtr;
typedef RefCountedPtr<TracePondLowPoint>     TracePondLowPointPtr;
typedef RefCountedPtr<TracePondEdge>         TracePondEdgePtr;
typedef RefCountedPtr<TracePondTriangle>     TracePondTrianglePtr;
typedef RefCountedPtr<TracePondFromPondExit> TracePondFromPondExitPtr;
typedef RefCountedPtr<TraceOnEdge> TraceOnEdgePtr;
typedef RefCountedPtr<TraceZSlopeLine> TraceZSlopeLinePtr;
typedef RefCountedPtr<TracePond> TracePondPtr;
typedef RefCountedPtr<TracePondExit> TracePondExitPtr;


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

        TraceFeature(DrainageTracer& tracer) : m_tracer(tracer)
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


        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) abstract;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) abstract;

        void HideChildren()
            {
            m_isHidden = true;
            for (auto child : m_children)
                {
                child->HideChildren();
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
        TraceStartPoint(DrainageTracer& tracer, DPoint3dCR startPoint) : TraceFeature(tracer), m_startPoint(startPoint)
            {
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            //            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
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

        TraceInTriangle(DrainageTracer& tracer, long P1, long P2, long P3, DPoint3dCR startPt) : TraceFeature(tracer), pnt1(P1), pnt2(P2), pnt3(P3), m_startPt(startPt)
            {
            m_points.push_back(startPt);
            }
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceInTrianglePtr Create(DrainageTracer& tracer, long P1, long P2, long P3, DPoint3dCR startPt)
            {
            return new TraceInTriangle(tracer, P1, P2, P3, startPt);
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

        TraceOnEdge(DrainageTracer& tracer, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle) : TraceFeature(tracer), pnt1(P1), pnt2(P2), pnt3(P3), m_pt(startPt), m_startPt(startPt), lastAngle(lastAngle)
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
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceOnEdgePtr Create(DrainageTracer& tracer, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle = -99)
            {
            return new TraceOnEdge(tracer, P1, P2, P3, startPt, lastAngle);
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

        TraceZSlopeLine(DrainageTracer& tracer, long P1, long P2) : TraceFeature(tracer), pnt1(P1), pnt2(P2)
            {
            }
    public:
        const bvector<DPoint3d>& GetPoints() const
            {
            return m_points;
            }
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceZSlopeLinePtr Create(DrainageTracer& tracer, long P1, long P2)
            {
            return new TraceZSlopeLine(tracer, P1, P2);
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
        TraceOnPoint(DrainageTracer& tracer, long startPtNum, DPoint3dCR startPoint, double lastAngle) : TraceFeature(tracer), m_pt(startPoint), m_lastAngle(lastAngle), m_ptNum(startPtNum)
            {
            m_points.push_back(startPoint);
            }

        void ProcessZSlope(bvector<TraceFeaturePtr>& newFeatures, long descentPnt1, long descentPnt2);
    public:
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            if (m_points.size() != 1 && !m_onHullPoint)   // One point we can ignore.
                loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
            }
        static TraceOnPointPtr Create(DrainageTracer& tracer, long startPtNum, DPoint3dCR startPoint, double lastAngle = -99)
            {
            return new TraceOnPoint(tracer, startPtNum, startPoint, lastAngle);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePond : public TraceFeature
    {
    protected:
        struct PondExitInfo
            {
            long exitPoint;
            long priorPoint;
            long nextPoint;

            PondExitInfo(long exitPoint, long priorPoint, long nextPoint) : exitPoint(exitPoint), priorPoint(priorPoint), nextPoint(nextPoint)
                { }
            };

        bvector<DPoint3d> m_points;
        bvector<long> m_exitPoints;
        double m_depth;
        DPoint3d m_pt;

        TracePond(DrainageTracer& tracer, DPoint3dCR pt) : TraceFeature(tracer), m_pt(pt)
            { }
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) abstract;
    public:
        DPoint3dCR GetLowPoint() const
            {
            return m_pt;
            }
        const bvector<DPoint3d>& GetPoints() const
            {
            return m_points;
            }
        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_pt, 1, args);
            loadFunction(DTMFeatureType::LowPointPond, 0, 0, m_points.data(), m_points.size(), args);
            }

        bool HasExitPoint(long pnt) const
            {
            for (auto exitPnt : m_exitPoints)
                {
                if (exitPnt == pnt)
                    return true;
                }
            return false;
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondLowPoint : public TracePond
    {
    private:
        long m_ptNum;
        TracePondLowPoint(DrainageTracer& tracer, long ptNum, DPoint3dCR pt) : TracePond(tracer, pt), m_ptNum(ptNum)
            {

            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(DrainageTracer& tracer, long ptNum, DPoint3dCR pt)
            {
            return new TracePondLowPoint(tracer, ptNum, pt);
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

        TracePondEdge(DrainageTracer& tracer, long ptNum1, long ptNum2, DPoint3dCR pt) : TracePond(tracer, pt), m_ptNum1(ptNum1), m_ptNum2(ptNum2)
            {}
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;

    public:
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
            if (!m_points.empty())
            __super::DoCallback(loadFunction, args);

            for (auto&& sump : m_sumpLines)
                loadFunction(DTMFeatureType::DescentTrace, 0, 0, sump.data(), (long)sump.size(), args);
            }

        static TracePondEdgePtr Create(DrainageTracer& tracer, long ptNum, long ptNum2, DPoint3dCR pt)
            {
            return new TracePondEdge(tracer, ptNum, ptNum2, pt);
            }
    };

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct TracePondTriangle : public TracePond
    {
    private:
        long m_ptNum, m_ptNum2, m_ptNum3;

        TracePondTriangle(DrainageTracer& tracer, long ptNum1, long ptNum2, long ptNum3, DPoint3dCR pt) : TracePond(tracer, pt), m_ptNum(ptNum1), m_ptNum2(ptNum2), m_ptNum3(ptNum3)
            {
            }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondPtr Create(DrainageTracer& tracer, long ptNum, long ptNum2, long ptNum3, DPoint3dCR pt)
            {
            return new TracePondTriangle(tracer, ptNum, ptNum2, ptNum3, pt);
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

        TracePondFromPondExit(DrainageTracer& tracer, TracePondExit& pondExit, DPoint3dCR lowPt, DPoint3dCR exitPt) : TracePond(tracer, lowPt), m_pondExit(pondExit), m_exitPt(exitPt)
            { }
    protected:
        virtual void GetExitInfo(bvector<PondExitInfo>& exits) override;
    public:

        static TracePondFromPondExitPtr Create(DrainageTracer& tracer, TracePondExit& pondExit, DPoint3dCR lowPt, DPoint3dCR exitPt)
            {
            return new TracePondFromPondExit(tracer, pondExit, lowPt, exitPt);
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

        long m_numExitFlows = 1; // This will always start with 1 the flow which created it.
        bvector<TracePond*> m_ponds;
        bool m_hasProcessedDeadPond = false;
        bool m_onHullPoint = false;

        TracePondExit(DrainageTracer& tracer, long exitPnt, long priorPnt, long nextPnt, DPoint3dCR exitPoint) : TraceFeature(tracer), m_exitPnt(exitPnt), m_priorPnt(priorPnt), m_nextPnt(nextPnt), m_exitPt(exitPoint)
            {
            m_points.push_back(exitPoint);
            }

    public:
        void ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures);

        virtual void Process(bvector<TraceFeaturePtr>& newFeatures) override;
        virtual void DoCallback(DTMFeatureCallback loadFunction, void* args) override
            {
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

        static TracePondExitPtr Create(DrainageTracer& tracer, long exitPnt, long priorPnt, long nextPnt, DPoint3dCR exitPoint)
            {
            return new TracePondExit(tracer, exitPnt, priorPnt, nextPnt, exitPoint);
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
            m_features.push_back(TraceStartPoint::Create(*this, startPt));
            ProcessFeatures();
            return DTM_SUCCESS;
            }

        void DoCallback(DTMFeatureCallback loadFunction, void* userArg)
            {
            for (const auto& feature : m_features)
                {
                if (!feature->IsHidden())
                    feature->DoCallback(loadFunction, userArg);
                }
            }
        void ProcessFeatures()
            {
            bool haveProcessedAFeature = false;
            bvector<TraceFeature*> featuresToProcess;
            bvector<TraceFeaturePtr> newFeatures;
            bvector<TraceFeature*> newFeaturesToProcess;
            
            for (auto&& feature : m_features)
                featuresToProcess.push_back(feature.get());

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
        auto child = TraceOnPoint::Create(m_tracer, trgPnt1, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
        newFeatures.push_back(child);
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
            auto child = TracePondTriangle::Create(m_tracer, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
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
        auto child = TraceInTriangle::Create(m_tracer, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
        newFeatures.push_back(child);
        m_children.push_back(child);

        }
    else if (trgPnt2 != dtmP->nullPnt && trgPnt3 == dtmP->nullPnt)
        {
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 < 0) { SetError(); return; }
        if (pointAddrP(dtmP, trgPnt3)->z < z && bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
            {
            auto child = TraceOnEdge::Create(m_tracer, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }

        std::swap(trgPnt1, trgPnt2);
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 < 0) { SetError(); return; }
        if (pointAddrP(dtmP, trgPnt3)->z < z && bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
            {
            auto child = TraceOnEdge::Create(m_tracer, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
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
        auto child = TraceOnEdge::Create(m_tracer, nextPnt1, nextPnt2, nextPnt3, nextPt, angle);
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
            auto child = TraceOnPoint::Create(m_tracer, pnt1, nextPt, lastAngle);
            newFeatures.push_back(child);
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
        auto child = TracePondTriangle::Create(m_tracer, pnt1, pnt2, pnt3, m_pt);
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
    //newFeatures.push_back(TraceZSlopeLine::Create(m_tracer, pnt1, pnt2, m_pt));

    long prevPnt = bcdtmList_nextClkDtmObject(dtmP, pnt1, pnt2);

    if (pointAddrP(dtmP, prevPnt)->z == m_pt.z)
        {
        auto child = TracePondTriangle::Create(m_tracer, pnt1, pnt2, pnt3, *pointAddrP(dtmP, pnt1));
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    else
        {
        auto child = TracePondEdge::Create(m_tracer, pnt1, pnt2, *pointAddrP(dtmP, pnt1));
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

        auto child = TraceOnPoint::Create(m_tracer, pnt2, nextPt, lastAngle);
        newFeatures.push_back(child);
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
        auto child = TraceOnPoint::Create(m_tracer, nextPnt1, nextPt, lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        return;
        }
    if (pnt3 == dtmP->nullPnt)
        {
        m_finished = true;
        if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0) { SetError(); return; }
        auto child = TraceOnEdge::Create(m_tracer, nextPnt1, nextPnt2, nextPnt3, nextPt, lastAngle);
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
                auto child = TraceOnEdge::Create(m_tracer, nextPnt1, nextPnt2, nextPnt3, nextPt, m_lastAngle);
                newFeatures.push_back(child);
                m_children.push_back(child);
                return;
                }
            }
        }

    //if (dbg) bcdtmWrite_message(0, 0, 0, "** Placing Pond About Zero Slope Triangle");
    //long exitPoint, priorPoint, nextPoint;
    //if (bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP, m_ptNum, descentPnt1, descentPnt2, nullptr, false, false, &exitPoint, &priorPoint, &nextPoint, nullptr, nullptr)) { SetError(); return; }
    auto pond = TracePondEdge::Create(m_tracer, descentPnt1, descentPnt2, m_pt);
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
        auto pond = TracePondLowPoint::Create(m_tracer, m_ptNum, m_pt);
        m_tracer.AddPond(*pond);
        newFeatures.push_back(pond);
        m_children.push_back(pond);
        return;

        }
    else if (type == 1)
        {
        if (slope == 0) // ZeroSlope
            {
            auto child = TracePondEdge::Create(m_tracer, m_ptNum, pnt1, m_pt);
            newFeatures.push_back(child);
            m_children.push_back(child);
            //long pnt3 = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
            //newFeatures.push_back(TraceOnEdge::Create(m_tracer, pnt1, pnt2, pnt3, *pointAddrP(dtmP, pnt1)));
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
        auto child = TraceOnEdge::Create(m_tracer, pnt1, pnt2, pnt3, m_pt, m_lastAngle);
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

    {
    long sp = startPoint;
    do
        {
        long np = nodeAddrP(dtmP, sp)->tPtr;

        if (!bcdtmList_testLineDtmObject(dtmP, sp, np))
            np = np;
        sp = np;
        } while (sp != startPoint);
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
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long hullPoint;
    long exitPoint, priorPoint, nextPoint;

    //if (dbg) bcdtmWrite_message(0, 0, 0, "Placing Pond About Low Point");
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_ptNum, &hullPoint))
        {
        SetError(); return;
        }
    if (hullPoint)
        {
        //            SetErrorNotImplemented(L"Pond on Hull Point");
        return;
        }

    if (m_tracer.AscentTrace())
        {
        SetErrorNotImplemented(L"Ascent Trace find hump about high Point, Not Implemented");
        return;
        }
    else
        {
        if (m_tracer.drainageTablesP != nullptr && m_tracer.drainageTablesP->SizeOfLowPointPondTable() > 0)
            {
            int exitPnt, priorPnt, nextPnt;
            m_tracer.drainageTablesP->FindLowPointPond(m_ptNum, exitPnt, priorPnt, nextPnt);
            exitPoint = exitPnt;
            priorPoint = priorPnt;
            nextPoint = nextPnt;
            }
        else
            {
            DtmPolygonPtr polygon;
            if (bcdtmDrainage_determinePondAboutLowPointDtmObject(dtmP, m_tracer.drainageTablesP, nullptr, nullptr, nullptr, m_ptNum, false, true, &exitPoint, &priorPoint, &nextPoint, &polygon.ptr, nullptr))
                {
                SetError(); return;
                }
            if (polygon.ptr != nullptr)
                {
                m_points.insert(m_points.end(), &polygon.ptr->polyPtsP[0], &polygon.ptr->polyPtsP[polygon.ptr->numPolyPts]);
                m_points.push_back(polygon.ptr->polyPtsP[0]);
                }
            }
        }
     exits.push_back(PondExitInfo(exitPoint, priorPoint, nextPoint));
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondEdge::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long exitPoint, priorPoint, nextPoint;
    double area;
    DtmSumpLinesPtr sumpLines;
    DtmPolygonPtr polygon;



 /*
 {
    if (dbg) bcdtmWrite_message(0, 0, 0, "Zero Slope Sump Line Detected");
    if (isFalseLow)
        {
        if (drainageTablesP != nullptr && drainageTablesP->SizeOfZeroSlopeLinePondTable() > 0)
            {
            drainageTablesP->FindZeroSlopeLinePond(startPnt1, startPnt2, exitPnt, priorPnt, nextPnt);
            *exitPointP = exitPnt;
            *priorPointP = priorPnt;
            *nextPointP = nextPnt;
            }
        else
            {
            if (bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP, nullptr, nullptr, nullptr, startPnt1, startPnt2, false, false, exitPointP, priorPointP, nextPointP, &sumpLinesP, &numSumpLines, &polygonP, nullptr, &area)) goto errexit;
            if (sumpLinesP != nullptr)
                {
                free(sumpLinesP); sumpLinesP = nullptr;
                }
            if (polygonP != nullptr) bcdtmPolygon_deletePolygonObject(&polygonP);
            }
        }
    // Get Exit Point From Zero Sump Lines ** Added 8/1/2007
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
    }
*/

    if (bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP, nullptr, nullptr, nullptr, m_ptNum1, m_ptNum2, false, true, &exitPoint, &priorPoint, &nextPoint, &sumpLines.ptr, &sumpLines.num, &polygon.ptr, nullptr, &area)) { SetError(); return; }
    if (polygon.ptr != nullptr)
        {
        m_points.insert(m_points.end(), &polygon.ptr->polyPtsP[0], &polygon.ptr->polyPtsP[polygon.ptr->numPolyPts]);
        m_points.push_back(polygon.ptr->polyPtsP[0]);
        }
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

    exits.push_back(PondExitInfo(exitPoint, priorPoint, nextPoint));
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondTriangle::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long exitPoint, priorPoint, nextPoint;
    DtmPolygonPtr polygon;
    if (bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP, m_ptNum, m_ptNum2, m_ptNum3, nullptr, 0, 1, &exitPoint, &priorPoint, &nextPoint, &polygon.ptr,nullptr))
        {
        SetError();
        return;
        }
    if (polygon.ptr != nullptr)
        {
        m_points.insert(m_points.end(), &polygon.ptr->polyPtsP[0], &polygon.ptr->polyPtsP[polygon.ptr->numPolyPts]);
        m_points.push_back(polygon.ptr->polyPtsP[0]);
        }
    exits.push_back(PondExitInfo(exitPoint, priorPoint, nextPoint));
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondFromPondExit::GetExitInfo(bvector<PondExitInfo>& exits)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long hullPoint;
    long exitPoint, priorPoint, nextPoint;
    long oldPondExitPt = m_pondExit.GetExitPoint();

    //if (dbg) bcdtmWrite_message(0, 0, 0, "Placing Pond About Low Point");
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, oldPondExitPt, &hullPoint))
        {
        SetError(); return;
        }
    if (hullPoint)
        {
        //            SetErrorNotImplemented(L"Pond on Hull Point");
        return;
        }

    if (m_tracer.AscentTrace())
        {
        SetErrorNotImplemented(L"Ascent Trace find hump about high Point, Not Implemented");
        return;
        }
    else
        {
        if (m_tracer.drainageTablesP != nullptr && m_tracer.drainageTablesP->SizeOfLowPointPondTable() > 0)
            {
            //int exitPnt, priorPnt, nextPnt;
            //m_tracer.drainageTablesP->FindLowPointPond(oldPondExitPt, exitPnt, priorPnt, nextPnt);
            //exitPoint = exitPnt;
            //priorPoint = priorPnt;
            //nextPoint = nextPnt;
            }
        else
            {
            DtmPolygonPtr polygon;
            if (bcdtmDrainage_determinePondAboutExitPointDtmObject(dtmP, m_tracer.drainageTablesP, nullptr, nullptr, nullptr, oldPondExitPt, false, true, &exitPoint, &priorPoint, &nextPoint, &polygon.ptr, nullptr))
                {
                SetError(); return;
                }
            if (polygon.ptr != nullptr)
                {
                m_points.insert(m_points.end(), &polygon.ptr->polyPtsP[0], &polygon.ptr->polyPtsP[polygon.ptr->numPolyPts]);
                m_points.push_back(polygon.ptr->polyPtsP[0]);
                }
            }
        }
    // Check if the exit point is higher.
    double dz = pointAddrP(dtmP, exitPoint)->z - m_exitPt.z;
    if (m_tracer.AscentTrace())
        dz = -dz;
    if (dz <= 0)
        {
        SetErrorNotImplemented(L"Temporary Dead Pond Exit missed.");
        return;
        }

    exits.push_back(PondExitInfo(exitPoint, priorPoint, nextPoint));
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    m_finished = true;

    bvector<PondExitInfo> exits;

    GetExitInfo(exits);

    //if (exitPoint == dtmP->nullPnt)
    //    {
    //    SetError(L"Low Point Pond Not Determined");
    //    return;
    //    }
    m_finished = true;
    for (auto& exitInfo : exits)
        {
        m_exitPoints.push_back(exitInfo.exitPoint);

        DPoint3d exitPt = *pointAddrP(dtmP, exitInfo.exitPoint);

        //              Check If Exit Point Pond Can Be Processed
        m_depth = fabs(exitPt.z - m_pt.z);
        if (m_depth > m_tracer.m_falseLowDepth)
            {
            // Finished in a real pond/hill.
            return;
            }

        TracePondExit* existingPondExit = m_tracer.FindPondExit(exitInfo.exitPoint);

        if (existingPondExit != nullptr)
            {
            existingPondExit->AddPond(*this);

            if (existingPondExit->IsDeadPond())
                {
                // Create new Pond from PondExit.
                existingPondExit->ProcessDeadPond(newFeatures);
                }
            return;
            }

        // Check if Pond exit is back to the original pond exit.

        auto pondExit = TracePondExit::Create(m_tracer, exitInfo.exitPoint, exitInfo.priorPoint, exitInfo.nextPoint, exitPt);

        pondExit->AddPond(*this);
        m_tracer.AddPondExit(*pondExit);
        newFeatures.push_back(pondExit);
        m_children.push_back(pondExit);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures)
    {
    if (m_hasProcessedDeadPond)
        return;

    m_hasProcessedDeadPond = true;

    for (auto&& pond : m_ponds)
        pond->HideChildren();

    DPoint3d lowPt = m_exitPt;
    for (auto&& pond : m_ponds)
        {
        if (pond->GetLowPoint().z < lowPt.z)
            lowPt = pond->GetLowPoint();
        }

    auto child = TracePondFromPondExit::Create(m_tracer, *this, lowPt, m_exitPt);
    newFeatures.push_back(child);
    m_children.push_back(child);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    m_finished = true;

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
        auto child = TraceOnPoint::Create(m_tracer, nextPnt1, nextPt, lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    else
        {
        auto child = TraceOnEdge::Create(m_tracer, nextPnt1, nextPnt2, nextPnt3, nextPt, lastAngle);
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
        if (dtmFeatureType == DTMFeatureType::AscentTrace || dtmFeatureType == DTMFeatureType::DescentTrace)
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

        tracer.m_falseLowDepth = falseLowDepth;
        tracer.DoTrace(DPoint3d::From(startX, startY, 0));
        QuickFeatureJoiner joiner(loadFunctionP, userP);
        tracer.DoCallback(&QuickFeatureJoiner::callback, &joiner);
        }
    return DTM_SUCCESS;
    }