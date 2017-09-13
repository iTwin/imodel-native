/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/StructuralAddition.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain/StructuralPhysical/StructuralAddition.h>
#include <StructuralDomain/StructuralCommon/StructuralCommonDefinitions.h>

HANDLER_DEFINE_MEMBERS(StructuralAdditionHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
STRUCTURAL_DOMAIN_EXPORT StructuralAdditionPtr StructuralAddition::Create(Structural::StructuralPhysicalModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
        {
        return nullptr;
            }

    Dgn::DgnDbR db = model.get()->GetDgnDb();

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, StructuralAddition::MyHandlerECClassName());

    BeAssert(nullptr != structuralClass);

    if (nullptr == structuralClass)
        {
        return nullptr;
        }

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, structuralClass->GetDisplayLabel().c_str());

    BeAssert(categoryId.IsValid());

    CreateParams createParams(db, modelId, QueryClassId(db), categoryId);

    return new StructuralAddition(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralAdditionCPtr StructuralAddition::Insert(Dgn::DgnDbStatus* insertStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(insertStatus, insertStatusOut);
    StructuralAdditionCPtr addition = GetDgnDb().Elements().Insert<StructuralAddition>(*this, &insertStatus);

    BeAssert(addition.IsValid());

    return addition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralAdditionCPtr StructuralAddition::Update(Dgn::DgnDbStatus* updateStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(updateStatus, updateStatusOut);
    StructuralAdditionCPtr addition = GetDgnDb().Elements().Update<StructuralAddition>(*this, &updateStatus);

    BeAssert(addition.IsValid());

    return addition;
    }