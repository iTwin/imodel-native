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
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetECNameFromOldName(Utf8CP name)
    {
    auto newName = TryGetNewNameFromOldName(name);
    if(nullptr == newName)
        return nullptr;
    return TryGetECNameFromNewName(newName);
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
// @bsimethod                                 Kyle.Abramowitz                 03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitNameMappings::TryGetOldNameFromECName(Utf8CP name)
    {
    auto newName = TryGetNewNameFromECName(name);
    if(nullptr == newName)
        return nullptr;
    return TryGetOldNameFromNewName(newName);
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
    AddMapping("BTU_PER_HOUR", "BTU/HR");
    AddMapping("KILOBTU_PER_HOUR", "KILOBTU/HR");
    AddMapping("HORSEPOWER", "HP");
    AddMapping("PASCAL", "PA");
    AddMapping("NEWTON_PER_METRE_SQUARED", "PA");
    AddMapping("NEWTON_PER_MILLIMETRE_SQUARED", "MEGAPASCAL");
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
    AddMapping("CAPITA", "PERSON");
    AddMapping("PERSON", "PERSON");
    AddMapping("CUSTOMER", "PERSON");
    AddMapping("EMPLOYEE", "PERSON");
    AddMapping("GUEST", "PERSON");
    AddMapping("HUNDRED_CAPITA", "HUNDRED_PERSON");
    AddMapping("THOUSAND_CAPITA", "THOUSAND_PERSON");
    AddMapping("PASSENGER", "PERSON");
    AddMapping("RESIDENT", "PERSON");
    AddMapping("STUDENT", "PERSON");
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
    
    AddECMapping("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", "UNITS:BTU_IN_PER_SQ_FT_HR_FAHRENHEIT");
    AddECMapping("(N*M)/DEG", "UNITS:N_M_PER_DEG");
    AddECMapping("(N*M)/RAD", "UNITS:N_M_PER_RAD");
    AddECMapping("(SQ.FT*HR*FAHRENHEIT)/BTU", "UNITS:SQ_FT_HR_FAHRENHEIT_PER_BTU");
    AddECMapping("(SQ.M*CELSIUS)/WATT", "UNITS:SQ_M_CELSIUS_PER_WATT");
    AddECMapping("(SQ.M*KELVIN)/WATT", "UNITS:SQ_M_KELVIN_PER_WATT");
    AddECMapping("2PI", "UNITS:TWO_PI");
    AddECMapping("360DEG", "UNITS:DEG360");
    AddECMapping("A", "UNITS:A");
    AddECMapping("ACRE", "UNITS:ACRE");
    AddECMapping("ACRE_FT", "UNITS:ACRE_FT");
    AddECMapping("ACRE_FT/DAY", "UNITS:ACRE_FT_PER_DAY");
    AddECMapping("ACRE_FT/HR", "UNITS:ACRE_FT_PER_HR");
    AddECMapping("ACRE_FT/MIN", "UNITS:ACRE_FT_PER_MIN");
    AddECMapping("ACRE_IN", "UNITS:ACRE_IN");
    AddECMapping("ACRE_IN/DAY", "UNITS:ACRE_IN_PER_DAY");
    AddECMapping("ACRE_IN/HR", "UNITS:ACRE_IN_PER_HR");
    AddECMapping("ACRE_IN/MIN", "UNITS:ACRE_IN_PER_MIN");
    AddECMapping("ARC_DEG", "UNITS:ARC_DEG");
    AddECMapping("ARC_MINUTE", "UNITS:ARC_MINUTE");
    AddECMapping("ARC_QUADRANT", "UNITS:ARC_QUADRANT");
    AddECMapping("ARC_SECOND", "UNITS:ARC_SECOND");
    AddECMapping("ARE", "UNITS:ARE");
    AddECMapping("AT", "UNITS:AT");
    AddECMapping("AT_GAUGE", "UNITS:AT_GAUGE");
    AddECMapping("ATM", "UNITS:ATM");
    AddECMapping("ATTO", "UNITS:ATTO");
    AddECMapping("BAR", "UNITS:BAR");
    AddECMapping("BAR/KM", "UNITS:BAR_PER_KM");
    AddECMapping("BAR_GAUGE", "UNITS:BAR_GAUGE");
    AddECMapping("BARYE", "UNITS:BARYE");
    AddECMapping("BTU", "UNITS:BTU");
    AddECMapping("BTU/(LB-MOL*RANKINE)", "UNITS:BTU_PER_LB_MOL_RANKINE");
    AddECMapping("BTU/(LBM*RANKINE)", "UNITS:BTU_PER_LBM_RANKINE");
    AddECMapping("BTU/(SQ.FT*HR*FAHRENHEIT)", "UNITS:BTU_PER_SQ_FT_HR_FAHRENHEIT");
    AddECMapping("BTU/HR", "UNITS:BTU_PER_HR");
    AddECMapping("BTU/LBM", "UNITS:BTU_PER_LBM");
    AddECMapping("CD", "UNITS:CD");
    AddECMapping("CELSIUS", "UNITS:CELSIUS");
    AddECMapping("CENTI", "UNITS:CENTI");
    AddECMapping("CENTIPOISE", "UNITS:CENTIPOISE");
    AddECMapping("CENTISTOKE", "UNITS:CENTISTOKE");
    AddECMapping("CHAIN", "UNITS:CHAIN");
    AddECMapping("CM", "UNITS:CM");
    AddECMapping("CM/DAY", "UNITS:CM_PER_DAY");
    AddECMapping("CM/HR", "UNITS:CM_PER_HR");
    AddECMapping("CM/M", "UNITS:CM_PER_M");
    AddECMapping("CM/MIN", "UNITS:CM_PER_MIN");
    AddECMapping("CM/SEC", "UNITS:CM_PER_SEC");
    AddECMapping("CM/SEC.SQ", "UNITS:CM_PER_SEC_SQ");
    AddECMapping("CM^4", "UNITS:CM_TO_THE_FOURTH");
    AddECMapping("CM^6", "UNITS:CM_TO_THE_SIXTH");
    AddECMapping("COULOMB", "UNITS:COULOMB");
    AddECMapping("CUB.CM", "UNITS:CUB_CM");
    AddECMapping("CUB.DM", "UNITS:CUB_DM");
    AddECMapping("CUB.FT", "UNITS:CUB_FT");
    AddECMapping("CUB.FT/(ACRE*SEC)", "UNITS:CUB_FT_PER_ACRE_SEC");
    AddECMapping("CUB.FT/(SQ.FT*MIN)", "UNITS:CUB_FT_PER_SQ_FT_MIN");
    AddECMapping("CUB.FT/(SQ.FT*SEC)", "UNITS:CUB_FT_PER_SQ_FT_SEC");
    AddECMapping("CUB.FT/(SQ.MILE*SEC)", "UNITS:CUB_FT_PER_SQ_MILE_SEC");
    AddECMapping("CUB.FT/DAY", "UNITS:CUB_FT_PER_DAY");
    AddECMapping("CUB.FT/LB-MOL", "UNITS:CUB_FT_PER_LB_MOL");
    AddECMapping("CUB.FT/LBM", "UNITS:CUB_FT_PER_LBM");
    AddECMapping("CUB.FT/MIN", "UNITS:CUB_FT_PER_MIN");
    AddECMapping("CUB.FT/SEC", "UNITS:CUB_FT_PER_SEC");
    AddECMapping("CUB.IN", "UNITS:CUB_IN");
    AddECMapping("CUB.IN/MIN", "UNITS:CUB_IN_PER_MIN");
    AddECMapping("CUB.IN/SEC", "UNITS:CUB_IN_PER_SEC");
    AddECMapping("CUB.KM", "UNITS:CUB_KM");
    AddECMapping("CUB.M", "UNITS:CUB_M");
    AddECMapping("CUB.M/(HECTARE*DAY)", "UNITS:CUB_M_PER_HECTARE_DAY");
    AddECMapping("CUB.M/(SQ.KM*DAY)", "UNITS:CUB_M_PER_SQ_KM_DAY");
    AddECMapping("CUB.M/(SQ.M*DAY)", "UNITS:CUB_M_PER_SQ_M_DAY");
    AddECMapping("CUB.M/(SQ.M*SEC)", "UNITS:CUB_M_PER_SQ_M_SEC");
    AddECMapping("CUB.M/CUB.M", "UNITS:CUB_M_PER_CUB_M");
    AddECMapping("CUB.M/DAY", "UNITS:CUB_M_PER_DAY");
    AddECMapping("CUB.M/HR", "UNITS:CUB_M_PER_HR");
    AddECMapping("CUB.M/KG", "UNITS:CUB_M_PER_KG");
    AddECMapping("CUB.M/KMOL", "UNITS:CUB_M_PER_KMOL");
    AddECMapping("CUB.M/MIN", "UNITS:CUB_M_PER_MIN");
    AddECMapping("CUB.M/MOL", "UNITS:CUB_M_PER_MOL");
    AddECMapping("CUB.M/SEC", "UNITS:CUB_M_PER_SEC");
    AddECMapping("CUB.MILE", "UNITS:CUB_MILE");
    AddECMapping("CUB.MM", "UNITS:CUB_MM");
    AddECMapping("CUB.MU", "UNITS:CUB_UM");
    AddECMapping("CUB.YRD", "UNITS:CUB_YRD");
    AddECMapping("CUSTOMER", "UNITS:PERSON");
    AddECMapping("DAY", "UNITS:DAY");
    AddECMapping("DECA", "UNITS:DECA");
    AddECMapping("DECI", "UNITS:DECI");
    AddECMapping("DECIMAL_PERCENT", "UNITS:DECIMAL_PERCENT");
    AddECMapping("DEG/HR", "UNITS:DEG_PER_HR");
    AddECMapping("DEG/MIN", "UNITS:DEG_PER_MIN");
    AddECMapping("DEG/SEC", "UNITS:DEG_PER_SEC");
    AddECMapping("DELTA_CELSIUS", "UNITS:DELTA_CELSIUS");
    AddECMapping("DELTA_FAHRENHEIT", "UNITS:DELTA_FAHRENHEIT");
    AddECMapping("DELTA_KELVIN", "UNITS:DELTA_KELVIN");
    AddECMapping("DELTA_RANKINE", "UNITS:DELTA_RANKINE");
    AddECMapping("DM", "UNITS:DM");
    AddECMapping("DYNE", "UNITS:DYNE");
    AddECMapping("EMPLOYEE", "UNITS:PERSON");
    AddECMapping("EXA", "UNITS:EXA");
    AddECMapping("FAHRENHEIT", "UNITS:FAHRENHEIT");
    AddECMapping("FEMTO", "UNITS:FEMTO");
    AddECMapping("FT", "UNITS:FT");
    AddECMapping("FT/DAY", "UNITS:FT_PER_DAY");
    AddECMapping("FT/FT", "UNITS:FT_PER_FT");
    AddECMapping("FT/HR", "UNITS:FT_PER_HR");
    AddECMapping("FT/IN", "UNITS:FT_PER_IN");
    AddECMapping("FT/MILE", "UNITS:FT_PER_MILE");
    AddECMapping("FT/MIN", "UNITS:FT_PER_MIN");
    AddECMapping("FT/SEC", "UNITS:FT_PER_SEC");
    AddECMapping("FT/SEC.SQ", "UNITS:FT_PER_SEC_SQ");
    AddECMapping("FT/THOUSAND_FOOT", "UNITS:FT_PER_THOUSAND_FOOT");
    AddECMapping("FT^4", "UNITS:FT_TO_THE_FOURTH");
    AddECMapping("FT^6", "UNITS:FT_TO_THE_SIXTH");
    AddECMapping("FT_FT", "UNITS:FT_FT");
    AddECMapping("FT_H2O", "UNITS:FT_H2O");
    AddECMapping("FT_HORIZONTAL/FT_VERTICAL", "UNITS:FT_HORIZONTAL_PER_FT_VERTICAL");
    AddECMapping("FT_MILE", "UNITS:FT_MILE");
    AddECMapping("FT_PDL", "UNITS:FT_PDL");
    AddECMapping("G", "UNITS:G");
    AddECMapping("G/CUB.CM", "UNITS:G_PER_CUB_CM");
    AddECMapping("G/HR", "UNITS:G_PER_HR");
    AddECMapping("G/MIN", "UNITS:G_PER_MIN");
    AddECMapping("G/SEC", "UNITS:G_PER_SEC");
    AddECMapping("G/SQ.M", "UNITS:G_PER_SQ_M");
    AddECMapping("GALLON", "UNITS:GALLON");
    AddECMapping("GALLON/(ACRE*DAY)", "UNITS:GALLON_PER_ACRE_DAY");
    AddECMapping("GALLON/(ACRE*MIN)", "UNITS:GALLON_PER_ACRE_MIN");
    AddECMapping("GALLON/(SQ.FT*DAY)", "UNITS:GALLON_PER_SQ_FT_DAY");
    AddECMapping("GALLON/(SQ.FT*MIN)", "UNITS:GALLON_PER_SQ_FT_MIN");
    AddECMapping("GALLON/(SQ.MILE*DAY)", "UNITS:GALLON_PER_SQ_MILE_DAY");
    AddECMapping("GALLON/(SQ.MILE*MIN)", "UNITS:GALLON_PER_SQ_MILE_MIN");
    AddECMapping("GALLON/DAY", "UNITS:GALLON_PER_DAY");
    AddECMapping("GALLON/MIN", "UNITS:GALLON_PER_MIN");
    AddECMapping("GALLON/SEC", "UNITS:GALLON_PER_SEC");
    AddECMapping("GALLON_IMPERIAL", "UNITS:GALLON_IMPERIAL");
    AddECMapping("GALLON_IMPERIAL/DAY", "UNITS:GALLON_IMPERIAL_PER_DAY");
    AddECMapping("GALLON_IMPERIAL/MIN", "UNITS:GALLON_IMPERIAL_PER_MIN");
    AddECMapping("GALLON_IMPERIAL/SEC", "UNITS:GALLON_IMPERIAL_PER_SEC");
    AddECMapping("GIGA", "UNITS:GIGA");
    AddECMapping("GJ", "UNITS:GJ");
    AddECMapping("GRAD", "UNITS:GRAD");
    AddECMapping("GRM", "UNITS:GRM");
    AddECMapping("GRM/LBM", "UNITS:GRM_PER_LBM");
    AddECMapping("GUEST", "UNITS:PERSON");
    AddECMapping("GW", "UNITS:GW");
    AddECMapping("GWH", "UNITS:GWH");
    AddECMapping("HECTARE", "UNITS:HECTARE");
    AddECMapping("HECTO", "UNITS:HECTO");
    AddECMapping("HECTOPASCAL", "UNITS:HECTOPASCAL");
    AddECMapping("HORIZONTAL/VERTICAL", "UNITS:HORIZONTAL_PER_VERTICAL");
    AddECMapping("HP", "UNITS:HP");
    AddECMapping("HR", "UNITS:HR");

    AddECMapping("HUNDRED_PERSON", "UNITS:HUNDRED_PERSON");
    AddECMapping("HZ", "UNITS:HZ");
    AddECMapping("IN", "UNITS:IN");
    AddECMapping("IN/DAY", "UNITS:IN_PER_DAY");
    AddECMapping("IN/DEGREE", "UNITS:IN_PER_DEGREE");
    AddECMapping("IN/FT", "UNITS:IN_PER_FT");
    AddECMapping("IN/HR", "UNITS:IN_PER_HR");
    AddECMapping("IN/MIN", "UNITS:IN_PER_MIN");
    AddECMapping("IN/SEC", "UNITS:IN_PER_SEC");
    AddECMapping("IN^4", "UNITS:IN_TO_THE_FOURTH");
    AddECMapping("IN^6", "UNITS:IN_TO_THE_SIXTH");
    AddECMapping("IN_FT", "UNITS:IN_FT");
    AddECMapping("IN_H2O@32F", "UNITS:IN_H2O_AT_32F");
    AddECMapping("IN_H2O@39.2F", "UNITS:IN_H2O_AT_39_2F");
    AddECMapping("IN_H2O@60F", "UNITS:IN_H2O_AT_60F");
    AddECMapping("IN_HG", "UNITS:IN_HG");
    AddECMapping("IN_HG@32F", "UNITS:IN_HG_AT_32F");
    AddECMapping("IN_HG@60F", "UNITS:IN_HG_AT_60F");
    AddECMapping("IN_M", "UNITS:IN_M");
    AddECMapping("IN_MILE", "UNITS:IN_MILE");
    AddECMapping("J", "UNITS:J");
    AddECMapping("J/(KG*K)", "UNITS:J_PER_KG_K");
    AddECMapping("J/(KMOL*K)", "UNITS:J_PER_KMOL_K");
    AddECMapping("J/(MOL*K)", "UNITS:J_PER_MOL_K");
    AddECMapping("J/CUB.M", "UNITS:J_PER_CUB_M");
    AddECMapping("J/KG", "UNITS:J_PER_KG");
    AddECMapping("K", "UNITS:K");
    AddECMapping("KELVIN/M", "UNITS:KELVIN_PER_M");
    AddECMapping("KG", "UNITS:KG");
    AddECMapping("KG/CUB.CM", "UNITS:KG_PER_CUB_CM");
    AddECMapping("KG/CUB.M", "UNITS:KG_PER_CUB_M");
    AddECMapping("KG/DAY", "UNITS:KG_PER_DAY");
    AddECMapping("KG/HECTARE", "UNITS:KG_PER_HECTARE");
    AddECMapping("KG/HR", "UNITS:KG_PER_HR");
    AddECMapping("KG/KG", "UNITS:KG_PER_KG");
    AddECMapping("KG/LITRE", "UNITS:KG_PER_LITRE");
    AddECMapping("KG/M", "UNITS:KG_PER_M");
    AddECMapping("KG/MIN", "UNITS:KG_PER_MIN");
    AddECMapping("KG/MM", "UNITS:KG_PER_MM");
    AddECMapping("KG/SEC", "UNITS:KG_PER_SEC");
    AddECMapping("KG/SQ.M", "UNITS:KG_PER_SQ_M");
    AddECMapping("KGF", "UNITS:KGF");
    AddECMapping("KGF/SQ.M", "UNITS:KGF_PER_SQ_M");
    AddECMapping("KHZ", "UNITS:KHZ");
    AddECMapping("KILO", "UNITS:KILO");
    AddECMapping("KILOAMPERE", "UNITS:KILOAMPERE");
    AddECMapping("KILOBTU", "UNITS:KILOBTU");
    AddECMapping("KILOBTU/HR", "UNITS:KILOBTU_PER_HR");
    AddECMapping("KILOPASCAL", "UNITS:KILOPASCAL");
    AddECMapping("KILOPASCAL_GAUGE", "UNITS:KILOPASCAL_GAUGE");
    AddECMapping("KILOVOLT", "UNITS:KILOVOLT");
    AddECMapping("KIP/CUB.FT", "UNITS:KIP_PER_CUB_FT");
    AddECMapping("KIPM", "UNITS:KIPM");
    AddECMapping("KJ", "UNITS:KJ");
    AddECMapping("KJ/(KMOL*K)", "UNITS:KJ_PER_KMOL_K");
    AddECMapping("KJ/CUB.M", "UNITS:KJ_PER_CUB_M");
    AddECMapping("KJ/KG", "UNITS:KJ_PER_KG");
    AddECMapping("KM", "UNITS:KM");
    AddECMapping("KM/HR", "UNITS:KM_PER_HR");
    AddECMapping("KM/SEC", "UNITS:KM_PER_SEC");
    AddECMapping("KMOL", "UNITS:KMOL");
    AddECMapping("KMOL/CUB.M", "UNITS:KMOL_PER_CUB_M");
    AddECMapping("KMOL/SEC", "UNITS:KMOL_PER_SEC");
    AddECMapping("KN", "UNITS:KN");
    AddECMapping("KN/CUB.FT", "UNITS:KN_PER_CUB_FT");
    AddECMapping("KN/CUB.M", "UNITS:KN_PER_CUB_M");
    AddECMapping("KNOT_INTERNATIONAL", "UNITS:KNOT_INTERNATIONAL");
    AddECMapping("KPF", "UNITS:KPF");
    AddECMapping("KSI", "UNITS:KSI");
    AddECMapping("KVA", "UNITS:KVA");
    AddECMapping("KW", "UNITS:KW");
    AddECMapping("KWH", "UNITS:KWH");
    AddECMapping("KWH/CUB.FT", "UNITS:KWH_PER_CUB_FT");
    AddECMapping("KWH/CUB.M", "UNITS:KWH_PER_CUB_M");
    AddECMapping("KWH/MILLION_GALLON", "UNITS:KWH_PER_MILLION_GALLON");
    AddECMapping("LB-MOL", "UNITS:LB_MOL");
    AddECMapping("LBF", "UNITS:LBF");
    AddECMapping("LBF/IN", "UNITS:LBF_PER_IN");
    AddECMapping("LBF/SQ.FT", "UNITS:LBF_PER_SQ_FT");
    AddECMapping("LBF_FT", "UNITS:LBF_FT");
    AddECMapping("LBM", "UNITS:LBM");
    AddECMapping("LBM/(FT*S)", "UNITS:LBM_PER_FT_S");
    AddECMapping("LBM/ACRE", "UNITS:LBM_PER_ACRE");
    AddECMapping("LBM/CUB.FT", "UNITS:LBM_PER_CUB_FT");
    AddECMapping("LBM/CUB.IN", "UNITS:LBM_PER_CUB_IN");
    AddECMapping("LBM/DAY", "UNITS:LBM_PER_DAY");
    AddECMapping("LBM/FT", "UNITS:LBM_PER_FT");
    AddECMapping("LBM/GALLON", "UNITS:LBM_PER_GALLON");
    AddECMapping("LBM/GALLON_IMPERIAL", "UNITS:LBM_PER_GALLON_IMPERIAL");
    AddECMapping("LBM/HR", "UNITS:LBM_PER_HR");
    AddECMapping("LBM/MILLION_GALLON", "UNITS:LBM_PER_MILLION_GALLON");
    AddECMapping("LBM/MIN", "UNITS:LBM_PER_MIN");
    AddECMapping("LBM/SEC", "UNITS:LBM_PER_SEC");
    AddECMapping("LITRE", "UNITS:LITRE");
    AddECMapping("LITRE/(SQ.M*SEC)", "UNITS:LITRE_PER_SQ_M_SEC");
    AddECMapping("LITRE/DAY", "UNITS:LITRE_PER_DAY");
    AddECMapping("LITRE/HR", "UNITS:LITRE_PER_HR");
    AddECMapping("LITRE/LITRE", "UNITS:LITRE_PER_LITRE");
    AddECMapping("LITRE/MIN", "UNITS:LITRE_PER_MIN");
    AddECMapping("LITRE/SEC", "UNITS:LITRE_PER_SEC");
    AddECMapping("LONG_TON_FORCE", "UNITS:LONG_TON_FORCE");
    AddECMapping("LONG_TON_MASS", "UNITS:LONG_TON_MASS");
    AddECMapping("LUMEN", "UNITS:LUMEN");
    AddECMapping("LUMEN/SQ.FT", "UNITS:LUMEN_PER_SQ_FT");
    AddECMapping("LUX", "UNITS:LUX");
    AddECMapping("M", "UNITS:M");
    AddECMapping("M/DAy", "UNITS:M_PER_DAy");
    AddECMapping("M/HR", "UNITS:M_PER_HR");
    AddECMapping("M/KM", "UNITS:M_PER_KM");
    AddECMapping("M/M", "UNITS:M_PER_M");
    AddECMapping("M/MIN", "UNITS:M_PER_MIN");
    AddECMapping("M/RAD", "UNITS:M_PER_RAD");
    AddECMapping("M/REVOLUTION", "UNITS:M_PER_REVOLUTION");
    AddECMapping("M/SEC", "UNITS:M_PER_SEC");
    AddECMapping("M/SEC.SQ", "UNITS:M_PER_SEC_SQ");
    AddECMapping("M^4", "UNITS:M_TO_THE_FOURTH");
    AddECMapping("M^6", "UNITS:M_TO_THE_SIXTH");
    AddECMapping("M_H2O", "UNITS:M_H2O");
    AddECMapping("M_HORIZONTAL/M_VERTICAL", "UNITS:M_HORIZONTAL_PER_M_VERTICAL");
    AddECMapping("M_KM", "UNITS:M_KM");
    AddECMapping("M_M", "UNITS:M_M");
    AddECMapping("MBAR", "UNITS:MBAR");
    AddECMapping("MEGA", "UNITS:MEGA");
    AddECMapping("MEGAGRAM", "UNITS:MEGAGRAM");
    AddECMapping("MEGAJ", "UNITS:MEGAJ");
    AddECMapping("MEGAJ/KG", "UNITS:MEGAJ_PER_KG");
    AddECMapping("MEGAPASCAL", "UNITS:MEGAPASCAL");
    AddECMapping("MEGAPASCAL_GAUGE", "UNITS:MEGAPASCAL_GAUGE");
    AddECMapping("MEGAVOLT", "UNITS:MEGAVOLT");
    AddECMapping("MEGAW", "UNITS:MEGAW");
    AddECMapping("MEGAWH", "UNITS:MEGAWH");
    AddECMapping("MG", "UNITS:MG");
    AddECMapping("MG/DAY", "UNITS:MG_PER_DAY");
    AddECMapping("MG/HR", "UNITS:MG_PER_HR");
    AddECMapping("MG/LITRE", "UNITS:MG_PER_LITRE");
    AddECMapping("MG/MIN", "UNITS:MG_PER_MIN");
    AddECMapping("MG/SEC", "UNITS:MG_PER_SEC");
    AddECMapping("MHZ", "UNITS:MHZ");
    AddECMapping("MICRO", "UNITS:MICRO");
    AddECMapping("MICROAMPERE", "UNITS:MICROAMPERE");
    AddECMapping("MICROINCH", "UNITS:MICROINCH");
    AddECMapping("MICROLITRE", "UNITS:MICROLITRE");
    AddECMapping("MICROMOL/CUB.DM", "UNITS:MICROMOL_PER_CUB_DM");
    AddECMapping("MILE", "UNITS:MILE");
    AddECMapping("MILLI", "UNITS:MILLI");
    AddECMapping("MILLIAMPERE", "UNITS:MILLIAMPERE");
    AddECMapping("MILLIFOOT", "UNITS:MILLIFOOT");
    AddECMapping("MILLIINCH", "UNITS:MILLIINCH");
    AddECMapping("MILLION_GALLON", "UNITS:MILLION_GALLON");
    AddECMapping("MILLION_LITRE", "UNITS:MILLION_LITRE");
    AddECMapping("MIN", "UNITS:MIN");
    AddECMapping("MKG", "UNITS:MKG");
    AddECMapping("MKG/DAY", "UNITS:MKG_PER_DAY");
    AddECMapping("MKG/HR", "UNITS:MKG_PER_HR");
    AddECMapping("MKG/LITRE", "UNITS:MKG_PER_LITRE");
    AddECMapping("MKG/MIN", "UNITS:MKG_PER_MIN");
    AddECMapping("MKG/SEC", "UNITS:MKG_PER_SEC");
    AddECMapping("MKS", "UNITS:MKS");
    AddECMapping("MM", "UNITS:MM");
    AddECMapping("MM/DAY", "UNITS:MM_PER_DAY");
    AddECMapping("MM/HR", "UNITS:MM_PER_HR");
    AddECMapping("MM/M", "UNITS:MM_PER_M");
    AddECMapping("MM/MIN", "UNITS:MM_PER_MIN");
    AddECMapping("MM/SEC", "UNITS:MM_PER_SEC");
    AddECMapping("MM^4", "UNITS:MM_TO_THE_FOURTH");
    AddECMapping("MM^6", "UNITS:MM_TO_THE_SIXTH");
    AddECMapping("MM_H2O", "UNITS:MM_H2O");
    AddECMapping("MM_HG@32F", "UNITS:MM_HG_AT_32F");
    AddECMapping("MM_KM", "UNITS:MM_KM");
    AddECMapping("MM_M", "UNITS:MM_M");
    AddECMapping("MM_MILE", "UNITS:MM_MILE");
    AddECMapping("MN", "UNITS:MN");
    AddECMapping("MOL", "UNITS:MOL");
    AddECMapping("MOL/CUB.DM", "UNITS:MOL_PER_CUB_DM");
    AddECMapping("MOL/CUB.FT", "UNITS:MOL_PER_CUB_FT");
    AddECMapping("MOL/CUB.M", "UNITS:MOL_PER_CUB_M");
    AddECMapping("MOL/SEC", "UNITS:MOL_PER_SEC");
    AddECMapping("MPH", "UNITS:MPH");
    AddECMapping("MS", "UNITS:MS");
    AddECMapping("MU", "UNITS:UM");
    AddECMapping("MVA", "UNITS:MVA");
    AddECMapping("N", "UNITS:N");
    AddECMapping("N/CUB.FT", "UNITS:N_PER_CUB_FT");
    AddECMapping("N/CUB.M", "UNITS:N_PER_CUB_M");
    AddECMapping("N/M", "UNITS:N_PER_M");
    AddECMapping("N/MM", "UNITS:N_PER_MM");
    AddECMapping("N/RAD", "UNITS:N_PER_RAD");
    AddECMapping("N_CM", "UNITS:N_CM");
    AddECMapping("N_M", "UNITS:N_M");
    AddECMapping("NANO", "UNITS:NANO");
    AddECMapping("NAUT_MILE", "UNITS:NAUT_MILE");
    AddECMapping("NG", "UNITS:NG");
    AddECMapping("NMOL/CUB.DM", "UNITS:NMOL_PER_CUB_DM");
    AddECMapping("ONE", "UNITS:ONE");
    AddECMapping("OZF", "UNITS:OZF");
    AddECMapping("OZM", "UNITS:OZM");
    AddECMapping("PA", "UNITS:PA");
    AddECMapping("PA-S", "UNITS:PA_S");
    AddECMapping("PA/M", "UNITS:PA_PER_M");
    AddECMapping("PA_GAUGE", "UNITS:PA_GAUGE");
    AddECMapping("PASSENGER", "UNITS:PERSON");
    AddECMapping("PDL", "UNITS:PDL");
    AddECMapping("PER_FT", "UNITS:PER_FT");
    AddECMapping("PER_KM", "UNITS:PER_KM");
    AddECMapping("PER_M", "UNITS:PER_M");
    AddECMapping("PER_MILE", "UNITS:PER_MILE");
    AddECMapping("PER_MM", "UNITS:PER_MM");
    AddECMapping("PER_THOUSAND_FT", "UNITS:PER_THOUSAND_FT");
    AddECMapping("PERCENT", "UNITS:PERCENT");
    AddECMapping("PERCENT_SLOPE", "UNITS:PERCENT_SLOPE");
    AddECMapping("PERSON", "UNITS:PERSON");
    AddECMapping("PERSON/ACRE", "UNITS:PERSON_PER_ACRE");
    AddECMapping("PERSON/HECTARE", "UNITS:PERSON_PER_HECTARE");
    AddECMapping("PERSON/SQ.FT", "UNITS:PERSON_PER_SQ_FT");
    AddECMapping("PERSON/SQ.KM", "UNITS:PERSON_PER_SQ_KM");
    AddECMapping("PERSON/SQ.M", "UNITS:PERSON_PER_SQ_M");
    AddECMapping("PERSON/SQ.MILE", "UNITS:PERSON_PER_SQ_MILE");
    AddECMapping("PETA", "UNITS:PETA");
    AddECMapping("PI", "UNITS:PI");
    AddECMapping("PI/2", "UNITS:HALF_PI");
    AddECMapping("PI/4", "UNITS:QUARTER_PI");
    AddECMapping("PICO", "UNITS:PICO");
    AddECMapping("PICOMOL/CUB.DM", "UNITS:PICOMOL_PER_CUB_DM");
    AddECMapping("POISE", "UNITS:POISE");
    AddECMapping("PSI", "UNITS:PSI");
    AddECMapping("PSIG", "UNITS:PSIG");
    AddECMapping("RAD", "UNITS:RAD");
    AddECMapping("RAD/HR", "UNITS:RAD_PER_HR");
    AddECMapping("RAD/MIN", "UNITS:RAD_PER_MIN");
    AddECMapping("RAD/SEC", "UNITS:RAD_PER_SEC");
    AddECMapping("RANKINE", "UNITS:RANKINE");
    AddECMapping("RESIDENT", "UNITS:PERSON");
    AddECMapping("REVOLUTION", "UNITS:REVOLUTION");
    AddECMapping("RPH", "UNITS:RPH");
    AddECMapping("RPM", "UNITS:RPM");
    AddECMapping("RPS", "UNITS:RPS");
    AddECMapping("S", "UNITS:S");
    AddECMapping("SHORT_TON/CUB.FT", "UNITS:SHORT_TON_PER_CUB_FT");
    AddECMapping("SHORT_TON/HR", "UNITS:SHORT_TON_PER_HR");
    AddECMapping("SHORT_TON_FORCE", "UNITS:SHORT_TON_FORCE");
    AddECMapping("SHORT_TON_MASS", "UNITS:SHORT_TON_MASS");
    AddECMapping("SLUG", "UNITS:SLUG");
    AddECMapping("SLUG/CUB.FT", "UNITS:SLUG_PER_CUB_FT");
    AddECMapping("SQ.CHAIN", "UNITS:SQ_CHAIN");
    AddECMapping("SQ.CM", "UNITS:SQ_CM");
    AddECMapping("SQ.DM", "UNITS:SQ_DM");
    AddECMapping("SQ.FT", "UNITS:SQ_FT");
    AddECMapping("SQ.FT/SEC", "UNITS:SQ_FT_PER_SEC");
    AddECMapping("SQ.IN", "UNITS:SQ_IN");
    AddECMapping("SQ.KM", "UNITS:SQ_KM");
    AddECMapping("SQ.M", "UNITS:SQ_M");
    AddECMapping("SQ.M/SEC", "UNITS:SQ_M_PER_SEC");
    AddECMapping("SQ.MILE", "UNITS:SQ_MILE");
    AddECMapping("SQ.MM", "UNITS:SQ_MM");
    AddECMapping("SQ.MU", "UNITS:SQ_UM");
    AddECMapping("SQ.US_SURVEY_CHAIN", "UNITS:SQ_US_SURVEY_CHAIN");
    AddECMapping("SQ.US_SURVEY_FT", "UNITS:SQ_US_SURVEY_FT");
    AddECMapping("SQ.US_SURVEY_IN", "UNITS:SQ_US_SURVEY_IN");
    AddECMapping("SQ.US_SURVEY_MILE", "UNITS:SQ_US_SURVEY_MILE");
    AddECMapping("SQ.US_SURVEY_YRD", "UNITS:SQ_US_SURVEY_YRD");
    AddECMapping("SQ.YRD", "UNITS:SQ_YRD");
    AddECMapping("STD_G", "UNITS:STD_G");
    AddECMapping("STERAD", "UNITS:STERAD");
    AddECMapping("STOKE", "UNITS:STOKE");
    AddECMapping("STRAIN/CELSIUS", "UNITS:STRAIN_PER_CELSIUS");
    AddECMapping("STRAIN/FAHRENHEIT", "UNITS:STRAIN_PER_FAHRENHEIT");
    AddECMapping("STRAIN/KELVIN", "UNITS:STRAIN_PER_KELVIN");
    AddECMapping("STRAIN/RANKINE", "UNITS:STRAIN_PER_RANKINE");
    AddECMapping("STUDENT", "UNITS:PERSON");
    AddECMapping("TERA", "UNITS:TERA");
    AddECMapping("THOUSAND_GALLON", "UNITS:THOUSAND_GALLON");
    AddECMapping("THOUSAND_LITRE", "UNITS:THOUSAND_LITRE");
    AddECMapping("THOUSAND_PERSON", "UNITS:THOUSAND_PERSON");
    AddECMapping("THOUSAND_SQ.FT", "UNITS:THOUSAND_SQ_FT");
    AddECMapping("TONNE", "UNITS:TONNE");
    AddECMapping("TONNE/HR", "UNITS:TONNE_PER_HR");
    AddECMapping("TORR", "UNITS:TORR");
    AddECMapping("US$", "UNITS:US_DOLLAR");
    AddECMapping("US_SURVEY_ACRE", "UNITS:US_SURVEY_ACRE");
    AddECMapping("US_SURVEY_CHAIN", "UNITS:US_SURVEY_CHAIN");
    AddECMapping("US_SURVEY_FT", "UNITS:US_SURVEY_FT");
    AddECMapping("US_SURVEY_IN", "UNITS:US_SURVEY_IN");
    AddECMapping("US_SURVEY_MILE", "UNITS:US_SURVEY_MILE");
    AddECMapping("US_SURVEY_YRD", "UNITS:US_SURVEY_YRD");
    AddECMapping("VA", "UNITS:VA");
    AddECMapping("VERTICAL/HORIZONTAL", "UNITS:VERTICAL_PER_HORIZONTAL");
    AddECMapping("VOLT", "UNITS:VOLT");
    AddECMapping("W", "UNITS:W");
    AddECMapping("W/(M*C)", "UNITS:W_PER_M_C");
    AddECMapping("W/(M*K)", "UNITS:W_PER_M_K");
    AddECMapping("W/(SQ.M*CELSIUS)", "UNITS:W_PER_SQ_M_CELSIUS");
    AddECMapping("W/(SQ.M*K)", "UNITS:W_PER_SQ_M_K");
    AddECMapping("W/SQ.M", "UNITS:W_PER_SQ_M");
    AddECMapping("WATT_SECOND", "UNITS:WATT_SECOND");
    AddECMapping("WEEK", "UNITS:WEEK");
    AddECMapping("YEAR_SIDEREAL", "UNITS:YEAR_SIDEREAL");
    AddECMapping("YEAR_TROPICAL", "UNITS:YEAR_TROPICAL");
    AddECMapping("YOCTO", "UNITS:YOCTO");
    AddECMapping("YOTTA", "UNITS:YOTTA");
    AddECMapping("YR", "UNITS:YR");
    AddECMapping("YRD", "UNITS:YRD");
    AddECMapping("YRD/SEC", "UNITS:YRD_PER_SEC");
    AddECMapping("ZEPTO", "UNITS:ZEPTO");
    AddECMapping("ZETTA", "UNITS:ZETTA");

    // These must be added last 
    AddECMapping("CAPITA", "UNITS:PERSON");
    AddECMapping("HUNDRED_CAPITA", "UNITS:HUNDRED_PERSON");
    AddECMapping("THOUSAND_CAPITA", "UNITS:THOUSAND_PERSON");
    AddECMapping("KIPF", "UNITS:KPF");
    AddECMapping("N/SQ.MM", "UNITS:MEGAPASCAL");
    AddECMapping("DEKA", "UNITS:DECA");
    }

END_BENTLEY_UNITS_NAMESPACE
