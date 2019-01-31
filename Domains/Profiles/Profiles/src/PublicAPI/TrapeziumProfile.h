/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/TrapeziumProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TrapeziumProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TrapeziumProfile, ParametricProfile);
    friend struct TrapeziumProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(TrapeziumProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double topWidth, double bottomWidth,
                                               double depth, double topOffset);

    public:
        //! Required properties
        double topWidth = 0.0;
        double bottomWidth = 0.0;
        double depth = 0.0;
        double topOffset = 0.0;
        };

protected:
    explicit TrapeziumProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TrapeziumProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TrapeziumProfile)

    PROFILES_EXPORT static TrapeziumProfilePtr Create (CreateParams const& params) { return new TrapeziumProfile (params); }

public:
    PROFILES_EXPORT double GetTopWidth() const;
    PROFILES_EXPORT void SetTopWidth (double value);

    PROFILES_EXPORT double GetBottomWidth() const;
    PROFILES_EXPORT void SetBottomWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetTopOffset() const;
    PROFILES_EXPORT void SetTopOffset (double value);

    }; // TrapeziumProfile

//=======================================================================================
//! Handler for TrapeziumProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TrapeziumProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TrapeziumProfile, TrapeziumProfile, TrapeziumProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TrapeziumProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
