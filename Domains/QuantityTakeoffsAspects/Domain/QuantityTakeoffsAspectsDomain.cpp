/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/QuantityTakeoffsAspectsDomain.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <QuantityTakeoffsAspects/Handlers/DimensionsAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/EnergyPerformanceAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/PerimeterAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/PileAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/PipeAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SideAreasAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SlabAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SlopeAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/ThicknessAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/VolumeAspectHandler.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(QuantityTakeoffsAspectsDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityTakeoffsAspectsDomain::QuantityTakeoffsAspectsDomain() : DgnDomain(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, "QuantityTakeoffsAspects Domain", 1)
    {
    RegisterHandler(DimensionsAspectHandler::GetHandler());
    RegisterHandler(EnergyPerformanceAspectHandler::GetHandler());
    RegisterHandler(PerimeterAspectHandler::GetHandler());
    RegisterHandler(PileAspectHandler::GetHandler());
    RegisterHandler(PipeAspectHandler::GetHandler());
    RegisterHandler(SideAreasAspectHandler::GetHandler());
    RegisterHandler(SlabAspectHandler::GetHandler());
    RegisterHandler(SlopeAspectHandler::GetHandler());
    RegisterHandler(ThicknessAspectHandler::GetHandler());
    RegisterHandler(VolumeAspectHandler::GetHandler());
    }
    
END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
