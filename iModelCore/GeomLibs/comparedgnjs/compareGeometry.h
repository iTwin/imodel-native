/*--------------------------------------------------------------------------------------+
|
|     $Source: comparedgnjs/compareGeometry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


// input "sorted" moments
// determine if equivalent allowing rotations.
bool equivalentMoments (DVec3dCR moment0, RotMatrixCR axis0, DVec3dCR moment1, RotMatrixCR axis1)
    {
    double angleTol = Angle::SmallAngle ();
    DVec3d column0X, column0Y, column0Z;
    DVec3d column1X, column1Y, column1Z;
    axis0.GetColumns (column0X, column0Y, column0Z);
    axis1.GetColumns (column1X, column1Y, column1Z);

    if (!moment0.AlmostEqual (moment1))
        return false;
    // if all directions have same moment, axes can spin arbitrarily ...
    if (DoubleOps::AlmostEqual (moment0.x, moment0.y)
        && DoubleOps::AlmostEqual (moment0.x, moment0.z))
        return true;
    // if xy are identical, only z axis has to match .. (z moment is not equal)
    if (DoubleOps::AlmostEqual (moment0.x, moment0.y))
        return column0Z.SmallerUnorientedAngleTo (column1Z) < angleTol;
    // if yz are identical, only z axis has to match .. (z moment is not equal)
    if (DoubleOps::AlmostEqual (moment0.y, moment0.z))
        return column0X.SmallerUnorientedAngleTo (column1X) < angleTol;
    // (do not test xz -- sorting should put like moments together)
    // no moment matches .. all axes must match, allowing for negation)
    RotMatrix product, product1;
    product.InitProductRotMatrixTransposeRotMatrix (axis0, axis1);
    product1.InitProduct (product, product);
    return product.IsDiagonal () && product1.IsIdentity ();
    }

struct EvolvingComparison
    {
    size_t numEqual;
    size_t numMatchingFirstTranslate;
    ValidatedDVec3d firstTranslate;
    size_t numMatchedMoments;
    EvolvingComparison () :
        firstTranslate (),
        numEqual (0),
        numMatchingFirstTranslate (0),
        numMatchedMoments (0)
        {
        }
    bool isCompatibleEqual ()
        {
        numEqual++;
        // an equal following a translate is invalid
        return !firstTranslate.IsValid ();
        }
    // FAIL if past history disagrees with new translation in any way ..
    bool isCompatibleTranslation (DVec3dCR vector)
        {
        // any equality disagrees with the new translation
        if (numEqual > 0)
            return false;
        if (numMatchingFirstTranslate == 0)
            {
            firstTranslate = ValidatedDVec3d (vector, true);
            numMatchingFirstTranslate = 1;
            return true;
            }
        if (numMatchingFirstTranslate > 0)
            {
            if (vector.AlmostEqual (firstTranslate.Value ()))
                return true;
            }
        return false;
        }
    void announceMatchedMoments () { numMatchedMoments++; }
    };
void EnforceSmallToLarge (double &a, DVec3dR vectorA, double &b, DVec3dR vectorB)
    {
    if (a > b)
        {
        std::swap (a, b);
        std::swap (vectorA, vectorB);
        }
    }
void SortMomentsSmallToLarge (DVec3dR moments, RotMatrixR axes)
    {
    DVec3d columnX, columnY, columnZ;
    axes.GetColumns (columnX, columnY, columnZ);
    EnforceSmallToLarge (moments.x, columnX, moments.y, columnY);
    EnforceSmallToLarge (moments.x, columnX, moments.z, columnZ);
    EnforceSmallToLarge (moments.y, columnY, moments.z, columnZ);
    axes.InitFromColumnVectors (columnX, columnY, columnZ);
    }

// 0==> different in orientation, area, or moments.
// 1==> identical area, orientations, and moments, but translated
// 2==> all identical
bool secondaryCompare (IGeometryPtr &g0, IGeometryPtr &g1, EvolvingComparison &stats, double tolerance)
    {
    if (g0->GetGeometryType () != g1->GetGeometryType ())
        return 0;
    if (g0->GetGeometryType () == IGeometry::GeometryType::Polyface)
        {
        auto mesh0 = g0->GetAsPolyfaceHeader ();
        auto mesh1 = g1->GetAsPolyfaceHeader ();
        double area0, area1;
        DPoint3d centroid0, centroid1;
        RotMatrix axis0, axis1;
        DVec3d moment0, moment1;
        mesh0->ComputePrincipalAreaMoments (area0, centroid0, axis0, moment0);
        SortMomentsSmallToLarge (moment0, axis0);
        mesh1->ComputePrincipalAreaMoments (area1, centroid1, axis1, moment1);
        SortMomentsSmallToLarge (moment1, axis1);
        if (!DoubleOps::AlmostEqual (area0, area1))
            return false;
        if (!equivalentMoments (moment0, axis0, moment1, axis1))
            return false;
        // moments are equal.  Record it and inspect the translation.
        stats.announceMatchedMoments ();
        if (centroid0.AlmostEqual (centroid1))
            return stats.isCompatibleEqual ();
        auto vector = centroid1 - centroid0;
        return stats.isCompatibleTranslation (vector);
        }

    else if (g0->GetGeometryType () == IGeometry::GeometryType::CurveVector)
        {
        Transform frame0, frame1;
        auto cv0 = g0->GetAsCurveVector ();
        auto cv1 = g1->GetAsCurveVector ();
        DPoint3d translate0, translate1;
        if (!cv0->IsSameStructure (*cv1))
            return 0;
        cv0->GetAnyFrenetFrame (frame0);
        cv1->GetAnyFrenetFrame (frame1);
        frame0.GetTranslation (translate0);
        frame1.GetTranslation (translate1);
        DVec3d vector = translate1 - translate0;
        Transform translate = Transform::From (vector);
        auto cv0A = cv0->Clone (translate);
        if (cv0A->IsSameStructureAndGeometry (*cv1))
            return stats.isCompatibleTranslation (vector);
        }
    else if (g0->GetGeometryType () == IGeometry::GeometryType::CurvePrimitive)
        {
        auto cp0 = g0->GetAsICurvePrimitive ();
        auto cp1 = g1->GetAsICurvePrimitive ();
        DPoint3d translate0, translate1;
        if (!cp0->IsSameStructure (*cp1))
            return 0;
        auto frame0 = cp0->FractionToFrenetFrame (0.0);
        auto frame1 = cp1->FractionToFrenetFrame (0.0);
        if (frame0.IsValid () && frame1.IsValid ())
            {
            frame0.Value ().GetTranslation (translate0);
            frame1.Value ().GetTranslation (translate1);
            DVec3d vector = translate1 - translate0;
            Transform translate = Transform::From (vector);
            auto cp0A = cp0->Clone (translate);
            if (cp0A->IsSameStructureAndGeometry (*cp1))
                return stats.isCompatibleTranslation (vector);
            }
        }
    else if (g0->GetGeometryType () == IGeometry::GeometryType::BsplineSurface)
        {
        auto surf0 = g0->GetAsMSBsplineSurface ();
        auto surf1 = g1->GetAsMSBsplineSurface ();
        DPoint3d translate0, translate1;
        if (!surf0->IsSameStructure (*surf1))
            return 0;
        Transform frame0, frame1;
        if (   surf0->EvaluateNormalizedFrame (frame0, 0.0, 0.0)
            && surf1->EvaluateNormalizedFrame (frame1, 0.0, 0.0))
            {
            frame0.GetTranslation (translate0);
            frame1.GetTranslation (translate1);
            DVec3d vector = translate1 - translate0;
            Transform translate = Transform::From (vector);
            auto surf0A = surf0->CreateCopyTransformed (translate);
            if (surf0A->IsSameStructureAndGeometry (*surf1, tolerance))
                return stats.isCompatibleTranslation (vector);
            }
        }

    return false;
    }
