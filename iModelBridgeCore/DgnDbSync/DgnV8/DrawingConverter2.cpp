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

        ModelRefInfo() : m_hasChanged(false), m_failed(false) {}
        };

    typedef bmap<DgnModelRefP, ModelRefInfo>    T_ModelRefInfoMap;

    DEFINE_T_SUPER(SimplifyViewDrawGeom)

    T_BuilderMap                                m_builders;
    ResolvedModelMapping const&                 m_parentModelMapping;
    Bentley::DgnModelRefP                       m_currentModelRef;
    ModelRefInfo                                m_currentModelRefInfo;
    Converter&                                  m_converter;
    SyncInfo::T_V8ElementSourceSet              m_attachmentsUnchanged;
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
Transform   GetCurrentTransform()
    {
    return (nullptr == m_context->GetCurrLocalToFrustumTransformCP()) ? Transform::FromIdentity() : DoInterop(*m_context->GetCurrLocalToFrustumTransformCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InitGeometryParams(Render::GeometryParams& params)
    {
    DgnV8Api::ProxyDisplayHitInfo const*    proxyInfo = nullptr;
    DgnCategoryId                           categoryId;
    DgnSubCategoryId                        subCategoryId;

    if (nullptr != m_context->GetDisplayStyleHandler() &&
        nullptr != (proxyInfo = m_converter.GetProxyDisplayHitInfo(*m_context)))
        {
        // Proxy Graphics...
        if (!m_currentModelRefInfo.m_hasChanged || m_currentModelRefInfo.m_failed)
            {
            m_attachmentsUnchanged.insert(m_currentModelRefInfo.m_attachElementMapping);
            return false;
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
        auto   v8Model = m_currentModelRef->GetDgnModelP();
        if (nullptr == v8Model || nullptr == v8Model->GetDgnFileP())
            {
            BeAssert(false);
            }
        else
            {
            auto    levelId = m_context->GetCurrentDisplayParams()->GetLevel();

            categoryId =    m_converter.ConvertDrawingLevel(*v8Model->GetDgnFileP(), levelId);
            subCategoryId = m_converter.ConvertLevelToSubCategory(v8Model->GetDgnFileP()->GetLevelCacheR().GetLevel(levelId), *v8Model, categoryId);
            }
        }

    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    m_converter.InitGeometryParams(params, *m_context->GetCurrentDisplayParams(), *m_context, false, m_parentModelMapping.GetV8ModelSource());

    if (m_processingText)
        {
        params.SetFillDisplay(FillDisplay::Always);
        params.SetFillColor(params.GetLineColor());
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawTextString (Bentley::TextStringCR v8Text, double* zDepth)  override
    {
    AutoRestore<bool>   saveInText(&m_processingText, true);

    InitCurrentModel();

    DgnAttachmentCP     currentAttachment = m_currentModelRef->AsDgnAttachmentCP();
    Transform           currentTransform = GetCurrentTransform();

    if (nullptr != currentAttachment && currentAttachment->IsCameraOn())
        return T_Super::_DrawTextString(v8Text, zDepth);

    Bentley::DVec3d         zVector, zAxis = Bentley::DVec3d::From(0.0, 0.0, 1.0);
    v8Text.GetRotMatrix().GetColumn(zVector, 2);
    currentTransform.MultiplyMatrixOnly(*((DPoint3dP) &zVector));

    if (!zVector.IsParallelTo(zAxis))
        return T_Super::_DrawTextString(v8Text, zDepth);

    m_converter.ShowProgress();

    Render::GeometryParams geometryParams;

    if (!InitGeometryParams(geometryParams) || nullptr == m_currentModelRef->GetDgnFileP())
        return T_Super::_DrawTextString(v8Text, zDepth);

    TextStringPtr textString;
    Converter::ConvertTextString(textString, v8Text, *m_currentModelRef->GetDgnFileP(), m_converter);
    
    // Flatten 3D -> 2D
    Transform   flattenTrans;
    flattenTrans.InitIdentity();
    flattenTrans.form3d[2][2] = 0.0;

    textString->ApplyTransform(Transform::FromProduct(flattenTrans, m_parentModelMapping.GetTransform(), currentTransform));

    auto primitive = GeometricPrimitive::Create(*textString);
    AddToBuilder(primitive, geometryParams);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::StatusInt _ProcessCurveVector(Bentley::CurveVectorCR v8curves, bool isFilled) override
    {
    m_converter.ShowProgress();

    InitCurrentModel();
    Render::GeometryParams geometryParams;

    if (!InitGeometryParams(geometryParams))
        return Bentley::SUCCESS;

    Bentley::Transform  currentTransform = (nullptr == m_context->GetCurrLocalToFrustumTransformCP()) ? Bentley::Transform::FromIdentity() : *m_context->GetCurrLocalToFrustumTransformCP();

    CurveVectorPtr bimcurves;
    Converter::ConvertCurveVector(bimcurves, v8curves, &m_parentModelMapping.GetTransform());   

    // Graphics are defined in 3-D coordinates. The ViewContext's "current transform" gets them into the parent V8 drawing or sheet model's coordinates.
    DgnAttachmentCP     currentAttachment = m_currentModelRef->AsDgnAttachmentCP();

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
    bimcurves->TransformInPlace(Transform::FromProduct(flattenTrans, m_parentModelMapping.GetTransform())); 

    auto    primitive = GeometricPrimitive::Create(bimcurves);
    return AddToBuilder(primitive, geometryParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::StatusInt  AddToBuilder(GeometricPrimitivePtr& primitive, Render::GeometryParams& geometryParams)
    {
    if (!primitive.IsValid())
        return Bentley::BSIERROR;

    auto&               byattachment = m_builders[m_currentModelRef];
    auto                elementRef = m_context->GetCurrDisplayPath()->GetHeadElem();
    auto&               byelement = byattachment[nullptr == elementRef ? 0 : elementRef->GetElementId()];
    GeometryBuilderPtr& builder = byelement[geometryParams.GetCategoryId()];
    if (!builder.IsValid())
        {
        builder = GeometryBuilder::Create(m_parentModelMapping.GetDgnModel(), geometryParams.GetCategoryId(), DPoint2d::FromZero());
        if (!builder.IsValid())
            return Bentley::SUCCESS;
        }

    builder->Append(geometryParams);
    builder->Append(*primitive, GeometryBuilder::CoordSystem::World);
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
    m_currentModelRef = m_context->GetCurrentModel();
    auto  foundInfo = m_modelRefInfoMap.find(m_currentModelRef);
    
    if (foundInfo != m_modelRefInfoMap.end())
        {
        m_currentModelRefInfo = foundInfo->second;
        return;
        }

    auto    attachment = m_currentModelRef->AsDgnAttachmentCP();
    ModelRefInfo info;

    if (nullptr == attachment)
        {
        m_modelRefInfoMap[m_currentModelRef] = info;
        m_currentModelRefInfo = info;
        return;
        }
    
    if (m_processingProxy)
        {
        DgnV8Api::DgnAttachment const* parentAttachment;
        while (nullptr != (parentAttachment = attachment->GetParentModelRefP()->AsDgnAttachmentCP()))
            attachment = parentAttachment;

        info.m_categoryId = m_converter.GetExtractionCategoryId(*attachment);
        }

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

    m_modelRefInfoMap[m_currentModelRef] = info;
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
            SyncInfo::V8ElementMapping originalElementMapping;
            
            if (nullptr != modelRef && nullptr != modelRef->GetDgnModelP())
                originalElementMapping = m_converter.FindFirstElementMappedTo(*modelRef->GetDgnModelP(), byElement.first);
            
            for (auto& bycategory : byElement.second)
                {
                DgnDbStatus     status;

                if (modelRefInfo.m_attachElementMapping.IsValid() && originalElementMapping.IsValid())
                    status = m_converter._CreateAndInsertExtractionGraphic(m_parentModelMapping, modelRefInfo.m_attachElementMapping, originalElementMapping, bycategory.first, *bycategory.second);
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
MergeProxyGraphicsDrawGeom(Converter& converter, ResolvedModelMapping const& parentModelMapping)
                           : m_converter(converter), m_parentModelMapping(parentModelMapping)
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
virtual Bentley::StatusInt       _OnNewModelRef (DgnModelRefP modelRef) override
    {
    Bentley::StatusInt   status = T_Super::_OnNewModelRef(modelRef);
    auto        viewFlags = *GetViewFlags();

    viewFlags.text_nodes = false;
    SetViewFlags(&viewFlags);

    return status;
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
void Process(ViewportR viewport, DgnV8Api::DgnModelRef* baseModelRef)
    {
    Attach (&viewport,  DgnV8Api::DrawPurpose::DgnDbConvert);
    InitScanRangeAndPolyhedron ();
    GetCurrentDisplayParams()->Init ();
    SetScanReturn();

    if (SUCCESS == PushModelRef(baseModelRef, true))
        _DrawModelRef(baseModelRef, nullptr, false, true);

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
void Converter::CreateProxyGraphics (DgnModelRefR modelRef, ViewportR viewport)
    {
    auto attachedV8Model = modelRef.GetDgnModelP();

    if (attachedV8Model == nullptr)
        return;

    DgnAttachmentP      attachment = modelRef.AsDgnAttachmentP();
    if (nullptr  != attachment)
        {
        if(_UseProxyGraphicsFor2(*attachment) &&
           !HasProxyGraphicsCache(*attachment, &viewport))
            {
            CreateCveMeter meter(*this);

            attachedV8Model->SetReadOnly(false);
            if (SUCCESS == generateCve(attachment, &viewport, nullptr, &meter, true))
                {
                attachment->SetProxyCachingOption(DgnV8Api::ProxyCachingOption::Cached);
                attachment->Rewrite(true, true);
                }
            }
        else
            {
            DgnAttachmentArrayP childAttachments = GetAttachments(modelRef);
            if (nullptr != childAttachments)
                for (auto& childAttachment : *childAttachments)
                    CreateProxyGraphics (*childAttachment, viewport);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::MergeDrawingGraphics(Bentley::DgnModelRefR baseModelRef, ResolvedModelMapping const& v8mm, ViewportR viewport)
    {
    MergeProxyGraphicsDrawGeom      drawGeom(*this, v8mm);
    MergeDrawingContext             mergeDrawingContext(drawGeom, *this);

    drawGeom.SetViewContext(&mergeDrawingContext);
    mergeDrawingContext.Process(viewport, &baseModelRef);
    drawGeom.CreateDrawingElements();
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8Api::ViewInfoPtr  createFakeViewInfo(DgnV8ModelR v8ParentModel, Converter& converter)
    {
    Bentley::DRange3d sheetModelRange;

    v8ParentModel.GetRange(sheetModelRange);
    if (sheetModelRange.IsEmpty() || sheetModelRange.IsNull() || sheetModelRange.IsPoint())
        sheetModelRange = Bentley::DRange3d::FromMinMax(-1000000, 1000000);

    return converter.CreateV8ViewInfo(v8ParentModel, sheetModelRange);
    }

struct FakeViewport : DgnV8Api::NonVisibleViewport
    {
    FakeViewport(DgnV8Api::ViewInfo& viewInfo) :  DgnV8Api::NonVisibleViewport(viewInfo) { m_viewNumber = 0; m_backgroundColor.m_int = 0xffffff; }
    };

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

    
    DgnV8Api::ViewInfo const*   foundViewInfo = nullptr;
    auto                        vg = FindFirstViewGroupShowing(v8ParentModel);
    if (vg.IsValid())
        {
        for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
            {
            if (vg->GetViewInfo(i).GetRootModelId() == v8ParentModel.GetModelId())
                {
                foundViewInfo = &vg->GetViewInfo(i);
                break;
                }
            }
        }

    DgnV8Api::ViewInfoPtr       viewInfo = (nullptr == foundViewInfo) ? createFakeViewInfo(v8ParentModel, *this):  DgnV8Api::ViewInfo::CopyFrom(*foundViewInfo, true, true, true);
    FakeViewport                fakeVp(*viewInfo);

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8ParentModel).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    // TBD - Needs work -- Need to handle "rendered" (renderMode > HiddenLine) seperately if this drawing is attached to a sheet. Ick, 

    CreateProxyGraphics(v8ParentModel, fakeVp);
    MergeDrawingGraphics(v8ParentModel, v8mm, fakeVp);

    //  Convert all views of this model
    for (DgnV8Api::ViewGroupPtr const& vg : v8ParentModel.GetDgnFileP()->GetViewGroups())
        {
        DrawingViewFactory svf(v8mm);
        Convert2dViewsOf(viewInfo, *vg, v8ParentModel, svf); 
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CreateSheetExtractionAttachments(ResolvedModelMapping const& v8SheetModelMapping, ProxyGraphicsDrawingFactory& drawingGenerator, Bentley::ViewInfoCP v8SheetView)
    {
    DgnV8ModelR             v8ParentModel = v8SheetModelMapping.GetV8Model();
    auto                    attachments = GetAttachments(v8ParentModel);

    if (nullptr == attachments)
        return;

    DgnV8Api::ViewInfoPtr   viewInfo = (nullptr == v8SheetView) ? createFakeViewInfo(v8ParentModel, *this):  DgnV8Api::ViewInfo::CopyFrom(*v8SheetView, true, true, true);
    FakeViewport            fakeVp(*viewInfo);
    
    for (auto attachment : *attachments)
        {
        if (attachment->Is3d())             // NEEDS_WORK -- rendered attachment.
            {
            auto                createdDrawing = drawingGenerator._CreateAndInsertDrawing(*attachment, v8SheetModelMapping, *this);

            createdDrawing.SetTransform(v8SheetModelMapping.GetTransform());
            CreateProxyGraphics(*attachment, fakeVp);
            MergeDrawingGraphics(*attachment, createdDrawing, fakeVp);
            }
        }
    }

END_DGNDBSYNC_DGNV8_NAMESPACE






















