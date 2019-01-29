/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CustomCardinalPointsAspect.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CustomCardinalPoint
    {
public:
    CustomCardinalPoint (Utf8CP pName, DPoint2d const& location)
        : name (pName), location (location)
        {}

public:
    Utf8String name;
    DPoint2d location;

    }; // CustomCardinalPoint

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CustomCardinalPointsAspect : Dgn::DgnElement::UniqueAspect
    {
    DEFINE_T_SUPER (Dgn::DgnElement::UniqueAspect);
    friend struct CustomCardinalPointsAspectHandler;

    DGNASPECT_DECLARE_MEMBERS (PRF_SCHEMA_NAME, PRF_CLASS_CustomCardinalPointsAspect, Dgn::DgnElement::UniqueAspect);

protected:
    virtual Dgn::DgnDbStatus _LoadProperties (Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _UpdateProperties (Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    virtual Dgn::DgnDbStatus _GetPropertyValue (ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    virtual Dgn::DgnDbStatus _SetPropertyValue (Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CustomCardinalPointsAspect)
    PROFILES_EXPORT static CustomCardinalPointsAspectPtr Create (/*TODO: args*/);

    }; // CustomCardinalPointsAspect

//=======================================================================================
//! Handler for CustomCardinalPointsAspect class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomCardinalPointsAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS (PRF_CLASS_CustomCardinalPointsAspect, CustomCardinalPointsAspectHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_EXPORT);

    }; // CustomCardinalPointsAspectHandler

END_BENTLEY_PROFILES_NAMESPACE