/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSchemaDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h>

struct DgnSchemaTableHandler
{
//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    11/2014
//=======================================================================================
struct Element : DgnDomain::TableHandler
{
    TABLEHANDLER_DECLARE_MEMBERS(DGN_TABLE(DGN_CLASSNAME_Element), Element, DGNPLATFORM_EXPORT)

private:
    void AddElementChange(bset<DgnElementId>& elSet, BeSQLite::Changes::Change const& change, bool useNew) const;
    virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override;
    virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override;
    virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override;

    void GetElementChangeInfo(DgnElementId&, DgnClassId&, DgnModelId&, DgnDbR, BeSQLite::Changes::Change const&, TxnSummary::ChangeType) const;
    void RecordElementChange(TxnSummary&, TxnSummary::ChangeType, DgnElementId, DgnModelId) const;
};

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      03/2015
//=======================================================================================
struct ElementDrivesElement : DgnDomain::TableHandler
{
protected:
    TABLEHANDLER_DECLARE_MEMBERS(DGN_TABLE(DGN_RELNAME_ElementDrivesElement), ElementDrivesElement, DGNPLATFORM_EXPORT)

    virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override    {UpdateSummary(summary,change,TxnSummary::ChangeType::Add);}
    virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {UpdateSummary(summary,change,TxnSummary::ChangeType::Mod);}
    virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {UpdateSummary(summary,change,TxnSummary::ChangeType::Delete);}

    void UpdateSummary(TxnSummary& summary, BeSQLite::Changes::Change change, TxnSummary::ChangeType changeType) const
        {
        Changes::Change::Stage stage = (TxnSummary::ChangeType::Add == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
        ECInstanceId instanceId(change.GetValue(0, stage).GetValueInt64()); // primary key is column 0
        summary.AddAffectedDependency(instanceId, changeType);
        }
};

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    11/2014
//=======================================================================================
struct ModelDrivesModel : DgnDomain::TableHandler
{
protected:
    TABLEHANDLER_DECLARE_MEMBERS(DGN_TABLE(DGN_RELNAME_ModelDrivesModel), ModelDrivesModel, DGNPLATFORM_EXPORT)

    virtual void _OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override 
        {
        summary.SetModelDependencyChanges(); 
        CheckDirection(summary, change.GetNewValue(0).GetValueId<EC::ECInstanceId>());
        }
    virtual void _OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override 
        {
        summary.SetModelDependencyChanges();
        CheckDirection(summary, change.GetOldValue(0).GetValueId<EC::ECInstanceId>());
        }
    virtual void _OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const override {summary.SetModelDependencyChanges();}
    void CheckDirection(TxnSummary&, EC::ECInstanceId) const;
};

}; // DgnSchemaTableHandler

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      01/15
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::Element::RecordElementChange (TxnSummary& summary, TxnSummary::ChangeType ct, DgnElementId elementId, DgnModelId modelId) const 
    {
    summary.AddAffectedElement (elementId, modelId, ct);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::Element::GetElementChangeInfo(DgnElementId& elementId, DgnClassId& classId, DgnModelId& modelId, DgnDbR db, BeSQLite::Changes::Change const& change, TxnSummary::ChangeType changeType) const
    {
    Changes::Change::Stage stage;
    switch (changeType)
        {
        case TxnSummary::ChangeType::Add:
            stage = Changes::Change::Stage::New; 
            break;

        case TxnSummary::ChangeType::Mod:
        case TxnSummary::ChangeType::Delete:    
            stage = Changes::Change::Stage::Old; 
            break;
        default: 
            BeAssert (false);
            return;
        }

    elementId = DgnElementId(change.GetValue(0, stage).GetValueInt64());

    if (TxnSummary::ChangeType::Mod == changeType)
        {
        // for updates, the element table must be queried for ECClassId and ModelId since the change set will only contain changed columns
        CachedStatementPtr stmt;
        db.GetCachedStatement (stmt, "SELECT ECClassId,ModelId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");

        stmt->BindId (1, elementId);
    
        if (BE_SQLITE_ROW != stmt->Step()) 
            return;
    
        classId = DgnClassId(stmt->GetValueInt64(0));
        modelId = stmt->GetValueId<DgnModelId>(1);
        return;
        }

    classId = DgnClassId(change.GetValue(1, stage).GetValueInt64());                // assumes ECClassId is column 1
    modelId = DgnModelId (change.GetValue(2, stage).GetValueInt64());   // assumes DgnModelId is column 2
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::Element::_OnAdd(TxnSummary& summary, BeSQLite::Changes::Change const& change) const 
    {
    DgnElementId elementId;
    DgnClassId   elementClassId;
    DgnModelId   elementModelId;

    GetElementChangeInfo(elementId, elementClassId, elementModelId, summary.GetDgnDb(), change, TxnSummary::ChangeType::Add);
    RecordElementChange(summary, TxnSummary::ChangeType::Add, elementId, elementModelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::Element::_OnDelete(TxnSummary& summary, BeSQLite::Changes::Change const& change) const 
    {
    DgnElementId elementId;
    DgnClassId   elementClassId;
    DgnModelId   elementModelId;

    GetElementChangeInfo (elementId, elementClassId, elementModelId, summary.GetDgnDb(), change, TxnSummary::ChangeType::Delete);
    RecordElementChange (summary, TxnSummary::ChangeType::Delete, elementId, elementModelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::Element::_OnUpdate(TxnSummary& summary, BeSQLite::Changes::Change const& change) const 
    {
    DgnElementId elementId;
    DgnClassId   elementClassId;
    DgnModelId   elementModelId;

    GetElementChangeInfo (elementId, elementClassId, elementModelId, summary.GetDgnDb(), change, TxnSummary::ChangeType::Mod);
    RecordElementChange (summary, TxnSummary::ChangeType::Mod, elementId, elementModelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  03/2015
//---------------------------------------------------------------------------------------
void DgnSchemaTableHandler::ModelDrivesModel::CheckDirection(TxnSummary& summary, EC::ECInstanceId relid) const
    {
    Statement stmt;
    stmt.Prepare(summary.GetDgnDb(), "SELECT RootModelId,DependentModelId FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel) " WHERE(ECInstanceId=?)");
    stmt.BindId(1, relid);
    if (stmt.Step() != BE_SQLITE_ROW)
        {
        BeAssert(false); // model was just added or modified -- it has to exist!
        return;
        }
    DgnModelId rootModel = stmt.GetValueId<DgnModelId>(0);
    DgnModelId depModel = stmt.GetValueId<DgnModelId>(1);

    DgnModels::Model root, dep;
    summary.GetDgnDb().Models().QueryModelById(&root, rootModel);
    summary.GetDgnDb().Models().QueryModelById(&dep, depModel);

    if (root.GetModelType() > dep.GetModelType())
        {
        //  A Physical model cannot depend on a Drawing model
        summary.GetDgnDb().GetTxnManager().ReportValidationError(*new DgnElementDependencyGraph::DirectionValidationError(""));
        }
    }


#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementHandler::FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, DgnPlatform::FenceClipFlags options)
    {
    StatusInt   status = SUCCESS;

    if (fp->AcceptElement (eh))
        {
        if (fp->HasOverlaps())
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
    outAgenda.Empty();

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

        if (0 == gpa->GetCount())
            continue;

        gpa->Transform (&clipToWorld);

        EditElementHandle   eeh;

        if (SUCCESS != gpa->ToElement (eeh, NULL, true, true, *fp.GetDgnModel()))
            return ERROR;

        if (outAgenda.IsEmpty())
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

            if (SUCCESS != regionContext.Boolean (*fp.GetDgnModel(), outAgenda, NULL, operation))
                return ERROR;

            if (SUCCESS != regionContext.GetRegions (outAgenda))
                return ERROR;
            }
        }

    return (outAgenda.IsEmpty() ? ERROR : SUCCESS);
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

    for (EditElementHandleP curr = fenceAgenda.GetFirstP(), end = curr + fenceAgenda.GetCount(); curr < end ; curr++)
        {
        tmpEeh.Duplicate (*curr);
        agenda.Insert (tmpEeh);
        }

    if (SUCCESS != regionContext.Boolean (modelRef, agenda, NULL, operation))
        return ERROR;

    ElementAgenda   resultAgenda;

    if (SUCCESS != regionContext.GetRegions (resultAgenda))
        return ERROR;

    for (EditElementHandleP curr = resultAgenda.GetFirstP(), end = curr + resultAgenda.GetCount(); curr < end ; curr++)
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

    if (pathCurve.IsNull() || !(pathCurve->IsClosedPath() || pathCurve->IsUnionRegion() || pathCurve->IsParityRegion()))
        return ERROR;

    bool            isOutside;
    ElementAgenda   fenceAgenda;

    if (SUCCESS != elementsFromFence (fenceAgenda, isOutside, *fp))
        return ERROR;

    StatusInt   outsideStatus = SUCCESS, insideStatus = SUCCESS;

    if (outside)
        outsideStatus = booleanWithFence (*outside, eh, fenceAgenda, !isOutside, *fp->GetDgnModel());

    if (inside)
        insideStatus = booleanWithFence (*inside, eh, fenceAgenda, isOutside, *fp->GetDgnModel());

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
BentleyStatus GetConvertStatus() {return m_convertStatus;}

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
        m_currentTransform.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (SUCCESS != m_convertStatus)
        return SUCCESS;

    if (!curves.IsAnyRegionType())
        return SUCCESS;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin();

    FenceParams tmpFp (m_fp);
    Transform   transform;

    transform.InitProduct (m_currentTransform, *tmpFp.GetTransform());
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

        if (SUCCESS == DraftingElementSchema::ToElement (eeh, *tmpCurves, m_eh, true, *m_eh->GetDgnModelP()))
            m_inside->Insert (eeh);
        }

    for (CurveVectorPtr tmpCurves: outsideCurves)
        {
        EditElementHandle   eeh;

        if (SUCCESS == DraftingElementSchema::ToElement (eeh, *tmpCurves, m_eh, true, *m_eh->GetDgnModelP()))
            m_outside->Insert (eeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessBody (ISolidKernelEntityCR entity) override
    {
    if (SUCCESS != m_convertStatus)
        return SUCCESS;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin();

    FenceParams tmpFp (m_fp);
    Transform   transform;

    transform.InitProduct (m_currentTransform, *tmpFp.GetTransform());
    tmpFp.SetTransform (transform);

    bvector<ISolidKernelEntityPtr> insideEntity;
    bvector<ISolidKernelEntityPtr> outsideEntity;

    if (SUCCESS != solidAdmin._FenceClipBody (m_inside ? &insideEntity : NULL, m_outside ? &outsideEntity : NULL, entity, tmpFp))
        {
        m_convertStatus = ERROR;

        return SUCCESS;
        }

    for (ISolidKernelEntityPtr entityOut : insideEntity)
        {
        EditElementHandle   eeh;

        if (SUCCESS != BrepCellHeaderHandler::CreateBRepCellElement (eeh, m_eh, *entityOut, *m_eh->GetDgnModelP()))
            continue;

        m_inside->Insert (eeh);
        }

    for (ISolidKernelEntityPtr entityOut : outsideEntity)
        {
        EditElementHandle   eeh;

        if (SUCCESS != BrepCellHeaderHandler::CreateBRepCellElement (eeh, m_eh, *entityOut, *m_eh->GetDgnModelP()))
            continue;

        m_outside->Insert (eeh);
        }

    return SUCCESS;
    }

}; // FenceClipOutputProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt doOptimizedClip3d (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp)
    {
    // Filter open elements and stuff like dimensions (which could draw shapes in terminators, etc.)...
    DisplayHandlerP dHandler = eh.GetDisplayHandler();

    if (!dHandler || !dHandler->IsRenderable (eh))
        return ERROR;

    FenceClipOutputProcessor processor (inside, outside, &eh, fp);

    // Ask element to draw and process output as bodies...
    ElementGraphicsOutput::Process (processor, eh);

    return processor.GetConvertStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt doOptimizedClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp)
    {
    if (!eh.IsValid() || !fp->GetDgnModel())
        return ERROR;

    // NOTE: Need 3d clip if 3d element or 2d element attached to 3d...
    if (eh.GetDgnModelP()->Is3d() || fp->GetDgnModel()->Is3d())
        return doOptimizedClip3d (inside, outside, eh, fp);

    return doOptimizedClip2d (inside, outside, eh, fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                 Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementHandler::_FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options)
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
StatusInt ElementHandler::_FenceStretch (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    // Standard fence stretch logic:
    FenceParams     tmpFp ((FenceParams *) fp);

    tmpFp.SetClipMode (FenceClipMode::Copy); // Need overlaps returned...
    tmpFp.SetOverlapMode (false);

    // Element is completely outside fence...return SUCCESS to do nothing...
    if (!tmpFp.AcceptElement (elemHandle))
        return SUCCESS;

    // Element is completely inside fence...transform it...
    if (!tmpFp.HasOverlaps())
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
StatusInt ElementHandler::_OnFenceStretchFinish (EditElementHandleR eeh, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                               Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementHandler::_ApplyTransform (EditElementHandleR elemHandle, TransformInfoCR t)
    {
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
StatusInt ElementHandler::_OnTransformFinish (EditElementHandleR elemHandle, TransformInfoCR tinfo)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementHandler::ApplyTransform (EditElementHandleR element, TransformInfoCR transform)
    {
    return _ApplyTransform (element, transform);
    }
#endif

TABLEHANDLER_DEFINE_MEMBERS(DgnSchemaTableHandler::Element)
TABLEHANDLER_DEFINE_MEMBERS(DgnSchemaTableHandler::ModelDrivesModel)
TABLEHANDLER_DEFINE_MEMBERS(DgnSchemaTableHandler::ElementDrivesElement)

HANDLER_DEFINE_MEMBERS(ModelHandler)
HANDLER_DEFINE_MEMBERS(PhysicalModelHandler)
HANDLER_DEFINE_MEMBERS(WebMercatorModelHandler)
HANDLER_DEFINE_MEMBERS(StreetMapModelHandler)
HANDLER_DEFINE_MEMBERS(SheetModelHandler)
HANDLER_DEFINE_MEMBERS(GraphicsModel2dHandler)
HANDLER_DEFINE_MEMBERS(PlanarPhysicalModelHandler)
HANDLER_DEFINE_MEMBERS(SectionDrawingModelHandler)
HANDLER_DEFINE_MEMBERS(ElementHandler)
HANDLER_DEFINE_MEMBERS(ElementGroupHandler)
HANDLER_DEFINE_MEMBERS(PhysicalElementHandler)
HANDLER_DEFINE_MEMBERS(DrawingElementHandler)
HANDLER_DEFINE_MEMBERS(ViewHandler)
HANDLER_DEFINE_MEMBERS(PointCloudBaseModelHandler)
HANDLER_DEFINE_MEMBERS(RasterBaseModelHandler)

HANDLER_EXTENSION_DEFINE_MEMBERS(IEditManipulatorExtension)
HANDLER_EXTENSION_DEFINE_MEMBERS(ViewHandlerOverride)

DOMAIN_DEFINE_MEMBERS(DgnSchemaDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerP ElementHandler::FindHandler(DgnDb const& db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return handler->_ToElementHandler();

    // not there, check via base classes
    //WIP: FInd safer way to make sure to find the best matching base class element handler
    //To avoid that ElementHandler is used for classes that don't have their own handler
    //but are derived from PhysicalElement or other well-known BIS element types
    bvector<ElementHandlerP> baseClassHandlers;
    baseClassHandlers.push_back(&PhysicalElementHandler::GetHandler());
    baseClassHandlers.push_back(&DrawingElementHandler::GetHandler());
    baseClassHandlers.push_back(&ElementHandler::GetHandler());

    for (ElementHandler* baseHandler : baseClassHandlers)
        {
        handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(*baseHandler));
        if (handler != nullptr)
            return handler->_ToElementHandler();
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSchemaDomain::DgnSchemaDomain() : DgnDomain (DGN_ECSCHEMA_NAME, "Base DgnDb Domain",1) 
    {
    RegisterHandler(ModelHandler::GetHandler());
    RegisterHandler(PhysicalModelHandler::GetHandler());
    RegisterHandler(WebMercatorModelHandler::GetHandler());
    RegisterHandler(StreetMapModelHandler::GetHandler());
    RegisterHandler(SheetModelHandler::GetHandler());
    RegisterHandler(GraphicsModel2dHandler::GetHandler());
    RegisterHandler(PlanarPhysicalModelHandler::GetHandler());
    RegisterHandler(SectionDrawingModelHandler::GetHandler());

    RegisterHandler(ElementHandler::GetHandler());
    RegisterHandler(ViewHandler::GetHandler());
    RegisterHandler(PhysicalElementHandler::GetHandler());
    RegisterHandler(DrawingElementHandler::GetHandler());
    RegisterHandler(ElementGroupHandler::GetHandler());
    RegisterHandler(PointCloudBaseModelHandler::GetHandler());
    RegisterHandler(RasterBaseModelHandler::GetHandler());
    RegisterHandler(PhysicalTextAnnotationElementHandler::GetHandler());

    RegisterDefaultDependencyHandlers();

    RegisterTableHandler(DgnSchemaTableHandler::Element::GetHandler());
    RegisterTableHandler(DgnSchemaTableHandler::ElementDrivesElement::GetHandler());
    RegisterTableHandler(DgnSchemaTableHandler::ModelDrivesModel::GetHandler());
    }
