/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralPhysicalDefinitions.h"
#include <DgnPlatform/DgnCoreAPI.h>

#ifdef _EXCLUDED_FROM_EAP_BUILD_

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Base class for MaterialProperties 
//! @ingroup StructuralMaterialGroup
//=======================================================================================
struct MaterialProperties : Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_MaterialProperties, Dgn::DgnElement::UniqueAspect)

private:
    double m_elasticModulus;
    double m_poissonsRatio;
    double m_tensileStrength;
    double m_thermalExpansionCoefficient;

    protected:
        //! Constructor
        MaterialProperties(double elasticModulus, double poissonsRatio, double tensileStrength, double thermalExpansionCoefficient) :m_elasticModulus(elasticModulus), m_poissonsRatio(poissonsRatio), m_tensileStrength(tensileStrength), m_thermalExpansionCoefficient(thermalExpansionCoefficient) {}

        //! Copy constructor
        MaterialProperties(MaterialPropertiesCR rhs);

        //! Empty constructor 
        MaterialProperties() :m_elasticModulus(0), m_poissonsRatio(0), m_tensileStrength(0), m_thermalExpansionCoefficient(0) {}

        Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el);
        Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
        Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
        Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
        Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }


public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(MaterialProperties)

    //! Create a new MaterialProperties 
    STRUCTURAL_DOMAIN_EXPORT static MaterialPropertiesPtr Create();

    STRUCTURAL_DOMAIN_EXPORT static MaterialPropertiesPtr Create(double elasticModulus, double poissonsRatio, double tensileStrength, double thermalExpansionCoefficient);

    STRUCTURAL_DOMAIN_EXPORT MaterialPropertiesPtr Clone() const;

    //! Assignment operator
    STRUCTURAL_DOMAIN_EXPORT MaterialProperties& operator= (MaterialPropertiesCR rhs);

    //! Validates 
    STRUCTURAL_DOMAIN_EXPORT bool IsValid() const;
    STRUCTURAL_DOMAIN_EXPORT bool IsEqual(MaterialPropertiesCR rhs) const;

    double GetElasticModulus() const;
    double GetPoissonsRatio() const;
    double GetTensileStrength() const;
    double GetThermalExpansionCoefficient() const;
    void   SetElasticModulus(double val);
    void   SetPoissonsRatio(double val);
    void   SetTensileStrength(double val);
    void   SetThermalExpansionCoefficient(double val);
    };

//=================================================================================
//! Handler for MaterialProperties
//! @ingroup StructuralMaterialGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialPropertiesHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_MaterialProperties, MaterialPropertiesHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };

END_BENTLEY_STRUCTURAL_NAMESPACE

#endif 