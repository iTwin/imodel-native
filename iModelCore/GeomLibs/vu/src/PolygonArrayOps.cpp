/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



#include <algorithm>

#ifdef Compile_lessThan
static bool cbCompareTag_lessThan (TaggedPolygonCR polygonA, TaggedPolygonCR polygonB)
    {
    return polygonA.GetTag () < polygonB.GetTag ();
    }
#endif
static bool cbCompareTag_greaterThan (TaggedPolygonCR polygonA, TaggedPolygonCR polygonB)
    {
    return polygonA.GetTag () > polygonB.GetTag ();
    }

static bool LoopContainsLoop (TaggedPolygonR outerLoop, TaggedPolygonR innerLoop, DVec3dCR direction)
    {
    DPoint3d testPoint, piercePoint;
    double fraction;
    int edgeIndex;
    if (!innerLoop.TryGetPoint (0, testPoint))
        return false;
    
    int pierce = bsiPolygon_piercePoint (&piercePoint, &fraction, &edgeIndex,
            outerLoop.GetDataP (), (int)outerLoop.GetPointSize (), 
                    (DPoint3dP)&direction,
                    &testPoint,
                    (DPoint3dP)&direction, 0.0);
    return pierce >= 0;
    }

void PolygonVectorOps::Reverse (TaggedPolygonVectorR loops)
    {
    size_t n = loops.size ();
    if (n <= 1)
        return;
    for (size_t i = 0, j = n - 1; i < j; i++, j--)
        loops[i].Swap (loops[j]);
    }

void PolygonVectorOps::ReverseEachPolygon (TaggedPolygonVectorR loops)
    {
    size_t n = loops.size ();
    for (size_t i = 0; i < n; i++)
        DPoint3dOps::Reverse (loops[i].GetPointsR ());
    }
// On return, polygons are sorted into outer/inner blocks.
void PolygonVectorOps::SortAndMarkContainment (TaggedPolygonVectorR loops, DVec3dCR planeNormal, bvector<size_t>&lastIndexInBlock)
    {
    lastIndexInBlock.clear ();
    size_t n = loops.size ();
    for (size_t i = 0; i < n; i++)
        loops[i].SetTag (fabs (loops[i].Area (planeNormal)));

    std::sort (loops.begin (), loops.end (), cbCompareTag_greaterThan);
    // Outer loops must have area larger than contained loops.
    // Mark front-most free loop as outer.
    // Bring subsequent contained loops forward.
    size_t lastInBlock = 0;
    for (size_t outerIndex = 0; outerIndex < n; outerIndex = lastInBlock + 1)
        {
        // The candidate is always outer....
        lastInBlock = outerIndex;
        for (size_t holeCandidateIndex = outerIndex + 1; holeCandidateIndex < n; holeCandidateIndex++)
            {
            if (LoopContainsLoop (loops.at(outerIndex), loops.at(holeCandidateIndex), planeNormal))
                {
                // the hole candidate is contained.   But is it contained in any others?
                bool isIsland = false;
                for (size_t acceptedHoleIndex = outerIndex + 1; acceptedHoleIndex <= lastInBlock; acceptedHoleIndex++)
                    {
                    if (LoopContainsLoop (loops.at (acceptedHoleIndex), loops.at (holeCandidateIndex), planeNormal))
                        isIsland = true;
                    }
                if (!isIsland)
                    {
                    // Finally.  It really is a hole.
                    // successively swap polygons forward until holeCandidate is in place at the end of the holes
                    for (size_t i = holeCandidateIndex - 1; i > lastInBlock; i--)
                        loops[i].Swap (loops[i+1]);
                    lastInBlock++;
                    }
                }
            }
        lastIndexInBlock.push_back (lastInBlock);
        }
    }
static bool HasOverlapXY (DRange3dCR rangeA, DRange3dCR rangeB)
    {
    if (rangeA.high.x < rangeB.low.x)
        return false;
    if (rangeA.low.x > rangeB.high.x)
        return false;
    if (rangeA.high.y < rangeB.low.y)
        return false;
    if (rangeA.low.y > rangeB.high.y)
        return false;
    return true;
    }


void PolygonVectorOps::AppendWithParentIndices (TaggedPolygonVectorR dest, TaggedPolygonVectorCR source, size_t indexA)
    {
    for (size_t i = 0, n = source.size (); i < n; i++)
        {
        dest.push_back (source.at(i));
        dest.back ().SetIndexA (indexA);
        dest.back ().SetIndexB (i);
        }
    }

void PolygonVectorOps::AppendWithParentIndices (TaggedPolygonVectorR dest, TaggedPolygonVectorCR source, size_t indexA, DRange3dCR xySelectRange)
    {
    for (size_t i = 0, n = source.size (); i < n; i++)
        {
        DRange3d range = source.at(i).GetRange ();
        if (HasOverlapXY (range, xySelectRange))
            {
            dest.push_back (source.at(i));
            dest.back ().SetIndexA (indexA);
            dest.back ().SetIndexB (i);
            }
        }
    }
    
void PolygonVectorOps::ReverseForAreaXYSign (TaggedPolygonVector &polygons, double factor)
    {
    for (auto & p : polygons)
        {
        if (PolygonOps::AreaXY (p.GetPointsR ()) < 0.0)
            DPoint3dOps::Reverse (p.GetPointsR ());
        }
    }    
//! Search for polygons that are non-planar.
//! Triangulate nonplanar polygons.
//! When a polygon is triangulated, one triangle replaces it, others go at end.
void PolygonVectorOps::TriangulateNonPlanarPolygons (TaggedPolygonVectorR polygons, double abstol)
    {
    size_t originalPolygonCount = polygons.size ();
    bvector<int> indexArray;
    TaggedPolygon oldPolygon, newPolygon;
    bvector<DPoint3d> xyzArray;
    
    DPoint3d centroid;
    DVec3d   normal;
    double area, perimeter, thickness;
    for (size_t polygonIndex = 0; polygonIndex < originalPolygonCount; polygonIndex++)
        {
        oldPolygon = polygons[polygonIndex];
        size_t numOriginalVertices = oldPolygon.GetTrimmedSize (abstol);

        if (numOriginalVertices > 3)
            {
#ifdef TEST_BY_NON_PLANARITY
            if (bsiPolygon_centroidAreaPerimeter (oldPolygon.GetDataP (), numOriginalVertices,
                    &centroid, &normal, &area, &perimeter, &thickness)
                && fabs (thickness) > abstol)
#else
            // Triangulate anything nonconvex, and also convexii that are nonplanar.
            if (    !bsiGeom_testPolygonConvex (oldPolygon.GetDataP (), (int)numOriginalVertices)
                ||
                    (
                    bsiPolygon_centroidAreaPerimeter (oldPolygon.GetDataP (), (int)numOriginalVertices,
                                &centroid, &normal, &area, &perimeter, &thickness)
                    && fabs (thickness) > abstol
                    )
                )
#endif
                {
                indexArray.clear ();
                xyzArray.clear ();
                if (SUCCESS == vu_triangulateSpacePolygonExt2 (&indexArray, NULL, &xyzArray, NULL, NULL,
                        oldPolygon.GetDataP (), (int)numOriginalVertices,
                        abstol, false))
                    {
                    size_t numIndex = indexArray.size ();
                    size_t numNewPolygon = 0;
                    size_t numXYZ = xyzArray.size ();
                    for (size_t i0 = 0, i1 = 0; i1 < numIndex; i1++)
                        {
                        if (indexArray[i1] < 0)
                            {
                            newPolygon.Clear ();
                            newPolygon.CopyTagsFrom (oldPolygon);
                            for (size_t j = i0; j < i1; j++)
                                {
                                if (indexArray[j] < (int)numXYZ)
                                    newPolygon.Add (xyzArray[indexArray[j]]);
                                }
                            if (numNewPolygon == 0)
                                polygons[polygonIndex] = newPolygon;
                            else
                                polygons.push_back (newPolygon);
                            numNewPolygon++;
                            i0 = i1 + 1;
                            }
                        }
                    }
                }
            }        
        }
    }
    
END_BENTLEY_GEOMETRY_NAMESPACE
