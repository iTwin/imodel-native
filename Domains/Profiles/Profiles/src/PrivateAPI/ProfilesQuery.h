#/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ProfilesQuery.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\ProfilesDefinitions.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//*
* Helper class used to perform common ECSql queries on Profiles.
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesQuery
    {
public:
    ProfilesQuery() = delete;

    static Dgn::DgnElementId SelectFirstByAspectNavigationProperty (Dgn::DgnDb const& db, Dgn::DgnElementId const& referencedProfileId,
                                                                    Utf8CP pAspectName, Utf8CP pNavigationPropertyName, Dgn::DgnDbStatus* pStatus = nullptr);

    template<typename T>
    static bvector<RefCountedPtr<T>> SelectByAspectNavigationProperty (Dgn::DgnDb const& db, Dgn::DgnElementId const& referencedProfileId,
                                                                       Utf8CP pAspectName, Utf8CP pNavigationPropertyName, Dgn::DgnDbStatus* pStatus = nullptr);

    };

END_BENTLEY_PROFILES_NAMESPACE
