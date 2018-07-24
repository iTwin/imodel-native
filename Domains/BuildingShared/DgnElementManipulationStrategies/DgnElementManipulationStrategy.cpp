/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnElementManipulationStrategies/DgnElementManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void DgnElementManipulationStrategy::_AddWorldOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const
    {
    bvector<IGeometryPtr> constructionGeometry = _FinishConstructionGeometry();
    if (constructionGeometry.empty())
        return;

    for (IGeometryPtr const& geometry : constructionGeometry)
        {
        if (geometry->GetAsCurveVector().IsValid())
            {
            builder.SetSymbology(contrastingToBackgroundColor, Dgn::ColorDef::Black(), 1, Dgn::Render::LinePixels::Code2);
            builder.AddCurveVector(*geometry->GetAsCurveVector(), false);
            }
        else if (geometry->GetAsICurvePrimitive().IsValid())
            {
            ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive();
            switch (primitive->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString:
                    {
                    builder.SetSymbology(contrastingToBackgroundColor, Dgn::ColorDef::Black(), 6, Dgn::Render::LinePixels::Code2);
                    bvector<DPoint3d> const* points = primitive->GetPointStringCP();
                    if (nullptr == points)
                        continue;

                    builder.AddPointString(points->size(), &points->front());
                    break;
                    }
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    builder.SetSymbology(contrastingToBackgroundColor, Dgn::ColorDef::Black(), 1, Dgn::Render::LinePixels::Code2);
                    bvector<DPoint3d> const* points = primitive->GetLineStringCP();
                    if (nullptr == points)
                        continue;

                    builder.AddLineString(points->size(), &points->front());
                    break;
                    }
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    builder.SetSymbology(contrastingToBackgroundColor, Dgn::ColorDef::Black(), 1, Dgn::Render::LinePixels::Code2);
                    DEllipse3d arc;
                    if (!primitive->TryGetArc(arc))
                        continue;

                    builder.AddArc(arc, !arc.IsCircular(), false);
                    }
                default:
                    break;
                }
            }
        }
    }