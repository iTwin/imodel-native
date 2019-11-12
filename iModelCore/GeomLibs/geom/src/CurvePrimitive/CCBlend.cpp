/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
struct CCBlendProcessor : public CurveCurveProcessor
{
BlendType   m_blendType;
double                  m_distanceA;
double                  m_distanceB;
bvector<BlendDetail> m_blends;

CCBlendProcessor (BlendType blendType, double distanceA, double distanceB, bool extend = false) :
    CurveCurveProcessor (NULL, 0.0),
    m_blendType (blendType), m_distanceA (distanceA), m_distanceB (distanceB)
    {
    SetExtend (extend);
    }

void GrabBlend (bvector<BlendDetail> &blends)
    {
    blends.clear ();
    blends.swap (m_blends);
    }
    
CurveLocationDetail FinalizedDetail (CurveLocationDetailCR detail0, double fraction, DPoint3dCR xyz)
    {
    CurveLocationDetail detail = detail0;
    detail.fraction = fraction;
    detail.point = xyz;
    if (detail.numComponent > 0)
        {
        double f0 = detail.componentIndex / (double) detail.numComponent;
        double f1 = (detail.componentIndex + 1) / (double) detail.numComponent;
        detail.fraction = DoubleOps::Interpolate (f0, fraction, f1);
        }
    return detail;
    }

void GenerateBlendParabola
(
DPoint3dCR BlendCenter,
CurveLocationDetailCR detailA,  // contains start point!!
CurveLocationDetailCR detailB,  // contains end point !!
bool       bReverseOrder
)
    {
    DPoint3d startPoint = detailA.point;
    DPoint3d endPoint = detailB.point;
    if (bReverseOrder)
        {
        GenerateBlendParabola (BlendCenter, detailB, detailA, false);
        }
    else
        {
        MSBsplineCurve parabola;
        parabola.Allocate (3, 3, false, false);

        parabola.knots[0] = parabola.knots[1] = parabola.knots[2] = 0.0;
        parabola.knots[3] = parabola.knots[4] = parabola.knots[5] = 1.0;

        parabola.poles[0] = startPoint;
        parabola.poles[1] = BlendCenter;
        parabola.poles[2] = endPoint;

        BlendDetail detail;
        detail.detailA = detailA;
        detail.detailB = detailB;
        detail.geometry = ICurvePrimitive::CreateBsplineCurve (parabola);
        m_blends.push_back (detail);
        parabola.ReleaseMem ();
        }
    }

bool validFraction (CurveLocationDetailCR detail, double fraction)
    {
    bool extensible0 = true;
    bool extensible1 = true;
    if (detail.componentIndex != 0)
        extensible0 = false;
    if (detail.componentIndex + 1 != detail.numComponent)
        extensible1 = false;
    if (!m_extend)
        extensible0 = extensible1 = false;
    if (fraction < 0.0 && !extensible0)
        return false;
    if (fraction > 1.0 && !extensible1)
        return false;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
On input, detailA0 and detailB0 identify curve and component index but have no point data.
* @bsimethod                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
CurveLocationDetailCR detailA0, DSegment3dCR segmentA,
CurveLocationDetailCR detailB0, DSegment3dCR segmentB,
bool bReverseOrder
)
    {
#ifdef SpiralBlends
    if (ProcessSegmentSegment_withSpirals (segmentA, segmentB, bReverseOrder))
        return;
#endif
    DVec3d tangentA, tangentB;

    tangentA.DifferenceOf (segmentA.point[1], segmentA.point[0]);
    tangentB.DifferenceOf (segmentB.point[1], segmentB.point[0]);
    if (tangentA.IsParallelTo (tangentB))
        return;

    double paramA, paramB, distanceA, distanceB;
    DPoint3d approachPointA, approachPointB;
    DPoint3d tangentPointA, tangentPointB;
    DVec3d unitTangentA, unitTangentB;
    unitTangentA.Normalize (tangentA);
    unitTangentB.Normalize (tangentB);
    
    DRay3d rayA = DRay3d::From (segmentA);
    DRay3d rayB = DRay3d::From (segmentB);

    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay
                        (
                        paramA, paramB,
                        approachPointA, approachPointB,
                        rayA, rayB
                        ))
        {
        for (double signA = -1.0; signA < 3.0; signA += 2.0)
            {
            for (double signB = -1.0; signB < 3.0; signB += 2.0)
                {

                // compute directions from the (intersection/approach) point in the forward or reverse line directions):
                DVec3d directionA, directionB;
                directionA.Scale (unitTangentA, signA);
                directionB.Scale (unitTangentB, signB);

                if (m_blendType == CURVE_CURVE_BLEND_BisectorParabola)
                    {
                    double alfa = directionA.AngleTo (directionB);
                    alfa *= 0.5;
                    distanceA = m_distanceA / sin (alfa);
                    distanceB = m_distanceB / sin (alfa);
                    }
                else
                    {
                    double alfaA = unitTangentA.SmallerUnorientedAngleTo (DVec3d::From (1.0, 0.0, 0.0));
                    double alfaB = unitTangentB.SmallerUnorientedAngleTo (DVec3d::From (1.0, 0.0, 0.0));
                    distanceA = m_distanceA / cos (alfaA);
                    distanceB = m_distanceB / cos (alfaB);
                    }

                tangentPointA.SumOf (approachPointA, directionA, distanceA);
                tangentPointB.SumOf (approachPointB, directionB, distanceB);
                DPoint3d projectionOnA, projectionOnB;  // These should be identical to tangentPointA, tangentPointB !!
                double projectionFractionA, projectionFractionB;
                segmentA.ProjectPoint (projectionOnA, projectionFractionA, tangentPointA);
                segmentB.ProjectPoint (projectionOnB, projectionFractionB, tangentPointB);

                if (   validFraction (detailA0, projectionFractionA)
                   && validFraction (detailB0, projectionFractionB)
                   )
                   {
                   GenerateBlendParabola (approachPointA, 
                            FinalizedDetail (detailA0, projectionFractionA, projectionOnA),
                            FinalizedDetail (detailB0, projectionFractionB, projectionOnB),
                            bReverseOrder);
                    }
                }
            }
        }
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB,
        bool bReverseOrder) override
    {
    CurveLocationDetail detailA (curveA);
    CurveLocationDetail detailB (curveB);
    ProcessLineLine (detailA, segmentA, detailB, segmentB, bReverseOrder);
    }    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLinestring(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder) override
    {
    size_t numPointsB = linestringB.size ();
    CurveLocationDetail detailA (curveA);
    CurveLocationDetail detailB (curveB, numPointsB - 1);
    for (size_t i = 1; i < numPointsB; i++)
        {
        DSegment3d segmentB = DSegment3d::From (linestringB[i-1], linestringB[i]);
        detailB.componentIndex = i - 1;
        ProcessLineLine (
                    detailA, segmentA,
                    detailB, segmentB, bReverseOrder);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
void ProcessLinestringLinestring(
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder) override
    {
    size_t numPointsA = linestringA.size ();
    size_t numPointsB = linestringB.size ();
    CurveLocationDetail detailA (curveA, numPointsA - 1);
    CurveLocationDetail detailB (curveB, numPointsB - 1);

    for (size_t i = 1; i < numPointsA; i++)
        {
        DSegment3d segmentA = DSegment3d::From (linestringA[i-1], linestringA[i]);
        detailA.componentIndex = i - 1;
        for (size_t j = 1; j < numPointsB; j++)
            {
            DSegment3d segmentB = DSegment3d::From (linestringB[j-1], linestringB[j]);
            detailB.componentIndex = j - 1;
            ProcessLineLine (
                        detailA, segmentA,
                        detailB, segmentB,
                        bReverseOrder);
            }
        }
    }

void ProcessLinestringInternal(ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA)
    {
    size_t numPointsA = linestringA.size ();
    CurveLocationDetail detailA (curveA, numPointsA - 1);
    CurveLocationDetail detailB (curveA, numPointsA - 1);
    
    for (size_t i = 1; i < numPointsA; i++)
        {
        DSegment3d segmentA = DSegment3d::From (linestringA[i-1], linestringA[i]);
        detailA.componentIndex = i - 1;
        for (size_t j = i + 1; j < numPointsA; j++)
            {
            DSegment3d segmentB = DSegment3d::From (linestringA[j-1], linestringA[j]);
            detailB.componentIndex = j - 1;
            ProcessLineLine (
                        detailA, segmentA,
                        detailB, segmentB, false);
            }
        }
    }

void ProcessLineArc(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    }
};
/*
static ICurvePrimitivePtr WrappedAccess (CurveVectorCR curves, ICurvePrimitivePtr &wrap, size_t i)
    {
    if (i < curves.size ())
        return curves[i];
    if (i == curves.size ())
        return wrap;
    return NULL;
    }
*/
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PeterYu      09/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::CollectBlends
(
CurveVectorCR chainA, 
CurveVectorCR chainB,
BlendType blendType,
double distanceA,
double distanceB,
bool extend,
bvector<BlendDetail> &blendCurves
)
    {
    CCBlendProcessor processor (blendType, distanceA, distanceB, extend);
    bool sameSource = &chainA == &chainB;
    blendCurves.clear ();
    if (!sameSource)
        {
        for (size_t iA = 0, nA = chainA.size (); iA < nA; iA++)
            {
            for (size_t iB = 0, nB = chainB.size (); iB < nB; iB++)
                processor.Process (chainA[iA].get (), chainB[iB].get ());                        
            }
        }
    else
        {
        for (size_t iA = 0, nA = chainA.size (); iA < nA; iA++)
            {
            bvector<DPoint3d> const *linestring = chainA[iA]->GetLineStringCP ();
            // special case interactions within the linestring
            if (NULL != linestring)
                {
                processor.ProcessLinestringInternal (chainA[iA].get (), *linestring);
                }
            // all interactions with successors 
            for (size_t iB = iA + 1, nB = chainB.size (); iB < nB; iB++)
                processor.Process (chainA[iA].get (), chainB[iB].get ());                        
            }
        }
    processor.GrabBlend (blendCurves);        
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PeterYu      08/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::CollectBlends
(
ICurvePrimitiveR curveA, 
ICurvePrimitiveR curveB,
BlendType blendType,
double distanceA,
double distanceB,
bool extend,
bvector<BlendDetail> &blendCurves
)
    {
    blendCurves.clear ();
    CCBlendProcessor processor (blendType, distanceA, distanceB, extend);
    bvector<DPoint3d> const *linestring = curveA.GetLineStringCP ();
    // special case interactions within the linestring
    if (&curveA == &curveB && NULL != linestring)
        {
        processor.ProcessLinestringInternal (&curveA, *linestring);
        }
    else
        processor.Process (&curveA, &curveB);
    processor.GrabBlend (blendCurves);
    }

CurveVectorPtr CurveVector::CloneWithBlends (BlendType blendType, double distanceA, double distanceB) const
    {
    CurveVector::BoundaryType boundaryType = GetBoundaryType ();
    if (size () < 1)
        return NULL;
    if (boundaryType == CurveVector::BOUNDARY_TYPE_Outer
        || boundaryType == CurveVector::BOUNDARY_TYPE_Inner
        || boundaryType == CurveVector::BOUNDARY_TYPE_Open
        )
        {
        if (CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString) > 0)
            {
            CurveVectorPtr source1 = CloneWithExplodedLinestrings ();
            return source1->CloneWithBlends (blendType, distanceA, distanceB);
            }
        else
            {
            CurveVectorPtr result = CurveVector::Create (GetBoundaryType ());
            CCBlendProcessor processor (blendType, distanceA, distanceB, false);
            ICurvePrimitivePtr primitiveA =  at(0);
            ICurvePrimitivePtr primitiveB;
            bvector<BlendDetail> blends;
            ICurvePrimitivePtr wrappedPrimitive = NULL;
            bool doWrap = IsClosedPath ();
            size_t n = size ();
            size_t nLoopStep = doWrap ? n + 1 : n;
            for (size_t i = 1; i < nLoopStep; i++)
                {
                if (!primitiveA.IsValid ())
                    break;
                if (i == n)
                    primitiveB = wrappedPrimitive;
                else
                    primitiveB = at(i);
                if (!primitiveB.IsValid ())
                    break;
                processor.Process (primitiveA.get (), primitiveB.get ());
                processor.GrabBlend (blends);
                if (blends.size () == 1)
                    {
                    BlendDetail detail = blends[0];
                    ICurvePrimitivePtr trimA = primitiveA->CloneBetweenFractions (0.0, detail.detailA.fraction, true);
                    ICurvePrimitivePtr trimB = primitiveB->CloneBetweenFractions (detail.detailB.fraction, 1.0, true);
                    if (doWrap && i == 1)
                        wrappedPrimitive = trimA;   // This will come back to get trimmed from the other end !!
                    else
                        result->push_back (trimA);
                    result->push_back (detail.geometry);
                    primitiveA = trimB;
                    }
                else
                    {
                    result->push_back (primitiveA->Clone ());
                    if (doWrap && i == 1)
                        wrappedPrimitive = primitiveA;
                    primitiveA = primitiveB;
                    }
                }
            if (primitiveA.IsValid ())
                result->push_back (primitiveA->Clone ());
            return result;
            }
        }
    return CurveVectorPtr (NULL);
    }

//! Construct a mutiple-radius fillet in a corner.
//! The turn angle is distributed evenly among the radii.
CurveVectorPtr CurveCurve::ConstructMultiRadiusBlend
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] outbound vector on A side.
DVec3dCR   vectorB,         //!< [in] outbound vector on B side.
bvector<double> radii,      //!< [in] radii for fillets
bool reverse                  //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
)
    {
    return ConstructMultiRadiusBlend (corner, vectorA, vectorB, Angle::FromDegrees (0.0), 0.0, radii, 0.0, Angle::FromDegrees (0.0), reverse);
    }
//! Construct a mutiple-radius fillet in a corner.
//! The turn angle is distributed evenly among the radii.
CurveVectorPtr CurveCurve::ConstructMultiRadiusBlend
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] outbound vector on A side.
DVec3dCR   vectorB,         //!< [in] outbound vector on B side.
Angle hardAngleAtStart,     //!< hard angle to turn from start tangent onto start line
double startDistance,       //!< distance to move on start line
bvector<double> radii,      //!< [in] radii for fillets
double endDistance,         //!< distance to move beyond tangent from final arc
Angle hardAngleAtEnd,       //!< hard angle to turn from end tangent to end line
bool reverse                  //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
)
    {
    if (radii.empty ())
        return nullptr;
    DPoint3dDVec3dDVec3d skewBasis (corner, vectorA, vectorB);
    DVec3d vectorA1 = vectorA, currentTangent;
    vectorA1.Negate ();
    double interiorTurnRadians = vectorA1.AngleTo (vectorB);
    double hardTurnRadians = hardAngleAtStart.Radians () + hardAngleAtEnd.Radians ();   // expected to be small. Can be negative
    double totalTurnRadians;
    double radiusSign;
    // set up so   ..
    // currentTangent is the true angle of start tangent.
    // totalTurnRadians is the (positive) total turn
    if (!reverse)
        {
        totalTurnRadians = interiorTurnRadians;        // That's positive
        currentTangent = vectorA1;
        radiusSign = -1.0;
        }
    else
        {
        totalTurnRadians = Angle::TwoPi () - interiorTurnRadians;
        currentTangent = vectorA;
        radiusSign = 1.0;
        }
    auto planeNormal = DVec3d::FromCrossProduct (vectorB, vectorA1);
    // start out along the currentTangent and append successive lines or arcs
    double arcTurnRadians = totalTurnRadians - hardTurnRadians;
    double singleTurnRadians = arcTurnRadians / (double)radii.size ();

    DPoint3d xyz = corner;
    auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    currentTangent.Normalize ();
    if (startDistance != 0.0)
        {
        auto rotatedTangent = DVec3d::FromRotateVectorAroundVector (currentTangent, planeNormal, radiusSign * hardAngleAtStart);
        auto xyz1 = xyz + rotatedTangent * startDistance;
        auto segment = DSegment3d::From (xyz, xyz1);
        curves->push_back (ICurvePrimitive::CreateLine (segment));
        curves->back ()->FractionToPoint (1.0, xyz, currentTangent);    // well, that's identical to the rotated tangent, but maintain the formality of asking the curve.
        }
    for(auto &radius : radii)
        {
        auto arc = DEllipse3d::FromStartTangentNormalRadiusSweep (xyz, currentTangent, planeNormal, radiusSign * radius, singleTurnRadians);
        if (!arc.IsValid ())
            return nullptr;
        curves->push_back (ICurvePrimitive::CreateArc (arc.Value ()));
        curves->back ()->FractionToPoint (1.0, xyz, currentTangent);
        }
    currentTangent.Normalize ();
    if (endDistance != 0.0)
        {
        auto xyz1 = xyz + currentTangent * endDistance;
        auto segment = DSegment3d::From (xyz, xyz1);
        curves->push_back (ICurvePrimitive::CreateLine (segment));
        curves->back ()->FractionToPoint (1.0, xyz, currentTangent); 
        currentTangent = DVec3d::FromRotateVectorAroundVector (currentTangent, planeNormal, radiusSign * hardAngleAtEnd);
        }


    // xyz is the final point.
    // It is not "on" the BC line -- but it only differs by some translation along vectorA to contact with the vectorB axis -- i.e. to u=0.0
    auto uv = skewBasis.ProjectPointToUV (xyz);
    if (!uv.IsValid ())
        return nullptr;
    auto xyz1 = skewBasis.Evaluate (0.0, uv.Value().y);

    auto transform = Transform::From (DVec3d::FromStartEnd (xyz, xyz1));
    curves->TransformInPlace (transform);
    return curves;
    }


//! Construct a sequence of arcs that have given radii and sweeps and join each other with tangent continuity.
CurveVectorPtr CurveCurve::ConstructTangentArcChain
(
DPoint3dCR startPoint,          //!< [in] corner of nominal sharp turn.
DVec3dCR   startTangent,         //!< [in] outbound vector on A side.
DVec3dCR   planeNormal,     //!< [in] normal vector for plane of arc.
bvector<double> radii,       //!< [in] vector of successive radii on the transition.
bvector<Angle> angles       //!< [in] angles for successive arcs.
)
    {
    DPoint3d xyz = startPoint;
    DVec3d currentTangent = startTangent;
    auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for(size_t i = 0; i < radii.size () && i < angles.size (); i++)
        {

        auto arc = DEllipse3d::FromStartTangentNormalRadiusSweep (xyz, currentTangent, planeNormal, radii[i], angles[i].Radians ());
        if (!arc.IsValid ())
            return nullptr;
        curves->push_back (ICurvePrimitive::CreateArc (arc.Value ()));
        curves->back ()->FractionToPoint (1.0, xyz, currentTangent);
        }
    return curves;
    }



//! Construct a line-line-arc-line-line transition within the corner defined by point and vectors.
CurveVectorPtr ConstructSymmetricFilletAndTaper
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] vector to inbound side of blend
DVec3dCR   vectorB,         //!< [in] vector to outbound side of blend
double filletRadius,        //!< [in] fillet radius
double taperSetback,        //!< [in] distance from fillet tangency to nominal curb
double taperDistance        //!< [in] length of (projection of) taper along the nominal curb
)
    {

    double cornerRadians = vectorA.AngleTo (vectorB);
    double bisectorRadians = 0.5 * cornerRadians;
    double taperRadians = atan2 (taperSetback, taperDistance);
    double innerBisectorRadians = bisectorRadians + taperRadians;
    auto unitA = vectorA.ValidatedNormalize ();
    auto unitB = vectorB.ValidatedNormalize ();
    auto unitBisector = DVec3d::FromSumOf (unitA, unitB).ValidatedNormalize ();
    double H1 = hypot (taperSetback, taperDistance);
    auto H2 = DoubleOps::ValidatedDivide (filletRadius, tan(innerBisectorRadians));
    if (   unitA.IsValid ()
        && unitB.IsValid ()
        && unitBisector.IsValid ()
        && H2.IsValid ()
        )
        {
        double H12 = H1 + H2.Value ();
        double T = H12 * sin(taperRadians);
        double E1 = H12 * cos (taperRadians);
        double E2 = T / tan (bisectorRadians);
        double E12 = E1 + E2;
        double Q1 = hypot (E2, T);
        double Q2 = hypot (filletRadius,H2);
        DPoint3d center = DPoint3d::FromSumOf (corner, unitBisector.Value (), (Q1+Q2));
        DPoint3d elbow  = DPoint3d::FromSumOf (corner, unitBisector.Value (), Q1);
        DPoint3d startA = DPoint3d::FromSumOf (corner, unitA.Value (), E12);
        DPoint3d startB = DPoint3d::FromSumOf (corner, unitB.Value (), E12);
        DVec3d unitA1 = DVec3d::FromStartEnd (elbow, startA).ValidatedNormalize ();
        DVec3d unitB1 = DVec3d::FromStartEnd (elbow, startB).ValidatedNormalize ();
        auto tangentA = DPoint3d::FromSumOf (elbow, unitA1, H2);
        auto tangentB = DPoint3d::FromSumOf (elbow, unitB1, H2);
        DEllipse3d filletArc;
        if (filletArc.InitFromArcCenterStartEnd (center, tangentA, tangentB))
            {
            auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (startA, tangentA)));
            curves->push_back (ICurvePrimitive::CreateArc (filletArc));
            curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (tangentB, startB)));
            return curves;
            }
        }
    return nullptr;
    }

DVec3d RotateVectorAroundAxis (DVec3dCR vector, DVec3dCR axis, double radians)
    {
    DVec3d vector1;
    RotMatrix matrix = RotMatrix::FromVectorAndRotationAngle (axis, radians);
    matrix.Multiply (vector1, vector);
    return vector1;
    }
typedef ValidatedValue<DPoint3dDVec3dDVec3d> ValidatedDPoint3dDVec3dDVec3d;

static ValidatedDPoint3dDVec3dDVec3d ComputeOffsetCornerAndUnitVectors (DPoint3dCR corner, DVec3dCR vectorA, DVec3dCR vectorB, double offsetA, double offsetB)
    {
    auto unitA = vectorA.ValidatedNormalize ();
    auto unitB = vectorB.ValidatedNormalize ();
    ValidatedDVec3d planeNormal = DVec3d::FromCrossProduct (vectorA, vectorB).ValidatedNormalize ();

    if (!unitA.IsValid ()  || !unitB.IsValid () || !planeNormal.IsValid ())
        return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (corner, vectorA, vectorB), false);

    if (offsetA == 0.0 && offsetB == 0.0)
        return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (corner, unitA.Value (), unitB.Value ()), true);

    auto perpA = DVec3d::FromCrossProduct (planeNormal.Value (), unitA);
    auto perpB = DVec3d::FromCrossProduct (unitB, planeNormal.Value ());
    DPoint3d startA = corner + perpA * offsetA;
    DPoint3d startB = corner + perpB * offsetB;
    DRay3d rayA = DRay3d::FromOriginAndVector (startA, vectorA);
    DRay3d rayB = DRay3d::FromOriginAndVector (startB, vectorB);
    double gA, gB;
    DPoint3d xyzA, xyzB;
    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (gA, gB, xyzA, xyzB, rayA, rayB))
        {
        DPoint3d xyz = DPoint3d::FromInterpolate (xyzA, 0.5, xyzB);
        return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (xyz, unitA, unitB), true);
        }
    return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (corner, vectorA, vectorB), false);
    }

//! Construct a line-line-arc-line-line transition within the corner defined by point and vectors.
CurveVectorPtr CurveCurve::ConstructTaperFilletTaper
(
DPoint3dCR corner,          //!< [in] corner of nominal sharp turn.
DVec3dCR   vectorA,         //!< [in] vector to inbound side of blend
DVec3dCR   vectorB,         //!< [in] vector to outbound side of blend
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,              //!< [in] taper distance along vectorB
double offsetA,             //!< [in] offset from curveA.
double offsetB              //!< [in] offset from curveA.
)
    {
    auto skewBasis1 = ComputeOffsetCornerAndUnitVectors (corner, vectorA, vectorB, offsetA, offsetB);
    if (!skewBasis1.IsValid ())
        return nullptr;
    DPoint3dDVec3dDVec3d skewBasis = skewBasis1.Value ();
    DPoint3d offsetCorner = skewBasis.origin;
    auto validatedSkewToWorld = skewBasis.LocalToWorldTransform ();
    auto validatedWorldToSkew = skewBasis.WorldToLocalTransform ();
    double taperARadians = atan2 (setbackA, taperA);
    double taperBRadians = atan2 (setbackB, taperB);

    auto unitA = skewBasis.vectorU;
    auto unitB = skewBasis.vectorV;

    double HA = hypot (setbackA, taperA);
    double HB = hypot (setbackB, taperB);
    if (   validatedSkewToWorld.IsValid ()
        && validatedWorldToSkew.IsValid ()
        )
        {
        auto skewToWorld = validatedSkewToWorld.Value ();
        auto worldToSkew = validatedWorldToSkew.Value ();
        DVec3d planeNormal;
        skewToWorld.GetMatrixColumn (planeNormal, 2);
        DVec3d tangent = unitA;
        tangent.Negate ();

        double cornerTurnRadians = unitB.AngleTo (tangent);
        double filletRadians = cornerTurnRadians - taperARadians - taperBRadians;
        DVec3d taperADirection = RotateVectorAroundAxis (tangent, planeNormal, - taperARadians);
        DVec3d taperBDirection = RotateVectorAroundAxis (taperADirection, planeNormal, -filletRadians);
        DPoint3d filletPointA = offsetCorner + taperADirection * HA;
        auto validatedFilletArc = DEllipse3d::FromStartTangentNormalRadiusSweep (filletPointA, taperADirection, planeNormal, -filletRadius, filletRadians);
        if (validatedFilletArc.IsValid ())
            {
            DEllipse3d arc = validatedFilletArc.Value ();
            DPoint3d filletPointB;
            arc.FractionParameterToPoint (filletPointB, 1.0);
            auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (offsetCorner, filletPointA)));
            curves->push_back (ICurvePrimitive::CreateArc (arc));
            DPoint3d endPoint = filletPointB +taperBDirection * HB;
            curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (filletPointB, endPoint)));

            DPoint3d uvw = worldToSkew * endPoint;
            uvw.x = 0.0;    // translate along vectorA
            DPoint3d endPointB = skewToWorld * uvw;

            auto transform = Transform::From (DVec3d::FromStartEnd (endPoint, endPointB));
            curves->TransformInPlace (transform);
            return curves;
            }
        }
    return nullptr;
    }


// interface for generic blend constructions
struct CompoundBlendConstructionObject
{

GEOMAPI_VIRTUAL CurveVectorPtr ConstructBlend (DPoint3dCR corner, DVec3dCR tangentA, DVec3dCR tangentB) = 0;
};

struct TaperFilletTaperBlendConstructionObject : public CompoundBlendConstructionObject
{
TaperFilletTaperBlendConstructionObject (double setbackA, double taperA, double filletRadius, double setbackB, double filletB, double offsetA, double offsetB)
    :
    m_setbackA (setbackA),
    m_taperA (taperA),
    m_filletRadius (filletRadius),
    m_setbackB (setbackB),
    m_taperB (filletB),
    m_offsetA (offsetA),
    m_offsetB (offsetB)
    {}
double m_offsetA;
double m_offsetB;
double m_setbackA;
double m_taperA;
double m_filletRadius;
double m_setbackB;
double m_taperB;

CurveVectorPtr ConstructBlend (DPoint3dCR corner, DVec3dCR tangentA, DVec3dCR tangentB) override 
    {
    return CurveCurve::ConstructTaperFilletTaper (corner, tangentA, tangentB, m_setbackA, m_taperA, m_filletRadius, m_setbackB, m_taperB, m_offsetA, m_offsetB);
    }
};

struct MultiRadiusBlendConstructionObject : public CompoundBlendConstructionObject
{
bvector<double> m_radii;
bool m_reverseBlend;
Angle m_thetaA, m_thetaB;
double m_distanceA, m_distanceB;
MultiRadiusBlendConstructionObject (Angle thetaA, double distanceA, bvector<double> radii, double distanceB, Angle thetaB, bool reverseBlend)
    :
    m_radii (radii),
    m_reverseBlend (reverseBlend),
    m_thetaA (thetaA),
    m_thetaB (thetaB),
    m_distanceA (distanceA),
    m_distanceB (distanceB)
    {}

CurveVectorPtr ConstructBlend (DPoint3dCR corner, DVec3dCR tangentA, DVec3dCR tangentB) override 
    {
    return CurveCurve::ConstructMultiRadiusBlend (corner, tangentA, tangentB, m_thetaA, m_distanceA, m_radii, m_distanceB, m_thetaB, m_reverseBlend);
    }
};

struct ParameterToPointEvaluator 
{
struct IParameterToPointAndUnitTangent
{
public: GEOMAPI_VIRTUAL ValidatedDRay3d ParameterToPointAndUnitTangent (double u) const = 0;
};

struct CurvePrimitiveEvaluator : public IParameterToPointAndUnitTangent
    {
    ICurvePrimitiveCR m_curve;
    public:
    CurvePrimitiveEvaluator (ICurvePrimitiveCR curvePrimitive) : m_curve (curvePrimitive) {}
    ValidatedDRay3d ParameterToPointAndUnitTangent (double u) const override 
        {
        return m_curve.FractionToPointAndUnitTangent (u);
        }
    };

struct CurveVectorWithDistanceIndexEvaluator : public IParameterToPointAndUnitTangent
    {
    CurveVectorWithDistanceIndex const & m_curve;
    public:
    CurveVectorWithDistanceIndexEvaluator (CurveVectorWithDistanceIndex const & curve) : m_curve (curve) {}
    ValidatedDRay3d ParameterToPointAndUnitTangent (double u) const override 
        {
        return m_curve.DistanceAlongToPointAndUnitTangent (u, true);
        }
    };
};
typedef ParameterToPointEvaluator::IParameterToPointAndUnitTangent const &ParameterToPointEvaluatorCR;

// Context struct with function to compute a blend and assess how well it meets requested conditions.
class CompoundBlendFunction : public    FunctionRRToRR
{


ParameterToPointEvaluatorCR m_curveA;
ParameterToPointEvaluatorCR m_curveB;

CurveVectorPtr m_finalBlend;
CompoundBlendConstructionObject &m_blendConstructor;



public:
CompoundBlendFunction (
CompoundBlendConstructionObject &blendConstructor,
ParameterToPointEvaluatorCR curveA,
ParameterToPointEvaluatorCR curveB
)
    :
    m_curveA(curveA),
    m_curveB(curveB),
    m_blendConstructor(blendConstructor)
    {}

CurveVectorPtr ExtractCurves ()
    {
    auto a = m_finalBlend;
    m_finalBlend = nullptr;
    return a;
    }
// Evaluate point and tangent on respective curves at uA, uB.
// Extend the tangents to intersection.
// Invoke the CompoundBlendConstructionFunction to build the blend in that particular corner.
// This blend will touch the corner lines at a point OTHER THAN the requested uA,uB.
// Return the distance along the corner lines as the residual function values.
bool EvaluateRRToRR
(
double uA,   //! fraction along curveA
double uB,  //! fraction along curveB
double &eA,  //! curveA placement error
double &eB   //! curveB placement error
)
    {
    m_finalBlend = nullptr;
    auto tangentA = m_curveA.ParameterToPointAndUnitTangent (uA);
    auto tangentB = m_curveB.ParameterToPointAndUnitTangent (uB);
    if (tangentA.IsValid () && tangentB.IsValid ())
        {
        auto rayA = tangentA.Value ();
        auto rayB = tangentB.Value ();
        DPoint3d vertexA, vertexB;
        double gA, gB;

        if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (gA, gB, vertexA, vertexB, rayA, rayB))
            {
            DPoint3d vertex = DPoint3d::FromInterpolate (vertexA, 0.5, vertexB); // The rays should be coplanar.   But average the approach points just in case.
            // We want the rays to be going away from the vertex, i.e. vertex behind.
            if (rayA.DirectionDotVectorToTarget (vertex) > 0.0)
                rayA.direction.Negate ();
            if (rayB.DirectionDotVectorToTarget (vertex) > 0.0)
                rayB.direction.Negate ();
            auto blend = m_blendConstructor.ConstructBlend (vertex, rayA.direction,  rayB.direction);
            DPoint3d endA, endB;
            if (blend.IsValid ())
                {
                blend->GetStartEnd (endA, endB);
                auto vectorA = DVec3d::FromStartEnd (rayA.origin, endA);
                auto vectorB = DVec3d::FromStartEnd (rayB.origin, endB);

                eA = vectorA.DotProduct (rayA.direction);
                eB = vectorB.DotProduct (rayB.direction);
                m_finalBlend = blend;
                return true;
                }
            }
        }
    return false;
    }
};

//! Search for a symmetric taper and fillet blend near given start fractions
CurveVectorPtr CurveCurve::ConstructTaperFilletTaper
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,              //!< [in] taper distance along vectorB
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,           //!< [in,out] fraction on curveB
double offsetA,
double offsetB
)
    {
    double maxStep = 0.30;
    double tol = Angle::SmallAngle ();
    NewtonIterationsRRToRR newton (tol);
    TaperFilletTaperBlendConstructionObject blendBuilder (setbackA, taperA, filletRadius, setbackB, taperB, offsetA, offsetB);
    ParameterToPointEvaluator::CurvePrimitiveEvaluator curveAEval(curveA);
    ParameterToPointEvaluator::CurvePrimitiveEvaluator curveBEval(curveB);
    CompoundBlendFunction F(blendBuilder, curveAEval, curveBEval);
    
    double fA = fractionA, fB = fractionB;
    double eA, eB;
    if (newton.RunApproximateNewton (fA, fB, F, maxStep, maxStep)
        && F.EvaluateRRToRR (fA, fB, eA, eB)
        )
        {
        fractionA = fA;
        fractionB = fB;
        return F.ExtractCurves ();
        }
    return nullptr;
    }

//! Search for a symmetric taper and fillet blend near given start distances
CurveVectorPtr CurveCurve::ConstructTaperFilletTaper
(
CurveVectorWithDistanceIndexR curveA,    //!< First source set
CurveVectorWithDistanceIndexR curveB,    //!< second source set
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB,              //!< [in] taper distance along vectorB
double &distanceA,          //!< [in,out] distance on curveA
double &distanceB,           //!< [in,out] distance on curveB
double offsetA,
double offsetB
)
    {
    double maxStep = 0.20 * curveA.TotalPathLength ();
    double reltol = Angle::SmallAngle ();
    double bignum = curveA.GetCurveVector ()->FastMaxAbs ()
                  + curveB.GetCurveVector ()->FastMaxAbs ();

    double abstol = reltol * bignum;
    NewtonIterationsRRToRR newton (abstol, reltol, 20, 2, 1.0e-6);
    TaperFilletTaperBlendConstructionObject blendBuilder (setbackA, taperA, filletRadius, setbackB, taperB, offsetA, offsetB);
    ParameterToPointEvaluator::CurveVectorWithDistanceIndexEvaluator evaluatorA (curveA);
    ParameterToPointEvaluator::CurveVectorWithDistanceIndexEvaluator evaluatorB (curveB);
    CompoundBlendFunction F (blendBuilder, evaluatorA, evaluatorB);
    
    double dA = distanceA, dB = distanceB;
    double eA, eB;
    if (newton.RunApproximateNewton (dA, dB, F, maxStep, maxStep)
        && F.EvaluateRRToRR (dA, dB, eA, eB)
        )
        {
        distanceA = dA;
        distanceB = dB;
        return F.ExtractCurves ();
        }
    return nullptr;
    }



CurveVectorPtr CurveCurve::ConstructMultiRadiusBlend
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
bvector<double> radii,      //!< [in] radii for fillets
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,           //!< [in,out] fraction on curveB
bool reverse                  //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
)
    {
    return CurveCurve::ConstructMultiRadiusBlend (curveA, curveB, Angle::FromDegrees (0.0), 0.0, radii, 0.0, Angle::FromDegrees (0.0), fractionA, fractionB, reverse);
    }

//! Search for a multi-radius blend near given start fractions.
CurveVectorPtr CurveCurve::ConstructMultiRadiusBlend
(
ICurvePrimitiveR curveA,    //!< First source set
ICurvePrimitiveR curveB,    //!< second source set
Angle hardAngleAtStart,     //!< hard angle to turn from start tangent onto start line
double startDistance,       //!< distance to move on start line
bvector<double> radii,      //!< [in] radii for fillets
double endDistance,         //!< distance to move beyond tangent from final arc
Angle hardAngleAtEnd,       //!< hard angle to turn from end tangent to end line
double &fractionA,          //!< [in,out] fraction on curveA
double &fractionB,           //!< [in,out] fraction on curveB
bool reverse                  //!<  [in] true to do reverse blend (e.g in a right angle, construct 270 turn initially heading away from the corner)
)
    {
    double maxStep = 0.30;
    double tol = Angle::SmallAngle ();
    NewtonIterationsRRToRR newton (tol);
    MultiRadiusBlendConstructionObject blendBuilder (hardAngleAtStart, startDistance, radii, endDistance, hardAngleAtEnd, reverse);
    ParameterToPointEvaluator::CurvePrimitiveEvaluator evaluatorA (curveA);
    ParameterToPointEvaluator::CurvePrimitiveEvaluator evaluatorB (curveB);
    CompoundBlendFunction F (blendBuilder, evaluatorA, evaluatorB);

    double fA = fractionA, fB = fractionB;
    double eA, eB;
    if (newton.RunApproximateNewton (fA, fB, F, maxStep, maxStep)
        && F.EvaluateRRToRR (fA, fB, eA, eB)
        )
        {
        fractionA = fA;
        fractionB = fB;
        return F.ExtractCurves ();
        }
    return nullptr;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
