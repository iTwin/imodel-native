/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/window.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ArchitecturalPhysical\ArchitecturalPhysicalApi.h>
#include <BuildingPhysical\BuildingPhysicalApi.h>

BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Window : ArchitecturalBaseElement
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_Window, ArchitecturalBaseElement);
    friend struct WindowHandler;

    protected:
        explicit Window(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static WindowPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WindowHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_Window, Window, WindowHandler, ArchitecturalBaseElementHandler, ARCHITECTURAL_PHYSICAL_EXPORT)
    };


//=======================================================================================
//! The SmallSquare tile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WindowType : Dgn::PhysicalType
    {
    DGNELEMENT_DECLARE_MEMBERS(AP_CLASS_WindowType, Dgn::PhysicalType);
    friend struct WindowTypeHandler;

    protected:
        explicit WindowType(CreateParams const& params) : T_Super(params) {}

    public:
        ARCHITECTURAL_PHYSICAL_EXPORT static WindowTypePtr Create(BuildingPhysical::BuildingTypeDefinitionModelR);
    };

//=======================================================================================
//! The ElementHandler for SmallSquareTile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WindowTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(AP_CLASS_WindowType, WindowType, WindowTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, ARCHITECTURAL_PHYSICAL_EXPORT)
    };

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE





