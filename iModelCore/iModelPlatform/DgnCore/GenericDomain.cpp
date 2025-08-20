/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/GenericDomain.h>

//=======================================================================================
//  Handler definitions
//=======================================================================================
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

DOMAIN_DEFINE_MEMBERS(GenericDomain)

namespace generic_ModelHandler
    {
    HANDLER_DEFINE_MEMBERS(GroupModel)
    };

namespace generic_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(Group)
    HANDLER_DEFINE_MEMBERS(SpatialLocation)
    HANDLER_DEFINE_MEMBERS(PhysicalObject)
    HANDLER_DEFINE_MEMBERS(PhysicalType)
    HANDLER_DEFINE_MEMBERS(GraphicalType2d)
    HANDLER_DEFINE_MEMBERS(Graphic3d)
    HANDLER_DEFINE_MEMBERS(DetailingSymbol)
    HANDLER_DEFINE_MEMBERS(Callout)
    HANDLER_DEFINE_MEMBERS(ViewAttachmentLabel)
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericDomain::GenericDomain() : DgnDomain(GENERIC_DOMAIN_NAME, "Generic Domain", 1) 
    {
    RegisterHandler(generic_ModelHandler::GroupModel::GetHandler());
    RegisterHandler(generic_ElementHandler::Group::GetHandler());
    RegisterHandler(generic_ElementHandler::SpatialLocation::GetHandler());
    RegisterHandler(generic_ElementHandler::PhysicalObject::GetHandler());
    RegisterHandler(generic_ElementHandler::PhysicalType::GetHandler());
    RegisterHandler(generic_ElementHandler::GraphicalType2d::GetHandler());
    RegisterHandler(generic_ElementHandler::Graphic3d::GetHandler());
    RegisterHandler(generic_ElementHandler::DetailingSymbol::GetHandler());
    RegisterHandler(generic_ElementHandler::Callout::GetHandler());
    RegisterHandler(generic_ElementHandler::ViewAttachmentLabel::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericDomain::~GenericDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void createGenericCodeSpecs(DgnDbR db)
    {
    CodeSpecPtr disciplineCodeSpec = CodeSpec::Create(db, GENERIC_CODESPEC_ViewAttachmentLabel, CodeScopeSpec::CreateModelScope());
    if (disciplineCodeSpec.IsValid())
        disciplineCodeSpec->Insert();

    BeAssert(db.CodeSpecs().QueryCodeSpecId(GENERIC_CODESPEC_ViewAttachmentLabel).IsValid());    
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void GenericDomain::_OnSchemaImported(DgnDbR db) const
    {
    createGenericCodeSpecs(db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericPhysicalObjectPtr GenericPhysicalObject::Create(PhysicalModelR model, DgnCategoryId categoryId)
    {
    DgnClassId classId = model.GetDgnDb().Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());

    if (!classId.IsValid() || !categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new GenericPhysicalObject(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericSpatialLocationPtr GenericSpatialLocation::Create(PhysicalModelR model, DgnCategoryId categoryId)
    {
    DgnClassId classId = model.GetDgnDb().Domains().GetClassId(generic_ElementHandler::SpatialLocation::GetHandler());

    if (!classId.IsValid() || !categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new GenericSpatialLocation(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericGroupPtr GenericGroup::Create(DgnModelR model, DgnCodeCR code)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::Group::GetHandler());

    if (!classId.IsValid())
        return nullptr;

    return new GenericGroup(CreateParams(db, modelId, classId, code));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericGroupModelPtr GenericGroupModel::Create(DgnElementCR modeledElement)
    {
    ModelHandlerR handler = generic_ModelHandler::GroupModel::GetHandler();
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(handler);
    if (!classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<GenericGroupModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GenericGroupModelPtr GenericGroupModel::CreateAndInsert(DgnElementCR modeledElement)
    {
    GenericGroupModelPtr model = Create(modeledElement);
    if (!model.IsValid())
        return nullptr;

    if (DgnDbStatus::Success != model->Insert())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus GenericGroupModel::_OnInsertElement(DgnElementR element)
    {
    if (nullptr != dynamic_cast<GroupInformationElementP>(&element))
        return T_Super::_OnInsertElement(element);

    BeAssert(false);
    return DgnDbStatus::WrongModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GraphicalModel3dPtr GenericGraphicalModel3d::Create(DgnElementCR modeledElement)
    {
    ModelHandlerR handler = dgn_ModelHandler::Graphical3d::GetHandler();
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId = db.Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_GraphicalModel3d);
    if (!classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<GraphicalModel3dP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
GraphicalModel3dPtr GenericGraphicalModel3d::CreateAndInsert(DgnElementCR modeledElement)
    {
    GraphicalModel3dPtr model = Create(modeledElement);
    if (!model.IsValid())
        return nullptr;

    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId GenericCallout::FindViewAttachment() const
    {
    auto drawingModelId = GetDrawingModel();
    if (!drawingModelId.IsValid())
        return DgnElementId();

    auto& db = GetDgnDb();
    auto findViewAttachment = db.GetPreparedECSqlStatement(
        "SELECT viewAttachment.ECInstanceId FROM bis.ViewAttachment viewAttachment, bis.ViewDefinition2d view2d"
        " WHERE (viewAttachment.[View].Id = view2d.ECInstanceId) AND (view2d.BaseModel.Id = ?)");
    findViewAttachment->BindId(1, drawingModelId);
    if (BE_SQLITE_ROW != findViewAttachment->Step())
        return DgnElementId();

    return findViewAttachment->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId GenericViewAttachmentLabel::FindFromViewAttachment(DgnDbR db, DgnElementId vaid)
    {
    auto findLabel = db.GetPreparedECSqlStatement(
        "SELECT label.ECInstanceId FROM generic.ViewAttachmentLabel label"
        " WHERE (label.ViewAttachment.Id = ?)");
    findLabel->BindId(1, vaid);
    if (BE_SQLITE_ROW != findLabel->Step())
        return DgnElementId();

    return findLabel->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> GenericViewAttachmentLabel::FindCalloutsFor(Sheet::ViewAttachmentCR viewAttachment)
    {
    auto& db = viewAttachment.GetDgnDb();
    auto findCallout = db.GetPreparedECSqlStatement(
        "SELECT callout.ECInstanceId FROM bis.ViewDefinition2d view2d, generic.Callout callout"
        " WHERE (callout.DrawingModel.Id = view2d.BaseModel.Id) AND (view2d.ECInstanceId = ?)");

    findCallout->BindId(1, viewAttachment.GetAttachedViewId());
    
    bvector<DgnElementId> callouts;
    while (BE_SQLITE_ROW == findCallout->Step())
        {
        callouts.push_back(findCallout->GetValueId<DgnElementId>(0));
        }

    return callouts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericCalloutDestination GenericCalloutDestination::FindDestinationOf(GenericCalloutCR callout)
    {
    auto& db = callout.GetDgnDb();
    GenericCalloutDestination dest;
    dest.m_drawingModel = db.Models().Get<DrawingModel>(callout.GetDrawingModel());
    dest.m_viewAttachment = db.Elements().Get<Sheet::ViewAttachment>(callout.FindViewAttachment());
    if (!dest.m_viewAttachment.IsValid())
        return dest;
    auto labelId = GenericViewAttachmentLabel::FindFromViewAttachment(db, dest.m_viewAttachment->GetElementId());
    dest.m_viewAttachmentLabel = db.Elements().Get<GenericViewAttachmentLabel>(labelId);
    dest.m_sheetView = db.Elements().Get<SheetViewDefinition>(Sheet::Model::FindFirstViewOfSheet(db, dest.m_viewAttachment->GetModel()->GetModelId()));

#ifndef NDEBUG
    // check that the reverse lookup works
    if (dest.m_viewAttachmentLabel.IsValid())
        {
        auto locs = GenericCalloutLocation::FindCalloutsFor(*dest.m_viewAttachmentLabel);
        bool foundThis = false;
        for (auto const& loc : locs)
            foundThis |= (loc.GetCallout() == &callout);
        BeAssert(foundThis);
        }
#endif

    return dest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GenericCalloutLocation> GenericCalloutLocation::FindCalloutsFor(Sheet::ViewAttachmentCR viewAttachment)
    {
    auto& db = viewAttachment.GetDgnDb();
    bvector<GenericCalloutLocation> locs;
    auto callouts = GenericViewAttachmentLabel::FindCalloutsFor(viewAttachment);
    for (auto callout : callouts)
        {
        GenericCalloutLocation loc;
        loc.m_callout = db.Elements().Get<GenericCallout>(callout);
        if (!loc.m_callout.IsValid())
            continue;
        loc.m_sheetView = db.Elements().Get<SheetViewDefinition>(Sheet::Model::FindFirstViewOfSheet(db, loc.m_callout->GetModel()->GetModelId()));
        locs.push_back(loc);
        }
    return locs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericPhysicalTypePtr GenericPhysicalType::Create(DefinitionModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::PhysicalType::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new GenericPhysicalType(CreateParams(db, model.GetModelId(), classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericGraphicalType2dPtr GenericGraphicalType2d::Create(DefinitionModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::GraphicalType2d::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new GenericGraphicalType2d(CreateParams(db, model.GetModelId(), classId, code));
    }
