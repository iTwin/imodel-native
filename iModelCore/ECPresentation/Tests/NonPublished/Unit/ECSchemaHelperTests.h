/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../Source/Shared/ECSchemaHelper.h"
#include "../Helpers/TestHelpers.h"
#include "../Helpers/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define SCHEMA_BASIC_1  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                           \
                        "<ECSchema schemaName=\"Basic1\" alias=\"b1\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"   \
                        "    <ECEntityClass typeName=\"Class1\">"                                                                                  \
                        "    </ECEntityClass>"                                                                                                                       \
                        "    <ECEntityClass typeName=\"Class1A\">"                                                                                 \
                        "        <BaseClass>Class1</BaseClass>"                                                                                                \
                        "        <ECProperty propertyName=\"DisplayLabel\" typeName=\"int\" />"                                                                \
                        "    </ECEntityClass>"                                                                                                                       \
                        "    <ECEntityClass typeName=\"Class1B\">"                                                                                 \
                        "        <BaseClass>Class1</BaseClass>"                                                                                                \
                        "        <ECProperty propertyName=\"DisplayLabel\" typeName=\"double\" />"                                                             \
                        "    </ECEntityClass>"                                                                                                                       \
                        "</ECSchema>"

#define SCHEMA_BASIC_2  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                           \
                        "<ECSchema schemaName=\"Basic2\" alias=\"b2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"   \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                  \
                        "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"                                                                     \
                        "    </ECEntityClass>"                                                                                                                       \
                        "</ECSchema>"

#define SCHEMA_BASIC_3  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                           \
                        "<ECSchema schemaName=\"Basic3\" alias=\"b3\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"   \
                        "    <ECEntityClass typeName=\"Class3\" displayLabel=\"Test Class 3\">"                                                    \
                        "        <ECProperty propertyName=\"SomeProperty\" typeName=\"point2d\" />"                                                            \
                        "    </ECEntityClass>"                                                                                                                       \
                        "</ECSchema>"

#define SCHEMA_COMPLEX_1 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
                        "<ECSchema schemaName=\"SchemaComplex\" alias=\"sc\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
                        "    <ECEntityClass typeName=\"Class1\">"                                                                                   \
                        "        <ECCustomAttributes>" \
                        "            <ClassMap xmlns=\"ECDbMap.02.00\"> " \
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>" \
                        "            </ClassMap>" \
                        "        </ECCustomAttributes>" \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"Class4\">"                                                                                   \
                        "        <BaseClass>Class1</BaseClass>"                                                                                \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"BaseOf2and3\" modifier=\"Abstract\">"                                                                                   \
                        "        <ECCustomAttributes>" \
                        "            <ClassMap xmlns=\"ECDbMap.02.00\"> " \
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>" \
                        "            </ClassMap>" \
                        "        </ECCustomAttributes>" \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                   \
                        "        <BaseClass>BaseOf2and3</BaseClass>"                                                                                \
                        "        <ECProperty propertyName=\"ParentId\" typeName=\"long\" />"                                                   \
                        "        <ECNavigationProperty propertyName=\"C1\" relationshipName=\"Class1HasClass2And3\" direction=\"Backward\" />"                  \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"Class3\">"                                                                                   \
                        "        <BaseClass>BaseOf2and3</BaseClass>"                                                                                \
                        "        <ECProperty propertyName=\"ParentId\" typeName=\"long\" />"                                                   \
                        "        <ECNavigationProperty propertyName=\"C1\" relationshipName=\"Class1HasClass2And3\" direction=\"Backward\" />"                  \
                        "        <ECNavigationProperty propertyName=\"C2\" relationshipName=\"Class1HasClass3\" direction=\"Backward\" />"                  \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECRelationshipClass typeName=\"Class1HasClass2And3\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"    \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class1 Has Class2 And 3\" polymorphic=\"True\">"                    \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class1 Has Class2 And 3 (reversed)\" polymorphic=\"True\" abstractConstraint=\"BaseOf2and3\">"         \
                        "            <Class class=\"Class2\" />"                                                                                 \
                        "            <Class class=\"Class3\" />"                                                                                 \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "    <ECRelationshipClass typeName=\"Class1HasClass3\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"        \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class1 Has Class3\" polymorphic=\"False\">"                         \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class1 Has Class3 (reversed)\" polymorphic=\"True\">"               \
                        "            <Class class=\"Class3\" />"                                                                                 \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "    <ECRelationshipClass typeName=\"DerivingRelationship\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">" \
                        "        <BaseClass>Class1HasClass3</BaseClass>"                                                                                           \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class1 Has Class3\" polymorphic=\"True\">"                         \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class1 Has Class3 (reversed)\" polymorphic=\"True\">"               \
                        "            <Class class=\"Class3\" />"                                                                                 \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "</ECSchema>"

#define SCHEMA_COMPLEX_2 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
                        "<ECSchema schemaName=\"SchemaComplex2\" alias=\"sc2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
                        "    <ECEntityClass typeName=\"Class1\">"                                                                                   \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                   \
                        "        <ECNavigationProperty propertyName=\"C1\" relationshipName=\"Class1HasClass2\" direction=\"Backward\" />"                  \
                        "        <ECNavigationProperty propertyName=\"C3\" relationshipName=\"Class3HasClass2\" direction=\"Backward\" />"                  \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECEntityClass typeName=\"Class3\">"                                                                                   \
                        "    </ECEntityClass>"                                                                                                       \
                        "    <ECRelationshipClass typeName=\"Class1HasClass2\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"        \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class1 Has Class2\" polymorphic=\"True\">"                          \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class1 Has Class2 (reversed)\" polymorphic=\"True\">"               \
                        "            <Class class=\"Class2\" />"                                                                               \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "    <ECRelationshipClass typeName=\"Class3HasClass2\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"        \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class3 Has Class2\" polymorphic=\"False\">"                         \
                        "            <Class class=\"Class3\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class3 Has Class2 (reversed)\" polymorphic=\"True\">"               \
                        "            <Class class=\"Class2\" />"                                                                               \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "</ECSchema>"

#define SCHEMA_COMPLEX_3 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
                        "<ECSchema schemaName=\"SchemaComplex3\" alias=\"sc3\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
                        "    <ECEntityClass typeName=\"Class1\">"                                                                              \
                        "       <ECNavigationProperty propertyName=\"Parent\" relationshipName=\"Class1HasClass1\" direction=\"Backward\" />"\
                        "    </ECEntityClass>"                                                                                                 \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                              \
                        "       <ECNavigationProperty propertyName=\"Parent\" relationshipName=\"Class1HasClass2\" direction=\"Backward\" />"\
                        "    </ECEntityClass>"                                                                                                 \
                        "    <ECEntityClass typeName=\"Class3\">"                                                                              \
                        "       <ECNavigationProperty propertyName=\"Parent\" relationshipName=\"Class3IsInClass1\" direction=\"Forward\" />"\
                        "    </ECEntityClass>"                                                                                                 \
                        "    <ECRelationshipClass typeName=\"Class1HasClass1\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"        \
                        "        <Source multiplicity=\"(0..1)\" roleLabel=\"Class1 Has Class1\" polymorphic=\"True\">"                          \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(0..*)\" roleLabel=\"Class1 Has Class1 (reversed)\" polymorphic=\"True\">"               \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "    <ECRelationshipClass typeName=\"Class1HasClass2\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"        \
                        "        <Source multiplicity=\"(0..1)\" roleLabel=\"Class1 Has Class2\" polymorphic=\"True\">"                          \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(0..*)\" roleLabel=\"Class2 belongs to Class1\" polymorphic=\"True\">"                   \
                        "            <Class class=\"Class2\" />"                                                                               \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "    <ECRelationshipClass typeName=\"Class3IsInClass1\" strength=\"referencing\" strengthDirection=\"Backward\" modifier=\"None\">"      \
                        "        <Source multiplicity=\"(0..*)\" roleLabel=\"Class3 is in Class1\" polymorphic=\"True\">"                        \
                        "            <Class class=\"Class3\" />"                                                                               \
                        "        </Source>"                                                                                                    \
                        "        <Target multiplicity=\"(0..1)\" roleLabel=\"Class1 has Class3\" polymorphic=\"True\">"                          \
                        "            <Class class=\"Class1\" />"                                                                               \
                        "        </Target>"                                                                                                    \
                        "    </ECRelationshipClass>"                                                                                           \
                        "</ECSchema>"

#define HIDDEN_SCHEMA "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                                                                                         \
                        "<ECSchema schemaName=\"HiddenSchema\" alias=\"hs\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"   \
                        "   <ECSchemaReference name=\"CoreCustomAttributes\" version=\"01.00\" alias=\"CoreCA\"/>"                                          \
                        "   <ECCustomAttributes>"                                                                                                           \
                        "       <HiddenSchema xmlns=\"CoreCustomAttributes.01.00\"/>"                                                                       \
                        "   </ECCustomAttributes>"                                                                                                          \
                        "   <ECEntityClass typeName=\"Class1\">"                                                                                            \
                        "       <ECProperty propertyName=\"StringProperty\" typeName=\"string\"/>"                                                          \
                        "   </ECEntityClass>"                                                                                                               \
                        "</ECSchema>"

#define VISIBLE_SCHEMA "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                                                                                         \
                        "<ECSchema schemaName=\"VisibleSchema\" alias=\"vs\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"   \
                        "   <ECSchemaReference name=\"CoreCustomAttributes\" version=\"01.00\" alias=\"CoreCA\"/>"                                          \
                        "   <ECEntityClass typeName=\"HiddenClass\">"                                                                                       \
                        "       <ECCustomAttributes>"                                                                                                       \
                        "           <HiddenClass xmlns=\"CoreCustomAttributes.01.00\"/>"                                                                    \
                        "       </ECCustomAttributes>"                                                                                                      \
                        "   </ECEntityClass>"                                                                                                               \
                        "   <ECEntityClass typeName=\"VisibleClass\">"                                                                                      \
                        "       <ECProperty propertyName=\"HiddenProperty\" typeName = \"int\">"                                                            \
                        "           <ECCustomAttributes>"                                                                                                   \
                        "               <HiddenProperty xmlns=\"CoreCustomAttributes.01.00\"/>"                                                             \
                        "           </ECCustomAttributes>"                                                                                                  \
                        "       </ECProperty>"                                                                                                              \
                        "       <ECProperty propertyName=\"VisibleProperty\" typeName=\"int\"/>"                                                            \
                        "   </ECEntityClass>"                                                                                                               \
                        "</ECSchema>"

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(ECSchemaHelperTests, name, schema_xml)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECSchemaHelperTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(ECSchemaHelperTests)

    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    ECSchemaHelper* m_helper;

    ECSchemaHelperTests() : m_helper(nullptr) {}

    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override;
    void TearDown() override;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstancesHelperTests : ECPresentationTest
    {
    ECDbTestProject m_project;
    RefCountedPtr<TestConnection> m_connection;
    void SetUp() override;
    void TearDown() override;
    };
