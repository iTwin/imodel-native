/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceClipToPlaneSets.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include  <Bentley/bset.h>
#include  <Bentley/bmap.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  OrderedIndexPair
{
    size_t                   m_index0;
    size_t                   m_index1;

            OrderedIndexPair () { }
    bool    operator ==    (OrderedIndexPair const& rhs) const { return m_index0 == rhs.m_index0 && m_index1 == rhs.m_index1; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
OrderedIndexPair (size_t index0, size_t index1) 
    {
    // Ignore signs; we just want to pair manifold edges
    if (index0 < index1)
        {
        m_index0 = index0;
        m_index1 = index1;
        }
    else
        {
        m_index0 = index1;
        m_index1 = index0;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void AddIndex (size_t index)
    {
    if (index < m_index0)
        {
        m_index1 = m_index0;
        m_index0= index;
        }
    else
        {
        m_index1 = index;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (OrderedIndexPair const& rhs) const
    {
    if (m_index0 == rhs.m_index0)
        return m_index1 < rhs.m_index1;
    else 
        return m_index0 < rhs.m_index0;
    }
};  //  OrderedIndexPair


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  ClippedSegment
{
    size_t      m_index0;
    size_t      m_index1;
    double      m_distance0;
    double      m_distance1;

    ClippedSegment() {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClippedSegment (OrderedIndexPair const& pair, DRay3dCR ray,  PolyfaceQueryR polyface)
    {
    double      distance0 = polyface.GetPointCP()[pair.m_index0 - 1].DotDifference (ray.origin, ray.direction),
                distance1 = polyface.GetPointCP()[pair.m_index1 - 1].DotDifference (ray.origin, ray.direction);

    if (distance0 < distance1)
        {
        m_index0    = pair.m_index0;
        m_distance0 = distance0;
        m_index1    = pair.m_index1;
        m_distance1 = distance1;
        }
    else
        {
        m_index0    = pair.m_index1;
        m_distance0 = distance1;
        m_index1    = pair.m_index0;
        m_distance1 = distance0;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (ClippedSegment const& rhs) const { return m_distance0 < rhs.m_distance0; }

}; //   ClippedSegment

typedef bset <OrderedIndexPair>     T_OrderedIndexPairs;
typedef bset <ClippedSegment>       T_ClippedSegments;
typedef bvector <size_t>            T_EdgeIndices;


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  OutputChainEdge
{
    DRay3d                      m_ray;
    T_OrderedIndexPairs         m_clippedIndices;

    OutputChainEdge (DRay3dR ray) : m_ray (ray) { } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
void GetClippedEdgeIndices (T_EdgeIndices& indices, PolyfaceQueryR facets)
    {
    T_ClippedSegments      clippedSegments;

    for (T_OrderedIndexPairs::iterator curr = m_clippedIndices.begin(); curr != m_clippedIndices.end(); curr++)
        clippedSegments.insert (ClippedSegment (*curr, m_ray, facets));
    
    // Needs work.  Coalesce.
    for (T_ClippedSegments::iterator curr = clippedSegments.begin(); curr != clippedSegments.end(); curr++)
        {
        if (indices.empty())
            {
            indices.push_back (curr->m_index0);
            }
        else if (indices.back() != curr->m_index0)
            {
            indices.push_back (0);
            indices.push_back (curr->m_index0);
            }

        indices.push_back (curr->m_index1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddClippedEdge (OrderedIndexPair const& edgeIndices)
    {
    if (m_clippedIndices.find (edgeIndices) == m_clippedIndices.end())
        m_clippedIndices.insert (edgeIndices);
    }

};  //  OutputChainEdge

typedef bvector <OutputChainEdge*>                          T_OutputChainEdges;
typedef bmap <OrderedIndexPair, OutputChainEdge*>       T_EdgeIndexMap;


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  OutputChain
{
    CurveTopologyId         m_id;
    T_OutputChainEdges      m_outputEdges;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
OutputChain (PolyfaceEdgeChain const& edgeChain, T_EdgeIndexMap& indexMap, PolyfaceQueryCR facets) : m_id (edgeChain.GetId())
    {
    int32_t const*      indices = edgeChain.GetIndexCP();
    for (size_t i=0; i<edgeChain.GetIndexCount()-1; i++)
        {
        DRay3d              ray;

        if (indices[i] <= 0 || indices[i+1] <= 0)
            {
            BeAssert (false);
            continue;
            }

        ray.origin = facets.GetPointCP()[indices[i]-1];
        ray.direction.NormalizedDifference (facets.GetPointCP()[indices[i+1]-1], ray.origin);

        OutputChainEdge*     edge = new OutputChainEdge (ray);

        m_outputEdges.push_back (edge);
        indexMap[OrderedIndexPair (indices[i], indices[i+1])] = edge;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
~OutputChain ()
    {
    for (T_OutputChainEdges::iterator curr = m_outputEdges.begin(); curr != m_outputEdges.end(); curr++)
        delete *curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddClippedEdges (PolyfaceHeaderR polyface)
    {
    T_EdgeIndices                       edgeIndices;

    for (T_OutputChainEdges::iterator curr = m_outputEdges.begin(); curr != m_outputEdges.end(); curr++)
        (*curr)->GetClippedEdgeIndices (edgeIndices, polyface);

    if (!edgeIndices.empty() && 0 != edgeIndices.back())
        edgeIndices.push_back (0);

    uint32_t            chainIndex = 0;
    PolyfaceEdgeChain   edgeChain (m_id);

    for (T_EdgeIndices::iterator curr = edgeIndices.begin(); curr != edgeIndices.end(); curr++)
        {
        if (0 == *curr)
            {
            polyface.EdgeChain().push_back (edgeChain);
            edgeChain = PolyfaceEdgeChain (CurveTopologyId (m_id, ++chainIndex));
            continue;
            }
                                             
        edgeChain.AddIndex ((int32_t) *curr);
        }
    }
};  //  OutputChain

typedef bvector <OutputChain*>          T_OutputChains;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  OutputChainMap
{
    T_OutputChains          m_outputChains;
    T_EdgeIndexMap          m_edgeIndexMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
OutputChainMap (PolyfaceQueryCR facets)
    {
    for (PolyfaceEdgeChainCP edgeChain = facets.GetEdgeChainCP(), end = edgeChain + facets.GetEdgeChainCount(); edgeChain < end; edgeChain++)
        m_outputChains.push_back (new OutputChain (*edgeChain, m_edgeIndexMap, facets));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
~OutputChainMap ()
    {
    for (T_OutputChains::iterator curr = m_outputChains.begin(); curr != m_outputChains.end(); curr++)
        delete *curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
OutputChainEdge* FindEdge (int32_t index0, int32_t index1)
    {
    T_EdgeIndexMap::iterator    found = m_edgeIndexMap.find (OrderedIndexPair (index0, index1));

    return (found == m_edgeIndexMap.end()) ? NULL : found->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/011
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddClippedEdgeChains (PolyfaceHeaderR facets)
    {
    for (T_OutputChains::iterator curr = m_outputChains.begin(); curr != m_outputChains.end(); curr++)
        (*curr)->AddClippedEdges (facets);
    }

};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011
*
* Contains a single facet of a polyface being clipped.  This is instantiated once and
* recycled during the clipping process
+===============+===============+===============+===============+===============+======*/
struct PolyfaceClipFacet
{
    bvector <DPoint3d>                  m_points;
    bvector <DVec3d>                    m_normals;
    bvector <DPoint2d>                  m_params;
    bvector <bool>                      m_visibility;
    bvector <OutputChainEdge*>          m_chainEdges;
    T_ClipPlaneSets const&              m_planeSets;
    mutable  size_t                     m_index;
                                
PolyfaceClipFacet (T_ClipPlaneSets const& planeSets, size_t index) : m_planeSets (planeSets), m_index (index) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
*
*  Constructor for a facet clipped to a plane from unclipped facet and plane
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceClipFacet (PolyfaceClipFacet const& unclipped, ClipPlaneCR plane, bool hideCutGeometry) : m_planeSets (unclipped.m_planeSets), m_index (unclipped.m_index)
    {
    size_t          nPoints = unclipped.m_points.size();
    DPoint3dCP      points  = &unclipped.m_points.front();

    // Combination of inside and outside, clip required.
    bool            thisInside, nextInside;
    double          thisDistance, nextDistance;

    nextDistance  = plane.EvaluatePoint (points[0]);
    nextInside    = nextDistance > 0.0;

    for (size_t i=0; i<nPoints; i++)
        {
        size_t         iNext = (i == nPoints - 1) ? 0 : (i + 1);

        thisDistance = nextDistance;
        thisInside   = nextInside;

        nextDistance  = plane.EvaluatePoint (points[iNext]);
        nextInside    = nextDistance > 0.0;

        if (nextInside != thisInside)
            AddInterpolatedPoint (unclipped, i, iNext, thisDistance / (thisDistance - nextDistance), (nextInside ? unclipped.m_visibility[i] : !hideCutGeometry), (nextInside ? unclipped.m_chainEdges[i] : NULL));

        if (nextInside)
            AddPoint (unclipped, iNext, unclipped.m_visibility[iNext], unclipped.m_chainEdges[iNext]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddPoint (PolyfaceClipFacet const& unclipped, size_t i, bool visibility, OutputChainEdge* chainEdge)
    {
    m_points.push_back (unclipped.m_points[i]);
    m_visibility.push_back (visibility);
    m_chainEdges.push_back (chainEdge);

    if (!unclipped.m_normals.empty())
        m_normals.push_back (unclipped.m_normals[i]);

    if (!unclipped.m_params.empty())
        m_params.push_back (unclipped.m_params[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddInterpolatedPoint (PolyfaceClipFacet const& unclipped, size_t i, size_t iNext, double t, bool visibility, OutputChainEdge* chainEdge)
    {
    m_points.push_back(DPoint3d::FromInterpolate (unclipped.m_points[i], t, unclipped.m_points[iNext]));
    m_visibility.push_back (visibility);
    m_chainEdges.push_back (chainEdge);

    if (!unclipped.m_normals.empty())
        m_normals.push_back(DVec3d::FromInterpolate (unclipped.m_normals[i], t, unclipped.m_normals[iNext]));

    if (!unclipped.m_params.empty())
        {
        DPoint2d        param;

        param.Interpolate (unclipped.m_params[i], t, unclipped.m_params[iNext]);
        m_params.push_back (param);
        }
     }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    Init (PolyfaceVisitorR visitor, OutputChainMap& outputChainMap)
    {
    size_t          count = visitor.NumEdgesThisFace();

    m_index = 0;

    m_points.resize (count);
    m_visibility.resize (count);
    m_chainEdges.resize (count);
    
    memcpy (&m_points[0], visitor.GetPointCP(), count * sizeof (DPoint3d));
    for (size_t i=0; i<count; i++)
        m_visibility[i] = visitor.Visible()[i];

    for (size_t i=0; i<count; i++)
        m_chainEdges[i] = m_visibility[i] ? outputChainMap.FindEdge (visitor.GetClientPointIndexCP()[i] + 1, visitor.GetClientPointIndexCP()[i+1] + 1) : NULL;

    if (NULL != visitor.GetNormalCP())
        {
        m_normals.resize (count);
        memcpy (&m_normals[0], visitor.GetNormalCP(), count * sizeof (DVec3d));
        }

    if (NULL != visitor.GetParamCP())
        {
        m_params.resize(count);
        memcpy (&m_params[0], visitor.GetParamCP(), count * sizeof (DPoint2d));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void   AddToPolyface  (IPolyfaceConstructionR builder, OutputChainMap& outputChainMap, double areaTolerance) const
    {
    size_t          count;

    if ((count = m_points.size()) < 3 ||
        bsiPolygon_polygonNormalAndArea (NULL, NULL, &m_points.front(), (int) m_points.size()) < areaTolerance)
        return;

    if (m_points[0].IsEqual (m_points[count-1]) && --count < 3)
        return;

    bvector<size_t>     pointIndices(count);
    for (size_t i=0; i<count; i++)
        {
        builder.AddPointIndex (pointIndices[i] = builder.FindOrAddPoint (m_points[i]), m_visibility[i]);                                          
        if (!m_normals.empty())
            builder.AddNormalIndex (builder.FindOrAddNormal (m_normals[i]));
            
        if (!m_params.empty())
            builder.AddParamIndex (builder.FindOrAddParam (m_params[i]));
        }
         
    for (size_t i=0; i<count; i++)
         if (NULL != m_chainEdges[i])
            m_chainEdges[i]->AddClippedEdge (OrderedIndexPair (1+pointIndices[i], 1 + pointIndices[(i + 1) % count]));


    builder.AddPointIndexTerminator();
                                                                                                                                         
    if (!m_normals.empty())
        builder.AddNormalIndexTerminator();
            
    if (!m_params.empty())
        builder.AddParamIndexTerminator();
    }

}; // PolyfaceClipFacet

enum    ClipStatus
    {
    ClipStatus_ClipRequired,
    ClipStatus_TrivialReject,
    ClipStatus_TrivialAccept
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011
+===============+===============+===============+===============+===============+======*/
struct PolyfaceClipContext
{
    IPolyfaceConstructionR      m_builder;
    bool                        m_triangulate;
    double                      m_tolerance;
    double                      m_areaTolerance;
    OutputChainMap&             m_outputChainMap;

    PolyfaceClipContext (IPolyfaceConstructionR output, OutputChainMap& chainMap, double tolerance, bool triangulate) : 
                        m_builder (output), 
                        m_outputChainMap (chainMap), 
                        m_tolerance (tolerance), 
                        m_areaTolerance (tolerance * tolerance), 
                        m_triangulate (triangulate) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ClipStatus PointSetSingleClipStatus
(
DPoint3dCP                  points,
size_t                      count,
ClipPlaneSetCR              clipSet,
double                      tolerance
)
    {
    bool                    anyTested = false;
    
    for (ConvexClipPlaneSetCR convexSet: clipSet)
        {
        bool            allOutsideSinglePlane = false, anyOutside = false;;

        anyTested = true;
        for (ClipPlaneCR plane: convexSet)
            {
            size_t          nInside = 0, nOutside = 0;
            double          planeDistance = plane.GetDistance() - tolerance;

            for (DPoint3dCP curr = points, end = points + count; curr != end; curr++)
                (curr->DotProduct (plane.GetNormal()) > planeDistance) ? nInside++ : nOutside++;

            if (0 != nOutside)
                anyOutside = true;

            if (0 == nInside)
                {
                allOutsideSinglePlane = true;
                break;
                }

            }
        if (!anyOutside)     // Totally inside this set- no clip required.
            return ClipStatus_TrivialAccept;

        if (!allOutsideSinglePlane)
            return ClipStatus_ClipRequired;                                     // Not completely outside of any planes - Clip necessary
        }
    return anyTested ? ClipStatus_TrivialReject : ClipStatus_TrivialAccept;     // Outside all sets - no processing required.
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClipStatus     PointSetSinglePlaneStatus
(
DPoint3dCP      points,
size_t          count,
DVec3dCR        planeNormal,
double          planeDistance
)                   // Return true if all points are inside the plane.
    {
    bool        allInside = true, anyInside = false;

    for (DPoint3dCP curr = points, end = points + count; curr != end; curr++)
        {
        if (planeNormal.DotProduct (*curr) > planeDistance)
            anyInside = true;
        else
            allInside = false;
        }
    if (allInside)
        return ClipStatus_TrivialAccept;
    else
        return anyInside ? ClipStatus_ClipRequired : ClipStatus_TrivialReject;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void  ClipPolyfaceFacetToPlanes (PolyfaceClipFacet const& facet, ClipPlaneCP plane, ClipPlaneCP planeEndP) 
    {
    if (facet.m_points.empty())
        return;

    if (plane == planeEndP)
        {
        ClipPolyfaceFacet (facet);
        return;
        }

    switch  (PointSetSinglePlaneStatus (&facet.m_points.front(), facet.m_points.size(), plane->GetNormal(), plane->GetDistance() - m_tolerance))
        {
        case ClipStatus_TrivialReject:
            return;

        case ClipStatus_TrivialAccept:
            ClipPolyfaceFacetToPlanes (facet, ++plane, planeEndP);
            return;
        }
    ClipPlaneCR      clipPlane = *plane;
    ClipPolyfaceFacetToPlanes (PolyfaceClipFacet (facet, clipPlane, !clipPlane.IsVisible()), ++plane, planeEndP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void   ClipPolyfaceFacet (PolyfaceClipFacet const& facet)
    {
    if (facet.m_index == facet.m_planeSets.size())
        {
        facet.AddToPolyface (m_builder, m_outputChainMap, m_areaTolerance);
        return;
        }

    switch  (PointSetSingleClipStatus (&facet.m_points.front(), facet.m_points.size(), facet.m_planeSets[facet.m_index], m_tolerance))
        {
        case ClipStatus_TrivialAccept:
            facet.m_index++;
            ClipPolyfaceFacet (facet);
            return;

        case ClipStatus_TrivialReject:
            return;
        }

    ClipPlaneSetCR   planeSet = facet.m_planeSets[facet.m_index];
    
    if (facet.m_index++ < facet.m_planeSets.size())
        for (ConvexClipPlaneSetCR convexSet: planeSet)
            ClipPolyfaceFacetToPlanes (facet, &convexSet.front(), &convexSet.front() + convexSet.size());
    }

};  // PolyfaceClipContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PolyfaceQuery::ClipToPlaneSetIntersection (T_ClipPlaneSets const& planeSets, PolyfaceQuery::IClipToPlaneSetOutput& output, bool triangulateOutput) const
    {
    DRange3d                range;
    static double           s_relativeTolerance = 1.0E-6;
    size_t                  index;

    range.InitFrom (GetPointCP(), (int) GetPointCount());

    double                  distanceTolerance = s_relativeTolerance * range.low.Distance (range.high); 
    bool                    doClip = false;

    
    for (index = 0; index < planeSets.size() && !doClip; index++)
        {
        switch  (PolyfaceClipContext::PointSetSingleClipStatus (GetPointCP(), GetPointCount(), planeSets[index], distanceTolerance))
            {
            case ClipStatus_TrivialReject:
                return SUCCESS;

            case ClipStatus_ClipRequired:
                doClip = true;
                break;
            }
        }
    if (!doClip)
        return output._ProcessUnclippedPolyface (*this);

    IFacetOptionsPtr            facetOptions = IFacetOptions::New();

    facetOptions->SetNormalsRequired (0 != GetNormalCount());
    facetOptions->SetParamsRequired (0 != GetParamCount());

    IPolyfaceConstructionPtr    outputBuilder = IPolyfaceConstruction::New (*facetOptions);
    PolyfaceClipFacet           facet (planeSets, index);
    OutputChainMap              outputChainMap (*this);
    PolyfaceClipContext         polyfaceClipContext (*outputBuilder, outputChainMap, distanceTolerance, triangulateOutput);
    size_t                      currentFaceIndex = 0, thisFaceIndex;
    FacetFaceData               faceData;
                                                                       
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        if (TryGetFacetFaceDataAtReadIndex (visitor->GetReadIndex(), faceData, thisFaceIndex))
            {
            if (thisFaceIndex != currentFaceIndex)
                {
                outputBuilder->EndFace ();
                outputBuilder->SetFaceData (faceData);
                currentFaceIndex = thisFaceIndex;
                }
            }
        facet.Init (*visitor, outputChainMap);
        polyfaceClipContext.ClipPolyfaceFacet (facet);
        }
    outputBuilder->SetFaceData (faceData);
    outputBuilder->EndFace();

    PolyfaceHeaderR     clippedMesh = outputBuilder->GetClientMeshR();

    if (0 == clippedMesh.GetPointIndexCount())
        return SUCCESS;

    BeAssert (clippedMesh.GetPointCount() >= 3);		

    outputChainMap.AddClippedEdgeChains (clippedMesh);

    if (triangulateOutput)
        clippedMesh.Triangulate();

    return output._ProcessClippedPolyface (clippedMesh);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
