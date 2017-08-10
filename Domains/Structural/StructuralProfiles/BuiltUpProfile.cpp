#include <StructuralDomain\StructuralProfiles\BuiltUpProfile.h>

HANDLER_DEFINE_MEMBERS(BuiltUpProfileHandler)

BuiltUpProfilePtr BuiltUpProfile::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new BuiltUpProfile(createParams);
    }
