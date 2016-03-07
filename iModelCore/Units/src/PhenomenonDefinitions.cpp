/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PhenomenonDefinitions.cpp $
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
void UnitRegistry::AddDefaultPhenomena ()
    {
    AddBasePhenomena(BasePhenomena::Length);
    AddBasePhenomena(BasePhenomena::Mass);
    AddBasePhenomena(BasePhenomena::Time);
    AddBasePhenomena(BasePhenomena::Temperature);
    AddBasePhenomena(BasePhenomena::TemperatureChange);
    AddBasePhenomena(BasePhenomena::ElectricCurrent);
    AddBasePhenomena(BasePhenomena::Mole);
    AddBasePhenomena(BasePhenomena::Luminosity);
    AddBasePhenomena(BasePhenomena::PlaneAngle);
    AddBasePhenomena(BasePhenomena::SolidAngle);
    AddBasePhenomena(BasePhenomena::Finance);
    AddBasePhenomena(BasePhenomena::Capita);
    AddBasePhenomena(BasePhenomena::Ratio);

    AddPhenomena(AREA, "LENGTH(2)");
    AddPhenomena(VOLUME, "LENGTH(3)");
    AddPhenomena(VELOCITY, "LENGTH*TIME(-1)");
    AddPhenomena(MOMENTUM, "MASS*VELOCITY");
    AddPhenomena(ANGULAR_VELOCITY, "ANGLE*TIME(-1)");
    AddPhenomena(ACCELERATION, "LENGTH*TIME(-2)");
    AddPhenomena(ANGULAR_ACCELERATION, "ANGLE*TIME(-2)");
    AddPhenomena(FORCE, "MASS*ACCELERATION");
    AddPhenomena(PRESSURE, "FORCE*AREA(-1)");
    AddPhenomena(FORCE_DENSITY, "FORCE*VOLUME(-1)");
    AddPhenomena(PRESSURE_GRADIENT, "PRESSURE*LENGTH(-1)");
    AddPhenomena(TORQUE, "FORCE*LENGTH*ANGLE");
    AddPhenomena(MOMENT_INERTIA, "MASS*LENGTH(2)");
    AddPhenomena(AREA_MOMENT_INERTIA, "LENGTH(4)");
    AddPhenomena(MASS_RATIO, "MASS*MASS(-1)");
    AddPhenomena(DENSITY, "MASS*VOLUME(-1)");
    AddPhenomena(SPECIFIC_VOLUME, "VOLUME*MASS(-1)");
    AddPhenomena(LINEAR_DENSITY, "MASS*LENGTH(-1)");
    AddPhenomena(SURFACE_DENSITY, "MASS*AREA(-1)");
    AddPhenomena(WORK, "FORCE*LENGTH"); // TODO: Multiple phenomemon equations: PRESSURE*VOLUME, POWER*TIME, ELECTRIC_CURRENT*TIME*ELECTRIC_POTENTIAL
    AddPhenomena(POWER, "WORK*TIME(-1)");
    AddPhenomena(FLOW, "VOLUME*TIME(-1)");
    AddPhenomena(SURFACE_FLOW_RATE, "FLOW*AREA(-1)");
    AddPhenomena(MASS_FLOW, "MASS*TIME(-1)");
    AddPhenomena(PARTICLE_FLOW, "MOLE*TIME(-1)");
    AddPhenomena(DYNAMIC_VISCOSITY, "PRESSURE*TIME"); // TODO: Check
    AddPhenomena(KINEMATIC_VISCOSITY, "FORCE*DYNAMIC_VISCOSITY(-1)"); // TODO: Check
    AddPhenomena(ELECTRIC_CHARGE, "CURRENT*TIME");
    AddPhenomena(ELECTRIC_POTENTIAL, "POWER*CURRENT(-1)");
    AddPhenomena(ELECTRIC_RESISTANCE, "ELECTRIC_POTENTIAL*CURRENT(-1)");
    AddPhenomena(CAPACITANCE, "ELECTRIC_CHARGE*ELECTRIC_POTENTIAL(-1)");
    AddPhenomena(MAGNETIC_FLUX, "ELECTRIC_POTENTIAL*TIME");
    AddPhenomena(MAGNETIC_FLUX_DENSITY, "MAGNETIC_FLUX*LENGTH(-2)");
    AddPhenomena(INDUCTANCE, "MAGNETIC_FLUX*CURRENT(-1)");
    AddPhenomena(LUMINOUS_FLUX, "LUMINOSITY*SOLIDANGLE"); // TODO: Check
    AddPhenomena(ILLUMINANCE, "LUMINOUS_FLUX*LENGTH(-2)");
    AddPhenomena(ROTATIONAL_SPRING_CONSTANT, "FORCE*LENGTH*ANGLE(-1)");  // TODO: Is this correct?  Should it be TORQUE/ANGLE?
    AddPhenomena(LINEAR_ROTATIONAL_SPRING_CONSTANT, "FORCE*ANGLE(-1)"); // TODO: Understand this phenomenon instead of just copying from old units.
    //AddPhenomena(RADIATION)
    //AddPhenomena(RADEXPOSURE)
    //AddPhenomena(RADABSORBDOSE, "WORK*MASS(-1)");
    //AddPhenomena(RADEQUDOSE, "WORK*MASS(-1)");
    AddPhenomena(SIZE_LENGTH_RATE, "LENGTH*LENGTH"); // TODO: ?
    AddPhenomena(THERMAL_CONDUCTIVITY, "POWER*LENGTH(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomena(THERMAL_RESISTANCE, "AREA*TEMPERATURE_CHANGE*POWER(-1)");
    //AddPhenomena(THERMAL_TRANSMITTANCE, "POWER*AREA(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomena(TEMPERATURE_GRADIENT, "TEMPERATURE_CHANGE*LENGTH(-1)");
    AddPhenomena(MOLAR_VOLUME, "VOLUME*MOLE(-1)");
    AddPhenomena(MOLAR_CONCENTRATION, "MOLE*VOLUME(-1)");
    AddPhenomena(SLOPE, "LENGTH*LENGTH(-1)");
    AddPhenomena(GRAVCONSTANT, "LENGTH(3)*MASS(-1)*TIME(-2)"); // TODO: Check
    AddPhenomena(THREAD_PITCH, "LENGTH*ANGLE(-1)"); // TODO: What about rotation portion?
    AddPhenomena(HEAT_TRANSFER, "POWER*AREA(-1)*TEMPERATURE_CHANGE(-1)"); // https://en.wikipedia.org/wiki/Heat_transfer_coefficient
    AddPhenomena(HEAT_FLUX_DENSITY, "POWER*AREA(-1)"); // https://en.wikipedia.org/wiki/Heat_flux  see description of heat flux density
    AddPhenomena(TORSIONAL_WARPING_CONSTANT, "LENGTH(6)"); // TODO: Could also be buckling resistance ... are we missing some angular portion to this unit?
    AddPhenomena(POPULATION_DENSITY, "CAPITA*AREA(-1)");
    AddPhenomena(FREQUENCY, "TIME(-1)");
    AddPhenomena(LINEAR_LOAD, "FORCE*LENGTH(-1)");
    //AddPhenomena(AREA_LOAD, );
    AddPhenomena(HEATING_VALUE_VOLUMETRIC, "WORK*VOLUME(-1)"); // TODO: Check
    AddPhenomena(HEATING_VALUE_MASS, "WORK*MASS(-1)");
    AddPhenomena(HEATING_VALUE_MOLE, "WORK*MOLE(-1)");
    AddPhenomena(SPECIFIC_HEAT_CAPACITY, "WORK*MASS(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomena(SPECIFIC_HEAT_CAPACITY_MOLAR, "WORK*MOLE(-1)*TEMPERATURE_CHANGE(-1)");
    AddPhenomena(PERCENTAGE, "RATIO");
    //AddPhenomena(ACTION, "WORK*TIME");
    }


