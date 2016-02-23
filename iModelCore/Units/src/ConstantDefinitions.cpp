/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ConstantDefinitions.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "StandardNames.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultConstants ()
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    reg.AddConstant(LENGTH, "EARTH_RAD", "M", 6.3781e6); //, "Radius Of Earth");
    reg.AddConstant(LENGTH, "ASTRO_UNIT", "M", 1.495978707e11); //, "Distance from Sun to Earth");
    reg.AddConstant(VELOCITY, "C", "M*S(-1)", 2.99792458e8); //, "Speed Of Light in Vaccum");
    reg.AddConstant(ONE, "PI", "ONE", PI); //, "Ratio of Circumference to its Diameter");
    reg.AddConstant(ONE, "PI/4", "[PI]", 0.25); //, "PI/4");
    reg.AddConstant(ONE, "PI/2", "[PI]", 0.5); //, "PI/2");
    reg.AddConstant(ONE, "PI2", "[PI]", 2); //, "2*PI");
    reg.AddConstant(ANGLE, "360DEG", "ARC_DEG", 360); //, "Full Circle");
    reg.AddConstant(ONE, "E", "ONE", E); //, "Base of the natural logarithm");
    reg.AddConstant(GRAVCONSTANT, "G0", "M(3)*KG(-1)*S(-2)", 6.67408e-11); //, "Gravitational constant");
    reg.AddConstant(ELECTRIC_CHARGE, "Q0", "A*S", 1.6021766208e-19);// , "Elementary Charge");
    reg.AddConstant(ACCELERATION, "STD_G", "M*S(-2)", 9.80665); //, "Standard Gravity");
    //reg.AddConstant(ACTION, "H", "J*S", 6.62607004e-34); //, "Planck constant"); // NOTE: Not used so removed instead of adding 'ACTION' phenomenon
    //reg.AddConstant(MOLE, "A0", "MOL(-1)", 6.022140857e23); //, "Avogadro Number"); // NOTE: Not actually MOLE but MOLE(-1), we don't use it anywhere so removed it
    // Densities of Water in KG/CUB.M
    reg.AddConstant(DENSITY, "H2O_0C", "KG*M(-3)", 0.99987e3); //, "Density of water at 0 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_4C", "KG*M(-3)", 1.00000e3); //, "Density of water at 4 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_10C", "KG*M(-3)", 0.99975e3); //, "Density of water at 10 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_20C", "KG*M(-3)", 0.99802e3); //, "Density of water at 20 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_30C", "KG*M(-3)", 0.9957e3); //, "Density of water at 30 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_40C", "KG*M(-3)", 0.9922e3); //, "Density of water at 40 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_50C", "KG*M(-3)", 0.9881e3); //, "Density of water at 50 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_60C", "KG*M(-3)", 0.98338e3); //, "Density of water at 60 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_70C", "KG*M(-3)", 0.97729e3); //, "Density of water at 70 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_80C", "KG*M(-3)", 0.97056e3); //, "Density of water at 80 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_90C", "KG*M(-3)", 0.9653e3); //, "Density of water at 90 degree Celsius");
    reg.AddConstant(DENSITY, "H2O_100C", "KG*M(-3)", 0.95865e3); //, "Density of water at 100 degree Celsius");
    // Densities of Water in LB/CUB.FT
    reg.AddConstant(DENSITY, "H2O_32F", "LBM*FT(-3)", 62.416); //, "Density of water at 32 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_39.2F", "LBM*FT(-3)", 62.424); //, "Density of water at 39.2 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_40F", "LBM*FT(-3)", 62.423); //, "Density of water at 40 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_50F", "LBM*FT(-3)", 62.408); //, "Density of water at 50 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_60F", "LBM*FT(-3)", 62.366); //, "Density of water at 60 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_70F", "LBM*FT(-3)", 62.300); //, "Density of water at 70 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_80F", "LBM*FT(-3)", 62.217); //, "Density of water at 80 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_90F", "LBM*FT(-3)", 62.118); //, "Density of water at 90 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_100F", "LBM*FT(-3)", 61.998); //, "Density of water at 100 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_120F", "LBM*FT(-3)", 61.719); //, "Density of water at 120 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_140F", "LBM*FT(-3)", 61.386); //, "Density of water at 140 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_160F", "LBM*FT(-3)", 61.006); //, "Density of water at 160 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_180F", "LBM*FT(-3)", 60.586); //, "Density of water at 180 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_200F", "LBM*FT(-3)", 60.135); //, "Density of water at 200 degree Fahrenheit");
    reg.AddConstant(DENSITY, "H2O_212F", "LBM*FT(-3)", 59.843); //, "Density of water at 212 degree Fahrenheit");

    reg.AddConstant(DENSITY, "HG_0C", "KG*M(-3)", 13.595e3); //, "Density of mercury 0 degree Celsius");
    reg.AddConstant(DENSITY, "HG_15C", "KG*M(-3)", 13.559e3); //, "Density of mercury 15 degree Celsius");
    reg.AddConstant(DENSITY, "HG_20C", "KG*M(-3)", 13.546e3); //, "Density of mercury 20 degree Celsius");
    reg.AddConstant(DENSITY, "HG_25C", "KG*M(-3)", 13.532e3); //, "Density of mercury 25 degree Celsius");
    reg.AddConstant(DENSITY, "HG_60F", "KG*M(-3)", 13.557e3); //, "Density of mercury 60 degree Fahrenheit");

    //Decimal multiples
    reg.AddConstant(ONE, "DECI", "ONE", 1.0e-1); //, "DECI-prefix");
    reg.AddConstant(ONE, "CENTI", "ONE", 1.0e-2); //, "CENTI-prefix");
    reg.AddConstant(ONE, "MILLI", "ONE", 1.0e-3); //, "MILLI-prefix");
    reg.AddConstant(ONE, "MICRO", "ONE", 1.0e-6); //, "MICRO-prefix");
    reg.AddConstant(ONE, "NANO", "ONE", 1.0e-9); //, "NANO-prefix");
    reg.AddConstant(ONE, "PICO", "ONE", 1.0e-12); //, "PICO-prefix");
    reg.AddConstant(ONE, "FEMTO", "ONE", 1.0e-15); //, "FEMTO-prefix");
    reg.AddConstant(ONE, "ATTO", "ONE", 1.0e-18); //, "ATTO-prefix");
    reg.AddConstant(ONE, "ZEPTO", "ONE", 1.0e-21); //, "ZEPTO-prefix");
    reg.AddConstant(ONE, "YOCTO", "ONE", 1.0e-24); //, "YOCTO-prefix");
    reg.AddConstant(ONE, "DEKA", "ONE", 1); //, "DEKA-prefix");
    reg.AddConstant(ONE, "HECTO", "ONE", 1.0e2); //, "HECTO-prefix");
    reg.AddConstant(ONE, "KILO", "ONE", 1.0e3); //, "KILO-prefix");
    reg.AddConstant(ONE, "MEGA", "ONE", 1.0e6); //, "MEGA-prefix");
    reg.AddConstant(ONE, "GIGA", "ONE", 1.0e9); //, "GIGA-prefix");
    reg.AddConstant(ONE, "TERA", "ONE", 1.0e12); //, "TERA-prefix");
    reg.AddConstant(ONE, "PETA", "ONE", 1.0e15); //, "PETA-prefix");
    reg.AddConstant(ONE, "EXA", "ONE", 1.0e18); //, "EXA-prefix");
    reg.AddConstant(ONE, "ZETTA", "ONE", 1.0e21); //, "ZETTA-prefix");
    reg.AddConstant(ONE, "YOTTA", "ONE", 1.0e24); //, "YOTTA-prefix");

    }
