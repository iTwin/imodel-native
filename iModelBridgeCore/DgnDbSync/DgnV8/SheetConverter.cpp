/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SheetViewFactory : ViewFactory
    {
    Converter& m_sheetConverter;

    ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) override;
    ViewDefinitionPtr _UpdateView(Converter& converter, ViewDefinitionParams const&, ViewDefinitionR existingViewDef) override;

    SheetViewFactory(Converter& s) : m_sheetConverter(s) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSheets()
    {
    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_SHEETS());

    bmap<DgnV8FileP, bmultiset<ResolvedModelMapping>> sheets;
    int32_t count = 0;
    for (auto v8mm : m_v8ModelMappings)
        {
        if (!IsFileAssignedToBridge(*v8mm.GetV8Model().GetDgnFileP()))
            continue;

        if (v8mm.GetDgnModel().IsSheetModel())
            {
            ++count;
            sheets[v8mm.GetV8Model().GetDgnFileP()].insert(v8mm);
            }
        }


    AddTasks(count);

    SpatialViewFactory nvvf(*this);

    for (auto& v8FileGroup: sheets)
        {
        for (auto v8mm : v8FileGroup.second)
            {
            SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), v8mm.GetDgnModel().GetName().c_str());
        
            BeAssert(!v8mm.GetDgnModel().Is3d() && "sheets are NEVER converted to 3D models!");

            SheetsConvertModelAndViewsWithExceptionHandling(v8mm, nvvf);
            }
        
        if (_GetParams().GetPushIntermediateRevisions() == iModelBridge::Params::PushIntermediateRevisions::ByFile)
            PushChangesForFile(*v8FileGroup.first, ConverterDataStrings::Sheets());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinitionPtr SheetViewFactory::_MakeView(Converter& converter, ViewDefinitionParams const& parms)
    {
    if (!parms.GetDgnModel().IsSheetModel())
        return nullptr;

    DefinitionModelPtr definitionModel = converter.GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return nullptr;

    parms.m_categories->AddCategory(converter.GetOrCreateDrawingCategoryId(*definitionModel, CATEGORY_NAME_Attachments));

    SheetViewDefinitionPtr view = new SheetViewDefinition(*definitionModel, parms.m_name, parms.GetDgnModel().GetModelId(), *parms.m_categories, *parms.m_dstyle->ToDisplayStyle2dP());

    parms.Apply(*view);

    converter.ConvertLevelMask(*view, parms.m_viewInfo, &parms.GetV8Model());

    return view;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
ViewDefinitionPtr SheetViewFactory::_UpdateView(Converter& converter, ViewDefinitionParams const& params, ViewDefinitionR existingViewDef)
    {
    ViewDefinitionPtr newDef = _MakeView(converter, params);
    if (!newDef.IsValid())
        return newDef;

    SheetViewDefinition* sheet = newDef->ToSheetViewP();

    DgnDbStatus stat;
    // need to update the DisplayStyle and CategorySelector.
    DisplayStyleR newDisplayStyle = sheet->GetDisplayStyle();
    newDisplayStyle.ForceElementIdForInsert(existingViewDef.GetDisplayStyleId());
    if (!existingViewDef.GetDisplayStyle().EqualState(newDisplayStyle))
        {
        newDisplayStyle.Update(&stat);
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }
    sheet->SetDisplayStyle(newDisplayStyle);

    // Update doesn't actually update the 'CategorySelector' list.  It deletes the old one and creates a new one.  This will cause a changeset to be created for every update, 
    // even if there were no changes.  Therefore, we have to manually determine if there were changes to the list of categories and only call Update if there were.
    CategorySelectorR newCategorySelector = sheet->GetCategorySelector();
    newCategorySelector.ForceElementIdForInsert(existingViewDef.GetCategorySelectorId());
    Json::Value newCategoryVal = newCategorySelector.ToJson();
    Json::Value oldCategoryVal = existingViewDef.GetCategorySelector().ToJson();
    if (newCategoryVal != oldCategoryVal)
        {
        newCategorySelector.Update(&stat);
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }
    sheet->SetCategorySelector(newCategorySelector);

    newDef->ForceElementIdForInsert(existingViewDef.GetElementId());
    return newDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DoConvertDrawingElementsInSheetModel(ResolvedModelMapping const& v8mm)
    {
    if (GetChangeDetector()._AreContentsOfModelUnChanged(*this, v8mm))
        return;

    v8mm.GetV8Model().FillSections(DgnV8Api::DgnModelSections::Model);

    DgnV8Api::PersistentElementRefList* controlElements = v8mm.GetV8Model().GetControlElementsP();
    if (nullptr != controlElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *controlElements)
            {
            DgnV8Api::EditElementHandle v8eh(v8Element);
            ElementConversionResults results;
            _ConvertControlElement(results, v8eh, v8mm);
            }
        }

    DgnV8Api::PersistentElementRefList* graphicElements = v8mm.GetV8Model().GetGraphicElementsP();
    if (nullptr != graphicElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
            {
            DgnV8Api::EditElementHandle v8eh(v8Element);
            _ConvertDrawingElement(v8eh, v8mm);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/1
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SheetsConvertModelAndViewsWithExceptionHandling(ResolvedModelMapping const& v8mm, ViewFactory& nvvf)
    {
    IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        SheetsConvertModelAndViews(v8mm, nvvf);
        }
    IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(ReportFailedModelConversion(v8mm))
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SheetsConvertModelAndViews(ResolvedModelMapping const& v8mm, ViewFactory& nvvf)
    {
    if (!v8mm.GetDgnModel().IsSheetModel())
        return;

    DgnV8ModelR v8model = v8mm.GetV8Model();

    v8model.FillSections(DgnV8Api::DgnModelSections::Model);

    uint32_t        preElementsConverted = m_elementsConverted;
    DoConvertDrawingElementsInSheetModel(v8mm);
    if (WasAborted())
        return;
    
    if (preElementsConverted == m_elementsConverted)
        m_unchangedModels.insert(v8mm.GetDgnModel().GetModelId());

    //  Now that we know levels and styles ...

    //  Convert all views of this model
    DgnV8Api::ViewInfoPtr firstViewInfo;
    for (DgnV8Api::ViewGroupPtr const& vg : v8model.GetDgnFileP()->GetViewGroups())
        {
        SheetViewFactory svf(*this);
        Convert2dViewsOf(firstViewInfo, *vg, v8model, svf); 
        }

    //  Now that we know the level masks for the attachments and we have a view of the sheet...

    SheetsConvertViewAttachments(v8mm, firstViewInfo.get(), nvvf);

    //v8model.Empty(); Since XDomains and bridges may need to cache elementRefs, v8Models shouldn't be freed prematurely. Thus, commenting this line out.
    }

struct ScaleVote
    {
    double scale=0.0; 
    int votes=0;
    ScaleVote() {}
    ScaleVote(double s, int v): scale(s), votes(v) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Converter::SheetsComputeScale(DgnV8ModelCR v8SheetModel)
    {
    // Note: the V8 annotation scale factor a big number. Normally, V8 uses this to scale small things like line styles and text up so that they look right in big sheets.
    auto sheetscale = v8SheetModel.GetModelInfo().GetAnnotationScaleFactor();
    if ((0 != BeNumerical::Compare(sheetscale, 1.0)) && (0 != BeNumerical::Compare(sheetscale, 0.0)))
        return sheetscale;  // The sheet's annotation scale property is what we call the sheet scale.

    // Look at the scales used by the 3D attachments 
    auto attachments = GetAttachments(const_cast<DgnV8ModelR>(v8SheetModel));
    if (nullptr == attachments)
        return 1.0;

    bvector<ScaleVote> votes;
    size_t totalVotesCast = 0;
    for (DgnV8Api::DgnAttachment* v8DgnAttachment : *attachments)
        {
        // We only care about scaled attachments. 
        // Only non-camera 3D attachments or attachments to drawings that contain 3D geometry (e.g., sections or just plain 3D attachments) are considered for spatial scaling.
        if (v8DgnAttachment->IsCameraOn())  
            continue;

        if (nullptr != v8DgnAttachment->GetDgnModelP())
            {
            if (!v8DgnAttachment->Is3d() && !DrawingHas3DAttachment(*v8DgnAttachment))
                continue;
            }
        else
            {
            if (!HasProxyGraphicsCache(*v8DgnAttachment, nullptr))
                continue;
            }

        //  Note that the scale factor on a V8 DgnAttachment is a small number (V8 uses this to scale the attached design down to the sheet).
        auto dscale = v8DgnAttachment->GetUserScaleFromRefToRoot(nullptr);
        if (0 == BeNumerical::Compare(dscale, 0.0))
            continue;

        dscale = 1.0 / dscale;  // state the scale as the denominator of the fraction

        auto iFound = std::find_if(votes.begin(), votes.end(), [&](ScaleVote const& v) {return 0 == BeNumerical::Compare(v.scale, dscale);});
        if (iFound != votes.end())
            iFound->votes++;
        else
            votes.push_back(ScaleVote(dscale, 1));

        ++totalVotesCast;
        }

    if (votes.size() == 0)
        return 1.0;

    if (votes.size() == 1)
        return votes.front().scale;     // It's unanimous

    // If there are many attachments, see which scale is used most often. That is, sort by #votes, from highest to lowest.
    std::sort(votes.begin(), votes.end(), [](ScaleVote const& lhs, ScaleVote const& rhs) {
        return lhs.votes > rhs.votes;
        });
    
    auto const& mostPopular = votes[0];
    // If one scale is used by two thirds or more of the attachments, then we can reasonably infer that that is the sheet's scale.
    if ((mostPopular.votes / (double)totalVotesCast) > 0.6)
        {
        return mostPopular.scale;
        }

    // If there's no clear favorite, then see if the scales are all related according to common practice
    double larger = votes[0].scale;
    double smaller = votes[1].scale;
    if (larger < smaller)
        std::swap(larger, smaller);
    double ratio = larger/smaller;
    if ((0 == BeNumerical::Compare(ratio, 2.0)) || (0 == BeNumerical::Compare(ratio, 3.0)) || (0 == BeNumerical::Compare(ratio, 4.0)))
        {
        // Looks like one is an expanded detail. Infer that the larger is the real scale.
        return larger;
        }

    // If there's no relationship among the scales, then conclude that this sheet is not scaled
    return 1.0;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
