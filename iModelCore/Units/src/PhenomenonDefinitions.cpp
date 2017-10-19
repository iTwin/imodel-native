/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PhenomenonDefinitions.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "StandardNames.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultPhenomena ()
    {
    AddBasePhenomenon(BasePhenomena::Length);
    AddBasePhenomenon(BasePhenomena::Mass);
    AddBasePhenomenon(BasePhenomena::Time);
    AddBasePhenomenon(BasePhenomena::Temperature);
    AddBasePhenomenon(BasePhenomena::TemperatureChange);
    AddBasePhenomenon(BasePhenomena::ElectricCurrent);
    AddBasePhenomenon(BasePhenomena::Mole);
    AddBasePhenomenon(BasePhenomena::Luminosity);
    AddBasePhenomenon(BasePhenomena::PlaneAngle);
    AddBasePhenomenon(BasePhenomena::SolidAngle);
    AddBasePhenomenon(BasePhenomena::Finance);
    AddBasePhenomenon(BasePhenomena::Capita);
    AddBasePhenomenon(BasePhenomena::Ratio);

    AddPhenomenon(APPARENT_POWER, "ELECTRIC_POTENTIAL*CURRENT");
    AddPhenomenon(AREA, "LENGTH(2)");
    AddPhenomenon(VOLUME, "LENGTH(3)");
    AddPhenomenon(VELOCITY, "LENGTH*TIME(-1)");
    AddPhenomenon(MOMENTUM, "MASS*VELOCITY");
    AddPhenomenon(ANGULAR_VELOCITY, "ANGLE*TIME(-1)");
    AddPhenomenon(ACCELERATION, "LENGTH*TIME(-2)");
    AddPhenomenon(ANGULAR_ACCELERATION, "ANGLE*TIME(-2)");
    AddPhenomenon(FORCE, "MASS*ACCELERATION");
    AddPhenomenon(PRESSURE, "FORCE*AREA(-1)");
    AddPhenomenon(FORCE_DENSITY, "FORCE*VOLUME(-1)");
    AddPhenomenon(PRESSURE_GRADIENT, "PRESSURE*LENGTH(-1)");
    AddPhenomenon(TORQUE, "FORCE*LENGTH*ANGLE");
    AddPhenomenon(MOMENT_INERTIA, "MASS*LENGTH(2)");
    AddPhenomenon(AREA_MOMENT_INERTIA, "LENGTH(4)");
    AddPhenomenon(MASS_RATIO, "MASS*MASS(-1)");
    AddPhenomenon(DENSITY, "MASS*VOLUME(-1)");
    AddPhenomenon(SPECIFIC_VOLUME, "VOLUME*MASS(-1)");
    AddPhenomenon(LINEAR_DENSITY, "MASS*LENGTH(-1)");
    AddPhenomenon(SURFACE_DENSITY, "MASS*AREA(-1)");
    AddPhenomenon(WORK, "FORCE*LENGTH"); // TODO: Multiple phenomemon equations: PRESSURE*VOLUME, POWER*TIME, ELECTRIC_CURRENT*TIME*ELECTRIC_POTENTIAL
    AddPhenomenon(POWER, "WORK*TIME(-1)");
    AddPhenomenon(FLOW, "VOLUME*TIME(-1)");
    AddPhenomenon(SURFACE_FLOW_RATE, "FLOW*AREA(-1)");
    AddPhenomenon(SURFACE_POWER_DENSITY, "POWER*AREA(-1)");
    AddPhenomenon(MASS_FLOW, "MASS*TIME(-1)");
    AddPhenomenon(PARTICLE_FLOW, "MOLE*TIME(-1)");
    AddPhenomenon(DYNAMIC_VISCOSITY, "PRESSURE*TIME"); // TODO: Check
    AddPhenomenon(KINEMATIC_VISCOSITY, "FORCE*DYNAMIC_VISCOSITY(-1)"); // TODO: Check
    AddPhenomenon(ELECTRIC_CHARGE, "CURRENT*TIME");
    AddPhenomenon(ELECTRIC_POTENTIAL, "POWER*CURRENT(-1)");
    AddPhenomenon(ELECTRIC_RESISTANCE, "ELECTRIC_POTENTIAL*CURRENT(-1)");
    AddPhenomenon(CAPACITANCE, "ELECTRIC_CHARGE*ELECTRIC_POTENTIAL(-1)");
    AddPhenomenon(MAGNETIC_FLUX, "ELECTRIC_POTENTIAL*TIME");
    AddPhenomenon(MAGNETIC_FLUX_DENSITY, "MAGNETIC_FLUX*LENGTH(-2)");
    AddPhenomenon(INDUCTANCE, "MAGNETIC_FLUX*CURRENT(-1)");
    AddPhenomenon(LUMINOUS_FLUX, "LUMINOSITY*SOLIDANGLE"); // TODO: Check
    AddPhenomenon(ILLUMINANCE, "LUMINOUS_FLUX*LENGTH(-2)");
    AddPhenomenon(ROTATIONAL_SPRING_CONSTANT, "FORCE*LENGTH*ANGLE(-1)");  // TODO: Is this correct?  Should it be TORQUE/ANGLE?
    AddPhenomenon(LINEAR_ROTATIONAL_SPRING_CONSTANT, "FORCE*ANGLE(-1)"); // TODO: Understand this phenomenon instead of just copying from old units.
    //AddPhenomenon(RADIATION)
    //AddPhenomenon(RADEXPOSURE)
    //AddPhenomenon(RADABSORBDOSE, "WORK*MASS(-1)");
    //AddPhenomenon(RADEQUDOSE, "WORK*MASS(-1)");
    AddPhenomenon(SIZE_LENGTH_RATE, "LENGTH*LENGTH"); // TODO: ?
    AddPhenomenon(THERMAL_CONDUCTIVITY, "POWER*LENGTH(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(THERMAL_RESISTANCE, "AREA*TEMPERATURE_CHANGE*POWER(-1)");
    //AddPhenomenon(THERMAL_TRANSMITTANCE, "POWER*AREA(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(TEMPERATURE_GRADIENT, "TEMPERATURE_CHANGE*LENGTH(-1)");
    AddPhenomenon(MOLAR_VOLUME, "VOLUME*MOLE(-1)");
    AddPhenomenon(MOLAR_CONCENTRATION, "MOLE*VOLUME(-1)");
    AddPhenomenon(SLOPE, "LENGTH*LENGTH(-1)");
    AddPhenomenon(GRAVCONSTANT, "LENGTH(3)*MASS(-1)*TIME(-2)"); // TODO: Check
    AddPhenomenon(THREAD_PITCH, "LENGTH*ANGLE(-1)"); // TODO: What about rotation portion?
    AddPhenomenon(HEAT_TRANSFER, "POWER*AREA(-1)*TEMPERATURE_CHANGE(-1)"); // https://en.wikipedia.org/wiki/Heat_transfer_coefficient
    AddPhenomenon(HEAT_FLUX_DENSITY, "POWER*AREA(-1)"); // https://en.wikipedia.org/wiki/Heat_flux  see description of heat flux density
    AddPhenomenon(TORSIONAL_WARPING_CONSTANT, "LENGTH(6)"); // TODO: Could also be buckling resistance ... are we missing some angular portion to this unit?
    AddPhenomenon(POPULATION_DENSITY, "CAPITA*AREA(-1)");
    AddPhenomenon(FREQUENCY, "TIME(-1)");
    AddPhenomenon(LINEAR_LOAD, "FORCE*LENGTH(-1)");
    //AddPhenomenon(AREA_LOAD, );
    AddPhenomenon(ENERGY_DENSITY, "WORK*VOLUME(-1)"); // TODO: Check
    AddPhenomenon(SPECIFIC_ENERGY, "WORK*MASS(-1)");
    AddPhenomenon(HEATING_VALUE_MOLE, "WORK*MOLE(-1)");
    AddPhenomenon(SPECIFIC_HEAT_CAPACITY, "WORK*MASS(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(SPECIFIC_HEAT_CAPACITY_MOLAR, "WORK*MOLE(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomenon(PERCENTAGE, "RATIO");
    AddPhenomenon(LINEAR_COST, "FINANCE*LENGTH(-1)");
    AddPhenomenon(LINEAR_RATE, "RATIO*LENGTH(-1)");

    AddPhenomenon(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, "LENGTH*LENGTH(-1)*TEMPERATURE_CHANGE(-1)");
    //AddPhenomenon(AREA_COEFFICIENT_OF_THERMAL_EXPANSION, "AREA*AREA(-1)*TEMPERATURE_CHANGE(-1)");
    //AddPhenomenon(VOLUMETRIC_COEFFICIENT_OF_THERMAL_EXPANSION, "VOLUME*VOLUME(-1)*TEMPERATURE_CHANGE(-1)");
    //AddPhenomenon(ACTION, "WORK*TIME");

    AddPhenomenon(VOLUME_RATIO, "VOLUME*VOLUME(-1)");
    }


