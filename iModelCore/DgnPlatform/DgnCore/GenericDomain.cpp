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

namespace generic_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(GenericSpatialGroupHandler)
    HANDLER_DEFINE_MEMBERS(GenericGraphicGroup2dHandler)
    HANDLER_DEFINE_MEMBERS(GenericSpatialLocationHandler)
    HANDLER_DEFINE_MEMBERS(GenericPhysicalObjectHandler)
    HANDLER_DEFINE_MEMBERS(GenericGraphic3dHandler)
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
GenericDomain::GenericDomain() : DgnDomain(GENERIC_DOMAIN_NAME, "Generic Domain", 1) 
    {
    RegisterHandler(generic_ElementHandler::GenericSpatialGroupHandler::GetHandler());
    RegisterHandler(generic_ElementHandler::GenericGraphicGroup2dHandler::GetHandler());
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
DgnDbStatus GenericDomain::ImportSchema(DgnDbR db, ImportSchemaOptions options)
    {
    BeFileName genericDomainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    genericDomainSchemaFile.AppendToPath(GENERIC_DOMAIN_ECSCHEMA_PATH);
    BeAssert(genericDomainSchemaFile.DoesPathExist());

    DgnDomainR genericDomain = GenericDomain::GetDomain();
    DgnDbStatus importSchemaStatus = genericDomain.ImportSchema(db, genericDomainSchemaFile, options);
    BeAssert(DgnDbStatus::Success == importSchemaStatus);
    return importSchemaStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2016
//---------------------------------------------------------------------------------------
GenericPhysicalObjectPtr GenericPhysicalObject::Create(SpatialModelR model, DgnCategoryId categoryId)
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
// @bsimethod                                   Shaun.Sewall                    05/2016
//---------------------------------------------------------------------------------------
GenericSpatialGroupPtr GenericSpatialGroup::Create(DgnDbR db, DgnCode const& code)
    {
    DgnModelId modelId = DgnModel::GroupInformationId();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::GenericSpatialGroupHandler::GetHandler());

    if (!classId.IsValid())
        return nullptr;

    return new GenericSpatialGroup(CreateParams(db, modelId, classId, code));
    }
