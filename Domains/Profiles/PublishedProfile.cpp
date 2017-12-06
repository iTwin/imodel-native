#include "PublicAPI\ProfilesDomain\PublishedProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(PublishedProfileHandler)

PublishedProfilePtr PublishedProfile::Create(Profiles::ProfilesModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new PublishedProfile(createParams);
    }

END_BENTLEY_PROFILES_NAMESPACE
