/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/StandardNames.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "UnitsPCH.h"

BEGIN_BENTLEY_UNITS_NAMESPACE

static double const PI = 3.1415926535897932384626433832795;
static double const E = 2.7182818284590452353602874713527;

static Utf8CP const SI = "SI";
static Utf8CP const CGS = "CGS";
static Utf8CP const METRIC = "METRIC";
static Utf8CP const IMPERIAL = "IMPERIAL";
static Utf8CP const PHYSICS = "PHYSICS";
static Utf8CP const CHEMISTRY = "CHEMISTRY";
static Utf8CP const THERMODYNAMICS = "THERMODYNAMICS";
static Utf8CP const ASTRONOMY = "ASTRONOMY";
static Utf8CP const MARITIME = "MARITIME";
static Utf8CP const SURVEYOR = "SURVEYOR";
static Utf8CP const TYPOGRAPHY = "TYPOGRAPHY";
static Utf8CP const POSTSCRIPT = "POSTSCRIPT";
static Utf8CP const TEXT = "TEXT";
static Utf8CP const INDUSTRIAL = "INDUSTRIAL";
static Utf8CP const PHARMACEUTICAL = "PHARMACEUTICAL";
static Utf8CP const AGRICULTURE = "AGRICULTURE";
static Utf8CP const INTERNATIONAL = "INTERNATIONAL";
static Utf8CP const USCUSTOM = "USCUSTOM";
static Utf8CP const BRITISH = "BRITISH";
static Utf8CP const JAPANESE = "JAPANESE";
static Utf8CP const HISTORICAL = "HISTORICAL";
static Utf8CP const STATISTICS = "STATISTICS";
static Utf8CP const BENTLEY = "BENTLEY";
static Utf8CP const CUSTOMARY = "CUSTOMARY";
static Utf8CP const FINANCE = "FINANCE";
static Utf8CP const CONSTANT = "CONSTANT";

static Utf8CP const LENGTH = "LENGTH";
static Utf8CP const MASS = "MASS";
static Utf8CP const TIME = "TIME";
static Utf8CP const TEMPERATURE = "TEMPERATURE";
static Utf8CP const CURRENT = "CURRENT";
static Utf8CP const MOLE = "MOLE";
static Utf8CP const LUMINOUSINTENSITY = "LUMINOUSINTENSITY";
static Utf8CP const RATIO = "RATIO";
static Utf8CP const ANGLE = "ANGLE";
static Utf8CP const SOLIDANGLE = "SOLIDANGLE";

static Utf8CP const AREA = "AREA";
static Utf8CP const VOLUME = "VOLUME";
static Utf8CP const VELOCITY = "VELOCITY";
static Utf8CP const MOMENTUM = "MOMENTUM";
static Utf8CP const ANGVELOCITY = "ANGVELOCITY";
static Utf8CP const ACCELERATION = "ACCELERATION";
static Utf8CP const ANGACCELERAT = "ANGACCELERAT";
static Utf8CP const FORCE = "FORCE";
static Utf8CP const PRESSURE = "PRESSURE";
static Utf8CP const PRESSURE_GRADIENT = "PRESSURE_GRADIENT";
static Utf8CP const TORQUE = "TORQUE";
static Utf8CP const MOMENTINERTIA = "MOMENTINERTIA";
static Utf8CP const DENSITY = "DENSITY";
static Utf8CP const SPECVOLUME = "SPECVOLUME";
static Utf8CP const LINEARDENSITY = "LINEARDENSITY";
static Utf8CP const SURFACEDENSITY = "SURFACEDENSITY";
static Utf8CP const WORK = "WORK";
static Utf8CP const POWER = "POWER";
static Utf8CP const FLOW = "FLOW";
static Utf8CP const SURFACE_FLOW_RATE = "SURFACE_FLOW_RATE";
static Utf8CP const MASS_FLOW = "MASS_FLOW";
static Utf8CP const PARTICLE_FLOW = "PARTICLE_FLOW";
static Utf8CP const DYNVISCOSITY = "DYNVISCOSITY";
static Utf8CP const KINVISCOSITY = "KINVISCOSITY";
static Utf8CP const ELECTRIC_CHARGE = "ELECTRIC_CHARGE";
static Utf8CP const VOLTAGE = "VOLTAGE";
static Utf8CP const ELRESISTANCE = "ELRESISTANCE";
static Utf8CP const CAPACITANCE = "CAPACITANCE";
static Utf8CP const MAGNETICFLUX = "MAGNETICFLUX";
static Utf8CP const MAGFLUXDENSITY = "MAGFLUXDENSITY";
static Utf8CP const INDUCTANCE = "INDUCTANCE";
static Utf8CP const LUMINANCE = "LUMINANCE";
static Utf8CP const LUMINOUSFLUX = "LUMINOUSFLUX";
static Utf8CP const ILLUMINANCE = "ILLUMINANCE";
static Utf8CP const RADIATION = "RADIATION";
static Utf8CP const RADEXPOSURE = "RADEXPOSURE";
static Utf8CP const RADABSORBDOSE = "RADABSORBDOSE";
static Utf8CP const RADEQUDOSE = "RADEQUDOSE";
static Utf8CP const SIZELENRATE = "SIZELENRATE";
static Utf8CP const CAPITA = "CAPITA";
static Utf8CP const THERMOCONDUCT = "THERMOCONDUCT";
static Utf8CP const MOLAR_VOLUME = "MOLAR_VOLUME";
static Utf8CP const MOLAR_CONCENTRATION = "MOLAR_CONCENTRATION";
//static Utf8CP const FINANCE = "FINANCE";
static Utf8CP const SLOPE = "SLOPE";
static Utf8CP const GRAVCONSTANT = "GRAVCONSTANT";
static Utf8CP const THREAD_PITCH = "THREAD_PITCH";
static Utf8CP const HEATTRASNFER = "HEATTRASNFER";
static Utf8CP const HEATFLUX = "HEATFLUX";
static Utf8CP const WARPING_CONSTANT = "WARPING_CONSTANT";
static Utf8CP const POPULATION_DENSITY = "POPULATION_DENSITY";
static Utf8CP const FREQUENCY = "FREQUENCY";
static Utf8CP const LINEAR_LOAD = "LINEAR_LOAD";
static Utf8CP const AREA_LOAD = "AREA_LOAD";
static Utf8CP const HEATING_VALUE = "HEATING_VALUE";
static Utf8CP const TBD = "TBD";


struct BasePhenomena
{
private:
    BasePhenomena();

public:
    static const Utf8Char Length = 'L';
    static const Utf8Char Mass = 'M';
    static const Utf8Char Time = 'T';
    static const Utf8Char Temperature = 'K';
    static const Utf8Char ElectricCurrent = 'I';
    static const Utf8Char Mole = 'N';
    static const Utf8Char Luminosity = 'J';
    static const Utf8Char PlaneAngle = 'A';
    static const Utf8Char SolidAngle = 'S';
    static const Utf8Char Finance = '$';
    static const Utf8Char Capita = 'X';
    static const Utf8Char Ratio = 'R';
};

END_BENTLEY_UNITS_NAMESPACE
