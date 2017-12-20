#include "ProfilesTestUtils.h"


Profiles::ProfilesPartitionPtr ProfilesTestUtils::CreateProfilesPartition()
    {
    return nullptr;
    }

Profiles::ProfileDefinitionModelPtr ProfilesTestUtils::GetProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (parentSubject.IsNull())
        parentSubject = db.Elements().GetRootSubject();

    Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildDefinitionModelCode(modelCodeName));
    Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    Dgn::DefinitionPartitionCPtr partition = db.Elements().Get<Dgn::DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;

    return dynamic_cast<Profiles::ProfileDefinitionModelP>(partition->GetSubModel().get());
    }

Utf8String ProfilesTestUtils::BuildDefinitionModelCode(Utf8StringCR modelCodeName)
    {
    return modelCodeName + ":ProfilesDefinition";
    }

Profiles::ProfileDefinitionModelPtr ProfilesTestUtils::CreateProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (parentSubject.IsNull())
        parentSubject = db.Elements().GetRootSubject();

    Utf8String defModelCode = BuildDefinitionModelCode(modelCodeName);

    Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*parentSubject, defModelCode);

    if (!defPartition.IsValid())
        return nullptr;

    Profiles::ProfileDefinitionModelPtr model = Profiles::ProfileDefinitionModel::Create(*defPartition);

    return model;
    }
