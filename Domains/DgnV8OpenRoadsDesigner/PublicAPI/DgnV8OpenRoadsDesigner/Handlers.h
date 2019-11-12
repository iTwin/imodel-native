/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnV8OpenRoadsDesigner.h"

#include "Aspects.h"

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//=======================================================================================
//! Handler for CorridorAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CorridorAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_CorridorAspect, CorridorAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new CorridorAspect(); }
}; // CorridorAspectHandler

//=======================================================================================
//! Handler for CorridorSurfaceAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CorridorSurfaceAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_CorridorSurfaceAspect, CorridorSurfaceAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new CorridorSurfaceAspect(); }
}; // CorridorSurfaceAspectHandler

//=======================================================================================
//! Handler for DiscreteQuantityAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DiscreteQuantityAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_DiscreteQuantityAspect, DiscreteQuantityAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new DiscreteQuantityAspect(); }
}; // DiscreteQuantityAspectHandler

//=======================================================================================
//! Handler for FeatureAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FeatureAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_FeatureAspect, FeatureAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new FeatureAspect(); }
}; // FeatureAspectHandler

//=======================================================================================
//! Handler for LinearQuantityAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearQuantityAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_LinearQuantityAspect, LinearQuantityAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new LinearQuantityAspect(); }
}; // LinearQuantityAspectHandler

//=======================================================================================
//! Handler for StationRangeAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StationRangeAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_StationRangeAspect, StationRangeAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new StationRangeAspect(); }
}; // StationRangeAspectHandler

//=======================================================================================
//! Handler for SuperelevationAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SuperelevationAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_SuperelevationAspect, SuperelevationAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new SuperelevationAspect(); }
}; // SuperelevationAspectHandler

//=======================================================================================
//! Handler for TemplateDropAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TemplateDropAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_TemplateDropAspect, TemplateDropAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new TemplateDropAspect(); }
}; // TemplateDropAspectHandler

//=======================================================================================
//! Handler for VolumetricQuantityAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VolumetricQuantityAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_VolumetricQuantityAspect, VolumetricQuantityAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new VolumetricQuantityAspect(); }
}; // VolumetricQuantityAspectHandler

//=======================================================================================
//! Handler for AlignmentAspect class
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(V8ORD_CLASS_AlignmentAspect, AlignmentAspectHandler, Dgn::dgn_AspectHandler::Aspect, DGNV8OPENROADSDESIGNER_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new AlignmentAspect(); }
}; // AlignmentAspectHandler

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
