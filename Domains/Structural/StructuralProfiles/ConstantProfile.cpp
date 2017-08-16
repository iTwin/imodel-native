#include <StructuralDomain/StructuralProfiles/ConstantProfile.h>

HANDLER_DEFINE_MEMBERS(ConstantProfileHandler)

ConstantProfilePtr ConstantProfile::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new ConstantProfile(createParams);
    }
