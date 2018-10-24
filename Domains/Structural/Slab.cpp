/*--------------------------------------------------------------------------------------+
|
|     $Source: Slab.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI\Slab.h"

HANDLER_DEFINE_MEMBERS(SlabHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SlabPtr Slab::Create(Dgn::PhysicalModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), Dgn::DgnCategoryId());

    return new Slab(createParams);
    }

