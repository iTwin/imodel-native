/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/Formatting/AliasMappings.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/bmap.h>
#include <Formatting/FormattingDefinitions.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//=======================================================================================
//! Comparison function that is used within various schema related data structures
//! for string comparison in STL collections.
// @bsistruct
//=======================================================================================
struct less_str
{
    bool operator()(Utf8String s1, Utf8String s2) const
        {
        if (BeStringUtilities::Stricmp(s1.c_str(), s2.c_str()) < 0)
            return true;
        return false;
        }
};

//=======================================================================================
// @bsistruct                                                    Caleb.Shafer      03/18
//=======================================================================================
struct AliasMappings
{
private:
    static AliasMappings * s_mappings;

    bmap<Utf8String, Utf8String, less_str> m_aliasNameMapping; // key: alias value: name
    bmap<Utf8String, Utf8String, less_str> m_nameAliasMapping; // key: name value: alias

    AliasMappings();
    void AddMappings();
    void AddMapping(Utf8CP alias, Utf8CP name)
        {
        m_aliasNameMapping[alias] = name;
        m_nameAliasMapping[name] = alias;
        }

    static AliasMappings * GetMappings();

public:
    UNITS_EXPORT static Utf8CP TryGetAliasFromName(Utf8CP name); //!< Returns the alias corresponding to the given name.
    UNITS_EXPORT static Utf8CP TryGetNameFromAlias(Utf8CP alias); //!< Returns the name corresponding to the given alias.
};

END_BENTLEY_FORMATTING_NAMESPACE
