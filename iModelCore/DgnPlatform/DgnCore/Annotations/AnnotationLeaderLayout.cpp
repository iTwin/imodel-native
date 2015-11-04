//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationLeaderLayout.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderLayout::AnnotationLeaderLayout(AnnotationLeaderCR leader, AnnotationFrameLayoutCR frameLayout) :
    T_Super()
    {
    m_isValid = false;
    m_leader = &leader;
    m_frameLayout = &frameLayout;
    m_frameTransform.InitIdentity();
    m_sourcePhysicalPoint.Zero();
    m_sourceTangent.Zero();
    m_targetPhysicalPoint.Zero();
    m_terminatorTransform.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeaderLayout::CopyFrom(AnnotationLeaderLayoutCR rhs)
    {
    m_isValid = rhs.m_isValid;
    m_leader = rhs.m_leader;
    m_frameLayout = rhs.m_frameLayout;
    m_frameTransform = rhs.m_frameTransform;
    m_sourcePhysicalPoint = rhs.m_sourcePhysicalPoint;
    m_sourceTangent = rhs.m_sourceTangent;
    m_targetPhysicalPoint = rhs.m_targetPhysicalPoint;
    m_lineGeometry = rhs.m_lineGeometry->Clone();
    m_terminatorGeometry = rhs.m_terminatorGeometry->Clone();
    m_terminatorTransform = rhs.m_terminatorTransform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static CurveVectorPtr createLineGeometry(AnnotationLeaderLineType lineType, DPoint3dCR start, DVec3dCR startTangent, DPoint3dCR end)
    {
    switch (lineType)
        {
        case AnnotationLeaderLineType::None:
            {
            return NULL;
            }

        case AnnotationLeaderLineType::Straight:
            {
            DPoint3d ptArray[] = { start, end };
            return CurveVector::CreateLinear(ptArray, _countof(ptArray));
            }

        case AnnotationLeaderLineType::Curved:
            {
            // Without a reliable end tangent, create a b-spline with a second pole that is along the start tangent at a distance of end-start/2 from start.
            DPoint3d poles[] =
                {
                start,
                DPoint3d::FromSumOf(start, startTangent, start.Distance(end) / 2.0),
                end
                };

            CurveVectorPtr lineGeometry = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
            MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(poles, _countof(poles), 3);
            lineGeometry->push_back(ICurvePrimitive::CreateBsplineCurve(curve));
            
            return lineGeometry;
            }

        default:
            {
            BeAssert(false); // Unknown/unexpected AnnotationLeaderLineType.
            return NULL;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static CurveVectorPtr createTerminatorGeometry(AnnotationLeaderTerminatorType terminatorType)
    {
    switch (terminatorType)
        {
        case AnnotationLeaderTerminatorType::None:
            {
            return NULL;
            }

        case AnnotationLeaderTerminatorType::OpenArrow:
            {
            DPoint3d arrowPts [] =
                {
                DPoint3d::From(-1.0, 0.5),
                DPoint3d::From(0.0, 0.0),
                DPoint3d::From(-1.0, -0.5)
                };
            ICurvePrimitivePtr arrowGeometry = ICurvePrimitive::CreateLineString(arrowPts, _countof(arrowPts));
            return CurveVector::Create(arrowGeometry, CurveVector::BOUNDARY_TYPE_Open);
            }

        case AnnotationLeaderTerminatorType::ClosedArrow:
            {
            DPoint3d arrowPts [] =
                {
                DPoint3d::From(-1.0, 0.5),
                DPoint3d::From(0.0, 0.0),
                DPoint3d::From(-1.0, -0.5),
                DPoint3d::From(-1.0, 0.5)
                };
            ICurvePrimitivePtr arrowGeometry = ICurvePrimitive::CreateLineString(arrowPts, _countof(arrowPts));
            return CurveVector::Create(arrowGeometry, CurveVector::BOUNDARY_TYPE_Outer);
            }

        default:
            {
            BeAssert(false); // Unknown/unexpected AnnotationLeaderLineType.
            return NULL;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static Transform computeTerminatorTransform(CurveVectorCR lineGeometry, DPoint3dCR translation, double scale)
    {
    DPoint3d startPt, endPt;
    DVec3d startTangent, endTangent;
    lineGeometry.GetStartEnd(startPt, endPt, startTangent, endTangent);

    DVec3d scaledEndTangent = DVec3d::FromScale(endTangent, scale);
    RotMatrix orientationAndScale = RotMatrix::From1Vector(scaledEndTangent, 0, false);

    Transform t = Transform::FromIdentity();
    t.SetTranslation(translation);
    t.SetMatrix(orientationAndScale);

    return t;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeaderLayout::Update()
    {
    if (m_isValid)
        return;

    m_isValid = true;

    auto leaderStyle = m_leader->CreateEffectiveStyle();

    switch (m_leader->GetSourceAttachmentType())
        {
        case AnnotationLeaderSourceAttachmentType::Id:
            m_frameLayout->ComputePhysicalPointForAttachmentId(m_sourcePhysicalPoint, m_sourceTangent, *m_leader->GetSourceAttachmentDataForId());
            m_frameTransform.Multiply(m_sourcePhysicalPoint);
            break;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderSourceAttachmentType
            return;
        }

    switch (m_leader->GetTargetAttachmentType())
        {
        case AnnotationLeaderTargetAttachmentType::PhysicalPoint:
            m_targetPhysicalPoint = *m_leader->GetTargetAttachmentDataForPhysicalPoint();
            break;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderTargetAttachmentType
            return;
        }
    
    m_lineGeometry = createLineGeometry(leaderStyle->GetLineType(), m_sourcePhysicalPoint, m_sourceTangent, m_targetPhysicalPoint);
    m_terminatorGeometry = createTerminatorGeometry(leaderStyle->GetTerminatorType());
    m_terminatorTransform = computeTerminatorTransform(*m_lineGeometry, m_targetPhysicalPoint, (leaderStyle->GetTerminatorScaleFactor() * m_frameLayout->GetEffectiveFontHeight()));
    }
