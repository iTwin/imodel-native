/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_integrals.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-------------------------------------------------------------------*//**
<summary> Accumulate volume properties of a tetrahedron into sums. </summary>
+----------------------------------------------------------------------*/
static void accumulateTetrahedralVolumeProperties
(
DVec3d  &u1,
DVec3d  &u2,
DVec3d  &u3,
double  &volumeSum,
double &ixSum,
double &iySum,
double &izSum,
double  &ixxSum,
double  &iyySum,
double  &izzSum,
double  &xySum,
double  &xzSum,
double  &yzSum
)
    {
    double volume;
    double momentScale;
    double uuScale, uvScale;

    volume = u1.TripleProduct (u2, u3) / 6.0;
    volumeSum += volume;
    momentScale = volume * 0.25;

    ixSum += momentScale * (u1.x + u2.x + u3.x);
    iySum += momentScale * (u1.y + u2.y + u3.y);
    izSum += momentScale * (u1.z + u2.z + u3.z);

    uuScale = volume / 10.0;

    ixxSum += uuScale *
                    (u1.x * u1.x + u2.x * u2.x + u3.x * u3.x +
                    u1.x * u2.x + u1.x * u3.x + u2.x * u3.x);

    iyySum += uuScale *
                   (u1.y * u1.y + u2.y * u2.y + u3.y * u3.y +
                    u1.y * u2.y + u1.y * u3.y + u2.y * u3.y);

    izzSum += uuScale *
                   (u1.z * u1.z + u2.z * u2.z + u3.z * u3.z +
                    u1.z * u2.z + u1.z * u3.z + u2.z * u3.z);

    uvScale = volume / 20.0;

    xySum += uvScale * (2.0 * (u1.x * u1.y + u2.x * u2.y + u3.x * u3.y) +
                                        u1.x * u2.y + u2.x * u1.y +
                                        u1.x * u3.y + u3.x * u1.y +
                                        u2.x * u3.y + u3.x * u2.y);

    xzSum += uvScale * (2.0 * (u1.x * u1.z + u2.x * u2.z + u3.x * u3.z) +
                                        u1.x * u2.z + u2.x * u1.z +
                                        u1.x * u3.z + u3.x * u1.z +
                                        u2.x * u3.z + u3.x * u2.z);

    yzSum += uvScale * (2.0 * (u1.z * u1.y + u2.z * u2.y + u3.z * u3.y) +
                                        u1.z * u2.y + u2.z * u1.y +
                                        u1.z * u3.y + u3.z * u1.y +
                                        u2.z * u3.y + u3.z * u2.y);
    }

/*-------------------------------------------------------------------*//**
<summary> Accumulate volume properties of a tetrahedron into sums. </summary>
+----------------------------------------------------------------------*/
static void accumulateTetrahedralVolumeMoments
(
DMatrix4dR moments,
DPoint3dCR point0,   // base point of tetrahedron.  THIS IS THE ORIGIN FOR MOMENTS INTEGRALS
DPoint3dCR point1,
DPoint3dCR point2,
DPoint3dCR point3
)
    {
    DVec3d u1, u2, u3;

    u1.DifferenceOf (point1, point0);
    u2.DifferenceOf (point2, point0);
    u3.DifferenceOf (point3, point0);

    accumulateTetrahedralVolumeProperties (u1, u2, u3,
            moments.coff[3][3],
            moments.coff[0][3], moments.coff[1][3], moments.coff[2][3],
            moments.coff[0][0], moments.coff[1][1], moments.coff[2][2],
            moments.coff[0][1], moments.coff[0][2], moments.coff[1][2]);
    // enforce symmetry ...
    for (int i = 1; i < 4; i++)
        for (int j = 0; j < i; j++)
            moments.coff[i][j] = moments.coff[j][i];
    }




static void AddScaledOuterProduct (DMatrix4dR matrix, double *vectorA, double *vectorB, double scale)
    {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            matrix.coff[i][j] += vectorA[i] * vectorB[j] * scale;
    }



// Return product integrals I(0<=u<=1) I (0<=v<= u)  (w*W + u *U + v * V)(w*W + u *U + v * V)^  du dv
//  where w = 1-u-v
//  W = column vector (point00.x, point00.y, point00.z, 1.0) etc.
static DMatrix4d TriangleProducts (DPoint3dCR point00, DPoint3dCR point10, DPoint3dCR point01)
    {
    double vectors[3][4] = 
      {
      {point00.x, point00.y, point00.z, 1.0},
      {point10.x, point10.y, point10.z, 1.0},
      {point01.x, point01.y, point01.z, 1.0}
      };
    double r1_12 = 1.0 / 12.0;
    double r1_24 = 1.0 / 24;

    double coff[3][3] =
      {
      {r1_12, r1_24, r1_24},
      {r1_24, r1_12, r1_24},
      {r1_24, r1_24, r1_12}
      };
    DMatrix4d products = DMatrix4d::FromZero ();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
          AddScaledOuterProduct (products, vectors[i], vectors[j], coff[i][j]);
    return products;
    }

void bsiDMatrix4d_addScaledDMatrix4dInPlace (DMatrix4dR dest, DMatrix4dCR data, double scale)
    {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            {
            dest.coff[i][j] += data.coff[i][j] * scale;
            }
    }

static void bsiDMatrix4d_zero (DMatrix4dR data)
    {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            data.coff[i][j] = 0.0;
    }


// 7 volume integrals:
// 0,1,2 scale by scales.x,scales.y,scales.z.
// 3,4,5 scale by absolute values
// 6 scale by complete magnitude of scales.
// 3 directional
//
// directionalProducts[0] sum of tetrahedral moments from global origin.
// directionalProducts[1] sum of tetrahedral moments for 3 tetrahedra formed when the triangle is swept in x direction to x=origin.x.
// sim for [1] in y direction, 2 with z direction sweep.
// 
static void AccumulateScaledPrincipalProducts
(
DirectionalVolumeData sums[7],
DMatrix4d directionalProducts[4],
DPoint3dCR origin,
DPoint3dCR point0,
DPoint3dCR point1,
DPoint3dCR point2,
DVec3dCR scales
)
    {
    DVec3d vector0, vector1, vector2;
    vector0.DifferenceOf (point0, origin);
    vector1.DifferenceOf (point1, origin);
    vector2.DifferenceOf (point2, origin);

    DMatrix4d triangleProducts = TriangleProducts (vector0, vector1, vector2);
    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[0].areaProducts, triangleProducts, scales.x);
    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[1].areaProducts, triangleProducts, scales.y);
    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[2].areaProducts, triangleProducts, scales.z);

    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[3].areaProducts, triangleProducts, fabs (scales.x));
    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[4].areaProducts, triangleProducts, fabs (scales.y));
    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[5].areaProducts, triangleProducts, fabs (scales.z));

    bsiDMatrix4d_addScaledDMatrix4dInPlace (sums[6].areaProducts, triangleProducts,
        scales.Magnitude ());
    DMatrix4d matrixB;
    DMatrix4d matrixA, matrixAT, matrixABAT;
    DPoint3d sweep0, sweep1, sweep2;
    for (int i = 0; i < 3; i++)
        {
        sweep0 = point0;
        sweep1 = point1;
        sweep2 = point2;
        if (i == 0)
            sweep0.x = sweep1.x = sweep2.x = origin.x;
        else if (i == 1)
            sweep0.y = sweep1.y = sweep2.y = origin.y;
        else
            sweep0.z = sweep1.z = sweep2.z = origin.z;
        bsiDMatrix4d_zero (matrixB);
        // These moments have the tetraheral moments relative to sweep0 ...
        accumulateTetrahedralVolumeMoments (matrixB, sweep0, point0, point1, point2);
        accumulateTetrahedralVolumeMoments (matrixB, sweep0, sweep2, point2, point1);
        accumulateTetrahedralVolumeMoments (matrixB, sweep0, sweep1, sweep2, point1);
        // Shift to global origin ...
        DVec3d translationVector;
        translationVector.DifferenceOf (sweep0, origin);
        matrixA.InitFromTranslation (translationVector.x, translationVector.y, translationVector.z);
        matrixAT.TransposeOf (matrixA);
        matrixABAT.InitProduct (matrixA, matrixB, matrixAT);
        bsiDMatrix4d_addScaledDMatrix4dInPlace (directionalProducts[i], matrixABAT, 1.0);
        }
    }



struct DirectionalVolumeDataSet
{
DPoint3d origin;
DirectionalVolumeData data [7];
DMatrix4d directionalProducts [3];
void Zero (DPoint3dCR _origin)
    {
    origin = _origin;
    for (int k = 0; k < 7; k++)
        {
        data[k].volume = 0.0;
        bsiDMatrix4d_zero (data[k].areaProducts);
        }
    for (int k = 0; k < 3; k++)
        bsiDMatrix4d_zero (directionalProducts[k]);
    }

void AddTriangle (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
    {
    DVec3d vector01, vector02, cross;
    DPoint3d triangleCentroid;
    vector01.DifferenceOf (point1, point0);
    vector02.DifferenceOf (point2, point0);
    static double oneThird = 1.0 / 3.0;

    cross.CrossProduct (vector01, vector02);
    //cross.Scale (0.5);
    triangleCentroid = DPoint3d::FromSumOf (point0, vector01, oneThird, vector02, oneThird);
    // full cross product is used as jacobian in area moment accumulations.
    AccumulateScaledPrincipalProducts (data, directionalProducts, origin, point0, point1, point2, cross);
    // but it is scaled for voume frusta
    data[0].volume += 0.5 * cross.x * (triangleCentroid.x - origin.x);
    data[1].volume += 0.5 * cross.y * (triangleCentroid.y - origin.y);
    data[2].volume += 0.5 * cross.z * (triangleCentroid.z - origin.z);            

    }
};

Public bool jmdlMTGFacets_directionalVolumeWithProducts
(
const MTGFacets*      pFacetHeader,
DirectionalVolumeData pData[7],
DMatrix4d             directionalProducts[3],
DPoint3dP             pOrigin
)
    {
    bool boolstat = false;
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    bvector<int>faceStart;
    const_cast<MTGGraph*>(pGraph)->CollectFaceLoops (faceStart);

    MTGNodeId node0Id, node1Id, node2Id;
    DPoint3d point0, point1, point2;
    DRange3d range;
    DirectionalVolumeDataSet allData;
    jmdlMTGFacets_getRange (pFacetHeader, &range);
    allData.Zero (range.low);

    // um.. need to filter out exterior loops !!!!
    for (size_t i = 0; i < faceStart.size (); i++)
        {
        node0Id = faceStart[i];
        if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader,
                                        &point0, node0Id))
            goto wrapup;

        if (jmdlMTGGraph_getMask (pGraph, node0Id, MTG_EXTERIOR_MASK))
            continue;
        node2Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
        if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader, &point2, node2Id))
                goto wrapup;

        for (;;)
            {
            node1Id = node2Id;
            node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
            if (node2Id == node0Id)
                break;

            point1  = point2;
            if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader, &point2, node2Id))
                goto wrapup;
            allData.AddTriangle (point0, point1, point2);            
            }
        }
    boolstat = TRUE;
wrapup:

    pData[0] = allData.data[0];
    pData[1] = allData.data[1];
    pData[2] = allData.data[2];
    pData[3] = allData.data[3];
    pData[4] = allData.data[4];
    pData[5] = allData.data[5];
    pData[6] = allData.data[6];
    directionalProducts[0] = allData.directionalProducts[0];
    directionalProducts[1] = allData.directionalProducts[1];
    directionalProducts[2] = allData.directionalProducts[2];
    if (NULL != pOrigin)
        *pOrigin = allData.origin;

    return boolstat;
    }


void PolyfaceQuery::DirectionalVolumeIntegrals
(
PolyfaceQueryCR       polyface,
DirectionalVolumeData pData[7],
DMatrix4d             directionalProducts[3],
DPoint3dR             origin
) const
    {
    DRange3d range = polyface.PointRange ();
    DirectionalVolumeDataSet allData;
    allData.Zero (range.low);
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (polyface, false);
    bvector<DPoint3d> &points = visitor ->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (size_t i = 1; i + 1 < points.size (); i++)
            {
            allData.AddTriangle (points[0], points[i], points[i+1]);            
            }
        }

    pData[0] = allData.data[0];
    pData[1] = allData.data[1];
    pData[2] = allData.data[2];
    pData[3] = allData.data[3];
    pData[4] = allData.data[4];
    pData[5] = allData.data[5];
    pData[6] = allData.data[6];
    directionalProducts[0] = allData.directionalProducts[0];
    directionalProducts[1] = allData.directionalProducts[1];
    directionalProducts[2] = allData.directionalProducts[2];
    origin = allData.origin;
    }


static void ExtractDirectionalSummaryData 
(
DirectionalVolumeData integrals[7],
DVec3dR directionalVolumes,
DVec3dR signedDirectionalAreas,
DVec3dR absoluteDirectionalAreas,
double &absoluteArea,
DVec3dR missingFacetFactors
)
    {
    DVec3d axx;
    DVec3d q;
    double ratios[3];
    for (int k = 0; k < 3; k++)
        {
        int k0 = (k+1) % 3;
        int k1 = (k+2) % 3;
        int kAbs = 3 + k;
        // pluck out the 2nd moments from the other two planes.  axx is the properly signed integral (and should vanish), q uses absolute differentials (and should not)
        // the ration is 
        axx.Init ( integrals[k].areaProducts.coff[k0][k0], integrals[k].areaProducts.coff[k0][k1], integrals[k].areaProducts.coff[k1][k1]);
        q.Init (   integrals[kAbs].areaProducts.coff[k0][k0], integrals[kAbs].areaProducts.coff[k0][k1], integrals[kAbs].areaProducts.coff[k1][k1]);
        DoubleOps::SafeDivide (ratios[k], axx.Magnitude (), q.Magnitude (), 1.0);
        }
    directionalVolumes.Init (integrals[0].volume, integrals[1].volume, integrals[2].volume);
    signedDirectionalAreas.Init (integrals[0].areaProducts.coff[3][3], integrals[1].areaProducts.coff[3][3], integrals[2].areaProducts.coff[3][3]);
    absoluteDirectionalAreas.Init (integrals[3].areaProducts.coff[3][3], integrals[4].areaProducts.coff[3][3], integrals[5].areaProducts.coff[3][3]);
    absoluteArea = integrals[6].areaProducts.coff[3][3];
    missingFacetFactors.Init (ratios[0], ratios[1], ratios[2]);
    }
//! Return the volume, centroid, orientation, and principal moments, using an algorithm that tolerates missing
//!  "side"facets.
bool PolyfaceQuery::ComputePrincipalMomentsAllowMissingSideFacets
(
double &volume,         //!< [out] mesh volume.
DPoint3dR centroid,     //!< [out] centroid of the mesh.
RotMatrixR axes,        //!< [out] columns of this matrix are the principal directions.
DVec3dR momentxyz,       //!< [out] moments (yy+zz,xx+zz,xx+yy) around the principal directions.
bool forcePositiveVolume , //!< [in] if true, the volume and moments are reversed if volume is negative.
double relativeTolerance  //!< [in] relative tolerance for assessing the checksums.  (Suggested value: 1e-8 or tighter)
) const
    {
    DirectionalVolumeData data[7];
    DMatrix4d products[3];
    DPoint3d origin;
    DVec3d centroid0;
    DirectionalVolumeIntegrals (*this, data, products, origin);
    //double area0 = data[2].areaProducts.coff[3][3];    // SIGNED area integral in z view -- should be zero
    //double area1 = data[5].areaProducts.coff[3][3];    // absolute area integral in z view -- reference number for tolerancing.
    DRange3d range = PointRange ();
    DVec3d diagonal = DVec3d::FromStartEnd (range.low, range.high);
    DVec3d rangeArea = DVec3d::From (diagonal.y * diagonal.z, diagonal.x * diagonal.z, diagonal.x * diagonal.y);

    DVec3d directionalVolumes, signedDirectionalAreas, absoluteDirectionalAreas, missingFacetFactors;
    double absoluteArea;
    ExtractDirectionalSummaryData (data, directionalVolumes, signedDirectionalAreas, absoluteDirectionalAreas, absoluteArea, missingFacetFactors);
    static double s_rangeAreaFactor = 0.05;
    if (products[2].ConvertInertiaProductsToPrincipalMoments (volume, centroid0, axes, momentxyz))
        {
        centroid = DPoint3d::FromSumOf (origin, centroid0);
        return 
               absoluteDirectionalAreas.z > s_rangeAreaFactor * rangeArea.z       // reject if projection looks skinny.
            && fabs (signedDirectionalAreas.z) < relativeTolerance * absoluteDirectionalAreas.z
            && missingFacetFactors.z < relativeTolerance;
        }
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
