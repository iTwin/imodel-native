#include "ProfilesDomain\ProfilesDomainUtilities.h"
#include "ProfilesDomain\ProfilesDomainDefinitions.h"
#include "ProfilesDomain\ProfilesModel.h"

BE_JSON_NAME(ProfilesDomain)

BEGIN_BENTLEY_PROFILES_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ProfilesDomainUtilities::RegisterDomainHandlers()
    {
    
    BentleyStatus registrationStatus = Dgn::DgnDomains::RegisterDomain(ProfilesDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    BeAssert(BentleyStatus::SUCCESS == registrationStatus);

    return registrationStatus;
    };

Utf8String ProfilesDomainUtilities::BuildDefinitionModelCode(Utf8StringCR modelCodeName)
    {
    return modelCodeName + ":ProfilesDefinition";
    }


ProfileDefinitionModelPtr ProfilesDomainUtilities::CreateProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (parentSubject.IsNull())
        parentSubject = db.Elements().GetRootSubject();

    Utf8String defModelCode = BuildDefinitionModelCode(modelCodeName);

    Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*parentSubject, defModelCode);

    if (!defPartition.IsValid())
        return nullptr;

    Profiles::ProfileDefinitionModelPtr model = ProfileDefinitionModel::Create(*defPartition);

    return model;
    }

ProfileDefinitionModelPtr ProfilesDomainUtilities::GetProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (parentSubject.IsNull())
        {
        parentSubject = db.Elements().GetRootSubject();
        }

    Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, ProfilesDomainUtilities::BuildDefinitionModelCode(modelCodeName));
    Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    Dgn::DefinitionPartitionCPtr partition = db.Elements().Get<Dgn::DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;

    return dynamic_cast<ProfileDefinitionModelP>(partition->GetSubModel().get());
    }



END_BENTLEY_PROFILES_NAMESPACE
