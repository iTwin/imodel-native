/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/StandardProfileAspect.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//! @defgroup GROUP_Aspects Profile Aspects
//! Aspects used to extend or attach additional information to Profiles

//=======================================================================================
//! 
//! @ingroup GROUP_Aspects
//=======================================================================================
struct StandardProfileAspect : Dgn::DgnElement::UniqueAspect
    {
    DEFINE_T_SUPER (Dgn::DgnElement::UniqueAspect);
    friend struct StandardProfileAspectHandler;

    DGNASPECT_DECLARE_MEMBERS (PRF_SCHEMA_NAME, PRF_CLASS_StandardProfileAspect, Dgn::DgnElement::UniqueAspect);

protected:
    virtual Dgn::DgnDbStatus _LoadProperties (Dgn::DgnElementCR el) override;
    virtual Dgn::DgnDbStatus _UpdateProperties (Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    virtual Dgn::DgnDbStatus _GetPropertyValue (ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    virtual Dgn::DgnDbStatus _SetPropertyValue (Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (StandardProfileAspect)
    PROFILES_EXPORT static StandardProfileAspectPtr Create (/*TODO: args*/);

    }; // StandardProfileAspect

//=======================================================================================
//! Handler for StandardProfileAspect class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StandardProfileAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS (PRF_CLASS_StandardProfileAspect, StandardProfileAspectHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_EXPORT);

    }; // StandardProfileAspectHandler

END_BENTLEY_PROFILES_NAMESPACE
