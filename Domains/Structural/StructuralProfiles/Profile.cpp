#include <StructuralDomain/StructuralProfiles/Profile.h>


HANDLER_DEFINE_MEMBERS(ProfileHandler)

/*ProfilePtr Profile::Create(Structural::StructuralTypeDefinitionModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new Profile(createParams);
    }*/

