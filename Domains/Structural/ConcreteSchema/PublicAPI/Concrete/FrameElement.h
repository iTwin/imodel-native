/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/FrameElement.h $
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
struct EXPORT_VTABLE_ATTRIBUTE FrameElement : ConcreteElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_FrameElement, ConcreteElement);
    //friend struct FrameElementHandler;

    protected:
        explicit FrameElement(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static FrameElementPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
//struct EXPORT_VTABLE_ATTRIBUTE FrameElementHandler : Dgn::dgn_ElementHandler::Physical
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_FrameElement, FrameElement, FrameElementHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
//    };


END_BENTLEY_CONCRETE_NAMESPACE
