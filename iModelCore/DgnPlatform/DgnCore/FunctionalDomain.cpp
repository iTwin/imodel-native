/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FunctionalDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/FunctionalDomain.h>

//=======================================================================================
//  Handler definitions
//=======================================================================================
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

DOMAIN_DEFINE_MEMBERS(FunctionalDomain)

namespace func_ModelHandler
    {
    HANDLER_DEFINE_MEMBERS(Functional)
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
FunctionalDomain::FunctionalDomain() : DgnDomain(FUNCTIONAL_DOMAIN_NAME, "Functional Domain", 1) 
    {
    RegisterHandler(func_ModelHandler::Functional::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
FunctionalDomain::~FunctionalDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalDomain::ImportSchema(DgnDbR db)
    {
    BeFileName domainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    domainSchemaFile.AppendToPath(FUNCTIONAL_DOMAIN_ECSCHEMA_PATH);
    BeAssert(domainSchemaFile.DoesPathExist());

    DgnDomainR domain = FunctionalDomain::GetDomain();
    DgnDbStatus importSchemaStatus = domain.ImportSchema(db, domainSchemaFile, ImportSchemaOptions::ImportOnly);
    BeAssert(DgnDbStatus::Success == importSchemaStatus);
    return importSchemaStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalModel::_OnInsertElement(DgnElementR element)
    {
    // FunctionModels can *only* contain FunctionalElements
    FunctionalElementP funcElement = dynamic_cast<FunctionalElementP>(&element);
    return (nullptr == funcElement) ? DgnDbStatus::WrongModel : T_Super::_OnInsertElement(element);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalElement::_OnInsert()
    {
    // FunctionalElements can reside *only* in a FunctionalModel
    DgnModelPtr model = GetModel();
    FunctionalModelP funcModel = dynamic_cast<FunctionalModelP>(model.get());
    return (nullptr == funcModel) ? DgnDbStatus::WrongModel : T_Super::_OnInsert();
    }
