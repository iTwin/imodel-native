/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/DTMEvars.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <TerrainModel/Core/PartitionArray.h>
#include <queue>
#include <set>

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int bcdtmList_testForBreakLineDtmObject(BC_DTM_OBJ* dtmP, long P1, long P2, bool isTestingForVoid)
/*
** This Function Tests If The Line P1P2 or Line P2P1 is A Break Line
*/
    {
    long clc;
    /*
    ** Test For P1 P2 Being Break Line
    */
    clc = nodeAddrP(dtmP, P1)->fPtr;
    while (clc != dtmP->nullPtr)
        {
        if (flistAddrP(dtmP, clc)->nextPnt == P2)
            {
            auto fTable = ftableAddrP(dtmP, flistAddrP(dtmP, clc)->dtmFeature);
            if (fTable->dtmFeatureType == DTMFeatureType::Breakline)
                {
                auto fType = (DTMFeatureType) fTable->dtmUserTag;
                if (!isTestingForVoid && fType == DTMFeatureType::Island)
                    return 1;

                if (isTestingForVoid && fType != DTMFeatureType::Island)
                    return 1;
                }
            }
        clc = flistAddrP(dtmP, clc)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return(0);
    }

static int bcdtmList_testForBreakLineDtmObjectQuick(BC_DTM_OBJ* dtmP, long P1, long P2)
/*
** This Function Tests If The Line P1P2 or Line P2P1 is A Break Line
*/
    {
    long clc;
    /*
    ** Test For P1 P2 Being Break Line
    */
    clc = nodeAddrP(dtmP, P1)->fPtr;
    while (clc != dtmP->nullPtr)
        {
        if (flistAddrP(dtmP, clc)->nextPnt == P2)
            {
            auto fTable = ftableAddrP(dtmP, flistAddrP(dtmP, clc)->dtmFeature);
            if (fTable->dtmFeatureType == DTMFeatureType::Breakline)
                {
                return 1;
                }
            }
        clc = flistAddrP(dtmP, clc)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return(0);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getDtmFeatureNumsLineDtmObject(BC_DTM_OBJ* dtmP, long pnt1, long pnt2, std::set<long>& featureNumList, bool isLookingForVoids)
/*
** This Function Tests If The Line pnt1-pnt2 is A DtmFeature Type Line
*/
    {
    long clc;
    clc = nodeAddrP(dtmP, pnt1)->fPtr;
    while (clc != dtmP->nullPtr)
        {
        if (flistAddrP(dtmP, clc)->nextPnt == pnt2)
            {
            long num = flistAddrP(dtmP, clc)->dtmFeature;
            auto fType = (DTMFeatureType)ftableAddrP(dtmP, num)->dtmUserTag;
            if (!isLookingForVoids && fType == DTMFeatureType::Island)
                {
                if (featureNumList.find(num) == featureNumList.end())
                    featureNumList.insert(num);
                }
            else if (isLookingForVoids && fType != DTMFeatureType::Island)
                {
                if (featureNumList.find(num) == featureNumList.end())
                    featureNumList.insert(num);
                }
            }
        clc = flistAddrP(dtmP, clc)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return(0);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
long bcdtmList_getFirstDtmFeatureLineDtmObject(BC_DTM_OBJ* dtmP, long pnt1, long pnt2)
/*
** This Function Tests If The Line pnt1-pnt2 is A DtmFeature Type Line
*/
    {
    long clc;
    clc = nodeAddrP(dtmP, pnt1)->fPtr;
    while (clc != dtmP->nullPtr)
        {
        if (flistAddrP(dtmP, clc)->nextPnt == pnt2)
            return flistAddrP(dtmP, clc)->dtmFeature;
        clc = flistAddrP(dtmP, clc)->nextPtr;
        }
    clc = nodeAddrP(dtmP, pnt2)->fPtr;
    while (clc != dtmP->nullPtr)
        {
        if (flistAddrP(dtmP, clc)->nextPnt == pnt1)
            return flistAddrP(dtmP, clc)->dtmFeature;
        clc = flistAddrP(dtmP, clc)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return -1;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmList_writeVectorList (std::set<long>& featureNumList)
    {
    for (auto featureNum : featureNumList)
        {
        bcdtmWrite_message(0,0,0,"FeatureNum %d", featureNum) ;
        }
    return (0);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static DTMFeatureId GetFeatureIdForLink(BC_DTM_OBJ* dtmP, std::set<long>& usedFeatureIndexes, long p1, long p2)
    {
    auto featureNum = bcdtmList_getFirstDtmFeatureLineDtmObject(dtmP, p1, p2);
    BeAssert(featureNum != -1);
    if (featureNum == -1)
        return ftableAddrP(dtmP, *usedFeatureIndexes.begin())->dtmFeatureId;
    return ftableAddrP(dtmP, featureNum)->dtmFeatureId;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void StoreInDTMFindEdges(BC_DTM_OBJ* dtmP, std::set<long>& usedFeatureIndexes, bvector<DPoint3d>& ls, DTMFeatureType featureType)
    {
    auto startPoint = &ls[0];
    long prevPtNum = -1;
    DTMFeatureId prevFtId = DTM_NULL_FEATURE_ID;

    DTMDirection direction;
    double area;
    bcdtmMath_getPolygonDirectionP3D(ls.data(), (long) ls.size(), &direction, &area);
    if (direction == DTMDirection::Clockwise)
        std::reverse(ls.begin(), ls.end());
    for (auto& pt : ls)
        {
        long ptNum;
        auto ret = bcdtmFind_closestPointDtmObject(dtmP, pt.x, pt.y, &ptNum);
        pt.z = pointAddrP(dtmP, ret)->z;
        BeAssert(ret == 1);
        if (prevPtNum != -1)
            {
            auto fId = GetFeatureIdForLink(dtmP, usedFeatureIndexes, prevPtNum, ptNum);

            if (fId != prevFtId)
                {
                if (prevFtId != DTM_NULL_FEATURE_ID)
                    {
                    prevFtId = -prevFtId;
                    const int numPoints = (long) (&pt - startPoint);
                    bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::Breakline, (DTMUserTag) featureType, 2, &prevFtId, startPoint, numPoints);
                    }

                startPoint = &pt - 1;
                prevFtId = fId;
                }
            }
        prevPtNum = ptNum;
        }
    if (prevFtId != DTM_NULL_FEATURE_ID)
        {
        prevFtId = -prevFtId;
        const long numPoints = (long) (&*ls.end() - startPoint);
        bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::Breakline, (DTMUserTag) featureType, 2, &prevFtId, startPoint, numPoints);
        }
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int loadTinEdges(BC_DTM_OBJ* dtmP, std::function <int(BC_DTM_OBJ* dtmP, long p1, long p2, void* userArg)> loadFunctionP, void* userP)
    {
    // Doesn't cope with voids, but we know there isn't any as we add them all as breaklines.
    long startPnt = 0;
    long lastPnt = dtmP->numPoints;
    long clPtr;
    for (long p1 = startPnt; p1 < lastPnt; ++p1)
        {
        auto nodeP = nodeAddrP(dtmP, p1);
        if ((clPtr = nodeP->cPtr) != dtmP->nullPtr)
            {
            while (clPtr != dtmP->nullPtr)
                {
                auto clistP = clistAddrP(dtmP, clPtr);
                auto p2 = clistP->pntNum;
                clPtr = clistP->nextPtr;
                if (p2 > p1)
                    loadFunctionP(dtmP, p1, p2, userP);
                }
            }
        }
    return SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int StoreLinksOnVoidEdge(BC_DTM_OBJ* dtm, long p1, long p2, void* userArg)
    {
    if (nodeAddrP(dtm, p1)->hPtr == p2 || nodeAddrP(dtm, p2)->hPtr == p1)
        {
        }
    else
        {
        bool addEdge = false;
        auto cv = (CurveVectorP) userArg;
        auto p3 = bcdtmList_nextAntDtmObject(dtm, p1, p2);
        auto p4 = bcdtmList_nextClkDtmObject(dtm, p1, p2);

        bool v1 = bcdtmList_testForVoidTriangleDtmObject(dtm, p2, p1, p3);
        bool v2 = bcdtmList_testForVoidTriangleDtmObject(dtm, p1, p2, p4);

        addEdge = v1 != v2;
        if(addEdge)
    {
            if (!bcdtmList_testForBreakLineDtmObject(dtm, p1, p2))
                {
                if (v1)
                    std::swap(p3, p4);
                if (bcdtmList_testForBreakLineDtmObject(dtm, p1, p3) &&
                    bcdtmList_testForBreakLineDtmObject(dtm, p3, p2))
                    {
                    DPoint3d pts[3];
                    pts[0] = *pointAddrP(dtm, p1);
                    pts[1] = *pointAddrP(dtm, p3);
                    //cv->Add(ICurvePrimitive::CreateLineString(pts, 2));
                    pts[2] = *pointAddrP(dtm, p2);
                    cv->Add(ICurvePrimitive::CreateLineString(pts, 3));
                    return DTM_SUCCESS;
                    }
                }
            DPoint3d pts[2];
            pts[0] = *pointAddrP(dtm, p1);
            pts[1] = *pointAddrP(dtm, p2);
            cv->Add(ICurvePrimitive::CreateLineString(pts, 2));
            }
        }
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int StoreLinksNotOnHull(BC_DTM_OBJ* dtm, long p1, long p2, void* userArg)
    {
    if (nodeAddrP(dtm, p1)->hPtr == p2 || nodeAddrP(dtm, p2)->hPtr == p1)
        return DTM_SUCCESS;

    if (bcdtmList_testForBreakLineDtmObjectQuick(dtm, p1, p2))
        {
        BC_DTM_OBJ* dtm2 = (BC_DTM_OBJ*) userArg;
        DPoint3d pts[2];
        pts[0] = *pointAddrP(dtm, p2);
        pts[1] = *pointAddrP(dtm, p1);
        DTMFeatureId dtmFeatureId;
        bcdtmObject_storeDtmFeatureInDtmObject(dtm2, DTMFeatureType::Breakline, dtm2->nullUserTag, 2, &dtmFeatureId, pts, 2);
        }
    else if (bcdtmList_testForBreakLineDtmObjectQuick(dtm, p2, p1))
        {
        BC_DTM_OBJ* dtm2 = (BC_DTM_OBJ*) userArg;
        DPoint3d pts[2];
        pts[0] = *pointAddrP(dtm, p1);
        pts[1] = *pointAddrP(dtm, p2);
        DTMFeatureId dtmFeatureId;
        bcdtmObject_storeDtmFeatureInDtmObject(dtm2, DTMFeatureType::Breakline, dtm2->nullUserTag, 2, &dtmFeatureId, pts, 2);
        }
    return DTM_SUCCESS;
    }
//#define DHDEBUG

#ifdef DHDEBUG

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void DebugCurveVector(BC_DTM_OBJ* dtmP, ICurvePrimitiveCR prim)
    {
    switch (prim.GetCurvePrimitiveType())
        {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            {
            auto ccv = prim.GetChildCurveVectorCP();
            BeAssert(nullptr != ccv);
            if (ccv != nullptr)
                {
                for (auto prim2 : *ccv)
                    DebugCurveVector(dtmP, *prim2);
                }
            }
            break;
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            auto ls = prim.GetLineStringCP();
            DTMFeatureId dtmFeatureId;
            bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::Breakline, dtmP->nullUserTag, 2, &dtmFeatureId, (DPoint3d*) ls->data(), (long) ls->size());
            }
            break;
            default:
                BeAssert(false);
        }
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void DebugCurveVector(CurveVectorCR cv)
    {
    BC_DTM_OBJ* tempDtm = nullptr;
    bcdtmObject_createDtmObject(&tempDtm);

    for (auto& prim : cv)
        DebugCurveVector(tempDtm, *prim);

    tempDtm->ppTol = 0.0;
    tempDtm->plTol = 0.0;
    bcdtmObject_createTinDtmObject(tempDtm, 1, 0.0, false);
    bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtm);
    bcdtmWrite_toFileDtmObject(tempDtm, L"d:\\debugCV.bcdtm");
    bcdtmObject_destroyDtmObject(&tempDtm);

    }
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObjectOld (BC_DTM_OBJ* dtmP, BC_DTM_OBJ* cleanedDtmP, std::set<long>& usedFeatureIndexes, DTMFeatureType featureType, bvector <DPoint3d>& points, DTMFeatureId dtmFeatureId)
    {
    bool useDtm = false;
    // If this polygon has 3 points or else then it isn't valid.
    if (points.size() <= 3)
        return DTM_ERROR;

    if (ftableAddrP(dtmP, *usedFeatureIndexes.begin())->dtmFeatureState == DTMFeatureState::Deleted)
        return DTM_SUCCESS;

    if (bcdtmObject_storeDtmFeatureInDtmObject(cleanedDtmP, featureType, cleanedDtmP->nullUserTag, 2, &dtmFeatureId, (DPoint3d*) &points[0], (long) points.size()))
        return DTM_ERROR;

    BC_DTM_OBJ* tempDtm = nullptr;
    bcdtmObject_createDtmObject(&tempDtm);
    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    bvector<bvector<DPoint3d>> voids;
    for (auto featureNum : usedFeatureIndexes)
        {
        auto dtmFeatureP = ftableAddrP(dtmP, featureNum);
        // This feature was part of an invalid combination so just add this as is to the DTM.
        long numFeaturePts;
        DPoint3dP tFeaturePts = nullptr;
        if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP, featureNum, &tFeaturePts, &numFeaturePts))
            continue;
        if (tFeaturePts[0].IsEqual(tFeaturePts[numFeaturePts - 1]))
            {
            bcdtmObject_storeDtmFeatureInDtmObject(tempDtm, DTMFeatureType::Breakline, tempDtm->nullUserTag, 2, &dtmFeatureId, (DPoint3d*)tFeaturePts, numFeaturePts);
            bvector<DPoint3d> pts;
            for (int j = 0; j < numFeaturePts; j++)
                pts.push_back(tFeaturePts[j]);
            voids.push_back(pts);
            }
        else
            cv->Add(ICurvePrimitive::CreateLineString(tFeaturePts, numFeaturePts));
        free(tFeaturePts);
        tFeaturePts = nullptr;
        }

    bvector<bvector<bvector<DPoint3d>>> regions;
    if (!cv->empty())
        {
        //DebugCurveVector(*cv);
        cv = cv->AssembleChains();
        //DebugCurveVector(*cv);

        for (auto& prim : *cv)
            {
            auto ccv = prim->GetChildCurveVectorP();
            ccv->CollectLinearGeometry(regions);
            for (auto& l1 : regions)
                {
                for (auto& l2 : l1)
                    {
                    bcdtmObject_storeDtmFeatureInDtmObject(tempDtm, DTMFeatureType::Breakline, tempDtm->nullUserTag, 2, &dtmFeatureId, (DPoint3d*) l2.data(), (long) l2.size());
                    voids.push_back(l2);
                    }
                }
            }
        }

    tempDtm->ppTol = 0.0;
    tempDtm->plTol = 0.0;
    bcdtmObject_createTinDtmObject(tempDtm, 1, 0.0, false);
    bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtm);
    //bcdtmWrite_toFileDtmObject(tempDtm, L"d:\\voidHulls.bcdtm");
    for (auto& pts : voids)
        {
        DTMDirection direction;
        double area;
        bcdtmMath_getPolygonDirectionP3D(pts.data(), (long) pts.size(), &direction, &area);
        if (direction == DTMDirection::Clockwise)
            std::reverse(pts.begin(), pts.end());

        auto status = bcdtmClip_toPolygonDtmObject(tempDtm, pts.data(), (long) pts.size(), DTMClipOption::Internal);

        if (status != DTM_SUCCESS)
            status = status;
        }
    //bcdtmWrite_toFileDtmObject(tempDtm, L"d:\\voidHulls.bcdtm");

    auto cv2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    loadTinEdges(tempDtm, StoreLinksOnVoidEdge, cv2.get());
    if (!cv2->empty())
        {
        bcdtmObject_destroyDtmObject(&tempDtm);

        //DebugCurveVector(*cv2);
        cv2 = cv2->AssembleChains();
        //DebugCurveVector(*cv2);
        int j = 0;
        for (auto& prim : *cv2)
            {
            auto ccv = prim->GetChildCurveVectorP();
            ccv->CollectLinearGeometry(regions);
            for (auto& l1 : regions)
                {
                for (auto& l2 : l1)
                    {
                    if (l2.front().IsEqual(l2.back()))
                        StoreInDTMFindEdges(dtmP, usedFeatureIndexes, l2, featureType != DTMFeatureType::Island ? DTMFeatureType::Island : DTMFeatureType::Void);
                    }
                }
            j++;
            }
        }
    return DTM_SUCCESS;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    static void ExtractInnerLoops(BC_DTM_OBJ& dtm, bvector<CurveVectorCP>& vectors)
        {
        BC_DTM_OBJ* tempDtm = nullptr;
        bcdtmObject_createDtmObject(&tempDtm);

        for (auto ccv : vectors)
                {
            bvector<bvector<bvector<DPoint3d>>> regions;
            ccv->CollectLinearGeometry(regions);

            for (auto& l1 : regions)
                    {
                for (auto& l2 : l1)
                    {
                    if (l2.front().AlmostEqualXY(l2.back()))
                        {
                        l2.front() = l2.back();
                        DTMFeatureId dtmFeatureId;
                        bcdtmObject_storeDtmFeatureInDtmObject(tempDtm, DTMFeatureType::Breakline, (DTMUserTag) DTMFeatureType::Island, 2, &dtmFeatureId, (DPoint3d*) l2.data(), (long) l2.size());
                    }
                }
            }
        }

        tempDtm->ppTol = 0.0;
        tempDtm->plTol = 0.0;
        bcdtmObject_createTinDtmObject(tempDtm, 1, 0.0, false);
        bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtm);
        loadTinEdges(tempDtm, StoreLinksNotOnHull, &dtm);
//        bcdtmWrite_toFileDtmObject(tempDtm, L"d:\\voidHullsEIL.bcdtm");
        bcdtmObject_destroyDtmObject(&tempDtm);

        }

    const long PTTYPE_MARK = 45;


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    static void AddTPtrToResultAndClearTPtr(BC_DTM_OBJ& dtm, long startPnt, bvector<bvector<DPoint3d>>& result)
        {
        result.push_back(bvector<DPoint3d>());
        auto& pts = result.back();

        while (startPnt != dtm.nullPnt)
            {
            pts.push_back(*pointAddrP(&dtm, startPnt));
            auto np = nodeAddrP(&dtm, startPnt)->tPtr;
            nodeAddrP(&dtm, startPnt)->tPtr = dtm.nullPnt;
            startPnt = np;
            }
        }

    static void FindPolygonsHelper(BC_DTM_OBJ& dtm, long startPnt, DTM_FEATURE_LIST* fListP, bvector<bvector<DPoint3d>>& result)
        {
        long p1 = startPnt;
        long p2 = fListP->nextPnt;
        if (p2 == dtm.nullPnt)
            return;
        while (true)
            {
            //if (p2 == dtm.nullPnt)
            //    {
            //    bcdtmList_cleanTptrPolygonDtmObject(&dtm, startPnt);
            //    break;
            //    }
            fListP->pntType = PTTYPE_MARK;
            if (nodeAddrP(&dtm, p1)->tPtr != dtm.nullPnt)
                {
                AddTPtrToResultAndClearTPtr(dtm, p1, result);
                }
            nodeAddrP(&dtm, p1)->tPtr = p2;

            long fList = nodeAddrP(&dtm, p2)->fPtr;
            fListP = flistAddrP(&dtm, fList);

            long np = fListP->nextPnt;

            if (fListP->nextPtr != dtm.nullPtr)
                {
                // There is more than 1 link for this point. find next Pnt clockwise.
                long tPnt = bcdtmList_nextClkDtmObject(&dtm, p2, p1);
                while(tPnt != p1)
                    {
                    if (bcdtmList_testForBreakLineDtmObjectQuick(&dtm, p2, tPnt))
                        break;
                    tPnt = bcdtmList_nextClkDtmObject(&dtm, p2, tPnt);
                    }
                np = tPnt;
                while (fListP->nextPnt != np)
                    {
                    if (fListP->nextPtr == dtm.nullPtr)
                        {
                        np = dtm.nullPnt;
                        break;
                        }
                    fListP = flistAddrP(&dtm, fListP->nextPtr);
                    }
                }

            if (np == dtm.nullPnt)
                {
                bcdtmList_nullTptrListDtmObject(&dtm, p1);
                break;
                }
            p1 = p2;
            p2 = np;
            if (p1 == startPnt)
                {
                AddTPtrToResultAndClearTPtr(dtm, p1, result);
                break;
                }
            }
        }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    static void FindPolygons(BC_DTM_OBJ& dtm, bvector<bvector<DPoint3d>>& result)
        {
        if (dtm.dtmState != DTMState::Tin)
            return;
        for (long ptNum = 0; ptNum < dtm.numPoints; ptNum++)
            {
            auto nodePP = nodeAddrP(&dtm, ptNum);
            auto fList = nodePP->fPtr;

            if (fList == dtm.nullPtr)
                continue;

            auto fListP = flistAddrP(&dtm, fList);

            while(fListP)
                {
                if (fListP->pntType != PTTYPE_MARK)
                    FindPolygonsHelper(dtm, ptNum, fListP, result);
                fList = fListP->nextPtr;
                if (fList == dtm.nullPtr)
                    break;
                fListP = flistAddrP(&dtm, fList);
                }
            }
        }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    static int bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject(BC_DTM_OBJ* dtmP, BC_DTM_OBJ* cleanedDtmP, std::set<long>& usedFeatureIndexes, DTMFeatureType featureType, bvector <DPoint3d>& points, DTMFeatureId dtmFeatureId)
        {
        bool useDtm = false;
        // If this polygon has 3 points or else then it isn't valid.
        if (points.size() <= 3)
            return DTM_ERROR;

        if (ftableAddrP(dtmP, *usedFeatureIndexes.begin())->dtmFeatureState == DTMFeatureState::Deleted)
            return DTM_SUCCESS;

        if (bcdtmObject_storeDtmFeatureInDtmObject(cleanedDtmP, featureType, cleanedDtmP->nullUserTag, 2, &dtmFeatureId, (DPoint3d*) &points[0], (long) points.size()))
            return DTM_ERROR;

        CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

        bvector<bvector<DPoint3d>> voids;
        for (auto featureNum : usedFeatureIndexes)
            {
            auto dtmFeatureP = ftableAddrP(dtmP, featureNum);
            // This feature was part of an invalid combination so just add this as is to the DTM.
            long numFeaturePts;
            DPoint3dP tFeaturePts = nullptr;
            if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP, featureNum, &tFeaturePts, &numFeaturePts))
                continue;
            if (nullptr != tFeaturePts)
                {
                if (tFeaturePts[0].IsEqual(tFeaturePts[numFeaturePts - 1]))
                    {
                    bvector<DPoint3d> pts;
                    for (int j = 0; j < numFeaturePts; j++)
                        pts.push_back(tFeaturePts[j]);
                    voids.push_back(pts);
                    }
                else
                    cv->Add(ICurvePrimitive::CreateLineString(tFeaturePts, numFeaturePts));
                free(tFeaturePts);
                tFeaturePts = nullptr;
                }
            }

        bvector<bvector<bvector<DPoint3d>>> regions;
        if (!cv->empty())
            {
            cv = cv->AssembleChains();

            for (auto& prim : *cv)
                {
                auto ccv = prim->GetChildCurveVectorP();
                ccv->CollectLinearGeometry(regions);
                for (auto& l1 : regions)
                    {
                    for (auto& l2 : l1)
                        voids.push_back(l2);
                    }
                }
            }

        cv = nullptr;

        for (auto&& pts : voids)
            {
            auto cv2 = CurveVector::CreateLinear(pts, CurveVector::BOUNDARY_TYPE_Outer);
            if (cv.IsNull())
                std::swap(cv, cv2);
            else
                {
                auto newCv = CurveVector::AreaUnion(*cv2, *cv);
                if (newCv.IsValid())
                    cv = newCv;
                }
            }

        BC_DTM_OBJ* tempDtm = nullptr;
        bcdtmObject_createDtmObject(&tempDtm);
        bvector<CurveVectorCP> outerLoops;
        if (cv->size() == 1)
            {
            outerLoops.push_back(cv.get());
            ExtractInnerLoops(*tempDtm, outerLoops);
            }
        else
            {
            for (auto& prim : *cv)
                {
                auto ccv = prim->GetChildCurveVectorCP();

                BeAssert(ccv != nullptr);

                if (nullptr != ccv)
                    {
                    if (ccv->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner)
                        {
                        for (auto& prim2 : *ccv)
                            {
                            auto ls = prim2->GetLineStringCP();
                            bcdtmObject_storeDtmFeatureInDtmObject(tempDtm, DTMFeatureType::Breakline, tempDtm->nullUserTag, 2, &dtmFeatureId, (DPoint3d*) ls->data(), (long) ls->size());
                            }
                        }
                    else
                        {
                        outerLoops.push_back(ccv);
                        //DebugCurveVector(*ccv);
                        }
                    }
                }
                ExtractInnerLoops(*tempDtm, outerLoops);
            }


        if (tempDtm->numFeatures != 0)
            {
            tempDtm->ppTol = 0.0;
            tempDtm->plTol = 0.0;
            bcdtmObject_createTinDtmObject(tempDtm, 1, 0.0, false);
            bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtm);
            //bcdtmWrite_toFileDtmObject(tempDtm, L"d:\\voidHulls.bcdtm");
            bvector<bvector<DPoint3d>> polygons;
            FindPolygons(*tempDtm, polygons);
            bcdtmObject_destroyDtmObject(&tempDtm);

            for (auto& l2 : polygons)
                {
                if (l2.front().IsEqual(l2.back()))
                    StoreInDTMFindEdges(dtmP, usedFeatureIndexes, l2, featureType != DTMFeatureType::Island ? DTMFeatureType::Island : DTMFeatureType::Void);
                }
            }
        return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTMUniqueFeatureItem
    {
    bvector<DPoint3d> pts;
    bvector<long> featuresNumbers;
    DTMFeatureType featureType;
    DRange3d range;
    };

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTMUniqueFeatureCollection : bvector<DTMUniqueFeatureItem>
    {
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    bool AddFeature(DPoint3d* pts, int numPoints, long featureNumber, DTMFeatureType featureType)
        {
        DRange3d range = DRange3d::From(pts, numPoints);

        for (auto& feature : *this)
            {
            if (feature.pts.size() != numPoints || feature.featureType != featureType)
                continue;

            if (!feature.range.IsEqual(range))
                continue;

            int startpoint = -1;

            for (int i = 0; i < (int)feature.pts.size(); i++)
                {
                if (pts[0].IsEqual(feature.pts[i]))
                    {
                    startpoint = i;
                    break;
                    }
                }

            if (startpoint == -1)
                continue;

            bool isDifferent = false;
            for (int i = 1; i < numPoints; i++)
                {
                long testpoint = (startpoint + i) % numPoints;
                if (!pts[i].IsEqual(feature.pts[testpoint]))
                    {
                    isDifferent = true;
                    break;
                    }
                }

            if (isDifferent)
                {
                isDifferent = false;
                for (int i = 1; i < numPoints; i++)
                    {
                    long testpoint = (startpoint + numPoints - i) % numPoints;
                    if (!pts[i].IsEqual(feature.pts[testpoint]))
                        {
                        isDifferent = true;
                        break;
                        }
                    }
                if (!isDifferent)
                    isDifferent = false;
                }

            if (isDifferent)
                continue;

            feature.featuresNumbers.push_back(featureNumber);
            return false;
            }
        DTMUniqueFeatureItem item;
        item.range = range;
        item.featuresNumbers.push_back(featureNumber);
        item.featureType = featureType;
        item.pts.resize(numPoints);
        memcpy(item.pts.data(), pts, sizeof(pts[0]) * numPoints);
        this->push_back(item);
        return true;
        }
    };

struct UnionTests
    {
    DRange3d range;
    CurveVectorPtr cv;
    bool addFeature;
    BC_DTM_FEATURE* feature;
    int parent;
    bvector<int> children;
    };

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
//int FindParentOld(UnionTests& childTest, bvector<UnionTests>& tests)
//    {
//    for (int i = (int) tests.size() - 1; i >= 0; i--)
//        {
//        auto& item2 = tests[i];
//
//        auto ucv = CurveVector::AreaIntersection(*childTest.cv, *item2.cv);
//        if (ucv.IsValid() && ucv->size() == 1)
//            {
//            return i;
//            }
//        }
//    return -1;
//    }

int FindParent(UnionTests& childTest, bvector<UnionTests>& tests, const bvector<int>& children)
    {
    if (!children.empty())
        {
        for (int childIndex : children)
            {
            auto& item2 = tests[childIndex];
            bool testArea = childTest.range.IsContained(item2.range);

            if (testArea)
                {
                int parent = FindParent(childTest, tests, item2.children);
                if (parent != -1)
                    return parent;

                DPoint3d testPt;
                childTest.cv->GetStartPoint(testPt);
                auto inOut = item2.cv->PointInOnOutXY(testPt);

                if (inOut == CurveVector::InOutClassification::INOUT_In)
                    return childIndex;
                else if (inOut == CurveVector::InOutClassification::INOUT_On)
                    {
                    auto ucv = CurveVector::AreaIntersection(*childTest.cv, *item2.cv);
                    bool isAreaIn = (ucv.IsValid() && ucv->size() == 1);
                    if (isAreaIn)
                        return childIndex;
                    }

                //if (inOut != CurveVector::InOutClassification::INOUT_Out)
                //    testArea = true;
                //else
                //    testArea = false;

                //if (testArea)
                //    {
                //    auto ucv = CurveVector::AreaIntersection(*childTest.cv, *item2.cv);
                //    bool isAreaIn = (ucv.IsValid() && ucv->size() == 1);
                //    if (isAreaIn)
                //        return childIndex;
                //    }
                }
            }
        }
    return -1;
    }

void CollectInternalConnectingVoids(BC_DTM_OBJ* dtmP, std::set<long>& usedFeatureIndexes)
    {
    std::queue<long> features;

    for (auto feature : usedFeatureIndexes)
        features.push(feature);

    bool isLookingForVoids = (DTMFeatureType)(ftableAddrP(dtmP, features.back())->dtmUserTag) != DTMFeatureType::Island;
    while (!features.empty())
        {
        long featureNum = features.front();
        features.pop();
        
        long firstPnt = ftableAddrP(dtmP, featureNum)->dtmFeaturePts.firstPoint;
        long p1 = firstPnt;
        do
            {
            long p2;
            bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP, featureNum, p1, &p2);
            if (p2 == dtmP->nullPnt)
                break;
            if (nodeAddrP(dtmP, p1)->tPtr != p2)
                {
                // loop all links, and find all voids, goind the same way, if the feature hasn't been added, add it.
                // Might beable to use TPtr to skip ones which are on the edge.
                std::set<long> newFeatures;
                //bcdtmList_getDtmFeatureNumsLineDtmObject(dtmP, p1, p2, newFeatures, isLookingForVoids);
                bcdtmList_getDtmFeatureNumsLineDtmObject(dtmP, p2, p1, newFeatures, isLookingForVoids);
                for (auto newFeatureNum : newFeatures)
                    {
                    if (usedFeatureIndexes.find(newFeatureNum) == usedFeatureIndexes.end())
                        {
                        usedFeatureIndexes.insert(newFeatureNum);
                        features.push(newFeatureNum);
                        }
                    }
                }
            p1 = p2;
            } while (p1 != firstPnt);
        }
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_resolveMultipleIntersectingPolygonalDtmObject
(
 BC_DTM_OBJ *dtmP
 )
 /*
 ** This Function Resolves Multiple Intersecting Polygonal Features
 */
    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature,numFeatureTypes=0,numFeaturePts=0 ;
    DPoint3d    *featurePtsP=NULL ;
    BC_DTM_OBJ *polyDtmP=NULL ;
    BC_DTM_OBJ *cleanedDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    char     dtmFeatureTypeName[50];
    int numVoids;
    long sp, np = 0, hp, ss;
    long numStartFeatures;
    bool isLookingForVoids = true;
    DTMUniqueFeatureCollection uniqueFeatures;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        }
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Check For Valid Feature Types
    */
    //      case DTMFeatureType::Void :
    //      case DTMFeatureType::BreakVoid :
    //      case DTMFeatureType::DrapeVoid :
    //      case DTMFeatureType::Island :
    //      case DTMFeatureType::Hole :
    /*
    ** Count Number Of Feature Types In DTM
    */
     for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if ((dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island))
            {
            if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP, dtmFeature, &featurePtsP, &numFeaturePts)) goto errexit;
            uniqueFeatures.AddFeature(featurePtsP, numFeaturePts, dtmFeature, dtmFeatureP->dtmFeatureType);
            free(featurePtsP);
            featurePtsP = nullptr;
            }
            numFeatureTypes = (long)uniqueFeatures.size();
        }
    /*
    ** Only Process If There Are More Than One Occurrence Of The Feature Type
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numFeatureTypes = %8ld",numFeatureTypes) ;
    if( numFeatureTypes > 1 )
        {
        /*
        **  Create Temporary Object To Store Feature Occurrences
        */
        if( bcdtmObject_createDtmObject(&polyDtmP))
            goto errexit ;

        /*
        **  Create Temporary Object to Store the Cleaned FeaturesResult
        */
        if( bcdtmObject_createDtmObject(&cleanedDtmP)) goto errexit ;
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(polyDtmP,10000,10000) ;
        polyDtmP->ppTol = dtmP->ppTol ;
        polyDtmP->plTol = dtmP->plTol ;
        /*
        **  Move Features To Temporary DTM
        */
        for (auto& feature : uniqueFeatures)
            {
            DTMFeatureId dtmFeatureId = (DTMFeatureId)feature.featuresNumbers.front();

            if (feature.pts.front().IsEqual(feature.pts.back()))
                {
                if (bcdtmObject_storeDtmFeatureInDtmObject(polyDtmP, DTMFeatureType::Breakline, (DTMUserTag)feature.featureType, 2, &dtmFeatureId, feature.pts.data(), (long)feature.pts.size()))
                    {
                    goto errexit;
                    }
                }
            else
                BeAssert(false);

            }

        /*
        **   Loop till they are no features.
        */
        bool changed = false;
        do
            {
            changed = false;
            polyDtmP->ppTol = 0.0 ;
            polyDtmP->plTol = 0.0 ;
            if (bcdtmObject_createTinDtmObject(polyDtmP, 1, 0.0, false))
                break;

//            bcdtmList_nullTptrValuesDtmObject (polyDtmP);

            /*
            ** Remove None Feature Hull Lines
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
            if( bcdtmList_removeNoneFeatureHullLinesDtmObject(polyDtmP)) goto errexit ;
            if (dbg) bcdtmWrite_toFileDtmObject(polyDtmP, L"d:\\voidHulls.bcdtm");
            /*
            ** Report DTM Stats
            */
            if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(polyDtmP) ;

            /*
            ** Set Pointer To Last Feature In Tin
            */
            numStartFeatures = dtmP->numFeatures ;
            /*
            ** Scan Tin Hull To Get Voids
            */
            if( dbg )bcdtmWrite_message(0,0,0,"Scanning For External %s",dtmFeatureTypeName) ;
            numVoids = 0 ;
            sp = polyDtmP->hullPoint ;
            do
                {
                np = nodeAddrP(polyDtmP,sp)->hPtr ;
                if (nodeAddrP(polyDtmP, sp)->tPtr == polyDtmP->nullPnt && bcdtmList_testForBreakLineDtmObject(polyDtmP, sp, np, isLookingForVoids))
                    {
                    bvector<DPoint3d> points;
                    std::set<long> usedFeatureIndexes;

                    /*
                    **        Scan Around External Edge Of Break Lines and get the Feature Numbers which are on this feature.
                    */
//                    bcdtmList_getDtmFeatureNumsLineDtmObject (polyDtmP, sp,np, usedFeatureIndexes);
                    hp = sp ;
                    nodeAddrP(polyDtmP,sp)->tPtr = np ;
                    do
                        {
                        if( ( hp = bcdtmList_nextAntDtmObject(polyDtmP,np,hp)) < 0 ) goto errexit ;
                        while (!bcdtmList_testForBreakLineDtmObject(polyDtmP, np, hp, isLookingForVoids))
                            {
                            if( ( hp = bcdtmList_nextAntDtmObject(polyDtmP,np,hp)) < 0 ) goto errexit ;
                            }

                        if (nodeAddrP(polyDtmP, np)->tPtr != polyDtmP->nullPnt)
                            bcdtmList_nullTptrListDtmObject(polyDtmP, np);
                        nodeAddrP(polyDtmP,np)->tPtr = hp ;
                        ss = hp ;
                        hp = np ;
                        np = ss ;
                        } while ( np != sp ) ;
                        /*
                        **        Check Connectivity
                        */
                        if( bcdtmList_checkConnectivityTptrListDtmObject(polyDtmP,sp,1)) goto errexit ;
                        if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(polyDtmP,sp) ;
                        if( dbg == 2 ) bcdtmList_writeVectorList (usedFeatureIndexes);

                            {
                            int tp = sp;
                            int np = 0;
                            points.push_back( *pointAddrP(polyDtmP, sp));
                            while ( nodeAddrP(polyDtmP,tp)->tPtr != polyDtmP->nullPnt && nodeAddrP(polyDtmP,tp)->tPtr >= 0 )
                                {
                                np = nodeAddrP(polyDtmP,tp)->tPtr ;
                                nodeAddrP(polyDtmP,tp)->tPtr = -(np+1) ;
                                points.push_back( *pointAddrP(polyDtmP, np));
                                bcdtmList_getDtmFeatureNumsLineDtmObject(polyDtmP, tp, np, usedFeatureIndexes, isLookingForVoids);
                                tp = np ;
                                }
                            bcdtmList_getDtmFeatureNumsLineDtmObject(polyDtmP, tp, np, usedFeatureIndexes, isLookingForVoids);
                            /*
                            **  Reset Tptr Values Positive
                            */
                            tp = sp ;
                            while( nodeAddrP(polyDtmP,tp)->tPtr < 0  )
                                {
                                np = -(nodeAddrP(polyDtmP,tp)->tPtr + 1 ) ;
                                nodeAddrP(polyDtmP,tp)->tPtr = np ;
                                tp = np ;
                                }
                            }


                        /*
                        **        Store Void Feature In Tin
                        */
                        if (usedFeatureIndexes.size () == 1)
                            {
                            /*
                            ** As they are only one feature which made up this element, it must be closed so add this element to the cleaned DTM as it should be the same as the original.
                            */
                            dtmFeatureP = ftableAddrP(polyDtmP, *usedFeatureIndexes.begin());
                            DTMFeatureType useFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag;
                            // If this is a type we are looking for then process it otherwise it will be processed on the next go round.
                            if ((useFeatureType == DTMFeatureType::Island && !isLookingForVoids) || (useFeatureType != DTMFeatureType::Island && isLookingForVoids))
                                {
                                dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                                // If the dtmFeatureId is negative this is a result of a hole in more than one feature.
                                if (dtmFeatureP->dtmFeatureId < 0)
                                    {
                                    dtmFeatureP = ftableAddrP(dtmP, -1 - (long)dtmFeatureP->dtmFeatureId);
                                    if( bcdtmObject_storeDtmFeatureInDtmObject (cleanedDtmP,useFeatureType, -999 ,2,&dtmFeatureP->dtmFeatureId,(DPoint3d*)&points[0],(long)points.size())) goto errexit ;
                                    }
                                else
                                    {
                                    dtmFeatureP = ftableAddrP(dtmP, (long)dtmFeatureP->dtmFeatureId);
                                    if( bcdtmObject_storeDtmFeatureInDtmObject (cleanedDtmP,useFeatureType, dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,(DPoint3d*)&points[0],(long)points.size())) goto errexit ;
                                    }
                                }
                            changed = true;
                            }
                        else
                            {
                            /*
                            ** OK This polygon feature is made from multiple features.
                            ** See if they are made from the same feature Type.
                            ** Ignore Islands if they are voids, so we can have an island on the edge of a void. (disabled for now as the engine doesn't like this.)
                            */
                            DTMFeatureType voidFeatureType = DTMFeatureType::None;
                            bool valid = false;
                            int numVoids = 0;
                            int numIslands = 0;
//                            int numHoles = 0;

                            CollectInternalConnectingVoids(polyDtmP, usedFeatureIndexes);
                            dtmFeatureP = ftableAddrP(polyDtmP, *usedFeatureIndexes.begin());
                            dtmFeatureP = ftableAddrP(dtmP, dtmFeatureP->dtmFeatureId < 0 ? -1 - (long)dtmFeatureP->dtmFeatureId : (long)dtmFeatureP->dtmFeatureId);
                            DTMFeatureId useFeatureId = dtmFeatureP->dtmFeatureId;

                            for (auto featureNum : usedFeatureIndexes)
                                {
                                dtmFeatureP = ftableAddrP(polyDtmP, featureNum) ;
                                DTMFeatureType dtmFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag;
                                if (dtmFeatureType == DTMFeatureType::Island)
                                    numIslands++;
//                                else if (dtmFeatureType == DTMFeatureType::Hole)
//                                    numHoles++;
                                else
                                    {
                                    numVoids++;
                                    if (voidFeatureType == DTMFeatureType::None)
                                        {
                                        valid = true;
                                        voidFeatureType = dtmFeatureType;
                                        }
                                    else
                                        {
                                        if (voidFeatureType != dtmFeatureType)
                                            valid = false;
                                        }
                                    }
                                }

                            if (!isLookingForVoids)
                                {
                                if (numIslands == 0)
                                    {
                                    valid = true;   // Don't care that there are a mixure as they are in voids anyway.
                                    }
                                else
                                    {
                                    if (numVoids == 0)
                                        {
                                        bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject (polyDtmP, cleanedDtmP, usedFeatureIndexes, DTMFeatureType::Island, points, useFeatureId);
                                        }
                                    else
                                        {
                                        // Mixure of voids and islands. Need to get the island and clip it with the island.
                                        // Fail for now.
                                        valid = false;
                                        }
                                    }
                                }
                            else
                                {
                                if (numVoids == 0)
                                    {
                                    // Islands not in a void.
                                    valid = true;
                                    }
                                else
                                    {
                                    if (numIslands == 0)
                                        {
                                        bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject (polyDtmP, cleanedDtmP, usedFeatureIndexes, voidFeatureType, points, useFeatureId);
                                        }
                                    else
                                        {
                                        // Mixure of voids and islands. Need to get the island and clip it with the island.
                                        // Fail for now.
                                        valid = false;
                                        }
                                    }
                                }

                            changed = true;
                            for (auto featureNum : usedFeatureIndexes)
                                {
                                dtmFeatureP = ftableAddrP(polyDtmP, featureNum) ;

                                //if (!valid)
                                //    {
                                //    // This feature was part of an invalid combination so just add this as is to the DTM.
                                //    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(polyDtmP,usedFeatureIndexes [i], &featurePtsP,&numFeaturePts)) goto errexit ;
                                //    if( bcdtmObject_storeDtmFeatureInDtmObject(cleanedDtmP,dtmFeatureP->dtmFeatureType, dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId, featurePtsP, numFeaturePts)) goto errexit;
                                //    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                                //    }
                                dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                                }

                            }
                        ++numVoids ;
                    }
                sp = nodeAddrP(polyDtmP,sp)->hPtr ;
                } while ( sp != polyDtmP->hullPoint ) ;
                isLookingForVoids = !isLookingForVoids;
                /*
                **  Remove Deleted Features
                */
                if( bcdtmData_compactFeatureTableDtmObject(polyDtmP))
                    goto errexit ;

                if (polyDtmP->numFeatures == 0)
                    break;
                if ( bcdtmObject_changeStateDtmObject(polyDtmP,DTMState::Data))
                    goto errexit ;
            } while (changed);

            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island))
                    {
                    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                    }
                }

            bvector<int> topLevel;
            bvector<UnionTests> uf;
            /*
            **  Copy Intersected Features To DTM
            */
            for( dtmFeature = 0 ; dtmFeature < cleanedDtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(cleanedDtmP,dtmFeature) ;
                if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                    {
                    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(cleanedDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                    UnionTests item;
                    item.feature = dtmFeatureP;
                    item.cv = CurveVector::CreateLinear(featurePtsP, numFeaturePts, CurveVector::BOUNDARY_TYPE_Outer);
                    item.cv->GetRange(item.range);
                    item.range.low.z = 0;
                    item.range.high.z = 0;
                    if (featurePtsP != NULL) { free(featurePtsP); featurePtsP = NULL; }
                    item.addFeature = true;
                    item.parent = -1;

                    //int parentOld = FindParentOld(item, uf);
                    item.parent = FindParent(item, uf, topLevel);
                    //if (item.parent != parentOld)
                    //    parentOld = item.parent;

                    if (item.parent != -1)
                        {
                        auto& item2 = uf[item.parent];
                        if ((item2.feature->dtmFeatureType == DTMFeatureType::Island && item.feature->dtmFeatureType == DTMFeatureType::Island) || (item2.feature->dtmFeatureType != DTMFeatureType::Island && item.feature->dtmFeatureType != DTMFeatureType::Island))
                            item.addFeature = false;
                        else
                            {
                            if (item2.range.IsEqual(item.range))
                                {
                                auto parity = CurveVector::AreaParity(*item2.cv, *item.cv);

                                if (parity.IsNull() || parity->size() == 0)
                                    {
                                    item.addFeature = false;
                                    item2.addFeature = false;
                                    }
                                }
                            }
                        if (item.addFeature)
                            item2.children.push_back((int) uf.size());
                        }
                    else
                        topLevel.push_back((int) uf.size());
                    if (item.addFeature)
                        uf.push_back(item);
                    }
                }

            for (auto& item : uf)
                {
                if (item.addFeature)
                    {
                    auto ls = (*item.cv)[0]->GetLineStringCP();
                    if (bcdtmObject_storeDtmFeatureInDtmObject(dtmP, item.feature->dtmFeatureType, item.feature->dtmUserTag, 2, &item.feature->dtmFeatureId, ls->data(), (int)ls->size())) goto errexit;
                    }
                }
        }
        if (dbg) bcdtmWrite_toFileDtmObject(cleanedDtmP, L"d:\\cleaned1.bcdtm");
        if (dbg) bcdtmWrite_toFileDtmObject(dtmP, L"d:\\cleaned.bcdtm");
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( polyDtmP    != NULL )   bcdtmObject_destroyDtmObject(&polyDtmP) ;
    if( cleanedDtmP != NULL )   bcdtmObject_destroyDtmObject(&cleanedDtmP) ;
    /*
    ** Return
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Error") ;
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
int bcdtmClean_validateDtmFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,                        // ==> Dtm Object
 long       forceClose,                   // ==> Force Close Polygonal DTM Features
 double     closeTolerance,               // ==> Close Tolerance For DTM Features
 double     filterTolerance,              // ==> Filter Tolerance For Dtm Feature Points
 int        onlyValidatePolygonalFeatures,// ==> Only check PolygonalFeatures
 long       *numErrorsP                   // <== Number Of Features With Errors
)
/*
** This Function Cleans Dtm Features In A Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,ofs3,ofs4,point,dtmFeature,dtmFeature2,numFeaturePts=0,closeFlag;
 long   numFeatures=0,validateResult,polygonalFeature=FALSE,numStartFeatures ;
 DPoint3d    *p3dP,*featurePtsP=NULL ;
 DPoint3d  *pointP,*point1P,*point2P ;
 BC_DTM_FEATURE *dtmFeatureP, *dtmFeature2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Validating Dtm Features") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"closeTolerance  = %12.8lf",closeTolerance) ;
    bcdtmWrite_message(0,0,0,"filterTolerance = %12.8lf",filterTolerance) ;
   }
/*
** Initialise
*/
 *numErrorsP = 0 ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulted DTM") ;
    goto errexit ;
   }
/*
** Scan Dtm Features For Dtm Feature Type
*/
 numStartFeatures = dtmP->numFeatures ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Only Process Data Features
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
       validateResult = 0;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Validating DTM Feature[%8ld] ** Number Of Feature Points = %6ld",dtmFeature,dtmFeatureP->numDtmFeaturePts) ;
       ++numFeatures ;
/*
**     Check For Polygonal Feature
*/
       polygonalFeature = FALSE ;
       if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island     ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon    ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region     ) polygonalFeature = TRUE ;

       if (!polygonalFeature || onlyValidatePolygonalFeatures)
           continue;
/*
**     Allocate Memory For Feature Points
*/
       numFeaturePts = dtmFeatureP->numDtmFeaturePts ;
       featurePtsP = ( DPoint3d * ) malloc ( numFeaturePts * sizeof(DPoint3d)) ;
       if( featurePtsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Copy Feature Points To Point Array
*/
       ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
       ofs2 = ofs1 + dtmFeatureP->numDtmFeaturePts - 1 ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Feature Start Offset = %6ld Feature End Offset = %6ld",ofs1,ofs2) ;
       for( point = ofs1 , p3dP = featurePtsP ; point <= ofs2 ; ++point , ++p3dP )
         {
          pointP = pointAddrP(dtmP,point) ;
          p3dP->x = pointP->x ;
          p3dP->y = pointP->y ;
          p3dP->z = pointP->z ;
         }
/*
**     Check For Closure
*/
       closeFlag = 0 ;
       if( featurePtsP->x == (featurePtsP+numFeaturePts-1)->x && featurePtsP->y == (featurePtsP+numFeaturePts-1)->y ) closeFlag = 1 ;
       if( dbg == 2 && ! closeFlag ) bcdtmWrite_message(0,0,0,"Open Feature") ;
       if( dbg == 2 &&   closeFlag ) bcdtmWrite_message(0,0,0,"Closed Feature") ;
/*
**     Check For Closure Within Close Tolerance
*/
       if( !closeFlag && bcdtmMath_distance(featurePtsP->x,featurePtsP->y,(featurePtsP+numFeaturePts-1)->x,(featurePtsP+numFeaturePts-1)->y) <= closeTolerance )
         {
          featurePtsP->x = (featurePtsP+numFeaturePts-1)->x ;
          featurePtsP->y = (featurePtsP+numFeaturePts-1)->y ;
          featurePtsP->z = (featurePtsP+numFeaturePts-1)->z ;
          closeFlag = 1 ;
         }
/*
**     Close Polygonal Features
*/
       if (polygonalFeature == TRUE && !closeFlag && forceClose)
           {
           numFeaturePts++;
           featurePtsP = (DPoint3dP)realloc(featurePtsP, numFeaturePts * sizeof(DPoint3d));
           *(featurePtsP + numFeaturePts - 1) = *featurePtsP;

           long IntFlag = 0;
           if (bcdtmData_checkPolygonForKnots(featurePtsP, numFeaturePts, &IntFlag)) goto errexit;
           if ((IntFlag & 1) != 1)
               closeFlag = 1;
           }
/*
**     Check For Close Error
*/
       if( polygonalFeature && ! closeFlag )
         {
         validateResult = 1;
          if( dbg ) bcdtmWrite_message(0,0,0,"Close Error In Feature") ;
         }
/*
**     Validate Feature Points
*/
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points Before Validate = %6ld",numFeaturePts) ;
          if( ! closeFlag ) validateResult = bcdtmClean_validateStringP3D(&featurePtsP,&numFeaturePts,filterTolerance) ;
          else
              {
              validateResult = bcdtmClean_validatePointArrayPolygon(&featurePtsP,&numFeaturePts,1,filterTolerance) ;
              if (!validateResult)
            {
                 DTMDirection direction;
                 double area;
                 if (bcdtmMath_getPolygonDirectionP3D (featurePtsP, numFeaturePts, &direction, &area) || area < 1e-8)
                            validateResult = 1;
                    if (area < 1e-8)
                        validateResult = 1;
            }
              }

/*
**        If No Validation Errors Store Validated Points
*/
            if( !validateResult)
            {
/*
**           Expand for New Points
*/
             if( numFeaturePts > dtmFeatureP->numDtmFeaturePts )
               {
                ofs3 = numFeaturePts - dtmFeatureP->numDtmFeaturePts;

                ofs2 = dtmP->numPoints - 1;
                ofs4 = dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1;
                dtmP->numPoints += ofs3;
                if (dtmP->numPoints > dtmP->memPoints)
                    {
                    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit  ;
                    }

                // increase the size of points
                ofs1 = dtmP->numPoints - 1;
                while( ofs2 > ofs4 )
                  {
                   point1P = pointAddrP(dtmP,ofs1) ;
                   point2P = pointAddrP(dtmP,ofs2) ;
                   *point1P = *point2P ;
                   --ofs1 ;
                   --ofs2 ;
                  }
                dtmFeatureP->numDtmFeaturePts = numFeaturePts ;
/*
**              Adjust First Point Offset For Remaing Features
*/
                for( dtmFeature2 = dtmFeature + 1 ; dtmFeature2 < dtmP->numFeatures ; ++dtmFeature2 )
                  {
                   dtmFeature2P = ftableAddrP(dtmP,dtmFeature2) ;
                   if( dtmFeature2P->dtmFeatureState == DTMFeatureState::Data ) dtmFeature2P->dtmFeaturePts.firstPoint += ofs3 ;
                  }
               ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
               }

             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points  After Validate = %6ld",numFeaturePts) ;
             for( point = ofs1 , p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++point , ++p3dP )
               {
                pointP = pointAddrP(dtmP,point) ;
                pointP->x = p3dP->x ;
                pointP->y = p3dP->y ;
                pointP->z = p3dP->z ;
               }
/*
**           Copy Over Deleted Points
*/
             if( numFeaturePts < dtmFeatureP->numDtmFeaturePts )
               {
                ofs3 = dtmFeatureP->numDtmFeaturePts - numFeaturePts ;
                dtmFeatureP->numDtmFeaturePts = numFeaturePts ;
                ofs1 = ofs1 + numFeaturePts ;
                ++ofs2 ;
                while( ofs2 < dtmP->numPoints )
                  {
                   point1P = pointAddrP(dtmP,ofs1) ;
                   point2P = pointAddrP(dtmP,ofs2) ;
                   *point1P = *point2P ;
                   ++ofs1 ;
                   ++ofs2 ;
                  }
                dtmP->numPoints = ofs1 ;
/*
**              Adjust First Point Offset For Remaing Features
*/
                for( ofs1 = dtmFeature + 1 ; ofs1 < dtmP->numFeatures ; ++ofs1 )
                  {
                   dtmFeatureP = ftableAddrP(dtmP,ofs1) ;
                   if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) dtmFeatureP->dtmFeaturePts.firstPoint -= ofs3 ;
                  }
               }
/*
**           Set Polygonal Feature Anti Clockwise
*/
             if( polygonalFeature == TRUE )
               {
                if( bcdtmClean_setDtmPolygonalFeatureAntiClockwiseDtmObject(dtmP,dtmFeature)) goto errexit ;
               }
            }
                if( validateResult)
                    {
                    ++*numErrorsP ;
                    if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId))
                        goto errexit;

                    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                    dtmFeatureP->numDtmFeaturePts = 0 ;

                    if( dbg ) bcdtmWrite_message(0,0,0,"Validation Errors In Feature") ;
                    }
                }
         }
/*
**     Free Feature Points Memory
*/
       free(featurePtsP) ; featurePtsP = NULL ;
      }
/*
** Log Number Of Errors
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number With Errors = %8ld of %8ld",*numErrorsP,numFeatures) ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP  != NULL ) { free(featurePtsP)  ; featurePtsP = NULL  ; }
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating DTM Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating DTM Feature Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}



int bcdtmCleanUp_joinVoidsAndHoles (BC_DTM_OBJ *dtmP)
    {
    int ret = DTM_SUCCESS;
    long numBeforeJoin;
    long numAfterJoin;
    DTM_JOIN_USER_TAGS* joinUserTagsP = NULL;
    long numJoinUserTags = 0;
    DTMRollbackData* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::VoidLine, DTMFeatureType::Void, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::HoleLine, DTMFeatureType::Hole, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;
    /*
    ** Clean Up
    */
cleanup :
    if (joinUserTagsP) free (joinUserTagsP);
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

int bcdtmCleanUp_joinHullLines (BC_DTM_OBJ *dtmP)
    {
    int ret = DTM_SUCCESS;
    int dtmFeature;
    long numBeforeJoin;
    long numAfterJoin;
    DTM_JOIN_USER_TAGS* joinUserTagsP = NULL;
    long numJoinUserTags = 0;
    DTMRollbackData* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::HullLine, DTMFeatureType::HullLine, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;

    if (numAfterJoin == 1)
        {
        BC_DTM_FEATURE *dtmFeatureP;
        /*
        ** Scan Dtm Features For Dtm Feature Type
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                {
                if (dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine)
                    {
                    long ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    long ofs2 = ofs1 + dtmFeatureP->numDtmFeaturePts - 1 ;
                    DPoint3d* startPtP = pointAddrP(dtmP,ofs1) ;
                    DPoint3d* endPtP = pointAddrP(dtmP,ofs2) ;
                    if( bcdtmMath_distance(startPtP->x,startPtP->y,endPtP->x,endPtP->y) <= dtmP->ppTol)
                        dtmFeatureP->dtmFeatureType = DTMFeatureType::Hull;
                    break;
                    }
                }
            }
        }

    /*
    ** Clean Up
    */
cleanup :
    if (joinUserTagsP) free (joinUserTagsP);
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
int bcdtmCleanUp_checkHullFeatures (BC_DTM_OBJ* dtmP)
    {
    int    ret=DTM_SUCCESS,dbg=0;
    long dtmFeature;
    int numHulls = 0;
    int numDrapeHulls= 0;
    int numHullLines = 0;
    bool removeDrapeHulls = false;
    bool removeHullLines = false;
    int numRemoved = 0;
    BC_DTM_FEATURE *dtmFeatureP;
    int n;
    DTMMemPnt featPtsPI = 0;
    DPoint3d     *featPtsP=NULL, *pntP=NULL;

    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"bcdtmCleanUp_checkboundayFeatures") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        }
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Data State
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulted DTM") ;
        goto errexit ;
        }
    /*
    ** Scan Dtm Features For Dtm Feature Type
    */
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
            {
            if (dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull)
                numDrapeHulls++;
            else if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull)
                numHulls++;
            else if (dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine)
                numHullLines++;
            }
        }

    if (numDrapeHulls != 0 && numHulls != 0)
        removeDrapeHulls = true;

    if (numHullLines != 0 && numHulls != 0 || numDrapeHulls != 0)
        removeHullLines = true;

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"numHulls = %d", numHulls) ;
        bcdtmWrite_message(0,0,0,"numDrapeHulls = %d", numDrapeHulls) ;
        bcdtmWrite_message(0,0,0,"numHullLines = %d", numHullLines) ;
        bcdtmWrite_message(0,0,0,"removeDrapeHulls = %d", removeDrapeHulls) ;
        bcdtmWrite_message(0,0,0,"removeHullLines = %d", removeHullLines) ;
        }

    if (removeDrapeHulls || removeHullLines)
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                {
                if ((removeDrapeHulls && dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull) ||
                    (removeHullLines && dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine))
                    {
                    numRemoved++;
                    if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data)
                        {
                        if( dbg ) bcdtmWrite_message(0,0,0,"Feature Insert Error") ;
                        featPtsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d));
                        featPtsP  = bcdtmMemory_getPointerP3D(dtmP, featPtsPI);
                        if( featPtsP == NULL )
                            {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                            }

                        for( n = 0 , pntP = featPtsP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
                         {
                          int point  = dtmFeatureP->dtmFeaturePts.firstPoint + n ;
                          DPoint3d* pointP = (DPoint3d*)pointAddrP (dtmP, point) ;
                              *pntP  = *pointP ;
                         }
                        dtmFeatureP->dtmFeaturePts.pointsPI = featPtsPI ;
                        featPtsP = NULL ;
                        }
                    dtmFeatureP->dtmFeatureState = DTMFeatureState::TinError ;
                    }
                }
            }
        }

    if (numHullLines != 0 && numHulls == 0 || numDrapeHulls == 0)
        bcdtmCleanUp_joinHullLines (dtmP);

    if (dbg)
        bcdtmWrite_message (0,0,0, "removed %d features", numRemoved);
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


int bcdtmCleanUp_resolveVoidAndIslandsDtmObject (BC_DTM_OBJ *dtmP)
    {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
    BC_DTM_OBJ* bndyDtmP = NULL;
    BC_DTM_FEATURE *dtmFeatureP;
    int dtmFeature;
    DPoint3d  *featurePtsP=NULL ;
    long numFeaturePts = 0;
    int k = 0;
//    DTMRollbackData* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    DTMFeatureId dtmFeatureId ;
    BC_DTM_FEATURE *dtmFeature2P=NULL ;
    int dtmFeature2 = 0;
    bvector<long> voidFeatureIndexes;
    int numVoids, numIslands;

    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Resolving Island Void Boundaries") ;
        bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
        }
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP))
        goto errexit  ;
    /*
    ** Check If Boundary DTM Is In Data State
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Un-Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Log DTM Stats
    */
    if( dbg == 1 )
        {
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        }

    if( bcdtmObject_createDtmObject(&bndyDtmP))
        goto errexit ;

    //  Copy Void Island Features

    if (dbg) bcdtmWrite_message(0,0,0,"Find all voids and Islands etc") ;

    numVoids = 0;
    numIslands = 0;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
            {
            if( dtmFeatureP->numDtmFeaturePts > 0 )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP,dtmFeature,&featurePtsP,&numFeaturePts))
                    goto errexit ;

                if (featurePtsP[0].x != featurePtsP[numFeaturePts - 1].x || featurePtsP[0].y != featurePtsP[numFeaturePts - 1].y)
                    {
                    if (dbg) bcdtmWrite_message (0,0,0,"None closed feature ignoring");
                    continue;
                    }
                if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid) ++numVoids ;
                else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
                int64_t featureId = dtmFeature;
                if( bcdtmObject_storeDtmFeatureInDtmObject(bndyDtmP,dtmFeatureP->dtmFeatureType, featureId, 2, &featureId, featurePtsP, numFeaturePts))
                    goto errexit ;
                voidFeatureIndexes.push_back (dtmFeature);
                }
            }
        }

    if (voidFeatureIndexes.size() != 0)
        {
        // call clean up
        /*
        **  Resolve Voids
        */
        if( numVoids > 1 || numIslands > 1 )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Voids") ;
            if( bcdtmCleanUp_resolveMultipleIntersectingPolygonalDtmObject (bndyDtmP))
                goto errexit ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Voids Resolved") ;
            }

        // Find all features which haven't changed.
        for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;

            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted)
                {
                if (dtmFeatureP->dtmUserTag == dtmFeatureP->dtmFeatureId && dtmFeatureP->dtmFeatureId >= 0)
                    {
                    for (unsigned int i = 0; i < voidFeatureIndexes.size(); i++)
                        {
                        if (voidFeatureIndexes[i] == dtmFeatureP->dtmUserTag)
                            {
                            voidFeatureIndexes[i] = -1;
                            break;
                            }
                        }
                    }
                }
            }

        // Add changed features as Rollback features.
        for (unsigned int i = 0; i < voidFeatureIndexes.size(); i++)
            {
            if (voidFeatureIndexes[i] >= 0)
                {
                dtmFeature2 = (long)voidFeatureIndexes[i];
                dtmFeature2P = ftableAddrP(dtmP, dtmFeature2) ;

                if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeature2P->dtmFeatureId))
                    goto errexit;
                if (dtmFeature2P->dtmFeatureState != DTMFeatureState::Data && dtmFeature2P->dtmFeatureState != DTMFeatureState::Tin)
                    {
                    if( dtmFeature2P->dtmFeaturePts.pointsPI != 0)
                        {
                        bcdtmMemory_free(dtmP,dtmFeature2P->dtmFeaturePts.pointsPI) ;
                        dtmFeature2P->dtmFeaturePts.pointsPI = 0;
                        }
                    }
                dtmFeature2P->dtmFeatureState = DTMFeatureState::Deleted;
                dtmFeature2P->numDtmFeaturePts = 0 ;
                }
            }
        }
        // Copy Features back to DTM reusing the Feature Id/UserTags.
        for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;

            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted)
                {
                if (dtmFeatureP->dtmUserTag == dtmFeatureP->dtmFeatureId && dtmFeatureP->dtmFeatureId >= 0)
                    {
                    // No need to do anything here.
                    }
                else
                    {
                    while (k < (int)voidFeatureIndexes.size())
                        {
                        long featureNum = voidFeatureIndexes[k++];
                        if (featureNum != -1)
                            {
                            dtmFeature2 = featureNum;
                            break;
                            }
                        };
                    dtmFeatureId = ftableAddrP(dtmP, dtmFeature2)->dtmFeatureId;
                    if (dbg) bcdtmWrite_message(0,0,0," ----- Adding cleanup void = %d %d %d", (long)dtmFeature2P->dtmFeatureId, (long)dtmFeature2P->dtmUserTag, dtmFeatureP->numDtmFeaturePts) ;
                    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(bndyDtmP,dtmFeature,&featurePtsP,&numFeaturePts))
                        goto errexit ;
                    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureP->dtmFeatureType,dtmFeature2P->dtmUserTag,2,&dtmFeatureId,featurePtsP,numFeaturePts))
                        goto errexit ;
                    }
                }
            }

    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP  != NULL ) free(featurePtsP) ;
    if( bndyDtmP     != NULL ) bcdtmObject_destroyDtmObject(&bndyDtmP) ;
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

static int (*bcdtmCleanUp_cleanDtmObjectOverrideP) (BC_DTM_OBJ *dtmP) = NULL;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_overrideCleanDtmObject (int (*overrideP) (BC_DTM_OBJ *dtmP))
    {
    bcdtmCleanUp_cleanDtmObjectOverrideP = overrideP;
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_cleanDtmObject (BC_DTM_OBJ *dtmP)
    {
    int dbg=DTM_TRACE_VALUE(0),ret=DTM_SUCCESS;
    long validationErrors;

    if (bcdtmCleanUp_cleanDtmObjectOverrideP)
        return bcdtmCleanUp_cleanDtmObjectOverrideP (dtmP);

    if ( bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::VoidsAndIslands))
        {
        bcdtmCleanUp_checkHullFeatures (dtmP);
        //  Join up Void Holes Line Features
        bcdtmCleanUp_joinVoidsAndHoles (dtmP);
        //  Validate Void Island Features
        if( dbg ) bcdtmWrite_message(0,0,0,"Validate Void Island Features") ;
        bcdtmClean_validateDtmFeaturesDtmObject (dtmP,1,dtmP->ppTol,dtmP->ppTol,0,&validationErrors);
        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Validation Errors = %8ld",validationErrors) ;
        if( validationErrors > 0 )
          {
           bcdtmWrite_message(0,0,0,"Errors In Void Island Features") ;
          }
        if (bcdtmCleanUp_resolveVoidAndIslandsDtmObject (dtmP)) goto errexit;
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
