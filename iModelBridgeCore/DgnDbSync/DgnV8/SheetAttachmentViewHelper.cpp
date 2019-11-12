/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "SheetAttachmentViewHelper.h"

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
//! Helps with generating drawing and spatial views for use by sheet view attachments
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct SheetAttachmentViewHelper
    {
    protected:
    Utf8String m_name;

    Converter& m_converter;
    ResolvedModelMapping const& m_sheetModelMapping;
    DgnAttachmentCR m_v8DgnAttachment;
    DgnV8ViewInfoCP m_v8SheetViewInfo;
    ViewFactory& m_nvvf;

    void SetViewFlags(DisplayStyleR dstyle);

    void SetCategories(ViewDefinitionR viewDef, SyncInfo::LevelExternalSourceAspect::Type ltype);

    SheetAttachmentViewHelper(Converter& converter, DgnAttachmentCR v8DgnAttachment, ResolvedModelMapping const& sheetModelMapping, 
                                DgnV8ViewInfoCP v8SheetModelView, ViewFactory& nvvf);
    public:
    DgnDbR GetDgnDb() {return m_converter.GetDgnDb();}
    };

//=======================================================================================
//! Helps with generating drawing views for use by sheet view attachments
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct DrawingViewHelper : SheetAttachmentViewHelper
    {
    DEFINE_T_SUPER(SheetAttachmentViewHelper)

    bool m_isFromProxyGraphics;
    DrawingModelR m_drawingModel;

    void SetViewGeometry(DrawingViewDefinitionR viewDef);

    public:
    DrawingViewHelper(Converter& converter, bool isFromProxyGraphics, DrawingModelR dmodel, 
                                DgnAttachmentCR v8DgnAttachment, ResolvedModelMapping const& sheetModelMapping, 
                                DgnV8ViewInfoCP v8SheetView, ViewFactory& nvvf)
        : T_Super(converter, v8DgnAttachment, sheetModelMapping, v8SheetView, nvvf), m_isFromProxyGraphics(isFromProxyGraphics), m_drawingModel(dmodel)
        {}
    
    DrawingViewDefinitionPtr CreateView();
    DrawingViewDefinitionPtr CreateModifiedCopyOfNamedView(DgnViewId namedViewId);
    };

//=======================================================================================
//! Helps with generating spatial views for use by sheet view attachments
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct SpatialViewHelper : SheetAttachmentViewHelper
    {
    DEFINE_T_SUPER(SheetAttachmentViewHelper)

    SpatialModelR m_spatialModel;

    void SetViewGeometry(SpatialViewDefinitionR viewDef);

    public:
    
    SpatialViewHelper(Converter& converter, SpatialModelR smodel, 
                                DgnAttachmentCR v8DgnAttachment, ResolvedModelMapping const& sheetModelMapping, 
                                DgnV8ViewInfoCP v8SheetView, ViewFactory& nvvf, DgnViewId namedViewId)
        : T_Super(converter, v8DgnAttachment, sheetModelMapping, v8SheetView, nvvf), m_spatialModel(smodel)
        {}

    SpatialViewDefinitionPtr CreateView();
    SpatialViewDefinitionPtr CreateModifiedCopyOfNamedView(DgnViewId namedViewId);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void turnOnAllCategoriesUsedInModel(CategorySelectorR cats, DgnModel const& model)
    {
    Utf8PrintfString selectSql("SELECT DISTINCT Category.Id FROM %s WHERE Model.Id=?", 
                               model.Is3dModel()? BIS_SCHEMA(BIS_CLASS_GeometricElement3d):
                                                  BIS_SCHEMA(BIS_CLASS_GeometricElement2d));
    auto stmt = model.GetDgnDb().GetPreparedECSqlStatement(selectSql.c_str());
    stmt->BindId(1, model.GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        cats.AddCategory(stmt->GetValueId<DgnCategoryId>(0));
        }
    }


#ifndef DRAWING_IS_CREATED_BY_MERGE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void turnOnAllClasses (DisplayStyleR displayStyle)
    {
    auto    viewFlags = displayStyle.GetViewFlags();

    viewFlags.SetShowConstructions(true);
    viewFlags.SetShowDimensions(true);
    viewFlags.SetShowPatterns(true);

    displayStyle.SetViewFlags(viewFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewDefinitionPtr DrawingViewHelper::CreateView()
    {
    DefinitionModelPtr definitionModel = m_converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    auto dstyle = new DisplayStyle2d(*definitionModel, m_name.c_str());
    auto catSel = new CategorySelector(*definitionModel, m_name.c_str());

    DrawingViewDefinitionPtr view = new DrawingViewDefinition(*definitionModel, m_name.c_str(), m_drawingModel.GetModelId(), *catSel, *dstyle);

    SetViewGeometry(*view);
    SetCategories(*view, SyncInfo::LevelExternalSourceAspect::Type::Drawing);
    SetViewFlags(view->GetDisplayStyle());

    // We are creating drawing entirely from displayed geometry - so turn on
    // all categories and display all viewflags.
    turnOnAllCategoriesUsedInModel(view->GetCategorySelector(), m_drawingModel);
    turnOnAllClasses(view->GetDisplayStyle());

    view->SetIsPrivate(true);
    view->GetDisplayStyle().SetIsPrivate(true);
    view->GetCategorySelector().SetIsPrivate(true);
    return view;
    }

#else

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewDefinitionPtr DrawingViewHelper::CreateView()
    {
    DefinitionModelPtr definitionModel = m_converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    auto dstyle = new DisplayStyle2d(*definitionModel, m_name.c_str());
    auto catSel = new CategorySelector(*definitionModel, m_name.c_str());

    DrawingViewDefinitionPtr view = new DrawingViewDefinition(*definitionModel, m_name.c_str(), m_drawingModel.GetModelId(), *catSel, *dstyle);

    SetViewGeometry(*view);

    // NB: We don't use the parent sheet's level mask tree if the generated view is based on proxy graphics. 
    // In that case, the level mask for the attachment would refer to the original 3D reference model. We have left 
    //  the 3D levels behind. The proxy graphics have their own categories, such as "SectionCut". We want to turn them on.
    if (m_isFromProxyGraphics)
        turnOnAllCategoriesUsedInModel(view->GetCategorySelector(), m_drawingModel);
    else
        {
        SetCategories(*view, SyncInfo::LevelExternalSourceAspect::Type::Drawing);
        m_converter._TurnOnExtractionCategories(view->GetCategorySelector());   // NB! The drawing model might contain the results of pulling in proxy graphics from 3D attachments. Make sure the proxies are displayed, too.
        }

    SetViewFlags(view->GetDisplayStyle());

    view->SetIsPrivate(true);
    view->GetDisplayStyle().SetIsPrivate(true);
    view->GetCategorySelector().SetIsPrivate(true);
    return view;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewDefinitionPtr SpatialViewHelper::CreateView()
    {
    DefinitionModelPtr definitionModel = m_converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    ModelSelectorPtr models = new ModelSelector(*definitionModel, m_name.c_str());
    auto dstyle = new DisplayStyle3d(*definitionModel, m_name.c_str());
    auto catSel = new CategorySelector(*definitionModel, m_name.c_str());

    SpatialViewDefinitionPtr view = new SpatialViewDefinition(*definitionModel, m_name.c_str(), *catSel, *dstyle, *models);

    m_converter.CreateModelSet(view->GetModelSelector().GetModelsR(), m_spatialModel, *m_v8DgnAttachment.GetDgnModelP(), m_converter.GetRootTrans());
    SetViewGeometry(*view);     // (depends on modelselector, so populate that first!)
    
    SetCategories(*view, SyncInfo::LevelExternalSourceAspect::Type::Spatial);

    SetViewFlags(view->GetDisplayStyle());
    auto& env = view->GetDisplayStyle3d().GetEnvironmentDisplayR();
    env.m_groundPlane.m_enabled = false;
    env.m_skybox.m_enabled = false;

    view->SetIsPrivate(true);
    view->GetModelSelector().SetIsPrivate(true);
    view->GetDisplayStyle().SetIsPrivate(true);
    view->GetCategorySelector().SetIsPrivate(true);

    if (m_v8DgnAttachment.IsCameraOn())
        {
        Transform thisTrans = m_converter.ComputeAttachmentTransform(m_converter.GetRootTrans(), m_v8DgnAttachment);
        ResolvedModelMapping modelMapping = m_converter.FindFirstResolvedModelMapping(*m_v8DgnAttachment.GetDgnModelP());
        if (modelMapping.IsValid())
            {
            DPoint3d    eyePoint;

            modelMapping.GetTransform().Multiply (eyePoint, (DPoint3dR) m_v8DgnAttachment.GetCameraPosition());
            view->TurnCameraOn();
            view->GetCameraR().SetEyePoint(eyePoint);
            view->GetCameraR().SetFocusDistance(m_v8DgnAttachment.GetCameraFocalLength() * modelMapping.GetTransform().MatrixColumnMagnitude(0));
            }
        }

    return view;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewDefinitionPtr SpatialViewHelper::CreateModifiedCopyOfNamedView(DgnViewId namedViewId)
    {
    auto existingView = GetDgnDb().Elements().Get<SpatialViewDefinition>(namedViewId);
    if (!existingView.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    // Start with a copy of the named view
    SpatialViewDefinitionPtr newView = dynamic_cast<SpatialViewDefinition*>(existingView->Clone().get());

    DefinitionModelPtr definitionModel = m_converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    newView->SetCode(ViewDefinition::CreateCode(*definitionModel, m_name.c_str()));

    // We'll share the same geometry, model selector, and clip with the named view

    // We'll adopt the attachment's view flags 
    newView->SetDisplayStyle3d(*new DisplayStyle3d(*definitionModel, m_name.c_str()));
    SetViewFlags(newView->GetDisplayStyle());

    // And, we will create the category selector based on the attachment
    newView->SetCategorySelector(*new CategorySelector(*definitionModel, m_name.c_str()));
    SetCategories(*newView, SyncInfo::LevelExternalSourceAspect::Type::Spatial);

    newView->SetIsPrivate(true);
    newView->GetDisplayStyle().SetIsPrivate(true);
    newView->GetCategorySelector().SetIsPrivate(true);
    return newView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewDefinitionPtr DrawingViewHelper::CreateModifiedCopyOfNamedView(DgnViewId namedViewId)
    {
    auto existingView = GetDgnDb().Elements().Get<DrawingViewDefinition>(namedViewId);
    if (!existingView.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    // Start with a copy of the named view
    DrawingViewDefinitionPtr newView = dynamic_cast<DrawingViewDefinition*>(existingView->Clone().get());

    DefinitionModelPtr definitionModel = m_converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    newView->SetCode(ViewDefinition::CreateCode(*definitionModel, m_name.c_str()));

    // We'll share the same geometry and basemodel with the named view

    // We'll adopt the attachment's view flags 
    newView->SetDisplayStyle2d(*new DisplayStyle2d(*definitionModel, m_name.c_str()));
    SetViewFlags(newView->GetDisplayStyle());

    // And, we will create the category selector based on the attachment
    newView->SetCategorySelector(*new CategorySelector(*definitionModel, m_name.c_str()));
    SetCategories(*newView, SyncInfo::LevelExternalSourceAspect::Type::Spatial);

    newView->SetIsPrivate(true);
    newView->GetDisplayStyle().SetIsPrivate(true);
    newView->GetCategorySelector().SetIsPrivate(true);
    return newView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawingViewHelper::SetViewGeometry(DrawingViewDefinitionR viewDef)
    {
    DRange3d sectionRange;
    if (BSISUCCESS == m_converter.GetRangeFromSectionView(sectionRange, m_v8DgnAttachment, m_sheetModelMapping, m_isFromProxyGraphics))
        {
        // If this is a view of a generated drawing, then get the the range from the NamedView that drives the generation algorithm
        viewDef.SetOrigin(sectionRange.low);
        viewDef.SetExtents(DVec3d::FromStartEnd(sectionRange.low, sectionRange.high));
        return;
        }

    DRange3d viewedModelRange = m_drawingModel.QueryElementsRange();

    if (viewedModelRange.IsEmpty() || viewedModelRange.IsNull() || viewedModelRange.IsPoint())
        return;

    RotMatrix rot = (RotMatrixCR)m_v8DgnAttachment.GetRotMatrix();
    viewDef.SetRotation(rot);
    viewDef.LookAtVolume(viewedModelRange);

    double skew;
    if (m_v8DgnAttachment.HasViewHandler() && m_v8DgnAttachment.GetDynamicViewSettingsCR().GetViewHandler().GetAspectRatioSkew(m_v8DgnAttachment.GetDynamicViewSettingsCR(), skew))
        viewDef.SetAspectRatioSkew(skew);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewHelper::SetViewGeometry(SpatialViewDefinitionR spatialView)
    {
    DRange3d viewedModelRange = DRange3d::NullRange();
    for (auto modelid : spatialView.GetModelSelector().GetModels())
        {
        auto model = GetDgnDb().Models().Get<SpatialModel>(modelid);
        if (model.IsValid())
            viewedModelRange.Extend(model->QueryElementsRange());
        }

    if (viewedModelRange.IsEmpty() || viewedModelRange.IsNull() || viewedModelRange.IsPoint())
        return;

    RotMatrix rot = (RotMatrixCR)m_v8DgnAttachment.GetRotMatrix();
    spatialView.SetRotation(rot);
    spatialView.LookAtVolume(viewedModelRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetAttachmentViewHelper::SetCategories(ViewDefinitionR viewDef, SyncInfo::LevelExternalSourceAspect::Type ltype)
    {
    DgnV8ViewInfoCP viewInfo = m_v8SheetViewInfo;

    DgnV8Api::ViewInfoPtr fakeViewInfo;
    if (nullptr == viewInfo)
        {
        //  If we don't have pre-existing V8 view of the sheet from which to get get the levelmask tree entry for this attachment,
        //  then we need to invoke the V8 logic to create a levelmasktree from the "global" on/off status of the V8 levels. 
        //  To do that, need to create a temporary ViewInfo which points to the V8 sheet model as its root model.
        fakeViewInfo = m_converter.CreateV8ViewInfo(m_sheetModelMapping.GetV8Model(), Bentley::DRange3d::NullRange());
        fakeViewInfo->EnsureLevelMaskCoverage();
        LevelMaskTreeP levelMasks = fakeViewInfo->GetLevelMasksP();
        if (levelMasks)
            {
            levelMasks->ResetEffectiveMasks();
            levelMasks->ResynchLevelMask(&m_sheetModelMapping.GetV8Model(), 0, nullptr, false);
            }
        viewInfo = fakeViewInfo.get();
        }

    // Use the levelmask for this attachment as per the sheet view's levelmask tree.
    //  TRICKY: Don't allow ConvertAttachmentLevelMask to generate new SubCategories in order to handle level appearance inconsistencies. 
    //          It is too late for that. Elements have already been converted and assigned their categories. We only create subcategories
    //          when we convert levels in the early phases of the conversion, before we convert elements, and only when we are processing
    //          the main root (physical) reference attachment hierarchy.
    viewDef.GetCategorySelector().GetCategoriesR().clear();
    auto copyLevels = m_converter._GetParams().GetCopyLevel();
    m_converter._GetParamsR().SetCopyLevel(Converter::Params::CopyLevel::Never);

    m_converter.ConvertAttachmentLevelMask(viewDef, *viewInfo, m_v8DgnAttachment, ltype);
    
    m_converter._GetParamsR().SetCopyLevel(copyLevels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SheetAttachmentViewHelper::SheetAttachmentViewHelper(Converter& c, DgnAttachmentCR a, ResolvedModelMapping const& sm, DgnV8ViewInfoCP sv, ViewFactory& nvvf)
    : m_converter(c), m_v8DgnAttachment(a), m_sheetModelMapping(sm), m_v8SheetViewInfo(sv), m_nvvf(nvvf)
    {
    Utf8String baseName = m_converter.SheetsComputeViewAttachmentName(m_sheetModelMapping.GetDgnModel().GetModelId(), m_v8DgnAttachment);
    m_name = baseName;
    int iter=1;
    while (ViewDefinition::QueryViewId(*c.GetJobDefinitionModel(), m_name).IsValid())
        {
        m_name = Utf8PrintfString("%s (%d)", baseName.c_str(), iter++);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetAttachmentViewHelper::SetViewFlags(DisplayStyleR dstyle)
    {
    DgnV8Api::ViewFlags flags;
    if (nullptr != m_v8SheetViewInfo)
        flags = m_v8SheetViewInfo->GetViewFlags();  // by default, use the SHEET'S ViewFlags for the attached view
    else
        {
        memset(&flags, 0, sizeof(flags));
        /* flags.text_nodes = */ flags.fast_text = flags.on_off = flags.dimens = flags.fill = flags.transparency = 
            flags.line_wghts = flags.textureMaps = flags.patterns = true;
        flags.SetRenderMode (DgnV8Api::MSRenderMode::Wireframe);
        }

    // Almost always use the attachment's rendermode
    // If the attachment "UseViewFlagsInAttachment" flag is use, use the attachment's viewflags.
    DgnV8Api::ViewFlags refViewFlags = m_v8DgnAttachment.GetViewFlags(m_v8SheetViewInfo? m_v8SheetViewInfo->m_viewNum: 0);
    DgnV8Api::ViewContext::MergeViewFlagsFromRef(flags, refViewFlags, false, m_v8DgnAttachment.UseViewFlagsInAttachment());

    dstyle.SetViewFlags(m_converter.ConvertV8Flags(flags));

    // If the attachment has a displaystyle, that takes precedence over both the viewflags of the sheet and the attachment
    Bentley::DisplayStyleCP  refDisplayStyle = m_v8DgnAttachment.GetDisplayStyle();
    if (nullptr != refDisplayStyle)
        {
        refDisplayStyle->CopySettingsTo(flags);
        m_converter.ConvertDisplayStyle(dstyle, *refDisplayStyle);
        }

    // Attachment's background color
    auto backgroundColor = m_v8DgnAttachment.GetParent().GetDgnModelP()->GetModelInfo().GetBackgroundColor();
    dstyle.SetBackgroundColor(ColorDef(backgroundColor.red, backgroundColor.green, backgroundColor.blue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId Converter::SheetsGetViewForAttachment(bool isFromProxyGraphics, GeometricModelR geometricModel, DgnAttachmentCR v8DgnAttachment, 
                                                ResolvedModelMapping const& sheetModelMapping, DgnV8ViewInfoCP v8SheetView, ViewFactory& nvvf)
    {
    DgnViewId nvId;
    if (!isFromProxyGraphics)
        {
        DgnV8Api::NamedViewPtr nv = v8DgnAttachment.GetNamedView();
        if (nv.IsValid())
            {
            nvId = ViewDefinition::QueryViewId(*GetJobDefinitionModel(), Utf8String(nv->GetName().c_str()));
            if (!nvId.IsValid())
                nvId = ConvertNamedView(*nv, GetRootTrans(), nvvf);
            //  If the attachment is based entirely on a named view, then use that.
            if (nvId.IsValid() && !v8DgnAttachment.UseViewFlagsInAttachment())
/*<==*/         return nvId;
            }
        }

    // See if we already have a ViewDefinition for this V8 attachment
    IChangeDetector::SearchResults prov;
    DgnV8Api::EditElementHandle v8AttachmentEh(v8DgnAttachment.GetElementId(), v8DgnAttachment.GetParentModelRefP());
    ViewDefinitionPtr newView;
    IChangeDetector::T_SyncInfoElementFilter chooseViewElement = ElementFilters::GetViewDefinitionElementFiter();
    if (GetChangeDetector()._IsElementChanged(prov, *this, v8AttachmentEh, sheetModelMapping, &chooseViewElement))
        {
        // Either this is a new attachment, or it has changed. In either case, we need an up-to-date BIM ViewDefinition for it.
        if (geometricModel.IsSpatialModel())
            {
            BeAssert(!isFromProxyGraphics && "when we capture proxy graphics, we produce a DrawingModel, not a SpatialModel");
            SpatialViewHelper helper(*this, *geometricModel.ToSpatialModelP(), v8DgnAttachment, sheetModelMapping, v8SheetView, nvvf, nvId);
            if (nvId.IsValid()) // If the attachment is based partially on a named view, then make and modify a copy of it.
                newView = helper.CreateModifiedCopyOfNamedView(nvId);
            else
                newView = helper.CreateView();
            }
        else if (geometricModel.IsDrawingModel())
            {
            DrawingViewHelper helper(*this, isFromProxyGraphics, *geometricModel.ToDrawingModelP(), v8DgnAttachment, sheetModelMapping, v8SheetView, nvvf);
            if (nvId.IsValid()) // If the attachment is based partially on a named view, then make and modify a copy of it.
                newView = helper.CreateModifiedCopyOfNamedView(nvId);
            else
                newView = helper.CreateView();
            }
        else
            {
            ReportIssueV(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::InvalidSheetAttachment(), 
                                        Converter::IssueReporter::FmtFileBaseName(*sheetModelMapping.GetV8Model().GetDgnFileP()).c_str(),
                                        Converter::IssueReporter::FmtModel(sheetModelMapping.GetV8Model()).c_str(),
                                        Converter::IssueReporter::FmtAttachment(v8DgnAttachment).c_str());
            }
        }
    
    // Note that a "newView" is in fact a bunch of elements, each of which must be updated separately.
    if ((prov.m_changeType == IChangeDetector::ChangeType::Update) && newView.IsValid())
        UpdateViewChildren(*newView, DgnViewId(prov.GetExistingElementId().GetValue()));

    // Update the BIM as needed (or if needed) for the ViewDefinition element itself.
    ElementConversionResults results;
    results.m_element = newView.get();  // (results.m_element is a Ptr that adds a ref to the newView element)
    ProcessConversionResults(results, prov, v8AttachmentEh, sheetModelMapping);

    if (!results.m_mapping.IsValid())
        return DgnViewId();
    return DgnViewId(results.m_mapping.GetElementId().GetValue());
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

