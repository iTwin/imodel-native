/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
void ConvertV8Lights::Register()
    {
    auto ext = new ConvertV8Lights();
    RegisterExtension(DgnV8Api::PointLightHandler::GetInstance(), *ext);
    RegisterExtension(DgnV8Api::AreaLightHandler::GetInstance(), *ext);
    RegisterExtension(DgnV8Api::DistantLightHandler::GetInstance(), *ext);
    RegisterExtension(DgnV8Api::SpotLightHandler::GetInstance(), *ext);
    RegisterExtension(DgnV8Api::SkyOpeningLightHandler::GetInstance(), *ext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertV8Lights::_DetermineElementParams(DgnClassId& classId, DgnCode& code, DgnCategoryId& category, DgnV8EhCR v8el, Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const& modelMapping) 
    {
    classId = Lighting::Location::QueryClassId(converter.GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertV8Lights::_ProcessResults(ElementConversionResults& results, DgnV8EhCR v8el, ResolvedModelMapping const& modelMapping, Converter& converter) 
    {
    if (!results.m_element.IsValid())
        {
        BeAssert(false);
        return;
        }

    // We only ever want a single result element; lights ignore public children, but area lights can have components on different levels...
    if (!results.m_childElements.empty())
        {
        GeometrySourceP source = results.m_element->ToGeometrySourceP();

        if (nullptr == source)
            return;

        GeometryBuilderPtr builder;

        for (ElementConversionResults& child : results.m_childElements)
            {
            GeometrySourceCP childSource = child.m_element->ToGeometrySource();

            if (nullptr == childSource)
                continue;

            if (!builder.IsValid())
                {
                // If parent already has geometry, target category and placement are good...
                if (source->GetGeometryStream().HasGeometry())
                    {
                    builder = GeometryBuilder::Create(*source, source->GetGeometryStream());
                    }
                else
                    {
                    DPoint3d    origin;
                    RotMatrix   rMatrix;
                    Transform   basisTransform;

                    if (!v8el.GetDisplayHandler()->GetBasisTransform(v8el, (Bentley::TransformR) basisTransform))
                        return;

                    basisTransform = Transform::FromProduct(modelMapping.GetTransform(), basisTransform); // Apply unit scaling...

                    basisTransform.GetTranslation(origin);
                    basisTransform.GetMatrix(rMatrix);

                    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);
                    basisTransform.InitFrom(rMatrix, origin);

                    builder = GeometryBuilder::Create(*results.m_element->GetModel(), childSource->GetCategoryId(), basisTransform);

                    source->SetCategoryId(childSource->GetCategoryId()); // Correct element's category...
                    }

                if (!builder.IsValid())
                    return;
                }

            // Combine child geometry...
            GeometryCollection collection(*childSource);

            for (auto iter : collection)
                {
                GeometricPrimitivePtr elemGeom = iter.GetGeometryPtr();

                if (!elemGeom.IsValid())
                    continue; // Don't need to check for GeometryParts as converter was told not to create them...

                elemGeom->TransformInPlace(iter.GetGeometryToWorld());
                builder->Append(iter.GetGeometryParams()); // Will be ignored if category doesn't match parent...
                builder->Append(*elemGeom, GeometryBuilder::CoordSystem::World);
                }
            }

        if (SUCCESS != builder->Finish(*source))
            return;

        results.m_childElements.clear();
        }

    Lighting::LocationPtr light = dynamic_cast<Lighting::Location*>(results.m_element.get());
    if (!light.IsValid())
        {
        BeAssert (false);
        return;
        }

    auto v8Light = DgnV8Api::LightElement::LoadFromElement(v8el, nullptr);
    if (!v8Light.IsValid())
        {
        BeAssert (false);
        return;
        }

    light->SetPropertyValue("Enabled", v8Light->IsEnabled());

    auto name = v8Light->GetName();
    if (!name.empty())
        light->SetUserLabel(Utf8String(name.c_str()).c_str());

    Lighting::Parameters params;
    params.SetIntensity(v8Light->GetIntensity());
    params.SetColor(ColorDef(((RgbFactor*)&v8Light->GetColor())->ToIntColor()));
    params.SetLumens(v8Light->GetBrightness());
    params.SetKelvin(v8Light->GetTemperatureInKelvin());
    params.SetShadowSamples(v8Light->CastsShadows() ? v8Light->GetShadowSamples() : 0);

    switch (v8Light->GetType())
        {
        case DgnV8Api::Light::LIGHTTYPE_Distant:
            params.SetType(Lighting::LightType::Distant);
            break;

        case DgnV8Api::Light::LIGHTTYPE_Point:
            {
            auto point = (DgnV8Api::PointLight*)v8Light.get();
            params.SetType(Lighting::LightType::Point);
            params.SetBulbCount(point->GetBulbCount());
            }
            break;
    
        case DgnV8Api::Light::LIGHTTYPE_Spot:
            {
            auto spot = (DgnV8Api::SpotLight*)v8Light.get();
            params.SetType(Lighting::LightType::Spot);
            params.SetBulbCount(spot->GetBulbCount());
            params.SetSpot(Lighting::Parameters::Spot(AngleInDegrees::FromRadians(spot->GetInnerAngleInRadians()), AngleInDegrees::FromRadians(spot->GetOuterAngleInRadians())));
            }
            break;

        case DgnV8Api::Light::LIGHTTYPE_Area:
            params.SetType(Lighting::LightType::Area);
            params.SetBulbCount(((DgnV8Api::AreaLight*)v8Light.get())->GetBulbCount());
            break;

        case DgnV8Api::Light::LIGHTTYPE_SkyOpening:
            params.SetType(Lighting::LightType::SkyOpening);
            break;
        }
    light->SetParameters(params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
BisConversionRule ConvertV8Lights::_DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR)
    {
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8eh.GetDisplayHandler();
    if (nullptr == v8DisplayHandler)
        return BisConversionRule::ToDefaultBisBaseClass;
    return BisConversionRule::ToAspectOnly;
    }
