/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DrawingConverter2.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#include <VersionedDgnV8Api/VisEdgesLib/VisEdgesLib.h>
#include <VersionedDgnV8Api/DgnPlatform/CVEHandler.h>
#include <DgnPlatform/AutoRestore.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

static BentleyApi::TransformCR DoInterop(Bentley::Transform const&source) { return (BentleyApi::TransformCR)source; }


typedef  bmap<DgnAttachmentCP, ResolvedModelMapping> T_AttachmentMap;
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     05/2018
+===============+===============+===============+===============+===============+======*/
struct MergeProxyGraphicsDrawGeom : public DgnV8Api::SimplifyViewDrawGeom
{
    typedef
        bmap<Bentley::DgnModelRefP,                            
            bmap<DgnV8Api::ElementId,                          
                bmap<DgnCategoryId, GeometryBuilderPtr>>>    T_BuilderMap;


    struct ModelRefInfo
        {
        DgnCategoryId                           m_categoryId;
        bool                                    m_hasChanged;
        bool                                    m_failed;
        SyncInfo::V8ElementMapping              m_attachElementMapping;
        ResolvedModelMapping                    m_modelMapping;

        ModelRefInfo() : m_hasChanged(false), m_failed(false) {}
        };

    typedef bmap<DgnModelRefP, ModelRefInfo>    T_ModelRefInfoMap;

    DEFINE_T_SUPER(SimplifyViewDrawGeom)

    T_BuilderMap                                m_builders;
    ResolvedModelMapping const&                 m_parentModelMapping;
    ModelRefInfo                                m_currentModelRefInfo;
    Converter&                                  m_converter;
    SyncInfo::T_V8ElementSourceSet              m_attachmentsUnchanged;
    T_AttachmentMap const&                      m_attachmentMap;
    T_ModelRefInfoMap                           m_modelRefInfoMap;
    bool                                        m_doClip = true;
    bool                                        m_processingProxy = false;
    bool                                        m_processingText = false;

protected:
    virtual bool        _DoClipping () const override {return !m_processingProxy; }         // Proxy is already clipped and clipping is expensive.
    virtual bool        _DoTextGeometry () const override {return true;}
    virtual bool        _DoSymbolGeometry () const override {return true;}
    virtual bool        _ProcessAsFacets (bool isPolyface) const override {return false; }
    virtual bool        _ProcessAsBody (bool isCurved) const override {return false; }
    virtual StatusInt   _ProcessSolidPrimitive (Bentley::ISolidPrimitiveCR primitive) override  { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessSurface (Bentley::MSBsplineSurfaceCR surface) override  { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessFacetSet (Bentley::PolyfaceQueryCR facets, bool filled) override { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessBody (Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments) override  { return Bentley::BSIERROR;}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessTextString(Bentley::TextStringCR v8Text)
    {
    return Bentley::ERROR; // Is there any reason we shouldn't always drop symbol text?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::StatusInt _ProcessCurveVector(Bentley::CurveVectorCR v8curves, bool isFilled) override
    {
    InitCurrentModel();
    DgnV8Api::ProxyDisplayHitInfo const*    proxyInfo = nullptr;
    DgnCategoryId                           categoryId;
    DgnSubCategoryId                        subCategoryId;
    auto                                    currentModelRef = m_context->GetCurrentModel();

    if (nullptr != m_context->GetDisplayStyleHandler() &&
        nullptr != (proxyInfo = m_converter.GetProxyDisplayHitInfo(*m_context)))
        {
        // Proxy Graphics...
        if (!m_currentModelRefInfo.m_hasChanged || m_currentModelRefInfo.m_failed)
            {
            m_attachmentsUnchanged.insert(m_currentModelRefInfo.m_attachElementMapping);
            return Bentley::SUCCESS;
            }

        categoryId = m_currentModelRefInfo.m_categoryId;
        subCategoryId = m_converter.GetExtractionSubCategoryId(m_currentModelRefInfo.m_categoryId, proxyInfo->m_viewHandlerPass.m_pass, proxyInfo->m_graphicsType);
        if (!subCategoryId.IsValid())
            {
            BeAssert(false);
            return Bentley::SUCCESS;
            }
        }
    else
        {
        auto&   v8Model = m_currentModelRefInfo.m_modelMapping.GetV8Model();
        auto    dgnFile  = v8Model.GetDgnFileP();
        auto    levelId = m_context->GetCurrentDisplayParams()->GetLevel();

        if (nullptr == dgnFile)
            BeAssert(false);
        else
            {
            categoryId =    m_converter.ConvertDrawingLevel(*dgnFile, levelId);
            subCategoryId = m_converter.ConvertLevelToSubCategory(dgnFile->GetLevelCacheR().GetLevel(levelId), v8Model, categoryId);
            }
        }

    m_converter.ShowProgress();

    Bentley::Transform  currentTransform = (nullptr == m_context->GetCurrLocalToFrustumTransformCP()) ? Bentley::Transform::FromIdentity() : *m_context->GetCurrLocalToFrustumTransformCP();

    //  Convert the CurveVector itself
    CurveVectorPtr bimcurves;
    Converter::ConvertCurveVector(bimcurves, v8curves, &m_parentModelMapping.GetTransform());   

    // Graphics are defined in 3-D coordinates. The ViewContext's "current transform" gets them into the parent V8 drawing or sheet model's coordinates.
    DgnAttachmentCP     currentAttachment = currentModelRef->AsDgnAttachmentCP();

    if (nullptr != currentAttachment && currentAttachment->IsCameraOn())
        {
        Bentley::DMap4d     parentMap, currentAndFromParent, composite;
        Bentley::Transform  fromParentTransform;

        currentAttachment->GetTransformFromParent (fromParentTransform, false);

        currentAndFromParent.InitFromTransform(Bentley::Transform::FromProduct(fromParentTransform, currentTransform), false);
        currentAttachment->GetMapToParent(parentMap, false);

        composite.InitProduct(parentMap, currentAndFromParent); 
        bimcurves = bimcurves->Clone ((DMatrix4dCR) composite.M0);          
        } 
    else
        {
        bimcurves->TransformInPlace(DoInterop(currentTransform));
        }

    // Flatten 3D -> 2D
    Transform   flattenTrans;
    flattenTrans.InitIdentity();
    flattenTrans.form3d[2][2] = 0.0;
    bimcurves->TransformInPlace(flattenTrans);

    // Convert to meters
    bimcurves->TransformInPlace(m_parentModelMapping.GetTransform());

    Render::GeometryParams params;

    m_converter.InitGeometryParams(params, *m_context->GetCurrentDisplayParams(), *m_context, false, m_parentModelMapping.GetV8ModelSource());
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);

    if (m_processingText)
        params.SetFillDisplay(FillDisplay::Always);

    auto& byattachment = m_builders[currentModelRef];
    auto  elementRef = m_context->GetCurrDisplayPath()->GetHeadElem();

    bmap<DgnCategoryId, GeometryBuilderPtr>& byelement = byattachment[nullptr == elementRef ? 0 : elementRef->GetElementId()];
    GeometryBuilderPtr& builder = byelement[categoryId];
    if (!builder.IsValid())
        {
        builder = GeometryBuilder::Create(m_parentModelMapping.GetDgnModel(), categoryId, DPoint2d::FromZero());
        if (!builder.IsValid())
            return Bentley::SUCCESS;
        }

    builder->Append(params);
    builder->Append(*GeometricPrimitive::Create(bimcurves), GeometryBuilder::CoordSystem::World);
    return Bentley::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawShape3d (int numPoints, Bentley::DPoint3dCP points, bool filled, Bentley::DPoint3dCP range) override
    {
    if (!filled)
        T_Super::_DrawLineString3d(numPoints, points, range);       // Avoid clipping with parasolid and potentially exposing facets if nonplanar (Civil profiles).
    else
        T_Super::_DrawShape3d(numPoints, points,filled, range);
    }
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InitCurrentModel()
    {
    auto  currentModelRef = m_context->GetCurrentModel();
    auto  foundInfo = m_modelRefInfoMap.find(currentModelRef);
    
    if (foundInfo != m_modelRefInfoMap.end())
        {
        m_currentModelRefInfo = foundInfo->second;
        return;
        }

    auto    attachment = currentModelRef->AsDgnAttachmentCP();
    ModelRefInfo info;

    if (nullptr == attachment)
        {
        info.m_modelMapping = m_parentModelMapping;
        m_modelRefInfoMap[currentModelRef] = info;
        m_currentModelRefInfo = info;
        return;
        }

    auto     found = m_attachmentMap.find(attachment);
    if (found == m_attachmentMap.end())
        {
        BeAssert(false);
        return;
        }
    
    info.m_modelMapping = found->second;

    if (m_processingProxy)
        info.m_categoryId = m_converter.GetExtractionCategoryId(*attachment);

    DgnV8Api::EditElementHandle         v8eh(attachment->GetElementId(), attachment->GetParent().GetDgnModelP());
    auto                                chooseDrawing = ElementFilters::GetDrawingElementFilter();
    IChangeDetector::SearchResults      syncInfoSearch;

    info.m_hasChanged = m_converter.GetChangeDetector()._IsElementChanged(syncInfoSearch, m_converter, v8eh, m_parentModelMapping, &chooseDrawing);
    if (!info.m_hasChanged)
        {
        // The existing V8 attachment is unchanged, and MergeDrawGraphicsConverter will skip it. Just to record the fact
        // that we have found a mapping to the BIM Drawing element.
        m_converter.GetChangeDetector()._OnElementSeen(m_converter, syncInfoSearch.GetExistingElementId());
        }
    else if (IChangeDetector::ChangeType::Update == syncInfoSearch.m_changeType)
        {
        // The existing V8 attachment has changed. At this stage of the update, we just update its provenance in syncinfo
        // AND record the fact that we've found a mapping to the BIM drawing element. We also tell MergeDrawGraphicsConverter to go ahead and
        // harvest the proxies. Later, the caller (ConvertExtractionAttachments), will update the elements in the DrawingModel from the harvested geometry.
        m_converter.UpdateMappingInSyncInfo(syncInfoSearch.GetExistingElementId(), v8eh, m_parentModelMapping);
        m_converter.GetChangeDetector()._OnElementSeen(m_converter, syncInfoSearch.GetExistingElementId());
        }
    else
        {
        ResolvedModelMappingWithElement newModel = ResolvedModelMappingWithElement(m_parentModelMapping, m_converter.RecordMappingInSyncInfo(m_parentModelMapping.GetDgnModel().GetModeledElementId(), v8eh, m_parentModelMapping));
        syncInfoSearch.m_v8ElementMapping = newModel.GetModeledElementMapping();
        }

    info.m_attachElementMapping = syncInfoSearch.m_v8ElementMapping;
    if (info.m_attachElementMapping.IsValid())
        info.m_failed = false;
    else
        {
        m_converter.ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(),
                    Utf8PrintfString("%s - MergeDrawGraphicsConverter DetectAttachment failed", Converter::IssueReporter::FmtAttachment(*attachment).c_str()).c_str());

        info.m_failed = true;
        }

    m_modelRefInfoMap[currentModelRef] = info;
    m_currentModelRefInfo = info;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateDrawingElements()
    {
    SyncInfo::T_V8ElementMapOfV8ElementSourceSet v8OriginalsSeen;

    for (auto& byModelRef : m_builders)
        {
        auto                    modelRef = byModelRef.first;
        auto&                   modelRefInfo = m_modelRefInfoMap[modelRef];

        for (auto& byElement : byModelRef.second)
            {
            SyncInfo::V8ElementMapping originalElementMapping = m_converter.FindFirstElementMappedTo(*modelRef->GetDgnModelP(), byElement.first);
            
            for (auto& bycategory : byElement.second)
                {
                DgnDbStatus     status;

                if (modelRefInfo.m_attachElementMapping.IsValid() && originalElementMapping.IsValid())
                    status = m_converter._CreateAndInsertExtractionGraphic(modelRefInfo.m_modelMapping, modelRefInfo.m_attachElementMapping, originalElementMapping, bycategory.first, *bycategory.second);
                else
                    status = m_converter.CreateDrawingElement(m_parentModelMapping.GetDgnModel(), bycategory.first, *bycategory.second);

                if (DgnDbStatus::Success != status)
                    {
#ifdef NEEDS_WORK
                    BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
                    ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "drawing extraction");
                    BeAssert(false);
#endif
                    }
                }
            }
        }

#ifdef NEEDS_WORK
    if (IsUpdating())
        _DetectDeletedExtractionGraphics(v8mm., v8OriginalsSeen, drawGeom.m_attachmentsUnchanged);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MergeProxyGraphicsDrawGeom(Converter& converter, ResolvedModelMapping const& parentModelMapping, T_AttachmentMap const& attachmentMap) 
                           : m_converter(converter), m_parentModelMapping(parentModelMapping), m_attachmentMap(attachmentMap)
    { 
    }

}; // MergeProxyGraphicsDrawGeom

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     05/2018
+===============+===============+===============+===============+===============+======*/
struct MergeDrawingContext : public DgnV8Api::NullContext
{
    DEFINE_T_SUPER(NullContext)

protected:

MergeProxyGraphicsDrawGeom&             m_output;
Converter&                              m_converter;

virtual bool _WantUndisplayed () { return false; }
virtual bool _HandleRefAsViewlet (DgnAttachmentCR thisRef) override  { return false; }


virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:

MergeDrawingContext(MergeProxyGraphicsDrawGeom& output, Converter& converter) : m_output (output), m_converter(converter)
    {
    m_setupScan = true;
    m_purpose = DgnV8Api::DrawPurpose::DgnDbConvert;
    m_wantMaterials = true; 

    SetBlockAsynchs (true);
    _SetupOutputs ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString (DgnV8Api::TextString const& text) override
    {
    // NOTE: When IElementGraphicsProcessor handles TextString we don't want to spew background/adornment geometry!
    text.GetGlyphSymbology(*GetCurrentDisplayParams());
    CookDisplayParams();

    double priority = GetDisplayPriority();

    AutoRestore<bool>   saveInText(&m_output.m_processingText, true);
    GetIDrawGeom().DrawTextString (text, text.Is3d() ? NULL : &priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSymbol (DgnV8Api::IDisplaySymbol* symbolDef, Bentley::Transform const* trans, Bentley::ClipPlaneSet* clipPlanes, bool ignoreColor, bool ignoreWeight) override
    {
    // Pass along any symbol that is drawn from _ExpandPatterns/_ExpandLineStyles, etc.
    m_output.ClipAndProcessSymbol (symbolDef, trans, clipPlanes, ignoreColor, ignoreWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
    {
    DgnV8Api::ElemDisplayParams tmpElParams(elParams);

    tmpElParams.Resolve (*this);
    elMatSymb.FromResolvedElemDisplayParams (tmpElParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawModelRef (DgnV8Api::DgnModelRef* baseModelRef, DgnV8Api::DgnModelRefList* includeList, bool useUpdateSequence, bool includeRefs) override
    {
    if (nullptr == baseModelRef->GetDgnModelP())
        return;

    AutoRestore <bool> saveInProxy(&m_output.m_processingProxy, nullptr != baseModelRef->AsDgnAttachmentCP() && nullptr != baseModelRef->AsDgnAttachmentCP()->FindProxyHandler(nullptr, GetViewport()));

    T_Super::_DrawModelRef(baseModelRef, includeList, useUpdateSequence, includeRefs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   _VisitElemHandle (DgnV8Api::ElementHandle const& inEl, bool checkRange, bool checkScanCriteria) override
    {
    // *** TRICKY: For drawings, we do not convert all levels ahead of time. We wait until we see which ones are used.
    //              That's how we tell which should be DrawingCategories instead of SpatialCategories. We must therefore
    //              visit complex children and ensure that their levels are converted.
    m_converter.ConvertLevels(inEl);

    return T_Super::_VisitElemHandle(inEl, checkRange, checkScanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Process(ViewportR viewport)
    {
    Attach (&viewport,  DgnV8Api::DrawPurpose::DgnDbConvert);
    InitScanRangeAndPolyhedron ();
    GetCurrentDisplayParams()->Init ();
    SetScanReturn();
    VisitAllModelElements (NULL, false /* update sequence */, true /* include refs */, false /* transients */);
    Detach ();
    }

}; // MergeDrawingContext

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DrawingViewFactory : ViewFactory
    {
    ResolvedModelMapping const& m_drawingModelMapping;
    DrawingViewFactory(ResolvedModelMapping const& v8mm) : m_drawingModelMapping(v8mm) {}

    ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) override;
    };

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     02/2013
+===============+===============+===============+===============+===============+======*/
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
                              
    if (SUCCESS ==  DgnV8Api::VisibleEdgesLib::CreateProxyCache (*proxyCache, NULL, modelRef, viewport, meter))
        {
        proxyCache->Resolve ();

        if (!setAlwaysValid)
            proxyCache->ComputeHash (modelRef, viewport);
        else
            proxyCache->ClearElementModifiedTimes (false);
        }


    // it's valid for the view we created it for. It might be valid for other views, depending on what levels, etc., they are displaying. We have to run the test.
    //UInt32  thisView = viewport->GetViewNumber();
    UInt32  thisView = 0; // *** we always use a fake view
    proxyCache->SetValidForView (thisView, true);

    refP->SetProxyCache (proxyCache, DgnV8Api::ProxyDgnAttachmentHandlerManager::GetManager().GetHandler (DgnV8Api::CachedVisibleEdgeHandlerId()));

    // write the proxy cache as XAttributes to the reference element.
    return proxyCache->Save (modelRef);

    // Note: It is up to the caller to make sure that the reference element itself is saved.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::_UseProxyGraphicsFor2(DgnAttachmentCR ref)
    {
    if (!ref.Is3d() || ref.IsTemporary() || IsSimpleWireframeAttachment(ref))
        return false;

    // Needs work -- need to to handle "rendered" attachments.
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CreateProxyGraphics (ResolvedModelMapping const& target, DgnV8ModelRefR v8ModelRef, ViewportR viewport, T_AttachmentMap& attachmentMap, TransformCR transformToParent)
    {
    auto            attachments = GetAttachments(v8ModelRef);

    if (nullptr != attachments)
        {
        for (auto& attachment : *attachments)
            {
            auto attachedV8Model = attachment->GetDgnModelP();

            if (attachedV8Model == nullptr)
                continue;

            if (nullptr == attachedV8Model)
                {
                BeAssert(false);
                ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), "reference file not found");
                return;
                }

            auto refTrans = ComputeAttachmentTransform(transformToParent, *attachment);

            ResolvedModelMapping mergedModelMapping = GetModelFromSyncInfo(*attachment, refTrans);
            if (!mergedModelMapping.IsValid() || mergedModelMapping.GetDgnModel().GetModelId() != target.GetDgnModel().GetModelId())
                {
                SyncInfo::V8ModelMapping refSyncInfoMapping;

                if (BSISUCCESS != GetSyncInfo().InsertModel(refSyncInfoMapping, target.GetDgnModel().GetModelId(), *attachedV8Model, refTrans))
                    {
                    BeAssert(false);
                    LOG.infov("DrawingRegisterModelattachmentMap %s -> %s failed - duplicate attachments??", IssueReporter::FmtModel(*attachedV8Model).c_str(), IssueReporter::FmtModel(target.GetDgnModel()).c_str());
                    return;
                    }
                mergedModelMapping = ResolvedModelMapping (target.GetDgnModel(), *attachedV8Model, refSyncInfoMapping, attachment);
                }
            attachmentMap.Insert(attachment, mergedModelMapping);
            if (_UseProxyGraphicsFor2 (*attachment) &&
                !HasProxyGraphicsCache (*attachment, &viewport))
                {
                CreateCveMeter meter(*this);

                attachedV8Model->SetReadOnly(false);
                if (SUCCESS != generateCve(attachment, &viewport, nullptr, &meter, true))
                    continue;

                attachment->SetProxyCachingOption(DgnV8Api::ProxyCachingOption::Cached);
                attachment->Rewrite(true, true);
                attachment->FixSelfReferenceAttachments(false);
                }
            else
                {
                CreateProxyGraphics (target, *attachment, viewport, attachmentMap, refTrans);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::MergeDrawingGraphics(ResolvedModelMapping const& v8mm, ViewportR viewport, T_AttachmentMap const& attachmentMap)
    {
    MergeProxyGraphicsDrawGeom      drawGeom(*this, v8mm, attachmentMap);
    MergeDrawingContext             mergeDrawingContext(drawGeom, *this);

    drawGeom.SetViewContext(&mergeDrawingContext);
    mergeDrawingContext.Process(viewport);
    drawGeom.CreateDrawingElements();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingsConvertModelAndViews2(ResolvedModelMapping const& v8mm)
    {
    if (!v8mm.GetDgnModel().IsDrawingModel())
        {
        BeAssert(false);
        return;
        }
    DgnV8ModelR v8ParentModel = v8mm.GetV8Model();

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8ParentModel).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    // NB: We must be sure that we have all of the attachments loaded and filled if we want to create proxy graphics. Don't leave
    //      that to chance. And don't assume that if v8ParentModel.GetDgnAttachmentsP() returns non-null then we have all the attachments.
    //      we might not have all nested refs loaded, and we might not have them filled. They must all be loaded and filled!
        {
        DgnV8Api::DgnAttachmentLoadOptions loadOptions;
        loadOptions.SetTopLevelModel(true);
        loadOptions.SetShowProgressMeter(false); // turn this off for now. It seems to increment the task count for every ref, but it doesn't decrement the count afterward.
        v8ParentModel.ReadAndLoadDgnAttachments(loadOptions);
        }

    DgnV8Api::ViewInfoPtr   firstViewInfo, fakeViewInfo;
    auto                    vg = FindFirstViewGroupShowing(v8ParentModel);

    if (vg.IsValid())
        {
        for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
            {
            if (vg->GetViewInfo(i).GetRootModelId() == v8ParentModel.GetModelId())
                {
                firstViewInfo = DgnV8Api::ViewInfo::CopyFrom(vg->GetViewInfo(i), true, true, true);
                break;
                }
            }
        }

    if (!firstViewInfo.IsValid())
        {
        Bentley::DRange3d sheetModelRange;

        v8ParentModel.GetRange(sheetModelRange);
        if (sheetModelRange.IsEmpty() || sheetModelRange.IsNull() || sheetModelRange.IsPoint())
            sheetModelRange = Bentley::DRange3d::FromMinMax(-1000000, 1000000);

        fakeViewInfo = CreateV8ViewInfo(v8ParentModel, sheetModelRange);
        firstViewInfo = fakeViewInfo.get();
        }

    struct MyViewport : DgnV8Api::NonVisibleViewport
        {
        MyViewport(DgnV8Api::ViewInfo& viewInfo) :  DgnV8Api::NonVisibleViewport(viewInfo) { m_viewNumber = 0; m_backgroundColor.m_int = 0xffffff; }
        };

    MyViewport fakeVp(*firstViewInfo);


    DgnV8ModelR v8model = v8mm.GetV8Model();

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8model).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    // TBD - Needs work -- Need to handle "rendered" (renderMode > HiddenLine) seperately if this drawing is attached to a sheet. Ick, 

    T_AttachmentMap attachmentMap;
    CreateProxyGraphics(v8mm, v8mm.GetV8Model(), fakeVp, attachmentMap, v8mm.GetTransform());
    MergeDrawingGraphics(v8mm, fakeVp, attachmentMap);

    //  Convert all views of this model
    for (DgnV8Api::ViewGroupPtr const& vg : v8model.GetDgnFileP()->GetViewGroups())
        {
        DrawingViewFactory svf(v8mm);
        Convert2dViewsOf(firstViewInfo, *vg, v8model, svf); 
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::CreateDrawingElement(DgnModelR model, DgnCategoryId categoryId, GeometryBuilder& builder)
    {
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

#ifdef NEEDS_WORK
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
#endif

    // This is a new graphic for this element.
    drawingGraphic->Insert(&status);
    if (DgnDbStatus::Success != status)
        return status;

#ifdef NEEDS_WORK
    GetSyncInfo().InsertElement();
#endif

    return DgnDbStatus::Success;

    }

END_DGNDBSYNC_DGNV8_NAMESPACE






















