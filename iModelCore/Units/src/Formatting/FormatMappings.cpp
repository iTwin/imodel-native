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
    AddMapping("DefaultReal", "FORMATS:DefaultReal");
    // Duplicates names of DefaultReal
    AddLegacyNameToFormatStringMapping("Real", "FORMATS:DefaultReal");
    AddLegacyNameToFormatStringMapping("DefaultInt", "FORMATS:DefaultReal");

    AddMapping("DefaultRealU", "FORMATS:DefaultRealU");
    // Duplicates names of FORMATS:DefaultRealU
    AddLegacyNameToFormatStringMapping("RealU", "FORMATS:DefaultRealU");
    AddLegacyNameToFormatStringMapping("Real6U", "FORMATS:DefaultRealU");
    
    AddMapping("Real2", "FORMATS:DefaultReal(2)");
    AddMapping("Real3", "FORMATS:DefaultReal(3)");
    AddMapping("Real4", "FORMATS:DefaultReal(4)");
    AddMapping("Real2U", "FORMATS:DefaultRealU(2)");
    AddMapping("Real3U", "FORMATS:DefaultRealU(3)");
    AddMapping("Real4U", "FORMATS:DefaultRealU(4)");

    AddMapping("Stop100-2-2z", "FORMATS:StationZ_100_2");
    AddMapping("Stop1000-2-3z", "FORMATS:StationZ_1000_3");

    AddMapping("AngleDMS", "FORMATS:AngleDMS");
    AddLegacyNameToFormatStringMapping("CAngleDMS", "FORMATS:AngleDMS"); // Duplicate name of FORMATS:AngleDMS
    AddMapping("AngleDMS8", "FORMATS:AngleDMS(8)");
    AddLegacyNameToFormatStringMapping("CAngleDMS8", "FORMATS:AngleDMS(8)"); // Duplicate name of FORMATS:AngleDMS(8)

    AddMapping("HMS", "FORMATS:HMS");

    AddMapping("AmerFI8", "FORMATS:AmerFI");
    AddMapping("AmerFI16", "FORMATS:AmerFI(16)");
    AddMapping("AmerFI32", "FORMATS:AmerFI(32)");

    AddMapping("DefaultFractional", "FORMATS:Fractional");
    AddLegacyNameToFormatStringMapping("Fractional64", "FORMATS:Fractional"); // Duplicate name of FORMATS:Fractional
    AddMapping("Fractional4", "FORMATS:Fractional(4)");
    AddMapping("Fractional8", "FORMATS:Fractional(8)");
    AddMapping("Fractional16", "FORMATS:Fractional(16)");
    AddMapping("Fractional32", "FORMATS:Fractional(32)");
    AddMapping("Fractional128", "FORMATS:Fractional(128)");

    AddMapping("Real2UNS", "FORMATS:DefaultRealUNS(2)");
    AddMapping("Real3UNS", "FORMATS:DefaultRealUNS(3)");
    AddMapping("Real4UNS", "FORMATS:DefaultRealUNS(4)");
    AddMapping("Real6UNS", "FORMATS:DefaultRealUNS");

    AddMapping("Meters4u", "FORMATS:DefaultRealUNS(4)[u:M|m]");
    AddMapping("Feet4u", "FORMATS:DefaultRealUNS(4)[u:FT|']");
    AddMapping("Inches4u", "FORMATS:DefaultRealUNS(4)[u:IN|&quot;]");
    AddMapping("DecimalDeg4", "FORMATS:DefaultRealUNS(4)[u:ARC_DEG|\xC2\xB0]");

    // The name is incorrect here. The actual precision is 8.
    AddMapping("Inches18u", "FORMATS:DefaultRealUNS(8)[u:IN|&quot;]");

    // No mapping for the following formats. They have been removed in the new generation.

    //AddMapping("Stop100-2", "FORMATS:Station_100");
    //AddMapping("Stop100-2u", "FORMATS:StationU_100");
    //AddMapping("Stop100-2uz", "FORMATS:StationUZ_100");

    //AddMapping("Stop100-2-4", "FORMATS:Station_100_4");
    //AddMapping("Stop100-2-4u", "FORMATS:StationU_100_4");

    //AddMapping("Stop1000-2", "FORMATS:Station_1000");
    //AddMapping("Stop1000-2u", "FORMATS:StationU_1000");

    //AddMapping("Stop1000-2-4", "FORMATS:Station_1000_4");
    //AddMapping("Stop1000-2-4u", "FORMATS:StationU_1000_4");

    //AddMapping("SignedReal", "FORMATS:SignedReal");
    //AddMapping("ParenthsReal", "FORMATS:ParensReal");

    //AddMapping("DefaultFractionalU", "FORMATS:FractionalU");
    //AddLegacyNameToFormatStringMapping("Fractional64U", "FORMATS:FractionalU"); // Duplicate name of FORMATS:FractionalU

    //AddMapping("SignedFractional", "FORMATS:SignedFractional");
    //AddMapping("Fractional4U", "FORMATS:FractionalU(4)");
    //AddMapping("Fractional8U", "FORMATS:FractionalU(8)");
    //AddMapping("Fractional16U", "FORMATS:FractionalU(16)");
    //AddMapping("Fractional32U", "FORMATS:FractionalU(32)");
    //AddMapping("Fractional128U", "FORMATS:FractionalU(128)");

    //AddMapping("DefaultExp", "FORMATS:Scientific");
    //AddMapping("SignedExp", "FORMATS:SignedScientific");
    //AddMapping("NormalizedExp", "FORMATS:ScientificNormal");

    //AddMapping("AngleDM8", "FORMATS:AngleDM");
    //AddLegacyNameToFormatStringMapping("CAngleDM8", "FORMATS:AngleDM"); // Duplicate name of FORMATS:AngleDM

    //AddMapping("AmerMYFI4", "FORMATS:AmerMYFI");

    //AddMapping("AmerYFI8", "FORMATS:AmerYFI");

    //AddMapping("StationFt2", "FORMATS:StationUNS_100[u:FT|']");
    //AddMapping("StationM4", "FORMATS:StationU_1000_4[u:M|m]");
    }

END_BENTLEY_FORMATTING_NAMESPACE
