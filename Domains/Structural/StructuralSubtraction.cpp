/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralPhysicalDefinitions.h"
#include "PublicApi\StructuralSubtraction.h"

#ifdef _EXCLUDED_FROM_EAP_BUILD_

HANDLER_DEFINE_MEMBERS(StructuralSubtractionHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
STRUCTURAL_DOMAIN_EXPORT StructuralSubtractionPtr StructuralSubtraction::Create(Structural::StructuralPhysicalModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
        {
        return nullptr;
        }

    Dgn::DgnDbR db = model.get()->GetDgnDb();

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, StructuralSubtraction::MyHandlerECClassName());

    BeAssert(nullptr != structuralClass);

    if (nullptr == structuralClass)
        {
        return nullptr;
        }

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, structuralClass->GetDisplayLabel().c_str());

    BeAssert(categoryId.IsValid());

    CreateParams createParams(db, modelId, QueryClassId(db), categoryId);

    return new StructuralSubtraction(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralSubtractionCPtr StructuralSubtraction::Insert(Dgn::DgnDbStatus* insertStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(insertStatus, insertStatusOut);
    StructuralSubtractionCPtr subtr = GetDgnDb().Elements().Insert<StructuralSubtraction>(*this, &insertStatus);

    BeAssert(subtr.IsValid());

    return subtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StructuralSubtractionCPtr StructuralSubtraction::Update(Dgn::DgnDbStatus* updateStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(updateStatus, updateStatusOut);
    StructuralSubtractionCPtr subtr = GetDgnDb().Elements().Update<StructuralSubtraction>(*this, &updateStatus);

    BeAssert(subtr.IsValid());

    return subtr;
    }
#endif