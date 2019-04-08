/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/StructuralSubtraction.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
//#include "PublicApi\StructuralCommonDefinitions.h"
#include "StructuralDomainApi.h"
#include "StructuralPhysicalDefinitions.h"

#ifdef _EXCLUDED_FROM_EAP_BUILD_

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! StructuralElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralSubtraction : Dgn::SpatialLocationElement
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralSubtraction, Dgn::SpatialLocationElement);
    
    friend struct StructuralSubtractionHandler;

protected:
    explicit StructuralSubtraction(CreateParams const& params) : T_Super(params) {}

    Dgn::DgnDbStatus _OnDelete() const override { return Dgn::DgnDbStatus::Success; }

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(StructuralSubtraction)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(StructuralSubtraction)

    STRUCTURAL_DOMAIN_EXPORT static StructuralSubtractionPtr Create(Structural::StructuralPhysicalModelCPtr model);
    STRUCTURAL_DOMAIN_EXPORT StructuralSubtractionCPtr Insert(Dgn::DgnDbStatus* insertStatus = nullptr);
    STRUCTURAL_DOMAIN_EXPORT StructuralSubtractionCPtr Update(Dgn::DgnDbStatus* updateStatus = nullptr);
    };

//=======================================================================================
//! The ElementHandler for StructuralSubtractionHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StructuralSubtractionHandler : Dgn::dgn_ElementHandler::SpatialLocation
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_StructuralSubtraction, StructuralSubtraction, StructuralSubtractionHandler, Dgn::dgn_ElementHandler::SpatialLocation, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE

#endif