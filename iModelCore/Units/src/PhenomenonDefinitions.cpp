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
    //AddPhenomena(MOMENTUM, )
    AddPhenomena(ANGVELOCITY, "SOLIDANGLE*TIME(-1)");
    AddPhenomena(ACCELERATION, "LENGTH*TIME(-2)");
    //AddPhenomena(ANGACCELERAT)
    AddPhenomena(FORCE, "MASS*LENGTH*TIME(-2)");
    AddPhenomena(PRESSURE, "MASS*LENGTH(-1)*TIME(-2)"); //TODO: Needs work
    AddPhenomena(FORCE_DENSITY, "FORCE*VOLUME(-1)");
    AddPhenomena(PRESSURE_GRADIENT, "PRESSURE*LENGTH(-1)");
    AddPhenomena(TORQUE, "FORCE*LENGTH*ANGLE"); // TODO: Double Check
    AddPhenomena(MOMENTINERTIA, "ANGVELOCITY*MOMENTUM"); //TODO: Needs work
    AddPhenomena(DENSITY, "MASS*VOLUME(-1)");
    AddPhenomena(SPECVOLUME, "VOLUME*MASS(-1)");
    AddPhenomena(LINEARDENSITY, "MASS*LENGTH(-1)");
    AddPhenomena(SURFACEDENSITY, "MASS*AREA(-1)");
    AddPhenomena(WORK, "FORCE*LENGTH"); // TODO: Multiple phenomemon equations: PRESSURE*VOLUME, POWER*TIME, ELECTRIC_CURRENT*TIME*VOLTAGE
    AddPhenomena(POWER, "WORK*TIME");
    AddPhenomena(FLOW, "VOLUME*TIME(-1)");
    AddPhenomena(SURFACE_FLOW_RATE, "VOLUME*TIME(-1)*AREQ(-1)");
    AddPhenomena(MASS_FLOW, "MASS*TIME(-1)");
    AddPhenomena(PARTICLE_FLOW, "MOLE*TIME(-1)");
    AddPhenomena(DYNVISCOSITY, "KINVISCOSITY*DENSITY"); // TODO: Check
    AddPhenomena(KINVISCOSITY, "DYNVISCOSITY*DENSITY(-1)"); // TODO: Check
    //AddPhenomena(ELECTRIC_CHARGE, )
    //AddPhenomena(VOLTAGE)
    //AddPhenomena(ELRESISTANCE)
    //AddPhenomena(CAPACITANCE)
    //AddPhenomena(MAGNETICFLUX)
    //AddPhenomena(MAGFLUXDENSITY)
    //AddPhenomena(INDUCTANCE)
    //AddPhenomena(LUMINANCE)
    //AddPhenomena(LUMINOUSFLUX)
    //AddPhenomena(ILLUMINANCE)
    //AddPhenomena(RADIATION)
    //AddPhenomena(RADEXPOSURE)
    //AddPhenomena(RADABSORBDOSE)
    //AddPhenomena(RADEQUDOSE)
    //AddPhenomena(SIZELENRATE) // TODO: ?
    AddPhenomena(THERMOCONDUCT, "POWER*LENGTH(-1)*TEMPERATURE(-1)"); // TODO: Check
    AddPhenomena(MOLAR_VOLUME, "VOLUME*MOLE(-1)");
    AddPhenomena(MOLAR_CONCENTRATION, "MOLE*VOLUME(-1)");
    AddPhenomena(SLOPE, "LENGTH*LENGTH(-1)");
    AddPhenomena(GRAVCONSTANT, "GRAVCONSTANT"); // TODO: Necessary?
    AddPhenomena(THREAD_PITCH, "LENGTH*ANGLE(-1)");
    AddPhenomena(HEATTRASNFER, "POWER*AREA(-1)*TEMPERATURE(-1)"); // TODO: Check ... HEATFLUX*TEMPERATURE(-1)?
    AddPhenomena(HEATFLUX, "POWER*AREA(-1)");
    AddPhenomena(WARPING_CONSTANT, "WARPING_CONSTANT"); // TODO: Actually do
    AddPhenomena(POPULATION_DENSITY, "CAPITA*AREA(-1)");
    AddPhenomena(FREQUENCY, "TIME(-1)"); // TODO: Make sense?
    AddPhenomena(LINEAR_LOAD, "FORCE*LENGTH(-1)");
    //AddPhenomena(AREA_LOAD, );
    AddPhenomena(HEATING_VALUE, "WORK*MASS(-1)");
    }


