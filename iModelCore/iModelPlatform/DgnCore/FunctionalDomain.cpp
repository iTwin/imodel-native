/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

namespace func_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(FunctionalPartitionHandler)
    HANDLER_DEFINE_MEMBERS(FunctionalBreakdownElementHandler)
    HANDLER_DEFINE_MEMBERS(FunctionalCompositeHandler)
    HANDLER_DEFINE_MEMBERS(FunctionalComponentElementHandler)
    HANDLER_DEFINE_MEMBERS(FunctionalPortionHandler)
    HANDLER_DEFINE_MEMBERS(FunctionalTypeHandler)
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalDomain::FunctionalDomain() : DgnDomain(FUNCTIONAL_DOMAIN_NAME, "Functional Domain", 1) 
    {
    RegisterHandler(func_ModelHandler::Functional::GetHandler());

    RegisterHandler(func_ElementHandler::FunctionalPartitionHandler::GetHandler());
    RegisterHandler(func_ElementHandler::FunctionalBreakdownElementHandler::GetHandler());
    RegisterHandler(func_ElementHandler::FunctionalCompositeHandler::GetHandler());
    RegisterHandler(func_ElementHandler::FunctionalComponentElementHandler::GetHandler());
    RegisterHandler(func_ElementHandler::FunctionalPortionHandler::GetHandler());
    RegisterHandler(func_ElementHandler::FunctionalTypeHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalDomain::~FunctionalDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    // A FunctionalPartition can only be modeled by a FunctionalModel
    return model.IsRoleModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalPartitionPtr FunctionalPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, func_ElementHandler::FunctionalPartitionHandler::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    FunctionalPartitionPtr partition = new FunctionalPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalPartitionCPtr FunctionalPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    FunctionalPartitionPtr partition = Create(parentSubject, name, description);
    if (!partition.IsValid())
        return nullptr;

    return parentSubject.GetDgnDb().Elements().Insert<FunctionalPartition>(*partition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalModelPtr FunctionalModel::Create(FunctionalPartitionCR modeledElement)
    {
    ModelHandlerR handler = func_ModelHandler::Functional::GetHandler();
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

    return dynamic_cast<FunctionalModelP>(model.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalModel::_OnInsertElement(DgnElementR element)
    {
    // FunctionModels can *only* contain FunctionalElements
    FunctionalElementP funcElement = dynamic_cast<FunctionalElementP>(&element);
    return (nullptr == funcElement) ? DgnDbStatus::WrongModel : T_Super::_OnInsertElement(element);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus FunctionalElement::_OnInsert()
    {
    // FunctionalElements can reside *only* in a FunctionalModel
    DgnModelPtr model = GetModel();
    FunctionalModelP funcModel = dynamic_cast<FunctionalModelP>(model.get());
    return (nullptr == funcModel) ? DgnDbStatus::WrongModel : T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalTypeCPtr FunctionalElement::GetFunctionalType() const
    {
    return GetDgnDb().Elements().Get<FunctionalType>(GetFunctionalTypeId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalCompositePtr FunctionalComposite::Create(FunctionalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(func_ElementHandler::FunctionalCompositeHandler::GetHandler());
    return new FunctionalComposite(CreateParams(db, model.GetModelId(), classId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FunctionalPortionPtr FunctionalPortion::Create(FunctionalModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(func_ElementHandler::FunctionalPortionHandler::GetHandler());
    return new FunctionalPortion(CreateParams(db, model.GetModelId(), classId));
    }
