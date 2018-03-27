/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PerformanceECXmlTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformanceTestsECXml : PerformanceTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Muhammad.hassan                 09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECXml, ReadingAndWritingSchema)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    TimeSchema(L"OpenPlant.01.02.ecschema.xml", schemaContext, TEST_DETAILS);
    TimeSchema(L"OpenPlant_PID.01.02.ecschema.xml", schemaContext, TEST_DETAILS);
    TimeSchema(L"OpenPlant_3D.01.02.ecschema.xml", schemaContext, TEST_DETAILS);
    TimeSchema(L"Bentley_Plant.06.00.ecschema.xml", schemaContext, TEST_DETAILS);
    TimeSchema(L"CustomAttributeTest.01.00.ecschema.xml", schemaContext, TEST_DETAILS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Muhammad.hassan                 09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECXml, ReadingAndWritingInstance)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    TimeInstance(L"ECRules.01.00.ecschema.xml", L"RuleSet.xml", schemaContext, TEST_DETAILS);
    TimeInstance(L"OpenPlant_3D.01.02.ecschema.xml", L"OpenPlant_3D_Instance.xml", schemaContext, TEST_DETAILS);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
