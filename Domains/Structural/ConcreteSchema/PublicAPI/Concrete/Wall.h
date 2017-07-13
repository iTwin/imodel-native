/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/Wall.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Concrete\ConcreteApi.h>

BEGIN_BENTLEY_CONCRETE_NAMESPACE


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Wall : SurfaceElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_Wall, SurfaceElement);
    //friend struct WallHandler;

    protected:
        explicit Wall(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static WallPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE WallHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_Wall, Wall, WallHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
