/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\Profile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//*
* Helper class used to manage CardinalPoints of a Profile.
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesCardinalPoints
    {
public:
    ProfilesCardinalPoints() = delete;

    static Dgn::DgnDbStatus AddStandardCardinalPoints (Profile& profile);
    static Dgn::DgnDbStatus UpdateStandardCardinalPoints (Profile& profile);
    static Dgn::DgnDbStatus AddCustomCardinalPoint (Profile& profile, CardinalPoint const& customCardinalPoint);
    static Dgn::DgnDbStatus RemoveCustomCardinalPoint (Profile& profile, Utf8String const& name);

    static bvector<CardinalPoint> GetCardinalPoints (Profile const& profile);
    static Dgn::DgnDbStatus GetCardinalPoint (Profile const& profile, StandardCardinalPoint standardType, CardinalPoint& cardinalPoint);
    static Dgn::DgnDbStatus GetCardinalPoint (Profile const& profile, Utf8String const& name, CardinalPoint& cardinalPoint);

    };

END_BENTLEY_PROFILES_NAMESPACE
