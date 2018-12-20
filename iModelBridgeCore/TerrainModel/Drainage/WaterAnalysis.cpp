/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/WaterAnalysis.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel/Drainage/WaterAnalysis.h>
#include <TerrainModel/Core/TMTransformHelper.h>
#include "bcdtmDrainage.h"
#include <set>

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

struct Flow
    {
    long pnt1;
    long pnt2;
    DPoint3d pt;
    Flow()
        { }
    Flow(long pnt1, long pnt2, DPoint3d pt) : pnt1(pnt1), pnt2(pnt2), pt(pt)
        { }
    };
//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool IsOnHull(BC_DTM_OBJ* dtmP, long ptNum, bool checkVoidAndIslands)
    {
    bool value = nodeAddrP(dtmP, ptNum)->hPtr != dtmP->nullPnt;

    if (!value && checkVoidAndIslands)
        value = bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP, ptNum) != 0;
    return value;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool isLineOnVoidOrHole(BC_DTM_OBJ* dtmP, long pnt1, long pnt2)
    {
    if (bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP, pnt1, pnt2) != 0)
        return true;
    return bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP, pnt2, pnt1) != 0;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool isLineOnHullOrVoidOrHole(BC_DTM_OBJ* dtmP, long pnt1, long pnt2)
    {
    if (nodeAddrP(dtmP, pnt1)->hPtr == pnt2 || nodeAddrP(dtmP, pnt2)->hPtr == pnt1)
        return true;
    return isLineOnVoidOrHole(dtmP, pnt1, pnt2);
    }
//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
template<class T, class I> I eraseSwapAndPop(T& container, I it)
    {
    if (it + 1 == container.end())
        {
        container.pop_back();
        }
    else
        {
        std::swap(*it, container.back());
        container.pop_back();
        }
    return it;
    }

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
PondAnalysis::PondAnalysis(PondAnalysisCR from, WaterAnalysis& tracer) : m_tracer(tracer)
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
PondAnalysis::PondAnalysis(WaterAnalysis& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3) : m_tracer(tracer), m_lowPnt(lowPnt), m_lowZ(lowZ), m_type(type), m_pntNum2(ptNum2), m_pntNum3(ptNum3), m_pointList(tracer.GetPointList())
    {
    dtmP = m_tracer.GetDTM().GetTinHandle();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysisPtr PondAnalysis::Create(WaterAnalysis& tracer, long lowPnt, double lowZ, LowPointType type, long ptNum2, long ptNum3)
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
bvector<DPoint3d> PondAnalysis::GetPolygonAtElevation(double elevation, TPtrList& list)
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
            elevation = pointAddrP(dtmP, startInfo.m_exitPoints.front().exitPoint)->z;

        auto points = GetPolygonAtElevation(elevation, list);
        if (!points.empty())
            {
            if (startInfo.m_location == ExpandingPondInfo::Location::Outer)
                boundaries.insert(boundaries.begin(), points);
            else
                boundaries.push_back(points);
            }
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
            it = eraseSwapAndPop(currentEpi->m_exitPoints, it);
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
        RemoveEmptyStartPoints();
    if (hasSplit)
        FindOuterBoundary();
    return hasExpanded;
    }

//----------------------------------------------------------------------------------------*
// @bsistruct                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void PondAnalysis::RemoveEmptyStartPoints()
    {
    for (auto it = m_startPoints.begin(); it != m_startPoints.end();)
        {
        if (!it->tPtr.empty())
            {
            it++;
            }
        else
            it = eraseSwapAndPop(m_startPoints, it);
        }
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
        if (!IsOnHull(dtmP, *np, m_analysis.DTMHasVoids()))
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

                    if (!isExit && !isSump)
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
    if (IsOnHull(dtmP, pnt, DTMHasVoids()))
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
        BeAssert(np != -99);
        if (np == -99)
            {
            SetError();
            return;
            }
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

    RemoveZSlope(PondAnalysisR analysis, bool doSumps) : m_analysis(analysis), m_doSumps(doSumps)
        {
        }

    bool func(BC_DTM_OBJ* dtmP, bvector<long>::iterator sp, bvector<long>::iterator np, TPtrList& list, double testZ)
        {
        // If this is not on the hull.
        const double npZ = pointAddrP(dtmP, *np)->z;

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
            bool isOnHull = isLineOnHullOrVoidOrHole(dtmP, *sp, *np); // , m_analysis.DTMHasVoids()); //  IsOnHull(dtmP, *np, m_analysis.DTMHasVoids());

            if (!isOnHull)
                {
                const double spZ = pointAddrP(dtmP, *sp)->z;
                // Is it a flat triangle @ testZ
                if (spZ == testZ)
                    {
                    long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
                    BeAssert(outerPt != -99);
                    if (outerPt == -99)
                        return false;

                    const double outerZ = pointAddrP(dtmP, outerPt)->z;
                    if (outerZ == testZ)
                        return true;
                    }

                bool isExit = false;
                bool isSump = false;
                m_analysis.IsSumpOrExit(isSump, isExit, sp, np, list);

                if (isExit)
                    return false;

                if (isSump && m_doSumps)
                    return true;
                //if (isOnHull || isExit)
                //    {
                //    isExit = true;
                //    }
                //else
                //    {
                //    if (*sp == *np)
                //        removePoint = true;
                //    else
                //        {
                //        long outerPt = bcdtmList_nextClkDtmObject(dtmP, *sp, *np);
                //        double outerZ = pointAddrP(dtmP, outerPt)->z;
                //        if (outerZ == testZ)
                //            removePoint = true;
                //        else
                //            {
                //            isExit = isSump;
                //            }
                //        }
                //    }
                }
            }
        return false;
        }
    };

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bool PondAnalysis::GetExitPoints(double testZ)
    {
    RemoveZSlope helper(*this, testZ == m_lowZ);

    bool ret = false;
    //if (testZ == m_lowZ)
        ret = ExpandPolygon(helper, testZ);

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
            bool isOnHull = IsOnHull(dtmP, *np, DTMHasVoids());
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
                        isExit = isSump;
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

        if (lowPtNum != -1 && (outLowPtNum == -1 || outLowZ > lowZ))
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

            if (pointAddrP(dtmP, np)->z <= testZ && !IsOnHull(dtmP, np, DTMHasVoids()))
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
        if (!pts.empty())
            {
            m_startPoints.push_back(ExpandingPondInfo(TPtrList(m_pointList, pts.begin(), pts.end()), m_nextIndex++));
            m_startPoints.back().m_location = ExpandingPondInfo::Location::Unknown;
            }
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
            for (auto pnt : epi.tPtr)
                TestLowPoint(pnt);

            TestLowPoint(m_lowPnt);
            break;
            }
        case LowPointType::Edge:
            {
            long antPnt = bcdtmList_nextAntDtmObject(dtmP, m_lowPnt, m_pntNum2);
            long clkPnt = bcdtmList_nextClkDtmObject(dtmP, m_lowPnt, m_pntNum2);

            if (nodeAddrP(dtmP, m_lowPnt)->hPtr == m_pntNum2)
                {
                nodeAddrP(dtmP, m_lowPnt)->tPtr = m_pntNum2;
                }
            else
                {
                nodeAddrP(dtmP, m_lowPnt)->tPtr = clkPnt;
                nodeAddrP(dtmP, clkPnt)->tPtr = m_pntNum2;
                }
            if (nodeAddrP(dtmP, m_pntNum2)->hPtr == m_lowPnt)
                {
                nodeAddrP(dtmP, m_pntNum2)->tPtr = m_lowPnt;
                }
            else
                {
                nodeAddrP(dtmP, m_pntNum2)->tPtr = antPnt;
                nodeAddrP(dtmP, antPnt)->tPtr = m_lowPnt;
                }

            AddTriangleToValues(m_lowPnt, m_pntNum2, antPnt, m_lowZ);
            AddTriangleToValues(m_lowPnt, m_pntNum2, clkPnt, m_lowZ);

            long firstPoint = m_lowPnt;
//            m_isZSlope = FixLowPts(firstPoint, m_lowZ);
            SetCurrentIndex(m_nextIndex);
            ExpandingPondInfo epi(TPtrList(m_pointList, m_lowPnt, dtmP), m_nextIndex++);
            epi.m_location = ExpandingPondInfo::Location::Outer;
            for (auto pnt : epi.tPtr)
                TestLowPoint(pnt);
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
            //m_isZSlope = FixLowPts(firstPoint, m_lowZ);
            SetCurrentIndex(m_nextIndex);
            ExpandingPondInfo epi(TPtrList(m_pointList, firstPoint, dtmP), m_nextIndex++);
            for (auto pnt : epi.tPtr)
                TestLowPoint(pnt);
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
    long np=0;
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
            {
            z = minZ + ((maxZ - minZ) / 2);
            break;
            }

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

        if (firstTime && m_currentZ < m_lowZ)
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
        m_currentVolumePoints = GetPolygonAtElevation();
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

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmFind_hullIntersectionDtmObject2(BC_DTM_OBJ *dtmP,long *intFoundP,long point,double x,double y,long *pnt1P,long *pnt2P,long *pnt3P )
    {
    /*
    ** Do Not Fucking Change This Function
    */
    int    ret=DTM_SUCCESS,sd1,sd2 ;
    long   fp,p1,p2,isw=1 ;
    double xInt,yInt,dist,dd  ;
    double xln,xlm,yln,ylm,xhn,xhm,yhn,yhm  ;
    /*
    ** Initialise Variables
    */
    *intFoundP = 0 ;
    p1 = fp = dtmP->hullPoint ;
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    dist = bcdtmMath_distance(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,x,y) ;
    if( pointAddrP(dtmP,point)->x <= x ) { xln = pointAddrP(dtmP,point)->x ; xlm = x ; }
    else                                 { xlm = pointAddrP(dtmP,point)->x ; xln = x ; }
    if( pointAddrP(dtmP,point)->y <= y ) { yln = pointAddrP(dtmP,point)->y ; ylm = y ; }
    else                                 { ylm = pointAddrP(dtmP,point)->y ; yln = y ; }
    /*
    ** Scan Convex Hull
    */
    do
        {
        if( p1 != point && p2 != point )
            {
            if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p2)->x ) { xhn = pointAddrP(dtmP,p1)->x ; xhm = pointAddrP(dtmP,p2)->x ; }
            else                                                   { xhm = pointAddrP(dtmP,p1)->x ; xhn = pointAddrP(dtmP,p2)->x ; }
            if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p2)->y ) { yhn = pointAddrP(dtmP,p1)->y ; yhm = pointAddrP(dtmP,p2)->y ; }
            else                                                   { yhm = pointAddrP(dtmP,p1)->y ; yhn = pointAddrP(dtmP,p2)->y ; }
            xhn = xhn - 0.0001 ; yhn = yhn - 0.0001 ;
            xhm = xhm + 0.0001 ; yhm = yhm + 0.0001 ;
            if( xln <= xhm && xlm >= xhn && yln <= yhm  && ylm >= yhn  )
                {
                sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y) ;
                sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
                if( sd1 != sd2 )
                    {
                    sd1 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
                    sd2 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
                    if( sd1 != sd2 )
                        {
                        bcdtmMath_normalIntersectCordLines(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&xInt,&yInt) ;
                        dd = bcdtmMath_distance(xInt,yInt,x,y) ;
                        if( isw || dd < dist )
                            {
                            *pnt1P = p1 ;
                            *pnt2P = p2 ;
                            if(( *pnt3P = bcdtmList_nextAntDtmObject(dtmP,p1,p2) ) < 0 ) goto errexit ;
                            *intFoundP = 1 ;
                            }
                        }
                    }
                }
            }
        p1 = p2 ; p2 = nodeAddrP(dtmP,p1)->hPtr ;
        } while ( p1 != fp ) ;
        /*
        ** Clean Up
        */
    cleanup :
        /*
        ** Job Completed
        */
        return(ret) ;
        /*
        ** Error Exit
        */
    errexit :
        if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
        goto cleanup ;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
bool IsOnLine(BC_DTM_OBJ* dtmP, DPoint3dCR p1, DPoint3dCR p2, double x, double y, double& dist)
    {
    long onLine;
    double nX, nY;
    dist = bcdtmMath_distanceOfPointFromLineSquared(&onLine,p1.x,p1.y,p2.x,p2.y,x, y ,&nX,&nY);
    return (dist < (dtmP->plTol * dtmP->plTol) && onLine);
    }

bool IsOnLine(BC_DTM_OBJ* dtmP, DPoint3dCR p1, DPoint3dCR p2, double x, double y)
    {
    double nD;
    return IsOnLine(dtmP, p1, p2, x, y, nD);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmFind_triangleForPointFromPointDtmObject2(BC_DTM_OBJ *dtmP,double x,double y,long closestPnt,long *pntTypeP,long *pnt1P,long *pnt2P,long *pnt3P)
/*
** This Function Finds The Triangle Containing Point x,y From closestPnt
**
** *pntTypeP = 0  Point External To DTMFeatureState::Tin
**           = 1  Point In Triangle pnt1P-pnt2P-pnt3P
**
*/
    {
    int  ret=DTM_SUCCESS,sdof1,sdof2 ;
    long p0,p1,p2,p3,clc,hullIntFnd,scan=1 ;
    /*
    ** Initialise
    */
    *pntTypeP = 0 ;
    *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
    /*
    ** Scan Circular List about Closest Point and determine if p lies within a Triangle
    */
    p0  = closestPnt ;
    while ( scan )
        {
        p1  = p0 ;
        clc = nodeAddrP(dtmP,p1)->cPtr ;
        p3  = clistAddrP(dtmP,clc)->pntNum ;
        if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
        sdof1 = bcdtmMath_linePointSideOfDtmObject(dtmP,p1,p2,x,y) ;
        while ( clc != dtmP->nullPtr )
            {
            p3  = clistAddrP(dtmP,clc)->pntNum ;
            clc = clistAddrP(dtmP,clc)->nextPtr ;
            sdof2 = bcdtmMath_linePointSideOfDtmObject(dtmP,p1,p3,x,y) ;
            if( nodeAddrP(dtmP,p1)->hPtr != p2 )
                {
                if( sdof1 <= 0 && sdof2 >= 0 )
                    {
                    /*
                    **           Test For Point In Triangle
                    */
                    if( bcdtmMath_linePointSideOfDtmObject(dtmP,p2,p3,x,y) <= 0 )
                        {
                        if( x == pointAddrP(dtmP,p1)->x && y == pointAddrP(dtmP,p1)->y ) { *pnt1P = p1 ; *pntTypeP = 1 ; goto cleanup ; }
                        if( x == pointAddrP(dtmP,p2)->x && y == pointAddrP(dtmP,p2)->y ) { *pnt1P = p2 ; *pntTypeP = 1 ; goto cleanup ; }
                        if( x == pointAddrP(dtmP,p3)->x && y == pointAddrP(dtmP,p3)->y ) { *pnt1P = p3 ; *pntTypeP = 1 ; goto cleanup ; }
                        *pnt1P = p1 ;
                        *pnt2P = p2 ;
                        *pnt3P = p3 ;
                        *pntTypeP = 2 ;
                        scan = 0 ;
                        goto cleanup ;
                        }
                    /*
                    **           Test For Point Going External From Hull Line pnt3P-pnt2P
                    */
                    if( nodeAddrP(dtmP,p3)->hPtr == p2 )
                        {
                        if( bcdtmFind_hullIntersectionDtmObject2(dtmP,&hullIntFnd,p1,x,y,&p0,&p2,&p3) ) goto errexit ;
                        if( ! hullIntFnd ) goto cleanup ;
                        }
                    /*
                    **           Get Next Point To Scan
                    */
                    else if(( p0 = bcdtmList_nextClkDtmObject(dtmP,p3,p2)) < 0 ) goto errexit ;
                    /*
                    **           Stop Scan Around Current Point
                    */
                    clc = dtmP->nullPtr ;
                    }
                }
            if (nodeAddrP(dtmP, p1)->hPtr == p2 || nodeAddrP(dtmP, p2)->hPtr == p1)
                {
                // Check if the point is on the hull line.
                if (IsOnLine(dtmP, *pointAddrP(dtmP,p1), *pointAddrP(dtmP,p2), x, y))
                    {
                    if (nodeAddrP(dtmP, p1)->hPtr == p2)
                        {
                        *pnt1P = p1;
                        *pnt2P = p2;
                        *pnt3P = bcdtmList_nextAntDtmObject(dtmP, p1, p2);
                        }
                    else
                        {
                        *pnt1P = p2;
                        *pnt2P = p1;
                        *pnt3P = bcdtmList_nextAntDtmObject(dtmP, p2, p1);
                        }
                    *pntTypeP = 2;
                    scan = 0;
                    goto cleanup;
                    }
                }
            p2 = p3 ;
            sdof1 = sdof2 ;
            }
        /*
        **  Test For Line Goining External From Hull Point pnt1P
        */
        if( p1 == p0 )
            {
            if( bcdtmFind_hullIntersectionDtmObject2(dtmP,&hullIntFnd,p1,x,y,&p0,&p2,&p3) ) goto errexit ;
            if( ! hullIntFnd ) goto cleanup ;
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

int bcdtmFind_triangleDtmObject2(BC_DTM_OBJ *dtmP,double x,double y,long *fndTypeP,long *pnt1P,long *pnt2P,long *pnt3P)
/*
** This routine finds the triangle the point x,y lies in
**
** Find Type ( fndTypeP ) Return Values
**
**   0 - Data Point Outside Data Set Area
**   1 - Point on Triangle Vertex p1
**   2 - Triangle Found vertices are p1,p2,p3
*/
    {
    int   ret=DTM_SUCCESS;
    long  closestPnt  ;
    /*
    ** Initialise
    */
    *fndTypeP = 0 ;
    *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;

    /*
    ** Check Dtm In Tin State
    */
    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated") ;
        goto errexit ;
        }
    /*
    ** Check If Point Is External To Minimum Bounding Rectangle Of DTMFeatureState::Tin
    */
    if( x >= dtmP->xMin - 1.0  && x <= dtmP->xMax + 1.0 && y >= dtmP->yMin - 1.0  && y <= dtmP->yMax + 1.0 )
        {
        /*
        **  Find Closest Tin Point to p(x,y)
        */
        if( bcdtmFind_closestPointDtmObject(dtmP,x,y,&closestPnt) == 1 )
            {
            *pnt1P = closestPnt ;
            *fndTypeP = 1 ;
            }
        /*
        ** Scan Tin Structure For Point
        */
        else if( bcdtmFind_triangleForPointFromPointDtmObject2(dtmP,x,y,closestPnt,fndTypeP,pnt1P,pnt2P,pnt3P)) goto errexit ;
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmFind_triangleForPointDtmObject2(BC_DTM_OBJ *dtmP,double x,double y,double *ZP,long *fndTypeP,long *pnt1P,long *pnt2P,long *pnt3P )
/*
**
** Note :- fndTypeP ( Point Find Type ) Return Values
**
**  == 0   Point External To Dtm
**  == 1   Point Coincident with Point pnt1P
**  == 2   Point On Line pnt1-Ppnt2P
**  == 3   Point On Hull Line pnt1P-pnt2P
**  == 4   Point In Triangle pnt1P-pnt2P-pnt3P
**
*/
    {
    int  ret=DTM_SUCCESS ;
    long p ;
    /*
    ** Initialise Variables
    */
    *ZP       = 0.0 ;
    *fndTypeP = 0   ;
    *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
    /*
    ** Find Triangle
    */
    if( bcdtmFind_triangleDtmObject2(dtmP,x,y,fndTypeP,pnt1P,pnt2P,pnt3P) ) goto errexit ;
    /*
    ** If Point Inside Tin Hull Interpolate Point
    */
    if( *fndTypeP  )
        {
        /*
        **  Point Coincident With Existing dtmP Point
        */
        if( *fndTypeP == 1 ) *ZP = pointAddrP(dtmP,*pnt1P)->z ;
        else
            {
            /*
            **     Set Find Type To Triangle
            */
            *fndTypeP = 4 ;
            /*
            **     Set Points Clockwise
            */
            if( bcdtmMath_pointSideOfDtmObject(dtmP,*pnt1P,*pnt2P,*pnt3P) > 0 ) { p = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = p ; }
            /*
            **     Test If Point On Tin Line
            */
            double d12, d23, d31;
            bool pIsOn12 = IsOnLine(dtmP, *pointAddrP(dtmP, *pnt1P), *pointAddrP(dtmP, *pnt2P), x, y,d12);
            bool pIsOn23 = IsOnLine(dtmP, *pointAddrP(dtmP, *pnt2P), *pointAddrP(dtmP, *pnt3P), x, y,d23);
            bool pIsOn31 = IsOnLine(dtmP, *pointAddrP(dtmP, *pnt3P), *pointAddrP(dtmP, *pnt1P), x, y,d31);

            if (pIsOn12 && pIsOn23)
                {
                if (d12 < d23)
                    pIsOn23 = false;
                else
                    pIsOn12 = false;
                }
            if (pIsOn23 && pIsOn31)
                {
                if (d23 < d31)
                    pIsOn31 = false;
                else
                    pIsOn23 = false;
                }
            if (pIsOn31 && pIsOn12)
                {
                if (d31 < d12)
                    pIsOn12 = false;
                else
                    pIsOn31 = false;
                }
            if     ( pIsOn12) { *fndTypeP = 2 ; *pnt3P = dtmP->nullPnt ; }
            else if( pIsOn23) { *fndTypeP = 2 ; *pnt1P = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = dtmP->nullPnt ; }
            else if( pIsOn31) { *fndTypeP = 2 ; *pnt2P = *pnt3P ; *pnt3P = dtmP->nullPnt ; }
            /*
            **     Test If Point On Hull Line
            */
            if( *fndTypeP == 2 )
                {
                if      ( nodeAddrP(dtmP,*pnt1P)->hPtr == *pnt2P )    *fndTypeP = 3 ;
                else if ( nodeAddrP(dtmP,*pnt2P)->hPtr == *pnt1P )  { *fndTypeP = 3 ;p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = p ; }
                }
            /*
            **     Set Lowest Point Number First
            */
            if( *fndTypeP == 4 ) while ( *pnt1P > *pnt2P || *pnt1P > *pnt3P ) { p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = p ; }
            if( *fndTypeP == 2 && *pnt1P > *pnt2P )  { p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = p ; }
            /*
            **     Interpolate Point
            */
            if( *fndTypeP  < 4 ) bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,ZP,*pnt1P,*pnt2P) ;
            if( *fndTypeP == 4 ) bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,x,y,ZP,*pnt1P,*pnt2P,*pnt3P) ;
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Return
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceStartPoint::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_startPoint, 1, args);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceStartPoint::AddResult(WaterAnalysisResultR result) const
    {
    result.AddPoint(m_startPoint, WaterAnalysisResult::PointType::Start, CurrentVolume());
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

    if (bcdtmFind_triangleForPointDtmObject2(dtmP, m_startPoint.x, m_startPoint.y, &z, &pointType, &trgPnt1, &trgPnt2, &trgPnt3))
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
        if (m_tracer.DTMHasVoids() && bcdtmList_testForVoidLineDtmObject(dtmP, trgPnt1, trgPnt2, pntInVoid))
            {
            SetError();
            return;
            }
        }
    if (pointType == 4)
        {
        if (m_tracer.DTMHasVoids() && bcdtmList_testForVoidTriangleDtmObject(dtmP, trgPnt1, trgPnt2, trgPnt3, pntInVoid))
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
            m_children.push_back(child);
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
        if (trgPnt3 >= 0)
            {
            if (/*pointAddrP(dtmP, trgPnt3)->z < z &&*/ bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
                {
                bool isVoid = m_tracer.DTMHasVoids() && bcdtmList_testForVoidTriangleDtmObject(dtmP, trgPnt1, trgPnt2, trgPnt3);
                if (!isVoid)
                    {
                    auto child = TraceOnEdge::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                }
            }

        std::swap(trgPnt1, trgPnt2);
        trgPnt3 = bcdtmList_nextAntDtmObject(dtmP, trgPnt1, trgPnt2);
        if (trgPnt3 >= 0)
            {
            if (/*pointAddrP(dtmP, trgPnt3)->z < z &&*/ bcdtmList_testLineDtmObject(dtmP, trgPnt3, trgPnt2))
                {
                bool isVoid = m_tracer.DTMHasVoids() && bcdtmList_testForVoidTriangleDtmObject(dtmP, trgPnt1, trgPnt2, trgPnt3);
                if (!isVoid)
                    {

                    auto child = TraceOnEdge::Create(m_tracer, *this, trgPnt1, trgPnt2, trgPnt3, DPoint3d::FromXYZ(m_startPoint.x, m_startPoint.y, z));
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    }
                }
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceInTriangle::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceInTriangle::AddResult(WaterAnalysisResultR result) const
    {
    result.AddStream(m_points, CurrentVolume());
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
        CreateAndAddEdge(newFeatures, nextPnt1, nextPnt2, nextPt, angle);
#ifdef OLD
        bool isOnHull = nodeAddrP(dtmP, nextPnt1)->hPtr == nextPnt2 || nodeAddrP(dtmP, nextPnt2)->hPtr == nextPnt1;
        bool voidLine = false;

        if (!isOnHull && m_tracer.DTMHasVoids())
            bcdtmList_testForVoidLineDtmObject(dtmP, nextPnt1, nextPnt2, voidLine);

        if (!voidLine && isOnHull) // Ignore if the edge is on the hull
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
            }

        auto child = TraceOnEdge::Create(m_tracer, *this, nextPnt1, nextPnt2, nextPnt3, nextPt, angle);
        newFeatures.push_back(child);
        m_children.push_back(child);
#endif
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnEdge::TraceOnEdge(WaterAnalysis& tracer, TraceFeature& parent, long P1, long P2, long P3, DPoint3dCR startPt, double lastAngle) : TraceFeature(tracer, &parent), pnt1(P1), pnt2(P2), pnt3(P3), m_pt(startPt), m_startPt(startPt), lastAngle(lastAngle)
    {
    if (pnt3 == -99)
        pnt3 = -99;
    m_points.push_back(startPt);

#ifdef DEBUG_CHK
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    if (nodeAddrP(dtmP, P1)->hPtr != P2 && nodeAddrP(dtmP, P2)->hPtr != P1) // Ignore if the edge is on the hull
        {
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
        }
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    loadFunction(DTMFeatureType::DescentTrace, 0, 0, m_points.data(), m_points.size(), args);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::AddResult(WaterAnalysisResultR result) const
    {
    result.AddStream(m_points, CurrentVolume());
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
            if (nodeAddrP(dtmP, pnt2)->hPtr == pnt1)
                {
                // This flows along the edge, so fall of the edge.
                return;
                }
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, pnt1, nextPt, lastAngle);
            m_children.push_back(child);
            return;
            }

        bool hullPoint = isLineOnHullOrVoidOrHole(dtmP, pnt1, pnt2);
        //if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, nextPnt3, &hullPoint))
        //    {
        //    SetError(); return;
        //    }

        // Flows of the edge of the triangulation.
        if (hullPoint)
            {
            m_finished = true;
            return;
            }

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
        bool isOnHull = nodeAddrP(dtmP, pnt1)->hPtr == pnt2 || nodeAddrP(dtmP, pnt2)->hPtr == pnt1;
        bool voidLine = false;

        if (!isOnHull && m_tracer.DTMHasVoids())
            {
            voidLine = isLineOnVoidOrHole(dtmP, pnt1, pnt2);
            //long pnt3a = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
            //voidLine = bcdtmList_testForValidTriangleDtmObject(dtmP, pnt1, pnt2, pnt3a) != 0;
            }

        if (!isOnHull && !voidLine)
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
        }
    m_finished = true;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceFeatureP TraceFeature::CreateAndAddEdge(bvector<TraceFeaturePtr>& newFeatures, long pnt1, long pnt2, DPoint3dCR pt, double m_lastAngle)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    bool isOnHull = nodeAddrP(dtmP, pnt1)->hPtr == pnt2 || nodeAddrP(dtmP, pnt2)->hPtr == pnt1;
    bool voidLine = false;

    if (!isOnHull && m_tracer.DTMHasVoids())
        {
        voidLine = isLineOnVoidOrHole(dtmP, pnt1, pnt2);
        //long pnt3a = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);
        //voidLine = bcdtmList_testForValidTriangleDtmObject(dtmP, pnt1, pnt2, pnt3a) != 0;
        //bool voidLine2 = bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP, pnt1, pnt2) != 0;

        //if (voidLine != voidLine2)
        //    voidLine2 = voidLine;
        }

    if (!isOnHull && !voidLine)
        {
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
                    return existingPond;
                    }
                else
                    {
                    auto child = TracePondEdge::Create(m_tracer, *this, pnt1, pnt2, *pointAddrP(dtmP, pnt1));
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    return child.get();
                    }
                }
            }
        }

    long pnt3 = bcdtmList_nextAntDtmObject(dtmP, pnt1, pnt2);

    if (pnt3 == -99)
        return nullptr;

    auto child = TraceOnEdge::Create(m_tracer, *this, pnt1, pnt2, pnt3, pt, m_lastAngle);
    newFeatures.push_back(child);
    m_children.push_back(child);
    return child.get();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnEdge::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long nextPnt1 = 0, nextPnt2 = dtmP->nullPnt, nextPnt3 = dtmP->nullPnt;
    DPoint3d nextPt;
    if (pnt3 == dtmP->nullPnt)
        {
        // This flows of the edge of the triangulation.
        m_finished = true;
        return;
        }

    if (nodeAddrP(dtmP, pnt2)->hPtr == pnt1)
        {
        // This is on the edge
        m_finished = true;
        return;
        }

    if (m_tracer.DTMHasVoids() && bcdtmList_testForVoidTriangleDtmObject(dtmP, pnt1, pnt2, pnt3))
        {
        m_finished = true;
        return;
        }
    //if (m_tracer.DTMHasVoids() && (bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP, pnt1) != 0 && bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP, pnt2) != 0))
    //    {
    //    m_finished = true;
    //    return;
    //    }

    if (pointAddrP(dtmP, pnt1)->z == pointAddrP(dtmP, pnt2)->z  && pointAddrP(dtmP, pnt1)->z == pointAddrP(dtmP, pnt3)->z)
        {
        ProcessZSlopeTriangle(newFeatures);
        return;
        }

    bool voidTriangle = false;
    int flowDirection;
    if (bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP, nullptr, pnt1, pnt2, pnt3, m_tracer.DTMHasVoids(), voidTriangle, flowDirection))
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
    if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, nullptr, pnt1, pnt3, pnt2, m_tracer.DTMHasVoids(), voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS)
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

        if (nodeAddrP(dtmP, pnt2)->hPtr == pnt1)
            {
            // This flows along the edge, so fall of the edge.
            return;
            }

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
    // @bsimethod                                                    Daryl.Holmwood  06/17
    // +---------------+---------------+---------------+---------------+---------------+------*
    TraceOnPoint::TraceOnPoint(WaterAnalysis& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle, long prevPtNum) : TraceFeature(tracer, &parent), m_pt(startPoint), m_lastAngle(lastAngle), m_ptNum(startPtNum), m_prevPtNum(prevPtNum)
    {
    if (m_prevPtNum != -1)
        m_points.push_back(*pointAddrP(m_tracer.GetDTM().GetTinHandle(), m_prevPtNum));
    m_points.push_back(startPoint);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnPoint* TraceOnPoint::GetOrCreate(bvector<TraceFeaturePtr>& newFeatures, WaterAnalysis& tracer, TraceFeature& parent, long startPtNum, DPoint3dCR startPoint, double lastAngle, long prevPt)
    {
    TraceOnPoint* existingPoint = tracer.FindExistingOnPoint(startPtNum);

    if (nullptr != existingPoint)
        {
        return existingPoint;
        }
    TraceOnPointPtr child = new TraceOnPoint(tracer, parent, startPtNum, startPoint, lastAngle, prevPt);
    newFeatures.push_back(child);
    tracer.AddOnPoint(startPtNum, *child);
    return child.get();
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnPoint::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    if (m_points.size() != 1 && !m_onHullPoint)   // One point we can ignore.
        {
        DPoint3d pts[2];
        pts[0] = m_points[0];

        for (size_t i = 1; i < m_points.size(); i++)
            {
            pts[1] = m_points[i];
            loadFunction(DTMFeatureType::DescentTrace, 0, 0, pts, 2, args);
            }
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnPoint::AddResult(WaterAnalysisResultR result) const
    {
    if (m_points.size() != 1 && !m_onHullPoint)   // One point we can ignore.
        {
        bvector<DPoint3d> pts;
        pts.resize(2);
        pts[0] = m_points[0];

        for (size_t i = 1; i < m_points.size(); i++)
            {
            pts[1] = m_points[i];
            result.AddStream(pts, CurrentVolume());
            }
        }
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


    bool voidLine = false;
    if (m_tracer.DTMHasVoids())
        {
        isLineOnVoidOrHole(dtmP, descentPnt1, descentPnt2);
        }

    if (!voidLine)
        {
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
    }


void TraceFeature::FindFlows(bvector<Flow>& flows, long pnt, long priorPnt, long nextPnt, bool isFlatPond)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    long sp;
    bool voidTriangle;
    double slope;
    double descentAngle;
    double ascentAngle;
    double previousSlope = -1;
    long useP1 = dtmP->nullPnt;
    long useP2 = 0;
    double useAngle = 0;
    bool rememberSteepest = false;
    size_t currentChildrenNum = m_children.size();
    DPoint3d pt = *pointAddrP(dtmP, pnt);
    bool first = true;
    bool noEndPnt = (nextPnt == -1);
    flows.clear();

    // If nextPnt == -1 then get the next and prior points.
    if (nextPnt == -1)
        {
        auto clistP = clistAddrP(dtmP, nodeAddrP(dtmP, pnt)->cPtr);
        priorPnt = clistP->pntNum;
        nextPnt = priorPnt;

        if (nodeAddrP(dtmP, pnt)->hPtr != dtmP->nullPnt)
            {
            priorPnt = nodeAddrP(dtmP, pnt)->hPtr;
            nextPnt = bcdtmList_nextClkDtmObject(dtmP, pnt, priorPnt);
            noEndPnt = false;
            }
        }
    sp = priorPnt;

    if (priorPnt == -1)
        {
        SetError(L"Invalid FindFlow parameter");
        return;
        }

    double angleSpPnt = bcdtmMath_getPointAngleDtmObject(dtmP, pnt, sp);
    while (first || sp != nextPnt)
        {
        first = false;

        long np = bcdtmList_nextAntDtmObject(dtmP, pnt, sp);
        double angleNpPnt = bcdtmMath_getPointAngleDtmObject(dtmP, pnt, np);

        // If it is a valley/sump and not on the hull.
        if ((noEndPnt|| np != nextPnt) &&
            (
            (isFlatPond && pointAddrP(dtmP, np)->z <= pt.z)
                ||
                (!isFlatPond && pointAddrP(dtmP, np)->z <= pt.z)
                )
            )
            {

            long anp = bcdtmList_nextAntDtmObject(dtmP, pnt, np);
            if (m_tracer.DTMHasVoids() && (bcdtmList_testForVoidTriangleDtmObject(dtmP, pnt, sp, np) || bcdtmList_testForVoidTriangleDtmObject(dtmP, pnt, np, anp)))
                {
                // void so ignore
                }
            else
                {
                DTMFeatureType lineType;
                if (bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP, nullptr, pnt, np, sp, anp, &lineType))
                    {
                    SetError(); return;
                    }

                // Check for Sump
                if (lineType == DTMFeatureType::SumpLine)
                    flows.push_back(Flow(np, dtmP->nullPnt, *pointAddrP(dtmP, np)));
                }
            }
        if (np < 0)
            {
            SetError(); return;
            }
        if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, nullptr, pnt, sp, np, voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS)
            {
            SetError(); return;
            }

        if (!voidTriangle)
            {
            bool add = false;
            bool reset = false;
            double a1 = angleNpPnt;
            double a2 = m_tracer.AscentTrace() ? ascentAngle : descentAngle;
            double a3 = angleSpPnt;
            if (a1 < a3) a1 += DTM_2PYE;
            if (a2 < a3) a2 += DTM_2PYE;
            if (a2 <= a1 && a2 >= a3)
                {
                //if (slope > previousSlope || useP1 == dtmP->nullPnt)
                //    {
                    useP1 = np;
                    useP2 = sp;
                    useAngle = m_tracer.AscentTrace() ? ascentAngle : descentAngle;
                //    rememberSteepest = true;
                //    }
                //else if (slope < previousSlope)
                //    {
                //    add = useP1 != dtmP->nullPnt;
                //    }
                //if (!noEndPnt && np == nextPnt)
                //    add = rememberSteepest;
                    add = true;
                }
            else
                {
                add = rememberSteepest;
                reset = true;
                }
            previousSlope = slope;

            if (add)
                {
                rememberSteepest = false;
                long intPnt;
                DPoint3d nextPt;
                if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, pnt, useP1, useP2, useAngle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
                    {
                    SetError(); return;
                    }
                if (intPnt != dtmP->nullPnt)
                    flows.push_back(Flow(intPnt, dtmP->nullPnt, nextPt));
                else
                    flows.push_back(Flow(useP1, useP2, nextPt));
                }
            if (reset)
                useP1 = dtmP->nullPnt;
            }
        angleSpPnt = angleNpPnt;
        sp = np;
        } 
    if (rememberSteepest)
        {
        long intPnt;
        DPoint3d nextPt;
        if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, pnt, useP1, useP2, useAngle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
            {
            SetError(); return;
            }
        if (intPnt != dtmP->nullPnt)
            flows.push_back(Flow(intPnt, dtmP->nullPnt, nextPt));
        else
            flows.push_back(Flow(useP1, useP2, nextPt));
        }
    bool hasRealExit = false;
    for (auto& flow : flows)
        {
        if (flow.pt.z < pt.z)
            {
            hasRealExit = true;
            break;
            }
        }

    for (auto& flow : flows)
        {
        if (hasRealExit && flow.pt.z == pt.z)
            {
            flow.pnt1 = dtmP->nullPnt;
            continue;
            }

/*
        if (flow.pnt1 == dtmP->nullPnt)
            continue;

        //m_numExitFlows++;
        m_points.push_back(flow.pt); // ToDo - Need to change the draw to handle multiple exits.
        double lastAngle = bcdtmMath_getAngle(pt.x, pt.y, flow.pt.x, flow.pt.y);

        if (flow.pnt2 == dtmP->nullPnt)
            {
            if (flow.pt.z == pt.z)
                {
                // Check next and previous points
                bool isOnHull = nodeAddrP(dtmP, flow.pnt1)->hPtr == pnt || nodeAddrP(dtmP, pnt)->hPtr == flow.pnt1;
                bool voidLine = false;

                if (!isOnHull && m_tracer.DTMHasVoids())
                    {
                    voidLine = isLineOnVoidOrHole(dtmP, flow.pnt1, pnt);
                    }

                if (!isOnHull && !voidLine)
                    {
                    long pnt1 = bcdtmList_nextClkDtmObject(dtmP, pnt, flow.pnt1);
                    long pnt2 = bcdtmList_nextAntDtmObject(dtmP, pnt, flow.pnt1);

                    if (pointAddrP(dtmP, pnt1)->z > pt.z  && pointAddrP(dtmP, pnt2)->z > pt.z)
                        {
                        auto existingPond = m_tracer.FindPondLowPt(pnt);

                        if (existingPond != nullptr)
                            {
                            m_children.push_back(existingPond);
                            }
                        else
                            {
                            auto child = TracePondEdge::Create(m_tracer, *this, pnt, flow.pnt1, pt);
                            newFeatures.push_back(child);
                            m_children.push_back(child);
                            }
                        continue;
                        }
                    }
                }

            if (flow.pt.z == pt.z)
                {
                auto existingPond = m_tracer.FindPondLowPt(pnt);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    continue;
                    }
                // Check if this is a flat triangle.
                long oPt1 = bcdtmList_nextAntDtmObject(dtmP, pnt, flow.pnt1);
                if (pt.z == pointAddrP(dtmP, oPt1)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, pnt, oPt1, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                long oPt2 = bcdtmList_nextClkDtmObject(dtmP, pnt, flow.pnt1);
                if (pt.z == pointAddrP(dtmP, oPt2)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, oPt2, pnt, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                }
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, flow.pnt1, flow.pt, lastAngle);
            m_children.push_back(child);
            }
        else
            {
            CreateAndAddEdge(newFeatures, flow.pnt1, flow.pnt2, flow.pt, lastAngle);
            }
*/
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TraceOnPoint::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    m_finished = true;

    TracePondP pond = m_tracer.FindPondLowPt(m_ptNum);

    if (nullptr != pond)
        {
        m_children.push_back(pond);
        return;
        }

    bvector<Flow> flows;
    FindFlows(flows, m_ptNum);

    // Test if this is a low point, eg no flows.
    if (flows.empty())
        {
        if (!IsOnHull(dtmP, m_ptNum, m_tracer.DTMHasVoids()))
            {
            auto pond = TracePondLowPoint::Create(m_tracer, *this, m_ptNum, m_pt);
            newFeatures.push_back(pond);
            m_children.push_back(pond);
            }
        return;
        }

    long pnt = m_ptNum;
    for (auto&& flow : flows)
        {
        if (flow.pnt1 == dtmP->nullPnt)
            continue;

        long pnt1 = flow.pnt1;
        long pnt2 = flow.pnt2;

        m_points.push_back(flow.pt); // ToDo - Need to change the draw to handle multiple exits.
        double lastAngle = bcdtmMath_getAngle(m_pt.x, m_pt.y, flow.pt.x, flow.pt.y);

        bool flowsToPoint = pnt2 == dtmP->nullPnt;
        /*
                if (flowsToPoint)
                {
                    bool isFlat = m_pt.z == flow.pt.z;
                    // Check on Hull.
                    if (IsOnHull(dtmP, pnt1, m_tracer.DTMHasVoids()) && isFlat)// slope == 0) // ZeroSlope
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
                        continue;
                        }
                    else
                        {
                        if (isLineOnHullOrVoidOrHole(dtmP, m_ptNum, pnt1))
                            {
                            m_onHullPoint = true;
                            m_finished = true;
                            continue;
                            }

                        //CreateAndAddEdge(newFeatures, flow.pnt1, flow.pnt2, flow.pt, lastAngle);

                        auto child = GetOrCreate(newFeatures, m_tracer, *this, flow.pnt1, flow.pt, lastAngle, m_ptNum);
                        m_children.push_back(child);

                        //m_prevPtNum = m_ptNum;
                        //m_ptNum = pnt1;
                        //m_pt = nextPt;
                        //m_points.push_back(m_pt);
                        m_finished = true;
                        continue;
                        }
                    }
                double slope, descentAngle, ascentAngle;
                if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, nullptr, m_ptNum, pnt1, pnt2, voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS)
                    {
                    SetError(); return;
                    }

                if (slope == 0.0)
                    {
                    ProcessZSlope(newFeatures, pnt1, pnt2);
                    continue;
                    }

                long intPnt;
                DPoint3d nextPt;
                if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, m_ptNum, pnt1, pnt2, angle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
                    {
                    SetError();
                    return;
                    }

                m_points.push_back(nextPt);
                CreateAndAddEdge(newFeatures, pnt1, pnt2, flow.pt, angle);

                continue;;
                */


        if (flow.pnt2 == dtmP->nullPnt)
            {
            if (flow.pt.z == m_pt.z)
                {
                // Check next and previous points
                bool isOnHull = nodeAddrP(dtmP, flow.pnt1)->hPtr == pnt || nodeAddrP(dtmP, pnt)->hPtr == flow.pnt1;
                bool voidLine = false;

                if (!isOnHull && m_tracer.DTMHasVoids())
                    {
                    voidLine = isLineOnVoidOrHole(dtmP, flow.pnt1, pnt);
                    }

                if (!isOnHull && !voidLine)
                    {
                    long pnt1 = bcdtmList_nextClkDtmObject(dtmP, pnt, flow.pnt1);
                    long pnt2 = bcdtmList_nextAntDtmObject(dtmP, pnt, flow.pnt1);

                    if (pointAddrP(dtmP, pnt1)->z > m_pt.z  && pointAddrP(dtmP, pnt2)->z > m_pt.z)
                        {
                        auto existingPond = m_tracer.FindPondLowPt(pnt);

                        if (existingPond != nullptr)
                            {
                            m_children.push_back(existingPond);
                            }
                        else
                            {
                            auto child = TracePondEdge::Create(m_tracer, *this, pnt, flow.pnt1, m_pt);
                            newFeatures.push_back(child);
                            m_children.push_back(child);
                            }
                        continue;
                        }
                    }
                }

            if (flow.pt.z == m_pt.z)
                {
                auto existingPond = m_tracer.FindPondLowPt(pnt);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    continue;
                    }
                // Check if this is a flat triangle.
                long oPt1 = bcdtmList_nextAntDtmObject(dtmP, pnt, flow.pnt1);
                if (m_pt.z == pointAddrP(dtmP, oPt1)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, pnt, oPt1, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                long oPt2 = bcdtmList_nextClkDtmObject(dtmP, pnt, flow.pnt1);
                if (m_pt.z == pointAddrP(dtmP, oPt2)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, oPt2, pnt, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                }
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, flow.pnt1, flow.pt, lastAngle);
            m_children.push_back(child);
            }
        else
            {
            CreateAndAddEdge(newFeatures, flow.pnt1, flow.pnt2, flow.pt, lastAngle);
            }

        }
#ifdef OLD

    // 
    if (m_tracer.AscentTrace())
        {
        long pnt1, pnt2;
        long type;
        double slope, angle;
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
        // Check on Hull.
        if (IsOnHull(dtmP, pnt1, m_tracer.DTMHasVoids()) && slope == 0) // ZeroSlope
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
            return;
            }
        else
            {
            DPoint3d nextPt = *pointAddrP(dtmP, pnt1);
            double lastAngle = bcdtmMath_getAngle(m_pt.x, m_pt.y, nextPt.x, nextPt.y);

            if (isLineOnHullOrVoidOrHole(dtmP, m_ptNum, pnt1))
                {
                m_onHullPoint = true;
                m_finished = true;
                return;
                }

            auto child = GetOrCreate(newFeatures, m_tracer, *this, pnt1, nextPt, lastAngle, m_ptNum);
            m_children.push_back(child);

            //m_prevPtNum = m_ptNum;
            //m_ptNum = pnt1;
            //m_pt = nextPt;
            //m_points.push_back(m_pt);
            m_finished = true;
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

        long intPnt;
        DPoint3d nextPt;
        if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, m_ptNum, pnt1, pnt2, angle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
            {
            SetError();
            return;
            }

        m_points.push_back(nextPt);
        CreateAndAddEdge(newFeatures, pnt1, pnt2, nextPt, angle);

        return;
        }
    else
        SetError(L"Unknown type");
#endif
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
TracePondEdge::TracePondEdge(WaterAnalysis& tracer, TraceFeature& parent, long ptNum1, long ptNum2, DPoint3dCR pt) : TracePond(tracer, parent, pt, ptNum1), m_ptNum1(ptNum1), m_ptNum2(ptNum2)
    {
    if (ptNum1 == 26362 && ptNum2 == 26618)
        ptNum1 = ptNum1;
    #ifdef DEBUG_CHK
    auto dtmP = tracer.GetDTM().GetTinHandle();
    if (nodeAddrP(dtmP, ptNum1)->hPtr == ptNum2 || nodeAddrP(dtmP, ptNum2)->hPtr == ptNum1)
        BeAssert(false);
#endif
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
TracePondEdge::TracePondEdge(TracePondEdgeCR from, WaterAnalysis& newTracer) : TracePond(from, newTracer)
    {
    m_ptNum1 = from.m_ptNum1;
    m_ptNum2 = from.m_ptNum2;
    m_sumpLines = from.m_sumpLines;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondEdge::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
    {
    if (!m_points.empty())
        TracePond::DoTraceCallback(waterCallback, loadFunction, args);

    for (auto&& sump : m_sumpLines)
        loadFunction(DTMFeatureType::SumpLine, 0, 0, sump.data(), (long)sump.size(), args);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondEdge::AddResult(WaterAnalysisResultR result) const
    {
    if (!m_points.empty())
        TracePond::AddResult(result);

    for (auto&& sump : m_sumpLines)
        result.AddStream(sump, CurrentVolume());
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
    //BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    //DtmSumpLinesPtr sumpLines;
    //if (bcdtmDrainage_concatenateZeroSlopeSumpLinesDtmObject(dtmP, m_ptNum, m_ptNum2, &sumpLines.ptr, &sumpLines.num))
    //    return;
    //else if (sumpLines.ptr != nullptr)
    //    {
    //    // Add Sumplines.
    //    bvector<DPoint3d> pts;
    //    long prevPtNum = -1;
    //    DTM_SUMP_LINES* sumpLine = sumpLines.ptr;
    //    for (int i = 0; i < sumpLines.num; i++, sumpLine++)
    //        {
    //        if (prevPtNum == sumpLine->sP2)
    //            std::swap(sumpLine->sP1, sumpLine->sP2);

    //        if (prevPtNum != sumpLine->sP1)
    //            {
    //            if (!pts.empty())
    //                {
    //                m_sumpLines.push_back(pts);
    //                pts.clear();
    //                }
    //            pts.push_back(*pointAddrP(dtmP, sumpLine->sP1));
    //            }
    //        prevPtNum = sumpLine->sP2;
    //        pts.push_back(*pointAddrP(dtmP, prevPtNum));
    //        }
    //    if (!pts.empty())
    //        m_sumpLines.push_back(pts);
    //    }

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
TracePond::TracePond(TracePondCR from, WaterAnalysis& newTracer) : TraceFeature(from, newTracer)
    {
    m_points = from.m_points;
    for (auto&& exit : from.m_exitPoints)
        m_exitPoints.push_back(ExitPointInfo(exit.ptNum, exit.exit));

    m_depth = from.m_depth;
    m_pt = from.m_pt;
    m_ptNum = from.m_ptNum;
    m_maxVolume = from.m_maxVolume;
    m_pondAnalysis = from.m_pondAnalysis->Clone(newTracer);
    m_allFull = from.m_allFull;
    m_topLevelPond = from.m_topLevelPond;
    }


//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<TraceFeatureP> TracePond::GetReferences() const
    {
    auto ret = TraceFeature::GetReferences();
    for (auto& exit : m_exitPoints)
        ret.push_back(exit.exit);
    ret.push_back(m_topLevelPond);
    return ret;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable)
    {
    TraceFeature::RemapFeatures(featureRemapTable);
    for (auto& exit : m_exitPoints)
        exit.exit = dynamic_cast<TracePondExitP>(featureRemapTable[exit.exit]);
    m_topLevelPond = dynamic_cast<TracePondP>(featureRemapTable[m_topLevelPond]);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
PondAnalysis& TracePond::GetPondAnalysis() const
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
            bvector<DPoint3d> points;
            if (IsFull())
                {
                bool draw = nullptr == m_topLevelPond;

                if (draw)
                    {
                    if (!m_exitPoints.empty())
                        {
                        auto enclosedPond = m_exitPoints.front().exit->GetEnclosedPond();
                        if (nullptr != enclosedPond)
                            draw = enclosedPond->CurrentVolume() == 0;
                        }
                    }
                if (draw)
                    {
                    for (auto& Tpoints : m_points)
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
                    }
                }
            else
                {
                if (m_pondAnalysis.IsValid())
                    {
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
                    }
                }
            if (!points.empty())
                loadFunction(DTMFeatureType::LowPointPond, 0, 0, const_cast<DPoint3dP>(points.data()), points.size(), args);
            //for (auto& p : m_points)
            //    loadFunction(DTMFeatureType::DescentTrace, 0, 0, p.data(), p.size(), args);   //ToDO Remove.
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
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::AddResult(WaterAnalysisResultR result) const
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
    result.AddPoint(m_pt, WaterAnalysisResult::PointType::Low, CurrentVolume());
    if (result.IsWaterVolumeResult())
        {
        if (CurrentVolume() != 0)
            {
            if (IsFull())
                {
                bool draw = nullptr == m_topLevelPond;

                if (draw)
                    {
                    if (!m_exitPoints.empty())
                        {
                        auto enclosedPond = m_exitPoints.front().exit->GetEnclosedPond();
                        if (nullptr != enclosedPond)
                            draw = enclosedPond->CurrentVolume() == 0;
                        }
                    }
                if (draw)
                    {
                    for (auto&& points : m_points)
                        {
                        if (curve->empty())
                            {
                            CurveVectorPtr outer = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
                            curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*outer));
                            }
                        else
                            {
                            CurveVectorPtr hole  = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Inner);
                            curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*hole));
                            }
                        }
                    }
                }
            else
                {
                if (m_pondAnalysis.IsValid())
                    {
                    for (auto&& points : GetPondAnalysis().GetCurrentVolumePoints())
                        {
                        if (curve->empty())
                            {
                            CurveVectorPtr outer = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
                            curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*outer));
                            }
                        else
                            {
                            CurveVectorPtr hole  = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Inner);
                            curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*hole));
                            }
                        }
                    }
                }
            }
        }
    else
        {
        for (auto&& points : m_points)
            {
            if (curve->empty())
                {
                CurveVectorPtr outer = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
                curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*outer));
                }
            else
                {
                CurveVectorPtr hole  = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Inner);
                curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*hole));
                }
            }
       }
    if (!curve->empty())
        result.AddPond(*curve, IsFull(), CurrentVolume(), m_depth);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePond::Process(bvector<TraceFeaturePtr>& newFeatures)
    {
    m_finished = true;
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    bvector<PondExitInfo> exits;

    GetExitInfo(exits);

    // Add Low Points.
    if (nullptr == dynamic_cast<TracePondFromPondExitP>(this))
        {
        for (auto pnt : m_pondAnalysis->GetLowPoints())
            {
            BeAssert(m_tracer.FindPondLowPt(pnt) == nullptr || m_tracer.FindPondExit(pnt) != nullptr);
            m_tracer.AddPondLowPond(pnt, *this);
            }
        }
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
    bool addedNewPond = false;
    bvector<TracePondExit*> existingPondsExits;
    for (auto& exitInfo : exits)
        {
        TracePondExitP existingPondExit = m_tracer.FindPondExit(exitInfo.exitPoint);

        if (existingPondExit != nullptr)
            {
            existingPondExit->CheckIsCalculated();
            existingPondsExits.push_back(existingPondExit);
            existingPondExit->AddPond(*this, exitInfo);
            }
        else
            {
            // Check if Pond exit is back to the original pond exit.
            auto pondExit = TracePondExit::Create(m_tracer, *this, exitInfo.exitPoint, *pointAddrP(dtmP, exitInfo.exitPoint));
            // Finished in a real pond/hill.
            newFeatures.push_back(pondExit);

            m_children.push_back(pondExit);

            pondExit->AddPond(*this, exitInfo);
            m_tracer.AddPondExit(*pondExit);
            addedNewPond = true;
            existingPondExit = pondExit.get();
            }
        m_exitPoints.push_back(ExitPointInfo(exitInfo.exitPoint, existingPondExit));

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
                    if (std::find(existingPondsExits.begin(), existingPondsExits.end(), pondExit.exit) == existingPondsExits.end())
                        {
                        existingPondsExits.push_back(pondExit.exit);
                        }
                    }
                }
            }

        if (!hasNonDeadPond)
            {
            // Create new Pond from PondExit.
            bool needsProcessing = false;
            auto enclosingPond = existingPondsExits[0]->GetEnclosedPond();

            for (auto&& a : existingPondsExits)
                {
                needsProcessing |= !a->HasProcessedDeadPond();
                if (enclosingPond == nullptr && a->GetEnclosedPond() != nullptr)
                    enclosingPond = a->GetEnclosedPond();
                else if (enclosingPond != a->GetEnclosedPond())
                    enclosingPond = a->GetEnclosedPond();
                }

            if (!existingPondsExits[0]->HasProcessedDeadPond() != needsProcessing)
                needsProcessing = true;

            if (needsProcessing)
                {
                enclosingPond = existingPondsExits[0]->ProcessDeadPond(newFeatures);
                for (auto&& a : existingPondsExits)
                    a->SetHasProcessedDeadPond(*enclosingPond);
                }
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
void TracePond::GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume)
    {
#ifdef TEST
    static const long ptNumToCheck = 10211;

    if (m_ptNum == ptNumToCheck)
        totalVol = totalVol;
#endif
    if (nullptr != m_topLevelPond)
        {
        newWaterVolume.push_back(WaterVolumeInfo(m_topLevelPond, totalVol));
        m_topLevelPond->AddVolumeToProcess(totalVol);
        return;
        }

    if (m_maxVolume < 0)
        {
        bool gotTargetVol = false;
        GetPondAnalysis().FindBoundaryForVolume(gotTargetVol, CurrentVolume());
        if (gotTargetVol)
            return;

        m_maxVolume = GetPondAnalysis().GetCurrentVolume();


        // as this pond is full Check all ponds are full.
        for (auto&& pondExit : GetPondExits())
            {
            pondExit.exit->CheckIsCalculated();
            }

        // Get new volume.
        double prevVolume = CurrentVolume() - totalVol;

        double vol = CurrentVolume() - m_maxVolume;
        if (prevVolume > m_maxVolume)
            vol = totalVol;
        BeAssert(vol >= 0);
        totalVol = vol;
        if (totalVol <= 0)
            return;
        }

    if (!m_allFull)
        {
        bvector<TracePondExitP> exits;
        bvector<TracePondExitP> newPondExits;
        bvector<TracePondExitP> foundExits;

        for (auto&& pondExit : GetPondExits())
            {
            newPondExits.push_back(pondExit.exit);
            foundExits.push_back(pondExit.exit);
            }

        while (!newPondExits.empty())
            {
            bvector<TracePondExitP> currentPondExits;

            std::swap(currentPondExits, newPondExits);
            for (auto&& pondExit : currentPondExits)
                {
                pondExit->CheckIsCalculated();

                if (!pondExit->IsDeadPond())
                    {
                    if (std::find(exits.begin(), exits.end(), pondExit) == exits.end())
                        exits.push_back(pondExit);
                    }

                for (auto pond : pondExit->GetPonds())
                    {
                    if (pond == this)
                        continue;

                    if (!pond->IsFull())
                        {
                        if (std::find(exits.begin(), exits.end(), pondExit) == exits.end())
                            exits.push_back(pondExit);
                        continue;
                        }

                    for (auto newPondExit : pond->GetPondExits())
                        {
                        if (pondExit != newPondExit.exit && std::find(foundExits.begin(), foundExits.end(), newPondExit.exit) == foundExits.end())
                            {
                            foundExits.push_back(newPondExit.exit);
                            newPondExits.push_back(newPondExit.exit);
                            }
                        }
                    }
                }
            }

        if (!exits.empty())
            {
            totalVol /= exits.size();
            for (auto exit : exits)
                {
                BeAssert(nullptr != exit);
                exit->ProcessWaterVolume(totalVol, newWaterVolume);
                }
            return;
            }

        TracePondP topLevelPond = nullptr;
        for (auto exit : foundExits)
            {
            if (nullptr != exit->GetEnclosedPond())
                {
                m_topLevelPond = exit->GetEnclosedPond();
                break;
                }
            }

        BeAssert(m_topLevelPond != nullptr);
        if (m_topLevelPond == nullptr)
            return;
        for (auto exit : foundExits)
            {
            for (auto pond : exit->GetPonds())
                pond->SetTopLevelPond(*m_topLevelPond);
            }
        m_allFull = true;
        }

    newWaterVolume.push_back(WaterVolumeInfo(m_topLevelPond, totalVol));
    m_topLevelPond->AddVolumeToProcess(totalVol);

    //vol /= m_exitPoints.size();
    //for (auto exitPoint: GetPondExits())
    //    {
    //    BeAssert(nullptr != exitPoint.exit);
    //    if (nullptr != exitPoint.exit)
    //        {
    //        exitPoint.exit->ProcessWaterVolume(vol, newWaterVolume);
    //        //newWaterVolume.push_back(WaterVolumeInfo(exitPoint.exit, vol));Get
    //        //exitPoint.exit->AddVolumeToProcess(vol);
    //        }
    //    }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::DoTraceCallback(bool waterCallback, DTMFeatureCallback loadFunction, void* args)
        {
        loadFunction(DTMFeatureType::LowPoint, 0, 0, &m_exitPt, 1, args);

        if (!m_onHullPoint)
            {
            DPoint3d points[2];
            points[0] = m_exitPt;
            for (auto&& pt : m_points)
                {
                points[1] = pt;
                loadFunction(DTMFeatureType::DescentTrace, 0, 0, points, 2, args);
                }
            }
        }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  08/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::AddResult(WaterAnalysisResultR result) const
    {
    result.AddPoint(m_exitPt, WaterAnalysisResult::PointType::Exit, CurrentVolume());
    if (!m_onHullPoint)
        {
        bvector<DPoint3d> points;
        points.resize(2);
        points[0] = m_exitPt;
        double vol = CurrentVolume() / m_points.size();
        for (auto&& pt : m_points)
            {
            points[1] = pt;
            result.AddStream(points, vol);
            }
        }

    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::AddPond(TracePond& pond, const PondExitInfo& exitInfo)
    {
    BeAssert(exitInfo.exitPoint == m_exitPnt);

    m_exits.push_back(ExitInfo(exitInfo.priorPoint, exitInfo.nextPoint, &pond));
    m_ponds.push_back(&pond);

    CheckFullPond(pond);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::CheckFullPond(TracePond& pond)
    {
    if (!m_calculated || m_children.empty())
        return;

    // Search m_children and see if it is in this exit range.
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();

    for (auto& flow : m_flows)
        {
        auto child = flow.child;
        if (flow.isNowPond)
            continue;
        long searchPnt = flow.pt;

        for (auto&& exitInfo : m_exits)
            {
            long sp = exitInfo.priorPnt;
            if (searchPnt == sp)
                {
                flow.isNowPond = true;
                flow.thePond = &pond;
                }
            else
                {
                while (sp != exitInfo.nextPnt)
                    {
                    long np = bcdtmList_nextClkDtmObject(dtmP, m_exitPnt, sp);
                    sp = np;
                    if (searchPnt == sp)
                        {
                        flow.isNowPond = true;
                        flow.thePond = &pond;
                        break;
                        }
                    }
                }
            }
        }
    bool hasLoseExit = false;
    for (auto& flow : m_flows)
        {
        if (!flow.isNowPond)
            {
            hasLoseExit = true;
            break;
            }
        }
    m_allEnclosed = !hasLoseExit;
    }


//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
bvector<TraceFeatureP> TracePondExit::GetReferences() const
    {
    auto ret = TraceFeature::GetReferences();
    ret.push_back(m_enclosingPond);
    for (auto& pond : m_ponds)
        ret.push_back(pond);
    for (auto& exit: m_exits)
        ret.push_back(exit.pond);
    for (auto& flow : m_flows)
        {
        ret.push_back(flow.child);
        ret.push_back(flow.thePond);
        }
    return ret;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::ReplaceChildren(TraceFeature& oldChild, TraceFeature& newChild)
    {
    TraceFeature::ReplaceChildren(oldChild, newChild);
    if (m_enclosingPond == &oldChild)
        m_enclosingPond = dynamic_cast<TracePondFromPondExitP>(&newChild);

    for (auto& pond : m_ponds)
        {
        if (&oldChild == pond)
            pond = dynamic_cast<TracePondP>(&newChild);
        }

    for (auto& flow : m_flows)
        {
        if (&oldChild == flow.child)
            flow.child = &newChild;
        if (&oldChild == flow.thePond)
            flow.thePond = &newChild;
        }

    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::RemapFeatures(bmap<TraceFeatureCP, TraceFeatureP>& featureRemapTable)
    {
    TraceFeature::RemapFeatures(featureRemapTable);
    m_enclosingPond = dynamic_cast<TracePondFromPondExitP>(featureRemapTable[m_enclosingPond]);
    for (auto& pond : m_ponds)
        pond = dynamic_cast<TracePondP>(featureRemapTable[pond]);
    for (auto& exit: m_exits)
        exit.pond = dynamic_cast<TracePondP>(featureRemapTable[exit.pond]);
    for (auto& flow : m_flows)
        {
        flow.child = featureRemapTable[flow.child];
        flow.thePond = featureRemapTable[flow.thePond];
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::CheckIsCalculated()
    {
    if (!m_calculated)
        {
        bvector<TraceFeaturePtr> newFeatures;
        Process(newFeatures, true);
        m_tracer.AddAndProcessFeatures(newFeatures);
        }
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                    Daryl.Holmwood  05/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::GetNewWaterVolumes(double totalVol, bvector<TraceFeature::WaterVolumeInfo>& newWaterVolume)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    if (IsOnHull(dtmP, m_exitPnt, m_tracer.DTMHasVoids()))
        return;

    CheckIsCalculated();

    BeAssertOnce((int)m_ponds.size() <= m_numExitFlows);
    if (m_allEnclosed) //m_ponds.size() >= m_numExitFlows)
        {
        if (m_numExitFlows == 1)
            return;                 // No Exit.

        if (!m_allPondsFull)
            {
            int numNonFullPonds = 0;
            for (auto pond : m_ponds)
                {
                if (!pond->IsFull())
                    {
                    if (pond->CurrentVolume() == 0)
                        {
                        bool foundExit = false;
                        for (auto&& flow : m_flows)
                            {
                            if (flow.thePond == pond)
                                {
                                foundExit = true;
                                numNonFullPonds++;
                                }
                            }
                        if (!foundExit)
                            {
                            BeAssert(m_processedInitialPondFlows == false);
                            m_processedInitialPondFlows = true;
                            bvector<TraceFeaturePtr> newFeatures;
                            size_t numFlows = m_flows.size();

                            for (auto it = m_exits.begin() + 1; it != m_exits.end(); it++)
                                {
                                long priorPnt = it->priorPnt;
                                long nextPnt = it->nextPnt;

                                GetExitFlows(newFeatures, priorPnt, nextPnt);
                                if (numFlows != m_flows.size())
                                    break;
                                }
                            m_tracer.AddAndProcessFeatures(newFeatures);
                            CheckFullPond(*pond);

                            foundExit = false;
                            for (auto&& flow : m_flows)
                                {
                                if (flow.thePond == pond)
                                    {
                                    foundExit = true;
                                    numNonFullPonds++;
                                    }
                                }
                            BeAssert(foundExit == true);
                            }

                        }
                    else
                        numNonFullPonds++;
                    }
                }

            if (numNonFullPonds != 0)
                {
                double vol = totalVol / numNonFullPonds;
                for (auto pond : m_ponds)
                    {
                    if (!pond->IsFull())
                        {
                        // The pond needs a volume to use it.
                        if (pond->CurrentVolume() == 0)
                            {
                            for (auto&& flow : m_flows)
                                {
                                if (flow.thePond == pond)
                                    {
                                    newWaterVolume.push_back(WaterVolumeInfo(flow.child, vol));
                                    flow.child->AddVolumeToProcess(vol);
                                    }
                                }
                            }
                        else
                            {
                            newWaterVolume.push_back(WaterVolumeInfo(pond, vol));
                            pond->AddVolumeToProcess(vol);
                            }
                        }
                    }

                return;
                }

            m_allPondsFull = true;
            }
        //if (nullptr == m_enclosingPond)
        //    {
        //    for (auto&& child : m_children)
        //        {
        //        m_enclosingPond = dynamic_cast<TracePondFromPondExit*>(child.get());
        //        if (nullptr != m_enclosingPond)
        //            break;
        //        }
        //    }
        if (nullptr != m_enclosingPond)
            {
            newWaterVolume.push_back(WaterVolumeInfo(m_enclosingPond, totalVol));
            m_enclosingPond->AddVolumeToProcess(totalVol);
            return;
            }
        BeAssert(false);
        }
    else
        {
        long num = 0;
        for (auto& child : m_flows)
            if (!child.isNowPond)
                num++;

        BeAssert(num != 0);

        for (auto pond : m_ponds)
            {
            if (!pond->IsFull())
                num++;
            }

        double vol = totalVol / num;

        for (auto pond : m_ponds)
            {
            if (!pond->IsFull())
                {
                newWaterVolume.push_back(WaterVolumeInfo(pond, vol));
                pond->AddVolumeToProcess(vol);
                }
            }

        for (auto& child : m_flows)
            if (!child.isNowPond)
                {
                newWaterVolume.push_back(WaterVolumeInfo(child.child, vol));
                child.child->AddVolumeToProcess(vol);
                }
        }
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
TracePondP TracePondExit::ProcessDeadPond(bvector<TraceFeaturePtr>& newFeatures)
    {
    if (m_hasProcessedDeadPond)
        return m_enclosingPond;

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
    return child.get();
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
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    if (bcdtmList_checkForPointOnHullLineDtmObject(dtmP, m_exitPnt, &hullPoint))
        {
        SetError(); return;
        }

    if (hullPoint)
        {
        m_onHullPoint = true;
        return;
        }

    GetExitFlows(newFeatures, m_exits.front().priorPnt, m_exits.front().nextPnt);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void TracePondExit::GetExitFlows(bvector<TraceFeaturePtr>& newFeatures, long priorPnt, long nextPnt)
    {
    BC_DTM_OBJ* dtmP = m_tracer.GetDTM().GetTinHandle();
    if (priorPnt == dtmP->nullPnt || nextPnt == dtmP->nullPnt)
        {
        SetError();
        return;
        }

    bool isFlatPond = true;
    size_t currentChildrenNum = m_children.size();

    for (auto&& pond : m_ponds)
        {
        if (pond->GetDepth() != 0)
            {
            isFlatPond = false;
            break;
            }
        }
    bvector<Flow> flows;
    FindFlows(flows, m_exitPnt, priorPnt, nextPnt, isFlatPond);
#ifdef OLD

    long sp = priorPnt;
    bool voidTriangle;
    double slope;
    double descentAngle;
    double ascentAngle;
    double previousSlope = -1;
    long useP1 = dtmP->nullPnt;
    long useP2 = 0;
    double useAngle = 0;
    double angleSpPnt = bcdtmMath_getPointAngleDtmObject(dtmP, m_exitPnt, sp);
    bool rememberSteepest = false;

    while (sp != nextPnt)
        {
        long np = bcdtmList_nextAntDtmObject(dtmP, m_exitPnt, sp);
        double angleNpPnt = bcdtmMath_getPointAngleDtmObject(dtmP, m_exitPnt, np);
        // If it is a valley/sump and not on the hull.
        if (np != nextPnt &&
            (
            (isFlatPond && pointAddrP(dtmP, np)->z <= m_exitPt.z)
                ||
                (!isFlatPond && pointAddrP(dtmP, np)->z <= m_exitPt.z)
                )
            )
            {
            long anp = bcdtmList_nextAntDtmObject(dtmP, m_exitPnt, np);
            DTMFeatureType lineType;
            if (bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP, nullptr, m_exitPnt, np, sp, anp, &lineType)) { SetError(); return; }

            // Check for Sump
            if( lineType == DTMFeatureType::SumpLine )
                flows.push_back(Flow(np, dtmP->nullPnt, *pointAddrP(dtmP, np)));
            }
        if (np < 0)
            {
            SetError(); return;
            }
        if (bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP, nullptr, m_exitPnt, sp, np, voidTriangle, slope, descentAngle, ascentAngle) != DTM_SUCCESS)
            {
            SetError(); return;
            }

        if (!voidTriangle)
            {
            bool add = false;
            bool reset = false;
            double a1 = angleNpPnt;
            double a2 = m_tracer.AscentTrace() ? ascentAngle : descentAngle;
            double a3 = angleSpPnt;
            if (a1 < a3) a1 += DTM_2PYE;
            if (a2 < a3) a2 += DTM_2PYE;
            if (a2 <= a1 && a2 >= a3)
                {
                if (slope > previousSlope || useP1 == dtmP->nullPnt)
                    {
                    useP1 = np;
                    useP2 = sp;
                    useAngle = m_tracer.AscentTrace() ? ascentAngle : descentAngle;
                    rememberSteepest = true;
                    }
                else if (slope < previousSlope)
                    {
                    add = useP1 != dtmP->nullPnt;
                    }
                if (np == nextPnt)
                    add = rememberSteepest;
                }
            else
                {
                add = rememberSteepest;
                reset = true;
                }
            previousSlope = slope;

            if (add)
                {
                rememberSteepest = false;
                long intPnt;
                DPoint3d nextPt;
                if (bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP, m_exitPnt, useP1, useP2, useAngle, &nextPt.x, &nextPt.y, &nextPt.z, &intPnt))
                    {
                    SetError(); return;
                    }
                if (intPnt != dtmP->nullPnt)
                    flows.push_back(Flow(intPnt, dtmP->nullPnt, nextPt));
                else
                    flows.push_back(Flow(useP1, useP2, nextPt));
                }
            if (reset)
                useP1 = dtmP->nullPnt;
            }
        angleSpPnt = angleNpPnt;
        sp = np;
        }
#ifdef TEST
    //    else
    bvector<Flow> oldFlows;
        {
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
        oldFlows.push_back(Flow(nextPnt1, nextPnt2, nextPt));
        }
        if (flows.size() == 1)
            {
            BeAssert(oldFlows.size() == 1);
            if (flows.front().pnt1 != oldFlows.front().pnt1 ||
                flows.front().pnt2 != oldFlows.front().pnt2)
                std::swap(oldFlows, flows);
            }

#endif
        bool hasRealExit = false;
        for (auto& flow : flows)
        {
        if (flow.pt.z < m_exitPt.z)
            {
            hasRealExit = true;
            break;
            }
        }
#endif
    for (auto& flow : flows)
        {
#ifdef OLD
        if (hasRealExit && flow.pt.z == m_exitPt.z)
            {
            flow.pnt1 = dtmP->nullPnt;
            continue;
            }
#endif
        for (auto& existingFlow : m_flows)
            {
            if (existingFlow.pt == flow.pnt1 && existingFlow.pt2 == flow.pnt2)
                {
                flow.pnt1 = dtmP->nullPnt;
                break;
                }
            }

        if (flow.pnt1 == dtmP->nullPnt)
            continue;

        m_numExitFlows++;
        m_points.push_back(flow.pt); // ToDo - Need to change the draw to handle multiple exits.
        double lastAngle = bcdtmMath_getAngle(m_exitPt.x, m_exitPt.y, flow.pt.x, flow.pt.y);

        if (flow.pnt2 == dtmP->nullPnt)
            {
            if (flow.pt.z == m_exitPt.z)
                {
                // Check next and previous points
                bool isOnHull = nodeAddrP(dtmP, flow.pnt1)->hPtr == m_exitPnt || nodeAddrP(dtmP, m_exitPnt)->hPtr == flow.pnt1;
                bool voidLine = false;

                if (!isOnHull && m_tracer.DTMHasVoids())
                    {
                    voidLine = isLineOnVoidOrHole(dtmP, flow.pnt1, m_exitPnt);
                    }

                if (!isOnHull && !voidLine)
                    {
                    long pnt1 = bcdtmList_nextClkDtmObject(dtmP, m_exitPnt, flow.pnt1);
                    long pnt2 = bcdtmList_nextAntDtmObject(dtmP, m_exitPnt, flow.pnt1);

                    if (pointAddrP(dtmP, pnt1)->z > m_exitPt.z  && pointAddrP(dtmP, pnt2)->z > m_exitPt.z)
                        {
                        auto existingPond = m_tracer.FindPondLowPt(m_exitPnt);

                        if (existingPond != nullptr)
                            {
                            m_children.push_back(existingPond);
                            }
                        else
                            {
                            auto child = TracePondEdge::Create(m_tracer, *this, m_exitPnt, flow.pnt1, m_exitPt);
                            newFeatures.push_back(child);
                            m_children.push_back(child);
                            }
                        continue;
                        }
                    }
                }

            if (flow.pt.z == m_exitPt.z)
                {
                auto existingPond = m_tracer.FindPondLowPt(m_exitPnt);

                if (existingPond != nullptr)
                    {
                    m_children.push_back(existingPond);
                    continue;
                    }
                    // Check if this is a flat triangle.
                long oPt1 = bcdtmList_nextAntDtmObject(dtmP, m_exitPnt, flow.pnt1);
                if (m_exitPt.z == pointAddrP(dtmP, oPt1)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, m_exitPnt, oPt1, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                long oPt2 = bcdtmList_nextClkDtmObject(dtmP, m_exitPnt, flow.pnt1);
                if (m_exitPt.z == pointAddrP(dtmP, oPt2)->z)
                    {
                    auto child = TracePondTriangle::Create(m_tracer, *this, flow.pnt1, oPt2, m_exitPnt, flow.pt);
                    newFeatures.push_back(child);
                    m_children.push_back(child);
                    continue;
                    }
                }
            auto child = TraceOnPoint::GetOrCreate(newFeatures, m_tracer, *this, flow.pnt1, flow.pt, lastAngle);
            m_children.push_back(child);
            }
        else
            {
            CreateAndAddEdge(newFeatures, flow.pnt1, flow.pnt2, flow.pt, lastAngle);
#ifdef OLD
            long pnt3;
            if ((pnt3 = bcdtmList_nextAntDtmObject(dtmP, flow.pnt1, flow.pnt2)) < 0)
                {
                SetError(); return;
                }
            if (flow.pt.z == pointAddrP(dtmP, flow.pnt2)->z)
                {
                CreateAndAddEdge(newFeatures, flow.pnt1, flow.pnt2, m_lastAngle);
                bool isOnHull = nodeAddrP(dtmP, flow.pnt1)->hPtr == flow.pnt2 || nodeAddrP(dtmP, flow.pnt2)->hPtr == flow.pnt1;
                bool voidLine = false;

                if (!isOnHull && m_tracer.DTMHasVoids())
                    bcdtmList_testForVoidLineDtmObject(dtmP, flow.pnt1, flow.pnt2, voidLine);

                if (!isOnHull && !voidLine)
                    {
                    // Check next and previous points
                    long pnt1 = bcdtmList_nextClkDtmObject(dtmP, flow.pnt1, flow.pnt2);
                    long pnt2 = bcdtmList_nextAntDtmObject(dtmP, flow.pnt1, flow.pnt2);
                    if (pointAddrP(dtmP, pnt1)->z > flow.pt.z  && pointAddrP(dtmP, pnt2)->z > flow.pt.z)
                        {
                        auto existingPond = m_tracer.FindPondLowPt(flow.pnt1);

                        if (existingPond != nullptr)
                            {
                            m_children.push_back(existingPond);
                            }
                        else
                            {
                            auto child = TracePondEdge::Create(m_tracer, *this, flow.pnt1, flow.pnt2, flow.pt);
                            newFeatures.push_back(child);
                            m_children.push_back(child);
                            }
                        continue;
                        }
                    }
                }


            auto child = TraceOnEdge::Create(m_tracer, *this, flow.pnt1, flow.pnt2, pnt3, flow.pt, lastAngle);
            newFeatures.push_back(child);
            m_children.push_back(child);
#endif
            }
        }

    int i = 0;
    for (auto& flow : flows)
        {
        if (flow.pnt1 == dtmP->nullPnt)
            continue;
        m_flows.push_back(FlowInfo(flow.pnt1, flow.pnt2, m_children[i++ + currentChildrenNum].get()));
        }

    BeAssert(!m_flows.empty());
    }


//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
WaterAnalysis::WaterAnalysis(BcDTMR dtm) : m_dtm(dtm), m_dtmObj(dtm.GetTinHandle())
    {
    m_pointList.resize(m_dtmObj->numPoints);
    bcdtmList_testForVoidsInDtmObject(m_dtmObj, m_dtmHasVoids) ;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
WaterAnalysis::~WaterAnalysis()
    {
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*
double WaterAnalysis::GetPondElevationTolerance() const
    {
    if (nullptr != GetDTM().GetTransformHelper())
        return GetDTM().GetTransformHelper()->convertDistanceFromDTM(m_pondElevationTolerance);
    return m_pondElevationTolerance;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::SetPondElevationTolerance(double value)
    {
    if (nullptr != GetDTM().GetTransformHelper())
        m_pondElevationTolerance = GetDTM().GetTransformHelper()->convertDistanceToDTM(value);
    else
        m_pondElevationTolerance = value;
    }
//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*

double WaterAnalysis::GetPondVolumeTolerance() const
    {
    if (nullptr != GetDTM().GetTransformHelper())
        return GetDTM().GetTransformHelper()->convertVolumeFromDTM(m_pondVolumeTolerance);
    return m_pondVolumeTolerance;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::SetPondVolumeTolerance(double value)
    {
    if (nullptr != GetDTM().GetTransformHelper())
        m_pondVolumeTolerance = GetDTM().GetTransformHelper()->convertVolumeToDTM(value);
    else
        m_pondVolumeTolerance = value;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*
double WaterAnalysis::GetMinimumDepth() const
    {
    if (nullptr != GetDTM().GetTransformHelper())
        return GetDTM().GetTransformHelper()->convertDistanceFromDTM(m_falseLowDepth);
    return m_falseLowDepth;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  09/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::SetMinimumDepth(double value)
    {
    if (nullptr != GetDTM().GetTransformHelper())
        m_falseLowDepth = GetDTM().GetTransformHelper()->convertDistanceToDTM(value);
    else
        m_falseLowDepth = value;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
int WaterAnalysis::DoTrace(DPoint3dCR startPt)
    {
    auto transformHelper = GetDTM().GetTransformHelper();
    TraceStartPointPtr startFeature = nullptr;
    
    if (nullptr != transformHelper)
        {
        DPoint3d cStartPoint = startPt;
        transformHelper->convertPointToDTM(cStartPoint);
        startFeature = TraceStartPoint::Create(*this, cStartPoint);
        }
    else
        startFeature = TraceStartPoint::Create(*this, startPt);
    bvector<TraceFeaturePtr> featuresToProcess;
    featuresToProcess.push_back(startFeature);
    AddAndProcessFeatures(featuresToProcess);
    return DTM_SUCCESS;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
int WaterAnalysis::AddWaterVolume(DPoint3dCR startPt, double volume)
    {
    auto transformHelper = GetDTM().GetTransformHelper();
    m_forWater = true;
    TraceStartPointPtr startFeature = nullptr;

    if (nullptr != transformHelper)
        {
        DPoint3d cStartPoint = startPt;
        transformHelper->convertPointToDTM(cStartPoint);
        startFeature = TraceStartPoint::Create(*this, cStartPoint);
        volume = transformHelper->convertVolumeToDTM(volume);
        }
    else
        startFeature = TraceStartPoint::Create(*this, startPt);

    bvector<TraceFeaturePtr> featuresToProcess;
    featuresToProcess.push_back(startFeature);
    AddAndProcessFeatures(featuresToProcess);

    bvector<TraceFeature::WaterVolumeInfo> features;
    bvector<TraceFeature::WaterVolumeInfo> newFeatures;

    newFeatures.push_back(TraceFeature::WaterVolumeInfo(startFeature.get(), volume));
    startFeature->AddVolumeToProcess(volume);
    //bmap<TraceFeature*, bool> processedFeatureMap;  // Temporary to help with debug.

    while (!newFeatures.empty())
        {
        std::swap(features, newFeatures);
        newFeatures.clear();
        for (auto&& feature : features)
            {
            double vol = feature.feature->GetVolumeToProcess();
            feature.feature->ClearVolumeToProcess();
            if (vol != 0)
                feature.feature->ProcessWaterVolume(vol, newFeatures);
            }
        }
    return DTM_SUCCESS;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::DoTraceCallback(DTMFeatureCallback loadFunction, void* userArg)
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
void WaterAnalysis::AddAndProcessFeatures(bvector<TraceFeaturePtr>& featuresToAdd)
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
TracePondExit* WaterAnalysis::FindPondExit(long exitPoint)
    {
    auto it = m_pondExits.find(exitPoint);
    if (it == m_pondExits.end())
        return nullptr;
    return it->second;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::AddPondExit(TracePondExit& pondexit)
    {
    m_pondExits[pondexit.GetExitPoint()] = &pondexit;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::AddOnPoint(long pointNum, TraceOnPoint& point)
    {
    m_onPointFeatures[pointNum] = &point;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TraceOnPoint* WaterAnalysis::FindExistingOnPoint(long pointNum)
    {
    auto it = m_onPointFeatures.find(pointNum);
    if (it != m_onPointFeatures.end())
        return it->second;
    return nullptr;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
void WaterAnalysis::AddPondLowPond(long pointNum, TracePond& pond)
    {
    m_pondlowPts[pointNum] = &pond;
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
TracePond* WaterAnalysis::FindPondLowPt(long pointNum)
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
    TMTransformHelperP m_transformHelper;
    void* m_userP;

    QuickFeatureJoiner(TMTransformHelperP transformHelper, DTMFeatureCallback loadFunctionP, void* userP) : m_loadFunctionP(loadFunctionP), m_userP(userP), m_transformHelper(transformHelper)
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
    int Callback(DTMFeatureType dtmFeatureType, DPoint3d *newPoints, size_t numPoints)
        {
        bvector<DPoint3d> points;

        points.resize(numPoints);
        memcpy(points.data(), newPoints, sizeof(newPoints[0]) * numPoints);

        if (nullptr != m_transformHelper)
            m_transformHelper->convertPointsToDTM(points.data(), (int)numPoints);

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
        return m_loadFunctionP(dtmFeatureType, 0, 0, points.data(), numPoints, m_userP);
        }

    };

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
WaterAnalysisPtr WaterAnalysis::Clone()
    {
    return new WaterAnalysis(*this);
    }

//----------------------------------------------------------------------------------------*
// @bsimethod                                                   Daryl.Holmwood  06/17
// +---------------+---------------+---------------+---------------+---------------+------*
WaterAnalysis::WaterAnalysis(WaterAnalysisCR from) : m_dtm(from.m_dtm), m_dtmObj(from.m_dtm.GetTinHandle())
    {
    bmap<TraceFeatureCP, TraceFeatureP> featureRemapTable;

    m_pointList = from.m_pointList;
    m_ascentTrace = from.m_ascentTrace;
    m_zeroSlopeOption = from.m_zeroSlopeOption;
    m_falseLowDepth = from.m_falseLowDepth;
    m_forWater = from.m_forWater;
    m_pondElevationTolerance = from.m_pondElevationTolerance;
    m_pondVolumeTolerance = from.m_pondVolumeTolerance;
    m_dtmHasVoids = from.m_dtmHasVoids;

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
WaterAnalysisPtr WaterAnalysis::Create(BcDTMR dtm)
    {
    return new WaterAnalysis(dtm);
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
    //bcdtmDrainage_traceMaximumDescentDtmObjectOld(dtmP, drainageTablesP, loadFunctionP, -falseLowDepth, startX, startY, userP);
    TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::CreateFromDtmHandle(*dtmP);
    WaterAnalysisPtr tracer = WaterAnalysis::Create(*dtm);

    tracer->SetMinimumDepth(falseLowDepth);
    tracer->DoTrace(DPoint3d::From(startX, startY, 0));
    QuickFeatureJoiner joiner(dtm->GetTransformHelper(), loadFunctionP, userP);
    tracer->DoTraceCallback(&QuickFeatureJoiner::callback, &joiner);
    return DTM_SUCCESS;
    }


struct WaterAnalysisResultImpl : WaterAnalysisResult
    {
    public:
        virtual void _AddPoint(DPoint3dCR point, WaterAnalysisResult::PointType type, double volume) override
            {
            if (nullptr == m_transformHelper)
                push_back(WaterAnalysisResultPoint::Create(point, type, volume));
            else
                push_back(WaterAnalysisResultPoint::Create(m_transformHelper->getPointFromDTM(point), type, volume));
            }

        virtual void _AddStream(CurveVector& geometry, double volume) override
            {
            if (nullptr != m_transformHelper)
                geometry.TransformInPlace(m_fromDTMTransformation);

            if (!empty())
                {
                auto lastItem = back();
                auto lastStream = lastItem->AsStream();

                if (nullptr != lastStream && lastStream->GetWaterVolume() == volume)
                    {
                    DPoint3d startPt, endPt;

                    if (lastStream->GetGeometry().GetStartEnd(startPt, endPt))
                        {
                        if (geometry.GetStartPoint(startPt))
                            {
                            if (endPt.IsEqual(startPt))
                                {
                                lastStream->AddPrimitives(geometry);
                                return;
                                }
                            }
                        }
                    }

                if (nullptr != lastStream && lastStream->GetGeometry().size() != 1)
                    lastStream->ConsolidateAdjacentPrimitives();
                }

            push_back(WaterAnalysisResultStream::Create(geometry, volume));
            }

        virtual void _AddStream(const bvector<DPoint3d>& points, double volume) override
            {
            CurveVectorPtr curve = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Open);
            AddStream(*curve, volume);
            }

        virtual void _AddPond(CurveVector& geometry, bool isFull, double volume, double depth) override
            {
            if (nullptr != m_transformHelper)
                {
                geometry.TransformInPlace(m_fromDTMTransformation);
                volume = m_transformHelper->convertVolumeFromDTM(volume);
                depth = m_transformHelper->convertDistanceFromDTM(depth);
                }

            if (geometry.size() == 1)
                push_back(WaterAnalysisResultPond::Create(*geometry.front()->GetChildCurveVectorP(), isFull, volume, depth));
            else
                push_back(WaterAnalysisResultPond::Create(geometry, isFull, volume, depth));
            }

        virtual bool _IsWaterVolumeResult() const override
            {
            return m_forWater;
            }
    private:
        bool m_showHidden = false;
        bool m_forWater = false;
        TMTransformHelperP m_transformHelper = nullptr;
        Transform m_fromDTMTransformation;

        WaterAnalysisResultImpl(WaterAnalysisCR analysis)
            {
            GetResult(analysis);
            }

    private:
        std::set<TraceFeatureCP> m_featuresProcessed;
        void GetResult(TraceFeatureCR feature)
            {
            bvector<TraceFeatureCP> features;
            features.push_back(&feature);

            while (!features.empty())
                {
                auto featureToAdd = features.back();
                features.pop_back();

                if (m_featuresProcessed.find(featureToAdd) != m_featuresProcessed.end())
                    continue;

                m_featuresProcessed.insert(featureToAdd);
                if (m_showHidden || !featureToAdd->IsHidden() || m_forWater)
                    featureToAdd->AddResult(*this);

                for (auto&& child : featureToAdd->GetChildren())
                    features.push_back(child.get());
                }
            }

        void GetResult(WaterAnalysisCR analysis)
            {
            m_transformHelper = analysis.GetDTM().GetTransformHelper();
            if (nullptr != m_transformHelper)
                {
                if (m_transformHelper->IsIdentity())
                    m_transformHelper = nullptr;
                else
                    m_transformHelper->GetTransformationFromDTM(m_fromDTMTransformation);
                }
            m_forWater = analysis.ForWater();
            for (const auto& feature : analysis.GetFeatures())
                {
                if (nullptr == feature->GetParent())
                    GetResult(*feature);
                }

            if (!empty())
                {
                auto lastItem = back();
                auto lastStream = lastItem->AsStream();
                if (nullptr != lastStream && lastStream->GetGeometry().size() != 1)
                    lastStream->ConsolidateAdjacentPrimitives();
                }
            m_featuresProcessed.clear();
            }

    public:
        static WaterAnalysisResultPtr Create(WaterAnalysisCR analysis)
            {
            return new WaterAnalysisResultImpl(analysis);
            }
    };


WaterAnalysisResultPtr WaterAnalysis::GetResult() const
    {
    return WaterAnalysisResultImpl::Create(*this);
    }

WaterAnalysisResultPointP WaterAnalysisResultItem::AsPoint()
    {
    return _AsPoint();
    }

WaterAnalysisResultStreamP WaterAnalysisResultItem::AsStream()
    {
    return _AsStream();
    }

WaterAnalysisResultPondP WaterAnalysisResultItem::AsPond()
    {
    return _AsPond();
    }

double WaterAnalysisResultItem::GetWaterVolume() const
    {
    return m_waterVolume;
    }

DPoint3dCR WaterAnalysisResultPoint::GetPoint() const
    {
    return m_pt;
    }

WaterAnalysisResult::PointType WaterAnalysisResultPoint::GetType()
    {
    return m_type;
    }

CurveVectorCR WaterAnalysisResultGeometry::GetGeometry() const
    {
    return *m_geometry;
    }

bool WaterAnalysisResultPond::IsFull() const
    {
    return m_isFull;
    }

double WaterAnalysisResultPond::Depth() const
    {
    return m_depth;
    }

void WaterAnalysisResult::AddPoint(DPoint3dCR point, PointType type, double volume)
    {
    _AddPoint(point, type, volume);
    }

void WaterAnalysisResult::AddStream(CurveVector& geometry, double volume)
    {
    _AddStream(geometry, volume);
    }

void WaterAnalysisResult::AddStream(const bvector<DPoint3d>& points, double volume)
    {
    _AddStream(points, volume);
    }

void WaterAnalysisResult::AddPond(CurveVector& geometry, bool isFull, double volume, double depth)
    {
    _AddPond(geometry, isFull, volume, depth);
    }

bool WaterAnalysisResult::IsWaterVolumeResult() const
    {
    return _IsWaterVolumeResult();
    }

// ToDo
// 1. Add Inner pond analysis to the pond from exit analysis, this will copy the area and the inner ponds.
// 3. Handle ponds in ponds, when they are reached, trace down and fill it up.
//  a. Send in the other ponds analysis.
//  b. Copy the area, and add the inner ponds.
// 4. Implement new callback, with CurveVector,depth, volume.
// 5. Fix display from pondexit if multiple exits.
// 6. If a pond is deep enough dont show the enclosing pond.
// ToDo sort out Zsumps on edit elevation.
END_BENTLEY_TERRAINMODEL_NAMESPACE

// ToDo.
// 1. Add Inner pond analysis to the pond from exit analysis, this will copy the area and the inner ponds.
// 2. For PondFromExit we need to get the other exit points, to make sure we have all boundaries.
// 3. Cope with voids.
// 4. Cope with inner ponds.
// Cope with a pond with multiple exits which all flow into a single pond and then is closed. combin.xml @ (290050.643m,6242831.536m,68.928m)
