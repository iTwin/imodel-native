#include <StructuralDomain/StructuralProfiles/ParametricProfile.h>

HANDLER_DEFINE_MEMBERS(ParametricProfileHandler)

ParametricProfilePtr ParametricProfile::Create(Structural::StructuralTypeDefinitionModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new ParametricProfile(createParams);
    }
