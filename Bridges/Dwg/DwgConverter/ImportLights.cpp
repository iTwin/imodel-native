/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgLightExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporter& importer)
    {
    m_dwgLight = DwgDbLight::Cast (context.GetEntityPtrR().get());
    if (nullptr == m_dwgLight || !context.GetModel().Is3d())
        return  BSIERROR;

    m_toBimContext = &context;
    m_importer = &importer;
    
    ElementInputsR  inputs = context.GetElementInputsR ();
    ElementResultsR results = context.GetElementResultsR ();

    // override the default GENERIC_CLASS_PhysicalObject as BIS_CLASS_LightLocation
    inputs.SetClassId (Lighting::Location::QueryClassId(importer.GetDgnDb()));

    BentleyStatus   status = BentleyStatus::SUCCESS;

    // import the light glyph/geometry, if not drawn by the toolkit:
    if (DwgGiDrawable::DrawableType::DistantLight == m_dwgLight->GetLightType() || (status = importer._ImportEntity(results, inputs)) != BentleyStatus::SUCCESS)
        status = this->CreateOrUpdateLightGlyph (results, inputs);

    // get imported light element
    Lighting::LocationPtr light = dynamic_cast <Lighting::Location*> (results.GetImportedElement());
    if (!light.IsValid())
        {
        BeAssert (false && "Light element has not been created!");
        return  BSIERROR;
        }

    // set the light name
    DwgString   name = m_dwgLight->GetName ();
    if (name.IsEmpty())
        name = m_dwgLight->GetDxfName ();
    if (name.IsEmpty())
        name.Assign (L"Light");
    light->SetUserLabel (DwgHelper::ToUtf8CP(name, false));

    // turn on/off the light
    light->SetPropertyValue ("Enabled", m_dwgLight->IsOn());

    // convert the lighting parameters from the DWG light
    Lighting::Parameters    params;
    status = this->ConvertLightParameters (params);

    if (status == BSISUCCESS)
        light->SetParameters(params);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgLightExt::CreateOrUpdateLightGlyph (ElementResultsR results, ElementInputsR inputs) const
    {
    DgnModelR   model = m_toBimContext->GetModel ();
    DwgImporter::ElementCreateParams createParams(model);
    BentleyStatus   status = m_importer->_GetElementCreateParams (createParams, inputs.GetTransform(), m_toBimContext->GetEntity());
    if (BSISUCCESS != status)
        return  status;

    // calculate light transformation
    Transform   lightTransform;
    DVec3d      lightVector = this->CalculateLightVector (lightTransform);

    // calculate light glyph size
    double      lightSize = this->CalculateLightGlyphSize ();

    // calculate Yaw, Pitch, and Roll
    DPoint3d    origin;
    YawPitchRollAngles  angles;
    YawPitchRollAngles::TryFromTransform (origin, angles, lightTransform);

    // create a geometry builder
    GeometryBuilderPtr  builder;
    if (model.Is3d())
        builder = GeometryBuilder::Create (model, createParams.GetCategoryId(), origin, angles);
    else
        builder = GeometryBuilder::Create (model, createParams.GetCategoryId(), DPoint2d::From(origin), angles.GetYaw());
    if (!builder.IsValid())
        return  BSIERROR;

    /*-----------------------------------------------------------------------------------
    Create geometry for the light glyph:
        1. Distant lights are not displayed in ACAD, so we always have to create them.
        2. OpenDWG does not draw any light, so we have to create all when Teigha is used.
    -----------------------------------------------------------------------------------*/
    switch (m_dwgLight->GetLightType())
        {
        case DwgGiDrawable::DrawableType::DistantLight:
            this->CreateDistantLightGlyph (*builder, lightSize);
            break;
        case DwgGiDrawable::DrawableType::SpotLight:
            this->CreateSpotLightGlyph (*builder, lightSize);
            break;
        case DwgGiDrawable::DrawableType::PointLight:
        case DwgGiDrawable::DrawableType::WebLight:
        default:
            {
            // a point geometry for all other types
            ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (DPoint3d::FromZero(), DPoint3d::FromZero());
            builder->Append (*primitive.get());
            }
        }
    
    // calculate display parameters for the light glyph
    Render::GeometryParams  display;
    display.SetCategoryId (createParams.GetCategoryId());
    display.SetSubCategoryId (createParams.GetSubCategoryId());
    display.SetGeometryClass (Render::DgnGeometryClass::Primary);
    display.SetLineColor (this->GetGlyphColor());
    display.SetWeight (this->GetGlyphWeight());

    LineStyleInfoPtr    lsInfo = LineStyleInfo::Create (this->GetGlyphLinestyle(), nullptr);
    if (lsInfo.IsValid())
        display.SetLineStyle (lsInfo.get());

    builder->Append (display);
    
    ElementFactory  factory(results, inputs, createParams, *m_importer);
    factory.SetGeometryBuilder (builder.get());

    status = factory.CreateElement ();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgLightExt::ConvertLightParameters (Lighting::ParametersR lightParams) const
    {
    // set the light intensity
    double  intensity = m_dwgLight->GetIntensity ();
    lightParams.SetIntensity (intensity);

    // set the filter color
    ColorDef    color(m_dwgLight->GetLightColor().GetRGB());
    lightParams.SetColor (color);
    
    // set the lamp color
    lightParams.SetKelvin (m_dwgLight->GetLampColorTemperature());

    // calculate light bulbs
    uint32_t    bulbCount = static_cast<uint32_t>(::floor(intensity));

    // set the light type and type specific parameters
    switch (m_dwgLight->GetLightType())
        {
        case DwgGiDrawable::DrawableType::DistantLight:
            lightParams.SetType (Lighting::LightType::Distant);
            lightParams.SetBulbCount (bulbCount);
            break;
        case DwgGiDrawable::DrawableType::PointLight:
            lightParams.SetType (Lighting::LightType::Point);
            lightParams.SetBulbCount (bulbCount);
            break;
        case DwgGiDrawable::DrawableType::SpotLight:
            {
            lightParams.SetType (Lighting::LightType::Spot);
            lightParams.SetBulbCount (bulbCount);

            AngleInDegrees  innerAngle = AngleInDegrees::FromRadians (m_dwgLight->GetHotspotAngle());
            AngleInDegrees  outerAngle = AngleInDegrees::FromRadians (m_dwgLight->GetFalloffAngle());
            lightParams.SetSpot (Lighting::Parameters::Spot(innerAngle, outerAngle));
            break;
            }
        case DwgGiDrawable::DrawableType::WebLight:
            lightParams.SetType (Lighting::LightType::Point);
            m_importer->ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::Message(), Utf8PrintfString("IES light %ls", m_dwgLight->GetName().c_str()).c_str());
            break;
        case DwgGiDrawable::DrawableType::SkyBackground:
        case DwgGiDrawable::DrawableType::ImageBasedLightingBackground:
            lightParams.SetType (Lighting::LightType::SkyOpening);
            break;
        default:
            m_importer->ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::Message(), Utf8PrintfString("light entity %ls", m_dwgLight->GetName().c_str()).c_str());
            return  BSIERROR;
        }

    // calculate light brightness from physical intensity
    double  lightBrightness = 1.0e+6;
    DwgDbLightingUnits  lightingUnits = m_dwgLight->GetDatabase()->GetLightingUnits ();
    if (lightingUnits != DwgDbLightingUnits::None)
        {
        // convert candelas and lux to luminous flux in lumen
        switch (m_dwgLight->GetPhysicalIntensityMethod())
            {
            case DwgDbLight::IntensityBy::Flux:          // lumen
                lightBrightness = m_dwgLight->GetPhysicalIntensity ();
                break;
            case DwgDbLight::IntensityBy::Illuminance:   // lux or foot-candles
                if (DwgGiDrawable::WebLight != m_dwgLight->GetLightType())
                    lightBrightness = this->ConvertCandelasToLumen() / this->CalculateUnitArea(lightingUnits);
                break;
            case DwgDbLight::IntensityBy::PeakIntensity:
            default:
                lightBrightness = this->ConvertCandelasToLumen ();
                break;
            }
        // NEEDSWORK - scale up the brightness to make Gist cast effective light!!!
        lightBrightness *= 1000.0;
        }
    lightParams.SetLumens (lightBrightness);

    // Attenuation - does DgnPlatform support attenuation yet?

    // Shadows
    uint32_t    shadowSamples = 0;
    DwgGiShadowParameters   shadows;
    if (m_dwgLight->CanCastShadows() && DwgDbStatus::Success == m_dwgLight->GetShadowParameters(shadows) && shadows.AreShadowsOn())
        {
        switch (shadows.GetShadowType())
            {
            case DwgGiShadowParameters::RayTraced:
                // Light::SHADOWQUALITY_Sharp = 1 in V8
                shadowSamples = 1;
                break;
            case DwgGiShadowParameters::Mapped:
                shadowSamples = shadows.GetShadowMapSize ();
                break;
            case DwgGiShadowParameters::Sampled:
                shadowSamples = shadows.GetShadowSamples ();
                break;
            }
        }
    lightParams.SetShadowSamples (shadowSamples);

    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgLightExt::CreateDistantLightGlyph (GeometryBuilderR builder, double lightSize) const
    {
    // draw a line reprenting the light glyph size on the xAxis
    DPoint3d    point1 = DPoint3d::FromZero ();
    DPoint3d    point2 = DPoint3d::From (lightSize, 0, 0);

    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());

    // draw an arrow head pointing away from the origin
    double  delta = 0.1 * lightSize;
    point1.x = point2.x - delta * cos(0.5);
    point1.y = point2.y + delta * sin(0.5);

    primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());

    point1.y = -point1.y;
    primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgLightExt::CreateSpotLightGlyph (GeometryBuilderR builder, double lightSize) const
    {
    // draw a line reprenting the light glyph size on the xAxis
    DPoint3d    point1 = DPoint3d::FromZero ();
    DPoint3d    point2 = DPoint3d::From (lightSize, 0, 0);

    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());

    // draw 2 lines from the origin at the hotspot angle
    point2.y += lightSize * sin(m_dwgLight->GetHotspotAngle());
    primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());

    point2.y = -point2.y;
    primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());

    // a mid-point line perpenticular to the light vector
    point1.x = point2.x = 0.5 * lightSize;
    point2.y *= 0.5;
    point1.y = -point2.y;
    primitive = ICurvePrimitive::CreateLine (point1, point2);
    builder.Append (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgLightExt::IsLightGlyphDisplayed () const
    {
    switch (m_dwgLight->GetGlyphDisplay())
        {
        case DwgDbLight::GlyphDisplay::On:      return  true;
        case DwgDbLight::GlyphDisplay::Off:     return  false;
        case DwgDbLight::GlyphDisplay::Auto:    return  m_dwgLight->GetDatabase()->GetLightGlyphDisplay();
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
double  DwgLightExt::CalculateLightGlyphSize () const
    {
    double  glyphSize = 0.0;
    if (this->IsLightGlyphDisplayed())
        {
        // take the diagnal size of the light glyph:
        DRange3d    range;
        if (DwgDbStatus::Success == m_dwgLight->GetRange(range))
            glyphSize = range.DiagonalDistance ();

        // do not allow 0 size glyph for no-point lights (OpenDWG does not draw lights at all!)
        if (glyphSize <= 0.0)
            {
            DwgGiDrawable::DrawableType type = m_dwgLight->GetLightType();
            if (DwgGiDrawable::DrawableType::DistantLight == type || DwgGiDrawable::DrawableType::SpotLight == type)
                {
                // use 5% viewport size
                DwgDbViewportTableRecordPtr viewport(m_dwgLight->GetDatabase()->GetActiveModelspaceViewportId(), DwgDbOpenMode::ForRead);
                if (!viewport.IsNull())
                    glyphSize = 0.1 * viewport->GetHeight();
                else
                    glyphSize = 0.05;   // 5cm
                }
            }

        glyphSize *= m_importer->GetScaleToMeters();
        }
    return  glyphSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d  DwgLightExt::CalculateLightVector (TransformR lightTransform) const
    {
    // light origin & direction
    DPoint3d    origin = m_dwgLight->GetPosition ();

    /*-----------------------------------------------------------------------------------
    RealDWG Doc days that lightDirection is valid only for distant light, and targetLocation
    valid only for spot light.  But lightDirection does not seem to return a valid vector
    for distant light and targetLocation seems to always return a valid point.
    -----------------------------------------------------------------------------------*/
    DVec3d  xAxis = DVec3d::FromStartEnd (origin, m_dwgLight->GetTargetLocation());
    xAxis.Normalize ();

    Transform   ecs;
    m_dwgLight->GetEcs (ecs);

    DVec3d  zAxis;
    ecs.GetMatrixColumn (zAxis, 2);
    zAxis.Normalize ();

    // build cell matrix by aligning x-axis on the light vector and z-axis on the ECS normal.
    RotMatrix   matrix;
    matrix.SetColumn (xAxis, 0);
    matrix.SetColumn (zAxis, 2);
    matrix.SquareAndNormalizeColumns (matrix, 0, 1);

    origin.Scale (m_importer->GetScaleToMeters());

    lightTransform.InitFrom (matrix, origin);

    return  xAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
double  DwgLightExt::ConvertCandelasToLumen () const
    {
    double  candelas = m_dwgLight->GetPhysicalIntensity ();
    double  lumen = 100.0;

    switch (m_dwgLight->GetLightType())
        {
        case DwgGiDrawable::DrawableType::PointLight:
        case DwgGiDrawable::DrawableType::DistantLight:
            // The luminous flux is 4*PI*candelas in lumens on a full sphere:
            lumen = 4.0 * Angle::Pi() * candelas;
            break;
        case DwgGiDrawable::DrawableType::SpotLight:
            /*---------------------------------------------------------------------------
            The luminous flux is the product of the intensity and the solid angle of the 
            hotspot cone, plus the incremental solid angle of the fall-off region. 

            FUTUREWORK: I can't find the increamental solid angle of the fall-off region 
            in the object's photometric info xrecord.  I leave this 2nd part not applied 
            until we can figure out how to get the value.
            ---------------------------------------------------------------------------*/
            lumen = m_dwgLight->GetHotspotAngle() * candelas;
            break;
        case DwgGiDrawable::DrawableType::WebLight:
            /*---------------------------------------------------------------------------
            There is no analytical formula for Luminous flux that I know of.  Rendering 
            system should numerically integrate the intensities provided in the IES file.
            ---------------------------------------------------------------------------*/
            break;
        }

    return  lumen;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
double  DwgLightExt::CalculateUnitArea (DwgDbLightingUnits lightingUnits) const
    {
    UnitDefinition  meters = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    /*-----------------------------------------------------------------------------------
    For American lighting units (foot-candles), convert square target units to square feet.
    For SI lighting units (lux), convert sqaure target units to square meters.
    -----------------------------------------------------------------------------------*/
    StandardUnit    sourceUnitNumber = DwgDbLightingUnits::American == lightingUnits ? StandardUnit::EnglishFeet : StandardUnit::MetricMeters;
    if (sourceUnitNumber == StandardUnit::MetricMeters)
        return  1.0;

    double  scale = 1.0;
    if (BSISUCCESS == meters.GetConversionFactorFrom(UnitDefinition::GetStandardUnit(sourceUnitNumber)))
        scale *= scale;

    return  scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef    DwgLightExt::GetGlyphColor () const
    {
    // prefer lamp color
    RgbFactor   lampRgb;
    if (DwgDbStatus::Success == m_dwgLight->GetLampColorRGB(lampRgb))
        return  ColorDef(lampRgb.ToIntColor());

    // otherwise try filter color
    DwgCmEntityColor    color = m_dwgLight->GetLightColor().GetEntityColor ();
    if (color.IsByACI())
        return  DwgHelper::GetColorDefFromACI (color.GetIndex()) ;

    if (color.IsByBlock())
        return  ColorDef::White();

    if (color.IsByLayer())
        {
        DwgDbLayerTableRecordPtr    layer(m_dwgLight->GetLayerId(), DwgDbOpenMode::ForRead);
        if (!layer.IsNull())
            color = layer->GetColor ();
        }
    
    return ColorDef(color.GetRed(), color.GetGreen(), color.GetBlue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t    DwgLightExt::GetGlyphWeight () const
    {
    DwgDbLineWeight weight = m_dwgLight->GetLineweight ();

    if (DwgDbLineWeight::WeightByBlock == weight)
        {
        weight = DwgDbLineWeight::Weight000;
        }
    else if (DwgDbLineWeight::WeightByLayer == weight)
        {
        DwgDbLayerTableRecordPtr    layer(m_dwgLight->GetLayerId(), DwgDbOpenMode::ForRead);
        if (!layer.IsNull())
            weight = layer->GetLineweight ();
        }

    if (DwgDbLineWeight::WeightByDefault == weight)
        weight = DwgDbLineWeight::Weight000;

    return  m_importer->GetOptions().GetDgnLineWeight (weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId  DwgLightExt::GetGlyphLinestyle () const
    {
    DwgDbObjectId   ltypeId = m_dwgLight->GetLinetypeId ();
    DwgDbDatabasePtr    dwg = m_dwgLight->GetDatabase ();
    if (!dwg.IsNull())
        {
        if (ltypeId == dwg->GetLinetypeByBlockId())
            {
            ltypeId = dwg->GetLinetypeContinuousId ();
            }
        else if (ltypeId == dwg->GetLinetypeByLayerId())
            {
            DwgDbLayerTableRecordPtr    layer(m_dwgLight->GetLayerId(), DwgDbOpenMode::ForRead);
            if (!layer.IsNull())
                ltypeId = layer->GetLinetypeId ();
            }
        }
    return  m_importer->GetDgnLineStyleFor (ltypeId);
    }
