/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/DgnElementManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

#define SET_PROPERTY_IMPL(value_type) \
    void DgnElementManipulationStrategy::SetProperty(Utf8CP key, value_type const& value) \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return; } \
        _SetProperty(key, value); \
        _OnPropertySet(key); \
        }
#define TRY_GET_PROPERTY_IMPL(value_type) \
    BentleyStatus DgnElementManipulationStrategy::TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        if (nullptr == key) { BeAssert(false && "NULL key"); return BentleyStatus::ERROR; } \
        return _TryGetProperty(key, value); \
        }
#define SET_TRYGET_PROPERTY_IMPL(value_type) \
    SET_PROPERTY_IMPL(value_type) \
    TRY_GET_PROPERTY_IMPL(value_type)

SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementCP)
SET_TRYGET_PROPERTY_IMPL(Dgn::DgnElementId)
SET_TRYGET_PROPERTY_IMPL(Dgn::ColorDef)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementManipulationStrategy::FinishElement
(
    Dgn::DgnModelR model
)
    {
    return _FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr DgnElementManipulationStrategy::FinishElement()
    {
    return _FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementManipulationStrategy::AddWorldOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    _AddWorldOverlay(builder, contrastingToBackgroundColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getVeryLargeLengthInWorldCS
(
Dgn::DgnViewportP viewport
)
    {
    //  Compute a length to be used when drawing "infinite" lines and planes.
    //  The length must not be so huge that it has little precision at the
    //  current view scale. So, we tailor it to the view size. This is a little fuzzy:
    //  we want it to exceed what's visible in even the most severely skewed perspective view....
    DRange3d viewCorners = viewport->GetViewCorners();

    //  We sometimes see some crazy z values in planar models!?
    if (fabs (viewCorners.low.z - viewCorners.high.z) > 1000000)
        viewCorners.low.z = viewCorners.high.z = 0;

    double v = viewCorners.DiagonalDistance();

    //  We also avoid values that would lead to coordinates greater than max float,
    //  as that causes trouble for QV.
    if (v > FLT_MAX/1000)
        v = FLT_MAX/1000;

    return v;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawLineString
(
    Dgn::Render::GraphicBuilderR builder,
    bvector<DPoint3d> const& lineString,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    builder.SetSymbology(color, Dgn::ColorDef::Black(), 1, linePixels);
    builder.AddLineString(lineString.size(), &lineString[0]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawPointString
(
    Dgn::Render::GraphicBuilderR builder,
    bvector<DPoint3d> const& pointString,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    builder.SetSymbology(color, Dgn::ColorDef::Black(), 6, linePixels);
    builder.AddPointString(pointString.size(), &pointString[0]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawArc
(
    Dgn::Render::GraphicBuilderR builder,
    DEllipse3dCR arc,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    builder.SetSymbology(color, Dgn::ColorDef::Black(), 1, linePixels);
    builder.AddArc(arc, !arc.IsCircular(), false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawLine
(
    Dgn::Render::GraphicBuilderR builder,
    DSegment3dCR line,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    bvector<DPoint3d> points(2);
    line.GetEndPoints(points[0], points[1]);
    drawLineString(builder, points, color, linePixels);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawXYCross
(
    Dgn::Render::GraphicBuilderR builder,
    DPoint3dCR intersection,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    Dgn::DgnViewportP vp = builder.GetViewport();
    if (nullptr == vp)
        return;

    BSIRect viewRect = vp->GetViewRect();
    double lineLength = vp->GetViewDelta()->x * (vp->PixelsFromInches(1.0 / 8.0) / viewRect.Width());

    DVec3d xVec = vp->GetXVector();
    DVec3d yVec = vp->GetYVector();
    xVec.ScaleToLength(lineLength);
    yVec.ScaleToLength(lineLength);

    drawLine(builder, DSegment3d::From(intersection + xVec, intersection - xVec), color, linePixels);
    drawLine(builder, DSegment3d::From(intersection + yVec, intersection - yVec), color, linePixels);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawInfiniteLine
(
    Dgn::Render::GraphicBuilderR builder,
    DSegment3dCR segmentOnLine,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    DPoint3d start, end, center;
    segmentOnLine.GetEndPoints(start, end);
    if (start.AlmostEqual(end))
        return;

    segmentOnLine.FractionParameterToPoint(center, 0.5);

    double len = getVeryLargeLengthInWorldCS(builder.GetViewport());
    DVec3d dir = end - start;
    dir.ScaleToLength(len / 2.0);

    DSegment3d infLine = DSegment3d::From(center + dir, center - dir);
    drawLine(builder, infLine, color, linePixels);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawCurveVector
(
    Dgn::Render::GraphicBuilderR builder,
    CurveVectorCR cv,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    builder.SetSymbology(color, Dgn::ColorDef::Black(), 1, linePixels);
    builder.AddCurveVector(cv, false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void drawCurvePrimitive
(
    Dgn::Render::GraphicBuilderR builder,
    ICurvePrimitiveCR primitive,
    Dgn::ColorDefCR color = Dgn::ColorDef::Black(),
    Dgn::Render::LinePixels linePixels = Dgn::Render::LinePixels::Solid
)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP line = primitive.GetLineCP();
            if (nullptr == line)
                return;

            drawLine(builder, *line, color, linePixels);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* lineString = primitive.GetLineStringCP();
            if (nullptr == lineString)
                return;

            drawLineString(builder, *lineString, color, linePixels);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* pointString = primitive.GetPointStringCP();
            if (nullptr == pointString)
                return;

            drawPointString(builder, *pointString, color, linePixels);
            break;
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP arc = primitive.GetArcCP();
            if (nullptr == arc)
                return;

            drawArc(builder, *arc, color, linePixels);
            break;
            }
        default:
            {
            BeAssert(false && "Not implemented.");
            break;
            }
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementManipulationStrategy::_AddWorldOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    bvector<ConstructionGeometry> constructionGeometry = _FinishConstructionGeometry();
    if (constructionGeometry.empty())
        return;

    for (ConstructionGeometry const& cgeom : constructionGeometry)
        {
        if (!cgeom.IsValid())
            continue;

        IGeometryPtr geometry = cgeom.GetGeometry();
        BeAssert(geometry.IsValid());

        switch (cgeom.GetType())
            {
            case CONSTRUCTION_GEOMTYPE_GenericCurveVector:
                {
                CurveVectorPtr cv = geometry->GetAsCurveVector();
                if (cv.IsNull())
                    continue;

                drawCurveVector(builder, *cv, contrastingToBackgroundColor);
                break;
                }
            case CONSTRUCTION_GEOMTYPE_GenericICurvePrimitive:
                {
                ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive();
                if (primitive.IsNull())
                    continue;

                drawCurvePrimitive(builder, *primitive, contrastingToBackgroundColor);
                break;
                }
            case CONSTRUCTION_GEOMTYPE_ArcCenterToStart:
            case CONSTRUCTION_GEOMTYPE_ArcCenterToEnd:
            case CONSTRUCTION_GEOMTYPE_SplineStartTangentLine:
            case CONSTRUCTION_GEOMTYPE_SplineEndTangentLine:
            case CONSTRUCTION_GEOMTYPE_ArcEdge:
            case CONSTRUCTION_GEOMTYPE_SplinePolePoints:
            case CONSTRUCTION_GEOMTYPE_SplineFitPoints:
            case CONSTRUCTION_GEOMTYPE_SplineStartTangentPoint:
            case CONSTRUCTION_GEOMTYPE_SplineEndTangentPoint:
            case CONSTRUCTION_GEOMTYPE_SplinePoleLines:
                {
                ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive();
                if (primitive.IsNull())
                    continue;

                drawCurvePrimitive(builder, *primitive, contrastingToBackgroundColor, Dgn::Render::LinePixels::Code2);
                break;
                }
            case CONSTRUCTION_GEOMTYPE_ArcCenter:
                {
                ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive();
                if (primitive.IsNull())
                    continue;

                bvector<DPoint3d> const* pointString = primitive->GetPointStringCP();
                if (nullptr == pointString || pointString->empty())
                    continue;

                DPoint3d center = pointString->at(0);
                drawXYCross(builder, center, contrastingToBackgroundColor);
                break;
                }
            case CONSTRUCTION_GEOMTYPE_ArcInfiniteLine:
                {
                ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive();
                if (primitive.IsNull())
                    continue;

                DSegment3dCP segmentOnInfLine = primitive->GetLineCP();
                if (nullptr == segmentOnInfLine)
                    continue;

                drawInfiniteLine(builder, *segmentOnInfLine, Dgn::ColorDef::Red(), Dgn::Render::LinePixels::Code2);
                break;
                }
            default:
                break;
            }
        }
    }