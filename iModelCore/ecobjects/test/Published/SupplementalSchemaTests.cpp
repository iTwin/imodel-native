/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SupplementalSchemaTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <objbase.h>
#include <comdef.h>
#include "TestFixture.h"

BEGIN_BENTLEY_EC_NAMESPACE

struct SupplementalSchemaMetaDataTests     : ECTestFixture  
    {
    virtual bool _WantInstanceLeakDetection () override { return false; }
    };

ECSchemaP CreatePrimarySchema(IECSchemaOwnerR schemaOwner)
    {
    ECSchemaP schema;
    ECClassP widgetClass;

    ECSchema::CreateSchema(schema, L"TestSchema", 1, 0, schemaOwner);
    schema->CreateClass(widgetClass, L"Widget");

    return schema;

    }

ECSchemaP CreateSupplementalSchema(ECSchemaCachePtr schemaOwner)
    {
    ECSchemaP schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);

    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, 
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"TestSchema_Supplemental_OverrideWidgets\" version=\"01.00\" displayLabel=\"TestSchema_Supplemental_OverrideWidgets\" description=\"TestSchema_Supplemental_OverrideWidgets Description\" nameSpacePrefix=\"tss\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        L"<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.04\" prefix=\"bsca\" />"
        L"</ECSchema>", *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    

    SupplementalSchemaMetaData metaData(L"TestSchema", 1, 0, 200, L"OverrideWidgets", false);
    EXPECT_STREQ(L"TestSchema", metaData.GetPrimarySchemaName().c_str());
    
    schema->SetCustomAttribute(*(metaData.CreateCustomAttribute()));
    return schema;
    }

TEST_F(SupplementalSchemaMetaDataTests, CanRetrieveFromSchema)
    {
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    ECSchemaCachePtr  schemaOwner = ECSchemaCache::Create();
    ECSchemaP supplemental = CreateSupplementalSchema(schemaOwner);

    EXPECT_TRUE(NULL != supplemental);

    Bentley::EC::SupplementalSchemaMetaDataPtr metaData;
    EXPECT_TRUE(SupplementalSchemaMetaData::TryGetFromSchema(metaData, *supplemental));
    EXPECT_TRUE(metaData.IsValid());
    EXPECT_STREQ(L"TestSchema", metaData->GetPrimarySchemaName().c_str());
    EXPECT_EQ(1, metaData->GetPrimarySchemaMajorVersion());
    EXPECT_EQ(0, metaData->GetPrimarySchemaMinorVersion());
    EXPECT_EQ(200, metaData->GetSupplementalSchemaPrecedence());
    EXPECT_STREQ(L"OverrideWidgets", metaData->GetSupplementalSchemaPurpose().c_str());
    EXPECT_FALSE(metaData->IsUserSpecific());

    CoUninitialize();
    }


END_BENTLEY_EC_NAMESPACE
