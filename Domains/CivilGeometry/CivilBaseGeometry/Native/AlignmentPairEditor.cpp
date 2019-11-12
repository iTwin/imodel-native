/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CivilBaseGeometryInternal.h"
#include <CivilBaseGeometry/AlignmentPairEditor.h>

#define BASEGRADEDIFFERENTIAL 1.0
// limit intersection/ramp approach lengths to a factor of the total alignment length
#define APPROACH_STATION_LIMIT_FACTOR 0.4
#define TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED 1E-04

#define PVI_LENGTH_FIT_SAFETY_FACTOR 0.05 // 5% safety factor


//=======================================================================================
// AlignmentPI
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
uint32_t AlignmentMarkerBits::GetMarkerBitsFromPrimitive(ICurvePrimitiveCR primitive)
    {
    uint32_t markerBits = 0;

    CurvePrimitiveIdCP pId = primitive.GetId();
    if (nullptr != pId && CurvePrimitiveId::Type::ConceptStationAlignmentIndex == pId->GetType())
        {
        BeAssert(sizeof(uint32_t) == pId->GetIdSize());
        markerBits = *reinterpret_cast<uint32_t const*>(pId->PeekId());
        }

    return markerBits;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
void AlignmentMarkerBits::SetMarkerBitsToPrimitive(ICurvePrimitiveR primitive, uint32_t markerBits)
    {
    auto id = CurvePrimitiveId::Create(CurvePrimitiveId::Type::ConceptStationAlignmentIndex, (void*)&markerBits, sizeof(markerBits));
    if (id.IsValid())
        primitive.SetId(id.get());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
void AlignmentMarkerBits::SetMarkerBit(ICurvePrimitiveR primitive, AlignmentMarkerBits::Bit bit, bool value)
    {
    uint32_t markerBits = GetMarkerBitsFromPrimitive(primitive);

    if (value)
        markerBits |= static_cast<uint32_t>(bit);
    else
        markerBits &= ~static_cast<uint32_t>(bit);

    SetMarkerBitsToPrimitive(primitive, markerBits);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
bool AlignmentMarkerBits::GetMarkerBit(ICurvePrimitiveCR primitive, AlignmentMarkerBits::Bit bit)
    {
    const uint32_t markerBits = GetMarkerBitsFromPrimitive(primitive);
    return 0 != (static_cast<uint32_t>(bit) & markerBits);
    }

//=======================================================================================
// AlignmentPI
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
AlignmentPI::AlignmentPI()
    {
    m_noCurveInfo = NoCurveInfo();
    m_noCurveInfo.piPoint.InitDisconnect();
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
    aInfo.arc.orientation = Orientation::ORIENTATION_Unknown;
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
    scsInfo.arc.orientation = Orientation::ORIENTATION_Unknown;
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
DPoint3dCR AlignmentPI::GetPILocation() const
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
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGE("AlignmentPI::GetPILocation - unexpected PI type");
            return m_noCurveInfo.piPoint;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPI::SetPILocation(DPoint3dCR piPoint)
    {
    const DPoint3d piPointXY = DPoint3d::From(piPoint.x, piPoint.y, 0.0);

    switch (m_type)
        {
        case TYPE_NoCurve:
            {
            m_noCurveInfo.piPoint = piPointXY;
            return true;
            }
        case TYPE_Arc:
            {
            m_arcInfo.arc.piPoint = piPointXY;
            return true;
            }
        case TYPE_SCS:
            {
            m_scsInfo.overallPI = piPointXY;
            return true;
            }
        case TYPE_SS:
            {
            m_ssInfo.overallPI = piPointXY;
            return true;
            }
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGE("AlignmentPI::SetPILocation - unexpected PI type");
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
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGE("AlignmentPI::GetPseudoTangentLength - unexpected PI type");
            return 0.0;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
bool AlignmentPI::TryGetArcData(AlignmentPI::Arc& arc) const
    {
    switch (m_type)
        {
        case TYPE_Arc:
            {
            arc = m_arcInfo.arc;
            return true;
            }
        case TYPE_SCS:
            {
            arc = m_scsInfo.arc;
            return true;
            }
        default:
            return false;
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
bool AlignmentPI::TryGetSpiralData(AlignmentPI::Spiral& spiral, bool isFirstSpiral) const
    {
    switch (m_type)
        {
        case TYPE_SS:
            {
            spiral = isFirstSpiral ? m_ssInfo.spiral1 : m_ssInfo.spiral2;
            return true;
            }
        case TYPE_SCS:
            {
            spiral = isFirstSpiral ? m_scsInfo.spiral1 : m_scsInfo.spiral2;
            return true;
            }
        default:
            return false;
        }
    }



//=======================================================================================
// AlignmentPVI
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
AlignmentPVI::AlignmentPVI()
    {
    m_gradeBreakInfo = GradeBreakInfo();
    m_gradeBreakInfo.pvi.InitDisconnect();
    m_type = TYPE_Uninitialized;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPVI::InitGradeBreak(DPoint3dCR pviPoint)
    {
    GradeBreakInfo gbInfo;
    gbInfo.pvi = DPoint3d::From(pviPoint.x, 0.0, pviPoint.z);

    m_gradeBreakInfo = gbInfo;
    m_type = TYPE_GradeBreak;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPVI::InitArc(DPoint3dCR pviPoint, double radius)
    {
    ArcInfo aInfo;
    aInfo.pvi = DPoint3d::From(pviPoint.x, 0.0, pviPoint.z);
    aInfo.radius = radius;
    aInfo.orientation = Orientation::ORIENTATION_Unknown;

    // Set same point for pvc/pvt/center so we can check whether that PVI has been solved or not
    aInfo.pvc = aInfo.pvi;
    aInfo.pvt = aInfo.pvi;

    m_arcInfo = aInfo;
    m_type = TYPE_Arc;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPVI::InitParabola(DPoint3dCR pviPoint, bool isLengthByK, double kValueOrLength)
    {
    ParabolaInfo parInfo;
    parInfo.pvi = DPoint3d::From(pviPoint.x, 0.0, pviPoint.z);
    parInfo.isLengthByK = isLengthByK;
    parInfo.length = isLengthByK ? 0.0 : kValueOrLength;
    parInfo.kValue = isLengthByK ? kValueOrLength : 0.0;

    // Set same point for pvc/pvt so we can check whether that PVI has been solved or not
    parInfo.pvc = parInfo.pvi;
    parInfo.pvt = parInfo.pvi;

    m_parabolaInfo = parInfo;
    m_type = TYPE_Parabola;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPVI::InitInvalid(AlignmentPVI::Type pviType)
    {
    switch (pviType)
        {
        case TYPE_Arc:
            m_arcInfo = ArcInfo();
            break;
        case TYPE_Parabola:
            m_parabolaInfo = ParabolaInfo();
            break;
        case TYPE_GradeBreak:
        default:
            m_gradeBreakInfo = GradeBreakInfo();
            pviType = TYPE_GradeBreak;
            break;
        }
    m_type = pviType;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
// k = L / | G1-G2 |
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPVI::ParabolaInfo::ComputeKValue() const
    {
    if (0.0 == length)
        return 0.0;

    const double G1 = 100 * Slope(pvc, pvi);
    const double G2 = 100 * Slope(pvi, pvt);

    const double diff = fabs(G1 - G2);
    if (DoubleOps::AlmostEqual(0.0, diff))
        return 0.0;

    return length / diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPVI::ParabolaInfo::LengthFromK(double kvalue) const
    {
    if (0.0 == length)
        return 0.0;

    const double G1 = 100 * Slope(pvc, pvi);
    const double G2 = 100 * Slope(pvi, pvt);
    return fabs(G1 - G2) * kvalue;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
StationRange AlignmentPVI::GetStationRange() const
    {
    return StationRange(GetPVCLocation().x, GetPVTLocation().x);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
StationRange AlignmentPVI::GetStationRangePVCPVI() const
    {
    return StationRange(GetPVCLocation().x, GetPVILocation().x);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
StationRange AlignmentPVI::GetStationRangePVIPVT() const
    {
    return StationRange(GetPVILocation().x, GetPVTLocation().x);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
DPoint3dCR AlignmentPVI::GetPVCLocation() const
    {
    switch (m_type)
        {
        case TYPE_Arc:
            return m_arcInfo.pvc;
        case TYPE_Parabola:
            return m_parabolaInfo.pvc;
        case TYPE_GradeBreak:
            return m_gradeBreakInfo.pvi;
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPVI::GetPVCLocation - unexpected PVI type");
            return m_gradeBreakInfo.pvi;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
DPoint3dCR AlignmentPVI::GetPVILocation() const
    {
    switch (m_type)
        {
        case TYPE_Arc:
            return m_arcInfo.pvi;
        case TYPE_Parabola:
            return m_parabolaInfo.pvi;
        case TYPE_GradeBreak:
            return m_gradeBreakInfo.pvi;
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPVI::GetPVILocation - unexpected PVI type");
            return m_gradeBreakInfo.pvi;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
DPoint3dCR AlignmentPVI::GetPVTLocation() const
    {
    switch (m_type)
        {
        case TYPE_Arc:
            return m_arcInfo.pvt;
        case TYPE_Parabola:
            return m_parabolaInfo.pvt;
        case TYPE_GradeBreak:
            return m_gradeBreakInfo.pvi;
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPVI::GetPVTLocation - unexpected PVI type");
            return m_gradeBreakInfo.pvi;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPVI::SetPVILocation(DPoint3dCR pviPoint)
    {
    const DPoint3d pviPointXZ = DPoint3d::From(pviPoint.x, 0.0, pviPoint.z);

    switch (m_type)
        {
        case TYPE_GradeBreak:
            {
            m_gradeBreakInfo.pvi = pviPointXZ;
            return true;
            }
        case TYPE_Arc:
            {
            // Also set PVC,PVT to indicate we need to solve this PVI
            m_arcInfo.pvi = pviPointXZ;
            m_arcInfo.pvc = pviPointXZ;
            m_arcInfo.pvt = pviPointXZ;
            return true;
            }
        case TYPE_Parabola:
            {
            // Also set PVC,PVT to indicate we need to solve this PVI
            m_parabolaInfo.pvi = pviPointXZ;
            m_parabolaInfo.pvc = pviPointXZ;
            m_parabolaInfo.pvt = pviPointXZ;
            return true;
            }
        case TYPE_Uninitialized:
        default:
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPVI::SetPVILocation - unexpected PVI type");
            return false;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double AlignmentPVI::Slope(DPoint3dCR p0, DPoint3dCR p1)
    {
    if (DoubleOps::AlmostEqual(p0.z, p1.z))
        return 0.0;

    if (DoubleOps::AlmostEqual(p0.x, p1.x))
        return CS_PVI_INFINITY;

    return (p0.z - p1.z) / (p0.x - p1.x);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPVI::IsCrest() const
    {
    if (m_type != TYPE_Parabola)
        {
        return false;
        }
    double slopeIn = Slope(m_parabolaInfo.pvc, m_parabolaInfo.pvi);
    double slopeOut = Slope(m_parabolaInfo.pvi, m_parabolaInfo.pvt);
    return slopeIn > 0 && slopeOut < 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPVI::IsSag() const
    {
    if (m_type != TYPE_Parabola)
        {
        return false;
        }
    double slopeIn = Slope(m_parabolaInfo.pvc, m_parabolaInfo.pvi);
    double slopeOut = Slope(m_parabolaInfo.pvi, m_parabolaInfo.pvt);
    return slopeIn < 0 && slopeOut > 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPVI::HighDistance(double& highDistance) const
    {
    if (IsCrest())
        {
        double G1 = Slope(m_parabolaInfo.pvc, m_parabolaInfo.pvi) * 100;
        highDistance = m_parabolaInfo.kValue * fabs(G1);
        return true;
        }

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPVI::LowDistance(double& lowDistance) const
    {
    if (IsSag())
        {
        double G1 = Slope(m_parabolaInfo.pvc, m_parabolaInfo.pvi) * 100;
        lowDistance = m_parabolaInfo.kValue * fabs(G1);
        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        02/2018
//---------------------------------------------------------------------------------------
AlignmentPVI::Provenance::Provenance(CurveVectorCR curve) : m_curve(&curve)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        02/2018
//---------------------------------------------------------------------------------------
bool AlignmentPVI::Provenance::ContainsLineString() const
    {
    return std::any_of(m_curve->begin(), m_curve->end(),
        [](ICurvePrimitivePtr const& prim) { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == prim->GetCurvePrimitiveType();});
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        02/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<AlignmentPVI::Provenance> AlignmentPVI::Provenance::Create(CurveVectorCR curve)
    {
    return new Provenance(curve);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        02/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<AlignmentPVI::Provenance> AlignmentPVI::Provenance::Create(ICurvePrimitiveCR primitive)
    {
    ICurvePrimitivePtr primitivePtr(const_cast<ICurvePrimitiveP>(&primitive));
    CurveVectorPtr cv = CurveVector::Create(primitivePtr, CurveVector::BOUNDARY_TYPE_Open);
    return new Provenance(*cv);
    }


//=======================================================================================
// AlignmentPairEditor
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditor::AlignmentPairEditor(CurveVectorCP pHorizontalAlignment, CurveVectorCP verticalAlignment) :
T_Super(pHorizontalAlignment, verticalAlignment)
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
void AlignmentPairEditor::_UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment)
    {
    T_Super::_UpdateHorizontalCurveVector(pHorizontalAlignment);
    m_cachedPIs.clear();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPairEditor::_UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment)
    {
    T_Super::_UpdateVerticalCurveVector(pVerticalAlignment);
    m_cachedPVIs.clear();
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
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitiveArc.GetCurvePrimitiveType());

    DEllipse3dCP pEllipse = primitiveArc.GetArcCP();
    if (pEllipse->IsFullEllipse())
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::LoadArcData - arc is a full ellipse.");
        return false;
        }

    if (!pEllipse->IsCircularXY(arc.radius))
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::LoadArcData - not a circular arc");
        return false;
        }

    DPoint3d start, end;
    DVec3d startTangent, endTangent;
    primitiveArc.GetStartEnd(start, end, startTangent, endTangent);

    if (start.AlmostEqualXY(end))
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::LoadArcData - arc has same start and end point");
        return false;
        }

    start.z = 0.0;
    end.z = 0.0;
    startTangent.z = 0.0;
    endTangent.z = 0.0;

    arc.startPoint = start;
    arc.endPoint = end;
    arc.piPoint = ComputePIFromPointsAndVectors(start, startTangent, end, endTangent);
    arc.centerPoint = pEllipse->center;
    arc.centerPoint.z = 0.0;
    arc.orientation = pEllipse->IsCCWSweepXY() ? Orientation::ORIENTATION_CCW : Orientation::ORIENTATION_CW;
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::LoadSpiralData(AlignmentPI::Spiral& spiral, ICurvePrimitiveCR primitiveSpiral) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral == primitiveSpiral.GetCurvePrimitiveType());

    DSpiral2dPlacementCP pPlacement = primitiveSpiral.GetSpiralPlacementCP();
    DSpiral2dBaseCP pSpiral = pPlacement->spiral;
    
    primitiveSpiral.GetStartEnd(spiral.startPoint, spiral.endPoint, spiral.startVector, spiral.endVector);
    spiral.startPoint.z = 0.0;
    spiral.endPoint.z = 0.0;
    spiral.startVector.z = 0.0;
    spiral.endVector.z = 0.0;

    spiral.piPoint = ComputePIFromPointsAndVectors(spiral.startPoint, spiral.startVector, spiral.endPoint, spiral.endVector);

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
    primitiveSpiral.FractionToFrenetFrame(0, frame0, curvature0, torsion);
    primitiveSpiral.FractionToFrenetFrame(1, frame1, curvature1, torsion);
    DVec3d zVector1;
    DVec3d zVector0;
    frame0.GetMatrixColumn(zVector0, 2);
    frame1.GetMatrixColumn(zVector1, 2);
    if (0.0 == curvature0)
        spiral.startRadius = (0.0 < zVector1.z) ? -1.0 * CS_SPI_INFINITY : CS_SPI_INFINITY;
    else
        spiral.startRadius = -1.0 / curvature0;

    if (0.0 == pSpiral->mCurvature1)
        spiral.endRadius = (0.0 < zVector0.z) ? -1.0 * CS_SPI_INFINITY : CS_SPI_INFINITY;
    else
        spiral.endRadius = -1.0 / pSpiral->mCurvature1;

    spiral.length = pSpiral->mLength;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetLinePI(AlignmentPIR pi, size_t index) const
    {
    CurveVectorCP pHorizontal = GetHorizontalCurveVector();
    if (nullptr == pHorizontal)
        return false;

    if (index + 1 < pHorizontal->size() && pHorizontal->at(index + 1)->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        DPoint3d dummy, end;
        pHorizontal->at(index)->GetStartEnd(dummy, end);
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
    CurveVectorCP pHorizontal = GetHorizontalCurveVector();
    if (nullptr == pHorizontal)
        return false;

    pi.InitInvalid(AlignmentPI::TYPE_Arc);
    AlignmentPI::ArcInfoP pArc = pi.GetArcP();

    return LoadArcData(pArc->arc, *pHorizontal->at(primitiveIdx));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetSCSPI(AlignmentPIR pi, size_t primitiveIdx) const
    {
    CurveVectorCP pHorizontal = GetHorizontalCurveVector();
    if (nullptr == pHorizontal || primitiveIdx + 2 >= pHorizontal->size())
        return false;

    pi.InitInvalid(AlignmentPI::TYPE_SCS);
    AlignmentPI::SCSInfoP pSCS = pi.GetSCSP();

    if (!LoadSpiralData(pSCS->spiral1, *pHorizontal->at(primitiveIdx)))
        return false;

    if (!LoadArcData(pSCS->arc, *pHorizontal->at(primitiveIdx + 1)))
        return false;

    if (!LoadSpiralData(pSCS->spiral2, *pHorizontal->at(primitiveIdx + 2)))
        return false;

    pSCS->overallPI = ComputePIFromPointsAndVectors(pSCS->spiral1.startPoint, pSCS->spiral1.startVector, pSCS->spiral2.endPoint, pSCS->spiral2.endVector);
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetSSPI(AlignmentPIR pi, size_t primitiveIdx) const
    {
    CurveVectorCP pHorizontal = GetHorizontalCurveVector();
    if (nullptr == pHorizontal || primitiveIdx + 1 >= pHorizontal->size())
        return false;

    pi.InitInvalid(AlignmentPI::TYPE_SS);
    AlignmentPI::SSInfoP pSS = pi.GetSSP();

    if (!LoadSpiralData(pSS->spiral1, *pHorizontal->at(primitiveIdx)))
        return false;

    if (!LoadSpiralData(pSS->spiral2, *pHorizontal->at(primitiveIdx + 1)))
        return false;

    pSS->overallPI = ComputePIFromPointsAndVectors(pSS->spiral1.startPoint, pSS->spiral1.startVector, pSS->spiral2.endPoint, pSS->spiral2.endVector);
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
bvector<AlignmentPI> AlignmentPairEditor::GetPIs() const
    {
    CurveVectorCP pHorizontal = GetHorizontalCurveVector();
    if (nullptr == pHorizontal || pHorizontal->empty())
        return bvector<AlignmentPI>();

    if (!m_cachedPIs.empty())
        return m_cachedPIs;

    DPoint3d hzStart, hzEnd;
    pHorizontal->GetStartEnd(hzStart, hzEnd);
    hzStart.z = hzEnd.z = 0.0;

    bool isError = false;
    bvector<AlignmentPI> pis;

    for (size_t i = 0; i < pHorizontal->size(); ++i)
        {
        switch (pHorizontal->at(i)->GetCurvePrimitiveType())
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
                if (i + 1 >= pHorizontal->size())
                    {
                    isError = true;
                    break;
                    }

                ICurvePrimitive::CurvePrimitiveType nextType = pHorizontal->at(i + 1)->GetCurvePrimitiveType();
                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == nextType)
                    {
                    if (i + 2 >= pHorizontal->size() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral != pHorizontal->at(i + 2)->GetCurvePrimitiveType())
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
                CIVILBASEGEOMETRY_LOGW("AlignmentPairEditorTest::GetPIs - Unexpected primitive type");
                isError = true;
                break;
                }
            }

        if (isError)
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::GetPIs - error");
            return bvector<AlignmentPI>();
            }
        }


    // Add the end PI unless we end with a spiral or arc
    ICurvePrimitive::CurvePrimitiveType pType = pHorizontal->back()->GetCurvePrimitiveType();
    DPoint3d primStart, primEnd;
    pHorizontal->back()->GetStartEnd(primStart, primEnd);

    if (!primEnd.AlmostEqualXY(hzEnd) || (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != pType && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral != pType))
        {
        AlignmentPI endPI;
        endPI.InitNoCurve(hzEnd);
        pis.push_back(endPI);
        }


#ifndef NDEBUG
    // DEBUG-Only sanity check. We should be able to rebuild the curve off the PIs we've just retrieved
    CurveVectorPtr sanityCurve = _BuildCurveVectorFromPIs(pis);
    BeAssert(sanityCurve.IsValid());
#endif

    m_cachedPIs = pis;
    return pis;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr AlignmentPairEditor::BuildArc(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, Orientation orientation) const
    {
    if (0.0 >= radius)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildArc - null or negative radius");
        return nullptr;
        }

    const DVec3d v0 = DVec3d::FromStartEndNormalize(currPI, prevPI);
    const DVec3d v1 = DVec3d::FromStartEndNormalize(currPI, nextPI);
    const DVec3d poi = DVec3d::From(currPI);

    // Returns angle between -pi to +pi
    const double angle = std::abs(v0.AngleToXY(v1));
    if (0.0 == angle)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildArc - null angle");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildArc - tangentArcPt validation failed");
        return nullptr;
        }
    if (arcTangentPt.DistanceSquaredXY(poi) > nextPI.DistanceSquaredXY(poi) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildArc - arcTangentPt validation failed");
        return nullptr;
        }

    DEllipse3d ellipse;
    if (!ellipse.InitFromArcCenterStartEnd(center, tangentArcPt, arcTangentPt))
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildArc - failed to create arc");
        return nullptr;
        }

    if (Orientation::ORIENTATION_Unknown != orientation)
        {
        if (ellipse.IsCCWSweepXY() && Orientation::ORIENTATION_CCW != orientation)
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - null or negative radius");
        return nullptr;
        }
    if (0.0 >= spiralLength1 || 0.0 >= spiralLength2)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - null or negative spiral length");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - failed to create SCS curve");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - validation of angle progression failed");
        return nullptr;
        
        // sdevtodo - _SolveSSPI should return false in many cases...
        //pis.at (index).curveType = AlignmentPI::HorizontalPIType::SS;
        //return _SolveSSPI (index, pis);
        }

    if (lineToSpiralA.DistanceSquaredXY(currPI) > prevPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - lineToSpiralA validation failed");
        return nullptr;
        }
    if (lineToSpiralB.DistanceSquaredXY(currPI) > nextPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSCSCurve - lineToSpiralB validation failed");
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

    DPoint3d arcStart, arcEnd;
    if (!ac->GetStartEnd(arcStart, arcEnd) || arcStart.AlmostEqualXY(arcEnd))
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSSCurve - failed to create SS curve");
        return nullptr;
        }

    if (lineToSpiralA.DistanceSquaredXY(currPI) > prevPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSSCurve - lineToSpiralA validation failed");
        return nullptr;
        }
    if (lineToSpiralB.DistanceSquaredXY(currPI) > nextPI.DistanceSquaredXY(currPI) + TOLERANCE_VALIDATION_OF_DISTANCE_SQUARED)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildSSCurve - lineToSpiralB validation failed");
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
                CIVILBASEGEOMETRY_LOGE("AlignmentPairEditor::_BuildCurveVectorFromPIs - invalid PI type");
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

    // We don't know the orientation beforehand, just pass in Unknown.
    ICurvePrimitivePtr primitive = BuildArc(prevPI, currPI, nextPI, radius, Orientation::ORIENTATION_Unknown);
    if (!primitive.IsValid())
        return false;

    // Update PI with new data
    return LoadArcData(pInfo->arc, *primitive);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* "fit" the SCS - potential modifications are the overall PI or the radius
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::SolveSCSPI(bvector<AlignmentPI>& pis, size_t index) const
    {
    if (0 == index || index + 1 >= pis.size())
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::SolveSCSPI - Spiral PI is first or last PI");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::SolveSSPI - Spiral PI is first or last PI");
        return false;
        }

    AlignmentPI::SSInfoP pInfo = pis[index].GetSSP();
    BeAssert(nullptr != pInfo);

    if (0.0 >= pInfo->spiral1.length)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::SolveSSPI - Null or negative spiral length");
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
    BeAssert(index < pis.size());

    switch (pis[index].GetType())
        {
        case AlignmentPI::TYPE_NoCurve:
            return true;
        case AlignmentPI::TYPE_Arc:
            return SolveArcPI(pis, index);
        case AlignmentPI::TYPE_SCS:
            return SolveSCSPI(pis, index);
        case AlignmentPI::TYPE_SS:
            {
            // If SS succeeded, try to solve as SCS using the arc radius from the SS result.
            if (SolveSSPI(pis, index))
                {
                const AlignmentPI ssPI = pis[index];
                AlignmentPI::SSInfoCP pInfo = ssPI.GetSS();

                pis[index].InitSCS(pInfo->overallPI, fabs(pInfo->spiral1.endRadius), pInfo->spiral1.length, pInfo->spiral2.length);

                // If SCS failed, use the solved SS
                if (!SolveSCSPI(pis, index))
                    pis[index] = ssPI;

                return true;
                }

            return false;
            }
        default:
            {
            CIVILBASEGEOMETRY_LOGE("AlignmentPairEditor::_SolvePI - Invalid PI Type");
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_ValidatePIs(bvector<AlignmentPI> const& pis) const
    {
    for (size_t i = 1; i < pis.size(); ++i)
        {
        const double piDistance = pis[i - 1].GetPILocation().DistanceXY(pis[i].GetPILocation());
        const double sumPseudoTangentLengths = pis[i - 1].GetPseudoTangentLength() + pis[i].GetPseudoTangentLength();
        if (0.0 == piDistance || piDistance < sumPseudoTangentLengths)
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment)
    {
    return new AlignmentPairEditor(pHorizontalAlignment, pVerticalAlignment);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::Create(AlignmentPairCR pair)
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
    if (2 > pis.size() || !pi.IsInitialized())
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
    if (2 > pis.size() || !pi.IsInitialized())
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr AlignmentPairEditor::CreateVerticalOnly(CurveVectorCR verticalAlignment)
    {
    DPoint3d vStart, vEnd;
    if (!verticalAlignment.GetStartEnd(vStart, vEnd))
        {
        CIVILBASEGEOMETRY_LOGE("AlignmentPairEditor::CreateVerticalOnly - cannot create editor. Invalid vertical alignment.");
        return nullptr;
        }

    const double vtLength = vEnd.x - vStart.x;
    if (0.0 >= vtLength)
        {
        CIVILBASEGEOMETRY_LOGE("AlignmentPairEditor::CreateVerticalOnly - cannot create editor. Horizontal would have no length.");
        return nullptr;
        }

    // Create a horizontal alignment that is a simple line from (0, 0, 0) to (length, 0, 0)
    CurveVectorPtr hzCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    hzCurve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(vtLength, 0, 0))));

    return new AlignmentPairEditor(hzCurve.get(), &verticalAlignment);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
TransformCR AlignmentPairEditor::GetFlipYZTransform()
    {
    static Transform s_transform = Transform::FromOriginAndVectors(DPoint3d::FromZero(), DVec3d::UnitX(), DVec3d::UnitZ(), DVec3d::UnitY());
    return s_transform;
    }

#if 0
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
#endif
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
AlignmentPairEditor::VerticalEditResult AlignmentPairEditor::SolveValidateAndBuild(bvector<AlignmentPVI>& pvis, size_t index, bool isDelete) const
    {
    BeAssert(index < pvis.size());

    VerticalEditResult result;
    result.modifiedRange = StationRange(pvis[index].GetPVILocation().x);

    if (!_SolvePVI(pvis, index))
        return result;

    if (0 < index)
        {
        if (!_SolvePVI(pvis, index - 1))
            return result;

        result.modifiedRange.Extend(pvis[index - 1].GetStationRangePVIPVT());
        }

    // If we deleted PVIs, we just need to update the previous and the current PVI,
    // as the index now points to the PVI that was after the deleted PVIs
    if (!isDelete && index + 1 < pvis.size())
        {
        if (!_SolvePVI(pvis, index + 1))
            return result;

        result.modifiedRange.Extend(pvis[index + 1].GetStationRangePVCPVI());
        }

    if (!_ValidatePVIs(pvis))
        return result;

    result.vtCurve = _BuildCurveVectorFromPVIs(pvis);
    return result;
    }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::AreStationsEqual(double station0, double station1) const
    {
    return DoubleOps::AlmostEqual(station0, station1);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::AreStationsEqual(DPoint3dCR p0, DPoint3dCR p1) const
    {
    return AreStationsEqual(p0.x, p1.x);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::LoadVerticalArcData(AlignmentPVIR pvi, ICurvePrimitiveCR primitiveArc) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitiveArc.GetCurvePrimitiveType());

    // Cheat: put it in XY and use the logic from the horizontal
    ICurvePrimitivePtr xyPrimitive = primitiveArc.Clone();
    xyPrimitive->TransformInPlace(GetFlipYZTransform());

    // Load Arc data using vertical code
    AlignmentPI::Arc arc;
    if (!LoadArcData(arc, *xyPrimitive))
        return false;

    pvi.InitInvalid(AlignmentPVI::TYPE_Arc);
    AlignmentPVI::ArcInfoP pArc = pvi.GetArcP();

    pArc->orientation = arc.orientation;
    pArc->pvc = DPoint3d::From(arc.startPoint.x, 0.0, arc.startPoint.y);
    pArc->pvi = DPoint3d::From(arc.piPoint.x, 0.0, arc.piPoint.y);
    pArc->pvt = DPoint3d::From(arc.endPoint.x, 0.0, arc.endPoint.y);
    pArc->radius = arc.radius;
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::LoadVerticalParabolaData(AlignmentPVIR pvi, ICurvePrimitiveCR primitiveParabola) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == primitiveParabola.GetCurvePrimitiveType());

    pvi.InitInvalid(AlignmentPVI::TYPE_Parabola);
    AlignmentPVI::ParabolaInfoP pPara = pvi.GetParabolaP();

    MSBsplineCurveCP pSpline = primitiveParabola.GetBsplineCurveCP();
    if (pSpline == nullptr)
        return false;

    size_t count = pSpline->GetNumPoles();
    if (3 == count)
        {
        bvector<DPoint3d> poles;
        pSpline->GetPoles(poles);

        pPara->pvc = DPoint3d::From(poles[0].x, 0.0, poles[0].z);
        pPara->pvi = DPoint3d::From(poles[1].x, 0.0, poles[1].z);
        pPara->pvt = DPoint3d::From(poles[2].x, 0.0, poles[2].z);
        pPara->length = fabs(poles[2].x - poles[0].x);
        pPara->kValue = pPara->ComputeKValue();
        pPara->isLengthByK = AlignmentMarkerBits::GetMarkerBit(primitiveParabola, AlignmentMarkerBits::Bit::BIT_Vertical_IsParabolaLengthByK);
        }

    return (3 == count);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr AlignmentPairEditor::BuildVerticalArc(AlignmentPVI::ArcInfoCR info) const
    {
    DPoint3d xyPVC = DPoint3d::From(info.pvc.x, info.pvc.z, 0.0);
    DPoint3d xyPVI = DPoint3d::From(info.pvi.x, info.pvi.z, 0.0);
    DPoint3d xyPVT = DPoint3d::From(info.pvt.x, info.pvt.z, 0.0);

    if (0.0 >= info.radius)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildVerticalArc - null or negative radius");
        return nullptr;
        }

    const DVec3d v0 = DVec3d::FromStartEndNormalize(xyPVI, xyPVC);
    const DVec3d v1 = DVec3d::FromStartEndNormalize(xyPVI, xyPVT);
    const DVec3d poi = DVec3d::From(xyPVI);

    // Returns angle between -pi to +pi
    const double angle = std::abs(v0.AngleToXY(v1));
    if (0.0 == angle)
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildVerticalArc - null angle");
        return nullptr;
        }

    DVec3d bisector = DVec3d::FromSumOf(v0, v1);
    bisector.ScaleToLength(info.radius / sin(0.5 * angle));
    const DPoint3d center = DPoint3d::FromSumOf(poi, bisector);

    // Compute distance from poi tangentArc
    const double tangentArcLength = info.radius / tan(0.5 * angle);
    const DPoint3d tangentArcPt = DPoint3d::FromSumOf(poi, v0, tangentArcLength);
    const DPoint3d arcTangentPt = DPoint3d::FromSumOf(poi, v1, tangentArcLength);

    DEllipse3d ellipse;
    if (!ellipse.InitFromArcCenterStartEnd(center, tangentArcPt, arcTangentPt))
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::BuildVerticalArc - failed to create arc");
        return nullptr;
        }

    if (Orientation::ORIENTATION_Unknown != info.orientation)
        {
        if (ellipse.IsCCWSweepXY() && Orientation::ORIENTATION_CCW != info.orientation)
            ellipse.ComplementSweep();
        }

    // primitive is created in XY. We need to flip it
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateArc(ellipse);
    if (primitive.IsValid())
        primitive->TransformInPlace(GetFlipYZTransform());

    return primitive;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr AlignmentPairEditor::BuildVerticalParabola(AlignmentPVI::ParabolaInfoCR info) const
    {
    const bvector<DPoint3d> poles { info.pvc, info.pvi, info.pvt };

    MSBsplineCurvePtr parabolaCurve = MSBsplineCurve::CreateFromPolesAndOrder(poles, NULL, NULL, 3, false, false);

    ICurvePrimitivePtr primitive = parabolaCurve.IsValid() ? ICurvePrimitive::CreateBsplineCurve(parabolaCurve) : nullptr;
    if (primitive.IsValid())
        AlignmentMarkerBits::SetMarkerBit(*primitive, AlignmentMarkerBits::BIT_Vertical_IsParabolaLengthByK, info.isLengthByK);

    return primitive;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AlignmentPVI> AlignmentPairEditor::GetPVIs() const
    {
    if (nullptr == GetVerticalCurveVector())
        return bvector<AlignmentPVI>();

    if (!m_cachedPVIs.empty())
        return m_cachedPVIs;

    CurveVectorCR vt = *GetVerticalCurveVector();

    DPoint3d vtStart, vtEnd;
    vt.GetStartEnd(vtStart, vtEnd);
    vtStart.y = 0.0;
    vtEnd.y = 0.0;

    bool isError = false;
    bvector<AlignmentPVI> pvis;

    for (size_t i = 0; i < vt.size(); ++i)
        {
        switch (vt[i]->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                AlignmentPVI pvi;
                if (0 == i)
                    {
                    pvi.InitGradeBreak(vtStart);
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*vt[i]));
                    pvis.push_back(pvi);
                    }

                if (i + 1 < vt.size())
                    {
                    auto nextType = vt[i + 1]->GetCurvePrimitiveType();
                    if (nextType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line || nextType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
                        {
                        DPoint3d start, end;
                        vt[i]->GetStartEnd(start, end);
                        end.y = 0.0;

                        CurveVectorPtr cvProvenance = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
                        cvProvenance->push_back(vt[i]);
                        cvProvenance->push_back(vt[i + 1]);

                        pvi.InitGradeBreak(end);
                        pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*cvProvenance));
                        pvis.push_back(pvi);
                        }
                    }
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> const* pts = vt[i]->GetLineStringCP();
                BeAssert(pts != nullptr);

                //check if were the first primitive
                if (i == 0)
                    {
                    AlignmentPVI pvi;
                    pvi.InitGradeBreak(vtStart);
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*vt[i]));
                    pvis.push_back(pvi);
                    }

                //for each point in the middle of the linestring, create a pvi
                for (int j = 1; j < pts->size() - 1; j++)
                    {
                    AlignmentPVI pvi;
                    DPoint3d pviPt = pts->at(j);
                    pviPt.y = 0.0;
                    pvi.InitGradeBreak(pviPt);
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*vt[i]));
                    pvis.push_back(pvi);
                    }

                //for the last pt in the linestring, create a pvi if the next primitive is a line
                if (i + 1 < vt.size())
                    {
                    auto nextType = vt[i + 1]->GetCurvePrimitiveType();
                    if (nextType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line || nextType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
                        {
                        AlignmentPVI pvi;
                        DPoint3d pviPt = pts->at(pts->size() - 1);
                        pviPt.y = 0.0;

                        CurveVectorPtr cvProvenance = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
                        cvProvenance->push_back(vt[i]);
                        cvProvenance->push_back(vt[i + 1]);

                        pvi.InitGradeBreak(pviPt);
                        pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*cvProvenance));
                        pvis.push_back(pvi);
                        }
                    }
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                AlignmentPVI pvi;
                if (LoadVerticalArcData(pvi, *vt.at(i)))
                    {
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*vt[i]));
                    pvis.push_back(pvi);
                    }
                else
                    isError = true;

                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve:
                {
                ICurvePrimitivePtr parent = vt[i]->GetPartialCurveDetailCP()->parentCurve;
                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve != parent->GetCurvePrimitiveType())
                    {
                    CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::GetPVIs - unexpected primitive type for partial curve");
                    isError = true;
                    break;
                    }

                AlignmentPVI pvi;
                if (LoadVerticalParabolaData(pvi, *parent))
                    {
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*parent));
                    pvis.push_back(pvi);
                    }
                else
                    isError = true;

                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                AlignmentPVI pvi;
                if (LoadVerticalParabolaData(pvi, *vt[i]))
                    {
                    pvi.SetProvenance(*AlignmentPVI::Provenance::Create(*vt[i]));
                    pvis.push_back(pvi);
                    }
                else
                    isError = true;

                break;
                }
            default:
                {
                CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::GetPVIs - unexpected primitive type");
                isError = true;
                break;
                }
            }

        if (isError)
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::GetPVIs - error");
            return bvector<AlignmentPVI>();
            }
        }


    // Add the end PVI unless the last PVI exceeds the vt length (which means we have a partial curve at the end)
    if (!pvis.empty() && pvis.back().GetPVTLocation().x < vtEnd.x)
        {
        AlignmentPVI endPVI;
        endPVI.InitGradeBreak(vtEnd);
        endPVI.SetProvenance(*AlignmentPVI::Provenance::Create(*vt.back()));
        pvis.push_back(endPVI);
        }

#ifndef NDEBUG
    // DEBUG-Only sanity check. We should be able to rebuild the curve off the PVIs we've just retrieved
    CurveVectorPtr sanityPVICurve = _BuildCurveVectorFromPVIs(pvis);
    BeAssert(sanityPVICurve.IsValid());
#endif

    m_cachedPVIs = pvis;
    return pvis;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::GetPVI(AlignmentPVIR pvi, size_t index) const
    {
    auto pvis = GetPVIs();
    if (index < pvis.size())
        {
        pvi = pvis[index];
        return true;
        }

    return false;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::_BuildCurveVectorFromPVIs(bvector<AlignmentPVI> const& pvis) const
    {
    if (2 > pvis.size())
        return nullptr;

    BeAssert(_ValidatePVIs(pvis) && "Caller is expected to validate PVIs before calling this");

    CurveVectorPtr vtCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    
    for (size_t i = 0; i < pvis.size(); ++i)
        {
        //! Tie with last PVI if PVC/PVT don't overlap
        if (0 < i)
            {
            DPoint3dCR lastPVT = pvis[i - 1].GetPVTLocation();
            DPoint3dCR currPVC = pvis[i].GetPVCLocation();

            if (!AreStationsEqual(lastPVT, currPVC))
                vtCurve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(lastPVT, currPVC)));
            }

        switch (pvis[i].GetType())
            {
            case AlignmentPVI::TYPE_GradeBreak:
                {
                break;
                }
            case AlignmentPVI::TYPE_Arc:
                {
                ICurvePrimitivePtr primitive = BuildVerticalArc(*pvis[i].GetArc());
                if (!primitive.IsValid())
                    return nullptr;

                vtCurve->push_back(primitive);
                break;
                }
            case AlignmentPVI::TYPE_Parabola:
                {
                ICurvePrimitivePtr primitive = BuildVerticalParabola(*pvis[i].GetParabola());
                if (!primitive.IsValid())
                    return nullptr;

                //&&AG needswork partial curves
                vtCurve->push_back(primitive);
                break;
                }
            default:
                {
                CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::_BuildCurveVectorFromPVIs - invalid PVI type");
                return nullptr;
                }
            }
        }

    if (!vtCurve->empty())
        return vtCurve;
            
    return nullptr;
    }

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
    if (index >= pis.size() || !inOutPI.IsInitialized())
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::MovePCorPT - Not an Arc or SCS transition");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::UpdateRadius - PI is not an Arc or SCS");
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
        CIVILBASEGEOMETRY_LOGE("AlignmentPairEditor::UpdateSpiralLength - PI is not a SCS or SS");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::RemoveSpirals - PI is not a SCS");
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
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::AddSpirals - PI is not a Arc");
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
    if (AlignmentPI::TYPE_SS != piType && AlignmentPI::TYPE_SCS != piType)
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
#endif


//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePVI(size_t index, DPoint3dCR toPt, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (index >= pvis.size())
        return nullptr;

    AlignmentPVI pvi = pvis[index];
    // Only allow update elevation for start/end pvis
    if (0 == index || index + 1 == pvis.size())
        {
        DPoint3d pviLocation = pvi.GetPVILocation();
        pviLocation.z = toPt.z;
        pvi.SetPVILocation(pviLocation);
        }
    else
        pvi.SetPVILocation(toPt);

    return MovePVI(index, pvi, pRangeEdit);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MovePVI(size_t index, AlignmentPVIR inOutPVI, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (index >= pvis.size() || !inOutPVI.IsInitialized())
        return nullptr;

    // Only allow update elevation for start/end pvis
    if (0 == index || index + 1 == pvis.size())
        {
        DPoint3d pviLocation = pvis[index].GetPVILocation();
        pviLocation.z = inOutPVI.GetPVILocation().z;
        inOutPVI.SetPVILocation(pviLocation);
        }

    pvis[index] = inOutPVI;

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, false/*isDelete*/);
    if (result.vtCurve.IsValid())
        {
        inOutPVI = pvis[index];

        if (nullptr != pRangeEdit)
            *pRangeEdit = StationRangeEdit(result.modifiedRange);
        }

    return result.vtCurve;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* allow the move of a tangent segment up or down (pass the old center x,z and new center x, z)
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveVerticalTangent(DPoint3dCR fromPt, DPoint3dCR toPt, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();

    for (size_t i = 1; i < pvis.size(); ++i)
        {
        if (fromPt.x > pvis[i - 1].GetPVILocation().x && fromPt.x < pvis[i].GetPVILocation().x)
            {
            const double deltaZ = toPt.z - fromPt.z;
            
            DPoint3d prevLocation = pvis[i - 1].GetPVILocation();
            prevLocation.z += deltaZ;
            pvis[i - 1].SetPVILocation(prevLocation);

            DPoint3d currLocation = pvis[i].GetPVILocation();
            currLocation.z += deltaZ;
            pvis[i].SetPVILocation(currLocation);

            // We've edited pvis[i - 1] and pvis[i].
            // We must also solve pvis[i - 2] and pvis[i + 1] if they are defined

            // Solve pvis[i - 2] (if applicable)
            if (1 < i && !_SolvePVI(pvis, i - 2))
                return nullptr;

            // this will solve pvis[i - 1], pvis[i], pvis[i + 1] (if applicable)
            VerticalEditResult result = SolveValidateAndBuild(pvis, i, false/*isDelete*/);
            if (result.vtCurve.IsValid())
                {
                if (1 < i)
                    result.modifiedRange.Extend(pvis[i - 1].GetStationRangePVIPVT());

                if (nullptr != pRangeEdit)
                    *pRangeEdit = StationRangeEdit(result.modifiedRange);
                }

            return result.vtCurve;
            }
        }

    return nullptr; // not found
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::UpdateVerticalRadius(size_t index, double radius, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (index >= pvis.size())
        return nullptr;

    if (0 == index || index + 1 == pvis.size())
        return nullptr; // todo start and end?

    AlignmentPVIR pvi = pvis[index];
    if (AlignmentPVI::TYPE_Arc != pvi.GetType())
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::UpdateVerticalRadius - PVI is not an Arc");
        return nullptr;
        }

    pvi.GetArcP()->radius = radius;;

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, false/*isDelete*/);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        01/2018
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::ForceGradeAtStation(double distanceAlongFromStart, double slope, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (2 > pvis.size())
        return nullptr;

    size_t index = FindNexOrEqualPVIIndex(pvis, distanceAlongFromStart);
    if (index == pvis.size())
        return nullptr;

    if (DoubleOps::AlmostEqual(distanceAlongFromStart, pvis[index].GetPVILocation().x))
        index++;

    if (0 == index || index >= pvis.size())
        return nullptr;

    // PVI is strictly after the provided distance. It is the PVI we will be editing
    AlignmentPVIR pvi = pvis[index];
    DPoint3dCR current = pvi.GetPVILocation();
    DPoint3dCR previous = pvis[index - 1].GetPVILocation();
    const double distance = current.x - previous.x;
    const double newZ = previous.z + (slope * distance);

    const DPoint3d newLocation = DPoint3d::From(current.x, 0.0, newZ);
    return MovePVI(index, newLocation, pRangeEdit);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* Allow the move of a pvc or pvt based on station.  Will return invalid if the result
* produces any overlapping curve or a 0 or negative length curve
* the result edit range is only the vertical curve
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::MoveParabolaPVCorPVT(double fromDistanceAlong, double toDistanceAlong, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();

    size_t index;
    bool isPVC;
    if (!FindEqualPVCorPVT(index, isPVC, pvis, fromDistanceAlong))
        return nullptr;

    if (AlignmentPVI::TYPE_Parabola != pvis[index].GetType())
        return nullptr;

    if (isPVC)
        {
        if (toDistanceAlong >= pvis[index].GetPVILocation().x)
            return nullptr;
        }
    else
        {
        if (toDistanceAlong <= pvis[index].GetPVILocation().x)
            return nullptr;
        }

    const double newLength = 2.0 * fabs(toDistanceAlong - pvis[index].GetPVILocation().x);

    AlignmentPVI::ParabolaInfoP pParabola = pvis[index].GetParabolaP();
    pParabola->length = newLength;
    pParabola->kValue = pParabola->ComputeKValue();

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, false/*isDelete*/);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::MoveArcPVCorPVT(double fromDistanceAlong, double toDistanceAlong, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();

    size_t index;
    bool isPVC;
    if (!FindEqualPVCorPVT(index, isPVC, pvis, fromDistanceAlong))
        return nullptr;

    AlignmentPVIR pvi = pvis[index];
    if (AlignmentPVI::TYPE_Arc != pvi.GetType())
        return nullptr;

    if (isPVC)
        {
        if (toDistanceAlong >= pvi.GetPVILocation().x)
            return nullptr;
        }
    else
        {
        if (toDistanceAlong <= pvi.GetPVILocation().x)
            return nullptr;
        }

    // Compute the ratio of movement of the PVC/PVT and apply the same ratio to the radius value
    const double oldLength = pvis[index].GetStationRange().Length();
    const double newLength = 2.0 * fabs(toDistanceAlong - pvi.GetPVILocation().x);
    const double ratio = newLength / oldLength;
    const double newRadius = ratio * pvi.GetArc()->radius;

    pvi.GetArcP()->radius = newRadius;

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, false/*isDelete*/);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
size_t AlignmentPairEditor::FindNexOrEqualPVIIndex(bvector<AlignmentPVI> const& pvis, double station) const
    {
    for (size_t i = pvis.size(); i > 0; --i)
        {
        if (pvis[i - 1].GetPVILocation().x < station)
            return i;
        }

    return 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
size_t AlignmentPairEditor::FindEqualPVIIndex(bvector<AlignmentPVI> const& pvis, double station) const
    {
    for (size_t i = 0; i < pvis.size(); ++i)
        {
        if (AreStationsEqual(pvis[i].GetPVILocation().x, station))
            return i;
        }

    return pvis.size();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::FindEqualPVCorPVT(size_t& pviIndex, bool& isPVC, bvector<AlignmentPVI> const& pvis, double station) const
    {
    for (size_t i = 0; i < pvis.size(); ++i)
        {
        if (AreStationsEqual(pvis[i].GetPVCLocation().x, station))
            {
            isPVC = true;
            pviIndex = i;
            return true;
            }
        if (AreStationsEqual(pvis[i].GetPVTLocation().x, station))
            {
            isPVC = false;
            pviIndex = i;
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::InsertPVI(AlignmentPVICR pvi, StationRangeEditP pRangeEdit) const
    {
    // Find the index to insert this point
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (2 > pvis.size() || !pvi.IsInitialized())
        return nullptr;

    size_t index = FindNexOrEqualPVIIndex(pvis, pvi.GetPVILocation().x);

    if (0 == index || index == pvis.size())
        return nullptr; // todo start and end?

    // Can't insert over an existing PVI
    if (AreStationsEqual(pvis[index].GetPVILocation(), pvi.GetPVILocation()))
        return nullptr;

    pvis.insert(pvis.begin() + index, pvi);

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, false);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairEditor::DeletePVI(DPoint3dCR pviPoint, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();

    size_t index = FindEqualPVIIndex(pvis, pviPoint.x);
    if (index == pvis.size())
        return nullptr;

    return DeletePVI(index, pRangeEdit);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::DeletePVI(size_t index, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (index >= pvis.size())
        return nullptr;

    if (0 == index || index + 1 == pvis.size())
        return nullptr; // todo start and end?

    pvis.erase(pvis.begin() + index);

    VerticalEditResult result = SolveValidateAndBuild(pvis, index, true/*isDelete*/);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPairEditor::DeletePVIs(StationRangeCR range, StationRangeEditP pRangeEdit) const
    {
    bvector<AlignmentPVI> pvis = GetPVIs();
    if (2 >= pvis.size())
        return nullptr;

    if (!range.IsValid())
        return nullptr;

    size_t lastDeleted = pvis.size();

    for (size_t i = pvis.size(); i > 0; --i)
        {
        // Can't delete start or end PVI for now
        if (i == pvis.size() || i == 1)
            continue;

        if (range.ContainsInclusive(pvis[i - 1].GetPVILocation().x))
            {
            lastDeleted = (i - 1);
            pvis.erase(pvis.begin() + (i - 1));
            }
        }

    if (pvis.size() == lastDeleted)
        return nullptr;

    VerticalEditResult result = SolveValidateAndBuild(pvis, lastDeleted, true/*isDelete*/);
    if (nullptr != pRangeEdit && result.vtCurve.IsValid())
        *pRangeEdit = StationRangeEdit(result.modifiedRange);

    return result.vtCurve;
    }
#if 0
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
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::IsPVIOverlap(bvector<AlignmentPVI> const& pvis, size_t index) const
    {
    AlignmentPVICR pvi = pvis[index];

    if (0 < index && pvis[index - 1].GetPVILocation().x >= pvi.GetPVILocation().x)
        return true;

    if ((index + 1) < pvis.size() && pvis[index + 1].GetPVILocation().x <= pvi.GetPVILocation().x)
        return true;

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
double AlignmentPairEditor::ComputeMaximumLength(bvector<AlignmentPVI> const& pvis, size_t index) const
    {
    double maxLength = CS_PVI_INFINITY;

    if (0 < index)
        {
        const double prevLength = 2.0 * (pvis[index].GetPVILocation().x - pvis[index - 1].GetPVTLocation().x);
        if (prevLength < maxLength)
            maxLength = prevLength;
        }

    if (index + 1 < pvis.size())
        {
        const double nextLength = 2.0 * (pvis[index + 1].GetPVCLocation().x - pvis[index].GetPVILocation().x);
        if (nextLength < maxLength)
            maxLength = nextLength;
        }

    return maxLength;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::SolveArcPVI(bvector<AlignmentPVI>& pvis, size_t index) const
    {
    if (IsPVIOverlap(pvis, index))
        return false;

    if (0 == index || index + 1 == pvis.size())
        return false; // todo start and end?

    AlignmentPVI::ArcInfoP pArc = pvis[index].GetArcP();
    if (0.0 >= pArc->radius)
        return false;

    // We don't know the orientation beforehand, just pass in Unknown.
    pArc->orientation = Orientation::ORIENTATION_Unknown;
    pArc->pvc = pvis[index - 1].GetPVILocation();
    pArc->pvt = pvis[index + 1].GetPVILocation();

    ICurvePrimitivePtr primitive = BuildVerticalArc(*pArc);
    if (!primitive.IsValid())
        return false;

    // Update PVI with new data
    return LoadVerticalArcData(pvis[index], *primitive);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::SolveParabolaPVI(bvector<AlignmentPVI>& pvis, size_t index) const
    {
    if (IsPVIOverlap(pvis, index))
        return false;

    if (0 == index || index + 1 == pvis.size())
        return false; // todo start and end?

    AlignmentPVI::ParabolaInfoP pParabola = pvis[index].GetParabolaP();
    if (0.0 >= pParabola->length)
        return false;
        
    double length = pParabola->length;
    
    if (pParabola->isLengthByK && 0.0 < pParabola->kValue)
        {
        pParabola->pvc = pvis[index - 1].GetPVILocation();
        pParabola->pvt = pvis[index + 1].GetPVILocation();
        length = pParabola->LengthFromK(pParabola->kValue);
        }

    // The maximum length the parabola can have based on the positions of previous and next PVIs
    const double maxLength = ComputeMaximumLength(pvis, index);

    //&&AG TODO FORCE FIT
    if (maxLength < length)
        return false;

    const double frontSlope = AlignmentPVI::Slope(pvis[index - 1].GetPVILocation(), pvis[index].GetPVILocation());
    pParabola->pvc.x = pParabola->pvi.x - (0.5 * length);
    pParabola->pvc.z = pParabola->pvi.z - (0.5 * length * frontSlope);

    const double backSlope = AlignmentPVI::Slope(pvis[index].GetPVILocation(), pvis[index + 1].GetPVILocation());
    pParabola->pvt.x = pParabola->pvi.x + (0.5 * length);
    pParabola->pvt.z = pParabola->pvi.z + (0.5 * length * backSlope);

    ICurvePrimitivePtr primitive = BuildVerticalParabola(*pParabola);
    if (!primitive.IsValid())
        return false;

    // Update PVI with new data
    return LoadVerticalParabolaData(pvis[index], *primitive);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairEditor::_SolvePVI(bvector<AlignmentPVI>& pvis, size_t index) const
    {
    BeAssert(index < pvis.size());

    switch (pvis[index].GetType())
        {
        case AlignmentPVI::TYPE_GradeBreak:
            return !IsPVIOverlap(pvis, index);
        case AlignmentPVI::TYPE_Arc:
            return SolveArcPVI(pvis, index);
        case AlignmentPVI::TYPE_Parabola:
            return SolveParabolaPVI(pvis, index);
        default:
            {
            CIVILBASEGEOMETRY_LOGW("AlignmentPairEditor::_SolvePVI - Invalid PVI type");
            return false;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPairEditor::_ValidatePVIs(bvector<AlignmentPVI> const& pvis) const
    {
    for (size_t i = 1; i < pvis.size(); ++i)
        {
        DPoint3dCR prevPVT = pvis[i - 1].GetPVTLocation();
        DPoint3dCR currPVC = pvis[i].GetPVCLocation();

        // Meeting at the same location is OK
        if (prevPVT.x > currPVC.x)
            return false;
        }

    return true;
    }


#if 0
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
