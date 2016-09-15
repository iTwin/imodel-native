/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GenericDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
GenericDomain::~GenericDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus GenericDomain::ImportSchema(DgnDbR db)
    {
    BeFileName genericDomainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    genericDomainSchemaFile.AppendToPath(GENERIC_DOMAIN_ECSCHEMA_PATH);
    BeAssert(genericDomainSchemaFile.DoesPathExist());

    DgnDomainR genericDomain = GenericDomain::GetDomain();
    DgnDbStatus importSchemaStatus = genericDomain.ImportSchema(db, genericDomainSchemaFile);
    BeAssert(DgnDbStatus::Success == importSchemaStatus);
    return importSchemaStatus;
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
GenericGroupPtr GenericGroup::Create(GenericGroupModelCR model, DgnCodeCR code)
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
GenericGroupModelPtr GenericGroupModel::Create(DgnElementCR modeledElement, DgnCodeCR code)
    {
    ModelHandlerR handler = generic_ModelHandler::GenericGroupModelHandler::GetHandler();
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(handler);
    if (!classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId(), code));
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
GenericGroupModelPtr GenericGroupModel::CreateAndInsert(DgnElementCR modeledElement, DgnCodeCR code)
    {
    GenericGroupModelPtr model = Create(modeledElement, code);
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
    if (nullptr != dynamic_cast<GenericGroupP>(&element))
        return T_Super::_OnInsertElement(element);

    BeAssert(false);
    return DgnDbStatus::WrongModel;
    }
