/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Annotations/AnnotationLeaderDraw.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderDraw::AnnotationLeaderDraw(AnnotationLeaderLayoutCR leaderLayout) :
    T_Super()
    {
    m_leaderLayout = &leaderLayout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeaderDraw::CopyFrom(AnnotationLeaderDrawCR rhs)
    {
    m_leaderLayout = rhs.m_leaderLayout;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setStrokeSymbology(ViewContextR context, AnnotationColorType colorType, ColorDef colorValue, uint32_t weight)
    {
    ElemDisplayParamsR displayParams = context.GetCurrentDisplayParams();

    displayParams.ResetAppearance();
    
    displayParams.SetWeight(weight);
    displayParams.SetFillDisplay(FillDisplay::Never);

    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: displayParams.SetLineColor(colorValue); break;
        case AnnotationColorType::ViewBackground: BeAssert(false) /* unsupported */; break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
static void setFillSymbology(ViewContextR context, AnnotationColorType colorType, ColorDef colorValue, double transparency)
    {
    ElemDisplayParamsR displayParams = context.GetCurrentDisplayParams();

    displayParams.ResetAppearance();

    displayParams.SetTransparency(transparency);
    displayParams.SetFillDisplay(FillDisplay::Always);

    switch (colorType)
        {
        case AnnotationColorType::ByCategory: /* don't override */break;
        case AnnotationColorType::RGBA: displayParams.SetFillColor(colorValue); break;
        case AnnotationColorType::ViewBackground: displayParams.SetFillColorToViewBackground(); break;
        default: BeAssert(false) /* unknown */; break;
        }

    context.CookDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2014
//---------------------------------------------------------------------------------------
static CurveVectorPtr createEffectiveLineGeometry(CurveVectorCR originalLineGeometry, CurveVectorCP terminatorGeometry)
    {
    // The goal of this is to clip open line geometry to closed terminators to avoid visual artifacts.
    
    //.............................................................................................
    // This simplified code requires originalLineGeometry to be a flat collection of open curves.

    auto isCurveUnacceptable = [](ICurvePrimitivePtr const& item)
        {
        if (!item.IsValid())
            return true;
        
        switch (item->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                break;
            
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_NotClassified:
            default:
                return true;
            }

            return false;
        };
    
    if (UNEXPECTED_CONDITION((CurveVector::BOUNDARY_TYPE_Open != originalLineGeometry.GetBoundaryType()) || std::any_of(originalLineGeometry.begin(), originalLineGeometry.end(), isCurveUnacceptable)))
        return originalLineGeometry.Clone();

    //.............................................................................................
    // If no terminator, nothing to process.
    if (NULL == terminatorGeometry)
        return originalLineGeometry.Clone();
    
    //.............................................................................................
    // Otherwise compute intersections.
    CurveVector lineIntersections(CurveVector::BOUNDARY_TYPE_None);
    CurveVector terminatorIntersections(CurveVector::BOUNDARY_TYPE_None);
    CurveCurve::IntersectionsXY(lineIntersections, terminatorIntersections, const_cast<CurveVectorR>(originalLineGeometry), const_cast<CurveVectorR>(*terminatorGeometry), NULL);

    // Is the whole line inside or outside?
    if (lineIntersections.empty())
        {
        DPoint3d lineGeometryStartPoint;
        if (UNEXPECTED_CONDITION(!originalLineGeometry.GetStartPoint(lineGeometryStartPoint)))
            return NULL;
        
        // If not outside, nothing to draw.
        if (CurveVector::INOUT_Out != terminatorGeometry->PointInOnOutXY(lineGeometryStartPoint))
            return NULL;
        
        // Outside, so draw the whole thing.
        return originalLineGeometry.Clone();
        }
    
    // Otherwise we have to clip.
    // CurveCurve::IntersectionsXY is not guaranteed to be in any particular order; we'd like it to go strictly from start to end.
    // bmap orders its keys; bset orders its values; this effectively ensures that we walk the curves in order, and the fractions in order.
    // Note that orignal curves that have no intersection still have to be processed, so first seed the map from the input, then augment with intersections.
    bmap<ICurvePrimitiveCP, bset<double>> sortedIntersections;
    for (auto const& curve : originalLineGeometry)
        {
        auto& fractions = sortedIntersections[curve.get()];
        fractions.insert(0.0);
        fractions.insert(1.0);
        }
    
    for (auto const& intersection : lineIntersections)
        sortedIntersections[intersection->GetPartialCurveDetailCP()->parentCurve.get()].insert(intersection->GetPartialCurveDetailCP()->fraction0);
    
    // Stated (and verified) restriction above is that originalLineGeometry is open.
    CurveVectorPtr effectiveLineGeometry = CurveVector::Create(originalLineGeometry.GetBoundaryType());
    
    // Iterate each fraction pair of each curve (including the synthetic ones inserted above), see if the segment is outside the terminator, and generate partials into effectiveLineGeometry.
    for (auto const& curveFractions : sortedIntersections)
        {
        auto const& curve = *curveFractions.first;
        auto const& fractions = curveFractions.second;
        auto previousFraction = fractions.begin();
        auto currentFraction = fractions.begin();
        for (++currentFraction; fractions.end() != currentFraction; ++previousFraction, ++currentFraction)
            {
            // If too close to represent a meaningful segment, skip.
            if (DoubleOps::AlmostEqual(*previousFraction, *currentFraction))
                continue;
            
            // Evaluate the mid-point to determine if it's within the terminator or not.
            auto testFraction = DoubleOps::Interpolate(*previousFraction, 0.5, *currentFraction);
            DPoint3d testPoint;
            if (UNEXPECTED_CONDITION(!curve.FractionToPoint(testFraction, testPoint)))
                continue;

            // If not outside, don't add the segment.
            if (CurveVector::INOUT_Out != terminatorGeometry->PointInOnOutXY(testPoint))
                continue;

            // Otherwise add the segment.
            effectiveLineGeometry->push_back(ICurvePrimitive::CreatePartialCurve(const_cast<ICurvePrimitiveP>(&curve), *previousFraction, *currentFraction));
            }
        }    
    
    // Most code (notably draw code) does not know how to handle partial curves; use CloneDereferenced to create "real" primitives again.
    return effectiveLineGeometry->CloneDereferenced(true, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderDraw::Draw(ViewContextR context) const
    {
    AnnotationLeaderStylePtr leaderStyle = m_leaderLayout->GetLeader().CreateEffectiveStyle();
    if ((AnnotationLeaderLineType::None == leaderStyle->GetLineType()) && (AnnotationLeaderTerminatorType::None == leaderStyle->GetTerminatorType()))
        return SUCCESS;
    
    GraphicR output = context.GetCurrentGraphicR();

    if (AnnotationLeaderLineType::None != leaderStyle->GetLineType())
        {
        CurveVectorPtr transformedTerminatorGeometry;
        if ((AnnotationLeaderTerminatorType::None != leaderStyle->GetTerminatorType()) && m_leaderLayout->GetTerminatorGeometry().IsAnyRegionType())
            {
            transformedTerminatorGeometry = m_leaderLayout->GetTerminatorGeometry().Clone();
            transformedTerminatorGeometry->TransformInPlace(m_leaderLayout->GetTerminatorTransform());
            }

        CurveVectorPtr effectiveLineGeometry = createEffectiveLineGeometry(m_leaderLayout->GetLineGeometry(), transformedTerminatorGeometry.get());
        if (effectiveLineGeometry.IsValid())
            {
            setStrokeSymbology(context, leaderStyle->GetLineColorType(), leaderStyle->GetLineColorValue(), leaderStyle->GetLineWeight());
            output.DrawCurveVector(*effectiveLineGeometry, false);
            }
        }
    
    if (AnnotationLeaderTerminatorType::None != leaderStyle->GetTerminatorType())
        {
        CurveVectorCR terminatorGeometry = m_leaderLayout->GetTerminatorGeometry();
        
        context.PushTransform(m_leaderLayout->GetTerminatorTransform());

        if (CurveVector::BOUNDARY_TYPE_Open == terminatorGeometry.GetBoundaryType())
            {
            setStrokeSymbology(context, leaderStyle->GetTerminatorColorType(), leaderStyle->GetTerminatorColorValue(), leaderStyle->GetTerminatorWeight());
            output.DrawCurveVector(terminatorGeometry, false);
            }
        else
            {
            setFillSymbology(context, leaderStyle->GetTerminatorColorType(), leaderStyle->GetTerminatorColorValue(), 0.0);
            output.DrawCurveVector(terminatorGeometry, true);
            }
        
        context.PopTransformClip(); // terminator transform
        }

    return SUCCESS;
    }
