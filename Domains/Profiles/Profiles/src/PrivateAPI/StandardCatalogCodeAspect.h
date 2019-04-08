/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/StandardCatalogCodeAspect.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\Profile.h>

#define PRF_CLASS_StandardCatalogCodeAspect                         "StandardCatalogCodeAspect"
#define PRF_PROP_StandardCatalogCodeAspect_Manufacturer             "Manufacturer"
#define PRF_PROP_StandardCatalogCodeAspect_StandardsOrganization    "StandardsOrganization"
#define PRF_PROP_StandardCatalogCodeAspect_Revision                 "Revision"

PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS (StandardCatalogCodeAspect)

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Aspect used to store information about standard catalog profiles.
//! @private
//=======================================================================================
struct StandardCatalogCodeAspect : Dgn::DgnElement::UniqueAspect
    {
    DEFINE_T_SUPER (Dgn::DgnElement::UniqueAspect);
    friend struct StandardCatalogCodeAspectHandler;

    DGNASPECT_DECLARE_MEMBERS (PRF_SCHEMA_NAME, PRF_CLASS_StandardCatalogCodeAspect, Dgn::DgnElement::UniqueAspect);

private:
    PROFILES_EXPORT explicit StandardCatalogCodeAspect() = default;
    PROFILES_EXPORT explicit StandardCatalogCodeAspect (StandardCatalogCode const& code);

protected:
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _LoadProperties (Dgn::DgnElementCR el) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties (Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _GetPropertyValue (ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _SetPropertyValue (Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (StandardCatalogCodeAspect)

    static StandardCatalogCodeAspectCPtr Get (Profile const& profile);
    static StandardCatalogCodeAspectPtr GetForEdit (Profile& profile);

    static StandardCatalogCodeAspectPtr Create (StandardCatalogCode const& code)
        {
        return new StandardCatalogCodeAspect (code);
        }

public:
    Utf8String manufacturer;
    Utf8String standardsOrganization;
    Utf8String revision;

    }; // StandardCatalogCodeAspect

//=======================================================================================
//! Handler for StandardCatalogCodeAspect class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StandardCatalogCodeAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS (PRF_CLASS_StandardCatalogCodeAspect, StandardCatalogCodeAspectHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_EXPORT);

    virtual RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() { return new StandardCatalogCodeAspect(); }

    }; // StandardCatalogCodeAspect

END_BENTLEY_PROFILES_NAMESPACE
