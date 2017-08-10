/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/FoundationMember.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain\StructuralPhysical\FoundationMember.h>

HANDLER_DEFINE_MEMBERS(FoundationMemberHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
FoundationMemberPtr FoundationMember::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), Dgn::DgnCategoryId());

    return new FoundationMember(createParams);
    }

