/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8OpenRoadsDesignerDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnV8OpenRoadsDesignerInternal.h"
#include <DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesignerDomain.h>

#include <DgnV8OpenRoadsDesigner/Handlers.h>

DOMAIN_DEFINE_MEMBERS(DgnV8OpenRoadsDesignerDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8OpenRoadsDesignerDomain::DgnV8OpenRoadsDesignerDomain() : DgnDomain(V8ORD_SCHEMA_NAME, "Bentley DgnV8OpenRoadsDesigner Domain", 1)
    {
    RegisterHandler(CorridorSurfaceAspectHandler::GetHandler());
    RegisterHandler(FeatureAspectHandler::GetHandler());
    }
