/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralSubtraction.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <StructuralDomain/StructuralCommon/StructuralCommonDefinitions.h>
#include <StructuralDomain/StructuralDomainApi.h>
#include "StructuralPhysicalDefinitions.h"

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
