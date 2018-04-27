/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/ElementAspects.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! UniqueAspect to be applied to elements, caching the facet of the mesh/surface 
//! they are associated with.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct AssociatedFacetAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect)
    friend struct AssociatedFacetAspectHandler;

public:
    enum class AssociatedFacetEnum : int32_t { Internal = 0, Top = 1, Bottom = 2, TopAndBottom = 4 };

private:
    AssociatedFacetEnum m_associatedFacet;

protected:
    AssociatedFacetAspect() : m_associatedFacet(AssociatedFacetEnum::Internal) {}
    AssociatedFacetAspect(AssociatedFacetEnum associatedFacet) : m_associatedFacet(associatedFacet) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return BRRP_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return BRRP_CLASS_AssociatedFacetAspect; }
    //! @private
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }
    //! @private
    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    //! @private
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    //! @private
    virtual Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    //! @private
    virtual Dgn::DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(AssociatedFacetAspect)

    //! @private
    ROADRAILPHYSICAL_EXPORT static AssociatedFacetAspectPtr Create(AssociatedFacetEnum associatedFacet);

    //! Retrieve the AssociatedFacetAspect instance related to an element, if any
    //! @return An instance of AssociatedFacetAspect, or nullptr
    ROADRAILPHYSICAL_EXPORT static AssociatedFacetAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    ROADRAILPHYSICAL_EXPORT static AssociatedFacetAspectP GetP(Dgn::DgnElementR el);
    //! @private
    ROADRAILPHYSICAL_EXPORT static void Set(Dgn::DgnElementR el, AssociatedFacetAspectR aspect);

    //! Get the AssociatedFacet value for this instance
    AssociatedFacetEnum GetAssociatedFacet() const { return m_associatedFacet; }
    //! @private
    void SetAssociatedFacet(AssociatedFacetEnum associatedFacet) { m_associatedFacet = associatedFacet; }
}; // AssociatedFacetAspect


//__PUBLISH_SECTION_END__
//=======================================================================================
//! Handler for AssociatedFacetAspect
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AssociatedFacetAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(BRRP_CLASS_AssociatedFacetAspect, AssociatedFacetAspectHandler, Dgn::dgn_AspectHandler::Aspect, ROADRAILPHYSICAL_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new AssociatedFacetAspect(); }
}; // AssociatedFacetAspectHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE