/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\Column.h"
#include "PublicAPI\StructuralPhysicalDefinitions.h"

HANDLER_DEFINE_MEMBERS(ColumnHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ColumnPtr Column::Create(Dgn::PhysicalModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, STRUCTURAL_PHYSICAL_CLASS_Column);
    CreateParams createParams(db, model.GetModelId(), QueryClassId(db), categoryId);

    return new Column(createParams);
    }

