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
    //reg.AddConstant(LENGTH, "EARTH_RAD", "M", 6.3781e6); //, "Radius Of Earth");
    //reg.AddConstant(LENGTH, "ASTRO_UNIT", "M", 1.495978707e11); //, "Distance from Sun to Earth");
    //reg.AddConstant(VELOCITY, "C", "M*S(-1)", 2.99792458e8); //, "Speed Of Light in Vaccum");
    AddConstant(LENGTH_RATIO, "PI", "ONE", PI); //, "Ratio of Circumference to its Diameter");
    AddConstant(LENGTH_RATIO, "PI/4", "PI", 0.25); //, "PI/4");
    AddConstant(LENGTH_RATIO, "PI/2", "PI", 0.5); //, "PI/2");
    AddConstant(LENGTH_RATIO, "2PI", "PI", 2); //, "2*PI");
    AddConstant(ANGLE, "360DEG", "ARC_DEG", 360); //, "Full Circle");
    //reg.AddConstant(ONE, "E", "ONE", E); //, "Base of the natural logarithm");
    //reg.AddConstant(GRAVCONSTANT, "G0", "M(3)*KG(-1)*S(-2)", 6.67408e-11); //, "Gravitational constant");
    //reg.AddConstant(ELECTRIC_CHARGE, "Q0", "A*S", 1.6021766208e-19);// , "Elementary Charge");
    AddConstant(ACCELERATION, "STD_G", "M*S(-2)", 9.80665); //, "Standard Gravity");
    //reg.AddConstant(ACTION, "H", "J*S", 6.62607004e-34); //, "Planck constant"); // NOTE: Not used so removed instead of adding 'ACTION' phenomenon
    //reg.AddConstant(MOLE, "A0", "MOL(-1)", 6.022140857e23); //, "Avogadro Number"); // NOTE: Not actually MOLE but MOLE(-1), we don't use it anywhere so removed it

    // TODO: These densities need a reference really really badly.  Using H20_4C for Conventional because it is the documented value for
    // conventional density that I found but it is not consistently the documented value for density of water at 4C.
    // Densities of Water in KG/CUB.M
    //reg.AddConstant(DENSITY, "H2O_0C", "KG*M(-3)", 0.99987e3); //, "Density of water at 0 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_4C", "KG*M(-3)", 1.00000e3); //, "Density of water at 4 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_10C", "KG*M(-3)", 0.99975e3); //, "Density of water at 10 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_20C", "KG*M(-3)", 0.99802e3); //, "Density of water at 20 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_30C", "KG*M(-3)", 0.9957e3); //, "Density of water at 30 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_40C", "KG*M(-3)", 0.9922e3); //, "Density of water at 40 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_50C", "KG*M(-3)", 0.9881e3); //, "Density of water at 50 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_60C", "KG*M(-3)", 0.98338e3); //, "Density of water at 60 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_70C", "KG*M(-3)", 0.97729e3); //, "Density of water at 70 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_80C", "KG*M(-3)", 0.97056e3); //, "Density of water at 80 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_90C", "KG*M(-3)", 0.9653e3); //, "Density of water at 90 degree Celsius");
    //reg.AddConstant(DENSITY, "H2O_100C", "KG*M(-3)", 0.95865e3); //, "Density of water at 100 degree Celsius");
    //// Densities of Water in LBM/CUB.FT
    //reg.AddConstant(DENSITY, "H2O_32F", "LBM*FT(-3)", 62.416); //, "Density of water at 32 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_39.2F", "LBM*FT(-3)", 62.424); //, "Density of water at 39.2 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_40F", "LBM*FT(-3)", 62.423); //, "Density of water at 40 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_50F", "LBM*FT(-3)", 62.408); //, "Density of water at 50 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_60F", "LBM*FT(-3)", 62.366); //, "Density of water at 60 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_70F", "LBM*FT(-3)", 62.300); //, "Density of water at 70 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_80F", "LBM*FT(-3)", 62.217); //, "Density of water at 80 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_90F", "LBM*FT(-3)", 62.118); //, "Density of water at 90 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_100F", "LBM*FT(-3)", 61.998); //, "Density of water at 100 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_120F", "LBM*FT(-3)", 61.719); //, "Density of water at 120 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_140F", "LBM*FT(-3)", 61.386); //, "Density of water at 140 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_160F", "LBM*FT(-3)", 61.006); //, "Density of water at 160 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_180F", "LBM*FT(-3)", 60.586); //, "Density of water at 180 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_200F", "LBM*FT(-3)", 60.135); //, "Density of water at 200 degree Fahrenheit");
    //reg.AddConstant(DENSITY, "H2O_212F", "LBM*FT(-3)", 59.843); //, "Density of water at 212 degree Fahrenheit");

    //reg.AddConstant(DENSITY, "HG_0C", "KG*M(-3)", 13.595e3); //, "Density of mercury 0 degree Celsius");
    //reg.AddConstant(DENSITY, "HG_15C", "KG*M(-3)", 13.559e3); //, "Density of mercury 15 degree Celsius");
    //reg.AddConstant(DENSITY, "HG_20C", "KG*M(-3)", 13.546e3); //, "Density of mercury 20 degree Celsius");
    //reg.AddConstant(DENSITY, "HG_25C", "KG*M(-3)", 13.532e3); //, "Density of mercury 25 degree Celsius");
    //reg.AddConstant(DENSITY, "HG_60F", "KG*M(-3)", 13.557e3); //, "Density of mercury 60 degree Fahrenheit");

    //reg.AddConstant(PRESSURE, "GAUGE_OFFSET", "PA", 101325); // TODO: Get NIST reference, There are other standards but this is the one we used in Old units framework

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
    AddConstant(METRIC_PREFIX, "DEKA", "ONE", 1); //, "DEKA-prefix");
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
