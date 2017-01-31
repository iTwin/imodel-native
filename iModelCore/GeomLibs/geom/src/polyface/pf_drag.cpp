/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_drag.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/bset.h>
#include "pf_halfEdgeArray.h"
#include <Geom/XYZRangeTree.h>
#include <Geom/cluster.h>
#include <Vu/VuApi.h>
//#include <assert.h>

#define     BUFFER_SIZE         10
//#define DEBUG_CODE(__contents__) __contents__
#define DEBUG_CODE(__contents__)

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void PolyfaceHeader::CollectAdjacentFacetAndPointIndices
(
bvector<size_t> &activeReadIndex,     //!< [in,out] readIndices of active facets.  This is sorted in place.
bvector<size_t> &fringeReadIndex, //!< [out] readIndices of facets that have a least one vertex indicent to the activeFacets.
bvector<size_t> &activePointIndex        //!< [out] indices of points incident to any active facet.
)
    {
    activePointIndex.clear ();
    fringeReadIndex.clear ();
    if (activeReadIndex.empty ())
        return;
    std::sort (activePointIndex.begin (), activePointIndex.end ());
    bvector<size_t> fringeFacets;
    bvector<uint32_t> pointUseCount;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this);
    auto &clientIndex = visitor->ClientPointIndex ();
    bvector<DPoint3d> &points = Point ();
    for (size_t i = 0; i < points.size (); i++)
        pointUseCount .push_back (0);

    for (size_t readIndex : activeReadIndex)
        {
        visitor->MoveToFacetByReadIndex (readIndex);
        for (size_t i : clientIndex)
            pointUseCount[i] += 1;
        }

    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t num0 = 0, num1 = 0;
        for (size_t i : clientIndex)
            {
            if (pointUseCount[i] == 0)
                num0++;
            else
                num1++;
            }
        if (num0 > 0 && num1 > 0)
            fringeReadIndex.push_back (visitor->GetReadIndex ());
        }



    for (size_t i = 0; i < pointUseCount.size (); i++)
        {
        if (pointUseCount[i] > 0)
            activePointIndex.push_back (i);
        }

    }


//! Apply a translation to a subset of facets.
GEOMDLLIMPEXP void PolyfaceHeader::TranslateSelectedFacets
(
bvector<size_t> &activeReadIndex,     //!< [in,out] readIndices of active facets.  This is sorted in place.
DVec3dCR vector,
FacetTranslationMode mode
)
    {

    if (mode == PolyfaceHeader::FacetTranslationMode::JustTranslatePoints)
        {
        bvector<size_t> activePointIndex;
        bvector<size_t> fringeFacetIndex;
        CollectAdjacentFacetAndPointIndices (activeReadIndex, fringeFacetIndex, activePointIndex);
        // Hello World .... just translate (let the fringe deform)
        for (size_t pointIndex : activePointIndex)
            {
            m_point[pointIndex].Add (vector);
            }
        }
    else
        {

        }
    }

static void FindComplementFacetIndices (PolyfaceVisitorR visitor, bvector<size_t> &indexA, bset<size_t> &setA, bvector<size_t> &indexB)
    {
    setA.clear ();
    indexB.clear ();
    for (size_t a : indexA)
        setA.insert (a);
    for (visitor.Reset (); visitor.AdvanceToNextFace ();)
        {
        size_t ri = visitor.GetReadIndex ();
        if (setA.find (ri) == setA.end ())
            indexB.push_back (ri);
        }
    }

struct CloneAndTranslateContext
{
CloneAndTranslateContext (){}

PolyfaceHeaderPtr CloneSimpleTranslation (PolyfaceHeaderR source, bvector<size_t> &activeReadIndex, DVec3dCR vector, bool retriangulateFringe)
    {
    bvector<size_t> activePointIndex;
    bvector<size_t> fringeFacetIndex;
    PolyfaceHeaderPtr result = source.Clone ();
    source.CollectAdjacentFacetAndPointIndices (activeReadIndex, fringeFacetIndex, activePointIndex);
    bvector<DPoint3d> &points = result->Point ();
    for (size_t pointIndex : activePointIndex)
        {
        points[pointIndex].Add (vector);
        }
    if (retriangulateFringe)
        result->Triangulate (1000); // forces retriangulation of nonplanar facets.   Unfortunately does the whole mesh, not just fringe.
    return result;
    }

PolyfaceHeaderPtr CloneWithSidePanels (PolyfaceHeaderR source, bvector<size_t> &activeReadIndex, DVec3dCR vector)
    {

    // Copy the active facets to a new facet set to get boundary . ..

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (source);
    bset<size_t> activeSet;
    bvector<size_t> otherFacets;
    FindComplementFacetIndices (*visitor, activeReadIndex, activeSet, otherFacets);
    PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
    meshA->CopyAllActiveFlagsFrom (source);
    PolyfaceCoordinateMap pfmap (*meshA);
    BentleyApi::Transform transform = Transform::From (vector);
    bvector<DPoint3d> &points = visitor->Point ();
    // pull the active patch into its own mesh, already shifted ...
    for (size_t readIndex : activeReadIndex)
        {
        if (visitor->MoveToFacetByReadIndex (readIndex))
            {
            transform.Multiply (points, points);
            pfmap.AddVisitorFace (*visitor);
            }
        }


    HalfEdgeArray halfEdges (&source);
    halfEdges.BuildArray (source, false, true, true);
    halfEdges.SortForEdgeMatching ();
    bvector<Size4> partners;
    halfEdges.ExtractPartnerEdges (activeReadIndex, partners);

    for (auto &quad : partners)
        {
        visitor->ClearFacet ();
        bool error = false;
        // Move all the data in ...
        for (int i = 0; i < 4; i++)
            error |= !visitor->TryAddVertexByReadIndex (quad.m_data[i]);
        // In the partners, the m_data[0] and m_data[1] were from the active set -- move those coordinates . .
        if (!error)
            {
            visitor->Point ()[0].Add (vector);
            visitor->Point ()[1].Add (vector);
            if (!visitor->Normal().Active () || visitor->TryRecomputeNormals ())
                pfmap.AddVisitorFace (*visitor);
            }
        }
    

    // Add the other facets ...
    for (size_t readIndex : otherFacets)
        {
        if (visitor->MoveToFacetByReadIndex (readIndex))
            {
            pfmap.AddVisitorFace (*visitor);
            }
        }
    meshA->Triangulate (100);
	
	//bullseye
	//it will make all edge of mesh facets hide and which is not allowed when using mesh drag tool
    //meshA->MarkTopologicalBoundariesVisible (false);

    return meshA;
    }
};


PolyfaceHeaderPtr PolyfaceHeader::CloneWithTranslatedFacets (bvector<size_t> &activeReadIndex, DVec3dCR vector, PolyfaceHeader::FacetTranslationMode mode)
    {
    CloneAndTranslateContext context;
    if (mode == PolyfaceHeader::FacetTranslationMode::JustTranslatePoints
        || mode == PolyfaceHeader::FacetTranslationMode::TranslatePointsAndTriangulateFringeFaces)
        {
        return context.CloneSimpleTranslation (*this, activeReadIndex, vector,
                    mode == PolyfaceHeader::FacetTranslationMode::TranslatePointsAndTriangulateFringeFaces);
        }
    else
        {
        return context.CloneWithSidePanels (*this, activeReadIndex, vector);
        }
    }



END_BENTLEY_GEOMETRY_NAMESPACE
