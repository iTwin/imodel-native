/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ConcreteMaterialProperties.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralPhysicalDefinitions.h"

#ifdef _EXCLUDED_FROM_EAP_BUILD_

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE


//=======================================================================================
//! Base class for ConcreteMaterialProperties 
//! @ingroup StructuralMaterialGroup
//=======================================================================================
struct ConcreteMaterialProperties : Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_ConcreteMaterialProperties, Dgn::DgnElement::UniqueAspect)

private:


protected:
    //! Copy constructor
    ConcreteMaterialProperties(ConcreteMaterialPropertiesCR rhs);

    //! Empty constructor 
    ConcreteMaterialProperties() {}

    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el);
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(ConcreteMaterialProperties)

    //! Create a new ConcreteMaterialProperties 
    STRUCTURAL_DOMAIN_EXPORT static ConcreteMaterialPropertiesPtr Create();

    //! Create a new ConcreteMaterialProperties 
    STRUCTURAL_DOMAIN_EXPORT static ConcreteMaterialPropertiesPtr Create(double k1, double k2, double k3);

    STRUCTURAL_DOMAIN_EXPORT ConcreteMaterialPropertiesPtr Clone() const;

    //! Assignment operator
    STRUCTURAL_DOMAIN_EXPORT ConcreteMaterialProperties& operator= (ConcreteMaterialPropertiesCR rhs);

    //! Validates 
    STRUCTURAL_DOMAIN_EXPORT bool IsValid() const;
    STRUCTURAL_DOMAIN_EXPORT bool IsEqual(ConcreteMaterialPropertiesCR rhs) const;
    };

//=================================================================================
//! Handler for ConcreteMaterialProperties
//! @ingroup StructuralMaterialGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConcreteMaterialPropertiesHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_ConcreteMaterialProperties, ConcreteMaterialPropertiesHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

END_BENTLEY_STRUCTURAL_NAMESPACE

#endif