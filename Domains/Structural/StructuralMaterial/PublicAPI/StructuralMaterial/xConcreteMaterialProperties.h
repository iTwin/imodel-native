/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterial/PublicAPI/StructuralMaterial/xConcreteMaterialProperties.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "MaterialProperties.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! ConcreteMaterialProperties
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConcreteMaterialProperties : MaterialProperties
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_ConcreteMaterialProperties, Dgn::DgnElement::UniqueAspect)

    //DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_ConcreteMaterialProperties, MaterialProperties);
    
    friend struct ConcreteMaterialPropertiesHandler;

protected:
    //explicit ConcreteMaterialProperties(CreateParams const& params) : T_Super(params) {}

    //Dgn::DgnDbStatus _OnDelete() const override { return Dgn::DgnDbStatus::Success; }

public:
    DECLARE_STRUCTURAL_MATERIAL_QUERYCLASS_METHODS(ConcreteMaterialProperties)

    };

//=======================================================================================
//! The ElementHandler for ConcreteMaterialProperties
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConcreteMaterialPropertiesHandler : MaterialPropertiesHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_MATERIAL_CLASS_ConcreteMaterialProperties, ConcreteMaterialPropertiesHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
