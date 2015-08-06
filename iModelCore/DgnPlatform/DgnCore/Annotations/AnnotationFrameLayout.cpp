//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrameLayout.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGN

static const double SQRT_TWO = sqrt(2.0);
static const double TWO_PI = (2.0 * Angle::Pi());

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static DPoint2d computeCenterOfRange(DRange2dCR range)
    {
    DPoint2d pt = range.low;
    pt.x += (range.XLength() / 2.0);
    pt.y += (range.YLength() / 2.0);

    return pt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrameLayout::AnnotationFrameLayout(AnnotationFrameCR frame, AnnotationTextBlockLayoutCR docLayout) :
    T_Super()
    {
    m_isValid = false;
    m_frame = &frame;
    m_docLayout = &docLayout;
    m_effectiveFontHeight = 0.0;
    m_contentRange.Init();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameLayout::CopyFrom(AnnotationFrameLayoutCR rhs)
    {
    m_isValid = rhs.m_isValid;
    m_frame = rhs.m_frame;
    m_docLayout = rhs.m_docLayout;
    m_effectiveFontHeight = rhs.m_effectiveFontHeight;
    m_contentRange = rhs.m_contentRange;

    if (rhs.m_frameGeometry.IsNull())
        m_frameGeometry = NULL;
    else
        m_frameGeometry = rhs.m_frameGeometry->Clone();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static double computeEffectiveFontHeight(AnnotationTextBlockLayoutCR docLayout)
    {
    // Try to use the first run's font height. If empty, arbitrarily use the document's style.
    if (docLayout.GetDocument().GetParagraphs().empty() || docLayout.GetDocument().GetParagraphs()[0]->GetRuns().empty())
        return docLayout.GetDocument().CreateEffectiveStyle()->GetHeight();
    
    return docLayout.GetDocument().GetParagraphs()[0]->GetRuns()[0]->CreateEffectiveStyle()->GetHeight();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static DRange2d computeContentRange(AnnotationFrameCR frame, AnnotationTextBlockLayoutCR docLayout, double effectiveFontHeight)
    {
    // Minimum range is a font height square. Primary use case is for placing empty and filling in later, so we need at least something.
    DRange2d range = docLayout.GetLayoutRange();
    if (range.XLength() < effectiveFontHeight)
        range.high.x = range.low.x + effectiveFontHeight;
    if (range.YLength() < effectiveFontHeight)
        range.high.y = range.low.y + effectiveFontHeight;

    auto frameStyle = frame.CreateEffectiveStyle();
    double hPad = 0.0;
    double vPad = 0.0;
    
    if (frameStyle->GetHorizontalPadding() > 0.0)
        hPad = (frameStyle->GetHorizontalPadding() * effectiveFontHeight);
    if (frameStyle->GetVerticalPadding() > 0.0)
        vPad = (frameStyle->GetVerticalPadding() * effectiveFontHeight);
    
    range.low.x -= hPad;
    range.low.y -= vPad;
    range.high.x += hPad;
    range.high.y += vPad;

    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static CurveVectorPtr createFrameGeometry(AnnotationFrameType frameType, DRange2dCR contentRange)
    {
    switch (frameType)
        {
        case AnnotationFrameType::InvisibleBox:
        case AnnotationFrameType::Box:
            {
            DPoint3d boxPts[] =
                {
                DPoint3d::From(contentRange.low),
                DPoint3d::From(contentRange.low.x, contentRange.high.y),
                DPoint3d::From(contentRange.high.x, contentRange.high.y),
                DPoint3d::From(contentRange.high.x, contentRange.low.y),
                DPoint3d::From(contentRange.low)
                };
            ICurvePrimitivePtr boxGeometry = ICurvePrimitive::CreateLineString(boxPts, _countof(boxPts));
            return CurveVector::Create(boxGeometry, CurveVector::BOUNDARY_TYPE_Outer);
            }
        
        case AnnotationFrameType::Circle:
            {
            double dominantAxisLength;
            if (contentRange.XLength() > contentRange.YLength())
                dominantAxisLength = contentRange.XLength();
            else
                dominantAxisLength = contentRange.YLength();

            DPoint3d center = DPoint3d::From(computeCenterOfRange(contentRange));
            double axisLength = (dominantAxisLength / 2.0);
            DVec3d axis0 = DVec3d::From(axisLength, 0.0, 0.0);
            DVec3d axis1 = DVec3d::From(0.0, axisLength, 0.0);
            
            DEllipse3d ellipse = DEllipse3d::FromVectors(center, axis0, axis1, 0.0, TWO_PI);
            ICurvePrimitivePtr ellipseGeometry = ICurvePrimitive::CreateArc(ellipse);
            return CurveVector::Create(ellipseGeometry, CurveVector::BOUNDARY_TYPE_Outer);
            }

        case AnnotationFrameType::Ellipse:
            {
            DPoint3d center = DPoint3d::From(computeCenterOfRange(contentRange));
            double axis0Length = ((contentRange.XLength() / 2.0) * SQRT_TWO);
            DVec3d axis0 = DVec3d::From(axis0Length, 0.0, 0.0);
            double axis1Length = ((contentRange.YLength() / 2.0) * SQRT_TWO);
            DVec3d axis1 = DVec3d::From(0.0, axis1Length, 0.0);
            
            DEllipse3d ellipse = DEllipse3d::FromVectors(center, axis0, axis1, 0.0, TWO_PI);
            ICurvePrimitivePtr ellipseGeometry = ICurvePrimitive::CreateArc(ellipse);
            return CurveVector::Create(ellipseGeometry, CurveVector::BOUNDARY_TYPE_Outer);
            }

        default:
            {
            BeAssert(false); // Unknown/unexpected AnnotationFrameType.
            return NULL;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void convertToCloud(CurveVectorPtr& frameGeometry, double bulgeFactor, double diameter)
    {
    PRECONDITION(frameGeometry.IsValid(), );

    // Basic sanity.
    if (diameter <= 0.0)
        return;

    double pathLength;
    DPoint3d pathCentroid;
    if (!frameGeometry->WireCentroid(pathLength, pathCentroid))
        return;

    if (pathLength <= 0.0)
        return;

    size_t numArcs = (size_t)(pathLength / diameter);
    if (numArcs < 2)
        return;

    size_t numArcPtDistances = (numArcs + 1);
    double arcLength = (pathLength / numArcs);

    bvector<double> arcPtDistances;
    arcPtDistances.reserve(numArcPtDistances);
    for (size_t iDist = 0; iDist < numArcPtDistances; ++iDist)
        arcPtDistances.push_back(arcLength * iDist);

    bvector<CurveLocationDetail> arcPts;
    if (!frameGeometry->AddSpacedPoints(arcPtDistances, arcPts))
        return;

    if (arcPts.size() < 2)
        return;

    CurveVectorPtr cloudGeometry = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
    for (size_t iArc = 0; iArc < numArcs; ++iArc)
        {
        DPoint3d pt0 = arcPts[iArc].point;
        DPoint3d pt1 = arcPts[iArc + 1].point;

        DEllipse3d arc;
        if (UNEXPECTED_CONDITION(!arc.InitArcFromPointPointArcLength(pt0, pt1, pt0.Distance(pt1) * bulgeFactor, DVec3d::FromStartEnd(pathCentroid, pt0))))
            continue;

        cloudGeometry->Add(ICurvePrimitive::CreateArc(arc));
        }

    frameGeometry = cloudGeometry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameLayout::Update()
    {
    if (m_isValid)
        return;

    m_isValid = true;

    auto frameStyle = m_frame->CreateEffectiveStyle();
    
    m_effectiveFontHeight = computeEffectiveFontHeight(*m_docLayout);
    m_contentRange = computeContentRange(*m_frame, *m_docLayout, m_effectiveFontHeight);
    m_frameGeometry = createFrameGeometry(frameStyle->GetType(), m_contentRange);

    if (frameStyle->IsStrokeCloud())
        convertToCloud(m_frameGeometry, frameStyle->GetCloudBulgeFactor(), (m_effectiveFontHeight * frameStyle->GetCloudDiameterFactor()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
size_t AnnotationFrameLayout::GetAttachmentIdCount() const
    {
    AnnotationFrameStylePtr frameStyle = m_frame->CreateEffectiveStyle();
    switch (frameStyle->GetType())
        {
        case AnnotationFrameType::InvisibleBox:
        case AnnotationFrameType::Box:
            return 8;
        
        case AnnotationFrameType::Circle:
        case AnnotationFrameType::Ellipse:
            return 4;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationFrameType.
            return 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
uint32_t AnnotationFrameLayout::GetAttachmentId(size_t iID) const
    {
    AnnotationFrameStylePtr frameStyle = m_frame->CreateEffectiveStyle();
    switch (frameStyle->GetType())
        {
        case AnnotationFrameType::InvisibleBox:
        case AnnotationFrameType::Box:
        case AnnotationFrameType::Circle:
        case AnnotationFrameType::Ellipse:
            return (uint32_t)iID;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationFrameType.
            return 0;
        }
    }

//---------------------------------------------------------------------------------------
/*
    Box             Circle/Ellipse
                    
     3   2   1         1
     +-------+        /-\
    4|       |0     2<   >0
     +-------+        \-/
     5   6   7         3
*/
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrameLayout::ComputePhysicalPointForAttachmentId(DPoint3dR physicalPoint, DVec3dR tangent, uint32_t attachmentID) const
    {
    const_cast<AnnotationFrameLayoutP>(this)->Update(); 
    
    physicalPoint.Zero();
    tangent.Zero();

    AnnotationFrameStylePtr frameStyle = m_frame->CreateEffectiveStyle();
    switch (frameStyle->GetType())
        {
        case AnnotationFrameType::InvisibleBox:
        case AnnotationFrameType::Box:
            {
            if (attachmentID > 7)
                attachmentID = 7;
            
            DPoint2d midPoint = DPoint2d::From(m_contentRange.low.x, m_contentRange.low.y);
            midPoint.x += (m_contentRange.XLength() / 2.0);
            midPoint.y += (m_contentRange.YLength() / 2.0);

            switch (attachmentID)
                {
                case 0: physicalPoint.Init( m_contentRange.high.x,  midPoint.y);            tangent.Init(   1.0,    0.0);   break;
                case 1: physicalPoint.Init( m_contentRange.high.x,  m_contentRange.high.y); tangent.Init(   1.0,    0.0);   break;
                case 2: physicalPoint.Init( midPoint.x,             m_contentRange.high.y); tangent.Init(   0.0,    1.0);   break;
                case 3: physicalPoint.Init( m_contentRange.low.x,   m_contentRange.high.y); tangent.Init(   -1.0,   0.0);   break;
                case 4: physicalPoint.Init( m_contentRange.low.x,   midPoint.y);            tangent.Init(   -1.0,   0.0);   break;
                case 5: physicalPoint.Init( m_contentRange.low.x,   m_contentRange.low.y);  tangent.Init(   -1.0,   0.0);   break;
                case 6: physicalPoint.Init( midPoint.x,             m_contentRange.low.y);  tangent.Init(   0.0,    -1.0);  break;
                case 7: physicalPoint.Init( m_contentRange.high.x,  m_contentRange.low.y);  tangent.Init(   1.0,    0.0);   break;

                default:
                    BeAssert(false); // Unknown/unexpected attachment ID
                    break;
                }
            
            break;
            }
        
        case AnnotationFrameType::Circle:
            {
            if (attachmentID > 3)
                attachmentID = 3;

            DPoint2d midPoint = DPoint2d::From(m_contentRange.low.x, m_contentRange.low.y);
            midPoint.x += (m_contentRange.XLength() / 2.0);
            midPoint.y += (m_contentRange.YLength() / 2.0);

            double dominantRadius;
            if (m_contentRange.XLength() > m_contentRange.YLength())
                dominantRadius = (m_contentRange.XLength() / 2.0);
            else
                dominantRadius = (m_contentRange.YLength() / 2.0);
            
            switch (attachmentID)
                {
                case 0: physicalPoint.Init( midPoint.x + dominantRadius,    midPoint.y);                    tangent.Init(   1.0,    0.0);   break;
                case 1: physicalPoint.Init( midPoint.x,                     midPoint.y + dominantRadius);   tangent.Init(   0.0,    1.0);   break;
                case 2: physicalPoint.Init( midPoint.x - dominantRadius,    midPoint.y);                    tangent.Init(   -1.0,   0.0);   break;
                case 3: physicalPoint.Init( midPoint.x,                     midPoint.y - dominantRadius);   tangent.Init(   0.0,    -1.0);  break;

                default:
                    BeAssert(false); // Unknown/unexpected attachment ID
                    break;
                }
            
            break;
            }

        case AnnotationFrameType::Ellipse:
            {
            if (attachmentID > 3)
                attachmentID = 3;
            
            DPoint2d midPoint = DPoint2d::From(m_contentRange.low.x, m_contentRange.low.y);
            midPoint.x += (m_contentRange.XLength() / 2.0);
            midPoint.y += (m_contentRange.YLength() / 2.0);

            switch (attachmentID)
                {
                case 0: physicalPoint.Init( m_contentRange.high.x,  midPoint.y);            tangent.Init(   1.0,    0.0);   break;
                case 1: physicalPoint.Init( midPoint.x,             m_contentRange.high.y); tangent.Init(   0.0,    1.0);   break;
                case 2: physicalPoint.Init( m_contentRange.low.x,   midPoint.y);            tangent.Init(   -1.0,   0.0);   break;
                case 3: physicalPoint.Init( midPoint.x,             m_contentRange.low.y);  tangent.Init(   0.0,    -1.0);  break;

                default:
                    BeAssert(false); // Unknown/unexpected attachment ID
                    break;
                }
            
            break;
            }

        default:
            {
            BeAssert(false); // Unknown/unexpected AnnotationFrameType.
            break;
            }
        }
    }
