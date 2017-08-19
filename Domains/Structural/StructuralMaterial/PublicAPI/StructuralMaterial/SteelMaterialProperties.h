/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterial/PublicAPI/StructuralMaterial/SteelMaterialProperties.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

//USING_NAMESPACE_BENTLEY_STRUCTURAL
#include "StructuralMaterialDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE


//=======================================================================================
//! Base class for SteelMaterialProperties 
//! @ingroup StructuralMaterialGroup
//=======================================================================================
struct SteelMaterialProperties : Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_SteelMaterialProperties, Dgn::DgnElement::UniqueAspect)

private:


protected:

    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el);
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }

public:
    //! Copy constructor
    SteelMaterialProperties(SteelMaterialPropertiesCR rhs);

    //! Empty constructor 
    SteelMaterialProperties() {}
    DECLARE_STRUCTURAL_MATERIAL_QUERYCLASS_METHODS(SteelMaterialProperties)

    //! Create a new SteelMaterialProperties 
    STRUCTURAL_DOMAIN_EXPORT static SteelMaterialPropertiesPtr Create();

    //! Create a new SteelMaterialProperties 
    STRUCTURAL_DOMAIN_EXPORT static SteelMaterialPropertiesPtr Create(double k1, double k2, double k3);

    STRUCTURAL_DOMAIN_EXPORT SteelMaterialPropertiesPtr Clone() const;

    //! Assignment operator
    STRUCTURAL_DOMAIN_EXPORT SteelMaterialProperties& operator= (SteelMaterialPropertiesCR rhs);

    //! Validates 
    STRUCTURAL_DOMAIN_EXPORT bool IsValid() const;
    STRUCTURAL_DOMAIN_EXPORT bool IsEqual(SteelMaterialPropertiesCR rhs) const;
    };

//=================================================================================
//! Handler for SteelMaterialProperties
//! @ingroup StructuralMaterialGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SteelMaterialPropertiesHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_MATERIAL_CLASS_SteelMaterialProperties, SteelMaterialPropertiesHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

END_BENTLEY_STRUCTURAL_NAMESPACE

