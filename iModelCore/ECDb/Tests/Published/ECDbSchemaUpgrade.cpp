/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaUpgrade.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECDbSchemaUpgrade : ECDbTestFixture
    {
    };

struct ECSchemaDuplicator :IECSchemaLocater
    {
    private:
        ECSchemaCacheCR m_fromCache;
    private:
        virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override
            {
            if (auto schema = schemaContext.GetCache().GetSchema(key, matchType))
                return schema;

            if (auto schema = m_fromCache.GetSchema(key, matchType))
                {
                Utf8String xml;
                if (SchemaWriteStatus::Success != schema->WriteToXmlString(xml))
                    return nullptr;

                ECSchemaPtr out;
                if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(out, xml.c_str(), schemaContext))
                    return nullptr;

                return out;
                }

            return nullptr;
            }
        ECSchemaDuplicator(ECSchemaCacheCR fromCache)
            :m_fromCache(fromCache)
            {}
    public:
        static ECSchemaCachePtr Duplicate(ECSchemaCacheCR fromCache)
            {
            ECSchemaReadContextPtr ptr = ECSchemaReadContext::CreateContext();
            ECSchemaDuplicator loc(fromCache);
            ptr->AddSchemaLocater(loc);
            bvector<ECSchemaP> schemas;
            fromCache.GetSchemas(schemas);
            for (auto schema : schemas)
                {
                ptr->LocateSchema(const_cast<SchemaKeyR>(schema->GetSchemaKey()), SchemaMatchType::Exact);
                }

            return &ptr->GetCache();
            }
        static ECSchemaCachePtr Duplicate(ECSchemaCR schema)
            {
            ECSchemaCachePtr ptr = ECSchemaCache::Create();
            ptr->AddSchema(const_cast<ECSchemaR>(schema));
            return Duplicate(*ptr);
            }
        static ECSchemaCachePtr Duplicate(ECSchemaReadContextR ctx)
            {
            return Duplicate(ctx.GetCache());
            }
    };
TEST_F(ECDbSchemaUpgrade, UpdateECSchemaAttributes)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
        {
        ECSchemaPtr test_1_0_0;
        ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
        test_1_0_0->SetDisplayLabel("Test ECSchema");
        test_1_0_0->SetDescription("This is a test schema");

        ECSchemaCachePtr cache1 = ECSchemaCache::Create();
        cache1->AddSchema(*test_1_0_0);
        ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
        }
        //Upgrade with some changes
        {
        ECSchemaPtr test_1_0_0;
        //modified schema info
        ECSchema::CreateSchema(test_1_0_0, "test", "t1", 1, 1, 1);
        test_1_0_0->SetDisplayLabel("Test ECSchema (MODIFIED)");
        test_1_0_0->SetDescription("This is a test schema  (MODIFIED)");

        ECSchemaCachePtr cache1 = ECSchemaCache::Create();
        cache1->AddSchema(*test_1_0_0);

        ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
        }


        ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
        ASSERT_TRUE(test != nullptr);
        ASSERT_TRUE(test->GetNamespacePrefix() == "t1");
        ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (MODIFIED)");
        ASSERT_TRUE(test->GetDescription() == "This is a test schema  (MODIFIED)");
    }

TEST_F(ECDbSchemaUpgrade, UpdateECClassAttributes)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    {
    ECSchemaPtr test_1_0_0;
    ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
    test_1_0_0->SetDisplayLabel("Test ECSchema");
    test_1_0_0->SetDescription("This is a test schema");
    
    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book");
    test_book->SetDescription("Represent a generic book");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }
    //Upgrade with some changes
    {
    ECSchemaPtr test_1_0_0;
    ECSchema::CreateSchema(test_1_0_0, "test", "t1", 1, 0, 0);
    test_1_0_0->SetDisplayLabel("Test ECSchema (MODIFIED)");
    test_1_0_0->SetDescription("This is a test schema (MODIFIED)");

    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book (MODIFIED)");
    test_book->SetDescription("Represent a generic book (MODIFIED)");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }

    ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
    ASSERT_TRUE(test != nullptr);
    ASSERT_TRUE(test->GetNamespacePrefix() == "t1");
    ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (MODIFIED)");
    ASSERT_TRUE(test->GetDescription() == "This is a test schema (MODIFIED)");

    ECClassCP book = test->GetClassCP("Book");
    ASSERT_TRUE(book != nullptr);
    ASSERT_TRUE(book->GetDisplayLabel() == "Book (MODIFIED)");
    ASSERT_TRUE(book->GetDescription() == "Represent a generic book (MODIFIED)");

    }



TEST_F(ECDbSchemaUpgrade, UpdateECPropertyAttributes)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    {
    ECSchemaPtr test_1_0_0;
    ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
    auto ecdbMap = ecdb.Schemas().GetECSchema("ECDbMap");
    test_1_0_0->AddReferencedSchema(const_cast<ECSchemaR>(*ecdbMap));
 
    test_1_0_0->SetDisplayLabel("Test ECSchema");
    test_1_0_0->SetDescription("This is a test schema");

    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book");
    test_book->SetDescription("Represent a generic book");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code");
    test_book_code->SetDescription("Code of the book");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);

    StandaloneECInstancePtr inst = ecdbMap->GetClassCP("PropertyMap")->GetDefaultStandaloneEnabler()->CreateInstance();
    inst->SetValue("IsNullable", ECValue(true));
    test_book_code->SetCustomAttribute(*inst);


    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }
    //Upgrade with some changes
    {
    ECSchemaPtr test_1_0_0;

    //modified schema info
    ECSchema::CreateSchema(test_1_0_0, "test", "t1", 1, 1, 1);
    auto ecdbMap = ecdb.Schemas().GetECSchema("ECDbMap");
    test_1_0_0->AddReferencedSchema(const_cast<ECSchemaR>(*ecdbMap));

    test_1_0_0->SetDisplayLabel("Test ECSchema (MODIFIED)");
    test_1_0_0->SetDescription("This is a test schema  (MODIFIED)");
    //MODIFIED existing class
    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book (MODIFIED)");
    test_book->SetDescription("Represent a generic book (MODIFIED)");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code (MODIFIED)");
    test_book_code->SetDescription("Code of the book (MODIFIED)");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);

    StandaloneECInstancePtr inst = ecdbMap->GetClassCP("PropertyMap")->GetDefaultStandaloneEnabler()->CreateInstance();
    inst->SetValue("IsNullable", ECValue(false));
    inst->SetValue("ColumnName", ECValue("Code__1"));

    test_book_code->SetCustomAttribute(*inst);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }

    ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
    ASSERT_TRUE(test != nullptr);
    ASSERT_TRUE(test->GetNamespacePrefix() == "t1");
    ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (MODIFIED)");
    ASSERT_TRUE(test->GetDescription() == "This is a test schema  (MODIFIED)");

    ECClassCP book = test->GetClassCP("Book");
    ASSERT_TRUE(book != nullptr);
    ASSERT_TRUE(book->GetDisplayLabel() == "Book (MODIFIED)");
    ASSERT_TRUE(book->GetDescription() == "Represent a generic book (MODIFIED)");
    }

TEST_F(ECDbSchemaUpgrade, AddECClass)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    {
    ECSchemaPtr test_1_0_0;
    ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
    test_1_0_0->SetDisplayLabel("Test ECSchema");
    test_1_0_0->SetDescription("This is a test schema");

    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book");
    test_book->SetDescription("Represent a generic book");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code");
    test_book_code->SetDescription("Code of the book");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }
    //Upgrade with some changes
    {
    ECSchemaPtr test_1_0_0;

    //modified schema info
    ECSchema::CreateSchema(test_1_0_0, "test", "t1", 1, 1, 1);
    test_1_0_0->SetDisplayLabel("Test ECSchema (MODIFIED)");
    test_1_0_0->SetDescription("This is a test schema  (MODIFIED)");
    //MODIFIED existing class
    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book (MODIFIED)");
    test_book->SetDescription("Represent a generic book (MODIFIED)");
    test_book->SetClassModifier(ECClassModifier::Abstract);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code (MODIFIED)");
    test_book_code->SetDescription("Code of the book (MODIFIED)");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);
    //New Class
    ECEntityClassP test_scifi_book;
    test_1_0_0->CreateEntityClass(test_scifi_book, "ScifiBook");
    test_scifi_book->SetDisplayLabel("Scifi Book");
    test_scifi_book->SetDescription("Scifi Book base class");
    test_scifi_book->SetClassModifier(ECClassModifier::None);
    test_scifi_book->AddBaseClass(*test_scifi_book);

    PrimitiveECPropertyP test_scifi_book_catalog_code;
    test_scifi_book->CreatePrimitiveProperty(test_scifi_book_catalog_code, "CatalogCode");
    test_scifi_book_catalog_code->SetDisplayLabel("Catalog Code");
    test_scifi_book_catalog_code->SetDescription("Catalog Code of the book");
    test_scifi_book_catalog_code->SetType(PrimitiveType::PRIMITIVETYPE_Integer);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }

    ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
    ASSERT_TRUE(test != nullptr);
    ASSERT_TRUE(test->GetNamespacePrefix() == "t1");
    ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (MODIFIED)");
    ASSERT_TRUE(test->GetDescription() == "This is a test schema  (MODIFIED)");

    ECClassCP book = test->GetClassCP("Book");
    ASSERT_TRUE(book != nullptr);
    ASSERT_TRUE(book->GetDisplayLabel() == "Book (MODIFIED)");
    ASSERT_TRUE(book->GetDescription() == "Represent a generic book (MODIFIED)");


    ECClassCP scifi_book = test->GetClassCP("ScifiBook");
    ASSERT_TRUE(scifi_book != nullptr);
    ASSERT_TRUE(scifi_book->GetDisplayLabel() == "Scifi Book");
    ASSERT_TRUE(scifi_book->GetDescription() == "Scifi Book base class");
    }
TEST_F(ECDbSchemaUpgrade, AddECProperty)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    {
    ECSchemaPtr test_1_0_0;
    ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
    test_1_0_0->SetDisplayLabel("Test ECSchema");
    test_1_0_0->SetDescription("This is a test schema");

    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book");
    test_book->SetDescription("Represent a generic book");
    test_book->SetClassModifier(ECClassModifier::None);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code");
    test_book_code->SetDescription("Code of the book");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }
    //Upgrade with some changes
    {
    ECSchemaPtr test_1_0_0;
    //

    //modified schema info
    ECSchema::CreateSchema(test_1_0_0, "test", "t1", 1, 1, 1);
    test_1_0_0->SetDisplayLabel("Test ECSchema (MODIFIED)");
    test_1_0_0->SetDescription("This is a test schema  (MODIFIED)");
    //MODIFIED existing class
    ECEntityClassP test_book;
    test_1_0_0->CreateEntityClass(test_book, "Book");
    test_book->SetDisplayLabel("Book (MODIFIED)");
    test_book->SetDescription("Represent a generic book (MODIFIED)");
    test_book->SetClassModifier(ECClassModifier::None);

    PrimitiveECPropertyP test_book_code;
    test_book->CreatePrimitiveProperty(test_book_code, "Code");
    test_book_code->SetDisplayLabel("DOI Code (MODIFIED)");
    test_book_code->SetDescription("Code of the book (MODIFIED)");
    test_book_code->SetType(PrimitiveType::PRIMITIVETYPE_String);

    PrimitiveECPropertyP test_book_name;
    test_book->CreatePrimitiveProperty(test_book_name, "Name");
    test_book_name->SetDisplayLabel("Name");
    test_book_name->SetDescription("Name of the book");
    test_book_name->SetType(PrimitiveType::PRIMITIVETYPE_String);

    ECSchemaCachePtr cache1 = ECSchemaCache::Create();
    cache1->AddSchema(*test_1_0_0);

    ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
    }

    ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
    ASSERT_TRUE(test != nullptr);
    ASSERT_TRUE(test->GetNamespacePrefix() == "t1");
    ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (MODIFIED)");
    ASSERT_TRUE(test->GetDescription() == "This is a test schema  (MODIFIED)");

    ECClassCP book = test->GetClassCP("Book");
    ASSERT_TRUE(book != nullptr);
    ASSERT_TRUE(book->GetDisplayLabel() == "Book (MODIFIED)");
    ASSERT_TRUE(book->GetDescription() == "Represent a generic book (MODIFIED)");

    ECPropertyCP code = book->GetPropertyP("Code");
    ASSERT_TRUE(code != nullptr);
    ASSERT_TRUE(code->GetDisplayLabel() == "DOI Code (MODIFIED)");
    ASSERT_TRUE(code->GetDescription() == "Code of the book (MODIFIED)");

    ECPropertyCP name = book->GetPropertyP("Name");
    ASSERT_TRUE(name != nullptr);
    ASSERT_TRUE(name->GetDisplayLabel() == "Name");
    ASSERT_TRUE(name->GetDescription() == "Name of the book");
    }
END_ECDBUNITTESTS_NAMESPACE
