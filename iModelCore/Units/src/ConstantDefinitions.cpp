/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ConstantDefinitions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Units/UnitRegistry.h>

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultConstants ()
    {
    AddConstant(LENGTH_RATIO, CONSTANT, "PI", "ONE", PI); //, "Ratio of Circumference to its Diameter");
    AddConstant(LENGTH_RATIO, CONSTANT, "PI/4", "PI", 1.0, 4.0); //, "PI/4");
    AddConstant(LENGTH_RATIO, CONSTANT, "PI/2", "PI", 1.0, 2.0); //, "PI/2");
    AddConstant(LENGTH_RATIO, CONSTANT, "2PI", "PI", 2); //, "2*PI");
    AddConstant(ANGLE, CONSTANT, "360DEG", "ARC_DEG", 360); //, "Full Circle");
    AddConstant(ACCELERATION, CONSTANT, "STD_G", "M*S(-2)", 9.80665); //, "Standard Gravity");

    //Decimal multiples
    AddConstant(METRIC_PREFIX, CONSTANT, "DECI", "ONE", 1.0e-1); //, "DECI-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "CENTI", "ONE", 1.0e-2); //, "CENTI-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "MILLI", "ONE", 1.0e-3); //, "MILLI-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "MICRO", "ONE", 1.0e-6); //, "MICRO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "NANO", "ONE", 1.0e-9); //, "NANO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "PICO", "ONE", 1.0e-12); //, "PICO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "FEMTO", "ONE", 1.0e-15); //, "FEMTO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "ATTO", "ONE", 1.0e-18); //, "ATTO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "ZEPTO", "ONE", 1.0e-21); //, "ZEPTO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "YOCTO", "ONE", 1.0e-24); //, "YOCTO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "DECA", "ONE", 1.0e1); //, "DECA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "HECTO", "ONE", 1.0e2); //, "HECTO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "KILO", "ONE", 1.0e3); //, "KILO-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "MEGA", "ONE", 1.0e6); //, "MEGA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "GIGA", "ONE", 1.0e9); //, "GIGA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "TERA", "ONE", 1.0e12); //, "TERA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "PETA", "ONE", 1.0e15); //, "PETA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "EXA", "ONE", 1.0e18); //, "EXA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "ZETTA", "ONE", 1.0e21); //, "ZETTA-prefix");
    AddConstant(METRIC_PREFIX, CONSTANT, "YOTTA", "ONE", 1.0e24); //, "YOTTA-prefix");
    }
