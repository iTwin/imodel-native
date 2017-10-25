#include <StructuralDomain/StructuralProfiles/PublishedProfile.h>
#include <StructuralDomain/StructuralCommon/StructuralCommonDefinitions.h>

HANDLER_DEFINE_MEMBERS(PublishedProfileHandler)

PublishedProfilePtr PublishedProfile::Create(Structural::StructuralTypeDefinitionModelCPtr model)
    {
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    if (!model.get()->GetModelId().IsValid())
        {
        return nullptr;
        }

    CreateParams createParams(model.get()->GetDgnDb(), model.get()->GetModelId(), QueryClassId(model.get()->GetDgnDb()));

    return new PublishedProfile(createParams);
    }
