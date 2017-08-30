/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/Wall.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <StructuralDomain/StructuralCommon/StructuralCommonDefinitions.h>
#include <StructuralDomain/StructuralDomainApi.h>
#include "SurfaceMember.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Wall
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Wall : SurfaceMember
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Wall, SurfaceMember);
    
    friend struct WallHandler;

protected:
    explicit Wall(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(Wall)
    DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(Wall)

    STRUCTURAL_DOMAIN_EXPORT static WallPtr Create(Structural::StructuralPhysicalModelCPtr model);

    STRUCTURAL_DOMAIN_EXPORT WallCPtr Insert(Dgn::DgnDbStatus* insertStatus = nullptr);
    STRUCTURAL_DOMAIN_EXPORT WallCPtr Update(Dgn::DgnDbStatus* updateStatus = nullptr);
    };

//=======================================================================================
//! The ElementHandler for WallHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WallHandler : SurfaceMemberHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PHYSICAL_CLASS_Wall, Wall, WallHandler, SurfaceMemberHandler, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
