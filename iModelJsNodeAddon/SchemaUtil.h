/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjectsAPI.h>
#include <unordered_set>

//=======================================================================================
//! @bsistruct
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
//! @bsiclass
//=======================================================================================
class SchemaUtil {
  public:
    static std::string ComputeChecksum(std::string &message, std::string schemaPath, std::unordered_set<Utf8String> refpaths);
    static std::string ComputeChecksumWithExactRefMatch(std::string &message, std::string schemaPath, std::unordered_set<Utf8String> refpaths);
  private:
    static std::string CreateChecksum(std::string &message, std::string schemaPath, std::unordered_set<Utf8String> refpaths, ECN::SearchPathSchemaFileLocaterPtr locator);
};
