/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Reporters.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

#define REPORT_CSV_FILE_SEPARATOR ","
#define REPORT_CSV_FILE_SEPARATOR_W L","

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Reporter::Record(Utf8StringCR field, Json::Value value)
    {
    if (m_fields.end() == std::find(m_fields.begin(), m_fields.end(), field))
        m_fields.push_back(field);

    m_results.back()[field] = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Reporter::Next() { m_results.push_back(bmap<Utf8String, Json::Value>()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void CreateJsonGroupingStructure(JsonValueR json, bvector<bmap<Utf8String, Json::Value>> const& records, bvector<Utf8String> const& groupingFields)
    {
    if (groupingFields.empty())
        return;

    auto const& groupingField = groupingFields[0];
    bvector<Utf8String> nestedGroupingFields(groupingFields.begin() + 1, groupingFields.end());
    for (auto const& record : records)
        {
        auto iter = record.find(groupingField);
        Utf8String groupingValue = (record.end() != iter) ? iter->second.asCString() : "";
        json[groupingValue] = Json::Value();
        CreateJsonGroupingStructure(json[groupingValue], records, nestedGroupingFields);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Reporter::ToJsonFile(BeFileNameCR path, bvector<Utf8String> const& groupingFields)
    {
    BeFileStatus status;
    BeTextFilePtr reportFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);
    if (BeFileStatus::Success != status)
        return ERROR;

    Json::Value json(Json::objectValue);
    CreateJsonGroupingStructure(json, m_results, groupingFields);

    for (auto const& record : m_results)
        {
        Json::Value* container = &json;
        for (auto const& groupingField : groupingFields)
            {
            auto iter = record.find(groupingField);
            Utf8CP groupingValue = (record.end() != iter) ? iter->second.asCString() : "";
            container = &(*container)[groupingValue];
            }

        for (auto const& field : m_fields)
            {
            if (groupingFields.end() != std::find(groupingFields.begin(), groupingFields.end(), field))
                continue;
            auto iter = record.find(field);
            Json::Value value = (record.end() != iter) ? iter->second : Json::Value();
            if (value.empty())
                continue;
            (*container)[field] = value;
            }
        }
    reportFile->PutLine(WString(Json::StyledWriter().write(json).c_str(), true).c_str(), true);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Reporter::ToExportCsvFile(BeFileNameCR path, Utf8StringCR suiteName, bvector<Utf8String> const& testNameFields, bvector<Utf8String> const& exportedFields)
    {
    BeFileStatus status;
    BeTextFilePtr reportFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);
    if (BeFileStatus::Success != status)
        return ERROR;

    // add header
    reportFile->PutLine(
        L"TestSuite" REPORT_CSV_FILE_SEPARATOR_W
        L"TestName" REPORT_CSV_FILE_SEPARATOR_W
        L"ValueDescription" REPORT_CSV_FILE_SEPARATOR_W
        L"Value" REPORT_CSV_FILE_SEPARATOR_W
        L"Date" REPORT_CSV_FILE_SEPARATOR_W
        L"Info",
        true);

    // add values
    for (auto const& record : m_results)
        {
        Utf8String testName;
        for (auto testNameField : testNameFields)
            {
            auto iter = record.find(testNameField);
            if (record.end() != iter && iter->second.isString())
                {
                if (!testName.empty())
                    testName.append(" + ");
                testName.append(iter->second.asCString());
                }
            }

        for (auto const& fieldName : exportedFields)
            {
            auto iter = record.find(fieldName);
            Utf8String value = (record.end() != iter) ? iter->second.ToString() : Utf8String();

            Utf8String line;
            line.append(suiteName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(testName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(fieldName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(value).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(DateTime::GetCurrentTimeUtc().ToString()).append(REPORT_CSV_FILE_SEPARATOR);
            line.append("{}");
            reportFile->PutLine(WString(line.c_str(), true).c_str(), true);
            }
        }
    return SUCCESS;
    }