/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMEdges.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDtmImpl.h"
#include <TerrainModel\Core\TMTransformHelper.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

BcDTMEdges::BcDTMEdges (BcDTMP dtm, const long* edges, long numEdges)
    {
    m_dtm = dtm;
    m_edges = edges;
    m_edgeCount = numEdges / 2;
    }

BcDTMEdges::~BcDTMEdges()
    {
    if (m_edges != NULL)
        free ((void*)m_edges);
    }

void BcDTMEdges::GetEdgeStartPoint (int index, DPoint3dR pt)
    {
    DTM_TIN_OBJ* tin = (DTM_TIN_OBJ*)m_dtm->GetTinHandle();
    pt = (tin->pointsP [m_edges [index * 2]]);
    TMTransformHelperP helper = m_dtm->GetTransformHelper();
    if (helper)
        helper->convertPointFromDTM (pt);
    }

void BcDTMEdges::GetEdgeEndPoint(int index, DPoint3dR pt)
    {
    DTM_TIN_OBJ* tin = (DTM_TIN_OBJ*)m_dtm->GetTinHandle();
    pt = (tin->pointsP [m_edges [index * 2 + 1]]);
    TMTransformHelperP helper = m_dtm->GetTransformHelper();
    if (helper)
        helper->convertPointFromDTM (pt);
    }

int BcDTMEdges::GetEdgeCount()
    {
    return m_edgeCount;
    }
