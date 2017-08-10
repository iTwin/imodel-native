/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/ConcreteSchema/PublicAPI/Concrete/Beam.h $
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
struct EXPORT_VTABLE_ATTRIBUTE Beam : FrameElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_Beam, FrameElement);
    //friend struct BeamHandler;

    protected:
        explicit Beam(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static BeamPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE BeamHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_Beam, Beam, BeamHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
