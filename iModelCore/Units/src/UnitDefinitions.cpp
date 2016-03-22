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
    reg.AddUnit(LENGTH, USCUSTOM, "MILLIFOOT", "[MILLI]*FT");

    reg.AddUnit(LENGTH, USCUSTOM, "IN", "MM", 25.4); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(LENGTH, USCUSTOM, "FT", "IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USCUSTOM, "YRD", "FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USCUSTOM, "CHAIN", "FT", 66.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    reg.AddUnit(LENGTH, USCUSTOM, "MILE", "YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_IN", "M", 100.0 / 3937.0); // Derived from the definition of us survey foot in terms of meters.  Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_FT", "US_SURVEY_IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_YRD", "US_SURVEY_FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_CHAIN", "US_SURVEY_FT", 66.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_MILE", "US_SURVEY_YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    reg.AddUnit(LENGTH, MARITIME, "NAUT_MILE", "M", 1852.0); // International Nautical Mile.  Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddSynonym("NAUT_MILE", "NAUTICAL_MILE_INTERNATIONAL");
    
    // NOTE: An Admiralty mile was defined as 1853.184 Meters, then redefined as 1853.0 by UK law see schedule 1995 No. 1804, can be found at legislation.gov.uk, search by year and number.
    //reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE_IMPERIAL", "M", 1853.0);
    //reg.AddSynonym("NAUT_MILE_IMPERIAL", "ADMIRALTY_MILE");

    //reg.AddUnit(LENGTH, SI, "ANGSTROM", "M", 1e-10);
    //reg.AddUnit(LENGTH, SI, "FERMI", "[FEMTO]*M");
    //reg.AddSynonym("FERMI", "FEMTOMETRE");
    //reg.AddUnit(LENGTH, IMPERIAL, "BARLEYCORN", "IN", (1.0 / 3.0));
    //reg.AddUnit(LENGTH, HISTORICAL, "CUBIT", "IN", 18.0);
    //reg.AddUnit(LENGTH, HISTORICAL, "ELL", "IN", 45.0);
    //reg.AddUnit(LENGTH, HISTORICAL, "FATHOM", "FT", 6.0);
    //reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_SEC", "[C]*S");
    //reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_MIN", "[C]*MIN");
    //reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_HR", "[C]*HR");
    //reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_YEAR", "[C]*YR");
    //reg.AddUnit(LENGTH, ASTRONOMY, "AU", "M", 1.495978707e11);
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

    reg.AddUnit(MASS, USCUSTOM, "LBM", "KG", 0.45359237); // Is Avoirdupois Pound.  Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Footnote 22
    reg.AddSynonym("LBM", "POUND_MASS");
    reg.AddUnit(MASS, USCUSTOM, "SLUG", "LBF*S(2)*FT(-1)");
    reg.AddUnit(MASS, USCUSTOM, "GRM", "LBM", 1.0 / 7000.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix B. Section 3.2, Page B-10
    reg.AddSynonym("GRM", "GRAIN_MASS");

    reg.AddUnit(MASS, USCUSTOM, "SHORT_TON_MASS", "LBM", 2000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddSynonym("SHORT_TON_MASS", "SHORT_TON");
    reg.AddUnit(MASS, USCUSTOM, "LONG_TON_MASS", "LBM", 2240); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddSynonym("LONG_TON_MASS", "LONG_TON");
    reg.AddUnit(MASS, USCUSTOM, "KIPM", "LBM", 1000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    }

void AddTime(UnitRegistry& reg)
    {
    reg.AddUnit(TIME, INTERNATIONAL, "MIN", "S", 60.0);
    reg.AddUnit(TIME, INTERNATIONAL, "HR", "MIN", 60.0);
    reg.AddUnit(TIME, INTERNATIONAL, "DAY", "HR", 24.0);
    reg.AddUnit(TIME, INTERNATIONAL, "WEEK", "DAY", 7.0);
    //reg.AddUnit(TIME, SI, "MONTH", "DAY", 30.0); // TODO: No standard definition of month
    reg.AddUnit(TIME, INTERNATIONAL, "YR", "DAY", 365);  //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Year is 3.1536 E+07 seconds which is equal to 365 * 24 * 60 * 60
    reg.AddUnit(TIME, INTERNATIONAL, "YEAR_SIDEREAL", "S", 3.155815e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, INTERNATIONAL, "YEAR_TROPICAL", "S", 3.155693e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, INTERNATIONAL, "MS", "[MILLI]*S");
    reg.AddUnit(TIME, INTERNATIONAL, "MKS", "[MICRO]*S");
    reg.AddSynonym("MKS", "MICROSECOND");

    }

void AddTemperature(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE, SI, "CELSIUS", "K", 1.0, 273.15);
    reg.AddUnit(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "CELSIUS", 5.0 / 9.0, -32);
    reg.AddUnit(TEMPERATURE, USCUSTOM, "RANKINE", "K", 5.0 / 9.0);

    //reg.AddUnit(TEMPERATURE, USCUSTOM, "ROMER", "CELSIUS", 40.0 / 21.0, -7.5);
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
    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/KELVIN", "DELTA_KELVIN(-1)");

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
    reg.AddUnit(MOLE, USCUSTOM, "LB-MOL", "MOL", 453.59237); // ASTM SI 10 standard SI1-.phhc8328.pdf page 29, 35 and http://en.wikipedia.org/wiki/Mole_%28unit%29
    reg.AddSynonym("LB-MOL", "POUND_MOLE");
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
    reg.AddUnit(ACCELERATION, SI, "M/SEC.SQ", "M*S(-2)");
    reg.AddUnit(ACCELERATION, SI, "CM/SEC.SQ", "CM*S(-2)");
    reg.AddUnit(ACCELERATION, USCUSTOM, "FT/SEC.SQ", "FT*S(-2)");
    }

void AddPlaneAngle(UnitRegistry& reg)
    {
    reg.AddUnit(ANGLE, SI, "ARC_DEG", "[PI]*RAD", 1.0 / 180.0);
    reg.AddUnit(ANGLE, SI, "ARC_MINUTE", "ARC_DEG", 1.0 / 60.0);
    reg.AddUnit(ANGLE, SI, "ARC_SECOND", "ARC_DEG", 1.0 / 3600.0);
    reg.AddUnit(ANGLE, SI, "ARC_QUADRANT", "[PI/2]*RAD");
    reg.AddUnit(ANGLE, SI, "GRAD", "[PI]*RAD", 1.0 / 200.0);
    reg.AddUnit(ANGLE, SI, "REVOLUTION", "[2PI]*RAD");
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

    reg.AddUnit(AREA, USCUSTOM, "SQ.IN", "IN(2)");
    reg.AddUnit(AREA, USCUSTOM, "SQ.FT", "FT(2)");
    reg.AddUnit(AREA, USCUSTOM, "THOUSAND_SQ.FT", "[KILO]*FT(2)");
    reg.AddUnit(AREA, USCUSTOM, "SQ.YRD", "YRD(2)");
    reg.AddUnit(AREA, USCUSTOM, "SQ.MILE", "MILE(2)");
    reg.AddUnit(AREA, USCUSTOM, "SQ.CHAIN", "CHAIN(2)");
    reg.AddUnit(AREA, USCUSTOM, "ACRE", "CHAIN(2)", 10.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9

    reg.AddUnit(AREA, USSURVEY, "SQ.US_SURVEY_IN", "US_SURVEY_IN(2)");
    reg.AddUnit(AREA, USSURVEY, "SQ.US_SURVEY_FT", "US_SURVEY_FT(2)");
    reg.AddUnit(AREA, USSURVEY, "SQ.US_SURVEY_YRD", "US_SURVEY_YRD(2)");
    reg.AddUnit(AREA, USSURVEY, "SQ.US_SURVEY_MILE", "US_SURVEY_MILE(2)");
    reg.AddUnit(AREA, USSURVEY, "SQ.US_SURVEY_CHAIN", "US_SURVEY_CHAIN(2)");
    reg.AddUnit(AREA, USSURVEY, "US_SURVEY_ACRE", "US_SURVEY_CHAIN(2)", 10.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9
    }

void AddSizeLengthRate(UnitRegistry& reg)
    {
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "IN_MILE", "IN*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "FT_MILE", "FT*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "FT_FT", "FT*FT");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "IN_FT", "IN*FT");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "IN_M", "IN*M");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MM_KM", "MM*KM");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MM_M", "MM*M");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "MM_MILE", "MM*MILE");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "M_KM", "M*KM");
    reg.AddUnit(SIZE_LENGTH_RATE, INDUSTRIAL, "M_M", "M*M");
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
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/GALLON", "LBM*GALLON(-1)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/GALLON_IMPERIAL", "LBM*GALLON_IMPERIAL(-1)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/CUB.IN", "LBM*IN(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "LBM/MILLION_GALLON", "LBM*GALLON(-1)", 1.0e-6);

    reg.AddUnit(DENSITY, USCUSTOM, "SLUG/CUB.FT", "SLUG*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "KIP/CUB.FT", "KIPM*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "SHORT_TON/CUB.FT", "SHORT_TON_MASS*FT(-3)");
    }

void AddForceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE_DENSITY, SI, "N/CUB.M", "N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, SI, "KN/CUB.M", "[KILO]*N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "N/CUB.FT", "N*FT(-3)");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "KN/CUB.FT", "[KILO]*N*FT(-3)");
    }

void AddPopulationDensity(UnitRegistry& reg)
    {
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.M", "PERSON*M(-2)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/HECTARE", "PERSON*HECTARE(-1)");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.KM", "PERSON*KM(-2)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/ACRE", "PERSON*ACRE(-1)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/SQ.FT", "PERSON*FT(-2)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/SQ.MILE", "PERSON*MILE(-2)");
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
    reg.AddUnit(WORK, SI, "KJ", "[KILO]*J");
    reg.AddUnit(WORK, SI, "MEGAJ", "[MEGA]*J");
    reg.AddUnit(WORK, SI, "GJ", "[GIGA]*J");
    reg.AddUnit(WORK, USCUSTOM, "FT_PDL", "PDL*FT");
    reg.AddUnit(WORK, INTERNATIONAL, "BTU", "J", 1.05505585262e3); // Is IT BTU.  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.  See foot note #9: 
    reg.AddSynonym("BTU", "BRITISH_THERMAL_UNIT");
    reg.AddUnit(WORK, USCUSTOM, "KILOBTU", "[KILO]*BTU");

    //reg.AddUnit(WORK, INTERNATIONAL, "CAL", "J", 4.1868);
    //reg.AddSynonym("CAL", "CALORIE");
    //reg.AddUnit(WORK, INTERNATIONAL, "KCAL", "[KILO]*CAL");
    //reg.AddSynonym("KCAL", "KILOCALORIE");
    //reg.AddUnit(WORK, INTERNATIONAL, "CUB.FT_ATM", "ATM*CUB.FT");
    //reg.AddSynonym("CUB.FT_ATM", "CUBIC_FOOT_OF_ATMOSPHERE");
    //reg.AddUnit(WORK, INTERNATIONAL, "CUB.YRD_ATM", "ATM*CUB.YRD");
    //reg.AddSynonym("CUB.YRD_ATM", "CUBIC_YARD_OF_ATMOSPHERE");
    reg.AddUnit(WORK, INTERNATIONAL, "WATT_SECOND", "W*S");
    reg.AddUnit(WORK, INTERNATIONAL, "KWH", "KW*HR");
    reg.AddUnit(WORK, INTERNATIONAL, "MEGAWH", "MEGAW*HR");
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
    reg.AddUnit(HEATING_VALUE_MASS, SI, "MEGAJ/KG", "MEGAJ*KG(-1)");

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

    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, USCUSTOM, "BTU/(LB-MOL*RANKINE)", "BTU*LB-MOL(-1)*DELTA_RANKINE(-1)");
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

    reg.AddUnit(FLOW, USCUSTOM, "CUB.IN/SEC", "CUB.IN*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "CUB.IN/MIN", "CUB.IN*MIN(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "CUB.FT/SEC", "CUB.FT*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "CUB.FT/MIN", "CUB.FT*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "CUB.FT/DAY", "CUB.FT*DAY(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FT/DAY", "ACRE_FT*DAY(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FT/HR", "ACRE_FT*HR(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FT/MIN", "ACRE_FT*MIN(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "ACRE_IN/DAY", "ACRE_IN*DAY(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_IN/HR", "ACRE_IN*HR(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_IN/MIN", "ACRE_IN*MIN(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/SEC", "GALLON_IMPERIAL*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/MIN", "GALLON_IMPERIAL*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL*DAY(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON/SEC", "GALLON*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/MIN", "GALLON*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/DAY", "GALLON*DAY(-1)");
    }

void AddFrequency(UnitRegistry& reg)
    {
    reg.AddUnit(FREQUENCY, SI, "HZ", "S(-1)");
    reg.AddSynonym("HZ", "HERTZ");
    reg.AddUnit(FREQUENCY, SI, "KHZ", "[KILO]*S(-1)");
    reg.AddSynonym("KHZ", "KILOHERTZ");
    reg.AddUnit(FREQUENCY, SI, "MHZ", "[MEGA]*S(-1)");
    reg.AddSynonym("MHZ", "MEGAHERTZ");
    }

void AddSurfaceFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(SQ.M*SEC)", "M*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(SQ.M*DAY)", "M*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(HECTARE*DAY)", "CUB.M*HECTARE(-1)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(SQ.KM*DAY)", "CUB.M*KM(-2)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*MIN)", "FT*MIN(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*SEC)", "FT*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.MILE*SEC)", "FT(3)*MILE(-2)*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(ACRE*SEC)", "FT(3)*ACRE(-1)*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(ACRE*DAY)", "GALLON*DAY(-1)*ACRE(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(ACRE*MIN)", "GALLON*MIN(-1)*ACRE(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(SQ.FT*MIN)", "GALLON*MIN(-1)*FT(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(SQ.FT*DAY)", "GALLON*DAY(-1)*FT(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(SQ.MILE*MIN)", "GALLON*MIN(-1)*MILE(-2)");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(SQ.MILE*DAY)", "GALLON*DAY(-1)*MILE(-2)");
    }

void AddMassRatio(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_RATIO, SI, "KG/KG", "KG*KG(-1)");
    reg.AddUnit(MASS_RATIO, USCUSTOM, "GRM/LBM", "GRM*LBM(-1)");
    }

void AddMassFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_FLOW, SI, "KG/SEC", "KG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/MIN", "KG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/HR", "KG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "KG/DAY", "KG*DAY(-1)");

    reg.AddUnit(MASS_FLOW, SI, "G/SEC", "G*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "G/MIN", "G*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "G/HR", "G*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/SEC", "MG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/MIN", "MG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/HR", "MG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MG/DAY", "MG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/SEC", "MKG*S(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/MIN", "MKG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/HR", "MKG*HR(-1)");
    reg.AddUnit(MASS_FLOW, SI, "MKG/DAY", "MKG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, SI, "TONNE/HR", "TONNE*HR(-1)");

    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/SEC", "LBM*S(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/MIN", "LBM*MIN(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/HR", "LBM*HR(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/DAY", "LBM*DAY(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "SHORT_TON/HR", "SHORT_TON_MASS*HR(-1)");
    }

void AddParticleFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(PARTICLE_FLOW, SI, "MOL/SEC", "MOL*S(-1)");
    reg.AddUnit(PARTICLE_FLOW, SI, "KMOL/SEC", "[KILO]*MOL*S(-1)");
    }

void AddForce(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE, SI, "N", "KG*M*S(-2)");
    reg.AddUnit(FORCE, SI, "KN", "[KILO]*N");
    reg.AddUnit(FORCE, SI, "MN", "[MILLI]*N");
    reg.AddUnit(FORCE, SI, "KGF", "[STD_G]*KG");
    reg.AddSynonym("KGF", "KILOPOND");
    reg.AddUnit(FORCE, SI, "LBF", "[STD_G]*LBM");
    reg.AddUnit(FORCE, SI, "KPF", "[KILO]*LBF");
    reg.AddUnit(FORCE, SI, "DYNE", "G*CM*S(-2)");
    
    reg.AddUnit(FORCE, USCUSTOM, "PDL", "LBM*FT*S(-2)");
    reg.AddSynonym("PDL", "POUNDAL");
    reg.AddUnit(FORCE, USCUSTOM, "SHORT_TON_FORCE", "[STD_G]*SHORT_TON_MASS");
    reg.AddUnit(FORCE, USCUSTOM, "LONG_TON_FORCE", "[STD_G]*LONG_TON_MASS");
    reg.AddUnit(FORCE, USCUSTOM, "KIPF", "[STD_G]*KIPM");
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
    reg.AddUnit(LINEAR_LOAD, USCUSTOM, "LBF/IN", "LBF*IN(-1)");
    }

// NOTE: Don't have a user right now
//void AddLinearCost(UnitRegistry& reg)
//    {
//    reg.AddUnit(LINEAR_COST, FINANCE, "US$/M", "US$*M(-1)");
//    reg.AddUnit(LINEAR_COST, FINANCE, "US$/MM", "US$*MM(-1)");
//    }

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
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/MOL", "CUB.M*MOL(-1)");
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/KMOL", "CUB.M*MOL(-1)", 1.0e-3);
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.FT/LB-MOL", "CUB.FT*LB-MOL(-1)");
    }

void AddMolarConcentration(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.M", "MOL*CUB.M(-1)");
    reg.AddSynonym("MOL/CUB.M", "MILLIMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "KMOL/CUB.M", "[KILO]*MOL*CUB.M(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.DM", "MOL*CUB.DM(-1)");
    reg.AddSynonym("MOL/CUB.DM", "MOL/LITRE");
    reg.AddSynonym("MOL/CUB.DM", "MOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MICROMOL/CUB.DM", "[MICRO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("MICROMOL/CUB.DM", "MICROMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "NMOL/CUB.DM", "[NANO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("NMOL/CUB.DM", "NANOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "PICOMOL/CUB.DM", "[PICO]*MOL*CUB.DM(-1)");
    reg.AddSynonym("PICOMOL/CUB.DM", "PICOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.FT", "MOL*CUB.FT(-1)");
    }

// NOTE: Changed to Area moment of inertia based on Wiki pages on moment of inertia and area moment of inertia
void AddMomentOfInertia(UnitRegistry& reg)
    {
    reg.AddUnit(AREA_MOMENT_INERTIA, SI, "MM^4", "MM(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, SI, "M^4", "M(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, SI, "CM^4", "CM(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "IN^4", "IN(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "FT^4", "FT(4)");
    }

void AddPower(UnitRegistry& reg)
    {
    reg.AddUnit(POWER, INTERNATIONAL, "W", "N*M*S(-1)");
    reg.AddUnit(POWER, INTERNATIONAL, "KW", "[KILO]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "MEGAW", "[MEGA]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "GW", "[GIGA]*W");
    reg.AddUnit(POWER, INTERNATIONAL, "BTU/HR", "BTU*HR(-1)");
    reg.AddUnit(POWER, INTERNATIONAL, "KILOBTU/HR", "[KILO]*BTU*HR(-1)");
    reg.AddUnit(POWER, USCUSTOM, "HP", "LBF*FT*S(-1)", 550.0);  // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.

    // TODO: No standard definition of a month ... 
    //reg.AddUnit(POWER, INTERNATIONAL, "BTU/MONTH", "BTU*MONTH(-1)");
    //reg.AddUnit(POWER, SI, "GJ/MONTH", "GJ*MONTH(-1)");
    }

void AddPressure(UnitRegistry& reg)
    {
    reg.AddUnit(PRESSURE, SI, "PA", "N*M(-2)");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "PA_GAUGE", "PA", 1, 101325);  // Offset is one standard atmosphere in PA.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    // TODO: See if this is equal to another unit here.
    reg.AddUnit(PRESSURE, SI, "N/SQ.MM", "N*MM(-2)");

    reg.AddUnit(PRESSURE, SI, "HECTOPASCAL", "[HECTO]*PA");

    reg.AddUnit(PRESSURE, SI, "KILOPASCAL", "[KILO]*PA");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "KILOPASCAL_GAUGE", "[KILO]*PA", 1, 101325e-3);  // Offset is one standard atmosphere converted to KiloPA.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, SI, "MEGAPASCAL", "[MEGA]*PA");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "MEGAPASCAL_GAUGE", "[MEGA]*PA", 1, 101325e-6);  // Offset is one standard atmosphere converted to MegaPA.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, INTERNATIONAL, "AT", "KGF*CM(-2)");  // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddSynonym("AT", "ATMOSPHERE_TECHNIAL");

    reg.AddUnit(PRESSURE, INDUSTRIAL, "AT_GAUGE", "AT", 1.0, 1.0332274527998859); // TODO: double check, used 101325 PA -> AT conversion

    reg.AddUnit(PRESSURE, INTERNATIONAL, "KGF/SQ.M", "KGF*M(-2)");

    reg.AddUnit(PRESSURE, SI, "ATM", "PA", 101325);  // Is standard atmosphere.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, SI, "BAR", "PA", 1.0e5); // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  
    reg.AddUnit(PRESSURE, INDUSTRIAL, "BAR_GAUGE", "PA", 1.0e5, 1.01325); // Offset is one standard atmosphere converted to BAR.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, SI, "MBAR", "[MILLI]*BAR");

    reg.AddUnit(PRESSURE, CGS, "BARYE", "PA", 0.1);   // 1.0 dyn/sq.cm


    reg.AddUnit(PRESSURE, USCUSTOM, "PSI", "LBF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "PSIG", "LBF*IN(-2)", 1, 14.695948775513449); // TODO: double check, used 101325 PA -> PSI conversion

    reg.AddUnit(PRESSURE, USCUSTOM, "KSI", "KIPF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "LBF/SQ.FT", "LBF*FT(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "TORR", "PA", 101325.0 / 760.0);   // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B. for approx conversion and Table 11 for a reference to the exact conversion

    // TODO: Go back to density conversion once we have verified sources for those values
    reg.AddUnit(PRESSURE, SI, "M_H2O", "[KILO]*MM_H2O"); // Meter of H2O Conventional
    reg.AddUnit(PRESSURE, SI, "MM_H2O", "PA", 9.80665); // Millimeter of H2O Conventional, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, SI, "MM_HG@32F", "PA", 1.33322e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  Used centimeter of mercury (0 C) to pascal
    reg.AddUnit(PRESSURE, USCUSTOM, "FT_H2O", "PA", 2.989067e3); // foot of H2O Conventional, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "IN_H2O@32F", "PA", 249.1083); // Inch of H2O at 32 Fahrenheit, Water is assumed to be in a liquid state. Equal to water at 0C.  No verified source
    reg.AddUnit(PRESSURE, USCUSTOM, "IN_H2O@39.2F", "PA", 2.49082e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "IN_H2O@60F", "PA", 2.4884e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.

    reg.AddUnit(PRESSURE, USCUSTOM, "IN_HG", "PA", 3.386389e3); // Inch of HG conventional, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "IN_HG@32F", "PA", 3.38638e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, USCUSTOM, "IN_HG@60F", "PA", 3.37685e3); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
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
    reg.AddUnit(SLOPE, USCUSTOM, "FT/THOUSAND_FOOT", "FT*FT(-1)", 1.0e-3);
    reg.AddUnit(SLOPE, USCUSTOM, "FT/FT", "FT*FT(-1)");


    reg.AddUnit(SLOPE, USCUSTOM, "IN/FT", "IN*FT(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/IN", "FT*IN(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/MILE", "FT*MILE(-1)");

    reg.AddUnit(SLOPE, INTERNATIONAL, "VERTICAL/HORIZONTAL", "M/M");

    reg.AddUnit(SLOPE, INTERNATIONAL, "PERCENT_SLOPE", "M/M", 1.0e-2);

    reg.AddInvertingUnit("VERTICAL/HORIZONTAL", "HORIZONTAL/VERTICAL");
    reg.AddInvertingUnit("FT/FT", "FT_HORIZONTAL/FT_VERTICAL");
    reg.AddInvertingUnit("M/M", "M_HORIZONTAL/M_VERTICAL");
    }

void AddSurfaceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_DENSITY, SI, "KG/SQ.M", "KG*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, SI, "G/SQ.M", "G*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, SI, "KG/HECTARE", "KG*HECTARE(-1)");
    reg.AddUnit(SURFACE_DENSITY, USCUSTOM, "LBM/ACRE", "LBM*ACRE(-1)");
    }

void AddThermalConductivity(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_CONDUCTIVITY, SI, "W/(M*K)", "W*M(-1)*DELTA_KELVIN(-1)");
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
    reg.AddUnit(THREAD_PITCH, SI, "MM/REVOLUTION", "MM*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "M/RAD", "M*RAD(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "M/DEGREE", "M*ARC_DEG(-1)");
    reg.AddUnit(THREAD_PITCH, SI, "MM/RAD", "MM*RAD(-1)");

    reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/REVOLUTION", "IN*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "FT/REVOLUTION", "FT*REVOLUTION(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/DEGREE", "IN*ARC_DEG(-1)");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/RAD", "IN*RAD(-1)");
    }

void AddVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(VELOCITY, SI, "M/SEC", "M*S(-1)");
    reg.AddUnit(VELOCITY, SI, "M/MIN", "M*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "M/HR", "M*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "M/DAy", "M*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/SEC", "MM*S(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/MIN", "MM*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/HR", "MM*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "MM/DAY", "MM*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "CM/SEC", "CM*S(-1)");
    reg.AddUnit(VELOCITY, SI, "CM/MIN", "CM*MIN(-1)");
    reg.AddUnit(VELOCITY, SI, "CM/HR", "CM*HR(-1)");
    reg.AddUnit(VELOCITY, SI, "CM/DAY", "CM*DAY(-1)");
    reg.AddUnit(VELOCITY, SI, "KM/SEC", "KM*S(-1)");
    reg.AddUnit(VELOCITY, SI, "KM/HR", "KM*HR(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "IN/SEC", "IN*S(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "IN/MIN", "IN*MIN(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "IN/HR", "IN*HR(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "IN/DAY", "IN*DAY(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "FT/SEC", "FT*S(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "FT/MIN", "FT*MIN(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "FT/HR", "FT*HR(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "FT/DAY", "FT*DAY(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "YRD/SEC", "YRD*S(-1)");
    reg.AddUnit(VELOCITY, USCUSTOM, "MPH", "MILE*HR(-1)");

    // NOTE: An Admiralty mile was defined as 1853.184 Meters, then redefined as 1853.0 by UK law see schedule 1995 No. 1804, can be found at legislation.gov.uk, search by year and number.
    //reg.AddUnit(VELOCITY, MARITIME, "KNOT_UK_ADMIRALTY", "M*HR(-1)", 1853.184);
    reg.AddUnit(VELOCITY, MARITIME, "KNOT_INTERNATIONAL", "NAUT_MILE*HR(-1)");
    }

void AddAngularVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/SEC", "RAD*S(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/MIN", "RAD*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RAD/HR", "RAD*HR(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPS", "[2PI]*RAD*S(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPM", "[2PI]*RAD*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "RPH", "[2PI]*RAD*HR(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/SEC", "ARC_DEG*S(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/MIN", "ARC_DEG*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, SI, "DEG/HR", "ARC_DEG*HR(-1)");
    }

void AddDynamicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "PA-S", "PA*S");
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "POISE", "PA-S", 0.1); // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "CENTIPOISE", "[CENTI]*POISE");
    reg.AddUnit(DYNAMIC_VISCOSITY, USCUSTOM, "LBM/(FT*S)", "LBM*FT(-1)*S(-1)");
    }

void AddKinematicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.M/SEC", "M(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, USCUSTOM, "SQ.FT/SEC", "FT(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "STOKE", "CM(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "CENTISTOKE", "MM(2)*S(-1)");
    }

void AddVolume(UnitRegistry& reg)
    {
    reg.AddUnit(VOLUME, SI, "CUB.M", "M(3)");
    reg.AddUnit(VOLUME, SI, "CUB.MU", "MU(3)");
    reg.AddUnit(VOLUME, SI, "CUB.MM", "MM(3)");
    reg.AddUnit(VOLUME, SI, "CUB.CM", "CM(3)");
    reg.AddUnit(VOLUME, SI, "CUB.DM", "DM(3)");
    reg.AddUnit(VOLUME, SI, "CUB.KM", "KM(3)");
    reg.AddUnit(VOLUME, SI, "LITRE", "CUB.DM");
    reg.AddUnit(VOLUME, SI, "THOUSAND_LITRE", "[KILO]*LITRE");
    reg.AddUnit(VOLUME, SI, "MILLION_LITRE", "[MEGA]*LITRE");
    reg.AddUnit(VOLUME, SI, "MICROLITRE", "[MICRO]*LITRE");

    reg.AddUnit(VOLUME, USCUSTOM, "CUB.IN", "IN(3)");
    reg.AddUnit(VOLUME, USCUSTOM, "CUB.FT", "FT(3)");
    reg.AddUnit(VOLUME, USCUSTOM, "CUB.YRD", "YRD(3)");
    reg.AddUnit(VOLUME, USCUSTOM, "CUB.MILE", "MILE(3)");

    reg.AddUnit(VOLUME, USCUSTOM, "ACRE_IN", "ACRE*IN");
    reg.AddUnit(VOLUME, USCUSTOM, "ACRE_FT", "ACRE*FT");
    reg.AddUnit(VOLUME, USCUSTOM, "GALLON", "IN(3)", 231.0);  // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-11
    reg.AddUnit(VOLUME, USCUSTOM, "THOUSAND_GALLON", "[KILO]*GALLON");
    reg.AddUnit(VOLUME, USCUSTOM, "MILLION_GALLON", "[MEGA]*GALLON");
    reg.AddUnit(VOLUME, IMPERIAL, "GALLON_IMPERIAL", "LITRE", 4.54609); // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    }

void AddSpecificVolume(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_VOLUME, SI, "CUB.M/KG", "M(3)*KG(-1)");
    reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.FT/LBM", "FT(3)*LBM(-1)");
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
    reg.AddSynonym("RAD", "RADIAN");
    reg.AddUnitForBasePhenomenon("STERAD", BasePhenomena::SolidAngle);
    reg.AddSynonym("STERAD", "STERADIAN");
    reg.AddUnitForBasePhenomenon("US$", BasePhenomena::Finance);
    reg.AddUnitForBasePhenomenon("PERSON", BasePhenomena::Capita);
    reg.AddUnitForBasePhenomenon("ONE", BasePhenomena::Ratio); // TODO: I don't like that Ratio has base unit of ONE and all unitless unit will have a phenomenon of Ratio ...



    AddLengths(reg);
    //AddLinearCost(reg);
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
    AddMassRatio(reg);
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
    AddMapping("MILLIMETRE", "MM");
    AddMapping("CENTIMETRE", "CM");
    AddMapping("DECIMETRE", "DM");
    AddMapping("KILOMETRE", "KM");
    AddMapping("MICROMETRE", "MU");
    AddMapping("INCH", "IN");
    AddMapping("FOOT", "FT");
    AddMapping("YARD", "YRD");
    AddMapping("GRAM", "G");
    AddMapping("MILLIGRAM", "MG");
    AddMapping("MICROGRAM", "MKG");
    AddMapping("POUND", "LBM");
    AddMapping("MINUTE", "MIN");
    AddMapping("HOUR", "HR");
    AddMapping("YEAR", "YR");
    AddMapping("MILLISECOND", "MS");
    AddMapping("DEGREE_CELSIUS", "CELSIUS");
    AddMapping("DEGREE_FAHRENHEIT", "FAHRENHEIT");
    AddMapping("DEGREE_RANKINE", "RANKINE");
    AddMapping("DELTA_DEGREE_CELSIUS", "DELTA_CELSIUS");
    AddMapping("DELTA_DEGREE_FAHRENHEIT", "DELTA_FAHRENHEIT");
    AddMapping("DELTA_DEGREE_RANKINE", "DELTA_RANKINE");
    AddMapping("DELTA_DEGREE_KELVIN_PER_METRE", "KELVIN/M");
    AddMapping("RECIPROCAL_DELTA_DEGREE_KELVIN", "STRAIN/KELVIN");
    AddMapping("RECIPROCAL_DELTA_DEGREE_CELSIUS", "STRAIN/CELSIUS");
    AddMapping("RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", "STRAIN/FAHRENHEIT");
    AddMapping("RECIPROCAL_DELTA_DEGREE_RANKINE", "STRAIN/RANKINE");
    AddMapping("LUMEN_PER_FOOT_SQUARED", "LUMEN/SQ.FT");
    AddMapping("NEWTON_METRE_PER_RADIAN", "(N*M)/RAD");
    AddMapping("NEWTON_METRE_PER_DEGREE", "(N*M)/DEG");
    AddMapping("NEWTON_PER_RADIAN", "N/RAD");
    AddMapping("DEGREE", "ARC_DEG");
    AddMapping("ANGLE_MINUTE", "ARC_MINUTE");
    AddMapping("ANGLE_SECOND", "ARC_SECOND");
    AddMapping("ANGLE_QUADRANT", "ARC_QUADRANT");
    AddMapping("GRADIAN", "GRAD");
    AddMapping("METRE_SQUARED", "SQ.M");
    AddMapping("MILLIMETRE_SQUARED", "SQ.MM");
    AddMapping("CENTIMETRE_SQUARED", "SQ.CM");
    AddMapping("KILOMETRE_SQUARED", "SQ.KM");
    AddMapping("INCH_SQUARED", "SQ.IN");
    AddMapping("FOOT_SQUARED", "SQ.FT");
    AddMapping("YARD_SQUARED", "SQ.YRD");
    AddMapping("MILE_SQUARED", "SQ.MILE");
    AddMapping("KILOGRAM_PER_METRE_CUBED", "KG/CUB.M");
    AddMapping("KILOGRAM_PER_CENTIMETRE_CUBED", "KG/CUB.CM");
    AddMapping("KILOGRAM_PER_DECIMETRE_CUBED", "KG/LITRE");
    AddMapping("KILOGRAM_PER_LITRE", "KG/LITRE");
    AddMapping("GRAM_PER_CENTIMETRE_CUBED", "G/CUB.CM");
    AddMapping("MICROGRAM_PER_LITRE", "MKG/LITRE");
    AddMapping("MILLIGRAM_PER_LITRE", "MG/LITRE");
    AddMapping("POUND_PER_FOOT_CUBED", "LBM/CUB.FT");
    AddMapping("POUND_PER_GALLON", "LBM/GALLON");
    AddMapping("POUND_PER_IMPERIAL_GALLON", "LBM/GALLON_IMPERIAL");
    AddMapping("POUND_PER_INCH_CUBED", "LBM/CUB.IN");
    AddMapping("POUND_PER_MILLION_GALLON", "LBM/MILLION_GALLON");
    AddMapping("SLUG_PER_FOOT_CUBED", "SLUG/CUB.FT");
    AddMapping("KIP_PER_FOOT_CUBED", "KIP/CUB.FT");
    AddMapping("SHORT_TON_PER_FOOT_CUBED", "SHORT_TON/CUB.FT");
    AddMapping("NEWTON_PER_METRE_CUBED", "N/CUB.M");
    AddMapping("KILONEWTON_PER_METRE_CUBED", "KN/CUB.M");
    AddMapping("KILONEWTON_PER_FOOT_CUBED", "KN/CUB.FT");
    AddMapping("PERSON_PER_METRE_SQUARED", "PERSON/SQ.M");
    AddMapping("PERSON_PER_HECTARE", "PERSON/HECTARE");
    AddMapping("PERSON_PER_KILOMETRE_SQUARED", "PERSON/SQ.KM");
    AddMapping("PERSON_PER_ACRE", "PERSON/ACRE");
    AddMapping("PERSON_PER_FOOT_SQUARED", "PERSON/SQ.FT");
    AddMapping("PERSON_PER_MILE_SQUARED", "PERSON/SQ.MILE");
    AddMapping("JOULE", "J");
    AddMapping("KILOJOULE", "KJ");
    AddMapping("MEGAJOULE", "MEGAJ");
    AddMapping("GIGAJOULE", "GJ");
    AddMapping("KILOWATT_HOUR", "KWH");
    AddMapping("MEGAWATT_HOUR", "MEGAWH");
    AddMapping("GIGAWATT_HOUR", "GWH");
    AddMapping("JOULE_PER_METRE_CUBED", "J/CUB.M");
    AddMapping("KILOJOULE_PER_METRE_CUBED", "KJ/CUB.M");
    AddMapping("KILOWATT_HOUR_PER_METRE_CUBED", "KWH/CUB.M");
    AddMapping("KILOWATT_HOUR_PER_FOOT_CUBED", "KWH/CUB.FT");
    AddMapping("KILOWATT_HOUR_PER_MILLION_GALLON", "KWH/MILLION_GALLON");
    AddMapping("KILOJOULE_PER_KILOGRAM", "KJ/KG");
    AddMapping("MEGAJOULE_PER_KILOGRAM", "MEGAJ/KG");
    AddMapping("BTU_PER_POUND_MASS", "BTU/LBM");
    AddMapping("JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", "J/(KG*K)");
    AddMapping("BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", "BTU/(LBM*RANKINE)");
    AddMapping("JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", "J/(KMOL*K)");
    AddMapping("KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", "KJ/(KMOL*K)");
    AddMapping("BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE", "BTU/(LB-MOL*RANKINE)");
    AddMapping("METRE_CUBED_PER_SECOND", "CUB.M/SEC");
    AddMapping("METRE_CUBED_PER_MINUTE", "CUB.M/MIN");
    AddMapping("METRE_CUBED_PER_HOUR", "CUB.M/HR");
    AddMapping("METRE_CUBED_PER_DAY", "CUB.M/DAY");
    AddMapping("LITRE_PER_SECOND", "LITRE/SEC");
    AddMapping("LITRE_PER_MINUTE", "LITRE/MIN");
    AddMapping("LITRE_PER_HOUR", "LITRE/HR");
    AddMapping("LITRE_PER_DAY", "LITRE/DAY");
    AddMapping("FOOT_CUBED_PER_SECOND", "CUB.FT/SEC");
    AddMapping("FOOT_CUBED_PER_MINUTE", "CUB.FT/MIN");
    AddMapping("FOOT_CUBED_PER_DAY", "CUB.FT/DAY");
    AddMapping("ACRE_FOOT_PER_DAY", "ACRE_FT/DAY");
    AddMapping("ACRE_FOOT_PER_HOUR", "ACRE_FT/HR");
    AddMapping("ACRE_FOOT_PER_MINUTE", "ACRE_FT/MIN");
    AddMapping("ACRE_INCH_PER_HOUR", "ACRE_IN/HR");
    AddMapping("ACRE_INCH_PER_MINUTE", "ACRE_IN/MIN");
    AddMapping("GALLON_IMPERIAL_PER_DAY", "GALLON_IMPERIAL/DAY");
    AddMapping("GALLON_IMPERIAL_PER_MINUTE", "GALLON_IMPERIAL/MIN");
    AddMapping("GALLON_IMPERIAL_PER_SECOND", "GALLON_IMPERIAL/SEC");
    AddMapping("GALLON_PER_SECOND", "GALLON/SEC");
    AddMapping("GALLON_PER_MINUTE", "GALLON/MIN");
    AddMapping("GALLON_PER_DAY", "GALLON/DAY");
    AddMapping("METRE_CUBED_PER_METRE_SQUARED_PER_DAY", "CUB.M/(SQ.M*DAY)");
    AddMapping("METRE_CUBED_PER_HECTARE_PER_DAY", "CUB.M/(HECTARE*DAY)");
    AddMapping("METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY", "CUB.M/(SQ.KM*DAY)");
    AddMapping("FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE", "CUB.FT/(SQ.FT*MIN)");
    AddMapping("FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND", "CUB.FT/(SQ.FT*SEC)");
    AddMapping("FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND", "CUB.FT/(SQ.MILE*SEC)");
    AddMapping("GALLON_PER_ACRE_PER_DAY", "GALLON/(ACRE*DAY)");
    AddMapping("GALLON_PER_ACRE_PER_MINUTE", "GALLON/(ACRE*MIN)");
    AddMapping("GALLON_PER_FOOT_SQUARED_PER_MINUTE", "GALLON/(SQ.FT*MIN)");
    AddMapping("GALLON_PER_FOOT_SQUARED_PER_DAY", "GALLON/(SQ.FT*DAY)");
    AddMapping("GALLON_PER_MILE_SQUARED_PER_MINUTE", "GALLON/(SQ.MILE*MIN)");
    AddMapping("GALLON_PER_MILE_SQUARED_PER_DAY", "GALLON/(SQ.MILE*DAY)");
    AddMapping("FOOT_CUBED_PER_ACRE_PER_SECOND", "CUB.FT/(ACRE*SEC)");
    AddMapping("KILOGRAM_PER_SECOND", "KG/SEC");
    AddMapping("KILOGRAM_PER_MINUTE", "KG/MIN");
    AddMapping("KILOGRAM_PER_HOUR", "KG/HR");
    AddMapping("KILOGRAM_PER_DAY", "KG/DAY");
    AddMapping("GRAM_PER_SECOND", "G/SEC");
    AddMapping("GRAM_PER_MINUTE", "G/MIN");
    AddMapping("GRAM_PER_HOUR", "G/HR");
    AddMapping("MILLIGRAM_PER_SECOND", "MG/SEC");
    AddMapping("MILLIGRAM_PER_MINUTE", "MG/MIN");
    AddMapping("MILLIGRAM_PER_HOUR", "MG/HR");
    AddMapping("MILLIGRAM_PER_DAY", "MG/DAY");
    AddMapping("MICROGRAM_PER_SECOND", "MKG/SEC");
    AddMapping("MICROGRAM_PER_MINUTE", "MKG/MIN");
    AddMapping("MICROGRAM_PER_HOUR", "MKG/HR");
    AddMapping("MICROGRAM_PER_DAY", "MKG/DAY");
    AddMapping("POUND_PER_SECOND", "LBM/SEC");
    AddMapping("POUND_PER_MINUTE", "LBM/MIN");
    AddMapping("POUND_PER_HOUR", "LBM/HR");
    AddMapping("POUND_PER_DAY", "LBM/DAY");
    AddMapping("TONNE_PER_HOUR", "TONNE/HR");
    AddMapping("SHORT_TON_PER_HOUR", "SHORT_TON/HR");
    AddMapping("MOLE_PER_SECOND", "MOL/SEC");
    AddMapping("KILOMOLE_PER_SECOND", "KMOL/SEC");
    AddMapping("NEWTON", "N");
    AddMapping("KILONEWTON", "KN");
    AddMapping("MILLINEWTON", "MN");
    AddMapping("KILOGRAM_FORCE", "KGF");
    AddMapping("POUND_FORCE", "LBF");
    AddMapping("KILOPOUND_FORCE", "KPF");
    AddMapping("WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", "W/(SQ.M*K)");
    AddMapping("WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS", "W/(SQ.M*CELSIUS)");
    AddMapping("BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", "BTU/(SQ.FT*HR*FAHRENHEIT)");
    AddMapping("KILOGRAM_PER_METRE", "KG/M");
    AddMapping("KILOGRAM_PER_MILLIMETRE", "KG/MM");
    AddMapping("POUND_MASS_PER_FOOT", "LBM/FT");
    AddMapping("NEWTON_PER_METRE", "N/M");
    AddMapping("NEWTON_PER_MILLIMETRE", "N/MM");
    AddMapping("POUND_FORCE_PER_INCH", "LBF/IN");
    AddMapping("ONE_PER_METRE", "PER_M");
    AddMapping("ONE_PER_MILLIMETRE", "PER_MM");
    AddMapping("ONE_PER_KILOMETRE", "PER_KM");
    AddMapping("ONE_PER_FOOT", "PER_FT");
    AddMapping("ONE_PER_MILE", "PER_MILE");
    AddMapping("ONE_PER_THOUSAND_FOOT", "PER_THOUSAND_FT");
    AddMapping("NEWTON_METRE", "N_M");
    AddMapping("POUND_FOOT", "LBF_FT");
    AddMapping("METRE_CUBED_PER_MOLE", "CUB.M/MOL");
    AddMapping("METRE_CUBED_PER_KILOMOLE", "CUB.M/KMOL");
    AddMapping("FOOT_CUBED_PER_POUND_MOLE", "CUB.FT/LB-MOL");
    AddMapping("MOLE_PER_METRE_CUBED", "MOL/CUB.M");
    AddMapping("KILOMOLE_PER_METRE_CUBED", "KMOL/CUB.M");
    AddMapping("METRE_TO_THE_FOURTH", "M^4");
    AddMapping("CENTIMETRE_TO_THE_FOURTH", "CM^4");
    AddMapping("INCH_TO_THE_FOURTH", "IN^4");
    AddMapping("FOOT_TO_THE_FOURTH", "FT^4");
    AddMapping("MILLIMETRE_TO_THE_FOURTH", "MM^4");
    AddMapping("WATT", "W");
    AddMapping("KILOWATT", "KW");
    AddMapping("MEGAWATT", "MEGAW");
    AddMapping("GIGAWATT", "GW");
    //AddMapping("BTU_PER_MONTH", "BTU/MONTH");
    AddMapping("BTU_PER_HOUR", "BTU/HR");
    AddMapping("KILOBTU_PER_HOUR", "KILOBTU/HR");
    AddMapping("HORSEPOWER", "HP");
    //AddMapping("GIGAJOULE_PER_MONTH", "GJ/MONTH");
    AddMapping("PASCAL", "PA");
    AddMapping("NEWTON_PER_METRE_SQUARED", "PA");
    AddMapping("NEWTON_PER_MILLIMETRE_SQUARED", "N/SQ.MM");
    AddMapping("KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", "AT");
    AddMapping("KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", "AT_GAUGE");
    AddMapping("KILOGRAM_FORCE_PER_METRE_SQUARED", "KGF/SQ.M");
    AddMapping("ATMOSPHERE", "ATM");
    AddMapping("MILLIBAR", "MBAR");
    AddMapping("POUND_FORCE_PER_INCH_SQUARED", "PSI");
    AddMapping("POUND_FORCE_PER_INCH_SQUARED_GAUGE", "PSIG");
    AddMapping("POUND_FORCE_PER_FOOT_SQUARED", "LBF/SQ.FT");
    AddMapping("PASCAL_PER_METRE", "PA/M");
    AddMapping("BAR_PER_KILOMETRE", "BAR/KM");
    AddMapping("PERCENT_PERCENT", "PERCENT");
    AddMapping("UNITLESS_PERCENT", "DECIMAL_PERCENT");
    AddMapping("METRE_PER_METRE", "M/M");
    AddMapping("METRE_VERTICAL_PER_METRE_HORIZONTAL", "M/M");
    AddMapping("CENTIMETRE_PER_METRE", "CM/M");
    AddMapping("MILLIMETRE_PER_METRE", "MM/M");
    AddMapping("METRE_PER_KILOMETRE", "M/KM");
    AddMapping("FOOT_PER_FOOT", "FT/FT");
    AddMapping("FOOT_VERTICAL_PER_FOOT_HORIZONTAL", "FT/FT");
    AddMapping("INCH_PER_FOOT", "IN/FT");
    AddMapping("FOOT_PER_INCH", "FT/IN");
    AddMapping("FOOT_PER_MILE", "FT/MILE");
    AddMapping("KILOGRAM_PER_METRE_SQUARED", "KG/SQ.M");
    AddMapping("GRAM_PER_METRE_SQUARED", "G/SQ.M");
    AddMapping("KILOGRAM_PER_HECTARE", "KG/HECTARE");
    AddMapping("POUND_PER_ACRE", "LBM/ACRE");
    AddMapping("WATT_PER_METRE_PER_DEGREE_KELVIN", "W/(M*K)");
    AddMapping("WATT_PER_METRE_PER_DEGREE_CELSIUS", "W/(M*C)");
    AddMapping("BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)");
    AddMapping("METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", "(SQ.M*KELVIN)/WATT");
    AddMapping("METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT", "(SQ.M*CELSIUS)/WATT");
    AddMapping("FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU", "(SQ.FT*HR*FAHRENHEIT)/BTU");
    AddMapping("METRE_PER_REVOLUTION", "M/REVOLUTION");
    AddMapping("MILLIMETRE_PER_REVOLUTION", "MM/REVOLUTION");
    AddMapping("METRE_PER_RADIAN", "M/RAD");
    AddMapping("METRE_PER_DEGREE", "M/DEGREE");
    AddMapping("MILLIMETRE_PER_RADIAN", "MM/RAD");
    AddMapping("INCH_PER_REVOLUTION", "IN/REVOLUTION");
    AddMapping("FOOT_PER_REVOLUTION", "FT/REVOLUTION");
    AddMapping("INCH_PER_DEGREE", "IN/DEGREE");
    AddMapping("INCH_PER_RADIAN", "IN/RAD");
    AddMapping("METRE_PER_SECOND", "M/SEC");
    AddMapping("METRE_PER_MINUTE", "M/MIN");
    AddMapping("METRE_PER_HOUR", "M/HR");
    AddMapping("METRE_PER_DAY", "M/DAy");
    AddMapping("MILLIMETRE_PER_MINUTE", "MM/MIN");
    AddMapping("MILLIMETRE_PER_HOUR", "MM/HR");
    AddMapping("MILLIMETRE_PER_DAY", "MM/DAY");
    AddMapping("CENTIMETRE_PER_SECOND", "CM/SEC");
    AddMapping("CENTIMETRE_PER_MINUTE", "CM/MIN");
    AddMapping("CENTIMETRE_PER_HOUR", "CM/HR");
    AddMapping("CENTIMETRE_PER_DAY", "CM/DAY");
    AddMapping("KILOMETRE_PER_HOUR", "KM/HR");
    AddMapping("INCH_PER_SECOND", "IN/SEC");
    AddMapping("INCH_PER_MINUTE", "IN/MIN");
    AddMapping("INCH_PER_HOUR", "IN/HR");
    AddMapping("INCH_PER_DAY", "IN/DAY");
    AddMapping("FOOT_PER_SECOND", "FT/SEC");
    AddMapping("FOOT_PER_MINUTE", "FT/MIN");
    AddMapping("FOOT_PER_HOUR", "FT/HR");
    AddMapping("FOOT_PER_DAY", "FT/DAY");
    AddMapping("MILE_PER_HOUR", "MPH");
    //AddMapping("KNOT", "KNOT_UK_ADMIRALTY");    // NOTE: OldSystem used admiralty knot conversion factor for knot conversion factor
    AddMapping("KNOT_INTERNATIONAL", "KNOT_INTERNATIONAL");
    AddMapping("RADIAN_PER_SECOND", "RAD/SEC");
    AddMapping("RADIAN_PER_MINUTE", "RAD/MIN");
    AddMapping("RADIAN_PER_HOUR", "RAD/HR");
    AddMapping("CYCLE_PER_SECOND", "RPS");
    AddMapping("CYCLE_PER_MINUTE", "RPM");
    AddMapping("REVOLUTION_PER_MINUTE", "RPM");
    AddMapping("CYCLE_PER_HOUR", "RPH");
    AddMapping("DEGREE_PER_SECOND", "DEG/SEC");
    AddMapping("DEGREE_PER_MINUTE", "DEG/MIN");
    AddMapping("DEGREE_PER_HOUR", "DEG/HR");
    AddMapping("PASCAL_SECOND", "PA-S");
    AddMapping("METRE_SQUARED_PER_SECOND", "SQ.M/SEC");
    AddMapping("FOOT_SQUARED_PER_SECOND", "SQ.FT/SEC");
    AddMapping("METRE_CUBED", "CUB.M");
    AddMapping("CENTIMETRE_CUBED", "CUB.CM");
    AddMapping("INCH_CUBED", "CUB.IN");
    AddMapping("FOOT_CUBED", "CUB.FT");
    AddMapping("YARD_CUBED", "CUB.YRD");
    AddMapping("METRE_CUBED_PER_KILOGRAM", "CUB.M/KG");
    AddMapping("FOOT_CUBED_PER_POUND_MASS", "CUB.FT/LBM");
    AddMapping("METRE_TO_THE_SIXTH", "M^6");
    AddMapping("MILLIMETRE_TO_THE_SIXTH", "MM^6");
    AddMapping("CENTIMETRE_TO_THE_SIXTH", "CM^6");
    AddMapping("INCH_TO_THE_SIXTH", "IN^6");
    AddMapping("FOOT_TO_THE_SIXTH", "FT^6");
    AddMapping("METRE", "M");
    AddMapping("KILOGRAM", "KG");
    AddMapping("SECOND", "S");
    AddMapping("DEGREE_KELVIN", "K");
    AddMapping("DELTA_DEGREE_KELVIN", "DELTA_KELVIN");
    AddMapping("AMPERE", "A");
    AddMapping("RADIAN", "RAD");
    AddMapping("DOLLAR", "US$");
    AddMapping("NONE", "ONE");
    AddMapping("UNITLESS_UNIT", "ONE");
    AddMapping("THOUSAND_FOOT_SQUARED", "THOUSAND_SQ.FT");
    AddMapping("DYNE", "DYNE");
    AddMapping("FOOT_POUNDAL", "FT_PDL");
    AddMapping("KILOPASCAL_GAUGE", "KILOPASCAL_GAUGE");
    AddMapping("PERCENT_SLOPE", "PERCENT_SLOPE");
    AddMapping("MILE", "MILE");
    AddMapping("MICROINCH", "MICROINCH");
    AddMapping("MILLIINCH", "MILLIINCH");
    AddMapping("MILLIFOOT", "MILLIFOOT");
    AddMapping("US_SURVEY_INCH", "US_SURVEY_IN");
    AddMapping("US_SURVEY_FOOT", "US_SURVEY_FT");
    AddMapping("US_SURVEY_MILE", "US_SURVEY_MILE");
    AddMapping("HECTARE", "HECTARE");
    AddMapping("ACRE", "ACRE");
    AddMapping("INCH_MILE", "IN_MILE");
    AddMapping("INCH_FOOT", "IN_FT");
    AddMapping("FOOT_MILE", "FT_MILE");
    AddMapping("FOOT_FOOT", "FT_FT");
    AddMapping("MILLIMETRE_METRE", "MM_M");
    AddMapping("MILLIMETRE_KILOMETRE", "MM_KM");
    AddMapping("METRE_METRE", "M_M");
    AddMapping("METRE_KILOMETRE", "M_KM");
    AddMapping("INCH_METRE", "IN_M");
    AddMapping("MILLIMETRE_MILE", "MM_MILE");
    AddMapping("ACRE_FOOT", "ACRE_FT");
    AddMapping("ACRE_INCH", "ACRE_IN");
    AddMapping("GALLON", "GALLON");
    AddMapping("GALLON_IMPERIAL", "GALLON_IMPERIAL");
    AddMapping("LITRE", "LITRE");
    AddMapping("MILLION_GALLON", "MILLION_GALLON");
    AddMapping("MILLION_LITRE", "MILLION_LITRE");
    AddMapping("THOUSAND_GALLON", "THOUSAND_GALLON");
    AddMapping("THOUSAND_LITRE", "THOUSAND_LITRE");
    AddMapping("GRAIN", "GRM");
    AddMapping("LONG_TON", "LONG_TON");
    AddMapping("MEGAGRAM", "MEGAGRAM");
    AddMapping("SHORT_TON", "SHORT_TON");
    AddMapping("DAY", "DAY");
    AddMapping("REVOLUTION", "REVOLUTION");
    AddMapping("CENTISTOKE", "CENTISTOKE");
    AddMapping("STOKE", "STOKE");
    AddMapping("LONG_TON_FORCE", "LONG_TON_FORCE");
    AddMapping("SHORT_TON_FORCE", "SHORT_TON_FORCE");
    AddMapping("BTU", "BTU");
    AddMapping("KILOBTU", "KILOBTU");
    AddMapping("WATT_SECOND", "WATT_SECOND");
    AddMapping("KILOVOLT", "KILOVOLT");
    AddMapping("VOLT", "VOLT");
    AddMapping("MEGAVOLT", "MEGAVOLT");
    AddMapping("CAPITA", "CAPITA");
    AddMapping("PERSON", "PERSON");
    AddMapping("CUSTOMER", "CUSTOMER");
    AddMapping("EMPLOYEE", "EMPLOYEE");
    AddMapping("GUEST", "GUEST");
    AddMapping("HUNDRED_CAPITA", "HUNDRED_CAPITA");
    AddMapping("THOUSAND_CAPITA", "THOUSAND_CAPITA");
    AddMapping("PASSENGER", "PASSENGER");
    AddMapping("RESIDENT", "RESIDENT");
    AddMapping("STUDENT", "STUDENT");
    AddMapping("VERTICAL_PER_HORIZONTAL", "VERTICAL/HORIZONTAL");
    AddMapping("FOOT_HORIZONTAL_PER_FOOT_VERTICAL", "FT_HORIZONTAL/FT_VERTICAL");
    AddMapping("METRE_HORIZONTAL_PER_METRE_VERTICAL", "M_HORIZONTAL/M_VERTICAL");
    AddMapping("HORIZONTAL_PER_VERTICAL", "HORIZONTAL/VERTICAL");
    AddMapping("FOOT_PER_1000_FOOT", "FT/THOUSAND_FOOT");
    AddMapping("CENTIPOISE", "CENTIPOISE");
    AddMapping("LUX", "LUX");
    AddMapping("KILOAMPERE", "KILOAMPERE");
    AddMapping("LUMEN", "LUMEN");
    AddMapping("KILOPASCAL", "KILOPASCAL");
    AddMapping("BAR", "BAR");
    AddMapping("BAR_GAUGE", "BAR_GAUGE");
    AddMapping("BARYE", "BARYE");
    AddMapping("FOOT_OF_H2O_CONVENTIONAL", "FT_H2O");
    AddMapping("HECTOPASCAL", "HECTOPASCAL");
    AddMapping("INCH_OF_H2O_AT_32_FAHRENHEIT", "IN_H2O@32F");
    AddMapping("INCH_OF_H2O_AT_39_2_FAHRENHEIT", "IN_H2O@39.2F");
    AddMapping("INCH_OF_H2O_AT_60_FAHRENHEIT", "IN_H2O@60F");
    AddMapping("INCH_OF_HG_AT_32_FAHRENHEIT", "IN_HG@32F");
    AddMapping("INCH_OF_HG_AT_60_FAHRENHEIT", "IN_HG@60F");
    AddMapping("INCH_OF_HG_CONVENTIONAL", "IN_HG");
    AddMapping("MEGAPASCAL", "MEGAPASCAL");
    AddMapping("MEGAPASCAL_GAUGE", "MEGAPASCAL_GAUGE");
    AddMapping("METRE_OF_H2O_CONVENTIONAL", "M_H2O");
    AddMapping("MILLIMETRE_OF_H2O_CONVENTIONAL", "MM_H2O");
    AddMapping("MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", "MM_HG@32F");
    AddMapping("TORR", "TORR");
    AddMapping("HERTZ", "HZ");
    AddMapping("KILOGRAM_PER_KILOGRAM", "KG/KG");
    AddMapping("GRAIN_MASS_PER_POUND_MASS", "GRM/LBM");

    }
