/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/QuantityTakeoffsAspectsDomain.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <QuantityTakeoffsAspects/Handlers/PileAspectHandler.h>

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
    RegisterHandler(PileAspectHandler::GetHandler());
    }
    
END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
