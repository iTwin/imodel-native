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
    }


