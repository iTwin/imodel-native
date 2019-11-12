/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTextFile.h>
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+===============+===============+===============+===============+===============+======*/
struct Reporter
{
private:
    bvector<Utf8String> m_fields;
    bvector<bmap<Utf8String, Json::Value>> m_results;
public:
    Reporter(bvector<Utf8String> fields) : m_fields(fields) {}
    void Record(Utf8StringCR field, Json::Value value);
    void Next();
    BentleyStatus ToCsvFile(BeFileNameCR path);
    BentleyStatus ToJsonFile(BeFileNameCR path, bvector<Utf8String> const& groupingFields);
};

END_ECPRESENTATIONTESTS_NAMESPACE
