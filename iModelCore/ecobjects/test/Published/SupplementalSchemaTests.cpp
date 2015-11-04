/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SupplementalSchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"


BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct SchemaHolderTestFixture : ECTestFixture
    {
    DEFINE_T_SUPER (ECTestFixture)

    protected:
        ECSchemaPtr m_bscaSchema;

        void CreateSupplementalSchema
        (
        ECSchemaPtr& supplementalSchema,
        Utf8String primarySchemaName,
        Utf8String purpose,
        uint32_t precedence,
        uint32_t primaryMajorVersion,
        uint32_t primaryMinorVersion,
        uint32_t supplementalMajorVersion,
        uint32_t supplementalMinorVersion
        )
            {
            Utf8String supplementalName = primarySchemaName + "_Supplemental_" + purpose;

            ECSchema::CreateSchema(supplementalSchema, supplementalName, supplementalMajorVersion, supplementalMinorVersion);
            SupplementalSchemaMetaData metaData(primarySchemaName, primaryMajorVersion, primaryMinorVersion, precedence, purpose, false);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));
            }

    public:
        virtual void            SetUp () override 
            { 
            // EXPECT_EQ (S_OK, CoInitialize(NULL)); 

            ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
            SchemaKey key ("Bentley_Standard_CustomAttributes", 1, 6);

            m_bscaSchema = ECSchema::LocateSchema(key, *schemaContext);
            ASSERT_TRUE( m_bscaSchema.IsValid() );
                
            T_Super::SetUp();
            }

        virtual void TearDown() override
            {
            // CoUninitialize();
            T_Super::TearDown();
            }

    };

struct SupplementalSchemaMetaDataTests     : SchemaHolderTestFixture {};

struct SupplementalSchemaInfoTests     : SchemaHolderTestFixture   {};

struct SupplementedSchemaBuilderTests : SchemaHolderTestFixture 
    {
    DEFINE_T_SUPER (SchemaHolderTestFixture)

    protected:
        ECSchemaPtr m_customAttributeSchema;
        StandaloneECEnablerPtr m_systemInfoCAEnabler;
        StandaloneECEnablerPtr m_uselessInfoCAEnabler;
        StandaloneECEnablerPtr m_otherInfoCAEnabler;

        ECSchemaPtr m_primaryTestSchema;
        ECSchemaPtr m_supplementalTestSchema1;
        ECSchemaPtr m_supplementalTestSchema2;
        ECSchemaPtr m_supplementalTestSchema3;
        ECSchemaPtr m_supplementalTestSchema4;

        ECSchemaPtr m_primaryTestSchemaCopy;
        ECSchemaPtr m_supplementalTestSchema1Copy;
        ECSchemaPtr m_supplementalTestSchema2Copy;
        ECSchemaPtr m_supplementalTestSchema3Copy;
        ECSchemaPtr m_supplementalTestSchema4Copy;

        void CreateCustomAttributeSchema()
            {
            ECSchema::CreateSchema(m_customAttributeSchema, "Test_Custom_Attributes", 1, 0);
            ECCustomAttributeClassP customAttributeClass;
            ECCustomAttributeClassP customAttributeClass2;
            ECCustomAttributeClassP customAttributeClass3;

            PrimitiveECPropertyP property1;
            PrimitiveECPropertyP property2;
            m_customAttributeSchema->CreateCustomAttributeClass(customAttributeClass, "SystemInfo");
            customAttributeClass->CreatePrimitiveProperty(property1, "Data1");
            customAttributeClass->CreatePrimitiveProperty(property2, "Data2");
            m_systemInfoCAEnabler = customAttributeClass->GetDefaultStandaloneEnabler();

            PrimitiveECPropertyP property3;
            PrimitiveECPropertyP property4;
            m_customAttributeSchema->CreateCustomAttributeClass(customAttributeClass2, "UselessInfo");
            customAttributeClass2->CreatePrimitiveProperty(property3, "NothingImportant");
            customAttributeClass2->CreatePrimitiveProperty(property4, "NotImportant");
            m_uselessInfoCAEnabler = customAttributeClass2->GetDefaultStandaloneEnabler();

            PrimitiveECPropertyP property5;
            PrimitiveECPropertyP property6;
            m_customAttributeSchema->CreateCustomAttributeClass(customAttributeClass3, "OtherInformation");
            customAttributeClass3->CreatePrimitiveProperty(property5, "SomeOtherInformation");
            customAttributeClass3->CreatePrimitiveProperty(property6, "SomeInformation");
            m_otherInfoCAEnabler = customAttributeClass3->GetDefaultStandaloneEnabler();
            }

        void SetCustomAttribute(IECCustomAttributeContainerP container, StandaloneECEnablerPtr enabler, Utf8CP propertyName1, Utf8CP propertyValue1, Utf8CP propertyName2, Utf8CP propertyValue2)
            {
            IECInstancePtr customAttribute = enabler->CreateInstance().get();
            customAttribute->SetValue(propertyName1, ECValue(propertyValue1));
            customAttribute->SetValue(propertyName2, ECValue(propertyValue2));
            container->SetCustomAttribute(*customAttribute);            
            }

        void CreatePrimarySchema(ECSchemaPtr& primarySchema)
            {
            ECEntityClassP fileClass;

            PrimitiveECPropertyP fileSizeProperty;
            PrimitiveECPropertyP creationDateProperty;
            PrimitiveECPropertyP hiddenProperty;
            ECSchema::CreateSchema(primarySchema, "TestSchema", 1, 0);
            primarySchema->CreateEntityClass(fileClass, "File");
            primarySchema->AddReferencedSchema(*m_customAttributeSchema);
            primarySchema->AddReferencedSchema(*m_bscaSchema);
            fileClass->CreatePrimitiveProperty(hiddenProperty, "Hidden", PRIMITIVETYPE_Boolean);
            fileClass->CreatePrimitiveProperty(creationDateProperty, "CreationDate", PRIMITIVETYPE_DateTime);
            fileClass->CreatePrimitiveProperty(fileSizeProperty, "FileSize", PRIMITIVETYPE_Long);

            SetCustomAttribute(fileClass, m_systemInfoCAEnabler, "Data1", "Data1 on File Class", "Data2", "Data2 on File Class");

            SetCustomAttribute(hiddenProperty, m_systemInfoCAEnabler, "Data1", "Data1 on Hidden Property on File Class", "Data2", "Data2 on Hidden Property on File Class");

            SetCustomAttribute(hiddenProperty, m_uselessInfoCAEnabler, "NothingImportant", "Nothing important on Hidden Property on File Class", "NotImportant", "Not important on Hidden Property on File Class");

            ECEntityClassP folderClass;
            PrimitiveECPropertyP hiddenProperty2;
            PrimitiveECPropertyP creationDateProperty2;
            PrimitiveECPropertyP readOnlyProperty;
            primarySchema->CreateEntityClass(folderClass, "Folder");
            folderClass->CreatePrimitiveProperty(hiddenProperty2, "Hidden", PRIMITIVETYPE_Boolean);
            folderClass->CreatePrimitiveProperty(creationDateProperty2, "CreationDate", PRIMITIVETYPE_DateTime);
            folderClass->CreatePrimitiveProperty(readOnlyProperty, "ReadOnly", PRIMITIVETYPE_Boolean);
            SetCustomAttribute(folderClass, m_systemInfoCAEnabler, "Data1", "Data1 on Folder Class", "Data2", "Data2 on Folder Class");
            SetCustomAttribute(hiddenProperty2, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on Hidden Property on Folder Class", "NotImportant", "Not important on Hidden Property on Folder Class");

            ECEntityClassP imageClass;
            primarySchema->CreateEntityClass(imageClass, "Image");
            imageClass->AddBaseClass(*fileClass);
            PrimitiveECPropertyP bitDepthProperty;
            PrimitiveECPropertyP widthProperty;
            PrimitiveECPropertyP heightProperty;
            imageClass->CreatePrimitiveProperty(bitDepthProperty, "BitDepth", PRIMITIVETYPE_Integer);
            imageClass->CreatePrimitiveProperty(widthProperty, "Width", PRIMITIVETYPE_Integer);
            imageClass->CreatePrimitiveProperty(heightProperty, "Height", PRIMITIVETYPE_Integer);
            SetCustomAttribute(imageClass, m_systemInfoCAEnabler, "Data1", "Data1 on Image Class", "Data2", "Data2 on Image Class");
            SetCustomAttribute(widthProperty, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on Width Property on Image Class", "NotImportant", "Not important on Width Property on Image Class");
            }

        void CreateSupplementalSchema1(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "TestSchema_Supplemental_OverrideFiles", 1, 0);
            SupplementalSchemaMetaData metaData("TestSchema", 1, 0, 200, "OverrideFiles", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP fileClass;
            PrimitiveECPropertyP hiddenProperty;
            supplementalSchema->CreateEntityClass(fileClass, "File");
            fileClass->CreatePrimitiveProperty(hiddenProperty, "Hidden", PRIMITIVETYPE_Boolean);

            SetCustomAttribute(fileClass, m_systemInfoCAEnabler, "Data1", "Data1 on File Class from SupplementalSchema1", "Data2", "Data2 on File Class from SupplementalSchema1");
            SetCustomAttribute(hiddenProperty, m_uselessInfoCAEnabler, "NothingImportant", "Nothing important on Hidden Property on File Class from SupplementalSchema1", "NotImportant", "Not important on Hidden Property on File Class from SupplementalSchema1");
            }

        void CreateSupplementalSchema2(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "TestSchema_Supplemental_UnderrideFiles_ExtraText", 1, 0);
            SupplementalSchemaMetaData metaData("TestSchema", 1, 0, 199, "UnderrideFiles", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP folderClass;
            PrimitiveECPropertyP creationDateProperty;
            supplementalSchema->CreateEntityClass(folderClass, "Folder");
            folderClass->CreatePrimitiveProperty(creationDateProperty, "CreationDate", PRIMITIVETYPE_DateTime);

            SetCustomAttribute(folderClass, m_uselessInfoCAEnabler, "NothingImportant", "Nothing important on Folder Class from SupplementalSchema2", "NotImportant", "Not important on Folder Class from SupplementalSchema2");
            SetCustomAttribute(creationDateProperty, m_systemInfoCAEnabler, "Data1", "Data1 on CreationDate Property on Folder Class from SupplementalSchema2", "Data2", "Data2 on CreationDate Property on Folder Class from SupplementalSchema2");
            }

        void CreateSupplementalSchema3(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "TestSchema_Supplemental_FileAndImageInfo", 3, 33);
            SupplementalSchemaMetaData metaData("TestSchema", 1, 0, 200, "FileAndImageInfo", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP fileClass;
            PrimitiveECPropertyP hiddenProperty;
            PrimitiveECPropertyP creationDateProperty;
            PrimitiveECPropertyP fileSizeProperty;
            supplementalSchema->CreateEntityClass(fileClass, "File");
            fileClass->CreatePrimitiveProperty(hiddenProperty, "Hidden", PRIMITIVETYPE_Boolean);
            fileClass->CreatePrimitiveProperty(creationDateProperty, "CreationDate", PRIMITIVETYPE_DateTime);
            fileClass->CreatePrimitiveProperty(fileSizeProperty, "FileSize", PRIMITIVETYPE_Long);

            SetCustomAttribute(fileClass, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on File Class from Supplemental3", "NotImportant", "Not important on File Class from Supplemental3");

            SetCustomAttribute(hiddenProperty, m_systemInfoCAEnabler, "Data1", "Data1 on Hidden Property on File Class from Supplemental3", "Data2", "Data2 on Hidden Property on File Class from Supplemental3");

            SetCustomAttribute(creationDateProperty, m_uselessInfoCAEnabler, "NothingImportant", "Nothing important on CreationDate Property on File Class from Supplemental3", "NotImportant", "Not important on CreationDate Property on File Class from Supplemental3");

            SetCustomAttribute(fileSizeProperty, m_systemInfoCAEnabler, "Data1", "Data1 on FileSize Property on File Class from Supplemental3", "Data2", "Data2 on FileSize Property on File Class from Supplemental3");

            ECEntityClassP imageClass;
            supplementalSchema->CreateEntityClass(imageClass, "Image");
            PrimitiveECPropertyP heightProperty;
            PrimitiveECPropertyP hiddenProperty2;
            imageClass->CreatePrimitiveProperty(heightProperty, "Height", PRIMITIVETYPE_Integer);
            imageClass->CreatePrimitiveProperty(hiddenProperty2, "Hidden", PRIMITIVETYPE_Boolean);
            SetCustomAttribute(imageClass, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on Image Class from Supplemental3", "NotImportant", "Not important on Image Class from Supplemental3");
            SetCustomAttribute(heightProperty, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on Height Property on Image Class from Supplemental3", "NotImportant", "Not important on Height Property on Image Class from Supplemental3");
            SetCustomAttribute(hiddenProperty2, m_uselessInfoCAEnabler,  "NothingImportant", "Nothing important on Hidden Property on Image Class from Supplemental3",  "NotImportant", "Not important on Hidden Property on Image Class from Supplemental3");
            }

        void CreateSupplementalSchema4(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "TestSchema_Supplemental_Conflict", 6, 66);
            SupplementalSchemaMetaData metaData("TestSchema", 1, 0, 199, "Conflict", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP folderClass;
            PrimitiveECPropertyP creationDateProperty;
            supplementalSchema->CreateEntityClass(folderClass, "Folder");
            folderClass->CreatePrimitiveProperty(creationDateProperty, "CreationDate", PRIMITIVETYPE_DateTime);

            SetCustomAttribute(folderClass, m_uselessInfoCAEnabler, "NothingImportant", "Nothing important on Folder Class from SupplementalSchema4", "NotImportant", "Not important on Folder Class from SupplementalSchema4");
            SetCustomAttribute(creationDateProperty, m_systemInfoCAEnabler, "Data1", "Data1 on CreationDate Property on Folder Class from SupplementalSchema4", "Data2", "Data2 on CreationDate Property on Folder Class from SupplementalSchema4");
            }

        void BuildSupplementedSchemaForCustomAttributeTests(ECClassP& classA, ECClassP& classB, ECSchemaPtr& schema)
            {
            ECSchemaPtr customAttributeSchema;
            ECSchema::CreateSchema(customAttributeSchema, "CustomAttributes", (uint32_t) 1, (uint32_t) 0);
            ECCustomAttributeClassP customAttributeA = NULL;
            customAttributeSchema->CreateCustomAttributeClass(customAttributeA, "CustomAttributeA");
            ECCustomAttributeClassP customAttributeB = NULL;
            customAttributeSchema->CreateCustomAttributeClass(customAttributeB, "CustomAttributeB");
            ECCustomAttributeClassP customAttributeC = NULL;
            customAttributeSchema->CreateCustomAttributeClass(customAttributeC, "CustomAttributeC");
            ECCustomAttributeClassP customAttributeD = NULL;
            customAttributeSchema->CreateCustomAttributeClass(customAttributeD, "CustomAttributeD");

            ECSchema::CreateSchema(schema, "testPrimary", (uint32_t) 1, (uint32_t) 0);
            schema->AddReferencedSchema(*customAttributeSchema, "cas");

            StandaloneECEnablerPtr customAttributeEnablerA = customAttributeA->GetDefaultStandaloneEnabler();
            StandaloneECEnablerPtr customAttributeEnablerB = customAttributeB->GetDefaultStandaloneEnabler();
            StandaloneECEnablerPtr customAttributeEnablerC = customAttributeC->GetDefaultStandaloneEnabler();
            StandaloneECEnablerPtr customAttributeEnablerD = customAttributeD->GetDefaultStandaloneEnabler();

            ECEntityClassP primaryClassA = NULL;
            schema->CreateEntityClass(primaryClassA, "A");
            primaryClassA->SetCustomAttribute(*customAttributeEnablerA->CreateInstance());
            ECEntityClassP primaryClassB = NULL;
            schema->CreateEntityClass(primaryClassB, "B");
            primaryClassB->SetCustomAttribute(*customAttributeEnablerB->CreateInstance());
            primaryClassB->AddBaseClass(*primaryClassA);

            ECSchemaPtr supplementalSchema;
            ECSchema::CreateSchema(supplementalSchema, "testSupplemental", (uint32_t) 1, (uint32_t) 0);
            SupplementalSchemaMetaData metaData("testPrimary", 1, 0, 354, "None", false);
            supplementalSchema->AddReferencedSchema(*customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));
            ECEntityClassP supplementalClassA = NULL;
            supplementalSchema->CreateEntityClass(supplementalClassA, "A");
            supplementalClassA->SetCustomAttribute(*customAttributeEnablerC->CreateInstance());
            ECEntityClassP supplementalClassB = NULL;
            supplementalSchema->CreateEntityClass(supplementalClassB, "B");
            supplementalClassB->SetCustomAttribute(*customAttributeEnablerD->CreateInstance());

            bvector<ECSchemaP> supplementalSchemas;
            supplementalSchemas.push_back(supplementalSchema.get());
            SupplementedSchemaBuilder builder;
            builder.UpdateSchema(*schema, supplementalSchemas);
            classA = schema->GetClassP("A");
            classB = schema->GetClassP("B");
            }

        void CreatePrimarySchemaForRelationshipTests(ECSchemaPtr& schema)
            {
            ECSchema::CreateSchema(schema, "RelationshipTestSchema", (uint32_t) 1, (uint32_t) 2);
            ECEntityClassP targetClass = NULL;
            schema->CreateEntityClass(targetClass, "TargetClass");
            ECEntityClassP sourceClass = NULL;
            schema->CreateEntityClass(sourceClass, "SourceClass");

            ECRelationshipClassP relClass = NULL;
            schema->CreateRelationshipClass(relClass, "RelationshipWithCustomAttributes");
            relClass->GetSource().AddClass(*sourceClass);
            relClass->GetTarget().AddClass(*targetClass);

            ECRelationshipClassP relClass2 = NULL;
            schema->CreateRelationshipClass(relClass2, "RelClass2");
            relClass2->GetSource().AddClass(*sourceClass);
            relClass2->GetTarget().AddClass(*targetClass);
            }

        void CreateSupplementalSchema0ForRelationshipTests(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "RTS_Supplemental", (uint32_t) 3, (uint32_t) 4);
            SupplementalSchemaMetaData metaData("RelationshipTestSchema", 1, 2, 200, "Test", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP targetClass_Sup = NULL;
            supplementalSchema->CreateEntityClass(targetClass_Sup, "TargetClass");
            ECEntityClassP sourceClass_Sup = NULL;
            supplementalSchema->CreateEntityClass(sourceClass_Sup, "SourceClass");

            ECRelationshipClassP relClass_Sup = NULL;
            supplementalSchema->CreateRelationshipClass(relClass_Sup, "RelationshipWithCustomAttributes");
            relClass_Sup->GetSource().AddClass(*sourceClass_Sup);
            relClass_Sup->GetTarget().AddClass(*targetClass_Sup);

            relClass_Sup->GetSource().SetCustomAttribute(*m_systemInfoCAEnabler->CreateInstance());
            relClass_Sup->GetTarget().SetCustomAttribute(*m_systemInfoCAEnabler->CreateInstance());
            }

        void CreateSupplementalSchema1ForRelationshipTests(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "RTS_Supplemental2", (uint32_t) 5, (uint32_t) 6);
            SupplementalSchemaMetaData metaData("RelationshipTestSchema", 1, 2, 200, "Test", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            ECEntityClassP targetClass_Sup = NULL;
            supplementalSchema->CreateEntityClass(targetClass_Sup, "TargetClass");
            ECEntityClassP sourceClass_Sup = NULL;
            supplementalSchema->CreateEntityClass(sourceClass_Sup, "SourceClass");

            ECRelationshipClassP relClass_Sup = NULL;
            supplementalSchema->CreateRelationshipClass(relClass_Sup, "RelationshipWithCustomAttributes");
            relClass_Sup->GetSource().AddClass(*sourceClass_Sup);
            relClass_Sup->GetTarget().AddClass(*targetClass_Sup);

            relClass_Sup->GetSource().SetCustomAttribute(*m_uselessInfoCAEnabler->CreateInstance());
            relClass_Sup->GetTarget().SetCustomAttribute(*m_uselessInfoCAEnabler->CreateInstance());

            ECRelationshipClassP relClass2_Sup = NULL;
            supplementalSchema->CreateRelationshipClass(relClass2_Sup, "RelClass2");
            relClass2_Sup->GetSource().AddClass(*sourceClass_Sup);
            relClass2_Sup->GetTarget().AddClass(*targetClass_Sup);

            relClass2_Sup->GetSource().SetCustomAttribute(*m_otherInfoCAEnabler->CreateInstance());
            relClass2_Sup->GetTarget().SetCustomAttribute(*m_otherInfoCAEnabler->CreateInstance());
            }

        void CreatePrimarySchemaForInheritTests(ECSchemaPtr& primarySchema)
            {
            ECSchema::CreateSchema(primarySchema, "InheritTestSchema", 2, 34);
            primarySchema->AddReferencedSchema(*m_customAttributeSchema);
            // BaseClass1
                // BC1Prop1
                    // SystemInfo
                // BC1Prop2
                // BC1Prop3

            ECEntityClassP baseClass1;
            primarySchema->CreateEntityClass(baseClass1, "BaseClass1");
            PrimitiveECPropertyP bc1Prop1;
            baseClass1->CreatePrimitiveProperty(bc1Prop1, "BC1Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop1, m_systemInfoCAEnabler, "Data1", "InheritTestSchema.BaseClass1.BC1Prop1", "Data2", "InheritTestSchema.BaseClass1.BC1Prop1");
            PrimitiveECPropertyP bc1Prop2;
            baseClass1->CreatePrimitiveProperty(bc1Prop2, "BC1Prop2", PRIMITIVETYPE_String);
            PrimitiveECPropertyP bc1Prop3;
            baseClass1->CreatePrimitiveProperty(bc1Prop3, "BC1Prop3", PRIMITIVETYPE_String);

            // BaseClass2
                // BC2Prop1
                // BC2Prop2
            ECEntityClassP baseClass2;
            primarySchema->CreateEntityClass(baseClass2, "BaseClass2");
            PrimitiveECPropertyP bc2Prop1;
            baseClass2->CreatePrimitiveProperty(bc2Prop1, "BC2Prop1", PRIMITIVETYPE_String);
            PrimitiveECPropertyP bc2Prop2;
            baseClass2->CreatePrimitiveProperty(bc2Prop2, "BC2Prop2", PRIMITIVETYPE_String);

            // DerivedClass1
                // BaseClass1
            ECEntityClassP derivedClass1;
            primarySchema->CreateEntityClass(derivedClass1, "DerivedClass1");
            derivedClass1->AddBaseClass(*baseClass1);

            // DerivedClass2
                // BaseClass1
                // BC1Prop2
                    // UselessInfo
            ECEntityClassP derivedClass2;
            primarySchema->CreateEntityClass(derivedClass2, "DerivedClass2");
            derivedClass2->AddBaseClass(*baseClass1);
            PrimitiveECPropertyP bc1Prop2DerivedClass2;
            derivedClass2->CreatePrimitiveProperty(bc1Prop2DerivedClass2, "BC1Prop2", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop2DerivedClass2, m_uselessInfoCAEnabler, "NothingImportant", "InheritTestSchema.DerivedClass2.BC1Prop2", "NotImportant", "InheritTestSchema.DerivedClass2.BC1Prop2");

            // DerivedClass3
                // DerivedClass2
                // BCProp1
                    // SystemInfo
            ECEntityClassP derivedClass3;
            primarySchema->CreateEntityClass(derivedClass3, "DerivedClass3");
            derivedClass3->AddBaseClass(*derivedClass2);
            //PrimitiveECPropertyP bc1Prop1DerivedClass3;
            //derivedClass3->CreatePrimitiveProperty(bc1Prop1DerivedClass3, "BC1Prop1", PRIMITIVETYPE_String);
            //SetCustomAttribute(bc1Prop1DerivedClass3, m_systemInfoCAEnabler, "Data1", "InheritTestSchema.DerivedClass3.BC1Prop1 - Data1", "Data2", "InheritTestSchema.DerivedClass3.BC1Prop1 - Data2");

            // DerivedClass4
                // BaseClass2
            ECEntityClassP derivedClass4;
            primarySchema->CreateEntityClass(derivedClass4, "DerivedClass4");
            derivedClass4->AddBaseClass(*baseClass2);
            }

        void CreateLowPrioritySchema1(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "LowPrioritySchema1", 4, 3);
            SupplementalSchemaMetaData metaData("InheritTest", 2, 34, 1, "Test", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            // DerivedClass1
                // BC1Prop1
                    // SystemInfo
            ECEntityClassP derivedClass1;
            supplementalSchema->CreateEntityClass(derivedClass1, "DerivedClass1");
            PrimitiveECPropertyP bc1Prop1;
            derivedClass1->CreatePrimitiveProperty(bc1Prop1, "BC1Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop1, m_systemInfoCAEnabler, "Data1", "LowPrioritySchema1.DerivedClass1.BC1Prop1", "Data2", "LowPrioritySchema1.DerivedClass1.BC1Prop1");

            // DerivedClass2
                // BC1Prop2
                    // UselessInfo
            ECEntityClassP derivedClass2;
            supplementalSchema->CreateEntityClass(derivedClass2, "DerivedClass2");
            PrimitiveECPropertyP bc1Prop2;
            derivedClass2->CreatePrimitiveProperty(bc1Prop2, "BC1Prop2", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop2, m_uselessInfoCAEnabler, "Data1", "LowPrioritySchema1.DerivedClass2.BC1Prop2", "Data2", "LowPrioritySchema1.DerivedClass2.BC1Prop2");

            // DerivedClass4
                // BC2Prop1
                    // SystemInfo
            ECEntityClassP derivedClass4;
            supplementalSchema->CreateEntityClass(derivedClass4, "DerivedClass4");
            PrimitiveECPropertyP bc2Prop1DerivedClass4;
            derivedClass4->CreatePrimitiveProperty(bc2Prop1DerivedClass4, "BC2Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc2Prop1DerivedClass4, m_systemInfoCAEnabler, "Data1", "LowPrioritySchema1.DerivedClass4.BC2Prop1", "Data2", "LowPrioritySchema1.DerivedClass4.BC2Prop1");
            }

        void CreateLowPrioritySchema199(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "LowPrioritySchema199", 4, 3);
            SupplementalSchemaMetaData metaData("InheritTest", 2, 34, 199, "Test", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            // DerivedClass1
                // BC1Prop1
                    // SystemInfo
            ECEntityClassP derivedClass1;
            supplementalSchema->CreateEntityClass(derivedClass1, "DerivedClass1");
            PrimitiveECPropertyP bc1Prop1;
            derivedClass1->CreatePrimitiveProperty(bc1Prop1, "BC1Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop1, m_systemInfoCAEnabler, "Data1", "LowPrioritySchema199.DerivedClass1.BC1Prop1", "Data2", "LowPrioritySchema199.DerivedClass1.BC1Prop1");

            // DerivedClass2
                // BC1Prop2
                    // UselessInfo
            ECEntityClassP derivedClass2;
            supplementalSchema->CreateEntityClass(derivedClass2, "DerivedClass2");
            PrimitiveECPropertyP bc1Prop2;
            derivedClass2->CreatePrimitiveProperty(bc1Prop2, "BC1Prop2", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop2, m_uselessInfoCAEnabler, "Data1", "LowPrioritySchema199.DerivedClass2.BC1Prop2", "Data2", "LowPrioritySchema199.DerivedClass2.BC1Prop2");

            // DerivedClass3
                // BC1Prop1
                    // SystemInfo
            ECEntityClassP derivedClass3;
            supplementalSchema->CreateEntityClass(derivedClass3, "DerivedClass3");
            PrimitiveECPropertyP bc1Prop1DerivedClass3;
            derivedClass3->CreatePrimitiveProperty(bc1Prop1DerivedClass3, "BC1Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc1Prop1DerivedClass3, m_systemInfoCAEnabler, "Data1", "LowPrioritySchema199.DerivedClass3.BC1Prop1", "Data2", "LowPrioritySchema199.DerivedClass3.BC1Prop1");
            }

        void CreateHighPrioritySchema200(ECSchemaPtr& supplementalSchema)
            {
            ECSchema::CreateSchema(supplementalSchema, "HighPrioritySchema200", 4, 3);
            SupplementalSchemaMetaData metaData("InheritTest", 2, 34, 200, "Test", false);
            supplementalSchema->AddReferencedSchema(*m_customAttributeSchema);
            supplementalSchema->AddReferencedSchema(*m_bscaSchema);
            supplementalSchema->GetCustomAttributeContainer().SetCustomAttribute(*(metaData.CreateCustomAttribute()));

            // BaseClass2
                // BC2Prop1
            ECEntityClassP baseClass2;
            supplementalSchema->CreateEntityClass(baseClass2, "BaseClass2");
            PrimitiveECPropertyP bc2Prop1;
            baseClass2->CreatePrimitiveProperty(bc2Prop1, "BC2Prop1", PRIMITIVETYPE_String);
            SetCustomAttribute(bc2Prop1, m_systemInfoCAEnabler, "Data1", "HighPrioritySchema200.BaseClass2,BC2Prop1", "Data2", "HighPrioritySchema200.BaseClass2.BC2Prop1");
            }

        void ValidatePropertyValuesAreEqual(IECInstancePtr instanceA, IECInstancePtr instanceB)
            {
            ECPropertyIterableCR    collection  = instanceA->GetClass().GetProperties(true);
            for (ECPropertyP propertyA: collection)
                {
                ECValue valueA;
                instanceA->GetValue(valueA, propertyA->GetName().c_str());
                ECValue valueB;
                instanceB->GetValue(valueB, propertyA->GetName().c_str());
                if (valueA.IsPrimitive())
                    {
                    if (!valueB.IsPrimitive())
                        FAIL() << "valueA is a primitive but valueB is not";
                    else
                        {
                        if (valueA.IsString())
                            {
                            if (valueB.IsString())
                                {
                                EXPECT_STREQ(valueA.GetUtf8CP(), valueB.GetUtf8CP());
                                }
                            else
                                FAIL() << "Values of different primitive types";
                            }
                        else if (valueA.IsInteger())
                            {
                            if (valueB.IsInteger())
                                {
                                EXPECT_EQ(valueA.GetInteger(), valueB.GetInteger());
                                }
                            else
                                FAIL() << "Values of different primitive types";
                            }
                        else if (valueA.IsBoolean())
                            {
                            if (valueB.IsBoolean())
                                {
                                EXPECT_EQ(valueA.GetBoolean(), valueB.GetBoolean());
                                }
                            else
                                FAIL() << "Values of different primitive types";
                            }
                        else if (valueA.IsLong())
                            {
                            if (valueB.IsLong())
                                {
                                EXPECT_EQ(valueA.GetLong(), valueB.GetLong());
                                }
                            else
                                FAIL() << "Values of different primitive types";
                            }
                        }
                    }
                else if (valueA.IsArray() && !valueB.IsArray())
                    FAIL() << "valueA is an array but valueB is not";
                else if (valueA.IsStruct() && !valueB.IsStruct())
                    FAIL() << "valueA is a struct but valueB is not";
                }
            }

        bool CustomAttributesAreEqual(ECCustomAttributeInstanceIterable customAttributesA, ECCustomAttributeInstanceIterable customAttributesB)
            {
            uint32_t countA = 0;
            for (IECInstancePtr attributeA: customAttributesA)
                {
                countA++;
                Utf8StringCR className = attributeA->GetClass().GetName();
                bool customAttributeFound = false;
                for (IECInstancePtr attributeB: customAttributesB)
                    {
                    ECClassCR classB = attributeB->GetClass();
                    if (0 == className.compare(classB.GetName()))
                        {
                        ValidatePropertyValuesAreEqual(attributeA, attributeB);
                        customAttributeFound = true;
                        break;
                        }
                    }
                if (!customAttributeFound)
                    return false;
                }

            uint32_t countB = 0;
            for (IECInstancePtr attributeB: customAttributesB)
                {
                countB++;
                }
            EXPECT_EQ(countA, countB);

            return true;
            }

        void ValidateAreClassesIdentical(ECClassP classA, ECClassP classB, bool compareCustomAttributes)
            {
            EXPECT_TRUE (ECClass::ClassesAreEqualByName(classA, classB));
            //EXPECT_EQ(classA->GetSchema().GetSchemaKey(), classB->GetSchema().GetSchemaKey());

            EXPECT_EQ (classB->GetClassType(), classA->GetClassType());
            if (compareCustomAttributes)
                EXPECT_TRUE(CustomAttributesAreEqual(classA->GetCustomAttributes(true), classB->GetCustomAttributes(true)));

            //Assert.IsFalse (TestHelpers.AreIdentical (supplementedClass, originalClass), "The Supplemented and Original classes are identical.");
            //Assert.IsTrue  (TestHelpers.HasSameValues (originalClass.GetPrimaryCustomAttributes (SPECIES_SPECIFIC_CA),
            //    supplementedClass.GetPrimaryCustomAttributes (SPECIES_SPECIFIC_CA)),
            //    "The Supplemented and Original classes have different CustomAttributes even though they were accessed using 'GetPrimaryCustomAttributes'");

            }

        void ValidateSystemInfoCustomAttribute(IECCustomAttributeContainerP consolidatedContainer, Utf8CP expectedValue1, Utf8CP expectedValue2)
            {
            IECInstancePtr customAttribute = consolidatedContainer->GetCustomAttribute("SystemInfo");
            EXPECT_TRUE(customAttribute.IsValid());
            ECValue ecValue;
            customAttribute->GetValue(ecValue, "Data1");
            EXPECT_STREQ(expectedValue1, ecValue.GetUtf8CP());

            customAttribute->GetValue(ecValue, "Data2");
            EXPECT_STREQ(expectedValue2, ecValue.GetUtf8CP());
            }

        void ValidateUselessInfoCustomAttribute(IECCustomAttributeContainerP consolidatedContainer, Utf8CP expectedValue1, Utf8CP expectedValue2)
            {
            IECInstancePtr customAttribute = consolidatedContainer->GetCustomAttribute("UselessInfo");
            EXPECT_TRUE(customAttribute.IsValid());
            ECValue ecValue;
            customAttribute->GetValue(ecValue, "NothingImportant");
            EXPECT_STREQ(expectedValue1, ecValue.GetUtf8CP());

            customAttribute->GetValue(ecValue, "NotImportant");
            EXPECT_STREQ(expectedValue2, ecValue.GetUtf8CP());
            }

        void ValidateFolderClass(ECClassP consolidatedClass, ECClassP originalClass)
            {
            // Make sure it is equal to the original class, but not identical
            ValidateAreClassesIdentical(consolidatedClass, originalClass, false);
            
            // Validate class level CAs
            ValidateSystemInfoCustomAttribute (consolidatedClass, "Data1 on Folder Class", "Data2 on Folder Class");
            ValidateUselessInfoCustomAttribute (consolidatedClass, "Nothing important on Folder Class from SupplementalSchema2", "Not important on Folder Class from SupplementalSchema2");

            ECPropertyP propertyP = consolidatedClass->GetPropertyP("Hidden");
            // Validate custom attributes on Hidden property
            ValidateUselessInfoCustomAttribute(propertyP, "Nothing important on Hidden Property on Folder Class", "Not important on Hidden Property on Folder Class");

            //Validate custom attributes on CreationDate property
            propertyP = consolidatedClass->GetPropertyP("CreationDate");
            ValidateSystemInfoCustomAttribute(propertyP, "Data1 on CreationDate Property on Folder Class from SupplementalSchema2", "Data2 on CreationDate Property on Folder Class from SupplementalSchema2");

            // validate custom attributes on ReadOnly property
            propertyP = consolidatedClass->GetPropertyP("ReadOnly");
            ECCustomAttributeInstanceIterable attributes = propertyP->GetCustomAttributes(false);
            uint32_t count = 0;
            for (IECInstancePtr attribute: attributes)
                {
                count++;
                }
            EXPECT_EQ(0, count);

            }

        void ValidateFileClass(ECClassP consolidatedClass, ECClassP originalClass)
            {
            // Make sure it is equal to the original class, but not identical
            ValidateAreClassesIdentical(consolidatedClass, originalClass, false);

            // validate class level CAs
            ValidateSystemInfoCustomAttribute(consolidatedClass, "Data1 on File Class from SupplementalSchema1", "Data2 on File Class from SupplementalSchema1");
            ValidateUselessInfoCustomAttribute(consolidatedClass, "Nothing important on File Class from Supplemental3", "Not important on File Class from Supplemental3");
            
            //validate custom attributes on Hidden property
            ECPropertyP propertyP = consolidatedClass->GetPropertyP("Hidden");
            ValidateUselessInfoCustomAttribute(propertyP, "Nothing important on Hidden Property on File Class from SupplementalSchema1", "Not important on Hidden Property on File Class from SupplementalSchema1");
            ValidateSystemInfoCustomAttribute(propertyP, "Data1 on Hidden Property on File Class from Supplemental3", "Data2 on Hidden Property on File Class from Supplemental3");

            // validate custom attributes on CreationDate property
            propertyP = consolidatedClass->GetPropertyP("CreationDate");
            ValidateUselessInfoCustomAttribute(propertyP, "Nothing important on CreationDate Property on File Class from Supplemental3", "Not important on CreationDate Property on File Class from Supplemental3");

            // validate custom attributes on FileSize property
            propertyP = consolidatedClass->GetPropertyP("FileSize");
            ValidateSystemInfoCustomAttribute(propertyP, "Data1 on FileSize Property on File Class from Supplemental3", "Data2 on FileSize Property on File Class from Supplemental3");

            }

        void ValidateImageClass(ECClassP consolidatedClass, ECClassP originalClass)
            {
            // Make sure it is equal to the original class, but not identical
            ValidateAreClassesIdentical(consolidatedClass, originalClass, false);

            // validate class level custom attributes
            ValidateSystemInfoCustomAttribute(consolidatedClass, "Data1 on Image Class", "Data2 on Image Class");
            ValidateUselessInfoCustomAttribute(consolidatedClass, "Nothing important on Image Class from Supplemental3", "Not important on Image Class from Supplemental3");

            // validate custom attributes on BitDepth property
            ECPropertyP propertyP = consolidatedClass->GetPropertyP("BitDepth");
            ECCustomAttributeInstanceIterable attributes = propertyP->GetCustomAttributes(false);
            uint32_t count = 0;
            for (IECInstancePtr attribute: attributes)
                {
                count++;
                }
            EXPECT_EQ(0, count);

            // validate custom attributes on Width property
            propertyP = consolidatedClass->GetPropertyP("Width");
            ValidateUselessInfoCustomAttribute(propertyP, "Nothing important on Width Property on Image Class", "Not important on Width Property on Image Class");

            //validate custom attributes on height property
            propertyP = consolidatedClass->GetPropertyP("Height");
            ValidateUselessInfoCustomAttribute(propertyP, "Nothing important on Height Property on Image Class from Supplemental3", "Not important on Height Property on Image Class from Supplemental3");
            }

        void CompareSchemasPostSupplement(ECSchemaPtr schemaPostSupplement, ECSchemaPtr schemaPreSupplement)
            {
            // Test that the schemas have the same basic information and class count
            EXPECT_TRUE(schemaPostSupplement->IsSamePrimarySchema(*schemaPreSupplement));

            // Test custom attributes at the top level of the schema
            ECCustomAttributeInstanceIterable postCustomAttributes = schemaPostSupplement->GetCustomAttributeContainer().GetCustomAttributes(true);
            ECCustomAttributeInstanceIterable preCustomAttributes = schemaPreSupplement->GetCustomAttributeContainer().GetCustomAttributes(true);

            EXPECT_TRUE(CustomAttributesAreEqual(preCustomAttributes, postCustomAttributes));

            uint32_t preClassCount = 0;
            for (ECClassP preSupplementClass: schemaPreSupplement->GetClasses())
                {
                EXPECT_TRUE(NULL != preSupplementClass);
                preClassCount++;
                ECClassP postClass = schemaPostSupplement->GetClassP(preSupplementClass->GetName().c_str());
                EXPECT_TRUE(NULL != postClass);
                ValidateAreClassesIdentical(preSupplementClass, postClass, true);
                EXPECT_TRUE(ECClass::ClassesAreEqualByName(preSupplementClass, postClass));
                }
            EXPECT_TRUE(0 < preClassCount);

            uint32_t postClassCount = 0;
            for (ECClassP postSupplementClass: schemaPostSupplement->GetClasses())
                {
                EXPECT_TRUE(NULL != postSupplementClass);
                postClassCount++;
                }
            EXPECT_EQ(preClassCount, postClassCount);
            }

        // Compare the copies and the original schemas to make sure they were not changed by the supplementing process
        void VerifySchemasAreUnchanged()
            {
            CompareSchemasPostSupplement(m_supplementalTestSchema1, m_supplementalTestSchema1Copy);
            CompareSchemasPostSupplement(m_supplementalTestSchema2, m_supplementalTestSchema2Copy);
            CompareSchemasPostSupplement(m_supplementalTestSchema3, m_supplementalTestSchema3Copy);
            CompareSchemasPostSupplement(m_supplementalTestSchema4, m_supplementalTestSchema4Copy);

            }
        void CompareSupplementedSchemaAndPrimary(ECSchemaPtr consolidatedSchema)
            {
            for (ECClassP consolidatedClass: consolidatedSchema->GetClasses())
                {
                if (0 == strcmp(consolidatedClass->GetName().c_str(), "Folder"))
                    ValidateFolderClass(consolidatedClass, m_primaryTestSchema->GetClassP("Folder"));
                else if (0 == strcmp(consolidatedClass->GetName().c_str(), "File"))
                    ValidateFileClass(consolidatedClass, m_primaryTestSchema->GetClassP("File"));
                else if (0 == strcmp(consolidatedClass->GetName().c_str(), "Image"))
                    ValidateImageClass(consolidatedClass, m_primaryTestSchema->GetClassP("Image"));
                else
                    FAIL() << "Unexpected class found in consolidated schema";
                }

            EXPECT_TRUE(m_primaryTestSchema->IsSamePrimarySchema(*consolidatedSchema));
            EXPECT_TRUE(consolidatedSchema->IsSamePrimarySchema(*m_primaryTestSchema));

            VerifySchemasAreUnchanged();

            EXPECT_TRUE(consolidatedSchema->IsSupplemented());
            }

    public:
        virtual void            SetUp () override 
            { 
            T_Super::SetUp();
            CreateCustomAttributeSchema();
            CreatePrimarySchema(m_primaryTestSchema);
            CreateSupplementalSchema1 (m_supplementalTestSchema1);
            CreateSupplementalSchema2 (m_supplementalTestSchema2);
            CreateSupplementalSchema3 (m_supplementalTestSchema3);
            CreateSupplementalSchema4 (m_supplementalTestSchema4);

            //m_customAttributeSchema->WriteToXmlFile("d:\\temp\\customAttribute.xml");

            //m_primaryTestSchema->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\primarySchema.xml");
            //m_supplementalTestSchema1->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema1.xml");
            //m_supplementalTestSchema2->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema2.xml");
            //m_supplementalTestSchema3->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema3.xml");
            //m_supplementalTestSchema4->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema4.xml");

            // Create duplicates so we can compare these schemas to the originals post supplementing the primary schema.
            CreatePrimarySchema (m_primaryTestSchemaCopy);
            CreateSupplementalSchema1 (m_supplementalTestSchema1Copy);
            CreateSupplementalSchema2 (m_supplementalTestSchema2Copy);
            CreateSupplementalSchema3 (m_supplementalTestSchema3Copy);
            CreateSupplementalSchema4 (m_supplementalTestSchema4Copy);

            }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementalSchemaMetaDataTests, CanRetrieveFromSchema)
    {
    ECSchemaPtr supplemental;
    CreateSupplementalSchema(supplemental, "TestSchema", "OverrideWidgets", (uint32_t) 200, (uint32_t) 1, (uint32_t) 0, 4, 2);

    EXPECT_TRUE(supplemental.IsValid());
                ECSchemaP tempSchema = supplemental.get();
            tempSchema->GetFullSchemaName();
            ECCustomAttributeInstanceIterable iter = tempSchema->GetCustomAttributeContainer().GetCustomAttributes(true);

    ECN::SupplementalSchemaMetaDataPtr metaData;
    EXPECT_TRUE(SupplementalSchemaMetaData::TryGetFromSchema(metaData, *supplemental));
    EXPECT_TRUE(metaData.IsValid());
    EXPECT_STREQ("TestSchema", metaData->GetPrimarySchemaName().c_str());
    EXPECT_EQ(1, metaData->GetPrimarySchemaMajorVersion());
    EXPECT_EQ(0, metaData->GetPrimarySchemaMinorVersion());
    EXPECT_EQ(200, metaData->GetSupplementalSchemaPrecedence());
    EXPECT_STREQ("OverrideWidgets", metaData->GetSupplementalSchemaPurpose().c_str());
    EXPECT_FALSE(metaData->IsUserSpecific());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementalSchemaInfoTests, CanSetAndRetrieveInfo)
    {
    bmap<Utf8String, Utf8String> schemaNamesAndPurposes1;
    schemaNamesAndPurposes1["Schema1.01.00"] = "Units";
    schemaNamesAndPurposes1["Schema2.02.00"] = "Units";
    schemaNamesAndPurposes1["Schema2.01.01"] = "Alpha";
    schemaNamesAndPurposes1["Schema3.01.00"] = "Beta";

    Utf8String primarySchemaFullName("PrimarySchema.08.02");

    ECSchemaPtr primarySchema;
    ECSchema::CreateSchema(primarySchema, "PrimarySchema", 8, 2);

    ECSchemaPtr supplementalSchema1;
    ECSchemaPtr supplementalSchema2;
    ECSchemaPtr supplementalSchema3;
    ECSchemaPtr supplementalSchema4;
    CreateSupplementalSchema(supplementalSchema1, "PrimarySchema", "Units", 200, 8, 2, 1, 0);
    CreateSupplementalSchema(supplementalSchema2, "PrimarySchema", "Units", 201, 8, 2, 2, 0);
    CreateSupplementalSchema(supplementalSchema3, "PrimarySchema", "Alpha", 202, 8, 2, 1, 1);
    CreateSupplementalSchema(supplementalSchema4, "PrimarySchema", "Beta", 203, 8, 2, 1, 0);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(supplementalSchema1.get());
    supplementalSchemas.push_back(supplementalSchema2.get());
    supplementalSchemas.push_back(supplementalSchema3.get());
    supplementalSchemas.push_back(supplementalSchema4.get());
    SupplementedSchemaBuilder builder;
    builder.UpdateSchema(*primarySchema, supplementalSchemas);

    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "PrimarySchema", 8, 2);
    SupplementedSchemaBuilder builder2;
    builder2.UpdateSchema(*schema2, supplementalSchemas);

    SupplementalSchemaInfoPtr schemaInfo1 = primarySchema->GetSupplementalInfo();

    EXPECT_TRUE(schemaInfo1->HasSameSupplementalSchemasForPurpose(*schema2, "Units"));
    }

/*---------------------------------------------------------------------------------**//**
* Test that tries to build a consolidated schema but there are two supplemental schemas that have
* conflicting custom attributes
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementedSchemaBuilderTests, BuildAConflictingConsolidatedSchema)
    {
    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(m_supplementalTestSchema1.get());
    supplementalSchemas.push_back(m_supplementalTestSchema2.get());
    supplementalSchemas.push_back(m_supplementalTestSchema3.get());
    supplementalSchemas.push_back(m_supplementalTestSchema4.get());

    SupplementedSchemaBuilder builder;
    ECSchemaPtr primaryTestSchema;
    CreatePrimarySchema(primaryTestSchema);
    EXPECT_EQ(SupplementedSchemaStatus::SchemaMergeException, builder.UpdateSchema(*primaryTestSchema, supplementalSchemas));
    VerifySchemasAreUnchanged();

    }

/*---------------------------------------------------------------------------------**//**
* Test that builds a consolidated schema that consists of a primary and 3 consolidated schemas.
* One of the schemas has lower precedence than the primary, two have greater than the primary.
* The two schemas that are greater have equal precedence
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementedSchemaBuilderTests, BuildANonConflictingConsolidatedSchema)
    {
    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(m_supplementalTestSchema1.get());
    supplementalSchemas.push_back(m_supplementalTestSchema2.get());
    supplementalSchemas.push_back(m_supplementalTestSchema3.get());

    SupplementedSchemaBuilder builder;
    ECSchemaPtr primaryTestSchema;
    CreatePrimarySchema(primaryTestSchema);
    EXPECT_EQ(SupplementedSchemaStatus::Success, builder.UpdateSchema(*primaryTestSchema, supplementalSchemas));
    //m_supplementalTestSchema1->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema1Post.xml");
    //m_supplementalTestSchema2->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema2Post.xml");
    //m_supplementalTestSchema3->WriteToXmlFile(L"d:\\temp\\supplementalSchemas\\supplementalSchema3Post.xml");

    CompareSupplementedSchemaAndPrimary(primaryTestSchema);
    }

void ValidateCustomAttributesOnDerivedClass(ECClassP supplementedClass)
    {
    ECCustomAttributeInstanceIterable localCustomAttributes = supplementedClass->GetCustomAttributes(false);
    uint32_t localCustomAttributesCount = 0;
    for (IECInstancePtr attribute: localCustomAttributes)
        {
        localCustomAttributesCount++;
        }
    EXPECT_EQ(2, localCustomAttributesCount);

    ECCustomAttributeInstanceIterable allCustomAttributes = supplementedClass->GetCustomAttributes(true);
    uint32_t allCustomAttributesCount = 0;
    for (IECInstancePtr attribute: allCustomAttributes)
        {
        allCustomAttributesCount++;
        }
    EXPECT_EQ(4, allCustomAttributesCount);

    ECCustomAttributeInstanceIterable localCustomAttributes2 = supplementedClass->GetCustomAttributes(false);
    localCustomAttributesCount = 0;
    for (IECInstancePtr attribute: localCustomAttributes2)
        {
        localCustomAttributesCount++;
        }
    EXPECT_EQ(2, localCustomAttributesCount);
    }

void ValidatePrimaryCustomAttributesOnDerivedClass(ECClassP supplementedClass)
    {
    ECCustomAttributeInstanceIterable localCustomAttributes = supplementedClass->GetPrimaryCustomAttributes(false);
    uint32_t localCustomAttributesCount = 0;
    for (IECInstancePtr attribute: localCustomAttributes)
        {
        localCustomAttributesCount++;
        }
    EXPECT_EQ(1, localCustomAttributesCount);

    ECCustomAttributeInstanceIterable allCustomAttributes = supplementedClass->GetPrimaryCustomAttributes(true);
    uint32_t allCustomAttributesCount = 0;
    for (IECInstancePtr attribute: allCustomAttributes)
        {
        allCustomAttributesCount++;
        }
    EXPECT_EQ(2, allCustomAttributesCount);

    ECCustomAttributeInstanceIterable localCustomAttributes2 = supplementedClass->GetPrimaryCustomAttributes(false);
    localCustomAttributesCount = 0;
    for (IECInstancePtr attribute: localCustomAttributes2)
        {
        localCustomAttributesCount++;
        }
    EXPECT_EQ(1, localCustomAttributesCount);
    }

/*---------------------------------------------------------------------------------**//**
* Test the GetCustomAttributes method on a derived class
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementedSchemaBuilderTests, GetCustomAttributesOnDerivedClass)
    {
    ECClassP classA = NULL;
    ECClassP classB = NULL;

    ECSchemaPtr schema;

    BuildSupplementedSchemaForCustomAttributeTests(classA, classB, schema);

    ValidateCustomAttributesOnDerivedClass(classB);
    }

/*---------------------------------------------------------------------------------**//**
* Test the GetPrimaryCustomAttributes method on a derived class
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementedSchemaBuilderTests, GetPrimaryCustomAttributesOnDerivedClass)
    {
    ECClassP classA = NULL;
    ECClassP classB = NULL;

    ECSchemaPtr schema;

    BuildSupplementedSchemaForCustomAttributeTests(classA, classB, schema);

    ValidatePrimaryCustomAttributesOnDerivedClass(classB);
    }

/*---------------------------------------------------------------------------------**//**
* Tests supplementing relationship classes
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SupplementedSchemaBuilderTests, SupplementCustomAttributesOnRelationshipClasses)
    {
    ECSchemaPtr schema;

    CreatePrimarySchemaForRelationshipTests(schema);

    // Create first supplemental schema
    ECSchemaPtr supplementalSchema0;
    CreateSupplementalSchema0ForRelationshipTests(supplementalSchema0);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(supplementalSchema0.get());
    SupplementedSchemaBuilder builder;
    EXPECT_EQ(SupplementedSchemaStatus::Success, builder.UpdateSchema(*schema, supplementalSchemas));

    ECClassP supplementedClass = schema->GetClassP("RelationshipWithCustomAttributes");
    EXPECT_TRUE(NULL != supplementedClass);
    ECRelationshipClassP supplementedRelClass = dynamic_cast<ECRelationshipClassP>(supplementedClass);
    EXPECT_TRUE(NULL != supplementedRelClass);
    IECInstancePtr targetCA = supplementedRelClass->GetTarget().GetCustomAttribute(m_systemInfoCAEnabler->GetClass());
    EXPECT_TRUE(targetCA.IsValid());
    IECInstancePtr sourceCA = supplementedRelClass->GetSource().GetCustomAttribute(m_systemInfoCAEnabler->GetClass());
    EXPECT_TRUE(sourceCA.IsValid());

    ECSchemaPtr supplementalSchema1 = NULL;
    CreateSupplementalSchema1ForRelationshipTests(supplementalSchema1);
    supplementalSchemas.push_back(supplementalSchema1.get());

    ECSchemaPtr schema2;

    CreatePrimarySchemaForRelationshipTests(schema2);
    SupplementedSchemaBuilder builder2;
    EXPECT_EQ(SupplementedSchemaStatus::Success, builder2.UpdateSchema(*schema2, supplementalSchemas));

    supplementedClass = schema2->GetClassP("RelationshipWithCustomAttributes");
    EXPECT_TRUE(NULL != supplementedClass);
    supplementedRelClass = dynamic_cast<ECRelationshipClassP>(supplementedClass);
    EXPECT_TRUE(NULL != supplementedRelClass);
    targetCA = supplementedRelClass->GetTarget().GetCustomAttribute(m_systemInfoCAEnabler->GetClass());
    EXPECT_TRUE(targetCA.IsValid());
    sourceCA = supplementedRelClass->GetSource().GetCustomAttribute(m_systemInfoCAEnabler->GetClass());
    EXPECT_TRUE(sourceCA.IsValid());

    targetCA = supplementedRelClass->GetTarget().GetCustomAttribute(m_uselessInfoCAEnabler->GetClass());
    EXPECT_TRUE(targetCA.IsValid());
    sourceCA = supplementedRelClass->GetSource().GetCustomAttribute(m_uselessInfoCAEnabler->GetClass());
    EXPECT_TRUE(sourceCA.IsValid());

    supplementedClass = schema2->GetClassP("RelClass2");
    EXPECT_TRUE(NULL != supplementedClass);
    supplementedRelClass = dynamic_cast<ECRelationshipClassP>(supplementedClass);
    EXPECT_TRUE(NULL != supplementedRelClass);
    targetCA = supplementedRelClass->GetTarget().GetCustomAttribute(m_otherInfoCAEnabler->GetClass());
    EXPECT_TRUE(targetCA.IsValid());
    sourceCA = supplementedRelClass->GetSource().GetCustomAttribute(m_otherInfoCAEnabler->GetClass());
    EXPECT_TRUE(sourceCA.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
TEST_F(SupplementedSchemaBuilderTests, SupplementingWithInheritance)
    {
    ECSchemaPtr inheritPrimarySchema;
    ECSchemaPtr lowPrioritySchema1;
    ECSchemaPtr lowPrioritySchema199;
    ECSchemaPtr highPrioritySchema200;

    CreatePrimarySchemaForInheritTests(inheritPrimarySchema);
    CreateLowPrioritySchema1(lowPrioritySchema1);
    CreateLowPrioritySchema199(lowPrioritySchema199);
    CreateHighPrioritySchema200(highPrioritySchema200);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(lowPrioritySchema1.get());
    supplementalSchemas.push_back(lowPrioritySchema199.get());
    supplementalSchemas.push_back(highPrioritySchema200.get());

    SupplementedSchemaBuilder builder;
    builder.UpdateSchema(*inheritPrimarySchema, supplementalSchemas);

    EXPECT_TRUE(inheritPrimarySchema->IsSupplemented());

    // DerivedClass1.BC1Prop1 should have its custom attribute applied by a low precedence supplemental schema
    ECClassP derivedClass1 = inheritPrimarySchema->GetClassP("DerivedClass1");
    EXPECT_TRUE(NULL != derivedClass1);
    ECPropertyP bc1Prop1DerivedClass1 = derivedClass1->GetPropertyP("BC1Prop1");
    EXPECT_TRUE(NULL != bc1Prop1DerivedClass1);
    ValidateSystemInfoCustomAttribute(bc1Prop1DerivedClass1, "LowPrioritySchema199.DerivedClass1.BC1Prop1", "LowPrioritySchema199.DerivedClass1.BC1Prop1");

    // DerivedClass2.BC1Prop2 should have its custom attribute applied by the primary schema
    ECClassP derivedClass2 = inheritPrimarySchema->GetClassP("DerivedClass2");
    EXPECT_TRUE(NULL != derivedClass2);
    ECPropertyP bc1Prop2DerivedClass2 = derivedClass2->GetPropertyP("BC1Prop2");
    EXPECT_TRUE(NULL != bc1Prop2DerivedClass2);
    ValidateUselessInfoCustomAttribute(bc1Prop2DerivedClass2,  "InheritTestSchema.DerivedClass2.BC1Prop2", "InheritTestSchema.DerivedClass2.BC1Prop2");

    // DerivedClass3.BC1Prop1 should have its custom attribute applied by a low precedence supplemental schema
    ECClassP derivedClass3 = inheritPrimarySchema->GetClassP("DerivedClass3");
    EXPECT_TRUE (NULL != derivedClass3);
    ECPropertyP bc1Prop1DerivedClass3 = derivedClass3->GetPropertyP("BC1Prop1");
    EXPECT_TRUE (NULL != bc1Prop1DerivedClass3);
    ValidateSystemInfoCustomAttribute(bc1Prop1DerivedClass3, "LowPrioritySchema199.DerivedClass3.BC1Prop1", "LowPrioritySchema199.DerivedClass3.BC1Prop1");

    // DerivedClass4.BC2Prop1 should have its custom attribute applied by a low precedence supplemental schema
    ECClassP derivedClass4 = inheritPrimarySchema->GetClassP("DerivedClass4");
    EXPECT_TRUE(NULL != derivedClass4);
    ECPropertyP bc2Prop1DerivedClass4 = derivedClass4->GetPropertyP("BC2Prop1");
    EXPECT_TRUE(NULL != bc2Prop1DerivedClass4);
    ValidateSystemInfoCustomAttribute(bc2Prop1DerivedClass4, "LowPrioritySchema1.DerivedClass4.BC2Prop1", "LowPrioritySchema1.DerivedClass4.BC2Prop1");
    }
#endif

TEST(SupplementalDeserializationTests, VerifyDeserializedSchemaIsSupplemented2)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());
    SchemaKey key("MasterSchema", 1, 0);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());

    }
END_BENTLEY_ECN_TEST_NAMESPACE
