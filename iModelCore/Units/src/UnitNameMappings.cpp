/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitNameMappings.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

UnitNameMappings * UnitNameMappings::s_mappings = nullptr;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
UnitNameMappings::UnitNameMappings()
    {
    AddMappings();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
UnitNameMappings* UnitNameMappings::GetMappings()
    {
    if (nullptr == s_mappings)
        s_mappings = new UnitNameMappings();
    return s_mappings;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetECNameFromNewName(Utf8CP name)
    {
    auto iter = GetMappings()->m_newNameECNameMapping.find(name);
    if (iter == GetMappings()->m_newNameECNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetOldNameFromNewName(Utf8CP name)
    {
    auto iter = GetMappings()->m_newNameOldNameMapping.find(name);
    if (iter == GetMappings()->m_newNameOldNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetNewNameFromOldName(Utf8CP name)
    {
    auto iter = GetMappings()->m_oldNameNewNameMapping.find(name);
    if (iter == GetMappings()->m_oldNameNewNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetNewNameFromECName(Utf8CP name)
    {
    auto iter = GetMappings()->m_ecNamenewNameMapping.find(name);
    if (iter == GetMappings()->m_ecNamenewNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     03/16
//--------------------------------------------------------------------------------------
void UnitNameMappings::AddMapping(Utf8CP oldName, Utf8CP newName)
    {
    // NOTE: New mappings overwrite previously added mappings
    m_oldNameNewNameMapping[oldName] = newName;
    m_newNameOldNameMapping[newName] = oldName;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
void UnitNameMappings::AddECMapping(Utf8CP name, Utf8CP ecName)
    {
    // NOTE: New mappings overwrite previously added mappings
    m_newNameECNameMapping[name] = ecName;
    m_ecNamenewNameMapping[ecName] = name;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     03/16
//--------------------------------------------------------------------------------------
void UnitNameMappings::AddMappings()
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
    AddMapping("LITRE_PER_METRE_SQUARED_PER_SECOND", "LITRE/(SQ.M*SEC)");
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
    AddMapping("LONG_TON", "LONG_TON_MASS");
    AddMapping("MEGAGRAM", "MEGAGRAM");
    AddMapping("SHORT_TON", "SHORT_TON_MASS");
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
    AddMapping("VOLT_AMPERE", "VA");
    AddMapping("KILOVOLT_AMPERE", "KVA");
    AddMapping("MEGAVOLT_AMPERE", "MVA");

    AddECMapping("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "BTU_IN_PER_SQ_FT_HR_FAHRENHEIT");
    AddECMapping("(N*M)/DEG", "N_M_PER_DEG");
    AddECMapping("(N*M)/RAD", "N_M_PER_RAD");
    AddECMapping("(SQ.FT*HR*FAHRENHEIT)/BTU", "SQ_FT_HR_FAHRENHEIT_PER_BTU");
    AddECMapping("(SQ.M*CELSIUS)/WATT", "SQ_M_CELSIUS_PER_WATT");
    AddECMapping("(SQ.M*KELVIN)/WATT", "SQ_M_KELVIN_PER_WATT");
    AddECMapping("2PI", "2PI");
    AddECMapping("360DEG", "360DEG");
    AddECMapping("ACRE_FT/DAY", "ACRE_FT_PER_DAY");
    AddECMapping("ACRE_FT/HR", "ACRE_FT_PER_HR");
    AddECMapping("ACRE_FT/MIN", "ACRE_FT_PER_MIN");
    AddECMapping("ACRE_IN/DAY", "ACRE_IN_PER_DAY");
    AddECMapping("ACRE_IN/HR", "ACRE_IN_PER_HR");
    AddECMapping("ACRE_IN/MIN", "ACRE_IN_PER_MIN");
    AddECMapping("BAR/KM", "BAR_PER_KM");
    AddECMapping("BTU/(LB-MOL*RANKINE)", "BTU_PER_LB_MOL_RANKINE");
    AddECMapping("BTU/(LBM*RANKINE)", "BTU_PER_LBM_RANKINE");
    AddECMapping("BTU/(SQ.FT*HR*FAHRENHEIT)", "BTU_PER_SQ_FT_HR_FAHRENHEIT");
    AddECMapping("BTU/HR", "BTU_PER_HR");
    AddECMapping("BTU/LBM", "BTU_PER_LBM");
    AddECMapping("CM/DAY", "CM_PER_DAY");
    AddECMapping("CM/HR", "CM_PER_HR");
    AddECMapping("CM/M", "CM_PER_M");
    AddECMapping("CM/MIN", "CM_PER_MIN");
    AddECMapping("CM/REVOLUTION", "CM_PER_REVOLUTION");
    AddECMapping("CM/SEC", "CM_PER_SEC");
    AddECMapping("CM/SEC.SQ", "CM_PER_SEC_SQ");
    AddECMapping("CM^4", "CM_P4");
    AddECMapping("CM^6", "CM_P6");
    AddECMapping("CUB.CM", "CUB_CM");
    AddECMapping("CUB.DM", "CUB_DM");
    AddECMapping("CUB.FT", "CUB_FT");
    AddECMapping("CUB.FT/(ACRE*SEC)", "CUB_FT_PER_ACRE_SEC");
    AddECMapping("CUB.FT/(SQ.FT*MIN)", "CUB_FT_PER_SQ_FT_MIN");
    AddECMapping("CUB.FT/(SQ.FT*SEC)", "CUB_FT_PER_SQ_FT_SEC");
    AddECMapping("CUB.FT/(SQ.MILE*SEC)", "CUB_FT_PER_SQ_MILE_SEC");
    AddECMapping("CUB.FT/DAY", "CUB_FT_PER_DAY");
    AddECMapping("CUB.FT/LB-MOL", "CUB_FT_PER_LB_MOL");
    AddECMapping("CUB.FT/LBM", "CUB_FT_PER_LBM");
    AddECMapping("CUB.FT/MIN", "CUB_FT_PER_MIN");
    AddECMapping("CUB.FT/SEC", "CUB_FT_PER_SEC");
    AddECMapping("CUB.IN", "CUB_IN");
    AddECMapping("CUB.IN/MIN", "CUB_IN_PER_MIN");
    AddECMapping("CUB.IN/SEC", "CUB_IN_PER_SEC");
    AddECMapping("CUB.KM", "CUB_KM");
    AddECMapping("CUB.M", "CUB_M");
    AddECMapping("CUB.M/(HECTARE*DAY)", "CUB_M_PER_HECTARE_DAY");
    AddECMapping("CUB.M/(SQ.KM*DAY)", "CUB_M_PER_SQ_KM_DAY");
    AddECMapping("CUB.M/(SQ.M*DAY)", "CUB_M_PER_SQ_M_DAY");
    AddECMapping("CUB.M/(SQ.M*SEC)", "CUB_M_PER_SQ_M_SEC");
    AddECMapping("CUB.M/CUB.M", "CUB_M_PER_CUB_M");
    AddECMapping("CUB.M/DAY", "CUB_M_PER_DAY");
    AddECMapping("CUB.M/HR", "CUB_M_PER_HR");
    AddECMapping("CUB.M/KG", "CUB_M_PER_KG");
    AddECMapping("CUB.M/KMOL", "CUB_M_PER_KMOL");
    AddECMapping("CUB.M/MIN", "CUB_M_PER_MIN");
    AddECMapping("CUB.M/MOL", "CUB_M_PER_MOL");
    AddECMapping("CUB.M/SEC", "CUB_M_PER_SEC");
    AddECMapping("CUB.MILE", "CUB_MILE");
    AddECMapping("CUB.MM", "CUB_MM");
    AddECMapping("CUB.MU", "CUB_MU");
    AddECMapping("CUB.YRD", "CUB_YRD");
    AddECMapping("DEG/HR", "DEG_PER_HR");
    AddECMapping("DEG/MIN", "DEG_PER_MIN");
    AddECMapping("DEG/SEC", "DEG_PER_SEC");
    AddECMapping("FT/DAY", "FT_PER_DAY");
    AddECMapping("FT/FT", "FT_PER_FT");
    AddECMapping("FT/HR", "FT_PER_HR");
    AddECMapping("FT/IN", "FT_PER_IN");
    AddECMapping("FT/MILE", "FT_PER_MILE");
    AddECMapping("FT/MIN", "FT_PER_MIN");
    AddECMapping("FT/REVOLUTION", "FT_PER_REVOLUTION");
    AddECMapping("FT/SEC", "FT_PER_SEC");
    AddECMapping("FT/SEC.SQ", "FT_PER_SEC_SQ");
    AddECMapping("FT/THOUSAND_FOOT", "FT_PER_THOUSAND_FOOT");
    AddECMapping("FT^4", "FT_P4");
    AddECMapping("FT^6", "FT_P6");
    AddECMapping("FT_HORIZONTAL/FT_VERTICAL", "FT_HORIZONTAL_PER_FT_VERTICAL");
    AddECMapping("G/CUB.CM", "G_PER_CUB_CM");
    AddECMapping("G/HR", "G_PER_HR");
    AddECMapping("G/MIN", "G_PER_MIN");
    AddECMapping("G/SEC", "G_PER_SEC");
    AddECMapping("G/SQ.M", "G_PER_SQ_M");
    AddECMapping("GALLON/(ACRE*DAY)", "GALLON_PER_ACRE_DAY");
    AddECMapping("GALLON/(ACRE*MIN)", "GALLON_PER_ACRE_MIN");
    AddECMapping("GALLON/(SQ.FT*DAY)", "GALLON_PER_SQ_FT_DAY");
    AddECMapping("GALLON/(SQ.FT*MIN)", "GALLON_PER_SQ_FT_MIN");
    AddECMapping("GALLON/(SQ.MILE*DAY)", "GALLON_PER_SQ_MILE_DAY");
    AddECMapping("GALLON/(SQ.MILE*MIN)", "GALLON_PER_SQ_MILE_MIN");
    AddECMapping("GALLON/DAY", "GALLON_PER_DAY");
    AddECMapping("GALLON/MIN", "GALLON_PER_MIN");
    AddECMapping("GALLON/SEC", "GALLON_PER_SEC");
    AddECMapping("GALLON_IMPERIAL/DAY", "GALLON_IMPERIAL_PER_DAY");
    AddECMapping("GALLON_IMPERIAL/MIN", "GALLON_IMPERIAL_PER_MIN");
    AddECMapping("GALLON_IMPERIAL/SEC", "GALLON_IMPERIAL_PER_SEC");
    AddECMapping("GRM/LBM", "GRM_PER_LBM");
    AddECMapping("HORIZONTAL/VERTICAL", "HORIZONTAL_PER_VERTICAL");
    AddECMapping("IN/DAY", "IN_PER_DAY");
    AddECMapping("IN/DEGREE", "IN_PER_DEGREE");
    AddECMapping("IN/FT", "IN_PER_FT");
    AddECMapping("IN/HR", "IN_PER_HR");
    AddECMapping("IN/MIN", "IN_PER_MIN");
    AddECMapping("IN/RAD", "IN_PER_RAD");
    AddECMapping("IN/REVOLUTION", "IN_PER_REVOLUTION");
    AddECMapping("IN/SEC", "IN_PER_SEC");
    AddECMapping("IN^4", "IN_P4");
    AddECMapping("IN^6", "IN_P6");
    AddECMapping("IN_H2O@32F", "IN_H2O_32F");
    AddECMapping("IN_H2O@39.2F", "IN_H2O_39_2F");
    AddECMapping("IN_H2O@60F", "IN_H2O_60F");
    AddECMapping("IN_HG@32F", "IN_HG_32F");
    AddECMapping("IN_HG@60F", "IN_HG_60F");
    AddECMapping("J/(KG*K)", "J_PER_KG_K");
    AddECMapping("J/(KMOL*K)", "J_PER_KMOL_K");
    AddECMapping("J/(MOL*K)", "J_PER_MOL_K");
    AddECMapping("J/CUB.M", "J_PER_CUB_M");
    AddECMapping("J/KG", "J_PER_KG");
    AddECMapping("KELVIN/M", "KELVIN_PER_M");
    AddECMapping("KG/CUB.CM", "KG_PER_CUB_CM");
    AddECMapping("KG/CUB.M", "KG_PER_CUB_M");
    AddECMapping("KG/DAY", "KG_PER_DAY");
    AddECMapping("KG/HECTARE", "KG_PER_HECTARE");
    AddECMapping("KG/HR", "KG_PER_HR");
    AddECMapping("KG/KG", "KG_PER_KG");
    AddECMapping("KG/LITRE", "KG_PER_LITRE");
    AddECMapping("KG/M", "KG_PER_M");
    AddECMapping("KG/MIN", "KG_PER_MIN");
    AddECMapping("KG/MM", "KG_PER_MM");
    AddECMapping("KG/SEC", "KG_PER_SEC");
    AddECMapping("KG/SQ.M", "KG_PER_SQ_M");
    AddECMapping("KGF/SQ.M", "KGF_PER_SQ_M");
    AddECMapping("KILOBTU/HR", "KILOBTU_PER_HR");
    AddECMapping("KIP/CUB.FT", "KIP_PER_CUB_FT");
    AddECMapping("KJ/(KMOL*K)", "KJ_PER_KMOL_K");
    AddECMapping("KJ/CUB.M", "KJ_PER_CUB_M");
    AddECMapping("KJ/KG", "KJ_PER_KG");
    AddECMapping("KM/HR", "KM_PER_HR");
    AddECMapping("KM/SEC", "KM_PER_SEC");
    AddECMapping("KMOL/CUB.M", "KMOL_PER_CUB_M");
    AddECMapping("KMOL/SEC", "KMOL_PER_SEC");
    AddECMapping("KN/CUB.FT", "KN_PER_CUB_FT");
    AddECMapping("KN/CUB.M", "KN_PER_CUB_M");
    AddECMapping("KWH/CUB.FT", "KWH_PER_CUB_FT");
    AddECMapping("KWH/CUB.M", "KWH_PER_CUB_M");
    AddECMapping("KWH/MILLION_GALLON", "KWH_PER_MILLION_GALLON");
    AddECMapping("LB-MOL", "LB_MOL");
    AddECMapping("LBF/IN", "LBF_PER_IN");
    AddECMapping("LBF/SQ.FT", "LBF_PER_SQ_FT");
    AddECMapping("LBM/(FT*S)", "LBM_PER_FT_S");
    AddECMapping("LBM/ACRE", "LBM_PER_ACRE");
    AddECMapping("LBM/CUB.FT", "LBM_PER_CUB_FT");
    AddECMapping("LBM/CUB.IN", "LBM_PER_CUB_IN");
    AddECMapping("LBM/DAY", "LBM_PER_DAY");
    AddECMapping("LBM/FT", "LBM_PER_FT");
    AddECMapping("LBM/GALLON", "LBM_PER_GALLON");
    AddECMapping("LBM/GALLON_IMPERIAL", "LBM_PER_GALLON_IMPERIAL");
    AddECMapping("LBM/HR", "LBM_PER_HR");
    AddECMapping("LBM/MILLION_GALLON", "LBM_PER_MILLION_GALLON");
    AddECMapping("LBM/MIN", "LBM_PER_MIN");
    AddECMapping("LBM/SEC", "LBM_PER_SEC");
    AddECMapping("LITRE/(SQ.M*SEC)", "LITRE_PER_SQ_M_SEC");
    AddECMapping("LITRE/DAY", "LITRE_PER_DAY");
    AddECMapping("LITRE/HR", "LITRE_PER_HR");
    AddECMapping("LITRE/LITRE", "LITRE_PER_LITRE");
    AddECMapping("LITRE/MIN", "LITRE_PER_MIN");
    AddECMapping("LITRE/SEC", "LITRE_PER_SEC");
    AddECMapping("LUMEN/SQ.FT", "LUMEN_PER_SQ_FT");
    AddECMapping("M/DAy", "M_PER_DAy");
    AddECMapping("M/DEGREE", "M_PER_DEGREE");
    AddECMapping("M/HR", "M_PER_HR");
    AddECMapping("M/KM", "M_PER_KM");
    AddECMapping("M/M", "M_PER_M");
    AddECMapping("M/MIN", "M_PER_MIN");
    AddECMapping("M/RAD", "M_PER_RAD");
    AddECMapping("M/REVOLUTION", "M_PER_REVOLUTION");
    AddECMapping("M/SEC", "M_PER_SEC");
    AddECMapping("M/SEC.SQ", "M_PER_SEC_SQ");
    AddECMapping("MEGAJ/KG", "MEGAJ_PER_KG");
    AddECMapping("MG/DAY", "MG_PER_DAY");
    AddECMapping("MG/HR", "MG_PER_HR");
    AddECMapping("MG/LITRE", "MG_PER_LITRE");
    AddECMapping("MG/MIN", "MG_PER_MIN");
    AddECMapping("MG/SEC", "MG_PER_SEC");
    AddECMapping("MICROMOL/CUB.DM", "MICROMOL_PER_CUB_DM");
    AddECMapping("MKG/DAY", "MKG_PER_DAY");
    AddECMapping("MKG/HR", "MKG_PER_HR");
    AddECMapping("MKG/LITRE", "MKG_PER_LITRE");
    AddECMapping("MKG/MIN", "MKG_PER_MIN");
    AddECMapping("MKG/SEC", "MKG_PER_SEC");
    AddECMapping("MM/DAY", "MM_PER_DAY");
    AddECMapping("MM/HR", "MM_PER_HR");
    AddECMapping("MM/M", "MM_PER_M");
    AddECMapping("MM/MIN", "MM_PER_MIN");
    AddECMapping("MM/RAD", "MM_PER_RAD");
    AddECMapping("MM/REVOLUTION", "MM_PER_REVOLUTION");
    AddECMapping("MM/SEC", "MM_PER_SEC");
    AddECMapping("MM^4", "MM_P4");
    AddECMapping("MM^6", "MM_P6");
    AddECMapping("MM_HG@32F", "MM_HG_32F");
    AddECMapping("MOL/CUB.DM", "MOL_PER_CUB_DM");
    AddECMapping("MOL/CUB.FT", "MOL_PER_CUB_FT");
    AddECMapping("MOL/CUB.M", "MOL_PER_CUB_M");
    AddECMapping("MOL/SEC", "MOL_PER_SEC");
    AddECMapping("M^4", "M_P4");
    AddECMapping("M^6", "M_P6");
    AddECMapping("M_HORIZONTAL/M_VERTICAL", "M_HORIZONTAL_PER_M_VERTICAL");
    AddECMapping("N/CUB.FT", "N_PER_CUB_FT");
    AddECMapping("N/CUB.M", "N_PER_CUB_M");
    AddECMapping("N/M", "N_PER_M");
    AddECMapping("N/MM", "N_PER_MM");
    AddECMapping("N/RAD", "N_PER_RAD");
    AddECMapping("N/SQ.MM", "N_PER_SQ_MM");
    AddECMapping("NMOL/CUB.DM", "NMOL_PER_CUB_DM");
    AddECMapping("PA-S", "PA_S");
    AddECMapping("PA/M", "PA_PER_M");
    AddECMapping("PERSON/ACRE", "PERSON_PER_ACRE");
    AddECMapping("PERSON/HECTARE", "PERSON_PER_HECTARE");
    AddECMapping("PERSON/SQ.FT", "PERSON_PER_SQ_FT");
    AddECMapping("PERSON/SQ.KM", "PERSON_PER_SQ_KM");
    AddECMapping("PERSON/SQ.M", "PERSON_PER_SQ_M");
    AddECMapping("PERSON/SQ.MILE", "PERSON_PER_SQ_MILE");
    AddECMapping("PI/2", "PI_PER_2");
    AddECMapping("PI/4", "PI_PER_4");
    AddECMapping("PICOMOL/CUB.DM", "PICOMOL_PER_CUB_DM");
    AddECMapping("RAD/HR", "RAD_PER_HR");
    AddECMapping("RAD/MIN", "RAD_PER_MIN");
    AddECMapping("RAD/SEC", "RAD_PER_SEC");
    AddECMapping("SHORT_TON/CUB.FT", "SHORT_TON_PER_CUB_FT");
    AddECMapping("SHORT_TON/HR", "SHORT_TON_PER_HR");
    AddECMapping("SLUG/CUB.FT", "SLUG_PER_CUB_FT");
    AddECMapping("SQ.CHAIN", "SQ_CHAIN");
    AddECMapping("SQ.CM", "SQ_CM");
    AddECMapping("SQ.DM", "SQ_DM");
    AddECMapping("SQ.FT", "SQ_FT");
    AddECMapping("SQ.FT/SEC", "SQ_FT_PER_SEC");
    AddECMapping("SQ.IN", "SQ_IN");
    AddECMapping("SQ.KM", "SQ_KM");
    AddECMapping("SQ.M", "SQ_M");
    AddECMapping("SQ.M/SEC", "SQ_M_PER_SEC");
    AddECMapping("SQ.MILE", "SQ_MILE");
    AddECMapping("SQ.MM", "SQ_MM");
    AddECMapping("SQ.MU", "SQ_MU");
    AddECMapping("SQ.US_SURVEY_CHAIN", "SQ_US_SURVEY_CHAIN");
    AddECMapping("SQ.US_SURVEY_FT", "SQ_US_SURVEY_FT");
    AddECMapping("SQ.US_SURVEY_IN", "SQ_US_SURVEY_IN");
    AddECMapping("SQ.US_SURVEY_MILE", "SQ_US_SURVEY_MILE");
    AddECMapping("SQ.US_SURVEY_YRD", "SQ_US_SURVEY_YRD");
    AddECMapping("SQ.YRD", "SQ_YRD");
    AddECMapping("STRAIN/CELSIUS", "STRAIN_PER_CELSIUS");
    AddECMapping("STRAIN/FAHRENHEIT", "STRAIN_PER_FAHRENHEIT");
    AddECMapping("STRAIN/KELVIN", "STRAIN_PER_KELVIN");
    AddECMapping("STRAIN/RANKINE", "STRAIN_PER_RANKINE");
    AddECMapping("THOUSAND_SQ.FT", "THOUSAND_SQ_FT");
    AddECMapping("TONNE/HR", "TONNE_PER_HR");
    AddECMapping("US$", "US_DOLLAR");
    AddECMapping("VERTICAL/HORIZONTAL", "VERTICAL_PER_HORIZONTAL");
    AddECMapping("W/(M*C)", "W_PER_M_C");
    AddECMapping("W/(M*K)", "W_PER_M_K");
    AddECMapping("W/(SQ.M*CELSIUS)", "W_PER_SQ_M_CELSIUS");
    AddECMapping("W/(SQ.M*K)", "W_PER_SQ_M_K");
    AddECMapping("W/SQ.M", "W_PER_SQ_M");
    AddECMapping("YRD/SEC", "YRD_PER_SEC");
    }

END_BENTLEY_UNITS_NAMESPACE
