/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/Slab.h $
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
struct EXPORT_VTABLE_ATTRIBUTE Slab : SurfaceElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_Slab, SurfaceElement);
    //friend struct SlabHandler;

    protected:
        explicit Slab(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static SlabPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE SlabHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_Slab, Slab, SlabHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
