/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_boreVolume.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct BoreMap : bmap<size_t, ValueSizeSize <DSegment3d>>
    {
    bool GetBottomZeroBasedIndexForTopOneBasedIndex (int top1, size_t &bottom)
        {
        bottom = SIZE_MAX;
        auto bore = find((size_t)(abs(top1) - 1));
        if (bore == end ())
            return false;
        auto data = bore->second;
        bottom = data.GetTagB ();
        return true;
        }
    void Map (size_t topIndex, DSegment3dCR segment, size_t segmentIndex, size_t bottomIndex)
        {
        (*this)[topIndex] = ValueSizeSize<DSegment3d> (segment, segmentIndex, bottomIndex);
        }
    };

//! Create a (closed, volumetric) mesh from "bore segment data"
//!<ul>
//!<li> Each bore segment is a pair of top and bottom points of the volume.
//!<li> Ideally the bore data is pure vertical.
//!<li> "somewhat non vertical" bores are ok.
//!<li>Severely non vertical bores will cause folded lower and side surfaces.
//!
PolyfaceHeaderPtr PolyfaceHeader::VolumeFromBoreData
(
bvector<DSegment3d> &segments,       //!< [inout] segment bottom and top.  During processing, start and end points are swapped as needed to point[0] is the lower point!!!
bool &foldedSurfaces,                //!< [out] true if bottom or side surfaces are folded.
bvector<ptrdiff_t> *topFacetReadIndex,     //!< [out] optional array of read indices of upper surface facets
bvector<ptrdiff_t> *bottomFacetReadIndex,     //!< [out] optional array of read indices of lower surface facets
bvector<ptrdiff_t> *sideFacetReadIndex     //!< [out] optional array of read indices of side facets
)
    {
    foldedSurfaces = false;
    bvector<DPoint3d> topPoints;
    // force segments to have bottom as point 0..
    for (auto &segment : segments)
        {
        if (segment.point[0].z > segment.point[1].z)
            std::swap (segment.point[0], segment.point[1]);
        topPoints.push_back (segment.point[1]);
        }
    // Simple triangulation of the top surface
    auto topMesh = PolyfaceHeader::CreateXYTriangulation (topPoints);

    topMesh->MarkTopologicalBoundariesVisible (false);

    auto options = IFacetOptions::Create ();
    auto builder = IPolyfaceConstruction::Create (*options);
    builder->AddPolyface (*topMesh);        // The topMesh has all the segment upper points.  Beware that their order in topMesh, segments, and builder are different
    // For each "top" point (in the segment array)
    //  1) find the upper point index (topIndex) in the builder point array.
    //  2) create (in the builder) a corresponding bottom point.
    //  3) enter (topIndex, segmentIndex, bottomIndex) in the bmap.

    BoreMap boreMap;
    for (size_t segmentIndex = 0; segmentIndex < segments.size (); segmentIndex++)
        {
        DSegment3d segment = segments[segmentIndex];
        size_t topIndex = builder->FindOrAddPoint (segment.point[1]);
        size_t bottomIndex = builder->FindOrAddPoint (segment.point[0]);
        boreMap.Map (topIndex, segment, segmentIndex, bottomIndex);
        }

    // Make a copy of the upper triangulation indices in the builder point arrays.
    // For each triangle in those indices:
    //  1) build a (reversed) lower triangle
    //  2) build side facets along visible edges.
    auto triangleIndices = builder->GetClientMeshR ().PointIndex ();        // COPY of point indices -- i.e. this is JUST the upper triangulation
    bvector<DPoint3d> &builderPoints = builder->GetClientMeshR ().Point ();  // REFERENCE to points
    auto &builderPointIndex = builder->GetClientMeshR ().PointIndex ();
    bvector<size_t> bottomIndex, topIndex;
    bvector<bool> visible;
    bvector<DPoint3d> bottomFacet;
    size_t iNext, iLast;
    for (size_t iFirst = 0; triangleIndices.DelimitFace (0, iFirst, iLast, iNext); iFirst = iNext)
        {
        BeAssert (iFirst + 2 == iLast); // It's supposed to be all triangles . ..
        bottomIndex.clear ();
        topIndex.clear ();
        visible.clear ();
        bottomFacet.clear ();

        if (topFacetReadIndex != nullptr)
            topFacetReadIndex->push_back (iFirst);

        for (size_t i = iFirst; i <= iLast; i++)
            {
            size_t bottomIndex0;
            if (boreMap.GetBottomZeroBasedIndexForTopOneBasedIndex (triangleIndices[i], bottomIndex0))
                {
                topIndex.push_back ((size_t)(abs (triangleIndices[i]) - 1));
                bottomIndex.push_back (bottomIndex0);
                bottomFacet.push_back (builderPoints[bottomIndex0]);
                visible.push_back (triangleIndices[i] > 0);
                }
            }
        // bottom facet should be upward facing ...
        if (PolygonOps::AreaXY (bottomFacet) < 0.0)
            foldedSurfaces = true;
        // topIndex = (zero based) point indices around an upper facet (triangle).
        // bottomIndex = (zero based) point indices around the corresponding lower facet (triangle, upward facing -- need to reverse bottom construction)
        // visible = true for true exterior edge.
        if (iFirst + bottomIndex.size() == iLast + 1)
            {
            size_t numEdge = bottomIndex.size ();       // We expect this to be 3, but this code block does not require it
            bottomIndex.push_back (bottomIndex.front ());
            topIndex.push_back (topIndex.front ());
            visible.push_back (visible.front ());
            // Add mirrored face on bottom.
            if (bottomFacetReadIndex != nullptr)
                bottomFacetReadIndex->push_back (builderPointIndex.size ());
            
            for (size_t i1 = numEdge; i1-- > 0;)
                builder->AddPointIndex (bottomIndex[i1], true);
            builder->AddPointIndexTerminator ();

            // Add side quads where upper edge visible
            for (size_t i = 0; i < numEdge; i++)
                {
                if (visible[i])
                    {
                    size_t top0 = topIndex[i];
                    size_t top1 = topIndex[i+1];
                    size_t bottom0 = bottomIndex[i];
                    size_t bottom1 = bottomIndex[i+1];
                    DPoint3d top0XYZ = builderPoints[top0];
                    DPoint3d top1XYZ = builderPoints[top1];
                    DPoint3d bottom0XYZ = builderPoints[bottom0];
                    DPoint3d bottom1XYZ = builderPoints[bottom1];
                    DVec3d crossA = DVec3d::FromCrossProductToPoints (top0XYZ, bottom0XYZ, top1XYZ);
                    DVec3d crossB = DVec3d::FromCrossProductToPoints (top1XYZ, bottom0XYZ, bottom1XYZ);
                    if (crossA.DotProduct (crossB) <= 0.0)
                        foldedSurfaces = true;
                    if (sideFacetReadIndex != nullptr)
                        sideFacetReadIndex->push_back (builderPointIndex.size ());
                    builder->AddPointIndexTriangle (top0, true, bottom0, true, top1, true);
                    if (sideFacetReadIndex != nullptr)
                        sideFacetReadIndex->push_back (builderPointIndex.size ());
                    builder->AddPointIndexTriangle (top1, true, bottom0, true, bottom1, true);
                    }
                }
            }
        }
    builder->GetClientMeshR ().MarkAllEdgesVisible ();
    return &builder->GetClientMeshR ();
    }


END_BENTLEY_GEOMETRY_NAMESPACE
