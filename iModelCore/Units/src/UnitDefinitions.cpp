/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitDefinitions.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "StandardNames.h"

USING_NAMESPACE_BENTLEY_UNITS

void AddLengths(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(LENGTH, SI, "MM", "[MILLI]*M");
    reg.AddSynonym(unit, "MILLIMETRE");
    unit = reg.AddUnit(LENGTH, SI, "CM", "[CENTI]*M");
    reg.AddSynonym(unit, "CENTIMETRE");
    unit = reg.AddUnit(LENGTH, SI, "DM", "[DECI]*M");
    reg.AddSynonym(unit, "DECIMETRE");
    unit = reg.AddUnit(LENGTH, SI, "KM", "[KILO]*M");
    reg.AddSynonym(unit, "KILOMETRE");
    unit = reg.AddUnit(LENGTH, SI, "MU", "[MICRO]*M");
    reg.AddSynonym(unit, "MICRON");
    reg.AddSynonym(unit, "MICROMETRE");
    reg.AddUnit(LENGTH, SI, "ANGSTROM", "M", 1e-10);// BISQFactOne, -10.0); //, BISQNoDescript); //, BISQSecUom);
    unit = reg.AddUnit(LENGTH, SI, "FERMI", "[FEMTO]*M");
    reg.AddSynonym(unit, "FEMTOMETRE");
    unit = reg.AddUnit(LENGTH, IMPERIAL, "IN", "MM", 25.4);
    reg.AddSynonym(unit, "INCH");
    unit = reg.AddUnit(LENGTH, IMPERIAL, "FT", "IN", 12.0);
    reg.AddSynonym(unit, "FOOT");
    reg.AddUnit(LENGTH, USCUSTOM, "MILLIINCH", "[MILLI]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "MICROINCH", "[MICRO]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILLIFOOT", "[MILLI]*FT");// BISQSecUom);
    unit = reg.AddUnit(LENGTH, IMPERIAL, "YRD", "FT", 3.0);
    reg.AddSynonym(unit, "YARD");
    reg.AddUnit(LENGTH, SURVEYOR, "CHAIN", "FT", 66.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILE", "YRD", 1760.0);// ); //, BISQSecUom);
    unit = reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE", "M", 1852.0);// ); //, BISQSecUom);
    reg.AddSynonym(unit, "NAUTICAL_MILE_INTERNATIONAL");
    unit = reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE_IMPERIAL", "M", 1853.0);
    reg.AddSynonym(unit, "ADMIRALTY_MILE");
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_INCH", "CM", 10000.0 / 3937.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_FOOT", "US_SURVEY_INCH", 12.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_YARD", "US_SURVEY_FOOT", 3.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_MILE", "US_SURVEY_YARD", 1760.0);// ); //, BISQSecUom);

    reg.AddUnit(LENGTH, IMPERIAL, "BARLEYCORN", "IN", (1.0 / 3.0));// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "CUBIT", "IN", 18.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "ELL", "IN", 45.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "FATHOM", "FT", 6.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_SEC", "[C]*S");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_MIN", "[C]*MIN");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_HOUR", "[C]*HR");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_YEAR", "[C]*YR");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "AU", "M", 1.495978707e11);// ); //, BISQNoDescript); //, BISQSecUom);
    }

void AddMass(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(MASS, SI, "G", "[MILLI]*KG");
    reg.AddSynonym(unit, "GRAM");
    unit = reg.AddUnit(MASS, SI, "MG", "[MILLI]*G");
    reg.AddSynonym(unit, "MILLIGRAM");
    unit = reg.AddUnit(MASS, SI, "MKG", "[MICRO]*G");
    reg.AddSynonym(unit, "MICROGRAM");
    unit = reg.AddUnit(MASS, SI, "NG", "[NANO]*G");
    reg.AddSynonym(unit, "NANOGRAM");
    reg.AddUnit(MASS, SI, "MEGAGRAM", "[MEGA]*G");
    //reg.AddUnit(MASS, SI, "TON", "[KILO]*KG");  // NOTE: REMOVED because TON is abigious, using TONNE, SHORT_TON and LONG_TON
    reg.AddUnit(MASS, SI, "TONNE", "[KILO]*KG"); // Also known as a metric ton http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.

    unit = reg.AddUnit(MASS, IMPERIAL, "LBM", "KG", 0.45359237); // TODO: Get reference
    reg.AddSynonym(unit, "POUND_MASS");
    reg.AddSynonym(unit, "POUND");
    unit = reg.AddUnit(MASS, IMPERIAL, "SLUG", "LBF*S(2)*FT(-1)");
    reg.AddSynonym(unit, "GEEPOUND");
    reg.AddUnit(MASS, USCUSTOM, "GRAIN", "LBM", 1.0 / 7000.0);

    reg.AddUnit(MASS, USCUSTOM, "SHORT_TON", "LBM", 2000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "LONG_TON", "LBM", 2240); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "KIP", "LBM", 1000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    }

void AddTime(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(TIME, SI, "MIN", "S", 60.0);
    reg.AddSynonym(unit, "MINUTE");
    unit = reg.AddUnit(TIME, SI, "HR", "MIN", 60.0);
    reg.AddSynonym(unit, "HOUR");
    reg.AddUnit(TIME, SI, "DAY", "HR", 24.0); //, BISQSecUom);
    reg.AddUnit(TIME, SI, "WEEK", "DAY", 7.0); //, BISQSecUom);
    reg.AddUnit(TIME, SI, "MONTH", "DAY", 30.0); //, BISQSecUom);
    unit = reg.AddUnit(TIME, SI, "YR", "DAY", 365);  //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Year is 3.1536 E+07 seconds
    reg.AddSynonym(unit, "YEAR");
    reg.AddUnit(TIME, SI, "YEAR_SIDEREAL", "S", 3.155815e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, SI, "YEAR_TROPICAL", "S", 3.155693e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    unit = reg.AddUnit(TIME, SI, "MS", "[MILLI]*S");
    reg.AddSynonym(unit, "MILLISECOND");
    unit = reg.AddUnit(TIME, SI, "MKS", "[MICRO]*S");
    reg.AddSynonym(unit, "MICROSECOND");

    }

//TODO: Handle temperature and delta temperature
void AddTemperature(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(TEMPERATURE, SI, "CELSIUS", "K", 1.0, 273.15);
    reg.AddSynonym(unit, "DEGREE_CELSIUS");
    unit = reg.AddUnit(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "CELSIUS", 5.0 / 9.0, -32);
    reg.AddSynonym(unit, "DEGREE_FAHRENHEIT");
    unit = reg.AddUnit(TEMPERATURE, USCUSTOM, "RANKINE", "K", 5.0 / 9.0);
    reg.AddSynonym(unit, "DEGREE_RANKINE");

    unit = reg.AddUnit(TEMPERATURE, USCUSTOM, "ROMER", "CELSIUS", 40.0 / 21.0, -7.5);
    reg.AddSynonym(unit, "DEGREE_ROMER");
    //unit = reg.AddUnit(TEMPERATURE, USCUSTOM, "REAMUR", "CELSIUS", 0.8);
    //reg.AddSynonym(unit, "DEGREE_REAMUR");

    }

void AddTemperatureChange(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_CHANGE, SI, "DELTA_CELSIUS", "DELTA_KELVIN", 1.0);
    reg.AddSynonym("DELTA_CELSIUS", "DELTA_DEGREE_CELSIUS");
    
    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_FAHRENHEIT", "DELTA_CELSIUS", 5.0 / 9.0);
    reg.AddSynonym("DELTA_FAHRENHEIT", "DELTA_DEGREE_FAHRENHEIT");
    
    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_RANKINE", "DELTA_KELVIN", 5.0 / 9.0);
    reg.AddSynonym("DELTA_RANKINE", "DELTA_DEGREE_RANKINE");
    }

void AddTemperatureGradient(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_GRADIENT, SI, "KELVIN/M", "DELTA_KELVIN*M(-1)");
    reg.AddSynonym("KELVIN/M", "DELTA_DEGREE_KELVIN_PER_METRE");
    }

void AddLinearThermalExpansionCoefficient(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/K", "DELTA_KELVIN(-1)");
    reg.AddSynonym("STRAIN/K", "RECIPROCAL_DELTA_DEGREE_KELVIN");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/CELSIUS", "DELTA_CELSIUS(-1)");
    reg.AddSynonym("STRAIN/CELSIUS", "RECIPROCAL_DELTA_DEGREE_CELSIUS");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, USCUSTOM, "STRAIN/FAHRENHEIT", "DELTA_FAHRENHEIT(-1)");
    reg.AddSynonym("STRAIN/FAHRENHEIT", "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, USCUSTOM, "STRAIN/RANKINE", "DELTA_RANKINE(-1)");
    reg.AddSynonym("STRAIN/RANKINE", "RECIPROCAL_DELTA_DEGREE_RANKINE");
    }

void AddLuminousFlux(UnitRegistry& reg)
    {
    reg.AddUnit(LUMINOUS_FLUX, SI, "LUMEN", "CANDELA*STERAD");
    }

void AddIlluminance(UnitRegistry& reg)
    {
    reg.AddUnit(ILLUMINANCE, SI, "LUX", "LUMEN*M(-2)");
    reg.AddUnit(ILLUMINANCE, USCUSTOM, "LUMEN/SQ.FT", "LUMEN*FT(-2)");
    reg.AddSynonym("LUMEN/SQ.FT", "LUMEN_PER_FOOT_SQUARED");
    }

void AddLuminosity(UnitRegistry& reg)
    {
    }

void AddMole(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(MOLE, SI, "KMOL", "[KILO]*MOL");
    reg.AddSynonym(unit, "KILOMOLE");
    unit = reg.AddUnit(MOLE, SI, "LB-MOLE", "MOL", 453.59237); // ASTM SI 10 standard SI1-.phhc8328.pdf page 29, 35 and http://en.wikipedia.org/wiki/Mole_%28unit%29
    reg.AddSynonym(unit, "POUND_MOLE");
    }

void AddCapita(UnitRegistry& reg)
    {
    reg.AddUnit(CAPITA, STATISTICS, "CAPITA", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "CUSTOMER", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "PASSENGER", "PERSON");// , BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "EMPLOYEE", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "STUDENT", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "GUEST", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "RESIDENT", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "HUNDRED_CAPITA", "PERSON", 100.0); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "THOUSAND_CAPITA", "[KILO]*PERSON");
    }

void AddFinance(UnitRegistry& reg)
    {
    }

void AddRatio(UnitRegistry& reg)
    {
    }

void AddRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, SI, "(N*M)/RAD", "N*M*RAD(-1)");
    reg.AddSynonym("(N*M)/RAD", "NEWTON_METRE_PER_RADIAN");

    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, SI, "(N*M)/DEG", "N*M*ARC_DEG(-1)");
    reg.AddSynonym("(N*M)/DEG", "NEWTON_METRE_PER_DEGREE");
    }

void AddLinearRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_ROTATIONAL_SPRING_CONSTANT, SI, "N/RAD", "N*RAD(-1)");
    reg.AddSynonym("N/RAD", "NEWTON_PER_RADIAN");
    }

void AddAcceleration(UnitRegistry& reg)
    {
    reg.AddUnit(ACCELERATION, SI, "M/SEC.SQUARED", "M*S(-2)"); //, BISQPrimUom);
    reg.AddUnit(ACCELERATION, SI, "CM/SEC.SQUARED", "CM*S(-2)"); //, BISQSecUom);
    reg.AddUnit(ACCELERATION, IMPERIAL, "FT/SEC.SQUARED", "FT*S(-2)"); //, BISQSecUom);
    }

void AddPlaneAngle(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(ANGLE, SI, "ARC_DEG", "[PI]*RAD", 1.0 / 180.0);
    reg.AddSynonym(unit, "DEGREE"); // Check validity of the synonym
    unit = reg.AddUnit(ANGLE, SI, "ARC_MINUTE", "ARC_DEG", 1.0 / 60.0);
    reg.AddSynonym(unit, "ANGLE_MINUTE");
    unit = reg.AddUnit(ANGLE, SI, "ARC_SECOND", "ARC_DEG", 1.0 / 3600.0);
    reg.AddSynonym(unit, "ANGLE_SECOND");
    unit = reg.AddUnit(ANGLE, SI, "ARC_QUADRANT", "[PI]*RAD", 0.5);
    reg.AddSynonym(unit, "ANGLE_QUADRANT");
    unit = reg.AddUnit(ANGLE, SI, "GRAD", "[PI]*RAD", 1.0 / 200.0);
    reg.AddSynonym(unit, "GRADIAN");
    reg.AddUnit(ANGLE, SI, "REVOLUTION", "[PI]*RAD", 2.0); //, BISQSecUom);
    }

void AddSolidAngle(UnitRegistry& reg)
    {
    }

void AddArea(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(AREA, SI, "SQ.M", "M(2)");
    reg.AddSynonym(unit, "METRE_SQUARED");
    unit = reg.AddUnit(AREA, SI, "SQ.MU", "MU(2)");
    reg.AddSynonym(unit, "MICRON_SQUARED");
    unit = reg.AddUnit(AREA, SI, "SQ.MM", "MM(2)");
    reg.AddSynonym(unit, "MILLIMETRE_SQUARED");
    unit = reg.AddUnit(AREA, SI, "SQ.CM", "CM(2)");
    reg.AddSynonym(unit, "CENTIMETRE_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.DM", "DM(2)"); //, BISQSecUom);
    unit = reg.AddUnit(AREA, SI, "SQ.KM", "KM(2)");
    reg.AddSynonym(unit, "KILOMETRE_SQUARED");
    reg.AddUnit(AREA, SI, "ARE", "[HECTO]*M(2)"); //, BISQSecUom);
    reg.AddUnit(AREA, SI, "HECTARE", "[HECTO]*ARE"); //, BISQSecUom);
    unit = reg.AddUnit(AREA, IMPERIAL, "SQ.IN", "IN(2)");
    reg.AddSynonym(unit, "INCH_SQUARED");
    unit = reg.AddUnit(AREA, IMPERIAL, "SQ.FT", "FT(2)");
    reg.AddSynonym(unit, "FOOT_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "THOUSAND_FOOT_SQUARED", "[KILO]*FT(2)"); // TODO: Make consistant name.
    unit = reg.AddUnit(AREA, IMPERIAL, "SQ.YRD", "YRD(2)");
    reg.AddSynonym(unit, "YARD_SQUARED");
    unit = reg.AddUnit(AREA, IMPERIAL, "SQ.MILE", "MILE(2)");
    reg.AddSynonym(unit, "MILE_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "ACRE", "CHAIN(2)", 10.0); //, BISQSecUom);
    }

void AddSizeLengthRate(UnitRegistry& reg)
    {
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "INCH_MILE", "IN*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "FOOT_MILE", "FT*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "FOOT_FOOT", "FT*FT");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "INCH_FOOT", "IN*FT");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "INCH_METRE", "IN*M");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MILLIMETRE_KILOMETRE", "MM*KM");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MILLIMETRE_METRE", "MM*M");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MILLIMETRE_MILE", "MM*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "METRE_KILOMETRE", "M*KM");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "METRE_METRE", "M*M");
    }

void AddDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(DENSITY, SI, "KG/CUB.M", "KG*M(-3)");
    reg.AddSynonym(unit, "KILOGRAM_PER_METRE_CUBED");
    unit = reg.AddUnit(DENSITY, SI, "KG/CUB.CM", "KG*CM(-3)");
    reg.AddSynonym(unit, "KILOGRAM_PER_CENTIMETRE_CUBED");
    unit = reg.AddUnit(DENSITY, SI, "KG/LITRE", "KG*DM(-3)");
    reg.AddSynonym(unit, "KILOGRAM_PER_DECIMETRE_CUBED");
    reg.AddSynonym(unit, "KILOGRAM_PER_LITRE");

    unit = reg.AddUnit(DENSITY, SI, "G/CUB.CM", "G*CM(-3)");
    reg.AddSynonym(unit, "GRAM_PER_CENTIMETRE_CUBED");

    unit = reg.AddUnit(DENSITY, SI, "MKG/LITRE", "MKG*DM(-3)");
    reg.AddSynonym(unit, "MICROGRAM_PER_LITRE");

    unit = reg.AddUnit(DENSITY, SI, "MG/LITRE", "MG*DM(-3)");
    reg.AddSynonym(unit, "MILLIGRAM_PER_LITRE");
    
    unit = reg.AddUnit(DENSITY, USCUSTOM, "LBM/CUB.FT", "LBM*FT(-3)");
    reg.AddSynonym(unit, "POUND_PER_FOOT_CUBED");
    unit = reg.AddUnit(DENSITY, USCUSTOM, "LBM/GAL", "LBM*GALLON(-1)");
    reg.AddSynonym(unit, "POUND_PER_GALLON");
    unit = reg.AddUnit(DENSITY, USCUSTOM, "LBM/GALLON_IMPERIAL", "LBM*GALLON_IMPERIAL(-1)");
    reg.AddSynonym(unit, "POUND_PER_IMPERIAL_GALLON");
    unit = reg.AddUnit(DENSITY, SI, "LBM/CUB.IN", "LBM*IN(-3)");
    reg.AddSynonym(unit, "POUND_PER_INCH_CUBED");
    
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/MILLION_GALLON", "LBM*GALLON(-1)", 1.0e-6);
    reg.AddSynonym("LBM/MILLION_GALLON", "POUND_PER_MILLION_GALLON");

    unit = reg.AddUnit(DENSITY, USCUSTOM, "SLUG/CUB.FT", "SLUG*FT(-3)");
    reg.AddSynonym(unit, "SLUG_PER_FOOT_CUBED");

    unit = reg.AddUnit(DENSITY, USCUSTOM, "KIP/CUB.FT", "KIP*FT(-3)");
    reg.AddSynonym(unit, "KIP_PER_FOOT_CUBED");

    reg.AddUnit(DENSITY, USCUSTOM, "SHORT_TON/CUB.FT", "SHORT_TON* FT(-3)");
    reg.AddSynonym("SHORT_TON/CUB.FT", "SHORT_TON_PER_FOOT_CUBED");
    }

void AddForceDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(FORCE_DENSITY, SI, "N/CUB.M", "N*M(-3)");
    reg.AddSynonym(unit, "NEWTON_PER_METRE_CUBED");
    unit = reg.AddUnit(FORCE_DENSITY, SI, "KN/CUB.M", "[KILO]*N*M(-3)");
    reg.AddSynonym(unit, "KILONEWTON_PER_METRE_CUBED");
    unit = reg.AddUnit(FORCE_DENSITY, USCUSTOM, "N/CUB.FT", "N*FT(-3)");
    reg.AddSynonym(unit, "NEWTON_PER_FOOT_CUBED");
    unit = reg.AddUnit(FORCE_DENSITY, USCUSTOM, "KN/CUB.FT", "[KILO]*N*FT(-3)");
    reg.AddSynonym(unit, "KILONEWTON_PER_FOOT_CUBED");
    }

void AddPopulationDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.M", "PERSON*M(-2)");
    reg.AddSynonym(unit, "PERSON_PER_METRE_SQUARED");
    unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/HECTARE", "PERSON*HECTARE(-1)");
    reg.AddSynonym(unit, "PERSON_PER_HECTARE");
    unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.KM", "PERSON*KM(-2)");
    reg.AddSynonym(unit, "PERSON_PER_KILOMETRE_SQUARED");
    unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/ACRE", "PERSON*ACRE(-1)");
    reg.AddSynonym(unit, "PERSON_PER_ACRE");
    unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.FT", "PERSON*FT(-2)");
    reg.AddSynonym(unit, "PERSON_PER_FOOT_SQUARED");
    unit = reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.MILE", "PERSON*MILE(-2)");
    reg.AddSynonym(unit, "PERSON_PER_MILE_SQUARED");
    }

void AddElectricCurrent(UnitRegistry& reg)
    {
    reg.AddUnit(CURRENT, SI, "KILOAMPERE", "[KILO]*A");
    reg.AddUnit(CURRENT, SI, "MILLIAMPERE", "[MILLI]*A"); //, BISQSecUom);
    reg.AddUnit(CURRENT, SI, "MICROAMPERE", "[MICRO]*A"); //, BISQSecUom);
    }

void AddElectricCharge(UnitRegistry& reg)
    {
    reg.AddUnit(ELECTRIC_CHARGE, SI, "COULOMB", "A*S");
    }

void AddElectricPotential(UnitRegistry& reg)
    {
    reg.AddUnit(ELECTRIC_POTENTIAL, SI, "VOLT", "N*M*COULOMB(-1)");
    reg.AddUnit(ELECTRIC_POTENTIAL, SI, "KILOVOLT", "[KILO]*VOLT");
    reg.AddUnit(ELECTRIC_POTENTIAL, SI, "MEGAVOLT", "[MEGA]*VOLT");
    }

void AddEnergy(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(WORK, SI, "J", "N*M");
    reg.AddSynonym(unit, "JOULE");
    unit = reg.AddUnit(WORK, SI, "KJ", "[KILO]*N*M");
    reg.AddSynonym(unit, "KILOJOULE");
    unit = reg.AddUnit(WORK, SI, "MJ", "[MEGA]*N*M");
    reg.AddSynonym(unit, "MEGAJOULE");
    unit = reg.AddUnit(WORK, SI, "GJ", "[GIGA]*N*M");
    reg.AddSynonym(unit, "GIGAJOULE");
    reg.AddUnit(WORK, USCUSTOM, "FOOT_POUNDAL", "POUNDAL*FT"); //, BISQSecUom);
    unit = reg.AddUnit(WORK, INTERNATIONAL, "BTU", "J", 1.05505585262e3); // Is IT BTU.  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.  See foot note #9: 
    reg.AddSynonym(unit, "BRITISH_THERMAL_UNIT");
    reg.AddUnit(WORK, USCUSTOM, "KILOBTU", "[KILO]*BTU"); //, BISQSecUom);

    unit = reg.AddUnit(WORK, INTERNATIONAL, "CAL", "J", 4.1868);
    reg.AddSynonym(unit, "CALORIE");
    unit = reg.AddUnit(WORK, INTERNATIONAL, "KCAL", "[KILO]*CAL");
    reg.AddSynonym(unit, "KILOCALORIE");
    unit = reg.AddUnit(WORK, INTERNATIONAL, "CUB.FT_ATM", "ATM*CUB.FT");
    reg.AddSynonym(unit, "CUBIC_FOOT_OF_ATMOSPHERE");
    unit = reg.AddUnit(WORK, INTERNATIONAL, "CUB.YRD_ATM", "ATM*CUB.YRD");
    reg.AddSynonym(unit, "CUBIC_YARD_OF_ATMOSPHERE");
    reg.AddUnit(WORK, USCUSTOM, "WATT_SECOND", "W*S"); //, BISQSecUom);
    unit = reg.AddUnit(WORK, INTERNATIONAL, "KWH", "KW*HOUR");
    reg.AddSynonym(unit, "KILOWATT_HOUR");
    unit = reg.AddUnit(WORK, INTERNATIONAL, "MWH", "MW*HOUR");
    reg.AddSynonym(unit, "MEGAWATT_HOUR");
    unit = reg.AddUnit(WORK, INTERNATIONAL, "GWH", "GW*HOUR");
    reg.AddSynonym(unit, "GIGAWATT_HOUR");
    }


// TODO: Check these phenomena, Energy Density?
void AddEnergyDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "J/CUB.M", "J*M(-3)");
    reg.AddSynonym(unit, "JOULE_PER_METRE_CUBED");

    unit = reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "KJ/CUB.M", "KJ*M(-3)");
    reg.AddSynonym(unit, "KILOJOULE_PER_METRE_CUBED");

    unit = reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "KWH/CUB.M", "KWH*M(-3)");
    reg.AddSynonym(unit, "KILOWATT_HOUR_PER_METRE_CUBED");

    unit = reg.AddUnit(HEATING_VALUE_VOLUMETRIC, USCUSTOM, "KWH/CUB.FT", "KWH*FT(-3)");
    reg.AddSynonym(unit, "KILOWATT_HOUR_PER_FOOT_CUBED");

    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, USCUSTOM, "KWH/MILLION_GALLON", "KWH*GALLON(-1)", 1.0e-6);
    reg.AddSynonym("KWH/MILLION_GALLON", "KILOWATT_HOUR_PER_MILLION_GALLON");
    }

void AddHeatingValue(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(HEATING_VALUE_MASS, SI, "J/KG", "J*KG(-1)");  // NOTE: Changed from J*M(-3)
    unit = reg.AddUnit(HEATING_VALUE_MASS, SI, "KJ/KG", "KJ*KG(-1)");
    reg.AddSynonym(unit, "KILOJOULE_PER_KILOGRAM");
    unit = reg.AddUnit(HEATING_VALUE_MASS, SI, "MEGAJ/KG", "MJ*KG(-1)");
    reg.AddSynonym(unit, "MEGAJOULE_PER_KILOGRAM");

    unit = reg.AddUnit(HEATING_VALUE_MASS, USCUSTOM, "BTU/LBM", "BTU*LBM(-1)");
    reg.AddSynonym(unit, "BTU_PER_POUND_MASS");
    //unit = reg.AddUnit(HEATING_VALUE_VOLUMETRIC, USCUSTOM, "BTU_PER_POUND_MOLE", "M_L2_PER_T2_MOL"); // TODO: Make expression and check dimension, add KILOJOULE_PER_KILOMOLE
    
    }

void AddSpecificHeatCapacity(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, SI, "J/(KG*K)", "J*KG(-1)*DELTA_KELVIN(-1)");
    reg.AddSynonym("J/(KG*K)", "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, USCUSTOM, "BTU/(LBM*RANKINE)", "BTU*LBM(-1)*DELTA_RANKINE(-1)");
    reg.AddSynonym("BTU/(LBM*RANKINE)", "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE");
    }

void AddSpecificHeatCapacityMolar(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "J/(KMOL*K)", "J*KMOL(-1)*DELTA_KELVIN(-1)");
    reg.AddSynonym("J/(KMOL*K)", "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "KJ/(KMOL*K)", "KJ*KMOL(-1)*DELTA_KELVIN(-1)");
    reg.AddSynonym("KJ/(KMOL*K)", "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "BTU/(LB-MOLE*RANKINE)", "BTU*LB-MOLE(-1)*DELTA_RANKINE(-1)");
    reg.AddSynonym("BTU/(LB-MOLE*RANKINE)", "BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE");
    }

void AddVolumeFlowRateByArea(UnitRegistry& reg)
    {

    }

void AddVolumeFlowRate(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(FLOW, SI, "CUB.M/SEC", "M(3)*S(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_SECOND");
    unit = reg.AddUnit(FLOW, SI, "CUB.M/MIN", "M(3)*MIN(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_MINUTE");
    unit = reg.AddUnit(FLOW, SI, "CUB.M/HR", "M(3)*HR(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_HOUR");
    unit = reg.AddUnit(FLOW, SI, "CUB.M/DAY", "M(3)*DAY(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_DAY");
    unit = reg.AddUnit(FLOW, SI, "LITRE/SEC", "LITRE*S(-1)");
    reg.AddSynonym(unit, "LITRE_PER_SECOND");
    unit = reg.AddUnit(FLOW, SI, "LITRE/MIN", "LITRE*MIN(-1)");
    reg.AddSynonym(unit, "LITRE_PER_MINUTE");
    unit = reg.AddUnit(FLOW, SI, "LITRE/HR", "LITRE*HR(-1)");
    reg.AddSynonym(unit, "LITRE_PER_HOUR");
    unit = reg.AddUnit(FLOW, SI, "LITRE/DAY", "LITRE*DAY(-1)");
    reg.AddSynonym(unit, "LITRE_PER_DAY");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/SEC", "CUB.IN*S(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/MIN", "CUB.IN*MIN(-1)"); //, BISQSecUom);
    unit = reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/SEC", "CUB.FT*S(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_SECOND");
    unit = reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/MIN", "CUB.FT*MIN(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_MINUTE");
    unit = reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/DAY", "CUB.FT*DAY(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_DAY");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/DAY", "ACRE_FOOT*DAY(-1)");
    reg.AddSynonym(unit, "ACRE_FOOT_PER_DAY");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/HR", "ACRE_FOOT*HR(-1)");
    reg.AddSynonym(unit, "ACRE_FOOT_PER_HOUR");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/MIN", "ACRE_FOOT*MIN(-1)");
    reg.AddSynonym(unit, "ACRE_FOOT_PER_MINUTE");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/DAY", "ACRE_INCH*DAY(-1)");
    reg.AddSynonym(unit, "ACRE_INCH_PER_DAY");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/HOUR", "ACRE_INCH*HR(-1)");
    reg.AddSynonym(unit, "ACRE_INCH_PER_HOUR");
    unit = reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/MIN", "ACRE_INCH*MIN(-1)");
    reg.AddSynonym(unit, "ACRE_INCH_PER_MINUTE");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL*DAY(-1)"); //, BISQSecUom);
    reg.AddSynonym("GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL_PER_DAY");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/MINUTE", "GALLON_IMPERIAL*MIN(-1)"); //, BISQSecUom);
    reg.AddSynonym("GALLON_IMPERIAL/MINUTE", "GALLON_IMPERIAL_PER_MINUTE");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/SECOND", "GALLON_IMPERIAL*S(-1)"); //, BISQSecUom);
    reg.AddSynonym("GALLON_IMPERIAL/SECOND", "GALLON_IMPERIAL_PER_SECOND");

    unit = reg.AddUnit(FLOW, USCUSTOM, "GALLON/S", "GALLON*S(-1)");
    reg.AddSynonym(unit, "GALLON_PER_SECOND");
    unit = reg.AddUnit(FLOW, USCUSTOM, "GALLON/MIN", "GALLON*MIN(-1)");
    reg.AddSynonym(unit, "GALLON_PER_MINUTE");
    unit = reg.AddUnit(FLOW, USCUSTOM, "GALLON/DAY", "GALLON*DAY(-1)");
    reg.AddSynonym(unit, "GALLON_PER_DAY");
    }

void AddFrequency(UnitRegistry& reg)
    {
    reg.AddUnit(FREQUENCY, SI, "HERTZ", "S(-1)"); //, BISQPrimUom);
    UnitCP unit = reg.AddUnit(FREQUENCY, SI, "KH", "[KILO]*S(-1)");
    reg.AddSynonym(unit, "KILOHERTZ");
    unit = reg.AddUnit(FREQUENCY, SI, "MH", "[MEGA]*S(-1)");
    reg.AddSynonym(unit, "MEGAHERTZ");
    }

void AddSurfaceFlowRate(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(SECOND*SQ.M)", "M*S(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_METRE_SQUARED_PER_SECOND");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(DAY*SQ.M)", "M*DAY(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_METRE_SQUARED_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*HECTARE)", "CUB.M*HECTARE(-1)*DAY(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_HECTARE_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*SQ.KM)", "CUB.M*KM(-2)*DAY(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*MIN)", "FT*MIN(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*S)", "FT*S(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.MILE*S)", "FT(3)*MILE(-2)*S(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*ACRE)", "GALLON*DAY(-1)*ACRE(-1)");
    reg.AddSynonym(unit, "GALLON_PER_ACRE_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*ACRE)", "GALLON*MIN(-1)*ACRE(-1)");
    reg.AddSynonym(unit, "GALLON_PER_ACRE_PER_MINUTE");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.FT)", "GALLON*MIN(-1)*FT(-2)");
    reg.AddSynonym(unit, "GALLON_PER_FOOT_SQUARED_PER_MINUTE");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.FT)", "GALLON*DAY(-1)*FT(-2)");
    reg.AddSynonym(unit, "GALLON_PER_FOOT_SQUARED_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.MILE)", "GALLON*MIN(-1)*MILE(-2)");
    reg.AddSynonym(unit, "GALLON_PER_MILE_SQUARED_PER_MINUTE");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.MILE)", "GALLON*DAY(-1)*MILE(-2)");
    reg.AddSynonym(unit, "GALLON_PER_MILE_SQUARED_PER_DAY");
    unit = reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(ACRE*SEC)", "FT(3)*ACRE(-1)*S(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_ACRE_PER_SECOND");
    }

void AddMassRatio(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_RATIO, SI, "KILOGRAM_PER_KILOGRAM", "KG*KG(-1)");
    reg.AddUnit(MASS_RATIO, USCUSTOM, "GRAIN_MASS_PER_POUND_MASS", "GRAIN*LBM(-1)");
    }

void AddMassFlowRate(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(MASS_FLOW, SI, "KG/S", "KG*S(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_SECOND");
    unit = reg.AddUnit(MASS_FLOW, SI, "KG/MIN", "KG*MIN(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_MINUTE");
    unit = reg.AddUnit(MASS_FLOW, SI, "KG/HR", "KG*HR(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_HOUR");
    unit = reg.AddUnit(MASS_FLOW, SI, "KG/DAY", "KG*DAY(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_DAY");

    unit = reg.AddUnit(MASS_FLOW, SI, "G/S", "GRAM*S(-1)");
    reg.AddSynonym(unit, "GRAM_PER_SECOND");
    unit = reg.AddUnit(MASS_FLOW, SI, "G/MIN", "GRAM*MIN(-1)");
    reg.AddSynonym(unit, "GRAM_PER_MINUTE");
    unit = reg.AddUnit(MASS_FLOW, SI, "G/HR", "GRAM*HR(-1)");
    reg.AddSynonym(unit, "GRAM_PER_HOUR");
    unit = reg.AddUnit(MASS_FLOW, SI, "MG/S", "MG*S(-1)");
    reg.AddSynonym(unit, "MILLIGRAM_PER_SECOND");
    unit = reg.AddUnit(MASS_FLOW, SI, "MG/MIN", "MG*MIN(-1)");
    reg.AddSynonym(unit, "MILLIGRAM_PER_MINUTE");
    unit = reg.AddUnit(MASS_FLOW, SI, "MG/HR", "MG*HR(-1)");
    reg.AddSynonym(unit, "MILLIGRAM_PER_HOUR");
    unit = reg.AddUnit(MASS_FLOW, SI, "MG/DAY", "MG*DAY(-1)");
    reg.AddSynonym(unit, "MILLIGRAM_PER_DAY");
    unit = reg.AddUnit(MASS_FLOW, SI, "MKG/S", "MKG*S(-1)");
    reg.AddSynonym(unit, "MICROGRAM_PER_SECOND");
    unit = reg.AddUnit(MASS_FLOW, SI, "MKG/MIN", "MKG*MIN(-1)");
    reg.AddSynonym(unit, "MICROGRAM_PER_MINUTE");
    unit = reg.AddUnit(MASS_FLOW, SI, "MKG/HR", "MKG*HR(-1)");
    reg.AddSynonym(unit, "MICROGRAM_PER_HOUR");
    unit = reg.AddUnit(MASS_FLOW, SI, "MKG/DAY", "MKG*DAY(-1)");
    reg.AddSynonym(unit, "MICROGRAM_PER_DAY");
    unit = reg.AddUnit(MASS_FLOW, SI, "LB/S", "LBM*S(-1)");
    reg.AddSynonym(unit, "POUND_PER_SECOND");
    unit = reg.AddUnit(MASS_FLOW, SI, "LB/MIN", "LBM*MIN(-1)");
    reg.AddSynonym(unit, "POUND_PER_MINUTE");
    unit = reg.AddUnit(MASS_FLOW, SI, "LB/HR", "LBM*HR(-1)");
    reg.AddSynonym(unit, "POUND_PER_HOUR");
    unit = reg.AddUnit(MASS_FLOW, SI, "LB/DAY", "LBM*DAY(-1)");
    reg.AddSynonym(unit, "POUND_PER_DAY");
    unit = reg.AddUnit(MASS_FLOW, SI, "TONNE/HR", "TONNE*HR(-1)");
    reg.AddSynonym(unit, "TONNE_PER_HOUR");

    unit = reg.AddUnit(MASS_FLOW, USCUSTOM, "SHORT_TON/HR", "SHORT_TON*HR(-1)");
    reg.AddSynonym(unit, "SHORT_TON_PER_HOUR");
    }

void AddParticleFlowRate(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(PARTICLE_FLOW, SI, "MOL/S", "MOL*S(-1)");
    reg.AddSynonym(unit, "MOLE_PER_SECOND");
    unit = reg.AddUnit(PARTICLE_FLOW, SI, "KMOL/S", "[KILO]*MOL*S(-1)");
    reg.AddSynonym(unit, "KILOMOLE_PER_SECOND");
    }

void AddForce(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(FORCE, SI, "N", "KG*M*S(-2)");
    reg.AddSynonym(unit, "NEWTON");
    unit = reg.AddUnit(FORCE, SI, "KN", "[KILO]*N");
    reg.AddSynonym(unit, "KILONEWTON");
    unit = reg.AddUnit(FORCE, SI, "mN", "[MILLI]*N");
    reg.AddSynonym(unit, "MILLINEWTON");
    unit = reg.AddUnit(FORCE, SI, "KGF", "[STD_G]*KG");
    reg.AddSynonym(unit, "KILOGRAM_FORCE");
    reg.AddSynonym(unit, "KILOPOND");
    unit = reg.AddUnit(FORCE, SI, "LBF", "[STD_G]*LBM");
    reg.AddSynonym(unit, "POUND_FORCE");
    unit = reg.AddUnit(FORCE, SI, "KPF", "[KILO]*LBF");
    reg.AddSynonym(unit, "KILOPOUND_FORCE");
    reg.AddUnit(FORCE, SI, "DYNE", "G*CM*S(-2)"); //, BISQSecUom);
    unit = reg.AddUnit(FORCE, SI, "PDL", "LBM*FT*S(-2)");
    reg.AddSynonym(unit, "POUNDAL");

    reg.AddUnit(FORCE, USCUSTOM, "SHORT_TON_FORCE", "[STD_G]*SHORT_TON");
    reg.AddUnit(FORCE, USCUSTOM, "LONG_TON_FORCE", "[STD_G]*LONG_TON");
    }

void AddHeatFlux(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_FLUX_DENSITY, SI, "W/SQ.M", "W*M(-2)"); //, BISQPrimUom);
    }

// TODO: Thermal Transmittance?
void AddHeatTransfer(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_TRANSFER, SI, "W/(SQ.M*K)", "W*M(-2)*DELTA_KELVIN(-1)");
    reg.AddSynonym("W/(SQ.M*K)", "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN");
    
    reg.AddUnit(HEAT_TRANSFER, SI, "W/(SQ.M*CELSIUS)", "W*M(-2)*DELTA_CELSIUS(-1)");
    reg.AddSynonym("W/(SQ.M*CELSIUS)", "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS");
    
    reg.AddUnit(HEAT_TRANSFER, USCUSTOM, "BTU/(SQ.FT*HR*FAHRENHEIT)", "BTU*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    reg.AddSynonym("BTU/(SQ.FT*HR*FAHRENHEIT)", "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT");
    }

void AddLinearDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(LINEAR_DENSITY, SI, "KG/M", "KG*M(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_METRE");
    unit = reg.AddUnit(LINEAR_DENSITY, SI, "KG/MM", "KG*MM(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_MILLIMETRE");
    reg.AddUnit(LINEAR_DENSITY, USCUSTOM, "LBM/FT", "LBM*FT(-1)");
    reg.AddSynonym("LBM/FT", "POUND_MASS_PER_FOOT");
    }

void AddLinearLoad(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(LINEAR_LOAD, SI, "N/M", "N*M(-1)");
    reg.AddSynonym(unit, "NEWTON_PER_METRE");
    unit = reg.AddUnit(LINEAR_LOAD, SI, "N/MM", "N*MM(-1)");
    reg.AddSynonym(unit, "NEWTON_PER_MILLIMETRE");
    unit = reg.AddUnit(LINEAR_LOAD, SI, "LBF/IN", "LBF*IN(-1)");
    reg.AddSynonym(unit, "POUND_FORCE_PER_INCH");
    }

void AddLinearCost(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_COST, FINANCE, "$/M", "US$*M(-1)");
    reg.AddUnit(LINEAR_COST, FINANCE, "$/MM", "US$*MM(-1)");
    }

void AddLinearRate(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_RATE, SI, "PER_M", "M(-1)");
    reg.AddSynonym("PER_M", "ONE_PER_METRE");

    reg.AddUnit(LINEAR_RATE, SI, "PER_MM", "MM(-1)");
    reg.AddSynonym("PER_MM", "ONE_PER_MILLIMETRE");

    reg.AddUnit(LINEAR_RATE, SI, "PER_KM", "KM(-1)");
    reg.AddSynonym("PER_KM", "ONE_PER_KILOMETRE");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_FT", "FT(-1)");
    reg.AddSynonym("PER_FT", "ONE_PER_FOOT");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_MILE", "MILE(-1)");
    reg.AddSynonym("PER_MILE", "ONE_PER_MILE");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_THOUSAND_FT", "FT(-1)", 1.0e-3);
    reg.AddSynonym("PER_THOUSAND_FT", "ONE_PER_THOUSAND_FOOT");
    }

void AddTorque(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(TORQUE, SI, "N_M", "N*M*RAD");
    reg.AddSynonym(unit, "NEWTON_METRE");
    unit = reg.AddUnit(TORQUE, SI, "N_CM", "N*CM*RAD");
    reg.AddSynonym(unit, "NEWTON_CENTIMETRE");
    unit = reg.AddUnit(TORQUE, USCUSTOM, "LBF_FT", "LBF*FT*RAD");
    reg.AddSynonym(unit, "POUND_FOOT");
    }

void AddMolarVolume(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/MOLE", "CUB.M*MOLE(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_MOLE");
    unit = reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/KMOL", "CUB.M*MOLE(-1)", 1.0e-3);
    reg.AddSynonym(unit, "METRE_CUBED_PER_KILOMOLE");
    unit = reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.FT/LB-MOLE", "CUB.FT*LB-MOLE(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_POUND_MOLE");
    }

void AddMolarConcentration(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.M", "MOLE*CUB.M(-1)");
    reg.AddSynonym(unit, "MOLE_PER_METRE_CUBED");
    reg.AddSynonym(unit, "MILLIMOLAR");
    unit = reg.AddUnit(MOLAR_CONCENTRATION, SI, "KMOL/CUB.M", "[KILO]*MOLE*CUB.M(-1)");
    reg.AddSynonym(unit, "KILOMOLE_PER_METRE_CUBED");  // deprecate
    unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.DM", "MOLE*CUB.DM(-1)");
    reg.AddSynonym(unit, "MOLE_PER_LITRE");
    reg.AddSynonym(unit, "MOLAR");
    unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MICROMOL/CUB.DM", "[MICRO]*MOLE*CUB.DM(-1)");
    reg.AddSynonym(unit, "MICROMOLAR");
    unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "NMOL/CUB.DM", "[NANO]*MOLE*CUB.DM(-1)");
    reg.AddSynonym(unit, "NANOMOLAR");
    unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "PICOMOL/CUB.DM", "[PICO]*MOLE*CUB.DM(-1)");
    reg.AddSynonym(unit, "PICOMOLAR");
    unit = reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.FT", "MOLE*CUB.FT(-1)");
    reg.AddSynonym(unit, "MOLE_PER_FOOT_CUBED");
    }

// NOTE: Changed to Area moment of inertia based on Wiki pages on moment of inertia and area moment of inertia
void AddMomentOfInertia(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "M^4", "M(4)");
    reg.AddSynonym(unit, "METRE_TO_THE_FOURTH");
    unit = reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "CM^4", "CM(4)");
    reg.AddSynonym(unit, "CENTIMETRE_TO_THE_FOURTH");
    unit = reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "IN^4", "IN(4)");
    reg.AddSynonym(unit, "INCH_TO_THE_FOURTH");
    unit = reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "FT^4", "FT(4)");
    reg.AddSynonym(unit, "FOOT_TO_THE_FOURTH");
    unit = reg.AddUnit(AREA_MOMENT_INERTIA, SI, "MM^4", "MM(4)");
    reg.AddSynonym(unit, "MILLIMETRE_TO_THE_FOURTH");
    }

void AddPower(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(POWER, INTERNATIONAL, "W", "N*M*S(-1)");
    reg.AddSynonym(unit, "WATT");
    unit = reg.AddUnit(POWER, INTERNATIONAL, "KW", "[KILO]*W");
    reg.AddSynonym(unit, "KILOWATT");
    unit = reg.AddUnit(POWER, INTERNATIONAL, "MW", "[MEGA]*W");
    reg.AddSynonym(unit, "MEGAWATT");
    unit = reg.AddUnit(POWER, INTERNATIONAL, "GW", "[GIGA]*W");
    reg.AddSynonym(unit, "GIGAWATT");
    unit = reg.AddUnit(POWER, INTERNATIONAL, "BTU/MONTH", "BTU*MONTH(-1)");
    reg.AddSynonym(unit, "BTU_PER_MONTH");
    unit = reg.AddUnit(POWER, INTERNATIONAL, "BTU/HOUR", "BTU*HOUR(-1)");
    reg.AddSynonym(unit, "BTU_PER_HOUR");
    unit = reg.AddUnit(POWER, USCUSTOM, "KILOBTU/HOUR", "[KILO]*BTU*HOUR(-1)");
    reg.AddSynonym(unit, "KILOBTU_PER_HOUR");
    unit = reg.AddUnit(POWER, USCUSTOM, "HP", "LBF*FT*S(-1)", 550.0);
    reg.AddSynonym(unit, "HORSEPOWER");

    unit = reg.AddUnit(POWER, SI, "GJ/MONTH", "GJ*MONTH(-1)");
    reg.AddSynonym(unit, "GIGAJOULE_PER_MONTH");
    }

// TODO: Wrong
//void AddPowerDensity(UnitRegistry& reg)
//    {
//    reg.AddUnit(BISPH_LENGTH, USCUSTOM, "BTU_PER_HOUR_PER_FOOT_CUBED", "M_PER_L_T3", 0.09662109117546, 0.0); //, BISQNoDescript); //, BISQSecUom);
//    }

void AddPressure(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(PRESSURE, SI, "PA", "N*M(-2)");
    reg.AddSynonym(unit, "PASCAL");
    reg.AddSynonym(unit, "NEWTON_PER_METRE_SQUARED");

    reg.AddUnit(PRESSURE, SI, "PA_GAUGE", "PA", 1, -101325); // TODO: Use constant for this

    // TODO: See if this is equal to another unit here.
    reg.AddUnit(PRESSURE, SI, "N/SQ.MM", "N*MM(-2)");
    reg.AddSynonym("N/SQ.MM", "NEWTON_PER_MILLIMETRE_SQUARED");

    reg.AddUnit(PRESSURE, SI, "HECTOPASCAL", "[HECTO]*PA"); //, BISQSecUom);

    reg.AddUnit(PRESSURE, SI, "KILOPASCAL", "[KILO]*PA"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, SI, "KILOPASCAL_GAUGE", "[KILO]*PA", 1, -101325e-3); // TODO: Use constant for this

    reg.AddUnit(PRESSURE, SI, "MEGAPASCAL", "[MEGA]*PA");
    reg.AddUnit(PRESSURE, SI, "MEGAPASCAL_GAUGE", "[MEGA]*PA", 1, -101325e-6); // TODO: Use constant for this

    unit = reg.AddUnit(PRESSURE, INTERNATIONAL, "AT", "KGF*CM(-2)");  // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddSynonym(unit, "ATMOSPHERE_TECHNIAL");
    reg.AddSynonym(unit, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED");

    unit = reg.AddUnit(PRESSURE, INDUSTRIAL, "AT_GAUGE", "AT", 1.0, -1.0332274527998859); // TODO: double check, used 101325 PA -> AT conversion
    reg.AddSynonym(unit, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE");

    unit = reg.AddUnit(PRESSURE, INTERNATIONAL, "KGF/SQ.M", "KGF*M(-2)");
    reg.AddSynonym(unit, "KILOGRAM_FORCE_PER_METRE_SQUARED");

    unit = reg.AddUnit(PRESSURE, SI, "ATM", "PA", 101325);  // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  Is standard atmosphere
    reg.AddSynonym(unit, "ATMOSPHERE");

    reg.AddUnit(PRESSURE, SI, "BAR", "PA", 1.0e5); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(PRESSURE, INDUSTRIAL, "BAR_GAUGE", "PA", 1.0e5, -1.01325); //, BISQNoDescript); //, BISQSecUom);

    unit = reg.AddUnit(PRESSURE, SI, "MBAR", "[MILLI]*BAR");
    reg.AddSynonym(unit, "MILLIBAR");

    reg.AddUnit(PRESSURE, CGS, "BARYE", "PA", 0.1); //, BISQSecUom);   // 1.0 dyn/sq.cm


    unit = reg.AddUnit(PRESSURE, USCUSTOM, "PSI", "LBF*IN(-2)");
    reg.AddSynonym(unit, "POUND_FORCE_PER_INCH_SQUARED");

    unit = reg.AddUnit(PRESSURE, USCUSTOM, "PSIG", "LBF*IN(-2)", 1, -14.695948775513449); // TODO: double check, used 101325 PA -> PSI conversion
    reg.AddSynonym(unit, "POUND_FORCE_PER_INCH_SQUARED_GAUGE");

    reg.AddUnit(PRESSURE, USCUSTOM, "KSI", "[KILO]*LBF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "LBF/SQ.FT", "LBF*FT(-2)");
    reg.AddSynonym("LBF/SQ.FT", "POUND_FORCE_PER_FOOT_SQUARED");

    reg.AddUnit(PRESSURE, USCUSTOM, "TORR", "PA", 1.333224e2);   // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.

    // TODO: Go back to density conversion once we have verified sources for those values
    unit = reg.AddUnit(PRESSURE, SI, "METRE_OF_H2O_CONVENTIONAL", "[KILO]*MILLIMETRE_OF_H2O_CONVENTIONAL");

    reg.AddUnit(PRESSURE, SI, "MILLIMETRE_OF_H2O_CONVENTIONAL", "PA", 9.80665); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "FOOT_OF_H2O_CONVENTIONAL", "PA", 2.989067e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_32_FAHRENHEIT", "PA", 249.1083); // Water is assumed to be in a liquid state. Equal to water at 0C.  No verified source
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", "PA", 2.49082e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_60_FAHRENHEIT", "PA", 2.4884e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    
    reg.AddUnit(PRESSURE, USCUSTOM, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", "PA", 1.33322e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  Used centimeter of mercury (0 C) to pascal
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_HG_CONVENTIONAL", "PA", 3.386389e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_HG_AT_32_FAHRENHEIT", "PA", 3.38638e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_HG_AT_60_FAHRENHEIT", "PA", 3.37685e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    }

void AddPressureGradient(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(PRESSURE_GRADIENT, SI, "PA/M", "PA*M(-1)");
    reg.AddSynonym(unit, "PASCAL_PER_METRE");
    unit = reg.AddUnit(PRESSURE_GRADIENT, SI, "BAR/KM", "BAR*KM(-1)");
    reg.AddSynonym(unit, "BAR_PER_KILOMETRE");
    }

void AddPercentage(UnitRegistry& reg)
    {
    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "PERCENT", "ONE");
    reg.AddSynonym("PERCENT", "PERCENT_PERCENT");

    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "DECIMAL_PERCENT", "PERCENT", 100);
    reg.AddSynonym("DECIMAL_PERCENT", "UNITLESS_PERCENT");
    }

// TODO: Handle inverted slopes
void AddSlope(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(SLOPE, SI, "M/M", "M*M(-1)");
    reg.AddSynonym(unit, "METRE_PER_METRE"); 
    reg.AddSynonym(unit, "METRE_VERTICAL_PER_METRE_HORIZONTAL");

    unit = reg.AddUnit(SLOPE, SI, "CM/M", "CM*M(-1)");
    reg.AddSynonym(unit, "CENTIMETRE_PER_METRE");
    unit = reg.AddUnit(SLOPE, SI, "MM/M", "MM*M(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_METRE");
    unit = reg.AddUnit(SLOPE, SI, "M/KM", "M*KM(-1)");
    reg.AddSynonym(unit, "METRE_PER_KILOMETRE");
    reg.AddUnit(SLOPE, USCUSTOM, "FOOT_PER_1000_FOOT", "FT*FT(-1)", 1.0e-3); //, BISQSecUom);
    unit = reg.AddUnit(SLOPE, USCUSTOM, "FT/FT", "FT*FT(-1)");
    reg.AddSynonym("FT/FT", "FOOT_PER_FOOT");
    reg.AddSynonym("FT/FT","FOOT_VERTICAL_PER_FOOT_HORIZONTAL");


    unit = reg.AddUnit(SLOPE, USCUSTOM, "IN/FT", "IN*FT(-1)");
    reg.AddSynonym(unit, "INCH_PER_FOOT");
    unit = reg.AddUnit(SLOPE, USCUSTOM, "FT/IN", "FT*IN(-1)");
    reg.AddSynonym(unit, "FOOT_PER_INCH");
    unit = reg.AddUnit(SLOPE, USCUSTOM, "FT/MILE", "FT*MILE(-1)");
    reg.AddSynonym(unit, "FOOT_PER_MILE");
    
    reg.AddUnit(SLOPE, INTERNATIONAL, "VERTICAL_PER_HORIZONTAL", "M/M");

    reg.AddUnit(SLOPE, INTERNATIONAL, "PERCENT_SLOPE", "M/M", 1.0e-2);

    reg.AddInvertingUnit("VERTICAL_PER_HORIZONTAL", "HORIZONTAL_PER_VERTICAL");
    reg.AddInvertingUnit("FT/FT", "FOOT_HORIZONTAL_PER_FOOT_VERTICAL");
    reg.AddInvertingUnit("M/M", "METRE_HORIZONTAL_PER_METRE_VERTICAL");
    }

void AddSurfaceDensity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(SURFACE_DENSITY, SI, "KG/SQ.M", "KG*M(-2)");
    reg.AddSynonym(unit, "KILOGRAM_PER_METRE_SQUARED");
    unit = reg.AddUnit(SURFACE_DENSITY, SI, "G/SQ.M", "G*M(-2)");
    reg.AddSynonym(unit, "GRAM_PER_METRE_SQUARED");
    unit = reg.AddUnit(SURFACE_DENSITY, SI, "KG/HECTARE", "KG*HECTARE(-1)");
    reg.AddSynonym(unit, "KILOGRAM_PER_HECTARE");
    unit = reg.AddUnit(SURFACE_DENSITY, USCUSTOM, "LB/ACRE", "LBM*ACRE(-1)");
    reg.AddSynonym(unit, "POUND_PER_ACRE");
    }

void AddThermalConductivity(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "W/(M*K)", "W*M(-1)*DELTA_KELVIN(-1)");
    reg.AddSynonym("W/(M*K)", "WATT_PER_METRE_PER_DEGREE_KELVIN");
    reg.AddUnit(THERMAL_CONDUCTIVITY, SI, "W/(M*C)", "W*M(-1)*DELTA_CELSIUS(-1)");
    reg.AddSynonym("W/(M*C)", "WATT_PER_METRE_PER_DEGREE_CELSIUS");

    reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "BTU*IN*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    reg.AddSynonym("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT");
    
    
    //reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "BTU_PER_HOUR_PER_FOOT_PER_DELTA_DEGREE_FAHRENHEIT", "BTU*HR(-1)*FT(-1)*DELTA_FAHRENHEIT(-1)"); //, BISQSecUom);
    //LoadUOM(THERMAL_CONDUCTIVITY, INDUSTRIAL, "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", "M_PER_T3_K", 0.1761101836823, 0.0); //, BISQNoDescript); //, BISQSecUom);
    //LoadUOM(THERMAL_CONDUCTIVITY, USCUSTOM, "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", "M_L_PER_T3_K", 6.933471798516, 0.0)->AddSynonym("BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT");
    }

void AddThermalResistance(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_RESISTANCE, SI, "(SQ.M*KELVIN)/WATT", "M(2)*DELTA_KELVIN*WATT(-1)");
    reg.AddSynonym("(SQ.M*KELVIN)/WATT", "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT");

    reg.AddUnit(THERMAL_RESISTANCE, SI, "(SQ.M*CELSIUS)/WATT", "M(2)*DELTA_CELSIUS*WATT(-1)");
    reg.AddSynonym("(SQ.M*CELSIUS)/WATT", "METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT");

    reg.AddUnit(THERMAL_RESISTANCE, USCUSTOM, "(SQ.FT*HR*FAHRENHEIT)/BTU", "FT(2)*HR*DELTA_FAHRENHEIT*BTU(-1)");
    reg.AddSynonym("(SQ.FT*HR*FAHRENHEIT)/BTU", "FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU");
    }

void AddThreadPitch(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(THREAD_PITCH, SI, "M/REVOLUTION", "M*REVOLUTION(-1)");
    reg.AddSynonym(unit, "METRE_PER_REVOLUTION");
    unit = reg.AddUnit(THREAD_PITCH, SI, "CM/REVOLUTION", "CM*REVOLUTION(-1)");
    reg.AddSynonym(unit, "CENTIMETRE_PER_REVOLUTION");
    unit = reg.AddUnit(THREAD_PITCH, SI, "MM/REVOLUTION", "MM*REVOLUTION(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_REVOLUTION");
    unit = reg.AddUnit(THREAD_PITCH, SI, "M/RAD", "M*RAD(-1)");
    reg.AddSynonym(unit, "METRE_PER_RADIAN");
    unit = reg.AddUnit(THREAD_PITCH, SI, "M/DEGREE", "M*ARC_DEG(-1)");
    reg.AddSynonym(unit, "METRE_PER_DEGREE");
    unit = reg.AddUnit(THREAD_PITCH, SI, "MM/RAD", "MM*RAD(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_RADIAN");

    unit = reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/REVOLUTION", "IN*REVOLUTION(-1)");
    reg.AddSynonym(unit, "INCH_PER_REVOLUTION");
    unit = reg.AddUnit(THREAD_PITCH, USCUSTOM, "FT/REVOLUTION", "FT*REVOLUTION(-1)");
    reg.AddSynonym(unit, "FOOT_PER_REVOLUTION");
    unit = reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/DEGREE", "IN*ARC_DEG(-1)");
    reg.AddSynonym(unit, "INCH_PER_DEGREE");
    unit = reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/RAD", "IN*RAD(-1)");
    reg.AddSynonym(unit, "INCH_PER_RADIAN");
    }

void AddVelocity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(VELOCITY, SI, "M/S", "M*S(-1)");
    reg.AddSynonym(unit, "METER/SECOND");
    reg.AddSynonym(unit, "METRE_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "M/MIN", "M*MIN(-1)");
    reg.AddSynonym(unit, "METRE_PER_MINUTE");
    unit = reg.AddUnit(VELOCITY, SI, "M/HR", "M*HR(-1)");
    reg.AddSynonym(unit, "METRE_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "M/DAy", "M*DAY(-1)");
    reg.AddSynonym(unit, "METRE_PER_DAY");
    unit = reg.AddUnit(VELOCITY, SI, "MM/S", "MM*S(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "MM/MIN", "MM*MIN(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_MINUTE");
    unit = reg.AddUnit(VELOCITY, SI, "MM/HR", "MM*HR(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "MM/DAY", "MM*DAY(-1)");
    reg.AddSynonym(unit, "MILLIMETRE_PER_DAY");
    unit = reg.AddUnit(VELOCITY, SI, "CM/S", "CM*S(-1)");
    reg.AddSynonym(unit, "CENTIMETER/SECOND");
    reg.AddSynonym(unit, "CENTIMETRE_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "CM/MIN", "CM*MIN(-1)");
    reg.AddSynonym(unit, "CENTIMETER/MINUTE");
    reg.AddSynonym(unit, "CENTIMETRE_PER_MINUTE");
    unit = reg.AddUnit(VELOCITY, SI, "CM/HOUR", "CM*HR(-1)");
    reg.AddSynonym(unit, "CENTIMETER/HOUR");
    reg.AddSynonym(unit, "CENTIMETRE_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "CM/DAY", "CM*DAY(-1)");
    reg.AddSynonym(unit, "CENTIMETER/DAY");
    reg.AddSynonym(unit, "CENTIMETRE_PER_DAY");
    unit = reg.AddUnit(VELOCITY, SI, "KM/S", "KM*S(-1)");
    reg.AddSynonym(unit, "KILOMETRE_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "KM/HR", "KM*HR(-1)");
    reg.AddSynonym(unit, "KILOMETRE_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "IN/SEC", "IN*S(-1)");
    reg.AddSynonym(unit, "INCH_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "IN/MIN", "IN*MIN(-1)");
    reg.AddSynonym(unit, "INCH_PER_MINUTE");
    unit = reg.AddUnit(VELOCITY, SI, "IN/HR", "IN*HR(-1)");
    reg.AddSynonym(unit, "INCH_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "IN/DAY", "IN*DAY(-1)");
    reg.AddSynonym(unit, "INCH_PER_DAY");
    unit = reg.AddUnit(VELOCITY, SI, "FT/SEC", "FT*S(-1)");
    reg.AddSynonym(unit, "FOOT_PER_SECOND");
    unit = reg.AddUnit(VELOCITY, SI, "FT/MIN", "FT*MIN(-1)");
    reg.AddSynonym(unit, "FOOT_PER_MINUTE");
    unit = reg.AddUnit(VELOCITY, SI, "FT/HR", "FT*HR(-1)");
    reg.AddSynonym(unit, "FOOT_PER_HOUR");
    unit = reg.AddUnit(VELOCITY, SI, "FT/DAY", "FT*DAY(-1)");
    reg.AddSynonym(unit, "FOOT_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "YRD/SEC", "YRD*S(-1)"); //, BISQSecUom);
    unit = reg.AddUnit(VELOCITY, SI, "MPH", "MILE*HR(-1)");
    reg.AddSynonym(unit, "MILE_PER_HOUR");

    unit = reg.AddUnit(VELOCITY, MARITIME, "KNOT_UK_ADMIRALTY", "M*HR(-1)", 1853.184);
    // TODO: This is a bad synonym but needed for compatiblity with legacy units ... consider removing all synonyms and only using the name matching for conversion
    reg.AddSynonym("KNOT_UK_ADMIRALTY", "KNOT");


    unit = reg.AddUnit(VELOCITY, MARITIME, "KNOT_INTERNATIONAL", "NAUT_MILE*HR(-1)");
    }

void AddAngularVelocity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/S", "RAD*S(-1)");
    reg.AddSynonym(unit, "RADIAN/SECOND");
    reg.AddSynonym(unit, "RADIAN_PER_SECOND");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/MIN", "RAD*MIN(-1)");
    reg.AddSynonym(unit, "RADIAN/MINUTE");
    reg.AddSynonym(unit, "RADIAN_PER_MINUTE");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/HR", "RAD*HR(-1)");
    reg.AddSynonym(unit, "RADIAN/HOUR");
    reg.AddSynonym(unit, "RADIAN_PER_HOUR");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RPS", "[PI]*RAD*S(-1)", 2.0);
    reg.AddSynonym(unit, "CYCLE_PER_SECOND");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RPM", "[PI]*RAD*MIN(-1)", 2.0);
    reg.AddSynonym(unit, "CYCLE_PER_MINUTE");
    reg.AddSynonym("RPM", "REVOLUTION_PER_MINUTE");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "RPH", "[PI]*RAD*HOUR(-1)", 2.0);
    reg.AddSynonym(unit, "CYCLE_PER_HOUR");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/S", "ARC_DEG*S(-1)", 1.0);
    reg.AddSynonym(unit, "ARC_DEG/S");
    reg.AddSynonym(unit, "DEGREE_PER_SECOND");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/MIN", "ARC_DEG*MIN(-1)", 1.0);
    reg.AddSynonym(unit, "ARC_DEG/MIN");
    reg.AddSynonym(unit, "DEGREE_PER_MINUTE");
    unit = reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/HR", "ARC_DEG*HR(-1)", 1.0);
    reg.AddSynonym(unit, "ARC_DEG/HOUR");
    reg.AddSynonym(unit, "DEGREE_PER_HOUR");
    }

void AddDynamicViscosity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(DYNAMIC_VISCOSITY, SI, "PA-S", "PA*S");
    reg.AddSynonym(unit, "PASCAL_SECOND");
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "POISE", "PA-S", 0.1); //, BISQSecUom);
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "CENTIPOISE", "[CENTI]*POISE"); //, BISQSecUom);
    unit = reg.AddUnit(DYNAMIC_VISCOSITY, SI, "LB/FT*S", "LBM*FT(-1)*S(-1)"); // TODO: Confirm that this is really pound mass
    reg.AddSynonym(unit, "POUND_PER_FOOT_SECOND");
    }

void AddKinematicViscosity(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.M/S", "M(2)*S(-1)");
    reg.AddSynonym(unit, "METRE_SQUARED_PER_SECOND");
    unit = reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.FT/S", "FT(2)*S(-1)");
    reg.AddSynonym(unit, "FOOT_SQUARED_PER_SECOND");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "STOKE", "CM(2)*S(-1)"); //, BISQSecUom);
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "CENTISTOKE", "MM(2)*S(-1)"); //, BISQSecUom);
    }

void AddVolume(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(VOLUME, SI, "CUB.M", "M(3)");
    reg.AddSynonym(unit, "METRE_CUBED");
    reg.AddUnit(VOLUME, SI, "CUB.MU", "MU(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "CUB.MM", "MM(3)"); //, BISQSecUom);
    unit = reg.AddUnit(VOLUME, SI, "CUB.CM", "CM(3)");
    reg.AddSynonym(unit, "CUBIC_CENTIMETRE");
    reg.AddSynonym(unit, "CENTIMETRE_CUBED");
    reg.AddUnit(VOLUME, SI, "CUB.DM", "DM(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "CUB.KM", "KM(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "LITRE", "CUB.DM"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "THOUSAND_LITRE", "[KILO]*LITRE");
    reg.AddUnit(VOLUME, INDUSTRIAL, "THOUSAND_GALLON", "[KILO]*GALLON");
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_GALLON", "[MEGA]*GALLON"); //, BISQSecUom);
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_LITRE", "[MEGA]*LITRE"); //, BISQSecUom);
    unit = reg.AddUnit(VOLUME, SI, "MICROLITRE", "[MICRO]*LITRE");
    reg.AddSynonym(unit, "LAMBDA");
    unit = reg.AddUnit(VOLUME, IMPERIAL, "CUB.IN", "IN(3)");
    reg.AddSynonym(unit, "INCH_CUBED");
    unit = reg.AddUnit(VOLUME, IMPERIAL, "CUB.FT", "FT(3)");
    reg.AddSynonym(unit, "FOOT_CUBED");
    unit = reg.AddUnit(VOLUME, IMPERIAL, "CUB.YRD", "YRD(3)");
    reg.AddSynonym(unit, "YARD_CUBED");
    unit = reg.AddUnit(VOLUME, IMPERIAL, "CUB.MILE", "MILE(3)");
    reg.AddSynonym(unit, "MILE_CUBED");
    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_INCH", "ACRE*IN"); //, BISQSecUom);
    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_FOOT", "ACRE*FT"); //, BISQSecUom);
    reg.AddUnit(VOLUME, USCUSTOM, "GALLON", "IN(3)", 231.0); //, BISQSecUom);
    reg.AddUnit(VOLUME, IMPERIAL, "GALLON_IMPERIAL", "LITRE", 4.54609); //, BISQSecUom);
    }

void AddSpecificVolume(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.M/KG", "M(3)*KG(-1)");
    reg.AddSynonym(unit, "METRE_CUBED_PER_KILOGRAM");
    unit = reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.FT/LB", "FT(3)*LBM(-1)");
    reg.AddSynonym(unit, "FOOT_CUBED_PER_POUND_MASS");
    }

void AddWarpingConstant(UnitRegistry& reg)
    {
    UnitCP unit = reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "M^6", "M(6)");
    reg.AddSynonym(unit, "METRE_TO_THE_SIXTH");
    unit = reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "MM^6", "MM(6)");
    reg.AddSynonym(unit, "MILLIMETRE_TO_THE_SIXTH");
    unit = reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "CM^6", "CM(6)");
    reg.AddSynonym(unit, "CENTIMETRE_TO_THE_SIXTH");
    unit = reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "IN^6", "IN(6)");
    reg.AddSynonym(unit, "INCH_TO_THE_SIXTH");
    unit = reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "FT^6", "FT(6)");
    reg.AddSynonym(unit, "FOOT_TO_THE_SIXTH");
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultUnits ()
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    UnitCP unit = reg.AddDimensionBaseUnit("M", BasePhenomena::Length);
    reg.AddSynonym(unit, "METRE");
    unit = reg.AddDimensionBaseUnit("KG", BasePhenomena::Mass);
    reg.AddSynonym(unit, "KILOGRAM");
    unit = reg.AddDimensionBaseUnit("S", BasePhenomena::Time);
    reg.AddSynonym(unit, "SECOND");
    unit = reg.AddDimensionBaseUnit("K", BasePhenomena::Temperature);
    reg.AddSynonym(unit, "KELVIN");
    reg.AddSynonym(unit, "DEGREE_KELVIN");

    reg.AddDimensionBaseUnit("DELTA_KELVIN", BasePhenomena::TemperatureChange);
    reg.AddSynonym("DELTA_KELVIN", "DELTA_DEGREE_KELVIN");

    unit = reg.AddDimensionBaseUnit("A", BasePhenomena::ElectricCurrent); //, BISQPrimUom);
    reg.AddSynonym(unit, "AMPERE");
    unit = reg.AddDimensionBaseUnit("MOL", BasePhenomena::Mole); // Where mol is the SI gram mol or gmol.
    reg.AddSynonym(unit, "MOLE");
    unit = reg.AddDimensionBaseUnit("CD", BasePhenomena::Luminosity);
    reg.AddSynonym(unit, "CANDELA");
    unit = reg.AddDimensionBaseUnit("RAD", BasePhenomena::PlaneAngle);
    reg.AddSynonym(unit, "RADIAN");
    unit = reg.AddDimensionBaseUnit("STERAD", BasePhenomena::SolidAngle);
    reg.AddSynonym(unit, "STERADIAN");
    unit = reg.AddDimensionBaseUnit("US$", BasePhenomena::Finance);
    reg.AddSynonym("US$", "DOLLAR");
    reg.AddDimensionBaseUnit("PERSON", BasePhenomena::Capita); //, BISQPrimUom);
    reg.AddDimensionBaseUnit("ONE", BasePhenomena::Ratio); // TODO: I don't like that Ratio has base unit of ONE and all unitless unit will have a phenomenon of Ratio ...
    reg.AddSynonym("ONE", "NONE");
    reg.AddSynonym("ONE", "UNITLESS_UNIT");



    AddLengths(reg);
    AddLinearCost(reg);
    AddLinearRate(reg);
    AddMass(reg);
    AddTime(reg);
    AddTemperature(reg);
    AddTemperatureChange(reg);
    AddLuminousFlux(reg);
    AddIlluminance(reg);
    AddLuminosity(reg);
    AddMole(reg);
    AddCapita(reg);
    AddFinance(reg);
    AddRatio(reg);
    AddRotationalSpringConstant(reg);
    AddLinearRotationalSpringConstant(reg);
    AddAcceleration(reg);
    AddPlaneAngle(reg);
    AddSolidAngle(reg);
    AddArea(reg);
    AddSizeLengthRate(reg);
    AddDensity(reg);
    AddForceDensity(reg);
    AddPopulationDensity(reg);
    AddElectricCurrent(reg);
    AddElectricCharge(reg);
    AddElectricPotential(reg);
    AddEnergy(reg);
    AddEnergyDensity(reg);
    AddHeatingValue(reg);
    AddSpecificHeatCapacity(reg);
    AddSpecificHeatCapacityMolar(reg);
    AddVolumeFlowRate(reg);
    AddFrequency(reg);
    AddSurfaceFlowRate(reg);
    AddMassFlowRate(reg);
    AddParticleFlowRate(reg);
    AddForce(reg);
    AddHeatFlux(reg);
    AddHeatTransfer(reg);
    AddLinearDensity(reg);
    AddLinearLoad(reg);
    AddTorque(reg);
    AddMolarVolume(reg);
    AddMolarConcentration(reg);
    AddMomentOfInertia(reg);
    AddPower(reg);
    //AddPowerDensity(reg);
    AddPressure(reg);
    AddPressureGradient(reg);
    AddPercentage(reg);
    AddSlope(reg);
    AddSurfaceDensity(reg);
    AddThermalConductivity(reg);
    AddThermalResistance(reg);
    AddTemperatureGradient(reg);
    AddLinearThermalExpansionCoefficient(reg);
    AddThreadPitch(reg);
    AddVelocity(reg);
    AddAngularVelocity(reg);
    AddDynamicViscosity(reg);
    AddKinematicViscosity(reg);
    AddVolume(reg);
    AddSpecificVolume(reg);
    AddWarpingConstant(reg);
    }
