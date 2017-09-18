/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ConvertProxyGraphics.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <VersionedDgnV8Api/DgnPlatform/IUnfoldableProfile.h>
#include <VersionedDgnV8Api/DgnPlatform/CVEHandler.h>
#include <VersionedDgnV8Api/DgnPlatform/DetailingSymbol/DetailingSymbolHandlers.h>
#include <VersionedDgnV8Api/VisEdgesLib/VisEdgesLib.h>

#include <VersionedDgnV8Api/DgnPlatform/DgnLinks.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnLinkManager.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnECManager.h>

#define DETAILINGSYMBOLS_TEMP_TABLE "temp.DetailingSymbols"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAttachmentArrayP Converter::GetAttachments(DgnV8ModelRefR v8Model)
    {
    DgnAttachmentArrayP attachments = v8Model.GetDgnAttachmentsP();
    if (nullptr != attachments)
        return attachments;

    DgnV8Api::DgnAttachmentLoadOptions loadOptions;
    loadOptions.SetTopLevelModel(true);
    loadOptions.SetShowProgressMeter(false); // turn this off for now. It seems to increment the task count for every ref, but it doesn't decrement the count afterward.
    if (v8Model.AreAttachmentsLoaded(loadOptions))
        return nullptr;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("Loading attachments for %s", v8Model.AsDgnAttachmentCP()? IssueReporter::FmtAttachment(*v8Model.AsDgnAttachmentCP()).c_str(): IssueReporter::FmtModel(*v8Model.GetDgnModelP()).c_str());

    v8Model.ReadAndLoadDgnAttachments(loadOptions);

    return v8Model.GetDgnAttachmentsP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::HasProxyGraphicsCache(DgnAttachmentR attachment)
    {
    ProxyDisplayCacheBaseP proxyCache;
    ProxyDgnAttachmentHandlerCP handler = attachment.FindProxyHandler(&proxyCache, nullptr);
    return (nullptr != handler) && (nullptr != proxyCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::HasProxyGraphicsCache(DgnV8ModelR v8Model)
    {
    auto attachments = GetAttachments(v8Model);

    if (nullptr == attachments)
        return false;

    for (DgnV8Api::DgnAttachment* attachment : *attachments)
        {
        if (HasProxyGraphicsCache(*attachment))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::GetExtractionCategoryId(V8NamedViewType vt)
    {
    Utf8CP catName = CATEGORY_NAME_ExtractedGraphics;
    switch (vt)
        {
        case V8NamedViewType::Section:      catName = CATEGORY_NAME_Section;   break;
        case V8NamedViewType::Plan:         catName = CATEGORY_NAME_Plan;      break;
        case V8NamedViewType::Elevation:    catName = CATEGORY_NAME_Elevation; break;
        case V8NamedViewType::Detail:       catName = CATEGORY_NAME_Detail;    break;
        }

    DefinitionModelR dictionary = GetDgnDb().GetDictionaryModel();
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(GetDgnDb(), DrawingCategory::CreateCode(dictionary, catName));
    if (categoryId.IsValid())
        return categoryId;

    categoryId = GetOrCreateDrawingCategoryId(dictionary, catName);
    GetOrCreateSubCategoryId(categoryId, SUBCATEGORY_NAME_Cut);
    GetOrCreateSubCategoryId(categoryId, SUBCATEGORY_NAME_InsideForward);
    GetOrCreateSubCategoryId(categoryId, SUBCATEGORY_NAME_InsideBackward);
    GetOrCreateSubCategoryId(categoryId, SUBCATEGORY_NAME_Outside);
    GetOrCreateSubCategoryId(categoryId, SUBCATEGORY_NAME_Inside);

    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::_GetExtractionCategoryId(DgnAttachmentCR attachment)
    {
    return GetExtractionCategoryId(GetV8NamedViewType(attachment));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_TurnOnExtractionCategories(CategorySelectorR selector)
    {
    // If this is a drawing, then any 3D attachment will be turned into proxy graphics, so turn all extraction categories on in that case.
    //  Note that this function is called when we are converting views. We might not have generated proxy graphics yet. So, don't base
    //  this decision on finding proxy graphics caches on the attachments.

    // Turn OFF the various 3-D categories/levels that were on in the V8 drawing view
    DgnCategoryIdSet& categories = selector.GetCategoriesR();
    bool isUncategorizedOn = categories.find(GetUncategorizedDrawingCategory()) != categories.end();

    DgnCategoryIdSet threed;
    for (auto catid : categories)
        {
        auto cat = DgnCategory::Get(GetDgnDb(), catid);
        if (nullptr == dynamic_cast<DrawingCategory const*>(cat.get()))
            threed.insert(catid);
        }
    for (auto badid : threed)
        categories.erase(badid);

    if (isUncategorizedOn)
        categories.insert(GetUncategorizedDrawingCategory());

    // Turn ON the extraction-related categories
    categories.insert(GetExtractionCategoryId(V8NamedViewType::Section));
    categories.insert(GetExtractionCategoryId(V8NamedViewType::Plan));
    categories.insert(GetExtractionCategoryId(V8NamedViewType::Elevation));
    categories.insert(GetExtractionCategoryId(V8NamedViewType::Detail));
    categories.insert(GetExtractionCategoryId(V8NamedViewType::Other));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId Converter::_GetExtractionSubCategoryId(DgnCategoryId categoryId, DgnV8Api::ClipVolumePass pass, DgnV8Api::ProxyGraphicsType gtype)
    {
    if (DgnV8Api::ClipVolumePass::None == pass || categoryId == DgnCategory::QueryCategoryId(GetDgnDb(), DrawingCategory::CreateCode(GetDgnDb().GetDictionaryModel(), CATEGORY_NAME_ExtractedGraphics)))
        return DgnCategory::GetDefaultSubCategoryId(categoryId);

    // *** WIP_CONVERT_CVE -- subcategory based on proxy graphics type??
    Utf8CP subCatName = nullptr;
    switch (pass)
        {
        case DgnV8Api::ClipVolumePass::Cut:             subCatName = SUBCATEGORY_NAME_Cut;             break;
        case DgnV8Api::ClipVolumePass::InsideForward:   subCatName = SUBCATEGORY_NAME_InsideForward;   break;
        case DgnV8Api::ClipVolumePass::InsideBackward:  subCatName = SUBCATEGORY_NAME_InsideBackward;  break;
        case DgnV8Api::ClipVolumePass::Outside:         subCatName = SUBCATEGORY_NAME_Outside;         break;
        case DgnV8Api::ClipVolumePass::Inside:          subCatName = SUBCATEGORY_NAME_Inside;          break;
        }

    if (nullptr == subCatName)
        {
        BeAssert(false && "new kind of section cut pass??");
        return DgnCategory::GetDefaultSubCategoryId(categoryId);
        }

    DgnSubCategoryId id = DgnSubCategory::QuerySubCategoryId(GetDgnDb(), DgnSubCategory::CreateCode(GetDgnDb(), categoryId, subCatName));
    if (!id.IsValid())
        {
        BeAssert(false && "should have created all section-related subcategories at the outset");
        id = DgnCategory::GetDefaultSubCategoryId(categoryId);
        }

    return id;
    }

namespace {

//=======================================================================================
//! Helper class for maintaining and querying the GraphicDerivedFromElement relationship
//! @see IElementGroup
//! @private
// @bsiclass                                                    Sam.Wilson      09/16
//=======================================================================================
struct GraphicDerivedFromElement : NonCopyableClass
{
public:
    static DgnDbStatus Insert(DrawingGraphicCR graphic, GeometricElement const& original);
    static DgnElementId QueryOriginal(DrawingGraphicCR graphic);
    static DgnElementIdSet QueryGraphics(GeometricElement const& original);
};

}

#define MUSTBEECSQLSUCCESS(STMT) if (BeSQLite::EC::ECSqlStatus::Success != STMT) {BeDataAssert(false); return DgnDbStatus::BadArg;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
DgnDbStatus GraphicDerivedFromElement::Insert(DrawingGraphicCR graphic, GeometricElement const& original)
    {
    return Converter::InsertLinkTableRelationship(graphic.GetDgnDb(), BIS_REL_DrawingGraphicRepresentsElement, graphic.GetElementId(), original.GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
DgnElementId GraphicDerivedFromElement::QueryOriginal(DrawingGraphicCR graphic)
    {
    auto statement = graphic.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_DrawingGraphicRepresentsElement) " WHERE SourceECInstanceId=?");

    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnElementId();
        }

    statement->BindId(1, graphic.GetElementId());

    DgnElementIdSet elementIdSet;
    if (BE_SQLITE_ROW == statement->Step())
        return statement->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
DgnElementIdSet GraphicDerivedFromElement::QueryGraphics(GeometricElement const& original)
    {
    auto statement = original.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_DrawingGraphicRepresentsElement) " WHERE TargetECInstanceId=?");

    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnElementIdSet();
        }

    statement->BindId(1, original.GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     02/2013
+===============+===============+===============+===============+===============+======*/
namespace {
struct CreateCveMeter : DgnV8Api::VisEdgesProgressMeter
{
    Converter&              m_converter;
    DgnProgressMeter::Abort m_aborted;

    CreateCveMeter(Converter& c) : m_converter(c), m_aborted(DgnProgressMeter::ABORT_No) {}

    virtual void            _SetTaskName (WCharCP taskName)  override       { m_aborted = m_converter.GetProgressMeter().ShowProgress(); }
    virtual void            _IndicateProgress (double fraction) override    { m_aborted = m_converter.GetProgressMeter().ShowProgress(); }
    virtual void            _Terminate () override                          {}
    virtual bool            _WasAborted () override                         { return DgnProgressMeter::ABORT_Yes == m_aborted; }
    virtual void            _DisplayMessageCenter (DgnV8Api::OutputMessagePriority priority, DgnV8Api::OutputMessageAlert openAlert, Bentley::WStringCR brief, Bentley::WStringCR detailed) override
        {
        Utf8String msg(brief.c_str());
        msg.append(" - ");
        msg.append(Utf8String(detailed.c_str()));

        Converter::IssueSeverity sev = (DgnV8Api::OutputMessagePriority::Error == priority || DgnV8Api::OutputMessagePriority::Fatal == priority)?
                                        Converter::IssueSeverity::Error: 
                                        (DgnV8Api::OutputMessagePriority::Warning == priority)?
                                        Converter::IssueSeverity::Warning:
                                        Converter::IssueSeverity::Info;

        m_converter.ReportIssue(sev, Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), msg.c_str());
        }
};  
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   createProxyCache (VisibleEdgeCacheR proxyCache, VisibleEdgeCalculationCacheCP calculationCache, DgnModelRefP modelRef, ViewportP viewport, CreateCveMeter* progressIndicator)
    {
    DynamicViewSettingsCP                         dvSettings;
    DgnV8Api::EditElementHandle                             clipElement;
    DgnV8Api::IUnfoldableProfileExtension*                  unfoldExtension;
    DgnV8Api::IUnfoldableProfileExtension::UnfoldedProfile  unfoldedProfile;


    if (NULL != viewport &&
        viewport->UseClipVolume (modelRef) &&
        NULL != modelRef->AsDgnAttachmentP() &&
        NULL != (dvSettings = viewport->GetDynamicViewSettings (modelRef)) &&
        SUCCESS == dvSettings->GetClipBoundElemHandle (clipElement, modelRef) &&
        NULL != (unfoldExtension = DgnV8Api::IUnfoldableProfileExtension::Cast (clipElement.GetHandler())) &&
        SUCCESS == unfoldExtension->_GetUnfoldedProfile (unfoldedProfile, clipElement))
        {
        return DgnV8Api::VisibleEdgesLib::CreateUnfoldedProfileProxyCache (proxyCache, *modelRef->AsDgnAttachmentP(), unfoldedProfile, *viewport, *dvSettings, progressIndicator);
        }
    
    return DgnV8Api::VisibleEdgesLib::CreateProxyCache (proxyCache, calculationCache, modelRef, viewport, progressIndicator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt generateCve (DgnAttachmentP refP, ViewportP viewport, CachedVisibleEdgeOptionsCP pInputOptions, CreateCveMeter* meter, bool setAlwaysValid)
    {
    DgnV8Api::VisibleEdgeCache*           currentCache;

    if (NULL == refP ||
        NULL == viewport)
        {
        BeAssert(false);
        return ERROR;
        }
    DgnModelRefP modelRef = refP;

    if (NULL != (currentCache = dynamic_cast <DgnV8Api::VisibleEdgeCache*> (refP->GetProxyCache())) && (NULL != viewport && currentCache->IsValidForViewport (*viewport)))
        return SUCCESS;

    //UstnViewport*   mstnVP = dynamic_cast <UstnViewport*> (viewport);
    //bool            doPreview = (NULL != mstnVP) && (NULL != mstnVP->GetWindow());

    BeAssert (refP->IsDisplayedInViewport (*viewport, true));

    if (NULL == refP->FindProxyHandler (NULL, viewport))
        {
        DgnV8Api::CachedVisibleEdgeOptions    options;

        // if we got options passed in, we use those.
        DgnV8Api::Tcb const* tcb = modelRef->GetDgnFileP()->GetPersistentTcb();
        if (NULL == pInputOptions)
            {
            if (NULL == currentCache)
                options = DgnV8Api::CachedVisibleEdgeOptions (*tcb, modelRef);
            else
                options = currentCache->GetOptions();
            }
        else
            {
            options = *pInputOptions;
            }

        DgnV8Api::VisibleEdgeCache*           proxyCache = DgnV8Api::VisibleEdgeCache::Create (modelRef, options);
        //CacheLoadPreviewContext*    previewContext = NULL;

        // when setAlwaysValid is true, neither the originating view nor the CurrentViewGroup is not useful.
        //if (!setAlwaysValid) 
        //    {
        //    proxyCache->SetOriginatingView (viewport->GetViewNumber());
        //    proxyCache->SetOriginatingViewGroup (getCurrentViewGroup());
        //    }

                                       
        //if (doPreview)
        //    {
        //    updateExceptReference (modelRef, viewport);
        //    proxyCache->SetLoadPreviewer (previewContext = new CacheLoadPreviewContext (*viewport, *proxyCache, NULL));       
        //    }
        //
        //ustnmdl_callRefProxyHooks (refP, REFPROXYEVENT_BeforeProxyCacheCalculation, ProxyCachingOption::Cached);

        //CompletionBarVisibleEdgesProgressMeter       progressMeter;

        StatusInt   status =createProxyCache (*proxyCache, NULL, modelRef, viewport, meter);

        //ustnmdl_callRefProxyHooks (refP, REFPROXYEVENT_AfterProxyCacheCalculation, ProxyCachingOption::Cached);

        //DELETE_AND_CLEAR (previewContext);

        //if ((SUCCESS != status) && doPreview)
        //    {
        //    reference_updateReferenceInView (modelRef, DRAW_MODE_Normal, viewport->GetViewNumber());  
        //    delete proxyCache;
        //    return status;
        //    }
        proxyCache->Resolve ();
       
        // Calculate the hash and find the newest element for the proxyCache we just created. Originally, we tried to do this while calculating hidden lines, but it
        //  wasn't reliable to do it that way because of the complexity of tiling, etc., that hidden line requires.
        // The "setAlwaysValid" flag is used when creating iModels. In that case, we create the cache from the actual elements, but later the elements are turned into XGraphics.
        //  The process of creating XGraphics sometimes invalidates the hash, and we would thus conclude that the cache wasn't valid. We want it to be considered always valid.
        if (!setAlwaysValid)
            proxyCache->ComputeHash (modelRef, viewport);
        else
            proxyCache->ClearElementModifiedTimes (false);
            

        // it's valid for the view we created it for. It might be valid for other views, depending on what levels, etc., they are displaying. We have to run the test.
        //UInt32  thisView = viewport->GetViewNumber();
        UInt32  thisView = 0; // *** we always use a fake view
        proxyCache->SetValidForView (thisView, true);

        refP->SetProxyCache (proxyCache, DgnV8Api::ProxyDgnAttachmentHandlerManager::GetManager().GetHandler (DgnV8Api::CachedVisibleEdgeHandlerId()));

        // write the proxy cache as XAttributes to the reference element.
        return proxyCache->Save (modelRef);

        // Note: It is up to the caller to make sure that the reference element itself is saved.
        }
    

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAttachmentP Converter::GetFirstNeedingCve(ResolvedModelMapping const& parentModel, ProxyGraphicsDetector& proxyDetector, bset<DgnV8Api::ElementId> const& ignoreList)
    {
    for (DgnAttachmentP attachment : *parentModel.GetV8Model().GetDgnAttachmentsP())
        {
        if (ignoreList.find(attachment->GetElementId()) != ignoreList.end())
            continue;

        DgnV8Api::EditElementHandle v8eh(attachment->GetElementId(), &parentModel.GetV8Model());
        if (!v8eh.IsValid())
            continue;
        ChangeDetector::SearchResults searchRes;
        if (!GetChangeDetector()._IsElementChanged(searchRes, *this, v8eh, parentModel))
            continue;
        if (!proxyDetector._UseProxyGraphicsFor(*attachment, *this))
            continue;
        if (!attachment->Is3d() || attachment->IsTemporary())
            {
            // DgnPlatform will only generate proxy graphics for attached 3d models.
            BeAssert(false && "the proxyDetector is confused if it thinks that it will get proxy graphics for an attached drawing or sheet.");
            continue;
            }
        if (HasProxyGraphicsCache(*attachment))
            continue;
        if (nullptr == attachment->GetDgnModelP())  // We can't generate proxy graphics unless we get to the original model's elements
            continue;
        return attachment;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_GenerateProxyGraphics(ResolvedModelMapping const& v8ModelMapping, DgnV8Api::Viewport& vp, ProxyGraphicsDetector& proxyDetector)
    {
    auto& v8Model = v8ModelMapping.GetV8Model();
    auto attachments = GetAttachments(v8Model);

    if (nullptr == attachments)
        return;

    BeAssert(v8Model.IsFilled(DgnV8Api::DgnModelSections::ControlElements));

    // NB: Don't iterate v8Model.GetDgnAttachmentsP. LOOK BELOW! We call v8Model.ReloadNestedDgnAttachments.
    //      After generating CVE for an attachment, you must re-start the search for attachments that need CVE.
    bset<DgnV8Api::ElementId> seen;
    bset<DgnV8Api::ElementId> failed;
    while (true)
        {
        GetAttachments(v8Model); // make sure attachments are loaded and caches are filled

        DgnAttachmentP attachment = GetFirstNeedingCve(v8ModelMapping, proxyDetector, failed);
        if (nullptr == attachment)
            break;

        if (!seen.insert(attachment->GetElementId()).second)
            {
            BeAssert(false && " generateCVE didn't seem to create CVE for an attachment, but it also didn't return error");
            break;
            }

        CreateCveMeter meter(*this);

        v8Model.SetReadOnly(false);
        if (SUCCESS != generateCve(attachment, &vp, nullptr, &meter, true))
            {
            failed.insert(attachment->GetElementId());
            continue;
            }

        attachment->SetProxyCachingOption(DgnV8Api::ProxyCachingOption::Cached);
        //mdlRefFile_writeAttachmentExtended(&dgnAttachment, true, true, true);
        attachment->Rewrite(true, true);
        attachment->FixSelfReferenceAttachments(false);

        // TFS#735518 - once we have the proxygraphics, we don't care about nested attachments any more.
        //v8Model.ReloadNestedDgnAttachments(true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ProxyDisplayHitInfo const* Converter::GetProxyDisplayHitInfo(DgnV8Api::ViewContext& context)
    {
    auto hitInfo = (DgnV8Api::ProxyDisplayHitInfo*)context.GetDisplayStyleHandler()->GetViewHandlerHitInfo(nullptr, Bentley::DPoint3d::FromZero());
    if (nullptr != hitInfo)
        return hitInfo;

    BeAssert(false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::_UseProxyGraphicsFor(DgnAttachmentCR ref)
    {
    if (!ref.Is3d() || ref.IsTemporary())
        return false;

    // If we haven't mapped the V8 3D model into a BIM spatial model (because it wasn't found via the root spatial model), then
    // we should capture a picture of it as proxy graphics. ***NEEDS WORK: If this model was converted using a spatial reference transform, we won't find that.
    if ((nullptr != ref.GetDgnModelP()) && !FindFirstModelMappedTo(*ref.GetDgnModelP()).IsValid())
        return true;

    // In the general case, we only use proxy graphics on attachments that are based on saved views, which might 
    // represent section cuts or might have 3d clips.

    DgnV8Api::EditElementHandle viewEEH;
    if (!ref.GetNamedViewElement(viewEEH))
        return false;

    // *** WIP_SHEETS - how to detect the kinds of named views that result in generated/cut geometry?
    switch(GetV8NamedViewType(viewEEH))
        {
        case V8NamedViewType::Section:  
        case V8NamedViewType::Plan:     
        case V8NamedViewType::Elevation:
            return true;
        }

    auto nv = DgnV8Api::NamedView::Create(viewEEH);
    if (nv.IsValid())
        {
        // If the view is 3D and is clipped, then we have to use proxy graphics, as BIM 3D view clipping does not support
        // all of the dynamic-views-related features of Vancouver view clipping.
        ViewInfoCR viewInfo = nv->GetViewInfo();
        return (nullptr != viewInfo.GetDynamicViewSettings().GetClipBoundElementRef(ref.GetDgnModelP()))
            || (nullptr != viewInfo.GetDynamicViewSettings().GetClipMaskElementRef(ref.GetDgnModelP()));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Converter::V8NamedViewType Converter::GetV8NamedViewTypeOfFirstAttachment(DgnV8ModelR v8Model)
    {
    auto attachments = GetAttachments(v8Model);

    if (nullptr == attachments)
        return V8NamedViewType::None;

    for (DgnAttachmentP attachment : *attachments)
        {
        DgnV8Api::EditElementHandle viewEEH;
        if (!attachment->GetNamedViewElement(viewEEH))
            continue;

        auto vt = GetV8NamedViewType(viewEEH);
        if (V8NamedViewType::Other != vt)
            return vt;
        }

    return V8NamedViewType::None;
    }

/*
bim                         v8
DefnModel(1)                DgnModel(drawing)(1)
    Drawing(5)                  DgnAttachment(1)    -----------> DgnModel(design)(2)
    ^                                 ProxyGraphicsCache                               
    |(breaksdown)
DrawingModel(2)
    DrawingGraphic(1)                     PG1 ----------------->  DgnElement(21)
                                          PG2 ----------------->
                                          ...
    DrawingGraphic(2)                     PGn ----------------->   DgnElement(22)
                                          ...
                                DgnAttachment(2)    -----------> DgnModel(design)(3)
                                          ...                       ...
SpatialModel(3)
    SpatialElement(3)
    SpatialElement(4)


            syncinfo

            Model
            DrawingModel(1) <- DgnModel(drawing)(1)

            Element
            SpatialElement(3) <- DgnElement(21)
            SpatialElement(4) <- DgnElement(22)
            ...
            Drawing(5) <- DgnAttachment(1)
            Drawing(5) <- DgnAttachment(2)

            ExtractedGraphic
            DrawingGraphic(1) <- DgnModel(drawing)(1),SpatialElement(3)
            DrawingGraphic(2) <- DgnModel(drawing)(1),SpatialElement(4)


*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::_CreateAndInsertExtractionGraphic(ResolvedModelMapping const& drawingModelMapping, 
                                                         SyncInfo::V8ElementSource const& attachmentSource,
                                                         SyncInfo::V8ElementMapping const& originalElementMapping,
                                                         DgnCategoryId categoryId, GeometryBuilder& builder)
    {
    DgnModelR model = drawingModelMapping.GetDgnModel();

    DgnCode code;
    if (WantDebugCodes())
        {
        // *** WIP_CONVERT_CVE code = CreateCode(defaultCodeValue, defaultCodeScope);
        }

    // *** WIP_CONVERT_CVE - what bis element class to use? Will it always be the same for all kinds of drawing extractions?
    DgnClassId elementClassId(GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic));
    DgnElementPtr drawingGraphic = CreateNewElement(model, elementClassId, categoryId, code);
    if (!drawingGraphic.IsValid())
        return DgnDbStatus::BadRequest;

    if (!drawingGraphic.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    if (BSISUCCESS != builder.Finish(*drawingGraphic->ToGeometrySourceP()))
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }

    ShowProgress();
    ++m_elementsConverted;

    DgnDbStatus status = DgnDbStatus::Success;

    if (IsUpdating())
        {
        auto existingDrawingGraphicId = GetSyncInfo().FindExtractedGraphic(attachmentSource, originalElementMapping, categoryId);
        if (existingDrawingGraphicId.IsValid())
            {
            // We already have a graphic for this element. Update it.
            DgnElementCPtr existingEl = GetDgnDb().Elements().GetElement(existingDrawingGraphicId);

            if (existingEl.IsValid())
                {
            DgnElementPtr writeEl = MakeCopyForUpdate(*drawingGraphic, *existingEl);
            writeEl->Update(&status);
            // There's no need to "update" the syncinfo record for this drawing graphic, as syncinfo does not store anything like a date or a hash
            // for drawing graphics. (We track only the dgnattachment's hash as a proxy for *all* drawing graphics in the attachment.)
            return status;
            }
            else 
                {
                BeAssert (!"ExistingDrawingGraphic missing. Possibly previously deleted in ChangeDetector::_DetectDeletedModels.");
                }
            }
        }

    // This is a new graphic for this element.
    drawingGraphic->Insert(&status);
    if (DgnDbStatus::Success != status)
        return status;

    GetSyncInfo().InsertExtractedGraphic(attachmentSource, originalElementMapping, categoryId, drawingGraphic->GetElementId());

    //  Create a relationship to the 3d element that it was derived from. (If the relationship already exists, this will be a nop.)
    auto originalInBim = GetDgnDb().Elements().Get<GeometricElement>(originalElementMapping.m_elementId);
    if (originalInBim.IsValid())
        GraphicDerivedFromElement::Insert(*drawingGraphic->ToDrawingGraphic(), *originalInBim);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_DetectDeletedExtractionGraphics(ResolvedModelMapping const& v8DrawingModel, 
                                                 SyncInfo::T_V8ElementMapOfV8ElementSourceSet const& v8OriginalElementsSeen,
                                                 SyncInfo::T_V8ElementSourceSet const& unchangedV8attachments)
    {
    // 'v8OrignalElementsSeen' is the set of all V8 elements that were the source of proxy graphics 
    // that the caller saw when processing the proxy caches of the attachments to the specified V8 drawing model.
    // If we find a record in syncinfo that is linked to this V8 drawing model and also to an original element that is 
    // NOT in v8OriginalElementsSeen, then we know that the caller did not see its and we can conclude that source of the graphic
    // in V8 has disappeared. We should therefore delete the drawing graphic.

    bset<DgnElementId> drawingGraphicsToDelete;
    SyncInfo::T_V8ElementSourceSet attachmentsToDelete;
    SyncInfo::T_V8ElementMapOfV8ElementSourceSet recordsToDelete;

    SyncInfo::V8ElementSource attachmentSource;
    attachmentSource.m_v8ModelSyncInfoId = v8DrawingModel.GetV8ModelSyncInfoId();

    CachedStatementPtr stmt = nullptr;
    //                                          0      1                     2                         3
    m_dgndb->GetCachedStatement(stmt, "SELECT Graphic,AttachmentV8ElementId,OriginalV8ModelSyncInfoId,OriginalV8ElementId FROM " 
                                SYNCINFO_ATTACH(SYNC_TABLE_ExtractedGraphic) " WHERE DrawingV8ModelSyncInfoId=?");
    stmt->BindInt64(1, v8DrawingModel.GetV8ModelSyncInfoId().GetValue());
    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        DgnElementId graphic               = stmt->GetValueId<DgnElementId>(0);
        uint64_t attachmentV8ElementId     = stmt->GetValueInt64(1);
        uint64_t originalV8ModelSyncInfoId = stmt->GetValueInt64(2);
        uint64_t originalV8ElementId       = stmt->GetValueInt64(3);

        attachmentSource.m_v8ElementId = attachmentV8ElementId;

        if (unchangedV8attachments.find(attachmentSource) != unchangedV8attachments.end())
            continue;

        auto attachmentSeen = v8OriginalElementsSeen.find(attachmentSource);
        if (attachmentSeen == v8OriginalElementsSeen.end())
            {
            drawingGraphicsToDelete.insert(graphic);
            attachmentsToDelete.insert(attachmentSource);
            continue;
            }

        SyncInfo::T_V8ElementSourceSet const& originalsSeen = attachmentSeen->second;

        SyncInfo::V8ElementSource originalsSeenource(originalV8ElementId, SyncInfo::V8ModelSyncInfoId(originalV8ModelSyncInfoId));
        if (originalsSeen.find(originalsSeenource) == originalsSeen.end())
            {
            drawingGraphicsToDelete.insert(graphic);
            recordsToDelete[attachmentSource].insert(originalsSeenource);
            }
        }

    for (auto g : drawingGraphicsToDelete)
        {
        auto graphic = GetDgnDb().Elements().GetElement(g);
        if (graphic.IsValid())
            graphic->Delete();
        }
    for (auto const& byattachment : recordsToDelete)
        {
        auto const& attachment = byattachment.first;
        for (auto const& original: byattachment.second)
            {
            GetSyncInfo().DeleteExtractedGraphics(attachment, original);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Converter::V8NamedViewType Converter::GetV8NamedViewType(DgnV8Api::ElementHandle const& viewEEH)
    {
    auto vh = dynamic_cast<DgnV8Api::ViewElementHandler const*>(&viewEEH.GetHandler());
    if (nullptr == vh)
        {
        BeAssert(false);
        return V8NamedViewType::Other;
        }

    if (nullptr != dynamic_cast <DgnV8Api::SectionViewElementHandler const*>(vh))
        return V8NamedViewType::Section;
    if (nullptr != dynamic_cast <DgnV8Api::ElevationViewElementHandler const*>(vh))
        return V8NamedViewType::Elevation;
    if (nullptr != dynamic_cast <DgnV8Api::DetailViewElementHandler const*>(vh))
        return V8NamedViewType::Detail;
    if (nullptr != dynamic_cast <DgnV8Api::PlanViewElementHandler const*>(vh))
        return V8NamedViewType::Plan;
    return V8NamedViewType::Other;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Converter::V8NamedViewType Converter::GetV8NamedViewType(DgnAttachmentCR attachment)
    {
    DgnV8Api::EditElementHandle viewEEH;
    if (!attachment.GetNamedViewElement(viewEEH))
        {
        return V8NamedViewType::None;
        }

    return GetV8NamedViewType(viewEEH);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnECInstanceCreateOptionsCR getCreateOptions()
    {
    // Change requested by OpenRoads: they create fields pointing to properties of hidden elements. By default FindInstancesScope ignores hidden elements
    static DgnV8Api::DgnECInstanceCreateOptions s_opts;
    s_opts.SetExposesHidden(true);
    return s_opts;
    }

struct TargetDrawingBoundary
    {
    ResolvedModelMapping m_modelMapping;
    DgnV8Api::ElementId m_eid = 0;
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Sam.Wilson      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static TargetDrawingBoundary getTargetDrawingBoundary(DgnV8EhCR viewEH, Converter& cvt)
    {
    DgnV8Api::DgnLinkTreeSpecPtr treeSpec = DgnV8Api::DgnLinkManager::CreateTreeSpec (viewEH);
    DgnV8Api::DgnLinkTreePtr annotationTree = DgnV8Api::DgnLinkManager::GetManager ().ReadLinkTree (*treeSpec, false);
    if (!annotationTree.IsValid ())
        return TargetDrawingBoundary();

    DgnLinkTreeBranchCR root = annotationTree->GetRoot ();

    for (size_t ilink = 0; ilink < root.GetChildCount (); ilink++)
        {
        DgnLinkTreeLeafCP leafRef = dynamic_cast<Bentley::DgnLinkTreeLeafCP>(root.GetChildCP (ilink));
        Bentley::DgnRegionLinkCP regionLink = dynamic_cast<Bentley::DgnRegionLinkCP>(leafRef->GetLinkCP ());

        if (0 != regionLink->GetTargetType().CompareToI(DGNLINK_REGIONTYPE_Drawing))
            continue;

        //if (RegionLinkProcessor::HasDetailSourceReferenceUserData(*m_regionLink))
        //    continue;

        /*
        wprintf(L"[%s] [%s]\n", m_regionLink->GetTargetType().c_str(),
                            m_regionLink->GetTargetName().c_str());
                            */

        // Let the link figure out the target.
        DgnV8Api::FindInstancesScopePtr   scope;
        ECQueryPtr              query;
        if (regionLink->GetECTarget (scope, query, nullptr, getCreateOptions()))
            {
            DgnV8Api::DgnECInstanceIterable targetInstances = DgnV8Api::DgnECManager::GetManager().FindInstances (*scope, *query);
            DgnV8Api::DgnECInstanceIterable::const_iterator targetInstance = targetInstances.begin ();
            if (targetInstance != targetInstances.end ())
                {
                DgnECInstanceP instance = const_cast <DgnECInstanceP>(*targetInstance);
                auto host = instance->GetInstanceHost();
                auto hostType = instance->GetHostType();
                if (host.IsElement())
                    {
                    //printf(" ==>> %s\n", Converter::IssueReporter::FmtElement(*host.GetElementHandle()).c_str());
                    TargetDrawingBoundary tdb;
                    auto dbEh = host.GetElementHandle();
                    if (dbEh != nullptr)
                        {
                        // Note: scope may be holding the V8 model open, and it may close and free all of its V8 ElementRefs when we return.
                        tdb.m_modelMapping = cvt.FindFirstModelMappedTo(*dbEh->GetDgnModelP());
                        tdb.m_eid = dbEh->GetElementId();
                        return tdb;
                        }
                    }

                //else if (host.IsModel())
                //    printf(" ==>> %s\n", Converter::IssueReporter::FmtModel(*host.GetModel()).c_str());
                }
            }


        /*
        auto mlink = m_regionLink->GetModelLinkCP();
        if (nullptr != mlink)
            {
            wprintf(L" -> %s\n", mlink->GetModelName().c_str());
            auto flink = mlink->GetFileLinkCP();
            if (nullptr != flink)
                wprintf(L"     -> %s\n", flink->GetMoniker().ResolveFileName().c_str());
            }
            */

        }

    return TargetDrawingBoundary();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::Register()
    {
    auto instance = new ConvertDetailingSymbolExtension();
    //  *** NB: Do not try to extend DetailingSymbolBaseHandler. This handler is not actually registered by V8. 
    // If we try to extend it, we end up extending Type2Handler
//    RegisterExtension(DgnV8Api::DetailingSymbolBaseHandler::GetInstance(), *instance);  
    //  *** Instead, we must extend each subclass of DetailingSymbolBaseHandler separately.
    RegisterExtension(DgnV8Api::TitleTextHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::DrawingBoundaryHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::UnderlineDrawingBoundaryHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::DrawingBoundaryProxyClipElementHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::SectionCalloutHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::OrthogonalSectionCalloutHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::ElevationCalloutHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::InteriorElevationCalloutHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::DetailCalloutHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::DetailCallout3DViewHandler::GetInstance(), *instance);
    RegisterExtension(DgnV8Api::PlanCalloutHandler::GetInstance(), *instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::_DetermineElementParams(DgnClassId& dbClass, DgnCode& dbCode, DgnCategoryId& dbCategory, DgnV8EhCR v8Eh, Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const& mm)
    {
    //  Background on V8 DetailingSymbols, specifically, "Callout" and "DrawingBoundary" elements.
    //  A section or detail:
    //      -- is a set of graphics,
    //      -- has a name/identifier,
    //      -- is placed on a sheet,
    //      -- is referenced by a callout on some other sheet.
    //  In low-level terms, a section or detail is defined by 3 things: 
    //      a) the drawing that contains the graphics, 
    //      b) a DgnAttachment that references that drawing into a sheet, and 
    //      c) a DrawingBoundary element in the same sheet that holds the name/identifier property of the section. 
    //  The referencing Callout contains a DgnLink to the DrawingBoundary element. This is how
    //  the V8 GUI allows the user to navigate from a callout on sheet A to the placed section or detail on sheet B.
    //  The DrawingBoundary itself points to the DgnAttachment that refers to the section or detail drawing.
    //
    //      SHEETA                      SHEETB            .----->   DRAWINGC
    //                                  DgnAttachment ---'          section or detail graphics
    //                                      ^
    //      Callout1                        |
    //         DgnLink--------------->  DrawingBoundary
    //                                      Name
    //                                      Identifier 
    //                                      DetailScale
    //
    //  Note that a Callout may actually have no DgnLink. That happens when the called out drawing does not exist or has not yet been placed on a sheet.
    //

    if (mm.GetDgnModel().Is3d())
        return;
    auto& handler = v8Eh.GetHandler();

    DgnV8Api::IDetailingSymbolPtr def = DgnV8Api::DetailingSymbolManager::CreateDetailingSymbol(v8Eh);
    if (!def.IsValid())
        {
        BeAssert(false && "I should be extending only detailing symbols");
        return;
        }

    Initialize(converter);       // make sure my temp tables are set up and my IFinishConversion callback is registered

    Utf8CP className = nullptr;

    switch (def->GetSymbolType())
        {
        case DetailingSymbolType::TitleText:
            className = GENERIC_CLASS_TitleText;
            break;

        case DetailingSymbolType::DrawingBoundary:
            className = GENERIC_CLASS_ViewAttachmentLabel;
            RecordDrawingBoundaryDependency(converter, v8Eh, *def, mm);
            break;

        default:
            switch (def->GetSymbolType())
                {
                case DetailingSymbolType::SectionCallout:   className = GENERIC_CLASS_SectionCallout;   break;
                case DetailingSymbolType::ElevationCallout: className = GENERIC_CLASS_ElevationCallout; break;
                case DetailingSymbolType::InteriorElevationCallout: className = GENERIC_CLASS_ElevationCallout; break;
                case DetailingSymbolType::PlanCallout:      className = GENERIC_CLASS_PlanCallout;      break;
                case DetailingSymbolType::DetailCallout:    className = GENERIC_CLASS_DetailCallout;    break;
                    // If we don't recognize the type, then convert to DrawingGraphic
                }

            if (nullptr != className)       // Iff we are going to convert the v8 callout to a BIS callout, then leave a note to relate that BIS callout to the target model.
                RecordCalloutDependency(converter, v8Eh, mm);
            else
                {
                BeDataAssert(false && "unknown detailing symbol type");
                }

            break;
        }
    
    if (nullptr != className)
        dbClass = converter.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      09/16
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::_ProcessResults(ElementConversionResults& results, DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm, Converter& converter)
    {
    if (!results.m_element.IsValid())
        {
        BeAssert(false);
        return;
        }

#ifdef WIP_DETAILING_SYMBOLS
    if (!results.m_childElements.empty())
        printf ("got here\n");
#endif

    DgnV8Api::IDetailingSymbolPtr def = DgnV8Api::DetailingSymbolManager::CreateDetailingSymbol(v8Eh);
    DgnV8Api::DrawingBoundaryDef const* drawingDef = dynamic_cast <DgnV8Api::DrawingBoundaryDef const*> (def.get());
    if (nullptr == drawingDef)
        return;

    if (results.m_element->GetElementClassId() != converter.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_ViewAttachmentLabel))
        {
        BeAssert(false);
        return;
        }

    Utf8String detailidStr(drawingDef->GetIdentifier().c_str());
    auto detailid = CodeSpec::CreateCode(GENERIC_CODESPEC_ViewAttachmentLabel, *v8mm.GetDgnModel().GetModeledElement(), detailidStr);
    results.m_element->SetCode(detailid);

    results.m_element->SetUserLabel(Utf8String(drawingDef->GetName().c_str()).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::_OnFinishConversion(Converter& converter)
    {
    if (true)
        {
        //  Create ViewAttachmentLabel -> ViewAttachment ECRelationships first. (The second loop below will use them.)
        auto dbounds = GetSelectStatement(converter.GetDgnDb(), DetType::DrawingBoundary);
        while (BE_SQLITE_ROW == dbounds->Step())
            RelateViewAttachmentLabelToViewAttachment(converter, *dbounds);
        }

    if (true)
        {
        //  Create Callout -> DrawingModel ECRelationships.
        auto callouts = GetSelectStatement(converter.GetDgnDb(), DetType::Callout);
        while (BE_SQLITE_ROW == callouts->Step())
            RelateCalloutToDrawing(converter, *callouts);

        //  Now delete the redundant callouts that are on drawings
        auto& db = converter.GetDgnDb();
        auto findCallout = db.GetPreparedECSqlStatement("SELECT callout.ECInstanceId, callout.Model.Id, callout.DrawingModel.Id FROM generic.Callout callout");
    
        bset<DgnModelId> modelsCalledOutFromSheets;
        bmap<DgnModelId, bvector<DgnElementId>> otherCallouts;
        while (BE_SQLITE_ROW == findCallout->Step())
            {
            auto calloutId = findCallout->GetValueId<DgnElementId>(0);
            auto calloutModelId = findCallout->GetValueId<DgnModelId>(1);
            auto calloutTargetModelId = findCallout->GetValueId<DgnModelId>(2);
            auto calloutHomeModel = db.Models().GetModel(calloutModelId);
            if (!calloutTargetModelId.IsValid())
                continue;

            if (calloutHomeModel.IsValid() && calloutHomeModel->IsSheetModel())
                modelsCalledOutFromSheets.insert(calloutTargetModelId);
            else
                otherCallouts[calloutTargetModelId].push_back(calloutId);
            }

        bvector<DgnElementId> redundantCallouts;
        for (auto const& otherCallout : otherCallouts)
            {
            if (modelsCalledOutFromSheets.find(otherCallout.first) != modelsCalledOutFromSheets.end())
                {
                for (auto redundantCallout : otherCallout.second)
                    redundantCallouts.push_back(redundantCallout);
                }
            }

        for (auto redundantCallout : redundantCallouts)
            {
            db.Elements().Delete(redundantCallout);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::Initialize(Converter& converter)
    {
    static bool s_initialized;
    if (s_initialized)
        return;
    s_initialized = true;

    DbResult result = converter.GetDgnDb().CreateTable(DETAILINGSYMBOLS_TEMP_TABLE, "SourceV8ModelSyncInfoId INT NOT NULL, SourceV8ElementId BIGINT NOT NULL, TargetV8ModelSyncInfoId INT, TargetV8ElementId BIGINT, DetType INT");
    BeAssert(result == BE_SQLITE_OK);
    UNUSED_VARIABLE(result);

    converter.AddFinisher(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
BeSQLite::CachedStatementPtr ConvertDetailingSymbolExtension::GetInsertStatement(DgnDbR db, DetType dtype)
    {
    auto stmt = db.GetCachedStatement("INSERT INTO " DETAILINGSYMBOLS_TEMP_TABLE " (SourceV8ModelSyncInfoId,SourceV8ElementId,TargetV8ModelSyncInfoId,TargetV8ElementId,DetType) VALUES(?,?,?,?,?)");
    stmt->BindInt(5, (int)dtype);
    return stmt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
BeSQLite::CachedStatementPtr ConvertDetailingSymbolExtension::GetSelectStatement(DgnDbR db, DetType dtype)
    {
    if (!db.TableExists(DETAILINGSYMBOLS_TEMP_TABLE))
        {
        BeAssert(false);
        return nullptr;
        }
    auto stmt = db.GetCachedStatement("SELECT SourceV8ModelSyncInfoId,SourceV8ElementId,TargetV8ModelSyncInfoId,TargetV8ElementId FROM " DETAILINGSYMBOLS_TEMP_TABLE " WHERE (DetType=?)");
    stmt->BindInt(1, (int)dtype);
    return stmt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::RecordCalloutDependency(Converter& converter, DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm)
    {
    //auto tdb = getTargetDrawingBoundary(v8Eh, converter);
    //if (!tdb.m_modelMapping.IsValid() || 0 == tdb.m_eid)
    //    return;

    auto stmt = GetInsertStatement(converter.GetDgnDb(), DetType::Callout);
    stmt->BindInt   (1, v8mm.GetV8ModelSyncInfoId().GetValue());
    stmt->BindInt64 (2, v8Eh.GetElementId());
    //stmt->BindInt   (3, tdb.m_modelMapping.GetV8ModelSyncInfoId().GetValue());
    //stmt->BindInt64 (4, tdb.m_eid);
    auto status = stmt->Step();
    BeAssert(BE_SQLITE_DONE == status);
    UNUSED_VARIABLE(status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::RecordDrawingBoundaryDependency(Converter& converter, DgnV8EhCR v8Eh, DgnV8Api::IDetailingSymbol& def, ResolvedModelMapping const& v8mm)
    {
    auto dbdef = dynamic_cast<DgnV8Api::DrawingBoundaryDef*>(&def);
    auto attachment = dbdef->GetAttachment();
    if (nullptr == attachment)
        return;

    auto attmm = converter.FindFirstModelMappedTo(*attachment->GetParent().GetDgnModelP());

    auto stmt = GetInsertStatement(converter.GetDgnDb(), DetType::DrawingBoundary);
    stmt->BindInt   (1, v8mm.GetV8ModelSyncInfoId().GetValue());
    stmt->BindInt64 (2, v8Eh.GetElementId());
    stmt->BindInt   (3, attmm.GetV8ModelSyncInfoId().GetValue());
    stmt->BindInt64 (4, attachment->GetElementId());
    auto status = stmt->Step();
    BeAssert(BE_SQLITE_DONE == status);
    UNUSED_VARIABLE(status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::RelateViewAttachmentLabelToViewAttachment(Converter& converter,BeSQLite::Statement& select)
    {
    // Read back what RecordDrawingBoundaryDependency stored:
    SyncInfo::V8ModelSyncInfoId drawingBoundaryModelId    (select.GetValueInt  (0));        // Source=DrawingBoundary
    DgnV8Api::ElementId         drawingBoundaryElementId = select.GetValueInt64(1);
    SyncInfo::V8ModelSyncInfoId attachmentParentModelId   (select.GetValueInt  (2));        // Target=DgnAttachment
    DgnV8Api::ElementId         attachmentElementId      = select.GetValueInt64(3);

    //  Find the ViewAttachmentLabel to which the DrawingBoundary was converted
    //  Note that I'm confident that there is only one syncinfo mapping for a given DrawingBoundary element, so there's no need for a filter
    auto drawingBoundaryElementMapping = converter.GetFirstElementBySyncInfoId(drawingBoundaryModelId, drawingBoundaryElementId, nullptr);  
    auto viewAttachmentLabelPersist = converter.GetDgnDb().Elements().GetElement(drawingBoundaryElementMapping.GetElementId());
    if (!viewAttachmentLabelPersist.IsValid())
        {
        BeAssert(false);
        return;
        }

    //  Find the ViewAttachment that was generated from the DgnAttachment
    //  Note that a given DgnAttachment (on a sheet) is mapped to several different elements. Therefore, I have to supply a filter to find the one that I want.
    IChangeDetector::T_SyncInfoElementFilter detectViewAttachment = 
        [](SyncInfo::ElementIterator::Entry const& entry, Converter& converter)
            {
            return converter.GetDgnDb().Elements().Get<Sheet::ViewAttachment>(entry.GetElementId()).IsValid();
            };
    auto dgnAttachmentElementMapping = converter.GetFirstElementBySyncInfoId(attachmentParentModelId, attachmentElementId, &detectViewAttachment);
    auto viewAttachmentElementId = dgnAttachmentElementMapping.GetElementId();
    if (!viewAttachmentElementId.IsValid())
        {
        // Turns out we didn't create a ViewAttachment for this DgnAttachment. So, the callout is a broken link. Nothing we can do about that.
        return;
        }

    // Relate the viewAttachmentLabel to the viewAttachment
    auto viewAttachmentLabel = viewAttachmentLabelPersist->CopyForEdit();
    auto status = viewAttachmentLabel->SetPropertyValue(GENERIC_ViewAttachmentLabel_ViewAttachment, viewAttachmentElementId);
    BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
    BeAssert(DgnDbStatus::Success == status);
    auto updateReturn = viewAttachmentLabel->Update();
    BeAssert(updateReturn.IsValid());
    UNUSED_VARIABLE(status);
    UNUSED_VARIABLE(updateReturn);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
void ConvertDetailingSymbolExtension::RelateCalloutToDrawing(Converter& converter, BeSQLite::Statement& select)
    {
    // Read back what RecordCalloutDependency stored:
    SyncInfo::V8ModelSyncInfoId v8CalloutModelId          (select.GetValueInt  (0));      // Source=Callout
    DgnV8Api::ElementId         v8CalloutElementId       = select.GetValueInt64(1);
    //SyncInfo::V8ModelSyncInfoId drawingBoundaryModelId    (select.GetValueInt  (2));      // Target=DrawingBoundary
    //DgnV8Api::ElementId         drawingBoundaryElementId = select.GetValueInt64(3);
    auto rmm = converter.FindResolvedModelMappingBySyncId(v8CalloutModelId);
    if (!rmm.IsValid())
        return;
    DgnV8Api::ElementHandle v8Eh(v8CalloutElementId, &rmm.GetV8Model());
    if (!v8Eh.IsValid())
        return;
    auto tdb = getTargetDrawingBoundary(v8Eh, converter);
    if (!tdb.m_modelMapping.IsValid() || 0 == tdb.m_eid)
        return;
    SyncInfo::V8ModelSyncInfoId drawingBoundaryModelId = tdb.m_modelMapping.GetV8ModelSyncInfoId();      // Target=DrawingBoundary
    DgnV8Api::ElementId         drawingBoundaryElementId = tdb.m_eid;

    //  Find the BIM Callout to which the V8 Callout was converted
    //  I'm confident that there is only one syncinfo mapping for a given V8 Callout element, so there's no need for a filter
    auto v8CalloutElementMapping = converter.GetFirstElementBySyncInfoId(v8CalloutModelId, v8CalloutElementId, nullptr);
    auto calloutPersist = converter.GetDgnDb().Elements().GetElement(v8CalloutElementMapping.GetElementId());
    if (!calloutPersist.IsValid())
        {
        BeAssert(false);
        return;
        }

#ifndef NDEBUG
    auto calloutClass = converter.GetDgnDb().Schemas().GetClass(GENERIC_DOMAIN_NAME, GENERIC_CLASS_Callout);
    BeAssert(calloutPersist->GetElementClass()->Is(calloutClass));
#endif

    //  Find the BIM ViewAttachmentLabel to which the DrawingBoundary was converted
    //  I'm confident that there is only one syncinfo mapping for a given DrawingBoundary element, so there's no need for a filter
    auto drawingBoundaryElementMapping = converter.GetFirstElementBySyncInfoId(drawingBoundaryModelId, drawingBoundaryElementId, nullptr);
    auto viewAttachmentLabel = converter.GetDgnDb().Elements().GetElement(drawingBoundaryElementMapping.GetElementId());
    if (!viewAttachmentLabel.IsValid())
        return;

    //  What we really want is the DrawingModel that is the ultimate target of the associated ViewAttachment
    auto viewAttachmentElementId = viewAttachmentLabel->GetPropertyValueId<DgnElementId>(GENERIC_ViewAttachmentLabel_ViewAttachment);
    auto viewAttachment = converter.GetDgnDb().Elements().Get<Sheet::ViewAttachment>(viewAttachmentElementId);
    if (!viewAttachment.IsValid())
        {
        // We have a label that does not correspond to an attachment. It's just an orphan graphic now.
        return;
        }
    auto view = converter.GetDgnDb().Elements().Get<ViewDefinition2d>(viewAttachment->GetAttachedViewId());
    if (!view.IsValid())
        {
        BeAssert(false);
        return;
        }
    auto drawingModelId = view->GetBaseModelId();
    if (!drawingModelId.IsValid())
        {
        BeAssert(false);
        return;
        }

    // Relate the Callout to the DrawingModel
    auto callout = calloutPersist->CopyForEdit();
    auto status = callout->SetPropertyValue(GENERIC_Callout_DrawingModel, DgnElementId(drawingModelId.GetValue()));
    BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
    BeAssert(DgnDbStatus::Success == status);
    auto updateReturn = callout->Update();
    BeAssert(updateReturn.IsValid());
    UNUSED_VARIABLE(status);
    UNUSED_VARIABLE(updateReturn);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
