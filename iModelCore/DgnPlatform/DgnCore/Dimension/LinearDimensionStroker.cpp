//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Dimension/LinearDimensionStroker.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Dimension/Dimension.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

static DVec2d s_xVec = DVec2d::From (1.0, 0.0);
static DVec2d s_yVec = DVec2d::From (0.0, 1.0);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */  LinearDimensionStroker::LinearDimensionStroker(LinearDimensionCR dim, GeometryBuilderR builder)
    :
    m_dim(dim),
    m_geomBuilder(builder)
    {
    m_dimStyle  = DimensionStyle::Get (m_dim.ToElement().GetDgnDb(), m_dim.GetDimensionStyleId());
    m_textStyleId = m_dimStyle->GetTextStyleId();

    AnnotationTextStyleCPtr textStyle = AnnotationTextStyle::Get (m_dim.ToElement().GetDgnDb(), m_textStyleId);
    m_textHeight  = textStyle->GetHeight();

    m_textMargin.x = 0.5 * m_textHeight;
    m_textMargin.y = 0.5 * m_textHeight;

    m_runningStackOffset = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::AppendDimensionLine (DPoint2dCR start, DPoint2dCR end)
    {
    DSegment3d  points = DSegment3d::From (start, end);

    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(points);
    m_geomBuilder.Append(*curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::AppendWitness (DPoint2dCR start, DPoint2dCR end)
    {
    DSegment3d  points = DSegment3d::From (start, end);

    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(points);
    m_geomBuilder.Append(*curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::AppendTerminator (bvector<DPoint2d> const& pointsIn, bool filled)
    {
    bvector<DPoint3d>   points;
    for (DPoint2dCR point2d : pointsIn)
        points.push_back (DPoint3d::From (point2d));

    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLineString(points);
    m_geomBuilder.Append(*curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      LinearDimensionStroker::FormatDistanceString (double distance)
    {
    DistanceFormatterPtr formatter = DistanceFormatter::Create();

    formatter->SetUnitFormat (DgnUnitFormat::MU);

    return formatter->ToString (distance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
double   LinearDimensionStroker::CalculateMeasureDistance (DPoint2dCR start, DPoint2dCR end)
    {
    return end.x - start.x;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::GenerateText (DPoint2dCR textPoint, DVec2dCR textDir, Utf8CP string)
    {
    //DEllipse3d  ellipse = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (textPoint), m_textHeight / 5.0);
    //ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc(ellipse);
    //m_geomBuilder.Append(*curve);

    AnnotationTextBlockPtr  textBlock = AnnotationTextBlock::Create (GetDimension().ToElement().GetDgnDb(), m_textStyleId);
    AnnotationTextRunPtr    newRun = AnnotationTextRun::Create (textBlock->GetDbR(), m_textStyleId, string);

    textBlock->AppendRun (*newRun);

    TextAnnotation textAnnotation (textBlock->GetDbR());
    textAnnotation.SetText (textBlock.get());

    textAnnotation.SetOrigin (DPoint3d::From (textPoint));
    textAnnotation.SetOrientation (YawPitchRollAngles::FromRadians (0.0, 0.0, s_xVec.AngleTo (textDir)));
    textAnnotation.SetAnchorPoint (TextAnnotation::AnchorPoint::CenterBottom);

    m_geomBuilder.Append (textAnnotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::GenerateTerminator (DPoint2dCR termPoint, DVec2dCR termDir)
    {
    static const double termHeight = m_textHeight * 0.5;
    static const double termWidth  = m_textHeight * 0.5;

    DVec2d              perpDir = DVec2d::FromRotate90CCW (termDir);

    bvector<DPoint2d>   points;
    points.resize(4);

    points[0] = termPoint;
    points[1].SumOf (points[0], termDir, termWidth);
    points[1].SumOf (points[1], perpDir, - termHeight / 2);
    points[2].SumOf (points[1], perpDir, termHeight);
    points[3] = termPoint;

    AppendTerminator (points, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::GenerateDimension (DPoint2dCR start, DPoint2dCR end)
    {
    AppendDimensionLine (start, end);

    DVec2d      dimLineDir      = s_xVec;
    DVec2d      dimLinePerpDir  = s_yVec;
    double      distance        = CalculateMeasureDistance (start, end);
    Utf8String  dimString       = FormatDistanceString (distance);
    DPoint2d    textPoint       = DPoint2d::FromInterpolate (start, 0.5, end);

    textPoint.SumOf (textPoint, dimLinePerpDir, m_textMargin.y);

    GenerateText (textPoint, dimLineDir, dimString.c_str());

    if (start.x > end.x)
        dimLineDir.Negate();

    GenerateTerminator (start, dimLineDir);

    dimLineDir.Negate();
    GenerateTerminator (end,   dimLineDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::GenerateSegment (uint32_t iSegment)
    {
    static const double witOffset = m_textHeight * 0.5;
    static const double witExtend = m_textHeight * 0.5;
    static const double dimHeight = m_textHeight * 5.0;
    static const bool   stacked   = true;

    double  segmentHeight = dimHeight + m_runningStackOffset;

    DPoint2d    point0 = m_dimPoints[stacked ? 0 : iSegment];;
    DPoint2d    point1 = m_dimPoints[iSegment + 1];;

    double      height0 = segmentHeight - point0.y;
    double      height1 = segmentHeight - point1.y;

    DPoint2d    dimLinePoints[2];

    dimLinePoints[0].SumOf (point0, s_yVec, height0);
    dimLinePoints[1].SumOf (point1, s_yVec, height1);

    GenerateDimension (dimLinePoints[0], dimLinePoints[1]);

    DPoint2d    witnessPoints[2];

    witnessPoints[0].SumOf (point0, s_yVec, witOffset);
    witnessPoints[1].SumOf (point0, s_yVec, height0 + witExtend);
    AppendWitness (witnessPoints[0], witnessPoints[1]);

    witnessPoints[0].SumOf (point1, s_yVec, witOffset);
    witnessPoints[1].SumOf (point1, s_yVec, height1 + witExtend);
    AppendWitness (witnessPoints[0], witnessPoints[1]);

    if (stacked)
        {
        double segmentStackOffset = m_textHeight + (3 * m_textMargin.y);  // NEEDSWORK: should be based on text range

        m_runningStackOffset += segmentStackOffset;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   LinearDimensionStroker::AppendDimensionGeometry ()
    {
    // Load m_dimPoints from the element
    for (uint32_t index = 0; index < m_dim.GetPointCount(); index++)
        m_dimPoints.push_back (m_dim.GetPointLocal(index));

    for (uint32_t iSegment = 0; iSegment < m_dimPoints.size() - 1; ++iSegment)
        GenerateSegment (iSegment);

#if defined (NOT_NOW)
    bvector<DPoint3d>  points;
    for (uint32_t index = 0; index < m_dimPoints.size(); index++)
        points.push_back (DPoint3d::From (m_dimPoints[index]));
    
    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLineString(points);
    m_geomBuilder.Append(*curve);
#endif
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
