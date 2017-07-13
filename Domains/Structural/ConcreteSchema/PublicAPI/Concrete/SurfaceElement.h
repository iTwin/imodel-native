/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/SurfaceElement.h $
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
struct EXPORT_VTABLE_ATTRIBUTE SurfaceElement : ConcreteElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_SurfaceElement, ConcreteElement);
    //friend struct SurfaceElementHandler;

    protected:
        explicit SurfaceElement(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static SurfaceElementPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE SurfaceElementHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_SurfaceElement, SurfaceElement, SurfaceElementHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
