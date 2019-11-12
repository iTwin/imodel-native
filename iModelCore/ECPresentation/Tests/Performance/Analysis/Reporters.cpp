/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Reporters.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Reporter::Record(Utf8StringCR field, Json::Value value) { m_results.back()[field] = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Reporter::Next() { m_results.push_back(bmap<Utf8String, Json::Value>()); }

#define REPORT_CSV_FILE_SEPARATOR ","
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Reporter::ToCsvFile(BeFileNameCR path)
    {
    BeFileStatus status;
    BeTextFilePtr reportFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);
    if (BeFileStatus::Success != status)
        return ERROR;
    
    reportFile->PutLine(WString(BeStringUtilities::Join(m_fields, REPORT_CSV_FILE_SEPARATOR).c_str(), true).c_str(), true);
    for (bmap<Utf8String, Json::Value> const& record : m_results)
        {
        Utf8String line;
        bool isFirst = true;
        for (Utf8StringCR field : m_fields)
            {
            auto iter = record.find(field);
            Utf8String value = (record.end() != iter) ? iter->second.ToString() : Utf8String();
            if (!isFirst)
                line.append(REPORT_CSV_FILE_SEPARATOR);
            line.append(value);
            isFirst = false;
            }
        reportFile->PutLine(WString(line.c_str(), true).c_str(), true);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void CreateJsonGroupingStructure(JsonValueR json, bvector<bmap<Utf8String, Json::Value>> const& records, bvector<Utf8String> const& groupingFields)
    {
    if (groupingFields.empty())
        return;

    Utf8StringCR groupingField = groupingFields[0];
    bvector<Utf8String> nestedGroupingFields(groupingFields.begin() + 1, groupingFields.end());
    for (bmap<Utf8String, Json::Value> const& record : records)
        {
        auto iter = record.find(groupingField);
        Utf8String groupingValue = (record.end() != iter) ? iter->second.asCString() : "";
        json[groupingValue] = Json::Value();
        CreateJsonGroupingStructure(json[groupingValue], records, nestedGroupingFields);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Reporter::ToJsonFile(BeFileNameCR path, bvector<Utf8String> const& groupingFields)
    {
    BeFileStatus status;
    BeTextFilePtr reportFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);
    if (BeFileStatus::Success != status)
        return ERROR;

    Json::Value json(Json::objectValue);
    CreateJsonGroupingStructure(json, m_results, groupingFields);

    for (bmap<Utf8String, Json::Value> const& record : m_results)
        {
        Json::Value* container = &json;
        for (Utf8StringCR groupingField : groupingFields)
            {
            auto iter = record.find(groupingField);
            Utf8String groupingValue = (record.end() != iter) ? iter->second.asCString() : "";
            container = &(*container)[groupingValue];
            }

        for (Utf8StringCR field : m_fields)
            {
            if (groupingFields.end() != std::find(groupingFields.begin(), groupingFields.end(), field))
                continue;
            auto iter = record.find(field);
            Json::Value value = (record.end() != iter) ? iter->second : Json::Value();
            (*container)[field] = value;
            }
        }
    reportFile->PutLine(WString(Json::StyledWriter().write(json).c_str(), true).c_str(), true);
    return SUCCESS;
    }
