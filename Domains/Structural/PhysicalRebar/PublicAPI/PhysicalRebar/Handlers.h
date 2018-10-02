/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/Handlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

#include "IReinforcable.h"
#include "Rebar.h"
#include "RebarAccessory.h"
#include "RebarAssembly.h"
#include "RebarEndDevice.h"
#include "RebarEndTreatment.h"
#include "RebarLayoutType.h"
#include "RebarMaterial.h"
#include "RebarMechanicalSplice.h"
#include "RebarSet.h"
#include "RebarSize.h"
#include "RebarSplicedEnd.h"
#include "RebarTerminator.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! Handler for Rebar class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_Rebar, Rebar, RebarHandler, Dgn::dgn_ElementHandler::Physical, PHYSICALREBAR_EXPORT)
}; // RebarHandler

//=======================================================================================
//! Handler for RebarAccessory class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarAccessoryHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarAccessory, RebarAccessory, RebarAccessoryHandler, Dgn::dgn_ElementHandler::Physical, PHYSICALREBAR_EXPORT)
}; // RebarAccessoryHandler

//=======================================================================================
//! Handler for RebarAccessoryType class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarAccessoryTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarAccessoryType, RebarAccessoryType, RebarAccessoryTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, PHYSICALREBAR_EXPORT)
}; // RebarAccessoryTypeHandler

//=======================================================================================
//! Handler for RebarAssembly class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarAssemblyHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarAssembly, RebarAssembly, RebarAssemblyHandler, Dgn::dgn_ElementHandler::Physical, PHYSICALREBAR_EXPORT)
}; // RebarAssemblyHandler

//=======================================================================================
//! Handler for RebarEndDevice class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarEndDeviceHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarEndDevice, RebarEndDevice, RebarEndDeviceHandler, Dgn::dgn_ElementHandler::Physical, PHYSICALREBAR_EXPORT)
}; // RebarEndDeviceHandler

//=======================================================================================
//! Handler for RebarEndDeviceType class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarEndDeviceTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarEndDeviceType, RebarEndDeviceType, RebarEndDeviceTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, PHYSICALREBAR_EXPORT)
}; // RebarEndDeviceTypeHandler

//=======================================================================================
//! Handler for RebarEndTreatment class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarEndTreatmentHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarEndTreatment, RebarEndTreatment, RebarEndTreatmentHandler, Dgn::dgn_ElementHandler::Definition, PHYSICALREBAR_EXPORT)
}; // RebarEndTreatmentHandler

//=======================================================================================
//! Handler for RebarMaterial class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarMaterialHandler : 
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarMaterial, RebarMaterial, RebarMaterialHandler, , PHYSICALREBAR_EXPORT)
}; // RebarMaterialHandler

//=======================================================================================
//! Handler for RebarMechanicalSplice class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarMechanicalSpliceHandler : RebarEndDeviceHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarMechanicalSplice, RebarMechanicalSplice, RebarMechanicalSpliceHandler, RebarEndDeviceHandler, PHYSICALREBAR_EXPORT)
}; // RebarMechanicalSpliceHandler

//=======================================================================================
//! Handler for RebarMechanicalSpliceType class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarMechanicalSpliceTypeHandler : RebarEndDeviceTypeHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarMechanicalSpliceType, RebarMechanicalSpliceType, RebarMechanicalSpliceTypeHandler, RebarEndDeviceTypeHandler, PHYSICALREBAR_EXPORT)
}; // RebarMechanicalSpliceTypeHandler

//=======================================================================================
//! Handler for RebarSet class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarSetHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarSet, RebarSet, RebarSetHandler, Dgn::dgn_ElementHandler::Physical, PHYSICALREBAR_EXPORT)
}; // RebarSetHandler

//=======================================================================================
//! Handler for RebarSize class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarSizeHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarSize, RebarSize, RebarSizeHandler, Dgn::dgn_ElementHandler::Definition, PHYSICALREBAR_EXPORT)
}; // RebarSizeHandler

//=======================================================================================
//! Handler for RebarSplicedEnd class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarSplicedEndHandler : 
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarSplicedEnd, RebarSplicedEnd, RebarSplicedEndHandler, , PHYSICALREBAR_EXPORT)
}; // RebarSplicedEndHandler

//=======================================================================================
//! Handler for RebarTerminator class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarTerminatorHandler : RebarEndDeviceHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarTerminator, RebarTerminator, RebarTerminatorHandler, RebarEndDeviceHandler, PHYSICALREBAR_EXPORT)
}; // RebarTerminatorHandler

//=======================================================================================
//! Handler for RebarTerminatorType class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarTerminatorTypeHandler : RebarEndDeviceTypeHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarTerminatorType, RebarTerminatorType, RebarTerminatorTypeHandler, RebarEndDeviceTypeHandler, PHYSICALREBAR_EXPORT)
}; // RebarTerminatorTypeHandler

//=======================================================================================
//! Handler for RebarType class
//! @ingroup GROUP_PhysicalRebar
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RebarTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
{
ELEMENTHANDLER_DECLARE_MEMBERS(SPR_CLASS_RebarType, RebarType, RebarTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, PHYSICALREBAR_EXPORT)
}; // RebarTypeHandler

END_BENTLEY_PHYSICALREBAR_NAMESPACE
