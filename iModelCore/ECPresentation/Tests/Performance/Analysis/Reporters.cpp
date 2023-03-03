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
void Reporter::Next() 
    { 
    BeJsValue json = m_results[m_results.size()]; 
    json.SetNull(); 
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
        Utf8String groupingValue = record.hasMember(groupingField.c_str()) ? record[groupingField].asCString() : "";
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

    m_results.ForEachArrayMemberValue(
        [&](BeJsConst::ArrayIndex i, BeJsValue record)
        {
        std::unique_ptr<BeJsValue> container = std::make_unique<BeJsValue>(json);
        for (auto const& groupingField : groupingFields)
            {
            Utf8String groupingValue = record.hasMember(groupingField.c_str()) ? record[groupingField].asCString() : "";
            container = std::make_unique<BeJsValue>((*container)[groupingValue]);
            }

        for (auto const& field : m_fields)
            {
            if (groupingFields.end() != std::find(groupingFields.begin(), groupingFields.end(), field))
                continue;

            if (!record.hasMember(field.c_str()))
                continue;

            (*container)[field].From(record[field]);
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
            if (record.hasMember(testNameField.c_str()) && record[testNameField].isString())
                {
                if (!testName.empty())
                    testName.append(" + ");
                testName.append(record[testNameField].asCString());
                }
            }

        for (auto const& fieldName : exportedFields)
            {
            Utf8String value = record.hasMember(fieldName.c_str()) ? record[fieldName].Stringify() : Utf8String();
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