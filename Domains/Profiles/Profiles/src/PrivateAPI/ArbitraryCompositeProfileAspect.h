/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ArbitraryCompositeProfileAspect.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\ProfilesDefinitions.h>
#include <Profiles\ArbitraryCompositeProfile.h>

#define PRF_CLASS_ArbitraryCompositeProfileAspect                           "ArbitraryCompositeProfileAspect"
#define PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile              "SingleProfile"
#define PRF_PROP_ArbitraryCompositeProfileAspect_Offset                     "Offset"
#define PRF_PROP_ArbitraryCompositeProfileAspect_Rotation                   "Rotation"
#define PRF_PROP_ArbitraryCompositeProfileAspect_MirrorProfileAboutYAxis    "MirrorProfileAboutYAxis"
#define PRF_PROP_ArbitraryCompositeProfileAspect_MemberPriority             "MemberPriority"

PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS (ArbitraryCompositeProfileAspect)

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Aspect used by ArbitraryCompostiteProfile to reference SinglePerimeterProfile and
//! geometry transformations for it. This data structure cannot be implemented by a relationship
//! class because we allow multiple references to the same profile with different transformations
//! e.g. same LShapeProfile used to form a 4 angle profile
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct ArbitraryCompositeProfileAspect : Dgn::DgnElement::MultiAspect
    {
    DEFINE_T_SUPER (Dgn::DgnElement::MultiAspect);
    friend struct ArbitraryCompositeProfileAspectHandler;

    DGNASPECT_DECLARE_MEMBERS (PRF_SCHEMA_NAME, PRF_CLASS_ArbitraryCompositeProfileAspect, Dgn::DgnElement::MultiAspect);

private:
    PROFILES_EXPORT explicit ArbitraryCompositeProfileAspect() = default;
    PROFILES_EXPORT explicit ArbitraryCompositeProfileAspect (ArbitraryCompositeProfileComponent const& component);

protected:
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _LoadProperties (Dgn::DgnElementCR el) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties (Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _GetPropertyValue (ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _SetPropertyValue (Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCompositeProfileAspect)

    static ArbitraryCompositeProfileAspectPtr Create (ArbitraryCompositeProfileComponent const& component)
        {
        return new ArbitraryCompositeProfileAspect (component);
        }

public:
    Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
    DPoint2d offset = DPoint2d::From (0.0, 0.0);
    Angle rotation = Angle::FromRadians (0.0);
    bool mirrorAboutYAxis = false;
    int memberPriority = -1;

    }; // ArbitraryCompositeProfileAspect

//=======================================================================================
//! Handler for ArbitraryCompositeProfileAspect class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCompositeProfileAspectHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCompositeProfileAspect, ArbitraryCompositeProfileAspectHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_EXPORT);

    virtual RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() { return new ArbitraryCompositeProfileAspect(); }

    }; // ArbitraryCompositeProfileAspect

END_BENTLEY_PROFILES_NAMESPACE
