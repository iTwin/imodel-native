/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/WaterAnalysis.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel\Drainage\WaterAnalysis.h>

#include "bcdtmDrainage.h"

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

int bcdtmDrainage_traceMaximumDescentDtmObjectOld(BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP, DTMFeatureCallback loadFunctionP, double falseLowDepth, double startX, double startY, void *userP);

#ifdef DEBUG
//#define TPTRVALIDATEDEBUG
//#define DEBUGCHK
//#define DEBUG_CHKPOINTLIST
//#define VOLUME_DEBUG
#endif
// DEBUG checking bools.
const bool checkVolumes = false;

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

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysis::PondAnalysis(PondAnalysisCR from, DrainageTracer& tracer) : m_tracer(tracer)
    {
    m_type = from.m_type;
    m_needsVolume = from.m_needsVolume;
    m_lowPnt = from.m_lowPnt;
    m_pntNum2 = from.m_pntNum2;
    m_pntNum3 = from.m_pntNum3;
    m_lowZ = from.m_lowZ;
    m_outerUnknown = from.m_outerUnknown;
    dtmP = from.dtmP;
    m_currentVolumePoints = from.m_currentVolumePoints;
    m_errorStatus = from.m_errorStatus;
    m_errorMessage = from.m_errorMessage;
    m_lowPoints = from.m_lowPoints;
    m_isFlatPond = from.m_isFlatPond;
    m_nextIndex = from.m_nextIndex;
    m_currentIndex = from.m_currentIndex;
    m_startPoints = from.m_startPoints;
    m_newStartPoints = from.m_newStartPoints;
    m_totalVol = from.m_totalVol;
    m_currentVol = from.m_currentVol;
    m_currentArea = from.m_currentArea;
    m_currentZ = from.m_currentZ;
    m_exitZ = from.m_exitZ;

    m_targetVolumeNeedsRefining = from.m_targetVolumeNeedsRefining;
    m_targetRefine_minZ = from.m_targetRefine_minZ;
    m_targetRefine_maxZ = from.m_targetRefine_maxZ;
    m_targetRefine_minVolume = from.m_targetRefine_minVolume;
    m_targetRefine_maxVolume = from.m_targetRefine_maxVolume;

    m_pointList = tracer.GetPointList();
    for (auto& pointInfo : m_startPoints)
        pointInfo.tPtr.SetPointList(tracer.GetPointList());

    for (auto& pointInfo : m_newStartPoints)
        pointInfo.tPtr.SetPointList(tracer.GetPointList());
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::SetCurrentIndex(int index)
    {
#ifdef DEBUGCHK
    if (GetCurrentStartInfo() != nullptr)
        GetCurrentStartInfo()->tPtr.Validate(dtmP);
#endif
    if (m_currentIndex == index)
        return;

    m_currentIndex = index;
#ifdef DEBUGCHK
    if (GetCurrentStartInfo() != nullptr)
        GetCurrentStartInfo()->tPtr.Validate(dtmP);
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysis::PondAnalysis(DrainageTracer& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3) : m_tracer(tracer), m_lowPnt(lowPnt), m_lowZ(lowZ), m_type(type), m_pntNum2(ptNum2), m_pntNum3(ptNum3), m_pointList(tracer.GetPointList())
    {
    dtmP = m_tracer.GetDTM().GetTinHandle();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysisPtr PondAnalysis::Create(DrainageTracer& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3)
    {
    return new PondAnalysis(tracer, lowPnt, lowZ, type, ptNum2, ptNum3);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::FindOuterBoundary()
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
        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);
        startInfo.m_location = ExpandingPondInfo::Location::Inner;

        for (auto sp : list)
            {
            if (sp < lowPtNum)
                {
                lowPtNum = sp;
                outerPond = &startInfo;
                }
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
        DTMDirection testDirection = startInfo.m_location == ExpandingPondInfo::Location::Outer ? DTMDirection::AntiClockwise : DTMDirection::Clockwise;

        if (testDirection == direction)
            direction = direction;
        else
            direction = direction;
        }
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
double PondAnalysis::GetVolumeOfTrianglesOnSide(double elevation)
    {
    double totalVol = 0;

    FindOuterBoundary();
    // OK This works for two point island boundary but not 1.
    for (auto&& startInfo : m_startPoints)
        {
        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);
        for (auto sp = list.begin(); sp != list.end(); sp++)
            {
            auto rotPnt = list.GetNext(sp);
            long endPnt;
            long thisPnt = *sp;

            if (*rotPnt == *sp)
                {
                long clc = nodeAddrP(dtmP, *sp)->cPtr;
                endPnt = clistAddrP(dtmP, clc)->pntNum;
                thisPnt = endPnt;
                }
            else
                {
                endPnt = *list.GetNext(rotPnt);
                }

            DPoint3dCP rotPt = pointAddrP(dtmP, *rotPnt);

            do
                {
                long anp = bcdtmList_nextClkDtmObject(dtmP, *rotPnt, thisPnt);

                if (anp != endPnt)
                    {
                    double cutVol = 0, fillVol = 0, cutArea = 0, fillArea = 0;
                    bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, thisPnt, *rotPnt, anp, elevation, cutVol, fillVol, cutArea, fillArea);
                    if (m_type == LowPointType::PondExit)
                        {
                        double lowZ1 = pointAddrP(dtmP, thisPnt)->z;
                        double lowZ2 = pointAddrP(dtmP, anp)->z;
                        if (std::min(lowZ1, lowZ2) < m_lowZ)
                            {
                            double cutVol2 = 0, fillVol2 = 0, cutArea2 = 0, fillArea2 = 0;
                            bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, thisPnt, *rotPnt, anp, m_lowZ, cutVol2, fillVol2, cutArea2, fillArea2);
                            fillVol -= fillVol2;
                            }
                        }
                    totalVol += fillVol;
                    }
                thisPnt = anp;
                } while (thisPnt != endPnt);
            }
        }
    return totalVol;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<DPoint3d > PondAnalysis::GetPolygonAtElevation(double elevation, TPtrList& list)
    {
    bvector<DPoint3d> points;

    for (auto sp = list.begin(); sp != list.end(); sp++)
        {
        auto rotPnt = list.GetNext(sp);
        long endPnt;
        long thisPnt = *sp;

        if (*rotPnt == *sp)
            {
            long clc = nodeAddrP(dtmP, *sp)->cPtr;
            endPnt = clistAddrP(dtmP, clc)->pntNum;
            thisPnt = endPnt;
            }
        else
            {
            endPnt = *list.GetNext(rotPnt);
            }

        DPoint3dCP rotPt = pointAddrP(dtmP, *rotPnt);

        do
            {
            long anp = bcdtmList_nextClkDtmObject(dtmP, *rotPnt, thisPnt);

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
            } while (thisPnt != endPnt);
        }
    // Close Polygon
    BeAssert(!points.empty());
    if (!points.empty())
        points.push_back(points.front());
    return points;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<bvector<DPoint3d>> PondAnalysis::GetPolygonAtElevation(double elevation)
    {
    bvector<bvector<DPoint3d>> boundaries;
    // OK This works for two point island boundary but not 1.
    for (auto&& startInfo : m_startPoints)
        {
        auto list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);
        auto points = GetPolygonAtElevation(elevation, list);
        if (!points.empty())
            boundaries.push_back(points);
        }
    return boundaries;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<bvector<DPoint3d>> PondAnalysis::GetPolygonAtElevation()
    {
    bvector<bvector<DPoint3d>> boundaries;
    double outerExitZ = m_exitZ;

    // OK This works for two point island boundary but not 1.
    for (auto&& startInfo : m_startPoints)
        {
        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);
        double elevation = outerExitZ;
        if (startInfo.m_hasFinished)
            pointAddrP(dtmP, startInfo.m_exitPoints.front().exitPoint)->z;

        auto points = GetPolygonAtElevation(elevation, list);
        if (!points.empty())
            boundaries.push_back(points);
        }
    return boundaries;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::SwapExit(long ptNum, ExpandingPondInfo& epi)
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::AddTriangleToValues(long p1, long p2, long p3, double elevation)
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
        if (m_type == LowPointType::PondExit)
            {
            double lowZ1 = pointAddrP(dtmP, p1)->z;
            double lowZ2 = pointAddrP(dtmP, p2)->z;
            double lowZ3 = pointAddrP(dtmP, p3)->z;
            if (std::min(std::min(lowZ1, lowZ2), lowZ3) < m_lowZ)
                {
                double cutVol2 = 0, fillVol2 = 0, cutArea2 = 0, fillArea2 = 0;
                bcdtmTinVolume_prismToFlatPlaneDtmObject(dtmP, p1, p2, p3, m_lowZ, cutVol2, fillVol2, cutArea2, fillArea2);
                fillVol -= fillVol2;
                }
            }
        m_currentVol += fillVol;
        m_currentArea += fillArea;
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::AddTrianglesToValues(long pnt, double elevation)
    {
    if (m_needsVolume)
        {
        long clc = nodeAddrP(dtmP, pnt)->cPtr;
        long startPnt = clistAddrP(dtmP, clc)->pntNum;
        long sp = startPnt;

        while (clc != dtmP->nullPtr)
            {
            clc = clistAddrP(dtmP, clc)->nextPtr;
            long np;
            if (clc == dtmP->nullPtr)
                np = startPnt;
            else
                np = clistAddrP(dtmP, clc)->pntNum;

            if (m_pointList[sp] == 0 && m_pointList[np] == 0)
                {
#ifdef VOLUME_DEBUG
                AddTriangle(np, thisPnt, anp, true);
#endif
                AddTriangleToValues(pnt, sp, np, elevation);
                }

            sp = np;
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::AddTrianglesToValues(long np, long sp, long endPnt, double elevation)
    {
    if (m_needsVolume)
        {
        auto rotPnt = np;
        long thisPnt = bcdtmList_nextClkDtmObject(dtmP, rotPnt, sp);

        DPoint3dCP rotPt = pointAddrP(dtmP, rotPnt);
        while (thisPnt != endPnt)
            {
            long anp = bcdtmList_nextClkDtmObject(dtmP, rotPnt, thisPnt);

            if (m_pointList[thisPnt] == 0 && m_pointList[anp] == 0)
                {
#ifdef VOLUME_DEBUG
                AddTriangle(np, thisPnt, anp, true);
#endif
                AddTriangleToValues(np, thisPnt, anp, elevation);
                }

            thisPnt = anp;
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool PondAnalysis::AddTriangleToTPtr(bvector<long>::iterator& sp, bvector<long>::iterator& np, TPtrList& list, double elevation)
    {
    long npn = *np;
    long spn = *sp;
    // If we are removing the last point.
    if (*sp == *np)
        {
        sp = list.RemovePoint(sp, sp);
        np = list.end();
        AddTrianglesToValues(npn, elevation);
        return true;
        }

    // Is is a 2 line island boundary.
    if (*list.GetNext(np) == *sp)
        {
        if (list.size() != 2)
            {
            sp = list.RemovePoint(sp, sp);
            if (sp == list.end())
                sp = list.begin();
            sp = list.RemovePoint(sp, sp);
            if (sp == list.end())
                {
                sp = list.begin() + list.size() - 1;
                }
            }
        else
            sp = list.RemovePoint(np, sp);

        AddTrianglesToValues(npn, spn, spn, elevation);
        return true;
        }

    if (list.NeedsSplit(*np))
        {
        bool success = false;
        TPtrList newList = list.Split(sp, np, success);
        if (success)
            {
            if (!newList.empty())
                {
                ExpandingPondInfo epi(newList, m_nextIndex++);
                ExpandingPondInfo* currentInfo = GetCurrentStartInfo();

                if (currentInfo->m_location != ExpandingPondInfo::Location::Inner)
                    {
                    m_outerUnknown = true;
                    currentInfo->m_location = ExpandingPondInfo::Location::Unknown;
                    }
                else
                    epi.m_location = ExpandingPondInfo::Location::Inner;

                for (auto&& anp : newList)
                    SwapExit(anp, epi);

                np = list.GetNext(sp);
                m_newStartPoints.push_back(epi);
                }
            //AddTrianglesToValues(npn); // Dont think we need this as both lists have points.
            return true;
            }
        }

    long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
    if (outerPt == -99)
        return false;

    if (pointAddrP(dtmP, outerPt)->z < elevation)
        {
        outerPt = outerPt;
        //BeAssert(false);
        }

    if (!list.Has(outerPt))
        {
        TestLowPoint(outerPt);
        sp = list.AddPoint(sp, outerPt);
        np = list.GetNext(sp);
        //continue;
        return true;
        }
    else if (outerPt == *list.GetNext(np))
        {
        long endPnt = *list.GetNext(np);
        sp = list.RemovePoint(np, sp);
        np = list.GetNext(sp);
        AddTrianglesToValues(npn, spn, endPnt, elevation);
        return true;
        }
    else
        {
        TestLowPoint(outerPt);
        sp = list.AddPoint(sp, outerPt);
        np = list.GetNext(sp);
        return true;
        }
    return false;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
template<class T> bool PondAnalysis::ExpandPolygon(T& helper, double testZ)
    {
    bool hasExpanded = false;
    bool removedIsland = false;
    bool hasSplit = false;
    for (auto&& startInfo : m_startPoints)
        {
        startInfo.needsExpanding = true;
        }

    while (true)
        {
        for (auto&& startInfo : m_startPoints)
            {
            auto& list = startInfo.tPtr;

            if (startInfo.lowZ > testZ || list.empty() || startInfo.m_hasFinished || !startInfo.needsExpanding)
                continue;

            SetCurrentIndex(startInfo.index);

            bool expanded = true;
            while (expanded && m_newStartPoints.empty())
                {
                expanded = false;
                for (auto sp = list.begin(); sp != list.end(); sp++)
                    {
                    bool expandPoint = true;
                    while (expandPoint)
                        {
                        auto np = list.GetNext(sp);
                        expandPoint = helper.func(dtmP, sp, np, list, testZ);

                        if (expandPoint)
                            {
#ifdef DEBUG
                            int spn = *sp;
                            int npn = *np;
#endif
                            if (!AddTriangleToTPtr(sp, np, list, testZ))
                                {
                                SetError();
                                return false;
                                }
#ifdef DEBUGCHK
                            list.Validate(dtmP);
#endif

                            hasExpanded = true;
                            expanded = true;

                            if (list.empty())
                                {
                                removedIsland = true;
                                break;
                                }
                            continue;
                            }
                        }
                    if (list.empty())
                        break;
                    }
                }
            startInfo.needsExpanding = expanded;
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
            if (!sp.tPtr.empty())
                newSP.push_back(sp);
            }
        std::swap(newSP, m_startPoints);
        }
    if (hasSplit)
        FindOuterBoundary();
    return hasExpanded;
    }

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct RemoveAtElevation
    {
    PondAnalysisR m_analysis;

    RemoveAtElevation(PondAnalysisR analysis) : m_analysis(analysis)
        {
        }

    bool func(BC_DTM_OBJ* dtmP, bvector<long>::iterator sp, bvector<long>::iterator np, TPtrList& list, double testZ)
        {
        bool removePoint = false;
        if (nodeAddrP(dtmP, *np)->hPtr == dtmP->nullPnt)
            {
            const double npZ = pointAddrP(dtmP, *np)->z;

            if (npZ == testZ)
                {

                if (*list.GetNext(np) == *sp)
                    removePoint = true;
                else
                    {
                    bool isExit, isSump;

                    m_analysis.IsSumpOrExit(isSump, isExit, sp, np, list);

                    if (!isExit)
                        removePoint = true;
                    }
                }
            }
        return removePoint;
        }
    };

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool PondAnalysis::ExpandPolygon(double testZ)
    {
    RemoveAtElevation helper(*this);
    return ExpandPolygon(helper, testZ);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::IsSump(long prevPnt, long pnt, long nextPnt, bool& isSump, bool& isExit)
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::IsSumpOrExit(bool& isSump, bool& isExit, bvector<long>::iterator sp, bvector<long>::iterator np, TPtrList& list)
    {
    BeAssert(m_pointList[*np] <= 2);
    if (m_pointList[*np] == 1)
        IsSump(*sp, *np, *list.GetNext(np), isSump, isExit);
    else
        {
        auto next = std::find(np + 1, list.end(), *np);
        if (next == list.end())
            next = std::find(list.begin(), np, *np);
        if (next != np)
            {
            isSump = false;
            isExit = false;
            }
        else
            IsSump(*sp, *np, *list.GetNext(np), isSump, isExit);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
struct RemoveZSlope
    {
    PondAnalysisR m_analysis;
    bool m_doSumps = false;

    RemoveZSlope(PondAnalysisR analysis) : m_analysis(analysis)
        {
        }

    bool func(BC_DTM_OBJ* dtmP, bvector<long>::iterator sp, bvector<long>::iterator np, TPtrList& list, double testZ)
        {
        // If this is not on the hull.
        const double npZ = pointAddrP(dtmP, *np)->z;
        bool removePoint = false;
        if (npZ == testZ)
            {
            if (*sp == *np)
                return true;
            if (list.GetPointList()[*np] != 1)
                {
                auto next = std::find(np + 1, list.end(), *np);
                if (next == list.end())
                    next = std::find(list.begin(), np, *np);
                if (next != np)
                    return true;
                }
            bool isOnHull = nodeAddrP(dtmP, *np)->hPtr != dtmP->nullPnt;
            const double spZ = pointAddrP(dtmP, *sp)->z;

            if (!isOnHull)
                {
                // Is it a flat triangle @ testZ
                if (spZ == testZ)
                    {
                    long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
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
                    bool isSump = false;
                    m_analysis.IsSumpOrExit(isSump, isExit, sp, np, list);

                    if (isOnHull || isExit)
                        {
                        isExit = true;
                        }
                    else
                        {
                        if (*sp == *np)
                            removePoint = true;
                        else
                            {
                            long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
                            double outerZ = pointAddrP(dtmP, outerPt)->z;
                            if (outerZ == testZ)
                                removePoint = true;
                            else
                                {
                                isExit = isSump;
                                }
                            }
                        }
                    }
                }
            }
        return removePoint;
        }
    };

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool PondAnalysis::GetExitPoints(double testZ)
    {
    RemoveZSlope helper(*this);

    bool ret = ExpandPolygon(helper, testZ);

    if (DTM_SUCCESS != m_errorStatus)
        return false;

    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.m_hasFinished)
            continue;

        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);
        for (auto sp = list.begin(); sp != list.end(); sp++)
            {
            auto np = list.GetNext(sp);

            if (nodeAddrP(dtmP, *np)->tPtr != dtmP->nullPnt)
                continue;
            bool isOnHull = nodeAddrP(dtmP, *np)->hPtr != dtmP->nullPnt;
            if (pointAddrP(dtmP, *np)->z == testZ)
                {
                bool isExit = false;
                bool isSump = false;
                IsSumpOrExit(isSump, isExit, sp, np, list);
                if (isOnHull || isExit)
                    {
                    isExit = true;
                    }
                else
                    {
                    if (*sp == *np)
                        {
                        }
                    else
                        {
                        long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
                        double outerZ = pointAddrP(dtmP, outerPt)->z;
                        if (outerZ == testZ)
                            {
                            }
                        else
                            {
                            isExit = isSump;
                            }
                        }
                    }
                if (isExit)
                    {
                    nodeAddrP(dtmP, *np)->tPtr = 1;
                    startInfo.m_exitPoints.push_back(PondExitInfo(*np, *sp, *list.GetNext(np)));
                    }
                }
            }

        if (!startInfo.m_exitPoints.empty())
            {
            for (const auto& pnt : startInfo.m_exitPoints)
                nodeAddrP(dtmP, pnt.exitPoint)->tPtr = dtmP->nullPnt;
            startInfo.m_hasFinished = true;
            }
        }
    return ret;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::SaveBoundaryList()
    {
    for (auto&& startInfo : m_startPoints)
        startInfo.tPtr.Save();

#ifdef DEBUG_CHKPOINTLIST
    for (int i = 0; i < dtmP->numPoints; i++)
        if (m_pointList[i] != 0)
            return;
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::RestoreBoundaryPoints()
    {
    for (auto&& startInfo : m_startPoints)
        startInfo.tPtr.Restore();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool PondAnalysis::FixLowPts(long& startPoint, double z)
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::GetLowestPoint(long& outLowPtNum, double& outLowZ)
    {
    outLowPtNum = -1;
    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.m_hasFinished)
            continue;

        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);

        long lowPtNum = -1;
        double lowZ = 0;

        for (auto&& pnt : list)
            {
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
            }
        startInfo.lowZ = lowZ;
        startInfo.lowPtNum = lowPtNum;

        if (outLowPtNum == -1 || outLowZ > lowZ)
            {
            outLowPtNum = lowPtNum;
            outLowZ = lowZ;
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::TestLowPoint(long pnt)
    {
    if (pointAddrP(dtmP, pnt)->z == m_lowZ)
        {
        if (std::find(m_lowPoints.begin(), m_lowPoints.end(), pnt) == m_lowPoints.end())
            m_lowPoints.push_back(pnt);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::AddInitialLowPts()
    {
    for (auto&& startInfo : m_startPoints)
        {
        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);

        for (auto sp : list)
            {
            if (pointAddrP(dtmP, sp)->z == m_lowZ)
                {
                if (std::find(m_lowPoints.begin(), m_lowPoints.end(), sp) == m_lowPoints.end())
                    m_lowPoints.push_back(sp);
                }
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::InsertTPtrPolygonAtElevation()
    {
    long startPnt;
    bvector<long> sidePnts;
    // ToDo Ascent
    // ToDo Hull tests and void tests!
    double testZ = pointAddrP(dtmP, m_lowPnt)->z;
    long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr;
    long sp = -1;
    while (clc != dtmP->nullPtr)
        {
        sp = clistAddrP(dtmP, clc)->pntNum;
        clc = clistAddrP(dtmP, clc)->nextPtr;
        if (pointAddrP(dtmP, sp)->z > testZ)
            sidePnts.push_back(sp);
        }

    if (sidePnts.empty())
        {
        SetError();
        return;
        }

    for (auto sp : sidePnts)
        {
        bvector<long> pts;

        bool pointExists = false;

        for (auto&& startInfo : m_startPoints)
            {
            if (std::find(startInfo.tPtr.begin(), startInfo.tPtr.end(), sp) != startInfo.tPtr.end())
                {
                pointExists = true;
                break;
                }
            }
        if (pointExists)
            continue;

        long firstPnt = sp;
        startPnt = sp;
        long rotPnt = m_lowPnt;
        long firstRotPnt = rotPnt;

        while (true)
            {
            long np = bcdtmList_nextClkDtmObject(dtmP, rotPnt, sp);

            if (pointAddrP(dtmP, np)->z <= testZ && nodeAddrP(dtmP, np)->hPtr == dtmP->nullPnt)
                {
                rotPnt = np;
                }
            else
                {
                pts.push_back(np);
                sp = np;
                }
            if (sp == firstPnt && firstRotPnt == rotPnt)
                break;
            }
        SetCurrentIndex(m_nextIndex);
        m_startPoints.push_back(ExpandingPondInfo(TPtrList(m_pointList, pts.begin(), pts.end()), m_nextIndex++));
        m_startPoints.back().m_location = ExpandingPondInfo::Location::Unknown;
        }

    m_outerUnknown = true;
    FindOuterBoundary();
    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
            std::reverse(startInfo.tPtr.begin(), startInfo.tPtr.end());
        }
    m_outerUnknown = false;

#ifdef DEBUG
    //for (auto&& si : m_startPoints)
    //    {
    //    if (si.m_location == ExpandingPondInfo::Location::Outer)
    //        {
    //        DTMDirection direction;
    //        double area;
    //        SetCurrentIndex(si.index);
    //        bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP, si.startPnt, &area, &direction);
    //        if (direction != DTMDirection::AntiClockwise)
    //            {
    //            //SetError();
    //            bcdtmList_reverseTptrPolygonDtmObject(dtmP, si.startPnt);
    //            }
    //        break;
    //        }
    //    }
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::Clear()
    {
    m_startPoints.clear();
    m_currentIndex = 0;
    m_currentVolumePoints.clear();
    m_newStartPoints.clear();
    m_nextIndex = 0;
    m_currentZ = -1e99;
    m_currentArea = 0;
    m_currentVol = 0;
    m_totalVol = -1;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::CreateInitialTPtr()
    {
    //LowPointType oldType = m_type;
    //GetType();
    //if (m_type != oldType)
    //    {
    //    if (!(m_type == LowPointType::Triangle && oldType == LowPointType::Edge))
    //        m_type = oldType;
    //    }

    // This shouldn't be needed.
    switch (m_type)
        {
        case LowPointType::Point:
            //            case LowPointType::Edge:
                        //case LowPointType::Triangle:
            {
            long firstPoint;
            if (bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP, m_lowPnt, &firstPoint)) return;
            SetCurrentIndex(m_nextIndex);

            ExpandingPondInfo epi(TPtrList(m_pointList, firstPoint, dtmP), m_nextIndex++);
            epi.m_location = ExpandingPondInfo::Location::Outer;
            m_outerUnknown = false;
            m_startPoints.push_back(epi);
            TestLowPoint(m_lowPnt);
            break;
            }
        case LowPointType::Edge:
            {
            long antPnt = bcdtmList_nextAntDtmObject(dtmP, m_lowPnt, m_pntNum2);
            long clkPnt = bcdtmList_nextClkDtmObject(dtmP, m_lowPnt, m_pntNum2);
            TestLowPoint(m_lowPnt);
            TestLowPoint(m_pntNum2);

            nodeAddrP(dtmP, m_lowPnt)->tPtr = clkPnt;
            nodeAddrP(dtmP, clkPnt)->tPtr = m_pntNum2;
            nodeAddrP(dtmP, m_pntNum2)->tPtr = antPnt;
            nodeAddrP(dtmP, antPnt)->tPtr = m_lowPnt;
            AddTriangleToValues(m_lowPnt, m_pntNum2, antPnt, m_lowZ);
            AddTriangleToValues(m_lowPnt, m_pntNum2, clkPnt, m_lowZ);

            long firstPoint = m_lowPnt;
//            m_isZSlope = FixLowPts(firstPoint, m_lowZ);
            SetCurrentIndex(m_nextIndex);
            ExpandingPondInfo epi(TPtrList(m_pointList, m_lowPnt, dtmP), m_nextIndex++);
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
            TestLowPoint(m_lowPnt);
            TestLowPoint(m_pntNum2);
            TestLowPoint(m_pntNum3);

            long firstPoint = m_lowPnt;
            //m_isZSlope = FixLowPts(firstPoint, m_lowZ);
            SetCurrentIndex(m_nextIndex);
            ExpandingPondInfo epi(TPtrList(m_pointList, firstPoint, dtmP), m_nextIndex++);
            epi.m_location = ExpandingPondInfo::Location::Outer;
            m_outerUnknown = false;
            m_startPoints.push_back(epi);
            break;
            }

        case LowPointType::PondExit:
            TestLowPoint(m_lowPnt);
            InsertTPtrPolygonAtElevation();
            break;
        }

    if (m_startPoints.empty() || m_startPoints.front().tPtr.empty())
        SetError();
#ifdef DEBUG

    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
            {
            DTMDirection direction;
            double area;
            auto list = startInfo.tPtr;
            list.push_back(list.front());
            bcdtmMath_getPointOffsetPolygonDirectionAndAreaDtmObject(dtmP, list.data(), (long)list.size(), &direction, &area);
            BeAssert(direction == DTMDirection::AntiClockwise);
            if (direction != DTMDirection::AntiClockwise)
                std::reverse(startInfo.tPtr.begin(), startInfo.tPtr.end());
            }
        }
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::GetType()
    {
    double testZ = pointAddrP(dtmP, m_lowPnt)->z;
    long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr;
    long sp = -1;
    long np;
    bool hasZSlope = false;
    while (clc != dtmP->nullPtr)
        {
        sp = clistAddrP(dtmP, clc)->pntNum;
        clc = clistAddrP(dtmP, clc)->nextPtr;
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
                np = clistAddrP(dtmP, nodeAddrP(dtmP, m_lowPnt)->cPtr)->pntNum;
            else
                np = clistAddrP(dtmP, clc)->pntNum;
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
        long clc = nodeAddrP(dtmP, m_lowPnt)->cPtr;
        sp = clistAddrP(dtmP, clc)->pntNum;
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::RefineTargetElevation()
    {
    if (!m_targetVolumeNeedsRefining)
        return;

    const double elevationTol = m_tracer.GetPondElevationTolerance();
    const double volumeTol = m_tracer.GetPondVolumeTolerance();
    m_targetVolumeNeedsRefining = false;

    double ascentFix = AscentTrace() ? -1 : 1;
    double minZ = m_targetRefine_minZ;
    double maxZ = m_targetRefine_maxZ;
    double minVol = m_targetRefine_minVolume;
    double maxVol = m_targetRefine_maxVolume;

    // Calculate new Volume.
    const double tol = 0.1;
    const double ptol = 0.001;
    double z = minZ;
    while (true)
        {
        if (maxZ - minZ < elevationTol)
            break;

        if (minZ >= maxZ || minVol >= maxVol)
            {
            SetError();
            return;
            }
        double percent = (m_totalVol - minVol) / (maxVol - minVol);
        z = minZ + (maxZ - minZ) * percent;

        const double newVolFromArea2 = m_currentArea * (ascentFix * (z - m_currentZ));
        const double newVolFromPartials2 = GetVolumeOfTrianglesOnSide(z);
        double newVol = m_currentVol + newVolFromArea2 + newVolFromPartials2;

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

        if (fabs(m_totalVol - newVol) < volumeTol)
            {
            break;
            }
        if (newVol > m_totalVol)
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
    m_exitZ = z;
    m_currentVolumePoints.clear();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::FindBoundaryForVolume(bool& gotTargetvolume, double targetVolume, double lowestExitPntZ)
    {
    gotTargetvolume = false;
    bool hasTargetVol = targetVolume > -1e89;
    bool haslowestExitPntZ = lowestExitPntZ > -1e89;
    m_needsVolume = hasTargetVol;
    double ascentFix = AscentTrace() ? -1 : 1;
    double ptZ = m_currentZ;

    if (m_isFlatPond)   // This pond doesn't have any elevation, eg it is flat, so exit.
        {
        gotTargetvolume = true;
        return;
        }

    if (m_totalVol <= 0 || targetVolume < m_currentVol)
        Clear();

    if (hasTargetVol)
        m_totalVol = 0;

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
        bool hasFinished = true;
        for (auto&& startInfo : m_startPoints)
            {
            if (!haslowestExitPntZ) //hasTargetVol)
                {
                if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
                    {
                    hasFinished = startInfo.m_hasFinished;
                    break;
                    }
                }
            else if (haslowestExitPntZ)
                {
                //if (pointAddrP(dtmP, startInfo.m_exitPoints[0].exitPoint)->z < lowestExitPntZ)
                //    hasFinished = true;
                }
            else if (!startInfo.m_hasFinished)
                {
                m_isFlatPond = firstTime;
                hasFinished = false;
                break;
                }
            }

        if (hasFinished)
            {
            m_exitZ = ptZ;
            break;
            }

        // This also needs to scan around the point
        if (!firstTime && !ExpandPolygon(m_currentZ))
            {
            //SetError(); // Didn't expand, can't find exit point.
            //return;
            }
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

        if (firstTime)
            m_currentZ = m_lowZ;
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
            m_totalVol = newVol;
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
                m_totalVol = targetVolume;
                m_targetRefine_minZ = m_currentZ;
                m_targetRefine_maxZ = lowZ;
                m_targetRefine_minVolume = previousVolume;
                m_targetRefine_maxVolume = newVol;
                m_targetVolumeNeedsRefining = true;
                gotTargetvolume = true;
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

        if (false)
            DebugSaveTPtr();
        firstTime = false;

        }
    if (hasTargetVol)
        m_currentVolumePoints.clear();

    return;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
const bvector<bvector<DPoint3d>>& PondAnalysis::GetCurrentVolumePoints()
    {
    if (m_currentVolumePoints.empty() || m_targetVolumeNeedsRefining)
        {
        RefineTargetElevation();
        m_currentVolumePoints = GetPolygonAtElevation(m_exitZ);
        }

    return m_currentVolumePoints;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<DPoint3d> PondAnalysis::GetOuterBoundary()
    {
    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
            {
            if (!startInfo.m_hasFinished)
                break;
            BoundaryListHelper helper(*this);
            auto& list = startInfo.tPtr;
            SetCurrentIndex(startInfo.index);
            return GetPolygonAtElevation(pointAddrP(dtmP, startInfo.m_exitPoints[0].exitPoint)->z, list);
            }
        }
    return bvector<DPoint3d>();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<bvector<DPoint3d>> PondAnalysis::GetBoundary()
    {
    BoundaryListHelper __helper(*this);

    return GetPolygonAtElevation();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<PondExitInfo> PondAnalysis::GetOuterExits()
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::FindPond()
    {
    bool gotTargetVol;
    FindBoundaryForVolume(gotTargetVol);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::DebugSaveTPtr(int n)
    {
    int curIndex = m_currentIndex;
    TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::Create();
    for (auto&& startInfo : m_startPoints)
        {
        if (startInfo.tPtr.empty())
            continue;

        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);

        bvector<DPoint3d> pts;
        for (auto& p : list)
            pts.push_back(*pointAddrP(dtmP, p));

        pts.push_back(pts.front());

        DTMFeatureId fId;
        dtm->AddLinearFeature(DTMFeatureType::Breakline, pts.data(), (int)pts.size(), &fId);
        }

    for (auto&& startInfo : m_newStartPoints)
        {
        if (startInfo.tPtr.empty())
            continue;

        auto& list = startInfo.tPtr;
        SetCurrentIndex(startInfo.index);

        bvector<DPoint3d> pts;
        for (auto& p : list)
            pts.push_back(*pointAddrP(dtmP, p));

        pts.push_back(pts.front());

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
            auto existingPond = m_tracer.FindPondLowPt(trgPnt1);

            if (existingPond != nullptr)
                return;
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
        if (trgPnt3 < 0)
            {
            SetError(); return;
            }
        if (pointAddrP(dtmP, trgPnt3)->z < z && bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
            {
            auto child = TraceOnEdge::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }

        std::swap(trgPnt1, trgPnt2);
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 < 0)
            {
            SetError(); return;
            }
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
        double pnt1Z = pointAddrP(dtmP, nextPnt1)->z;
        if (pnt1Z == pointAddrP(dtmP, nextPnt2)->z)
            {
            long sidePnt1 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2);
            long sidePnt2 = bcdtmList_nextClkDtmObject(dtmP, nextPnt1, nextPnt2);

            if (pointAddrP(dtmP, sidePnt1)->z > pnt1Z && pointAddrP(dtmP, sidePnt2)->z > pnt1Z)
                {
                auto existingPond = m_tracer.FindPondLowPt(nextPnt1);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    }
                else
                    {
                    auto child = TracePondEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, *pointAddrP(dtmP, nextPnt1));
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                return;
                }
            }

        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, angle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnEdge::TraceOnEdge(DrainageTracer& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2), pnt3(P3), m_pt(startPt), m_startPt(startPt), lastAngle(lastAngle)
    {
    m_points.push_back(startPt);

#ifdef DEBUG
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    double pnt1Z = pointAddrP(dtmP, P1)->z;
    if (pnt1Z == pointAddrP(dtmP, P2)->z)
        {
        long sidePnt1 = bcdtmList_nextAntDtmObject(dtmP, P1, P2);
        long sidePnt2 = bcdtmList_nextClkDtmObject(dtmP, P1, P2);

        if (pointAddrP(dtmP, sidePnt1)->z > pnt1Z && pointAddrP(dtmP, sidePnt2)->z > pnt1Z)
            {
            BeAssert(false);
            }
        }
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::ProcessZSlopeTriangle(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    //if (dbg) bcdtmWrite_message(0, 0, 0, "Zero Slope Triangle From Triangle Edge");
    if (m_tracer.GetZeroSlopeOption() == ZeroSlopeTraceOption::TraceLastAngle)
        {
        //if (dbg) bcdtmWrite_message(0, 0, 0, "** Tracing At Last Angle");

        // Check Last Angle has Been Initialised

        BeAssert(lastAngle != -99.99);
        if (lastAngle == -99.99)
            {
            SetError(L"No LastAngle set.");
            return;
            //lastAngle = bcdtmMath_getPointAngleDtmObject(dtmP, startPnt1, startPnt2) + DTM_2PYE / 4.0;
            //while (lastAngle > DTM_2PYE) lastAngle = lastAngle - DTM_2PYE;
            }
        DPoint3d nextPt;
        long nextPnt1, nextPnt2, nextPnt3;
        if (bcdtmDrainage_calculateAngleIntersectOfRadialFromTriangleEdgeWithTriangleDtmObject(dtmP, pnt1, pnt2, pnt3, m_pt.x, m_pt.y, lastAngle, &nextPt.x, &nextPt.y, &nextPt.z, &nextPnt1, &nextPnt2, &nextPnt3))
            {
            SetError(); return;
            }
        m_points.push_back(nextPt);

        if (nextPnt3 == dtmP->nullPnt)  // This flows of the triangulation?
            {
            m_finished = true;
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, pnt1, nextPt, lastAngle);
            m_children.push_back(child);
            return;
            }
        long hullPoint = 0;
        if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, nextPnt3, &hullPoint))
            {
            SetError(); return;
            }

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
        m_finished = true;
        auto existingPond = m_tracer.FindPondLowPt(pnt1);

        if (existingPond != nullptr)
            {
            m_children.push_back(existingPond);
            }
        else
            {
            auto child = TracePondTriangle::Create(m_tracer, *this, pnt1, pnt2, pnt3, m_pt);
            newFeatures.push_back(child);
            m_children.push_back(child);
            }
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
        auto existingPond = m_tracer.FindPondLowPt(pnt1);

        if (existingPond != nullptr)
            {
            m_children.push_back(existingPond);
            }
        else
            {
            auto child = TracePondTriangle::Create(m_tracer, *this, pnt1, pnt2, prevPnt, *pointAddrP(dtmP, pnt1));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }
        }
    else
        {
        auto existingPond = m_tracer.FindPondLowPt(pnt1);

        if (existingPond != nullptr)
            {
            m_children.push_back(existingPond);
            }
        else
            {
            auto child = TracePondEdge::Create(m_tracer, *this, pnt1, pnt2, *pointAddrP(dtmP, pnt1));
            newFeatures.push_back(child);
            m_children.push_back(child);
            }
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
    if (bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP, nullptr, pnt1, pnt2, pnt3, voidTriangle, flowDirection))
        {
        SetError(); return;
        }

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
    if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, nullptr, pnt1, pnt3, pnt2, voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS)
        {
        SetError(); return;
        }
    //    if (dbg) bcdtmWrite_message(0, 0, 0, "slope = %8.4lf ascentAngle = %12.10lf descentAngle = %12.10lf", slope, ascentAngle, descentAngle);
    /*
    **        Calculate Radial Out From Start X And Start Y At Descent Angle
    */
    if (m_tracer.AscentTrace()) descentAngle = ascentAngle;

    lastAngle = descentAngle;
    const double dx = dtmP->xMax - dtmP->xMin;
    const double dy = dtmP->yMax - dtmP->yMin;

    const double radius = sqrt(dx * dx + dy * dy);
    const double xRad = m_pt.x + radius * cos(descentAngle);
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
        if (bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP, m_pt.x, m_pt.y, xRad, yRad, pnt3, pnt2, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
            {
            SetError(); return;
            }
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
            if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0)
                {
                SetError(); return;
                }
            }
        }
    /*
    **        Flow Intersects Edge P1-P3
    */
    else if (sdof < 0)
        {
        long intPnt;
        if (bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP, m_pt.x, m_pt.y, xRad, yRad, pnt1, pnt3, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
            {
            SetError(); return;
            }
        if (intPnt != dtmP->nullPnt)
            {
            nextPnt1 = intPnt;
            nextPt = *pointAddrP(dtmP, nextPnt1);
            }
        else
            {
            nextPnt1 = pnt1;
            nextPnt2 = pnt3;
            if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0)
                {
                SetError(); return;
                }
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
        if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2)) < 0)
            {
            SetError(); return;
            }
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

    if (m_tracer.GetZeroSlopeOption() == ZeroSlopeTraceOption::TraceLastAngle)
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
        if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, m_ptNum, descentPnt1, descentPnt2, m_lastAngle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
            {
            SetError(); return;
            }
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
                if ((nextPnt3 = bcdtmList_nextAntDtmObject(dtmP, descentPnt1, descentPnt2)) < 0)
                    {
                    SetError(); return;
                    }
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
    auto existingPond = m_tracer.FindPondLowPt(descentPnt1);
    if (existingPond != nullptr)
        {
        m_children.push_back(existingPond);
        }
    else
        {
        auto pond = TracePondEdge::Create(m_tracer, *this, descentPnt1, descentPnt2, *pointAddrP(dtmP, descentPnt1));
        //m_tracer.AddPond(*pond);
        newFeatures.push_back(pond);
        m_children.push_back(pond);
        }
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
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_ptNum, &hullPoint))
        {
        SetError(); return;
        }

    if (hullPoint)
        {
        m_onHullPoint = true;
        m_finished = true;
        return;
        }

    if (m_tracer.AscentTrace())
        {
        if (bcdtmDrainage_scanPointForMaximumAscentDtmObject(dtmP, nullptr, m_ptNum, m_prevPtNum, &type, &pnt1, &pnt2, &slope, &angle))
            {
            SetError(); return;
            }
        }
    else
        {
        if (bcdtmDrainage_scanPointForMaximumDescentDtmObject(dtmP, nullptr, m_ptNum, m_prevPtNum, &type, &pnt1, &pnt2, &slope, &angle))
            {
            SetError(); return;
            }
        }

    // type : 0 = Low/HighPoint
    //        1 = To a Point
    //        2 = Down a Triangle

    m_finished = true;

    if (type == 0)
        {
        auto existingPond = m_tracer.FindPondLowPt(m_ptNum);
        if (existingPond != nullptr)
            {
            m_children.push_back(existingPond);
            }
        else
            {
            auto pond = TracePondLowPoint::Create(m_tracer, *this, m_ptNum, m_pt);
            newFeatures.push_back(pond);
            m_children.push_back(pond);
            }
        return;

        }
    else if (type == 1)
        {
        if (slope == 0) // ZeroSlope
            {
            auto existingPond = m_tracer.FindPondLowPt(m_ptNum);
            if (existingPond != nullptr)
                {
                m_children.push_back(existingPond);
                }
            else
                {
                auto child = TracePondEdge::Create(m_tracer, *this, m_ptNum, pnt1, m_pt);
                newFeatures.push_back(child);
                m_children.push_back(child);
                }
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

        double pnt1Z = pointAddrP(dtmP, pnt1)->z;
        if (pnt1Z == pointAddrP(dtmP, pnt2)->z)
            {
            long sidePnt1 = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
            long sidePnt2 = bcdtmList_nextClkDtmObject(dtmP, pnt1, pnt2);

            if (pointAddrP(dtmP, sidePnt1)->z > pnt1Z && pointAddrP(dtmP, sidePnt2)->z > pnt1Z)
                {
                auto existingPond = m_tracer.FindPondLowPt(pnt1);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    }
                else
                    {
                    auto child = TracePondEdge::Create(m_tracer, *this, pnt1, pnt2, m_pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                return;
                }
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

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
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
    m_pondAnalysis = PondAnalysis::Create(m_tracer, m_ptNum, m_pt.z, PondAnalysis::LowPointType::Triangle, m_ptNum3, m_ptNum2);

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
    long oldPondExitPt = m_pondExit->GetExitPoint();
    m_pondAnalysis = PondAnalysis::Create(m_tracer, oldPondExitPt, pointAddrP(dtmP, oldPondExitPt)->z, PondAnalysis::LowPointType::PondExit);

    m_pondAnalysis->FindPond();
    exits = m_pondAnalysis->GetOuterExits();

    m_points = m_pondAnalysis->GetBoundary();

    if (exits.empty())
        {
        // Recover hidden ponds.
        m_pondExit->RecoverInnerPonds();
        }
    return;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
TracePond::TracePond(TracePondCR from, DrainageTracer& newTracer) : TraceFeature(from, newTracer)
    {
    m_points = from.m_points;
    m_exitPoints = from.m_exitPoints;
    m_depth = from.m_depth;
    m_pt = from.m_pt;
    m_ptNum = from.m_ptNum;
    m_maxVolume = from.m_maxVolume;
    m_pondAnalysis = from.m_pondAnalysis->Clone(newTracer);
    m_allFull = m_allFull;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysis& TracePond::GetPondAnalysis()
    {
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
        if (CurrentVolume() != 0)
            {
            if (IsFull())
                {
                for (auto& p : m_points)
                    loadFunction(DTMFeatureType::LowPointPond, 0, 0, p.data(), p.size(), args);   //ToDO Remove.
                }
            else
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
                        loadFunction(DTMFeatureType::LowPointPond, 0, 0, const_cast<DPoint3dP>(points.data()), points.size(), args);
                        }
                    }

                for (auto& p : m_points)
                    loadFunction(DTMFeatureType::DescentTrace, 0, 0, p.data(), p.size(), args);   //ToDO Remove.
                }
            }
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

    // Add Low Points.
    for (auto pnt : m_pondAnalysis->GetLowPoints())
        m_tracer.AddPondLowPond(pnt, *this);

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
            if (!pond->IsCalculated())
                {
                bvector<TraceFeaturePtr> newFeatures;
                pond->Process(newFeatures, true);
                m_tracer.AddAndProcessFeatures(newFeatures);
                }
            if (!pond->IsDeadPond())
                {
                hasNonDeadPond = true;
                break;
                }
            for (auto newPond : pond->GetPonds())
                {
                for (auto pondExit : newPond->GetPondExits())
                    {
                    if (std::find(existingPondsExits.begin(), existingPondsExits.end(), pondExit) == existingPondsExits.end())
                        {
                        existingPondsExits.push_back(pondExit);
                        }
                    }
                }
            }

        if (!hasNonDeadPond)
            {
            // Create new Pond from PondExit.
            existingPondsExits[0]->ProcessDeadPond(newFeatures);
            for (auto&& a : existingPondsExits)
                a->SetHasProcessedDeadPond();
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
bvector<TracePondExitP> TracePond::GetPondExits()
    {
    bvector<TracePondExitP> ret;
    for (int exitIndex : m_exitPoints)
        {
        auto exit = m_tracer.FindPondExit(exitIndex);
        if (nullptr != exit)
            ret.push_back(exit);
        }
    //for (auto&& child : GetChildren())
    //    {
    //    auto pondExit = dynamic_cast<TracePondExit*>(child.get());
    //    if (nullptr != pondExit)
    //        ret.push_back(pondExit);
    //    }
    return ret;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume)
    {
#ifdef DEBUG
    static const long ptNumToCheck = 10211;

    if (m_ptNum == ptNumToCheck)
        totalVol = totalVol;
#endif
    if (m_maxVolume < 0)
        {
        bool gotTargetVol = false;
        GetPondAnalysis().FindBoundaryForVolume(gotTargetVol, CurrentVolume());
        if (gotTargetVol)
            return;

        m_maxVolume = GetPondAnalysis().GetCurrentVolume();
        //GetPondAnalysis().SetToMax(m_points);
        }

    double prevVolume = CurrentVolume() - totalVol;

    double vol = CurrentVolume() - m_maxVolume;
    if (prevVolume > m_maxVolume)
        vol = totalVol;

    BeAssert(vol >= 0);
    if (vol <= 0)
        return;

    bvector<TracePondExitP> exits;
    if (!m_allFull)
        {
        bvector<TracePondExitP> newPondExits;
        bvector<TracePondExitP> foundExits;

        for (auto pondExit : GetPondExits())
            {
            newPondExits.push_back(pondExit);
            foundExits.push_back(pondExit);
            }

        while (exits.empty() && !newPondExits.empty())
            {
            bvector<TracePondExitP> currentPondExits;

            std::swap(currentPondExits, newPondExits);
            for (auto pondExit : currentPondExits)
                {
                if (!pondExit->IsCalculated())
                    {
                    if (std::find(exits.begin(), exits.end(), pondExit) == exits.end())
                        exits.push_back(pondExit);
                    break;
                    }

                for (auto pond : pondExit->GetPonds())
                    {
                    if (!pond->IsFull())
                        {
                        if (std::find(exits.begin(), exits.end(), pondExit) == exits.end())
                            exits.push_back(pondExit);
                        break;
                        }

                    for (auto newPondExit : pond->GetPondExits())
                        {
                        if (std::find(foundExits.begin(), foundExits.end(), newPondExit) == foundExits.end())
                            {
                            foundExits.push_back(newPondExit);
                            newPondExits.push_back(newPondExit);
                            }
                        }
                    }
                }
            }
        }
    if (!exits.empty())
        {
        vol /= exits.size();
        for (auto exit : exits)
            {
            BeAssert(nullptr != exit);
            newWaterVolume.push_back(WaterVolumeInfo(exit, vol));
            }
        }
    else
        {
        m_allFull = true;
        vol /= m_exitPoints.size();
        for (int exitIndex : m_exitPoints)
            {
            auto exitPoint = m_tracer.FindPondExit(exitIndex);
            BeAssert(nullptr != exitPoint);
            if (nullptr != exitPoint)
                newWaterVolume.push_back(WaterVolumeInfo(exitPoint, vol));

            }
        }
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

    BeAssertOnce(m_ponds.size() <= m_numExitFlows);
    if (m_ponds.size() >= m_numExitFlows)
        {
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
                newWaterVolume.push_back(WaterVolumeInfo(pond, vol));

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
        }
    else
        __super::GetNewWaterVolumes(totalVol, newWaterVolume);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
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
    //BeAssert(m_tracer.FindPondLowPt(m_exitPnt) == nullptr);    // Not sure if this is needed.

    auto child = TracePondFromPondExit::Create(m_tracer, *this, *this, lowPt, lowPtNum, m_exitPt);
    //m_tracer.AddPond(*child);
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

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject_IncludeZSlope
(
    BC_DTM_OBJ        *dtmP,                  // ==> Pointer To Tin Object
    DTMDrainageTables *drainageTablesP,       // ==> Pointer To Drainage Tables
    long              priorPnt,               // ==> Prior Point On Pond Boundary
    long              exitPnt,                // ==> Exit  Point On Pond Boundary
    long              nextPnt,                // ==> Next  Point On Pond Boundary
    double            startX,                 // ==> X Coordinate Of Triangle Point To Start Trace
    double            startY,                 // ==> Y Coordinate Of Triangle Point To Start Trace
    long              *nextPnt1P,             // <== Point 1 Of Next Triangle Edge
    long              *nextPnt2P,             // <== Point 2 Of Next Triangle Edge
    long              *nextPnt3P,             // <== Point 3 Of Next Triangle Edge
    double            *nextXP,                // <== Next Trace Point X Coordinate
    double            *nextYP,                // <== Next Trace Point Y Coordinate
    double            *nextZP,                // <== Next Trace Point Z Coordinate
    bool              *processP               // <== Next Trace Point Found
)
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0);
    long   intPnt, descentType = 0, descentPnt1 = 0, descentPnt2 = 0;
    double descentAngle = 0.0, descentSlope = 0.0;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Tracing Maximum Descent From Pond Exit Point");
        bcdtmWrite_message(0, 0, 0, "dtmP            = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "drainageTablesP = %pld", drainageTablesP);
        bcdtmWrite_message(0, 0, 0, "priorPnt        = %8ld", priorPnt);
        bcdtmWrite_message(0, 0, 0, "exitPnt         = %8ld", exitPnt);
        bcdtmWrite_message(0, 0, 0, "startX          = %12.5lf", startX);
        bcdtmWrite_message(0, 0, 0, "startY          = %12.5lf", startY);
        }
    /*
    ** Initialise Variables
    */
    *processP = false;
    *nextXP = 0.0;
    *nextYP = 0.0;
    *nextZP = 0.0; ;
    *nextPnt1P = dtmP->nullPnt;
    *nextPnt2P = dtmP->nullPnt;
    *nextPnt3P = dtmP->nullPnt;
    /*
    ** Range Scan Point For Maximum Descent
    */
    if( bcdtmDrainage_rangeScanPointForMaximumDescentDtmObject(dtmP,drainageTablesP,exitPnt,priorPnt,nextPnt,&descentType,&descentPnt1,&descentPnt2,&descentSlope,&descentAngle)) goto errexit ;
    if (dbg) bcdtmWrite_message(0, 0, 0, "descentType = %2ld descentSlope = %8.3lf descentAngle = %12.10lf descentPnt1 = %9ld descentPnt2 = %9ld", descentType, descentSlope, descentAngle, descentPnt1, descentPnt2);
    /*
    ** Check Descent Slope Is Not Zero
    */
    //if (descentSlope == 0.0) descentType = 0;
    /*
    **  Maximum Descent Is Down A Sump Line
    */
    if (descentType == 1)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Maximum Descent Down A Sump Line");
        *nextPnt1P = descentPnt1;
        *nextXP = pointAddrP(dtmP, descentPnt1)->x;
        *nextYP = pointAddrP(dtmP, descentPnt1)->y;
        *nextZP = pointAddrP(dtmP, descentPnt1)->z;
        *processP = true;
        }
    /*
    **  Maximum Descent Is Down A Triangle Face
    */
    if (descentType == 2)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Maximum Descent Down A Triangle Face");
        if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, exitPnt, descentPnt1, descentPnt2, descentAngle, nextXP, nextYP, nextZP, &intPnt)) goto errexit;
        if (dbg) bcdtmWrite_message(0, 0, 0, "intPnt = %9ld ** X = %12.5lf Y = %12.5lf Z = %10.4lf ** intAngle = %12.10lf", intPnt, *nextXP, *nextYP, *nextZP, bcdtmMath_getAngle(startX, startY, *nextXP, *nextYP));
        if (intPnt != dtmP->nullPnt) *nextPnt1P = intPnt;
        else
            {
            *nextPnt1P = descentPnt1;
            *nextPnt2P = descentPnt2;
            if ((*nextPnt3P = bcdtmList_nextAntDtmObject(dtmP, descentPnt1, descentPnt2)) < 0) goto errexit;
            }
        *processP = true;
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Tracing Maximum Descent From Triangle Point Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Tracing Maximum Descent From Triangle Point Error");
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
            if (pond->GetDepth() > m_tracer.GetMinimumDepth())
                {
                return;
                }
            }
        }

    m_calculated = true;

    long hullPoint = 0;
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_exitPnt, &hullPoint))
        {
        SetError(); return;
        }

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
    // Todo rewrite.
    if (bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject_IncludeZSlope(dtmP, nullptr, m_priorPnt, m_exitPnt, m_nextPnt, m_exitPt.x, m_exitPt.y, &nextPnt1, &nextPnt2, &nextPnt3, &nextPt.x, &nextPt.y, &nextPt.z, &process))
        {
        SetError(); return;
        }

    if (!process)
        return;

    m_points.push_back(nextPt);

    m_numExitFlows++;
    // Set Parameters For Next Maximum Descent Trace
    double lastAngle = bcdtmMath_getAngle(m_exitPt.x, m_exitPt.y, nextPt.x, nextPt.y);
    if (nextPnt2 == dtmP->nullPnt)  // Point
        {
        if (nextPt.z == m_exitPt.z)
            {
            // Check next and previous points 
            long pnt1 = bcdtmList_nextClkDtmObject(dtmP, m_exitPnt, nextPnt1);
            long pnt2 = bcdtmList_nextAntDtmObject(dtmP, m_exitPnt, nextPnt1);
            if (pointAddrP(dtmP, pnt1)->z > m_exitPt.z  && pointAddrP(dtmP, pnt2)->z > m_exitPt.z)
                {
                auto existingPond = m_tracer.FindPondLowPt(m_exitPnt);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    }
                else
                    {
                    auto child = TracePondEdge::Create(m_tracer, *this, m_exitPnt, nextPnt1, m_exitPt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                return;
                }
            }
        auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, nextPnt1, nextPt, lastAngle);
        m_children.push_back(child);
        }
    else
        {
        if (nextPt.z == pointAddrP(dtmP, nextPnt2)->z)
            {
            // Check next and previous points 
            long pnt1 = bcdtmList_nextClkDtmObject(dtmP, nextPnt1, nextPnt2);
            long pnt2 = bcdtmList_nextAntDtmObject(dtmP, nextPnt1, nextPnt2);
            if (pointAddrP(dtmP, pnt1)->z > nextPt.z  && pointAddrP(dtmP, pnt2)->z > nextPt.z)
                {
                auto existingPond = m_tracer.FindPondLowPt(nextPnt1);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    }
                else
                    {
                    auto child = TracePondEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                return;
                }
            }


        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, lastAngle);
        newFeatures.push_back(child);
        m_children.push_back(child);
        }
    }


//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
DrainageTracer::~DrainageTracer()
    {
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
int DrainageTracer::DoTrace(DPoint3dCR startPt)
    {
    auto startFeature = TraceStartPoint::Create(*this, startPt);
    m_features.push_back(startFeature);
    bvector<TraceFeaturePtr> featuresToProcess;
    featuresToProcess.push_back(startFeature);
    AddAndProcessFeatures(featuresToProcess);
    return DTM_SUCCESS;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
int DrainageTracer::AddWaterVolume(DPoint3dCR startPt, double volume)
    {
    m_forWater = true;
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

        newFeatures.clear();
        for (auto&& feature : features)
            {
            feature.feature->ProcessWaterVolume(feature.vol, newFeatures);
            }
        }
    return DTM_SUCCESS;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void DrainageTracer::DoTraceCallback(DTMFeatureCallback loadFunction, void* userArg)
    {
    static bool showHidden = false;
    for (const auto& feature : m_features)
        {
        bool hasWater = ForWater();

        if (showHidden || !feature->IsHidden() || hasWater)
            feature->DoTraceCallback(hasWater, loadFunction, userArg);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void DrainageTracer::AddAndProcessFeatures(bvector<TraceFeaturePtr>& featuresToAdd)
    {
    bool haveProcessedAFeature = false;
    bvector<TraceFeature*> featuresToProcess;

    for (auto&& feature : featuresToAdd)
        {
        m_features.push_back(feature.get());
        featuresToProcess.push_back(feature.get());
        }

    while (!featuresToProcess.empty())
        {
        auto& feature = featuresToProcess.back();
        bvector<TraceFeaturePtr> newFeatures;
        if (!feature->IsFinished())
            feature->Process(newFeatures);

        if (feature->IsFinished())
            featuresToProcess.pop_back();

        if (!newFeatures.empty())
            {
            for (auto&& newFeature : newFeatures)
                featuresToProcess.push_back(newFeature.get());
            m_features.insert(m_features.end(), newFeatures.begin(), newFeatures.end());
            newFeatures.clear();
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TracePondExit* DrainageTracer::FindPondExit(long exitPoint)
    {
    auto it = m_pondExits.find(exitPoint);
    if (it == m_pondExits.end())
        return nullptr;
    return it->second;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void DrainageTracer::AddPondExit(TracePondExit& pondexit)
    {
    m_pondExits[pondexit.GetExitPoint()] = &pondexit;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void DrainageTracer::AddOnPoint(long pointNum, TraceOnPoint& point)
    {
    m_onPointFeatures[pointNum] = &point;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnPoint* DrainageTracer::FindExistingOnPoint(long pointNum)
    {
    auto it = m_onPointFeatures.find(pointNum);
    if (it != m_onPointFeatures.end())
        return it->second;
    return nullptr;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void DrainageTracer::AddPondLowPond(long pointNum, TracePond& pond)
    {
    m_pondlowPts[pointNum] = &pond;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TracePond* DrainageTracer::FindPondLowPt(long pointNum)
    {
    auto it = m_pondlowPts.find(pointNum);
    if (it != m_pondlowPts.end())
        return it->second;
    return nullptr;
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
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
DrainageTracerPtr DrainageTracer::Clone()
    {
    return new DrainageTracer(*this);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
DrainageTracer::DrainageTracer(DrainageTracerCR from) : m_dtm(from.m_dtm), m_dtmObj(from.m_dtm.GetTinHandle())
    {
    bmap<TraceFeatureCP, TraceFeatureP> featureRemapTable;

    m_pointList = from.m_pointList;
    m_ascentTrace = from.m_ascentTrace;
    m_zeroSlopeOption = from.m_zeroSlopeOption;
    m_falseLowDepth = from.m_falseLowDepth;
    m_forWater = from.m_forWater;
    m_pondElevationTolerance = from.m_pondElevationTolerance;
    m_pondVolumeTolerance = from.m_pondVolumeTolerance;

    for (auto&& fromFeature : from.m_features)
        {
        TraceFeaturePtr newfeature = fromFeature->Clone(*this);
        featureRemapTable[fromFeature.get()] = newfeature.get();
        m_features.push_back(newfeature);
        }

    for (auto&& feature : m_features)
        feature->RemapFeatures(featureRemapTable);

    for (auto it : from.m_onPointFeatures)
        m_onPointFeatures[it.first] = dynamic_cast<TraceOnPoint*>(featureRemapTable[it.second]);

    for (auto it : from.m_pondlowPts)
        m_pondlowPts[it.first] = dynamic_cast<TracePond*>(featureRemapTable[it.second]);

    //for (auto it : from.m_ponds)
    //    m_ponds.push_back(dynamic_cast<TracePond*>(featureRemapTable[it]));

    for (auto it : from.m_pondExits)
        m_pondExits[it.first] = dynamic_cast<TracePondExit*>(featureRemapTable[it.second]);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
DrainageTracerPtr DrainageTracer::Create(BcDTMR dtm)
    {
    return new DrainageTracer(dtm);
    }

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
    //startX = 773307.51124000002;
    //startY = 1785829.5027520000;

    //startX = 773308.56915500003;
    //startY = 1785700.5027520000;
    //startX = 773116.12396525347;
    //startY = 1785563.6789070573;

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
        DrainageTracerPtr tracer = DrainageTracer::Create(*dtm);

        //tracer->m_falseLowDepth = 0;
        //tracer->AddWaterVolume(DPoint3d::From(startX, startY, 0), falseLowDepth);

        tracer->SetMinimumDepth(falseLowDepth);
        tracer->DoTrace(DPoint3d::From(startX, startY, 0));
        QuickFeatureJoiner joiner(loadFunctionP, userP);
        tracer->DoTraceCallback(&QuickFeatureJoiner::callback, &joiner);

        }
    return DTM_SUCCESS;
    }



// ToDo
// 1. Add Inner pond analysis to the pond from exit analysis, this will copy the area and the inner ponds.
// 2. Expose tolerances.
// 3. Handle ponds in ponds, when they are reached, trace down and fill it up.
//  a. Send in the other ponds analysis.
//  b. Copy the area, and add the inner ponds.
END_BENTLEY_TERRAINMODEL_NAMESPACE
