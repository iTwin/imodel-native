/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/Beam.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain/StructuralPhysical/Beam.h>
#include <StructuralDomain/StructuralPhysical/StructuralPhysicalDomain.h>

HANDLER_DEFINE_MEMBERS(BeamHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeamPtr Beam::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // TODO: needs a real category, not a fake one just passed
    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(model.GetDgnDb(), STRUCTURAL_PHYSICAL_CATEGORY_StructuralCategory);

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId);

    return new Beam(createParams);
    }

