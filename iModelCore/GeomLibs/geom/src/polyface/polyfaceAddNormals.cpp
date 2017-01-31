/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/polyfaceAddNormals.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <assert.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct ApproximateVertexNormalContext
{
MTGGraphP graph;
MTGFacetsP oldFacets;
MTGMask barrierEdgeMask;
MTGMask visitMask;
int     readIndexLabel;
PolyfaceHeaderR mesh;

ApproximateVertexNormalContext (PolyfaceHeaderR mesh1)
    : mesh (mesh1)
    {
    oldFacets = jmdlMTGFacets_new ();
    graph = jmdlMTGFacets_getGraph (oldFacets);
    barrierEdgeMask = graph->GrabMask ();
    visitMask       = graph->GrabMask ();
    readIndexLabel = -1;
    }
    
~ApproximateVertexNormalContext ()
    {
    graph->DropMask (barrierEdgeMask);
    graph->DropMask (visitMask);
    jmdlMTGFacets_free (oldFacets);    
    }

bool TryGetNormal (MTGNodeId nodeId, DVec3dR normal)
    {
    int readIndex;
    return graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && mesh.TryGetNormalAtReadIndex ((size_t)readIndex, normal);
    }

bool TryGetNormalIndex (MTGNodeId nodeId, bvector<int> const &normalIndexArray, int &normalIndex)
    {
    int readIndex;
    if (graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && (size_t)readIndex < normalIndexArray.size()
        )
        {
        normalIndex = normalIndexArray[(size_t)readIndex];
        return true;
        }
    return false;
    }

bool TrySetVisibility (MTGNodeId nodeId, bool visible)
    {
    int readIndex;
    if (   graph->TryGetLabel (nodeId, readIndexLabel, readIndex)
        && (size_t)readIndex < mesh.PointIndex ().size()
        )
        {
        int s = visible ? 1 : -1;
        mesh.PointIndex ()[readIndex] = s * abs (mesh.PointIndex ()[readIndex]);
        }
    return false;
    }

void Report (size_t num0, size_t numInterior, size_t numHidden)
    {
    }
bool ComputeFaceNormals (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markTransitionsVisible)
    {
    bvector<DVec3d> newNormal;
    // Complete copy of prior indices ...
    bvector<int>    newNormalIndex = mesh.NormalIndex ();
    // Make existing indices negative so we can see if they are not changed ...
    // (but 0 remains zero -- these are one-based indices)
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        newNormalIndex[i] = - abs (newNormalIndex[i]);
        
    bvector<MTGNodeId> baseNodes;
    graph->ClearMask (barrierEdgeMask);

    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        MTGNodeId nodeB = graph->FSucc (nodeA);
        MTGNodeId nodeC = graph->VSucc (nodeB);
        MTGNodeId nodeD = graph->FSucc (nodeC);
        DVec3d normalA, normalD;
        bool isBarrier = false;
        if (graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK))
            {   // exterior side of boundary
            isBarrier = true;   // but do NOT record as base !!!
            }
        else if (graph->GetMaskAt (nodeC, MTG_EXTERIOR_MASK))
            {   // interior side of boundary
            isBarrier = true;
            baseNodes.push_back (nodeA);
            }
        else
            {   // interior edge
            if (!TryGetNormal (nodeA, normalA)
                || !TryGetNormal (nodeD, normalD))
                return false;
            double theta = normalA.AngleTo (normalD);
            if (theta > maxSingleEdgeAngle)
                {
                isBarrier = true;
                baseNodes.push_back (nodeA);
                }
            }
        if (isBarrier)
            graph->SetMaskAt (nodeA, barrierEdgeMask);

        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)


    // Find purely interior vertex loops with no barriers -- record a representaitve as a baseNode.
    graph->ClearMask (visitMask);
    MTGARRAY_SET_LOOP (nodeA, graph)
        {
        if (!graph->GetMaskAt (nodeA, visitMask))
            {
            // this is the first visit to this vertex.
            graph->SetMaskAroundVertex (nodeA, visitMask);
            if (0 == graph->CountMaskAroundVertex (nodeA, barrierEdgeMask))
                baseNodes.push_back (nodeA);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA, graph)
        
    
    DVec3d accumulatedNormal;
    bvector<DVec3d> extendedSectorNormal;
    bvector<MTGNodeId> extendedSectorNode;
    for (size_t i = 0, n = baseNodes.size (); i < n; i++)
        {
        // Walk forward to accumulate the candidate nodes around this vertex ...
        extendedSectorNode.clear ();
        extendedSectorNormal.clear ();
        MTGNodeId nodeId = baseNodes[i];
        MTGNodeId nodeId0 = nodeId;
        do {
            DVec3d normal;
            if (!TryGetNormal (nodeId, normal))
                return false;   // should never happen
            extendedSectorNormal.push_back (normal);
            extendedSectorNode.push_back (nodeId);
            nodeId = graph->VSucc (nodeId);
            } while (nodeId != nodeId0 && !graph->GetMaskAt (nodeId, barrierEdgeMask));
            
            
            
        size_t numSector = extendedSectorNode.size ();
        for (size_t k0 = 0; k0 < numSector; )
            {
            double accumulatedAngle = 0.0;
            accumulatedNormal = extendedSectorNormal[k0];
            size_t k1 = k0 + 1;
            for (; k1 < numSector;)
                {
                DVec3d normal1 = extendedSectorNormal[k1];
                double delta = extendedSectorNormal[k1-1].AngleTo (normal1);
                accumulatedAngle += delta;
                accumulatedNormal.Add (normal1); // NEEDS WORK -- weight by sector size?
                k1++;
                }
            // [k0..k1-1] is a sequence of {k1-k0) sectors that share a normal.
            // crate the new normal, index back to it, and mark the graph edge as a transition.
            DVec3d averageNormalA, averageNormalB;
            averageNormalA.Normalize (accumulatedNormal);    // don't have to divide by count -- normalize 
            averageNormalB.Scale (accumulatedNormal, 1.0 / (double)numSector);
            double dot = averageNormalA.DotProduct (averageNormalB);
            static double s_averageNormalDot = 0.92;
            if (dot > s_averageNormalDot && accumulatedAngle < maxAccumulatedAngle) // QV cutoff for 
                {
                newNormal.push_back (averageNormalA);
                graph->SetMaskAt (extendedSectorNode [k0], barrierEdgeMask);
                size_t oneBasedNormalIndex = newNormal.size ();

                for (size_t k = k0; k < k1; k++)
                    {
                    int readIndexI;
                    if (!graph->TryGetLabel (extendedSectorNode[k], readIndexLabel, readIndexI))
                        return false;
                    size_t readIndex = (size_t)readIndexI;
                    if (readIndex >= newNormalIndex.size ())
                        return false;
                    newNormalIndex[readIndex] = static_cast<int>(oneBasedNormalIndex);
                    }
                }
            else
                {
                for (size_t k = k0; k < k1; k++)
                    {
                    newNormal.push_back (extendedSectorNormal[k]);
                    graph->SetMaskAt (extendedSectorNode [k], barrierEdgeMask);
                    size_t oneBasedNormalIndex = newNormal.size ();
                    int readIndexI;
                    if (!graph->TryGetLabel (extendedSectorNode[k], readIndexLabel, readIndexI))
                        return false;
                    size_t readIndex = (size_t)readIndexI;
                    if (readIndex >= newNormalIndex.size ())
                        return false;
                    newNormalIndex[readIndex] = static_cast<int>(oneBasedNormalIndex);
                    }
                }

            k0 = k1;
            }
        }

    // confirm that all normal indices have been touched ...
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        {
        if (newNormalIndex[i] < 0)
            return false;
        }

    // Ah !!! time to install ...
    bvector<DVec3d> &meshNormal = mesh.Normal ();
    bvector<int> &meshNormalIndex = mesh.NormalIndex ();
    
    meshNormal.clear ();
    // normals are "new"  --- probably smaller.  clear and copy.
    for (size_t i = 0, n = newNormal.size (); i < n; i++)
        meshNormal.push_back (newNormal[i]);
    // indices are one-to-one replacements ...
    assert (newNormalIndex.size () == meshNormalIndex.size ());
    for (size_t i = 0, n = newNormalIndex.size (); i < n; i++)
        meshNormalIndex[i] = newNormalIndex[i];
    
    if (markTransitionsVisible)
        {
        bvector<int> &pointIndex = mesh.PointIndex ();
        for (size_t i = 0; i < pointIndex.size (); i++)
            pointIndex[i] = abs (pointIndex[i]);
        size_t numTotal = 0;
        size_t numInteriorSector = 0;
        size_t numHidden = 0;
        MTGARRAY_SET_LOOP (nodeB, graph)
            {
            numTotal++;
            if (!graph->GetMaskAt (nodeB, MTG_EXTERIOR_MASK))
                {
                numInteriorSector++;
                MTGNodeId nodeA = graph->VPred (nodeB);
                if (!graph->GetMaskAt (nodeA, MTG_EXTERIOR_MASK))
                    {
                    int normalIndexA, normalIndexB;
                    if (  TryGetNormalIndex (nodeA, meshNormalIndex, normalIndexA)
                       && TryGetNormalIndex (nodeB, meshNormalIndex, normalIndexB)
                       && normalIndexA == normalIndexB
                       )
                        {
                        TrySetVisibility (nodeB, false);
                        numHidden++;
                        }
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (nodeB, graph)
        Report (numTotal, numInteriorSector, numHidden);
        }
    return true;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool go (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markAllTransitionsVisible)
    {    
    if (!mesh.BuildPerFaceNormals ())
        return false;
    if (!PolyfaceToMTG_FromPolyfaceConnectivity (oldFacets, mesh))
        return false;
    if (!graph->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, readIndexLabel))
        return false;

    return ComputeFaceNormals (maxSingleEdgeAngle, maxAccumulatedAngle, markAllTransitionsVisible);
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildApproximateNormals (double maxSingleEdgeAngle, double maxAccumulatedAngle, bool markAllTransitionsVisible)
    {
    ApproximateVertexNormalContext context (*this);
    return context.go (maxSingleEdgeAngle, maxAccumulatedAngle, markAllTransitionsVisible);
    }
    
    
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceParameters (LocalCoordinateSelect selector)
    {
    size_t numIndex = m_pointIndex.size ();
    // We don't know how to do this for non-indexed ...
    if (m_pointIndex.size () == 0)
        return false;
        
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    BentleyApi::Transform worldToLocal, localToWorld;
    visitor->SetNumWrap (0);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    bvector <DPoint3d> &visitorPoint = visitor->Point ();
    
    // We will have distinct params on every corner of every face.  Don't know how many ...
    ClearParameters (true);
    // There has to be a param for each point index ..
    m_paramIndex.reserve (numIndex);
    for (size_t i = 0; i < numIndex; i++)
        m_paramIndex.push_back (0);

    for (size_t i = 0; i < m_faceData.size (); i++)
        m_faceData[i].m_paramRange = DRange2d::NullRange ();
        
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        bool frameOK = visitor->TryGetLocalFrame (localToWorld, worldToLocal, selector);

        for (size_t i = 0, n = visitor->NumEdgesThisFace (); i < n; i++)
            {
            DPoint3d uvw;
            DPoint2d uv;
            if (frameOK)
                worldToLocal.Multiply (uvw, visitorPoint[i]);
            else
                uvw.Zero ();
            size_t newParamIndex = m_param.size ();
            uv.x = uvw.x;
            uv.y = uvw.y;
            m_param.push_back (uv);
            size_t readPos = indexPosition[i];
            if (readPos < numIndex)   // really really better be...
                m_paramIndex[readPos] = static_cast <int>(newParamIndex + 1);   // ONE BASED !!!!
            size_t faceIndex;
            if (m_faceIndex.Active ()
                && readPos < m_faceIndex.size ()
                && (faceIndex = m_faceIndex[readPos]) < m_faceData.size ()
                )
                {
                m_faceData[faceIndex].m_paramRange.Extend (uv);
                }
            }
        }
    
    return true;
    }  

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearNormals (bool active)
    {
    Normal ().clear ();
    NormalIndex ().clear ();
    Normal ().SetActive (active);
    NormalIndex ().SetActive (active);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearParameters (bool active)
    {
    Param ().clear ();
    ParamIndex ().clear ();
    Param ().SetActive (active);
    ParamIndex ().SetActive (active);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceNormals ()
    {
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return false;
     size_t numIndex = m_pointIndex.size ();
    // We don't know how to do this for non-indexed ...
    if (m_pointIndex.size () == 0)
        return false;
        
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    BentleyApi::Transform worldToLocal, localToWorld;
    visitor->SetNumWrap (0);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    
    ClearNormals (true);
    
    // There has to be a param for each point index ..
    m_normalIndex.reserve (numIndex);
    for (size_t i = 0; i < numIndex; i++)
        m_normalIndex.push_back (0);
    DVec3d zvector;
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        if (visitor->TryGetLocalFrame (localToWorld, worldToLocal, LOCAL_COORDINATE_SCALE_UnitAxesAtStart))
            {
            localToWorld.GetMatrixColumn (zvector, 2);
            size_t newNormalIndex = m_normal.size ();
            m_normal.push_back (zvector);
            
            for (size_t i = 0, n = visitor->NumEdgesThisFace (); i < n; i++)
                {
                size_t readPos = indexPosition[i];
                if (readPos < numIndex)   // really really better be...
                    m_normalIndex[readPos] = static_cast <int>(newNormalIndex + 1);   // ONE BASED !!!!
                }
            }
        }
    
    return true;
    }
 
 
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildPerFaceFaceData ()
    {
    FaceIndex().SetActive(true);

    if (0 !=  GetParamCount())
        {
        SetNewFaceData (NULL, GetPointIndexCount());

        for (FacetFaceDataR faceData : m_faceData)
            faceData.m_paramRange.InitFrom (0.0, 0.0, 1.0, 1.0);
        }
    else
        {
        FacetFaceData       faceData;
        PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (*this, true);

        for (visitor->Reset (); visitor->AdvanceToNextFace (); )
            {
            SetNewFaceData (&faceData, visitor->GetReadIndex() + visitor->NumEdgesThisFace() + (0 == GetNumPerFace() ? 1 : 0));
            faceData.Init ();
            }
        }


    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2017
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildXYParameters (LocalCoordinateSelect selector, TransformR localToWorld, TransformR worldToLocal)
    {
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    if (0 == m_point.size ())
        return false;

    DRange3d range = DRange3d::From (m_point);
    DPoint3d points [5];
    double z = range.low.z;
    points[0] = DPoint3d::From (range.low.x, range.low.y, z);
    points[1] = DPoint3d::From (range.high.x, range.low.y, z);
    points[2] = DPoint3d::From (range.high.x, range.high.y, z);
    points[3] = DPoint3d::From (range.low.x, range.high.y, z);
    points[4] = points[0];

    if (!PolygonOps::CoordinateFrame
        (
        points, 5,
        localToWorld, worldToLocal, selector
        ))
        return false;
    return BuildParametersFromTransformedPoints (worldToLocal);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2017
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::BuildParametersFromTransformedPoints (TransformCR worldToLocal)
    {
    if (0 == m_point.size ())
        return false;

    // * one param per point.
    m_param.SetActive (true);
    m_param.clear ();
    for (auto xyz : m_point    )
        {
        m_param.push_back (DPoint2d::From (xyz.x, xyz.y));
        }
    worldToLocal.Multiply (m_param, m_param);

    // * param index matches point index.
    m_paramIndex.SetActive (m_pointIndex.Active ());
    if (m_paramIndex.Active ())
        {
        m_paramIndex.clear ();
        m_paramIndex.insert (m_paramIndex.begin (), m_pointIndex.begin (), m_pointIndex.end());
        }

    return true;
    }
   
END_BENTLEY_GEOMETRY_NAMESPACE
















