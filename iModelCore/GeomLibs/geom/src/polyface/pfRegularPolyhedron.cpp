/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description Multiply xyz array by scale factor (from origin) and transform.  Add to output array.
* @param points      OUT output array
* @param pXYZ       IN source points.
* @param numXYZ     IN number of points
* @param pTransform     IN  coordinate frame.
* @param prescale         IN local scal factor.
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void transformAndAddCoordinates
(
bvector<DPoint3d> &points,
DPoint3dCP pXYZ,
int        numXYZ,
TransformCP pTransform,
double      prescale
)
    {
    Transform transform;
    int i;
    if (pTransform)
        transform = *pTransform;
    else
        transform.InitIdentity ();

    transform.ScaleMatrixColumns (transform, prescale, prescale, prescale);
    for (i = 0; i < numXYZ; i++)
        {
        DPoint3d xyzi;
        transform.Multiply (xyzi, pXYZ[i]);
        points.push_back (xyzi);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Shift indices from zerobased with negative terminator to one-based with zero terminator.
* @param pointIndex      OUT output array
* @param pIndices       IN source indices
* @param n           IN number of indices
* @param baseIndex  IN input index zero shifts to this plus 1.  I.e. number of vertices BEFORE adding
            current batch.
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addShiftedZeroBasedIndices
(
    bvector<int> &pointIndex,
    int         *pIndices,
    int        n,
    int         baseIndex
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        int k = pIndices[i];
        if (k < 0)
            pointIndex.push_back (0);
        else
            pointIndex.push_back (k + baseIndex + 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description fill index arrays with a dodecahedron
*
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN local radius (to vertices)
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addDodecahedron
(
PolyfaceHeaderPtr &polyface,
    TransformCP             pTransform,
    double                  radius
)
    {
    bvector<DPoint3d> &points = polyface->Point();
    bvector<int> &pointIndex = polyface->PointIndex();
    double refSize = 1.0 / sqrt(3.0);

    double c = refSize;
    double b = refSize * (1.0 + sqrt(5.0)) / 2.0;
    double a = refSize * refSize / b;
    int sNumXYZ = 20;
    DPoint3d xyz[20] =
        {
        {-c,-c,-c},
        {-c,-c, c},
        {-c, c,-c},
        {-c, c, c},
        { c,-c,-c},
        { c,-c, c},
        { c, c,-c},
        { c, c, c},
        {-a,-b, 0},
        { a,-b, 0},
        {-a, b, 0},
        { a, b, 0},
        { 0,-a,-b},
        { 0,-a, b},
        { 0, a,-b},
        { 0, a, b},
        {-b, 0,-a},
        {-b, 0, a},
        { b, 0,-a},
        { b, 0, a},
        };
    // Each pentagon has an internal edge on the unit square
    static int sNumIndex = 72;
    static int sUseDebugIndices = 0;
    static int sUsePrimaryIndices = 1;
    int index[72] =
        {
         0, 8, 1,17,16,-1,
         0,16, 2,14,12,-1,
         6,14, 2,10,11,-1,
         3,10, 2,16,17,-1,
         6,11, 7,19,18,-1,
         6,18, 4,12,14,-1,
         5, 9, 4,18,19,-1,
         0,12, 4, 9, 8,-1,
         3,17, 1,13,15,-1,
         3,15, 7,11,10,-1,
         5,19, 7,15,13,-1,
         5,13, 1, 8, 9,-1,
        };

    static int sNumDebugIndex = 25;
    // These indices expose rectangles among the mysterious coordinates.
    int debugIndex[25] =
        {
        0,1,3,2,    -1,
        4,6,7,5,    -1,
        8,9,11,10,  -1,
        18,19,17,16,-1,
        12,13,15,14,-1
        };

    int baseIndex = (int)points.size ();
    transformAndAddCoordinates(points, xyz, sNumXYZ, pTransform, radius);
    if (sUsePrimaryIndices)
        addShiftedZeroBasedIndices(pointIndex, index, sNumIndex, baseIndex);
    if (sUseDebugIndices)
        addShiftedZeroBasedIndices(pointIndex, debugIndex, sNumDebugIndex, baseIndex);
    }
// input the mesh constructed with arbitrary radius.
// apply scale factors so that the first (vertex, edge midpoint, face centroid) is at given radius.
// Rotate so keypoint used for scaling is along the x axis, and the first edge in its facet is parallel to y.
// Return false if empty mesh
// EXCEPT -- if doRotation is false, the scaling is applied without rotating.
static bool ScaleAndOrientForRadiusAtKeyPoint(PolyfaceHeaderR mesh, DPoint3dCR origin, double radius, int radiusSelect, bool doRotation)
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (mesh);
    visitor->Reset ();
    DVec3d edgeVector, keyPointVector;
    bvector<DPoint3d> &points = visitor->Point ();
    int secondaryAxisIndex = 2;
    if (visitor->AdvanceToNextFace() && visitor->Point ().size () > 2)
        {
        edgeVector = points[1] - points[0];
        if (radiusSelect == 1)
            keyPointVector = DPoint3d::FromInterpolate (points[0], 0.5, points[1]) - origin;
        else if (radiusSelect == 2)
            {
            DPoint3d centroid;
            DVec3d normal;
            double area;
            PolygonOps::CentroidNormalAndArea (points, centroid, normal, area);
            keyPointVector = centroid - origin;
            secondaryAxisIndex = 1;
            }
        else
            {
            int n = visitor->NumEdgesThisFace ();
            int k = (n-1)/2;    // lower vertex index for reference edge "half way around the facet"
            edgeVector = points[k+1]-points[k];
            keyPointVector = points[0] - origin;
            }
        auto scale = DoubleOps::ValidatedDivide (radius, keyPointVector.Magnitude ());
        if (!scale.IsValid ())
            return false;
        if (!doRotation)
            {
            auto scaleMatrix = RotMatrix::FromScale (scale.Value ());
            auto transform = Transform::FromMatrixAndFixedPoint(scaleMatrix, origin);
            transform.Multiply(mesh.Point());
            return true;
            }
        else
            {
            RotMatrix axes = RotMatrix::FromColumnVectors(keyPointVector, edgeVector, edgeVector);
            if (axes.SquareAndNormalizeColumns (axes, 0, secondaryAxisIndex))
                {
                // it's an orthogonal matrix, the transpose is the inverse ...
                RotMatrix transpose = RotMatrix::FromTransposeOf (axes);
                double s = scale.Value ();
                transpose.ScaleRows (transpose, s, s, s);
                auto transform = Transform::FromMatrixAndFixedPoint (transpose, origin);
                transform.Multiply (mesh.Point());
                return true;
                }
            }
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @description fill index arrays with an icosohedron
*
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN local radius (to vertices)
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addIcosahedron
(
    PolyfaceHeaderPtr &polyface,
    TransformCP             pTransform,
    double                  radius
)
    {
    bvector<DPoint3d> &points = polyface->Point();
    bvector<int> &pointIndex = polyface->PointIndex();
    double a = (1.0 + sqrt(5.0)) / 2.0;
    double b = 1.0;
    static int sNumXYZ = 12;
    DPoint3d xyz[12] =
        {
        { a,-b, 0},
        { a, b, 0},
        {-a, b, 0},
        {-a,-b, 0},

        { 0, a,-b},
        { 0, a, b},
        { 0,-a, b},
        { 0,-a,-b},

        {-b,0, a},
        { b,0, a},
        { b,0,-a},
        {-b,0,-a},
        };

    static int sNumIndex = 80;
    int index[80] =
        {
         1, 5, 9, -1,
        10, 4, 1, -1,
         2, 8, 5, -1,
        11, 2, 4, -1,
         6, 8, 3, -1,
         3,11, 7, -1,
         0, 9, 6, -1,
         7,10, 0, -1,

         0, 1, 9, -1,
         1, 0,10, -1,

         2, 3, 8, -1,
         3, 2,11, -1,

         8, 9, 5, -1,
         9, 8, 6, -1,

        10,11, 4, -1,
        11,10, 7, -1,

         4, 5, 1, -1,
         5, 4, 2, -1,

         7, 6, 3, -1,
         6, 7, 0, -1,
        };

    int baseIndex = (int)points.size ();
    transformAndAddCoordinates(points, xyz, sNumXYZ, pTransform, radius);
    //if (sUsePrimaryIndices)
    addShiftedZeroBasedIndices(pointIndex, index, sNumIndex, baseIndex);
    //    if (sUseDebugIndices)
    //      addShiftedZeroBasedIndices (pointIndex, debugIndex, sNumDebugIndex, baseIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description fill index arrays with an icosohedron
*
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN local radius (to vertices)
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addCube
(
    PolyfaceHeaderPtr &polyface,
    TransformCP             pTransform,
    double                  radius
)
    {
    bvector<DPoint3d> &points = polyface->Point();
    bvector<int> &pointIndex = polyface->PointIndex();
    static int sNumXYZ = 8;
    DPoint3d xyz[8] =
        {
        {-1,-1,-1},
        {-1,-1, 1},
        {-1, 1,-1},
        {-1, 1, 1},
        { 1,-1,-1},
        { 1,-1, 1},
        { 1, 1,-1},
        { 1, 1, 1},
        };

    static int sNumIndex = 30;
    int index[30] =
        {
        6,7,5,4,-1,
        0,1,3,2,-1,
        2,3,7,6,-1,
        1,5,7,3,-1,
        4,5,1,0,-1,
        0,2,6,4,-1,

        };

    int baseIndex = (int)points.size ();
    transformAndAddCoordinates(points, xyz, sNumXYZ, pTransform, radius);
    //if (sUsePrimaryIndices)
    addShiftedZeroBasedIndices(pointIndex, index, sNumIndex, baseIndex);
    //    if (sUseDebugIndices)
    //      addShiftedZeroBasedIndices (pointIndex, debugIndex, sNumDebugIndex, baseIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description fill index arrays with a tetrahedron
*
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN local radius (to vertices)
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addTetrahedron
(
    PolyfaceHeaderPtr &polyface,
    TransformCP             pTransform,
    double                  radius
)
    {
    bvector<DPoint3d> &points = polyface->Point();
    bvector<int> &pointIndex = polyface->PointIndex();
    double a = sqrt(0.5);
    double b = 1.0;
    static int sNumXYZ = 4;
    DPoint3d xyz[4] =
        {
        {-a,0,-b},
        {-a,0, b},
        { a,-b,0},
        { a, b,0}
        };

    static int sNumIndex = 16;
    int index[16] =
        {
        0,1,3,-1,
        1,2,3,-1,
        2,1,0,-1,
        0,3,2,-1,
        };


    int baseIndex = (int)points.size ();
    transformAndAddCoordinates(points, xyz, sNumXYZ, pTransform, radius);
    //if (sUsePrimaryIndices)
    addShiftedZeroBasedIndices(pointIndex, index, sNumIndex, baseIndex);
    //    if (sUseDebugIndices)
    //      addShiftedZeroBasedIndices (pointIndex, debugIndex, sNumDebugIndex, baseIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description fill index arrays with an octahedron
*
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN local radius (to vertices)
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addOctahedron
(
    PolyfaceHeaderPtr &polyface,
    TransformCP             pTransform,
    double                  radius
)
    {
    bvector<DPoint3d> &points = polyface->Point();
    bvector<int> &pointIndex = polyface->PointIndex();
    double a = 1.0;
    double b = sqrt(0.5);
    static int sNumXYZ = 6;
    DPoint3d xyz[6] =
        {
        { 0, 0,-a},
        { 0, 0, a},
        { -b,-b, 0},
        { b, b, 0},
        {b, -b, 0},
        { -b, b, 0},
        };

    static int sNumIndex = 32;
    int index[32] =
        {
        1,3,4,-1,
        1,5,3,-1,
        1,2,5,-1,
        1,4,2,-1,
        0,4,3,-1,
        0,3,5,-1,
        0,5,2,-1,
        0,2,4,-1
        };

    int baseIndex = (int)points.size ();
    transformAndAddCoordinates(points, xyz, sNumXYZ, pTransform, radius);
    //if (sUsePrimaryIndices)
    addShiftedZeroBasedIndices(pointIndex, index, sNumIndex, baseIndex);
    //    if (sUseDebugIndices)
    //      addShiftedZeroBasedIndices (pointIndex, debugIndex, sNumDebugIndex, baseIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description create a mesh index arrays for a regular polyhedron.
* @param ppMeshED           OUT     pointer to new descriptor tree.
* @param pTemplate          IN      template element
* @param points      OUT coordinate array.
* @param pointIndex    OUT index array.
* @param pTransform     IN  coordinate frame.
* @param radius         IN radius, before applying pTransform.
* @param polyhedronSelect IN mesh selector:
            (0,Tetrahedron)
            (1,slab)
            (2,Octahedron)
            (3,Dodecahedron)
            (4,Icosahedron)
* @param radiusSelect IN 0 for radius to vertices, 1 for radius to midedge, 2 for radius to midface.
* @return true
* @group    "Mesh Elements"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateRegularPolyhedron
(
double                  radius,
int                     polyhedronSelect,
int                     radiusSelect,
TransformCP             transform
)
    {
    bool doRotation = true;
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();

    if (polyhedronSelect == 0)
        addTetrahedron(polyface,nullptr, radius);
    else if (polyhedronSelect == 1)
        {
        doRotation = radiusSelect < 100;
        addCube(polyface, nullptr, radius);
        }
    else if (polyhedronSelect == 2)
        {
        doRotation = radiusSelect < 100;
        addOctahedron(polyface, nullptr, radius);
        }
    else if (polyhedronSelect == 3)
        addDodecahedron(polyface, nullptr, radius);
    else if (polyhedronSelect == 4)
        addIcosahedron(polyface, nullptr, radius);
    if (polyface.IsValid ())
        {
        ScaleAndOrientForRadiusAtKeyPoint(
                *polyface,
                DPoint3d::From (0,0,0),
                radius, radiusSelect, 
                doRotation);
        if (transform)
            polyface->Transform (*transform);
        }
    return polyface;
    }
static void AddTriangle(PolyfaceHeaderR mesh, bvector<DPoint3d> &work, DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
    {
    work.clear ();
    work.push_back (point0);
    work.push_back (point1);
    work.push_back (point2);
    mesh.AddPolygon (work);
    }
/*---------------------------------------------------------------------------------**//**
* @description create mesh of nearly identical triangles on the sphere.
* <ul>
* <li> For tetrahedron, Octahedron, and Icosahedron, the initial polyhedron is already triangulated.
* <li> For slab and Icosahedron, the initial polyhedron is not triangulated.
*   <ul>
*   <li> A vertex is inserted in the center of each untriangulated facet.
*   <li> this vertex is the moved radially to the sphere.
*   </ul>
* <li> In the initial triangles, each edge is subdivided with numExtraVerticesInBaseEdges new vertices
* <li> Each triangle is then filled with a mesh of (numExtraVerticesInBaseEdges-1)*(numExtraVerticesInBaseEdges-1) triangles.
* <li> Note that a max of 5 is applied to numExtraVerticesInBaseEdges.
* </ul>
* @param [in] radius radius of sphere
* @param [in] polyhedronSelect integer selector, as enumerated above.
* @param [in] transform optional transform to place the sphere.
* @param numExtraVerticesInBaseEdges Must be 5 or less.   On each edge of the initial triangle,
*           this number of vertices are inserted to each edge, and additional points within
*           the triangles.
* @param [in] transform optional transform to place the sphere.
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation
(
    double                  radius,
    int                     polyhedronSelect,
    uint32_t                numExtraVerticesInBaseEdges,
    TransformCP             transform
)
    {
    uint32_t numTriangulationEdges = numExtraVerticesInBaseEdges + 1;
    if (numTriangulationEdges > 6)
        numTriangulationEdges = 6;
    auto polyfaceA = CreateRegularPolyhedron (radius, polyhedronSelect, 100, nullptr);
    bvector<DPoint3d> workTriangle;
    bvector<DPoint3d> work1;
    bvector<DPoint3d> work2;
    // Cube and icosohedron -- insert mid-face vertex (projected out to sphere) for first triangulation.
    if (polyhedronSelect == 1 || polyhedronSelect == 3)
        {
        auto polyfaceB = PolyfaceHeader::CreateVariableSizeIndexed();
        auto visitorA = PolyfaceVisitor::Attach (*polyfaceA);
        bvector<DPoint3d> &points = visitorA->Point ();
        visitorA->SetNumWrap (1);
        for (visitorA->Reset(); visitorA->AdvanceToNextFace();)
            {
            // centroid is simple average
            DVec3d sum = DVec3d::From (0,0,0);
            size_t n = visitorA->NumEdgesThisFace();
            for (size_t i = 0; i < n; i++)
                {
                DVec3d xyzi = DVec3d::From (points[i]);
                sum = DVec3d::FromSumOf (sum, xyzi);
                }
            // simultaneously average back to centroid and out to sphere.
            sum.ScaleToLength (radius);
            // Triangle to each edge ..
            for (size_t i = 0; i < n; i++)
                {
                AddTriangle(*polyfaceB, workTriangle, sum, points[i], points[i+1]);
                }
            }
        polyfaceA = polyfaceB;
        }
    if (numTriangulationEdges > 1)
        {
        auto polyfaceC = PolyfaceHeader::CreateVariableSizeIndexed();
        auto visitorA = PolyfaceVisitor::Attach(*polyfaceA);        // this visitor does not need wrap !
        bvector<DPoint3d> &points = visitorA->Point();
        if (numTriangulationEdges > 6)
            numTriangulationEdges = 6;
        for (visitorA->Reset(); visitorA->AdvanceToNextFace();)
            {
            DPoint3d point0 = points[0];
            DPoint3d point1 = points[1];
            DPoint3d point2 = points[2];
            work1.clear ();
            work2.clear ();
            for (size_t row = 0; row <= numTriangulationEdges; row++)
                {
                double v = (double)row / (double)numTriangulationEdges;
                DPoint3d pointA = DPoint3d::FromInterpolate (point0, v, point2);
                DPoint3d pointB = DPoint3d::FromInterpolate (point1, v, point2);
                pointA.ScaleToLength (radius);
                pointB.ScaleToLength(radius);
                work2.clear ();
                size_t kMax = numTriangulationEdges - row;
                work2.push_back (pointA);
                for (size_t k = 1; k <= kMax; k++)
                    {
                    work2.push_back(DPoint3d::FromInterpolate (pointA, (double)k / (double) kMax, pointB));
                    work2.back ().ScaleToLength (radius);
                    }
                if (row > 0)
                    {
                    for (size_t k = 0; k <= kMax; k++)
                        {
                        AddTriangle (*polyfaceC, workTriangle, work1[k], work1[k+1], work2[k]);
                        if (k+1 <= kMax)
                            AddTriangle (*polyfaceC, workTriangle, work1[k+1], work2[k], work2[k+1]);
                        }
                    }
                work1.swap (work2);
                }
            }
        polyfaceA = polyfaceC;
        }
    polyfaceA->Compress();
    if (transform)
        polyfaceA->Transform (*transform);
    return polyfaceA;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

