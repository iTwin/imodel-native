/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/StdFormatSet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/AliasMappings.h"

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

END_BENTLEY_FORMATTING_NAMESPACE
