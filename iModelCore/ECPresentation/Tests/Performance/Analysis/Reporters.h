/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTextFile.h>
#include <UnitTests/ECPresentation/ECPresentationTest.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Reporter
{
private:
    bvector<Utf8String> m_fields;
    BeJsDocument m_results;
public:
    Reporter() {}
    Reporter(bvector<Utf8String> fields) : m_fields(fields) {}
    template <typename T>
    void Record(Utf8StringCR field, T value)
        {
        if (m_fields.end() == std::find(m_fields.begin(), m_fields.end(), field)) m_fields.push_back(field);
        m_results[m_results.size() - 1][field] = value;
        }
    void Record(Utf8StringCR field, BeJsConst value)
        {
        if (m_fields.end() == std::find(m_fields.begin(), m_fields.end(), field)) m_fields.push_back(field);
        m_results[m_results.size() - 1][field].From(value);
        }

    void Next();
    BentleyStatus ToJsonFile(BeFileNameCR path, bvector<Utf8String> const& groupingFields);
    BentleyStatus ToExportCsvFile(BeFileNameCR path, Utf8StringCR suiteName, bvector<Utf8String> const& testNameFields, bvector<Utf8String> const& exportedFields);
};

END_ECPRESENTATIONTESTS_NAMESPACE
