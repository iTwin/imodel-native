/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECSchemaTestFixture.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#include "..\..\Published\TestFixture.h"

#include <iostream>
#include <objbase.h>
#include <comdef.h>
using namespace std;

using namespace TestHelpers;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif      7/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTestFixture::SetUp()
    {
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));
    //Load Schema
    //TestDataManager tdm(L"Widgets.09.06.ecschema.xml", __FILE__, OPENMODE_READWRITE);
    WString schemaPath = ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml");
    wcout<<" Schema Path: "<<schemaPath<<endl;
    schemaOwner = ECSchemaOwner::CreateOwner();
    SchemaDeserializationStatus status = ECSchemaVerifier::ReadXmlFromFile (schemaOwner, m_schema, schemaPath.c_str(), NULL, NULL);

    ASSERT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, status)<< schemaPath.c_str();
    ASSERT_TRUE(m_schema != NULL);
    }
