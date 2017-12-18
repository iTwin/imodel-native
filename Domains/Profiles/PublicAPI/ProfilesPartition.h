#pragma once

//__PUBLISH_SECTION_START__
#include "ProfilesDomainDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A ProfilesPartition provides a starting point for a ProfilesModel hierarchy
//! @note ProfilesPartition elements only reside in the RepositoryModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfilesPartition : Dgn::DefinitionPartition
{
    DGNELEMENT_DECLARE_MEMBERS(PROFILES_ProfilesPartition, Dgn::DefinitionPartition);
    friend struct ProfilesPartitionHandler;

protected:
    PROFILES_DOMAIN_EXPORT Dgn::DgnDbStatus _OnSubModelInsert(Dgn::DgnModelCR model) const override;
    explicit ProfilesPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new ProfilesPartition
    //! @param[in] parentSubject The new ProfilesPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this ProfilesPartition
    //! @see DgnElements::GetRootSubject
    PROFILES_DOMAIN_EXPORT static ProfilesPartitionPtr Create(Dgn::SubjectCR parentSubject, Utf8CP name, Utf8CP description=nullptr);
    //! Create and insert a new ProfilesPartition
    //! @param[in] parentSubject The new ProfilesPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this ProfilesPartition
    //! @see DgnElements::GetRootSubject
    PROFILES_DOMAIN_EXPORT static ProfilesPartitionCPtr CreateAndInsert(Dgn::SubjectCR parentSubject, Utf8CP name, Utf8CP description = nullptr);
};

struct EXPORT_VTABLE_ATTRIBUTE ProfilesPartitionHandler : Dgn::dgn_ElementHandler::DefinitionPartition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PROFILES_ProfilesPartition, ProfilesPartition, ProfilesPartitionHandler, Dgn::dgn_ElementHandler::DefinitionPartition, PROFILES_DOMAIN_EXPORT)
    };

END_BENTLEY_PROFILES_NAMESPACE
