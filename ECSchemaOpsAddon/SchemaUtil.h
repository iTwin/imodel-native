/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjectsAPI.h>
#include <string>
#include <unordered_set>

// Hash implementation for BentleyM0200::Utf8String type
namespace std
    {
    template<> struct hash<BentleyM0200::Utf8String>
        {
        typedef BentleyM0200::Utf8String argument_type;
        typedef size_t result_type;
        result_type operator()(argument_type const& string) const { return hash<std::string>{}(string.c_str()); }
        };
    }

//=======================================================================================
//! @bsistruct                                   Chris.Lawson                   05/2020
//=======================================================================================
struct ExactSearchPathSchemaFileLocater : ECN::SearchPathSchemaFileLocater
{
    using ECN::SearchPathSchemaFileLocater::SearchPathSchemaFileLocater;
protected:
    ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext) override
        {
        return SearchPathSchemaFileLocater::_LocateSchema(key, ECN::SchemaMatchType::Exact, schemaContext);
        }
public:
    static ECN::SearchPathSchemaFileLocaterPtr CreateExactSearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt = false)
        {
        return new ExactSearchPathSchemaFileLocater(searchPaths, includeFilesWithNoVerExt);
        }
};

//=======================================================================================
//! @bsiclass                                    Chris.Lawson                   05/2020
//=======================================================================================
class SchemaUtil {
  public:
    std::string ComputeChecksum(std::string &message, std::string schemaPath, std::unordered_set<Utf8String> refpaths);
};
