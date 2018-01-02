/*--------------------------------------------------------------------------------------+
|
|     $Source: PileCap.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\PileCap.h"
#include "PublicAPI\StructuralPhysicalDefinitions.h"

HANDLER_DEFINE_MEMBERS(PileCapHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PileCapPtr PileCap::Create(Structural::StructuralPhysicalModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
        {
        return nullptr;
        }

    Dgn::DgnDbR db = model.get()->GetDgnDb();

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, PileCap::MyHandlerECClassName());

    BeAssert(nullptr != structuralClass);

    if (nullptr == structuralClass)
        {
        return nullptr;
        }

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, structuralClass->GetDisplayLabel().c_str());

    BeAssert(categoryId.IsValid());

    CreateParams createParams(db, modelId, QueryClassId(db), categoryId);

    return new PileCap(createParams);
    }
