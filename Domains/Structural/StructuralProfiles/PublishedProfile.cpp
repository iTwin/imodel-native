#include <StructuralDomain/StructuralProfiles/PublishedProfile.h>

HANDLER_DEFINE_MEMBERS(PublishedProfileHandler)

PublishedProfilePtr PublishedProfile::Create(Dgn::PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new PublishedProfile(createParams);
    }
