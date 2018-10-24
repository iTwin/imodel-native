/*--------------------------------------------------------------------------------------+
|
|     $Source: Beam.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\Beam.h"
#include "PublicAPI\StructuralPhysicalDomain.h"

HANDLER_DEFINE_MEMBERS(BeamHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeamPtr Beam::Create(Dgn::PhysicalModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(model.GetDgnDb(), STRUCTURAL_PHYSICAL_CATEGORY_StructuralCategory);

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId);

    return new Beam(createParams);
    }

