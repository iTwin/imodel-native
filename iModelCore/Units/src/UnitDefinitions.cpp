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
    reg.AddUnit(LENGTH, SI, "MM", "[MILLI]*M");// BISQSecUom)->AddSynonym("MILLIMETRE");
    reg.AddUnit(LENGTH, SI, "CM", "[CENTI]*M");// BISQSecUom)->AddSynonym("CENTIMETRE");
    reg.AddUnit(LENGTH, SI, "DM", "[DECI]*M");// BISQSecUom)->AddSynonym("DECIMETRE");
    reg.AddUnit(LENGTH, SI, "KM", "[KILO]*M");// BISQSecUom)->AddSynonym("KILOMETRE");
    reg.AddUnit(LENGTH, SI, "MU", "[MICRO]*M");// BISQFactOne); //, BISQZeroE10); //, BISQNoDescript); //, BISQSecUom)->AddSynonyms("MICRON", "MICROMETRE");
    reg.AddUnit(LENGTH, SI, "ANGSTROM", "M");// BISQFactOne, -10.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(LENGTH, SI, "FERMI", "[FEMTO]*M");// BISQSecUom)->AddSynonym("FEMTOMETRE");
    reg.AddUnit(LENGTH, IMPERIAL, "IN", "MM", 25.4);// ); //, BISQSecUom)->AddSynonym("INCH");
    reg.AddUnit(LENGTH, IMPERIAL, "FT", "IN", 12.0);// ); //, BISQSecUom)->AddSynonym("FOOT");
    reg.AddUnit(LENGTH, USCUSTOM, "MILLIINCH", "[MILLI]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "MICROINCH", "[MICRO]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILLIFOOT", "[MILLI]*FT");// BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "YRD", "FT", 3.0);// ); //, BISQSecUom)->AddSynonym("YARD");
    reg.AddUnit(LENGTH, SURVEYOR, "CHAIN", "FT", 66.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILE", "YRD", 1760.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE", "M", 1852.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_INCH", "CM", 10000.0 / 3937.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_FOOT", "US_SURVEY_INCH", 12.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_YARD", "US_SURVEY_FOOT", 3.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_MILE", "US_SURVEY_YARD", 1760.0);// ); //, BISQSecUom);

    reg.AddUnit(LENGTH, IMPERIAL, "BARLEYCORN", "IN", (1.0 / 3.0));// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "CUBIT", "IN", 18.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "ELL", "IN", 45.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "FATHOM", "FT", 6.0);// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_SEC", "[C]*SEC");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_MIN", "[C]*MIN");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_HOUR", "[C]*HR");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_YEAR", "[C]*YR");// ); //, BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "AU", "M", 1.495978707, 11.0);// ); //, BISQNoDescript); //, BISQSecUom);
    }

void AddMass(UnitRegistry& reg)
    {
    reg.AddUnit(MASS, SI, "G", "[MILLI]*KG"); //, BISQSecUom)->AddSynonym("GRAM");
    reg.AddUnit(MASS, SI, "MG", "[MILLI]*G"); //, BISQSecUom)->AddSynonym("MILLIGRAM");
    reg.AddUnit(MASS, SI, "MKG", "[MICRO]*G"); //, BISQSecUom)->AddSynonym("MICROGRAM");
    reg.AddUnit(MASS, SI, "NG", "[NANO]*G"); //, BISQSecUom)->AddSynonym("NANOGRAM");
    reg.AddUnit(MASS, SI, "TON", "[KILO]*KG"); //, BISQSecUom);
    reg.AddUnit(MASS, IMPERIAL, "LBM", "KG", 0.45359237); //, BISQSecUom)->AddSynonyms("POUND_MASS", "POUND");
    reg.AddUnit(MASS, IMPERIAL, "SLUG", "LBF*S(2)*FT(-1)"); //, BISQSecUom)->AddSynonym("GEEPOUND");
    reg.AddUnit(MASS, USCUSTOM, "GRAIN", "LBM", 1.0 / 7000.0); //, BISQSecUom);
    }

void AddTime(UnitRegistry& reg)
    {
    reg.AddUnit(TIME, SI, "MIN", "S", 60.0); //, BISQSecUom)->AddSynonym("MINUTE");
    reg.AddUnit(TIME, SI, "HR", "MIN", 60.0); //, BISQSecUom)->AddSynonym("HOUR");
    reg.AddUnit(TIME, SI, "DAY", "HR", 24.0); //, BISQSecUom);
    reg.AddUnit(TIME, SI, "WEEK", "DAY", 7.0); //, BISQSecUom);
    reg.AddUnit(TIME, SI, "MONTH", "DAY", 30.0); //, BISQSecUom);
    reg.AddUnit(TIME, SI, "YR", "DAY", 365.25); //, BISQSecUom)->AddSynonym("YEAR");
    reg.AddUnit(TIME, SI, "MS", "[MILLI]*S"); //, BISQSecUom)->AddSynonym("MILLISECOND");
    reg.AddUnit(TIME, SI, "MKS", "[MICRO]*S"); //, BISQSecUom)->AddSynonym("MICROSECOND");

    }

void AddTemperature(UnitRegistry& reg)
    {
    reg.AddUnit(TEMPERATURE, SI, "CELSIUS", "#K", 1.0, -273.15); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("DEGREE_CELSIUS");
    //SimpleUOM(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "#CELSIUS", 1.8); //, BISQSecUom)->AddSynonym("DEGREE_FAHRENHEIT");
    reg.AddUnit(TEMPERATURE, USCUSTOM, "FAHRENHEIT", "#CELSIUS", 1.8, 1.0); //, BISQZeroE10, 32.0); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("DEGREE_FAHRENHEIT");
    reg.AddUnit(TEMPERATURE, USCUSTOM, "RANKINE", "#K", 1.8); //, BISQSecUom)->AddSynonym("DEGREE_RANKINE");
    reg.AddUnit(TEMPERATURE, USCUSTOM, "ROMER", "#CELSIUS", 21.0 / 40.0, 7.5); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("DEGREE_ROMER");
    reg.AddUnit(TEMPERATURE, USCUSTOM, "REAMUR", "#CELSIUS", 0.8); //, BISQSecUom)->AddSynonym("DEGREE_REAMUR");

    /* the algorithm will decided whether to use full conversion or delta
    some indicator could be used to explicitly indicate using deltas (like ^ preceding the name)
    this prfeix will be ignored for all units havig additive = 0, and it will be overruled in many formulas
    reg.AddUnit(BISPH_LENGTH, SI, "DELTA_DEGREE_CELSIUS", "K", 1.8, 1.0, 0.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(BISPH_LENGTH, USCUSTOM, "DELTA_DEGREE_FAHRENHEIT", "K", 1.8, 1.0, 0.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(BISPH_LENGTH, SI, "DELTA_DEGREE_KELVIN", "K", 1, 0.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(BISPH_LENGTH, SI, "DELTA_DEGREE_KELVIN_PER_METRE", "K_PER_L", 1, 0.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(BISPH_LENGTH, USCUSTOM, "DELTA_DEGREE_RANKINE", "K", 1.8, 0.0); //, BISQNoDescript); //, BISQSecUom);
    */
    }

void AddLuminosity(UnitRegistry& reg)
    {
    }

void AddMole(UnitRegistry& reg)
    {
    reg.AddUnit(MOLE, SI, "KMOL", "[KILO]*MOL"); //, BISQSecUom)->AddSynonym("KILOMOLE");
    reg.AddUnit(MOLE, SI, "LB-MOLE", "MOL", 453.59237); //, BISQSecUom)->AddSynonym("POUND_MOLE");
    }

void AddCapita(UnitRegistry& reg)
    {
    reg.AddUnit(CAPITA, STATISTICS, "CAPITA", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "CUSTOMER", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "EMPLOYEE", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "STUDENT", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "GUEST", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "RESIDENT", "PERSON"); //, BISQSecUom);
    reg.AddUnit(CAPITA, STATISTICS, "HUNDRED_CAPITA", "PERSON", 100.0); //, BISQSecUom);
    }

void AddFinance(UnitRegistry& reg)
    {
    }

void AddRatio(UnitRegistry& reg)
    {
    }

void AddAcceleration(UnitRegistry& reg)
    {
    reg.AddUnit(ACCELERATION, SI, "M/SEC.SQUARED", "M*S(-2)"); //, BISQPrimUom);
    reg.AddUnit(ACCELERATION, SI, "CM/SEC.SQUARED", "CM*S(-2)"); //, BISQSecUom);
    reg.AddUnit(ACCELERATION, IMPERIAL, "FT/SEC.SQUARED", "FT*S(-2)"); //, BISQSecUom);
    }

void AddPlaneAngle(UnitRegistry& reg)
    {
    reg.AddUnit(ANGLE, SI, "ARC_DEG", "[PI]*RAD", 1.0 / 180.0); //, BISQSecUom)->AddSynonym("DEGREE"); // Check validity of the synonym
    reg.AddUnit(ANGLE, SI, "ARC_MINUTE", "ARC_DEG", 1.0 / 60.0); //, BISQSecUom)->AddSynonym("ANGLE_MINUTE");
    reg.AddUnit(ANGLE, SI, "ARC_SECOND", "ARC_DEG", 1.0 / 3600.0); //, BISQSecUom)->AddSynonym("ANGLE_SECOND");
    reg.AddUnit(ANGLE, SI, "ARC_QUADRANT", "[PI]*RAD", 0.5); //, BISQSecUom)->AddSynonym("ANGLE_QUADRANT");
    reg.AddUnit(ANGLE, SI, "GRAD", "[PI]*RAD", 1.0 / 200.0); //, BISQSecUom)->AddSynonym("GRADIAN");
    reg.AddUnit(ANGLE, SI, "REVOLUTION", "[PI2]"); //, BISQSecUom);
    }

void AddSolidAngle(UnitRegistry& reg)
    {
    }

void AddArea(UnitRegistry& reg)
    {
    reg.AddUnit(AREA, SI, "SQ.M", "M(2)"); //, BISQPrimUom)->AddSynonym("METRE_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.MU", "MU(2)"); //, BISQSecUom)->AddSynonym("MICRON_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.MM", "MM(2)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.CM", "CM(2)"); //, BISQSecUom)->AddSynonym("CENTIMETRE_SQUARED");
    reg.AddUnit(AREA, SI, "SQ.DM", "DM(2)"); //, BISQSecUom);
    reg.AddUnit(AREA, SI, "SQ.KM", "KM(2)"); //, BISQSecUom)->AddSynonym("KILOMETRE_SQUARED");
    reg.AddUnit(AREA, SI, "ARE", "[HECTO]*M(2)"); //, BISQSecUom);
    reg.AddUnit(AREA, SI, "HECTARE", "[HECTO]*ARE"); //, BISQSecUom);
    reg.AddUnit(AREA, IMPERIAL, "SQ.IN", "IN(2)"); //, BISQSecUom)->AddSynonym("INCH_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "SQ.FT", "FT(2)"); //, BISQSecUom)->AddSynonym("FOOT_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "SQ.YRD", "YRD(2)"); //, BISQSecUom)->AddSynonym("YARD_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "SQ.MILE", "MILE(2)"); //, BISQSecUom)->AddSynonym("MILE_SQUARED");
    reg.AddUnit(AREA, IMPERIAL, "ACRE", "CHAIN(2)", 10.0); //, BISQSecUom);
    }

void AddDensity(UnitRegistry& reg)
    {
    reg.AddUnit(DENSITY, SI, "KG/CUB.M", "KG*M(-3)"); //, BISQPrimUom)->AddSynonym("KILOGRAM_PER_METRE_CUBED");
    reg.AddUnit(DENSITY, SI, "KG/CUB.CM", "KG*CM(-3)"); //, BISQSecUom)->AddSynonym("KILOGRAM_PER_CENTIMETRE_CUBED");
    reg.AddUnit(DENSITY, SI, "KG/LITRE", "KG*DM(-3)"); //, BISQSecUom)->AddSynonyms("KILOGRAM_PER_DECIMETRE_CUBED", "KILOGRAM_PER_LITRE");
    }

void AddPopulationDensity(UnitRegistry& reg)
    {
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.M", "ONE*M(-2)"); //, BISQPrimUom)->AddSynonym("PERSON_PER_METRE_SQUARED");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/HECTARE", "ONE*HECTARE(-1)"); //, BISQSecUom)->AddSynonym("PERSON_PER_HECTARE");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.KM", "ONE*KM(-2)"); //, BISQSecUom)->AddSynonym("PERSON_PER_KILOMETRE_SQUARED");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/ACRE", "ONE*ACRE(-1)"); //, BISQSecUom)->AddSynonym("PERSON_PER_ACRE");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.FT", "ONE*FT(-2)"); //, BISQSecUom)->AddSynonym("PERSON_PER_FOOT_SQUARED");
    reg.AddUnit(POPULATION_DENSITY, SI, "PERSON/SQ.MILE", "ONE*MILE(-2)"); //, BISQSecUom)->AddSynonym("PERSON_PER_MILE_SQUARED");
    }

void AddElectriCurrent(UnitRegistry& reg)
    {
    reg.AddUnit(CURRENT, SI, "MILLIAMPERE", "[MILLI]*A"); //, BISQSecUom);
    reg.AddUnit(CURRENT, SI, "MICROAMPERE", "[MICRO]*A"); //, BISQSecUom);
    }

void AddEnergy(UnitRegistry& reg)
    {
    reg.AddUnit(WORK, SI, "J", "N*M"); //, BISQPrimUom)->AddSynonym("JOULE");
    reg.AddUnit(WORK, SI, "KJ", "[KILO]*N*M"); //, BISQSecUom)->AddSynonym("KILOJOULE");
    reg.AddUnit(WORK, SI, "MJ", "[MEGA]*N*M"); //, BISQSecUom)->AddSynonym("MEGAJOULE");
    reg.AddUnit(WORK, SI, "GJ", "[GIGA]*N*M"); //, BISQSecUom)->AddSynonym("GIGAJOULE");
    reg.AddUnit(WORK, USCUSTOM, "FOOT_POUNDAL", "POUNDAL*FT"); //, BISQSecUom);
    reg.AddUnit(WORK, INTERNATIONAL, "BTU", "J", 1.055056, 3.0); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("BRITISH_THERMAL_UNIT");
    reg.AddUnit(WORK, USCUSTOM, "KILOBTU", "[KILO]*BTU"); //, BISQSecUom);

    reg.AddUnit(WORK, INTERNATIONAL, "CAL", "J", 4.1868); //, BISQSecUom)->AddSynonym("CALORIE");
    reg.AddUnit(WORK, INTERNATIONAL, "KCAL", "[KILO]*CAL"); //, BISQSecUom)->AddSynonym("KILOCALORIE");
    reg.AddUnit(WORK, INTERNATIONAL, "CUB.FT_ATM", "ATM*CUB.FT"); //, BISQSecUom)->AddSynonym("CUBIC_FOOT_OF_ATMOSPHERE");
    reg.AddUnit(WORK, INTERNATIONAL, "CUB.YRD_ATM", "ATM*CUB.YRD"); //, BISQSecUom)->AddSynonym("CUBIC_YARD_OF_ATMOSPHERE");
    reg.AddUnit(WORK, USCUSTOM, "WATT_SECOND", "W*S"); //, BISQSecUom);
    reg.AddUnit(WORK, INTERNATIONAL, "KWH", "KW*HOUR"); //, BISQSecUom)->AddSynonym("KILOWATT_HOUR");
    reg.AddUnit(WORK, INTERNATIONAL, "MWH", "MW*HOUR"); //, BISQSecUom)->AddSynonym("MEGAWATT_HOUR");
    reg.AddUnit(WORK, INTERNATIONAL, "GWH", "GW*HOUR"); //, BISQSecUom)->AddSynonym("GIGAWATT_HOUR");
    }


// TODO: Check these phenomenas
void AddEnergyDensity(UnitRegistry& reg)
    {
    reg.AddUnit(HEATING_VALUE, SI, "J/CUB.M", "J*M(-3)"); //, BISQPrimUom)->AddSynonym("JOULE_PER_METRE_CUBED");
    }

void AddHeatingValue(UnitRegistry& reg)
    {
    reg.AddUnit(HEATING_VALUE, SI, "J/KG", "J*M(-3)"); //, BISQPrimUom)->AddSynonym("JOULE_PER_METRE_CUBED"); // TODO: Check
    reg.AddUnit(HEATING_VALUE, USCUSTOM, "BTU/LBM", "BTU*LBM(-1)"); //, BISQSecUom)->AddSynonym("BTU_PER_POUND_MASS");
    reg.AddUnit(HEATING_VALUE, USCUSTOM, "BTU_PER_POUND_MOLE", "M_L2_PER_T2_MOL"); //, BISQSecUom)->AddSynonym("BTU_PER_POUND_MASS");
    }

void AddEnergySpecificCapacity(UnitRegistry& reg)
    {
    // TODO
    }

void AddVolumeFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(FLOW, SI, "CUB.M/SEC", "M(3)*S(-1)"); //, BISQPrimUom)->AddSynonym("METRE_CUBED_PER_SECOND");
    reg.AddUnit(FLOW, SI, "CUB.M/MIN", "M(3)*MIN(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_MINUTE");
    reg.AddUnit(FLOW, SI, "CUB.M/HR", "M(3)*HR(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_HOUR");
    reg.AddUnit(FLOW, SI, "CUB.M/DAY", "M(3)*DAY(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_DAY");
    reg.AddUnit(FLOW, SI, "LITRE/SEC", "LITRE*S(-1)"); //, BISQSecUom)->AddSynonym("LITRE_PER_SECOND");
    reg.AddUnit(FLOW, SI, "LITRE/MIN", "LITRE*MIN(-1)"); //, BISQSecUom)->AddSynonym("LITRE_PER_MINUTE");
    reg.AddUnit(FLOW, SI, "LITRE/HR", "LITRE*HR(-1)"); //, BISQSecUom)->AddSynonym("LITRE_PER_HOUR");
    reg.AddUnit(FLOW, SI, "LITRE/DAY", "LITRE*DAY(-1)"); //, BISQSecUom)->AddSynonym("LITRE_PER_DAY");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/SEC", "CUB.IN*S(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, IMPERIAL, "CUB.IN/MIN", "CUB.IN*MIN(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/SEC", "CUB.FT*S(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_SECOND");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/MIN", "CUB.FT*MIN(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_MINUTE");
    reg.AddUnit(FLOW, IMPERIAL, "CUB.FT/DAY", "CUB.FT*DAY(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_DAY");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/DAY", "ACRE_FOOT*DAY(-1)"); //, BISQSecUom)->AddSynonym("ACRE_FOOT_PER_DAY");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/HR", "ACRE_FOOT*HR(-1)"); //, BISQSecUom)->AddSynonym("ACRE_FOOT_PER_HOUR");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_FOOT/MIN", "ACRE_FOOT*MIN(-1)"); //, BISQSecUom)->AddSynonym("ACRE_FOOT_PER_MINUTE");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/DAY", "ACRE_INCH*DAY(-1)"); //, BISQSecUom)->AddSynonym("ACRE_INCH_PER_DAY");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/HOUR", "ACRE_INCH*HR(-1)"); //, BISQSecUom)->AddSynonym("ACRE_INCH_PER_HOUR");
    reg.AddUnit(FLOW, USCUSTOM, "ACRE_INCH/MIN", "ACRE_INCH*MIN(-1)"); //, BISQSecUom)->AddSynonym("ACRE_INCH_PER_MINUTE");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL_PER_DAY", "GALLON_IMPERIAL*DAY(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL_PER_MINUTE", "GALLON_IMPERIAL*MIN(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, USCUSTOM, "GALLON_IMPERIAL_PER_SECOND", "GALLON_IMPERIAL*S(-1)"); //, BISQSecUom);
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/S", "GALLON*SEC(-1)"); //, BISQSecUom)->AddSynonym("GALLON_PER_SECOND");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/MIN", "GALLON*MIN(-1)"); //, BISQSecUom)->AddSynonym("GALLON_PER_MINUTE");
    reg.AddUnit(FLOW, USCUSTOM, "GALLON/DAY", "GALLON*DAY(-1)"); //, BISQSecUom)->AddSynonym("GALLON_PER_DAY");
    }

void AddFrequency(UnitRegistry& reg)
    {
    reg.AddUnit(FREQUENCY, SI, "HERTZ", "S(-1)"); //, BISQPrimUom);
    reg.AddUnit(FREQUENCY, SI, "KH", "[KILO]*S(-1)"); //, BISQSecUom)->AddSynonym("KILOHERTZ");
    reg.AddUnit(FREQUENCY, SI, "MH", "[MEGA]*S(-1)"); //, BISQSecUom)->AddSynonym("MEGAHERTZ");
    }

void AddSurfaceFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(SECOND*SQ.M)", "M*SEC(-1)"); //, BISQPrimUom)->AddSynonym("METRE_CUBED_PER_METRE_SQUARED_PER_SECOND");
    reg.AddUnit(SURFACE_FLOW_RATE, INDUSTRIAL, "CUB.M/(DAY*SQ.M)", "M*DAY(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_METRE_SQUARED_PER_DAY");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*HECTARE)", "CUB.M*HECTARE(-1)*DAY(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_HECTARE_PER_DAY");
    reg.AddUnit(SURFACE_FLOW_RATE, SI, "CUB.M/(DAY*SQ.KM)", "CUB.M*KM(-2)*DAY(-1)"); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*MIN)", "FT*MIN(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.FT*S)", "FT*S(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "CUB.FT/(SQ.MILE*S)", "FT(3)*MILE(-2)*S(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*ACRE)", "GALLON*DAY(-1)*ACRE(-1)"); //, BISQSecUom)->AddSynonym("GALLON_PER_ACRE_PER_DAY");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*ACRE)", "GALLON*MIN(-1)*ACRE(-1)"); //, BISQSecUom)->AddSynonym("GALLON_PER_ACRE_PER_MINUTE");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.FT)", "GALLON*MIN(-1)*FT(-2)"); //, BISQSecUom)->AddSynonym("GALLON_PER_FOOT_SQUARED_PER_MINUTE");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.FT)", "GALLON*DAY(-1)*FT(-2)"); //, BISQSecUom)->AddSynonym("GALLON_PER_FOOT_SQUARED_PER_DAY");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(MIN*SQ.MILE)", "GALLON*MIN(-1)*MILE(-2)"); //, BISQSecUom)->AddSynonym("GALLON_PER_MILE_SQUARED_PER_MINUTE");
    reg.AddUnit(SURFACE_FLOW_RATE, USCUSTOM, "GALLON/(DAY*SQ.MILE)", "GALLON*DAY(-1)*MILE(-2)"); //, BISQSecUom)->AddSynonym("GALLON_PER_MILE_SQUARED_PER_DAY");
    }

void AddMassFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(MASS_FLOW, SI, "KG/S", "KG*S(-1)"); //, BISQPrimUom)->AddSynonym("KILOGRAM_PER_SECOND");
    reg.AddUnit(MASS_FLOW, SI, "KG/MIN", "KG*MIN(-1)"); //, BISQSecUom)->AddSynonym("KILOGRAM_PER_MINUTE");
    reg.AddUnit(MASS_FLOW, SI, "KG/HR", "KG*HR(-1)"); //, BISQSecUom)->AddSynonym("KILOGRAM_PER_HOUR");
    reg.AddUnit(MASS_FLOW, SI, "KG/DAY", "KG*DAY(-1)"); //, BISQSecUom)->AddSynonym("KILOGRAM_PER_DAY");

    reg.AddUnit(MASS_FLOW, SI, "G/S", "GRAM*S(-1)"); //, BISQSecUom)->AddSynonym("GRAM_PER_SECOND");
    reg.AddUnit(MASS_FLOW, SI, "G/MIN", "GRAM*M(-1)"); //, BISQSecUom)->AddSynonym("GRAM_PER_MINUTE");
    reg.AddUnit(MASS_FLOW, SI, "G/HR", "GRAM*HR(-1)"); //, BISQSecUom)->AddSynonym("GRAM_PER_HOUR");
    reg.AddUnit(MASS_FLOW, SI, "MG/S", "MG*S(-1)"); //, BISQSecUom)->AddSynonym("MILLIGRAM_PER_SECOND");
    reg.AddUnit(MASS_FLOW, SI, "MG/MIN", "MG*MIN(-1)"); //, BISQSecUom)->AddSynonym("MILLIGRAM_PER_MINUTE");
    reg.AddUnit(MASS_FLOW, SI, "MG/HR", "MG*HR(-1)"); //, BISQSecUom)->AddSynonym("MILLIGRAM_PER_HOUR");
    reg.AddUnit(MASS_FLOW, SI, "MG/DAY", "MG*DAY(-1)"); //, BISQSecUom)->AddSynonym("MILLIGRAM_PER_DAY");
    reg.AddUnit(MASS_FLOW, SI, "MKG/S", "MKG*S(-1)"); //, BISQSecUom)->AddSynonym("MICROGRAM_PER_SECOND");
    reg.AddUnit(MASS_FLOW, SI, "MKG/MIN", "MKG*MIN(-1)"); //, BISQSecUom)->AddSynonym("MICROGRAM_PER_MINUTE");
    reg.AddUnit(MASS_FLOW, SI, "MKG/HR", "MKG*HR(-1)"); //, BISQSecUom)->AddSynonym("MICROGRAM_PER_HOUR");
    reg.AddUnit(MASS_FLOW, SI, "MKG/DAY", "MKG*DAY(-1)"); //, BISQSecUom)->AddSynonym("MICROGRAM_PER_DAY");
    reg.AddUnit(MASS_FLOW, SI, "LB/S", "LBM*S(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_SECOND");
    reg.AddUnit(MASS_FLOW, SI, "LB/MIN", "LBM*MIN(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_MINUTE");
    reg.AddUnit(MASS_FLOW, SI, "LB/HR", "LBM*HR(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_HOUR");
    reg.AddUnit(MASS_FLOW, SI, "LB/DAY", "LBM*DAY(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_DAY");
    }

void AddParticleFlowRate(UnitRegistry& reg)
    {
    reg.AddUnit(PARTICLE_FLOW, SI, "MOL/S", "MOL*S(-1)"); //, BISQPrimUom)->AddSynonym("MOLE_PER_SECOND");
    reg.AddUnit(PARTICLE_FLOW, SI, "KMOL/S", "[KILO]*MOL*S(-1)"); //, BISQSecUom)->AddSynonym("KILOMOLE_PER_SECOND");
    }

void AddForce(UnitRegistry& reg)
    {
    reg.AddUnit(FORCE, SI, "N", "KG*M*S(-2)"); //, BISQPrimUom)->AddSynonym("NEWTON");
    reg.AddUnit(FORCE, SI, "KN", "[KILO]*N"); //, BISQSecUom)->AddSynonym("KILONEWTON");
    reg.AddUnit(FORCE, SI, "KGF", "[G]*KG"); //, BISQSecUom)->AddSynonyms("KILOGRAM_FORCE", "KILOPOND");
    reg.AddUnit(FORCE, SI, "LBF", "[G]*LBM"); //, BISQSecUom)->AddSynonym("POUND_FORCE");
    reg.AddUnit(FORCE, SI, "KPF", "[KILO]*LBF"); //, BISQSecUom)->AddSynonym("KILOPOUND_FORCE");
    reg.AddUnit(FORCE, SI, "DYNE", "G*CM*S(-2)"); //, BISQSecUom);
    reg.AddUnit(FORCE, SI, "PDL", "LBM*FT*S(-2)"); //, BISQSecUom)->AddSynonyms("POUNDAL");
    }

void AddHeatFlux(UnitRegistry& reg)
    {
    reg.AddUnit(HEATFLUX, SI, "W/SQ.M", "W*M(-2)"); //, BISQPrimUom);
    }

void AddHeatTransfer(UnitRegistry& reg)
    {
    reg.AddUnit(HEATRASNFER, SI, "W/(SQ.M*K)", "W*M(-2)*K(-1)"); //, BISQPrimUom)->AddSynonym("WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN");
    reg.AddUnit(HEATRASNFER, SI, "W/(SQ.M*CELSIUS)", "W*M(-2)*CELSIUS(-1)"); //, BISQSecUom)->AddSynonym("WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS");
    }

void AddLinearDensity(UnitRegistry& reg)
    {
    reg.AddUnit(LINEARDENSITY, SI, "KG/M", "KG*M(-1)"); //, BISQPrimUom)->AddSynonym("KILOGRAM_PER_METRE");
    }

void AddLinearLoad(UnitRegistry& reg)
    {
    reg.AddUnit(LINEAR_LOAD, SI, "N/M", "N*M(-1)"); //, BISQPrimUom)->AddSynonym("NEWTON_PER_METRE");
    reg.AddUnit(LINEAR_LOAD, SI, "N/MM", "N*MM(-1)"); //, BISQSecUom)->AddSynonym("NEWTON_PER_MILLIMETRE");
    }

void AddTorque(UnitRegistry& reg)
    {
    reg.AddUnit(TORQUE, SI, "N_M", "N*M"); //, BISQPrimUom)->AddSynonym("NEWTON_METRE");
    reg.AddUnit(TORQUE, SI, "N_CM", "N*CM"); //, BISQSecUom)->AddSynonym("NEWTON_CENTIMETRE");
    }

void AddMolarVolume(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/MOLE", "CUB.M*MOLE(-1)"); //, BISQPrimUom)->AddSynonym("METRE_CUBED_PER_MOLE");
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.M/KMOL", "CUB.M*MOLE(-1)", 1.0, -3.0); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("METRE_CUBED_PER_KILOMOLE");
    reg.AddUnit(MOLAR_VOLUME, CHEMISTRY, "CUB.FT/MOLE", "CUB.FT*MOLE(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_POUND_MOLE");
    }

void AddMolarConcentration(UnitRegistry& reg)
    {
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.M", "MOLE*CUB.M(-1)"); //, BISQPrimUom)->AddSynonyms("MOLE_PER_METRE_CUBED", "MILLIMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, SI, "KMOL/CUB.M", "[KILO]*MOLE*CUB.M(-1)"); //, BISQSecUom)->AddSynonym("KILOMOLE_PER_METRE_CUBED");  // deprecate
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.DM", "MOLE*CUB.DM(-1)"); //, BISQSecUom)->AddSynonyms("MOLE_PER_LITRE", "MOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MICROMOL/CUB.DM", "[MICRO]*MOLE*CUB.DM(-1)"); //, BISQSecUom)->AddSynonym("MICROMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "NMOL/CUB.DM", "[NANO]*MOLE*CUB.DM(-1)"); //, BISQSecUom)->AddSynonym("NANOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "PICOMOL/CUB.DM", "[PICO]*MOLE*CUB.DM(-1)"); //, BISQSecUom)->AddSynonym("PICOMOLAR");
    reg.AddUnit(MOLAR_CONCENTRATION, CHEMISTRY, "MOL/CUB.FT", "MOLE*CUB.FT(-1)"); //, BISQSecUom)->AddSynonym("MOLE_PER_FOOT_CUBED");
    }

void AddMomentOfInertia(UnitRegistry& reg)
    {
    reg.AddUnit(MOMENTINERTIA, USCUSTOM, "M^4", "M(4)"); //, BISQPrimUom)->AddSynonym("METRE_TO_THE_FOURTH");
    reg.AddUnit(MOMENTINERTIA, INDUSTRIAL, "CM^4", "CM(4)"); //, BISQSecUom)->AddSynonym("CENTIMETRE_TO_THE_FOURTH");
    reg.AddUnit(MOMENTINERTIA, INDUSTRIAL, "IN^4", "IN(4)"); //, BISQSecUom)->AddSynonym("INCH_TO_THE_FOURTH");
    reg.AddUnit(MOMENTINERTIA, INDUSTRIAL, "FT^4", "FT(4)"); //, BISQSecUom)->AddSynonym("FOOT_TO_THE_FOURTH");
    }

void AddPower(UnitRegistry& reg)
    {
    reg.AddUnit(POWER, INTERNATIONAL, "W", "N*M*S(-1)"); //, BISQPrimUom)->AddSynonym("WATT");
    reg.AddUnit(POWER, INTERNATIONAL, "KW", "[KILO]*W"); //, BISQSecUom)->AddSynonym("KILOWATT");
    reg.AddUnit(POWER, INTERNATIONAL, "MW", "[MEGA]*W"); //, BISQSecUom)->AddSynonym("MEGAWATT");
    reg.AddUnit(POWER, INTERNATIONAL, "GW", "[GIGA]*W"); //, BISQSecUom)->AddSynonym("GIGAWATT");
    reg.AddUnit(POWER, INTERNATIONAL, "BTU/MONTH", "BTU*MONTH(-1)"); //, BISQSecUom)->AddSynonym("BTU_PER_MONTH");
    reg.AddUnit(POWER, INTERNATIONAL, "BTU/HOUR", "BTU*HOUR(-1)"); //, BISQSecUom)->AddSynonym("BTU_PER_HOUR");
    reg.AddUnit(POWER, USCUSTOM, "KILOBTU/HOUR", "[KILO]BTU*HOUR(-1)"); //, BISQSecUom);
    reg.AddUnit(POWER, USCUSTOM, "HP", "LBF*FT", 550.0); //, BISQSecUom)->AddSynonym("HORSEPOWER");
    }

// TODO: Wrong
//void AddPowerDensity(UnitRegistry& reg)
//    {
//    reg.AddUnit(BISPH_LENGTH, USCUSTOM, "BTU_PER_HOUR_PER_FOOT_CUBED", "M_PER_L_T3", 0.09662109117546, 0.0); //, BISQNoDescript); //, BISQSecUom);
//    }

void AddPressure(UnitRegistry& reg)
    {
    reg.AddUnit(PRESSURE, SI, "PA", "N*M(-2)"); //, BISQPrimUom)->AddSynonym("PASCAL");
    reg.AddUnit(PRESSURE, SI, "HECTOPASCAL", "[HECTO]*PA"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, SI, "KILOPASCAL", "[KILO]*PA"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, SI, "KILOPASCAL_GAUGE", "[KILO]*PA", 1.0, 1.0); //, BISQNoDescript); //, BISQSecUom);

    reg.AddUnit(PRESSURE, INTERNATIONAL, "AT", "KGF*CM(-2)"); //, BISQSecUom)->AddSynonyms("ATMOSPHERE", "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED");
    reg.AddUnit(PRESSURE, INTERNATIONAL, "KGF/SQ.M", "KGF*M(-2)"); //, BISQSecUom)->AddSynonym("KILOGRAM_FORCE_PER_METRE_SQUARED");
    reg.AddUnit(PRESSURE, SI, "ATM", "PA", 1.01325, 5.0); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("STANDARD_ATMOSPHERE");
    reg.AddUnit(PRESSURE, SI, "BAR", "PA", 1.0, 5.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(PRESSURE, SI, "MBAR", "[MILLI]*BAR"); //, BISQSecUom)->AddSynonym("MILLIBAR");
    reg.AddUnit(PRESSURE, CGS, "BARYE", "PA", 0.1); //, BISQSecUom);   // 1.0 dyn/sq.cm
    reg.AddUnit(PRESSURE, SI, "PSI", "LBF*IN(-2)"); //, BISQSecUom)->AddSynonym("POUND_PER_SQUARED_INCH");
    reg.AddUnit(PRESSURE, SI, "KSI", "[KILO]*LBF*IN(-2)"); //, BISQSecUom)->AddSynonym("KILOPOUND_PER_SQUARED_INCH");
    reg.AddUnit(PRESSURE, INDUSTRIAL, "BAR_GAUGE", "BAR", 1.0, -1.0); //, BISQNoDescript); //, BISQSecUom);
    reg.AddUnit(PRESSURE, INDUSTRIAL, "ATM_GAUGE", "AT", 1.0, 1.0332274528); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE");
    reg.AddUnit(PRESSURE, SI, "METRE_OF_H2O_CONVENTIONAL", "[G]*[H2O_0C]*M"); //, BISQSecUom)->AddSynonym("MAQ");
    reg.AddUnit(PRESSURE, SI, "MILLIMETRE_OF_H2O_CONVENTIONAL", "[G]*[H2O_0C]*MM"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "FOOT_OF_H2O_CONVENTIONAL", "[G]*[H2O_4C]*FT"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_32_FAHRENHEIT", "[G]*[H2O_32F]*IN"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", "[G]*[H2O_39.2F]*IN"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_H2O_AT_60_FAHRENHEIT", "[G]*[H2O_60F]*IN"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", "[G]*[HG_0C]*MM"); //, BISQSecUom);
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_HG_CONVENTIONAL", "[G]*[HG_0C]*IN"); //, BISQSecUom)->AddSynonym("INCH_OF_HG_AT_32_FAHRENHEIT");
    reg.AddUnit(PRESSURE, USCUSTOM, "INCH_OF_HG_AT_60_FAHRENHEIT", "[G]*[HG_60F]*IN"); //, BISQSecUom);
    }

void AddPressureGradient(UnitRegistry& reg)
    {
    reg.AddUnit(PRESSURE_GRADIENT, SI, "PA/M", "PA*M(-1)"); //, BISQPrimUom)->AddSynonym("PASCAL_PER_METRE");
    reg.AddUnit(PRESSURE_GRADIENT, SI, "BAR/KM", "BAR*KM(-1)"); //, BISQSecUom)->AddSynonym("BAR_PER_KILOMETRE");
    }

void AddSlope(UnitRegistry& reg)
    {
    reg.AddUnit(SLOPE, USCUSTOM, "M/M", "M*M(-1)"); //, BISQPrimUom)->AddSynonyms("METRE_PER_METRE", "METRE_HORIZONTAL_PER_METRE_VERTICAL", "METRE_VERTICAL_PER_METRE_HORIZONTAL");
    reg.AddUnit(SLOPE, USCUSTOM, "CM/M", "CM*M(-1)"); //, BISQSecUom)->AddSynonym("CENTIMETRE_PER_METRE");
    reg.AddUnit(SLOPE, USCUSTOM, "MM/M", "MM*M(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_METRE");
    reg.AddUnit(SLOPE, SI, "M/KM", "M*KM(-1)"); //, BISQSecUom)->AddSynonym("METRE_PER_KILOMETRE");
    reg.AddUnit(SLOPE, USCUSTOM, "FOOT_PER_1000_FOOT", "FT*FT(-1)", 1000); //, BISQSecUom);
    reg.AddUnit(SLOPE, USCUSTOM, "FT/FT", "FT*FT(-1)"); //, BISQSecUom)->AddSynonyms("FOOT_PER_FOOT", "FOOT_VERTICAL_PER_FOOT_HORIZONTAL", "FOOT_HORIZONTAL_PER_FOOT_VERTICAL");
    reg.AddUnit(SLOPE, USCUSTOM, "IN/FT", "IN*FT(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_FOOT");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/IN", "FT*IN(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_INCH");
    reg.AddUnit(SLOPE, USCUSTOM, "FT/MILE", "FT*MILE(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_MILE");
    }

void AddSurfaceDensity(UnitRegistry& reg)
    {
    reg.AddUnit(SURFACEDENSITY, SI, "KG/SQ.M", "KG*M(-2)"); //, BISQPrimUom)->AddSynonym("KILOGRAM_PER_METRE_SQUARED");
    reg.AddUnit(SURFACEDENSITY, SI, "G/SQ.M", "G*M(-2)"); //, BISQSecUom)->AddSynonym("GRAM_PER_METRE_SQUARED");
    reg.AddUnit(SURFACEDENSITY, SI, "KG/HECTARE", "KG*HECTARE(-1)"); //, BISQSecUom)->AddSynonym("KILOGRAM_PER_HECTARE");
    reg.AddUnit(SURFACEDENSITY, USCUSTOM, "LB/ACRE", "LBM*ACRE(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_ACRE");
    }

void AddThermalConductivity(UnitRegistry& reg)
    {
    reg.AddUnit(THERMOCONDUCT, USCUSTOM, "W/(M*K)", "W*M(-1)*K(-1)"); //, BISQPrimUom)->AddSynonym("WATT_PER_METRE_PER_DEGREE_KELVIN");
    reg.AddUnit(THERMOCONDUCT, USCUSTOM, "BTU/(HR*FT*FAHRENHEIT)", "BTU*HR(-1)*IN*FT(-2)*FAHRENHEIT(-1)"); //, BISQSecUom)->AddSynonym("BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT");
    reg.AddUnit(THERMOCONDUCT, USCUSTOM, "BTU_PER_HOUR_PER_FOOT_PER_DELTA_DEGREE_FAHRENHEIT", "BTU*HR(-1)*FT(-1)*FAHRENHEIT(-1)"); //, BISQSecUom);
    //LoadUOM(THERMOCONDUCT, INDUSTRIAL, "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", "M_PER_T3_K", 0.1761101836823, 0.0); //, BISQNoDescript); //, BISQSecUom);
    //LoadUOM(THERMOCONDUCT, USCUSTOM, "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", "M_L_PER_T3_K", 6.933471798516, 0.0); //, BISQNoDescript); //, BISQSecUom)->AddSynonym("BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT");
    }

void AddThreadPitch(UnitRegistry& reg)
    {
    reg.AddUnit(THREAD_PITCH, SI, "M/REVOLUTION", "M*REVOLUTION(-1)"); //, BISQPrimUom)->AddSynonym("METRE_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, SI, "CM/REVOLUTION", "CM*REVOLUTION(-1)"); //, BISQSecUom)->AddSynonym("CENTIMETRE_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, SI, "MM/REVOLUTION", "MM*REVOLUTION(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "IN/REVOLUTION", "IN*REVOLUTION(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "FT/REVOLUTION", "FT*REVOLUTION(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_REVOLUTION");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "MM/RAD", "MM*RAD(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_RADIAN");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/DEGREE", "IN*ARC_DEG(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_DEGREE");
    reg.AddUnit(THREAD_PITCH, USCUSTOM, "INCH/RAD", "IN*RAD(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_RADIAN");
    }

void AddVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(VELOCITY, SI, "M/S", "M*S(-1)"); //, BISQPrimUom)->AddSynonyms("METER/SECOND", "METRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "M/MIN", "M*MIN(-1)"); //, BISQSecUom)->AddSynonym("METRE_PER_MINUTE");
    reg.AddUnit(VELOCITY, SI, "M/HR", "M*HR(-1)"); //, BISQSecUom)->AddSynonym("METRE_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "M/DAy", "M*DAY(-1)"); //, BISQSecUom)->AddSynonym("METRE_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "MM/S", "MM*S(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "MM/MIN", "MM*MIN(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_MINUTE");
    reg.AddUnit(VELOCITY, SI, "MM/HR", "MM*HR(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "MM/DAY", "MM*DAY(-1)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "CM/S", "CM*S(-1)"); //, BISQSecUom)->AddSynonyms("CENTIMETER/SECOND", "CENTIMETRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "CM/MIN", "CM*MIN(-1)"); //, BISQSecUom)->AddSynonyms("CENTIMETER/MINUTE", "CENTIMETRE_PER_MINUTE");
    reg.AddUnit(VELOCITY, SI, "CM/HOUR", "CM*HR(-1)"); //, BISQSecUom)->AddSynonyms("CENTIMETER/HOUR", "CENTIMETRE_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "CM/DAY", "CM*DAY(-1)"); //, BISQSecUom)->AddSynonyms("CENTIMETER/DAY", "CENTIMETRE_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "KM/S", "KM*S(-1)"); //, BISQSecUom)->AddSynonym("KILOMETRE_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "KM/HR", "KM*HR(-1)"); //, BISQSecUom)->AddSynonym("KILOMETRE_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "IN/SEC", "IN*SEC(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "IN/MIN", "IN*MIN(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_MINUTE");
    reg.AddUnit(VELOCITY, SI, "IN/HR", "IN*HR(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "IN/DAY", "IN*DAY(-1)"); //, BISQSecUom)->AddSynonym("INCH_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "FT/SEC", "FT*SEC(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_SECOND");
    reg.AddUnit(VELOCITY, SI, "FT/MIN", "FT*MIN(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_MINUTE");
    reg.AddUnit(VELOCITY, SI, "FT/HR", "FT*HR(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_HOUR");
    reg.AddUnit(VELOCITY, SI, "FT/DAY", "FT*DAY(-1)"); //, BISQSecUom)->AddSynonym("FOOT_PER_DAY");
    reg.AddUnit(VELOCITY, SI, "YRD/SEC", "YRD*SEC(-1)"); //, BISQSecUom);
    reg.AddUnit(VELOCITY, SI, "MPH", "MILE*HR(-1)"); //, BISQSecUom)->AddSynonym("MILE_PER_HOUR");
    }

void AddAngularVelocity(UnitRegistry& reg)
    {
    reg.AddUnit(ANGVELOCITY, SI, "RAD/S", "RAD*S(-1)"); //, BISQPrimUom)->AddSynonyms("RADIAN/SECOND", "RADIAN_PER_SECOND");
    reg.AddUnit(ANGVELOCITY, SI, "RAD/MIN", "RAD*MIN(-1)"); //, BISQSecUom)->AddSynonyms("RADIAN/MINUTE", "RADIAN_PER_MINUTE");
    reg.AddUnit(ANGVELOCITY, SI, "RAD/HR", "RAD*HR(-1)"); //, BISQSecUom)->AddSynonyms("RADIAN/HOUR", "RADIAN_PER_HOUR");
    reg.AddUnit(ANGVELOCITY, SI, "RPS", "[PI]*RAD*S(-1)", 1.0); //, BISQSecUom)->AddSynonym("CYCLE_PER_SECOND");
    reg.AddUnit(ANGVELOCITY, SI, "RPM", "[PI]*RAD*MIN(-1)", 1.0); //, BISQSecUom)->AddSynonym("CYCLE_PER_MINUTE");
    reg.AddUnit(ANGVELOCITY, SI, "RPH", "[PI]*RAD*HOUR(-1)", 1.0); //, BISQSecUom)->AddSynonym("CYCLE_PER_HOUR");
    reg.AddUnit(ANGVELOCITY, SI, "DEG/S", "ARC_DEG*S(-1)", 1.0); //, BISQSecUom)->AddSynonyms("ARC_DEG/S", "DEGREE_PER_SECOND");
    reg.AddUnit(ANGVELOCITY, SI, "DEG/MIN", "ARC_DEG*MIN(-1)", 1.0); //, BISQSecUom)->AddSynonyms("ARC_DEG/MIN", "DEGREE_PER_MINUTE");
    reg.AddUnit(ANGVELOCITY, SI, "DEG/HR", "ARC_DEG*HR(-1)", 1.0); //, BISQSecUom)->AddSynonyms("ARC_DEG/HOUR", "DEGREE_PER_HOUR");
    }

void AddDynamicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(DYNVISCOSITY, SI, "PA-S", "PA*S"); //, BISQPrimUom)->AddSynonym("PASCAL-SECOND");
    reg.AddUnit(DYNVISCOSITY, SI, "POISE", "PA-S", 0.1); //, BISQSecUom);
    reg.AddUnit(DYNVISCOSITY, SI, "CENTIPOISE", "[CENTI]*POISE"); //, BISQSecUom);
    reg.AddUnit(DYNVISCOSITY, SI, "LB/FT*S", "LBF*FT(-1)*S(-1)"); //, BISQSecUom)->AddSynonym("POUND_PER_FOOT_SECOND");
    }

void AddKinematicViscosity(UnitRegistry& reg)
    {
    reg.AddUnit(KINVISCOSITY, SI, "SQ.M/S", "M(2)*S(-1)"); //, BISQPrimUom)->AddSynonym("SQUARE_METRE_PER_SECOND");
    reg.AddUnit(KINVISCOSITY, SI, "SQ.FT/S", "FT(2)*S(-1)"); //, BISQSecUom)->AddSynonym("FOOT_SQUARED_PER_SECOND");
    reg.AddUnit(KINVISCOSITY, SI, "STOKES", "CM(2)*S(-1)"); //, BISQSecUom);
    reg.AddUnit(KINVISCOSITY, SI, "CENTISTOKE", "MM(2)*S(-1)"); //, BISQSecUom);
    }

void AddVolume(UnitRegistry& reg)
    {
    reg.AddUnit(VOLUME, SI, "CUB.M", "M(3)"); //, BISQPrimUom)->AddSynonym("METRE_CUBED");
    reg.AddUnit(VOLUME, SI, "CUB.MU", "MU(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "CUB.MM", "MM(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "CUB.CM", "CM(3)"); //, BISQSecUom)->AddSynonyms("CUBIC_CENTIMETRE", "CENTIMETRE_CUBED");
    reg.AddUnit(VOLUME, SI, "CUB.DM", "DM(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "CUB.KM", "KM(3)"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "LITRE", "CUB.DM"); //, BISQSecUom);
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_GALLON", "[MEGA]*GALLON"); //, BISQSecUom);
    reg.AddUnit(VOLUME, INDUSTRIAL, "MILLION_LITRE", "[MEGA]*LITRE"); //, BISQSecUom);
    reg.AddUnit(VOLUME, SI, "MICROLITRE", "[MICRO]*LITRE"); //, BISQSecUom)->AddSynonym("LAMBDA");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.IN", "IN(3)"); //, BISQSecUom)->AddSynonym("INCH_CUBED");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.FT", "FT(3)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.YRD", "YRD(3)"); //, BISQSecUom)->AddSynonym("YARD_CUBED");
    reg.AddUnit(VOLUME, IMPERIAL, "CUB.MILE", "MILE(3)"); //, BISQSecUom)->AddSynonym("MILE_CUBED");
    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_INCH", "ACRE*IN"); //, BISQSecUom);
    reg.AddUnit(VOLUME, IMPERIAL, "ACRE_FOOT", "ACRE*FT"); //, BISQSecUom);
    reg.AddUnit(VOLUME, USCUSTOM, "GALLON", "IN(3)", 231.0); //, BISQSecUom);
    reg.AddUnit(VOLUME, IMPERIAL, "GALLON_IMPERIAL", "LITRE", 4.54609); //, BISQSecUom);
    }

void AddSpecificVolume(UnitRegistry& reg)
    {
    reg.AddUnit(SPECVOLUME, USCUSTOM, "CUB.M/KG", "M(3)*KG(-1)"); //, BISQPrimUom)->AddSynonym("METRE_CUBED_PER_KILOGRAM");
    reg.AddUnit(SPECVOLUME, USCUSTOM, "CUB.FT/LB", "FT(3)*LBM(-1)"); //, BISQSecUom)->AddSynonym("FOOT_CUBED_PER_POUND_MASS");
    }

void AddWarpingConstant(UnitRegistry& reg)
    {
    reg.AddUnit(WARPING_CONSTANT, INDUSTRIAL, "M^6", "M(6)"); //, BISQPrimUom)->AddSynonym("METRE_TO_THE_SIXTH");
    reg.AddUnit(WARPING_CONSTANT, INDUSTRIAL, "MM^6", "MM(6)"); //, BISQSecUom)->AddSynonym("MILLIMETRE_TO_THE_SIXTH");
    reg.AddUnit(WARPING_CONSTANT, INDUSTRIAL, "CM^6", "CM(6)"); //, BISQSecUom)->AddSynonym("CENTIMETRE_TO_THE_SIXTH");
    reg.AddUnit(WARPING_CONSTANT, INDUSTRIAL, "IN^6", "IN(6)"); //, BISQSecUom)->AddSynonym("INCH_TO_THE_SIXTH");
    reg.AddUnit(WARPING_CONSTANT, INDUSTRIAL, "FT^6", "FT(6)"); //, BISQSecUom)->AddSynonym("FOOT_TO_THE_SIXTH");
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultUnits ()
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    reg.AddSIBaseUnit("M", BasePhenomena::Length); // BISQPrimUom)->AddSynonym("METRE");
    reg.AddSIBaseUnit("KG", BasePhenomena::Mass); //, BISQPrimUom)->AddSynonym("KILOGRAM");
    reg.AddSIBaseUnit("S", BasePhenomena::Time); //, BISQPrimUom)->AddSynonyms("SEC", "SECOND");
    reg.AddSIBaseUnit("K", BasePhenomena::Temperature); //, BISQPrimUom)->AddSynonyms("KELVIN", "DEGREE_KELVIN");
    reg.AddSIBaseUnit("A", BasePhenomena::ElectricCurrent); //, BISQPrimUom);
    reg.AddSIBaseUnit("MOL", BasePhenomena::Mole); //, BISQPrimUom)->AddSynonyms("MOLE", "GMOL", "GRAMMOLE");
    reg.AddSIBaseUnit("CD", BasePhenomena::Luminosity); //, BISQPrimUom)->AddSynonym("CANDELA");
    reg.AddSIBaseUnit("RAD", BasePhenomena::PlaneAngle); //, BISQPrimUom)->AddSynonym("RADIAN");
    reg.AddSIBaseUnit("STERAD", BasePhenomena::SolidAngle); //, BISQPrimUom)->AddSynonym("STERADIAN");
    reg.AddSIBaseUnit("US$", BasePhenomena::Finance); //, BISQPrimUom)->AddSynonyms("DOLLAR", "US_DOLLAR");
    reg.AddSIBaseUnit("PERSON", BasePhenomena::Capita); //, BISQPrimUom);
    reg.AddSIBaseUnit("ONE", BasePhenomena::Ratio); //, BISQPrimUom);


    AddLengths(reg);
    AddMass(reg);
    AddTime(reg);
    AddTemperature(reg);
    AddLuminosity(reg);
    AddMole(reg);
    AddCapita(reg);
    AddFinance(reg);
    AddRatio(reg);
    AddAcceleration(reg);
    AddPlaneAngle(reg);
    AddSolidAngle(reg);
    AddArea(reg);
    AddDensity(reg);
    AddPopulationDensity(reg);
    AddElectriCurrent(reg);
    AddEnergy(reg);
    AddEnergyDensity(reg);
    AddHeatingValue(reg);
    AddEnergySpecificCapacity(reg);
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
    AddSlope(reg);
    AddSurfaceDensity(reg);
    AddThermalConductivity(reg);
    AddThreadPitch(reg);
    AddVelocity(reg);
    AddAngularVelocity(reg);
    AddDynamicViscosity(reg);
    AddKinematicViscosity(reg);
    AddVolume(reg);
    AddSpecificVolume(reg);
    AddWarpingConstant(reg);
    }
