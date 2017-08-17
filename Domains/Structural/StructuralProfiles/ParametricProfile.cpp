#include <StructuralDomain/StructuralProfiles/ParametricProfile.h>

HANDLER_DEFINE_MEMBERS(ParametricProfileHandler)

ParametricProfilePtr ParametricProfile::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new ParametricProfile(createParams);
    }
