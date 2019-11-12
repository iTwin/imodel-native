/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI\Wall.h"
#include "PublicAPI\StructuralPhysicalDefinitions.h"

HANDLER_DEFINE_MEMBERS(WallHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WallPtr Wall::Create(Dgn::PhysicalModelCR model)
    {
    Dgn::DgnModelId modelId = model.GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
        {
        return nullptr;
        }

    Dgn::DgnDbR db = model.GetDgnDb();

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, Wall::MyHandlerECClassName());
    
    BeAssert(nullptr != structuralClass);
    
    if (nullptr == structuralClass)
        {
        return nullptr;
        }

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, structuralClass->GetDisplayLabel().c_str());

    BeAssert(categoryId.IsValid());

    CreateParams createParams(db, modelId, QueryClassId(db), categoryId);

    return new Wall(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WallCPtr Wall::Insert(Dgn::DgnDbStatus* insertStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(insertStatus, insertStatusOut);
    WallCPtr wall = GetDgnDb().Elements().Insert<Wall>(*this, &insertStatus);
    
    BeAssert(wall.IsValid());

    return wall;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WallCPtr Wall::Update(Dgn::DgnDbStatus* updateStatusOut)
    {
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(updateStatus, updateStatusOut);
    WallCPtr wall = GetDgnDb().Elements().Update<Wall>(*this, &updateStatus);

    BeAssert(wall.IsValid());

    return wall;
    }
