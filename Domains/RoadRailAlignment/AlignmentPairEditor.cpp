/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentPairEditor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentPairEditor.h>

#define BASEGRADEDIFFERENTIAL 1.0
// limit intersection/ramp approach lengths to a factor of the total alignment length
#define APPROACH_STATION_LIMIT_FACTOR 0.4

#define TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED 1E-04

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
//
// k = L / | G1-G2 |
//
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPVI::KValue () const
    {
#if 0
    double G1 = AlignmentPairEditor::_Slope (poles[PVC], poles[PVI]);
    double G2 = AlignmentPairEditor::_Slope (poles[PVI], poles[PVT]);
#else
    double G1 = 0.0;
    double G2 = 0.0;
#endif
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
#if 0
    double G1 = AlignmentPairEditor::_Slope (poles[PVC], poles[PVI]);
    double G2 = AlignmentPairEditor::_Slope (poles[PVI], poles[PVT]);
#else
    double G1 = 0.0;
    double G2 = 0.0;
#endif
    G1 = G1 * 100.0;
    G2 = G2 * 100.0;
    return fabs (G1 - G2) * kvalue;
    }



//=======================================================================================
// AlignmentPI
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
AlignmentPI::AlignmentPI()
    {
#ifndef NDEBUG
    int size = sizeof(AlignmentPI);
    memset(this, 0xBAADF00D, size);
#endif

    m_type = TYPE_Uninitialized;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPI::InitNoCurve(DPoint3dCR piPoint)
    {
    NoCurveInfo ncInfo;
    ncInfo.piPoint = DPoint3d::From(piPoint.x, piPoint.y, 0.0);

    m_noCurveInfo = ncInfo;
    m_type = TYPE_NoCurve;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPI::InitArc(DPoint3dCR piPoint, double radius)
    {
    ArcInfo aInfo;
    aInfo.arc.orientation = ORIENTATION_Unknown;
    aInfo.arc.piPoint = DPoint3d::From(piPoint.x, piPoint.y, 0.0);
    aInfo.arc.radius = radius;

    // Set same point for start/end so we can check whether that PI has been solved or not
    aInfo.arc.startPoint = aInfo.arc.piPoint;
    aInfo.arc.endPoint = aInfo.arc.piPoint;

    m_arcInfo = aInfo;
    m_type = TYPE_Arc;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPI::InitSCS(DPoint3dCR overallPI, double arcRadius, double spiralLength1, double spiralLength2)
    {
    SCSInfo scsInfo;
    scsInfo.overallPI = DPoint3d::From(overallPI.x, overallPI.y, 0.0);
    scsInfo.arc.radius = arcRadius;
    scsInfo.arc.orientation = ORIENTATION_Unknown;
    scsInfo.spiral1.length = spiralLength1;
    scsInfo.spiral2.length = spiralLength2;

    m_scsInfo = scsInfo;
    m_type = TYPE_SCS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPI::InitSS(DPoint3dCR overallPI, double spiralLength)
    {
    SSInfo ssInfo;
    ssInfo.overallPI = DPoint3d::From(overallPI.x, overallPI.y, 0.0);
    ssInfo.spiral1.length = spiralLength;
    ssInfo.spiral2.length = spiralLength;

    m_ssInfo = ssInfo;
    m_type = TYPE_SS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPI::InitInvalid(AlignmentPI::Type piType)
    {
    switch (piType)
        {
        case TYPE_Arc:
            m_arcInfo = ArcInfo();
            break;
        case TYPE_SCS:
            m_scsInfo = SCSInfo();
            break;
        case TYPE_SS:
            m_ssInfo = SSInfo();
            break;
        case TYPE_NoCurve:
        default:
            m_noCurveInfo = NoCurveInfo();
            piType = TYPE_NoCurve;
            break;
        }
    m_type = piType;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
DPoint3d AlignmentPI::GetPILocation() const
    {
    switch (m_type)
        {
        case TYPE_NoCurve:
            return m_noCurveInfo.piPoint;
        case TYPE_Arc:
            return m_arcInfo.arc.piPoint;
        case TYPE_SCS:
            return m_scsInfo.overallPI;
        case TYPE_SS:
            return m_ssInfo.overallPI;
        default:
            {
            ROADRAILALIGNMENT_LOGE("AlignmentPI::GetPILocation - unexpected PI type");
            DPoint3d point;
            point.InitDisconnect();
            return point;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPI::SetPILocation(DPoint3dCR piPoint)
    {
    switch (m_type)
        {
        case TYPE_NoCurve:
            {
            m_noCurveInfo.piPoint = piPoint;
            return true;
            }
        case TYPE_Arc:
            {
            m_arcInfo.arc.piPoint = piPoint;
            return true;
            }
        case TYPE_SCS:
            {
            m_scsInfo.overallPI = piPoint;
            return true;
            }
        case TYPE_SS:
            {
            m_ssInfo.overallPI = piPoint;
            }
        default:
            {
            return false;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
// This is just to be able to validate that PIs are not overlapping each other
// The returned value doesn't make any sense in the civil world.
//---------------------------------------------------------------------------------------
double AlignmentPI::GetPseudoTangentLength() const
    {
    switch (m_type)
        {
        case TYPE_NoCurve:
            return 0.0;
        case TYPE_Arc:
            return m_arcInfo.arc.piPoint.DistanceXY(m_arcInfo.arc.startPoint);
        case TYPE_SCS:
            return m_scsInfo.overallPI.DistanceXY(m_scsInfo.spiral1.startPoint);
        case TYPE_SS:
            return m_ssInfo.overallPI.DistanceXY(m_ssInfo.spiral1.startPoint);
        default:
            {
            ROADRAILALIGNMENT_LOGE("AlignmentPI::GetPseudoTangentLength() - unexpected PI type");
            return 0.0;
            }
        }
    }


//=======================================================================================
// AlignmentPairEditor
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AlignmentPairEditor::AlignmentPairEditor() :
    T_Super()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditor::AlignmentPairEditor (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment) :
    T_Super (horizontalAlignment, verticalAlignment)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AlignmentPairPtr AlignmentPairEditor::_Clone() const
    {
    return AlignmentPairEditor::Create(GetHorizontalCurveVector(), GetVerticalCurveVector());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor::_UpdateHorizontalCurveVector(CurveVectorCR horizontalAlignment)
    {
    T_Super::_UpdateHorizontalCurveVector(horizontalAlignment);
    m_cachedPIs.clear();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor::_UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment)
    {
    T_Super::_UpdateVerticalCurveVector(pVerticalAlignment);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
DPoint3d AlignmentPairEditor::ComputePIFromPointsAndVectors(DPoint3d pointA, DVec3d vectorA, DPoint3d pointB, DVec3d vectorB) const
    {
    pointA.z = 0.0;
    vectorA.z = 0.0;
    pointB.z = 0.0;
    vectorB.z = 0.0;

    const DRay3d rayA = DRay3d::FromOriginAndVector(pointA, vectorA);
    const DRay3d rayB = DRay3d::FromOriginAndVector(pointB, vectorB);
    double fractionA, fractionB;
    DPoint3d resultA, resultB;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay(fractionA, fractionB, resultA, resultB, rayA, rayB);
    return resultA;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::LoadArcData(AlignmentPI::Arc& arc, ICurvePrimitiveCR primitiveArc) const
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != primitiveArc.GetCurvePrimitiveType())
        {
        ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::LoadArcData - not an arc");
        return false;
        }

    DEllipse3dCP pEllipse = primitiveArc.GetArcCP();
    if (pEllipse->IsFullEllipse())
        {
        ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::LoadArcData - arc is a full ellipse");
        return false;
        }

    if (!pEllipse->IsCircularXY(arc.radius))
        {
        ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::LoadArcData - not a circular arc");
        return false;
        }

    DPoint3d start, end;
    DVec3d startTangent, endTangent;
    primitiveArc.GetStartEnd(start, end, startTangent, endTangent);
    start.z = 0.0;
    end.z = 0.0;
    startTangent.z = 0.0;
    endTangent.z = 0.0;

    arc.startPoint = start;
    arc.endPoint = end;
    arc.piPoint = ComputePIFromPointsAndVectors(start, startTangent, end, endTangent);
    arc.centerPoint = pEllipse->center;
    arc.centerPoint.z = 0.0;
    arc.orientation = pEllipse->IsCCWSweepXY() ? AlignmentPI::ORIENTATION_CCW : AlignmentPI::ORIENTATION_CW;
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::LoadSpiralData(AlignmentPI::Spiral& spiral, ICurvePrimitiveCR primitiveSpiral) const
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral != primitiveSpiral.GetCurvePrimitiveType())
        {
        ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::LoadSpiralData - not a spiral");
        return false;
        }

    DSpiral2dPlacementCP pPlacement = primitiveSpiral.GetSpiralPlacementCP();
    DSpiral2dBaseCP pSpiral = pPlacement->spiral;

    
    primitiveSpiral.GetStartEnd(spiral.startPoint, spiral.endPoint, spiral.startVector, spiral.endVector);
    spiral.startPoint.z = 0.0;
    spiral.endPoint.z = 0.0;
    spiral.startVector.z = 0.0;
    spiral.endVector.z = 0.0;

    spiral.piPoint = ComputePIFromPointsAndVectors(spiral.startPoint, spiral.startVector, spiral.endPoint, spiral.endVector);

    if (0.0 == pSpiral->mCurvature0)
        spiral.startRadius = (0.0 < pSpiral->mCurvature1) ? -1.0 * CS_SPI_INFINITY : CS_SPI_INFINITY;
    else
        spiral.startRadius = -1.0 / pSpiral->mCurvature0;

    if (0.0 == pSpiral->mCurvature1)
        spiral.endRadius = (0.0 < pSpiral->mCurvature0) ? -1.0 * CS_SPI_INFINITY : CS_SPI_INFINITY;
    else
        spiral.endRadius = -1.0 / pSpiral->mCurvature1;

#if 0 //&&Ag following code is from DgnDb06 branch. not sure which one is the good one ?!
    DSpiral2dPlacementCP plSpiral = primitive->GetSpiralPlacementCP ();
    DSpiral2dBaseP vectorSpiral = plSpiral->spiral;

    //frenet
    // DVec3d zVector;
    // transform.GetMatrixColumn(zVector, 2);
    // per Earlin...
    // The curvature from the frenet frame is always positive in the direction of the frenet frame Y vector.X is forward,
    // Y is toward center of curvature.The transform's Z Vector is (exactly) X cross Y).  So if the Z vector points up 
    // (Zvector.z > 0) the spiral is curling to the left.  If ZVector.z < 0 the spiral is curling to the right.

    Transform frame0;
    Transform frame1;
    double curvature0, curvature1, torsion;
    primitive->FractionToFrenetFrame(0, frame0, curvature0, torsion);
    primitive->FractionToFrenetFrame(1, frame1, curvature1, torsion);
    DVec3d zVector1;
    DVec3d zVector0;
    frame0.GetMatrixColumn(zVector0, 2);
    frame1.GetMatrixColumn(zVector1, 2);
    if (curvature0 == 0.0)
        {
        if (zVector1.z > 0.0)
            roadSpiral.entranceRadius = -1.0 * CS_SPI_INFINITY;
        else
            roadSpiral.entranceRadius = CS_SPI_INFINITY;
        }
    else
        roadSpiral.entranceRadius = -1.0 / curvature0;

    if (vectorSpiral->mCurvature1 == 0.0)
        {
        if (zVector0.z > 0.0)
            roadSpiral.exitRadius = -1.0 * CS_SPI_INFINITY;
        else
            roadSpiral.exitRadius = CS_SPI_INFINITY;
        }
    else
        roadSpiral.exitRadius = -1.0 / vectorSpiral->mCurvature1;

#endif

    spiral.length = pSpiral->mLength;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetLinePI(AlignmentPIR pi, size_t index) const
    {
    CurveVectorCR hz = GetHorizontalCurveVector();

    if (index + 1 < hz.size() && hz[index + 1]->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        DPoint3d dummy, end;
        hz[index]->GetStartEnd(dummy, end);
        end.z = 0.0;

        pi.InitInvalid(AlignmentPI::TYPE_NoCurve);
        pi.GetNoCurveP()->piPoint = end;
        return true;
        }

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetArcPI(AlignmentPIR pi, size_t primitiveIdx) const
    {
    pi.InitInvalid(AlignmentPI::TYPE_Arc);
    AlignmentPI::ArcInfoP pArc = pi.GetArcP();

    if (!LoadArcData(pArc->arc, *GetHorizontalCurveVector()[primitiveIdx]))
        return false;

    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetSCSPI(AlignmentPIR pi, size_t primitiveIdx) const
    {
    CurveVectorCR hz = GetHorizontalCurveVector();
    if (primitiveIdx + 2 >= hz.size())
        return false;

    pi.InitInvalid(AlignmentPI::TYPE_SCS);
    AlignmentPI::SCSInfoP pSCS = pi.GetSCSP();

    if (!LoadSpiralData(pSCS->spiral1, *hz[primitiveIdx]))
        return false;

    if (!LoadArcData(pSCS->arc, *hz[primitiveIdx + 1]))
        return false;

    if (!LoadSpiralData(pSCS->spiral2, *hz[primitiveIdx + 2]))
        return false;

    pSCS->overallPI = ComputePIFromPointsAndVectors(pSCS->spiral1.startPoint, pSCS->spiral1.startVector, pSCS->spiral2.endPoint, pSCS->spiral2.endVector);
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetSSPI(AlignmentPIR pi, size_t primitiveIdx) const
    {
    CurveVectorCR hz = GetHorizontalCurveVector();
    if (primitiveIdx + 1 >= hz.size())
        return false;

    pi.InitInvalid(AlignmentPI::TYPE_SS);
    AlignmentPI::SSInfoP pSS = pi.GetSSP();

    if (!LoadSpiralData(pSS->spiral1, *hz[primitiveIdx]))
        return false;

    if (!LoadSpiralData(pSS->spiral2, *hz[primitiveIdx + 1]))
        return false;

    pSS->overallPI = ComputePIFromPointsAndVectors(pSS->spiral1.startPoint, pSS->spiral1.startVector, pSS->spiral2.endPoint, pSS->spiral2.endVector);
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bvector<AlignmentPI> AlignmentPairEditor::GetPIs() const
    {
    CurveVectorCR hz = GetHorizontalCurveVector();
    if (hz.empty())
        return bvector<AlignmentPI>();

    if (!m_cachedPIs.empty())
        return m_cachedPIs;

    DPoint3d hzStart, hzEnd;
    hz.GetStartEnd(hzStart, hzEnd);
    hzStart.z = hzEnd.z = 0.0;

    bool isError = false;
    bvector<AlignmentPI> pis;

    for (size_t i = 0; i < hz.size(); ++i)
        {
        switch (hz[i]->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                AlignmentPI pi;
                if (0 == i)
                    {
                    pi.InitNoCurve(hzStart);
                    pis.push_back(pi);
                    }

                if (GetLinePI(pi, i))
                    pis.push_back(pi);

                //! Unlike other primitives, failure to read a Line PI isn't an error
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                AlignmentPI pi;
                if (GetArcPI(pi, i))
                    pis.push_back(pi);
                else
                    isError = true;

                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                if (i + 1 >= hz.size())
                    {
                    isError = true;
                    break;
                    }

                ICurvePrimitive::CurvePrimitiveType nextType = hz[i + 1]->GetCurvePrimitiveType();
                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == nextType)
                    {
                    if (i + 2 >= hz.size() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral != hz[i + 2]->GetCurvePrimitiveType())
                        {
                        isError = true;
                        break;
                        }

                    AlignmentPI pi;
                    if (GetSCSPI(pi, i))
                        pis.push_back(pi);
                    else
                        isError = true;

                    i += 2; // Skip the next 2 primitives (arc, spiral)
                    }    
                else if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral == nextType)
                    {
                    AlignmentPI pi;
                    if (GetSSPI(pi, i))
                        pis.push_back(pi);
                    else
                        isError = true;

                    i += 1; // Skip the next primitive (spiral)
                    }
                else
                    isError = true;

                break;
                }
            default:
                {
                ROADRAILALIGNMENT_LOGE("AlignmentPairEditorTest::GetPIs - Unexpected primitive type");
                isError = true;
                break;
                }
            }

        if (isError)
            {
            ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::GetPIs - error");
            return bvector<AlignmentPI>();
            }
        }


    // Add the end PI unless we end with a spiral or arc
    ICurvePrimitive::CurvePrimitiveType pType = hz.back()->GetCurvePrimitiveType();
    DPoint3d primStart, primEnd;
    hz.back()->GetStartEnd(primStart, primEnd);

    if (!primEnd.AlmostEqualXY(hzEnd) || (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != pType && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral != pType))
        {
        AlignmentPI endPI;
        endPI.InitNoCurve(hzEnd);
        pis.push_back(endPI);
        }


#ifndef NDEBUG
    // DEBUG-Only sanity check. We should be able to rebuild the curve off the PIs we've just retrieved
    CurveVectorPtr sanityPVICurve = _BuildCurveVectorFromPIs(pis);
    BeAssert(sanityPVICurve.IsValid());
#endif

    m_cachedPIs = pis;
    return pis;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr AlignmentPairEditor::BuildArc(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, AlignmentPI::Orientation orientation) const
    {
    if (0.0 >= radius)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildArc - null or negative radius");
        return nullptr;
        }

    const DVec3d v0 = DVec3d::FromStartEndNormalize(currPI, prevPI);
    const DVec3d v1 = DVec3d::FromStartEndNormalize(currPI, nextPI);
    const DVec3d poi = DVec3d::From(currPI);

    // Returns angle between -pi to +pi
    const double angle = std::abs(v0.AngleToXY(v1));
    if (0.0 == angle)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildArc - null angle");
        return nullptr;
        }

    DVec3d bisector = DVec3d::FromSumOf(v0, v1);
    bisector.ScaleToLength(radius / sin(0.5 * angle));
    const DPoint3d center = DPoint3d::FromSumOf(poi, bisector);

    // Compute distance from poi tangentArc
    const double tangentArcLength = radius / tan(0.5 * angle);
    const DPoint3d tangentArcPt = DPoint3d::FromSumOf(poi, v0, tangentArcLength);
    const DPoint3d arcTangentPt = DPoint3d::FromSumOf(poi, v1, tangentArcLength);

    if (tangentArcPt.DistanceSquaredXY(poi) > prevPI.DistanceSquaredXY(poi) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildArc - tangentArcPt validation failed");
        return nullptr;
        }
    if (arcTangentPt.DistanceSquaredXY(poi) > nextPI.DistanceSquaredXY(poi) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildArc - arcTangentPt validation failed");
        return nullptr;
        }

    DEllipse3d ellipse;
    if (!ellipse.InitFromArcCenterStartEnd(center, tangentArcPt, arcTangentPt))
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildArc - failed to create arc");
        return nullptr;
        }

    if (AlignmentPI::ORIENTATION_Unknown != orientation)
        {
        if (ellipse.IsCCWSweepXY() && AlignmentPI::ORIENTATION_CCW != orientation)
            ellipse.ComplementSweep();
        }

    return ICurvePrimitive::CreateArc(ellipse);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr AlignmentPairEditor::BuildArc(AlignmentPI::ArcInfoCR info) const
    {
    return BuildArc(info.arc.startPoint, info.arc.piPoint, info.arc.endPoint, info.arc.radius, info.arc.orientation);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::BuildSCSCurve(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, double spiralLength1, double spiralLength2) const
    {
    if (0.0 >= radius)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - null or negative radius");
        return nullptr;
        }
    if (0.0 >= spiralLength1 || 0.0 >= spiralLength2)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - null or negative spiral length");
        return nullptr;
        }

    std::unique_ptr<DSpiral2dBase> spiralA(DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid));
    std::unique_ptr<DSpiral2dBase> spiralB(DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid));
    DPoint3d lineToSpiralA;
    DPoint3d lineToSpiralB;
    DPoint3d spiralAToArc;
    DPoint3d spiralBToArc;
    DEllipse3d arc;

    if (false == DSpiral2dBase::LineSpiralArcSpiralLineTransition(prevPI, nextPI, currPI, radius, spiralLength1, spiralLength2,
        *spiralA, *spiralB, lineToSpiralA, lineToSpiralB, spiralAToArc, spiralBToArc, arc))
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - failed to create SCS curve");
        return nullptr;
        }

    // Validate that the transition is continuous by analyzing the progression of the angles
    const DVec3d v00 = DVec3d::FromStartEnd(currPI, prevPI);
    const DVec3d v11 = DVec3d::FromStartEnd(currPI, nextPI);
    const DVec3d vec0 = DVec3d::FromStartEnd(currPI, spiralAToArc);
    const DVec3d vec1 = DVec3d::FromStartEnd(currPI, spiralBToArc);
    const DVec3d vec2 = DVec3d::FromStartEnd(currPI, lineToSpiralB);
    const DVec3d orientation = DVec3d::FromCrossProduct (v00, v11);
    const bool sign0 = v00.SignedAngleTo (v11, orientation) >= 0.0;
    const bool sign1 = vec0.SignedAngleTo (vec1, orientation) >= 0.0;
    const bool sign2 = vec1.SignedAngleTo (vec2, orientation) >= 0.0;
    if (sign0 != sign1 || sign0 != sign2)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - validation of angle progression failed");
        return nullptr;
        
        // sdevtodo - _SolveSSPI should return false in many cases...
        //pis.at (index).curveType = AlignmentPI::HorizontalPIType::SS;
        //return _SolveSSPI (index, pis);
        }

    if (lineToSpiralA.DistanceSquaredXY(currPI) > prevPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - lineToSpiralA validation failed");
        return nullptr;
        }
    if (lineToSpiralB.DistanceSquaredXY(currPI) > nextPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSCSCurve - lineToSpiralB validation failed");
        return nullptr;
        }

    // Valid solution!
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

    ICurvePrimitivePtr sp1 = ICurvePrimitive::CreateSpiral (*spiralA, frameA, 0.0, 1.0);
    ICurvePrimitivePtr sp2 = ICurvePrimitive::CreateSpiral (*spiralB, frameB, 0.0, 1.0);
    ICurvePrimitivePtr ac = ICurvePrimitive::CreateArc(arc);

    if (!sp1.IsValid() || !sp2.IsValid() || !ac.IsValid())
        return nullptr;

    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    cv->push_back(sp1);
    cv->push_back(ac);
    cv->push_back(sp2);
    return cv;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::BuildSCSCurve(AlignmentPI::SCSInfoCR info) const
    {
    return BuildSCSCurve(info.spiral1.startPoint, info.overallPI, info.spiral2.endPoint, info.arc.radius, info.spiral1.length, info.spiral2.length);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::BuildSSCurve(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double spiralLength) const
    {
    std::unique_ptr<DSpiral2dBase> spiralA(DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid));
    std::unique_ptr<DSpiral2dBase> spiralB(DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid));
    DPoint3d lineToSpiralA;
    DPoint3d lineToSpiralB;
    DPoint3d spiralToSpiral;
    double radius;

    if (false == DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition(prevPI, nextPI, currPI, spiralLength,
        *spiralA, *spiralB, lineToSpiralA, lineToSpiralB, spiralToSpiral, radius))
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSSCurve - failed to create SS curve");
        return nullptr;
        }

    if (lineToSpiralA.DistanceSquaredXY(currPI) > prevPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSSCurve - lineToSpiralA validation failed");
        return nullptr;
        }
    if (lineToSpiralB.DistanceSquaredXY(currPI) > nextPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::BuildSSCurve - lineToSpiralB validation failed");
        return nullptr;
        }

    // Valid solution!
    const Transform frameA = Transform::From(lineToSpiralA);
    const Transform frameB = Transform::From(spiralToSpiral);

    // SpiralB is oriented from tangentB to Arc. Reverse it to make a continuous road path (tangentA to tangentB)
    const double theta0 = spiralB->mTheta0;
    const double theta1 = spiralB->mTheta1;
    const double curvature0 = spiralB->mCurvature0;
    const double curvature1 = spiralB->mCurvature1;
    spiralB->mTheta0 = theta1 + Angle::Pi ();
    spiralB->mTheta1 = theta0 + Angle::Pi ();
    spiralB->mCurvature0 = -curvature1;
    spiralB->mCurvature1 = -curvature0;

    ICurvePrimitivePtr sp1 = ICurvePrimitive::CreateSpiral(*spiralA, frameA, 0.0, 1.0);
    ICurvePrimitivePtr sp2 = ICurvePrimitive::CreateSpiral(*spiralB, frameB, 0.0, 1.0);
    if (!sp1.IsValid() || !sp2.IsValid())
        return nullptr;

    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    cv->push_back(sp1);
    cv->push_back(sp2);
    return cv;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::BuildSSCurve(AlignmentPI::SSInfoCR info) const
    {
    return BuildSSCurve(info.spiral1.startPoint, info.overallPI, info.spiral2.endPoint, info.spiral1.length);
    }

BEGIN_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
// Adds a line primitive to the curve.
// 'lastPoint' becomes the line end point
//---------------------------------------------------------------------------------------
void AddLineToCurve(CurveVectorR curve, DPoint3dR lastPoint, DPoint3dCR point)
    {
    curve.push_back(ICurvePrimitive::CreateLine(DSegment3d::From(lastPoint, point)));
    lastPoint = point;
    }
END_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::_BuildCurveVectorFromPIs(bvector<AlignmentPI> const& pis) const
    {
    if (2 > pis.size())
        return nullptr;

    BeAssert(_ValidatePIs(pis) && "Caller is expected to validate PIs before calling this");

    CurveVectorPtr hzCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d lastPoint;
    lastPoint.InitDisconnect();

    for (int i = 0; i < pis.size(); ++i)
        {
        switch (pis[i].GetType())
            {
            case AlignmentPI::TYPE_NoCurve:
                {
                AlignmentPI::NoCurveInfoCP pInfo = pis[i].GetNoCurve();

                if (0 == i)
                    lastPoint = pInfo->piPoint;
                else
                    AddLineToCurve(*hzCurve, lastPoint, pInfo->piPoint);

                break;
                }
            case AlignmentPI::TYPE_Arc:
                {
                ICurvePrimitivePtr primitive = BuildArc(*pis[i].GetArc());
                if (!primitive.IsValid())
                    return nullptr;

                DPoint3d pStart, pEnd;
                primitive->GetStartEnd(pStart, pEnd);

                if (0 < i && !lastPoint.AlmostEqualXY(pStart))
                    AddLineToCurve(*hzCurve, lastPoint, pStart);
                
                hzCurve->push_back(primitive);
                lastPoint = pEnd;
                break;
                }
            case AlignmentPI::TYPE_SCS:
            case AlignmentPI::TYPE_SS:
                {
                CurveVectorPtr curve;
                if (AlignmentPI::TYPE_SCS == pis[i].GetType())
                    curve = BuildSCSCurve(*pis[i].GetSCS());
                else
                    curve = BuildSSCurve(*pis[i].GetSS());

                if (!curve.IsValid())
                    return nullptr;

                DPoint3d cStart, cEnd;
                curve->GetStartEnd(cStart, cEnd);

                if (0 < i && !lastPoint.AlmostEqualXY(cStart))
                    AddLineToCurve(*hzCurve, lastPoint, cStart);

                hzCurve->AddPrimitives(*curve);
                lastPoint = cEnd;
                break;
                }
            default:
                {
                ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::_BuildCurveVectorFromPIs - invalid PI type");
                return nullptr;
                }
            }
        }

    if (!hzCurve->empty())
        return hzCurve;

    return nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::SolveArcPI(bvector<AlignmentPI>& pis, size_t index) const
    {
    if (index >= pis.size())
        return false;

    AlignmentPI::ArcInfoP pInfo = pis[index].GetArcP();
    BeAssert(nullptr != pInfo);

    if (0 == index || index + 1 == pis.size())
        {
        // If we want to solve start or end PI, we need to make sure we have valid data inside to solve it.
        if (pInfo->arc.startPoint.AlmostEqualXY(pInfo->arc.endPoint))
            return false;
        }

    const DPoint3d prevPI = (0 < index) ? pis[index - 1].GetPILocation() : pInfo->arc.startPoint;
    const DPoint3d currPI = pis[index].GetPILocation();
    const DPoint3d nextPI = (index + 1 < pis.size()) ? pis[index + 1].GetPILocation() : pInfo->arc.endPoint;
    const double radius = pInfo->arc.radius;
    const AlignmentPI::Orientation orientation = pInfo->arc.orientation;

    ICurvePrimitivePtr primitive = BuildArc(prevPI, currPI, nextPI, radius, orientation);
    if (!primitive.IsValid())
        return false;

    // Update PI with new data
    if (!LoadArcData(pInfo->arc, *primitive))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* "fit" the SCS - potential modifications are the overall PI or the radius
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::SolveSCSPI(bvector<AlignmentPI>& pis, size_t index) const
    {
    if (0 == index || index + 1 >= pis.size())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::SolveSCSPI - Spiral PI is first or last PI");
        return false;
        }

    AlignmentPI::SCSInfoP pInfo = pis[index].GetSCSP();
    BeAssert(nullptr != pInfo);

    const DPoint3d prevPI = pis[index - 1].GetPILocation();
    const DPoint3d currPI = pis[index].GetPILocation();
    const DPoint3d nextPI = pis[index + 1].GetPILocation();
    const double radius = pInfo->arc.radius;
    const double spiralLength1 = pInfo->spiral1.length;
    const double spiralLength2 = pInfo->spiral2.length;

    CurveVectorPtr scsCurve = BuildSCSCurve(prevPI, currPI, nextPI, radius, spiralLength1, spiralLength2);
    if (!scsCurve.IsValid())
        return false;

    // Update PI with new data
    if (!LoadSpiralData(pInfo->spiral1, *scsCurve->at(0)))
        return false;

    if (!LoadArcData(pInfo->arc, *scsCurve->at(1)))
        return false;

    if (!LoadSpiralData(pInfo->spiral2, *scsCurve->at(2)))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
* overall PI changed or Length change
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::SolveSSPI(bvector<AlignmentPI>& pis, size_t index) const
    {
    if (0 == index || index + 1 >= pis.size())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::SolveSSPI - Spiral PI is first or last PI");
        return false;
        }

    AlignmentPI::SSInfoP pInfo = pis[index].GetSSP();
    BeAssert(nullptr != pInfo);

    if (0.0 >= pInfo->spiral1.length)
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::SolveSSPI - Null or negative spiral length");
        return false;
        }

    const DPoint3d prevPI = pis[index - 1].GetPILocation();
    const DPoint3d currPI = pis[index].GetPILocation();
    const DPoint3d nextPI = pis[index + 1].GetPILocation();
    const double spiralLength = pInfo->spiral1.length;

    CurveVectorPtr ssCurve = BuildSSCurve(prevPI, currPI, nextPI, spiralLength);
    if (!ssCurve.IsValid())
        return false;

    // Update PI with new data
    if (!LoadSpiralData(pInfo->spiral1, *ssCurve->at(0)))
        return false;

    if (!LoadSpiralData(pInfo->spiral2, *ssCurve->at(1)))
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_SolvePI(bvector<AlignmentPI>& pis, size_t index) const
    {
    if (index >= pis.size())
        return false;

    switch (pis[index].GetType())
        {
        case AlignmentPI::TYPE_NoCurve:
            return true;
        case AlignmentPI::TYPE_Arc:
            return SolveArcPI(pis, index);
        case AlignmentPI::TYPE_SCS:
            return SolveSCSPI(pis, index);
        case AlignmentPI::TYPE_SS:
            return SolveSSPI(pis, index);
        default:
            {
            ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::_SolvePI - Invalid PI Type");
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_ValidatePIs(bvector<AlignmentPI> const& pis) const
    {
    for (size_t i = 0; i < pis.size() - 1; ++i)
        {
        const double piDistance = pis[i].GetPILocation().DistanceXY(pis[i + 1].GetPILocation());
        const double sumPseudoTangentLengths = pis[i].GetPseudoTangentLength() + pis[i + 1].GetPseudoTangentLength();
        if (0.0 == piDistance || piDistance < sumPseudoTangentLengths)
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment)
    {
    return new AlignmentPairEditor(horizontalAlignment, pVerticalAlignment);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create (AlignmentPairCR pair)
    {
    return AlignmentPairEditor::Create(pair.GetHorizontalCurveVector(), pair.GetVerticalCurveVector());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPI(AlignmentPICR pi) const
    {
    // Find the index to insert this point
    bvector<AlignmentPI> pis = GetPIs();
    if (2 > pis.size())
        return nullptr;

    // first find nearest point
    size_t nearestIndex = 0;
    double distance = DBL_MAX;

    const DPoint3d piLocation = pi.GetPILocation();

    for (size_t i = 0; i < pis.size (); ++i)
        {
        const double testDistance = piLocation.DistanceSquaredXY(pis[i].GetPILocation());
        if (testDistance < distance)
            {
            nearestIndex = i;
            distance = testDistance;
            }
        }

    if (DBL_MAX == distance || 0.0 == distance)
        return nullptr;

    size_t index;
    // test the direction...
    if (nearestIndex == 0)
        index = 1;
    else if (nearestIndex == (pis.size() - 1))
        index = pis.size () - 1;
    else
        {
        const double newPiDistFromStart = HorizontalDistanceAlongFromStart(piLocation);
        const double nearestDistFromStart = HorizontalDistanceAlongFromStart(pis[nearestIndex].GetPILocation(), nullptr);
        index = (newPiDistFromStart < nearestDistFromStart) ? nearestIndex : nearestIndex + 1;
        }

    return InsertPI(pi, index);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPI(AlignmentPICR pi, size_t index) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (2 > pis.size())
        return nullptr;

    pis.insert(pis.begin () + index, pi);

    if (0 < index && !_SolvePI(pis, index - 1))
        return nullptr;

    if (!_SolvePI(pis, index))
        return nullptr;

    if (index + 1 < pis.size() && !_SolvePI(pis, index + 1))
        return nullptr;

    if (_ValidatePIs(pis))
        {
        CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
        if (hzGeom.IsValid ())
            return hzGeom;
        }
    return nullptr;
    }



#if 0
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
    UpdateCurveVectors (*hz, vt.get ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_GenerateFromSingleVector (CurveVectorCR curveVector)
    {
    UpdateCurveVectors (curveVector, nullptr);
    bvector<AlignmentPI> pis = GetPIs(); // flatten to zero;
    CurveVectorPtr hzAlign = _BuildCurveVectorFromPIs(pis);
    if (!hzAlign.IsValid ()) return false;

    UpdateCurveVectors (*hzAlign, nullptr);
    
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
        double x = HorizontalDistanceAlongFromStart (end);
        DPoint3d location2 = DPoint3d::From (x, 0.0, end.z);
        AlignmentPVI pvi2 (location2, 0.0);
        pvis.push_back (pvi);
        }
    CurveVectorPtr vtAlign = _BuildVectorFromPVIS (pvis);
    if (!vtAlign.IsValid ()) return false;
    
    UpdateCurveVectors(*hzAlign, vtAlign.get ());

    return true;
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

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ben.Bartholomew                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::GetPI(AlignmentPIR pi, size_t index) const
    {
    auto pis = GetPIs();
    if (index < pis.size())
        {
        pi = pis.at(index);
        return true;
        }

    return false;

    }

#if 0

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
        pt.z = GetVerticalElevationAt(pt.x);
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
    if (!IsValidVertical())
        return pvis;

    return _GetPVIs(*GetVerticalCurveVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AlignmentPVI> AlignmentPairEditor::_GetPVIs(CurveVectorCR vt)
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
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePI(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    // Copy the existing PI
    AlignmentPI newPI = pis[index];
    newPI.SetPILocation(DPoint3d::From(toPt.x, toPt.y, 0.0));

    CurveVectorPtr result = MovePI(index, newPI);
    if (nullptr != pOutPI && result.IsValid())
        *pOutPI = newPI;

    return result;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MovePI(size_t index, AlignmentPIR inOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    pis[index] = inOutPI;

    if (0 < index && !_SolvePI(pis, index - 1))
        return nullptr;

    if (!_SolvePI(pis, index))
        return nullptr;

    if (index + 1 < pis.size() && !_SolvePI(pis, index + 1))
        return nullptr;

    if (!_ValidatePIs(pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (hzGeom.IsValid())
        inOutPI = pis[index];

    return hzGeom;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePCorPT(size_t index, DPoint3dCR toPt, bool isPC, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    if (0 == index || index + 1 == pis.size())
        return nullptr; // todo start and end?

    AlignmentPI::Arc* pArc = nullptr;

    if (AlignmentPI::TYPE_Arc == pis[index].GetType())
        pArc = &pis[index].GetArcP()->arc;
    else if (AlignmentPI::TYPE_SCS == pis[index].GetType())
        pArc = &pis[index].GetSCSP()->arc;
    else
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::MovePCorPT - Not an Arc or SCS transition");
        return nullptr;
        }

    DPoint3dCR arcPoint = isPC ? pArc->startPoint : pArc->endPoint;
    // compute a new radius then just solve the PI
    // T = R tan I/2
    // update the radius R = T/tani/2
    const double tangentLength = toPt.DistanceXY(pis[index].GetPILocation());
    double taniover2 = pis[index].GetPILocation().DistanceXY(arcPoint) / pArc->radius;
    pArc->radius = tangentLength / taniover2;

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs(pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pis[index];

    return hzGeom;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePC (size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI) const
    { 
    return MovePCorPT(index, toPt, true/*isPC*/, pOutPI);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePT(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI) const
    {
    return MovePCorPT(index, toPt, false/*isPC*/, pOutPI);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::UpdateRadius(size_t index, double radius, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    if (0 == index || index + 1 == pis.size())
        return nullptr; // todo start and end?

    AlignmentPIR pi = pis[index];
    if (AlignmentPI::TYPE_Arc == pi.GetType())
        {
        pi.GetArcP()->arc.radius = radius;
        }
    else if (AlignmentPI::TYPE_SCS == pi.GetType())
        {
        pi.GetSCSP()->arc.radius = radius;
        }
    else
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::UpdateRadius - PI is not an Arc or SCS");
        return nullptr;
        }

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs (pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pi;

    return hzGeom;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::UpdateSpiralLengths(size_t index, double spiralLength, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    if (0 == index || index + 1 == pis.size())
        return nullptr; // todo start and end?

    AlignmentPIR pi = pis[index];
    if (AlignmentPI::TYPE_SCS == pi.GetType())
        {
        AlignmentPI::SCSInfoP pInfo = pi.GetSCSP();
        pInfo->spiral1.length = spiralLength;
        pInfo->spiral2.length = spiralLength;
        }
    else if (AlignmentPI::TYPE_SS == pi.GetType())
        {
        AlignmentPI::SSInfoP pInfo = pi.GetSSP();
        pInfo->spiral1.length = spiralLength;
        pInfo->spiral2.length = spiralLength;
        }
    else
        {
        ROADRAILALIGNMENT_LOGE("AlignmentPairEditor::UpdateSpiralLength - PI is not a SCS or SS");
        return nullptr;
        }

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs(pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pi;

    return hzGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::RemoveSpirals(size_t index, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    if (0 == index || index + 1 == pis.size())
        return nullptr; // todo start and end?

    if (AlignmentPI::TYPE_SCS != pis[index].GetType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::RemoveSpirals - PI is not a SCS");
        return nullptr;
        }

    AlignmentPI::SCSInfoCP pSCS = pis[index].GetSCS();
    
    AlignmentPI newPI;
    newPI.InitArc(pSCS->overallPI, pSCS->arc.radius);
    pis[index] = newPI;

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs(pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pis[index];

    return hzGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::AddSpirals(size_t index, double spiralLength, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    if (0 == index || index + 1 == pis.size())
        return nullptr; // todo start and end?

    if (AlignmentPI::TYPE_Arc != pis[index].GetType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPairEditor::AddSpirals - PI is not a Arc");
        return nullptr;
        }

    AlignmentPI::ArcInfoCP pArc = pis[index].GetArc();
    
    AlignmentPI newPI;
    newPI.InitSCS(pArc->arc.piPoint, pArc->arc.radius, spiralLength, spiralLength);
    pis[index] = newPI;

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs(pis))
        return nullptr;

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pis[index];

    return hzGeom;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MoveBSorES(size_t index, DPoint3dCR toPt, bool isBS, AlignmentPI* pOutPI) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size())
        return nullptr;

    AlignmentPI::Type piType = pis[index].GetType();
    if (AlignmentPI::TYPE_Arc != piType && AlignmentPI::TYPE_SCS != piType)
        return nullptr;

    if (index == 0 || index >= (pis.size() - 1))
        return nullptr; // todo start and end?

    AlignmentPI::Spiral& spiral1 = (AlignmentPI::TYPE_SCS == piType) ? pis[index].GetSCSP()->spiral1 : pis[index].GetSSP()->spiral1;
    AlignmentPI::Spiral& spiral2 = (AlignmentPI::TYPE_SCS == piType) ? pis[index].GetSCSP()->spiral2 : pis[index].GetSSP()->spiral2;

    const double spiralLength = isBS ? toPt.DistanceXY(spiral1.endPoint) : toPt.DistanceXY(spiral2.startPoint);
    spiral1.length = spiralLength;
    spiral2.length = spiralLength;

    if (!_SolvePI(pis, index))
        return nullptr;
    if (!_ValidatePIs(pis))
        return nullptr;

    // SCS checks only
    if (auto pSCS = pis[index].GetSCS())
        {
        if (isBS)
            {
            if (toPt.DistanceXY(spiral1.endPoint) >= toPt.DistanceXY(pSCS->arc.endPoint))
                return nullptr;
            }
        else
            {
            if (toPt.DistanceXY(spiral1.endPoint) <= toPt.DistanceXY(pSCS->arc.endPoint))
                return nullptr;

            if (toPt.DistanceXY(spiral2.startPoint) >= toPt.DistanceXY(pSCS->arc.startPoint))
                return nullptr;
            }
        }

    CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
    if (nullptr != pOutPI && hzGeom.IsValid())
        *pOutPI = pis[index];

    return hzGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveBS(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI) const
    {
    return MoveBSorES(index, toPt, true/*isBS*/, pOutPI);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveES(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI) const
    {
    return MoveBSorES(index, toPt, false/*isBS*/, pOutPI);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePI(DPoint3dCR piPoint) const
    {
    const bvector<AlignmentPI> pis = GetPIs();
    for (size_t i = 0; i < pis.size(); ++i)
        {
        if (piPoint.AlmostEqualXY(pis[i].GetPILocation()))
            return DeletePI(i);
        }

    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePI(size_t index) const
    {
    bvector<AlignmentPI> pis = GetPIs();
    if (index >= pis.size ())
        return nullptr;

    if (index == 0 || index + 1 == pis.size()) // can't delete beginning or end for now
        {
        AlignmentPIR pi = pis[index];
        if (AlignmentPI::TYPE_NoCurve != pi.GetType())
            {
            // just convert to grade break
            const DPoint3d piPoint = pi.GetPILocation();
            pi.InitNoCurve(piPoint);
            return _BuildCurveVectorFromPIs(pis);
            }
        return nullptr;
        }

    pis.erase(pis.begin() + index);

    if (!_SolvePI(pis, index - 1))
        return nullptr;

    if (!_SolvePI(pis, index))
        return nullptr;

    if (_ValidatePIs(pis))
        {
        CurveVectorPtr hzGeom = _BuildCurveVectorFromPIs(pis);
        if (hzGeom.IsValid())
            return hzGeom;
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPairEditor::GetPIPoints() const
    {
    const bvector<AlignmentPI> pis = GetPIs();

    bvector<DPoint3d> result;
    result.reserve(pis.size());

    for (auto const& pi : pis)
        result.push_back(pi.GetPILocation());

    return result;
    }

#if 0
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPairEditor::CrestAndSagPointsXZ (ZeroSlopePoints zsType)
    {
    BeAssert (IsValidVertical());
    double runningLength = 0.0;
    bvector<DPoint3d> returnVector;
    for (auto primitive : *GetVerticalCurveVector())
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
    BeAssert (IsValidVertical());
    double maxGrade = 0.0;
    for (auto primitive : *GetVerticalCurveVector())
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
    BeAssert (IsValidVertical());
    double G1 = 0.0, G2 = 0.0;
    double maxGradeChange = 0.0;
    for (auto primitive : *GetVerticalCurveVector())
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

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPairEditor::SlopeAtStation (const double& station)
    {
#if 1
    return 0.0;
#else
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
#endif
    }
#if 0
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
        UpdateVerticalCurveVector(newVertical.get());

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
            AlignmentPairEditorPtr vGeom = AlignmentPairEditor::Create (GetHorizontalCurveVector(), GetVerticalCurveVector());
            // move the two pvis vertically and update the corresponding curves if necessary
            DPoint3d newPVILocation = DPoint3d::From (pvis[i].poles[PVI].x, 0.0, pvis[i].poles[PVI].z + deltaZ);
            CurveVectorPtr newVertical;
            if ((newVertical = vGeom->MovePVI (pvis[i].poles[PVI], newPVILocation, edit1)) == nullptr)
                return nullptr;
            vGeom->UpdateVerticalCurveVector(newVertical.get());
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
        return CloneVerticalCurveVector(); // no reason to change

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
            return CloneVerticalCurveVector (); // no reason to change


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
        DPoint3d pt = GetPointAt (vec[i].x);
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
        UpdateVerticalCurveVector(newCurve.get());
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
        UpdateVerticalCurveVector(newCurve.get());
        return true;
        }
    return false;
    }

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
        UpdateVerticalCurveVector(newCurve.get());
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

    AlignmentPI calcedPI;
    StationRangeEdit rangeEdit;
    bvector<AlignmentPI> pis = GetPIs();
    if (pis.size () < 2)
        return false;

    DPoint3d referencePt;
    if (isStart)
        {
        rangeEdit.preEditRange.startStation = 0.0;
        AlignmentPI nextPI = pis[1];
        if (nextPI.curveType == AlignmentPI::NONE)
            referencePt = nextPI.GetPILocation();
        else if (nextPI.curveType == AlignmentPI::ARC)
            referencePt = nextPI.arc.endPoint;
        else if (nextPI.curveType == AlignmentPI::SCS)
            referencePt = nextPI.spiral2.endPoint;
        rangeEdit.preEditRange.endStation = HorizontalDistanceAlongFromStart(referencePt);
        newHz = MovePI (0, toPt, calcedPI);
        }
    else
        {
        rangeEdit.preEditRange.endStation = LengthXY ();
        AlignmentPI prevPI = pis[pis.size () - 2];
        if (prevPI.curveType == AlignmentPI::NONE)
            rangeEdit.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.GetPILocation());
        else if (prevPI.curveType == AlignmentPI::ARC)
            rangeEdit.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.arc.startPoint);
        else if (prevPI.curveType == AlignmentPI::SCS)
            rangeEdit.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.spiral1.startPoint);
        newHz = MovePI (pis.size () - 1, toPt, calcedPI);
        }
    if (!newHz.IsValid ()) return false;
    AlignmentPairEditorPtr modifiedAlign = AlignmentPairEditor::Create (*newHz, nullptr);
    pis = modifiedAlign->GetPIs();
    // compute the post edit range
    if (isStart)
        {
        rangeEdit.postEditRange.startStation = 0.0;
        rangeEdit.postEditRange.endStation = modifiedAlign->HorizontalDistanceAlongFromStart(referencePt);
        }
    else
        {
        rangeEdit.postEditRange.startStation = rangeEdit.preEditRange.startStation;
        rangeEdit.postEditRange.endStation = modifiedAlign->LengthXY ();
        }

    CurveVectorPtr newVertical = ModifyVerticalRange (rangeEdit, modifiedAlign->LengthXY());

#ifndef NDEBUG
    if (nullptr != GetVerticalCurveVector())
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

    UpdateCurveVectors(*newHz, newVertical.get());

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
    CurveVectorPtr hzAlign = CloneHorizontalCurveVector();
    if (!hzAlign.IsValid ()) return nullptr;

    hzAlign->ReverseCurvesInPlace ();
    if (nullptr != GetVerticalCurveVector())
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
    GetStartEnd (start, end);
    newHorizontal.GetStartEnd (newStart, newEnd);
    if (!start.AlmostEqualXY (newStart))
        isStart = true;
    else if (!end.AlmostEqualXY (newEnd))
        isEnd = true;
    DPoint3d referencePt;
    AlignmentPairEditorPtr modifiedAlign = AlignmentPairEditor::Create (newHorizontal, nullptr);
    bvector<AlignmentPI> pis = GetPIs();
    if (pis.size () < 2)
        return computedRange;
    if (isStart)
        {
        computedRange.preEditRange.startStation = 0.0;
        AlignmentPI nextPI = pis[1];
        if (nextPI.curveType == AlignmentPI::NONE)
            referencePt = nextPI.GetPILocation();
        else if (nextPI.curveType == AlignmentPI::ARC)
            referencePt = nextPI.arc.endPoint;
        else if (nextPI.curveType == AlignmentPI::SCS)
            referencePt = nextPI.spiral2.endPoint;
        computedRange.preEditRange.endStation = HorizontalDistanceAlongFromStart(referencePt);

        computedRange.postEditRange.startStation = LengthXY () - modifiedAlign->LengthXY ();
        computedRange.postEditRange.endStation = computedRange.preEditRange.endStation;
        }
    else if (isEnd)
        {
        computedRange.preEditRange.endStation = LengthXY ();
        AlignmentPI prevPI = pis[pis.size () - 2];
        if (prevPI.curveType == AlignmentPI::NONE)
            computedRange.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.GetPILocation());
        else if (prevPI.curveType == AlignmentPI::ARC)
            computedRange.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.arc.startPoint);
        else if (prevPI.curveType == AlignmentPI::SCS)
            computedRange.preEditRange.startStation = HorizontalDistanceAlongFromStart(prevPI.spiral1.startPoint);

        computedRange.postEditRange.startStation = computedRange.preEditRange.startStation;
        computedRange.postEditRange.endStation = modifiedAlign->LengthXY ();
        }
    else
        {
        CurveVectorWithDistanceIndexPtr newPath = CurveVectorWithDistanceIndex::Create ();
        CurveVectorPtr newHz = newHorizontal.Clone ();
        newPath->SetPath (newHz);

        bvector<PathLocationDetailPair> pathAIntervals;
        bvector<PathLocationDetailPair> pathBIntervals;

        GeometryDebug::Announce (*HorizontalIndexVector(), "FindCommonPaths.C");
        GeometryDebug::Announce (*newPath, "FindCommonPaths.B");


        // here is a wiki link to this function:
        // http://bsw-wiki.bentley.com/bin/view.pl/Main/CurveVectorWithDistanceIndex
        // effectively we want to compare the original alignment with new or current dynamic one and see 
        // where the changes have occured.
        CurveVectorWithDistanceIndex::FindCommonSubPaths (*HorizontalIndexVector(), *newPath, pathAIntervals, pathBIntervals, true, true);
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
    if (nullptr != GetVerticalCurveVector())
        {
        CurveVectorPtr vt = GetPartialVerticalAlignment (startStation, endStation);
        AlignmentPairEditorPtr returnAlignment = AlignmentPairEditor::Create (*hz, vt.get ());
        if (returnAlignment.IsValid ())
            {
            bvector<AlignmentPVI> pvis = returnAlignment->_GetPVIs ();
            CurveVectorPtr newVT = returnAlignment->_BuildVectorFromPVIS (pvis, returnAlignment->LengthXY ());
            if (newVT.IsValid ())
                {
                returnAlignment->UpdateVerticalCurveVector(newVT.get());
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

#endif
