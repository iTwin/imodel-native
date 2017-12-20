#include "ProfilesDomain\ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(ParametricProfileHandler)

ParametricProfilePtr ParametricProfile::Create(Profiles::ProfileDefinitionModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new ParametricProfile(createParams);
    }
END_BENTLEY_PROFILES_NAMESPACE
