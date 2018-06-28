/*----------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/IndexedClipEdge.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <map>
#include "IndexedClipEdge.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ClipEdgeData::ClipEdgeData(int vertexIndexA, int vertexIndexB, int planeIndex) :
    m_vertexA (vertexIndexA),
    m_vertexB (vertexIndexB),
    m_planeIndex (planeIndex)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ClipEdgeData::SetNeighbors (size_t succ, size_t pred)
    {
    m_succ = succ;
    m_pred = pred;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipEdgeDataArray::compare_PlaneIdVertexAVertexB (ClipEdgeData const& edge0, ClipEdgeData const& edge1)
    {
    if (edge0.m_planeIndex != edge1.m_planeIndex)
        return edge0.m_planeIndex < edge1.m_planeIndex;
    if (edge0.m_vertexA != edge1.m_vertexA)
        return edge0.m_vertexA < edge1.m_vertexA;
    if (edge0.m_vertexB != edge1.m_vertexB)
        return edge0.m_vertexB < edge1.m_vertexB;
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t ClipEdgeDataArray::FindEndOfPlaneCluster (size_t beginIndex)
    {
    size_t endIndex = beginIndex;
    size_t n = size ();
    if (endIndex >= n)
        return endIndex;

    int planeIndex = at (beginIndex).m_planeIndex;
    while (endIndex < n)
        {
        if (at(endIndex).m_planeIndex != planeIndex)
            return endIndex;
        endIndex++;
        }
    return endIndex;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipEdgeDataArray::IsValid (size_t index)
    {
    return index < size ();
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ClipEdgeDataArray::SetFlag (size_t beginIndex, size_t endIndex, bool flag)
    {
    for (size_t i = beginIndex; i < endIndex; i++)
        at(i).m_flag = flag;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ClipEdgeDataArray::SetFlag (size_t index, bool flag)
    {
    if (IsValid (index))
        at(index).m_flag = flag;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipEdgeDataArray::GetFlag (size_t index)
    {
    return IsValid(index) ? at(index).m_flag : false;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ClipEdgeDataArray::ExtendChain
    (
    size_t beginIndex,
    size_t endIndex,
    size_t index,
    bvector <size_t>&chainIndices,
    bool includeFinalVertex
    )
    {
    int lastVertex = -1;
    while (index >= beginIndex && index < endIndex && !at(index).m_flag)
        {
        at(index).m_flag = true;
        chainIndices.push_back (at(index).m_vertexA);
        lastVertex = at(index).m_vertexB;
        index = at(index).m_succ;
        }
    if (includeFinalVertex)
        chainIndices.push_back (lastVertex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ClipEdgeDataArray::SortByPlaneVertexAVertexB ()
    {
    std::sort (begin (), end (), compare_PlaneIdVertexAVertexB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t ClipEdgeDataArray::CountVertexA (size_t vertexA, size_t beginIndex, size_t endIndex, size_t &lastMatch)
    {
    lastMatch = endIndex;
    size_t numMatch = 0;
    for (size_t index = beginIndex; index < endIndex; index++)
        {
        if (at(index).m_vertexA == vertexA)
            {
            lastMatch = index;
            numMatch++;
            }
        }
    return numMatch;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t ClipEdgeDataArray::CountVertexB (size_t vertexB, size_t beginIndex, size_t endIndex, size_t &lastMatch)
    {
    lastMatch = endIndex;
    size_t numMatch = 0;
    for (size_t index = beginIndex; index < endIndex; index++)
        {
        if (at(index).m_vertexB == vertexB)
            {
            lastMatch = index;
            numMatch++;
            }
        }
    return numMatch;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ClipEdgeDataArray::AnalyzeCutPlane (size_t beginIndex, size_t endIndex, bvector <size_t> &startIndices, bvector<size_t> &endIndices)
    {
    startIndices.clear ();
    //size_t numDuplicateStart = 0;
    //size_t numOK = 0;
    size_t numError = 0;
    for (size_t i = beginIndex; i < endIndex; i++)
        {
        at(i).SetNeighbors (i, i);
        }
    //size_t numEnd   = 0;
    for (size_t i = beginIndex; i < endIndex; i++)
        {
        size_t succ, pred;
        size_t numSucc   = CountVertexA (at(i).m_vertexB, beginIndex, endIndex, succ);
        size_t numPred   = CountVertexB (at(i).m_vertexA, beginIndex, endIndex, pred);
        if (numSucc != 1)
            succ = i;
        if (numPred != 1)
            pred = i;
        at(i).SetNeighbors (succ, pred);
        if (numPred == 0 && numSucc == 0)
            {
            startIndices.push_back (i);
            endIndices.push_back (i);
            }
        else if (numPred == 0 && numSucc == 1)
            {
            startIndices.push_back (i);
            }
        else if (numPred == 1 && numSucc == 0)
            {
            endIndices.push_back (i);
            }
        else
            {
            numError = 0;
            }
        }    
    return numError == 0;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

