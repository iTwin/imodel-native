/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterial/PublicAPI/StructuralMaterial/MaterialProperties.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralMaterialDefinitions.h"
#include <DgnPlatform/DgnCoreAPI.h>

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE


//=======================================================================================
//! Base class for MaterialProperties 
//! @ingroup StructuralMaterialGroup
//=======================================================================================
struct MaterialProperties : Dgn::DgnElement::UniqueAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_MaterialProperties, Dgn::DgnElement::UniqueAspect)

private:


protected:


    //! Copy constructor
    //MaterialProperties(MaterialPropertiesCR rhs);

    //virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;
    //virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;


public:
    // default constructor
    MaterialProperties();

    DECLARE_STRUCTURAL_MATERIAL_QUERYCLASS_METHODS(MaterialProperties)

    //! Create a new MaterialProperties 
    //STRUCTURAL_DOMAIN_EXPORT static MaterialPropertiesPtr Create();

    //STRUCTURAL_DOMAIN_EXPORT MaterialPropertiesPtr Clone() const;

    //! Assignment operator
    //STRUCTURAL_DOMAIN_EXPORT MaterialProperties& operator= (MaterialPropertiesCR rhs);

    //! Validates 
    //STRUCTURAL_DOMAIN_EXPORT bool IsValid() const;
    //STRUCTURAL_DOMAIN_EXPORT bool IsEqual(MaterialPropertiesCR rhs) const;
    };

//=================================================================================
//! Handler for MaterialProperties
//! @ingroup StructuralMaterialGroup
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MaterialPropertiesHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_MATERIAL_CLASS_MaterialProperties, MaterialPropertiesHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        //RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override;
    };


END_BENTLEY_STRUCTURAL_NAMESPACE

