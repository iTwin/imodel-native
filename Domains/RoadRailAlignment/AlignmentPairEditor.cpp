/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentPairEditor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

#define BASEGRADEDIFFERENTIAL 1.0
// limit intersection/ramp approach lengths to a factor of the total alignment length
#define APPROACH_STATION_LIMIT_FACTOR 0.4

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
//
// k = L / | G1-G2 |
//
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPVI::KValue () const
    {
    double G1 = AlignmentPairEditor::_Slope (poles[PVC], poles[PVI]);
    double G2 = AlignmentPairEditor::_Slope (poles[PVI], poles[PVT]);
    G1 = G1* 100.0;
    G2 = G2 * 100.0;
    if (fabs (G1 - G2) == 0.0)
        return 0.0;
    return length / fabs (G1 - G2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPVI::LengthFromK (const double& kvalue)
    {
    double G1 = AlignmentPairEditor::_Slope (poles[PVC], poles[PVI]);
    double G2 = AlignmentPairEditor::_Slope (poles[PVI], poles[PVT]);
    G1 = G1 * 100.0;
    G2 = G2 * 100.0;
    return fabs (G1 - G2) * kvalue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment)
    {
    return new AlignmentPairEditor (horizontalAlignment, verticalAlignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::CreateFromSingleCurveVector (CurveVectorCR curveVector)
    {
    AlignmentPairEditorPtr roadAlignment = new AlignmentPairEditor ();
    if (roadAlignment->_GenerateFromSingleVector (curveVector))
        return roadAlignment;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create (AlignmentPairCR roadAlignment)
    {
    CurveVectorPtr hz = roadAlignment.HorizontalCurveVector ();
    CurveVectorPtr vt = roadAlignment.VerticalCurveVector ();
    return new AlignmentPairEditor (*hz, vt.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::CreateVerticalOnly (CurveVectorCR verticalAlignment, bool inXY)
    {
    return new AlignmentPairEditor (verticalAlignment, inXY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditor::AlignmentPairEditor (CurveVectorCR vertical, bool inXY)
    {
    DPoint3d start, end;
    vertical.GetStartEnd (start, end);
    CurveVectorPtr hz = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ICurvePrimitivePtr prim = ICurvePrimitive::CreateLine (DSegment3d::From (0.0, 0.0, 0.0, end.x, 0.0, 0.0));
    hz->Add (prim); // fake hz
    CurveVectorPtr vt = vertical.Clone ();
    if (inXY == true)
        {
        DVec3d u = DVec3d::From (1, 0, 0), v = DVec3d::From (0, 0, 1), w = DVec3d::From (0, 1, 0);
        DPoint3d origin {0.0, 0.0, 0.0};
        Transform flipAxes = Transform::FromOriginAndVectors (origin, u, v, w);
        vt->TransformInPlace (flipAxes);
        }
    UpdateCurveVectors (hz.get(), vt.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditor::AlignmentPairEditor (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment) :
    T_Super (horizontalAlignment, verticalAlignment)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_GenerateFromSingleVector (CurveVectorCR curveVector)
    {
    UpdateCurveVectors (&curveVector, nullptr);
    bvector<AlignmentPI> pis = _GetPIs (); // flatten to zero;
    CurveVectorPtr hzAlign = _BuildVectorFromPIS (pis);
    if (!hzAlign.IsValid ()) return false;

    UpdateCurveVectors (hzAlign.get (), nullptr);
    
    // now create a vertical...
    bvector<AlignmentPVI> pvis;
    DPoint3d start, end;
    curveVector.GetStartEnd (start, end);
    DPoint3d location = DPoint3d::From(0.0, 0.0, start.z);
    AlignmentPVI pvi (location, 0.0);
    pvis.push_back (pvi);
    for (auto primitive : curveVector)
        {
        primitive->GetStartEnd (start, end);
        double x = HorizontalDistanceFromStart (end);
        DPoint3d location2 = DPoint3d::From (x, 0.0, end.z);
        AlignmentPVI pvi2 (location2, 0.0);
        pvis.push_back (pvi);
        }
    CurveVectorPtr vtAlign = _BuildVectorFromPVIS (pvis);
    if (!vtAlign.IsValid ()) return false;
    
    UpdateCurveVectors (hzAlign.get (), vtAlign.get ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::IsHorizontalValid ()
    {
    if (HorizontalCurveVector ().IsValid () && HorizontalCurveVector ()->Length () > 0.0)
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPairEditor::_GetValidEditRange (bvector<AlignmentPVI> const& pvis, int index, double * from, double *to)
    {
    if (index == 0)
        {
        *from = *to = 0.0;
        return;
        }
    if (index == ( pvis.size () - 1 ))
        {
        *from = *to = pvis[index].poles.at (VC::PVI).x;
        return;
        }

    // compute previous range.
    *from = pvis[index - 1].poles.at (VC::PVI).x + ( pvis[index - 1].length * 0.5 ) + ( pvis[index].length * 0.5 );
    // compute next station range
    *to = pvis[index + 1].poles.at (VC::PVI).x - (( pvis[index + 1].length * 0.5 ) + ( pvis[index].length * 0.5 ));

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_ValidatePIs (bvector<AlignmentPI> const& pis)
    {
    for (size_t i = 0; i < pis.size () - 1; i++)
        {
        if (pis.at (i).location.DistanceXY (pis.at (i + 1).location) <
            ( _FullTangentLength (pis.at (i)) + _FullTangentLength (pis.at (i + 1)) ))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_ValidateMove (size_t index, DPoint3d toPt, bvector<AlignmentPI> const& pis)
    {
    if (pis.size () <= 1) return false;
    if (index == 0)
        {
        if (pis.at (index + 1).location.DistanceXY (toPt) < _FullTangentLength (pis.at (index + 1)))
            return false;
        }
    else if (index == ( pis.size () - 1 )) // end point
        {
        if (pis.at (index - 1).location.DistanceXY (toPt) < _FullTangentLength (pis.at (index - 1)))
            return false;
        }
    else
        {
        if (pis.at (index - 1).location.DistanceXY (toPt) <
            ( _FullTangentLength (pis.at (index)) + _FullTangentLength (pis.at (index - 1)) ))
            return false;
        if (pis.at (index + 1).location.DistanceXY (toPt) <
            ( _FullTangentLength (pis.at (index)) + _FullTangentLength (pis.at (index + 1)) ))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::_SolveArcPI (size_t index, bvector<AlignmentPI>& pis)
    {
    if (index == 0 || index == ( pis.size () - 1 )) return ERROR;
    //    const DVec3d v0 = DVec3d::FromStartEnd (pis.at (index).location, pis.at (index - 1).location);
    const DVec3d v0_norm = DVec3d::FromStartEndNormalize (pis.at (index).location, pis.at (index - 1).location);
    const DVec3d v1_norm = DVec3d::FromStartEndNormalize (pis.at (index).location, pis.at (index + 1).location);
    const DVec3d poi = DVec3d::From (pis.at (index).location);

    const double angle = fabs (v0_norm.AngleToXY (v1_norm));
    DVec3d bisector = DVec3d::FromSumOf (v0_norm, v1_norm);
    bisector.ScaleToLength (pis.at (index).arc.radius / sin (angle / 2.0));
    const double tgArcLen = ( pis.at (index).arc.radius / tan (angle / 2.0) );
    DVec3d tangentArc = v0_norm;
    tangentArc.ScaleToLength (tgArcLen);
    tangentArc.Add (poi);

    DVec3d arcTangent = v1_norm;
    arcTangent.ScaleToLength (tgArcLen);
    arcTangent.Add (poi);

    const DVec3d center = DVec3d::FromSumOf (bisector, poi);

    DEllipse3d ellipse;
    if (!ellipse.InitFromArcCenterStartEnd (center, tangentArc, arcTangent))
        return ERROR;
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateArc (ellipse);

    primitive->GetStartEnd (pis.at (index).arc.startPoint, pis.at (index).arc.endPoint);
    pis.at (index).arc.centerPoint = ellipse.center;
    double startAng;
    ellipse.GetSweep (startAng, pis.at (index).arc.sweep);
    if (ellipse.vector0.CrossProductXY (ellipse.vector90) * ellipse.sweep > 0.0)
        pis.at (index).arc.clockwise = false;
    else
        pis.at (index).arc.clockwise = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
*
* "fit" the SCS - potential modifications are the overall PI or the radius
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::_SolveSpiralPI (size_t index, bvector<AlignmentPI>& pis)
    {
    if (index == 0 || index == ( pis.size () - 1 )) return ERROR;
    DPoint3d  lineToSpiralA;
    DPoint3d  lineToSpiralB;
    DPoint3d  spiralAToArc;
    DPoint3d  spiralBToArc;
    DEllipse3d arc;

    DSpiral2dBaseP spiralA = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);

    if (false == DSpiral2dBase::LineSpiralArcSpiralLineTransition (pis.at(index-1).location,
        pis.at(index+1).location, pis.at(index).location, pis.at(index).arc.radius, pis.at(index).spiral1.length,
        pis.at(index).spiral2.length, *spiralA, *spiralB, lineToSpiralA, lineToSpiralB, spiralAToArc, spiralBToArc, arc))
        return ERROR;

    // Validate that the transition is continuous by analyzing the progression of the angles
    const DVec3d v00 = DVec3d::FromStartEnd (pis.at(index).location, pis.at(index-1).location);
    const DVec3d v11 = DVec3d::FromStartEnd (pis.at(index).location, pis.at(index+1).location);
    const DVec3d vec0 = DVec3d::FromStartEnd (pis.at (index).location, spiralAToArc);
    const DVec3d vec1 = DVec3d::FromStartEnd (pis.at (index).location, spiralBToArc);
    const DVec3d vec2 = DVec3d::FromStartEnd (pis.at (index).location, lineToSpiralB);
    const DVec3d orientation = DVec3d::FromCrossProduct (v00, v11);
    const bool sign0 = v00.SignedAngleTo (v11, orientation) >= 0.0;
    const bool sign1 = vec0.SignedAngleTo (vec1, orientation) >= 0.0;
    const bool sign2 = vec1.SignedAngleTo (vec2, orientation) >= 0.0;
    if (sign0 != sign1 || sign0 != sign2)
        {
        return ERROR; // sdevtodo - _SolveSSPI should return false in many cases...
        //pis.at (index).curveType = AlignmentPI::HorizontalPIType::SS;
        //return _SolveSSPI (index, pis);
        }
    const double fractionA = 0;
    const double fractionB = 1;
    const Transform frameA = Transform::From (lineToSpiralA);
    const Transform frameB = Transform::From (spiralBToArc);

    // SpiralB is oriented from tangentB to Arc. Reverse it to make a continuous road path (tangentA to tangentB)
    const double theta0 = spiralB->mTheta0;
    const double theta1 = spiralB->mTheta1;
    const double curvature0 = spiralB->mCurvature0;
    const double curvature1 = spiralB->mCurvature1;
    spiralB->mTheta0 = theta1 + Angle::Pi ();
    spiralB->mTheta1 = theta0 + Angle::Pi ();
    spiralB->mCurvature0 = -curvature1;
    spiralB->mCurvature1 = -curvature0;

    DPoint3d start, end;
    ICurvePrimitivePtr sp1 = ICurvePrimitive::CreateSpiral (*spiralA, frameA, fractionA, fractionB);
    ICurvePrimitivePtr sp2 = ICurvePrimitive::CreateSpiral (*spiralB, frameB, fractionA, fractionB);
    ICurvePrimitivePtr ac = ICurvePrimitive::CreateArc (arc);
    ac->GetStartEnd (start, end);
    pis.at (index).arc.startPoint = start;
    pis.at (index).arc.endPoint = end;
    _LoadSpiralData (sp1.get (), pis.at (index).spiral1);
    _LoadSpiralData (sp2.get (), pis.at (index).spiral2);

    pis.at (index).arc.centerPoint = arc.center;
    double startAng;
    arc.GetSweep (startAng, pis.at (index).arc.sweep);
    if (arc.vector0.CrossProductXY (arc.vector90) * arc.sweep > 0.0)
        pis.at (index).arc.clockwise = false;
    else
        pis.at (index).arc.clockwise = true;
    double radius;
    if (!arc.IsCircularXY(radius))
        {
        BeAssert(false);
        }
    pis.at (index).arc.radius;

    delete spiralA;
    delete spiralB;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
* overall PI changed or Length change
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::_SolveSSPI (size_t index, bvector<AlignmentPI>& pis)
    {
    if (index == 0 || index == ( pis.size () - 1 )) return ERROR;
    double resultRadius;
    DPoint3d  lineToSpiralA;
    DPoint3d  lineToSpiralB;
    DPoint3d  spiralAToArc;

    DSpiral2dBaseP spiralA = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);

    if (false == DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition (pis.at (index-1).location,
        pis.at (index+1).location, pis.at (index).location, pis.at (index).spiral1.length, *spiralA, *spiralB,
        lineToSpiralA, lineToSpiralB, spiralAToArc, resultRadius))
        return ERROR;  // could look at an automatic conversion to SCS

    const double fractionA = 0;
    const double fractionB = 1;
    const Transform frameA = Transform::From (lineToSpiralA);

    // SpiralB is oriented from tangentB to Arc. Reverse it to make a continuous road path (tangentA to tangentB)
    const double theta0 = spiralB->mTheta0;
    const double theta1 = spiralB->mTheta1;
    const double curvature0 = spiralB->mCurvature0;
    const double curvature1 = spiralB->mCurvature1;
    spiralB->mTheta0 = theta1 + Angle::Pi ();
    spiralB->mTheta1 = theta0 + Angle::Pi ();
    spiralB->mCurvature0 = -curvature1;
    spiralB->mCurvature1 = -curvature0;

    ICurvePrimitivePtr sp1 = ICurvePrimitive::CreateSpiral (*spiralA, frameA, fractionA, fractionB);
    const Transform frameB = Transform::From (spiralAToArc);
    ICurvePrimitivePtr sp2 = ICurvePrimitive::CreateSpiral (*spiralB, frameB, fractionA, fractionB);

    _LoadSpiralData (sp1.get(), pis.at (index).spiral1);
    _LoadSpiralData (sp2.get(), pis.at (index).spiral2);

    delete spiralA;
    delete spiralB;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::_SolvePI (size_t index, bvector<AlignmentPI>& pis)
    {
    if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::ARC)
        return _SolveArcPI (index, pis);
    else if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::SCS)
        return _SolveSpiralPI (index, pis);
    else if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::SS)
        {
        StatusInt retVal;
        pis.at (index).curveType = AlignmentPI::HorizontalPIType::SCS;
        if (( retVal = _SolveSpiralPI (index, pis)) == SUCCESS )
            return retVal;
        pis.at (index).curveType = AlignmentPI::HorizontalPIType::SS;
        return _SolveSSPI (index, pis);
        }
    else
        return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_FullTangentLength (AlignmentPICR pi)
    {
    switch (pi.curveType)
        {
            case AlignmentPI::HorizontalPIType::ARC:
                {
                return pi.location.DistanceXY (pi.arc.startPoint);
                }
                break;
            case AlignmentPI::HorizontalPIType::SCS:
                {
                return pi.location.DistanceXY (pi.spiral1.beginSpiralPt);
                }
                break;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr AlignmentPairEditor::CreateSpiralPrimitive (AlignmentSpiral const& spiral)
    {
    Transform placement = Transform::From (spiral.beginSpiralPt);
    const DVec3d v0 = DVec3d::FromStartEnd (spiral.beginSpiralPt, spiral.spiralPIPt);
    const DVec3d v1 = DVec3d::FromStartEnd (spiral.spiralPIPt, spiral.endSpiralPt);
    double startRadians = atan2 (v0.y, v0.x);
    double startRadius = ( fabs (spiral.entranceRadius) > 1e20 ? 0.0 : fabs (spiral.entranceRadius) );
    double delta = v0.AngleToXY (v1);
    double side = delta < 0.0 ? -1.0 : 1.0;
    double endRadius = ( fabs (spiral.exitRadius) > 1e20 ? 0.0 : fabs (spiral.exitRadius) );

    return ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
        DSpiral2dBase::TransitionType_Clothoid, startRadians, side*startRadius, spiral.length, side*endRadius, placement, 0, 1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::_LoadSpiralData (ICurvePrimitiveCP primitive, AlignmentSpiral& roadSpiral)
    {
    if (primitive == nullptr) return ERROR;

    DSpiral2dPlacementCP plSpiral = primitive->GetSpiralPlacementCP ();
    DSpiral2dBaseP vectorSpiral = plSpiral->spiral;

    primitive->GetStartEnd (roadSpiral.beginSpiralPt, roadSpiral.endSpiralPt, roadSpiral.startVector, roadSpiral.endVector);
    roadSpiral.beginSpiralPt.z = 0.0; roadSpiral.endSpiralPt.z = 0.0;
    DRay3d rayA = DRay3d::FromOriginAndVector (roadSpiral.beginSpiralPt, roadSpiral.startVector);
    DRay3d rayB = DRay3d::FromOriginAndVector (roadSpiral.endSpiralPt, roadSpiral.endVector);
    DPoint3d pi2;
    double fractionA, fractionB;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, roadSpiral.spiralPIPt, pi2, rayA, rayB);
    if (vectorSpiral->mCurvature0 == 0.0)
        {
        if (vectorSpiral->mCurvature1 > 0.0)
            roadSpiral.entranceRadius = -1.0 * CS_SPI_INFINITY;
        else
            roadSpiral.entranceRadius = CS_SPI_INFINITY;
        }
    else
        roadSpiral.entranceRadius = -1.0 / vectorSpiral->mCurvature0;

    if (vectorSpiral->mCurvature1 == 0.0)
        {
        if (vectorSpiral->mCurvature0 > 0.0)
            roadSpiral.exitRadius = -1.0 * CS_SPI_INFINITY;
        else
            roadSpiral.exitRadius = CS_SPI_INFINITY;
        }
    else
        roadSpiral.exitRadius = -1.0 / vectorSpiral->mCurvature1;
    roadSpiral.length = vectorSpiral->mLength;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::GetAlignmentPIs (bvector<AlignmentPI>& pis)
    {
    pis = _GetPIs ();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ben.Bartholomew                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::GetAlignmentPI (size_t index, AlignmentPI& pi)
    {
    auto pis = _GetPIs ();
    if (index < pis.size())
        {
        pi = pis.at(index);
        return SUCCESS;
        }

    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AlignmentPI> AlignmentPairEditor::_GetPIs ()
    {
    bvector<AlignmentPI> pis;
    DPoint3d startPt, endPt;
    HorizontalCurveVector()->GetStartEnd (startPt, endPt);
    startPt.z = 0.0; endPt.z = 0.0;
    AlignmentPI startPI (startPt);
    pis.push_back (startPI);
    bool addEndPI = true;
    for (int i = 0; i < HorizontalCurveVector()->size (); i++)
        {
        switch (HorizontalCurveVector()->at (i)->GetCurvePrimitiveType ())
            {
            // unless should be that only Arcs have PIs,
            // will be a simple PI unless, we have an SCS
            // situation, in which case we can iterate it on
            // the first spiral that we come to
            // 9/30 we need to support PIs that were inserted
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    // only in a line/line situation
                    if (i < HorizontalCurveVector ()->size () - 1) // Not last
                        {
                        if (HorizontalCurveVector ()->at (i + 1)->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
                            {
                            DPoint3d start, end;
                            HorizontalCurveVector ()->at (i)->GetStartEnd (start, end);
                            start.z = 0.0; end.z = 0.0;
                            AlignmentPI piPt;
                            piPt.curveType = AlignmentPI::HorizontalPIType::NONE;
                            piPt.location = end;

                            pis.push_back (piPt);
                            }
                        }
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DPoint3d start, end;
                    DVec3d startTangent, endTangent;
                    HorizontalCurveVector()->at (i)->GetStartEnd (start, end, startTangent, endTangent);
                    start.z = 0.0; end.z = 0.0;
                    DRay3d rayA = DRay3d::FromOriginAndVector (start, startTangent);
                    DRay3d rayB = DRay3d::FromOriginAndVector (end, endTangent);
                    rayA.direction.z = 0.0; rayB.direction.z = 0.0;
                    DPoint3d pi, pi2;
                    double fractionA, fractionB;
                    DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, pi, pi2, rayA, rayB);

                    AlignmentPI piPt (pi);
                    piPt.curveType = AlignmentPI::HorizontalPIType::ARC;

                    HorizontalCurveVector()->at (i)->GetStartEnd (piPt.arc.startPoint, piPt.arc.endPoint);
                    DEllipse3dCP arc = HorizontalCurveVector()->at (i)->GetArcCP ();
                    piPt.arc.centerPoint = arc->center;
                    piPt.arc.centerPoint.z = 0.0;
                    double startAng;
                    arc->GetSweep (startAng, piPt.arc.sweep);
                    if (arc->vector0.CrossProductXY (arc->vector90) * arc->sweep > 0.0)
                        piPt.arc.clockwise = false;
                    else
                        piPt.arc.clockwise = true;
                    if (arc->IsCircularXY (piPt.arc.radius) == false)
                        break;  // not sure what happens now

                    pis.push_back (piPt);

                    if (piPt.arc.endPoint.AlmostEqualXY (endPt))
                        addEndPI = false;
                    }
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    // see if it's ss or SCS
                    if (i >= HorizontalCurveVector ()->size () - 1) break; //ack!
                    if (HorizontalCurveVector ()->at (i + 1)->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral)
                        {
                        AlignmentPI piPt;
                        piPt.curveType = AlignmentPI::HorizontalPIType::SS;
                        _LoadSpiralData (( HorizontalCurveVector ()->at (i) ).get (), piPt.spiral1);
                        auto placementSpiral = HorizontalCurveVector ()->at (i)->GetSpiralPlacementCP ();
                        DSpiral2dBaseP spiral = placementSpiral->spiral;
                        i++;
                        _LoadSpiralData (( HorizontalCurveVector ()->at (i) ).get (), piPt.spiral2);
                        // compute the "overall" PI
                        DRay3d rayA = DRay3d::FromOriginAndVector (piPt.spiral1.beginSpiralPt, piPt.spiral1.startVector);
                        DRay3d rayB = DRay3d::FromOriginAndVector (piPt.spiral2.endSpiralPt, piPt.spiral2.endVector);
                        rayA.direction.z = 0.0; rayB.direction.z = 0.0;
                        double fractionA, fractionB;
                        DPoint3d pi2;
                        DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, piPt.location, pi2, rayA, rayB);
                         
                        if (spiral->mCurvature1 != 0.0)
                            {
                            piPt.arc.radius = 1.0 / spiral->mCurvature1;
                            }
                        pis.push_back (piPt);
                        if (piPt.spiral2.endSpiralPt.AlmostEqualXY (endPt))
                            addEndPI = false;
                        }
                    else
                        {
                        // use 3 primitives as we only support SCS
                        // can get the overall PI 
                        AlignmentPI piPt;
                        piPt.curveType = AlignmentPI::HorizontalPIType::SCS;

                        _LoadSpiralData (( HorizontalCurveVector ()->at (i) ).get (), piPt.spiral1);
                        i++;
                        HorizontalCurveVector ()->at (i)->GetStartEnd (piPt.arc.startPoint, piPt.arc.endPoint);
                        DEllipse3dCP arc = HorizontalCurveVector ()->at (i)->GetArcCP ();
                        BeAssert(arc != nullptr);
                        piPt.arc.centerPoint = arc->center;
                        double startAng;
                        arc->GetSweep (startAng, piPt.arc.sweep);
                        i++;
                        _LoadSpiralData (( HorizontalCurveVector ()->at (i) ).get (), piPt.spiral2);

                        // compute the "overall" PI
                        DRay3d rayA = DRay3d::FromOriginAndVector (piPt.spiral1.beginSpiralPt, piPt.spiral1.startVector);
                        DRay3d rayB = DRay3d::FromOriginAndVector (piPt.spiral2.endSpiralPt, piPt.spiral2.endVector);
                        rayA.direction.z = 0.0; rayB.direction.z = 0.0;
                        double fractionA, fractionB;
                        DPoint3d pi2;
                        DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, piPt.location, pi2, rayA, rayB);

                        if (arc->vector0.CrossProductXY (arc->vector90) * arc->sweep > 0.0)
                            piPt.arc.clockwise = false;
                        else
                            piPt.arc.clockwise = true;
                        if (arc->IsCircularXY (piPt.arc.radius) == false)
                            break;  // not sure what happens now

                        pis.push_back (piPt);
                        if (piPt.spiral2.endSpiralPt.AlmostEqualXY (endPt))
                            addEndPI = false;
                        }
                    }
                    break;
            }
        }
    if (addEndPI)
        pis.push_back (endPt);

    return pis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
// only curve types!
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::AddPrimitivesFromPI (CurveVectorR hvec, const AlignmentPI& pi, DPoint3dR lastPoint)
    {
    switch (pi.curveType)
        {
            case AlignmentPI::HorizontalPIType::NONE:
                lastPoint = pi.location;
                return true;
            case AlignmentPI::HorizontalPIType::ARC:
                {
                DEllipse3d ell = DEllipse3d::FromArcCenterStartEnd (pi.arc.centerPoint,
                                                                    pi.arc.startPoint, pi.arc.endPoint);
                if (fabs (pi.arc.sweep) > PI)
                    ell.ComplementSweep ();
                hvec.Add (ICurvePrimitive::CreateArc (ell));

                lastPoint = pi.arc.endPoint;
                }
                return true;
            case AlignmentPI::HorizontalPIType::SS:
                {
                double resultRadius;
                DPoint3d  lineToSpiralA;
                DPoint3d  lineToSpiralB;
                DPoint3d  spiralAToArc;

                DSpiral2dBaseP spiralA = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);
                DSpiral2dBaseP spiralB = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);

                if (false == DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition (pi.spiral1.beginSpiralPt,
                    pi.spiral2.endSpiralPt, pi.location, pi.spiral1.length, *spiralA, *spiralB,
                    lineToSpiralA, lineToSpiralB, spiralAToArc, resultRadius))
                    break;

                const double fractionA = 0;
                const double fractionB = 1;
                const Transform frameA = Transform::From (lineToSpiralA);

                // SpiralB is oriented from tangentB to Arc. Reverse it to make a continuous road path (tangentA to tangentB)
                const double theta0 = spiralB->mTheta0;
                const double theta1 = spiralB->mTheta1;
                const double curvature0 = spiralB->mCurvature0;
                const double curvature1 = spiralB->mCurvature1;
                spiralB->mTheta0 = theta1 + Angle::Pi ();
                spiralB->mTheta1 = theta0 + Angle::Pi ();
                spiralB->mCurvature0 = -curvature1;
                spiralB->mCurvature1 = -curvature0;

                hvec.Add (ICurvePrimitive::CreateSpiral (*spiralA, frameA, fractionA, fractionB));
                const Transform frameB = Transform::From (spiralAToArc);
                hvec.Add (ICurvePrimitive::CreateSpiral (*spiralB, frameB, fractionA, fractionB));

                delete spiralA;
                delete spiralB;
                }
                return true;
            case AlignmentPI::HorizontalPIType::SCS:
                {

                ICurvePrimitivePtr spiralP = CreateSpiralPrimitive (pi.spiral1);
                if (spiralP == nullptr)
                    {
                    hvec.Add (
                        ICurvePrimitive::CreateLine (DSegment3d::From (pi.spiral1.beginSpiralPt,
                        pi.spiral1.endSpiralPt)));
                    }
                else
                    hvec.Add (spiralP);

                DEllipse3d ell = DEllipse3d::FromArcCenterStartEnd (pi.arc.centerPoint,
                                                                    pi.arc.startPoint, pi.arc.endPoint);
                if (fabs (pi.arc.sweep) > PI)
                    ell.ComplementSweep ();
                hvec.Add (ICurvePrimitive::CreateArc (ell));

                spiralP = CreateSpiralPrimitive (pi.spiral2);
                if (spiralP == nullptr)
                    {
                    hvec.Add (
                        ICurvePrimitive::CreateLine (DSegment3d::From (pi.spiral2.beginSpiralPt,
                        pi.spiral2.endSpiralPt)));
                    }
                else
                    hvec.Add (spiralP);

                lastPoint = pi.spiral2.endSpiralPt;
                }
                return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::_BuildVectorFromPIS (bvector<AlignmentPI> const& pis)
    {
    if (pis.size () == 0)
        return nullptr;
    CurveVectorPtr horizontalAlignment = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d lastPoint;
    AddPrimitivesFromPI (*horizontalAlignment, pis.at (0), lastPoint);
    
    for (int i = 1; i < pis.size (); i++)
        {
        switch (pis.at (i).curveType)
            {
                case AlignmentPI::HorizontalPIType::NONE:
                    { // just do the back tangent, unless next is none also..
                    horizontalAlignment->Add (
                        ICurvePrimitive::CreateLine (
                        DSegment3d::From (lastPoint, pis.at (i).location)));
                    lastPoint = pis.at (i).location;
                    }
                    break;
                case AlignmentPI::HorizontalPIType::ARC:
                case AlignmentPI::HorizontalPIType::SCS:
                case AlignmentPI::HorizontalPIType::SS:
                    {
                    if (pis.at (i).curveType == AlignmentPI::ARC)
                        {
                        if (!lastPoint.AlmostEqualXY (pis.at (i).arc.startPoint))
                            {
                            horizontalAlignment->Add (
                                ICurvePrimitive::CreateLine (
                                DSegment3d::From (lastPoint, pis.at (i).arc.startPoint)));
                            }
                        }
                    else
                        {
                        if (!lastPoint.AlmostEqualXY (pis.at (i).spiral1.beginSpiralPt))
                            {
                            horizontalAlignment->Add (
                                ICurvePrimitive::CreateLine (
                                DSegment3d::From (lastPoint, pis.at (i).spiral1.beginSpiralPt)));
                            }
                        }
                    AddPrimitivesFromPI (*horizontalAlignment, pis.at (i), lastPoint);
                    }
                    break;
            }
        }
    return horizontalAlignment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d AlignmentPairEditor::_GetZeroSlopePoint (MSBsplineCurveCP vcurve, double runningLength)
    {
    double x = _VcurveZeroSlopeDistance (vcurve);
    DPoint3d pt = DPoint3d::From (-1.0, 0.0, 0.0);
    if (x >= 0.0)
        {
        pt.x = x + runningLength;
        pt.y = 0.0;
        pt.z = GeometryHelper::GetVerticalElevationAtStation (*VerticalCurveVector(), pt.x);
        }
    return pt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
*
*  x = G1*L        returns the point distance along the sag/crest curve for the low/high point
*      -----                (point at which there is 0% slope)
*      G1 - G2
*
* -1.0 for no solution
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_VcurveZeroSlopeDistance (MSBsplineCurveCP vcurve)
    {
    VerticalCurveType vType = _GetVerticalCurveType (vcurve);
    if (vType != VerticalCurveType::TypeIII && vType != VerticalCurveType::TypeI)
        return -1.0;

    size_t count = vcurve->GetNumPoles ();
    if (count != 3)
        return -1.0; // invalid curve
    bvector<DPoint3d> poles;
    vcurve->GetPoles (poles);
    double G1 = AlignmentPairEditor::_Slope (poles[0], poles[1]) * 100.0; // formula wants G1 and G2 in %
    double G2 = AlignmentPairEditor::_Slope (poles[1], poles[2]) * 100.0;
    double L = fabs (poles[2].x - poles[0].x);
    if (fabs (G1 - G2) > DBL_EPSILON)
        {
        return ( G1 * L ) / ( G1 - G2 );
        }
    return -1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_Slope (DPoint3d p1, DPoint3d p2)
    {
    if (fabs (p1.z - p2.z) > DBL_EPSILON)
        {
        if (fabs (p1.x - p2.x) > DBL_EPSILON)
            {
            return ( p1.z - p2.z ) / ( p1.x - p2.x );
            }
        return DBL_MAX;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalCurveType AlignmentPairEditor::_GetVerticalCurveType (MSBsplineCurveCP vcurve)
    {
    size_t count = vcurve->GetNumPoles ();
    if (count != 3)
        {
        return VerticalCurveType::Invalid;
        }
    bvector<DPoint3d> poles;
    vcurve->GetPoles (poles);
    double G1 = AlignmentPairEditor::_Slope (poles[0], poles[1]);
    double G2 = AlignmentPairEditor::_Slope (poles[1], poles[2]);
    return _GetVerticalCurveType (G1, G2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalCurveType AlignmentPairEditor::_GetVerticalCurveType (double G1, double G2)
    {
    if (G1 == G2) return VerticalCurveType::Linear;
    if (G1 > 0.0 && G2 < 0.0) return VerticalCurveType::TypeI;
    if (G1 < 0.0 && G2 > 0.0) return VerticalCurveType::TypeIII;
    if (G1 > 0.0 && G2 > 0.0 && fabs(G1) > fabs(G2)) return VerticalCurveType::TypeII;
    if (G1 < 0.0 && G2 < 0.0 && fabs(G1) < fabs(G2)) return VerticalCurveType::TypeII;
    return VerticalCurveType::TypeIV;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_StationCompare (DPoint3d p, DPoint3d p1)
    {
    return _StationCompare (p.x, p1.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_StationCompare (double x, double x1)
    {
    return ( fabs (x - x1) > DBL_EPSILON ) ? false : true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AlignmentPVI> AlignmentPairEditor::_GetPVIs ()
    {
    bvector<AlignmentPVI> pvis;
    CurveVectorPtr vt = VerticalCurveVector ();
    if (!vt.IsValid ())
        return pvis;
    return _GetPVIs (*vt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AlignmentPVI> AlignmentPairEditor::_GetPVIs (CurveVectorR vt)
    {
    bvector<AlignmentPVI> pvis;
    int count = 0;
    DPoint3d start = DPoint3d::From (0.0, 0.0, 0.0), end = DPoint3d::From (0.0, 0.0, 0.0), lastPt = DPoint3d::From (0.0, 0.0, 0.0);
    
    ICurvePrimitive::CurvePrimitiveType lastCurveType = ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve;
    for (auto primitive : vt)
        {
        primitive->GetStartEnd (start, end);
        if (count == 0)
            {
            AlignmentPVI startpvi (start, 0.0);
            pvis.push_back (startpvi);
            lastPt = start;
            }
        switch (primitive->GetCurvePrimitiveType ())
            {
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
                    if (lastCurveType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
                        {
                        DPoint3d segStart, segEnd;
                        primitive->GetStartEnd (segStart, segEnd);
                        AlignmentPVI apvi (segStart, segStart, segStart, 0.0);
                        if (!_StationCompare (apvi.poles[PVI].x, lastPt.x))
                            {
                            pvis.push_back (apvi);;
                            lastPt = segStart;
                            }
                        }
                    break;
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    {
                    MSBsplineCurveCP bspline = primitive->GetBsplineCurveCP ();
                    size_t count = bspline->GetNumPoles ();
                    if (count == 3)
                        {
                        bvector<DPoint3d> poles;
                        bspline->GetPoles (poles);
                        AlignmentPVI apvi (poles.at (1), poles.at (0), poles.at (2), poles.at (0).DistanceXY (poles.at (2)));
                        pvis.push_back (apvi);;
                        lastPt = poles.at (1);
                        }
                    }
                    break;
            }
        lastCurveType = primitive->GetCurvePrimitiveType ();
        count++;
        }
    if (count > 0)
        {
        if (!_StationCompare (end, lastPt))
            {
            AlignmentPVI endPvi (end, 0.0);
            endPvi.poles[PVC].x = endPvi.poles[PVT].x = endPvi.poles[PVI].x;
            pvis.push_back (endPvi); // add the end point
            }
        }
    return pvis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::_BuildVectorFromPVIS (bvector<AlignmentPVI> pvis, double matchLen)
    {
    // force last PVI to always equal horizontal length...
    CurveVectorPtr verticalAlignment = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    if (pvis.size () == 2) // special case;
        {
        if (matchLen > 0.0)
            pvis[1].poles[PVI].x = matchLen;
        verticalAlignment->Add (
            ICurvePrimitive::CreateLine (
            DSegment3d::From (pvis[0].poles[PVI], pvis[1].poles[PVI])));
        }
    else
        {
        for (int i = 1; i < pvis.size (); i++)
            {
            if (i == 1)
                {
                if (_StationCompare (pvis[i - 1].poles[PVI], pvis[i].poles[PVC]) == false)
                    {
                    verticalAlignment->Add (
                        ICurvePrimitive::CreateLine (
                        DSegment3d::From (pvis[i - 1].poles[PVI], pvis[i].poles[PVC])));
                    }
                }
            else
                {
                if (i < pvis.size () - 1) // not last point
                    {
                    if (_StationCompare (pvis[i - 1].poles[PVT], pvis[i].poles[PVC]) == false)
                        {
                        verticalAlignment->Add (
                            ICurvePrimitive::CreateLine (
                            DSegment3d::From (pvis[i - 1].poles[PVT], pvis[i].poles[PVC])));
                        }
                    }
                }
            if (pvis[i].length > 0.0) // insert a vertical curve
                {
                if (matchLen > 0.0)
                    {
                    if (pvis[i].poles[PVT].x > matchLen)
                        pvis[i].poles[PVT].x = matchLen;
                    }
                MSBsplineCurvePtr parabolaCurve = MSBsplineCurve::CreateFromPolesAndOrder (pvis[i].poles, NULL, NULL, 3, false, false);
                ICurvePrimitivePtr parabolaPrimitive = ICurvePrimitive::CreateBsplineCurve (parabolaCurve);
                verticalAlignment->Add (parabolaPrimitive);
                }
            if (i == pvis.size () - 1) // last point
                {
                if (matchLen > 0.0)
                    pvis[i].poles[PVI].x = matchLen;
                if (_StationCompare (pvis[i - 1].poles[PVT], pvis[i].poles[PVI]) == false)
                    {
                    verticalAlignment->Add (
                        ICurvePrimitive::CreateLine (
                        DSegment3d::From (pvis[i - 1].poles[PVT], pvis[i].poles[PVI])));
                    }
                }
            }
        }
    if (verticalAlignment->size () > 0)
        return verticalAlignment;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePI (size_t index, DPoint3d toPt, AlignmentPIR outPi, double minRadius, bvector<AlignmentPI>* pisP, AlignmentPI::HorizontalPIType piType)
    {
    toPt.z = 0.0;

    // Retrieve pis if not passed in
    bvector<AlignmentPI> pis;

    if (NULL == pisP)
        pis = _GetPIs();
    else
        pis = *pisP;

    // !!!! special case, not the beginning or end!
    DPoint3d cachePt = pis.at (index).location;
    if (index != 0 && index != pis.size () - 1) 
        {
        if (minRadius > 0.0)
            {
            if (pis.at (index).curveType == AlignmentPI::NONE)
                {
                pis.at (index).location = toPt;
                pis.at (index).curveType = piType;
                pis.at (index).arc.radius = minRadius;
                if (_SolvePI (index, pis) == ERROR)
                    {
                    pis.at (index).curveType = AlignmentPI::NONE;
                    pis.at (index).arc.radius = 0.0;
                    pis.at (index).location = cachePt;
                    }
                }
            }
        }
    if (_ValidateMove (index, toPt, pis) == false)
        {
        pis.at (index).location = cachePt;
        return nullptr;
        }

    pis.at (index).location = toPt;
    StatusInt retCode = SUCCESS;
    if (index == 0) // special...
        {
        retCode = _SolvePI (index + 1, pis);
        }
    else if (index == pis.size () - 1) // end point,special
        {
        retCode = _SolvePI (index - 1, pis);
        }
    else
        {
        if (( retCode = _SolvePI (index, pis) ) == SUCCESS)
            {
            if (( retCode = _SolvePI (index - 1, pis) ) == SUCCESS)
                {
                retCode = _SolvePI (index + 1, pis);
                }
            }
        }
    if (retCode != SUCCESS)
        return nullptr;

    if (_ValidatePIs (pis) == false)
        {
        pis.at (index).location = cachePt;
        return nullptr;
        }

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePC (size_t index, DPoint3d toPt, AlignmentPIR outPi)
    { 
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::NONE || pis.at (index).arc.radius == 0.0) return nullptr;
    if (index == 0 || index >= ( pis.size () - 1 )) return nullptr; // todo start and end?

    // validate that this will fit
    double tangentLength = toPt.DistanceXY (pis.at (index).location);
    double fullLength = pis.at (index).location.DistanceXY (pis.at (index - 1).location);
    double prevtangent = 0.0;
    if (pis.at (index-1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        prevtangent = pis.at (index - 1).location.DistanceXY (pis.at (index - 1).arc.endPoint);
        }
    if (tangentLength + prevtangent > fullLength) return nullptr;

    fullLength = pis.at(index).location.DistanceXY(pis.at(index + 1).location);
    double nexttangent = 0.0;
    if (pis.at(index + 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        nexttangent = pis.at(index + 1).location.DistanceXY(pis.at(index + 1).arc.startPoint);
        }
    if (tangentLength + nexttangent > fullLength) return nullptr;

    // compute a new radius then just solve the PI
    // T = R tan I/2
    double taniover2 = pis.at (index).location.DistanceXY (pis.at (index).arc.startPoint) / pis.at (index).arc.radius;
    // update the radius R = T/tani/2
    pis.at (index).arc.radius = tangentLength / taniover2;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;
    if (!_ValidatePIs (pis))
        return nullptr;

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::UpdateRadius (size_t index, double radius, AlignmentPIR outPi, bool validate)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (index == 0 || index >= ( pis.size () - 1 )) return nullptr; // todo start and end?

    pis.at (index).arc.radius = radius;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    if (validate && !_ValidatePIs (pis))
        return nullptr;

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::RemoveSpirals (size_t index, AlignmentPIR outPi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (index == 0 || index >= ( pis.size () - 1 )) return nullptr; // todo start and end?

    pis.at (index).curveType = AlignmentPI::HorizontalPIType::ARC;
    pis.at (index).spiral1.length = 0.0;
    pis.at (index).spiral2.length = 0.0;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::AddSpirals (size_t index, double length, AlignmentPIR outPi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (index == 0 || index >= ( pis.size () - 1 )) return nullptr; // todo start and end?

    pis.at (index).curveType = AlignmentPI::HorizontalPIType::SCS;
    pis.at (index).spiral1.length = length;
    pis.at (index).spiral2.length = length;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    if (!_ValidatePIs (pis))
        return nullptr;

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPI::HorizontalPIType AlignmentPairEditor::GetHorizontalPIType (size_t index)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (/*index < 0 || */index >= pis.size ()) return AlignmentPI::HorizontalPIType::NONE;

    return pis[index].curveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePT (size_t index, DPoint3d toPt, AlignmentPIR outPi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::NONE || pis.at (index).arc.radius == 0.0) return nullptr;
    if (index == 0 || index >= ( pis.size () - 1 )) return nullptr; // todo start and end?
    
    // validate that this will fit
    double tangentLength = toPt.DistanceXY(pis.at(index).location);
    double fullLength = pis.at(index).location.DistanceXY(pis.at(index - 1).location);
    double prevtangent = 0.0;
    if (pis.at(index - 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        prevtangent = pis.at(index - 1).location.DistanceXY(pis.at(index - 1).arc.endPoint);
        }
    if (tangentLength + prevtangent > fullLength) return nullptr;
    
    fullLength = pis.at (index).location.DistanceXY (pis.at (index + 1).location);
    double nexttangent = 0.0;
    if (pis.at (index + 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        nexttangent = pis.at (index + 1).location.DistanceXY (pis.at (index + 1).arc.startPoint);
        }
    if (tangentLength + nexttangent > fullLength) return nullptr;

    // compute a new radius then just solve the PI
    // T = R tan I/2
    double taniover2 = pis.at (index).location.DistanceXY (pis.at (index).arc.endPoint) / pis.at (index).arc.radius;
    // update the radius R = T/tani/2
    pis.at (index).arc.radius = tangentLength / taniover2;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveBS (size_t index, DPoint3d toPt, AlignmentPIR outPi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::NONE || pis.at (index).arc.radius == 0.0) return nullptr;
    if (index == 0 || index >= ( pis.size () - 1 ))
        return nullptr; // todo start and end?
                                      
    // validate that this will fit
    double tangentLength = toPt.DistanceXY (pis.at (index).location);
    double fullLength = pis.at (index).location.DistanceXY (pis.at (index - 1).location);
    double prevtangent = 0.0;
    if (pis.at (index - 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        prevtangent = pis.at (index - 1).location.DistanceXY (pis.at (index - 1).arc.endPoint);
        }
    if (tangentLength + prevtangent > fullLength)
        return nullptr;

    fullLength = pis.at(index).location.DistanceXY(pis.at(index + 1).location);
    double nexttangent = 0.0;
    if (pis.at(index + 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        nexttangent = pis.at(index + 1).location.DistanceXY(pis.at(index + 1).arc.startPoint);
        }
    if (tangentLength + nexttangent > fullLength)
        return nullptr;

    // compute a new spiral length then just solve the PI
    double spiralLength = toPt.DistanceXY (pis.at (index).spiral1.endSpiralPt); // this is close enough, perhaps round?
    pis.at (index).spiral1.length = spiralLength;
    pis.at (index).spiral2.length = spiralLength;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    if (pis.at(index).curveType == AlignmentPI::HorizontalPIType::SCS)
        {
        if (pis.at(index).spiral1.endSpiralPt.DistanceXY(toPt) >= pis.at(index).arc.endPoint.DistanceXY(toPt))
            return nullptr;
        }

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveES (size_t index, DPoint3d toPt, AlignmentPIR outPi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.at (index).curveType == AlignmentPI::HorizontalPIType::NONE || pis.at (index).arc.radius == 0.0) return nullptr;
    if (index == 0 || index >= ( pis.size () - 1 ))
        return nullptr; // todo start and end?

    // validate that this will fit
    double tangentLength = toPt.DistanceXY (pis.at (index).location);
    double fullLength = pis.at (index).location.DistanceXY (pis.at (index + 1).location);
    double nexttangent = 0.0;
    if (pis.at (index + 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        nexttangent = pis.at (index + 1).location.DistanceXY (pis.at (index + 1).arc.startPoint);
        }
    if (tangentLength + nexttangent > fullLength)
        return nullptr;

    fullLength = pis.at(index).location.DistanceXY(pis.at(index - 1).location);
    double prevtangent = 0.0;
    if (pis.at(index - 1).curveType != AlignmentPI::HorizontalPIType::NONE)
        {
        prevtangent = pis.at(index - 1).location.DistanceXY(pis.at(index - 1).arc.endPoint);
        }
    if (tangentLength + prevtangent > fullLength)
        return nullptr;

    // compute a new spiral length then just solve the PI
    double spiralLength = toPt.DistanceXY (pis.at (index).spiral2.beginSpiralPt); // this is close enough, perhaps round?
    pis.at (index).spiral1.length = spiralLength;
    pis.at (index).spiral2.length = spiralLength;
    if (_SolvePI (index, pis) != SUCCESS)
        return nullptr;

    if (pis.at(index).curveType == AlignmentPI::HorizontalPIType::SCS)
        {
        if (pis.at(index).spiral1.endSpiralPt.DistanceXY(toPt) <= pis.at(index).arc.endPoint.DistanceXY(toPt))
            return nullptr;

        if (pis.at(index).spiral2.beginSpiralPt.DistanceXY(toPt) >= pis.at(index).arc.startPoint.DistanceXY(toPt))
            return nullptr;
        }

    memcpy (&outPi, &pis.at (index), sizeof (AlignmentPI));
    CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
    if (hzGeom.IsValid ())
        return hzGeom;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPI (DPoint3d piPt)
    {
    // find the index to insert this point
    bvector<AlignmentPI> pis = _GetPIs ();
    // first find nearest point
    size_t nearestIndex = 0;
    double distance = DBL_MAX;
    for (size_t i = 0; i < pis.size (); i++)
        {
        double testDistance = piPt.DistanceXY (pis.at (i).location);
        if (testDistance < distance)
            {
            nearestIndex = i;
            distance = testDistance;
            }
        }
    if (distance == DBL_MAX) return nullptr;
    AlignmentPI roadPI;
    roadPI.curveType = AlignmentPI::NONE;
    roadPI.location = piPt;
    size_t index;
    // test the direction...
    if (nearestIndex == 0)
        index = 1;
    else if (nearestIndex == ( pis.size () - 1 ))
        index = pis.size () - 1;
    else
        {
        double nearestDistFromStart = this->HorizontalDistanceFromStart(pis.at(nearestIndex).location, nullptr);
        double newPiDistFromStart = this->HorizontalDistanceFromStart(piPt);
        if (newPiDistFromStart < nearestDistFromStart)
            index = nearestIndex;
        else
            index = nearestIndex + 1;
        }

    return InsertPI (index, roadPI);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPI (size_t index, AlignmentPIR pi)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    pis.insert (pis.begin () + index, pi);

    if (index != 0)
        _SolvePI (index - 1, pis);
    _SolvePI (index, pis);
    if (index != pis.size () - 1)
        _SolvePI (index + 1, pis);

    if (_ValidatePIs (pis))
        {
        CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
        if (hzGeom.IsValid ())
            return hzGeom;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePI (DPoint3d pviPt)
    {
    bvector<DPoint3d> points;
    if (GetPIPoints (points) == SUCCESS)
        {
        for (size_t i = 0; i < points.size (); i++)
            {
            if (points.at (i).AlmostEqualXY (pviPt) == true)
                {
                return DeletePI (i);
                }
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePI (size_t index)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (/*index < 0 || */index >= pis.size ())
        return nullptr;
    if (index == 0 || index == ( pis.size () - 1 )) // can't delete beginning or end for now
        {
        AlignmentPI pi = pis.at (index);
        if (pi.curveType == AlignmentPI::HorizontalPIType::ARC || pi.curveType == AlignmentPI::HorizontalPIType::SCS)
            // just remove the curve data at begin or end
            {
            pis.at(index).curveType = AlignmentPI::HorizontalPIType::NONE;
            CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
            if (hzGeom.IsValid ())
                return hzGeom;
            }
        return nullptr;
        }
    size_t i = 0;
    for (bvector<AlignmentPI>::iterator iter = pis.begin (); iter != pis.end (); ++iter)
        {
        if (i == index)
            {
            pis.erase (iter);
            _SolvePI (index - 1, pis);
            _SolvePI (index, pis);
            CurveVectorPtr hzGeom = _BuildVectorFromPIS (pis);
            if (hzGeom.IsValid ())
                return hzGeom;
            }
        i++;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AlignmentPairEditor::GetPIPoints (bvector<DPoint3d>& pts)
    {
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.size () <= 0)
        return ERROR;

    for (auto pi : pis)
        {
        pts.push_back (pi.location);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::IsVerticalValid ()
    {
    return ( VerticalCurveVector().IsValid () );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPairEditor::CrestAndSagPointsXZ (ZeroSlopePoints zsType)
    {
    BeAssert (this->IsVerticalValid ());
    double runningLength = 0.0;
    bvector<DPoint3d> returnVector;
    for (auto primitive : *VerticalCurveVector())
        {
        DPoint3d start, end;
        primitive->GetStartEnd (start, end);
        switch (primitive->GetCurvePrimitiveType ())
            {
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    {
                    MSBsplineCurveCP bspline = primitive->GetBsplineCurveCP ();
                    VerticalCurveType vType = _GetVerticalCurveType (bspline);
                    if (vType == VerticalCurveType::TypeIII)
                        {
                        if (zsType == ZeroSlopePoints::SagOnly || zsType == ZeroSlopePoints::BothSagAndCrest)
                            {
                            DPoint3d pt = _GetZeroSlopePoint (bspline, runningLength);
                            if (pt.x >= 0.0)
                                returnVector.push_back (pt);
                            }
                        }
                    if (vType == VerticalCurveType::TypeI)
                        {
                        if (zsType == ZeroSlopePoints::CrestOnly || zsType == ZeroSlopePoints::BothSagAndCrest)
                            {
                            DPoint3d pt = _GetZeroSlopePoint (bspline, runningLength);
                            if (pt.x >= 0.0)
                                returnVector.push_back (pt);
                            }
                        }
                    }
                    break;
            }
        runningLength += fabs (end.x - start.x);
        }

    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<double> AlignmentPairEditor::CrestAndSagPointsStation (ZeroSlopePoints zsType)
    {
    bvector<DPoint3d> pts = CrestAndSagPointsXZ (zsType);
    bvector<double> returnVector;
    for (int i = 0; i < pts.size (); i++)
        {
        returnVector.push_back (pts[i].x);
        }
    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::HasSag ()
    {
    bvector<DPoint3d> sags = CrestAndSagPointsXZ(ZeroSlopePoints::SagOnly);
    return ( sags.size () > 0 ? true : false );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* note: max grade might be negative
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::MaximumGradeInPercent ()
    {
    BeAssert (this->IsVerticalValid ());
    double maxGrade = 0.0;
    for (auto primitive : *VerticalCurveVector())
        {
        DPoint3d start, end;
        primitive->GetStartEnd (start, end);
        switch (primitive->GetCurvePrimitiveType ())
            {
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    double slope = AlignmentPairEditor::_Slope (start, end);
                    maxGrade = ( fabs (maxGrade) > fabs (slope) ? maxGrade : slope );
                    }
                    break;
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    auto lineString = primitive->GetLineStringCP ();
                    for (size_t i = 1; i < lineString->size (); i++)
                        {
                        double slope = AlignmentPairEditor::_Slope (lineString->at (i - 1), lineString->at (i));
                        maxGrade = ( fabs (maxGrade) > fabs (slope) ? maxGrade : slope );
                        }
                    }
                    break;
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    {
                    MSBsplineCurveCP bspline = primitive->GetBsplineCurveCP ();
                    size_t count = bspline->GetNumPoles ();
                    if (count != 3)
                        {
                        continue; //ignore?
                        }
                    bvector<DPoint3d> poles;
                    bspline->GetPoles (poles);
                    for (size_t i = 1; i < 3; i++)
                        {
                        double slope = AlignmentPairEditor::_Slope (poles[i - 1], poles[i]);
                        maxGrade = ( fabs (maxGrade) > fabs (slope) ? maxGrade : slope );
                        }
                    }
                    break;
            }
        }
    return maxGrade * 100.0; // %
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::MaximumGradeChangeInPercent ()
    {
    BeAssert (this->IsVerticalValid ());
    double G1 = 0.0, G2 = 0.0;
    double maxGradeChange = 0.0;
    for (auto primitive : *VerticalCurveVector())
        {
        DPoint3d start, end;
        primitive->GetStartEnd (start, end);
        switch (primitive->GetCurvePrimitiveType ())
            {
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    G2 = AlignmentPairEditor::_Slope (start, end);
                    }
                    break;
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    auto lineString = primitive->GetLineStringCP ();
                    for (size_t i = 1; i < lineString->size (); i++)
                        {
                        G2 = AlignmentPairEditor::_Slope (lineString->at (i - 1), lineString->at (i));
                        if (maxGradeChange < fabs (G1 - G2))
                            maxGradeChange = fabs (G1 - G2);
                        G1 = G2;
                        }
                    }
                    break;
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    {
                    MSBsplineCurveCP bspline = primitive->GetBsplineCurveCP ();
                    size_t count = bspline->GetNumPoles ();
                    if (count != 3)
                        {
                        continue; //ignore?
                        }
                    bvector<DPoint3d> poles;
                    bspline->GetPoles (poles);
                    for (size_t i = 1; i < 3; i++)
                        {
                        G2 = AlignmentPairEditor::_Slope (poles[i - 1], poles[i]);
                        if (maxGradeChange < fabs (G1 - G2))
                            maxGradeChange = fabs (G1 - G2);
                        G1 = G2;
                        }
                    }
                    break;
            }
        if (maxGradeChange < fabs (G1 - G2))
            maxGradeChange = fabs (G1 - G2);
        G1 = G2;
        }
    return maxGradeChange * 100.0; // %
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
* compute the default design length when we have 0
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_GetDesignLength (DPoint3d newPt, bvector<AlignmentPVI> const& pvis, int index, bool useComputedG)
    {
    if (index == 0 || index == pvis.size () - 1) return 0.0;
    // potential size...
    double backDist = pvis[index].poles[PVI].x - pvis[index - 1].poles[PVT].x;
    double foreDist = pvis[index + 1].poles[PVC].x - pvis[index].poles[PVI].x;
    double potentialDist = std::min(backDist, foreDist);

    double designDist = 100.0;
    /*TODO: double G1 = AlignmentPairEditor::_Slope (pvis[index - 1].poles[PVI], newPt);
    double G2 = AlignmentPairEditor::_Slope (newPt, pvis[index + 1].poles[PVI]);    
    if (m_segment.IsValid())
        {
        auto pDesignRoadSegmentItem = m_segment->GetRoadSegmentItem();
        if (pDesignRoadSegmentItem != nullptr)
            {
            auto designItem = pDesignRoadSegmentItem->ToDesignRoadSegmentItem();
            if (designItem != nullptr)
                {
                DesignSpeedId speedId = m_segment->GetRoadSpecification()->GetDesignSpeedId();
                DesignSpeedCPtr designSpeed = DesignSpeed::Get(m_segment->GetDgnDb(), speedId);
                double crestk = designSpeed->GetCrestK_SSD();
                double sagk = designSpeed->GetSagK_SSD();

                VerticalCurveType cType = _GetVerticalCurveType(G1, G2);
                double differential = BASEGRADEDIFFERENTIAL; // so now "fake" the length based on a 1 difference in grade
                if (useComputedG == true)
                    differential = fabs (G1 - G2);

                if (cType == VerticalCurveType::TypeI || cType == VerticalCurveType::TypeII)
                    designDist = differential * crestk;
                else
                    designDist = differential * sagk;
                }
            }
        }*/
    if (designDist > (potentialDist * 1.95)) // dont let the EVC/PVC be rigt on each other by default
        return potentialDist * 1.95;
    return designDist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::SlopeAtStation (const double& station)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0) return 0.0f; // just go with flat, since there is no vertical alignment

    // find the controlling PVIs
    double G2 = 0.0;
    for (int i = 0; i < pvis.size () - 2; i++)
        {
        double G1 = 0.0;
        if (i > 0)
            G1 = AlignmentPairEditor::_Slope (pvis.at (i - 1).poles[PVI], pvis.at (i).poles[PVI]);
        G2 = AlignmentPairEditor::_Slope (pvis.at (i).poles[PVI], pvis.at (i + 1).poles[PVI]);
        if (pvis.at (i).poles[PVI].x <= station && station < pvis.at (i + 1).poles[PVI].x)
            {
            // is station on a vertical curve?
            // starting curve
            // on vertical curve...
            // = -g2 - x'' (A/L) x'' is distance from end..
            if (pvis.at (i).poles[PVT].x > station)
                {
                double xprime = pvis.at (i).poles[PVT].x - station;
                return G2 - ( xprime * ( ( G2 - G1 ) / pvis.at (i).length ) );
                }
            else if (pvis.at (i + 1).poles[PVC].x < station)
                {
                double xprime = pvis.at (i+1).poles[PVT].x - station;
                return G2 - ( xprime * ( ( G2 - G1 ) / pvis.at (i+1).length ) );
                }
            else
                return G2;
            }
        }
    // check for end station
    if (station == pvis.at (pvis.size () - 1).poles[PVI].x)
        return G2;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::_RemovePVIs (const double& startStation, const double& endStation)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0) return nullptr;
    bvector<AlignmentPVI> newPvis;
    for (auto pvi : pvis)
        {
        if ((pvi.poles[PVC].x > startStation && pvi.poles[PVC].x < endStation)
            || (pvi.poles[PVT].x > startStation && pvi.poles[PVT].x < endStation)
            || ( pvi.poles[PVI].x > startStation && pvi.poles[PVI].x < endStation))
            continue;
        newPvis.push_back (pvi);
        }
    if (newPvis.size () <= 1) return nullptr;
    CurveVectorPtr newVertical = _BuildVectorFromPVIS (newPvis);
    if (newVertical.IsValid ())
        return newVertical;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::ValidateVerticalData (bool updateInternalCurveVector)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0) return nullptr;
    bool changed = false;
    // skip start and end
    for (int i = 1; i < pvis.size () - 1; i++)
        {
        if (pvis[i].length == 0.0)
            {
            pvis[i].length = _GetDesignLength (pvis[i].poles[PVI], pvis, i);
            pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
            pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
            changed = true;
            }
        }
    if (changed == false)
        return _BuildVectorFromPVIS (pvis);

    // rebuild curve vector based on AlignmentPVI
    CurveVectorPtr newVertical = _BuildVectorFromPVIS (pvis);
    if (updateInternalCurveVector && newVertical.IsValid ())
        UpdateCurveVectors (HorizontalCurveVector ().get (), newVertical.get ());

    return newVertical;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePVI (DPoint3d fromPt, DPoint3d toPt, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    bool changed = false;
    for (int i = 0; i < pvis.size () && changed == false; i++)
        {
        if (_StationCompare (pvis[i].poles.at (VC::PVI), fromPt))  // found pvi to move
            {
            if (i == 0)  // special case first point, can't move in X only Z
                {
                pvis[i].poles[PVI].z = toPt.z;
                // update next pole
                pvis[i + 1].poles[PVC].z = pvis[i + 1].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i + 1].length );
                changed = true;

                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i].poles[PVI].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;
                }
            else if (i == ( pvis.size () - 1 ))  // special case last point, cant't move in X only Z
                {
                pvis[i].poles[PVI].z = toPt.z;
                // update previous poles
                pvis[i - 1].poles[PVT].z = pvis[i - 1].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i - 1].length );
                changed = true;

                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i - 1].poles[PVC].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i].poles[PVI].x;
                }
            else
                {
                if (pvis[i].length == 0.0)
                    {
                    pvis[i].length = _GetDesignLength (toPt, pvis, i);
                    pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - (pvis[i].length * 0.5);
                    pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                    }
                double minSta, maxSta;
                _GetValidEditRange (pvis, i, &minSta, &maxSta);
                if (toPt.x < minSta || toPt.x > maxSta)
                    break;
                // update the current pvi and curves
                pvis[i].poles[PVI].z = toPt.z;
                pvis[i].poles[PVI].x = toPt.x;
                pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
                pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
                pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );

                // update the next and previous
                pvis[i + 1].poles[PVC].z = pvis[i + 1].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i + 1].length );
                pvis[i - 1].poles[PVT].z = pvis[i - 1].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i - 1].length );
                changed = true;

                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i - 1].poles[PVC].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;
                }
            break;
            }
        }
    if (changed == false)
        return nullptr;

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* allow the move of a tangent segment up or down (pass the old center x,z and new center x, z)
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveVerticalTangent (DPoint3d fromPt, DPoint3d toPt, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    bool changed = false;
    double deltaZ = toPt.z - fromPt.z;
    StationRangeEdit edit1, edit2;
    for (int i = 0; i < ( pvis.size () - 1 ) && changed == false; i++)
        {
        // maybe should do a better check, but this will identify the tangent line
        // and the delta Z is what matters here.
        if (fromPt.x > pvis[i].poles[PVI].x && fromPt.x < pvis[i + 1].poles[PVI].x)
            {
            AlignmentPairEditorPtr vGeom = AlignmentPairEditor::Create (*HorizontalCurveVector(), VerticalCurveVector().get());
            // move the two pvis vertically and update the corresponding curves if necessary
            DPoint3d newPVILocation = DPoint3d::From (pvis[i].poles[PVI].x, 0.0, pvis[i].poles[PVI].z + deltaZ);
            CurveVectorPtr newVertical;
            if ((newVertical = vGeom->MovePVI (pvis[i].poles[PVI], newPVILocation, edit1)) == nullptr)
                return nullptr;
            vGeom->UpdateCurveVectors (HorizontalCurveVector ().get (), newVertical.get ());
            newPVILocation = DPoint3d::From (pvis[i + 1].poles[PVI].x, 0.0, pvis[i + 1].poles[PVI].z + deltaZ);
            if (( newVertical = vGeom->MovePVI (pvis[i + 1].poles[PVI], newPVILocation, edit2) ) == nullptr)
                return nullptr;
            editRange.preEditRange.startStation = editRange.postEditRange.startStation = fmin (edit1.preEditRange.startStation, edit2.preEditRange.startStation);
            editRange.preEditRange.endStation = editRange.postEditRange.endStation = fmax (edit1.preEditRange.endStation, edit2.preEditRange.endStation);
            return newVertical;
            }
        }
    return nullptr;  // not found
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::ForceGradeAtStation (const double& station, const double& slopeAbsolute, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    // find the PVI to work from
    for (int i = 0; i < ( pvis.size () - 1 ); i++)
        {
        if (pvis[i].poles[PVI].x <= station && station < pvis[i + 1].poles[PVI].x)
            {
            // Compute the new potential pvi at i+1 and call MovePVI
            double distance = pvis[i + 1].poles[PVI].x - pvis[i].poles[PVI].x;
            double newZ = pvis[i].poles[PVI].z + ( slopeAbsolute * distance );

            DPoint3d currentPt = DPoint3d::From (pvis[i + 1].poles[PVI].x, 0.0, pvis[i + 1].poles[PVI].z);
            DPoint3d newPt = DPoint3d::From (pvis[i + 1].poles[PVI].x, 0.0, newZ);

            return MovePVI (currentPt, newPt, editRange);
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* Allow the move of a pvc or pvt based on station.  Will return invalid if the result
* produces any overlapping curve or a 0 or negative length curve
* the result edit range is only the vertical curve
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePVCorPVT (double fromSta, double toSta, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    bool changed = false;
    for (int i = 1; i < ( pvis.size () - 1 ) && changed == false; i++)
        {
        if (pvis[i].length > 0.0)
            {
            if (_StationCompare (pvis[i].poles[PVC].x, fromSta) == true)
                {
                if (toSta >= pvis[i].poles[PVI].x)
                    return nullptr;
                if (pvis[i - 1].length > 0.0)
                    {
                    if (toSta < pvis[i - 1].poles[PVT].x)
                        return nullptr;
                    }
                pvis[i].length = fabs (toSta - pvis[i].poles[PVI].x) * 2.0;
                // update the current vertical curve at the point we've found
                pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
                pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
                pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );

                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i].poles[PVC].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i].poles[PVT].x;
                changed = true;
                }
            else if (_StationCompare (pvis[i].poles[PVT].x, fromSta) == true)
                {
                if (toSta <= pvis[i].poles[PVI].x)
                    return nullptr;
                if (pvis[i + 1].length > 0.0)
                    {
                    if (toSta > pvis[i + 1].poles[PVC].x)
                        return nullptr;
                    }
                pvis[i].length = fabs (toSta - pvis[i].poles[PVI].x) * 2.0;
                // update the current vertical curve at the point we've found
                pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
                pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
                pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );

                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i].poles[PVC].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i].poles[PVT].x;
                changed = true;
                }
            }
        }
    if (changed == false)
        return nullptr;

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPVI (DPoint3d pviPt, double lengthOfCurve, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    bool changed = false;
    bvector<AlignmentPVI>::iterator iter = pvis.begin ();
    for (int i = 0; i < ( pvis.size () - 1 ) && changed == false; i++)
        {
        ++iter;
        if (pviPt.x == 0.0)// not sure what to do...(update start z?)
            return nullptr;
        if (pviPt.x < pvis[i + 1].poles[PVI].x)
            {
            // check for previous curve / pvi overlap
            if (pvis[i].poles[PVI].x > pviPt.x - ( 0.5 * lengthOfCurve ))
                return nullptr;
            if (pvis[i].length > 0.0)
                {
                if (pvis[i].poles[PVT].x > pviPt.x - ( 0.5 * lengthOfCurve ))
                    return nullptr;
                }
            // check for next curve / pvi overlap
            if (pvis[i + 1].poles[PVI].x < pviPt.x + ( 0.5 * lengthOfCurve ))
                return nullptr;
            if (pvis[i + 1].length > 0.0)
                {
                if (pvis[i + 1].poles[PVC].x < pviPt.x + ( 0.5 * lengthOfCurve ))
                    return nullptr;
                }
            AlignmentPVI newItem (pviPt, lengthOfCurve);
            newItem.poles[PVC].x = pviPt.x - ( 0.5* lengthOfCurve );
            newItem.poles[PVT].x = pviPt.x + ( 0.5 * lengthOfCurve );
            newItem.poles[PVC].z = newItem.poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], newItem.poles[PVI]) * 0.5 * lengthOfCurve );
            newItem.poles[PVT].z = newItem.poles[PVI].z + ( AlignmentPairEditor::_Slope (newItem.poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * lengthOfCurve );

            // change the previous curve's pvt if necessary
            if (i > 0)
                {
                if (pvis[i].length > 0.0)
                    {
                    pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], newItem.poles[PVI]) * 0.5 * pvis[i].length );
                    editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i].poles[PVC].x;
                    }
                else
                    editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i].poles[PVI].x;
                }
            else
                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[0].poles[PVI].x;

            // change the forward curve's pvc if necessary
            if (pvis[i + 1].length > 0.0)
                {
                pvis[i + 1].poles[PVC].z = pvis[i + 1].poles[PVI].z - ( AlignmentPairEditor::_Slope (newItem.poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i + 1].length );
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;
                }
            else
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVI].x;

            pvis.insert (iter, newItem);
            changed = true;
            break;
            }
        }
    if (changed == false)
        return nullptr;

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePVI (DPoint3d pviPt, StationRangeEditR editRange)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 2)
        return nullptr;

    bool changed = false;
    int i = 0;
    for (bvector<AlignmentPVI>::iterator iter = pvis.begin (); iter != pvis.end (); ++iter)
        {
        if (_StationCompare (iter->poles[PVI], pviPt) == true)
            {
            if (i == 0 || i == ( pvis.size () - 1 )) // can't delete beginning or end for now
                return nullptr;

            // update previous and next curves
            if (i - 1 > 0)
                {
                pvis[i - 1].poles[PVT].z = pvis[i - 1].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i - 1].length );
                }
            if (( i + 1 ) < ( pvis.size () - 1 ))
                {
                pvis[i + 1].poles[PVC].z = pvis[i + 1].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i + 1].length );
                }
            editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i - 1].poles[PVC].x;
            editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;

            pvis.erase (iter);
            changed = true;
            break;
            }
        i++;
        }
    if (changed == false)
        return nullptr;

    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::RemovePVIs (const double& fromSta, const double& toSta, StationRangeEditR editRange) 
    { 
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 2)
        return nullptr;

    bool changed = false;
    int i = 0;
    for (bvector<AlignmentPVI>::iterator iter = pvis.begin (); iter != pvis.end (); ++iter)
        {
        if (iter->poles[PVI].x > fromSta && iter->poles[PVI].x < toSta)
            {
            if (i == 0 || i == ( pvis.size () - 1 )) // can't delete beginning or end for now
                continue;

            // update previous and next curves
            if (i - 1 > 0)
                {
                pvis[i - 1].poles[PVT].z = pvis[i - 1].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i - 1].length );
                }
            if (( i + 1 ) < ( pvis.size () - 1 ))
                {
                pvis[i + 1].poles[PVC].z = pvis[i + 1].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i + 1].length );
                }
            if (changed)
                {
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;
                }
            else
                {
                editRange.preEditRange.startStation = editRange.postEditRange.startStation = pvis[i - 1].poles[PVC].x;
                editRange.preEditRange.endStation = editRange.postEditRange.endStation = pvis[i + 1].poles[PVT].x;
                }
            changed = true;
            pvis.erase (iter);
            }
        i++;
        }
    if (changed == false)
        return nullptr;

    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* based on a horizontal change, the length of the vertical profile must change also
* try and fudge in a "modified" vertical based on the stations and the new range
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::ModifyVerticalRange (StationRangeEditR editRange, double matchEndStation)
    {
    double oldrange = editRange.preEditRange.Distance ();
    double newrange = editRange.postEditRange.Distance ();
    double delta = editRange.Delta ();
    if (delta == 0.0)
        return VerticalCurveVector(); // no reason to change

    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;         // TODO: Find case that executes this branch to verify it. I feel like we shouldn't return null here, but I'd need to understand this case

    bool startSet = false;
    int startEditIndex = 0;
    int endEditIndex = 0;
    // first iterate through and update the station values
    for (int i = 0; i < pvis.size (); i++)
        {
        if (DoubleOps::TolerancedComparison(pvis[i].poles[PVI].x, editRange.preEditRange.startStation) == 1 && 
            DoubleOps::TolerancedComparison(pvis[i].poles[PVI].x, editRange.preEditRange.endStation) == -1)
            {
            if (startSet == false)
                {
                startEditIndex = i;
                startSet = true;
                }
            pvis[i].poles[PVI].x = editRange.preEditRange.startStation + ((pvis[i].poles[PVI].x - editRange.preEditRange.startStation) * (newrange / oldrange));
            pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
            pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
            endEditIndex = i;
            }
        else if (DoubleOps::TolerancedComparison(pvis[i].poles[PVI].x, editRange.preEditRange.endStation) >= 0)
            {
            pvis[i].poles[PVI].x += delta;
            pvis[i].poles[PVC].x += delta;
            pvis[i].poles[PVT].x += delta;
            }
        }

    // now iterate again and update all the vertical curve locations appropriately, check ahead for overlapping curves and shorten both if necessary
    for (int i = 1; i < pvis.size () - 1; i++)
        {
        if (( pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 ) ) > (pvis[i + 1].poles[PVI].x - ( pvis[i + 1].length * 0.5 )))
            {
            // trim the lengths
            double distanceBetweenPVIS = pvis[i + 1].poles[PVI].x - pvis[i].poles[PVI].x;
            pvis[i].length = distanceBetweenPVIS;
            pvis[i + 1].length = distanceBetweenPVIS;
            }
        if (( pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 ) ) < ( pvis[i - 1].poles[PVI].x + ( pvis[i - 1].length * 0.5 ) ))
            {
            //trim the lengths
            double distanceBetweenPVIs = pvis[i].poles[PVI].x - pvis[i - 1].poles[PVI].x;
            pvis[i].length = distanceBetweenPVIs;
            }
        pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
        pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
        pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
        pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );
        }

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis, matchEndStation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::ModifyVerticalRange (bvector<StationRangeEdit> editRanges, double matchEndStation)
    {
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;         // TODO: Find case that executes this branch to verify it. I feel like we shouldn't return null here, but I'd need to understand this case

    for (auto editRange : editRanges)
        {
        double oldrange = editRange.preEditRange.Distance ();
        double newrange = editRange.postEditRange.Distance ();
        double delta = editRange.Delta ();
        if (delta == 0.0)
            return VerticalCurveVector (); // no reason to change


        bool startSet = false;
        int startEditIndex = 0;
        int endEditIndex = 0;
        // first iterate through and update the station values
        for (int i = 0; i < pvis.size (); i++)
            {
            if (DoubleOps::TolerancedComparison (pvis[i].poles[PVI].x, editRange.preEditRange.startStation) == 1 &&
                DoubleOps::TolerancedComparison (pvis[i].poles[PVI].x, editRange.preEditRange.endStation) == -1)
                {
                if (startSet == false)
                    {
                    startEditIndex = i;
                    startSet = true;
                    }
                pvis[i].poles[PVI].x = editRange.preEditRange.startStation + ( ( pvis[i].poles[PVI].x - editRange.preEditRange.startStation ) * ( newrange / oldrange ) );
                pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
                pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                endEditIndex = i;
                }
            else if (DoubleOps::TolerancedComparison (pvis[i].poles[PVI].x, editRange.preEditRange.endStation) >= 0)
                {
                pvis[i].poles[PVI].x += delta;
                pvis[i].poles[PVC].x += delta;
                pvis[i].poles[PVT].x += delta;
                }
            }

        // now iterate again and update all the vertical curve locations appropriately, check ahead for overlapping curves and shorten both if necessary
        for (int i = 1; i < pvis.size () - 1; i++)
            {
            if (( pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 ) ) > (pvis[i + 1].poles[PVI].x - ( pvis[i + 1].length * 0.5 )))
                {
                // trim the lengths
                double distanceBetweenPVIS = pvis[i + 1].poles[PVI].x - pvis[i].poles[PVI].x;
                pvis[i].length = distanceBetweenPVIS;
                pvis[i + 1].length = distanceBetweenPVIS;
                }
            if (( pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 ) ) < ( pvis[i - 1].poles[PVI].x + ( pvis[i - 1].length * 0.5 ) ))
                {
                //trim the lengths
                double distanceBetweenPVIs = pvis[i].poles[PVI].x - pvis[i - 1].poles[PVI].x;
                pvis[i].length = distanceBetweenPVIs;
                }
            pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
            pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
            pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
            pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );
            }
        }

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis, matchEndStation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPairEditor::CrestAndSagPointsXYZ (ZeroSlopePoints zsType)
    {
    bvector<DPoint3d> vec = CrestAndSagPointsXZ (zsType);
    bvector<DPoint3d> returnVector;
    for (int i = 0; i < vec.size (); i++)
        {
        DPoint3d pt = GetPointFromStation (vec[i].x);
        pt.z = vec[i].z;
        returnVector.push_back (pt);
        }
    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_ComputeVerticalCurveByThroughPoint (DPoint3d pt, AlignmentPVI pvi, double * resultantLength)
    {
    double G1 = AlignmentPairEditor::_Slope (pvi.poles[PVC], pvi.poles[PVI]);
    double G2 = AlignmentPairEditor::_Slope (pvi.poles[PVI], pvi.poles[PVT]);
    double xp = pvi.poles[PVI].x - pt.x; // distance from pvi
    //
    // through point equation...
    // A/  L^2 - [2(Elevp - Elevpvi) - (g1 + g2)xp]L + Axp^2 = 0
    //
    double cooefs[3];
    cooefs[0] = ( G2 - G1 ) / 4.0;
    cooefs[1] = ( 2 * ( pt.z - pvi.poles[PVI].z ) ) - ( ( G1 + G2 )* xp );
    cooefs[2] = ( G2 - G1 ) * ( xp * xp );

    // Compute what should be the new length of vertical curve...
    // using the quadratic equation
    double roots[2];
    int num = AnalyticRoots::SolveQuadric (cooefs, roots);
    if (num != 1) return false;

    *resultantLength = roots[0];
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_SolveAllPVIs (bvector<AlignmentPVI>& pvis)
    {
    if (pvis.size () <= 1) return false;
    if (pvis.size () <= 2) return true;
    for (size_t i = 1; i < pvis.size () - 1; i++)
        {
        if (!_SolvePVI (pvis.at (i), pvis.at (i - 1), pvis.at (i + 1)))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_SolvePVI (AlignmentPVI& pviToSolve, const AlignmentPVI& prevPVI, const AlignmentPVI& nextPVI, bool forceFit)
    {
    // check for previous curve / pvi overlap
    if (prevPVI.poles[PVI].x > pviToSolve.poles[PVI].x)
        return false;
    if (nextPVI.poles[PVI].x < pviToSolve.poles[PVI].x)
        return false;
    if (pviToSolve.length > 0.0)
        {
        if (prevPVI.poles[PVT].x > pviToSolve.poles[PVI].x - ( 0.5 * pviToSolve.length ))
            {
            if (forceFit)
                {
                pviToSolve.length = ( pviToSolve.poles[PVT].x - prevPVI.poles[PVT].x) * 0.45;
                }
            else
                return false;
            }

        if (nextPVI.poles[PVC].x < pviToSolve.poles[PVI].x + ( 0.5 * pviToSolve.length ))
            {
            if (forceFit)
                {
                double testLen = ( nextPVI.poles[PVC].x - pviToSolve.poles[PVI].x ) * 0.45;
                if (testLen < pviToSolve.length)
                    pviToSolve.length = testLen;
                }
            else
                return false;
            }
        }
    pviToSolve.poles[PVC].x = pviToSolve.poles[PVI].x - ( 0.5 * pviToSolve.length );
    pviToSolve.poles[PVT].x = pviToSolve.poles[PVI].x + ( 0.5 * pviToSolve.length );
    pviToSolve.poles[PVC].z = pviToSolve.poles[PVI].z - ( AlignmentPairEditor::_Slope (prevPVI.poles[PVI], pviToSolve.poles[PVI]) * 0.5 * pviToSolve.length );
    pviToSolve.poles[PVT].z = pviToSolve.poles[PVI].z + ( AlignmentPairEditor::_Slope (pviToSolve.poles[PVI], nextPVI.poles[PVI]) * 0.5 * pviToSolve.length );

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::InsertStartIntersection (CurveVectorCR drapeVector, double lengthofGradeApproachInMeters, double endSlope, StationRangeEditP  editP)
    {
    if (drapeVector.empty())
        return false;
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return false;

    double approachStationLimit = pvis.at(pvis.size() - 1).poles[PVI].x * APPROACH_STATION_LIMIT_FACTOR; // don't build an approach too long

    DPoint3d drapeStart, drapeEnd;
    drapeVector.GetStartEnd(drapeStart, drapeEnd);

    bvector<AlignmentPVI> newPVIs;
    double lastDrapeStation = 0.0;
    // add entries from the drape. DrapeVector station order should be 0 at at the TEE, and increasing as you move out to the road edge
    for (auto primitive : drapeVector)
        {
        DPoint3d start, end;
        primitive->GetStartEnd(start, end);
        if (newPVIs.empty())
            {
            AlignmentPVI pvi(DPoint3d::From(0.0, 0.0, start.z), 0.0);
            newPVIs.push_back(pvi);
            }
        
        AlignmentPVI pvi(DPoint3d::From(drapeStart.DistanceXY(end), 0.0, end.z), 0.0);
        newPVIs.push_back(pvi);
        lastDrapeStation = pvi.poles[PVI].x;
        }

    // next insert the grade approach, compute the location of the PVI point
    double approachLength = lengthofGradeApproachInMeters;
    if (lastDrapeStation + approachLength > approachStationLimit)
        approachLength = MAX(1.0, approachStationLimit - lastDrapeStation);

    DPoint3d endOfApproach;
    endOfApproach.x = lastDrapeStation + approachLength;
    endOfApproach.y = 0.0;
    endOfApproach.z = newPVIs.at (newPVIs.size () - 1).poles[PVI].z + ( approachLength * endSlope );
    
    size_t approachIndex = newPVIs.size ();
    AlignmentPVI approachPVI (endOfApproach, approachLength*0.5);
    newPVIs.push_back (approachPVI);

    // append any subsequent entries (beyond the drape) from the old PVI list 
    for (auto pvi : pvis)
        {
        if (pvi.poles[PVI].x > endOfApproach.x && pvi.poles[PVC].x > endOfApproach.x)
            newPVIs.push_back(pvi);
        }

    if (approachIndex > 0 && approachIndex < newPVIs.size () - 1)
        _SolvePVI (newPVIs.at (approachIndex), newPVIs.at (approachIndex - 1), newPVIs.at (approachIndex + 1));
    approachIndex++;
    if (approachIndex > 0 && approachIndex < newPVIs.size () - 1)
        _SolvePVI (newPVIs.at (approachIndex), newPVIs.at (approachIndex - 1), newPVIs.at (approachIndex + 1));

    if (editP != nullptr)
        {
        editP->preEditRange.startStation = 0.0;  editP->postEditRange.startStation = 0.0;
        if (approachIndex < newPVIs.size())
            editP->postEditRange.endStation = editP->preEditRange.endStation = newPVIs.at(approachIndex).poles[PVT].x;
        else
            editP->postEditRange.endStation = editP->preEditRange.endStation = LengthXY();
        }
    CurveVectorPtr newCurve = _BuildVectorFromPVIS (newPVIs);
    if (newCurve.IsValid ())
        {
        UpdateCurveVectors (HorizontalCurveVector ().get (), newCurve.get ());
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::InsertEndIntersection (CurveVectorCR drapeVector, double lengthofGradeApproachInMeters, double endSlope, StationRangeEditP editP)
    {
    if (drapeVector.empty())
        return false;
    bvector<AlignmentPVI> pvis = _GetPVIs();
    if (pvis.size () <= 0)
        return false;

    double totalEndStation = pvis.back().poles[PVI].x;

    DPoint3d drapeStart, drapeEnd;
    drapeVector.GetStartEnd(drapeStart, drapeEnd);
    
    // assemble drape PVIs, transforming them into the correct road station (from a relative 'end' station format). DrapeVector station order should be 
    // 0 at at the road edge and increasing as you move to the center, TEE point
    bvector<AlignmentPVI> drapedPVIs;
    double firstDrapeStation = 0.0;
    for (auto primitive : drapeVector)
        {
        DPoint3d start, end;
        primitive->GetStartEnd(start, end);
        if (drapedPVIs.empty())
            {
            AlignmentPVI pvi(DPoint3d::From(totalEndStation - drapeEnd.DistanceXY(start), 0.0, start.z), 0.0);
            drapedPVIs.push_back(pvi);
            firstDrapeStation = pvi.poles[PVI].x;
            }

        AlignmentPVI pvi(DPoint3d::From(totalEndStation - drapeEnd.DistanceXY(end), 0.0, end.z), 0.0);
        drapedPVIs.push_back(pvi);
        }

    // next insert the grade approach, compute the location of the PVI point
    double approachStationLimit = totalEndStation - (totalEndStation * APPROACH_STATION_LIMIT_FACTOR);
    double approachLength = lengthofGradeApproachInMeters;
    if (firstDrapeStation - approachLength < approachStationLimit)
        approachLength = MAX(1.0, firstDrapeStation - approachStationLimit);

    DPoint3d endOfApproach;
    endOfApproach.x = firstDrapeStation - approachLength;
    endOfApproach.y = 0.0;
    endOfApproach.z = drapedPVIs.at (0).poles[PVI].z - ( approachLength * endSlope );
    AlignmentPVI approachPVI (endOfApproach, approachLength* 0.5);

    bvector<AlignmentPVI> newPVIs;
    // add earlier entries (before the drape) from the old PVI list 
    for (auto pvi : pvis)
        {
        if (pvi.poles[PVI].x < endOfApproach.x && pvi.poles[PVT].x < endOfApproach.x)
            newPVIs.push_back(pvi);
        }
    int32_t approachIndex = static_cast<int32_t>(newPVIs.size());
    newPVIs.push_back (approachPVI);
    for (auto pvi : drapedPVIs)
        newPVIs.push_back (pvi);

    if (approachIndex > 0 && approachIndex < newPVIs.size () - 1)
        _SolvePVI (newPVIs.at (approachIndex), newPVIs.at (approachIndex - 1), newPVIs.at (approachIndex + 1));
    approachIndex--;
    if (approachIndex > 0 && approachIndex < newPVIs.size () - 1)
        _SolvePVI (newPVIs.at (approachIndex), newPVIs.at (approachIndex - 1), newPVIs.at (approachIndex + 1));

    if (editP != nullptr)
        {
        if (approachIndex >= 0)
            editP->preEditRange.startStation = editP->postEditRange.startStation = newPVIs.at (approachIndex).poles[PVC].x;
        else
            editP->preEditRange.startStation = 0.0;

        editP->postEditRange.endStation = editP->postEditRange.endStation = LengthXY ();
        }

    CurveVectorPtr newCurve = _BuildVectorFromPVIS (newPVIs);
    if (newCurve.IsValid ())
        {
        UpdateCurveVectors (HorizontalCurveVector ().get (), newCurve.get ());
        return true;
        }
    return false;
    }

const auto pvi_sorter = [] (AlignmentPVI first, AlignmentPVI second) ->bool
    {
    if (first.poles[PVI].x < second.poles[PVI].x) return true; 
    return false;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_PreviousVerticalCurveStation (bvector<AlignmentPVI> const & pvis, const double& fromStation, VC vc)
    {
    double returnStation = 0.0;
    for (auto pvi : pvis)
        {
        if (pvi.poles[vc].x < fromStation)
            returnStation = pvi.poles[vc].x;
        }
    return returnStation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::_NextVerticalCurveStation (bvector<AlignmentPVI> const & pvis, const double& fromStation, VC vc)
    {
    BeAssert (pvis.size () > 0);
    double returnStation = 0.0;

    for (int i = (int)pvis.size () - 1; i >= 0; i--)
        {
        if (fromStation < pvis.at(i).poles[vc].x)
            returnStation = pvis.at (i).poles[vc].x;
        }
    return returnStation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::InsertCrossingIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double entranceSlope, double exitSlope,
                                                      double stationWhereSecondaryIntersectsPrimaryPavement, StationRangeEditP editP)
    {
    if (drapeVector.empty())
        return false;
    bvector<AlignmentPVI> pvis = _GetPVIs();
    if (pvis.size () <= 0)
        return false;
    DPoint3d drapeStartPt, drapeEndPt;
    drapeVector.GetStartEnd (drapeStartPt, drapeEndPt);
    // assemble drape PVIs, transforming them into the correct road station (from a relative station format). DrapeVector station order should be 
    // 0 at the first (entrance) crossing with the primary pavement, and should end on the other (exiting) crossing of the primary pavement
    bvector<AlignmentPVI> drapedPVIs;
    double firstDrapeStation = 0.0, lastDrapeStation = 0.0;
    for (auto primitive : drapeVector)
        {
        DPoint3d start, end;
        primitive->GetStartEnd(start, end);
        if (drapedPVIs.empty())
            {
            AlignmentPVI pvi(DPoint3d::From(stationWhereSecondaryIntersectsPrimaryPavement + drapeStartPt.DistanceXY(start), 0.0, start.z), 0.0);
            drapedPVIs.push_back(pvi);
            }

        AlignmentPVI pvi(DPoint3d::From(stationWhereSecondaryIntersectsPrimaryPavement + drapeStartPt.DistanceXY(end), 0.0, end.z), 0.0);
        drapedPVIs.push_back(pvi);
        }

    //std::sort (drapedPVIs.begin (), drapedPVIs.end (), pvi_sorter);

    AlignmentPVI drapeStart = drapedPVIs.at (0);
    AlignmentPVI drapeEnd = drapedPVIs.at (drapedPVIs.size () - 1);
    firstDrapeStation = drapeStart.poles[PVI].x;
    lastDrapeStation = drapeEnd.poles[PVI].x;
    // compute range to be modified
    double lowRange, highRange;
    if (firstDrapeStation < lengthofgradeapproachinmeters)
        lowRange = firstDrapeStation * 0.5;
    else
        lowRange = firstDrapeStation - lengthofgradeapproachinmeters;
    if (lengthofgradeapproachinmeters > LengthXY () - lastDrapeStation)
        highRange = LengthXY () - ( LengthXY () - lastDrapeStation);
    else
        highRange = lastDrapeStation + lengthofgradeapproachinmeters;

    DPoint3d endOfApproach;
    endOfApproach.x = highRange;
    endOfApproach.y = 0.0;
    endOfApproach.z = drapeEnd.poles[PVI].z + ( (highRange - lastDrapeStation) * exitSlope );
    AlignmentPVI exitApproachPVI (endOfApproach, highRange - lastDrapeStation );

    DPoint3d startOfApproach;
    startOfApproach.x = lowRange;
    startOfApproach.y = 0.0;
    startOfApproach.z = drapeStart.poles[PVI].z + ( ( firstDrapeStation - lowRange ) * entranceSlope );
    AlignmentPVI entranceApproachPVI (startOfApproach, firstDrapeStation - lowRange);

    bvector<AlignmentPVI> newPVIs;
    // add earlier entries (before the drape) from the old PVI list 
    for (auto pvi : pvis)
        {
        if (pvi.poles[PVI].x < lowRange && pvi.poles[PVT].x < lowRange)
            newPVIs.push_back(pvi);
        }

    newPVIs.push_back (entranceApproachPVI);
    AlignmentPVI lastPVI (startOfApproach, 0.0);
    // append new draped entries
    for (auto pvi : drapedPVIs)
        {
        if (!DoubleOps::AlmostEqual (pvi.poles[PVI].x, lastPVI.poles[PVI].x))
            newPVIs.push_back (pvi);
        lastPVI = pvi;
        }
    newPVIs.push_back (exitApproachPVI);

    // append any subsequent entries (beyond the drape) from the old PVI list 
    for (auto pvi : pvis)
        {
        if (pvi.poles[PVI].x > highRange && pvi.poles[PVC].x > highRange)
            newPVIs.push_back(pvi);
        }

    if (editP != nullptr)
        {
        editP->preEditRange.startStation = editP->postEditRange.startStation = _PreviousVerticalCurveStation(newPVIs, lowRange);
        editP->preEditRange.endStation = editP->postEditRange.endStation = _NextVerticalCurveStation(newPVIs, highRange);
        }
    if (!_SolveAllPVIs (newPVIs))
        BeAssert (false);
    CurveVectorPtr newCurve = _BuildVectorFromPVIS(newPVIs);
    if (newCurve.IsValid ())
        {
        UpdateCurveVectors (HorizontalCurveVector ().get (), newCurve.get ());
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
// always want to edit the NEXT PVI
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::ForceThroughPoint (DPoint3d pt, StationRangeEditR editRange)
    { 
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 0)
        return nullptr;
    bool changed = false;
    // figure out of station (pt.x) is on a vertical curve or on a tangent..
    for (int i = 1; i < ( pvis.size () ) && changed == false; i++)
        {
        if (pvis.at(i-1).poles[PVT].x < pt.x && pt.x < pvis.at (i).poles[PVC].x) // on a tangent
            {
            // compute the new tangent based on the slope
            double newSlope = _Slope (pt, pvis.at (i-1).poles[PVI]);
            pvis[i].poles[PVI].z = pvis[i-1].poles[PVI].z + ( newSlope * ( pvis[i].poles[PVI].x - pvis[i-1].poles[PVI].x ) );
            if (_SolveAllPVIs (pvis))
                changed = true;
            break;
            }
        else if (pvis.at(i).poles[PVC].x < pt.x && pt.x < pvis.at (i).poles[PVT].x) // on this vertical curve
            {
            double newLength;
            if (_ComputeVerticalCurveByThroughPoint (pt, pvis.at (i), &newLength) == true)
                {
                pvis[i].length = newLength;
                // update the current vertical curve at the point we've found
                pvis[i].poles[PVC].x = pvis[i].poles[PVI].x - ( pvis[i].length * 0.5 );
                pvis[i].poles[PVC].z = pvis[i].poles[PVI].z - ( AlignmentPairEditor::_Slope (pvis[i - 1].poles[PVI], pvis[i].poles[PVI]) * 0.5 * pvis[i].length );
                pvis[i].poles[PVT].x = pvis[i].poles[PVI].x + ( pvis[i].length * 0.5 );
                pvis[i].poles[PVT].z = pvis[i].poles[PVI].z + ( AlignmentPairEditor::_Slope (pvis[i].poles[PVI], pvis[i + 1].poles[PVI]) * 0.5 * pvis[i].length );
                changed = true;
                break;
                }
            }
        }
    if (changed == false) return nullptr;

    // rebuild curve vector based on AlignmentPVI
    return _BuildVectorFromPVIS (pvis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::MoveEndPIWithVerticalChange (DPoint3d toPt, bool isStart, StationRangeEditP stationRangeEditP)
    {
    toPt.z = 0.0;

    CurveVectorPtr newHz;
    CurveVectorPtr rangeHorizontal = HorizontalCurveVector ();
    CurveVectorWithDistanceIndexPtr originalPath = CurveVectorWithDistanceIndex::Create ();
    originalPath->SetPath (rangeHorizontal);

    AlignmentPI calcedPI;
    StationRangeEdit rangeEdit;
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.size () < 2)
        return false;

    DPoint3d referencePt;
    if (isStart)
        {
        rangeEdit.preEditRange.startStation = 0.0;
        AlignmentPI nextPI = pis[1];
        if (nextPI.curveType == AlignmentPI::NONE)
            referencePt = nextPI.location;
        else if (nextPI.curveType == AlignmentPI::ARC)
            referencePt = nextPI.arc.endPoint;
        else if (nextPI.curveType == AlignmentPI::SCS)
            referencePt = nextPI.spiral2.endSpiralPt;
        rangeEdit.preEditRange.endStation = HorizontalDistanceFromStart (referencePt);
        newHz = MovePI (0, toPt, calcedPI);
        }
    else
        {
        rangeEdit.preEditRange.endStation = LengthXY ();
        AlignmentPI prevPI = pis[pis.size () - 2];
        if (prevPI.curveType == AlignmentPI::NONE)
            rangeEdit.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.location);
        else if (prevPI.curveType == AlignmentPI::ARC)
            rangeEdit.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.arc.startPoint);
        else if (prevPI.curveType == AlignmentPI::SCS)
            rangeEdit.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.spiral1.beginSpiralPt);
        newHz = MovePI (pis.size () - 1, toPt, calcedPI);
        }
    if (!newHz.IsValid ()) return false;
    AlignmentPairEditorPtr modifiedAlign = AlignmentPairEditor::Create (*newHz, nullptr);
    pis = modifiedAlign->_GetPIs ();
    // compute the post edit range
    if (isStart)
        {
        rangeEdit.postEditRange.startStation = 0.0;
        rangeEdit.postEditRange.endStation = modifiedAlign->HorizontalDistanceFromStart (referencePt);
        }
    else
        {
        rangeEdit.postEditRange.startStation = rangeEdit.preEditRange.startStation;
        rangeEdit.postEditRange.endStation = modifiedAlign->LengthXY ();
        }

    CurveVectorPtr newVertical = ModifyVerticalRange (rangeEdit, modifiedAlign->LengthXY());

#ifndef NDEBUG
    if (VerticalCurveVector().IsValid())
        {
        BeAssert(newVertical.IsValid());    // I don't think we should ever end up with an invalid vert if we had a valid one to start with (but if I'm wrong remove this)
        }
    if (newVertical.IsValid())
        {
        DPoint3d start, end;
        newVertical->GetStartEnd(start, end);
        CurveVectorWithDistanceIndexPtr newHzIdx = CurveVectorWithDistanceIndex::Create();
        newHzIdx->SetPath(newHz);
        double len = newHzIdx->TotalPathLength ();
        BeAssert(DoubleOps::AlmostEqual(end.x, len));
        }
#endif

    UpdateCurveVectors(newHz.get(), newVertical.get());

#ifndef NDEBUG
    if (newVertical.IsValid ())
        {
        DPoint3d start, end;
        newVertical->GetStartEnd (start, end);
        double len = LengthXY ();
        BeAssert (DoubleOps::AlmostEqual (end.x, len));
        }
#endif

    if (stationRangeEditP) 
        *stationRangeEditP = rangeEdit;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPairEditor::FlipAlignment ()
    {
    CurveVectorPtr hzAlign = HorizontalCurveVector ();
    if (!hzAlign.IsValid ()) return nullptr;

    hzAlign->ReverseCurvesInPlace ();
    if (VerticalCurveVector ().IsValid ())
        {
        CurveVectorPtr vtAlign = _ReverseVertical ();
        if (vtAlign.IsValid ())
            {
            return AlignmentPair::Create (*hzAlign, vtAlign.get ());
            }
        }
    return AlignmentPair::Create (*hzAlign, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::_ReverseVertical ()
    {
    double length = LengthXY ();
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    if (pvis.size () <= 1)
        return nullptr;
    for (int i = 0; i < pvis.size (); i++)
        {
        pvis.at (i).poles[PVC].x = length - pvis.at (i).poles[PVC].x;
        pvis.at (i).poles[PVT].x = length - pvis.at (i).poles[PVT].x;
        pvis.at (i).poles[PVI].x = length - pvis.at (i).poles[PVI].x;
        }

    // rebuild curve vector based on AlignmentPVI
    CurveVectorPtr newVertical = _BuildVectorFromPVIS (pvis);
    if (newVertical.IsValid ())
        return newVertical;
    return nullptr;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StationRangeEdit AlignmentPairEditor::ComputeHorizontalEditRange (CurveVectorCR newHorizontal)
    {
    StationRangeEdit computedRange;
    bool isStart = false;
    bool isEnd = false;

    DPoint3d start, end;
    DPoint3d newStart, newEnd;
    HorizontalCurveVector ()->GetStartEnd (start, end);
    newHorizontal.GetStartEnd (newStart, newEnd);
    if (!start.AlmostEqualXY (newStart))
        isStart = true;
    else if (!end.AlmostEqualXY (newEnd))
        isEnd = true;
    DPoint3d referencePt;
    AlignmentPairEditorPtr modifiedAlign = AlignmentPairEditor::Create (newHorizontal, nullptr);
    bvector<AlignmentPI> pis = _GetPIs ();
    if (pis.size () < 2)
        return computedRange;
    if (isStart)
        {
        computedRange.preEditRange.startStation = 0.0;
        AlignmentPI nextPI = pis[1];
        if (nextPI.curveType == AlignmentPI::NONE)
            referencePt = nextPI.location;
        else if (nextPI.curveType == AlignmentPI::ARC)
            referencePt = nextPI.arc.endPoint;
        else if (nextPI.curveType == AlignmentPI::SCS)
            referencePt = nextPI.spiral2.endSpiralPt;
        computedRange.preEditRange.endStation = HorizontalDistanceFromStart (referencePt);

        computedRange.postEditRange.startStation = LengthXY () - modifiedAlign->LengthXY ();
        computedRange.postEditRange.endStation = computedRange.preEditRange.endStation;
        }
    else if (isEnd)
        {
        computedRange.preEditRange.endStation = LengthXY ();
        AlignmentPI prevPI = pis[pis.size () - 2];
        if (prevPI.curveType == AlignmentPI::NONE)
            computedRange.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.location);
        else if (prevPI.curveType == AlignmentPI::ARC)
            computedRange.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.arc.startPoint);
        else if (prevPI.curveType == AlignmentPI::SCS)
            computedRange.preEditRange.startStation = HorizontalDistanceFromStart (prevPI.spiral1.beginSpiralPt);

        computedRange.postEditRange.startStation = computedRange.preEditRange.startStation;
        computedRange.postEditRange.endStation = modifiedAlign->LengthXY ();
        }
    else
        {
        CurveVectorPtr rangeHorizontal = HorizontalCurveVector ();
        CurveVectorWithDistanceIndexPtr originalPath = CurveVectorWithDistanceIndex::Create ();
        originalPath->SetPath (rangeHorizontal);

        CurveVectorWithDistanceIndexPtr newPath = CurveVectorWithDistanceIndex::Create ();
        CurveVectorPtr newHz = newHorizontal.Clone ();
        newPath->SetPath (newHz);

        bvector<PathLocationDetailPair> pathAIntervals;
        bvector<PathLocationDetailPair> pathBIntervals;

        GeometryDebug::Announce (*originalPath, "FindCommonPaths.C");
        GeometryDebug::Announce (*newPath, "FindCommonPaths.B");


        // here is a wiki link to this function:
        // http://bsw-wiki.bentley.com/bin/view.pl/Main/CurveVectorWithDistanceIndex
        // effectively we want to compare the original alignment with new or current dynamic one and see 
        // where the changes have occured.
        CurveVectorWithDistanceIndex::FindCommonSubPaths (*originalPath, *newPath, pathAIntervals, pathBIntervals, true, true);
        GeometryDebug::Announce (pathAIntervals, pathBIntervals, "CommonSubPaths");
        // with these edits, we should only get one item in each vector.
        // the PathAIntervals A and B cover the original location/station of the edit, 
        // the PathBIntervals will give us the new length and therefore the ratio of any edits
        // that need to happen to the vertical and to the segments
        PathLocationDetailPair const* originalPairCP = nullptr;
        for (PathLocationDetailPair &pair : pathAIntervals)
            {
            if (pair.GetTagA () == 0 && pair.GetTagB () == 0)
                {
                originalPairCP = &pair;
                break;
                }
            }

        if (!originalPairCP)
            return computedRange;

        PathLocationDetailPair const* finalPairCP = nullptr;
        for (PathLocationDetailPair &apair : pathBIntervals)
            {
            if (apair.GetTagA () == 0 && apair.GetTagB () == 0)
                {
                finalPairCP = &apair;
                break;
                }
            }

        if (!finalPairCP)
            return computedRange;

        computedRange.preEditRange.startStation = originalPairCP->DetailA ().DistanceFromPathStart ();
        computedRange.preEditRange.endStation = originalPairCP->DetailB ().DistanceFromPathStart ();
        computedRange.postEditRange.startStation = computedRange.preEditRange.startStation;
        computedRange.postEditRange.endStation = computedRange.postEditRange.startStation + ( finalPairCP->DetailB ().DistanceFromPathStart () - finalPairCP->DetailA ().DistanceFromPathStart () );

        }

    return computedRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPairEditor::GetPartialAlignment (double startStation, double endStation) const
    {
    CurveVectorPtr hz = GetPartialHorizontalAlignment (startStation, endStation);
    if (!hz.IsValid ()) return nullptr;
    if (VerticalCurveVector ().IsValid ())
        {
        CurveVectorPtr vt = GetPartialVerticalAlignment (startStation, endStation);
        AlignmentPairEditorPtr returnAlignment = AlignmentPairEditor::Create (*hz, vt.get ());
        if (returnAlignment.IsValid ())
            {
            bvector<AlignmentPVI> pvis = returnAlignment->_GetPVIs ();
            CurveVectorPtr newVT = returnAlignment->_BuildVectorFromPVIS (pvis, returnAlignment->LengthXY ());
            if (newVT.IsValid ())
                {
                returnAlignment->UpdateCurveVectors (returnAlignment->HorizontalCurveVector ().get (), newVT.get ());
                return returnAlignment;
                }
            }
        }
    else
        {
        AlignmentPairEditorPtr returnAlignment = AlignmentPairEditor::Create (*hz, nullptr);
        return returnAlignment;
        }
    return nullptr;
    }