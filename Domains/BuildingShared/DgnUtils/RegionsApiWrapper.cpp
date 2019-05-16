/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/BuildingDgnUtilsApi.h"

USING_NAMESPACE_BUILDING_SHARED

#define SHAPE_MAX_EDGE_LENGTH 1000.0

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Wouter.Rombouts 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
RegionsApiWrapper::RegionsApiWrapper
(
    Transform const& localToWorld
)
    : m_uorToMeter(1.0E-6) //RG works in UORS!
    , m_pRG(jmdlRG_new())
    , m_pCurves(jmdlRIMSBS_newContext())
    , m_localToWorldTransform(localToWorld)
    , m_worldToLocalTransform(localToWorld.ValidatedInverse())
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Wouter.Rombouts 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
RegionsApiWrapper::~RegionsApiWrapper()
    {
    jmdlRG_free(m_pRG);
    jmdlRIMSBS_freeContext(m_pCurves);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::ImprintPoly
(
    DPoint3d* pPointArray,
    size_t    pointArrayCount,
    int32_t   userInt32,
    int64_t   userInt64,
    void*     pUserData
)
    {
    bool status = true;

    if (pPointArray && (pointArrayCount > 1))
        {
        m_worldToLocalTransform.Multiply(pPointArray, pointArrayCount);

        for (size_t i = 0; i < pointArrayCount; i++)
            {
            pPointArray[i].z = 0.0;
            pPointArray[i].Scale (1.0 / m_uorToMeter);
            pPointArray[i].z = 0.0;
            }

        int parentCurveId = jmdlRIMSBS_addDataCarrier(m_pCurves, userInt32, pUserData);

        jmdlRIMSBS_setUserInt64(m_pCurves, parentCurveId, userInt64);

        if (!jmdlRG_addMaskedLinear(m_pRG, NULL, pPointArray, (int) pointArrayCount, parentCurveId, MTG_NULL_MASK, MTG_NULL_MASK))
            {
            status = false;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Wouter.Rombouts 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::ImprintCurveVector
(
    CurveVectorCR cv,
    int32_t userInt32,
    int64_t userInt64,
    void* pUserData
)
    {
    CurveVectorPtr cvFaceted = cv.Clone();
    cvFaceted->ConsolidateAdjacentPrimitives();

    IFacetOptionsPtr  facetOptions = IFacetOptions::Create();
    facetOptions->SetEdgeChainsRequired(true);
    facetOptions->SetMaxEdgeLength(SHAPE_MAX_EDGE_LENGTH);

    bvector<bvector<DPoint3d>> polyPoints;
    if (cvFaceted->CollectLinearGeometry(polyPoints, facetOptions.get()))
        {
        for (bvector<bvector<DPoint3d>>::iterator ppIter = polyPoints.begin(); ppIter != polyPoints.end(); ppIter++)
            {
            ImprintPoly(&(*ppIter)[0], (*ppIter).size(), userInt32, userInt64, pUserData);
            }
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::InferTopology
(
    void*     pAbortFunc,            /* => (NOT EXPOSED IN COM API=NULL) */
    double    vertexVertexTolerance, /* =>         */
    double    vertexEdgeTolerance    /* =>         */
)
    {
    if (pAbortFunc)
        {
        jmdlRG_setAbortFunction(m_pRG, (RGC_AbortFunction) pAbortFunc);
        }

    bool status = jmdlRG_mergeWithGapTolerance(m_pRG, vertexVertexTolerance, vertexEdgeTolerance);

    jmdlRG_setAbortFunction(m_pRG, NULL);

    status = status && jmdlRG_buildFaceRangeTree(m_pRG, 0.0, jmdlRG_getTolerance(m_pRG));
    status = status && jmdlRG_buildEdgeRangeTree(m_pRG, 0.0, jmdlRG_getTolerance(m_pRG));
    jmdlRG_buildFaceHoleArray(m_pRG);

    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::GetEndPoints
(
    MTGNodeId       edgeNode,
    DPoint3d*       pStartPoint, /* <= (NULL) start point (APICURRTRANS) */
    DPoint3d*       pEndPoint    /* <= (NULL) end point (APICURRTRANS) */
)
    {
    bool status = jmdlRG_getCurveData(m_pRG, NULL, NULL, pStartPoint, pEndPoint, edgeNode);

    if (pStartPoint)
        {
        pStartPoint->Scale (m_uorToMeter);
        m_localToWorldTransform.Multiply(*pStartPoint);
        }

    if (pEndPoint)
        {
        pEndPoint->Scale (m_uorToMeter);
        m_localToWorldTransform.Multiply(*pEndPoint);
        }

    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Nerijus.Jakeliunas                 09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionsApiWrapper::GetFacePoints
(
    bvector<DPoint3d>& points,
    MTGNodeId faceId
)
    {
    points.clear();
    MTGGraph* pGraph = jmdlRG_getGraph(m_pRG);
    MTGARRAY_FACE_LOOP(currId, pGraph, faceId)
        {
        DPoint3d end[2];
        GetEndPoints(currId, &end[0], &end[1]);
        
        if (points.empty())
            {
            points.push_back(end[0]);
            }

        points.push_back(end[1]);
        }
    MTGARRAY_END_FACE_LOOP(currId, pGraph, faceId)

    return points.empty() ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool RegionsApiWrapper::DoesFaceHaveHole
(
    MTGNodeId faceId
)
    {
    return jmdlRG_faceHasHoles(m_pRG, faceId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void RegionsApiWrapper::SearchFaceHoleArray
(
    EmbeddedIntArray* pHoleNodeIdArray, 
    MTGNodeId outerFaceNodeId
)
    {
    jmdlRG_searchFaceHoleArray(m_pRG, pHoleNodeIdArray, outerFaceNodeId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 09/14
+---------------+---------------+---------------+---------------+---------------+------*/
MTGNodeId RegionsApiWrapper::GetLowestFaceNodeId(MTGNodeId faceNode /* => subject */)
    {
    MTGNodeId id = INT_MAX;

    if (faceNode != MTG_NULL_NODEID)
        {
        MTGGraph* pBodyGraph = jmdlRG_getGraph(m_pRG);

        MTGARRAY_FACE_LOOP(currNodeId, pBodyGraph, faceNode)
            {
            if (currNodeId < id)
                {
                id = currNodeId;
                }
            }
        MTGARRAY_END_FACE_LOOP(currNodeId, pBodyGraph, faceNode)
        }

    return id;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::GetEdgeUserData(MTGNodeId edgeNode, int32_t& userInt32, int64_t& userInt64)
    {
    int parentIndex = 0;
    userInt64 = 0;
    userInt32 = 0;

    if (!jmdlRG_getParentCurveIndex(m_pRG, &parentIndex, edgeNode) || parentIndex < 0)
        {
        return false;
        }

    jmdlRIMSBS_getUserInt64(m_pCurves, &userInt64, parentIndex);
    jmdlRIMSBS_getUserInt(m_pCurves, &userInt32, parentIndex);

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 09/14
+---------------+---------------+---------------+---------------+---------------+------*/
MTGNodeId RegionsApiWrapper::GetParentFace(MTGNodeId faceNode /* => subject */)
    {
    MTGNodeId parentId = -1;
    MTGGraph* pBodyGraph = jmdlRG_getGraph(m_pRG);

    MTGARRAY_FACE_LOOP(currEdgeNodeId, pBodyGraph, faceNode)
        {
        MTGNodeId curMate = jmdlRG_skipNullFacesToEdgeMate(m_pRG, currEdgeNodeId);

        if (jmdlRG_faceIsHoleLoop(m_pRG, curMate))
            {
            EmbeddedIntArrayP pFaceArray = jmdlEmbeddedIntArray_grab();

            jmdlRG_resolveHoleNodeId(m_pRG, pFaceArray, curMate);

            jmdlEmbeddedIntArray_getInt(pFaceArray, &parentId, 0);

            jmdlEmbeddedIntArray_drop(pFaceArray);

            break;
            }
        }
    MTGARRAY_END_FACE_LOOP(currEdgeNodeId, pBodyGraph, faceNode)

    return parentId;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::GetFaceParentCurves
(
    MTGNodeId faceNode,
    bset<MTGNodeId>& addList,
    bmap<int64_t, CurveInfo>& isInsideIds,
    bmap<int64_t, CurveInfo>& isOutsideIds,
    int generation
)
    {
    MTGGraph* pBodyGraph = jmdlRG_getGraph(m_pRG);

    MTGARRAY_FACE_LOOP(currNodeId, pBodyGraph, faceNode)
        {
        AnalyzeEdgeCurveDeep(currNodeId, addList, isInsideIds, isOutsideIds, generation);
        }
    MTGARRAY_END_FACE_LOOP(currNodeId, pBodyGraph, faceNode)

        if (MTG_NULL_NODEID != (faceNode = GetParentFace(faceNode)))
            {
            addList.insert(faceNode);
            }

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionsApiWrapper::AddEdgeCurve
(
    bmap<int64_t, CurveInfo>& ids,
    int32_t userInt32,
    int64_t userInt64,
    int generation,
    MTGNodeId edgeNodeId
)
    {
    bmap<int64_t, CurveInfo>::iterator pFound = ids.find(userInt64);
    if (pFound != ids.end())
        {
        if (generation == 0)
            {
            CurveInfo& ci = pFound->second;
            ci.primaryUse = ci.primaryUse + 1;
            ci.edgeNodeIds.insert(edgeNodeId);
            ids[userInt64] = ci;
            }
        }
    else
        {
        CurveInfo ci;
        ci.primaryUse = (generation == 0) ? 1 : 0;
        ci.ownerType = userInt32;
        if (generation == 0)
            {
            ci.edgeNodeIds.insert(edgeNodeId);
            }
        ids[userInt64] = ci;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionsApiWrapper::AnalyzeEdgeCurve
(
    RG_Header* pRG,
    MTGNodeId edgeNodeId,
    bmap<int64_t, CurveInfo>& isInsideIds,
    bmap<int64_t, CurveInfo>& isOutsideIds,
    int32_t userInt32,
    int64_t userInt64,
    int generation,
    MTGNodeId mainEdgeNodeId
)
    {
    if (jmdlRG_edgeIsDirected(pRG, edgeNodeId))
        {
        if (isOutsideIds.find(userInt64) == isOutsideIds.end())
            {
            AddEdgeCurve(isInsideIds, userInt32, userInt64, generation, mainEdgeNodeId);
            }
        }
    else
        {
        if (isInsideIds.find(userInt64) == isInsideIds.end())
            {
            AddEdgeCurve(isOutsideIds, userInt32, userInt64, generation, mainEdgeNodeId);
            }
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionsApiWrapper::AnalyzeEdgeCurveDeep
(
    MTGNodeId currNodeId,
    bset<MTGNodeId>& addList,
    bmap<int64_t, CurveInfo>& isInsideIds,
    bmap<int64_t, CurveInfo>& isOutsideIds,
    int generation
)
    {
    int32_t    userInt32;
    int64_t    userInt64;

    MTGGraph* pBodyGraph = jmdlRG_getGraph(m_pRG);
    MTGNodeId edgeNodeId = currNodeId;

    while (true)
        {
        if (GetEdgeUserData(edgeNodeId, userInt32, userInt64))
            {
            AnalyzeEdgeCurve(m_pRG, edgeNodeId, isInsideIds, isOutsideIds, userInt32, userInt64, generation, currNodeId);
            }

        edgeNodeId = jmdlMTGGraph_getEdgeMate(pBodyGraph, edgeNodeId);

        if (!jmdlRG_faceIsNull(m_pRG, edgeNodeId))
            {
            addList.insert(edgeNodeId);
            break;
            }

        edgeNodeId = jmdlMTGGraph_getFSucc(pBodyGraph, edgeNodeId);
        }
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionsApiWrapper::GetFaceParentCurvesDeep
(
    MTGNodeId thisNode,
    bmap<int64_t, CurveInfo>& isInsideIds,
    bmap<int64_t, CurveInfo>& isOutsideIds
)
    {
    if (!jmdlRG_faceIsNegativeArea(m_pRG, thisNode))
        {
        bset<MTGNodeId> checked;
        bset<MTGNodeId> checkList;
        int             generation = 0;

        checkList.insert(GetLowestFaceNodeId(thisNode));

        while (!checkList.empty())
            {
            bset<MTGNodeId> addList;

            MTGNodeId checkFace = *checkList.begin();

            GetFaceParentCurves(checkFace, addList, isInsideIds, isOutsideIds, generation++);

            checked.insert(checkFace);
            checkList.erase(checkList.find(checkFace));

            for (bset<MTGNodeId>::iterator pIter = addList.begin(); pIter != addList.end(); pIter++)
                {
                MTGNodeId addFaceNode = *pIter;
                MTGNodeId faceId = GetLowestFaceNodeId(addFaceNode);
                if (checked.find(faceId) == checked.end())
                    {
                    if (!jmdlRG_faceIsNegativeArea(m_pRG, faceId))
                        {
                        checkList.insert(faceId);
                        }
                    }
                }
            }
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool RegionsApiWrapper::IsFaceNull
(
    MTGNodeId faceId
)
    {
    return jmdlRG_faceIsNull(m_pRG, faceId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool RegionsApiWrapper::DoesFaceHaveNegativeArea
(
    MTGNodeId faceId
)
    {
    return jmdlRG_faceIsNegativeArea(m_pRG, faceId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/00
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<MTGNodeId> RegionsApiWrapper::GetFaces
(
    bool*  pReqSign //  => (NULL) TRUE to return faces with positive area; FALSE to return faces with negative area (holes, exterior face); NULL to return all non-degenerate faces
)
    {
    bvector<MTGNodeId>nodes;
    bvector<MTGNodeId> faceStartArray;
    jmdlRG_getGraph (m_pRG)->CollectFaceLoops (faceStartArray);

    for (auto faceNodeId : faceStartArray)
        {
        // DO NOT test for directed edge here; the face may have been formed from linestrings, in which case its edges may be directed or not!
        bool isPositive = !jmdlRG_faceIsNegativeArea(m_pRG, faceNodeId);

        if (!pReqSign || (*pReqSign == isPositive))
            {
            nodes.push_back(faceNodeId);
            }
        }

    return nodes;
    }