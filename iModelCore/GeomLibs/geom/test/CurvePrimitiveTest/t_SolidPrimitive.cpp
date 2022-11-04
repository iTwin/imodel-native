/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include "Mtg/MtgApi.h"
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

#define CheckTryGet(__PrimType__) \
    {\
    __PrimType__##Detail data;\
    Check::Bool (primitiveType == SolidPrimitiveType_##__PrimType__,\
                primitive->TryGet##__PrimType__##Detail (data));\
    }

struct IntTriple {int i0, i1, i2;
IntTriple (int j0, int j1, int j2)
    {
    i0 = j0;
    i1 = j1;
    i2 = j2;
    }
};

void SaveEdgeChains (PolyfaceHeaderR facets, bool showNoChainX)
    {
    BlockedVector<PolyfaceEdgeChain>&edgeChains = facets.EdgeChain ();
    bvector<DPoint3d> &point = facets.Point ();
    if (edgeChains.empty ())
        {
        if (showNoChainX)
            {
            DRange3d range = facets.PointRange ();
            bvector<DPoint3d> xyz;
            DPoint3d corners[8];
            range.Get8Corners (corners);
            xyz.push_back (corners[0]);
            xyz.push_back (corners[3]);
            xyz.push_back (range.LocalToGlobal (0.5, 0.5, 0.0));
            xyz.push_back (corners[2]);
            xyz.push_back (corners[1]);
            Check::SaveTransformed (xyz);
            }
        }
    else
        {
        bvector<DPoint3d> xyz;
        for (auto &chain : edgeChains)
            {
            chain.GetXYZ (xyz, point);
            size_t n = xyz.size (); // clumsy way to expose n to debugger.
            if (n > 0)
                {
                Check::SaveTransformed (xyz);
                xyz.clear ();
                }
            }
        }
    }


void CheckPick (ISolidPrimitivePtr primitive, DRay3dCR ray, size_t expectedHits = -1, bvector<IntTriple> * expectedIds = NULL)
    {
    bvector<SolidLocationDetail> pickData;
    primitive->AddRayIntersections (pickData, ray, 0, -DBL_MAX);
    for (size_t i = 0; i < pickData.size (); i++)
        {
        SolidLocationDetail::FaceIndices indices0 = pickData[i].GetFaceIndices ();
        DPoint2d uv0 = pickData[i].GetUV ();
        DPoint3d xyz0 = pickData[i].GetXYZ ();
        DPoint3d xyz1;
        DVec3d dXdu1, dXdv1;
        bool ok = primitive->TryUVFractionToXYZ (indices0, uv0.x, uv0.y, xyz1, dXdu1, dXdv1);
        Check::True (ok, "Reevaluate pick point");
        if (ok)
            Check::Near (xyz0, xyz1, "Revaluate pick point xyz");
        ok = true;  /// debugger spot.
        }
    if (expectedHits > 0 )
        {
        int hits = (int)pickData.size ();
        Check::Size (expectedHits, hits);
        }
    if (expectedIds != NULL)
        {
        for (size_t i = 0; i < expectedIds->size (); i++)
            {
            int id0 = expectedIds->at(i).i0;
            int id1 = expectedIds->at(i).i1;
            int n   = expectedIds->at(i).i2;
            int numFound = 0;
            for (size_t j = 0; j < pickData.size (); j++)
                {
                if (pickData[j].GetPrimarySelector () == id0
                    && pickData[j].GetSecondarySelector () == id1)
                    numFound++;
                }
            Check::Int (n, numFound);
            }
        }
    }

void CheckConsistency (ISolidPrimitiveCR solid, bvector<SolidLocationDetail> solidDetails)
    {
    Check::StartScope ("SolidDetail consistency");
    for (SolidLocationDetail detail : solidDetails)
        {
        auto uv = detail.GetUV ();
        auto indices = detail.GetFaceIndices ();
        DPoint3d xyz1;
        DVec3d dXdu1, dXdv1;
        if (Check::True (solid.TryUVFractionToXYZ (indices, uv.x, uv.y, xyz1, dXdu1, dXdv1)))
            {
            Check::Near (detail.GetXYZ (), xyz1);
            }
        }
    Check::EndScope ();
    }
void CheckConsistency (CurveLocationDetailCR curveDetail, SolidLocationDetailCR solidDetail)
    {
    Check::StartScope ("CurveDetail consistency");
    DPoint3d xyz0 = solidDetail.GetXYZ ();
    DPoint3d xyz1 = curveDetail.point;
    Check::Near (xyz0, xyz1, "Same coordinates in curve, solid details");
    DPoint3d xyz2;
    curveDetail.curve->FractionToPoint (curveDetail.fraction, xyz2);
    Check::Near (xyz1, xyz2, "Curve Location Detail point matches evaluation");
    Check::EndScope ();
    }

void CheckConsistency (bvector<CurveLocationDetail> &curveDetail, bvector<SolidLocationDetail> &solidDetail)
    {
    if (Check::Size (curveDetail.size (), solidDetail.size ()))
    for (size_t i = 0; i < curveDetail.size (); i++)
        CheckConsistency (curveDetail[i], solidDetail[i]);
    }



void CheckPick (ISolidPrimitivePtr primitive, DSegment3dCP segment, int expectedHits)
    {
    if (NULL != segment)
        {
        DRay3d ray = DRay3d::FromOriginAndTarget (segment->point[0], segment->point[1]);
        CheckPick (primitive, ray, expectedHits, NULL);
        }
    }
void CheckCone (ISolidPrimitivePtr primitive)
    {
    DgnConeDetail cone;
    if (primitive->TryGetDgnConeDetail (cone))
        {
        Transform localToWorld, worldToLocal;
        double radiusA, radiusB;
        if (cone.GetTransforms (localToWorld, worldToLocal, radiusA, radiusB))
            {
            DPoint3d pointA, pointB;
            double d = radiusA + radiusB + 1.0;
            localToWorld.Multiply (pointA, d, 0.0, 0.5);
            localToWorld.Multiply (pointB, -d, 0.0, 0.5);
            DRay3d ray = DRay3d::FromOriginAndTarget (pointA, pointB);
            bvector<IntTriple> expectedHits;
            expectedHits.push_back (IntTriple (0,0,2));
            CheckPick (primitive, ray, 2, &expectedHits);

            localToWorld.Multiply (pointA, 0.0, 0.0, 2.0);
            localToWorld.Multiply (pointB, 0.0, 0.0, -0.5);

            primitive->SetCapped (false);
            CheckPick (primitive, ray, 0, NULL);
            expectedHits.clear ();
            ray = DRay3d::FromOriginAndTarget (pointA, pointB);
            primitive->SetCapped (true);
            DEllipse3d ellipse0, ellipse1;
            if (cone.FractionToSection (0.0, ellipse0)
                && cone.FractionToSection (1.0, ellipse1)
                )
                {
                expectedHits.push_back (IntTriple(SolidLocationDetail::PrimaryIdCap, 0, 1));
                expectedHits.push_back (IntTriple(SolidLocationDetail::PrimaryIdCap, 1, 1));
                CheckPick (primitive, ray, 2, &expectedHits);
                }
            }
        }

    if (primitive->GetCapped ())
        {
        DgnConeDetail detail;
        primitive->TryGetDgnConeDetail (detail);
        if (detail.IsRealCap (0) && detail.IsRealCap (1))
            {
            SolidLocationDetail::FaceIndices cap0Indices = SolidLocationDetail::FaceIndices::Cap0 ();
            SolidLocationDetail::FaceIndices cap1Indices = SolidLocationDetail::FaceIndices::Cap1 ();
            IGeometryPtr g0 = primitive->GetFace (cap0Indices);
            IGeometryPtr g1 = primitive->GetFace (cap1Indices);
            CurveVectorPtr cv0 = g0->GetAsCurveVector ();
            CurveVectorPtr cv1 = g1->GetAsCurveVector ();
            if (Check::True (cv0.IsValid ()) && Check::True (cv1.IsValid ()))
                {
                DPoint3d centroid0, centroid1;
                DVec3d normal0, normal1;
                double area0, area1;
                Check::True (cv0->CentroidNormalArea (centroid0, normal0, area0));
                Check::True (cv1->CentroidNormalArea (centroid1, normal1, area1));
                Check::True (normal0.DotProduct (normal1) < 0.0, "Opposing normals on cone");
                }
            }
        }
    }

void CheckSphere (ISolidPrimitivePtr primitive)
    {
    DgnSphereDetail sphere;
    if (primitive->TryGetDgnSphereDetail (sphere))
        {
        DRange3d range;
        Check::True (primitive->GetRange (range));
        double lat0, lat1, z0, z1;
        bool isClipped = sphere.GetSweepLimits (lat0, lat1, z0, z1);
        DRay3d ray = DRay3d::FromOriginAndTarget (range.low, range.high);
        bvector<IntTriple> expectedHits;
        // We expect two hits on the full sphere face ...
        expectedHits.push_back (IntTriple (0,0,2));
        if (!isClipped) // Full sphere, have to hit main face twice
            CheckPick (primitive, ray, 2, &expectedHits);
        else if (primitive->GetCapped ())   // Clipped with caps.  2 hits, but don't know which faces
            CheckPick (primitive, ray, 2, NULL);
        else
            {
            // hmph.  Don't know much about what we'll hit.
            }


        expectedHits.clear ();
        Transform localToWorld, worldToLocal;
        
        DPoint3d pointA, pointB;
        double a = 0.1;
        sphere.GetTransforms (localToWorld, worldToLocal);
        localToWorld.Multiply (pointA, a,0,2);
        localToWorld.Multiply (pointB, a,a,-2);
        // Points above and below.  Ray will hit twice for sure if capped or complete...
        ray =DRay3d::FromOriginAndTarget (pointA, pointB);
        CheckPick (primitive, ray, 2, NULL);
        }
    }

struct DPoint3dBilinear
{
DPoint3d xyz[4];
DPoint3d Evaluate (double u, double v)
    {
    return DPoint3d::FromInterpolate (
            DPoint3d::FromInterpolate (xyz[0], u, xyz[1]),
            v,
            DPoint3d::FromInterpolate (xyz[2], u, xyz[3]));
    
    }
};
struct DPoint3dTrilinear
{
DPoint3d xyz[8];
DPoint3d Evaluate (double u, double v, double w)
    {
    DPoint3d slice[4];
    DPoint3d edge[2];
    for (int i = 0; i < 4; i++)
        slice[i].Interpolate (xyz[i], w, xyz[i+4]);
    for (int i = 0; i < 2; i++)
        edge[i].Interpolate (slice[i], v, xyz[i+2]);
    return DPoint3d::FromInterpolate (edge[0], u, edge[1]);
    }
};
void CheckBox (ISolidPrimitivePtr primitive)
    {
    DgnBoxDetail box;
    if (primitive->TryGetDgnBoxDetail (box))
        {
        DPoint3dTrilinear corners;
        DPoint3d pointA, pointB;
        DRay3d ray;
        box.GetCorners (corners.xyz);
        pointA = corners.Evaluate (2.0, 0.5, 0.5);
        pointB = corners.Evaluate (-1.0, 0.5, 0.5);
        ray = DRay3d::FromOriginAndTarget (pointA, pointB);
        CheckPick (primitive, ray, 2, NULL);

        pointA = corners.Evaluate (0.5, 0.5, 2.0);
        pointB = corners.Evaluate (0.5, 0.2, -0.1);
        primitive->SetCapped (false);
        CheckPick (primitive, ray, 0, NULL);
        primitive->SetCapped (true);
        CheckPick (primitive, ray, 2, NULL);
        }
    }

void CheckRotation (ISolidPrimitivePtr primitive)
    {
    DgnRotationalSweepDetail detail;
    DRange3d range;

    if (!primitive->TryGetDgnRotationalSweepDetail (detail))
        return;
    if (primitive->GetRange (range))
        {
        DRay3d ray = DRay3d::FromOriginAndTarget (range.low, range.high);
        // hmmm... Don't really know how many hits.
        CheckPick (primitive, ray, 0, NULL);

        
        DPoint3d middleLow = range.low;
        DPoint3d middleHigh = range.high;
        middleLow.z = middleHigh.z = 0.5 * (middleLow.z + middleHigh.z);
        ray = DRay3d::FromOriginAndTarget (middleLow, middleHigh);
        CheckPick (primitive, ray, 0, NULL);
        }

    DRay3d axis;
    DVec3d dXdu, dXdv, radialVector;
    DPoint3d xyzFace, xyzAxis;
    double f;
    for (double faceFraction = 0.2; faceFraction < 0.95; faceFraction += 0.25)
        {
        double rotationAngle;
        if (Check::True (detail.TryGetRotationAxis (axis.origin, axis.direction, rotationAngle),
                "GetRotationAxis")
            && Check::True (primitive->TryUVFractionToXYZ (
                    SolidLocationDetail::FaceIndices (0,0, 0),
                    faceFraction, faceFraction, xyzFace, dXdu, dXdv),
                    "TryUVToXYZ")
            )
            {
            axis.ProjectPointUnbounded (xyzAxis, f, xyzFace);
            radialVector = DVec3d::FromStartEnd (xyzAxis, xyzFace);
            Check::Perpendicular (radialVector, dXdv);
            }
        }
    }

void CheckFaceEvaluations (ISolidPrimitivePtr primitive,
        SolidLocationDetail::FaceIndices const &face,
        double u, double v)
    {
    DVec3d uVector, vVector;
    DPoint3d xyz;
    Check::True (
        primitive->TryUVFractionToXYZ (face, u, v, xyz, uVector, vVector), "TryGetXYZ");
    DVec3d wVector = DVec3d::FromCrossProduct (uVector, vVector);
    double magW = wVector.Normalize ();
    if (Check::True (magW > 0.001, "independent nonzero surface tangents"))
        {
        static double s_setbackDistance = 0.01; // a pretty small move?
        DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::FromSumOf (xyz, wVector, -s_setbackDistance), wVector);
        bvector<SolidLocationDetail> pickData;
        primitive->AddRayIntersections (pickData, ray, 0, -DBL_MAX);
        int numMatch = 0;
        for (size_t i = 0; i < pickData.size (); i++)
            {
            DPoint2d uv0 = pickData[i].GetUV ();
            DPoint3d xyz0 = pickData[i].GetXYZ ();
            DVec3d   uDirection0 = pickData[i].GetUDirection ();
            DVec3d   vDirection0 = pickData[i].GetVDirection ();
            double   rayFraction = pickData[i].GetPickParameter ();
            SolidLocationDetail::FaceIndices indices0 = pickData[i].GetFaceIndices ();
            if (indices0.Is (face.Index0 (), face.Index1 (), face.Index2 ()) && DPoint3dOps::AlmostEqual (xyz, xyz0))
                {
                Check::Near (uVector, uDirection0, "pick uDirection");
                Check::Near (vVector, vDirection0, "pick vDirection");
                Check::Near (u, uv0.x, "pick u");
                Check::Near (v, uv0.y, "pick v");
                Check::Near (s_setbackDistance, rayFraction, "pick ray fraction");
                numMatch++;
                }
            }
        Check::Int (1, numMatch, "face evaluation inversion");
        // We hope that the setback distance is small enough that xyz is still the closest point !!!
        SolidLocationDetail pick1;
        if (primitive->ClosestPoint (ray.origin, pick1)) // Change to Check::True when all are supported !!!!
            {
            Check::Near (xyz, pick1.GetXYZ (), "closest point");    // need to vette the uv and face id !!!
            }
        else
            {
            printf ("ClosestPoint not supported?\n");
            }
        }
    }

#if defined (_WIN32) && !defined(BENTLEY_WINRT)

static int s_noisyFaces = 0;
void CheckAllFaces (ISolidPrimitivePtr primitive, char const* typeName)
    {
    DRange3d range;
    Check::True (primitive->GetRange (range));
    bvector <SolidLocationDetail::FaceIndices> faces;
    primitive->GetFaceIndices (faces);
    for (size_t i = 0; i < faces.size (); i++)
        {
        IGeometryPtr face = primitive->GetFace (faces[i]);
        DRange3d faceRange = DRange3d::NullRange();
        Check::True
            (
            face->TryGetRange (faceRange),
            " Single Face range"
            );
        Check::True (
            faceRange.IsContained (range),
            "face range within primitive range"
            );

        double uCut = 0.45;
        double vCut = 0.55;
        Check::StartScope ("FacePick", (double)i);
        CheckFaceEvaluations (primitive, faces[i], uCut, vCut);
        Check::EndScope ();
        ICurvePrimitivePtr uSection = primitive->GetConstantUSection (faces[i], uCut);
        ICurvePrimitivePtr vSection = primitive->GetConstantVSection (faces[i], vCut);

        DVec2d uvSize;
        Check::True (primitive->TryGetMaxUVLength (faces[i], uvSize), "Get maxUV");
        static double s_rangeToleranceFactor = 1.05;
        if (uSection.IsValid ())
            {
            DRange3d sectionRange;
            Check::True
                (
                   uSection->GetRange (sectionRange)
                && sectionRange.IsContained (faceRange),
                "u Section range"
                );
            double uLength;
            Check::True (uSection->Length (uLength), "get length");
            // Allow fluff in v sampling??
            Check::True (uLength <= s_rangeToleranceFactor * uvSize.y, "u Section within face size");
            }
        else
            printf (" %s (face %" PRIu64 "%" PRIu64") no uSection\n",
                    typeName, (uint64_t)faces[i].Index0 (), (uint64_t)faces[i].Index1 ());

        if (vSection.IsValid ())
            {
            DRange3d sectionRange;
            Check::True
                (
                   vSection->GetRange (sectionRange)
                && sectionRange.IsContained (faceRange),
                "v Section range"
                );
            double vLength;
            Check::True (vSection->Length (vLength), "get length");
            Check::True (vLength <= s_rangeToleranceFactor * uvSize.x, "v Section within face size");
            }
        else        
            printf (" %s (%" PRIu64 ",%" PRIu64 ") no vSection\n",
                    typeName, (uint64_t)faces[i].Index0 (), (uint64_t)faces[i].Index1 ());
        }
    }

#endif

DRange1d FractionalRange (double maxValue, double fraction0 = 0.0, double fraction1 = 1.0)
    {
    return DRange1d::From (fraction0 * maxValue, fraction1 * maxValue);
    }

// Input two primitives that should have the same moments.
// (e.g. box and extruded rectangle)
bool CheckVolumeMomentMatch_go (ISolidPrimitivePtr prim0, char const* description0, ISolidPrimitivePtr prim1, char const*description1)
    {
    double volume0, volume1;
    DVec3d centroid0, centroid1;
    RotMatrix axes0, axes1;
    DVec3d moment0, moment1;
    Check::StartScope ("Compare moments");
    Check::StartScope (description0);
    Check::StartScope (description1);
    bool stat = 
           Check::True (prim1->ComputePrincipalMoments (volume0, centroid0, axes0, moment0), "Compute primt1")
        && Check::True (prim1->ComputePrincipalMoments (volume1, centroid1, axes1, moment1),"Compute prim2")
        && Check::Near (volume0, volume1, "volume")
        && Check::Near (centroid0, centroid1, "centroid")
        && Check::Near (axes0, axes1, "centroid")
        && Check::Near (moment0, moment1, "moment");
    Check::EndScope ();
    Check::EndScope ();
    Check::EndScope ();
    return stat;
    }

    
bool CheckAreaMomentMatch_go (ISolidPrimitivePtr prim0, char const* description0, ISolidPrimitivePtr prim1, char const*description1)
    {
    double area0 = 0.0, area1 = 0.0;
    DVec3d centroid0, centroid1;
    RotMatrix axes0, axes1;
    DVec3d moment0, moment1;
    Check::StartScope ("Compare moments");
    Check::StartScope (description0);
    Check::StartScope (description1);
    bool stat =
            Check::True (prim0->ComputePrincipalAreaMoments (area0, centroid0, axes0, moment0), "Compute primt1");
    if (stat)
        stat = Check::True (prim1->ComputePrincipalAreaMoments (area1, centroid1, axes1, moment1),"Compute prim2");
    if (stat)
        stat = Check::Near (area0, area1, "area");
    if (stat)
        stat = Check::Near (centroid0, centroid1, "centroid");
    if (stat)
        stat = Check::NearMoments (axes0, moment0, axes1, moment1);
    Check::EndScope ();
    Check::EndScope ();
    Check::EndScope ();
    return stat;
    }

void CheckMomentMatch (ISolidPrimitivePtr prim0, char const* description0, ISolidPrimitivePtr prim1, char const*description1,
    bool checkVolumeMoments = true,
    bool checkAreaMoments = true
    )
    {
    if (checkVolumeMoments && prim0->GetCapped () && prim0->GetCapped ())
        CheckVolumeMomentMatch_go (prim0, description0, prim1, description1);
    if (checkAreaMoments)
        CheckAreaMomentMatch_go (prim0, description0, prim1, description1);

    Check::StartScope ("RotatedPrimitives");
    static double s_rotationAngle = 0.2;
    Transform rotation = Transform::FromLineAndRotationAngle (
                DPoint3d::From (1,2,3),
                DPoint3d::From (1,2,9),
                s_rotationAngle);
    ISolidPrimitivePtr prim0Rotated = prim0->Clone ();
    prim0Rotated->TransformInPlace (rotation);
    ISolidPrimitivePtr prim1Rotated = prim1->Clone ();
    prim1Rotated->TransformInPlace (rotation);
    if (checkVolumeMoments)
        CheckVolumeMomentMatch_go (prim0Rotated, description0, prim1Rotated, description1);
    if (checkAreaMoments)
        CheckAreaMomentMatch_go (prim0Rotated, description0, prim1Rotated, description1);
    Check::EndScope ();
    }


void CheckMoments (ISolidPrimitivePtr primitive)
    {
    static bool s_checkResults = true;
    if (!primitive->IsClosedVolume ())
        return;
    Check::StartScope ("Moments");
    RotMatrix axis0, axis1;
    DVec3d moment0, moment1;
    DVec3d centroid0, centroid1;
    double volume0, volume1;
    // Facet moments really have to work all the time.  Don't expect it for exact moments ...
    if (
        (s_checkResults && Check::True (primitive->ComputeFacetedPrincipalMoments (NULL, volume0, centroid0, axis0, moment0), "facet moments"))
        || (!s_checkResults && primitive->ComputeFacetedPrincipalMoments (NULL, volume0, centroid0, axis0, moment0))
        )
        {
        if (primitive->ComputePrincipalMoments (volume1, centroid1, axis1, moment1))
            {
            static double s_relTol[] = {0.05, 0.05, 0.10, 0.15, 0.15, 0.15};
            double absTol [6];
            double refLength = 0.5 * sqrt (moment0.x + moment0.y + moment0.z);
            if (refLength < 1.0)
                refLength = 1.0;
            double a = 1.0;
            // absTol[i] = tolerance for comparing quantities with length appearing to power i
            for (int i = 0; i < 6; i++)
                {
                absTol[i] = a * s_relTol[i];
                a *= refLength;
                }

            double centroidDistance = centroid0.Distance (centroid1);
            if (s_checkResults)
                {
                Check::True (DoubleOps::AlmostEqual (volume0, volume1, absTol[3]), "volume");
                Check::True (DoubleOps::AlmostEqual(moment0.Magnitude (), moment1.Magnitude (), absTol[5]), "moment");
                //Check::True (axis0, axis1, "axes", axisRef);
                Check::True (DoubleOps::AlmostEqual (0, centroidDistance, absTol[1]), "centroid distance");
                }
            }
        }
    Check::EndScope ();
    }
    
void CheckAreaMoments (ISolidPrimitivePtr primitive)
    {
    static bool s_checkResults = true;
    Check::StartScope ("AreaMoments");
    RotMatrix axis0, axis1;
    DVec3d moment0, moment1;
    DVec3d centroid0, centroid1;
    double area0, area1;
    // Facet moments really have to work all the time.  Don't expect it for exact moments ...
    if (
        (s_checkResults && Check::True (primitive->ComputeFacetedPrincipalAreaMoments (NULL, area0, centroid0, axis0, moment0), "facet moments"))
        || (!s_checkResults && primitive->ComputeFacetedPrincipalAreaMoments (NULL, area0, centroid0, axis0, moment0))
        )
        {
        if (primitive->ComputePrincipalAreaMoments (area1, centroid1, axis1, moment1))
            {
            static double s_relTol[] = {0.05, 0.05, 0.10, 0.15, 0.15, 0.15};
            double absTol [6];
            double refLength = 0.5 * sqrt (moment0.x + moment0.y + moment0.z);
            if (refLength < 1.0)
                refLength = 1.0;
            double a = 1.0;
            // absTol[i] = tolerance for comparing quantities with length appearing to power i
            for (int i = 0; i < 6; i++)
                {
                absTol[i] = a * s_relTol[i];
                a *= refLength;
                }

            double centroidDistance = centroid0.Distance (centroid1);
            if (s_checkResults)
                {
                Check::True (DoubleOps::AlmostEqual (area0, area1, absTol[2]), "area");
                Check::True (DoubleOps::AlmostEqual(moment0.Magnitude (), moment1.Magnitude (), absTol[4]), "moment");
                //Check::True (axis0, axis1, "axes", axisRef);
                Check::True (DoubleOps::AlmostEqual (0, centroidDistance, absTol[1]), "centroid distance");
                }
            }
        }
    Check::EndScope ();
    }
    
    
    
void CheckPolyfaceToMTG (ISolidPrimitivePtr primitive, PolyfaceHeaderPtr polyface)
    {
    Check::StartScope ("PolyfaceToMTG");
    MTGFacets* mtgFacets = jmdlMTGFacets_new ();
    if (Check::True (PolyfaceToMTG_FromPolyfaceConnectivity (mtgFacets, *polyface)))
        {
        MTGGraphP graph = jmdlMTGFacets_getGraph (mtgFacets);
        bool expectClosed = primitive->IsClosedVolume ();
        size_t numPolar    = graph->CountMask (MTG_POLAR_LOOP_MASK);
        size_t numBoundary = graph->CountMask (MTG_EXTERIOR_MASK);
        if (expectClosed)
            {
            Check::Size (numBoundary, numPolar, "No boundary for mesh of closed primitive");
            }
        else
            Check::True (numBoundary > 0, "Mesh of closed primitive should have boundary");
        }
    jmdlMTGFacets_free (mtgFacets);
    Check::EndScope ();
    }
    
void CheckFacets (ISolidPrimitivePtr primitive,
bool expectExactMeshFaceCountMatch  // true to enforce exact match of mesh and SolidPrimitive "faces". (e.g. set false for bsplines that might get interior visible edges)
)
    {
    PolyfaceHeaderPtr facets = PolyfaceHeader::CreateVariableSizeIndexed ();
    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    if (!Check::True (builder->AddSolidPrimitive (*primitive), "Builder.AddSolidPrimitive"))
        return;
        

    DRange3d range;
    Check::True (primitive->GetRange (range));
    DVec3d diagonal = DVec3d::FromStartEnd (range.low, range.high);
    //double rangeVolume = diagonal.x * diagonal.y * diagonal.z;
    double rangeArea = 2.0 * (diagonal.x * diagonal.y + diagonal.y * diagonal.z + diagonal.z * diagonal.x);
    PolyfaceHeaderPtr header = builder->GetClientMeshPtr ();
    
    double facetArea = header->SumFacetAreas ();
    DRange3d facetRange = header->PointRange ();
    static double s_areaFraction = 0.1;
    static double s_rangeFraction = 0.8;
    
    Check::StartScope ("CheckPrimitiveFacets");
    if (Check::Contains (FractionalRange (rangeArea, s_areaFraction),
                    facetArea, "Minimal facet area"))
        {
        Check::Contains (FractionalRange (range.low.Distance (range.high), s_rangeFraction),
                facetRange.low.Distance (facetRange.high), "Minimal facet range");
        }

    CheckPolyfaceToMTG (primitive, header);
    Check::EndScope ();

    bvector<SolidLocationDetail::FaceIndices> faces;
    primitive->GetFaceIndices (faces);
    bvector<ptrdiff_t> blockedIndices;
    header->PartitionByConnectivity (2, blockedIndices);
    size_t numBlocks = 0;
    for (size_t i = 0; i < blockedIndices.size (); i++)
        {
        if (blockedIndices[i] < 0)
            numBlocks++;
        }
#ifdef considerExactMeshFaceCounts
     if (expectExactMeshFaceCountMatch)
        Check::Size (faces.size (), numBlocks, "Solid Primitive face count per visibility");            
    else 
#endif
    Check::True (faces.size () <= numBlocks, "Mesh must have at least as many faces as solid primitive");
    }

void CheckPrimitiveAsTrimmedSurfaces (ISolidPrimitivePtr primitive)
    {
    Check::StartScope ("TrimmedSurfaces");
    bvector<MSBsplineSurfacePtr> surfaces;
    MSBsplineSurface::CreateTrimmedSurfaces (surfaces, *primitive);
    Check::EndScope ();
    }
    
void CheckPrimitiveA (ISolidPrimitivePtr primitive, SolidPrimitiveType primitiveType, DSegment3dCP segment = NULL, int numHits = 0)
    {
    Check::Int (primitiveType, primitive.get ()->GetSolidPrimitiveType ());
    CheckTryGet (DgnTorusPipe);
    CheckTryGet (DgnCone);
    CheckTryGet (DgnExtrusion);
    CheckTryGet (DgnBox);
    CheckTryGet (DgnSphere);
    CheckTryGet (DgnRuledSweep);
    CheckTryGet (DgnRotationalSweep);

    DRange3d range;
    bool rstat = primitive->GetRange (range);

    if (!rstat)
        {
        printf ("No range %d\n",
            (int)primitive.get ()->GetSolidPrimitiveType ());
        return;
        }
#ifdef PrintRangeDetails
    printf (" (Range (%g,%g,%g) (%g,%g,%g)\n",
            range.low.x,  range.low.y,  range.low.z,
            range.high.x, range.high.y, range.high.z);
#endif
    // rudimentary uvToXYZ checks (no numerical validation, just prove it can be called
    // for each cap and for (0,0) side face.
    {
    DPoint3d xyz;
    DVec3d dXdu, dXdv;
    Check::True (
        primitive->TryUVFractionToXYZ (
                SolidLocationDetail::FaceIndices (SolidLocationDetail::PrimaryIdCap, 0, 0),
                0.5, 0.5, xyz, dXdu, dXdv),
            "Cap0 @.5,.5");
    Check::True (
        primitive->TryUVFractionToXYZ (
                SolidLocationDetail::FaceIndices (SolidLocationDetail::PrimaryIdCap, 1, 0),
                0.5, 0.5, xyz, dXdu, dXdv),
            "Cap1 @.5,.5");
    Check::True (
        primitive->TryUVFractionToXYZ (
                SolidLocationDetail::FaceIndices (0, 0, 0),
                0.5, 0.5, xyz, dXdu, dXdv),
            "face00 @.5,.5");
    }
    CheckCone (primitive);
    CheckSphere (primitive);
    CheckBox (primitive);
    CheckRotation (primitive);
    CheckPick (primitive, segment, numHits);

    }

void CheckTransformedCurveInRange (ICurvePrimitivePtr curve, TransformCR worldToLocal, DRange3dCR localRange)
    {
    DPoint3d xyzWorld, xyzLocal;
    if (curve->GetStartPoint (xyzWorld))
        {
        worldToLocal.Multiply (xyzLocal, xyzWorld);
        Check::True (localRange.IsContained (xyzLocal), "CurvePoint in range");
        }
    }

void CheckConstructiveFrame (ISolidPrimitiveR primitive, char const*typeName)
    {
    char message[1024];
    snprintf (message, sizeof(message), "ConstructiveFrame %s", typeName == NULL ? "" : typeName);
    Check::StartScope (message);
    Transform localToWorld, worldToLocal;
    DRange3d rangeA;
    static double s_tolerance = 1.0e-8;
    if (Check::True (primitive.TryGetConstructiveFrame (localToWorld, worldToLocal), "GetConstructiveFrame")
        && Check::True (primitive.GetRange (rangeA, worldToLocal), "GetRange(T)")
        )
        {
        rangeA.Extend (s_tolerance);
        bvector<SolidLocationDetail::FaceIndices> faces;
        primitive.GetFaceIndices (faces);
        for (size_t i = 0; i < faces.size (); i++)
            {
            for (double f = 0.0; f <= 1.0; f += 0.25)
                {
                ICurvePrimitivePtr uCut = primitive.GetConstantUSection (faces[i], f);
                ICurvePrimitivePtr vCut = primitive.GetConstantVSection (faces[i], f);
                CheckTransformedCurveInRange (uCut, worldToLocal, rangeA);
                CheckTransformedCurveInRange (vCut, worldToLocal, rangeA);
                }
            }
        }
    Check::EndScope ();
    }

#if defined (_WIN32) && !defined(BENTLEY_WINRT)

void CheckPrimitiveB (ISolidPrimitivePtr primitive, char const*typeName, bool expectExactMeshFaceCountMatch)
    {

    CheckAllFaces (primitive, typeName);
    CheckFacets (primitive, expectExactMeshFaceCountMatch);
    
    CheckConstructiveFrame (*primitive, typeName);
    if (primitive->GetCapped ())
        {
        Check::StartScope ("REMOVE CAP FOR MOMENT CHECK");
        primitive->SetCapped (false);
        //CheckMoments (primitive);
        CheckAreaMoments (primitive);
        primitive->SetCapped (true);
        Check::EndScope ();
        }
    
    }

void CheckPrimitiveSimplify (ISolidPrimitivePtr primitiveA)
    {
    ISolidPrimitivePtr primitiveB = primitiveA->Clone ();
    if (ISolidPrimitive::Simplify (primitiveB))
        {
        Check::False (primitiveA->IsSameStructureAndGeometry (*primitiveB));
        }
    }

void CheckPrimitive (ISolidPrimitivePtr primitive, SolidPrimitiveType primitiveType, DSegment3dCP segment = NULL, int numHits = 0, bool expectExactMeshFaceCountMatch = true)
    {
    static bool s_printNames = false;
    char const*typeName = "";
    Check::Int (primitiveType, primitive.get ()->GetSolidPrimitiveType ());
    CheckTryGet (DgnTorusPipe);
    CheckTryGet (DgnCone);
    CheckTryGet (DgnExtrusion);
    CheckTryGet (DgnBox);
    CheckTryGet (DgnSphere);
    CheckTryGet (DgnRuledSweep);
    CheckTryGet (DgnRotationalSweep);

    if (s_printNames)
        printf ("(SolidPrimitiveType %d ", primitive.get ()->GetSolidPrimitiveType ());
    SolidPrimitiveType type1 = primitive.get ()->GetSolidPrimitiveType ();
    if (type1 == SolidPrimitiveType_DgnBox)
        typeName = "Box";
    else if (type1 == SolidPrimitiveType_DgnSphere)
        typeName = "Sphere";
    else if (type1 == SolidPrimitiveType_DgnTorusPipe)
        typeName = "TorusPipe";
    else if (type1 == SolidPrimitiveType_DgnCone)
        typeName = "Cone";
    else if (type1 == SolidPrimitiveType_DgnExtrusion)
        typeName = "Extrusion";
    else if (type1 == SolidPrimitiveType_DgnRotationalSweep)
        typeName = "RotationalSweep";
    else if (type1 == SolidPrimitiveType_DgnRuledSweep)
        typeName = "RuledSweep";
    else
        typeName = "Unknown";
    if (s_printNames)
        printf ("%s (%s)\n", typeName, primitive->GetCapped () ? "Capped" : "NoCap");
    Check::StartScope (typeName);

    CheckPrimitiveA (primitive, primitiveType, segment, numHits);
    CheckPrimitiveB (primitive, typeName, expectExactMeshFaceCountMatch);

    CheckMoments (primitive);
    CheckAreaMoments (primitive);
    
    CheckPrimitiveAsTrimmedSurfaces (primitive);
    CheckPrimitiveSimplify (primitive);

    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, DgnExtrusion)
    {
    double ax = 1.0;
    double ay = 2.0;
    double bx = 3.0;
    double by = 4.0;
    double z0 = 0.5;
    double dz = 4.0;
    double z1 = z0 + dz;
    CurveVectorPtr pathA = CurveVector::CreateRectangle (ax, ay, bx, by, z0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr pathB = CurveVector::CreateRectangle (ax, ay, bx, by, z1, CurveVector::BOUNDARY_TYPE_Outer);
    
    Transform localToWorld, worldToLocal;
    Check::True (pathA->IsRectangle (localToWorld, worldToLocal));
 
    
    bvector<CurveVectorPtr> sections;
    sections.push_back (pathA);
    sections.push_back (pathB);
    DgnExtrusionDetail extrusionData (
            pathA,
            DVec3d::From (0,0,dz),
            false
            );

    ISolidPrimitivePtr extrusion = ISolidPrimitive::CreateDgnExtrusion (extrusionData);
    CheckPrimitive (extrusion, SolidPrimitiveType_DgnExtrusion);

    DgnRuledSweepDetail ruledDetail (sections, false);
    ISolidPrimitivePtr ruled = ISolidPrimitive::CreateDgnRuledSweep (ruledDetail);
    CheckPrimitive (ruled, SolidPrimitiveType_DgnRuledSweep);

    CheckMomentMatch (extrusion, "ExtrudedBox sides", ruled, "RuledBox sides", false, true);

    extrusion->SetCapped (true);
    ruled->SetCapped (true);
    CheckPrimitive (extrusion, SolidPrimitiveType_DgnExtrusion);
    
    DgnBoxDetail boxData (
        DPoint3d::From (ax, ay, z0),
        DPoint3d::From (ax, ay, z1),
        DVec3d::From (1,0,0),
        DVec3d::From (0,1,0),
        bx - ax, by - ay,
        bx - ax, by - ay,
        true
        );
    ISolidPrimitivePtr box = ISolidPrimitive::CreateDgnBox (boxData);
    CheckMomentMatch (extrusion, "extruded rectangle", box, "box");

    CheckMomentMatch (extrusion, "ExtrudedBox", ruled, "RuledBox", false, true);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateTorus0)
    {
    double sweep = 0.10;
    double R = 10.0;
    double dR = 0.1;
    double sweepFactor = 2.5;
    for (;sweep < 3.0; sweep = sweepFactor * sweep, R += dR)
        {
        DgnTorusPipeDetail torusData (
            DPoint3d::From (0,0,0),
            DVec3d::From (1,0,0),
            DVec3d::From (0,1,0),
            R,
            0.5,
            sweep,
            true);
        DgnTorusPipeDetail torusData1;
        ISolidPrimitivePtr torus = ISolidPrimitive::CreateDgnTorusPipe (torusData);
        torus->TryGetDgnTorusPipeDetail (torusData1);
        CheckPrimitive (torus, SolidPrimitiveType_DgnTorusPipe);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateTorus)
    {
    DgnTorusPipeDetail torusData (
        DPoint3d::From (0,0,0),
        DVec3d::From (1,0,0),
        DVec3d::From (0,1,0),
        2,
        0.5,
        Angle::PiOver2 (),
        true);
    DgnTorusPipeDetail torusData1;
    ISolidPrimitivePtr torus = ISolidPrimitive::CreateDgnTorusPipe (torusData);
    torus->TryGetDgnTorusPipeDetail (torusData1);
    CheckPrimitive (torus, SolidPrimitiveType_DgnTorusPipe);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateTorus1)
    {
    DgnTorusPipeDetail torusData (
        DPoint3d::From (0,0,0),
        DVec3d::From (1,0,0),
        DVec3d::From (0,1,0),
        2,
        0.5,
        Angle::TwoPi (),
        false);
    DgnTorusPipeDetail torusData1;
    ISolidPrimitivePtr torus = ISolidPrimitive::CreateDgnTorusPipe (torusData);
    torus->TryGetDgnTorusPipeDetail (torusData1);
    CheckPrimitive (torus, SolidPrimitiveType_DgnTorusPipe);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateCone)
    {
    double rA = 1.3;
    Transform spinner;
    double zA = 0.0;
    double zB = 1.0;
    spinner = Transform::FromLineAndRotationAngle (DPoint3d::From (1,2,3), DPoint3d::From (2,5,1), 1.0);
    int n = 0;
    static int nMax = 5;
    for (double rB = 0.2; rB < 3.4 && ++n < nMax; rB *= 1.5, zB += 0.2)
        {
        DgnConeDetail ConeData (
            DPoint3d::From (0,0,zA),
            DPoint3d::From (0,0,zB),
            //DVec3d::From (1,0,0),
            //DVec3d::From (0,1,0),
            rA, rB, true);
        ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone (ConeData);
        CheckPrimitive (cone, SolidPrimitiveType_DgnCone);
        Check::StartScope ("SPIN");
        cone->TransformInPlace (spinner);
        CheckPrimitive (cone, SolidPrimitiveType_DgnCone);
        Check::EndScope ();
        }
        
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail (DPoint3d::From (0,0,0), DPoint3d::From (0,0,dz), radius, radius, true);
    CurveVectorPtr ellipse = CurveVector::CreateDisk (
                    DEllipse3d::From (0,0,0,   radius, 0,0,   0, radius,0,   0.0, Angle::TwoPi ()));
    Transform localToWorld, worldToLocal;
    Check::False (ellipse->IsRectangle (localToWorld, worldToLocal));
                    
    DgnExtrusionDetail extrusionDetail (ellipse, DVec3d::From (0,0,dz), true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone (cylinderDetail);
    ISolidPrimitivePtr extrusion = ISolidPrimitive::CreateDgnExtrusion (extrusionDetail);
    
    CheckMomentMatch (cylinder, "cylinder", extrusion, "extrudedCircle");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (SolidPrimitive,TrueCone)
    {
    Check::StartScope ("Cone Tip");
    DPoint3d centerA = DPoint3d::From(0,0,0);
    DPoint3d centerB = DPoint3d::From (0,0,1);
    double rA = 1.0;
    double rB = 0.0;
    auto coneA = ISolidPrimitive::CreateDgnCone (DgnConeDetail (centerA, centerB, rA, rB, true));
    // Same physical surface, ends swapped for definition.
    auto coneB = ISolidPrimitive::CreateDgnCone (DgnConeDetail (centerB, centerA, rB, rA, true));
    CheckPrimitive (coneA, SolidPrimitiveType_DgnCone);
    CheckPrimitive (coneB, SolidPrimitiveType_DgnCone);
    CheckMomentMatch (coneA, "Cone tip at B", coneB, "Cone tip at A");
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateZeroLengthCone)
    {
    double rA = 1.3;
    double rB = 1.0;
    double zA = 0.0;
    double zB = 1.0;
    DgnConeDetail ConeData(
        DPoint3d::From(0, 0, zA),
        DPoint3d::From(0, 0, zB),
        rA, rB, true);
    ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone(ConeData);
    CheckPrimitive(cone, SolidPrimitiveType_DgnCone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (SolidPrimitive, ConeAreaMoments)
    {
    double dz = 3.0;
    double rA = 4.0;
    for (double rB = rA; rB > 1.0; rB *= 0.5)
        {
        DPoint3d centerA = DPoint3d::From (0,0,0);
        DPoint3d centerB = DPoint3d::From (0,0,dz);
        DgnConeDetail cylinderDetail (centerA, centerB, rA, rB, false);
        CurveVectorPtr ellipseA = CurveVector::CreateDisk (
                        DEllipse3d::From (centerA.x, centerA.y, centerA.z,   rA, 0,0,   0, rA,0,   0.0, Angle::TwoPi ()));
        CurveVectorPtr ellipseB = CurveVector::CreateDisk (
                        DEllipse3d::From (centerB.x, centerB.y, centerB.z,   rB, 0,0,   0, rB,0,   0.0, Angle::TwoPi ()));
        bvector<CurveVectorPtr> sections;
        sections.push_back (ellipseA);
        sections.push_back (ellipseB);
        DgnRuledSweepDetail ruledDetail (sections, false);
        ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone (cylinderDetail);
        ISolidPrimitivePtr ruled = ISolidPrimitive::CreateDgnRuledSweep (ruledDetail);
        
        CheckMomentMatch (cone, "cone", ruled, "ruled", false, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateSphere)
    {
    DgnSphereDetail sphereData;
    ISolidPrimitivePtr sphere = ISolidPrimitive::CreateDgnSphere (sphereData);
    CheckPrimitive (sphere, SolidPrimitiveType_DgnSphere);

    sphereData = DgnSphereDetail  (
                    DPoint3d::From(10,11,9),
                    DVec3d::From (1,2,3),
                    DVec3d::From (-2,1,5),
                    4,3,
                    0.0, 1.0, true);
    sphereData = DgnSphereDetail  (
                    DPoint3d::From(0,0,0),
                    DVec3d::From (1,0,0),
                    DVec3d::From (0,1,0),
                    1,1,
                    0.0, 0.1, true);
    sphere = ISolidPrimitive::CreateDgnSphere (sphereData);
    CheckPrimitive (sphere, SolidPrimitiveType_DgnSphere);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateBox)
    {
    DgnBoxDetail boxData (
                DPoint3d::From (1,1,1),
                DPoint3d::From (1,1,2),
                DVec3d::From (1,0,0), DVec3d::From (0,1,0), 2,2,2,2, true);
    ISolidPrimitivePtr box = ISolidPrimitive::CreateDgnBox (boxData);
    CheckPrimitive (box, SolidPrimitiveType_DgnBox);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateSkewedBox)
    {
    DgnBoxDetail boxData (
                DPoint3d::From (1,1,1),
                DPoint3d::From (2,3,4),
                DVec3d::From (1,0,0), DVec3d::From (0,1,0), 1,2,2.5,2.9, true);
    ISolidPrimitivePtr box = ISolidPrimitive::CreateDgnBox (boxData);
    CheckPrimitive (box, SolidPrimitiveType_DgnBox);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IsRectangle,Test0)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (2,0,0));
    points.push_back (DPoint3d::From (2,1,0));
    points.push_back (DPoint3d::From (0,1,0));
    bvector<DPoint3d> savedPoints = points;
    Transform A, B;
    Check::True (PolylineOps::IsRectangle (points, A, B, false));

    points[2].x = 3.0;
    Check::False(PolylineOps::IsRectangle (points, A, B, false));
    points = savedPoints;
    
    points.push_back (savedPoints[0]);
    Check::True (PolylineOps::IsRectangle (points, A, B, true));

    points[2].x = 3.0;
    Check::False(PolylineOps::IsRectangle (points, A, B, true));
    points = savedPoints;
    points.push_back (savedPoints[0]);    
    points[4].x += 1.0;
    Check::False(PolylineOps::IsRectangle (points, A, B, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateExtrusion)
    {
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ICurvePrimitivePtr lineA = ICurvePrimitive::CreateLine (
                    DSegment3d::From (DPoint3d::From (1,1,0), DPoint3d::From (2,1,0)));
    pathA->push_back (lineA);
    Transform localToWorld, worldToLocal;
    Check::False (pathA->IsRectangle (localToWorld, worldToLocal));


    CurveVectorPtr pathB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ICurvePrimitivePtr lineB = ICurvePrimitive::CreateLine (
                    DSegment3d::From (DPoint3d::From (1,1,1), DPoint3d::From (2,1,1)));
    pathB->push_back (lineB);
    
    DgnExtrusionDetail extrusionData (
            pathA,
            DVec3d::From (0,0,2),
            false
            );

    ISolidPrimitivePtr extrusion = ISolidPrimitive::CreateDgnExtrusion (extrusionData);
    CheckPrimitive (extrusion, SolidPrimitiveType_DgnExtrusion);



    bvector<CurveVectorPtr> ruledSections;
    ruledSections.push_back (pathA);
    ruledSections.push_back (pathB);
    DgnRuledSweepDetail ruledSweepData (ruledSections, false);
    ISolidPrimitivePtr ruledSweep = ISolidPrimitive::CreateDgnRuledSweep (ruledSweepData);
    CheckPrimitive (ruledSweep, SolidPrimitiveType_DgnRuledSweep);
    ruledSweep->SetCapped (false);
    CheckPrimitive (ruledSweep, SolidPrimitiveType_DgnRuledSweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateRuledBsplines)
    {
    bvector<DPoint3d> poleA, poleB;
    double dTheta = Angle::DegreesToRadians (22.5);
    static int s_poleCount = 5;
    double scaleA = 1.0;
    double scaleB = 2.0;
    for (int i = 0; i < s_poleCount; i++)
        {
        double theta = i * dTheta;
        poleA.push_back (DPoint3d::From (scaleA * cos(theta), scaleA * sin(theta), 0.0));
        poleB.push_back (DPoint3d::From (scaleB * cos(theta), scaleB * sin(theta), 1.0));
        }


    MSBsplineCurvePtr curveA = MSBsplineCurve::CreateFromPolesAndOrder (poleA, NULL, NULL, 3, false, false);
    MSBsplineCurvePtr curveB = MSBsplineCurve::CreateFromPolesAndOrder (poleB, NULL, NULL, 3, false, false);

    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (ICurvePrimitive::CreateBsplineCurve (*curveA));

    CurveVectorPtr pathB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathB->push_back (ICurvePrimitive::CreateBsplineCurve (*curveB));

    bvector<CurveVectorPtr> ruledSections;
    ruledSections.push_back (pathA);
    ruledSections.push_back (pathB);
    DgnRuledSweepDetail ruledSweepData (ruledSections, false);
    ISolidPrimitivePtr ruledSweep = ISolidPrimitive::CreateDgnRuledSweep (ruledSweepData);
    CheckPrimitive (ruledSweep, SolidPrimitiveType_DgnRuledSweep, nullptr, 0, false);
    }



void CheckRuledArcs (double b, double scale)
    {
    double sweep = 2.0;
    DEllipse3d ellipseA = DEllipse3d::From (1,1,1,    1,0,0,   0,b,0,   0.0, sweep);
    DEllipse3d ellipseB = DEllipse3d::FromVectors (
            DPoint3d::From (1,1,2),
            DVec3d::FromScale (ellipseA.vector0, scale), 
            DVec3d::FromScale (ellipseA.vector90, scale),
            0.0, sweep); 
    // NEEDS WORK -- ray pick does not work when skewed.
    //DEllipse3d ellipseB = DEllipse3d::From (1,1,4,    1.3,0,0,   0,1.8,0,   0.0, sweep);

    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (ICurvePrimitive::CreateArc (ellipseA));

    CurveVectorPtr pathB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathB->push_back (ICurvePrimitive::CreateArc (ellipseB));

    bvector<CurveVectorPtr> ruledSections;
    ruledSections.push_back (pathA);
    ruledSections.push_back (pathB);
    DgnRuledSweepDetail ruledSweepData (ruledSections, false);
    ISolidPrimitivePtr ruledSweep = ISolidPrimitive::CreateDgnRuledSweep (ruledSweepData);
    CheckPrimitive (ruledSweep, SolidPrimitiveType_DgnRuledSweep);
    }

// Fails in rayIntersectRule !!!!!
// (Newer "closest point" works fine ....
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, CreateRuledArcs)
    {
    for (double f = 0.5; f < 3.0; f *= 2.0)
        {
        CheckRuledArcs (1.0, f);
        CheckRuledArcs (2.0, f);
        CheckRuledArcs (0.5, f);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, Rotation)
    {
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    //    3               E
    //    |            
    //    |            
    //    2       C>>>D       F   
    //    |      /
    //    |     /
    //    1   B
    //    |   ^
    //    |   ^
    //    0---A---2---3---4---5---6
    DPoint3d pointA = DPoint3d::From (1,0,0);
    DPoint3d pointB = DPoint3d::From (1,0,1);
    DPoint3d pointC = DPoint3d::From (2,0,2);
    DPoint3d pointD = DPoint3d::From (3,0,2);
    DPoint3d pointE = DPoint3d::From (4,0,3);
    DPoint3d pointF = DPoint3d::From (5,0,2);
    DEllipse3d ellipseDEF = DEllipse3d::FromPointsOnArc (pointD, pointE, pointF);

    // cylinder
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    // cone
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    // annulus
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointD)));
    pathA->push_back (ICurvePrimitive::CreateArc (ellipseDEF));

    DPoint3d center = DPoint3d::From (0,0,0);
    DVec3d   axis   = DVec3d::From (0,0,1);
    DgnRotationalSweepDetail rotationalSweepData (pathA, center, axis, Angle::TwoPi (), false);
    ISolidPrimitivePtr rotationalSweep = ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepData);
    DSegment3d segment = DSegment3d::From (DPoint3d::From (0,0,0), DPoint3d::From (2,0,0.5));
    CheckPrimitive (rotationalSweep, SolidPrimitiveType_DgnRotationalSweep, &segment, 1);

    rotationalSweepData = DgnRotationalSweepDetail (pathA, DPoint3d::From (0,0,0), DVec3d::From (1,0,0), Angle::TwoPi (), false);
    rotationalSweep = ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepData);
    segment = DSegment3d::From(DPoint3d::From (0,0,1), DPoint3d::From(5,0,3)); // hits x rotation 4 times.
    CheckPrimitive (rotationalSweep, SolidPrimitiveType_DgnRotationalSweep, &segment, 4);
#ifdef TestOutOfPlaneRotation
    rotationalSweepData = DgnRotationalSweepDetail (pathA, DPoint3d::From (1,2,3), DVec3d::From (0.3, 0.4, -0.2), Angle::TwoPi (), false);
    rotationalSweep = ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepData);
    CheckPrimitive (rotationalSweep, SolidPrimitiveType_DgnRotationalSweep);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TorusSurf,Implicits)
    {
    Polynomial::Implicit::Torus surface1 (10, 1, DgnTorusPipeDetail::GetReverseVector90 ());
    Polynomial::Implicit::Torus  surface2 (10, 2, DgnTorusPipeDetail::GetReverseVector90 ());
    Polynomial::Implicit::Torus  surface3 (10, 3, DgnTorusPipeDetail::GetReverseVector90 ());
    Polynomial::Implicit::Torus  surface4 (10,3.1, DgnTorusPipeDetail::GetReverseVector90 ());

    for (double theta = -2.0; theta < 4.0; theta += 1.0)
        {
        for (double phi = -3.0; phi < 4.0; phi += 1.0)
            {
            DPoint3d point1 = surface1.EvaluateThetaPhi (theta, phi);
            //DPoint3d point2 = surface2.EvaluateThetaPhi (theta, phi);
            DPoint3d point3 = surface3.EvaluateThetaPhi (theta, phi);
            DPoint3d point4 = surface4.EvaluateThetaPhi (theta, phi);
            double f1 = surface2.EvaluateImplicitFunction (point1);
            //double f2 = surface2.EvaluateImplicitFunction (point2);
            double f3 = surface2.EvaluateImplicitFunction (point3);
            double f4 = surface2.EvaluateImplicitFunction (point4);
            Check::True (f1 * f3 < 0.0, "torus implicit sign change");
            Check::True (f3 * f4 > 0.0, "torus implicit no sign change");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TorusSurf,RayPierce)
    {
    Polynomial::Implicit::Torus  surface (10, 1, DgnTorusPipeDetail::GetReverseVector90 ());
    static double s_fractionTol = 1.0e-8;
    double thetaShift = 0.0;
    double phiShift = Angle::Pi ();
    static double offset = 1;
    for (double theta0 = 0.0; theta0 < 4.0; theta0 += 1.0)
        {
        thetaShift += 0.1;
        for (double phi0 = 0.0; phi0 < 4.0; phi0 += 1.0)
            {
            double theta1 = theta0 + thetaShift;
            double phi1   = phi0   + phiShift;
            DPoint3d point0 = surface.EvaluateThetaPhi (theta0, phi0);
            DPoint3d point1 = surface.EvaluateThetaPhi (theta1, phi1);
            DRay3d ray = DRay3d::FromOriginAndTarget (point0, point1);
            double fractions[10];
            DPoint3d points[10];
            int numHit = surface.IntersectRay (ray, fractions, points, 10);
            Check::True (numHit >= 2, "At least two hits");
            Check::Near (offset, offset + surface.EvaluateImplicitFunction (point0), "ray start");
            Check::Near (offset, offset + surface.EvaluateImplicitFunction (point1), "ray end");
            int num0 = 0;
            int num1 = 0;
            for (int i = 0; i < numHit; i++)
                {
                double s = fractions[i];
                if (fabs (s) < s_fractionTol)
                    num0++;
                if (fabs (1.0 - s) < s_fractionTol)
                    num1++;
                double f = surface.EvaluateImplicitFunction (points[i]);
                Check::Near (offset + f, offset, "pierce point function");
                }

            Check::Int (num0, 1);
            Check::Int (num1, 1);
            
            }
        }
    }



double CheckRotate90 (DVec3dCR vector)
    {
    RotMatrix matrix0 = RotMatrix::FromVectorAndRotationAngle (vector, Angle::PiOver2 ());
    RotMatrix matrix1 = RotMatrix::FromRotate90 (vector);
    Check::Near (matrix0, matrix1, "rotate90"); 
    return matrix0.MaxDiff (matrix1);
    }

void CheckRotationDerivative (DVec3dCR axis, double angle)
    {
    RotMatrix derivative0;    
    RotMatrix matrix0 = RotMatrix::FromVectorAndRotationAngle (axis, angle, derivative0);
    DVec3d vector[3];
    axis.GetNormalizedTriad (vector[0], vector[1], vector[2]);
    Check::Near (0.0, vector[0].DotProduct (axis), "TriadX");
    Check::Near (0.0, vector[1].DotProduct (axis), "TriadX");
    Check::Near (axis.Magnitude (), vector[2].DotProduct (axis), "TriadX");
    for (int i = 0; i < 2; i++)
        {
        DVec3d AX, DX;
        AX.Multiply (matrix0, vector[i]);
        DX.Multiply (derivative0, vector[i]);
        Check::NearPeriodic (angle, vector[i].SignedAngleTo (AX, axis), "rotation by angle");
        Check::NearPeriodic (angle + Angle::PiOver2 (),
                vector[i].SignedAngleTo (DX, axis), "rotation by angle derivative");
        }

    for (double theta = 0.0; theta < 1.0; theta += 0.25)
        {
        double c = cos (theta);
        double s = sin (theta);
        DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::From (1,2,3), axis);
        Transform rotationTransform, derivativeTransform;
        rotationTransform = Transform::FromAxisAndRotationAngle (ray, angle, derivativeTransform);
        //DPoint3d xyzOnRay = DPoint3d::FromSumOf (ray.origin, vector[2], 0.729);
        DPoint3d xyz0 = DPoint3d::FromSumOf (ray.origin, vector[0], c, vector[1], s);
        DPoint3d xyz1;
        rotationTransform.Multiply (xyz1, xyz0);
        DVec3d radialVector0 = DVec3d::FromStartEnd (ray.origin, xyz0);
        DVec3d radialVector1 = DVec3d::FromStartEnd (ray.origin, xyz1);
        DVec3d tangentVector1;
        derivativeTransform.Multiply (tangentVector1, xyz0);
        Check::Perpendicular (radialVector1, tangentVector1);
        Check::Perpendicular (radialVector1, axis);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,Rotate90)
    {
    double d = 0.0;
    d = DoubleOps::Max (d, CheckRotate90 (DVec3d::From (2,3,5)));
    d = DoubleOps::Max (d, CheckRotate90 (DVec3d::From (-2,7,-13)));
    d = DoubleOps::Max (d, CheckRotate90 (DVec3d::From (2,-3,5)));

    for (double theta = 0.0; theta < 2; theta += 0.5)
        {
        CheckRotationDerivative (DVec3d::From (1,0,0), theta);
        CheckRotationDerivative (DVec3d::From (0,1,0), theta);
        CheckRotationDerivative (DVec3d::From (0,0,1), theta);
        }

    double dTheta = 0.1;
    for (int i = 0; i < 70; i++)
        {
        double theta = i * dTheta;
        double z = 5 * cos (theta);
        d = DoubleOps::Max (d, CheckRotate90 (DVec3d::From (2,3,z)));
        CheckRotationDerivative (DVec3d::From (2,3,z), theta);
        CheckRotationDerivative (DVec3d::From (2,3,z), theta + Angle::PiOver2 ());
        CheckRotationDerivative (DVec3d::From (2,3,z), theta + 0.6 * Angle::Pi ());
        }
    printf ("RotMatrix::FromRotate90 MaxDiff %g\n", d);
    }
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP DMatrix4d RotateMoments
(
DMatrix4dCR baseWedgeIntegrals,
double theta0,
double theta1
);

GEOMDLLIMPEXP DMatrix4d RotateMoments_fast
(
DMatrix4dCR baseWedgeIntegrals,
double theta0,
double theta1
);
END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DMatrix4d, RotateMoments)
    {
    DMatrix4d wedgeMoments =
        {
        1,2,3,4,
        2,5,7,11,
        3,7,13,17,
        4,11,17,23
        };
    double theta0 = 1.0;
    double theta1 = 1.5;
    DMatrix4d solid1 = RotateMoments (wedgeMoments, theta0, theta1);
    DMatrix4d solid2 = RotateMoments_fast (wedgeMoments, theta0, theta1);
    Check::Near (solid1, solid2, "Rotated Moments");

    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (SolidPrimitive,RotationalSolidMoments)
    {
    bvector<DPoint3d> trianglePoints;
    double ax = 10;
    double bx = 11;
    double c = 1.0;
    trianglePoints.push_back (DPoint3d::From (ax, 0, 0));
    trianglePoints.push_back (DPoint3d::From (bx, 0, 0));
    trianglePoints.push_back (DPoint3d::From (ax, 0, c));
    trianglePoints.push_back (DPoint3d::From (ax, 0, 0));
    CurveVectorPtr triangle = CurveVector::CreateLinear (trianglePoints, CurveVector::BOUNDARY_TYPE_Outer, false);
    DRay3d rotationAxis;
    rotationAxis.InitFromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (0,0,1));
                        
    // Create chiseled frustum.  Its xx,xz,zz,x,z products should match
    // The chisel plane is x=y
    PolyfaceHeaderPtr facets = PolyfaceHeader::CreateVariableSizeIndexed ();
    facets->Point().push_back (DPoint3d::From ( ax, -ax/2.0, 0));
    facets->Point().push_back (DPoint3d::From ( bx, -bx/2.0, 0));
    facets->Point().push_back (DPoint3d::From ( ax, -ax/2.0, c));
    facets->Point().push_back (DPoint3d::From ( ax,  ax/2.0, 0));
    facets->Point().push_back (DPoint3d::From ( bx,  bx/2.0, 0));
    facets->Point().push_back (DPoint3d::From ( ax,  ax/2.0, c));
    int index[] =
        {
        1,2,3,0,
        6,5,4,0,
        1,3,6,4,0,
        5,6,3,2,0,
        5,2,1,4,0, 
        10000
        };
    for (int i = 0; abs (index[i]) <= (int)facets->Point ().size (); i++)
        facets->PointIndex ().push_back (index[i]);

    Check::StartScope ("Differential Moments");
    Check::True (facets->IsClosedByEdgePairing (), "Chisel block closed");
    DMatrix4d differentialProducts;
    RotMatrix facetProducts;
    double facetVolume;
    DVec3d facetMoment;
    Transform worldToRotation, rotationToWorld;
    Check::True (triangle->ComputeSecondMomentDifferentialAreaRotationProducts (
                rotationAxis, rotationToWorld, differentialProducts),
                "Compute differentials");
    worldToRotation.InverseOf (rotationToWorld);
    
    facets->SumTetrahedralMomentProducts (worldToRotation, facetVolume, facetMoment, facetProducts);
    DgnRotationalSweepDetail rotationalSweepData (triangle, rotationAxis.origin, rotationAxis.direction, Angle::TwoPi (), true);
    ISolidPrimitivePtr rotationalSweep = ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepData);
    CheckPrimitive (rotationalSweep, SolidPrimitiveType_DgnRotationalSweep);

    Check::EndScope ();
    }
    

void CheckBilinearPatch (DBilinearPatch3dCR patch)
    {
    DPoint3d xyz;
    DVec3d U, V;
    bvector<DPoint2d> uv;
    for (double u = 0.2; u < 1.0; u += 0.33)
        {
        for (double v = 0.1; v < 0.98; v += 0.25)
            {
            patch.Evaluate (u, v, xyz, U, V);
            DVec3d W = DVec3d::FromCrossProduct (U, V);
            DPoint3d spacePoint = DPoint3d::FromSumOf (xyz, W, 0.4);
            Check::True (patch.PerpendicularsOnBoundedPatch (spacePoint, uv), "Bilinear projection");
            if (Check::Size (1, uv.size (), "Simple bilinear projection count"))
                {
                Check::Near (u, uv[0].x, "u coordinate");
                Check::Near (v, uv[0].y, "v coordinate");
                }
            }    
         }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST (BilinearPatch, Projections)
    {
    DPoint3d xyz00 = DPoint3d::From (0,0,0);
    DPoint3d xyz10 = DPoint3d::From (1,0,0);
    DPoint3d xyz01 = DPoint3d::From (0,1,0);
    DPoint3d xyz11 = DPoint3d::From (1,1,0);


    Check::StartScope ("BilinearPatchUnitSquare");
    CheckBilinearPatch (
        DBilinearPatch3d (xyz00, xyz10, xyz01, xyz11));
    Check::EndScope ();

    Check::StartScope ("BilinearPatchRectangle");
    CheckBilinearPatch (
        DBilinearPatch3d (xyz00, xyz10, DPoint3d::From (0,2,0), DPoint3d::From (1, 2, 0)));
    Check::EndScope ();
    
    
    Check::StartScope ("BilinearPatchParallelogram");
    CheckBilinearPatch (
        DBilinearPatch3d (xyz00, xyz10, DPoint3d::From (1,2,0), DPoint3d::From (2, 2, 0)));
    Check::EndScope ();

    Check::StartScope ("BilinearPatchPlanarWarped");
    CheckBilinearPatch (
        DBilinearPatch3d (xyz00, xyz10, DPoint3d::From (1,2,0), DPoint3d::From (2.2, 2, 0)));
    Check::EndScope ();

    Check::StartScope ("BilinearPatchNonPlanar");
    CheckBilinearPatch (
        DBilinearPatch3d (xyz00, xyz10, DPoint3d::From (1,2,0), DPoint3d::From (2.2, 2, 0.1)));
    Check::EndScope ();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (SphereNormals, Test1)
    {
    static int s_dumpFaces = 0;
    DPoint3d origin = DPoint3d::From (0,0,0);
    DgnSphereDetail sphereData = DgnSphereDetail  (
                    origin,
                    DVec3d::From (1,0,0),
                    DVec3d::From (0,1,0),
                    3,3,
                    -Angle::PiOver2 (),
                    Angle::Pi (),
                    true);
    ISolidPrimitivePtr sphere = ISolidPrimitive::CreateDgnSphere (sphereData);
    CheckPrimitive (sphere, SolidPrimitiveType_DgnSphere);

    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetNormalsRequired (true);
    options->SetParamsRequired (true);
    options->SetAngleTolerance (Angle::DegreesToRadians (45.0));
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    if (!Check::True (builder->AddSolidPrimitive (*sphere), "Builder.AddSolidPrimitive"))
        return;
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (builder->GetClientMeshR (), true);
    // Sphere about origin should have normals parallel with vector to point....
    static double s_normalTol (1.0e-12);
    bvector <DPoint3d> const &points = visitor->Point ();
    bvector <DPoint2d> const &params = visitor->Param ();
    bvector <DVec3d> const &normals = visitor->Normal ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (size_t i = 0; i < visitor->NumEdgesThisFace (); i++)
            {
            DPoint3d xyz = points [i];
            DVec3d   normal = normals [i];
            DPoint2d param = params[i];
            if (s_dumpFaces)
                {
                Check::PrintIndent (2);
                Check::Print (param, "PhiTheta");
                Check::Print (xyz, "xyz");
                Check::Print (normal, "normal");
                }
                
            DVec3d   radialVector = DVec3d::FromStartEnd (origin, xyz);
            double dot = radialVector.DotProduct (normal);
            double theta = radialVector.SmallerUnorientedAngleTo (normal);
            Check::True (dot > 0.0, "normal outward");
            Check::True (fabs (theta) < s_normalTol, "normal angle");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,LinestringFaces)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,2,0));    
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,1,0));
    points.push_back (DPoint3d::From (0,2,0));
    DRay3d ray1 = DRay3d::FromOriginAndVector (DPoint3d::From (-1,0, 0.2), DVec3d::From (1,0,0));   // Grazes an edge
    DRay3d ray2 = DRay3d::FromOriginAndVector (DPoint3d::From (-1,0.2, 0.2), DVec3d::From (1,0,0)); // in and out
    CurveVectorPtr baseCurve = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnExtrusion (
                DgnExtrusionDetail (baseCurve, DVec3d::From (0,0,4), true));
    bvector <SolidLocationDetail> pickData;
    pickData.clear ();
    solid->AddRayIntersections (pickData, ray2, 0, -DBL_MAX);
    Check::Size (2, pickData.size (), "Multiple hits within swept linestring");
    pickData.clear ();
    solid->AddRayIntersections (pickData, ray1, 0, -DBL_MAX);
    Check::True (pickData.size () >= 1, "Edge pick hits at least once");         
    }    
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SelecteBlockIndices)
    {
    size_t numPartition = 15;
    size_t numPerPartition = 6;
    size_t partitionBase = 100;
    bvector<ptrdiff_t> indices;
    bvector<ptrdiff_t> representativeIndices;
    // Make some fixed size blocks, and remember one from each block ...
    for (size_t i = 0; i < numPartition; i++)
        {
        ptrdiff_t base = i * partitionBase;
        for (size_t j = 0; j < numPerPartition; j++)
            indices.push_back (base + j);
        indices.push_back (-1);
        representativeIndices.push_back (base);
        }

    // put some of the 
    bvector<ptrdiff_t> selectedIndices;
    size_t kStep = 2;
    for (size_t k = 0; k < numPartition; k += kStep)
        selectedIndices.push_back (representativeIndices[k]);
    
    bvector<ptrdiff_t> blockA, blockB;
    PolyfaceHeader::SelectBlockedIndices (indices, selectedIndices, true, blockA);
    PolyfaceHeader::SelectBlockedIndices (indices, selectedIndices, false, blockB);
    size_t numA = (numPerPartition + 1) * selectedIndices.size ();
    Check::Size (numA, blockA.size (), "selected indices TRUE");
    Check::Size (indices.size (), blockA.size () + blockB.size (), "selected indices total");
    bvector<ptrdiff_t> blockA1, blockA2, blockB1, blockB2;
    PolyfaceHeader::SelectBlockedIndices (blockA, selectedIndices, true, blockA1);
    PolyfaceHeader::SelectBlockedIndices (blockA, selectedIndices, false, blockA2);
    PolyfaceHeader::SelectBlockedIndices (blockB, selectedIndices, true, blockB1);
    PolyfaceHeader::SelectBlockedIndices (blockB, selectedIndices, false, blockB2);
    Check::Size (blockA.size (), blockA1.size (), "Reselect A true");
    Check::Size (0, blockA2.size (), "Reselect false");
    Check::Size (0, blockB1.size (), "Reselect B true");
    Check::Size (blockB.size (), blockB2.size (), "Reselect B false");
    
    }    

void testRotationalMoments (double cx, double cz, double ax, double az)
    {
    CurveVectorPtr disk = CurveVector::CreateDisk (
            DEllipse3d::FromVectors (DPoint3d::From (ax,0,az), DVec3d::From (0.5,0,0), DVec3d::From (0,0, 0.5),
                            0.0, Angle::TwoPi ()));
    DgnRotationalSweepDetail rotatedDetail (disk, DPoint3d::From (cx,0,cz), DVec3d::From (0,0,1), Angle::TwoPi (), true);
    DMatrix4d localProducts;
    Transform localToWorld;
    Check::True (rotatedDetail.ComputeSecondMomentVolumeProducts (localToWorld, localProducts), "compute products");
    DPoint3d expectedCentroid = DPoint3d::From (cx,0, az);   
    DVec3d centroid;
    RotMatrix axes;
    DVec3d moments;
    double volume;
    Check::True (localProducts.ConvertInertiaProductsToPrincipalMoments (localToWorld, volume, centroid, axes, moments), "Convert to moments");
    Check::Near (expectedCentroid, centroid, "centroid");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,RotateTorusCentroidInMoments)
    {
    testRotationalMoments (0,0,    3,0);
    testRotationalMoments (0,0,    3,7);
    testRotationalMoments (0,0,    3,13);

    testRotationalMoments (0,2,    3,0);
    testRotationalMoments (0,2,    3,7);
    testRotationalMoments (0,2,    3,13);

    testRotationalMoments (1,0,    3,0);
    testRotationalMoments (1,0,    3,7);
    testRotationalMoments (1,0,    3,13);

    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,DiskNormal)
    {
    for (double s = -1.0; s < 2.0; s += 2.0)
        {
        DEllipse3d ellipse = DEllipse3d::From (0,0,0, 1,0,0, 0,1,0, 0.0, s * Angle::TwoPi ());
        DPoint3d centroid;
        DVec3d normal;
        double area;
        CurveVectorPtr disk = CurveVector::CreateDisk (ellipse);
        disk->CentroidNormalArea (centroid, normal, area);

        }

    }

#endif
    

static double s_oneThird = 1.0 / 3.0;

// Methods to compute volumes between (a) fragments of triangles split by a cutPlane and (b) a projectionPlane.
// If the projection plane is the same as the cut plane, these are the volumes between the fragments and the cut plane.
// If the projection plane is perpendicular to the cut plane, the sum of all such fragmetary volumes from all triangles of a closed volume mesh
//   are the portions of the mesh volume above and below the cut plane.
//   It is important that the projection plane be perpendicular to the cut plane for this overall volume split to be correct. 
//    (Really?  The missing cut faces are on the projection plane -- they would have zero contribution to the sweeps.)
struct CutVolumeContext
{
private:
DPlane3d m_cutPlane;          // Unit normal !!!
DPlane3d m_projectionPlane;   // Unit normal !!!
DPoint3d Centroid (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
    {
    DPoint3d xyz = {
        s_oneThird * (point0.x + point1.x + point2.x),
        s_oneThird * (point0.y + point1.y + point2.y),
        s_oneThird * (point0.z + point1.z + point2.z)
        };
    return xyz;
    }
public:
CutVolumeContext (DPlane3d cutPlane, DPlane3d projectionPlane) : m_cutPlane (cutPlane), m_projectionPlane (projectionPlane){}

// Compute projected volumes with prepared data:
// point0, altitude0 = point and altitude on "A" side.  Known nonzero.
// point1, altitude1, point2, altitude2 = point and altitude on the "B" side.
// (return) volumeA = volume on the A side
// (return) volumeB = volume on the B side
void SplitTriangleProjectedVolumes (DPoint3dCR point0, double altitude0, DPoint3dCR point1, double altitude1, DPoint3dCR point2, double altitude2, double &volumeA, double &volumeB)
    {
    double fraction01 = (-altitude0) / (altitude1 - altitude0);  // caller guarantees denominator is nonzero !!!
    double fraction20 = (-altitude2) / (altitude0 - altitude2);  // caller guarantees denominator is nonzero !!!
    DPoint3d pointC = DPoint3d::FromInterpolate (point0, fraction01, point1);
    DPoint3d pointD = DPoint3d::FromInterpolate (point2, fraction20, point0);
    double volume012 = ProjectedVolume (point0, point1, point2);
    double volume0CD = ProjectedVolume (point0, pointC, pointD);
    volumeA = volume0CD;
    volumeB = volume012 - volume0CD;
    }
public:

    static CutVolumeContext FromSplitPlaneAndArbitraryPerpendicular (DPlane3dCR plane)
        {
        DVec3d unitX, unitY, unitZ;
        plane.normal.GetNormalizedTriad (unitX, unitY, unitZ);
        DPlane3d cutPlane = DPlane3d::FromOriginAndNormal (plane.origin, unitZ);
        DPlane3d projectionPlane = DPlane3d::FromOriginAndNormal (plane.origin, unitX);
        return CutVolumeContext (cutPlane, projectionPlane);
        }
    bool ValidateNormals ()
        {
        return m_cutPlane.normal.Normalize () != 0.0 && m_projectionPlane.normal.Normalize ();
        }
    // Compute the (signed !!!) volume between this triangle and the projection plane.
    // The sign has two orientation flips:
    // For the same triangle, translation above and below the plane reverses sign.
    // For the same triangle, reversing point order reverses sign.
    // (A triangle with centroid above and pointing up has positive volume.  Above and pointing down is negative.  Below and down is positive. Below and up is negative.)
    double ProjectedVolume (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
        {
        DPoint3d centroid = Centroid (point0, point1, point2);
        DVec3d normal = DVec3d::FromCrossProductToPoints (point0, point1, point2);
        double projectedArea = 0.5 * normal.DotProduct (m_projectionPlane.normal);
        double centroidAltitude = m_projectionPlane.Evaluate (centroid);
        return projectedArea * centroidAltitude;
        }        
    // SPlit the triangle by the cut plane.
    // Compute the volume when the parts above and below are swept to the projection plane.
    void ComputeSplitTriangleProjectedVolumes (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, double &volumeBelow, double &volumeAbove)
        {
        double altitudes[6];
        altitudes[0] = m_cutPlane.Evaluate (point0);
        altitudes[1] = m_cutPlane.Evaluate (point1);
        altitudes[2] = m_cutPlane.Evaluate (point2);
        altitudes[3] = altitudes[0];      // for easy wraparound
        altitudes[4] = altitudes[1];
        altitudes[5] = altitudes[2];
        DPoint3d points[6] = {point0, point1, point2, point0, point1, point2};
        int numBelow = 0;
        int numAbove = 0;
        int lastBelow = -1;
        int lastAbove = -1;
        for (int i = 0; i < 3; i++)
            {
            if (altitudes[i] < 0.0)
                {
                numBelow++;
                lastBelow = i;
                }
            else
                {
                numAbove++;
                lastAbove = i;
                }
            }
        if (numBelow == 1)
            {
            SplitTriangleProjectedVolumes (points[lastBelow], altitudes[lastBelow], points[lastBelow+1], altitudes[lastBelow+1], points[lastBelow+2], altitudes[lastBelow+2], volumeBelow, volumeAbove);
            }
        else if (numAbove == 1)
            {
            SplitTriangleProjectedVolumes (points[lastAbove], altitudes[lastAbove], points[lastAbove+1], altitudes[lastAbove+1], points[lastAbove+2], altitudes[lastAbove+2], volumeAbove, volumeBelow);
            }
        else if (numBelow == 3)
            {
            volumeBelow = ProjectedVolume (point0, point1, point2);
            volumeAbove = 0.0;
            }
        else // numBelow == 0
            {
            volumeAbove = ProjectedVolume (point0, point1, point2);
            volumeBelow = 0.0;
            }
        }        

    void ComputeSplitPolygonProjectedVolumesWithWrapEdge (bvector<DPoint3d> const &points, double &volumeAbove, double &volumeBelow)
        {
        double vAbove, vBelow;
        volumeAbove = 0.0;
        volumeBelow = 0.0;
        for (size_t i = 2; i < points.size (); i++)
            {
            ComputeSplitTriangleProjectedVolumes (points[0], points[i-1], points[i], vAbove, vBelow);
            volumeAbove += vAbove;
            volumeBelow += vBelow;
            }
        }

};
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SplitTriangle)
    {
    double splitZ = 5.0;
    DPlane3d splitPlane = DPlane3d::FromOriginAndNormal (0,0,splitZ,  0,0, 1);
    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal (0,0,0, 0,0,1);
    CutVolumeContext perpendicularContext = CutVolumeContext::FromSplitPlaneAndArbitraryPerpendicular (splitPlane);
    CutVolumeContext parallelContext (splitPlane, xyPlane);
    if (Check::True (perpendicularContext.ValidateNormals ())
        && Check::True (parallelContext.ValidateNormals ())
        )
        {
        double vAbove, vBelow;
        for (double z = 0.0; z < 10.0; z+= 3.0)
            {
            // triangles parallel to the plane ... should get 0 volume to perpendicular plane, simple sweep to parallel
            DPoint3d points[3] = {DPoint3d::From (0,0,z), DPoint3d::From (1,0,z), DPoint3d::From (0,1,z)};
            double area = DVec3d::FromCrossProductToPoints (points[0], points[1], points[2]).Magnitude () * 0.5;
            double altitude = z;
            perpendicularContext.ComputeSplitTriangleProjectedVolumes (points[0], points[1], points[2], vBelow, vAbove);
            Check::Near (0.0, vBelow);
            Check::Near (0.0, vAbove);
            
            parallelContext.ComputeSplitTriangleProjectedVolumes (points[0], points[1], points[2], vBelow, vAbove);
            if (z < splitZ)
                {
                Check::Near (altitude * area, vBelow);
                Check::Near (0.0, vAbove);
                }
            else
                {
                Check::Near (altitude * area, vAbove);
                Check::Near (0.0, vBelow);
                }
            }

        // tilted triangle
        CutVolumeContext splitToXYContext (splitPlane, xyPlane);
        bvector<double>below, above, sum;
        double a = 10.0;
        double hz = 4.0;
        double dz = 1.0;
        for (double z = 0.0; z < 10.0; z+= dz)
            {
            DPoint3d points[3] = {DPoint3d::From (a,0,z), DPoint3d::From (a,2*a,z), DPoint3d::From (0,a,z+hz)};
            splitToXYContext.ComputeSplitTriangleProjectedVolumes (points[0], points[1], points[2], vBelow, vAbove);
            below.push_back (vBelow);
            above.push_back (vAbove);
            sum.push_back (vAbove + vBelow);
            }
        double areaXY = a * a;
        double slabVolume = areaXY * dz;
        for (size_t i = 1; i < above.size (); i++)
            {
            Check::Near (slabVolume, sum[i] - sum[i-1]);
            }
        }
    }    


void ComputeSplitVolumes (PolyfaceQueryCR polyface, bvector<DPlane3d> &splitPlanes, bvector<DPoint2d> &volumes)
    {
    bvector<CutVolumeContext> splitters;
    for (DPlane3dCR plane : splitPlanes)
        {
        CutVolumeContext context (plane, plane);
        context.ValidateNormals ();
        splitters.push_back (context);
        volumes.push_back (DPoint2d::From (0,0));
        }

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (polyface);
    bvector<DPoint3d> &facetPoints = visitor->Point ();
    double vBelow, vAbove;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (size_t i = 0; i < splitters.size (); i++)
            {
            splitters[i].ComputeSplitPolygonProjectedVolumesWithWrapEdge (facetPoints, vBelow, vAbove);
            volumes[i].x += vBelow;
            volumes[i].y += vAbove;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface,SplitVolumesA)
    {
    IFacetOptionsPtr options = IFacetOptions::Create ();

    for (size_t numGon = 4; numGon < 65; numGon *= 2)
        {
        IPolyfaceConstructionPtr builder =  IPolyfaceConstruction::Create (*options);
        builder->AddSweptNGon (numGon, 1.0, 0.0, 2.0, true, true);
        PolyfaceHeaderPtr mesh = builder->GetClientMeshPtr ();
        double volume = mesh->SumTetrahedralVolumes (DPoint3d::From (2,3,4));
        bvector<DPlane3d> planes;
        planes.push_back (DPlane3d::FromOriginAndNormal (0,0,0, 0,0,1));
        planes.push_back (DPlane3d::FromOriginAndNormal (0,0,1, 0,0,1));
        planes.push_back (DPlane3d::FromOriginAndNormal (0,0,1, 0,1,0));
        planes.push_back (DPlane3d::FromOriginAndNormal (0.5,0,1, 1,1,0));
        planes.push_back (DPlane3d::FromOriginAndNormal (0.5,0,0.234, 1,1,0.3));
        DVec3d normal = DVec3d::From (1,2,3);
        normal.Normalize ();
        DPoint3d origin = DPoint3d::From (-2,-2,-2);
        static double originStep = 1.0;
        for (size_t i = 0; i < 10; i++)
            {
            planes.push_back (DPlane3d::FromOriginAndNormal (origin, normal));
            origin = DPoint3d::FromSumOf (origin, normal, originStep);
            }
        bvector<DPoint2d> volumes;
        ComputeSplitVolumes (*mesh, planes, volumes);
        for (size_t i = 0; i < volumes.size (); i++)
            {
            Check::Near (volume, volumes[i].x + volumes[i].y);
            }
        }
    }


void TestTwoSidedMeshConstruction (IGeometry& g)
    {
    static double shiftFactor = 1.5;
    ISolidPrimitivePtr s = g.GetAsISolidPrimitive ();
    if (s.IsValid ())
        {
        auto baseTransform = Check::GetTransform ();
        Check::SaveTransformed (&g);
        DRange3d range;
        if (g.TryGetRange (range))
            Check::Shift (shiftFactor * range.XLength (), 0, 0);

        IFacetOptionsPtr options = IFacetOptions::Create ();
        options->SetParamsRequired (true);
        options->SetNormalsRequired (true);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);

        if (Check::True (builder->AddSolidPrimitive (*s), "Builder.AddSolidPrimitive"))
            {
            PolyfaceHeaderPtr header = builder->GetClientMeshPtr ();
            if (header.IsValid ())
                Check::SaveTransformed (*header);
            if (Check::True (header.IsValid ()) && s->GetCapped ())
                {
                // We expect a capped primitive to generate a closed, onesided mesh.
                Check::True (header->IsClosedByEdgePairing ());
                double volume;
                DPoint3d centroid;
                DVec3d momentxyz;
                RotMatrix axes;
                Check::True (header->ComputePrincipalMomentsAllowMissingSideFacets (volume, centroid, axes, momentxyz, false),
                      "valid volume of capped primitive");
                Check::True (volume > 0.0, "Mesher should produce postive volumes");
                Check::False (header->GetTwoSided (), "Mesher should produce 1-sided mesh on closed solid primitive");
                }
            }
        Check::SetTransform (baseTransform);
        Check::Shift (0.0, shiftFactor * range.YLength (), 0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TwoSidedMesh,Test1)
    {
    Check::QuietFailureScope scoper;

    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSimplestSolidPrimitives (geometry, true);
    SampleGeometryCreator::AddAllSolidTypes (geometry);
    for (size_t i = 0; i < geometry.size (); i++)
        TestTwoSidedMeshConstruction (*geometry[i]);
    Check::ClearGeometry ("SolidPrimitive.TwoSidedMesh");
    }

void SaveFacets(ISolidPrimitivePtr &primitive)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetNormalsRequired (true);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->AddSolidPrimitive(*primitive);
    Check::SaveTransformed(*builder->GetClientMeshPtr());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, MirrorTransforms)
    {
    Check::QuietFailureScope scoper;

    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSimplestSolidPrimitives(geometry, true);
    SampleGeometryCreator::AddAllSolidTypes(geometry);
#ifdef TEST1SPHERE
    geometry.clear ();
    geometry.push_back (IGeometry::Create(ISolidPrimitive::CreateDgnSphere(
        DgnSphereDetail(DPoint3d::From(0, 0, 0),
        DVec3d::From (1,0,0),
        DVec3d::From (0,0,1),
        1.0, 1.0,
        0, 0.7, false))));
#endif
    auto mirrorYMatrix = RotMatrix::FromRowValues(1, 0, 0, 0, -1, 0, 0, 0, 1);
    auto mirrorXMatrix = RotMatrix::FromRowValues(-1, 0, 0, 0, 1, 0, 0, 0, 1);
    auto mirrorZMatrix = RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, -1);
    auto vectorQ = DVec3d::From (1,0.4,0.5);
    vectorQ.Normalize ();
    for (size_t i = 0; i < geometry.size(); i++)
        {
        auto g = geometry[i]->GetAsISolidPrimitive();
        if (g.IsValid ())
            {
            DRange3d range;
            g->GetRange(range);
            auto dy = range.high.y - range.low.y;
            SaveAndRestoreCheckTransform shifter(5.0 * (range.high.x - range.low.x), 0, 0);
            auto mirrorY = Transform::FromMatrixAndFixedPoint(mirrorYMatrix, range.high);
            auto mirrorZ = Transform::FromMatrixAndFixedPoint(mirrorZMatrix, range.high);
            auto mirrorX = Transform::FromMatrixAndFixedPoint(mirrorXMatrix, range.high);
            Check::SaveTransformed (*g);

            auto gMirrorY = g->Clone ();
            gMirrorY->TransformInPlace (mirrorY);
            Check::SaveTransformed(*gMirrorY);
            auto rotateX = Transform::FromMatrixAndFixedPoint(
                    RotMatrix::FromVectorAndRotationAngle (
                                DVec3d::From (1,0,0),
                                Angle::DegreesToRadians (-80)),
                    range.low);

            auto gMirrorX = g->Clone ();
            gMirrorX->TransformInPlace(mirrorX);
            Check::SaveTransformed(*gMirrorX);

            auto gMirrorZ = g->Clone();
            gMirrorZ->TransformInPlace(mirrorZ);
            Check::SaveTransformed(*gMirrorZ);

            Transform mirrorQ;
            mirrorQ.InitFromMirrorPlane(range.high, vectorQ);
            auto gMirrorQ = g->Clone();
            gMirrorQ->TransformInPlace(mirrorQ);
            Check::SaveTransformed(*gMirrorQ);


            auto gRotate = g->Clone ();
            gRotate->TransformInPlace (rotateX);
            Check::SaveTransformed(*gRotate);
            Check::Shift (0, 4.0 * dy, 0);
            SaveFacets(g);
            SaveFacets(gMirrorY);
            SaveFacets(gMirrorX);
            SaveFacets(gMirrorZ);
            SaveFacets(gRotate);
            SaveFacets (gMirrorQ);
            }
        }
    Check::ClearGeometry("SolidPrimitive.MirrorTransforms");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TwoSidedMesh,Test2)
    {
    Check::QuietFailureScope scoper;

    bvector<IGeometryPtr> basePaths;
    SampleGeometryCreator::AddXYOpenPaths (basePaths);
    }

void CreateAllSolidsAroundOrigin (bvector<ISolidPrimitivePtr> &solids, bool doVariations)
    {
    solids.clear ();
    double zA = -1.0;
    double zB = 1.0;
    double dz = zB - zA;
    double rA = 2.0;
    double rB = 1.0;
    static double s_zPipeCenter = 0.0;


    solids.push_back(
        ISolidPrimitive::CreateDgnCone (
            DgnConeDetail (
            DPoint3d::From (zA,0,zA),
            DPoint3d::From (zB, 0,0),
            rA, rB, true)));

    solids.push_back (ISolidPrimitive::CreateDgnTorusPipe (DgnTorusPipeDetail (
                DPoint3d::From (0,0,s_zPipeCenter),
                DVec3d::From (1,0,0),
                DVec3d::From (0,1,0),
                0.75 * rA,
                0.25 * rA,
                Angle::DegreesToRadians (30.0),//Angle::TwoPi (),
                false
                )));

    solids.push_back (ISolidPrimitive::CreateDgnTorusPipe (DgnTorusPipeDetail (
                DPoint3d::From (0,0,0.0),
                DVec3d::From (1,0,0),
                DVec3d::From (0,1,0),
                0.75 * rA,
                0.25 * rA,
                Angle::DegreesToRadians (30.0),//Angle::TwoPi (),
                false
                )));
    bvector<DPoint3d> uPoints
        {
        DPoint3d::From (-1,-1,zA),
        DPoint3d::From (-1, 1,zA),
        DPoint3d::From ( 1, 1,zA)
        };

    auto pathA = CurveVector::CreateLinear (uPoints);
    pathA->push_back (ICurvePrimitive::CreateLine (
                    DSegment3d::From (DPoint3d::From (1,1,zA), DPoint3d::From (1,-1,zA))));

    DgnExtrusionDetail extrusionData (
                pathA,
                DVec3d::From (0,0,dz),
                false
                );

    auto extrusion = ISolidPrimitive::CreateDgnExtrusion (extrusionData);
    solids.push_back (extrusion); 


    solids.push_back(
        ISolidPrimitive::CreateDgnCone (
            DgnConeDetail (
            DPoint3d::From (0,0,zA),
            DPoint3d::From (0,0,zB),
            rA, rB, true)));

    if (doVariations)
        {
        solids.push_back(
            ISolidPrimitive::CreateDgnCone (
                DgnConeDetail (
                DPoint3d::From (0,0,zA),
                DPoint3d::From (0,0,zB),
                rA, 0.0, true)));

        solids.push_back(
            ISolidPrimitive::CreateDgnCone (
                DgnConeDetail (
                DPoint3d::From (0,0,zA),
                DPoint3d::From (0,0,zB),
                0.0, rB, true)));
        }
    
    solids.push_back (ISolidPrimitive::CreateDgnSphere (DgnSphereDetail (DPoint3d::From (0,0,0), rA)));

    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (1,0,-2,    1,0,2));
    auto baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    baseCurve->push_back (lineA);
    solids.push_back (ISolidPrimitive::CreateDgnRotationalSweep (
       DgnRotationalSweepDetail (
            baseCurve,
            DPoint3d::From (0,0,0),
            DVec3d::From (0,0,1),
            Angle::DegreesToRadians (270.0),
            false
            )));

    solids.push_back (ISolidPrimitive::CreateDgnBox (
       DgnBoxDetail::InitFromCenterAndSize (
            DPoint3d::From (0,0,0),
            DPoint3d::From (2,2,3),
            false
            )));
   

    }


#if defined (_WIN32) && !defined(BENTLEY_WINRT)

void CheckMessages (MeshAnnotationVector &messages, ISolidPrimitivePtr &solid, ICurvePrimitivePtr curve)
    {
    if (messages.size () > 0 || messages.GetTotalFail () > 0)
        {
        printf ("\nMessages");
        for (auto &message : messages)
            {
            Check::PrintIndent (2);
            printf ("%s", message.m_description.c_str());
            Check::Print ((int)message.m_fail, "failures");
            }
        Check::PrintStructure (curve.get ());
        Check::PrintStructure (solid.get ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, LineSegmentIntersection)
    {
    auto segment = ICurvePrimitive::CreateLine (DSegment3d::From (-4,0,0, 2,0,0));
    bvector<ISolidPrimitivePtr> solids;
    CreateAllSolidsAroundOrigin (solids, false);
    for (ISolidPrimitivePtr &solid : solids)
        {
        bvector<CurveLocationDetail> curvePoints;
        bvector<SolidLocationDetail> solidPoints;
        MeshAnnotationVector messages (false);
        solid->AddCurveIntersections (*segment, curvePoints, solidPoints, messages);
        CheckMessages (messages, solid, segment);
        CheckConsistency (curvePoints, solidPoints);
        CheckConsistency (*solid, solidPoints);
    // What's right?  Enforce "one or the other"
        Check::True ((messages.GetTotalFail () > 0) != (curvePoints.size () > 0), "AppendCurveIntersections");
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, LineStringIntersection)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (-4,0.5,0),
        DPoint3d::From (-0.5,0.5,0),
        DPoint3d::From (2,0.5,0)
        };
    auto segment = ICurvePrimitive::CreateLineString (points);
    bvector<ISolidPrimitivePtr> solids;
    CreateAllSolidsAroundOrigin (solids, false);
    for (ISolidPrimitivePtr &solid : solids)
        {
        bvector<CurveLocationDetail> curvePoints;
        bvector<SolidLocationDetail> solidPoints;
        MeshAnnotationVector messages (false);
        solid->AddCurveIntersections (*segment, curvePoints, solidPoints, messages);
        CheckMessages (messages, solid, segment);
        CheckConsistency (curvePoints, solidPoints);
        CheckConsistency (*solid, solidPoints);
    // What's right?  Enforce "one or the other"
        Check::True ((messages.GetTotalFail () > 0) != (curvePoints.size () > 0), "AppendCurveIntersections");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, ArcIntersection)
    {
    bvector<ISolidPrimitivePtr> solids;
    CreateAllSolidsAroundOrigin (solids, false);
    for (ISolidPrimitivePtr &solid : solids)
        {
        for (double dz0 : bvector<double> {0,0.2, 1})
            {
            auto segment = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0,    4,0,dz0, 0,0.1,0,   0.0, Angle::TwoPi ()));
            bvector<CurveLocationDetail> curvePoints;
            bvector<SolidLocationDetail> solidPoints;
            MeshAnnotationVector messages (false);
            solid->AddCurveIntersections (*segment, curvePoints, solidPoints, messages);
            CheckMessages (messages, solid, segment);
            CheckConsistency (curvePoints, solidPoints);
            CheckConsistency (*solid, solidPoints);
                    // What's right?  Enforce "one or the other"
            Check::True ((messages.GetTotalFail () > 0) != (curvePoints.size () > 0), "AppendCurveIntersections");
            }
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive, BCurveIntersection)
    {
    bvector<DPoint3d> poles
        {
        DPoint3d::From (0,2.5,0),
        DPoint3d::From (0,0.5,0),
        DPoint3d::From (0.5,0.5,0),
        DPoint3d::From (2.5,0,0),
        };

    auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, 3, false, true);
    auto segment = ICurvePrimitive::CreateBsplineCurve (bcurve);

    bvector<ISolidPrimitivePtr> solids;
    CreateAllSolidsAroundOrigin (solids, false);
    for (ISolidPrimitivePtr &solid : solids)
        {
        bvector<CurveLocationDetail> curvePoints;
        bvector<SolidLocationDetail> solidPoints;
        MeshAnnotationVector messages (false);
        solid->AddCurveIntersections (*segment, curvePoints, solidPoints, messages);
        CheckMessages (messages, solid, segment);
        CheckConsistency (curvePoints, solidPoints);
        CheckConsistency (*solid, solidPoints);
    // What's right?  Enforce "one or the other"
        Check::True ((messages.GetTotalFail () > 0) != (curvePoints.size () > 0), "AppendCurveIntersections");
        }
    }    
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,CurveVectorIntersection)
    {
    auto segment = ICurvePrimitive::CreateLine (DSegment3d::From (-2,0,0, 2,0,0));
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    cv->push_back (segment);
    bvector<ISolidPrimitivePtr> solids;
    CreateAllSolidsAroundOrigin (solids, false);
    for (ISolidPrimitivePtr &solid : solids)
        {
        bvector<CurveLocationDetail> curvePoints;
        bvector<SolidLocationDetail> solidPoints;
        MeshAnnotationVector messages (false);
        solid->AddCurveIntersections (*cv, curvePoints, solidPoints, messages);
        CheckConsistency (curvePoints, solidPoints);
        CheckConsistency (*solid, solidPoints);
        // What's right?  Enforce "one or the other"
        Check::True ((messages.GetTotalFail () > 0) != (curvePoints.size () > 0), "AppendCurveIntersections");
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,IntersectRotatedCurveLine)
    {
    auto spaceCurve = ICurvePrimitive::CreateLine (DSegment3d::From (1,0,0, 3,0,3));
    auto contour = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    auto cylinderSide = ICurvePrimitive::CreateLine (DSegment3d::From (2,0,0, 2,0,5));
    contour->push_back (cylinderSide);
    Transform worldToLocal = Transform::FromIdentity ();
    bvector<CurveLocationDetail> intersectionA, intersectionB;
    CurveCurve::IntersectRotatedCurveSpaceCurve (worldToLocal, *contour, *spaceCurve, intersectionA, intersectionB);
    Check::True (intersectionA.size () == 1 && intersectionB.size () == 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,IntersectRotatedCurveArc)
    {
    auto spaceCurve = ICurvePrimitive::CreateArc (DEllipse3d::From (1,0,0,    2,0,0,  0,0,2, 0.0, Angle::DegreesToRadians (90.0)));
    auto contour = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    auto cylinderSide = ICurvePrimitive::CreateLine (DSegment3d::From (2,0,0, 2,0,5.4));
    contour->push_back (cylinderSide);
    Transform worldToLocal = Transform::FromIdentity ();
    bvector<CurveLocationDetail> intersectionA, intersectionB;
    CurveCurve::IntersectRotatedCurveSpaceCurve (worldToLocal, *contour, *spaceCurve, intersectionA, intersectionB);
    Check::True (intersectionA.size () == 1 && intersectionB.size () == 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polynomials,Torus)
    {
    Polynomial::Implicit::Torus torus (1.5, 0.5, DgnTorusPipeDetail::GetReverseVector90 ());
    for (double theta : bvector<double>{0, 0.1})
        {
        Check::StartScope ("theta", theta);
        for (double phi : bvector<double>{0, 0.1})
            {
            Check::StartScope ("phi", phi);
            DPoint3d xyz1;
            double theta1, phi1, distance1, rho1;
            DPoint3d xyz = torus.EvaluateThetaPhi (theta, phi);
            torus.XYZToThetaPhiDistance (xyz, theta1, phi1, distance1, rho1);
            Check::Near (phi, phi1, "phi roundtrip");
            Check::Near (theta, theta1, "theta roundtrip");
            xyz1 = torus.EvaluateThetaPhi (theta1, phi1);
            Check::Near (xyz, xyz1, "xyz round trip");
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }

bool ShowFrame (ISolidPrimitivePtr &solid, SolidLocationDetail::FaceIndices &indices, double u, double v, double frameScale = 0.10)
    {
    DPoint3d X;
    DVec3d dXdu, dXdv;
    if (Check::True (solid->TryUVFractionToXYZ (indices, u, v, X, dXdu, dXdv)))
        {
        DVec3d normal;
        normal.GeometricMeanCrossProduct (dXdu, dXdv);
        bvector<DPoint3d> points
            {
            X + frameScale * dXdv,
            X,
            X + frameScale * dXdu,
            X + (0.1 * frameScale * dXdv),
            X,
            X + frameScale * normal
            };
        Check::SaveTransformed (points);
        return true;
        }
    return false;
    }

void TestFaceUV (IGeometryPtr const& geometry)
    {
    auto solid = geometry->GetAsISolidPrimitive ();
    if (!solid.IsValid ())
        return;
    Check::SaveTransformed (*solid);
    bvector<DPoint2d> allUV
        {
        DPoint2d::From (0.25, 0.25),
        DPoint2d::From (0.5, 0.25),
        DPoint2d::From (0.25, 0.5),
        DPoint2d::From (0.5, 0.5)
        };
    bvector<SolidLocationDetail::FaceIndices> allFaces;
    solid->GetFaceIndices (allFaces);
    for (auto &f : allFaces)
        {
        for (auto uv : allUV)
            {
            ShowFrame (solid, f, uv.x, uv.y);
            }
        }
    }

bool IsSameTopType (IGeometryPtr &g0, IGeometryPtr &g1)
    {
    if (g0.IsNull () || g1.IsNull ())
        return false;
    if (g0->GetGeometryType () != g1->GetGeometryType ())
        return false;
    if (g0->GetGeometryType () == IGeometry::GeometryType::SolidPrimitive)
        {
        auto s0 = g0->GetAsISolidPrimitive ();
        auto s1 = g1->GetAsISolidPrimitive ();
        if (s0.IsValid () && s1.IsValid ())
            return s0->GetSolidPrimitiveType () == s1->GetSolidPrimitiveType ();
        }
    return true;
    }


void ShowFrames (bvector<IGeometryPtr> &geometry)
    {
    auto baseFrame = Check::GetTransform ();
    DRange3d range0;
    range0.Init ();
    for (size_t i = 0; i < geometry.size (); i++)
        {
        IGeometryPtr g = geometry[i];
        DRange3d range;
        if (g->TryGetRange (range))
            {
            SaveAndRestoreCheckTransform shifter (3.0 * range.XLength (), 0,0);
            range0.Extend (range.low);
            range0.Extend (range.high);
            TestFaceUV (g);
            }
        }
    Check::SetTransform (baseFrame);
    Check::Shift (0, 3.0 * range0.YLength (), 0);
    }

void ShowFacets (bvector<IGeometryPtr> &geometry)
    {
    auto baseFrame = Check::GetTransform ();
    DRange3d range0;
    range0.Init ();
    for (size_t i = 0; i < geometry.size (); i++)
        {
        IGeometryPtr g = geometry[i];
        DRange3d range;
        ISolidPrimitivePtr primitive = g->GetAsISolidPrimitive ();
        if (primitive.IsValid () && g->TryGetRange (range) )
            {
            SaveAndRestoreCheckTransform shifter (4.0 * range.XLength (), 0,0);
            range0.Extend (range.low);
            range0.Extend (range.high);
            PolyfaceHeaderPtr facets = PolyfaceHeader::CreateVariableSizeIndexed ();
            IFacetOptionsPtr options = IFacetOptions::Create ();
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            builder->AddSolidPrimitive (*primitive);
            Check::SaveTransformed (*primitive);            
            Check::Shift (1.5 * range.XLength (), 0,0);
            Check::SaveTransformed (*builder->GetClientMeshPtr ());
            }
        }
    Check::SetTransform (baseFrame);
    Check::Shift (0, 3.0 * range0.YLength (), 0);
    }

void ShowEdgeChains (bvector<IGeometryPtr> &geometry)
    {
    auto baseFrame = Check::GetTransform ();
    DRange3d range0;
    range0.Init ();
    double shift = 10.0;
    for (size_t i = 0; i < geometry.size (); i++)
        {
        IGeometryPtr g = geometry[i];
        DRange3d range;
        ISolidPrimitivePtr primitive = g->GetAsISolidPrimitive ();
        if (primitive.IsValid () && g->TryGetRange (range) )
            {
            SaveAndRestoreCheckTransform shifter (4.0 * DoubleOps::Max (range.XLength (), shift), 0,0);
            range0.Extend (range.low);
            range0.Extend (range.high);
            IFacetOptionsPtr options = IFacetOptions::Create ();
            options->SetEdgeChainsRequired (true);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            builder->AddSolidPrimitive (*primitive);
            Check::SaveTransformed (*primitive);            
            Check::Shift (1.5 * DoubleOps::Max (range.XLength (), shift), 0,0);
            auto facets = builder->GetClientMeshPtr ();
            Check::SaveTransformed (*facets);
            Check::Shift (0.0, 1.5 * DoubleOps::Max (range.YLength (), shift), 0);
            SaveEdgeChains (*facets, true);
            }
        }
    Check::SetTransform (baseFrame);
    Check::Shift (0, 5.0 * DoubleOps::Max (range0.YLength (), shift), 0);
    }


void AnnounceAllSolids (void (*function)(bvector<IGeometryPtr> &))
    {
    Check::QuietFailureScope scoper;
    //SampleGeometryCreator::AddAllSolidTypes (geometry);

    bvector<IGeometryPtr> geometry;
    for (bool capped : bvector<bool>{false, true})
        {
        SampleGeometryCreator::AddAllCones (geometry, capped);
        function (geometry);
        geometry.clear ();

        SampleGeometryCreator::AddSpheres (geometry, capped);
        function (geometry);
        geometry.clear ();

        SampleGeometryCreator::AddExtrusions (geometry, capped);
        function (geometry);
        geometry.clear ();
 
        SampleGeometryCreator::AddRotations (geometry, capped);
        function (geometry);
        geometry.clear ();

        SampleGeometryCreator::AddBoxes (geometry, 3,2,1, capped);
        function (geometry);
        geometry.clear ();

        SampleGeometryCreator::AddTorusPipes (geometry, 3, 1, capped);
        function (geometry);
        geometry.clear ();

        SampleGeometryCreator::AddRuled (geometry, capped);
        function (geometry);
        geometry.clear ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,FaceUV)
    {
    Check::QuietFailureScope scoper;
    //SampleGeometryCreator::AddAllSolidTypes (geometry);

    AnnounceAllSolids (ShowFrames);
    Check::ClearGeometry ("SolidPrimitive.FaceUV");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,Facets)
    {
    Check::QuietFailureScope scoper;
    //SampleGeometryCreator::AddAllSolidTypes (geometry);

    AnnounceAllSolids (ShowFacets);
    Check::ClearGeometry ("SolidPrimitive.Facets");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,EdgeChains)
    {
    Check::QuietFailureScope scoper;
    //SampleGeometryCreator::AddAllSolidTypes (geometry);

    AnnounceAllSolids (ShowEdgeChains);
    Check::ClearGeometry ("SolidPrimitive.EdgeChains");

    }

void showEyePoint (DPoint4dCR eyePoint)
    {
    static double s_vectorScale = 20.0;
    auto w = eyePoint.w;
    if (w == 0.0)
        Check::SaveTransformed (DSegment3d::From (0,0,0, s_vectorScale * eyePoint.x, s_vectorScale * eyePoint.y, s_vectorScale * eyePoint.z));
    else
        {

        Check::SaveTransformed (DSegment3d::From (0,0,0, eyePoint.x / w, eyePoint.y / w , eyePoint.z / w));
        }
    }

void testSilhouette (IGeometryPtr &g, DPoint4dCR eyePoint)
    {
    static bool s_doFlat = true;
    static bool s_doEye = true;
    ISolidPrimitivePtr s = g->GetAsISolidPrimitive ();
    DPoint4d eyePoint0 = eyePoint;
    eyePoint0.w = 0.0;
    if (s.IsValid ())
        {
        Check::SaveTransformed (*s);
        showEyePoint (eyePoint);
        CurveVectorPtr silhouettes, silhouettes0;


        if (s_doFlat && s->SilhouetteCurves (eyePoint0, silhouettes0))
            if (silhouettes0.IsValid ())
                Check::SaveTransformed (*silhouettes0);

        if (s_doEye && s->SilhouetteCurves (eyePoint, silhouettes))
            if (silhouettes.IsValid ())
                Check::SaveTransformed (*silhouettes);

        }
    }
void testSilhouettes (bvector<IGeometryPtr> &geometry)
    {
    Check::QuietFailureScope scoper;
    DPoint4d eyePointA = DPoint4d::From (0,-5,10, 1);
    DPoint4d eyePointB = DPoint4d::From (0,-15,10, 1);
    DPoint4d eyePointC = DPoint4d::From (0,0,20, 1);
    DPoint4d eyePointD = DPoint4d::From (20,0,0, 1);
    // APRIL 30 2018 -- after removing RotConic, the new special case logic
    // misses the perspective-from-z-axis !! (double roots not counted correctly)

    for (size_t i = 0; i < geometry.size (); i++)
        {
        SaveAndRestoreCheckTransform shifter (30,0,0);
        testSilhouette (geometry[i], eyePointA);
        Check::Shift (0,40,0);
        testSilhouette (geometry[i], eyePointB);
        Check::Shift (0,40,0);
        testSilhouette (geometry[i], eyePointC);
        Check::Shift (0,40,0);
        testSilhouette (geometry[i], eyePointD);
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,SilhouetteTorusPipe)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddTorusPipes (geometry, 1, 0.1, true);
    testSilhouettes (geometry);
    Check::ClearGeometry ("SolidPrimitive.SilhouetteTorusPipe");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,SilhouetteCone)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddCones (geometry, 1, 1, true);
    SampleGeometryCreator::AddCones (geometry, 1, 0.1, true);
    testSilhouettes (geometry);
    Check::ClearGeometry ("SolidPrimitive.SilhouetteCone");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,Silhouette)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSimplestSolidPrimitives (geometry, true);
    SampleGeometryCreator::AddAllSolidTypes (geometry);
    testSilhouettes (geometry);
    Check::ClearGeometry ("SolidPrimitive.Silhouette");
    }

static void facetAndSave (ISolidPrimitivePtr solid)
    {
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetAngleTolerance (0.23);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    if (Check::True (builder->AddSolidPrimitive (*solid), "Builder.AddSolidPrimitive.RotationalSweep"))
        {
        Check::SaveTransformed (builder->GetClientMeshR ());
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SolidPrimitive,Seams)
    {
    Check::QuietFailureScope scoper;
    bvector<CurveVectorPtr> paths;
    SampleGeometryCreator::AddMultiPrimitiveXYOpenPaths (paths, true, true);
    // We expect ...
    // * The paths include linestring, line, arc, bspline
    // * Joints are "tangent" to exercise curve-curve transition marking.
    // * Curves ar in the xy plane -- so z is good direction for sweep
    for (auto &path : paths)
        {
        // each path is in the xy plane.
        SaveAndRestoreCheckTransform shifter (15,0,0);
        Check::SaveTransformed (*path);
        Check::Shift (0,15,0);
        // extrude in z direction ..
        facetAndSave (ISolidPrimitive::CreateDgnExtrusion (
            DgnExtrusionDetail(path, DVec3d::From (0,0,3), false)));
        Check::Shift (0,15,0);

        // rotate about a line in the x directionabove the y top of the range.
        DRange3d range;
        path->GetRange (range);
        DPoint3d axisOrigin = range.LocalToGlobal (0, 2.0, 0);
        auto rotationalSweepData = DgnRotationalSweepDetail (path, axisOrigin, DVec3d::From (1,0,0), 0.4 * Angle::Pi (), false);
        facetAndSave (ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepData));


        Check::Shift (0,15,0);
        auto shiftZ = Transform::From (0,0,3);
        auto scale = Transform::FromMatrixAndFixedPoint (RotMatrix::FromScaleFactors (1.1, 2, 1), range.low);
        auto path0 = path->Clone ();
        auto path1 = path->Clone ();
        auto path2 = path->Clone ();
        path1->TransformInPlace (shiftZ);
        path2->TransformInPlace (scale);
        path2->TransformInPlace (shiftZ);
        path2->TransformInPlace (shiftZ);
        auto path3 = path2->Clone (shiftZ);
        bvector<CurveVectorPtr> contours {path0, path1, path2, path3};
        DgnRuledSweepDetail ruledDetail (contours, false);
        ISolidPrimitivePtr ruled = ISolidPrimitive::CreateDgnRuledSweep (ruledDetail);
        facetAndSave (ruled);
        }
    Check::ClearGeometry ("SolidPrimitive.Seams");
    }

//!
//! Create a curve vector for an elliptic disk.
//! This should be an single arc primitive in a loop.
//! But PP is aggressive about converting that to an ellipse element whose start point
//!   might be shifted to a major axis point.
//! To prevent that, split the arc in two pieces.
CurveVectorPtr CreateSplitDisk (DEllipse3dCR arc, bool closed = true)
    {
    auto cv = CurveVector::Create (closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    auto arc0 = arc;
    auto arc1 = arc;
    arc0.sweep *= 0.5;
    arc1.sweep *= 0.5;
    arc1.start = arc0.start + arc0.sweep;
    cv->push_back (ICurvePrimitive::CreateArc (arc0));
    cv->push_back (ICurvePrimitive::CreateArc (arc1));
    return cv;
    }

// find s such that (vectorR + s * vectorV) DOT planeNormal = 0.
DVec3d MoveVectorToPlane (DVec3dCR vectorR, DVec3dCR vectorV, DVec3dCR planeNormal)
    {
    double dotRN = vectorR.DotProduct (planeNormal);
    double dotVN = vectorV.DotProduct (planeNormal);
    auto s = DoubleOps::ValidatedDivideDistance ( -dotRN, dotVN, 0.0);
    return vectorR + s.Value () * vectorV;
    }

// Create Chiseled pipe surfaces for a string of centerlines.
// At each end of each pipe, the pipe is cut by the plane that bisects the angle between successive pipe centerlines.
// 
void CreateChiseledPipes (
bvector<DPoint3d> &centerline,  //!< [in] points on pipe centerlines.
double radius,                      //!< [in] pipe radius.
bool combineAllPipesToSingleSolid,  //!< [in] if true, combine all pipes into a single DgnRuledSweep with many contours.
                                    //! If false, create an array of single pipes.
bvector<DEllipse3d>   &sectionArcs,    //!< [out] elliptic sections at the successive centerline points.
bvector<IGeometryPtr> &pipes       //!< [out] constructed geometry
)
    {
    pipes.clear ();
    sectionArcs.clear ();
    if (centerline.size () < 2)
        return;
    // establish coordinate system on first section . . .
    DVec3d axis0 = centerline[1] - centerline[0];
    DVec3d unitX, unitY, unitZ;
    axis0.GetNormalizedTriad (unitX, unitY, unitZ);
    // circular section on base plane ....
    auto ellipseA = DEllipse3d::FromVectors (centerline[0], radius * unitX, radius * unitY, 0.0, Angle::TwoPi ());
    DEllipse3d ellipseB = ellipseA;
    sectionArcs.push_back (ellipseA);
    for (auto i = 1; i < centerline.size (); i++, ellipseA = ellipseB)
        {
        ellipseB.center = centerline[i];
        DVec3d vectorAB = ellipseB.center - ellipseA.center;
        DVec3d vectorBC = i + 1 < centerline.size () ? centerline[i+1] - ellipseB.center : vectorAB;
        // The bisector plane normal is the average of the unit normals along incoming and outgoing centerlines . ..
        auto unitAB = vectorAB.ValidatedNormalize ();
        auto unitBC = vectorBC.ValidatedNormalize ();
        if (unitAB.IsValid () && unitBC.IsValid ())
            {
            auto bisector = 0.5 * (unitAB.Value () + unitBC.Value ());
            // On the end ellipse for this pipe section. ..
            // center comes directly from centerline[i]
            // vector0 and vector90 are obtained by sweeping the corresponding vectors of the start ellipse to the split plane.
            ellipseB.center = centerline[i];
            ellipseB.vector0 = MoveVectorToPlane (ellipseA.vector0, vectorAB, bisector);
            ellipseB.vector90 = MoveVectorToPlane (ellipseA.vector90, vectorAB, bisector);
            sectionArcs.push_back (ellipseB);
            if (!combineAllPipesToSingleSolid){
                auto cvA = CreateSplitDisk (ellipseA);
                auto cvB = CreateSplitDisk (ellipseB);
                auto sections = bvector<CurveVectorPtr> { cvA, cvB};
                auto pipeSurface = ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetail (sections, false));
                pipes.push_back (IGeometry::Create (pipeSurface));
                }
            }
        }

    if (combineAllPipesToSingleSolid)
        {
        bvector<CurveVectorPtr> sections;
        for (auto &arc : sectionArcs)
            sections.push_back (CreateSplitDisk (arc));
        auto pipeSurface = ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetail (sections, false));
        pipes.push_back (IGeometry::Create (pipeSurface));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledSurface,ChiseledPipes)
    {
    bvector<DPoint3d> centerline {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,5),
        DPoint3d::From (20, 10, 0),
        DPoint3d::From (30, 10, 0),
        DPoint3d::From (30,10, 5),
        DPoint3d::From (30,15,7),
        DPoint3d::From (35,5,3)
        };
    double r = 0.5;
    bvector<IGeometryPtr> pipes;
    bvector<DEllipse3d> sectionArcs;

    // construct separate pipes ...
    CreateChiseledPipes (centerline, r, false, sectionArcs, pipes);
    Check::SaveTransformed (centerline);
    for (auto &arc : sectionArcs)
        Check::SaveTransformed (arc);
        
    Check::Shift (0,10,0);
    Check::SaveTransformed (pipes);

    // consturct a single welded pipe...
    CreateChiseledPipes (centerline, r, true, sectionArcs, pipes);
    Check::Shift (0,10,0);
    Check::SaveTransformed (pipes);

    Check::ClearGeometry ("RuledSurface.ChiseledPipes");
    }

void SaveEllipseWithDefiningLines (DEllipse3dCR arc)
    {
    Check::SaveTransformed (arc);
    Check::SaveTransformed (
        bvector<DPoint3d>{
            arc.center + arc.vector0,
            arc.center,
            arc.center + arc.vector90
            });
    }

// GIVEN ... vectorA from centerA to any point on one section ellipse for a cone.
//           (Note that pointA = centerA + vectorA is on both (a) the ellipse at section A and (b) a particular
//                rule line of the cone.)
// AND   ... the defining vectors (usually major/minor vectors) of the other ellipse . . .
// COMPUTE A vector from centerB to that is (a) on ellipseB and (b) on the same cone rule line as vectorA
//
DVec3d ConstructCorrespondingEllipseVector
(
DPoint3dCR centerA,         // center of ellipse A
DVec3dCR   vectorA,         // any vector from centerA to a point on ellipseA
DPoint3dCR centerB,         // center of ellipseB
DVec3dCR   vectorB0,        // 0 degree vector (e.g. major axis)
DVec3dCR   vectorB90        // 90 degree vector (e.g. minor axis
)
    {
    DVec3d vectorAB = centerB - centerA;
    DVec3d planeNormal = DVec3d::FromCrossProduct (vectorA, vectorAB);
    // The centerA, centerB, and vectorA are in a plane, with planeNormal perpendicular to all vectors in that plane.
    // Find the intersections of ellipseB with that plane.
    // There are two intersections, 180 degrees apart in the parameter space of ellipseB.
    // A vector from centerB to a point "at parameter space angle theta" on ellipseB
    //     vectorB0 * cos(theta) + vectorB90 * sin(theta)
    // To make that vector "in plane", it must be perpendicular to the planeNormal:
    //     planeNormal DOT (vectorB0 * cos(theta) + vectorB90 * sin(theta)) = 0
    double dot0 = planeNormal.DotProduct (vectorB0);
    double dot90 = planeNormal.DotProduct (vectorB90);
    // we need         dot0 * cos(theta) + dot90 * sin(theta) = 0
    //      i.e.    sin(theta)/cos(theta) = tan (theta) = - dot0 / dot90
    // There are two such angles.  The angles are 180 degrees apart and define vectors that are opposite of each other.
    double theta = atan2 (-dot0, dot90);
    DVec3d candidateVector = vectorB0 * cos (theta) + vectorB90 * sin (theta);
    if (candidateVector.DotProduct (vectorA) < 0.0)
        candidateVector = candidateVector * -1.0;
    return candidateVector;  
    }

// When a cone (or cylinder) is cut but unrelated planes at two ends, the "Major and minor axis" points commonly
// used to define the ellipse are determined inedpendently on each plane, and are not on the same rule lines of the cone.
//
// This logic takes ellipseA and ellipseB from any two sections and constructs replacement for ellipseB, such that
// the parameterizations agree and thus preserve rule lines of the (possibly skewed) cone.
//
// Typically both inputs will be "major minor" ellipse form.
// But the "top ellipse" of the ruled surface will have non-perpendicular defining vectors.
ISolidPrimitivePtr ConstructChiseledConeFromSectionsWithUnrelatedEllipseDefinitions
(
DEllipse3d ellipseA,     // Ellipse from cross section A
DEllipse3d ellipseB     // Ellipse from cross section B
)
    {
    // vectorC0 and vectorC90 are defining vectors for the same ellipse points as ellipseB.
    // But they are shifted so that they are in the same rule lines as vectorA0 and vectorA90.
    DVec3d vectorC0 = ConstructCorrespondingEllipseVector (ellipseA.center, ellipseA.vector0, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
    DVec3d vectorC90 = ConstructCorrespondingEllipseVector (ellipseA.center, ellipseA.vector90, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
    DEllipse3d ellipseC = DEllipse3d::FromVectors (ellipseB.center, vectorC0, vectorC90, 0.0, Angle::TwoPi ());

    auto cvA = CreateSplitDisk (ellipseA);
    auto cvC = CreateSplitDisk (ellipseC);
    auto sections = bvector<CurveVectorPtr> { cvA, cvC};
    auto pipeSurface = ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetail (sections, false));
    return pipeSurface;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledSurface,ChiseledPipesFromArbitrarySectionEllipses)
    {

    for (double majorAxis : {1.0, 2.0})
        {
        // First cone tip is one the z axis, second is skewed off
        for (DPoint3d coneTip : {DPoint3d::From (0,0,10), DPoint3d::From (1,2,10)})
            {
            SaveAndRestoreCheckTransform shifter (0.0, 10.0, 0.0);
            // Construct an elliptic base with obvious major/minor vectors ...
            DEllipse3d ellipseA = DEllipse3d::FromVectors (
                        DPoint3d::From (0,0,0),
                        DVec3d::From (majorAxis, 0,0),
                        DVec3d::From (0.0, 1.0, 0),
                        0.0, Angle::TwoPi ());

            // These are the major/minor axis points ...
            DPoint3d majorA = ellipseA.center + ellipseA.vector0;
            DPoint3d minorA = ellipseA.center + ellipseA.vector90;


            // Make points on the axis and the 0 and 90 degree rule lines.  These are at unrelated fractional positions,
            //  so the plane through these three points is pretty random ..
            DPoint3d majorRulePointB = DPoint3d::FromInterpolate (majorA, 0.3835, coneTip);
            DPoint3d minorRulePointB = DPoint3d::FromInterpolate (minorA, 0.4, coneTip);
            DPoint3d centerB = DPoint3d::FromInterpolate (ellipseA.center, 0.378, coneTip);

            // Assemble these into an ellipse where they are 90 degrees apart parametrically but not physically (they are NOT on its major and minor axes)
            // (This is actually the ellipse we want to end up with, but we're going to make a major/minor version and back the skewed one out)
            DEllipse3d skewedEllipseB = DEllipse3d::FromPoints (centerB, majorRulePointB, minorRulePointB,
                            0.0, Angle::TwoPi ());
            // Get a major/minor ellipse that goes through all the points of ellipseB.   This is the major/minor ellipse
            //   for the cut plane that independent of ellipseA other than having the rule points 90 degrees apart
            //   in its parameterization.
            DEllipse3d majorMinorB = DEllipse3d::FromPerpendicularAxes (skewedEllipseB);


            auto ruledSurface = ConstructChiseledConeFromSectionsWithUnrelatedEllipseDefinitions (ellipseA, majorMinorB);

            // Save the major/minor base and skewed top -- note that the linework from cone tip to the 0 and 90 degree points of the
            // base hits the 0 and 90 degree points of the skewed ellipse.
            SaveEllipseWithDefiningLines (ellipseA);
            SaveEllipseWithDefiningLines (skewedEllipseB);
            Check::SaveTransformed (bvector<DPoint3d> {majorA, coneTip, minorA});
            // Now draw the same base and cone tip, but use the major minor form of the section cut.
            // Note that the rule lines do NOT hit the major and minor axis points of the upper cut.
            Check::Shift (3,0,0);
            SaveEllipseWithDefiningLines (ellipseA);
            SaveEllipseWithDefiningLines (majorMinorB);
            Check::SaveTransformed (bvector<DPoint3d> {majorA, coneTip, minorA});

            Check::Shift (10,0,0);
            Check::SaveTransformed (*ruledSurface);
            SaveEllipseWithDefiningLines (skewedEllipseB);
            Check::SaveTransformed (bvector<DPoint3d> {majorA, coneTip, minorA});
            }
        }
    Check::ClearGeometry ("RuledSurface.ChiseledPipesAnyEllipsePair");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Cone,IntersectgBoundedArc)
    {
    auto centerA = DPoint3d::From (1,0,0);
    auto centerB = DPoint3d::From (3,2,1);

    auto cone = ISolidPrimitive::CreateDgnCone (
        DgnConeDetail (centerA, centerB,
            1.0, 0.5, false
            ));
    DgnConeDetail detail;
    Check::SaveTransformed (*cone);
    if (Check::True(cone->TryGetDgnConeDetail (detail)))
        {
        auto xyzC = DPoint3d::FromInterpolate (centerA, 0.1, centerB);
        auto xyzE = DPoint3d::FromInterpolate (centerA, 0.86, centerB);
        auto xyzD = DPoint3d::FromInterpolateAndPerpendicularXY (xyzC, 0.5, xyzE, 0.8);
        auto arc = DEllipse3d::FromPointsOnArc (xyzC, xyzD, xyzE);
        Transform localToWorld, worldToLocal;
        double rA, rB;
        bvector<DPoint3d> normalizedConePoints;
        bvector<double> arcFractions;
        detail.IntersectBoundedArc (arc, arcFractions, normalizedConePoints,
            localToWorld, worldToLocal, rA, rB, true);
        bvector<DPoint3d> worldPoints;
        for (auto f : arcFractions)
            worldPoints.push_back (arc.FractionToPoint (f));
        Check::SaveTransformed (arc);
        Check::SaveTransformedMarkers (worldPoints, 0.08);
        }
    Check::ClearGeometry ("Cone.IntersectgBoundedArc");
    }
TEST(Box, ZeroSizeCap)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetParamsRequired (true);
    options->SetNormalsRequired (true);

    DPoint3d bottomOrigin = DPoint3d::From (1,1,1);
    DPoint3d topOrigin = DPoint3d::From (3,2,2);
    DVec3d vectorX = DVec3d::From (1,0,0);
    DVec3d vectorY = DVec3d::From (0,1,0);
    bool capped = true;
    for (double bottomXSize : {3.0, 0.0})
        {
        SaveAndRestoreCheckTransform shifter1(30, 0, 0);
        for (double bottomYSize : {2.5, 0.0})
            {
            SaveAndRestoreCheckTransform shifter2(0, 30, 0);
            for (double topXSize : {0.2, 0.0})
                {
                SaveAndRestoreCheckTransform shifter3(5, 0, 0);
                for (double topYSize : { 0.5, 0.0})
                    {
                    SaveAndRestoreCheckTransform shifter4 (0, 5,0);
                    auto box = ISolidPrimitive::CreateDgnBox(
                            DgnBoxDetail(bottomOrigin, topOrigin, vectorX, vectorY,
                            bottomXSize, bottomYSize, topXSize, topYSize, capped));
                    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
                    if (Check::True(builder->AddSolidPrimitive(*box), "Adding box with size combinations"))
                        {
                        PolyfaceHeaderPtr mesh = builder->GetClientMeshPtr();
                        Check::SaveTransformed(*mesh);
                        }
                    Check::Shift (0,0,-5);
                    Check::SaveTransformed(*box);
                    Check::Shift(0, 0, 5);
                    }
                }
            }
        }
    Check::ClearGeometry("Box.ZeroSizedCap");
    }
bool ReadDgnjsGeometry(bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC);
TEST(Loop, ClosureTest)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    bvector<IGeometryPtr> originalGeometry;
    // A file with linear and ruled sweeps.   Lots of full circles buried as Loop in Parity Region.
    // Rejected by IsClosedPath test . ..
    ReadDgnjsGeometry(originalGeometry, 1, L"CurveVector", L"LoopClosure", L"ClosedLoopTestFail");
    for (auto &g : originalGeometry)
        {
        auto s = g->GetAsISolidPrimitive();
        if (s.IsValid())
            {
            DRange3d range;
            s->GetRange (range);
            SaveAndRestoreCheckTransform (3.0 * range.XLength(), 0, 0);
            // Check::Shift (-range.low.x, -range.low.y, -range.low.z);
            Check::SaveTransformed (*s);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
            if (Check::True(builder->AddSolidPrimitive(*s), "Builder.AddSolidPrimitive"))
                {
                Check::Shift (0, 2.0 * range.YLength (), 0);
                Check::SaveTransformed (*builder->GetClientMeshPtr ());
                }
            }
        }
    Check::ClearGeometry("Loop.ClosureTest");
    }
