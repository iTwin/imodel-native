/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUtil.h"
#include <ECObjects/ECSchema.h>

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string SchemaUtil::ComputeChecksum(std::string &message, std::string schemaPathString, std::unordered_set<Utf8String> refpaths)
{
  bvector<WString> searchPaths;
	for (auto const& path : refpaths)
		searchPaths.push_back(WString(BeFileName(path).GetName()));

  SearchPathSchemaFileLocaterPtr locator;
  locator = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths, true);

  return SchemaUtil::CreateChecksum(message, schemaPathString, refpaths, locator);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string SchemaUtil::ComputeChecksumWithExactRefMatch(std::string &message, std::string schemaPathString, std::unordered_set<Utf8String> refpaths)
{
  bvector<WString> searchPaths;
	for (auto const& path : refpaths)
		searchPaths.push_back(WString(BeFileName(path).GetName()));

  SearchPathSchemaFileLocaterPtr locator;
  locator = ExactSearchPathSchemaFileLocater::CreateExactSearchPathSchemaFileLocater(searchPaths, true);

  return SchemaUtil::CreateChecksum(message, schemaPathString, refpaths, locator);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string SchemaUtil::CreateChecksum(std::string &message, std::string schemaPathString, std::unordered_set<Utf8String> refpaths, SearchPathSchemaFileLocaterPtr locator)
{
  auto context = ECSchemaReadContext::CreateContext(true, true);
	context->SetPreserveElementOrder(false);
	context->SetSkipValidation(true);
  context->AddSchemaLocater(*locator);

  ECSchemaPtr schema;
  BeFileName schemaPath(schemaPathString);

  auto const status = ECSchema::ReadFromXmlFile(schema, schemaPath.GetName(), *context, false);
  if (status == SchemaReadStatus::DuplicateSchema && (schema = ECSchema::LocateSchema(schemaPath.GetName(), *context)).IsNull()
    || status != SchemaReadStatus::Success || schema.IsNull())
  {
    Utf8String utfFileName;
    WString wFileName = schemaPath.GetFileNameWithoutExtension();
    BeStringUtilities::WCharToUtf8(utfFileName, wFileName.c_str());
    message = std::string("Failed to read schema ") + utfFileName.c_str();
    return "";
  }

  return schema->ComputeCheckSum().c_str();
}