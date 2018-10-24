/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ConvertView.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <VersionedDgnV8Api/DgnPlatform/ViewGroup.h>
#include <VersionedDgnV8Api/DgnPlatform/NamedView.h>
#include <VersionedDgnV8Api/DgnPlatform/RenderStore.h>
#include <VersionedDgnV8Api/DgnPlatform/LxoEnvironment.h>

#include <PointCloud/PointCloudSettings.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

static Utf8CP VIEW_SETTING_PointCloud = "pointCloud";

static RenderMode toRenderMode(int v8mode)
    {
    switch (v8mode)
        {
        case DgnV8Api::MSRenderMode::Wireframe:
        case DgnV8Api::MSRenderMode::CrossSection:
        case DgnV8Api::MSRenderMode::Wiremesh:
            return RenderMode::Wireframe;

        case DgnV8Api::MSRenderMode::HiddenLine:
            return RenderMode::HiddenLine;

        case DgnV8Api::MSRenderMode::SolidFill:
            return RenderMode::SolidFill;
            break;
        }

    return RenderMode::SmoothShade;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlags Converter::ConvertV8Flags(DgnV8Api::ViewFlags v8flags)
    {
    ViewFlags flags;

    flags.SetShowConstructions(v8flags.constructs);
    flags.SetShowDimensions(v8flags.dimens);
    flags.SetShowPatterns(v8flags.patterns);
    flags.SetShowWeights(v8flags.line_wghts);
    flags.SetShowStyles(!v8flags.inhibitLineStyles);
    flags.SetShowTransparency(v8flags.transparency);
    flags.SetShowFill(v8flags.fill);
    flags.SetShowGrid(v8flags.grid);
    flags.SetShowAcsTriad(v8flags.auxDisplay);
    flags.SetShowTextures(v8flags.textureMaps);
    flags.SetShowMaterials(!v8flags.inhibitRenderMaterials);
    flags.SetShowVisibleEdges(v8flags.renderDisplayEdges);
    flags.SetShowHiddenEdges(v8flags.renderDisplayHidden);
    flags.SetShowShadows(v8flags.renderDisplayShadows);
    flags.SetShowClipVolume(!v8flags.noClipVolume);
    flags.SetRenderMode(toRenderMode(v8flags.renderMode));
    flags.SetShowSourceLights(!v8flags.ignoreSceneLights); // in v8 "scene lights" means source lights
    flags.SetShowSolarLight(!v8flags.ignoreSceneLights);   // in v8 this also controls solar light
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::AddAttachmentsToSelection(DgnModelIdSet& selector, DgnV8ModelRefR v8ModelRef, TransformCR trans)
    {
    if (nullptr == v8ModelRef.GetDgnAttachmentsP())
        return;

    for (auto* attachment : *v8ModelRef.GetDgnAttachmentsP())
        {
        auto attachedModel = attachment->GetDgnModelP();

        if ((nullptr == attachedModel) || (!attachment->IsDisplayed())) //*** TODO || !_WantAttachment(*attachment))
            continue;

        Transform thisTrans = ComputeAttachmentTransform(trans, *attachment);

        ResolvedModelMapping modelMapping = FindModelForDgnV8Model(*attachedModel, thisTrans);
        if (!modelMapping.IsValid())
            continue;

        selector.insert(modelMapping.GetDgnModel().GetModelId());
        AddAttachmentsToSelection(selector, *attachment, thisTrans);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CreateModelSet(DgnModelIdSet& selection, DgnModel const& rootModel, DgnV8ModelR v8RootModel, TransformCR trans)
    {
    selection.insert(rootModel.GetModelId());
    AddAttachmentsToSelection(selection, v8RootModel, trans);
    }

static Render::LinePixels toLinePixels(int styleNo)
    {
    static uint32_t s_linePixels[] = {
        0,     // 0
        0x80808080,     // 1
        0xf8f8f8f8,     // 2
        0xffe0ffe0,     // 3
        0xfe10fe10,     // 4
        0xe0e0e0e0,     // 5
        0xf888f888,     // 6
        0xff18ff18      // 7
        };
    return (styleNo>=0 && styleNo<_countof(s_linePixels)) ? (Render::LinePixels) s_linePixels[styleNo] : Render::LinePixels::Invalid;
    }

static ColorDef toColorDef(Bentley::RgbFactor const& val) {return ColorDef(((RgbFactor const&)val).ToIntColor());}
static ColorDef toColorDef(UInt32 val, DgnFileR dgnFile)
    {
    bool trueColor;
    DgnV8Api::IntColorDef intColor;
    if (SUCCESS != DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColor, nullptr, &trueColor, nullptr, nullptr, val, dgnFile) &&
        SUCCESS != DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColor, nullptr, &trueColor, nullptr, nullptr, val & 0xff, dgnFile))
            return ColorDef::White();

    return ColorDef(intColor.m_int);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void convertGradientSkybox(DisplayStyle3d::EnvironmentDisplay& env, LxoEnvironmentGradientLayerR gradLayer)
    {
    env.m_skybox.m_zenithColor = toColorDef(gradLayer.GetZenithColor());
    env.m_skybox.m_skyColor = toColorDef(gradLayer.GetSkyColor());
    env.m_skybox.m_groundColor = toColorDef(gradLayer.GetGroundColor());
    env.m_skybox.m_nadirColor = toColorDef(gradLayer.GetNadirColor());
    env.m_skybox.m_skyExponent = gradLayer.GetSkyExponent();
    env.m_skybox.m_groundExponent = gradLayer.GetGroundExponent();
    env.m_skybox.m_twoColor = gradLayer.Is2Color();
    env.m_skybox.m_enabled = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void convertImageSkybox(DisplayStyle3d::EnvironmentDisplay& env, LxoEnvironmentImageLayerR layer, DgnFileR v8File, Converter& converter)
    {
    using ImageType = DisplayStyle3d::EnvironmentDisplay::SkyBox::Image::Type;

    ImageType imageType = ImageType::None;
    auto projection = layer.GetProjectionType();
    switch (projection)
        {
        case DgnV8Api::LxoEnvironmentImageLayer::LXO_ENVPROJECT_Spherical:
            imageType = ImageType::Spherical;
            break;
        case DgnV8Api::LxoEnvironmentImageLayer::LXO_ENVPROJECT_Cylindrical:
            imageType = ImageType::Cylindrical;
            break;
        default:
            return; // ###TODO: Image Cube apparently soon to be supported by MicroStation; any others we can support?
        }

    WCharCP imageFileName = layer.GetFileName();
    DgnTextureId textureId = converter.FindOrInsertTextureImage(imageFileName, v8File);
    if (!textureId.IsValid())
        return;

    env.m_skybox.m_image.m_type = imageType;
    env.m_skybox.m_image.m_textureId = textureId;
    env.m_skybox.m_enabled = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void convertSkyboxDisplay(DisplayStyle3dR dstyle3d, DgnV8Api::DisplayStyle const& v8displayStyle, DgnFileR dgnFile, Converter& converter)
    {
    auto v8env = DgnV8Api::LxoEnvironmentManager::GetManagerR().FindLxoEnvironmentByName(v8displayStyle.GetEnvironmentName().c_str(), dgnFile);

    if (nullptr == v8env)
        v8env  = DgnV8Api::DgnPlatformLib::GetHost().GetGraphicsAdmin()._FindEnvironmentInDgnLibs(v8displayStyle.GetEnvironmentName().c_str());

    if (nullptr == v8env)
        return;

    auto layer = v8env->GetCurrentLayer();
    auto& env = dstyle3d.GetEnvironmentDisplayR();
    switch (layer->GetType())
        {
        case DgnV8Api::LxoEnvironmentLayerType::Gradient:
            convertGradientSkybox(env, *(LxoEnvironmentGradientLayerP)layer);
            break;
        case DgnV8Api::LxoEnvironmentLayerType::Image:
            convertImageSkybox(env, *(LxoEnvironmentImageLayerP)layer, dgnFile, converter);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertDisplayStyle(DisplayStyleR style, DgnV8Api::DisplayStyle const& v8displayStyle)
    {
    DgnFileR dgnFile = v8displayStyle.GetFileR();
    auto style3d = style.ToDisplayStyle3dP();

    auto& overrides = v8displayStyle.GetOverrides();
    auto& v8flags = v8displayStyle.GetFlags();
    if (style3d)
        {
        auto env = style3d->GetEnvironmentDisplay();
        env.m_skybox.m_enabled = false;
        env.m_groundPlane.m_enabled = false;
        style3d->SetEnvironmentDisplay(env);

        if (v8flags.m_overrideBackgroundColor)
            {
            if (v8displayStyle.GetEnvironmentTypeDisplayed() == DgnV8Api::EnvironmentDisplay::Color)
                style.SetBackgroundColor(toColorDef(overrides.m_backgroundColor, dgnFile));
            else
                {
                convertSkyboxDisplay(*style3d, v8displayStyle, dgnFile, *this);

                // due to a bug in MS, when this flag is on, viewInfo.ResolveBGColor returns the wrong answer. 
                // It matters for transparency with QV. 255 is the right bg color. 
                style.SetBackgroundColor(toColorDef(255, dgnFile));
                }
            }
        }

    // some viewflags can be "overridden" in the display style
    auto viewFlags = style.GetViewFlags();
    viewFlags.SetRenderMode(toRenderMode(v8flags.m_displayMode));
    viewFlags.SetShowVisibleEdges(v8flags.m_displayVisibleEdges);
    viewFlags.SetShowHiddenEdges(v8flags.m_displayHiddenEdges);
    viewFlags.SetShowShadows(v8flags.m_displayShadows);
    viewFlags.SetShowMaterials(!overrides.m_flags.m_material || 0 != overrides.m_material);
    viewFlags.SetShowTextures(!v8flags.m_ignoreImageMaps);
    if (overrides.m_flags.m_smoothIgnoreLights)
        {
        viewFlags.SetShowSourceLights(false);
        viewFlags.SetShowCameraLights(false);
        viewFlags.SetShowSolarLight(false);
        }

    style.SetViewFlags(viewFlags);

    Render::HiddenLineParams hlParams;
    if (overrides.m_flags.m_visibleEdgeColor)
        {
        hlParams.m_visible.m_ovrColor = true;
        hlParams.m_visible.m_color = toColorDef(overrides.m_visibleEdgeColor, dgnFile);
        hlParams.m_hidden.m_ovrColor = true;
        hlParams.m_hidden.m_color = hlParams.m_visible.m_color;
        }

    if (overrides.m_flags.m_visibleEdgeWeight)
        hlParams.m_visible.m_width = overrides.m_visibleEdgeWeight + 1;
    else
        hlParams.m_visible.m_width = 0;

    if (overrides.m_flags.m_hiddenEdgeWeightZero)
        hlParams.m_hidden.m_width = 1;
    else
        hlParams.m_hidden.m_width = overrides.m_hiddenEdgeWeight==0xffff ? hlParams.m_visible.m_width : overrides.m_hiddenEdgeWeight + 1;

    if (overrides.m_flags.m_visibleEdgeStyle)
        hlParams.m_visible.m_pattern = Render::LinePixels::Solid;
    else
        hlParams.m_visible.m_pattern = Render::LinePixels::Invalid;

    if (overrides.m_flags.m_hiddenEdgeStyle)
        hlParams.m_hidden.m_pattern = 0 == v8flags.m_hiddenEdgeLineStyle ? Render::LinePixels::HiddenLine : toLinePixels(v8flags.m_hiddenEdgeLineStyle);
    else
        hlParams.m_hidden.m_pattern = Render::LinePixels::Solid;

    if (overrides.m_flags.m_hLineTransparency)
        hlParams.m_transparencyThreshold = overrides.m_hLineTransparencyThreshold;

    if (overrides.m_flags.m_elementColor)
        {
        auto viewFlags = style.GetViewFlags();
        viewFlags.SetMonochrome(true);
        style.SetViewFlags(viewFlags);
        style.SetMonochromeColor(toColorDef(overrides.m_elementColor, dgnFile));
        }

    if (style3d && hlParams != Render::HiddenLineParams())
        style3d->SetHiddenLineParams(hlParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Lighting::Parameters toLight(Lighting::LightType type, double intensity, Bentley::RgbFactor const& color, double intensity2=1.0, Bentley::RgbFactor const* color2=nullptr)
    {
    Lighting::Parameters light(type);
    light.SetIntensity(intensity);
    light.SetColor(ColorDef(((RgbFactor*)&color)->ToIntColor()));
    if (color2)
        {
        light.SetIntensity2(intensity2);
        light.SetColor2(ColorDef(((RgbFactor*)color2)->ToIntColor()));
        }
    return light;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keit1h.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertSceneLighting(DisplayStyle3dR displayStyle, DgnV8ViewInfoCR viewInfo, DgnV8ModelR model)
    {
    auto& v8ViewFlags = viewInfo.GetViewFlags();

    auto& manager = DgnV8Api::LightManager::GetManagerR();
    auto setup = manager.GetActiveLightSetupForModel(v8ViewFlags.ignoreSceneLights, model);
    if (nullptr == setup)
        return;

    auto& ambientLight = setup->GetAmbientLight();
    if (ambientLight.IsEnabled() && ambientLight.GetIntensity() > 0.0)
        displayStyle.SetSceneLight(toLight(Lighting::LightType::Ambient, ambientLight.GetIntensity()*100., ambientLight.GetColor()));

    auto& flashLight = setup->GetFlashLight();
    if (flashLight.IsEnabled() && flashLight.GetIntensity() > 0.0)
        displayStyle.SetSceneLight(toLight(Lighting::LightType::Flash, flashLight.GetIntensity()*100., flashLight.GetColor()));
    
    auto& solarLight = setup->GetSolarLight();
    DVec3d dir;
    if (0.0 < solarLight.GetEffectiveVector((Bentley::DVec3dR)dir, model).z)
        {
        dir.Negate();
        Bentley::RgbFactor skyColor;
        displayStyle.SetSolarLight(toLight(Lighting::LightType::Solar, setup->CalculateEffectiveSkyIntensity(skyColor, solarLight, setup->GetSkyDomeLight(), model), solarLight.GetColor()), dir);
        }

    if (v8ViewFlags.ignoreSceneLights)
        { // v8 only shows portrait light if "default lighting" is on
        auto& modelLight1 = setup->GetModelLight1();
        auto& modelLight2 = setup->GetModelLight2();
        if (modelLight1.IsEnabled() || modelLight2.IsEnabled())
            displayStyle.SetSceneLight(toLight(Lighting::LightType::Portrait, !modelLight1.IsEnabled() ? 0 : modelLight1.GetIntensity()*100., modelLight1.GetColor(),
                                                                              !modelLight2.IsEnabled() ? 0 : modelLight2.GetIntensity()*100., &modelLight2.GetColor()));
        }

    double oneOverLog2 = (1.0 / log(2.0));
    double fstop = setup->GetBrightnessMultiplierForView();
    fstop = fstop > 0.0 ? (float)(oneOverLog2 * log(fstop)) : 0.0;
    displayStyle.SetSceneBrightness(fstop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinitionPtr SpatialViewFactory::_MakeView(Converter& converter, ViewDefinitionParams const& parms)
    {
    if (!parms.GetDgnModel().IsPhysicalModel())
        return nullptr;

    DgnDbR db = converter.GetDgnDb();
    DefinitionModelPtr definitionModel = converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    ModelSelectorPtr models = new ModelSelector(*definitionModel, parms.m_name.c_str());
    converter.CreateModelSet(models->GetModelsR(), parms.GetDgnModel(), parms.GetV8Model(), parms.m_trans);

    double toMeters = converter.ComputeUnitsScaleFactor(parms.GetV8Model());
    BeAssert(0.0 < toMeters);

    auto dstyle3d = parms.m_dstyle->ToDisplayStyle3dP();
    BeAssert(dstyle3d);

    auto v8displayStyle = parms.m_viewInfo.GetDisplayStyleCP();
    if (nullptr == v8displayStyle)
        {
        auto env = dstyle3d->GetEnvironmentDisplay();
        env.m_skybox.m_enabled = converter.GetConfig().GetOptionValueBool("SkyBox", true);
        env.m_groundPlane.m_enabled = converter.GetConfig().GetOptionValueBool("GroundPlane", false);
        dstyle3d->SetEnvironmentDisplay(env);
        }

    converter.ConvertSceneLighting(*dstyle3d, parms.m_viewInfo, parms.GetV8Model());

    ViewDefinition3d::Camera camera;
    ViewDefinition3d::Camera* cameraP = nullptr;
    if (parms.m_viewInfo.GetViewFlags().camera)
        {
        auto& v8camera = parms.m_viewInfo.GetCamera();
        cameraP = &camera;
        camera.SetLensAngle(Angle::FromRadians(v8camera.m_angle));
        camera.ValidateLens();
        camera.SetFocusDistance(v8camera.m_focalLength * toMeters);
        camera.SetEyePoint(DPoint3d::FromScale((DPoint3dCR)v8camera.m_position, toMeters));
        }

    ViewDefinitionPtr view = new SpatialViewDefinition(*definitionModel, parms.m_name, *parms.m_categories, *dstyle3d, *models, cameraP);

    // Carry over the simple stuff like view geometry and description
    parms.Apply(*view);

    // Convert the level mask
    converter.ConvertLevelMask(*view, parms.m_viewInfo, &parms.GetV8Model());

    // Convert point cloud view settings from V8 format to DgnDb
    ViewControllerPtr viewController = view->LoadViewController();
    if (viewController.IsValid())
        {
        auto spatialVC = viewController->ToSpatialViewP();
        if (spatialVC)
            {
            m_spatialConverter.ConvertV8PointCloudViewSettings(*spatialVC, parms.m_viewInfo);
            spatialVC->StoreState();
            auto val = spatialVC->GetViewDefinitionR().GetDisplayStyle().GetStyle(VIEW_SETTING_PointCloud);
            if (!val.isNull())
                view->GetDisplayStyle().SetStyle(VIEW_SETTING_PointCloud, val);
            }
        }

    return view;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
ViewDefinitionPtr SpatialViewFactory::_UpdateView(Converter& converter, ViewDefinitionParams const& params, ViewDefinitionR existing)
    {
    ViewDefinitionPtr newDef = _MakeView(converter, params);
    if (!newDef.IsValid())
        return newDef;
    
    SpatialViewDefinition* spatial = newDef->ToSpatialViewP();
    SpatialViewDefinitionP existingViewDef = existing.ToSpatialViewP();

    DgnDbStatus stat;
    // need to update the ModelSelector, DisplayStyle, etc.
    ModelSelectorR newModelSelector = spatial->GetModelSelector();
    newModelSelector.ForceElementIdForInsert(existingViewDef->GetModelSelectorId());

    // Update doesn't actually update the 'ModelSelectorRefersToModels' list.  It deletes the old one and creates a new one.  This will cause a changeset to be created for every update, 
    // even if there were no changes.  Therefore, we have to manually determine if there were changes to the list of models and only call Update if there were.
    Json::Value newModelVal = newModelSelector.ToJson();
    Json::Value oldModelVal = existingViewDef->GetModelSelector().ToJson();
    if (newModelVal != oldModelVal)
        {
        newModelSelector.Update(&stat);
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }
    spatial->SetModelSelector(newModelSelector);

    DisplayStyle3dR newDisplayStyle = spatial->GetDisplayStyle3d();
    newDisplayStyle.ForceElementIdForInsert(existingViewDef->GetDisplayStyleId());

    if (!newDisplayStyle.EqualState(existingViewDef->GetDisplayStyle()))
        {
        newDisplayStyle.Update(&stat);
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }
    spatial->SetDisplayStyle(newDisplayStyle);

    CategorySelectorR newCategorySelector = spatial->GetCategorySelector();
    newCategorySelector.ForceElementIdForInsert(existingViewDef->GetCategorySelectorId());
    Json::Value newCategoryVal = newCategorySelector.ToJson();
    Json::Value oldCategoryVal = existingViewDef->GetCategorySelector().ToJson();
    if (newCategoryVal != oldCategoryVal)
        {
        newCategorySelector.Update(&stat);
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }
    spatial->SetCategorySelector(newCategorySelector);

    newDef->ForceElementIdForInsert(existingViewDef->GetElementId());
    return newDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
ViewFactory::ViewDefinitionParams::ViewDefinitionParams(Converter* c, Utf8StringCR n, ResolvedModelMapping const& m, Bentley::ViewInfoCR vi, bool is3d) : m_name(n), m_modelMapping(m), m_viewInfo(vi)
    {
    auto& db = m_modelMapping.GetDgnModel().GetDgnDb();
    DefinitionModelPtr definitionModel = c->GetJobDefinitionModel();
    m_dstyle = !is3d ? (DisplayStyleP) new DisplayStyle2d(*definitionModel, m_name.c_str()) : new DisplayStyle3d(*definitionModel, m_name.c_str());
    m_categories = new CategorySelector(*definitionModel, m_name.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewFactory::ViewDefinitionParams::Apply(ViewDefinitionR viewDef) const
    {
    viewDef.SetOrigin(m_origin);
    viewDef.SetExtents(m_extents);
    viewDef.SetRotation(m_rot);
    viewDef.SetDescription(m_description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr Converter::ConvertClip(Bentley::ClipVectorPtr v8clip, TransformCP trans)
    {
    if (!v8clip.IsValid())
        return nullptr; // no input, return null

    ClipVectorPtr clipVector = new ClipVector();

    // a clip vector is an array of clip primitives. Convert them one by one
    for (auto& v8ClipPrimitive : *v8clip)
        {
        // a ClipPrimitive can either be a ClipShape or a ClipPlaneSet. See which we have
        auto polygon = v8ClipPrimitive->GetPolygon();
        if (nullptr != polygon) // a non-null polygon means it's a ClipShape
            {
            double zlow = v8ClipPrimitive->GetZLow();
            double zhigh = v8ClipPrimitive->GetZHigh();
            clipVector->push_back(ClipPrimitive::CreateFromShape((DPoint2dCP) polygon->data(), polygon->size(), v8ClipPrimitive->IsMask(), &zlow, &zhigh, (TransformCP)v8ClipPrimitive->GetTransformFromClip(), v8ClipPrimitive->GetInvisible()));
            }
        else if (nullptr != v8ClipPrimitive->GetClipPlanes())
            {
            // we have a ClipPlaneSet. This assumes that ClipPlaneSet is the same in V8 and BIM so we can cast it. I guess that will always be true.
            clipVector->push_back(ClipPrimitive::CreateFromClipPlanes(reinterpret_cast<ClipPlaneSetCR>(*v8ClipPrimitive->GetClipPlanes())));
            }
        else
            {
            BeAssert(false); // what happened?
            }
        }

    if (trans) // this is usually a units from V8->BIM transform
        clipVector->TransformInPlace(*trans);

    return clipVector->empty() ? nullptr : clipVector; // don't return an empty clipVector
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertViewClips(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, TransformCR trans)
    {
    DgnV8Api::EditElementHandle eh;
    ClipVectorPtr clip;

    // the outer clip is stored in the "ClipBoundElement"
    if (SUCCESS == viewInfo.GetDynamicViewSettings().GetClipBoundElemHandle(eh, &v8Model))
        clip = ConvertClip(DgnV8Api::ClipVector::CreateFromElement(eh, &v8Model), &trans); // got one, turn it into a ClipVector

    // clip masks are stored in the "ClipMaskElement"
    if (SUCCESS == viewInfo.GetDynamicViewSettings().GetClipMaskElemHandle(eh, &v8Model))
        {
        // got one, turn it into a ClipVector
        auto clipMask = ConvertClip(DgnV8Api::ClipVector::CreateFromElement(eh, &v8Model, nullptr, DgnV8Api::ClipVolumePass::Outside), &trans);
        if (clipMask.IsValid()) // did it work?
            {
            if (clip.IsValid()) // if we already have a clip bound, append the mask
                clip->Append(*clipMask);
            else
                clip = clipMask; // otherwise just save the mask
            }
        }

    if (clip.IsValid())
        view->SetViewClip(clip); // save the ClipVector in the ViewDefinition
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertViewGrids(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, double toMeters)
    {
    if (nullptr == v8Model.GetDgnFileP()->GetPersistentTcb())
        return;

    // Set grid settings from V8 model info (orientation is stored in tcb, yuck!)...
    GridOrientationType gridOrientation = (GridOrientationType) v8Model.GetDgnFileP()->GetPersistentTcb()->gridOrientation;
    DgnV8Api::ModelInfo const& v8ModelInfo = v8Model.GetModelInfo();
    DPoint2d gridSpacing = DPoint2d::From(v8ModelInfo.GetUorPerGrid(), v8ModelInfo.GetUorPerGrid() * v8ModelInfo.GetGridRatio());
    uint32_t gridsPerRef = v8ModelInfo.GetGridPerReference();

    gridSpacing.Scale(gridSpacing, toMeters); // Account for uor->meter scale...
    view->SetGridSettings(gridOrientation, gridSpacing, gridsPerRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertViewACS(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, TransformCR toMeters, Utf8StringCR name)
    {
    // Set ACS from V8 view if it's not just the default ACS...
    DgnV8Api::IAuxCoordSys* v8acs = viewInfo.GetAuxCoordinateSystem();

    if (nullptr != v8acs && !DgnV8Api::IACSManager::GetManager().CreateACS()->Equals(v8acs))
        {
        Utf8String acsName = name; // Use view name if acs is un-named...
        Bentley::WString acsNameV8 = v8acs->GetName();

        if (!Bentley::WString::IsNullOrEmpty(acsNameV8.c_str()))
            acsName.Assign(acsNameV8.c_str());

        // See if this ACS already exists...
        DgnCode acsCode = AuxCoordSystem::CreateCode(*view, acsName);
        DgnElementId acsId = GetDgnDb().Elements().QueryElementIdByCode(acsCode);

        if (!acsId.IsValid()) // Do we need to do something here for update???
            {
            AuxCoordSystemPtr   acsElm = AuxCoordSystem::CreateNew(*view, acsName);
            Bentley::DPoint3d   acsOrigin;
            Bentley::RotMatrix  acsRMatrix;

            v8acs->GetRotation(acsRMatrix);
            v8acs->GetOrigin(acsOrigin);
            DPoint3d acsOriginMeters;
            toMeters.Multiply(acsOriginMeters, (DPoint3dCR)acsOrigin); // Account for uor->meter scale, global origin differences, GCS transform, ...

            acsElm->SetType((ACSType) v8acs->GetType());
            acsElm->SetOrigin(acsOriginMeters);
            acsElm->SetRotation((RotMatrixCR) acsRMatrix);

            Bentley::WString acsDescrV8 = v8acs->GetDescription();

            if (!Bentley::WString::IsNullOrEmpty(acsDescrV8.c_str()))
                {
                Utf8String acsDescr;

                acsDescr.Assign(acsDescrV8.c_str());
                acsElm->SetDescription(acsDescr.c_str());
                }

            DgnDbStatus acsStatus;
            acsElm->Insert(&acsStatus);

            if (DgnDbStatus::Success == acsStatus)
                acsId = acsElm->GetElementId();
            }

        view->SetAuxiliaryCoordinateSystem(acsId);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool convertBackgroundMap(DisplayStyle& displayStyle, DgnV8ViewInfoCR viewInfo, DgnFileR dgnFile, DgnV8ModelCR v8Model)
    {
    auto        elementRef = dgnFile.FindByElementId(viewInfo.GetElementId(), true);
    size_t      stringLength = 0;
    static      uint32_t   STRING_LINKAGE_KEY_BackgroundMapJson = 89;

    if (nullptr == elementRef)
        return false;

    DgnV8Api::ElementHandle       eeh(elementRef);

    if (BSISUCCESS != DgnV8Api::StringXAttribute::Extract(nullptr, 0, &stringLength, eeh, 0, STRING_LINKAGE_KEY_BackgroundMapJson) ||
        0 == stringLength)
        return false;

    WChar*      wBackgroundMapJson = (WChar*) _alloca ((1+stringLength) * sizeof (WChar));
    DgnV8Api::StringXAttribute::Extract (wBackgroundMapJson, stringLength, nullptr, eeh, 0, STRING_LINKAGE_KEY_BackgroundMapJson);
    Json::Value     value = Json::Reader::DoParse(Utf8String(wBackgroundMapJson));

    if (value.isObject())
        {
        if (value.isMember("groundBias"))
            {
            auto const&        modelInfo = v8Model.GetModelInfo();
            value["groundBias"] =  value["groundBias"].asDouble() * (DgnV8Api::ModelInfo::GetUorPerMaster(&modelInfo) / DgnV8Api::ModelInfo::GetUorPerMeter(&modelInfo));
            }
            
        displayStyle.SetStyle(DisplayStyle::json_backgroundMap(), value);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::ConvertView(DgnViewId& viewId, DgnV8ViewInfoCR viewInfo, Utf8StringCR defaultName,
                                     Utf8StringCR defaultDescription, TransformCR trans, ViewFactory& viewFactory)
    {
    Utf8String name = _GetNamePrefix().append(_ComputeViewName(defaultName, viewInfo));

    DefinitionModelPtr definitionModel = GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return BSIERROR;

    DgnViewId existingViewId;
    Utf8String v8ViewName;
    if (IsUpdating())
        {
        double lastModified;
        GetSyncInfo().TryFindView(existingViewId, lastModified, v8ViewName, viewInfo);
        if (!existingViewId.IsValid())
            existingViewId = ViewDefinition::QueryViewId(*definitionModel, name);
        }

    DgnFilePtr v8File = viewInfo.GetRootDgnFileP(); // the file needs to have a non-zero reference count so LoadRootModelById will work.
    auto v8Model = v8File->LoadRootModelById(nullptr, viewInfo.GetRootModelId());
    if (nullptr == v8Model)
        {
        if (LOG_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG.tracev("ConvertView [%s] - root model is missing in V8 file => view not converted", name.c_str());
        return BSIERROR;
        }

    GetAttachments(*v8Model);   // if v8Model is not the converter's root model, then we won't already have its reference attachments loaded.

    if (LOG_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG.tracev("ConvertView [%s] (root=%d)", name.c_str(), v8Model->GetModelId());

    ResolvedModelMapping modelMapping = FindModelForDgnV8Model(*v8Model, trans);
    if (!modelMapping.IsValid())
        {
        if (LOG_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG.tracev("    [%s] root model was not converted => view not converted", name.c_str());
        return BSIERROR;
        }
    DgnModelP model = &modelMapping.GetDgnModel();

    if (!viewInfo.GetViewFlags().on_off || _FilterOutView(viewInfo))
        return BSIERROR;

    // Get a unique name
    Utf8String suffix;
    Utf8String baseName(name);
    for (int i=0; ViewDefinition::QueryViewId(*definitionModel, name).IsValid();)
        {
        name = RemapViewName(baseName, *v8Model->GetDgnFileP(), suffix);
        suffix.Sprintf("-%d", ++i);
        }

    // If the view exists and the name hasn't changed, use its existing name.  Otherwise, the previous loop will have changed its name thus causing an actual update
    if (existingViewId.IsValid() && defaultName.EqualsI(v8ViewName))
        {
        ViewDefinitionPtr existingDef = GetDgnDb().Elements().GetForEdit<ViewDefinition>(existingViewId);
        name = existingDef->GetCode().GetValueUtf8();
        }
    ViewFactory::ViewDefinitionParams parms(this, name, modelMapping, viewInfo, viewFactory._Is3d());
    parms.m_description = defaultDescription;
    parms.m_trans = trans;

    // DisplayStyle
    ViewFlags flags = ConvertV8Flags(viewInfo.GetViewFlags());

    flags.SetShowBackgroundMap(convertBackgroundMap(*parms.m_dstyle, parms.m_viewInfo, *v8File, *v8Model));

    parms.m_dstyle->SetViewFlags(flags);
    ColorDef bgColor(DgnV8Api::IntColorDef(viewInfo.ResolveBGColor()).m_int); // Always set view's background color to "resolved" background color from V8.
    parms.m_dstyle->SetBackgroundColor(bgColor);

    auto v8displayStyle = parms.m_viewInfo.GetDisplayStyleCP();
    if (v8displayStyle)
        {
        Utf8String styleName = Utf8String(v8displayStyle->GetName().c_str());   
        auto existingStyle = m_dgndb->Elements().Get<DisplayStyle>(m_dgndb->Elements().QueryElementIdByCode(DisplayStyle::CreateCode(*definitionModel, styleName)));
        if (existingStyle.IsValid() && (existingStyle->Is3d() == parms.m_dstyle->Is3d())) // only share display styles if the same dimension (V8 doesn't care)
            parms.m_dstyle = existingStyle->MakeCopy<DisplayStyle>();
        else
            {
            if (!existingStyle.IsValid())
                parms.m_dstyle->SetCode(DisplayStyle::CreateCode(*definitionModel, styleName));
            ConvertDisplayStyle(*parms.m_dstyle, *v8displayStyle);
            }
        // Need to ensure that the json value holds a uint, not an int
        parms.m_dstyle->SetBackgroundColor(parms.m_dstyle->GetBackgroundColor());
        }

    // View geometry
    trans.Multiply(parms.m_origin, (DPoint3dCR)viewInfo.GetOrigin());

    // TFS#341830 - Some DgnV8 files have views that claim to be both defined and on, however have unusable geominfo. Attempt to detect and inject something that won't cause crashes later.
    if (viewInfo.GetDelta().Magnitude() < mgds_fc_epsilon)
        parms.m_extents.Init(DgnUnits::OneMeter(), DgnUnits::OneMeter(), DgnUnits::OneMeter());
    else
        {
        auto viewCornerSrc = Bentley::DPoint3d::FromSumOf(viewInfo.GetOrigin(), viewInfo.GetDelta());  // corner of view (source coordinates)
        DPoint3d viewCorner;
        trans.Multiply(viewCorner, (DPoint3dCR)viewCornerSrc); // corner of view (BIM coordinates)
        parms.m_extents = DVec3d::FromStartEnd(parms.m_origin, viewCorner);
        }

    Bentley::RotMatrix v8Rotation = viewInfo.GetRotation();
    if (v8Rotation.MaxAbs() < mgds_fc_epsilon)
        v8Rotation.InitIdentity();

    parms.m_rot = (RotMatrixCR)v8Rotation;

    ViewDefinitionPtr view;
    ViewDefinitionPtr existingDef;
    if (IsUpdating() && existingViewId.IsValid())
        {
        existingDef = GetDgnDb().Elements().GetForEdit<ViewDefinition>(existingViewId);
        // When loading a ViewDefinition element, the Json for the gridPerRef is inaccurately stored as an int instead of a uint. This means that if it is being compared to a ViewDefinition
        // created on the fly and not loaded, these two values will not match and return a false negative.
        GridOrientationType orientation;
        DPoint2d spacing;
        uint32_t gridsPerRef;
        existingDef->GetGridSettings(orientation, spacing, gridsPerRef);
        existingDef->SetGridSettings(orientation, spacing, gridsPerRef);

        // When the display style is loaded from the db, the type of the background color is set to type int instead of uint.  This makes comparisons to a display style that was set on the
        // fly give a false negative.
        existingDef->GetDisplayStyle().SetBackgroundColor(existingDef->GetDisplayStyle().GetBackgroundColor());
        ViewControllerPtr viewController = existingDef->LoadViewController();
        if (viewController.IsValid())
            {
            auto spatialVC = viewController->ToSpatialViewP();
            if (spatialVC)
                {
                PointCloudViewSettings pcvs = PointCloudViewSettings::FromView(*spatialVC);
                pcvs._Save(*existingDef);
                }
            }

        view = viewFactory._UpdateView(*this, parms, *existingDef);
        }
    else
        view = viewFactory._MakeView(*this, parms);

    if (!view.IsValid())
        return BSIERROR;

    ConvertViewClips(view, viewInfo, *v8Model, trans);
    ConvertViewGrids(view, viewInfo, *v8Model, ComputeUnitsScaleFactor(*v8Model));
    ConvertViewACS(view, viewInfo, *v8Model, trans, name);

    if (existingViewId.IsValid())
        {
        if (!view->EqualState(*existingDef) || view->GetName() != existingDef->GetName())
            {
            if (!view->Update().IsValid())
                {
                ReportError(IssueCategory::CorruptData(), Issue::Error(), name.c_str());
                return BSIERROR;
                }
            }
        }
    else
        {
        if (!view->Insert().IsValid())
            {
            // this sometimes happens when a corrupt DgnV8 file has more than one viewgroup with the same name (which is an error).
            // Just ignore the later viewgroups.
            ReportError(IssueCategory::CorruptData(), Issue::Error(), name.c_str());
            return BSIERROR;
            }
        }

    if (LOG_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG.tracev("    [%s] -> ViewId %lld", name.c_str(), view->GetViewId().GetValue());

    // Keep a map between V8 views and DgnDb views. This is needed to set the display on/off for external models in DgnDb.
    m_viewNumberMap.insert({view->GetViewId(), viewInfo.GetViewNumber()});
    if (!viewId.IsValid())
        viewId = view->GetViewId();
    GetChangeDetector()._OnViewSeen(*this, viewId);
    if (IsUpdating() && existingViewId.IsValid())
        GetSyncInfo().UpdateView(viewId, defaultName.c_str(), viewInfo);
    else
        GetSyncInfo().InsertView(viewId, viewInfo, defaultName.c_str());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertViewGroup(DgnViewId& firstView, DgnV8Api::ViewGroup& vg, TransformCR trans, ViewFactory& fac)
    {
    // Find default "selected" view index using similiar logic to UstnViewStateAdmin::_OnViewGroupMadeActive..
    int  lastViewOpened = -1;
    byte viewZOrderStack[DgnV8Api::MAX_VIEWS];
    bool viewOnZStack[DgnV8Api::MAX_VIEWS];

    vg.GetZOrder(viewZOrderStack, _countof(viewZOrderStack));
    memset(viewOnZStack, 0, sizeof(viewOnZStack));

    for (UInt32 iView = 0; iView < DgnV8Api::MAX_VIEWS; ++iView)
        {
        // Root model of each view must be filled. Otherwise, EnsureLevelMaskCoverage will refuse to work!
        vg.GetViewInfo(iView).GetRootModelP(true);

        if (0 < viewZOrderStack[iView] && viewZOrderStack[iView] <= DgnV8Api::MAX_VIEWS)
            viewOnZStack[viewZOrderStack[iView]-1] = true;
        }

    // do this backwards so we get lowered numbered views which display in front...
    for (Int32 iView = DgnV8Api::MAX_VIEWS-1; iView >= 0; iView--)
        {
        if (viewOnZStack[iView])
            continue;

        if (!vg.GetViewInfo(iView).GetViewFlags().on_off || !vg.GetViewPortInfo(iView).m_wasDefined)
            continue;

        lastViewOpened = iView;
        }

    // now process the view z order stacked views...
    for (UInt32 iView = 0; iView < DgnV8Api::MAX_VIEWS; ++iView)
        {
        int viewIndex = viewZOrderStack[iView];
        if (viewIndex <= 0 || viewIndex >= DgnV8Api::MAX_VIEWS)
            continue;

        if (!viewOnZStack[--viewIndex])
            continue;

        viewOnZStack[viewIndex] = false; // once we have processed a view ignore any repeats for it...

        if (!vg.GetViewInfo(viewIndex).GetViewFlags().on_off || !vg.GetViewPortInfo(viewIndex).m_wasDefined)
            continue;

        lastViewOpened = viewIndex;
        }

    vg.EnsureLevelMaskCoverage(nullptr, false);
    vg.ResetEffectiveLevelMasks();
    vg.ResynchLevelMasks(nullptr, false);

    for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
        {
        Utf8PrintfString defaultName("%s - View %d", Bentley::Utf8String(vg.GetName()).c_str(), i + 1);
        DgnViewId viewId;
        ConvertView(viewId, vg.GetViewInfo(i), defaultName, "", trans, fac);
        if (!firstView.IsValid() || i == lastViewOpened)
            firstView = viewId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId Converter::ConvertNamedView(DgnV8Api::NamedView& namedView, TransformCR trans, ViewFactory& fac)
    {
    ViewInfoR viewInfo = namedView.GetViewInfoR();
    DgnV8ModelP v8Model = viewInfo.GetRootModelP();
    if (nullptr == v8Model)
        return DgnViewId();

    auto v8mm = FindModelForDgnV8Model(*v8Model, trans);
    if (!v8mm.IsValid())
        return DgnViewId();

    DgnModel* model = &v8mm.GetDgnModel();
    if (nullptr == model)
        return DgnViewId();

    viewInfo.EnsureLevelMaskCoverage();
    LevelMaskTreeP levelMasks = viewInfo.GetLevelMasksP();
    if (levelMasks)
        {
        levelMasks->ResetEffectiveMasks();
        levelMasks->ResynchLevelMask(v8Model, 0, nullptr, false);
        }

    Utf8String name(namedView.GetName().c_str());
    Utf8String descr(namedView.GetDescription().c_str());
    DgnViewId viewId;
    ConvertView(viewId, viewInfo, name, descr, trans, fac);
    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::Convert2dViewsOf(DgnV8Api::ViewInfoPtr& firstViewInfo, DgnV8Api::ViewGroup& vg, DgnV8ModelR rootModel, ViewFactory& fac)
    {
    for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
        {
        if (vg.GetViewInfo(i).GetRootModelId() != rootModel.GetModelId())
            continue;

        Bentley::DgnFilePtr filePtr(vg.GetViewInfo(i).GetRootDgnFileP());   // We must make sure that the V8 file is referenced before we can get its root model
        rootModel.FillSections(DgnV8Api::DgnModelSections::Model);  // Root model of each view must be filled. Otherwise, EnsureLevelMaskCoverage will refuse to work!
        vg.EnsureLevelMaskCoverage(nullptr, false);
        vg.ResetEffectiveLevelMasks();
        vg.ResynchLevelMasks(nullptr, false);

        Utf8PrintfString defaultName("%s - View %d", Bentley::Utf8String(vg.GetName()).c_str(), i + 1);
        DgnViewId viewId;

        auto uorToMeter = ComputeUnitsScaleTransform(*vg.GetViewInfo(i).GetRootModelP());

        ConvertView(viewId, vg.GetViewInfo(i), defaultName, "", uorToMeter, fac);

        if (viewId.IsValid() && !m_defaultViewId.IsValid())
            m_defaultViewId = viewId;

        if (!firstViewInfo.IsValid())
            firstViewInfo = DgnV8Api::ViewInfo::CopyFrom(vg.GetViewInfo(i), true, true, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ViewGroupPtr Converter::FindFirstViewGroupShowing(DgnV8ModelR rootModel)
    {
    for (DgnV8Api::ViewGroupPtr const& vg : rootModel.GetDgnFileP()->GetViewGroups())
        {
        for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
            {
            if (vg->GetViewInfo(i).GetRootModelId() == rootModel.GetModelId())
                return vg;
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ViewInfoPtr Converter::CreateV8ViewInfo(DgnV8ModelR rootModel, Bentley::DRange3dCR modelRange)
    {
    auto viewInfo = DgnV8Api::ViewInfo::Create(false);
    viewInfo->SetRootModel(&rootModel);
    auto& geomInfo = viewInfo->GetGeomInfoR();
    geomInfo.m_origin = modelRange.low;
    geomInfo.m_delta = Bentley::DVec3d::FromStartEnd(modelRange.low, modelRange.high);
    geomInfo.m_rotation = Bentley::RotMatrix::FromIdentity();
    DgnV8Api::ViewInfo::GetDefaultFlags(viewInfo->GetGeomInfoR().m_viewFlags);
    return viewInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::UpdateViewChildren(ViewDefinitionR newViewDef, DgnViewId existingViewDefId)
    {
    auto existingViewDefC = ViewDefinition::Get(GetDgnDb(), existingViewDefId);
    if (!existingViewDefC.IsValid())
        {
        BeAssert(false);
        return;
        }
    ViewDefinitionPtr existingViewDef = existingViewDefC->MakeCopy<ViewDefinition>();

    // CategorySelector
    if (true)
        {
        auto newCatSel = MakeCopyForUpdate(newViewDef.GetCategorySelector(), existingViewDef->GetCategorySelector());
        newCatSel->Update();
        newViewDef.SetCategorySelector(*dynamic_cast<CategorySelector*>(newCatSel.get()));
        }

    // DisplayStyle
    if (true)
        {
        auto newDStyle = MakeCopyForUpdate(newViewDef.GetDisplayStyle(), existingViewDef->GetDisplayStyle());
        newDStyle->Update();

        DisplayStyle& newStyle = *dynamic_cast<DisplayStyle*>(newDStyle.get());
        auto view3d = newViewDef.ToView3dP();
        if (view3d)
            {
            auto style3d = newStyle.ToDisplayStyle3dP();
            BeAssert(style3d);
            view3d->SetDisplayStyle3d(*style3d);
            }
        else
            {
            auto view2d = newViewDef.ToView2dP();
            auto style2d = newStyle.ToDisplayStyle2dP();
            BeAssert(style2d);
            if (view2d)
                view2d->SetDisplayStyle2d(*style2d);
            }
        }

    // ModelSelector
    SpatialViewDefinitionP existingSpatialViewDef = existingViewDef->ToSpatialViewP();
    if (nullptr != existingSpatialViewDef)
        {
        SpatialViewDefinitionP newSpatialViewDef = newViewDef.ToSpatialViewP();
        auto newModSel = MakeCopyForUpdate(newSpatialViewDef->GetModelSelector(), existingSpatialViewDef->GetModelSelector());
        newModSel->Update();
        newSpatialViewDef->SetModelSelector(*dynamic_cast<ModelSelector*>(newModSel.get()));
        }
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
