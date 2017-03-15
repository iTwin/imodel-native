/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GenericDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/GenericDomain.h>

//=======================================================================================
//  Handler definitions
//=======================================================================================
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

DOMAIN_DEFINE_MEMBERS(GenericDomain)

namespace generic_ModelHandler
    {
    HANDLER_DEFINE_MEMBERS(GenericGroupModelHandler)
    };

namespace generic_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(GenericGroupHandler)
    HANDLER_DEFINE_MEMBERS(GenericSpatialLocationHandler)
    HANDLER_DEFINE_MEMBERS(GenericPhysicalObjectHandler)
    HANDLER_DEFINE_MEMBERS(GenericGraphic3dHandler)
    HANDLER_DEFINE_MEMBERS(GenericDetailingSymbolHandler)
    HANDLER_DEFINE_MEMBERS(GenericCalloutHandler)
    HANDLER_DEFINE_MEMBERS(GenericViewAttachmentLabelHandler)
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
GenericDomain::GenericDomain() : DgnDomain(GENERIC_DOMAIN_NAME, "Generic Domain", 1) 
    {
    RegisterHandler(generic_ModelHandler::GenericGroupModelHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericGroupHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericSpatialLocationHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericGraphic3dHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericDetailingSymbolHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericCalloutHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericViewAttachmentLabelHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
GenericDomain::~GenericDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                  02 / 2017
//---------------------------------------------------------------------------------------
static BeFileName getSchemaPathname()
    {
    BeFileName genericDomainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    genericDomainSchemaFile.AppendToPath(GENERIC_DOMAIN_ECSCHEMA_PATH);
    BeAssert(genericDomainSchemaFile.DoesPathExist());

    return genericDomainSchemaFile;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                  02 / 2017
//---------------------------------------------------------------------------------------
DbResult GenericDomain::ValidateSchema(DgnDbR db)
    {
    DgnDomainCR dgnDomain = GenericDomain::GetDomain();
    return dgnDomain.ValidateSchema(db, getSchemaPathname());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                  02 / 2017
//---------------------------------------------------------------------------------------
DgnDbStatus GenericDomain::UpgradeSchema(DgnDbR db)
    {
    DgnDomainCR dgnDomain = GenericDomain::GetDomain();
    DgnDbStatus status = dgnDomain.UpgradeSchema(db, getSchemaPathname());
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus GenericDomain::ImportSchema(DgnDbR db)
    {
    DgnDomainCR dgnDomain = GenericDomain::GetDomain();
    DgnDbStatus status = dgnDomain.ImportSchema(db, getSchemaPathname());
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/17
//---------------------------------------------------------------------------------------
static void createGenericCodeSpecs(DgnDbR db)
    {
    CodeSpecPtr disciplineCodeSpec = CodeSpec::Create(db, GENERIC_CODESPEC_ViewAttachmentLabel, CodeScopeSpec::CreateModelScope());
    if (disciplineCodeSpec.IsValid())
        disciplineCodeSpec->Insert();

    BeAssert(db.CodeSpecs().QueryCodeSpecId(GENERIC_CODESPEC_ViewAttachmentLabel).IsValid());    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/17
//---------------------------------------------------------------------------------------
void GenericDomain::_OnSchemaImported(DgnDbR db) const
    {
    createGenericCodeSpecs(db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2016
//---------------------------------------------------------------------------------------
GenericPhysicalObjectPtr GenericPhysicalObject::Create(PhysicalModelR model, DgnCategoryId categoryId)
    {
    DgnClassId classId = model.GetDgnDb().Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());

    if (!classId.IsValid() || !categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new GenericPhysicalObject(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
GenericSpatialLocationPtr GenericSpatialLocation::Create(PhysicalModelR model, DgnCategoryId categoryId)
    {
    DgnClassId classId = model.GetDgnDb().Domains().GetClassId(generic_ElementHandler::GenericSpatialLocationHandler::GetHandler());

    if (!classId.IsValid() || !categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new GenericSpatialLocation(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
GenericGroupPtr GenericGroup::Create(GenericGroupModelR model, DgnCodeCR code)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::GenericGroupHandler::GetHandler());

    if (!classId.IsValid())
        return nullptr;

    return new GenericGroup(CreateParams(db, modelId, classId, code));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
GenericGroupModelPtr GenericGroupModel::Create(DgnElementCR modeledElement)
    {
    ModelHandlerR handler = generic_ModelHandler::GenericGroupModelHandler::GetHandler();
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
// @bsimethod                                   Shaun.Sewall                    09/2016
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
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
DgnDbStatus GenericGroupModel::_OnInsertElement(DgnElementR element)
    {
    if (nullptr != dynamic_cast<GroupInformationElementP>(&element))
        return T_Super::_OnInsertElement(element);

    BeAssert(false);
    return DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson  02/17
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
* @bsimethod                                                    Sam.Wilson  02/17
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
* @bsimethod                                                    Sam.Wilson  02/17
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
* @bsimethod                                                    Sam.Wilson  02/17
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
* @bsimethod                                                    Sam.Wilson  02/17
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
