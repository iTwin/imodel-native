#include <StructuralDomain\StructuralProfiles\VaryingProfileByZone.h>

HANDLER_DEFINE_MEMBERS(VaryingProfileByZoneHandler)

VaryingProfileByZonePtr VaryingProfileByZone::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new VaryingProfileByZone(createParams);
    }
