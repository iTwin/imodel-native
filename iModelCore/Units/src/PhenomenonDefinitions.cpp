/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PhenomenonDefinitions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Units/UnitRegistry.h>

USING_NAMESPACE_BENTLEY_UNITS

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetSI()
    {
    static UnitSystemP s_si = new UnitSystem(SI);
    return *s_si;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetCGS()
    {
    static UnitSystemP s_cgs = new UnitSystem(CGS);
    return *s_cgs;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetMetric()
    {
    static UnitSystemP s_metric = new UnitSystem(METRIC);
    return *s_metric;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetImperial()
    {
    static UnitSystemP s_imperial = new UnitSystem(IMPERIAL);
    return *s_imperial;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetMaritime()
    {
    static UnitSystemP s_maritime = new UnitSystem(MARITIME);
    return *s_maritime;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetUSSurvey()
    {
    static UnitSystemP s_survey = new UnitSystem(USSURVEY);
    return *s_survey;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetIndustrial()
    {
    static UnitSystemP s_industrial = new UnitSystem(INDUSTRIAL);
    return *s_industrial;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetInternational()
    {
    static UnitSystemP s_international = new UnitSystem(INTERNATIONAL);
    return *s_international;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetUSCustom()
    {
    static UnitSystemP s_custom = new UnitSystem(USCUSTOM);
    return *s_custom;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetStatistics()
    {
    static UnitSystemP s_stats = new UnitSystem(STATISTICS);
    return *s_stats;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetFinance()
    {
    static UnitSystemP s_finance = new UnitSystem(FINANCE);
    return *s_finance;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetConstant()
    {
    static UnitSystemP s_constant = new UnitSystem(CONSTANT);
    return *s_constant;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitSystemR StandardUnitSystems::GetDummy()
    {
    static UnitSystemP s_dummy = new UnitSystem(DUMMY);
    return *s_dummy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
void UnitRegistry::AddBaseSystems()
    {
    AddSystem(StandardUnitSystems::GetSI());
    AddSystem(StandardUnitSystems::GetCGS());
    AddSystem(StandardUnitSystems::GetMetric());
    AddSystem(StandardUnitSystems::GetImperial());
    AddSystem(StandardUnitSystems::GetMaritime());
    AddSystem(StandardUnitSystems::GetUSSurvey());
    AddSystem(StandardUnitSystems::GetIndustrial());
    AddSystem(StandardUnitSystems::GetInternational());
    AddSystem(StandardUnitSystems::GetUSCustom());
    AddSystem(StandardUnitSystems::GetStatistics());
    AddSystem(StandardUnitSystems::GetFinance());
    AddSystem(StandardUnitSystems::GetConstant());
    AddSystem(StandardUnitSystems::GetDummy());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
void UnitRegistry::AddBasePhenomena()
    {
    AddBasePhenomenon(LENGTH);
    AddBasePhenomenon(MASS);
    AddBasePhenomenon(TIME);
    AddBasePhenomenon(TEMPERATURE);
    AddBasePhenomenon(TEMPERATURE_CHANGE);
    AddBasePhenomenon(CURRENT);
    AddBasePhenomenon(MOLE);
    AddBasePhenomenon(LUMINOSITY);
    AddBasePhenomenon(ANGLE);
    AddBasePhenomenon(SOLIDANGLE);
    AddBasePhenomenon(CURRENCY);
    AddBasePhenomenon(CAPITA);
    AddBasePhenomenon(NUMBER);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultPhenomena ()
    {
    AddPhenomenon(APPARENT_POWER, "ELECTRIC_POTENTIAL*CURRENT");
    AddPhenomenon(AREA, "LENGTH(2)");
    AddPhenomenon(VOLUME, "LENGTH(3)");
    AddPhenomenon(VELOCITY, "LENGTH*TIME(-1)");
    AddPhenomenon(ANGULAR_VELOCITY, "ANGLE*TIME(-1)");
    AddPhenomenon(ACCELERATION, "LENGTH*TIME(-2)");
    AddPhenomenon(FORCE, "MASS*ACCELERATION");
    AddPhenomenon(PRESSURE, "FORCE*AREA(-1)");
    AddPhenomenon(FORCE_DENSITY, "FORCE*VOLUME(-1)");
    AddPhenomenon(PRESSURE_GRADIENT, "PRESSURE*LENGTH(-1)");
    AddPhenomenon(TORQUE, "FORCE*LENGTH*ANGLE(-1)");
    AddPhenomenon(AREA_MOMENT_INERTIA, "LENGTH(4)");
    AddPhenomenon(MASS_RATIO, "MASS*MASS(-1)");
    AddPhenomenon(DENSITY, "MASS*VOLUME(-1)");
    AddPhenomenon(SPECIFIC_VOLUME, "VOLUME*MASS(-1)");
    AddPhenomenon(LINEAR_DENSITY, "MASS*LENGTH(-1)");
    AddPhenomenon(SURFACE_DENSITY, "MASS*AREA(-1)");
    AddPhenomenon(WORK, "FORCE*LENGTH");
    AddPhenomenon(POWER, "WORK*TIME(-1)");
    AddPhenomenon(FLOW, "VOLUME*TIME(-1)");
    AddPhenomenon(SURFACE_FLOW_RATE, "FLOW*AREA(-1)");
    AddPhenomenon(MASS_FLOW, "MASS*TIME(-1)");
    AddPhenomenon(PARTICLE_FLOW, "MOLE*TIME(-1)");
    AddPhenomenon(DYNAMIC_VISCOSITY, "PRESSURE*TIME");
    AddPhenomenon(KINEMATIC_VISCOSITY, "FORCE*DYNAMIC_VISCOSITY(-1)");
    AddPhenomenon(ELECTRIC_CHARGE, "CURRENT*TIME");
    AddPhenomenon(ELECTRIC_POTENTIAL, "POWER*CURRENT(-1)");
    AddPhenomenon(LUMINOUS_FLUX, "LUMINOSITY*SOLIDANGLE");
    AddPhenomenon(ILLUMINANCE, "LUMINOUS_FLUX*LENGTH(-2)");
    AddPhenomenon(ROTATIONAL_SPRING_CONSTANT, "TORQUE*ANGLE(-1)");
    AddPhenomenon(LINEAR_ROTATIONAL_SPRING_CONSTANT, "FORCE*ANGLE(-1)");
    AddPhenomenon(SIZE_LENGTH_RATE, "LENGTH*LENGTH");
    AddPhenomenon(THERMAL_CONDUCTIVITY, "POWER*LENGTH(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(THERMAL_RESISTANCE, "AREA*TEMPERATURE_CHANGE*POWER(-1)");
    AddPhenomenon(TEMPERATURE_GRADIENT, "TEMPERATURE_CHANGE*LENGTH(-1)");
    AddPhenomenon(MOLAR_VOLUME, "VOLUME*MOLE(-1)");
    AddPhenomenon(MOLAR_CONCENTRATION, "MOLE*VOLUME(-1)");
    AddPhenomenon(SLOPE, "LENGTH*LENGTH(-1)");
    AddPhenomenon(HEAT_TRANSFER, "POWER*AREA(-1)*TEMPERATURE_CHANGE(-1)"); // https://en.wikipedia.org/wiki/Heat_transfer_coefficient
    AddPhenomenon(HEAT_FLUX_DENSITY, "POWER*AREA(-1)"); // https://en.wikipedia.org/wiki/Heat_flux see description of heat flux density
    AddPhenomenon(TORSIONAL_WARPING_CONSTANT, "LENGTH(6)");
    AddPhenomenon(POPULATION_DENSITY, "CAPITA*AREA(-1)");
    AddPhenomenon(FREQUENCY, "TIME(-1)");
    AddPhenomenon(LINEAR_LOAD, "FORCE*LENGTH(-1)");

    AddPhenomenon(ENERGY_DENSITY, "WORK*VOLUME(-1)");
    AddPhenomenon(SPECIFIC_ENERGY, "WORK*MASS(-1)");
    AddPhenomenon(SPECIFIC_HEAT_CAPACITY, "WORK*MASS(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(SPECIFIC_HEAT_CAPACITY_MOLAR, "WORK*MOLE(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(PERCENTAGE, NUMBER);
    AddPhenomenon(LINEAR_RATE, "NUMBER*LENGTH(-1)");

    AddPhenomenon(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, "LENGTH*LENGTH(-1)*TEMPERATURE_CHANGE(-1)");

    AddPhenomenon(VOLUME_RATIO, "VOLUME*VOLUME(-1)");

    AddPhenomenon(LENGTH_RATIO, "LENGTH*LENGTH(-1)");
    AddPhenomenon(METRIC_PREFIX, "NUMBER");
    }
