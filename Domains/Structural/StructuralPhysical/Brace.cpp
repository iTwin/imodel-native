/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/Brace.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain/StructuralPhysical/Brace.h>

HANDLER_DEFINE_MEMBERS(BraceHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BracePtr Brace::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), Dgn::DgnCategoryId());

    return new Brace(createParams);
    }

