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
    reg.AddUnit(LENGTH, SI, "MM", "[MILLI]*M");
    reg.AddUnit(LENGTH, SI, "CM", "[CENTI]*M");
    reg.AddUnit(LENGTH, SI, "DM", "[DECI]*M");
    reg.AddUnit(LENGTH, SI, "KM", "[KILO]*M");
    reg.AddUnit(LENGTH, SI, "MU", "[MICRO]*M");
    reg.AddSynonym("MU", "MICRON");

    reg.AddUnit(LENGTH, USCUSTOM, "MILLIINCH", "[MILLI]*IN");
    reg.AddUnit(LENGTH, USCUSTOM, "MICROINCH", "[MICRO]*IN");
    reg.AddUnit(LENGTH, IMPERIAL, "MILLIFOOT", "[MILLI]*FT");

    reg.AddUnit(LENGTH, IMPERIAL, "IN", "MM", 25.4); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(LENGTH, IMPERIAL, "FT", "IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, IMPERIAL, "YRD", "FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, SURVEYOR, "CHAIN", "FT", 66.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    reg.AddUnit(LENGTH, IMPERIAL, "MILE", "YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_INCH", "CM", 10000.0 / 3937.0);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_FOOT", "US_SURVEY_INCH", 12.0);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_YARD", "US_SURVEY_FOOT", 3.0);
    reg.AddUnit(LENGTH, SURVEYOR, "US_SURVEY_CHAIN", "US_SURVEY_FOOT", 66.0);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_MILE", "US_SURVEY_YARD", 1760.0);

    reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE", "M", 1852.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddSynonym("NAUT_MILE", "NAUTICAL_MILE_INTERNATIONAL");
    
    reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE_IMPERIAL", "M", 1853.0);
    reg.AddSynonym("NAUT_MILE_IMPERIAL", "ADMIRALTY_MILE");

    reg.AddUnit(LENGTH, SI, "ANGSTROM", "M", 1e-10);
    reg.AddUnit(LENGTH, SI, "FERMI", "[FEMTO]*M");
    reg.AddSynonym("FERMI", "FEMTOMETRE");
    reg.AddUnit(LENGTH, IMPERIAL, "BARLEYCORN", "IN", (1.0 / 3.0));
    reg.AddUnit(LENGTH, HISTORICAL, "CUBIT", "IN", 18.0);
    reg.AddUnit(LENGTH, HISTORICAL, "ELL", "IN", 45.0);
    reg.AddUnit(LENGTH, HISTORICAL, "FATHOM", "FT", 6.0);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_SEC", "[C]*S");
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_MIN", "[C]*MIN");
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_HOUR", "[C]*HR");
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_YEAR", "[C]*YR");
    reg.AddUnit(LENGTH, ASTRONOMY, "AU", "M", 1.495978707e11);
    }

void AddMass(UnitRegistry& reg)
    {
    reg.AddUnit(MASS, SI, "G", "[MILLI]*KG");
    reg.AddUnit(MASS, SI, "MG", "[MILLI]*G");
    reg.AddUnit(MASS, SI, "MKG", "[MICRO]*G");
    reg.AddUnit(MASS, SI, "NG", "[NANO]*G");
    reg.AddSynonym("NG", "NANOGRAM");
    reg.AddUnit(MASS, SI, "MEGAGRAM", "[MEGA]*G");
    reg.AddUnit(MASS, SI, "TONNE", "[KILO]*KG"); // Also known as a metric ton http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.

    reg.AddUnit(MASS, IMPERIAL, "LBM", "KG", 0.45359237); // Is Avoirdupois Pound.  Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Footnote 22
    reg.AddSynonym("LBM", "POUND_MASS");
    reg.AddUnit(MASS, IMPERIAL, "SLUG", "LBF*S(2)*FT(-1)");
    reg.AddSynonym("SLUG", "GEEPOUND");
    reg.AddUnit(MASS, USCUSTOM, "GRAIN", "LBM", 1.0 / 7000.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix B. Section 3.2, Page B-10

    reg.AddUnit(MASS, USCUSTOM, "SHORT_TON", "LBM", 2000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "LONG_TON", "LBM", 2240); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "KIP", "LBM", 1000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    }

void AddTime(UnitRegistry& reg)
    {
    reg.AddUnit(TIME, SI, "MIN", "S", 60.0);
    reg.AddUnit(TIME, SI, "HR", "MIN", 60.0);
    reg.AddUnit(TIME, SI, "DAY", "HR", 24.0);
    reg.AddUnit(TIME, SI, "WEEK", "DAY", 7.0);
    reg.AddUnit(TIME, SI, "MONTH", "DAY", 30.0);
    reg.AddUnit(TIME, SI, "YR", "DAY", 365);  //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Year is 3.1536 E+07 seconds which is equal to 365 * 24 * 60 * 60
    reg.AddUnit(TIME, SI, "YEAR_SIDEREAL", "S", 3.155815e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, SI, "YEAR_TROPICAL", "S", 3.155693e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, SI, "MS", "[MILLI]*S");
    reg.AddUnit(TIME, SI, "MKS", "[MICRO]*S");
    reg.AddSynonym("MKS", "MICROSECOND");

    }

//TODO: Handle temperature and delta temperature
void AddTemperature(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE, SI, "CELSIUS", "K", 1.0, 273.15);
    reg.AddUnit(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "CELSIUS", 5.0 / 9.0, -32);
    reg.AddUnit(TEMPERATURE, USCUSTOM, "RANKINE", "K", 5.0 / 9.0);

    reg.AddUnit(TEMPERATURE, USCUSTOM, "ROMER", "CELSIUS", 40.0 / 21.0, -7.5);
    reg.AddSynonym("ROMER", "DEGREE_ROMER");
    }

void AddTemperatureChange(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_CHANGE, SI, "DELTA_CELSIUS", "DELTA_KELVIN", 1.0);

    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_FAHRENHEIT", "DELTA_CELSIUS", 5.0 / 9.0);

    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_RANKINE", "DELTA_KELVIN", 5.0 / 9.0);
    }

void AddTemperatureGradient(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_GRADIENT, SI, "KELVIN/M", "DELTA_KELVIN*M(-1)");
    }

void AddLinearThermalExpansionCoefficient(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/K", "DELTA_KELVIN(-1)");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/CELSIUS", "DELTA_CELSIUS(-1)");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, USCUSTOM, "STRAIN/FAHRENHEIT", "DELTA_FAHRENHEIT(-1)");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, USCUSTOM, "STRAIN/RANKINE", "DELTA_RANKINE(-1)");
    }

void AddLuminousFlux(UnitRegistry& reg)
    {
    reg.AddUnit(LUMINOUS_FLUX, SI, "LUMEN", "CD*STERAD");
    }

void AddIlluminance(UnitRegistry& reg)
    {
    reg.AddUnit(ILLUMINANCE, SI, "LUX", "LUMEN*M(-2)");
    reg.AddUnit(ILLUMINANCE, USCUSTOM, "LUMEN/SQ.FT", "LUMEN*FT(-2)");
    }

void AddLuminosity(UnitRegistry& reg)
    {}

void AddMole(UnitRegistry& reg)
    {
    reg.AddUnit(MOLE, SI, "KMOL", "[KILO]*MOL");
    reg.AddSynonym("KMOL", "KILOMOLE");
    reg.AddUnit(MOLE, SI, "LB-MOLE", "MOL", 453.59237); // ASTM SI 10 standard SI1-.phhc8328.pdf page 29, 35 and http://en.wikipedia.org/wiki/Mole_%28unit%29
    reg.AddSynonym("LB-MOLE", "POUND_MOLE");
    }

void AddCapita(UnitRegistry& reg)
    {
    reg.AddUnit(CAPITA, STATISTICS, "CAPITA", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "CUSTOMER", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "PASSENGER", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "EMPLOYEE", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "STUDENT", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "GUEST", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "RESIDENT", "PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "HUNDRED_CAPITA", "[HECTO]*PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "THOUSAND_CAPITA", "[KILO]*PERSON");
    }

void AddFinance(UnitRegistry& reg)
    {}

void AddRatio(UnitRegistry& reg)
    {}

void AddRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, SI, "(N*M)/RAD", "N*M*RAD(-1)");

    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, SI, "(N*M)/DEG", "N*M*ARC_DEG(-1)");
    }

void AddLinearRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_ROTATIONAL_SPRING_CONSTANT, SI, "N/RAD", "N*RAD(-1)");
    }

void AddAcceleration(UnitRegistry& reg)
    {
    reg.AddUnit(ACCELERATION, SI, "M/SEC.SQUARED", "M*S(-2)");
    reg.AddUnit(ACCELERATION, SI, "CM/SEC.SQUARED", "CM*S(-2)");
    reg.AddUnit(ACCELERATION, IMPERIAL, "FT/SEC.SQUARED", "FT*S(-2)");
    }

void AddPlaneAngle(UnitRegistry& reg)
    {
    reg.AddUnit(ANGLE, SI, "ARC_DEG", "[PI]*RAD", 1.0 / 180.0);
    reg.AddUnit(ANGLE, SI, "ARC_MINUTE", "ARC_DEG", 1.0 / 60.0);
    reg.AddUnit(ANGLE, SI, "ARC_SECOND", "ARC_DEG", 1.0 / 3600.0);
    reg.AddUnit(ANGLE, SI, "ARC_QUADRANT", "[PI]*RAD", 0.5);
    reg.AddUnit(ANGLE, SI, "GRAD", "[PI]*RAD", 1.0 / 200.0);
    reg.AddUnit(ANGLE, SI, "REVOLUTION", "[PI]*RAD", 2.0);
    }

void AddSolidAngle(UnitRegistry& reg)
    {}

void AddArea(UnitRegistry& reg)
    {
    reg.AddUnit(AREA, SI, "SQ.M", "M(2)");
    reg.AddUnit(AREA, SI, "SQ.MU", "MU(2)");
    reg.AddSynonym("SQ.MU", "MICRON_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.MM", "MM(2)");
    reg.AddUnit(AREA, SI, "SQ.CM", "CM(2)");
    reg.AddUnit(AREA, SI, "SQ.DM", "DM(2)");
    reg.AddUnit(AREA, SI, "SQ.KM", "KM(2)");
    reg.AddUnit(AREA, SI, "ARE", "[HECTO]*M(2)");
    reg.AddUnit(AREA, SI, "HECTARE", "[HECTO]*ARE");

    reg.AddUnit(AREA, IMPERIAL, "SQ.IN", "IN(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.FT", "FT(2)");
    reg.AddUnit(AREA, IMPERIAL, "THOUSAND_SQ.FT", "[KILO]*FT(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.YRD", "YRD(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.MILE", "MILE(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.CHAIN", "CHAIN(2)");
    reg.AddUnit(AREA, IMPERIAL, "ACRE", "CHAIN(2)", 10.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9

    reg.AddUnit(AREA, IMPERIAL, "SQ.US_SURVEY_IN", "US_SURVEY_INCH(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.US_SURVEY_FT", "US_SURVEY_FOOT(2)");
    reg.AddUnit(AREA, IMPERIAL, "SQ.US_SURVEY_YRD", "US_SURVEY_YARD(2)");
    reg.AddUnit(AREA, SURVEYOR, "SQ.US_SURVEY_MILE", "US_SURVEY_MILE(2)");
    reg.AddUnit(AREA, SURVEYOR, "SQ.US_SURVEY_CHAIN", "US_SURVEY_CHAIN(2)");
    reg.AddUnit(AREA, SURVEYOR, "US_SURVEY_ACRE", "US_SURVEY_CHAIN(2)", 10.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9
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
    reg.AddUnit(DENSITY, SI, "KG/CUB.M", "KG*M(-3)");
    reg.AddUnit(DENSITY, SI, "KG/CUB.CM", "KG*CM(-3)");
    reg.AddUnit(DENSITY, SI, "KG/LITRE", "KG*DM(-3)");

    reg.AddUnit(DENSITY, SI, "G/CUB.CM", "G*CM(-3)");

    reg.AddUnit(DENSITY, SI, "MKG/LITRE", "MKG*DM(-3)");

    reg.AddUnit(DENSITY, SI, "MG/LITRE", "MG*DM(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "LBM/CUB.FT", "LBM*FT(-3)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/GAL", "LBM*GALLON(-1)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/GALLON_IMPERIAL", "LBM*GALLON_IMPERIAL(-1)");
    reg.AddUnit(DENSITY, SI, "LBM/CUB.IN", "LBM*IN(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "LBM/MILLION_GALLON", "LBM*GALLON(-1)", 1.0e-6);

    reg.AddUnit(DENSITY, USCUSTOM, "SLUG/CUB.FT", "SLUG*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "KIP/CUB.FT", "KIP*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "SHORT_TON/CUB.FT", "SHORT_TON* FT(-3)");
    }

void AddForceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE_DENSITY, SI, "N/CUB.M", "N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, SI, "KN/CUB.M", "[KILO]*N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "N/CUB.FT", "N*FT(-3)");
    reg.AddSynonym("N/CUB.FT", "NEWTON_PER_FOOT_CUBED");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "KN/CUB.FT", "[KILO]*N*FT(-3)");
    }

void AddPopulationDensity(UnitRegistry& reg)
    {
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.M", "PERSON*M(-2)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/HECTARE", "PERSON*HECTARE(-1)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.KM", "PERSON*KM(-2)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/ACRE", "PERSON*ACRE(-1)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.FT", "PERSON*FT(-2)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.MILE", "PERSON*MILE(-2)");
    }

void AddElectricCurrent(UnitRegistry& reg)
    {
    reg.AddUnit(CURRENT, SI, "KILOAMPERE", "[KILO]*A");
    reg.AddUnit(CURRENT, SI, "MILLIAMPERE", "[MILLI]*A");
    reg.AddUnit(CURRENT, SI, "MICROAMPERE", "[MICRO]*A");
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
    reg.AddUnit(WORK, SI, "J", "N*M");
    reg.AddUnit(WORK, SI, "KJ", "[KILO]*N*M");
    reg.AddUnit(WORK, SI, "MJ", "[MEGA]*N*M");
    reg.AddUnit(WORK, SI, "GJ", "[GIGA]*N*M");
    reg.AddUnit(WORK, USCUSTOM, "FOOT_POUNDAL", "PDL*FT");
    reg.AddUnit(WORK, INTERNATIONAL, "BTU", "J", 1.05505585262e3); // Is IT BTU.  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.  See foot note #9: 
    reg.AddSynonym("BTU", "BRITISH_THERMAL_UNIT");
    reg.AddUnit(WORK, USCUSTOM, "KILOBTU", "[KILO]*BTU");

    reg.AddUnit(WORK, INTERNATIONAL, "CAL", "J", 4.1868);
    reg.AddSynonym("CAL", "CALORIE");
    reg.AddUnit(WORK, INTERNATIONAL, "KCAL", "[KILO]*CAL");
    reg.AddSynonym("KCAL", "KILOCALORIE");
    reg.AddUnit(WORK, INTERNATIONAL, "CUB.FT_ATM", "ATM*CUB.FT");
    reg.AddSynonym("CUB.FT_ATM", "CUBIC_FOOT_OF_ATMOSPHERE");
    reg.AddUnit(WORK, INTERNATIONAL, "CUB.YRD_ATM", "ATM*CUB.YRD");
    reg.AddSynonym("CUB.YRD_ATM", "CUBIC_YARD_OF_ATMOSPHERE");
    reg.AddUnit(WORK, USCUSTOM, "WATT_SECOND", "W*S");
    reg.AddUnit(WORK, INTERNATIONAL, "KWH", "KW*HR");
    reg.AddUnit(WORK, INTERNATIONAL, "MWH", "MW*HR");
    reg.AddUnit(WORK, INTERNATIONAL, "GWH", "GW*HR");
    }


// TODO: Check these phenomena, Energy Density?
void AddEnergyDensity(UnitRegistry& reg)
    {
    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "J/CUB.M", "J*M(-3)");

    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "KJ/CUB.M", "KJ*M(-3)");

    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, SI, "KWH/CUB.M", "KWH*M(-3)");

    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, USCUSTOM, "KWH/CUB.FT", "KWH*FT(-3)");

    reg.AddUnit(HEATING_VALUE_VOLUMETRIC, USCUSTOM, "KWH/MILLION_GALLON", "KWH*GALLON(-1)", 1.0e-6);
    }

void AddHeatingValue(UnitRegistry& reg)
    {
    reg.AddUnit(HEATING_VALUE_MASS, SI, "J/KG", "J*KG(-1)");
    reg.AddUnit(HEATING_VALUE_MASS, SI, "KJ/KG", "KJ*KG(-1)");
    reg.AddUnit(HEATING_VALUE_MASS, SI, "MEGAJ/KG", "MJ*KG(-1)");

    reg.AddUnit(HEATING_VALUE_MASS, USCUSTOM, "BTU/LBM", "BTU*LBM(-1)");
    }

void AddSpecificHeatCapacity(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, SI, "J/(KG*K)", "J*KG(-1)*DELTA_KELVIN(-1)");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, USCUSTOM, "BTU/(LBM*RANKINE)", "BTU*LBM(-1)*DELTA_RANKINE(-1)");
    }

void AddSpecificHeatCapacityMolar(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "J/(KMOL*K)", "J*KMOL(-1)*DELTA_KELVIN(-1)");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "KJ/(KMOL*K)", "KJ*KMOL(-1)*DELTA_KELVIN(-1)");

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "BTU/(LB-MOLE*RANKINE)", "BTU*LB-MOLE(-1)*DELTA_RANKINE(-1)");
    }

void AddVolumeFlowRateByArea(UnitRegistry& reg)
    {

    }

void AddVolumeFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(FLOW, SI, "CUB.M/SEC", "M(3)*S(-1)");
    reg.AddUnit(FLOW, SI, "CUB.M/MIN", "M(3)*MIN(-1)");
    reg.AddUnit(FLOW, SI, "CUB.M/HR", "M(3)*HR(-1)");
    reg.AddUnit(FLOW, SI, "CUB.M/DAY", "M(3)*DAY(-1)");
    reg.AddUnit(FLOW, SI, "LITRE/SEC", "LITRE*S(-1)");
    reg.AddUnit(FLOW, SI, "LITRE/MIN", "LITRE*MIN(-1)");
    reg.AddUnit(FLOW, SI, "LITRE/HR", "LITRE*HR(-1)");
    reg.AddUnit(FLOW, SI, "LITRE/DAY", "LITRE*DAY(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/SEC", "CUB.IN*S(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/MIN", "CUB.IN*MIN(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/SEC", "CUB.FT*S(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/MIN", "CUB.FT*MIN(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/DAY", "CUB.FT*DAY(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/DAY", "ACRE_FOOT*DAY(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/HR", "ACRE_FOOT*HR(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/MIN", "ACRE_FOOT*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/DAY", "ACRE_INCH*DAY(-1)");
    reg.AddSynonym("ACRE_INCH/DAY", "ACRE_INCH_PER_DAY");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/HOUR", "ACRE_INCH*HR(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/MIN", "ACRE_INCH*MIN(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL*DAY(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/MINUTE", "GALLON_IMPERIAL*MIN(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/SECOND", "GALLON_IMPERIAL*S(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON/S", "GALLON*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/MIN", "GALLON*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/DAY", "GALLON*DAY(-1)");
    }

void AddFrequency(UnitRegistry& reg)
    {
    reg.AddUnit(FREQUENCY, SI, "HERTZ", "S(-1)");
    reg.AddUnit(FREQUENCY, SI, "KH", "[KILO]*S(-1)");
    reg.AddSynonym("KH", "KILOHERTZ");
    reg.AddUnit(FREQUENCY, SI, "MH", "[MEGA]*S(-1)");
    reg.AddSynonym("MH", "MEGAHERTZ");
    }

void AddSurfaceFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(SECOND*SQ.M)", "M*S(-1)");
    reg.AddSynonym("CUB.M/(SECOND*SQ.M)", "METRE_CUBED_PER_METRE_SQUARED_PER_SECOND");
    reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(DAY*SQ.M)", "M*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*HECTARE)", "CUB.M*HECTARE(-1)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*SQ.KM)", "CUB.M*KM(-2)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*MIN)", "FT*MIN(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*S)", "FT*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.MILE*S)", "FT(3)*MILE(-2)*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*ACRE)", "GALLON*DAY(-1)*ACRE(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*ACRE)", "GALLON*MIN(-1)*ACRE(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.FT)", "GALLON*MIN(-1)*FT(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.FT)", "GALLON*DAY(-1)*FT(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.MILE)", "GALLON*MIN(-1)*MILE(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.MILE)", "GALLON*DAY(-1)*MILE(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(ACRE*SEC)", "FT(3)*ACRE(-1)*S(-1)");
    }

void AddMassRatio(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_RATIO, SI, "KILOGRAM_PER_KILOGRAM", "KG*KG(-1)");
    reg.AddUnit(MASS_RATIO, USCUSTOM, "GRAIN_MASS_PER_POUND_MASS", "GRAIN*LBM(-1)");
    }

void AddMassFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_FLOW, SI, "KG/S", "KG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/MIN", "KG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/HR", "KG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/DAY", "KG*DAY(-1)");

    reg.AddUnit(MASS_FLOW, SI, "G/S", "G*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "G/MIN", "G*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "G/HR", "G*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/S", "MG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/MIN", "MG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/HR", "MG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/DAY", "MG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/S", "MKG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/MIN", "MKG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/HR", "MKG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/DAY", "MKG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, SI, "LB/S", "LBM*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "LB/MIN", "LBM*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "LB/HR", "LBM*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "LB/DAY", "LBM*DAY(-1)");
    reg.AddUnit(MASS_FLOW, SI, "TONNE/HR", "TONNE*HR(-1)");

    reg.AddUnit(MASS_FLOW, USCUSTOM, "SHORT_TON/HR", "SHORT_TON*HR(-1)");
    }

void AddParticleFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(PARTICLE_FLOW, SI, "MOL/S", "MOL*S(-1)");
    reg.AddUnit(PARTICLE_FLOW, SI, "KMOL/S", "[KILO]*MOL*S(-1)");
    }

void AddForce(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE, SI, "N", "KG*M*S(-2)");
    reg.AddUnit(FORCE, SI, "KN", "[KILO]*N");
    reg.AddUnit(FORCE, SI, "mN", "[MILLI]*N");
    reg.AddUnit(FORCE, SI, "KGF", "[STD_G]*KG");
    reg.AddSynonym("KGF", "KILOPOND");
    reg.AddUnit(FORCE, SI, "LBF", "[STD_G]*LBM");
    reg.AddUnit(FORCE, SI, "KPF", "[KILO]*LBF");
    reg.AddUnit(FORCE, SI, "DYNE", "G*CM*S(-2)");
    reg.AddUnit(FORCE, SI, "PDL", "LBM*FT*S(-2)");
    reg.AddSynonym("PDL", "POUNDAL");

    reg.AddUnit(FORCE, USCUSTOM, "SHORT_TON_FORCE", "[STD_G]*SHORT_TON");
    reg.AddUnit(FORCE, USCUSTOM, "LONG_TON_FORCE", "[STD_G]*LONG_TON");
    }

void AddHeatFlux(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_FLUX_DENSITY, SI, "W/SQ.M", "W*M(-2)");
    }

// TODO: Thermal Transmittance?
void AddHeatTransfer(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_TRANSFER, SI, "W/(SQ.M*K)", "W*M(-2)*DELTA_KELVIN(-1)");

    reg.AddUnit(HEAT_TRANSFER, SI, "W/(SQ.M*CELSIUS)", "W*M(-2)*DELTA_CELSIUS(-1)");

    reg.AddUnit(HEAT_TRANSFER, USCUSTOM, "BTU/(SQ.FT*HR*FAHRENHEIT)", "BTU*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    }

void AddLinearDensity(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_DENSITY, SI, "KG/M", "KG*M(-1)");
    reg.AddUnit(LINEAR_DENSITY, SI, "KG/MM", "KG*MM(-1)");
    reg.AddUnit(LINEAR_DENSITY, USCUSTOM, "LBM/FT", "LBM*FT(-1)");
    }

void AddLinearLoad(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_LOAD, SI, "N/M", "N*M(-1)");
    reg.AddUnit(LINEAR_LOAD, SI, "N/MM", "N*MM(-1)");
    reg.AddUnit(LINEAR_LOAD, SI, "LBF/IN", "LBF*IN(-1)");
    }

void AddLinearCost(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_COST, FINANCE, "$/M", "US$*M(-1)");
    reg.AddUnit(LINEAR_COST, FINANCE, "$/MM", "US$*MM(-1)");
    }

void AddLinearRate(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_RATE, SI, "PER_M", "M(-1)");

    reg.AddUnit(LINEAR_RATE, SI, "PER_MM", "MM(-1)");

    reg.AddUnit(LINEAR_RATE, SI, "PER_KM", "KM(-1)");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_FT", "FT(-1)");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_MILE", "MILE(-1)");

    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_THOUSAND_FT", "FT(-1)", 1.0e-3);
    }

void AddTorque(UnitRegistry& reg)
    {
    reg.AddUnit(TORQUE, SI, "N_M", "N*M*RAD");
    reg.AddUnit(TORQUE, SI, "N_CM", "N*CM*RAD");
    reg.AddSynonym("N_CM", "NEWTON_CENTIMETRE");
    reg.AddUnit(TORQUE, USCUSTOM, "LBF_FT", "LBF*FT*RAD");
    }

void AddMolarVolume(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/MOLE", "CUB.M*MOL(-1)");
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/KMOL", "CUB.M*MOL(-1)", 1.0e-3);
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.FT/LB-MOLE", "CUB.FT*LB-MOLE(-1)");
    }

void AddMolarConcentration(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.M", "MOL*CUB.M(-1)");
    reg.AddSynonym("MOL/CUB.M", "MILLIMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, SI, "KMOL/CUB.M", "[KILO]*MOL*CUB.M(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.DM", "MOL*CUB.DM(-1)");
    reg.AddSynonym("MOL/CUB.DM", "MOLE_PER_LITRE");
    reg.AddSynonym("MOL/CUB.DM", "MOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MICROMOL/CUB.DM", "[MICRO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("MICROMOL/CUB.DM", "MICROMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "NMOL/CUB.DM", "[NANO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("NMOL/CUB.DM", "NANOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "PICOMOL/CUB.DM", "[PICO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("PICOMOL/CUB.DM", "PICOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.FT", "MOL*CUB.FT(-1)");
    reg.AddSynonym("MOL/CUB.FT", "MOLE_PER_FOOT_CUBED");
    }

// NOTE: Changed to Area moment of inertia based on Wiki pages on moment of inertia and area moment of inertia
void AddMomentOfInertia(UnitRegistry& reg)
    {
    reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "M^4", "M(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "CM^4", "CM(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "IN^4", "IN(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, INDUSTRIAL, "FT^4", "FT(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, SI, "MM^4", "MM(4)");
    }

void AddPower(UnitRegistry& reg)
    {
    reg.AddUnit(POWER, INTERNATIONAL, "W", "N*M*S(-1)");
    reg.AddUnit(POWER, INTERNATIONAL, "KW", "[KILO]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "MW", "[MEGA]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "GW", "[GIGA]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "BTU/MONTH", "BTU*MONTH(-1)");
    reg.AddUnit(POWER, INTERNATIONAL, "BTU/HOUR", "BTU*HR(-1)");
    reg.AddUnit(POWER, USCUSTOM, "KILOBTU/HOUR", "[KILO]*BTU*HR(-1)");
    reg.AddUnit(POWER, USCUSTOM, "HP", "LBF*FT*S(-1)", 550.0);

    reg.AddUnit(POWER, SI, "GJ/MONTH", "GJ*MONTH(-1)");
    }

// TODO: Wrong
//void AddPowerDensity(UnitRegistry& reg)
//    {
//    reg.AddUnit(BISPH_LENGTH, USCUSTOM, "BTU_PER_HOUR_PER_FOOT_CUBED", "M_PER_L_T3", 0.09662109117546, 0.0); //, BISQNoDescript);
//    }

void AddPressure(UnitRegistry& reg)
    {
    reg.AddUnit(PRESSURE, SI, "PA", "N*M(-2)");

    reg.AddUnit(PRESSURE, SI, "PA_GAUGE", "PA", 1, -101325); // TODO: Use constant for this

    // TODO: See if this is equal to another unit here.
    reg.AddUnit(PRESSURE, SI, "N/SQ.MM", "N*MM(-2)");

    reg.AddUnit(PRESSURE, SI, "HECTOPASCAL", "[HECTO]*PA");

    reg.AddUnit(PRESSURE, SI, "KILOPASCAL", "[KILO]*PA");
    reg.AddUnit(PRESSURE, SI, "KILOPASCAL_GAUGE", "[KILO]*PA", 1, -101325e-3); // TODO: Use constant for this

    reg.AddUnit(PRESSURE, SI, "MEGAPASCAL", "[MEGA]*PA");
    reg.AddUnit(PRESSURE, SI, "MEGAPASCAL_GAUGE", "[MEGA]*PA", 1, -101325e-6); // TODO: Use constant for this

    reg.AddUnit(PRESSURE, INTERNATIONAL, "AT", "KGF*CM(-2)");  // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddSynonym("AT", "ATMOSPHERE_TECHNIAL");

    reg.AddUnit(PRESSURE, INDUSTRIAL, "AT_GAUGE", "AT", 1.0, -1.0332274527998859); // TODO: double check, used 101325 PA -> AT conversion

    reg.AddUnit(PRESSURE, INTERNATIONAL, "KGF/SQ.M", "KGF*M(-2)");

    reg.AddUnit(PRESSURE, SI, "ATM", "PA", 101325);  // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  Is standard atmosphere

    reg.AddUnit(PRESSURE, SI, "BAR", "PA", 1.0e5); //, BISQNoDescript);
    reg.AddUnit(PRESSURE, INDUSTRIAL, "BAR_GAUGE", "PA", 1.0e5, -1.01325); //, BISQNoDescript);

    reg.AddUnit(PRESSURE, SI, "MBAR", "[MILLI]*BAR");

    reg.AddUnit(PRESSURE, CGS, "BARYE", "PA", 0.1);   // 1.0 dyn/sq.cm


    reg.AddUnit(PRESSURE, USCUSTOM, "PSI", "LBF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "PSIG", "LBF*IN(-2)", 1, -14.695948775513449); // TODO: double check, used 101325 PA -> PSI conversion

    reg.AddUnit(PRESSURE, USCUSTOM, "KSI", "[KILO]*LBF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "LBF/SQ.FT", "LBF*FT(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "TORR", "PA", 1.333224e2);   // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.

    // TODO: Go back to density conversion once we have verified sources for those values
    reg.AddUnit(PRESSURE, SI, "METRE_OF_H2O_CONVENTIONAL", "[KILO]*MILLIMETRE_OF_H2O_CONVENTIONAL");

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
    reg.AddUnit(PRESSURE_GRADIENT, SI, "PA/M", "PA*M(-1)");
    reg.AddUnit(PRESSURE_GRADIENT, SI, "BAR/KM", "BAR*KM(-1)");
    }

void AddPercentage(UnitRegistry& reg)
    {
    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "PERCENT", "ONE");

    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "DECIMAL_PERCENT", "PERCENT", 100);
    }

void AddSlope(UnitRegistry& reg)
    {
    reg.AddUnit(SLOPE, SI, "M/M", "M*M(-1)");

    reg.AddUnit(SLOPE, SI, "CM/M", "CM*M(-1)");
    reg.AddUnit(SLOPE, SI, "MM/M", "MM*M(-1)");
    reg.AddUnit(SLOPE, SI, "M/KM", "M*KM(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FOOT_PER_1000_FOOT", "FT*FT(-1)", 1.0e-3);
    reg.AddUnit(SLOPE, USCUSTOM, "FT/FT", "FT*FT(-1)");


    reg.AddUnit(SLOPE, USCUSTOM, "IN/FT", "IN*FT(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/IN", "FT*IN(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/MILE", "FT*MILE(-1)");

    reg.AddUnit(SLOPE, INTERNATIONAL, "VERTICAL_PER_HORIZONTAL", "M/M");

    reg.AddUnit(SLOPE, INTERNATIONAL, "PERCENT_SLOPE", "M/M", 1.0e-2);

    reg.AddInvertingUnit("VERTICAL_PER_HORIZONTAL", "HORIZONTAL_PER_VERTICAL");
    reg.AddInvertingUnit("FT/FT", "FOOT_HORIZONTAL_PER_FOOT_VERTICAL");
    reg.AddInvertingUnit("M/M", "METRE_HORIZONTAL_PER_METRE_VERTICAL");
    }

void AddSurfaceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_DENSITY, SI, "KG/SQ.M", "KG*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, SI, "G/SQ.M", "G*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, SI, "KG/HECTARE", "KG*HECTARE(-1)");
    reg.AddUnit(SURFACE_DENSITY, USCUSTOM, "LB/ACRE", "LBM*ACRE(-1)");
    }

void AddThermalConductivity(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "W/(M*K)", "W*M(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(THERMAL_CONDUCTIVITY, SI, "W/(M*C)", "W*M(-1)*DELTA_CELSIUS(-1)");

    reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "BTU*IN*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    }

void AddThermalResistance(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_RESISTANCE, SI, "(SQ.M*KELVIN)/WATT", "M(2)*DELTA_KELVIN*W(-1)");

    reg.AddUnit(THERMAL_RESISTANCE, SI, "(SQ.M*CELSIUS)/WATT", "M(2)*DELTA_CELSIUS*W(-1)");

    reg.AddUnit(THERMAL_RESISTANCE, USCUSTOM, "(SQ.FT*HR*FAHRENHEIT)/BTU", "FT(2)*HR*DELTA_FAHRENHEIT*BTU(-1)");
    }

void AddThreadPitch(UnitRegistry& reg)
    {
    reg.AddUnit(THREAD_PITCH, SI, "M/REVOLUTION", "M*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "CM/REVOLUTION", "CM*REVOLUTION(-1)");
    reg.AddSynonym("CM/REVOLUTION", "CENTIMETRE_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, SI, "MM/REVOLUTION", "MM*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "M/RAD", "M*RAD(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "M/DEGREE", "M*ARC_DEG(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "MM/RAD", "MM*RAD(-1)");

    reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/REVOLUTION", "IN*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "FT/REVOLUTION", "FT*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/DEGREE", "IN*ARC_DEG(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/RAD", "IN*RAD(-1)");
    }

void AddVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(VELOCITY, SI, "M/S", "M*S(-1)");
    reg.AddSynonym("M/S", "METER/SECOND");
    reg.AddUnit(VELOCITY, SI, "M/MIN", "M*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "M/HR", "M*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "M/DAy", "M*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/S", "MM*S(-1)");
    reg.AddSynonym("MM/S", "MILLIMETRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "MM/MIN", "MM*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/HR", "MM*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/DAY", "MM*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "CM/S", "CM*S(-1)");
    reg.AddSynonym("CM/S", "CENTIMETER/SECOND");
    reg.AddUnit(VELOCITY, SI, "CM/MIN", "CM*MIN(-1)");
    reg.AddSynonym("CM/MIN", "CENTIMETER/MINUTE");
    reg.AddUnit(VELOCITY, SI, "CM/HOUR", "CM*HR(-1)");
    reg.AddSynonym("CM/HOUR", "CENTIMETER/HOUR");
    reg.AddUnit(VELOCITY, SI, "CM/DAY", "CM*DAY(-1)");
    reg.AddSynonym("CM/DAY", "CENTIMETER/DAY");
    reg.AddUnit(VELOCITY, SI, "KM/S", "KM*S(-1)");
    reg.AddSynonym("KM/S", "KILOMETRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "KM/HR", "KM*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "IN/SEC", "IN*S(-1)");
    reg.AddUnit(VELOCITY, SI, "IN/MIN", "IN*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "IN/HR", "IN*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "IN/DAY", "IN*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "FT/SEC", "FT*S(-1)");
    reg.AddUnit(VELOCITY, SI, "FT/MIN", "FT*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "FT/HR", "FT*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "FT/DAY", "FT*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "YRD/SEC", "YRD*S(-1)");
    reg.AddUnit(VELOCITY, SI, "MPH", "MILE*HR(-1)");

    reg.AddUnit(VELOCITY, MARITIME, "KNOT_UK_ADMIRALTY", "M*HR(-1)", 1853.184);
    reg.AddUnit(VELOCITY, MARITIME, "KNOT_INTERNATIONAL", "NAUT_MILE*HR(-1)");
    }

void AddAngularVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/S", "RAD*S(-1)");
    reg.AddSynonym("RAD/S", "RADIAN/SECOND");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/MIN", "RAD*MIN(-1)");
    reg.AddSynonym("RAD/MIN", "RADIAN/MINUTE");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/HR", "RAD*HR(-1)");
    reg.AddSynonym("RAD/HR", "RADIAN/HOUR");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPS", "[PI]*RAD*S(-1)", 2.0);
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPM", "[PI]*RAD*MIN(-1)", 2.0);
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPH", "[PI]*RAD*HR(-1)", 2.0);
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/S", "ARC_DEG*S(-1)", 1.0);
    reg.AddSynonym("DEG/S", "ARC_DEG/S");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/MIN", "ARC_DEG*MIN(-1)", 1.0);
    reg.AddSynonym("DEG/MIN", "ARC_DEG/MIN");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/HR", "ARC_DEG*HR(-1)", 1.0);
    reg.AddSynonym("DEG/HR", "ARC_DEG/HOUR");
    }

void AddDynamicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "PA-S", "PA*S");
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "POISE", "PA-S", 0.1);
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "CENTIPOISE", "[CENTI]*POISE");
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "LB/FT*S", "LBM*FT(-1)*S(-1)"); // TODO: Confirm that this is really pound mass
    reg.AddSynonym("LB/FT*S", "POUND_PER_FOOT_SECOND");
    }

void AddKinematicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.M/S", "M(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.FT/S", "FT(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "STOKE", "CM(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "CENTISTOKE", "MM(2)*S(-1)");
    }

void AddVolume(UnitRegistry& reg)
    {
    reg.AddUnit(VOLUME, SI, "CUB.M", "M(3)");
    reg.AddUnit(VOLUME, SI, "CUB.MU", "MU(3)");
    reg.AddUnit(VOLUME, SI, "CUB.MM", "MM(3)");
    reg.AddUnit(VOLUME, SI, "CUB.CM", "CM(3)");
    reg.AddSynonym("CUB.CM", "CUBIC_CENTIMETRE");
    reg.AddUnit(VOLUME, SI, "CUB.DM", "DM(3)");
    reg.AddUnit(VOLUME, SI, "CUB.KM", "KM(3)");
    reg.AddUnit(VOLUME, SI, "LITRE", "CUB.DM");
    reg.AddUnit(VOLUME, SI, "THOUSAND_LITRE", "[KILO]*LITRE");
    reg.AddUnit(VOLUME, INDUSTRIAL, "THOUSAND_GALLON", "[KILO]*GALLON");
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_GALLON", "[MEGA]*GALLON");
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_LITRE", "[MEGA]*LITRE");
    reg.AddUnit(VOLUME, SI, "MICROLITRE", "[MICRO]*LITRE");
    reg.AddSynonym("MICROLITRE", "LAMBDA"); // TODO: Check this out

    reg.AddUnit(VOLUME, IMPERIAL, "CUB.IN", "IN(3)");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.FT", "FT(3)");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.YRD", "YRD(3)");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.MILE", "MILE(3)");
    reg.AddSynonym("CUB.MILE", "MILE_CUBED");

    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_INCH", "ACRE*IN");
    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_FOOT", "ACRE*FT");
    reg.AddUnit(VOLUME, USCUSTOM, "GALLON", "IN(3)", 231.0);
    reg.AddUnit(VOLUME, IMPERIAL, "GALLON_IMPERIAL", "LITRE", 4.54609);
    }

void AddSpecificVolume(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.M/KG", "M(3)*KG(-1)");
    reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.FT/LB", "FT(3)*LBM(-1)");
    }

void AddWarpingConstant(UnitRegistry& reg)
    {
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "M^6", "M(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "MM^6", "MM(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "CM^6", "CM(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "IN^6", "IN(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, INDUSTRIAL, "FT^6", "FT(6)");
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultUnits()
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    reg.AddUnitForBasePhenomenon("M", BasePhenomena::Length);
    reg.AddUnitForBasePhenomenon("KG", BasePhenomena::Mass);
    reg.AddUnitForBasePhenomenon("S", BasePhenomena::Time);
    reg.AddUnitForBasePhenomenon("K", BasePhenomena::Temperature);
    reg.AddSynonym("K", "KELVIN");

    reg.AddUnitForBasePhenomenon("DELTA_KELVIN", BasePhenomena::TemperatureChange);

    reg.AddUnitForBasePhenomenon("A", BasePhenomena::ElectricCurrent);
    reg.AddUnitForBasePhenomenon("MOL", BasePhenomena::Mole); // Where mol is the SI gram mol or gmol.
    reg.AddSynonym("MOL", "MOLE");
    reg.AddUnitForBasePhenomenon("CD", BasePhenomena::Luminosity);
    reg.AddSynonym("CD", "CANDELA");
    reg.AddUnitForBasePhenomenon("RAD", BasePhenomena::PlaneAngle);
    reg.AddUnitForBasePhenomenon("STERAD", BasePhenomena::SolidAngle);
    reg.AddSynonym("STERAD", "STERADIAN");
    reg.AddUnitForBasePhenomenon("US$", BasePhenomena::Finance);
    reg.AddUnitForBasePhenomenon("PERSON", BasePhenomena::Capita);
    reg.AddUnitForBasePhenomenon("ONE", BasePhenomena::Ratio); // TODO: I don't like that Ratio has base unit of ONE and all unitless unit will have a phenomenon of Ratio ...



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

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultMappings ()
    {
    m_oldNameNewNameMapping["MILLIMETRE"] = "MM";
    m_oldNameNewNameMapping["CENTIMETRE"] = "CM";
    m_oldNameNewNameMapping["DECIMETRE"] = "DM";
    m_oldNameNewNameMapping["KILOMETRE"] = "KM";
    m_oldNameNewNameMapping["MICROMETRE"] = "MU";
    m_oldNameNewNameMapping["INCH"] = "IN";
    m_oldNameNewNameMapping["FOOT"] = "FT";
    m_oldNameNewNameMapping["YARD"] = "YRD";
    m_oldNameNewNameMapping["GRAM"] = "G";
    m_oldNameNewNameMapping["MILLIGRAM"] = "MG";
    m_oldNameNewNameMapping["MICROGRAM"] = "MKG";
    m_oldNameNewNameMapping["POUND"] = "LBM";
    m_oldNameNewNameMapping["MINUTE"] = "MIN";
    m_oldNameNewNameMapping["HOUR"] = "HR";
    m_oldNameNewNameMapping["YEAR"] = "YR";
    m_oldNameNewNameMapping["MILLISECOND"] = "MS";
    m_oldNameNewNameMapping["DEGREE_CELSIUS"] = "CELSIUS";
    m_oldNameNewNameMapping["DEGREE_FAHRENHEIT"] = "FAHRENHEIT";
    m_oldNameNewNameMapping["DEGREE_RANKINE"] = "RANKINE";
    m_oldNameNewNameMapping["DELTA_DEGREE_CELSIUS"] = "DELTA_CELSIUS";
    m_oldNameNewNameMapping["DELTA_DEGREE_FAHRENHEIT"] = "DELTA_FAHRENHEIT";
    m_oldNameNewNameMapping["DELTA_DEGREE_RANKINE"] = "DELTA_RANKINE";
    m_oldNameNewNameMapping["DELTA_DEGREE_KELVIN_PER_METRE"] = "KELVIN/M";
    m_oldNameNewNameMapping["RECIPROCAL_DELTA_DEGREE_KELVIN"] = "STRAIN/K";
    m_oldNameNewNameMapping["RECIPROCAL_DELTA_DEGREE_CELSIUS"] = "STRAIN/CELSIUS";
    m_oldNameNewNameMapping["RECIPROCAL_DELTA_DEGREE_FAHRENHEIT"] = "STRAIN/FAHRENHEIT";
    m_oldNameNewNameMapping["RECIPROCAL_DELTA_DEGREE_RANKINE"] = "STRAIN/RANKINE";
    m_oldNameNewNameMapping["LUMEN_PER_FOOT_SQUARED"] = "LUMEN/SQ.FT";
    m_oldNameNewNameMapping["NEWTON_METRE_PER_RADIAN"] = "(N*M)/RAD";
    m_oldNameNewNameMapping["NEWTON_METRE_PER_DEGREE"] = "(N*M)/DEG";
    m_oldNameNewNameMapping["NEWTON_PER_RADIAN"] = "N/RAD";
    m_oldNameNewNameMapping["DEGREE"] = "ARC_DEG";
    m_oldNameNewNameMapping["ANGLE_MINUTE"] = "ARC_MINUTE";
    m_oldNameNewNameMapping["ANGLE_SECOND"] = "ARC_SECOND";
    m_oldNameNewNameMapping["ANGLE_QUADRANT"] = "ARC_QUADRANT";
    m_oldNameNewNameMapping["GRADIAN"] = "GRAD";
    m_oldNameNewNameMapping["METRE_SQUARED"] = "SQ.M";
    m_oldNameNewNameMapping["MILLIMETRE_SQUARED"] = "SQ.MM";
    m_oldNameNewNameMapping["CENTIMETRE_SQUARED"] = "SQ.CM";
    m_oldNameNewNameMapping["KILOMETRE_SQUARED"] = "SQ.KM";
    m_oldNameNewNameMapping["INCH_SQUARED"] = "SQ.IN";
    m_oldNameNewNameMapping["FOOT_SQUARED"] = "SQ.FT";
    m_oldNameNewNameMapping["YARD_SQUARED"] = "SQ.YRD";
    m_oldNameNewNameMapping["MILE_SQUARED"] = "SQ.MILE";
    m_oldNameNewNameMapping["KILOGRAM_PER_METRE_CUBED"] = "KG/CUB.M";
    m_oldNameNewNameMapping["KILOGRAM_PER_CENTIMETRE_CUBED"] = "KG/CUB.CM";
    m_oldNameNewNameMapping["KILOGRAM_PER_DECIMETRE_CUBED"] = "KG/LITRE";
    m_oldNameNewNameMapping["KILOGRAM_PER_LITRE"] = "KG/LITRE";
    m_oldNameNewNameMapping["GRAM_PER_CENTIMETRE_CUBED"] = "G/CUB.CM";
    m_oldNameNewNameMapping["MICROGRAM_PER_LITRE"] = "MKG/LITRE";
    m_oldNameNewNameMapping["MILLIGRAM_PER_LITRE"] = "MG/LITRE";
    m_oldNameNewNameMapping["POUND_PER_FOOT_CUBED"] = "LBM/CUB.FT";
    m_oldNameNewNameMapping["POUND_PER_GALLON"] = "LBM/GAL";
    m_oldNameNewNameMapping["POUND_PER_IMPERIAL_GALLON"] = "LBM/GALLON_IMPERIAL";
    m_oldNameNewNameMapping["POUND_PER_INCH_CUBED"] = "LBM/CUB.IN";
    m_oldNameNewNameMapping["POUND_PER_MILLION_GALLON"] = "LBM/MILLION_GALLON";
    m_oldNameNewNameMapping["SLUG_PER_FOOT_CUBED"] = "SLUG/CUB.FT";
    m_oldNameNewNameMapping["KIP_PER_FOOT_CUBED"] = "KIP/CUB.FT";
    m_oldNameNewNameMapping["SHORT_TON_PER_FOOT_CUBED"] = "SHORT_TON/CUB.FT";
    m_oldNameNewNameMapping["NEWTON_PER_METRE_CUBED"] = "N/CUB.M";
    m_oldNameNewNameMapping["KILONEWTON_PER_METRE_CUBED"] = "KN/CUB.M";
    m_oldNameNewNameMapping["KILONEWTON_PER_FOOT_CUBED"] = "KN/CUB.FT";
    m_oldNameNewNameMapping["PERSON_PER_METRE_SQUARED"] = "PERSON/SQ.M";
    m_oldNameNewNameMapping["PERSON_PER_HECTARE"] = "PERSON/HECTARE";
    m_oldNameNewNameMapping["PERSON_PER_KILOMETRE_SQUARED"] = "PERSON/SQ.KM";
    m_oldNameNewNameMapping["PERSON_PER_ACRE"] = "PERSON/ACRE";
    m_oldNameNewNameMapping["PERSON_PER_FOOT_SQUARED"] = "PERSON/SQ.FT";
    m_oldNameNewNameMapping["PERSON_PER_MILE_SQUARED"] = "PERSON/SQ.MILE";
    m_oldNameNewNameMapping["JOULE"] = "J";
    m_oldNameNewNameMapping["KILOJOULE"] = "KJ";
    m_oldNameNewNameMapping["MEGAJOULE"] = "MJ";
    m_oldNameNewNameMapping["GIGAJOULE"] = "GJ";
    m_oldNameNewNameMapping["KILOWATT_HOUR"] = "KWH";
    m_oldNameNewNameMapping["MEGAWATT_HOUR"] = "MWH";
    m_oldNameNewNameMapping["GIGAWATT_HOUR"] = "GWH";
    m_oldNameNewNameMapping["JOULE_PER_METRE_CUBED"] = "J/CUB.M";
    m_oldNameNewNameMapping["KILOJOULE_PER_METRE_CUBED"] = "KJ/CUB.M";
    m_oldNameNewNameMapping["KILOWATT_HOUR_PER_METRE_CUBED"] = "KWH/CUB.M";
    m_oldNameNewNameMapping["KILOWATT_HOUR_PER_FOOT_CUBED"] = "KWH/CUB.FT";
    m_oldNameNewNameMapping["KILOWATT_HOUR_PER_MILLION_GALLON"] = "KWH/MILLION_GALLON";
    m_oldNameNewNameMapping["KILOJOULE_PER_KILOGRAM"] = "KJ/KG";
    m_oldNameNewNameMapping["MEGAJOULE_PER_KILOGRAM"] = "MEGAJ/KG";
    m_oldNameNewNameMapping["BTU_PER_POUND_MASS"] = "BTU/LBM";
    m_oldNameNewNameMapping["JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN"] = "J/(KG*K)";
    m_oldNameNewNameMapping["BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE"] = "BTU/(LBM*RANKINE)";
    m_oldNameNewNameMapping["JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN"] = "J/(KMOL*K)";
    m_oldNameNewNameMapping["KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN"] = "KJ/(KMOL*K)";
    m_oldNameNewNameMapping["BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE"] = "BTU/(LB-MOLE*RANKINE)";
    m_oldNameNewNameMapping["METRE_CUBED_PER_SECOND"] = "CUB.M/SEC";
    m_oldNameNewNameMapping["METRE_CUBED_PER_MINUTE"] = "CUB.M/MIN";
    m_oldNameNewNameMapping["METRE_CUBED_PER_HOUR"] = "CUB.M/HR";
    m_oldNameNewNameMapping["METRE_CUBED_PER_DAY"] = "CUB.M/DAY";
    m_oldNameNewNameMapping["LITRE_PER_SECOND"] = "LITRE/SEC";
    m_oldNameNewNameMapping["LITRE_PER_MINUTE"] = "LITRE/MIN";
    m_oldNameNewNameMapping["LITRE_PER_HOUR"] = "LITRE/HR";
    m_oldNameNewNameMapping["LITRE_PER_DAY"] = "LITRE/DAY";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_SECOND"] = "CUB.FT/SEC";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_MINUTE"] = "CUB.FT/MIN";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_DAY"] = "CUB.FT/DAY";
    m_oldNameNewNameMapping["ACRE_FOOT_PER_DAY"] = "ACRE_FOOT/DAY";
    m_oldNameNewNameMapping["ACRE_FOOT_PER_HOUR"] = "ACRE_FOOT/HR";
    m_oldNameNewNameMapping["ACRE_FOOT_PER_MINUTE"] = "ACRE_FOOT/MIN";
    m_oldNameNewNameMapping["ACRE_INCH_PER_HOUR"] = "ACRE_INCH/HOUR";
    m_oldNameNewNameMapping["ACRE_INCH_PER_MINUTE"] = "ACRE_INCH/MIN";
    m_oldNameNewNameMapping["GALLON_IMPERIAL_PER_DAY"] = "GALLON_IMPERIAL/DAY";
    m_oldNameNewNameMapping["GALLON_IMPERIAL_PER_MINUTE"] = "GALLON_IMPERIAL/MINUTE";
    m_oldNameNewNameMapping["GALLON_IMPERIAL_PER_SECOND"] = "GALLON_IMPERIAL/SECOND";
    m_oldNameNewNameMapping["GALLON_PER_SECOND"] = "GALLON/S";
    m_oldNameNewNameMapping["GALLON_PER_MINUTE"] = "GALLON/MIN";
    m_oldNameNewNameMapping["GALLON_PER_DAY"] = "GALLON/DAY";
    m_oldNameNewNameMapping["METRE_CUBED_PER_METRE_SQUARED_PER_DAY"] = "CUB.M/(DAY*SQ.M)";
    m_oldNameNewNameMapping["METRE_CUBED_PER_HECTARE_PER_DAY"] = "CUB.M/(DAY*HECTARE)";
    m_oldNameNewNameMapping["METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY"] = "CUB.M/(DAY*SQ.KM)";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE"] = "CUB.FT/(SQ.FT*MIN)";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND"] = "CUB.FT/(SQ.FT*S)";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND"] = "CUB.FT/(SQ.MILE*S)";
    m_oldNameNewNameMapping["GALLON_PER_ACRE_PER_DAY"] = "GALLON/(DAY*ACRE)";
    m_oldNameNewNameMapping["GALLON_PER_ACRE_PER_MINUTE"] = "GALLON/(MIN*ACRE)";
    m_oldNameNewNameMapping["GALLON_PER_FOOT_SQUARED_PER_MINUTE"] = "GALLON/(MIN*SQ.FT)";
    m_oldNameNewNameMapping["GALLON_PER_FOOT_SQUARED_PER_DAY"] = "GALLON/(DAY*SQ.FT)";
    m_oldNameNewNameMapping["GALLON_PER_MILE_SQUARED_PER_MINUTE"] = "GALLON/(MIN*SQ.MILE)";
    m_oldNameNewNameMapping["GALLON_PER_MILE_SQUARED_PER_DAY"] = "GALLON/(DAY*SQ.MILE)";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_ACRE_PER_SECOND"] = "CUB.FT/(ACRE*SEC)";
    m_oldNameNewNameMapping["KILOGRAM_PER_SECOND"] = "KG/S";
    m_oldNameNewNameMapping["KILOGRAM_PER_MINUTE"] = "KG/MIN";
    m_oldNameNewNameMapping["KILOGRAM_PER_HOUR"] = "KG/HR";
    m_oldNameNewNameMapping["KILOGRAM_PER_DAY"] = "KG/DAY";
    m_oldNameNewNameMapping["GRAM_PER_SECOND"] = "G/S";
    m_oldNameNewNameMapping["GRAM_PER_MINUTE"] = "G/MIN";
    m_oldNameNewNameMapping["GRAM_PER_HOUR"] = "G/HR";
    m_oldNameNewNameMapping["MILLIGRAM_PER_SECOND"] = "MG/S";
    m_oldNameNewNameMapping["MILLIGRAM_PER_MINUTE"] = "MG/MIN";
    m_oldNameNewNameMapping["MILLIGRAM_PER_HOUR"] = "MG/HR";
    m_oldNameNewNameMapping["MILLIGRAM_PER_DAY"] = "MG/DAY";
    m_oldNameNewNameMapping["MICROGRAM_PER_SECOND"] = "MKG/S";
    m_oldNameNewNameMapping["MICROGRAM_PER_MINUTE"] = "MKG/MIN";
    m_oldNameNewNameMapping["MICROGRAM_PER_HOUR"] = "MKG/HR";
    m_oldNameNewNameMapping["MICROGRAM_PER_DAY"] = "MKG/DAY";
    m_oldNameNewNameMapping["POUND_PER_SECOND"] = "LB/S";
    m_oldNameNewNameMapping["POUND_PER_MINUTE"] = "LB/MIN";
    m_oldNameNewNameMapping["POUND_PER_HOUR"] = "LB/HR";
    m_oldNameNewNameMapping["POUND_PER_DAY"] = "LB/DAY";
    m_oldNameNewNameMapping["TONNE_PER_HOUR"] = "TONNE/HR";
    m_oldNameNewNameMapping["SHORT_TON_PER_HOUR"] = "SHORT_TON/HR";
    m_oldNameNewNameMapping["MOLE_PER_SECOND"] = "MOL/S";
    m_oldNameNewNameMapping["KILOMOLE_PER_SECOND"] = "KMOL/S";
    m_oldNameNewNameMapping["NEWTON"] = "N";
    m_oldNameNewNameMapping["KILONEWTON"] = "KN";
    m_oldNameNewNameMapping["MILLINEWTON"] = "mN";
    m_oldNameNewNameMapping["KILOGRAM_FORCE"] = "KGF";
    m_oldNameNewNameMapping["POUND_FORCE"] = "LBF";
    m_oldNameNewNameMapping["KILOPOUND_FORCE"] = "KPF";
    m_oldNameNewNameMapping["WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN"] = "W/(SQ.M*K)";
    m_oldNameNewNameMapping["WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS"] = "W/(SQ.M*CELSIUS)";
    m_oldNameNewNameMapping["BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT"] = "BTU/(SQ.FT*HR*FAHRENHEIT)";
    m_oldNameNewNameMapping["KILOGRAM_PER_METRE"] = "KG/M";
    m_oldNameNewNameMapping["KILOGRAM_PER_MILLIMETRE"] = "KG/MM";
    m_oldNameNewNameMapping["POUND_MASS_PER_FOOT"] = "LBM/FT";
    m_oldNameNewNameMapping["NEWTON_PER_METRE"] = "N/M";
    m_oldNameNewNameMapping["NEWTON_PER_MILLIMETRE"] = "N/MM";
    m_oldNameNewNameMapping["POUND_FORCE_PER_INCH"] = "LBF/IN";
    m_oldNameNewNameMapping["ONE_PER_METRE"] = "PER_M";
    m_oldNameNewNameMapping["ONE_PER_MILLIMETRE"] = "PER_MM";
    m_oldNameNewNameMapping["ONE_PER_KILOMETRE"] = "PER_KM";
    m_oldNameNewNameMapping["ONE_PER_FOOT"] = "PER_FT";
    m_oldNameNewNameMapping["ONE_PER_MILE"] = "PER_MILE";
    m_oldNameNewNameMapping["ONE_PER_THOUSAND_FOOT"] = "PER_THOUSAND_FT";
    m_oldNameNewNameMapping["NEWTON_METRE"] = "N_M";
    m_oldNameNewNameMapping["POUND_FOOT"] = "LBF_FT";
    m_oldNameNewNameMapping["METRE_CUBED_PER_MOLE"] = "CUB.M/MOLE";
    m_oldNameNewNameMapping["METRE_CUBED_PER_KILOMOLE"] = "CUB.M/KMOL";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_POUND_MOLE"] = "CUB.FT/LB-MOLE";
    m_oldNameNewNameMapping["MOLE_PER_METRE_CUBED"] = "MOL/CUB.M";
    m_oldNameNewNameMapping["KILOMOLE_PER_METRE_CUBED"] = "KMOL/CUB.M";
    m_oldNameNewNameMapping["METRE_TO_THE_FOURTH"] = "M^4";
    m_oldNameNewNameMapping["CENTIMETRE_TO_THE_FOURTH"] = "CM^4";
    m_oldNameNewNameMapping["INCH_TO_THE_FOURTH"] = "IN^4";
    m_oldNameNewNameMapping["FOOT_TO_THE_FOURTH"] = "FT^4";
    m_oldNameNewNameMapping["MILLIMETRE_TO_THE_FOURTH"] = "MM^4";
    m_oldNameNewNameMapping["WATT"] = "W";
    m_oldNameNewNameMapping["KILOWATT"] = "KW";
    m_oldNameNewNameMapping["MEGAWATT"] = "MW";
    m_oldNameNewNameMapping["GIGAWATT"] = "GW";
    m_oldNameNewNameMapping["BTU_PER_MONTH"] = "BTU/MONTH";
    m_oldNameNewNameMapping["BTU_PER_HOUR"] = "BTU/HOUR";
    m_oldNameNewNameMapping["KILOBTU_PER_HOUR"] = "KILOBTU/HOUR";
    m_oldNameNewNameMapping["HORSEPOWER"] = "HP";
    m_oldNameNewNameMapping["GIGAJOULE_PER_MONTH"] = "GJ/MONTH";
    m_oldNameNewNameMapping["PASCAL"] = "PA";
    m_oldNameNewNameMapping["NEWTON_PER_METRE_SQUARED"] = "PA";
    m_oldNameNewNameMapping["NEWTON_PER_MILLIMETRE_SQUARED"] = "N/SQ.MM";
    m_oldNameNewNameMapping["KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED"] = "AT";
    m_oldNameNewNameMapping["KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE"] = "AT_GAUGE";
    m_oldNameNewNameMapping["KILOGRAM_FORCE_PER_METRE_SQUARED"] = "KGF/SQ.M";
    m_oldNameNewNameMapping["ATMOSPHERE"] = "ATM";
    m_oldNameNewNameMapping["MILLIBAR"] = "MBAR";
    m_oldNameNewNameMapping["POUND_FORCE_PER_INCH_SQUARED"] = "PSI";
    m_oldNameNewNameMapping["POUND_FORCE_PER_INCH_SQUARED_GAUGE"] = "PSIG";
    m_oldNameNewNameMapping["POUND_FORCE_PER_FOOT_SQUARED"] = "LBF/SQ.FT";
    m_oldNameNewNameMapping["PASCAL_PER_METRE"] = "PA/M";
    m_oldNameNewNameMapping["BAR_PER_KILOMETRE"] = "BAR/KM";
    m_oldNameNewNameMapping["PERCENT_PERCENT"] = "PERCENT";
    m_oldNameNewNameMapping["UNITLESS_PERCENT"] = "DECIMAL_PERCENT";
    m_oldNameNewNameMapping["METRE_PER_METRE"] = "M/M";
    m_oldNameNewNameMapping["METRE_VERTICAL_PER_METRE_HORIZONTAL"] = "M/M";
    m_oldNameNewNameMapping["CENTIMETRE_PER_METRE"] = "CM/M";
    m_oldNameNewNameMapping["MILLIMETRE_PER_METRE"] = "MM/M";
    m_oldNameNewNameMapping["METRE_PER_KILOMETRE"] = "M/KM";
    m_oldNameNewNameMapping["FOOT_PER_FOOT"] = "FT/FT";
    m_oldNameNewNameMapping["FOOT_VERTICAL_PER_FOOT_HORIZONTAL"] = "FT/FT";
    m_oldNameNewNameMapping["INCH_PER_FOOT"] = "IN/FT";
    m_oldNameNewNameMapping["FOOT_PER_INCH"] = "FT/IN";
    m_oldNameNewNameMapping["FOOT_PER_MILE"] = "FT/MILE";
    m_oldNameNewNameMapping["KILOGRAM_PER_METRE_SQUARED"] = "KG/SQ.M";
    m_oldNameNewNameMapping["GRAM_PER_METRE_SQUARED"] = "G/SQ.M";
    m_oldNameNewNameMapping["KILOGRAM_PER_HECTARE"] = "KG/HECTARE";
    m_oldNameNewNameMapping["POUND_PER_ACRE"] = "LB/ACRE";
    m_oldNameNewNameMapping["WATT_PER_METRE_PER_DEGREE_KELVIN"] = "W/(M*K)";
    m_oldNameNewNameMapping["WATT_PER_METRE_PER_DEGREE_CELSIUS"] = "W/(M*C)";
    m_oldNameNewNameMapping["BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT"] = "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)";
    m_oldNameNewNameMapping["METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT"] = "(SQ.M*KELVIN)/WATT";
    m_oldNameNewNameMapping["METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT"] = "(SQ.M*CELSIUS)/WATT";
    m_oldNameNewNameMapping["FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU"] = "(SQ.FT*HR*FAHRENHEIT)/BTU";
    m_oldNameNewNameMapping["METRE_PER_REVOLUTION"] = "M/REVOLUTION";
    m_oldNameNewNameMapping["MILLIMETRE_PER_REVOLUTION"] = "MM/REVOLUTION";
    m_oldNameNewNameMapping["METRE_PER_RADIAN"] = "M/RAD";
    m_oldNameNewNameMapping["METRE_PER_DEGREE"] = "M/DEGREE";
    m_oldNameNewNameMapping["MILLIMETRE_PER_RADIAN"] = "MM/RAD";
    m_oldNameNewNameMapping["INCH_PER_REVOLUTION"] = "IN/REVOLUTION";
    m_oldNameNewNameMapping["FOOT_PER_REVOLUTION"] = "FT/REVOLUTION";
    m_oldNameNewNameMapping["INCH_PER_DEGREE"] = "INCH/DEGREE";
    m_oldNameNewNameMapping["INCH_PER_RADIAN"] = "INCH/RAD";
    m_oldNameNewNameMapping["METRE_PER_SECOND"] = "M/S";
    m_oldNameNewNameMapping["METRE_PER_MINUTE"] = "M/MIN";
    m_oldNameNewNameMapping["METRE_PER_HOUR"] = "M/HR";
    m_oldNameNewNameMapping["METRE_PER_DAY"] = "M/DAy";
    m_oldNameNewNameMapping["MILLIMETRE_PER_MINUTE"] = "MM/MIN";
    m_oldNameNewNameMapping["MILLIMETRE_PER_HOUR"] = "MM/HR";
    m_oldNameNewNameMapping["MILLIMETRE_PER_DAY"] = "MM/DAY";
    m_oldNameNewNameMapping["CENTIMETRE_PER_SECOND"] = "CM/S";
    m_oldNameNewNameMapping["CENTIMETRE_PER_MINUTE"] = "CM/MIN";
    m_oldNameNewNameMapping["CENTIMETRE_PER_HOUR"] = "CM/HOUR";
    m_oldNameNewNameMapping["CENTIMETRE_PER_DAY"] = "CM/DAY";
    m_oldNameNewNameMapping["KILOMETRE_PER_HOUR"] = "KM/HR";
    m_oldNameNewNameMapping["INCH_PER_SECOND"] = "IN/SEC";
    m_oldNameNewNameMapping["INCH_PER_MINUTE"] = "IN/MIN";
    m_oldNameNewNameMapping["INCH_PER_HOUR"] = "IN/HR";
    m_oldNameNewNameMapping["INCH_PER_DAY"] = "IN/DAY";
    m_oldNameNewNameMapping["FOOT_PER_SECOND"] = "FT/SEC";
    m_oldNameNewNameMapping["FOOT_PER_MINUTE"] = "FT/MIN";
    m_oldNameNewNameMapping["FOOT_PER_HOUR"] = "FT/HR";
    m_oldNameNewNameMapping["FOOT_PER_DAY"] = "FT/DAY";
    m_oldNameNewNameMapping["MILE_PER_HOUR"] = "MPH";
    m_oldNameNewNameMapping["KNOT"] = "KNOT_UK_ADMIRALTY";    // NOTE: OldSystem used admiralty knot conversion factor for knot conversion factor
    m_oldNameNewNameMapping["KNOT_INTERNATIONAL"] = "KNOT_INTERNATIONAL";
    m_oldNameNewNameMapping["RADIAN_PER_SECOND"] = "RAD/S";
    m_oldNameNewNameMapping["RADIAN_PER_MINUTE"] = "RAD/MIN";
    m_oldNameNewNameMapping["RADIAN_PER_HOUR"] = "RAD/HR";
    m_oldNameNewNameMapping["CYCLE_PER_SECOND"] = "RPS";
    m_oldNameNewNameMapping["CYCLE_PER_MINUTE"] = "RPM";
    m_oldNameNewNameMapping["REVOLUTION_PER_MINUTE"] = "RPM";
    m_oldNameNewNameMapping["CYCLE_PER_HOUR"] = "RPH";
    m_oldNameNewNameMapping["DEGREE_PER_SECOND"] = "DEG/S";
    m_oldNameNewNameMapping["DEGREE_PER_MINUTE"] = "DEG/MIN";
    m_oldNameNewNameMapping["DEGREE_PER_HOUR"] = "DEG/HR";
    m_oldNameNewNameMapping["PASCAL_SECOND"] = "PA-S";
    m_oldNameNewNameMapping["METRE_SQUARED_PER_SECOND"] = "SQ.M/S";
    m_oldNameNewNameMapping["FOOT_SQUARED_PER_SECOND"] = "SQ.FT/S";
    m_oldNameNewNameMapping["METRE_CUBED"] = "CUB.M";
    m_oldNameNewNameMapping["CENTIMETRE_CUBED"] = "CUB.CM";
    m_oldNameNewNameMapping["INCH_CUBED"] = "CUB.IN";
    m_oldNameNewNameMapping["FOOT_CUBED"] = "CUB.FT";
    m_oldNameNewNameMapping["YARD_CUBED"] = "CUB.YRD";
    m_oldNameNewNameMapping["METRE_CUBED_PER_KILOGRAM"] = "CUB.M/KG";
    m_oldNameNewNameMapping["FOOT_CUBED_PER_POUND_MASS"] = "CUB.FT/LB";
    m_oldNameNewNameMapping["METRE_TO_THE_SIXTH"] = "M^6";
    m_oldNameNewNameMapping["MILLIMETRE_TO_THE_SIXTH"] = "MM^6";
    m_oldNameNewNameMapping["CENTIMETRE_TO_THE_SIXTH"] = "CM^6";
    m_oldNameNewNameMapping["INCH_TO_THE_SIXTH"] = "IN^6";
    m_oldNameNewNameMapping["FOOT_TO_THE_SIXTH"] = "FT^6";
    m_oldNameNewNameMapping["METRE"] = "M";
    m_oldNameNewNameMapping["KILOGRAM"] = "KG";
    m_oldNameNewNameMapping["SECOND"] = "S";
    m_oldNameNewNameMapping["DEGREE_KELVIN"] = "K";
    m_oldNameNewNameMapping["DELTA_DEGREE_KELVIN"] = "DELTA_KELVIN";
    m_oldNameNewNameMapping["AMPERE"] = "A";
    m_oldNameNewNameMapping["RADIAN"] = "RAD";
    m_oldNameNewNameMapping["DOLLAR"] = "US$";
    m_oldNameNewNameMapping["NONE"] = "ONE";
    m_oldNameNewNameMapping["UNITLESS_UNIT"] = "ONE";
    m_oldNameNewNameMapping["THOUSAND_FOOT_SQUARED"] = "THOUSAND_SQ.FT";
    m_oldNameNewNameMapping["DYNE"] = "DYNE";
    m_oldNameNewNameMapping["FOOT_POUNDAL"] = "FOOT_POUNDAL";
    m_oldNameNewNameMapping["KILOPASCAL_GAUGE"] = "KILOPASCAL_GAUGE";
    m_oldNameNewNameMapping["PERCENT_SLOPE"] = "PERCENT_SLOPE";
    m_oldNameNewNameMapping["MILE"] = "MILE";
    m_oldNameNewNameMapping["MICROINCH"] = "MICROINCH";
    m_oldNameNewNameMapping["MILLIINCH"] = "MILLIINCH";
    m_oldNameNewNameMapping["MILLIFOOT"] = "MILLIFOOT";
    m_oldNameNewNameMapping["US_SURVEY_INCH"] = "US_SURVEY_INCH";
    m_oldNameNewNameMapping["US_SURVEY_FOOT"] = "US_SURVEY_FOOT";
    m_oldNameNewNameMapping["US_SURVEY_MILE"] = "US_SURVEY_MILE";
    m_oldNameNewNameMapping["HECTARE"] = "HECTARE";
    m_oldNameNewNameMapping["ACRE"] = "ACRE";
    m_oldNameNewNameMapping["INCH_MILE"] = "INCH_MILE";
    m_oldNameNewNameMapping["INCH_FOOT"] = "INCH_FOOT";
    m_oldNameNewNameMapping["FOOT_MILE"] = "FOOT_MILE";
    m_oldNameNewNameMapping["FOOT_FOOT"] = "FOOT_FOOT";
    m_oldNameNewNameMapping["MILLIMETRE_METRE"] = "MILLIMETRE_METRE";
    m_oldNameNewNameMapping["MILLIMETRE_KILOMETRE"] = "MILLIMETRE_KILOMETRE";
    m_oldNameNewNameMapping["METRE_METRE"] = "METRE_METRE";
    m_oldNameNewNameMapping["METRE_KILOMETRE"] = "METRE_KILOMETRE";
    m_oldNameNewNameMapping["INCH_METRE"] = "INCH_METRE";
    m_oldNameNewNameMapping["MILLIMETRE_MILE"] = "MILLIMETRE_MILE";
    m_oldNameNewNameMapping["ACRE_FOOT"] = "ACRE_FOOT";
    m_oldNameNewNameMapping["ACRE_INCH"] = "ACRE_INCH";
    m_oldNameNewNameMapping["GALLON"] = "GALLON";
    m_oldNameNewNameMapping["GALLON_IMPERIAL"] = "GALLON_IMPERIAL";
    m_oldNameNewNameMapping["LITRE"] = "LITRE";
    m_oldNameNewNameMapping["MILLION_GALLON"] = "MILLION_GALLON";
    m_oldNameNewNameMapping["MILLION_LITRE"] = "MILLION_LITRE";
    m_oldNameNewNameMapping["THOUSAND_GALLON"] = "THOUSAND_GALLON";
    m_oldNameNewNameMapping["THOUSAND_LITRE"] = "THOUSAND_LITRE";
    m_oldNameNewNameMapping["GRAIN"] = "GRAIN";
    m_oldNameNewNameMapping["LONG_TON"] = "LONG_TON";
    m_oldNameNewNameMapping["MEGAGRAM"] = "MEGAGRAM";
    m_oldNameNewNameMapping["SHORT_TON"] = "SHORT_TON";
    m_oldNameNewNameMapping["DAY"] = "DAY";
    m_oldNameNewNameMapping["REVOLUTION"] = "REVOLUTION";
    m_oldNameNewNameMapping["CENTISTOKE"] = "CENTISTOKE";
    m_oldNameNewNameMapping["STOKE"] = "STOKE";
    m_oldNameNewNameMapping["LONG_TON_FORCE"] = "LONG_TON_FORCE";
    m_oldNameNewNameMapping["SHORT_TON_FORCE"] = "SHORT_TON_FORCE";
    m_oldNameNewNameMapping["BTU"] = "BTU";
    m_oldNameNewNameMapping["KILOBTU"] = "KILOBTU";
    m_oldNameNewNameMapping["WATT_SECOND"] = "WATT_SECOND";
    m_oldNameNewNameMapping["KILOVOLT"] = "KILOVOLT";
    m_oldNameNewNameMapping["VOLT"] = "VOLT";
    m_oldNameNewNameMapping["MEGAVOLT"] = "MEGAVOLT";
    m_oldNameNewNameMapping["CAPITA"] = "CAPITA";
    m_oldNameNewNameMapping["PERSON"] = "PERSON";
    m_oldNameNewNameMapping["CUSTOMER"] = "CUSTOMER";
    m_oldNameNewNameMapping["EMPLOYEE"] = "EMPLOYEE";
    m_oldNameNewNameMapping["GUEST"] = "GUEST";
    m_oldNameNewNameMapping["HUNDRED_CAPITA"] = "HUNDRED_CAPITA";
    m_oldNameNewNameMapping["THOUSAND_CAPITA"] = "THOUSAND_CAPITA";
    m_oldNameNewNameMapping["PASSENGER"] = "PASSENGER";
    m_oldNameNewNameMapping["RESIDENT"] = "RESIDENT";
    m_oldNameNewNameMapping["STUDENT"] = "STUDENT";
    m_oldNameNewNameMapping["VERTICAL_PER_HORIZONTAL"] = "VERTICAL_PER_HORIZONTAL";
    m_oldNameNewNameMapping["FOOT_HORIZONTAL_PER_FOOT_VERTICAL"] = "FOOT_HORIZONTAL_PER_FOOT_VERTICAL";
    m_oldNameNewNameMapping["METRE_HORIZONTAL_PER_METRE_VERTICAL"] = "METRE_HORIZONTAL_PER_METRE_VERTICAL";
    m_oldNameNewNameMapping["HORIZONTAL_PER_VERTICAL"] = "HORIZONTAL_PER_VERTICAL";
    m_oldNameNewNameMapping["FOOT_PER_1000_FOOT"] = "FOOT_PER_1000_FOOT";
    m_oldNameNewNameMapping["CENTIPOISE"] = "CENTIPOISE";
    m_oldNameNewNameMapping["LUX"] = "LUX";
    m_oldNameNewNameMapping["KILOAMPERE"] = "KILOAMPERE";
    m_oldNameNewNameMapping["LUMEN"] = "LUMEN";
    m_oldNameNewNameMapping["KILOPASCAL"] = "KILOPASCAL";
    m_oldNameNewNameMapping["BAR"] = "BAR";
    m_oldNameNewNameMapping["BAR_GAUGE"] = "BAR_GAUGE";
    m_oldNameNewNameMapping["BARYE"] = "BARYE";
    m_oldNameNewNameMapping["FOOT_OF_H2O_CONVENTIONAL"] = "FOOT_OF_H2O_CONVENTIONAL";
    m_oldNameNewNameMapping["HECTOPASCAL"] = "HECTOPASCAL";
    m_oldNameNewNameMapping["INCH_OF_H2O_AT_32_FAHRENHEIT"] = "INCH_OF_H2O_AT_32_FAHRENHEIT";
    m_oldNameNewNameMapping["INCH_OF_H2O_AT_39_2_FAHRENHEIT"] = "INCH_OF_H2O_AT_39_2_FAHRENHEIT";
    m_oldNameNewNameMapping["INCH_OF_H2O_AT_60_FAHRENHEIT"] = "INCH_OF_H2O_AT_60_FAHRENHEIT";
    m_oldNameNewNameMapping["INCH_OF_HG_AT_32_FAHRENHEIT"] = "INCH_OF_HG_AT_32_FAHRENHEIT";
    m_oldNameNewNameMapping["INCH_OF_HG_AT_60_FAHRENHEIT"] = "INCH_OF_HG_AT_60_FAHRENHEIT";
    m_oldNameNewNameMapping["INCH_OF_HG_CONVENTIONAL"] = "INCH_OF_HG_CONVENTIONAL";
    m_oldNameNewNameMapping["MEGAPASCAL"] = "MEGAPASCAL";
    m_oldNameNewNameMapping["MEGAPASCAL_GAUGE"] = "MEGAPASCAL_GAUGE";
    m_oldNameNewNameMapping["METRE_OF_H2O_CONVENTIONAL"] = "METRE_OF_H2O_CONVENTIONAL";
    m_oldNameNewNameMapping["MILLIMETRE_OF_H2O_CONVENTIONAL"] = "MILLIMETRE_OF_H2O_CONVENTIONAL";
    m_oldNameNewNameMapping["MILLIMETRE_OF_HG_AT_32_FAHRENHEIT"] = "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT";
    m_oldNameNewNameMapping["TORR"] = "TORR";
    m_oldNameNewNameMapping["HERTZ"] = "HERTZ";



    m_newNameOldNameMapping["MM"] = "MILLIMETRE";
    m_newNameOldNameMapping["CM"] = "CENTIMETRE";
    m_newNameOldNameMapping["DM"] = "DECIMETRE";
    m_newNameOldNameMapping["KM"] = "KILOMETRE";
    m_newNameOldNameMapping["MU"] = "MICROMETRE";
    m_newNameOldNameMapping["IN"] = "INCH";
    m_newNameOldNameMapping["FT"] = "FOOT";
    m_newNameOldNameMapping["YRD"] = "YARD";
    m_newNameOldNameMapping["G"] = "GRAM";
    m_newNameOldNameMapping["MG"] = "MILLIGRAM";
    m_newNameOldNameMapping["MKG"] = "MICROGRAM";
    m_newNameOldNameMapping["LBM"] = "POUND";
    m_newNameOldNameMapping["MIN"] = "MINUTE";
    m_newNameOldNameMapping["HR"] = "HOUR";
    m_newNameOldNameMapping["YR"] = "YEAR";
    m_newNameOldNameMapping["MS"] = "MILLISECOND";
    m_newNameOldNameMapping["CELSIUS"] = "DEGREE_CELSIUS";
    m_newNameOldNameMapping["FAHRENHEIT"] = "DEGREE_FAHRENHEIT";
    m_newNameOldNameMapping["RANKINE"] = "DEGREE_RANKINE";
    m_newNameOldNameMapping["DELTA_CELSIUS"] = "DELTA_DEGREE_CELSIUS";
    m_newNameOldNameMapping["DELTA_FAHRENHEIT"] = "DELTA_DEGREE_FAHRENHEIT";
    m_newNameOldNameMapping["DELTA_RANKINE"] = "DELTA_DEGREE_RANKINE";
    m_newNameOldNameMapping["KELVIN/M"] = "DELTA_DEGREE_KELVIN_PER_METRE";
    m_newNameOldNameMapping["STRAIN/K"] = "RECIPROCAL_DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["STRAIN/CELSIUS"] = "RECIPROCAL_DELTA_DEGREE_CELSIUS";
    m_newNameOldNameMapping["STRAIN/FAHRENHEIT"] = "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT";
    m_newNameOldNameMapping["STRAIN/RANKINE"] = "RECIPROCAL_DELTA_DEGREE_RANKINE";
    m_newNameOldNameMapping["LUMEN/SQ.FT"] = "LUMEN_PER_FOOT_SQUARED";
    m_newNameOldNameMapping["(N*M)/RAD"] = "NEWTON_METRE_PER_RADIAN";
    m_newNameOldNameMapping["(N*M)/DEG"] = "NEWTON_METRE_PER_DEGREE";
    m_newNameOldNameMapping["N/RAD"] = "NEWTON_PER_RADIAN";
    m_newNameOldNameMapping["ARC_DEG"] = "DEGREE";
    m_newNameOldNameMapping["ARC_MINUTE"] = "ANGLE_MINUTE";
    m_newNameOldNameMapping["ARC_SECOND"] = "ANGLE_SECOND";
    m_newNameOldNameMapping["ARC_QUADRANT"] = "ANGLE_QUADRANT";
    m_newNameOldNameMapping["GRAD"] = "GRADIAN";
    m_newNameOldNameMapping["SQ.M"] = "METRE_SQUARED";
    m_newNameOldNameMapping["SQ.MM"] = "MILLIMETRE_SQUARED";
    m_newNameOldNameMapping["SQ.CM"] = "CENTIMETRE_SQUARED";
    m_newNameOldNameMapping["SQ.KM"] = "KILOMETRE_SQUARED";
    m_newNameOldNameMapping["SQ.IN"] = "INCH_SQUARED";
    m_newNameOldNameMapping["SQ.FT"] = "FOOT_SQUARED";
    m_newNameOldNameMapping["SQ.YRD"] = "YARD_SQUARED";
    m_newNameOldNameMapping["SQ.MILE"] = "MILE_SQUARED";
    m_newNameOldNameMapping["KG/CUB.M"] = "KILOGRAM_PER_METRE_CUBED";
    m_newNameOldNameMapping["KG/CUB.CM"] = "KILOGRAM_PER_CENTIMETRE_CUBED";
    m_newNameOldNameMapping["KG/LITRE"] = "KILOGRAM_PER_LITRE";
    m_newNameOldNameMapping["G/CUB.CM"] = "GRAM_PER_CENTIMETRE_CUBED";
    m_newNameOldNameMapping["MKG/LITRE"] = "MICROGRAM_PER_LITRE";
    m_newNameOldNameMapping["MG/LITRE"] = "MILLIGRAM_PER_LITRE";
    m_newNameOldNameMapping["LBM/CUB.FT"] = "POUND_PER_FOOT_CUBED";
    m_newNameOldNameMapping["LBM/GAL"] = "POUND_PER_GALLON";
    m_newNameOldNameMapping["LBM/GALLON_IMPERIAL"] = "POUND_PER_IMPERIAL_GALLON";
    m_newNameOldNameMapping["LBM/CUB.IN"] = "POUND_PER_INCH_CUBED";
    m_newNameOldNameMapping["LBM/MILLION_GALLON"] = "POUND_PER_MILLION_GALLON";
    m_newNameOldNameMapping["SLUG/CUB.FT"] = "SLUG_PER_FOOT_CUBED";
    m_newNameOldNameMapping["KIP/CUB.FT"] = "KIP_PER_FOOT_CUBED";
    m_newNameOldNameMapping["SHORT_TON/CUB.FT"] = "SHORT_TON_PER_FOOT_CUBED";
    m_newNameOldNameMapping["N/CUB.M"] = "NEWTON_PER_METRE_CUBED";
    m_newNameOldNameMapping["KN/CUB.M"] = "KILONEWTON_PER_METRE_CUBED";
    m_newNameOldNameMapping["KN/CUB.FT"] = "KILONEWTON_PER_FOOT_CUBED";
    m_newNameOldNameMapping["PERSON/SQ.M"] = "PERSON_PER_METRE_SQUARED";
    m_newNameOldNameMapping["PERSON/HECTARE"] = "PERSON_PER_HECTARE";
    m_newNameOldNameMapping["PERSON/SQ.KM"] = "PERSON_PER_KILOMETRE_SQUARED";
    m_newNameOldNameMapping["PERSON/ACRE"] = "PERSON_PER_ACRE";
    m_newNameOldNameMapping["PERSON/SQ.FT"] = "PERSON_PER_FOOT_SQUARED";
    m_newNameOldNameMapping["PERSON/SQ.MILE"] = "PERSON_PER_MILE_SQUARED";
    m_newNameOldNameMapping["J"] = "JOULE";
    m_newNameOldNameMapping["KJ"] = "KILOJOULE";
    m_newNameOldNameMapping["MJ"] = "MEGAJOULE";
    m_newNameOldNameMapping["GJ"] = "GIGAJOULE";
    m_newNameOldNameMapping["KWH"] = "KILOWATT_HOUR";
    m_newNameOldNameMapping["MWH"] = "MEGAWATT_HOUR";
    m_newNameOldNameMapping["GWH"] = "GIGAWATT_HOUR";
    m_newNameOldNameMapping["J/CUB.M"] = "JOULE_PER_METRE_CUBED";
    m_newNameOldNameMapping["KJ/CUB.M"] = "KILOJOULE_PER_METRE_CUBED";
    m_newNameOldNameMapping["KWH/CUB.M"] = "KILOWATT_HOUR_PER_METRE_CUBED";
    m_newNameOldNameMapping["KWH/CUB.FT"] = "KILOWATT_HOUR_PER_FOOT_CUBED";
    m_newNameOldNameMapping["KWH/MILLION_GALLON"] = "KILOWATT_HOUR_PER_MILLION_GALLON";
    m_newNameOldNameMapping["KJ/KG"] = "KILOJOULE_PER_KILOGRAM";
    m_newNameOldNameMapping["MEGAJ/KG"] = "MEGAJOULE_PER_KILOGRAM";
    m_newNameOldNameMapping["BTU/LBM"] = "BTU_PER_POUND_MASS";
    m_newNameOldNameMapping["J/(KG*K)"] = "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["BTU/(LBM*RANKINE)"] = "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE";
    m_newNameOldNameMapping["J/(KMOL*K)"] = "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["KJ/(KMOL*K)"] = "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["BTU/(LB-MOLE*RANKINE)"] = "BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE";
    m_newNameOldNameMapping["CUB.M/SEC"] = "METRE_CUBED_PER_SECOND";
    m_newNameOldNameMapping["CUB.M/MIN"] = "METRE_CUBED_PER_MINUTE";
    m_newNameOldNameMapping["CUB.M/HR"] = "METRE_CUBED_PER_HOUR";
    m_newNameOldNameMapping["CUB.M/DAY"] = "METRE_CUBED_PER_DAY";
    m_newNameOldNameMapping["LITRE/SEC"] = "LITRE_PER_SECOND";
    m_newNameOldNameMapping["LITRE/MIN"] = "LITRE_PER_MINUTE";
    m_newNameOldNameMapping["LITRE/HR"] = "LITRE_PER_HOUR";
    m_newNameOldNameMapping["LITRE/DAY"] = "LITRE_PER_DAY";
    m_newNameOldNameMapping["CUB.FT/SEC"] = "FOOT_CUBED_PER_SECOND";
    m_newNameOldNameMapping["CUB.FT/MIN"] = "FOOT_CUBED_PER_MINUTE";
    m_newNameOldNameMapping["CUB.FT/DAY"] = "FOOT_CUBED_PER_DAY";
    m_newNameOldNameMapping["ACRE_FOOT/DAY"] = "ACRE_FOOT_PER_DAY";
    m_newNameOldNameMapping["ACRE_FOOT/HR"] = "ACRE_FOOT_PER_HOUR";
    m_newNameOldNameMapping["ACRE_FOOT/MIN"] = "ACRE_FOOT_PER_MINUTE";
    m_newNameOldNameMapping["ACRE_INCH/HOUR"] = "ACRE_INCH_PER_HOUR";
    m_newNameOldNameMapping["ACRE_INCH/MIN"] = "ACRE_INCH_PER_MINUTE";
    m_newNameOldNameMapping["GALLON_IMPERIAL/DAY"] = "GALLON_IMPERIAL_PER_DAY";
    m_newNameOldNameMapping["GALLON_IMPERIAL/MINUTE"] = "GALLON_IMPERIAL_PER_MINUTE";
    m_newNameOldNameMapping["GALLON_IMPERIAL/SECOND"] = "GALLON_IMPERIAL_PER_SECOND";
    m_newNameOldNameMapping["GALLON/S"] = "GALLON_PER_SECOND";
    m_newNameOldNameMapping["GALLON/MIN"] = "GALLON_PER_MINUTE";
    m_newNameOldNameMapping["GALLON/DAY"] = "GALLON_PER_DAY";
    m_newNameOldNameMapping["CUB.M/(DAY*SQ.M)"] = "METRE_CUBED_PER_METRE_SQUARED_PER_DAY";
    m_newNameOldNameMapping["CUB.M/(DAY*HECTARE)"] = "METRE_CUBED_PER_HECTARE_PER_DAY";
    m_newNameOldNameMapping["CUB.M/(DAY*SQ.KM)"] = "METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY";
    m_newNameOldNameMapping["CUB.FT/(SQ.FT*MIN)"] = "FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE";
    m_newNameOldNameMapping["CUB.FT/(SQ.FT*S)"] = "FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND";
    m_newNameOldNameMapping["CUB.FT/(SQ.MILE*S)"] = "FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND";
    m_newNameOldNameMapping["GALLON/(DAY*ACRE)"] = "GALLON_PER_ACRE_PER_DAY";
    m_newNameOldNameMapping["GALLON/(MIN*ACRE)"] = "GALLON_PER_ACRE_PER_MINUTE";
    m_newNameOldNameMapping["GALLON/(MIN*SQ.FT)"] = "GALLON_PER_FOOT_SQUARED_PER_MINUTE";
    m_newNameOldNameMapping["GALLON/(DAY*SQ.FT)"] = "GALLON_PER_FOOT_SQUARED_PER_DAY";
    m_newNameOldNameMapping["GALLON/(MIN*SQ.MILE)"] = "GALLON_PER_MILE_SQUARED_PER_MINUTE";
    m_newNameOldNameMapping["GALLON/(DAY*SQ.MILE)"] = "GALLON_PER_MILE_SQUARED_PER_DAY";
    m_newNameOldNameMapping["CUB.FT/(ACRE*SEC)"] = "FOOT_CUBED_PER_ACRE_PER_SECOND";
    m_newNameOldNameMapping["KG/S"] = "KILOGRAM_PER_SECOND";
    m_newNameOldNameMapping["KG/MIN"] = "KILOGRAM_PER_MINUTE";
    m_newNameOldNameMapping["KG/HR"] = "KILOGRAM_PER_HOUR";
    m_newNameOldNameMapping["KG/DAY"] = "KILOGRAM_PER_DAY";
    m_newNameOldNameMapping["G/S"] = "GRAM_PER_SECOND";
    m_newNameOldNameMapping["G/MIN"] = "GRAM_PER_MINUTE";
    m_newNameOldNameMapping["G/HR"] = "GRAM_PER_HOUR";
    m_newNameOldNameMapping["MG/S"] = "MILLIGRAM_PER_SECOND";
    m_newNameOldNameMapping["MG/MIN"] = "MILLIGRAM_PER_MINUTE";
    m_newNameOldNameMapping["MG/HR"] = "MILLIGRAM_PER_HOUR";
    m_newNameOldNameMapping["MG/DAY"] = "MILLIGRAM_PER_DAY";
    m_newNameOldNameMapping["MKG/S"] = "MICROGRAM_PER_SECOND";
    m_newNameOldNameMapping["MKG/MIN"] = "MICROGRAM_PER_MINUTE";
    m_newNameOldNameMapping["MKG/HR"] = "MICROGRAM_PER_HOUR";
    m_newNameOldNameMapping["MKG/DAY"] = "MICROGRAM_PER_DAY";
    m_newNameOldNameMapping["LB/S"] = "POUND_PER_SECOND";
    m_newNameOldNameMapping["LB/MIN"] = "POUND_PER_MINUTE";
    m_newNameOldNameMapping["LB/HR"] = "POUND_PER_HOUR";
    m_newNameOldNameMapping["LB/DAY"] = "POUND_PER_DAY";
    m_newNameOldNameMapping["TONNE/HR"] = "TONNE_PER_HOUR";
    m_newNameOldNameMapping["SHORT_TON/HR"] = "SHORT_TON_PER_HOUR";
    m_newNameOldNameMapping["MOL/S"] = "MOLE_PER_SECOND";
    m_newNameOldNameMapping["KMOL/S"] = "KILOMOLE_PER_SECOND";
    m_newNameOldNameMapping["N"] = "NEWTON";
    m_newNameOldNameMapping["KN"] = "KILONEWTON";
    m_newNameOldNameMapping["mN"] = "MILLINEWTON";
    m_newNameOldNameMapping["KGF"] = "KILOGRAM_FORCE";
    m_newNameOldNameMapping["LBF"] = "POUND_FORCE";
    m_newNameOldNameMapping["KPF"] = "KILOPOUND_FORCE";
    m_newNameOldNameMapping["W/(SQ.M*K)"] = "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["W/(SQ.M*CELSIUS)"] = "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS";
    m_newNameOldNameMapping["BTU/(SQ.FT*HR*FAHRENHEIT)"] = "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT";
    m_newNameOldNameMapping["KG/M"] = "KILOGRAM_PER_METRE";
    m_newNameOldNameMapping["KG/MM"] = "KILOGRAM_PER_MILLIMETRE";
    m_newNameOldNameMapping["LBM/FT"] = "POUND_MASS_PER_FOOT";
    m_newNameOldNameMapping["N/M"] = "NEWTON_PER_METRE";
    m_newNameOldNameMapping["N/MM"] = "NEWTON_PER_MILLIMETRE";
    m_newNameOldNameMapping["LBF/IN"] = "POUND_FORCE_PER_INCH";
    m_newNameOldNameMapping["PER_M"] = "ONE_PER_METRE";
    m_newNameOldNameMapping["PER_MM"] = "ONE_PER_MILLIMETRE";
    m_newNameOldNameMapping["PER_KM"] = "ONE_PER_KILOMETRE";
    m_newNameOldNameMapping["PER_FT"] = "ONE_PER_FOOT";
    m_newNameOldNameMapping["PER_MILE"] = "ONE_PER_MILE";
    m_newNameOldNameMapping["PER_THOUSAND_FT"] = "ONE_PER_THOUSAND_FOOT";
    m_newNameOldNameMapping["N_M"] = "NEWTON_METRE";
    m_newNameOldNameMapping["LBF_FT"] = "POUND_FOOT";
    m_newNameOldNameMapping["CUB.M/MOLE"] = "METRE_CUBED_PER_MOLE";
    m_newNameOldNameMapping["CUB.M/KMOL"] = "METRE_CUBED_PER_KILOMOLE";
    m_newNameOldNameMapping["CUB.FT/LB-MOLE"] = "FOOT_CUBED_PER_POUND_MOLE";
    m_newNameOldNameMapping["MOL/CUB.M"] = "MOLE_PER_METRE_CUBED";
    m_newNameOldNameMapping["KMOL/CUB.M"] = "KILOMOLE_PER_METRE_CUBED";
    m_newNameOldNameMapping["M^4"] = "METRE_TO_THE_FOURTH";
    m_newNameOldNameMapping["CM^4"] = "CENTIMETRE_TO_THE_FOURTH";
    m_newNameOldNameMapping["IN^4"] = "INCH_TO_THE_FOURTH";
    m_newNameOldNameMapping["FT^4"] = "FOOT_TO_THE_FOURTH";
    m_newNameOldNameMapping["MM^4"] = "MILLIMETRE_TO_THE_FOURTH";
    m_newNameOldNameMapping["W"] = "WATT";
    m_newNameOldNameMapping["KW"] = "KILOWATT";
    m_newNameOldNameMapping["MW"] = "MEGAWATT";
    m_newNameOldNameMapping["GW"] = "GIGAWATT";
    m_newNameOldNameMapping["BTU/MONTH"] = "BTU_PER_MONTH";
    m_newNameOldNameMapping["BTU/HOUR"] = "BTU_PER_HOUR";
    m_newNameOldNameMapping["KILOBTU/HOUR"] = "KILOBTU_PER_HOUR";
    m_newNameOldNameMapping["HP"] = "HORSEPOWER";
    m_newNameOldNameMapping["GJ/MONTH"] = "GIGAJOULE_PER_MONTH";
    m_newNameOldNameMapping["PA"] = "NEWTON_PER_METRE_SQUARED";
    m_newNameOldNameMapping["N/SQ.MM"] = "NEWTON_PER_MILLIMETRE_SQUARED";
    m_newNameOldNameMapping["AT"] = "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED";
    m_newNameOldNameMapping["AT_GAUGE"] = "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE";
    m_newNameOldNameMapping["KGF/SQ.M"] = "KILOGRAM_FORCE_PER_METRE_SQUARED";
    m_newNameOldNameMapping["ATM"] = "ATMOSPHERE";
    m_newNameOldNameMapping["MBAR"] = "MILLIBAR";
    m_newNameOldNameMapping["PSI"] = "POUND_FORCE_PER_INCH_SQUARED";
    m_newNameOldNameMapping["PSIG"] = "POUND_FORCE_PER_INCH_SQUARED_GAUGE";
    m_newNameOldNameMapping["LBF/SQ.FT"] = "POUND_FORCE_PER_FOOT_SQUARED";
    m_newNameOldNameMapping["PA/M"] = "PASCAL_PER_METRE";
    m_newNameOldNameMapping["BAR/KM"] = "BAR_PER_KILOMETRE";
    m_newNameOldNameMapping["PERCENT"] = "PERCENT_PERCENT";
    m_newNameOldNameMapping["DECIMAL_PERCENT"] = "UNITLESS_PERCENT";
    m_newNameOldNameMapping["M/M"] = "METRE_VERTICAL_PER_METRE_HORIZONTAL";
    m_newNameOldNameMapping["CM/M"] = "CENTIMETRE_PER_METRE";
    m_newNameOldNameMapping["MM/M"] = "MILLIMETRE_PER_METRE";
    m_newNameOldNameMapping["M/KM"] = "METRE_PER_KILOMETRE";
    m_newNameOldNameMapping["FT/FT"] = "FOOT_VERTICAL_PER_FOOT_HORIZONTAL";
    m_newNameOldNameMapping["IN/FT"] = "INCH_PER_FOOT";
    m_newNameOldNameMapping["FT/IN"] = "FOOT_PER_INCH";
    m_newNameOldNameMapping["FT/MILE"] = "FOOT_PER_MILE";
    m_newNameOldNameMapping["KG/SQ.M"] = "KILOGRAM_PER_METRE_SQUARED";
    m_newNameOldNameMapping["G/SQ.M"] = "GRAM_PER_METRE_SQUARED";
    m_newNameOldNameMapping["KG/HECTARE"] = "KILOGRAM_PER_HECTARE";
    m_newNameOldNameMapping["LB/ACRE"] = "POUND_PER_ACRE";
    m_newNameOldNameMapping["W/(M*K)"] = "WATT_PER_METRE_PER_DEGREE_KELVIN";
    m_newNameOldNameMapping["W/(M*C)"] = "WATT_PER_METRE_PER_DEGREE_CELSIUS";
    m_newNameOldNameMapping["(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)"] = "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT";
    m_newNameOldNameMapping["(SQ.M*KELVIN)/WATT"] = "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT";
    m_newNameOldNameMapping["(SQ.M*CELSIUS)/WATT"] = "METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT";
    m_newNameOldNameMapping["(SQ.FT*HR*FAHRENHEIT)/BTU"] = "FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU";
    m_newNameOldNameMapping["M/REVOLUTION"] = "METRE_PER_REVOLUTION";
    m_newNameOldNameMapping["MM/REVOLUTION"] = "MILLIMETRE_PER_REVOLUTION";
    m_newNameOldNameMapping["M/RAD"] = "METRE_PER_RADIAN";
    m_newNameOldNameMapping["M/DEGREE"] = "METRE_PER_DEGREE";
    m_newNameOldNameMapping["MM/RAD"] = "MILLIMETRE_PER_RADIAN";
    m_newNameOldNameMapping["IN/REVOLUTION"] = "INCH_PER_REVOLUTION";
    m_newNameOldNameMapping["FT/REVOLUTION"] = "FOOT_PER_REVOLUTION";
    m_newNameOldNameMapping["INCH/DEGREE"] = "INCH_PER_DEGREE";
    m_newNameOldNameMapping["INCH/RAD"] = "INCH_PER_RADIAN";
    m_newNameOldNameMapping["M/S"] = "METRE_PER_SECOND";
    m_newNameOldNameMapping["M/MIN"] = "METRE_PER_MINUTE";
    m_newNameOldNameMapping["M/HR"] = "METRE_PER_HOUR";
    m_newNameOldNameMapping["M/DAy"] = "METRE_PER_DAY";
    m_newNameOldNameMapping["MM/MIN"] = "MILLIMETRE_PER_MINUTE";
    m_newNameOldNameMapping["MM/HR"] = "MILLIMETRE_PER_HOUR";
    m_newNameOldNameMapping["MM/DAY"] = "MILLIMETRE_PER_DAY";
    m_newNameOldNameMapping["CM/S"] = "CENTIMETRE_PER_SECOND";
    m_newNameOldNameMapping["CM/MIN"] = "CENTIMETRE_PER_MINUTE";
    m_newNameOldNameMapping["CM/HOUR"] = "CENTIMETRE_PER_HOUR";
    m_newNameOldNameMapping["CM/DAY"] = "CENTIMETRE_PER_DAY";
    m_newNameOldNameMapping["KM/HR"] = "KILOMETRE_PER_HOUR";
    m_newNameOldNameMapping["IN/SEC"] = "INCH_PER_SECOND";
    m_newNameOldNameMapping["IN/MIN"] = "INCH_PER_MINUTE";
    m_newNameOldNameMapping["IN/HR"] = "INCH_PER_HOUR";
    m_newNameOldNameMapping["IN/DAY"] = "INCH_PER_DAY";
    m_newNameOldNameMapping["FT/SEC"] = "FOOT_PER_SECOND";
    m_newNameOldNameMapping["FT/MIN"] = "FOOT_PER_MINUTE";
    m_newNameOldNameMapping["FT/HR"] = "FOOT_PER_HOUR";
    m_newNameOldNameMapping["FT/DAY"] = "FOOT_PER_DAY";
    m_newNameOldNameMapping["MPH"] = "MILE_PER_HOUR";
    m_newNameOldNameMapping["KNOT_UK_ADMIRALTY"] = "KNOT";    // NOTE: OldSystem used admiralty knot conversion factor for knot conversion factor
    m_newNameOldNameMapping["KNOT_INTERNATIONAL"] = "KNOT_INTERNATIONAL";
    m_newNameOldNameMapping["RAD/S"] = "RADIAN_PER_SECOND";
    m_newNameOldNameMapping["RAD/MIN"] = "RADIAN_PER_MINUTE";
    m_newNameOldNameMapping["RAD/HR"] = "RADIAN_PER_HOUR";
    m_newNameOldNameMapping["RPS"] = "CYCLE_PER_SECOND";
    m_newNameOldNameMapping["RPM"] = "REVOLUTION_PER_MINUTE";
    m_newNameOldNameMapping["RPH"] = "CYCLE_PER_HOUR";
    m_newNameOldNameMapping["DEG/S"] = "DEGREE_PER_SECOND";
    m_newNameOldNameMapping["DEG/MIN"] = "DEGREE_PER_MINUTE";
    m_newNameOldNameMapping["DEG/HR"] = "DEGREE_PER_HOUR";
    m_newNameOldNameMapping["PA-S"] = "PASCAL_SECOND";
    m_newNameOldNameMapping["SQ.M/S"] = "METRE_SQUARED_PER_SECOND";
    m_newNameOldNameMapping["SQ.FT/S"] = "FOOT_SQUARED_PER_SECOND";
    m_newNameOldNameMapping["CUB.M"] = "METRE_CUBED";
    m_newNameOldNameMapping["CUB.CM"] = "CENTIMETRE_CUBED";
    m_newNameOldNameMapping["CUB.IN"] = "INCH_CUBED";
    m_newNameOldNameMapping["CUB.FT"] = "FOOT_CUBED";
    m_newNameOldNameMapping["CUB.YRD"] = "YARD_CUBED";
    m_newNameOldNameMapping["CUB.M/KG"] = "METRE_CUBED_PER_KILOGRAM";
    m_newNameOldNameMapping["CUB.FT/LB"] = "FOOT_CUBED_PER_POUND_MASS";
    m_newNameOldNameMapping["M^6"] = "METRE_TO_THE_SIXTH";
    m_newNameOldNameMapping["MM^6"] = "MILLIMETRE_TO_THE_SIXTH";
    m_newNameOldNameMapping["CM^6"] = "CENTIMETRE_TO_THE_SIXTH";
    m_newNameOldNameMapping["IN^6"] = "INCH_TO_THE_SIXTH";
    m_newNameOldNameMapping["FT^6"] = "FOOT_TO_THE_SIXTH";
    m_newNameOldNameMapping["M"] = "METRE";
    m_newNameOldNameMapping["KG"] = "KILOGRAM";
    m_newNameOldNameMapping["S"] = "SECOND";
    m_newNameOldNameMapping["K"] = "DEGREE_KELVIN";
    m_newNameOldNameMapping["DELTA_KELVIN"] = "DELTA_DEGREE_KELVIN";
    m_newNameOldNameMapping["A"] = "AMPERE";
    m_newNameOldNameMapping["RAD"] = "RADIAN";
    m_newNameOldNameMapping["US$"] = "DOLLAR";
    m_newNameOldNameMapping["ONE"] = "UNITLESS_UNIT";
    m_newNameOldNameMapping["THOUSAND_SQ.FT"] = "THOUSAND_FOOT_SQUARED";
    m_newNameOldNameMapping["DYNE"] = "DYNE";
    m_newNameOldNameMapping["FOOT_POUNDAL"] = "FOOT_POUNDAL";
    m_newNameOldNameMapping["KILOPASCAL_GAUGE"] = "KILOPASCAL_GAUGE";
    m_newNameOldNameMapping["PERCENT_SLOPE"] = "PERCENT_SLOPE";
    m_newNameOldNameMapping["MILE"] = "MILE";
    m_newNameOldNameMapping["MICROINCH"] = "MICROINCH";
    m_newNameOldNameMapping["MILLIINCH"] = "MILLIINCH";
    m_newNameOldNameMapping["MILLIFOOT"] = "MILLIFOOT";
    m_newNameOldNameMapping["US_SURVEY_INCH"] = "US_SURVEY_INCH";
    m_newNameOldNameMapping["US_SURVEY_FOOT"] = "US_SURVEY_FOOT";
    m_newNameOldNameMapping["US_SURVEY_MILE"] = "US_SURVEY_MILE";
    m_newNameOldNameMapping["HECTARE"] = "HECTARE";
    m_newNameOldNameMapping["ACRE"] = "ACRE";
    m_newNameOldNameMapping["INCH_MILE"] = "INCH_MILE";
    m_newNameOldNameMapping["INCH_FOOT"] = "INCH_FOOT";
    m_newNameOldNameMapping["FOOT_MILE"] = "FOOT_MILE";
    m_newNameOldNameMapping["FOOT_FOOT"] = "FOOT_FOOT";
    m_newNameOldNameMapping["MILLIMETRE_METRE"] = "MILLIMETRE_METRE";
    m_newNameOldNameMapping["MILLIMETRE_KILOMETRE"] = "MILLIMETRE_KILOMETRE";
    m_newNameOldNameMapping["METRE_METRE"] = "METRE_METRE";
    m_newNameOldNameMapping["METRE_KILOMETRE"] = "METRE_KILOMETRE";
    m_newNameOldNameMapping["INCH_METRE"] = "INCH_METRE";
    m_newNameOldNameMapping["MILLIMETRE_MILE"] = "MILLIMETRE_MILE";
    m_newNameOldNameMapping["ACRE_FOOT"] = "ACRE_FOOT";
    m_newNameOldNameMapping["ACRE_INCH"] = "ACRE_INCH";
    m_newNameOldNameMapping["GALLON"] = "GALLON";
    m_newNameOldNameMapping["GALLON_IMPERIAL"] = "GALLON_IMPERIAL";
    m_newNameOldNameMapping["LITRE"] = "LITRE";
    m_newNameOldNameMapping["MILLION_GALLON"] = "MILLION_GALLON";
    m_newNameOldNameMapping["MILLION_LITRE"] = "MILLION_LITRE";
    m_newNameOldNameMapping["THOUSAND_GALLON"] = "THOUSAND_GALLON";
    m_newNameOldNameMapping["THOUSAND_LITRE"] = "THOUSAND_LITRE";
    m_newNameOldNameMapping["GRAIN"] = "GRAIN";
    m_newNameOldNameMapping["LONG_TON"] = "LONG_TON";
    m_newNameOldNameMapping["MEGAGRAM"] = "MEGAGRAM";
    m_newNameOldNameMapping["SHORT_TON"] = "SHORT_TON";
    m_newNameOldNameMapping["DAY"] = "DAY";
    m_newNameOldNameMapping["REVOLUTION"] = "REVOLUTION";
    m_newNameOldNameMapping["CENTISTOKE"] = "CENTISTOKE";
    m_newNameOldNameMapping["STOKE"] = "STOKE";
    m_newNameOldNameMapping["LONG_TON_FORCE"] = "LONG_TON_FORCE";
    m_newNameOldNameMapping["SHORT_TON_FORCE"] = "SHORT_TON_FORCE";
    m_newNameOldNameMapping["BTU"] = "BTU";
    m_newNameOldNameMapping["KILOBTU"] = "KILOBTU";
    m_newNameOldNameMapping["WATT_SECOND"] = "WATT_SECOND";
    m_newNameOldNameMapping["KILOVOLT"] = "KILOVOLT";
    m_newNameOldNameMapping["VOLT"] = "VOLT";
    m_newNameOldNameMapping["MEGAVOLT"] = "MEGAVOLT";
    m_newNameOldNameMapping["CAPITA"] = "CAPITA";
    m_newNameOldNameMapping["PERSON"] = "PERSON";
    m_newNameOldNameMapping["CUSTOMER"] = "CUSTOMER";
    m_newNameOldNameMapping["EMPLOYEE"] = "EMPLOYEE";
    m_newNameOldNameMapping["GUEST"] = "GUEST";
    m_newNameOldNameMapping["HUNDRED_CAPITA"] = "HUNDRED_CAPITA";
    m_newNameOldNameMapping["THOUSAND_CAPITA"] = "THOUSAND_CAPITA";
    m_newNameOldNameMapping["PASSENGER"] = "PASSENGER";
    m_newNameOldNameMapping["RESIDENT"] = "RESIDENT";
    m_newNameOldNameMapping["STUDENT"] = "STUDENT";
    m_newNameOldNameMapping["VERTICAL_PER_HORIZONTAL"] = "VERTICAL_PER_HORIZONTAL";
    m_newNameOldNameMapping["FOOT_HORIZONTAL_PER_FOOT_VERTICAL"] = "FOOT_HORIZONTAL_PER_FOOT_VERTICAL";
    m_newNameOldNameMapping["METRE_HORIZONTAL_PER_METRE_VERTICAL"] = "METRE_HORIZONTAL_PER_METRE_VERTICAL";
    m_newNameOldNameMapping["HORIZONTAL_PER_VERTICAL"] = "HORIZONTAL_PER_VERTICAL";
    m_newNameOldNameMapping["FOOT_PER_1000_FOOT"] = "FOOT_PER_1000_FOOT";
    m_newNameOldNameMapping["CENTIPOISE"] = "CENTIPOISE";
    m_newNameOldNameMapping["LUX"] = "LUX";
    m_newNameOldNameMapping["KILOAMPERE"] = "KILOAMPERE";
    m_newNameOldNameMapping["LUMEN"] = "LUMEN";
    m_newNameOldNameMapping["KILOPASCAL"] = "KILOPASCAL";
    m_newNameOldNameMapping["BAR"] = "BAR";
    m_newNameOldNameMapping["BAR_GAUGE"] = "BAR_GAUGE";
    m_newNameOldNameMapping["BARYE"] = "BARYE";
    m_newNameOldNameMapping["FOOT_OF_H2O_CONVENTIONAL"] = "FOOT_OF_H2O_CONVENTIONAL";
    m_newNameOldNameMapping["HECTOPASCAL"] = "HECTOPASCAL";
    m_newNameOldNameMapping["INCH_OF_H2O_AT_32_FAHRENHEIT"] = "INCH_OF_H2O_AT_32_FAHRENHEIT";
    m_newNameOldNameMapping["INCH_OF_H2O_AT_39_2_FAHRENHEIT"] = "INCH_OF_H2O_AT_39_2_FAHRENHEIT";
    m_newNameOldNameMapping["INCH_OF_H2O_AT_60_FAHRENHEIT"] = "INCH_OF_H2O_AT_60_FAHRENHEIT";
    m_newNameOldNameMapping["INCH_OF_HG_AT_32_FAHRENHEIT"] = "INCH_OF_HG_AT_32_FAHRENHEIT";
    m_newNameOldNameMapping["INCH_OF_HG_AT_60_FAHRENHEIT"] = "INCH_OF_HG_AT_60_FAHRENHEIT";
    m_newNameOldNameMapping["INCH_OF_HG_CONVENTIONAL"] = "INCH_OF_HG_CONVENTIONAL";
    m_newNameOldNameMapping["MEGAPASCAL"] = "MEGAPASCAL";
    m_newNameOldNameMapping["MEGAPASCAL_GAUGE"] = "MEGAPASCAL_GAUGE";
    m_newNameOldNameMapping["METRE_OF_H2O_CONVENTIONAL"] = "METRE_OF_H2O_CONVENTIONAL";
    m_newNameOldNameMapping["MILLIMETRE_OF_H2O_CONVENTIONAL"] = "MILLIMETRE_OF_H2O_CONVENTIONAL";
    m_newNameOldNameMapping["MILLIMETRE_OF_HG_AT_32_FAHRENHEIT"] = "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT";
    m_newNameOldNameMapping["TORR"] = "TORR";
    m_newNameOldNameMapping["HERTZ"] = "HERTZ";

    }
