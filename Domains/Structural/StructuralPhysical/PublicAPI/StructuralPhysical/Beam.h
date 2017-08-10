/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/Beam.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CurveMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Beam
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Beam : CurveMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Beam, CurveMember);
    
    friend struct BeamHandler;

protected:
    explicit Beam(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Beam)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Beam)

    STRUCTURAL_DOMAIN_EXPORT static BeamPtr Create(Dgn::PhysicalModelR model);
    };

//=======================================================================================
//! The ElementHandler for BeamHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BeamHandler : CurveMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Beam, Beam, BeamHandler, CurveMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
