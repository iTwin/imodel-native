/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/Column.h $
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
struct EXPORT_VTABLE_ATTRIBUTE Column : FrameElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CONCRETE_CLASS_Column, FrameElement);
    friend struct ColumnHandler;

    protected:
        explicit Column(CreateParams const& params) : T_Super(params) {}

    public:
        //CONCRETE_EXPORT static ColumnPtr Create(BuildingPhysical::BuildingPhysicalModelR);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ColumnHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(CONCRETE_CLASS_Column, Column, ColumnHandler, Dgn::dgn_ElementHandler::Physical, CONCRETE_EXPORT)
    };


END_BENTLEY_CONCRETE_NAMESPACE
