/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

// Standard header
#include <math.h>
#include <algorithm>

// Bentley Headers
#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Logging/bentleylogging.h>

// Bentley Standard Macros

#ifdef __UNITS_BUILD__
    #define UNITS_EXPORT EXPORT_ATTRIBUTE
#else
    #define UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_UNITS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Units {
#define END_BENTLEY_UNITS_NAMESPACE   } }
#define USING_NAMESPACE_BENTLEY_UNITS using namespace BENTLEY_NAMESPACE_NAME::Units;

#define UNITS_TYPEDEFS(_name_)  \
    BEGIN_BENTLEY_UNITS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_UNITS_NAMESPACE

// Units Public TypeDefs

UNITS_TYPEDEFS(Expression)
UNITS_TYPEDEFS(InverseUnit)
UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(IUnitsContext)
UNITS_TYPEDEFS(UnitsSymbol)
UNITS_TYPEDEFS(UnitSynonymMap)
UNITS_TYPEDEFS(UnitSystem)
UNITS_TYPEDEFS(Phenomenon)
UNITS_TYPEDEFS(SpecificAccuracy)

// All Units headers included in the PublicApi

#include <Units/UnitNameMappings.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/UnitsContext.h>

