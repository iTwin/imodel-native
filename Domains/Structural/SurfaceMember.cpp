/*--------------------------------------------------------------------------------------+
|
|     $Source: SurfaceMember.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\SurfaceMember.h"

HANDLER_DEFINE_MEMBERS(SurfaceMemberHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceMemberPtr SurfaceMember::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), Dgn::DgnCategoryId());

    return new SurfaceMember(createParams);
    }

