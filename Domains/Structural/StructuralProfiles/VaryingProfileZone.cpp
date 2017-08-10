#include <StructuralDomain\StructuralProfiles\VaryingProfileZone.h>

HANDLER_DEFINE_MEMBERS(VaryingProfileZoneHandler)

VaryingProfileZonePtr VaryingProfileZone::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    // TODO: needs a real category, not a fake one just passed
    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), Dgn::DgnCategoryId());

    return new VaryingProfileZone(createParams);
    }
