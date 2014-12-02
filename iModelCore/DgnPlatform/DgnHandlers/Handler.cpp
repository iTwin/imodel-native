/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Handler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnHandlers/Annotations/TextAnnotationHandler.h>
#include <DgnPlatform/CVEHandler.h> // added in graphite

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, DgnPlatform::FenceClipFlags options)
    {
    StatusInt   status = SUCCESS;

    if (fp->AcceptElement (eh))
        {
        if (fp->HasOverlaps ())
            {
            status = _FenceClip (inside, outside, eh, fp, options);
            }
        else if (inside)
            {
            EditElementHandle   tmpEeh;

            tmpEeh.Duplicate (eh);
            inside->Insert (tmpEeh);
            }
        }
    else if (outside)
        {
        EditElementHandle   tmpEeh;

        tmpEeh.Duplicate (eh);
        outside->Insert (tmpEeh);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt elementsFromFence (ElementAgendaR outAgenda, bool& outside, FenceParamsR fp)
    {
    outAgenda.Empty ();

    if (!fp.GetClipVector().IsValid())
        return ERROR;

    for (ClipPrimitivePtr const& primitive: *fp.GetClipVector())
        {
        ClipPolygonCP       clipPolygon;

        if (NULL == (clipPolygon = primitive->GetPolygon()))
            continue; // Clip mask w/o clip boundary...

        Transform   clipToWorld;

        if (SUCCESS != fp.GetClipToWorldTransform (clipToWorld, *primitive))
            return ERROR;

        GPArraySmartP   gpa;

        gpa->CopyContentsOf (primitive->GetGPA (false));

        if (0 == gpa->GetCount ())
            continue;

        gpa->Transform (&clipToWorld);

        EditElementHandle   eeh;

        if (SUCCESS != gpa->ToElement (eeh, NULL, true, true, *fp.GetDgnModel ()))
            return ERROR;

        if (outAgenda.IsEmpty ())
            {
            outAgenda.Insert (eeh);

            outside = primitive->IsMask();
            }
        else
            {
            RegionGraphicsContext   regionContext;
            RegionType              operation = RegionType::Difference;

            if (outside == primitive->IsMask())
                {
                outAgenda.Insert (eeh);

                operation = (outside ? RegionType::Union : RegionType::Intersection);
                }
            else
                {
                if (outside)
                    {
                    outAgenda.Insert (eeh, true);

                    outside = primitive->IsMask();
                    }
                else
                    {
                    outAgenda.Insert (eeh);
                    }
                }

            if (SUCCESS != regionContext.Boolean (*fp.GetDgnModel (), outAgenda, NULL, operation))
                return ERROR;

            if (SUCCESS != regionContext.GetRegions (outAgenda))
                return ERROR;
            }
        }

    return (outAgenda.IsEmpty () ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static int booleanWithFence (ElementAgendaR out, ElementHandleCR eh, ElementAgendaR fenceAgenda, bool outside, DgnModelR modelRef)
    {
    RegionGraphicsContext   regionContext;
    RegionType              operation = (outside ? RegionType::Difference : RegionType::Intersection);
    ElementAgenda           agenda;
    EditElementHandle       tmpEeh;

    tmpEeh.Duplicate (eh);
    agenda.Insert (tmpEeh);

    for (EditElementHandleP curr = fenceAgenda.GetFirstP (), end = curr + fenceAgenda.GetCount (); curr < end ; curr++)
        {
        tmpEeh.Duplicate (*curr);
        agenda.Insert (tmpEeh);
        }

    if (SUCCESS != regionContext.Boolean (modelRef, agenda, NULL, operation))
        return ERROR;

    ElementAgenda   resultAgenda;

    if (SUCCESS != regionContext.GetRegions (resultAgenda))
        return ERROR;

    for (EditElementHandleP curr = resultAgenda.GetFirstP (), end = curr + resultAgenda.GetCount (); curr < end ; curr++)
        {
        ElementPropertiesSetter::ApplyTemplate (*curr, eh);
        out.Insert (*curr);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt doOptimizedClip2d (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp)
    {
    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (pathCurve.IsNull () || !(pathCurve->IsClosedPath () || pathCurve->IsUnionRegion () || pathCurve->IsParityRegion ()))
        return ERROR;

    bool            isOutside;
    ElementAgenda   fenceAgenda;

    if (SUCCESS != elementsFromFence (fenceAgenda, isOutside, *fp))
        return ERROR;

    StatusInt   outsideStatus = SUCCESS, insideStatus = SUCCESS;

    if (outside)
        outsideStatus = booleanWithFence (*outside, eh, fenceAgenda, !isOutside, *fp->GetDgnModel ());

    if (inside)
        insideStatus = booleanWithFence (*inside, eh, fenceAgenda, isOutside, *fp->GetDgnModel ());

    return ((SUCCESS == outsideStatus || SUCCESS == insideStatus) ? SUCCESS : ERROR);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FenceClipOutputProcessor : IElementGraphicsProcessor
{
private:

ElementAgendaP  m_inside;
ElementAgendaP  m_outside;
FenceParamsP    m_fp;
ElementHandleCP m_eh;
Transform       m_currentTransform;
BentleyStatus   m_convertStatus;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
explicit FenceClipOutputProcessor (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCP eh, FenceParamsP fp)
    {
    m_inside        = inside;
    m_outside       = outside;
    m_eh            = eh;
    m_fp            = fp;
    m_convertStatus = SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetConvertStatus () {return m_convertStatus;}

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsBody (bool isCurved) const override {return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (SUCCESS != m_convertStatus)
        return SUCCESS;

    if (!curves.IsAnyRegionType ())
        return SUCCESS;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin ();

    FenceParams tmpFp (m_fp);
    Transform   transform;

    transform.InitProduct (m_currentTransform, *tmpFp.GetTransform ());
    tmpFp.SetTransform (transform);

    bvector<CurveVectorPtr> insideCurves;
    bvector<CurveVectorPtr> outsideCurves;

    if (SUCCESS != solidAdmin._FenceClipCurveVector (m_inside ? &insideCurves : NULL, m_outside ? &outsideCurves : NULL, curves, tmpFp))
        {
        m_convertStatus = ERROR;

        return SUCCESS;
        }

    for (CurveVectorPtr tmpCurves: insideCurves)
        {
        EditElementHandle   eeh;

        if (SUCCESS == DraftingElementSchema::ToElement (eeh, *tmpCurves, m_eh, true, *m_eh->GetDgnModelP ()))
            m_inside->Insert (eeh);
        }

    for (CurveVectorPtr tmpCurves: outsideCurves)
        {
        EditElementHandle   eeh;

        if (SUCCESS == DraftingElementSchema::ToElement (eeh, *tmpCurves, m_eh, true, *m_eh->GetDgnModelP ()))
            m_outside->Insert (eeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP) override
    {
    if (SUCCESS != m_convertStatus)
        return SUCCESS;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin ();

    FenceParams tmpFp (m_fp);
    Transform   transform;

    transform.InitProduct (m_currentTransform, *tmpFp.GetTransform ());
    tmpFp.SetTransform (transform);

    bvector<ISolidKernelEntityPtr> insideEntity;
    bvector<ISolidKernelEntityPtr> outsideEntity;

    if (SUCCESS != solidAdmin._FenceClipBody (m_inside ? &insideEntity : NULL, m_outside ? &outsideEntity : NULL, entity, tmpFp))
        {
        m_convertStatus = ERROR;

        return SUCCESS;
        }

#if defined (NEEDS_WORK_DGNITEM)
    for (ISolidKernelEntityPtr entityOut : insideEntity)
        {
        EditElementHandle   eeh;

        if (SUCCESS != BrepCellHeaderHandler::CreateBRepCellElement (eeh, m_eh, *entityOut, *m_eh->GetDgnModelP ()))
            continue;

        m_inside->Insert (eeh);
        }

    for (ISolidKernelEntityPtr entityOut : outsideEntity)
        {
        EditElementHandle   eeh;

        if (SUCCESS != BrepCellHeaderHandler::CreateBRepCellElement (eeh, m_eh, *entityOut, *m_eh->GetDgnModelP ()))
            continue;

        m_outside->Insert (eeh);
        }
#endif

    return SUCCESS;
    }

}; // FenceClipOutputProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt doOptimizedClip3d (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp)
    {
    // Filter open elements and stuff like dimensions (which could draw shapes in terminators, etc.)...
    DisplayHandlerP dHandler = eh.GetDisplayHandler ();

    if (!dHandler || !dHandler->IsRenderable (eh))
        return ERROR;

    FenceClipOutputProcessor processor (inside, outside, &eh, fp);

    // Ask element to draw and process output as bodies...
    ElementGraphicsOutput::Process (processor, eh);

    return processor.GetConvertStatus ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt doOptimizedClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp)
    {
    if (!eh.IsValid () || !fp->GetDgnModel ())
        return ERROR;

    // NOTE: Need 3d clip if 3d element or 2d element attached to 3d...
    if (eh.GetElementCP ()->Is3d() || fp->GetDgnModel ()->Is3d ())
        return doOptimizedClip3d (inside, outside, eh, fp);

    return doOptimizedClip2d (inside, outside, eh, fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                 Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::_FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options)
    {
    StatusInt   status = ERROR;

    // First give optimized clip a shot...
    if (FenceClipFlags::None != (options & FenceClipFlags::Optimized))
        status = doOptimizedClip (inside, outside, eh, fp);

    // Default clipping...
    if (SUCCESS != status)
        status = _OnFenceClip (inside, outside, eh, fp, options);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                 Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::_FenceStretch (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    // Standard fence stretch logic:
    FenceParams     tmpFp ((FenceParams *) fp);

    tmpFp.SetClipMode (FenceClipMode::Copy); // Need overlaps returned...
    tmpFp.SetOverlapMode (false);

    // Element is completely outside fence...return SUCCESS to do nothing...
    if (!tmpFp.AcceptElement (elemHandle))
        return SUCCESS;

    // Element is completely inside fence...transform it...
    if (!tmpFp.HasOverlaps ())
        return _ApplyTransform (elemHandle, transform);

    // Element overlaps the fence: let the subclass decide what to do. (Note: subclass will process its children)
    StatusInt   status = _OnFenceStretch (elemHandle, transform, fp, options);

    if (SUCCESS != status)
        return status;

    return _OnFenceStretchFinish (elemHandle, transform, fp, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::_OnFenceStretchFinish (EditElementHandleR eeh, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    DgnElementCP el = eeh.GetElementCP ();

    if (el->GetSizeWords() > el->GetAttributeOffset())
        ElementUtil::FenceStretchXDataLinkage (eeh, transform, fp);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                               Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::_ApplyTransform (EditElementHandleR elemHandle, TransformInfoCR t)
    {
    if (NULL == elemHandle.GetElementCP())
/*<=*/  return DGNMODEL_STATUS_ReadOnly;

    //  Tell sub-class to transform
    /* NOTE: If not being able update the range on a complex header, which for example
             was created by a call to mdlElement_transform inside a mdlModify_ function
             is considered an error, will break existing applications?  Just return SUCCESS */
    StatusInt status = _OnTransform (elemHandle, t);
    if (SUCCESS != status)
        return status;

    return _OnTransformFinish (elemHandle, t);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                               Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::_OnTransformFinish (EditElementHandleR elemHandle, TransformInfoCR tinfo)
    {
    // Transform known linkages that are not specific to graphic elements
    ElementUtil::ApplyTransformToXDataLinkage (elemHandle, tinfo);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  Handler::_DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP)
    {
    if (NULL == levelsOn)
        return  ScanTestResult::Pass;

    DgnElementCP elem = eh.GetElementCP();
    int myLevel = elem->GetLevelValue();
    return ((0 == myLevel) || levelsOn->Test (myLevel-1)) ? ScanTestResult::Pass : ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    // Ask handler for type name...
    _GetTypeName (descr, desiredLength);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/12
//=======================================================================================
struct MissingDisplayHandler : public DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (MissingDisplayHandler,)
    static Handler* CreateMissingHandler () {return z_CreateInstance();}

protected:
    virtual void _GetTypeName (WStringR descr, UInt32 desiredLength) override 
        {
        descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_MISSING_HANDLER));
        }
};

//----------------------------------------------------------------------------------------
// Use ELEMENTHANDLER_DEFINE_MEMBERS for subclasses of Handler, but not for Handler itself.
//    Must keep the implementations within the ELEMENTHANDLER_DEFINE_MEMBERS macro 
//    synchronized with the Handler implementations below.
//----------------------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerP Handler::z_CreateInstance () 
    {
    Handler* instance= new Handler(); 
    instance->SetSuperClass ((Handler*) 0); 
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerP& Handler::z_PeekInstance () 
    {
    static HandlerP s_instance = 0; 
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/11
+---------------+---------------+---------------+---------------+---------------+------*/
Handler& Handler::z_GetHandlerInstance ()
    {
    HandlerP& instance = z_PeekInstance (); 

    if (0 == instance) 
        instance = z_CreateInstance (); 
    
    return *instance;
    }

ELEMENTHANDLER_DEFINE_MEMBERS(MissingDisplayHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(DisplayHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(ExtendedElementHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Handler::ExtensionEntry* Handler::ExtensionEntry::Find(ExtensionEntry* start, Extension::Token const& id)
    {
    while (start && &start->m_token != &id)
        start = start->m_next;
    return start;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Handler::AddExtension (Handler::Extension::Token& id, Handler::Extension& extension)
    {
    ExtensionEntry* prev = ExtensionEntry::Find (m_extensions, id);
    if (NULL != prev)
        return  ERROR;

    m_extensions = new Handler::ExtensionEntry(id, extension, m_extensions);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Handler::DropExtension (Handler::Extension::Token& id)
    {
    for (ExtensionEntry* prev=0, *entry=m_extensions; NULL != entry; prev=entry, entry=entry->m_next)
        {
        if (&entry->m_token != &id)
            continue;

        if (prev)
            prev->m_next = entry->m_next;
        else
            m_extensions = entry->m_next;

        delete entry;
        return SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Handler::Extension* Handler::FindExtension (Extension::Token& id)
    {
    ExtensionEntry* found = ExtensionEntry::Find (m_extensions, id);
    if (NULL != found)
        return  &found->m_extension;

    return  m_superClass ? m_superClass->FindExtension(id) : NULL;
    }

ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IDragManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IDeleteManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(ITransformManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IPropertyManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IVertexManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IPopupDialogManipulatorExtension)
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(IMaterialPropertiesExtension)

DEFINE_KEY_METHOD(IDragManipulatorExtension)
DEFINE_KEY_METHOD(IDeleteManipulatorExtension)
DEFINE_KEY_METHOD(ITransformManipulatorExtension)
DEFINE_KEY_METHOD(IPropertyManipulatorExtension)
DEFINE_KEY_METHOD(IVertexManipulatorExtension)
DEFINE_KEY_METHOD(IPopupDialogManipulatorExtension)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
HandlerP DgnSystemDomain::_GenerateMissingHandler (ElementHandlerId id)
    {
    HandlerP handler = MissingDisplayHandler::CreateMissingHandler();
    RegisterHandler (id, *handler);
    return  handler;
    }

DgnSystemDomain::DgnSystemDomain() : DgnDomain ("system", "system domain", 1) {}
DgnDraftingDomain::DgnDraftingDomain() : DgnDomain ("drafting", "drafting domain",1) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::locked_StaticInitializeDomains()
    {
    BeAssert (DgnPlatformLib::InStaticInitialization());

    DgnSystemDomain* systemDomain     = new DgnSystemDomain();
    DgnDraftingDomain* draftingDomain = new DgnDraftingDomain(); 
    systemDomain->InitDgnCore(*draftingDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::locked_StaticInitializeRegisterHandlers()
    {
    BeAssert (DgnPlatformLib::InStaticInitialization());

    DgnDraftingDomain& draftingDomain = DgnDraftingDomain::GetInstance(); 
    draftingDomain.RegisterHandler (ElementHandlerId (LEGACY_ELEMENT_TypeHandlerMajorId, 106), ELEMENTHANDLER_INSTANCE(ExtendedElementHandler));

#if defined (NEEDS_WORK_DGNITEM)
    DgnSystemDomain& systemDomain     = DgnSystemDomain::GetInstance();
    systemDomain.RegisterTypeHandler (EXTENDED_NONGRAPHIC_ELM, ELEMENTHANDLER_INSTANCE(ExtendedNonGraphicsHandler)); 
    DgnDraftingDomain& draftingDomain = DgnDraftingDomain::GetInstance(); 
    systemDomain.RegisterTypeHandler (DGNSTORE_HDR, ELEMENTHANDLER_INSTANCE(DgnStoreHdrHandler));
    systemDomain.RegisterTypeHandler (DGNFIL_HEADER_ELM, ELEMENTHANDLER_INSTANCE(Type9Handler)); 
    systemDomain.RegisterTypeHandler (MICROSTATION_ELM, ELEMENTHANDLER_INSTANCE(Type66Handler));
    systemDomain.RegisterTypeHandler (GROUP_DATA_ELM, ELEMENTHANDLER_INSTANCE(GroupDataHandler));

    draftingDomain.RegisterTypeHandler (CELL_HEADER_ELM,           ELEMENTHANDLER_INSTANCE(NormalCellHeaderHandler));
    draftingDomain.RegisterTypeHandler (LINE_ELM,                  ELEMENTHANDLER_INSTANCE(LineHandler));
    draftingDomain.RegisterTypeHandler (LINE_STRING_ELM,           ELEMENTHANDLER_INSTANCE(LineStringHandler));
    draftingDomain.RegisterTypeHandler (POINT_STRING_ELM,          ELEMENTHANDLER_INSTANCE(PointStringHandler));
    draftingDomain.RegisterTypeHandler (SHAPE_ELM,                 ELEMENTHANDLER_INSTANCE(ShapeHandler));
    draftingDomain.RegisterTypeHandler (CURVE_ELM,                 ELEMENTHANDLER_INSTANCE(CurveHandler));
    draftingDomain.RegisterTypeHandler (ELLIPSE_ELM,               ELEMENTHANDLER_INSTANCE(EllipseHandler));
    draftingDomain.RegisterTypeHandler (ARC_ELM,                   ELEMENTHANDLER_INSTANCE(ArcHandler));
    draftingDomain.RegisterTypeHandler (TEXT_NODE_ELM,             ELEMENTHANDLER_INSTANCE(TextNodeHandler));
    draftingDomain.RegisterTypeHandler (CMPLX_STRING_ELM,          ELEMENTHANDLER_INSTANCE(ComplexStringHandler));
    draftingDomain.RegisterTypeHandler (CMPLX_SHAPE_ELM,           ELEMENTHANDLER_INSTANCE(ComplexShapeHandler));
    draftingDomain.RegisterTypeHandler (TEXT_ELM,                  ELEMENTHANDLER_INSTANCE(TextElemHandler));
    draftingDomain.RegisterTypeHandler (SURFACE_ELM,               ELEMENTHANDLER_INSTANCE(SurfaceHandler));
    draftingDomain.RegisterTypeHandler (SOLID_ELM,                 ELEMENTHANDLER_INSTANCE(SolidHandler));
    draftingDomain.RegisterTypeHandler (CONE_ELM,                  ELEMENTHANDLER_INSTANCE(ConeHandler));
    draftingDomain.RegisterTypeHandler (BSPLINE_POLE_ELM,          ELEMENTHANDLER_INSTANCE(BSplinePoleHandler));
    draftingDomain.RegisterTypeHandler (BSPLINE_CURVE_ELM,         ELEMENTHANDLER_INSTANCE(BSplineCurveHandler));
    draftingDomain.RegisterTypeHandler (BSPLINE_SURFACE_ELM,       ELEMENTHANDLER_INSTANCE(BSplineSurfaceHandler));
    draftingDomain.RegisterTypeHandler (RASTER_FRAME_ELM,          ELEMENTHANDLER_INSTANCE(RasterFrameHandler));
    draftingDomain.RegisterTypeHandler (DIMENSION_ELM,             ELEMENTHANDLER_INSTANCE(DimensionHandler));
    draftingDomain.RegisterTypeHandler (SHARED_CELL_ELM,           ELEMENTHANDLER_INSTANCE(SharedCellHandler));
    draftingDomain.RegisterTypeHandler (SHAREDCELL_DEF_ELM,        ELEMENTHANDLER_INSTANCE(SharedCellDefHandler));
    draftingDomain.RegisterTypeHandler (MULTILINE_ELM,             ELEMENTHANDLER_INSTANCE(MultilineHandler));
    draftingDomain.RegisterTypeHandler (MESH_HEADER_ELM,           ELEMENTHANDLER_INSTANCE(MeshHeaderHandler));
    draftingDomain.RegisterTypeHandler (MATRIX_DOUBLE_DATA_ELM,    ELEMENTHANDLER_INSTANCE(MatrixDoubleDataHandler));
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_GroupedHole), ELEMENTHANDLER_INSTANCE(GroupedHoleHandler), (SubTypeHandlerPriority)(SUBTYPEHANDLER_PRIORITY_Normal-1));
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_AssocRegion), ELEMENTHANDLER_INSTANCE(AssocRegionCellHeaderHandler));
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_OleCell), ELEMENTHANDLER_INSTANCE(OleCellHeaderHandler));
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_BrepCell), ELEMENTHANDLER_INSTANCE(BrepCellHeaderHandler));
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_Note), ELEMENTHANDLER_INSTANCE(NoteCellHeaderHandler));

    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_AnnotationCell), ELEMENTHANDLER_INSTANCE(AnnotationCellHeaderHandler)); // Josh/Sunand
    draftingDomain.RegisterV8SubTypeHandler (CELL_HEADER_ELM, ElementHandlerId(LEGACY_ELEMENT_SubTypeHandlerMajorId,LEGACY_SUBTYPE_MissingFeatureSolid), ELEMENTHANDLER_INSTANCE(MissingFeatureSolidAppHandler), SUBTYPEHANDLER_PRIORITY_Low);
    draftingDomain.RegisterHandler (PointCloudHandler::GetElemHandlerId (), ELEMENTHANDLER_INSTANCE(PointCloudHandler));

    draftingDomain.RegisterHandler (OgreLeadHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(OgreLeadHandler));
    draftingDomain.RegisterHandler (OgreFollowerHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(OgreFollowerHandler));
    draftingDomain.RegisterHandler (StretchableExtendedElementHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(StretchableExtendedElementHandler));
    systemDomain.RegisterHandler (NonGraphicsGenericGroupLeaderHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE(NonGraphicsGenericGroupLeaderHandler));

    draftingDomain.RegisterHandler (CivilComplexStringHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE (CivilComplexStringHandler));
    draftingDomain.RegisterHandler (CivilComplexShapeHandler::GetElemHandlerId(), ELEMENTHANDLER_INSTANCE (CivilComplexShapeHandler));

    draftingDomain.RegisterHandler(TextAnnotationHandler::GetHandlerId(), ELEMENTHANDLER_INSTANCE(TextAnnotationHandler));

    ContentAreaHandler::Register ();
    ThematicDisplayStyleHandler::RegisterHandlers ();
    ProxyDisplayHandlerUtils::RegisterHandlers (draftingDomain);
    DetailingSymbolBaseHandler::RegisterHandlers();

    PersistentElementPathXAttributeHandler::StaticInitialize ();
    PersistentSnapPathXAttributeHandler::StaticInitialize ();
#endif
    }
