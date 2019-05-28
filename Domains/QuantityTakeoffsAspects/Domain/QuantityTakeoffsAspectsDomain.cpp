/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/QuantityTakeoffsAspectsDomain.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <QuantityTakeoffsAspects/Handlers/PerimeterAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/PileAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SideAreasAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SlabAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/SlopeAspectHandler.h>
#include <QuantityTakeoffsAspects/Handlers/ThicknessAspectHandler.h>

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
    RegisterHandler(PerimeterAspectHandler::GetHandler());
    RegisterHandler(PileAspectHandler::GetHandler());
    RegisterHandler(SideAreasAspectHandler::GetHandler());
    RegisterHandler(SlabAspectHandler::GetHandler());
    RegisterHandler(SlopeAspectHandler::GetHandler());
    RegisterHandler(ThicknessAspectHandler::GetHandler());
    }
    
END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
