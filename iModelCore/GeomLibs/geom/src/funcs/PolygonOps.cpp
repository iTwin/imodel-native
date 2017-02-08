/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/PolygonOps.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Compute the volume of the tent formed by a polygon base and a given "pole" vector.
* @remarks The tent is a collection of tetrahedra.  Each tetrahedron is formed from the first
*       point of the polygon, the top of the pole vector, and two points of a polygon edge.
* @param pPointArray => polygon vertices
* @param numPoint => number of polygon vertices
* @param pDirection => the vector along the pole
* @return summed (possibly negative) tent volume
* @group Polygons
* @bsimethod                                                    EarlinLutz      03/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double           bsiPolygon_tentVolume

(
DPoint3dCP pPointArray,
int             numPoint,
DVec3dCP    pDirection
)
    {
    DPoint3d vector0, vector1;
    int i;
    double volume = 0.0;

    vector0 = DVec3d::FromStartEnd (pPointArray[0], pPointArray[1]);

    for (i = 2; i < numPoint; i++)
        {
        vector1 = DVec3d::FromStartEnd (pPointArray[0], pPointArray[i]);
        volume += vector0.TripleProduct (vector1, *pDirection);
        vector0 = vector1;
        }
    volume /= 6.0;
    return volume;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the normal and the area of the polygon.
@remarks This function computes vectors from the first point to all other points, and sums
    the cross product of each successive pair of these vectors.  For a planar polygon, this
    vector's length is the area of the polygon, and the direction is the plane normal.
@param pNormal OUT polygon unit normal
@param pOrigin OUT origin (first point in array)
@param pXYZ IN polygon points
@param numXYZ IN number of polygon points
@return half the pre-normalization magnitude of the cross product sum.
@group Polygons
@bsimethod                                                      EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiPolygon_polygonNormalAndArea

(
DVec3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pXYZ,
int numXYZ
)
    {
    int         i;
    DVec3d    normal;


    normal.x = normal.y = normal.z = 0.0;

    if (!pXYZ)
        return 0.0;

    if (pOrigin)
        *pOrigin = pXYZ[0];

    // default normal is zero vector (could be problematic...)
    if (pNormal)
        *pNormal = normal;
    else
        pNormal = &normal;

    if (numXYZ < 3)
        return 0.0;

    if (numXYZ == 3)
        {
        pNormal->CrossProductToPoints (pXYZ[0], pXYZ[1], pXYZ[2]);
        }
    else
        {
        /* compute normal = vector whose x component is the area of the polygon projected
            on the yz plane, etc.   The ratios of these areas are the normal.
        */
        DVec3d uVec, vVec, wVec;
        DPoint3d basePoint;
        int numThisLoop = 0;
        for (i = 0; i < numXYZ; i++)
            {
            if (pXYZ[i].IsDisconnect ())
                {
                numThisLoop = 0;
                }
            else if (numThisLoop == 0)
                {
                basePoint = pXYZ[i];
                numThisLoop++;
                }
            else if (numThisLoop == 1)
                {
                uVec.DifferenceOf (pXYZ[i], basePoint);
                numThisLoop++;
                }
            else
                {
                vVec.DifferenceOf (pXYZ[i], basePoint);
                wVec.CrossProduct (uVec, vVec);
                pNormal->SumOf (*pNormal, wVec);
                uVec = vVec;
                numThisLoop++;
                }
            }
        }
    return 0.5 * pNormal->Normalize ();
    }

/*---------------------------------------------------------------------------------**//**
@description Compute centroid, unit normal, and area of a polygon.
@remarks Duplication of first/last points is optional and will not affect results.
@remarks Disconnect points may be used to separate loops.    Caller is responsible for ensuring compatible orientations of outer and holes.
  (e.g. outer loops CCW, holes CW).  There is no test for containment or compatible orientation.
@param pXYZ IN array of points
@param numXYZ IN number of points
@param pCentroid OUT centroid of polygon
@param pUnitNormal OUT unit normal to best fit plane
@param pArea OUT area of polygon
@param pPerimeter OUT perimeter of polygon
@param pMaxPlanarError OUT max height difference between polygon points above and below the best fit plane
@return true if polygon has nonzero area
@group Polygons
@bsimethod                                                    EarlinLutz      06/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_centroidAreaPerimeter

(
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dP pCentroid,
DVec3dP pUnitNormal,
double      *pArea,
double      *pPerimeter,
double      *pMaxPlanarError
)
    {
    DPoint3d origin;
    DPoint3d centroid;
    DVec3d centroidVector;
    DVec3d   normal;
    bool    boolstat = false;
    double perimeter = 0.0;
    double maxError = 0.0;
    centroid.Zero ();
    double area1 = bsiPolygon_polygonNormalAndArea (&normal, &origin, pXYZ, numXYZ);
    double area = 0.0;
    if (area1 != 0.0)
        {
        int numThisLoop = 0;
        int i;
        DPoint3d basePoint;
        DVec3d vecA, vecB;
        double da;
        double b;
        double scaleFactor = 1.0 / 3.0;
        centroidVector.Zero ();
        for (i = 0; i < numXYZ; i++ )
            {
            if (pXYZ[i].IsDisconnect ())
                {
                if (numThisLoop > 1)
                    perimeter += vecA.Magnitude ();
                numThisLoop = 0;
                }
            else if (numThisLoop == 0)
                {
                basePoint = pXYZ[i];
                numThisLoop++;
                }
            else if (numThisLoop == 1)
                {
                vecA.DifferenceOf (pXYZ[i], basePoint);
                perimeter += vecA.Magnitude ();
                numThisLoop++;
                }
            else
                {
                vecB.DifferenceOf (pXYZ[i], basePoint);
                da = 0.5 * normal.TripleProduct (vecA, vecB);
                area += da;
                b = da * scaleFactor;
                centroidVector.SumOf (centroidVector, vecA, b, vecB, b);
                perimeter += vecA.Distance (vecB);
                vecA = vecB;
                numThisLoop++;
                }
            }

        if (numThisLoop > 1)
            perimeter += vecA.Magnitude ();
        // For planar case, area and area1 should be the same.
        // Nonplanar? Who knows.
        if (area != 0.0)
            {
            centroid.SumOf (basePoint, centroidVector, 1.0 / area);
            boolstat = true;
            }

        if (pMaxPlanarError)
            {
            DRange1d range = DRange1d::FromAltitudes (pXYZ, (size_t)numXYZ, DPlane3d::FromOriginAndNormal (centroid, normal));
            maxError = range.High () - range.Low ();
            }
        }

    if (pCentroid)
        *pCentroid = centroid;
    if (pUnitNormal)
        *pUnitNormal = normal;
    if (pArea)
        *pArea = area;
    if (pPerimeter)
        *pPerimeter = perimeter;
    if (pMaxPlanarError)
        *pMaxPlanarError = maxError;

    return boolstat;
    }


DVec3d PolygonOps::AreaNormal (bvector<DPoint3d> const &xyz, size_t numUse)
    {
    size_t count = xyz.size ();
    if (numUse < count)
        count = numUse;
        
    DVec3d sum;
    sum.Zero ();
    for (size_t i = 2; i < count; i++)
        {
        if (xyz[i-1].IsDisconnect () || xyz[i].IsDisconnect ())
            {
            }
        else
            {
            DVec3d cross;
            cross.CrossProductToPoints (xyz[0], xyz[i-1], xyz[i]);
            sum.SumOf (sum, cross);
            }
        }
    sum.Scale (0.5);
    return sum;
    }

DVec3d PolygonOps::AreaNormal (bvector<DPoint3d> const &xyz)
    {
    return AreaNormal (xyz, xyz.size ());
    }

bool PolygonOps::IsConvex (bvector<DPoint3d> const &xyz)
    {
    DVec3d vecA, vecB, maxCross, cross;
    int numPoint = (int)xyz.size ();
    static double s_areaRelTol = 1.0e-12;
    DVec3d unitNormal;

    double positiveArea = 0.0;
    double negativeArea = 0.0;

    while (numPoint > 2 && xyz[numPoint - 1].IsEqual (xyz[0]))
        numPoint--;

    if (numPoint < 3)
        return false;
    double a2 = 0.0;
    maxCross.Zero ();
    // Find largest cross product as reference vector.
    vecA.DifferenceOf (xyz[1], xyz[0]);
    for (int i = 2; i < numPoint; i++, vecA = vecB)
        {
        vecB.DifferenceOf (xyz[i], xyz[0]);
        cross.CrossProduct (vecA, vecB);
        double b2 = cross.MagnitudeSquared ();
        if (b2 > a2)
            {
            a2 = b2;
            maxCross = cross;
            }
        }

    unitNormal.Normalize (maxCross);
    // 
    vecA.DifferenceOf (xyz[0], xyz[numPoint - 1]);
    for (int i = 1; i <= numPoint; i++, vecA = vecB)
        {
        vecB.DifferenceOf (xyz[i % numPoint], xyz[i-1]);
        cross.CrossProduct (vecA, vecB);
        double b = cross.DotProduct (unitNormal);
        if (b >= 0.0)
            positiveArea += b;
        else
            negativeArea += b;
        }

    return fabs (negativeArea) < s_areaRelTol * positiveArea;
    }

/**
* Triangulate a single xy polygon.  Triangulation preserves original
*   indices.
* @param pSignedOneBasedIndices <= array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pExteriorLoopIndices <= array of indices of actual outer loops. (optional)
*           (These are clockwise loops as viewed.)
* @param pXYZOut <= output points.  The first numPoint points are exactly
*           the input points.   If necessary and permitted, additional
*           xyz are added at crossings.  In the usual case in which crossings
*           are not expected, this array may be NULL.
* @param pointP => array of polygon points.
* @param numPoint => number of polygon points.
* @param xyTolerance => tolerance for short edges on input polygon.
* @param signedOneBasedIndices => if true, output indices are 1 based, with 0 as separator.
           If false, indices are zero based wtih -1 as separator.
* @param addVerticesAtCrossings => true if new coorinates can be added to pXYZOut
* @return false if nonsimple polygon.
*/
bool PolygonOps::FixupAndTriangulateLoopsXY
(
bvector<int>    *pIndices,
bvector<int>    *pExteriorLoopIndices,
bvector<DPoint3d> *pXYZOut,
bvector<DPoint3d> *pXYZIn,
double              xyTolerance,
int                 maxPerFace,
bool             signedOneBasedIndices,
bool             addVerticesAtCrossings
)
    {
#ifdef BUILD_FOR_811
    return false;
#else
    VuSetP      graphP = vu_newVuSet (0);
    VuArrayP    faceArrayP = vu_grabArray (graphP);
    VuMask      numberedNodeMask = vu_grabMask (graphP);
    VuP         faceP, originalNodeP;
    bool status = false;
    int       i, numError;
    int       originalIndex;
    int       outputIndex;
    int     separator = signedOneBasedIndices ? 0 : -1;

    pIndices->clear ();
    if (pExteriorLoopIndices != NULL)
        pExteriorLoopIndices->clear ();
    DPoint3dOps::Copy (pXYZOut, pXYZIn);

    vu_clearMaskInSet (graphP, numberedNodeMask);

    VuMask newNodeMask = numberedNodeMask | VU_BOUNDARY_EDGE;
    VuOps::MakeIndexedLoopsFromArrayWithDisconnects (graphP, pXYZIn,
                newNodeMask, newNodeMask, xyTolerance);

    // TR #191917: keep dangling edges in triangulation
    vu_mergeOrUnionLoops (graphP, VUUNION_UNION);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_splitMonotoneFacesToEdgeLimit (graphP, maxPerFace);

    if (maxPerFace == 3)
        vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);

    vu_collectInteriorFaceLoops (faceArrayP, graphP);
    numError = 0;

    vu_arrayOpen (faceArrayP);
    status = true;
    for (i = 0; status && vu_arrayRead (faceArrayP, &faceP); i++)
        {
        // We triangulated.  So of course there are 3 nodes per face.
        // Really?  If the input polygon retraces itself, there will be
        // sliver faces with only 2 edges.
        if (vu_faceLoopSize (faceP) < 3)
            continue;

        VU_FACE_LOOP (currP, faceP)
            {
            /* Is this an original node, or is it an intersection? */
            originalNodeP = vu_findMaskAroundVertex (currP, numberedNodeMask);
            if (originalNodeP)
                {
                originalIndex = vu_getUserDataPAsInt (originalNodeP);
                if (signedOneBasedIndices)
                    {
                    outputIndex = 1 + originalIndex;
                    if (!vu_getMask (currP, newNodeMask))
                        outputIndex = -outputIndex;
                    }
                else
                    outputIndex = originalIndex;
                pIndices->push_back (outputIndex);
                }
            else if (addVerticesAtCrossings  && pXYZOut)
                {
                /* It's a new point.  Add the coordinates, assign an index,
                   and promote outgoing edges to first class status */
                DPoint3d newXYZ;
                vu_getDPoint3d (&newXYZ, currP);
                pXYZOut->push_back (newXYZ);
                int newVertexIndex = (int)pXYZOut->size ();  // 1-based

                vu_setMask (currP, numberedNodeMask);
                vu_setUserDataPAsInt (currP, newVertexIndex-1);

                if (signedOneBasedIndices)
                    {
                    outputIndex = newVertexIndex;
                    if (!vu_getMask (currP, newNodeMask))
                        outputIndex = -outputIndex;
                    }
                else
                    outputIndex = newVertexIndex - 1;

                pIndices->push_back(outputIndex);
                }
            else
                {
                status = false;
                break;
                }
            }
        END_VU_FACE_LOOP (currP, faceP)
        pIndices->push_back (separator);
        }

    // Exterior loops
    if (status && pExteriorLoopIndices)
        {
        // There may be edges closing over concave parts of the exterior.
        // These will be "exterior" on both sides -- delete them.
        vu_freeEdgesByMaskCount (graphP, VU_EXTERIOR_EDGE, false, false, true);

        vu_collectExteriorFaceLoops (faceArrayP, graphP);
        vu_arrayOpen (faceArrayP);
        status = true;
        for (i = 0; status && vu_arrayRead (faceArrayP, &faceP); i++)
            {
            VuP lowIndexP = faceP;
            int lowIndex = vu_getUserDataPAsInt (faceP);

            VU_FACE_LOOP (currP, faceP)
                {
                int index = vu_getUserDataPAsInt (currP);
                if (index < lowIndex)
                    {
                    lowIndex = index;
                    lowIndexP = currP;
                    }
                }
            END_VU_FACE_LOOP (currP, faceP)

            VU_FACE_LOOP (currP, lowIndexP)
                {
                int index = vu_getUserDataPAsInt (currP);
                if (signedOneBasedIndices)
                    outputIndex = index + 1;
                else
                    outputIndex = index;
                pExteriorLoopIndices->push_back ( outputIndex);
                }
            END_VU_FACE_LOOP (currP, lowIndexP)
            pExteriorLoopIndices->push_back ( separator);
            }
        }

    vu_returnMask (graphP, numberedNodeMask);
    vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);

    return status;
#endif
    }


bool PolygonOps::CoordinateFrame
(
bvector<DPoint3d> *pXYZIn,
TransformR  localToWorld,
TransformR  worldToLocal
)
    {
    return CoordinateFrame (&pXYZIn->at(0), pXYZIn->size (), localToWorld, worldToLocal);
    }

bool PolygonOps::CoordinateFrame
(
DPoint3dCP pXYZIn,
size_t    numXYZ,
TransformR  localToWorld,
TransformR  worldToLocal
)
    {
    return CoordinateFrame (pXYZIn, numXYZ, localToWorld, worldToLocal, 
            LOCAL_COORDINATE_SCALE_UnitAxesAtStart);
    }

bool PolygonOps::CoordinateFrame
(
DPoint3dCP pXYZIn,
size_t    numXYZ,
TransformR  localToWorld,
TransformR  worldToLocal,
enum LocalCoordinateSelect selector
)
    {
    size_t numBeforeDisconnect = 0;
    //bool bDone = false;
    static double sFirstEdgeFactor = 0.001;

    bool bOK = false;
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();

    for (size_t i = 0; i < numXYZ && !pXYZIn[i].IsDisconnect(); i++)
        {
        numBeforeDisconnect++;
        }

    if (numBeforeDisconnect >= 3)
        {
        // Capture x direction along first edge, and upwards orientation by first polygon area vectors ..
        DPoint3d origin;
        DVec3d normal;
        DVec3d unit01;
        RotMatrix axes, skewFrame;

        double area = bsiPolygon_polygonNormalAndArea (&normal, &origin, pXYZIn, (int)numBeforeDisconnect);
        double lengthRef = sqrt (area);
        bOK = false;
        for (size_t k = 1; k < numBeforeDisconnect && !bOK; k++)
            {
            double mag01 = unit01.NormalizedDifference(pXYZIn[k], pXYZIn[0]);
            if (mag01 > sFirstEdgeFactor * lengthRef)
                {
                DVec3d perpVector;
                perpVector.CrossProduct (normal, unit01);
                skewFrame.InitFromColumnVectors (unit01, perpVector, normal);
                // Policy decision:  The normal is the most reliable thing.  Keep it exactly, even if
                // the first edge ends up not being exactly in plane.
                axes.SquareAndNormalizeColumns (skewFrame, 2, 0);
                localToWorld.InitFrom (axes, pXYZIn[0]);
                bOK = worldToLocal.InverseOf (localToWorld) ? true : false;
                }
            }
        }


    if (bOK && selector != LOCAL_COORDINATE_SCALE_UnitAxesAtStart)
        {
        DRange3d localRange;
        localRange.Init ();
        localRange.Extend (worldToLocal, pXYZIn, (int)numXYZ);
        Transform::CorrectCoordinateFrameXYRange (localToWorld, worldToLocal, localRange, selector);
        }
    return bOK;
    }

bool PolygonOps::FixupAndTriangulateProjectedLoops
(
bvector<int>*       pIndices,
bvector<int>*       pExteriorLoopIndices,
bvector<DPoint3d>*  pXYZOut,
TransformCR             localToWorld,
TransformCR             worldToLocal,
bvector<DPoint3d>   *pXYZIn,
double                  xyTolerance,
bool                 bSignedOneBasedIndices
)
    {
    bvector<DPoint3d> localXYZ;
    bool status = false;

    if (!pIndices || !pXYZIn)
        return false;

    if (pIndices != NULL)
        pIndices->clear ();
    if (pXYZOut != NULL)
        pXYZOut->clear ();
    if (pExteriorLoopIndices != NULL)
        pExteriorLoopIndices->clear ();
    DPoint3dOps::Copy (&localXYZ, pXYZIn);

    DPoint3dOps::Multiply (&localXYZ, worldToLocal);

    status = FixupAndTriangulateLoopsXY (pIndices, pExteriorLoopIndices, pXYZOut, &localXYZ, xyTolerance, 3, bSignedOneBasedIndices, pXYZOut != NULL);
    if (pXYZOut != NULL)
        DPoint3dOps::Multiply (pXYZOut, localToWorld);
    return status;
    }

bool PolygonOps::FixupAndTriangulateSpaceLoops
(
bvector<int>*       pIndices,
bvector<int>*       pExteriorLoopIndices,
bvector<DPoint3d>*  pXYZOut,
TransformR              localToWorld,
TransformR              worldToLocal,
bvector<DPoint3d>*  pXYZIn,
double                  xyTolerance,
bool                 bSignedOneBasedIndices
)
    {
    //size_t numPoints = pXYZIn->size ();
    bool                   status = false;

    if (CoordinateFrame (pXYZIn, localToWorld, worldToLocal))
        {
        status = FixupAndTriangulateProjectedLoops
                    (
                    pIndices, pExteriorLoopIndices, pXYZOut,
                    localToWorld, worldToLocal,
                    pXYZIn, xyTolerance, bSignedOneBasedIndices);
        }

    return status;
    }

bool PolygonOps::ReorientTriangulationIndices
(
bvector<int>    &indices,
bool             *pbReversed,
bool             bSignedOneBasedIndices
)
    {
    size_t n = indices.size ();
    size_t i, nInc, nDec;
    int test, succ, term = bSignedOneBasedIndices ? 0 : -1;

    if (pbReversed)
        *pbReversed = false;

    // assume triangles
    if (n % 4)
        return false;

    // count original edges in original (increasing) and reversed (decreasing) orientations
    for (i = nInc = nDec = 0; i < n; i++)
        {
        if ((test = indices[i]) > term)
            {
            succ = (((i+1) % 4) == 3) ? indices[i - 2] : indices[i + 1];
            if (succ < 0)
                succ = -succ;

            if (succ == test + 1)
                nInc++;
            else if (succ == test - 1)
                nDec++;
            }
        }

    // TR #191917: We used to stop at the first original edge found, however when the original edge is dangling, it may legitimately occur
    //             in the triangulation with reversed orientation (e.g., without requiring reorientation).  Without comparing vertices, we
    //             can only use a heuristic to make the decision to reorient given the possibility of dangling edges.
    if (nInc >= nDec)
        return true;

    // rotate signs
    if (bSignedOneBasedIndices)
        {
        for (i = 0; i < n; i+=4)
            {
            if (indices[i] < 0)
                {
                if (indices[i+1] < 0)
                    {
                    if (indices[i+2] < 0)
                        {
                        // all negative => no rotation
                        }
                    else
                        {
                        indices[i]   *= -1;
                        indices[i+2] *= -1;
                        }
                    }
                else
                    {
                    if (indices[i+2] < 0)
                        {
                        indices[i+1] *= -1;
                        indices[i+2] *= -1;
                        }
                    else
                        {
                        indices[i]   *= -1;
                        indices[i+1] *= -1;
                        }
                    }
                }
            else
                {
                if (indices[i+1] < 0)
                    {
                    if (indices[i+2] < 0)
                        {
                        indices[i]   *= -1;
                        indices[i+1] *= -1;
                        }
                    else
                        {
                        indices[i+1] *= -1;
                        indices[i+2] *= -1;
                        }
                    }
                else
                    {
                    if (indices[i+2] < 0)
                        {
                        indices[i]   *= -1;
                        indices[i+2] *= -1;
                        }
                    else
                        {
                        // all positive => no rotation
                        }
                    }
                }
            }
        }

    // reverse triangle orientations
    for (i = 0; i < n; i+=4)
        {
        int t = indices[i+1];
        indices[i+1] = indices[i+2];
        indices[i+2] = t;
        }

    if (pbReversed)
        *pbReversed = true;

    return true;
    }

double PolygonOps::AreaXY (bvector<DPoint3d> const &xyz)
    {
    return AreaXY (&xyz[0], (int)xyz.size ());
    }

double PolygonOps::AreaXY (DPoint3dCP pPointArray, size_t numPoint)
    {
    double area = 0.0;
    DVec3d head, tail;
    DPoint3d origin;
    head.Zero ();
    if (numPoint > 2)
        {
        size_t numThisLoop = 0;
        for (size_t i = 0; i < (size_t)numPoint; i++)
            {
            if (pPointArray[i].IsDisconnect ())
                {
                numThisLoop = 0;
                }
            else if (numThisLoop == 0)
                {
                origin = pPointArray[i];
                numThisLoop++;
                }
            else if (numThisLoop == 1)
                {
                head.DifferenceOf (pPointArray[i], origin);
                numThisLoop++;
                }
            else
                {
                tail = head;
                head.DifferenceOf (pPointArray[i], origin);
                area += tail.x * head.y - tail.y * head.x;
                numThisLoop++;
                }
            }
        area *= 0.5;
        }
    return  area;
    }


double PolygonOps::Area (bvector<DPoint2d> const &xyz)
    {
    return PolygonOps::Area (&xyz[0], (int)xyz.size ());
    }

double PolygonOps::Area (DPoint2dCP pPointArray, size_t numPoint)
    {
    double area = 0.0;
    DVec2d head, tail;
    DPoint2d origin;
    head.Zero ();
    if (numPoint > 2)
        {
        size_t numThisLoop = 0;
        for (size_t i = 0; i < (size_t)numPoint; i++)
            {
            if (pPointArray[i].IsDisconnect ())
                {
                numThisLoop = 0;
                }
            else if (numThisLoop == 0)
                {
                origin = pPointArray[i];
                numThisLoop++;
                }
            else if (numThisLoop == 1)
                {
                head.DifferenceOf (pPointArray[i], origin);
                numThisLoop++;
                }
            else
                {
                tail = head;
                head.DifferenceOf (pPointArray[i], origin);
                area += tail.x * head.y - tail.y * head.x;
                numThisLoop++;
                }
            }
        area *= 0.5;
        }
    return  area;
    }

bool PolygonOps::CentroidAndArea (bvector<DPoint2d> &points, DPoint2dR centroid, double &area)
    {
    area = 0.0;
    centroid = DPoint2d::From (0,0);
    if (points.size () < 3)
        return false;
    DPoint2d origin = points[0];
    DVec2d vectorSum = DVec2d::From(0, 0);  // == sum ((U+V)/3) * (U CROSS V)/2 -- but leave out divisions
    double    areaSum = 0.0;    // == sum (U CROSS V) / 2 -- but leave out divisions
    origin = points[0];
    for (size_t i = 1; i + 1< points.size (); i++)
        {
        DVec2d vector0 = DVec2d::FromStartEnd (origin, points[i]);
        DVec2d vector1 = DVec2d::FromStartEnd (origin, points[i+1]);
        double area = vector0.CrossProduct (vector1);
        vectorSum = vectorSum + (vector0 + vector1) * area;
        areaSum += area;
        }
    area = areaSum * 0.5;
    double a;
    if (DoubleOps::SafeDivide (a, 1.0, 6.0 * area, 0.0))
        {
        centroid = origin + vectorSum * a;
        return true;
        }
    centroid = origin;
    return false;
    }


bool PolygonOps::CentroidNormalAndArea (bvector<DPoint3d> const &xyz, DPoint3dR centroid, DVec3dR normal, double &area)
    {
    return bsiPolygon_centroidAreaPerimeter (
                xyz.data (),
                (int)xyz.size (),
                &centroid, &normal, &area, NULL, NULL);
    }

bool PolygonOps::SecondAreaMomentProducts (bvector<DPoint3d> const &xyz, DPoint3dCR origin, DMatrix4dR products)
    {
    // localProducts * localFactor is the integrals of [uu, uv, 0, u; uv vv 0 v; 0 0 0 0; u v 0 1] over triangle 00,10,01 in uv plane.
    DMatrix4d localProducts = DMatrix4d::FromRowValues
        (
        2, 1, 0, 4,
        1, 2, 0, 4,
        0, 0, 0, 0,
        4, 4, 0, 12
        );
    Transform T;
    double localFactor = 1.0 / 24;
    DPoint3d centroid;
    DVec3d normal;
    double area;
    products = DMatrix4d::FromZero ();
    if (bsiPolygon_centroidAreaPerimeter (
                const_cast <DPoint3dP>(&xyz.at(0)),
                (int)xyz.size (),
                &centroid, &normal, &area, NULL, NULL))
        {
        size_t numPoints = xyz.size ();
        while (numPoints > 1 && xyz[0].IsEqual (xyz[numPoints - 1]))
            numPoints--;        
        DPoint3d point0 = xyz[0];
        DVec3d vec01, vec02;
        double detJ;
        DMatrix4d triangleProducts;
        for (size_t i2 = 2; i2 < numPoints; i2++)
            {
            vec01.DifferenceOf (xyz[i2-1], point0);
            vec02.DifferenceOf (xyz[i2], point0);
            detJ = normal.TripleProduct (vec01, vec02);
            T.InitFromOriginAndVectors (point0, vec01, vec02, normal);  // should z column just be zeros?   Doesn't matter -- it only multiplies by 0
            triangleProducts = DMatrix4d::FromSandwichProduct (T, localProducts, localFactor * detJ);
            products.Add (triangleProducts);
            }
        return true;
        }
    return false;
    }



bool PolygonOps::CentroidNormalAndArea (DPoint3dCP pXYZ, size_t numXYZ, DPoint3dR centroid, DVec3dR normal, double &area)
    {
#ifdef BUILD_FOR_811
    return bsiPolygon_centroidAreaPerimeter (
                const_cast <DPoint3dP>(pXYZ), (int)numXYZ,
                &centroid, &normal, &area, NULL, NULL) ? true : false;
#else
    return bsiPolygon_centroidAreaPerimeter (
                const_cast <DPoint3dP>(pXYZ), (int)numXYZ,
                &centroid, &normal, &area, NULL, NULL);
#endif
    }

// Test logic for polygon containment with based on triangles.
// For 2d:
//  vectorP = vector from base point to test point
//  In a triangle from base point to points 0 and 1 of a far edge.
//   a0=vector0 CROSS vector
//   a1=vector CROSS vector1
// An interior point of the (unbounded) sector has same sign of a0 and a1.
// The a0 for the "next" triangle is "current" a1 negated.
struct BarycentricTester
{
private:
size_t m_numHit;
double m_a0;
double m_b0, m_b1;
double m_c0, m_c1, m_c01;
//DPoint3d m_uvw;   never used
size_t m_savedHitIndex;
size_t m_numTest;
public:
BarycentricTester (){};

bool GetBarycentric (DPoint3dR uvw, size_t &index)
    {
    if (0 != (m_numHit & 0x01)
        && m_savedHitIndex != SIZE_MAX
        && m_c01 != 0.0)
        {
        uvw.z = m_c0 / m_c01;
        uvw.y = m_c1 / m_c01;
        uvw.x = 1.0 - uvw.y - uvw.z;
        index = m_savedHitIndex;
        return true;
        }
    return false;
    }
void Reset (double a0)
    {
    m_a0 = a0;
    m_numHit = 0;
    m_b0 = m_b1 = 0.0;
    m_c0 = m_c1 = 0.0;
    m_c01 = 1.0;
    m_savedHitIndex = SIZE_MAX;
    };
bool Test1 (double a1)
    {
    bool between = false;
    m_b0 = m_a0;
    m_b1 = a1;
    if (m_a0 >= 0.0)
        {
        between = a1 > 0.0;
        }
    else
        {
        between = a1 < 0.0;
        }
    m_a0 = - a1;
    m_numTest++;
    return between;
    }

bool Test2 (double a01, size_t index)
    {
    double delta = a01 - m_b0 - m_b1;
    if (a01 == 0.0)
        return false;
    if (a01 * m_b0 < 0.0)
        return false;
    if (a01 * m_b1 < 0.0)
        return false;
    if (delta * a01 < 0.0)
        return false;
    if (m_numHit == 0 || fabs (a01) > fabs (m_c01))
        {
        m_c0 = m_b0;
        m_c1 = m_b1;
        m_c01 = a01;
        m_savedHitIndex = index;
        }
    m_numHit++;
    return true;
    }
};

bool PolygonOps::PickTriangleFromStart (DPoint2dCP xyPoints, size_t n, DPoint2dCR xy,
            size_t &edgeBaseIndex, DPoint3dR uvw, DPoint3dR duvwdX, DPoint3dCR duvwdY)
    {
    edgeBaseIndex = 0;
    if (n < 3)
        return false;
    BarycentricTester tester;
    DVec2d uvTest = DVec2d::FromStartEnd (xyPoints[0], xy);
    DVec2d uvSweep0 = DVec2d::FromStartEnd (xyPoints[0], xyPoints[1]);
    DVec2d uvSweep1;
    tester.Reset (uvSweep0.CrossProduct (uvTest));
    for (size_t i1 = 2; i1 < n; i1++, uvSweep0 = uvSweep1)
        {
        uvSweep1.DifferenceOf (xyPoints[i1], xyPoints[0]);
        if (tester.Test1 (uvTest.CrossProduct (uvSweep1)))
            tester.Test2 (uvSweep0.CrossProduct (uvSweep1), i1 - 1);
        }
    if (tester.GetBarycentric (uvw, edgeBaseIndex))
        {
        return true;
        }
    return false;
    }

bool PolygonOps::PickTriangleFromStart
    (
    DPoint3dCP pXYZ,
    size_t numXYZ,
    DRay3dCR ray,
    DPoint3dR xyz,
    DPoint3dR triangleFractions,
    double &rayFraction,
    size_t &edgeBaseIndex
    )
    {
    edgeBaseIndex = 0;
    if (numXYZ < 3)
        return false;
    DTriangle3d triangle;
    triangle.point[0] = pXYZ[0];
    double fi;
    DPoint3d xyzi;
    size_t numHit = 0;
    for (size_t i0 = 1, i1 = 2; i1 < numXYZ; i0 = i1++)
        {
        DPoint3d uvwLocal;
        triangle.point[1] = pXYZ[i0];
        triangle.point[2] = pXYZ[i1];
        if (triangle.TransverseIntersection (ray, xyzi, uvwLocal, fi))
            {
            edgeBaseIndex = i0;
            xyz = xyzi;
            triangleFractions = uvwLocal;
            rayFraction = fi;
            numHit++;
            return true;
            }
        }
    return false;
    }



//! @param [out] touchData details about exact "on" points and segments
//!<pre>
//!<ul>
//!<li> A segment that is completely on is recorded as its start point with segment index, and a=1.0;
//!<li> A single point touch without crossing is recorded as the point, its index, and a=0.0
//!</ul>
//!<pre>
PlanePolygonSSICode PolygonOps::PlaneIntersectionPoints
(
bvector<DPoint3d> const &points,
DPlane3dCR plane,
double touchTolerance,
bvector<CurveLocationDetail> &trueCrossings,
bvector<CurveLocationDetail> *touchData,
DRange1d &altitudeRange
)
    {
    trueCrossings.clear ();
    if (NULL != touchData)
        touchData->clear ();
    altitudeRange = DRange1d ();
    double hA = 0.0;
    size_t iStart = SIZE_MAX;      // make this the index of any strictly nonzero point ..
    size_t n = points.size ();
    if (n < 2)
        return PlanePolygonSSICode::Unknown;
    hA = plane.Evaluate (points[0]);
    iStart = 0;
    altitudeRange.Extend (hA);
    if (hA == 0.0)
        {
        iStart = SIZE_MAX;
        // walk around as needed to find a true off-plane start point.
        for (size_t i = 0; i < n; i++)
            {
            double h = DoubleOps::SnapZero (plane.Evaluate (points[i]), touchTolerance);
            if (h != 0.0)
                {
                hA = h;
                iStart = i;
                break;
                }
            }
        if (iStart == SIZE_MAX)
            {
            return PlanePolygonSSICode::Coincident;
            }
        }

    // h0, h1, hA are true altitudes.
    double h1, h0 = hA;
    // hA is the most recent strictly nonzero vertex.
    // h0 is the tail of the current edge.
    // h1 is the head of the current edge.
    // h1 * hA is negative when a crossing (possibly including an extended sequence of ON segments) has happened.
    //    The RECORDED crossing is always within the segemnt that actually reached the other sign.
    for (size_t k = 1; k <= n; k++, h0 = h1)
        {
        size_t i1 = (iStart + k) % n;
        size_t i0 = i1 > 0 ? i1 - 1 : n - 1;
        h1 = plane.Evaluate (points[i1]);
        if (0 == DoubleOps::TolerancedSign (h1, touchTolerance))
            {
            CurveLocationDetail detail (NULL, 0.0, points[i1], i1, n, 0.0, 0.0);
            touchData->push_back (detail);
            }
        altitudeRange.Extend (h1);
        if (hA * h1 < 0.0)
            {
            // the most recent segment completes a true crossing.
            double edgeParameter = - h0 / (h1 - h0);
            DPoint3d edgePoint = DPoint3d::FromInterpolate (points[i0], edgeParameter, points[i1]);
            CurveLocationDetail detail (NULL, edgeParameter, edgePoint, i0, n, 0.0, 0.0);
            trueCrossings.push_back (detail);
            }

        if (h1 != 0.0)
            hA = h1;
        }
    return PlanePolygonSSICode::Transverse;          
    }    

bool PolygonOps::ReverseForPreferedNormal (bvector<DPoint3d> &xyz, DVec3dCR positiveDirection)
    {
    DVec3d normal = AreaNormal (xyz);
    if (positiveDirection.DotProduct (normal) < 0.0)
        {
        DPoint3dOps::Reverse (xyz);
        return true;
        }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
