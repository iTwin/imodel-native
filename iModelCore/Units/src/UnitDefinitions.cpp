/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Units/UnitRegistry.h>

USING_NAMESPACE_BENTLEY_UNITS

void AddLengths(UnitRegistry& reg)
    {
    reg.AddUnit(LENGTH, METRIC, "MM", "[MILLI]*M");
    reg.AddUnit(LENGTH, METRIC, "CM", "[CENTI]*M");
    reg.AddUnit(LENGTH, METRIC, "DM", "[DECI]*M");
    reg.AddUnit(LENGTH, METRIC, "KM", "[KILO]*M");
    reg.AddUnit(LENGTH, METRIC, "MU", "[MICRO]*M");

    reg.AddUnit(LENGTH, USCUSTOM, "MILLIINCH", "[MILLI]*IN");
    reg.AddUnit(LENGTH, USCUSTOM, "MICROINCH", "[MICRO]*IN");
    reg.AddUnit(LENGTH, USCUSTOM, "MILLIFOOT", "[MILLI]*FT");

    reg.AddUnit(LENGTH, USCUSTOM, "IN", "MM", 25.4); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix B. Section 3.1, Page B-10
    reg.AddUnit(LENGTH, USCUSTOM, "FT", "IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4

    reg.AddUnit(LENGTH, USCUSTOM, "YRD", "FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USCUSTOM, "CHAIN", "FT", 66.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    reg.AddUnit(LENGTH, USCUSTOM, "MILE", "YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_IN", "M", 100.0, 3937.0); // 100/3937 Derived from the definition of us survey foot in terms of meters.  Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-9
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_FT", "US_SURVEY_IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_YRD", "US_SURVEY_FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_CHAIN", "US_SURVEY_FT", 66.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    reg.AddUnit(LENGTH, USSURVEY, "US_SURVEY_MILE", "US_SURVEY_YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    reg.AddUnit(LENGTH, MARITIME, "NAUT_MILE", "M", 1852.0); // International Nautical Mile.  Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    }

void AddMass(UnitRegistry& reg)
    {
    reg.AddUnit(MASS, METRIC, "G", "[MILLI]*KG");
    reg.AddUnit(MASS, METRIC, "MG", "[MILLI]*G");
    reg.AddUnit(MASS, METRIC, "MKG", "[MICRO]*G");
    reg.AddUnit(MASS, METRIC, "NG", "[NANO]*G");
    reg.AddUnit(MASS, METRIC, "MEGAGRAM", "[MEGA]*G");
    reg.AddUnit(MASS, METRIC, "TONNE", "[KILO]*KG"); // Also known as a metric ton http://phyMETRICcs.nist.gov/cuu/pdf/sp811.pdf, Appendix B.

    reg.AddUnit(MASS, USCUSTOM, "LBM", "KG", 0.45359237); // Is Avoirdupois Pound.  Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Footnote 22
    reg.AddUnit(MASS, USCUSTOM, "SLUG", "LBF*S(2)*FT(-1)");
    reg.AddUnit(MASS, USCUSTOM, "GRM", "LBM", 1.0, 7000.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix B. Section 3.2, Page B-10

    reg.AddUnit(MASS, USCUSTOM, "SHORT_TON_MASS", "LBM", 2000); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "LONG_TON_MASS", "LBM", 2240); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "KIPM", "[KILO]*LBM"); // Exact, http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(MASS, USCUSTOM, "OZM", "LBM", 1.0, 16.0); // 1/16 Exact, https://en.wikipedia.org/wiki/Ounce
    }

void AddTime(UnitRegistry& reg)
    {
    reg.AddUnit(TIME, INTERNATIONAL, "MIN", "S", 60.0);
    reg.AddUnit(TIME, INTERNATIONAL, "HR", "MIN", 60.0);
    reg.AddUnit(TIME, INTERNATIONAL, "DAY", "HR", 24.0);
    reg.AddUnit(TIME, INTERNATIONAL, "WEEK", "DAY", 7.0);
    reg.AddUnit(TIME, INTERNATIONAL, "YR", "DAY", 365);  //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B. Year is 3.1536 E+07 seconds which is equal to 365 * 24 * 60 * 60
    reg.AddUnit(TIME, INTERNATIONAL, "YEAR_SIDEREAL", "S", 3.155815e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, INTERNATIONAL, "YEAR_TROPICAL", "S", 3.155693e7); //  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.
    reg.AddUnit(TIME, INTERNATIONAL, "MS", "[MILLI]*S");
    reg.AddUnit(TIME, INTERNATIONAL, "MKS", "[MICRO]*S");
    }

void AddTemperature(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE, METRIC, "CELSIUS", "K", 1.0, 1.0, 273.15);
    reg.AddUnit(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "CELSIUS", 5.0, 9.0, -32); // Factor is 5/9
    reg.AddUnit(TEMPERATURE, USCUSTOM, "RANKINE", "K", 5.0, 9.0); // Factor is 5/9
    }

void AddTemperatureChange(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_CHANGE, METRIC, "DELTA_CELSIUS", "DELTA_KELVIN", 1.0);
    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_FAHRENHEIT", "DELTA_CELSIUS", 5.0, 9.0); // Factor is 5/9
    reg.AddUnit(TEMPERATURE_CHANGE, USCUSTOM, "DELTA_RANKINE", "DELTA_KELVIN", 5.0, 9.0); // Factor is 5/9
    }

void AddTemperatureGradient(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE_GRADIENT, SI, "KELVIN/M", "DELTA_KELVIN*M(-1)");
    }

void AddLinearThermalExpansionCoefficient(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, SI, "STRAIN/KELVIN", "DELTA_KELVIN(-1)");

    reg.AddUnit(LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION, METRIC, "STRAIN/CELSIUS", "DELTA_CELSIUS(-1)");

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

void AddMole(UnitRegistry& reg)
    {
    reg.AddUnit(MOLE, METRIC, "KMOL", "[KILO]*MOL");
    reg.AddUnit(MOLE, USCUSTOM, "LB-MOL", "MOL", 453.59237); // ASTM SI 10 standard SI1-.phhc8328.pdf page 29, 35 and http://en.wikipedia.org/wiki/Mole_%28unit%29
    }

void AddPerson(UnitRegistry& reg)
    {
    reg.AddUnit(CAPITA, STATISTICS, "HUNDRED_PERSON", "[HECTO]*PERSON");
    reg.AddUnit(CAPITA, STATISTICS, "THOUSAND_PERSON", "[KILO]*PERSON");
    }

void AddRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, SI, "(N*M)/RAD", "N_M*RAD(-1)");
    reg.AddUnit(ROTATIONAL_SPRING_CONSTANT, METRIC, "(N*M)/DEG", "N_M*ARC_DEG(-1)");
    }

void AddLinearRotationalSpringConstant(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_ROTATIONAL_SPRING_CONSTANT, SI, "N/RAD", "N*RAD(-1)");
    }

void AddAcceleration(UnitRegistry& reg)
    {
    reg.AddUnit(ACCELERATION, SI, "M/SEC.SQ", "M*S(-2)");
    reg.AddUnit(ACCELERATION, METRIC, "CM/SEC.SQ", "CM*S(-2)");
    reg.AddUnit(ACCELERATION, USCUSTOM, "FT/SEC.SQ", "FT*S(-2)");
    }

void AddPlaneAngle(UnitRegistry& reg)
    {
    reg.AddUnit(ANGLE, METRIC, "ARC_DEG", "[PI]*RAD", 1.0, 180.0 ); // 1/180

    reg.AddUnit(ANGLE, METRIC, "ARC_MINUTE", "ARC_DEG", 1.0, 60.0); // 1/60
    reg.AddUnit(ANGLE, METRIC, "ARC_SECOND", "ARC_DEG", 1.0, 3600.0); // 1/3600
    reg.AddUnit(ANGLE, METRIC, "ARC_QUADRANT", "[PI/2]*RAD");
    reg.AddUnit(ANGLE, METRIC, "GRAD", "[PI]*RAD", 1.0, 200.0); // 1/200
    reg.AddUnit(ANGLE, METRIC, "REVOLUTION", "[2PI]*RAD");
    }

void AddArea(UnitRegistry& reg)
    {
    reg.AddUnit(AREA, SI, "SQ.M", "M(2)");
    reg.AddUnit(AREA, METRIC, "SQ.MU", "MU(2)");
    reg.AddUnit(AREA, METRIC, "SQ.MM", "MM(2)");
    reg.AddUnit(AREA, METRIC, "SQ.CM", "CM(2)");
    reg.AddUnit(AREA, METRIC, "SQ.DM", "DM(2)");
    reg.AddUnit(AREA, METRIC, "SQ.KM", "KM(2)");
    reg.AddUnit(AREA, METRIC, "ARE", "[HECTO]*M(2)");
    reg.AddUnit(AREA, METRIC, "HECTARE", "[HECTO]*ARE");

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
    reg.AddUnit(SIZE_LENGTH_RATE, SI, "M_M", "M*M");
    }

void AddDensity(UnitRegistry& reg)
    {
    reg.AddUnit(DENSITY, SI, "KG/CUB.M", "KG*M(-3)");
    reg.AddUnit(DENSITY, METRIC, "KG/CUB.CM", "KG*CM(-3)");
    reg.AddUnit(DENSITY, METRIC, "KG/LITRE", "KG*DM(-3)");

    reg.AddUnit(DENSITY, METRIC, "G/CUB.CM", "G*CM(-3)");

    reg.AddUnit(DENSITY, METRIC, "MKG/LITRE", "MKG*DM(-3)");

    reg.AddUnit(DENSITY, METRIC, "MG/LITRE", "MG*DM(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "LBM/CUB.FT", "LBM*FT(-3)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/GALLON", "LBM*GALLON(-1)");
    reg.AddUnit(DENSITY, IMPERIAL, "LBM/GALLON_IMPERIAL", "LBM*GALLON_IMPERIAL(-1)");
    reg.AddUnit(DENSITY, USCUSTOM, "LBM/CUB.IN", "LBM*IN(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "LBM/MILLION_GALLON", "LBM*[MEGA](-1)*GALLON(-1)");

    reg.AddUnit(DENSITY, USCUSTOM, "SLUG/CUB.FT", "SLUG*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "KIP/CUB.FT", "KIPM*FT(-3)");

    reg.AddUnit(DENSITY, USCUSTOM, "SHORT_TON/CUB.FT", "SHORT_TON_MASS*FT(-3)");
    }

void AddForceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE_DENSITY, SI, "N/CUB.M", "N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, METRIC, "KN/CUB.M", "[KILO]*N*M(-3)");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "N/CUB.FT", "N*FT(-3)");
    reg.AddUnit(FORCE_DENSITY, USCUSTOM, "KN/CUB.FT", "[KILO]*N*FT(-3)");
    }

void AddPopulationDensity(UnitRegistry& reg)
    {
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.M", "PERSON*M(-2)");
    reg.AddUnit(POPULATION_DENSITY, METRIC, "PERSON/HECTARE", "PERSON*HECTARE(-1)");
    reg.AddUnit(POPULATION_DENSITY, METRIC, "PERSON/SQ.KM", "PERSON*KM(-2)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/ACRE", "PERSON*ACRE(-1)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/SQ.FT", "PERSON*FT(-2)");
    reg.AddUnit(POPULATION_DENSITY, USCUSTOM, "PERSON/SQ.MILE", "PERSON*MILE(-2)");
    }

void AddElectricCurrent(UnitRegistry& reg)
    {
    reg.AddUnit(CURRENT, METRIC, "KILOAMPERE", "[KILO]*A");
    reg.AddUnit(CURRENT, METRIC, "MILLIAMPERE", "[MILLI]*A");
    reg.AddUnit(CURRENT, METRIC, "MICROAMPERE", "[MICRO]*A");
    }

void AddElectricCharge(UnitRegistry& reg)
    {
    reg.AddUnit(ELECTRIC_CHARGE, SI, "COULOMB", "A*S");
    }

void AddElectricPotential(UnitRegistry& reg)
    {
    reg.AddUnit(ELECTRIC_POTENTIAL, SI, "VOLT", "N*M*COULOMB(-1)");
    reg.AddUnit(ELECTRIC_POTENTIAL, METRIC, "KILOVOLT", "[KILO]*VOLT");
    reg.AddUnit(ELECTRIC_POTENTIAL, METRIC, "MEGAVOLT", "[MEGA]*VOLT");
    }

void AddEnergy(UnitRegistry& reg)
    {
    reg.AddUnit(WORK, SI, "J", "N*M");
    reg.AddUnit(WORK, METRIC, "KJ", "[KILO]*J");
    reg.AddUnit(WORK, METRIC, "MEGAJ", "[MEGA]*J");
    reg.AddUnit(WORK, METRIC, "GJ", "[GIGA]*J");
    reg.AddUnit(WORK, USCUSTOM, "FT_PDL", "PDL*FT");
    reg.AddUnit(WORK, INTERNATIONAL, "BTU", "J", 1.05505585262e3); // Is IT BTU.  http://physics.nist.gov/cuu/pdf/sp811.pdf, Appendix B.  See foot note #9: 
    reg.AddUnit(WORK, USCUSTOM, "KILOBTU", "[KILO]*BTU");

    reg.AddUnit(WORK, INTERNATIONAL, "WATT_SECOND", "W*S");
    reg.AddUnit(WORK, INTERNATIONAL, "KWH", "KW*HR");
    reg.AddUnit(WORK, INTERNATIONAL, "MEGAWH", "MEGAW*HR");
    reg.AddUnit(WORK, INTERNATIONAL, "GWH", "GW*HR");
    }

void AddEnergyDensity(UnitRegistry& reg)
    {
    reg.AddUnit(ENERGY_DENSITY, SI, "J/CUB.M", "J*M(-3)");

    reg.AddUnit(ENERGY_DENSITY, METRIC, "KJ/CUB.M", "KJ*M(-3)");

    reg.AddUnit(ENERGY_DENSITY, METRIC, "KWH/CUB.M", "KWH*M(-3)");

    reg.AddUnit(ENERGY_DENSITY, USCUSTOM, "KWH/CUB.FT", "KWH*FT(-3)");

    reg.AddUnit(ENERGY_DENSITY, USCUSTOM, "KWH/MILLION_GALLON", "KWH*[MEGA](-1)*GALLON(-1)");
    }

void AddHeatingValue(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_ENERGY, SI, "J/KG", "J*KG(-1)");
    reg.AddUnit(SPECIFIC_ENERGY, METRIC, "KJ/KG", "KJ*KG(-1)");
    reg.AddUnit(SPECIFIC_ENERGY, METRIC, "MEGAJ/KG", "MEGAJ*KG(-1)");

    reg.AddUnit(SPECIFIC_ENERGY, USCUSTOM, "BTU/LBM", "BTU*LBM(-1)");
    }

void AddSpecificHeatCapacity(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, SI, "J/(KG*K)", "J*KG(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY, USCUSTOM, "BTU/(LBM*RANKINE)", "BTU*LBM(-1)*DELTA_RANKINE(-1)");
    }

void AddSpecificHeatCapacityMolar(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, SI, "J/(MOL*K)", "J*MOL(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, METRIC, "J/(KMOL*K)", "J*KMOL(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, METRIC, "KJ/(KMOL*K)", "KJ*KMOL(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(SPECIFIC_HEAT_CAPACITY_MOLAR, USCUSTOM, "BTU/(LB-MOL*RANKINE)", "BTU*LB-MOL(-1)*DELTA_RANKINE(-1)");
    }

void AddVolumeFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(FLOW, SI, "CUB.M/SEC", "M(3)*S(-1)");
    reg.AddUnit(FLOW, METRIC, "CUB.M/MIN", "M(3)*MIN(-1)");
    reg.AddUnit(FLOW, METRIC, "CUB.M/HR", "M(3)*HR(-1)");
    reg.AddUnit(FLOW, METRIC, "CUB.M/DAY", "M(3)*DAY(-1)");

    reg.AddUnit(FLOW, METRIC, "LITRE/SEC", "LITRE*S(-1)");
    reg.AddUnit(FLOW, METRIC, "LITRE/MIN", "LITRE*MIN(-1)");
    reg.AddUnit(FLOW, METRIC, "LITRE/HR", "LITRE*HR(-1)");
    reg.AddUnit(FLOW, METRIC, "LITRE/DAY", "LITRE*DAY(-1)");

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

    reg.AddUnit(FLOW, IMPERIAL, "GALLON_IMPERIAL/SEC", "GALLON_IMPERIAL*S(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "GALLON_IMPERIAL/MIN", "GALLON_IMPERIAL*MIN(-1)");
    reg.AddUnit(FLOW, IMPERIAL, "GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL*DAY(-1)");

    reg.AddUnit(FLOW, USCUSTOM, "GALLON/SEC", "GALLON*S(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/MIN", "GALLON*MIN(-1)");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/DAY", "GALLON*DAY(-1)");
    }

void AddFrequency(UnitRegistry& reg)
    {
    reg.AddUnit(FREQUENCY, SI, "HZ", "S(-1)");
    reg.AddUnit(FREQUENCY, METRIC, "KHZ", "[KILO]*S(-1)");
    reg.AddUnit(FREQUENCY, METRIC, "MHZ", "[MEGA]*S(-1)");
    }

void AddSurfaceFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(SQ.M*SEC)", "M*S(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, METRIC, "CUB.M/(SQ.M*DAY)", "M*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, METRIC, "CUB.M/(HECTARE*DAY)", "CUB.M*HECTARE(-1)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, METRIC, "CUB.M/(SQ.KM*DAY)", "CUB.M*KM(-2)*DAY(-1)");
    reg.AddUnit(SURFACE_FLOW_RATE, METRIC, "LITRE/(SQ.M*SEC)", "LITRE*M(-2)*S(-1)");
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
    reg.AddUnit(MASS_FLOW, METRIC, "KG/MIN", "KG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "KG/HR", "KG*HR(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "KG/DAY", "KG*DAY(-1)");

    reg.AddUnit(MASS_FLOW, METRIC, "G/SEC", "G*S(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "G/MIN", "G*MIN(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "G/HR", "G*HR(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MG/SEC", "MG*S(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MG/MIN", "MG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MG/HR", "MG*HR(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MG/DAY", "MG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MKG/SEC", "MKG*S(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MKG/MIN", "MKG*MIN(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MKG/HR", "MKG*HR(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "MKG/DAY", "MKG*DAY(-1)");
    reg.AddUnit(MASS_FLOW, METRIC, "TONNE/HR", "TONNE*HR(-1)");

    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/SEC", "LBM*S(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/MIN", "LBM*MIN(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/HR", "LBM*HR(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "LBM/DAY", "LBM*DAY(-1)");
    reg.AddUnit(MASS_FLOW, USCUSTOM, "SHORT_TON/HR", "SHORT_TON_MASS*HR(-1)");
    }

void AddParticleFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(PARTICLE_FLOW, SI, "MOL/SEC", "MOL*S(-1)");
    reg.AddUnit(PARTICLE_FLOW, METRIC, "KMOL/SEC", "[KILO]*MOL*S(-1)");
    }

void AddForce(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE, SI, "N", "KG*M*S(-2)");
    reg.AddUnit(FORCE, METRIC, "KN", "[KILO]*N");
    reg.AddUnit(FORCE, METRIC, "MN", "[MILLI]*N");
    reg.AddUnit(FORCE, METRIC, "KGF", "[STD_G]*KG");
    reg.AddUnit(FORCE, METRIC, "DYNE", "G*CM*S(-2)");

    reg.AddUnit(FORCE, USCUSTOM, "PDL", "LBM*FT*S(-2)");
    reg.AddUnit(FORCE, USCUSTOM, "SHORT_TON_FORCE", "[STD_G]*SHORT_TON_MASS");
    reg.AddUnit(FORCE, USCUSTOM, "LONG_TON_FORCE", "[STD_G]*LONG_TON_MASS");
    reg.AddUnit(FORCE, USCUSTOM, "LBF", "[STD_G]*LBM");
    reg.AddUnit(FORCE, USCUSTOM, "OZF", "[STD_G]*OZM");
    reg.AddUnit(FORCE, USCUSTOM, "KPF", "[KILO]*LBF");
    }

void AddHeatFlux(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_FLUX_DENSITY, SI, "W/SQ.M", "W*M(-2)");
    }

// TODO: Thermal Transmittance?
void AddHeatTransfer(UnitRegistry& reg)
    {
    reg.AddUnit(HEAT_TRANSFER, SI, "W/(SQ.M*K)", "W*M(-2)*DELTA_KELVIN(-1)");

    reg.AddUnit(HEAT_TRANSFER, METRIC, "W/(SQ.M*CELSIUS)", "W*M(-2)*DELTA_CELSIUS(-1)");
    reg.AddUnit(HEAT_TRANSFER, USCUSTOM, "BTU/(SQ.FT*HR*FAHRENHEIT)", "BTU*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    }

void AddLinearDensity(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_DENSITY, SI, "KG/M", "KG*M(-1)");
    reg.AddUnit(LINEAR_DENSITY, METRIC, "KG/MM", "KG*MM(-1)");
    reg.AddUnit(LINEAR_DENSITY, USCUSTOM, "LBM/FT", "LBM*FT(-1)");
    }

void AddLinearLoad(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_LOAD, SI, "N/M", "N*M(-1)");
    reg.AddUnit(LINEAR_LOAD, METRIC, "N/MM", "N*MM(-1)");
    reg.AddUnit(LINEAR_LOAD, USCUSTOM, "LBF/IN", "LBF*IN(-1)");
    }

void AddLinearRate(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_RATE, SI, "PER_M", "M(-1)");

    reg.AddUnit(LINEAR_RATE, METRIC, "PER_MM", "MM(-1)");
    reg.AddUnit(LINEAR_RATE, METRIC, "PER_KM", "KM(-1)");
    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_FT", "FT(-1)");
    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_MILE", "MILE(-1)");
    reg.AddUnit(LINEAR_RATE, USCUSTOM, "PER_THOUSAND_FT", "FT(-1)", 1.0e-3);
    }

void AddTorque(UnitRegistry& reg)
    {
    reg.AddUnit(TORQUE, SI, "N_M", "N*M*RAD(-1)");
    reg.AddUnit(TORQUE, METRIC, "N_CM", "N*CM*RAD(-1)");
    reg.AddUnit(TORQUE, USCUSTOM, "LBF_FT", "LBF*FT*RAD(-1)");
    }

void AddMolarVolume(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_VOLUME, SI, "CUB.M/MOL", "CUB.M*MOL(-1)");
    reg.AddUnit(MOLAR_VOLUME, METRIC, "CUB.M/KMOL", "CUB.M*[KILO](-1)*MOL(-1)");
    reg.AddUnit(MOLAR_VOLUME, USCUSTOM, "CUB.FT/LB-MOL", "CUB.FT*LB-MOL(-1)");
    }

void AddMolarConcentration(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_CONCENTRATION, SI, "MOL/CUB.M", "MOL*CUB.M(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, METRIC, "KMOL/CUB.M", "[KILO]*MOL*CUB.M(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, METRIC, "MOL/CUB.DM", "MOL*CUB.DM(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, METRIC, "MICROMOL/CUB.DM", "[MICRO]*MOL*CUB.DM(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, METRIC, "NMOL/CUB.DM", "[NANO]*MOL*CUB.DM(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, METRIC, "PICOMOL/CUB.DM", "[PICO]*MOL*CUB.DM(-1)");
    reg.AddUnit(MOLAR_CONCENTRATION, USCUSTOM, "MOL/CUB.FT", "MOL*CUB.FT(-1)");
    }

void AddMomentOfInertia(UnitRegistry& reg)
    {
    reg.AddUnit(AREA_MOMENT_INERTIA, METRIC, "MM^4", "MM(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, SI, "M^4", "M(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, METRIC, "CM^4", "CM(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "IN^4", "IN(4)");
    reg.AddUnit(AREA_MOMENT_INERTIA, USCUSTOM, "FT^4", "FT(4)");
    }

void AddPower(UnitRegistry& reg)
    {
    reg.AddUnit(POWER, SI, "W", "N*M*S(-1)");
    reg.AddUnit(POWER, METRIC, "KW", "[KILO]*W");
    reg.AddUnit(POWER, METRIC, "MEGAW", "[MEGA]*W");
    reg.AddUnit(POWER, METRIC, "GW", "[GIGA]*W");
    reg.AddUnit(POWER, USCUSTOM, "BTU/HR", "BTU*HR(-1)");
    reg.AddUnit(POWER, USCUSTOM, "KILOBTU/HR", "[KILO]*BTU*HR(-1)");
    reg.AddUnit(POWER, USCUSTOM, "HP", "LBF*FT*S(-1)", 550.0);  // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    }

void AddPressure(UnitRegistry& reg)
    {
    reg.AddUnit(PRESSURE, SI, "PA", "N*M(-2)");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "PA_GAUGE", "PA", 1, 1.0, 101325);  // Offset is one standard atmosphere in PA.  Offset is exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, METRIC, "HECTOPASCAL", "[HECTO]*PA");

    reg.AddUnit(PRESSURE, METRIC, "KILOPASCAL", "[KILO]*PA");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "KILOPASCAL_GAUGE", "[KILO]*PA", 1, 1.0, 101325e-3);  // Offset is one standard atmosphere (101325 PA) converted to kilopascal.  Offset is exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, METRIC, "MEGAPASCAL", "[MEGA]*PA");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "MEGAPASCAL_GAUGE", "[MEGA]*PA", 1, 1.0, 101325e-6);  // Offset is one standard atmosphere (101325 PA) converted to megapascal.  Offset is exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, INTERNATIONAL, "AT", "KGF*CM(-2)");  // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.

    reg.AddUnit(PRESSURE, INDUSTRIAL, "AT_GAUGE", "AT", 1.0, 1.0, 1.0332274527998859); // Offset is one standard atmosphere (101325 PA) converted to atmosphere-technical (AT).  Offset is exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, INTERNATIONAL, "KGF/SQ.M", "KGF*M(-2)");

    reg.AddUnit(PRESSURE, METRIC, "ATM", "PA", 101325);  // Standard atmosphere, see AT for atmosphere-technical.  Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, METRIC, "BAR", "PA", 1.0e5); // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  
    reg.AddUnit(PRESSURE, INDUSTRIAL, "BAR_GAUGE", "PA", 1.0e5, 1.0, 1.01325); // Offset is one standard atmosphere converted to BAR.  Offset is exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  

    reg.AddUnit(PRESSURE, METRIC, "MBAR", "[MILLI]*BAR");

    reg.AddUnit(PRESSURE, METRIC, "BARYE", "DYNE*CM(-2)");


    reg.AddUnit(PRESSURE, USCUSTOM, "PSI", "LBF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "PSIG", "LBF*IN(-2)", 1, 1.0, 14.695948775513449); // Offset is one standard atmosphere (101325 PA) converted to PSI

    reg.AddUnit(PRESSURE, USCUSTOM, "KSI", "KPF*IN(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "LBF/SQ.FT", "LBF*FT(-2)");

    reg.AddUnit(PRESSURE, USCUSTOM, "TORR", "PA", 101325.0, 760.0);   // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B. for approx conversion and Table 11 for a reference to the exact conversion

    // TODO: Go back to density conversion once we have verified sources for those values
    reg.AddUnit(PRESSURE, METRIC, "M_H2O", "[KILO]*MM_H2O"); // Meter of H2O Conventional
    reg.AddUnit(PRESSURE, METRIC, "MM_H2O", "PA", 9.80665); // Millimeter of H2O Conventional, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(PRESSURE, METRIC, "MM_HG@32F", "PA", 1.33322e2); // See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.  Used centimeter of mercury (0 C) to pascal
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
    reg.AddUnit(PRESSURE_GRADIENT, METRIC, "BAR/KM", "BAR*KM(-1)");
    }

void AddPercentage(UnitRegistry& reg)
    {
    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "PERCENT", "ONE");

    reg.AddUnit(PERCENTAGE, INTERNATIONAL, "DECIMAL_PERCENT", "PERCENT", 100);
    }

void AddSlope(UnitRegistry& reg)
    {
    reg.AddUnit(SLOPE, SI, "M/M", "M*M(-1)");

    reg.AddUnit(SLOPE, METRIC, "CM/M", "CM*M(-1)");
    reg.AddUnit(SLOPE, METRIC, "MM/M", "MM*M(-1)");
    reg.AddUnit(SLOPE, METRIC, "M/KM", "M*KM(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/THOUSAND_FOOT", "FT*[KILO](-1)*FT(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/FT", "FT*FT(-1)");

    reg.AddUnit(SLOPE, USCUSTOM, "IN/FT", "IN*FT(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/IN", "FT*IN(-1)");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/MILE", "FT*MILE(-1)");

    reg.AddUnit(SLOPE, INTERNATIONAL, "VERTICAL/HORIZONTAL", "M/M");

    reg.AddUnit(SLOPE, INTERNATIONAL, "PERCENT_SLOPE", "DECIMAL_PERCENT(-1)*M/M");

    reg.AddInvertedUnit("VERTICAL/HORIZONTAL", "HORIZONTAL/VERTICAL", INTERNATIONAL);
    reg.AddInvertedUnit("FT/FT", "FT_HORIZONTAL/FT_VERTICAL", USCUSTOM);
    reg.AddInvertedUnit("M/M", "M_HORIZONTAL/M_VERTICAL", SI);
    }

void AddSurfaceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_DENSITY, SI, "KG/SQ.M", "KG*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, METRIC, "G/SQ.M", "G*M(-2)");
    reg.AddUnit(SURFACE_DENSITY, METRIC, "KG/HECTARE", "KG*HECTARE(-1)");
    reg.AddUnit(SURFACE_DENSITY, USCUSTOM, "LBM/ACRE", "LBM*ACRE(-1)");
    }

void AddThermalConductivity(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_CONDUCTIVITY, SI, "W/(M*K)", "W*M(-1)*DELTA_KELVIN(-1)");
    reg.AddUnit(THERMAL_CONDUCTIVITY, METRIC, "W/(M*C)", "W*M(-1)*DELTA_CELSIUS(-1)");

    reg.AddUnit(THERMAL_CONDUCTIVITY, USCUSTOM, "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "BTU*IN*FT(-2)*HR(-1)*DELTA_FAHRENHEIT(-1)");
    }

void AddThermalResistance(UnitRegistry& reg)
    {
    reg.AddUnit(THERMAL_RESISTANCE, SI, "(SQ.M*KELVIN)/WATT", "M(2)*DELTA_KELVIN*W(-1)");
    reg.AddUnit(THERMAL_RESISTANCE, METRIC, "(SQ.M*CELSIUS)/WATT", "M(2)*DELTA_CELSIUS*W(-1)");

    reg.AddUnit(THERMAL_RESISTANCE, USCUSTOM, "(SQ.FT*HR*FAHRENHEIT)/BTU", "FT(2)*HR*DELTA_FAHRENHEIT*BTU(-1)");
    }

void AddVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(VELOCITY, SI, "M/SEC", "M*S(-1)");
    reg.AddUnit(VELOCITY, METRIC, "M/MIN", "M*MIN(-1)");
    reg.AddUnit(VELOCITY, METRIC, "M/HR", "M*HR(-1)");
    reg.AddUnit(VELOCITY, METRIC, "M/DAy", "M*DAY(-1)");
    reg.AddUnit(VELOCITY, METRIC, "MM/SEC", "MM*S(-1)");
    reg.AddUnit(VELOCITY, METRIC, "MM/MIN", "MM*MIN(-1)");
    reg.AddUnit(VELOCITY, METRIC, "MM/HR", "MM*HR(-1)");
    reg.AddUnit(VELOCITY, METRIC, "MM/DAY", "MM*DAY(-1)");
    reg.AddUnit(VELOCITY, METRIC, "CM/SEC", "CM*S(-1)");
    reg.AddUnit(VELOCITY, METRIC, "CM/MIN", "CM*MIN(-1)");
    reg.AddUnit(VELOCITY, METRIC, "CM/HR", "CM*HR(-1)");
    reg.AddUnit(VELOCITY, METRIC, "CM/DAY", "CM*DAY(-1)");
    reg.AddUnit(VELOCITY, METRIC, "KM/SEC", "KM*S(-1)");
    reg.AddUnit(VELOCITY, METRIC, "KM/HR", "KM*HR(-1)");
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
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "RAD/MIN", "RAD*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "RAD/HR", "RAD*HR(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "RPS", "[2PI]*RAD*S(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "RPM", "[2PI]*RAD*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "RPH", "[2PI]*RAD*HR(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "DEG/SEC", "ARC_DEG*S(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "DEG/MIN", "ARC_DEG*MIN(-1)");
    reg.AddUnit(ANGULAR_VELOCITY, METRIC, "DEG/HR", "ARC_DEG*HR(-1)");
    }

void AddDynamicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(DYNAMIC_VISCOSITY, SI, "PA-S", "PA*S");
    reg.AddUnit(DYNAMIC_VISCOSITY, METRIC, "POISE", "[DECI]*PA-S"); // Exact, See http://physics.nist.gov/cuu/pdf/sp811.pdf Appendix B.
    reg.AddUnit(DYNAMIC_VISCOSITY, METRIC, "CENTIPOISE", "[CENTI]*POISE");
    reg.AddUnit(DYNAMIC_VISCOSITY, USCUSTOM, "LBM/(FT*S)", "LBM*FT(-1)*S(-1)");
    }

void AddKinematicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(KINEMATIC_VISCOSITY, SI, "SQ.M/SEC", "M(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, USCUSTOM, "SQ.FT/SEC", "FT(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, METRIC, "STOKE", "CM(2)*S(-1)");
    reg.AddUnit(KINEMATIC_VISCOSITY, METRIC, "CENTISTOKE", "MM(2)*S(-1)");
    }

void AddVolume(UnitRegistry& reg)
    {
    reg.AddUnit(VOLUME, SI, "CUB.M", "M(3)");
    reg.AddUnit(VOLUME, METRIC, "CUB.MU", "MU(3)");
    reg.AddUnit(VOLUME, METRIC, "CUB.MM", "MM(3)");
    reg.AddUnit(VOLUME, METRIC, "CUB.CM", "CM(3)");
    reg.AddUnit(VOLUME, METRIC, "CUB.DM", "DM(3)");
    reg.AddUnit(VOLUME, METRIC, "CUB.KM", "KM(3)");
    reg.AddUnit(VOLUME, METRIC, "LITRE", "CUB.DM");
    reg.AddUnit(VOLUME, METRIC, "THOUSAND_LITRE", "[KILO]*LITRE");
    reg.AddUnit(VOLUME, METRIC, "MILLION_LITRE", "[MEGA]*LITRE");
    reg.AddUnit(VOLUME, METRIC, "MICROLITRE", "[MICRO]*LITRE");

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

void AddVolumeRatio(UnitRegistry& reg)
    {
    reg.AddUnit(VOLUME_RATIO, SI, "CUB.M/CUB.M", "M(3)*M(-3)");
    reg.AddUnit(VOLUME_RATIO, METRIC, "LITRE/LITRE", "LITRE*LITRE(-1)");
    }

void AddSpecificVolume(UnitRegistry& reg)
    {
    reg.AddUnit(SPECIFIC_VOLUME, SI, "CUB.M/KG", "M(3)*KG(-1)");
    reg.AddUnit(SPECIFIC_VOLUME, USCUSTOM, "CUB.FT/LBM", "FT(3)*LBM(-1)");
    }

void AddWarpingConstant(UnitRegistry& reg)
    {
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, SI, "M^6", "M(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, METRIC, "MM^6", "MM(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, METRIC, "CM^6", "CM(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, USCUSTOM, "IN^6", "IN(6)");
    reg.AddUnit(TORSIONAL_WARPING_CONSTANT, USCUSTOM, "FT^6", "FT(6)");
    }

void AddApparentPower(UnitRegistry& reg)
    {
    reg.AddUnit(APPARENT_POWER, SI, "VA", "VOLT*A");
    reg.AddUnit(APPARENT_POWER, METRIC, "KVA", "[KILO]*VA");
    reg.AddUnit(APPARENT_POWER, METRIC, "MVA", "[MEGA]*VA");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Caleb.Shafer            01/18
//---------------------------------------------------------------------------------------
void UnitRegistry::AddBaseUnits()
    {
    AddUnitForBasePhenomenon("M", LENGTH);
    AddUnitForBasePhenomenon("KG", MASS);
    AddUnitForBasePhenomenon("S", TIME);
    AddUnitForBasePhenomenon("K", TEMPERATURE);
    AddUnitForBasePhenomenon("DELTA_KELVIN", TEMPERATURE_CHANGE);

    AddUnitForBasePhenomenon("A", CURRENT);
    AddUnitForBasePhenomenon("MOL", MOLE); // Where mol is the SI gram mol or gmol.
    AddUnitForBasePhenomenon("CD", LUMINOSITY);
    AddUnitForBasePhenomenon("RAD", ANGLE);

    AddUnitForBasePhenomenon("STERAD", SOLIDANGLE);
    AddUnitForBasePhenomenon("US$", CURRENCY);
    AddUnitForBasePhenomenon("PERSON", CAPITA);

    AddUnitForBasePhenomenon("ONE", NUMBER);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultUnits()
    {
    AddLengths(*this);
    AddLinearRate(*this);
    AddMass(*this);
    AddTime(*this);
    AddTemperature(*this);
    AddTemperatureChange(*this);
    AddLuminousFlux(*this);
    AddIlluminance(*this);
    AddMole(*this);
    AddPerson(*this);
    AddRotationalSpringConstant(*this);
    AddLinearRotationalSpringConstant(*this);
    AddAcceleration(*this);
    AddPlaneAngle(*this);
    AddArea(*this);
    AddSizeLengthRate(*this);
    AddDensity(*this);
    AddForceDensity(*this);
    AddPopulationDensity(*this);
    AddElectricCurrent(*this);
    AddElectricCharge(*this);
    AddElectricPotential(*this);
    AddEnergy(*this);
    AddEnergyDensity(*this);
    AddHeatingValue(*this);
    AddSpecificHeatCapacity(*this);
    AddSpecificHeatCapacityMolar(*this);
    AddVolumeFlowRate(*this);
    AddFrequency(*this);
    AddSurfaceFlowRate(*this);
    AddMassRatio(*this);
    AddMassFlowRate(*this);
    AddParticleFlowRate(*this);
    AddForce(*this);
    AddHeatFlux(*this);
    AddHeatTransfer(*this);
    AddLinearDensity(*this);
    AddLinearLoad(*this);
    AddTorque(*this);
    AddMolarVolume(*this);
    AddMolarConcentration(*this);
    AddMomentOfInertia(*this);
    AddPower(*this);
    AddPressure(*this);
    AddPressureGradient(*this);
    AddPercentage(*this);
    AddSlope(*this);
    AddSurfaceDensity(*this);
    AddThermalConductivity(*this);
    AddThermalResistance(*this);
    AddTemperatureGradient(*this);
    AddLinearThermalExpansionCoefficient(*this);
    AddVelocity(*this);
    AddAngularVelocity(*this);
    AddDynamicViscosity(*this);
    AddKinematicViscosity(*this);
    AddVolume(*this);
    AddSpecificVolume(*this);
    AddWarpingConstant(*this);
    AddVolumeRatio(*this);
    AddApparentPower(*this);
    }
