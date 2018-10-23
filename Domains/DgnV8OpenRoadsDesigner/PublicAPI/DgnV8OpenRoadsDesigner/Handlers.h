/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnV8OpenRoadsDesigner/Handlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DgnV8OpenRoadsDesigner.h"

#include "Aspects.h"

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

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

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
