#include "PublicAPI\ProfilesDomain\ProfilesPartition.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(ProfilesPartitionHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                 11/2017
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus ProfilesPartition::_OnSubModelInsert(Dgn::DgnModelCR model) const
    {
    /*if (nullptr == dynamic_cast<ProfilesModelCP>(&model))
        {
        BeAssert(false && "A ProfilesPartition can only be modeled by a ProfilesModel");
        return Dgn::DgnDbStatus::ElementBlockedChange;
        }*/

    return T_Super::_OnSubModelInsert(model);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                 11/2017
//---------------------------------------------------------------------------------------
ProfilesPartitionPtr ProfilesPartition::Create(Dgn::SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    ProfilesPartitionPtr partition = nullptr;
    CreateParams createParams = InitCreateParams(parentSubject, label, ProfilesPartitionHandler::GetHandler());
    
    if (false != createParams.IsValid())
        {
        partition = new ProfilesPartition(createParams);
        if (description && *description)
            {
            partition->SetDescription(description);
            }
        }

    return partition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                 11/2017
//---------------------------------------------------------------------------------------
ProfilesPartitionCPtr ProfilesPartition::CreateAndInsert(Dgn::SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    ProfilesPartitionPtr partition = Create(parentSubject, label, description);
    
    if (!partition.IsValid())
        {
        return nullptr;
        }

    return parentSubject.GetDgnDb().Elements().Insert<ProfilesPartition>(*partition);
    }

END_BENTLEY_PROFILES_NAMESPACE
