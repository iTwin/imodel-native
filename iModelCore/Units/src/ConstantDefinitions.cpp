/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ConstantDefinitions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultConstants ()
    {
    AddConstant(LENGTH_RATIO, "PI", "ONE", PI); //, "Ratio of Circumference to its Diameter");
    AddConstant(LENGTH_RATIO, "PI/4", "PI", 0.25); //, "PI/4");
    AddConstant(LENGTH_RATIO, "PI/2", "PI", 0.5); //, "PI/2");
    AddConstant(LENGTH_RATIO, "2PI", "PI", 2); //, "2*PI");
    AddConstant(ANGLE, "360DEG", "ARC_DEG", 360); //, "Full Circle");
    AddConstant(ACCELERATION, "STD_G", "M*S(-2)", 9.80665); //, "Standard Gravity");

    //Decimal multiples
    AddConstant(METRIC_PREFIX, "DECI", "ONE", 1.0e-1); //, "DECI-prefix");
    AddConstant(METRIC_PREFIX, "CENTI", "ONE", 1.0e-2); //, "CENTI-prefix");
    AddConstant(METRIC_PREFIX, "MILLI", "ONE", 1.0e-3); //, "MILLI-prefix");
    AddConstant(METRIC_PREFIX, "MICRO", "ONE", 1.0e-6); //, "MICRO-prefix");
    AddConstant(METRIC_PREFIX, "NANO", "ONE", 1.0e-9); //, "NANO-prefix");
    AddConstant(METRIC_PREFIX, "PICO", "ONE", 1.0e-12); //, "PICO-prefix");
    AddConstant(METRIC_PREFIX, "FEMTO", "ONE", 1.0e-15); //, "FEMTO-prefix");
    AddConstant(METRIC_PREFIX, "ATTO", "ONE", 1.0e-18); //, "ATTO-prefix");
    AddConstant(METRIC_PREFIX, "ZEPTO", "ONE", 1.0e-21); //, "ZEPTO-prefix");
    AddConstant(METRIC_PREFIX, "YOCTO", "ONE", 1.0e-24); //, "YOCTO-prefix");
    AddConstant(METRIC_PREFIX, "DECA", "ONE", 1.0e1); //, "DECA-prefix");
    AddConstant(METRIC_PREFIX, "HECTO", "ONE", 1.0e2); //, "HECTO-prefix");
    AddConstant(METRIC_PREFIX, "KILO", "ONE", 1.0e3); //, "KILO-prefix");
    AddConstant(METRIC_PREFIX, "MEGA", "ONE", 1.0e6); //, "MEGA-prefix");
    AddConstant(METRIC_PREFIX, "GIGA", "ONE", 1.0e9); //, "GIGA-prefix");
    AddConstant(METRIC_PREFIX, "TERA", "ONE", 1.0e12); //, "TERA-prefix");
    AddConstant(METRIC_PREFIX, "PETA", "ONE", 1.0e15); //, "PETA-prefix");
    AddConstant(METRIC_PREFIX, "EXA", "ONE", 1.0e18); //, "EXA-prefix");
    AddConstant(METRIC_PREFIX, "ZETTA", "ONE", 1.0e21); //, "ZETTA-prefix");
    AddConstant(METRIC_PREFIX, "YOTTA", "ONE", 1.0e24); //, "YOTTA-prefix");
    }
