#include "ProfilesDomain\BuiltUpProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(BuiltUpProfileHandler)

BuiltUpProfilePtr BuiltUpProfile::Create(Profiles::ProfileDefinitionModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
    {
        return nullptr;
    }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new BuiltUpProfile(createParams);
    }
END_BENTLEY_PROFILES_NAMESPACE
