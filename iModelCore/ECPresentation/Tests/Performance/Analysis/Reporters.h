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
    bvector<bmap<Utf8String, Json::Value>> m_results;
public:
    Reporter() {}
    Reporter(bvector<Utf8String> fields) : m_fields(fields) {}
    void Record(Utf8StringCR field, Json::Value value);
    void Next();
    BentleyStatus ToJsonFile(BeFileNameCR path, bvector<Utf8String> const& groupingFields);
    BentleyStatus ToExportCsvFile(BeFileNameCR path, Utf8StringCR suiteName, bvector<Utf8String> const& testNameFields, bvector<Utf8String> const& exportedFields);
};

END_ECPRESENTATIONTESTS_NAMESPACE
