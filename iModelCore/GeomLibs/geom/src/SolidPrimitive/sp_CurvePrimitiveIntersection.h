/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// This h file is to be included in only one c file (sp_curveIntersection.cpp)
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
template <typename SolidPrimitiveSubType>
struct DgnSolidCurveIntersectionProcessor : ICurvePrimitiveProcessor
{
SolidPrimitiveSubType const &m_primitive;
bvector<CurveLocationDetail> &m_curvePoints;
bvector<SolidLocationDetail> &m_solidPoints;
MeshAnnotationVector &m_messages;
DgnSolidCurveIntersectionProcessor (
    SolidPrimitiveSubType const &primitive,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    )
    : m_curvePoints (curvePoints),
      m_solidPoints (solidPoints),
      m_primitive (primitive),
      m_messages (messages)
    {
    }

// Called by default implementations --- useful place for debugger breaks during class development
void _ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval) override 
    {
        m_messages.Assert (false, "Unimplmented Curve*SolidPrimitive intersection type");
    }



// @remark Default action: use AddRayIntersection for the line segment.
void _ProcessLine (ICurvePrimitiveCR curve, DSegment3dCR segment, DSegment1dCP interval) override 
    {
    size_t baseSize = m_solidPoints.size ();
    DRay3d ray = DRay3d::FromOriginAndTarget (segment.point[0], segment.point[1]);
    m_primitive.AddRayIntersections (m_solidPoints, ray, 0, 0.0);
    FilterMaxParameter (m_solidPoints, baseSize, 1.0);
    for (size_t i = baseSize; i < m_solidPoints.size (); i++)
        {
        SolidLocationDetail &solidDetail = m_solidPoints.at (i);
        CurveLocationDetail curveDetail (&curve, solidDetail.GetPickParameter (), solidDetail.GetXYZ ());
        m_curvePoints.push_back (curveDetail);
        }
    }

// @remark Default action: use AddRayIntersection for each line segment.
void _ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override 
    {
    size_t numPoints = points.size ();
    if (numPoints > 2)
        {
        size_t numComponent = numPoints - 1;
        for (size_t i = 0; i + 1 < numPoints; i++)
            {
            auto segment = DSegment3d::From (points[i], points[i+1]);
            size_t baseIndex = m_curvePoints.size ();
            _ProcessLine (curve, segment, interval);
            // point fractions are relative to the single segment .. map to the linestring.
            for (size_t k = baseIndex; k < m_curvePoints.size (); k++)
                m_curvePoints[k].SetFractionFromComponentFraction (m_curvePoints[k].fraction, i, numComponent);
            }
        }
    }

// Called by default implementations --- useful place for debugger breaks during class development
//virtual void _ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval);

// Visit all contained curves (subject to abort flag)
//virtual void _ProcessCurveVector (CurveVector const &curveVector, DSegment1dCP interval);


// @remark Default action: noop.
//virtual void _ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval);

// @remark Default action: noop.
//virtual void _ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval);
// Default action: noop
//virtual void _ProcessProxyBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval);
// Default action: noop
//virtual void _ProcessPartialCurve (ICurvePrimitiveCR curve, PartialCurveDetailCR detail, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
//virtual void _ProcessInterpolationCurve (ICurvePrimitiveCR curve, MSInterpolationCurveCR icurve, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
//virtual void _ProcessSpiral (ICurvePrimitiveCR curve, DSpiral2dPlacementCR spiral, DSegment1dCP interval);
// Default action: recurse
//virtual void _ProcessChildCurveVector (ICurvePrimitiveCR curve, CurveVectorCR child, DSegment1dCP interval);
// Default action: noop
//virtual void _ProcessPointString (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval);
// Default action: invoke _ProcessProxyBsplineCurve
//virtual void _ProcessAkimaCurve (ICurvePrimitiveCR curve, bvector<DPoint3d>const &points, DSegment1dCP interval);


};


// ************************* TorusPipe ******************************************

struct DgnTorusPipeCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnTorusPipeDetail>
{
DgnTorusPipeCurveIntersectionProcessor (
    DgnTorusPipeDetailCR TorusPipe, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (TorusPipe, curvePoints, solidPoints, messages)
    {
    }
// 
void _ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval) override
    {
    Transform localToWorld, worldToLocal;
    bvector<double> curveFractions;
    bvector<DPoint3d> normalizedConePoints;
    double rMajor, rMinor, sweepRadians;
    m_primitive.IntersectCurveLocal (curve, curveFractions, normalizedConePoints, localToWorld, worldToLocal, rMajor, rMinor, sweepRadians, true);
    for (size_t i = 0; i < curveFractions.size (); i++)
        {
        double curveFraction = curveFractions[i];
        DPoint3d xyz;
        curve.FractionToPoint (curveFraction, xyz);
        CurveLocationDetail curveDetail (&curve, curveFraction, xyz);
        SolidLocationDetail solidDetail (0, curveFraction, xyz);
        DgnTorusPipeDetail::SetDetailCoordinatesFromLocalPipeCoordinates (solidDetail, normalizedConePoints[i], localToWorld, rMajor, rMinor, sweepRadians);
        solidDetail.SetFaceIndices (0,0,0);
#define ConfirmFractions 1
#ifdef ConfirmFractions
        DPoint3d xyz1;
        DVec3d dXdu1, dXdv1;
        auto indices = solidDetail.GetFaceIndices ();
        m_primitive.TryUVFractionToXYZ (indices, solidDetail.GetU (), solidDetail.GetV (), xyz1, dXdu1, dXdv1);
#endif

        m_curvePoints.push_back (curveDetail);
        m_solidPoints.push_back (solidDetail);
        }
    }
};

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnTorusPipeDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    DgnTorusPipeCurveIntersectionProcessor processor (*this, curvePoints, solidPoints, messages);
    curve._Process (processor, nullptr);
    }

// ************************* Box ******************************************

struct DgnBoxCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnBoxDetail>
{
DgnBoxCurveIntersectionProcessor (
    DgnBoxDetailCR Box, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (Box, curvePoints, solidPoints, messages)
    {
    }
};
//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnBoxDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    DgnBoxCurveIntersectionProcessor processor (*this, curvePoints, solidPoints, messages);
    curve._Process (processor, nullptr);
    }

// ************************* Sphere ******************************************

struct DgnSphereCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnSphereDetail>
{
DgnSphereCurveIntersectionProcessor (
    DgnSphereDetailCR Sphere, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (Sphere, curvePoints, solidPoints, messages)
    {
    }

void _ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval) override 
    {
    Transform localToWorld, worldToLocal;
    bvector<double> arcFractions;
    bvector<DPoint3d> normalizedSpherePoints;
    m_primitive.IntersectBoundedArc (arc, arcFractions, normalizedSpherePoints, localToWorld, worldToLocal, true);
    for (size_t i = 0; i < arcFractions.size (); i++)
        {
        double arcFraction = arcFractions[i];
        DPoint3d xyz = arc.FractionToPoint (arcFraction);
        CurveLocationDetail curveDetail (&curve, arcFractions[i], xyz);
        SolidLocationDetail solidDetail (0, arcFraction, xyz);
        DgnSphereDetail::SetDetailUVFromUnitSphereCoordinates (solidDetail, normalizedSpherePoints[i], localToWorld, m_primitive.m_startLatitude, m_primitive.m_latitudeSweep);
        solidDetail.SetFaceIndices (0,0,0);
        m_curvePoints.push_back (curveDetail);
        m_solidPoints.push_back (solidDetail);

        }
    }
};

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnSphereDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    DgnSphereCurveIntersectionProcessor processor (*this, curvePoints, solidPoints, messages);
    curve._Process (processor, nullptr);
    }


// ************************* Cone ******************************************

struct DgnConeCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnConeDetail>
{
DgnConeCurveIntersectionProcessor (
    DgnConeDetailCR cone, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (cone, curvePoints, solidPoints, messages)
    {
    }

// Called by default implementations --- useful place for debugger breaks during class development
void _ProcessDefault (ICurvePrimitiveCR curve, DSegment1dCP interval) override
    {
    Transform localToWorld, worldToLocal;
    bvector<double> curveFractions;
    bvector<DPoint3d> normalizedConePoints;
    double r0, r1;
    // main body ...
    m_primitive.IntersectCurveLocal (curve, curveFractions, normalizedConePoints, localToWorld, worldToLocal, r0, r1, true);
    for (size_t i = 0; i < curveFractions.size (); i++)
        {
        double curveFraction = curveFractions[i];
        DPoint3d xyz;
        curve.FractionToPoint (curveFraction, xyz);
        CurveLocationDetail curveDetail (&curve, curveFraction, xyz);
        SolidLocationDetail solidDetail (0, curveFraction, xyz);
        DgnConeDetail::SetDetailCoordinatesFromFractionalizedConeCoordinates (solidDetail, normalizedConePoints[i], localToWorld, r0, r1);
        solidDetail.SetFaceIndices (0,0,0);
        m_curvePoints.push_back (curveDetail);
        m_solidPoints.push_back (solidDetail);
        }
    // caps ...
    if (m_primitive.IsClosedVolume ())
        {
        DEllipse3d capEllipse[2];
        m_primitive.FractionToSection (0.0, capEllipse[0]);
        m_primitive.FractionToSection (1.0, capEllipse[1]);
        bvector<CurveAndSolidLocationDetail> capData;
        int parentId = 0;
        for (int i = 0; i < 2; i++)
            {
            if (m_primitive.IsRealCap (i))
                {
                DPoint3dDVec3dDVec3d capPlane (capEllipse[i]);
                capData.clear ();
                curve.AppendCurvePlaneIntersections (capPlane, UVBoundarySelect::UnitCircle, capData);
                for (auto &hitData : capData)
                    {
                    SolidLocationDetail solidDetail (
                        parentId,
                        hitData.m_curveDetail.fraction,
                        hitData.m_curveDetail.point);
                    solidDetail.SetCapSelector (i);
                    double uFraction, vFraction;
                    double r = 1.0; // The capPlane vectors capture the radius.
                    double da = 2.0 * r;    // distance across the ellipse disk is twice the arc vector length.
                    FractionalizeInCircle (
                        hitData.m_solidDetail.GetU (), hitData.m_solidDetail.GetV (),
                        r, uFraction, vFraction);
                    DVec3d uVector = capPlane.vectorU;
                    DVec3d vVector = capPlane.vectorV;
                    uVector.Scale (da);
                    vVector.Scale (da);
                    solidDetail.SetUV (uFraction, vFraction, uVector, vVector);
                    m_curvePoints.push_back (hitData.m_curveDetail);
                    m_solidPoints.push_back (solidDetail);
                    }
                }
            }
        }
    }
};


//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnConeDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    DgnConeCurveIntersectionProcessor processor (*this, curvePoints, solidPoints, messages);
    curve._Process (processor, nullptr);
    }


// ************************* Extrusion ******************************************

struct DgnExtrusionCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnExtrusionDetail>
{
DgnExtrusionCurveIntersectionProcessor (
    DgnExtrusionDetailCR Extrusion, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (Extrusion, curvePoints, solidPoints, messages)
    {
    }
};

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnExtrusionDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    Transform localToWorld, worldToLocal;
    DMatrix4d viewingTransform;
    if (TryGetZExtrusionFrame (localToWorld, worldToLocal))
        {
        viewingTransform.InitFrom (worldToLocal);
        CurveVectorPtr intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::IntersectionsXY (*intersectionsA, *intersectionsB,
                const_cast <ICurvePrimitiveR> (curve),
                *m_baseCurve,
                &viewingTransform);
        for (size_t i = 0; i < intersectionsA->size (); i++)
            {
            CurveLocationDetail detailA, detailB;
            if (CurveCurve::IsSinglePointPair (*intersectionsA, *intersectionsB, i, detailA, detailB))
                {
                DRay3d ray = DRay3d::FromOriginAndVector (detailB.point, m_extrusionVector);
                double rayFraction;
                DPoint3d xyz;
                size_t leafIndex;
                DRay3d baseCurveTangent;
                if (ray.ProjectPointBounded (xyz, rayFraction, detailA.point)
                    && m_baseCurve->LeafToIndex (detailB.curve, leafIndex)
                    && detailB.curve->FractionToPoint (detailA.fraction, baseCurveTangent)
                    )
                    {
                    // curve detail is ready to go !!
                    // Just have to assemble the solid detail ....
                    SolidLocationDetail solidDetail (0, detailB.fraction, xyz);
                    solidDetail.SetFaceIndices (0, leafIndex, detailB.componentIndex);
                    solidDetail.SetUV (detailB.componentFraction, rayFraction, baseCurveTangent.direction, m_extrusionVector);
                    curvePoints.push_back (detailA);
                    solidPoints.push_back (solidDetail);
                    }
                }
            }
        }
    }

// ************************* RotationalSweep ******************************************


//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnRotationalSweepDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    Transform localToWorld, worldToLocal;
    if (GetTransforms (localToWorld, worldToLocal))
        {
        bvector<CurveLocationDetail> cciDetailA, cciDetailB;
        CurveCurve::IntersectRotatedCurveSpaceCurve (worldToLocal, *m_baseCurve, curve, cciDetailA, cciDetailB);
        for (size_t i = 0; i < cciDetailA.size (); i++)
            {
            // restrict the cci details to actual rotation space ...
            auto &solidDetail = cciDetailA.at (i);  // a curve of hte space curve
            auto &curveDetail = cciDetailB.at (i);  // a curve of the base curve
            DRay3d baseCurveTangent;
            size_t leafIndex;
            double thetaCurve = curveDetail.a;      // angle of the physical point on the curve
            double thetaSolid = solidDetail.a;      // if the solid started on xz plane, this is zero.   But it might be off, and solid sweep starts here.
            if (Angle::InSweepAllowPeriodShift (thetaCurve, thetaSolid, m_sweepAngle)
                && m_baseCurve->LeafToIndex (solidDetail.curve, leafIndex)
                && solidDetail.curve->FractionToPoint (solidDetail.fraction, baseCurveTangent)  // that's a point and derivative on the pre-rotation curve.
                )
                {

#ifdef ComputeDerivativesFromScratch

                RotMatrix rotate01 = RotMatrix::FromAxisAndRotationAngle (2, theta);
                DPoint3d xyz0;          // on the base curve
                DVec3d   dXdu0, dXdv0;  // on the base curve
                DVec3d   dXdu1, dXdv1;
                DVec3d   dXdu2, dXdv2;
                DVec3d   xyz0AsVector;
                worldToLocal.Multiply (xyz0, baseCurveTangent.origin);
                worldToLocal.MultiplyMatrixOnly (dXdu0, baseCurveTangent.direction);
                xyz0AsVector = DVec3d::From (xyz0);
                dXdv0 = DVec3d::FromCrossProduct (DVec3d::From (0,0,m_sweepAngle), xyz0AsVector);

                // rotate derivatives from base curve to local v point . . .
                rotate01.Multiply (dXdu1, dXdu0);
                rotate01.Multiply (dXdv1, dXdv0);

                // and get back to world 
                localToWorld.MultiplyMatrixOnly (dXdu2, dXdu1);
                localToWorld.MultiplyMatrixOnly (dXdv2, dXdv1);

                double sweepToPoint = Angle::NormalizeToSweep (cciDetailB[i].a, 0.0, m_sweepAngle);
                double sweepFraction = DoubleOps::ValidatedDivideParameter (sweepToPoint, m_sweepAngle, 0.0);

                SolidLocationDetail solidDetailB (0, solidDetail.fraction, solidDetail.point);
                solidDetailB.SetFaceIndices (0, leafIndex, solidDetail.componentIndex);
                // UMMM.. does the U derivative have to be a scaled up by component fraction step?????? 
                solidDetailB.SetUV (solidDetail.componentFraction, sweepFraction, dXdu2, dXdv2);
                curvePoints.push_back (curveDetail);
                solidPoints.push_back (solidDetailB);
#else
                double sweepFraction = Angle::NormalizeToSweep (cciDetailB[i].a, thetaSolid, m_sweepAngle);
                auto faceId = SolidLocationDetail::FaceIndices (0, leafIndex, solidDetail.componentIndex);
                DPoint3d xyzA;
                DVec3d dAdu, dAdv;
                // The point on the rotated base curve has to be evaluated in rotated position . . .
                TryUVFractionToXYZ (faceId, solidDetail.componentFraction, sweepFraction, xyzA, dAdu, dAdv);

                SolidLocationDetail solidDetailB (0, solidDetail.fraction, solidDetail.point);
                solidDetailB.SetFaceIndices (faceId);
                // UMMM.. does the U derivative have to be a scaled up by component fraction step?????? 
                solidDetailB.SetUV (solidDetail.componentFraction, sweepFraction, dAdu, dAdv);
                solidDetailB.SetXYZ (xyzA);
                curvePoints.push_back (curveDetail);
                solidPoints.push_back (solidDetailB);
#endif
                }

            }
        }
    }

// ************************* RuledSweep ******************************************
struct DgnRuledSweepCurveIntersectionProcessor : DgnSolidCurveIntersectionProcessor<DgnRuledSweepDetail>
{
DgnRuledSweepCurveIntersectionProcessor (
    DgnRuledSweepDetailCR RuledSweep, 
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages)
    : DgnSolidCurveIntersectionProcessor (RuledSweep, curvePoints, solidPoints, messages)
    {
    }
};

//! Compute intersections with a single curve and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnRuledSweepDetail::AddCurveIntersections
    (
    ICurvePrimitiveCR curve,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    DgnRuledSweepCurveIntersectionProcessor processor (*this, curvePoints, solidPoints, messages);
    curve._Process (processor, nullptr);
    }



//  ***************************** Solid Intersect CurveVector ******************************************
// Generic implementation of AddCurveIntersections for solid and curve vector -- just loop over primitives in the curve vector.
template <typename SolidPrimitiveType>
static void AddCurveVectorIntersections (
    SolidPrimitiveType const &solid,
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    )
    {
    for (ICurvePrimitivePtr const &curve : curves)
        solid.AddCurveIntersections (*curve, curvePoints, solidPoints, messages);
    }

 //! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnTorusPipeDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnConeDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnSphereDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnBoxDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnExtrusionDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnRotationalSweepDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }

//! Compute intersections with curves and add to the data array.
//! @param [in] curves
//! @param [in,out] curvePoints growing array of curve points.
//! @param [in,out] solidPoints growing array of solid points.
//! @param [in,out] messages array of error messages
GEOMDLLIMPEXP void DgnRuledSweepDetail::AddCurveIntersections
    (
    CurveVectorCR curves,
    bvector<CurveLocationDetail> &curvePoints,
    bvector<SolidLocationDetail> &solidPoints,
    MeshAnnotationVector &messages
    ) const
    {
    AddCurveVectorIntersections (*this, curves, curvePoints, solidPoints, messages);
    }



END_BENTLEY_GEOMETRY_NAMESPACE


