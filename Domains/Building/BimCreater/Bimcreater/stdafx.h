// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <BuildingDomain/BuildingDomainApi.h>
#include <Grids/gridsApi.h>
#include <AecUnits/AecUnitsApi.h>

#define USE_PROTOTYPE

//#ifdef USE_PROTOTYPE
//    #include <PlantPrototype\CommonEquipmentFunctional.h>
//    #include <PlantPrototype\CommonPipingFunctionalBreakdown.h>
//    #include <PlantPrototype\CommonPipingPhysical.h>
//    #include <PlantPrototype\CommonPipingFunctional.h>
//#else
//    #include <Plant\PlantDomainApi.h>
//#endif
#include <Plant\PlantDomainApi.h>
#include <Plantprototype\PlantprototypeDomainApi.h>

#include <SpacePlanning\Domain\SpacePlanningDomain.h>
#include <SpacePlanning\Elements\ElementsApi.h>
#include <SpacePlanning\Handlers\HandlersApi.h>

#include "Geometry.h"
#include "DoorTools.h"



// TODO: reference additional headers your program requires here
