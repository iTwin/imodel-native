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
void Reporter::Next() { auto x = m_results[m_results.size()]; x.SetNull(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetString(BeJsConst record, Utf8String groupingField)
    {
    Utf8String groupingValue = "";
    record.ForEachProperty(
        [&groupingField, &groupingValue](Utf8CP name, BeJsConst group)
        {
        if (groupingField == name)
            {
            groupingValue = group.asCString();
            return true;
            }
        return false;
        });
    return groupingValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void CreateJsonGroupingStructure(BeJsValue json, BeJsConst records, bvector<Utf8String> const& groupingFields)
    {
    if (groupingFields.empty())
        return;

    auto const& groupingField = groupingFields[0];
    bvector<Utf8String> nestedGroupingFields(groupingFields.begin() + 1, groupingFields.end());
    records.ForEachArrayMember(
        [&](BeJsConst::ArrayIndex i, BeJsConst record)
        {
        Utf8String groupingValue = GetString(record, groupingField);
        CreateJsonGroupingStructure(json[groupingValue], records, nestedGroupingFields);
        return false;
        });
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

    BeJsDocument json;
    json.toObject();
    CreateJsonGroupingStructure(json, m_results, groupingFields);

    m_results.ForEachArrayMember(
        [&](BeJsConst::ArrayIndex i, BeJsConst record)
        {
        json.From(json);
        for (auto const& groupingField : groupingFields)
            {
            Utf8String groupingValue = GetString(record, groupingField);
            json.From(json[groupingValue]);
            }

        for (auto const& field : m_fields)
            {
            if (groupingFields.end() != std::find(groupingFields.begin(), groupingFields.end(), field))
                continue;
            Utf8String groupingField = GetString(record, field);
            if (groupingField == "")
                continue;
            BeJsConst value = record[groupingField];
            json[field].From(value);
            }
        return false;
        });
    reportFile->PutLine(WString(json.Stringify(Indented).c_str(), true).c_str(), true);
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
    m_results.ForEachArrayMember(
        [&](BeJsConst::ArrayIndex i, BeJsConst record)
        {
        Utf8String testName;
        for (auto testNameField : testNameFields)
            {
            Utf8String field;
            auto allItemsIterated = record.ForEachProperty(
                [&testNameField, &field](Utf8CP name, BeJsConst json)
                {
                if (name == testNameField)
                    {
                    if (json.isString()) field = json.asCString();
                    return true;
                    }
                return false;
                }
            );
            
            if (!allItemsIterated && !field.empty())
                {
                if (!testName.empty())
                    testName.append(" + ");
                testName.append(field);
                }
            }

        for (auto const& fieldName : exportedFields)
            {
            Utf8String field;
            auto allItemsIterated = record.ForEachProperty(
                [&fieldName, &field](Utf8CP name, BeJsConst json)
                {
                if (name == fieldName)
                    {
                    if (json.isString()) field = json.asCString();
                    return true;
                    }
                return false;
                }
            );
            Utf8String value = !allItemsIterated ? field : Utf8String();

            Utf8String line;
            line.append(suiteName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(testName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(fieldName).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(value).append(REPORT_CSV_FILE_SEPARATOR);
            line.append(DateTime::GetCurrentTimeUtc().ToString()).append(REPORT_CSV_FILE_SEPARATOR);
            line.append("{}");
            reportFile->PutLine(WString(line.c_str(), true).c_str(), true);
            }
        return false;
        }
    );
    return SUCCESS;
    }