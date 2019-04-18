/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PhysicalRebarInternal.h"
#include <PhysicalRebar/PhysicalRebarDomain.h>

#include <PhysicalRebar/Handlers.h>

DOMAIN_DEFINE_MEMBERS(PhysicalRebarDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRebarDomain::PhysicalRebarDomain() : DgnDomain(SPR_SCHEMA_NAME, "Bentley PhysicalRebar Domain", 1)
    {
    RegisterHandler(IReinforcableHandler::GetHandler());
    RegisterHandler(RebarHandler::GetHandler());
    RegisterHandler(RebarTypeHandler::GetHandler());
    RegisterHandler(RebarAccessoryHandler::GetHandler());
    RegisterHandler(RebarAccessoryTypeHandler::GetHandler());
    RegisterHandler(RebarAssemblyHandler::GetHandler());
    RegisterHandler(RebarEndDeviceHandler::GetHandler());
    RegisterHandler(RebarEndDeviceTypeHandler::GetHandler());
    RegisterHandler(RebarEndTreatmentHandler::GetHandler());
    RegisterHandler(RebarLayoutTypeHandler::GetHandler());
    RegisterHandler(RebarMaterialHandler::GetHandler());
    RegisterHandler(RebarMechanicalSpliceHandler::GetHandler());
    RegisterHandler(RebarMechanicalSpliceTypeHandler::GetHandler());
    RegisterHandler(RebarSetHandler::GetHandler());
    RegisterHandler(RebarSizeHandler::GetHandler());
    RegisterHandler(RebarSplicedEndHandler::GetHandler());
    RegisterHandler(RebarTerminatorHandler::GetHandler());
    RegisterHandler(RebarTerminatorTypeHandler::GetHandler());
    }
