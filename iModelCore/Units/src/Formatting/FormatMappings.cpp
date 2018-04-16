/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatMappings.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include <Formatting/AliasMappings.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//=======================================================================================
// AliasMappings
//=======================================================================================

AliasMappings * AliasMappings::s_mappings = nullptr;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
AliasMappings::AliasMappings()
    {
    AddMappings();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
AliasMappings * AliasMappings::GetMappings()
    {
    if (nullptr == s_mappings)
        s_mappings = new AliasMappings();
    return s_mappings;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP AliasMappings::TryGetAliasFromName(Utf8CP name)
    {
    auto iter = GetMappings()->m_nameAliasMapping.find(name);
    if (iter == GetMappings()->m_nameAliasMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP AliasMappings::TryGetNameFromAlias(Utf8CP alias)
    {
    auto iter = GetMappings()->m_aliasNameMapping.find(alias);
    if (iter == GetMappings()->m_aliasNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void AliasMappings::AddMappings()
    {
    AddMapping("cdm8", "CAngleDM8");
    AddMapping("cdms", "CAngleDMS");
    AddMapping("cdms8", "CAngleDMS8");

    AddMapping("decimalDeg4", "DecimalDeg4");
    AddMapping("dm8", "AngleDM8");
    AddMapping("dms", "AngleDMS");
    AddMapping("dms8", "AngleDMS8");
    AddMapping("feet4u", "Feet4u");
    AddMapping("fi8", "AmerFI8");
    AddMapping("fi16", "AmerFI16");
    AddMapping("fi32", "AmerFI32");
    AddMapping("fract", "DefaultFractional");
    AddMapping("fractu", "DefaultFractionalU");
    AddMapping("fract4", "Fractional4");
    AddMapping("fract4u", "Fractional4U");
    AddMapping("fract8", "Fractional8");
    AddMapping("fract8u", "Fractional8U");
    AddMapping("fract16", "Fractional16");
    AddMapping("fract16u", "Fractional16U");
    AddMapping("fract32", "Fractional32");
    AddMapping("fract32u", "Fractional32U");
    AddMapping("fract64", "Fractional64");
    AddMapping("fract64u", "Fractional64U");
    AddMapping("fract128", "Fractional128");
    AddMapping("fract128u", "Fractional128U");

    AddMapping("fractSign", "SignedFractional");

    AddMapping("yfi8", "AmerYFI8");
    AddMapping("meters4u", "Meters4u");
        
    AddMapping("inches4u", "Inches4u");
    AddMapping("Inches18u", "Inches18u");
    AddMapping("int", "DefaultInt");

    AddMapping("myfi4", "AmerMYFI4");

    AddMapping("hms", "HMS");

    AddMapping("real", "DefaultReal");
    AddMapping("realu", "DefaultRealU");
    AddMapping("real2", "Real2");
    AddMapping("real2u", "Real2U");
    AddMapping("real2uns", "Real2UNS");
    AddMapping("real3", "Real3");
    AddMapping("real3u", "Real3U");
    AddMapping("real3uns", "Real3UNS");
    AddMapping("real4", "Real4");
    AddMapping("real4u", "Real4U");
    AddMapping("real4uns", "Real4UNS");
    AddMapping("real6u", "Real6U");
    AddMapping("real6uns", "Real6UNS");

    AddMapping("realPth", "ParenthsReal");
    AddMapping("realSign", "SignedReal");

    AddMapping("sci", "DefaultExp");
    AddMapping("sciN", "NormalizedExp");
    AddMapping("sciSign", "SignedExp");

    AddMapping("stationFt2", "StationFt2");
    AddMapping("stationM4", "StationM4");

    AddMapping("stop100-2-2z", "Stop100-2-2z");
    AddMapping("stop100-2-4", "Stop100-2-4");
    AddMapping("stop100-2-4u", "Stop100-2-4u");
    AddMapping("stop100-2", "Stop100-2");
    AddMapping("stop100-2u", "Stop100-2u");
    AddMapping("stop100-2uz", "Stop100-2uz");

    AddMapping("stop1000-2-3z", "Stop1000-2-3z");
    AddMapping("stop1000-2-4", "Stop1000-2-4");
    AddMapping("stop1000-2-4u", "Stop1000-2-4u");
    AddMapping("stop1000-2", "Stop1000-2");
    AddMapping("stop1000-2u", "Stop1000-2u");
    }

//=======================================================================================
// LegacyNameMappings
//=======================================================================================

LegacyNameMappings* LegacyNameMappings::s_mappings = nullptr;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
LegacyNameMappings::LegacyNameMappings()
    {
    AddMappings();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
// static
LegacyNameMappings* LegacyNameMappings::GetMappings()
    {
    if (nullptr == s_mappings)
        s_mappings = new LegacyNameMappings();
    return s_mappings;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP LegacyNameMappings::TryGetLegacyNameFromFormatString(Utf8CP formatString)
    {
    auto iter = GetMappings()->m_formatToNameMapping.find(formatString);
    if (iter == GetMappings()->m_formatToNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP LegacyNameMappings::TryGetFormatStringFromLegacyName(Utf8CP name)
    {
    auto iter = GetMappings()->m_nameToFormatMapping.find(name);
    if (iter == GetMappings()->m_nameToFormatMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
void LegacyNameMappings::AddMappings()
    {
    AddMapping("DefaultReal", "f:DefaultReal");
    AddMapping("DefaultRealU", "f:DefaultRealU");
    AddMapping("Real", "f:DefaultReal");
    AddMapping("Real2", "f:DefaultReal(2)");
    AddMapping("Real3", "f:DefaultReal(3)");
    AddMapping("Real4", "f:DefaultReal(4)");
    AddMapping("RealU", "f:DefaultRealU");
    AddMapping("Real2U", "f:DefaultRealU(2)");
    AddMapping("Real3U", "f:DefaultRealU(3)");
    AddMapping("Real4U", "f:DefaultRealU(4)");
    AddMapping("Real6U", "f:DefaultRealU");
    AddMapping("Real2UNS", "f:DefaultRealUNS(2)");
    AddMapping("Real3UNS", "f:DefaultRealUNS(3)");
    AddMapping("Real4UNS", "f:DefaultRealUNS(4)");
    AddMapping("Real6UNS", "f:DefaultRealUNS");

    // TODO Stations
    AddMapping("Stop100-2", "f:Station_100");
    AddMapping("Stop100-2u", "f:StationU_100");
    AddMapping("Stop100-2uz", "f:StationUZ_100");

    // AddMapping("Stop100-2-2z", "");
    // AddMapping("Stop100-4", "");
    // AddMapping("Stop100-4u", "");

    AddMapping("Stop1000-2", "f:Station_1000");
    AddMapping("Stop1000-2u", "f:StationU_1000");

    // AddMapping("Stop1000-2-3z", "");
    // AddMapping("Stop1000-2-4", "");
    // AddMapping("Stop1000-2-4u", "");
    //

    AddMapping("SignedReal", "f:SignedReal");
    AddMapping("ParenthsReal", "f:ParensReal");

    AddMapping("DefaultFractional", "f:Fractional");
    AddMapping("DefaultFractionalU", "f:FractionalU");
    AddMapping("SignedFractional", "f:SignedFractional");

    AddMapping("Fractional4", "f:Fractional(4)");
    AddMapping("Fractional4U", "f:FractionalU(4)");
    AddMapping("Fractional8", "f:Fractional(8)");
    AddMapping("Fractional8U", "f:FractionalU(8)");
    AddMapping("Fractional16", "f:Fractional(16)");
    AddMapping("Fractional16U", "f:FractionalU(16)");
    AddMapping("Fractional32", "f:Fractional(32)");
    AddMapping("Fractional32U", "f:FractionalU(32)");
    AddMapping("Fractional64", "f:Fractional");
    AddMapping("Fractional64U", "f:FractionalU");
    AddMapping("Fractional128", "f:Fractional(128)");
    AddMapping("Fractional128U", "f:FractionalU(128)");

    AddMapping("DefaultExp", "f:Scientific");
    AddMapping("SignedExp", "f:SignedScientific");
    AddMapping("NormalizedExp", "f:ScientificNormal");
    AddMapping("DefaultInt", "f:DefaultReal");

    AddMapping("AngleDMS", "f:AngleDMS");
    AddMapping("AngleDMS8", "f:AngleDMS(8)");
    AddMapping("AngleDM8", "f:AngleDM");

    AddMapping("HMS", "f:HMS");

    AddMapping("AmerMYFI4", "f:AmerMYFI");

    AddMapping("AmerFI8", "f:AmerFI");
    AddMapping("AmerFI16", "f:AmerFI(16)");
    AddMapping("AmerFI32", "f:AmerFI(32)");

    AddMapping("AmerYFI8", "f:AmerYFI");

    AddMapping("Meters4u", "f:Meters4u");
    AddMapping("Feet4u", "f:Feet4U");
    AddMapping("Inches4u", "f:InchesU");
    AddMapping("Inches18u", "f:InchesU(8)");

    AddMapping("DecimalDeg4", "f:DecimalDeg4");
    AddMapping("StationFt2", "f:StationFt2");
    AddMapping("StationM4", "f:StationM4");

    AddMapping("CAngleDMS", "f:AngleDMS");
    AddMapping("CAngleDMS8", "f:AngleDMS(8)");
    AddMapping("CAngleDM8", "f:AngleDM");
    }

END_BENTLEY_FORMATTING_NAMESPACE
