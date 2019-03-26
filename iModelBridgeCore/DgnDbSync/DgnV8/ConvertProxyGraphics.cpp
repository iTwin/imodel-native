/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ConvertProxyGraphics.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    RealityMeshAttachmentConversion::ForceClassifierAttachmentLoad (v8Model);


    return v8Model.GetDgnAttachmentsP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::HasProxyGraphicsCache(DgnAttachmentR attachment, ViewportP vp)
    {
    ProxyDisplayCacheBaseP proxyCache;
    ProxyDgnAttachmentHandlerCP handler = attachment.FindProxyHandler(&proxyCache, nullptr);
    return (nullptr != handler) && (nullptr != proxyCache) && (nullptr == vp || proxyCache->IsValidForViewport(*vp));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::HasProxyGraphicsCache(DgnV8ModelR v8Model, ViewportP vp)
    {
    auto attachments = GetAttachments(v8Model);

    if (nullptr == attachments)
        return false;

    for (DgnV8Api::DgnAttachment* attachment : *attachments)
        {
        if (HasProxyGraphicsCache(*attachment, vp) ||
            IsSimpleWireframeAttachment(*attachment))
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

    DefinitionModelPtr definitionModel = GetJobDefinitionModel();
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(GetDgnDb(), DrawingCategory::CreateCode(*definitionModel, catName));
    if (categoryId.IsValid())
        return categoryId;

    categoryId = GetOrCreateDrawingCategoryId(*definitionModel, catName);
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
    if (DgnV8Api::ClipVolumePass::None == pass || categoryId == DgnCategory::QueryCategoryId(GetDgnDb(), DrawingCategory::CreateCode(*GetJobDefinitionModel(), CATEGORY_NAME_ExtractedGraphics)))
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::_CreateAndInsertExtractionGraphic(ResolvedModelMapping const& drawingModelMapping, 
                                                         SyncInfo::V8ElementExternalSourceAspect const& sectionedElementXsa, DgnV8Api::ElementHandle& sectionedV8Element,
                                                         DgnCategoryId categoryId, GeometryBuilder& builder, 
                                                         Utf8StringCR v8SectionedElementPath, Utf8StringCR attachmentInfo, ResolvedModelMapping const& rootParentModel)
    {
    // "sectionedV8Element" is the 3-D element that was sectioned/projected/rendered
    
    // Note that sectionedElementXsa may be invalid even if sectionedV8Element is valid. That happens if we did not actually convert the 3-D elements but only drawings that reference them.

    DgnModelR model = drawingModelMapping.GetDgnModel();

    DgnCode code;
    if (WantDebugCodes())
        {
        // *** WIP_CONVERT_CVE code = CreateCode(defaultCodeValue, defaultCodeScope);
        }

    DgnClassId elementClassId;
    V8ElementECContent ecContent;
    bool hasPrimaryInstance = false;
    bool hasSecondaryInstances = false;
    if (sectionedV8Element.IsValid()) // This is the 3-D element that was sectioned/projected/rendered
        {
        bool isNewElement = true;
        if (IsUpdating())
            {
            IChangeDetector::SearchResults changeInfo;
            if (GetChangeDetector()._IsElementChanged(changeInfo, *this, sectionedV8Element, drawingModelMapping) && IChangeDetector::ChangeType::Update == changeInfo.m_changeType)
                isNewElement = false;
            }
        GetECContentOfElement(ecContent, sectionedV8Element, drawingModelMapping, true);
        hasPrimaryInstance = ecContent.m_primaryV8Instance != nullptr;
        hasSecondaryInstances = !ecContent.m_secondaryV8Instances.empty();
        elementClassId = _ComputeElementClass(sectionedV8Element, ecContent, drawingModelMapping);
        }
    else
        {
#ifdef WIP_EXTERNAL_SOURCE_ASPECT // sectionedElementXsa will almost always be invalid in this case. If we want to report this issue, then the caller will have to pass in the ElementId
        // Issue a warning about missing sectioned element
        auto rlink = GetRepositoryLinkElement(drawingModelMapping.GetRepositoryLinkId());
        if (rlink.IsValid())
            {
            auto aspect = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspect(*rlink);
            ReportIssueV(IssueSeverity::Warning, IssueCategory::Unknown(), Issue::ExtractedGraphicMissingElement(), "", sectionedElementXsa.GetV8ElementId(), model.GetName().c_str(), aspect.GetFileName().c_str());
            }
#endif
        }

    if (!elementClassId.IsValid())
        elementClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);

    if (!GetDgnDb().Schemas().GetClass(elementClassId)->Is(GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic)))
        {
        elementClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
        if (hasPrimaryInstance)
            {
            ecContent.m_secondaryV8Instances.push_back(std::make_pair(ecContent.m_primaryV8Instance.get(), BisConversionRule::ToAspectOnly));
            hasPrimaryInstance = false;
            }
        }

    DgnElementPtr drawingGraphic = CreateNewElement(model, elementClassId, categoryId, code);
    if (!drawingGraphic.IsValid())
        return DgnDbStatus::BadRequest;

    if (!drawingGraphic.IsValid())
        {
        ReportIssueV(IssueSeverity::Error, IssueCategory::Unknown(), Issue::ExtractedGraphicCreationFailure(), "", model.GetName().c_str(), elementClassId.GetValue(), categoryId.GetValue(), code.GetValueUtf8().c_str());
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    if (BSISUCCESS != builder.Finish(*drawingGraphic->ToGeometrySourceP()))
        {
        if (sectionedElementXsa.IsValid())
            ReportIssueV(IssueSeverity::Error, IssueCategory::Unknown(), Issue::ExtractedGraphicBuildFailure(), "", sectionedElementXsa.GetV8ElementId(), model.GetName().c_str());
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }

    ShowProgress();
    ++m_elementsConverted;

    DgnDbStatus status = DgnDbStatus::Success;
    ElementConversionResults results;
    results.m_element = drawingGraphic;
    if (hasPrimaryInstance)
        {
        if (nullptr == m_elementConverter)
            m_elementConverter = new ElementConverter(*this);
        m_elementConverter->ConvertToElementItem(results, ecContent.m_primaryV8Instance.get(), &ecContent.m_elementConversionRule);

        Bentley::WString displayLabel;
        ecContent.m_primaryV8Instance->GetDisplayLabel(displayLabel);
        results.m_element->SetUserLabel(Utf8String(displayLabel.c_str()).c_str());

        if (ecContent.m_v8ElementType == V8ElementType::NamedGroup)
            OnNamedGroupConverted(sectionedV8Element, results.m_element->GetElementClassId());
        }
    //item or aspects only if there was an ECInstance on the element at all
    if (hasSecondaryInstances)
        {
        if (nullptr == m_elementAspectConverter)
            m_elementAspectConverter = new ElementAspectConverter(*this);
        SyncInfo::V8ElementExternalSourceAspect* nonConstXsa = const_cast<SyncInfo::V8ElementExternalSourceAspect*>(&sectionedElementXsa);
        if (BentleyApi::SUCCESS != m_elementAspectConverter->ConvertToAspects(nonConstXsa, results, ecContent.m_secondaryV8Instances))
            {
            //ReportIssueV(IssueSeverity::Error, IssueCategory::Unknown(), Issue::ExtractedGraphicBuildFailure(), "", sectionedElementXsa.m_v8ElementId, model.GetName().c_str());

            }
        }

    if (IsUpdating())
        {
        // See "How ProxyGraphics are tracked"
        auto existingDrawingGraphicId = SyncInfo::ProxyGraphicExternalSourceAspect::FindDrawingGraphic(rootParentModel.GetDgnModel(), v8SectionedElementPath, categoryId, elementClassId, GetDgnDb());
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
            }
        }

    // This is a new graphic for this element.

    // See "How ProxyGraphics are tracked"
    auto aspect = SyncInfo::ProxyGraphicExternalSourceAspect::CreateAspect(rootParentModel.GetDgnModel(), v8SectionedElementPath, attachmentInfo, GetDgnDb());
    aspect.AddAspect(*drawingGraphic);

    drawingGraphic->Insert(&status);
    if (DgnDbStatus::Success != status)
        return status;

    if (sectionedV8Element.IsValid())
        {
        BeSQLite::EC::ECInstanceKey bisElementKey = drawingGraphic->GetECInstanceKey();
        RepositoryLinkId fileId = GetRepositoryLinkId(*sectionedV8Element.GetDgnFileP());
        if (results.m_v8PrimaryInstance.IsValid())
            ECInstanceInfo::Insert(GetDgnDb(), fileId, results.m_v8PrimaryInstance, bisElementKey, true);

        for (bpair<V8ECInstanceKey, BECN::IECInstancePtr> const& v8SecondaryInstanceMapping : results.m_v8SecondaryInstanceMappings)
            {
            BECN::IECInstanceCR aspect = *v8SecondaryInstanceMapping.second;
            BeSQLite::EC::ECInstanceId aspectId;
            if (SUCCESS != BeSQLite::EC::ECInstanceId::FromString(aspectId, aspect.GetInstanceId().c_str()))
                {
                BeAssert(false && "Could not convert IECInstance's instance id to a BeSQLite::EC::ECInstanceId.");
                continue;
                }

            ECInstanceInfo::Insert(GetDgnDb(), fileId, v8SecondaryInstanceMapping.first, BeSQLite::EC::ECInstanceKey(aspect.GetClass().GetId(), aspectId), false);
            }
        }

    //  Create a relationship to the 3d element that it was derived from. (If the relationship already exists, this will be a nop.)
    if (sectionedElementXsa.IsValid())
        {
        auto originalInBim = GetDgnDb().Elements().Get<GeometricElement>(sectionedElementXsa.GetElementId());
        if (originalInBim.IsValid())
            GraphicDerivedFromElement::Insert(*drawingGraphic->ToDrawingGraphic(), *originalInBim);
        }

    return DgnDbStatus::Success;
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
                        tdb.m_modelMapping = cvt.FindFirstResolvedModelMapping(*dbEh->GetDgnModelP());
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
        {
        // If the application creates multiple converters in succession, then s_initialized will be true, but the tables won't exist
        if (converter.GetDgnDb().TableExists(DETAILINGSYMBOLS_TEMP_TABLE))
            return;
        }
    s_initialized = true;

    DbResult result = converter.GetDgnDb().CreateTable(DETAILINGSYMBOLS_TEMP_TABLE, "SourceModelId BIGINT NOT NULL, SourceV8ElementId BIGINT NOT NULL, TargetModelId BIGINT, TargetV8ElementId BIGINT, DetType INT");
    BeAssert(result == BE_SQLITE_OK);
    UNUSED_VARIABLE(result);

    converter.AddFinisher(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      02/17
//---------------------------------------------------------------------------------------
BeSQLite::CachedStatementPtr ConvertDetailingSymbolExtension::GetInsertStatement(DgnDbR db, DetType dtype)
    {
    auto stmt = db.GetCachedStatement("INSERT INTO " DETAILINGSYMBOLS_TEMP_TABLE " (SourceModelId,SourceV8ElementId,TargetModelId,TargetV8ElementId,DetType) VALUES(?,?,?,?,?)");
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
    auto stmt = db.GetCachedStatement("SELECT SourceModelId,SourceV8ElementId,TargetModelId,TargetV8ElementId FROM " DETAILINGSYMBOLS_TEMP_TABLE " WHERE (DetType=?)");
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
    stmt->BindId    (1, v8mm.GetDgnModel().GetModelId());
    stmt->BindInt64 (2, v8Eh.GetElementId());
    //stmt->BindInt   (3, tdb.m_modelMapping.GetiModelExternalSourceAspectID().GetValue());
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

    auto attmm = converter.FindFirstResolvedModelMapping(*attachment->GetParent().GetDgnModelP());

    auto stmt = GetInsertStatement(converter.GetDgnDb(), DetType::DrawingBoundary);
    stmt->BindId    (1, v8mm.GetDgnModel().GetModelId());
    stmt->BindInt64 (2, v8Eh.GetElementId());
    stmt->BindId    (3, attmm.GetDgnModel().GetModelId());
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
    auto                drawingBoundaryModelId   = select.GetValueId<DgnModelId>(0);         // Source=DrawingBoundary
    DgnV8Api::ElementId drawingBoundaryElementId = select.GetValueInt64(1);
    auto                attachmentParentModelId  = select.GetValueId<DgnModelId>(2);         // Target=DgnAttachment
    DgnV8Api::ElementId attachmentElementId      = select.GetValueInt64(3);

    //  Find the ViewAttachmentLabel to which the DrawingBoundary was converted
    //  Note that I'm confident that there is only one syncinfo mapping for a given DrawingBoundary element, so there's no need for a filter
    auto drawingBoundaryElementMapping = converter.FindFirstElementMappedTo(drawingBoundaryModelId, drawingBoundaryElementId, nullptr);  
    auto viewAttachmentLabelPersist = converter.GetDgnDb().Elements().GetElement(drawingBoundaryElementMapping.GetElementId());
    if (!viewAttachmentLabelPersist.IsValid())
        {
        BeAssert(false);
        return;
        }

    //  Find the ViewAttachment that was generated from the DgnAttachment
    //  Note that a given DgnAttachment (on a sheet) is mapped to several different elements. Therefore, I have to supply a filter to find the one that I want.
    IChangeDetector::T_SyncInfoElementFilter detectViewAttachment = 
        [](SyncInfo::V8ElementExternalSourceAspectIterator::Entry const& entry, Converter& converter)
            {
            return converter.GetDgnDb().Elements().Get<Sheet::ViewAttachment>(entry->GetElementId()).IsValid();
            };
    auto dgnAttachmentElementMapping = converter.FindFirstElementMappedTo(attachmentParentModelId, attachmentElementId, &detectViewAttachment);
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
    auto                    v8CalloutModelId        = select.GetValueId<DgnModelId>(0);      // Source=Callout
    DgnV8Api::ElementId     v8CalloutElementId      = select.GetValueInt64(1);
    //auto                  drawingBoundaryModelId  = select.GetValueId<DgnModelId>(2);      // Target=DrawingBoundary
    //DgnV8Api::ElementId   drawingBoundaryElementId = select.GetValueInt64(3);
    auto rmm = converter.FindResolvedModelMappingByModelId(v8CalloutModelId);
    if (!rmm.IsValid())
        return;
    DgnV8Api::ElementHandle v8Eh(v8CalloutElementId, &rmm.GetV8Model());
    if (!v8Eh.IsValid())
        return;
    auto tdb = getTargetDrawingBoundary(v8Eh, converter);
    if (!tdb.m_modelMapping.IsValid() || 0 == tdb.m_eid)
        return;
    auto                    drawingBoundaryModelId = tdb.m_modelMapping.GetDgnModel().GetModelId();      // Target=DrawingBoundary
    DgnV8Api::ElementId     drawingBoundaryElementId = tdb.m_eid;

    //  Find the BIM Callout to which the V8 Callout was converted
    //  I'm confident that there is only one syncinfo mapping for a given V8 Callout element, so there's no need for a filter
    auto v8CalloutElementMapping = converter.FindFirstElementMappedTo(v8CalloutModelId, v8CalloutElementId, nullptr);
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
    auto drawingBoundaryElementMapping = converter.FindFirstElementMappedTo(drawingBoundaryModelId, drawingBoundaryElementId, nullptr);
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

/*  Proxy Graphics

Background:

A proxy graphic is some part of a section or projection of a 3-D element. (Sectioning a single 3-D element could result in many separate proxy graphics.)
The 3-D element that was sectioned or projected is called the "v8SectionedElement" in this code.
The v8SectionedElement may *or may not* be converted by the V8Converter.
If it is converted, then the converter will create a relationship between it and the generated DrawingGraphic that captures its proxy graphics.
It will not be converted if it is not in a spatial model that is found by the converter starting from the root spatial model.
It will not be converted if does not actually exist any more in the source files.
In either case, the proxy graphics may still exist in the sheet or drawing's attachment element. And in that case, the converter will
convert the proxy graphics, without the originals.
Therefore, the code that converts proxy graphics must be careful not to assume that the v8SectionedElement corresponds to anything in the iModel.

In V8, the source of proxy graphics looks like this:
Sheet Model ---DgnAttachment#1--> Drawing Model --DgnAttachment(SectionView)#2--> 3D Design Model { 3D element#99 }
or
Sheet Model ---DgnAttachment#1--> Drawing Model --DgnAttachment(SectionView)#2--> 3D Design Model ---DgnAttachment#3---> 3D DesignModel 2 { 3D element#100 }
or
Sheet Model --DgnAttachment(SectionView)#6--------------------------------------> 3D Design Model { 3D element#99 }
or
Sheet Model --DgnAttachment(SectionView)#6--------------------------------------> 3D Design Model ---DgnAttachment#3---> 3D DesignModel 2 { 3D element#100 }

Note how the v8SectionedElement may be in a nested reference attachment. In fact the sheet may reference another sheet and/or the drawings may reference other drawings. There is no limit to the levels or combinations of nested references.
Note that there may or may not be a V8 Drawing Model. The V8 Sheet model may reference the 3D Design Model directly.

In an iModel, proxy graphics are captured as DrawingGraphic elements and are stored in Drawing models. Views of Drawing models are attached to Sheet models.
That is, reference attachments are collapsed.

"How ProxyGraphics are tracked"

Identity = path
The ProxyGraphicExternalSourceAspect Identity is the full path from the BIS Sheet to the v8SectionedElement. For example, in the first case described above,
the path would be: 1/2/99, where 1 and 2 are the ElementIds of the two DgnAttachments, and 99 is the ElementId of the sectioned V8 element in the 3D design model.
In the last case, the path wouild be: 6/3/100. 

Scope = Sheet
The Scope of a ProxyGraphicExternalSourceAspect is the BIS Sheet. 
Note in the examples above how the path begins at the (BIS) Sheet and then identifies the (V8) attachments
used to reach the final 3D element. This works whether or not there is a drawing model. 
Note that the BIS Sheet's own ModelExternalSourceAspect identifies the V8 Sheet model.


iModel                        DGN
------------------------      ----------------------
Sheet
^ | XSA Identifer-----------> SheetModel
| |                             DgnAttachment#1
| |                               DrawingModel?
| views:                            DgnAttachment#2
|   Drawing                           DesignModel
|       DrawingGraphic                  3DElement#3
+---Scope  XSA  Identifier --------------/


*/