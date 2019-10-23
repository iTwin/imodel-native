/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaPolicyTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, SchemaPolicesNotIncludedByDefault)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaPolicesNotIncludedByDefault.ecdb"));
    ASSERT_FALSE(m_ecdb.Schemas().ContainsSchema("ECDbSchemaPolicies"));
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan.Khan                    10/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, ReservedPropertyNames_Inheritance)
    {
    auto bisCore = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <!-- Applied to an ECClass to reserve list properties that cannot be used by that class or its dervied classes. -->
            <ECCustomAttributeClass typeName="ReservedPropertyNames" modifier="Sealed" appliesTo="EntityClass, RelationshipClass"
                                    description="Declare a list of properties as reserved. The property name listed would be forbidden from use in that class context.">
                <ECArrayProperty propertyName="PropertyNames" typeName="string" minOccurs="0" maxOccurs="unbounded" 
                                description="Name of the property. System will do case insensitive comparison."/>
            </ECCustomAttributeClass>
        </ECSchema>)xml");
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
                                                               R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P1</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>ClassA</BaseClass>
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassC" modifier="None" >
                <BaseClass>ClassB</BaseClass>
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>       
                <ECProperty propertyName="P1" typeName="int" />     
                <ECProperty propertyName="P5" typeName="int" />     
            </ECEntityClass>

        </ECSchema>)xml")}))
        << "Class with ReservedPropertyNames reject its on properties";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
                                                               R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P1</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>ClassA</BaseClass>
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassC" modifier="None" >
                <BaseClass>ClassB</BaseClass>
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>       
                <ECProperty propertyName="P2" typeName="int" />     
                <ECProperty propertyName="P5" typeName="int" />     
            </ECEntityClass>

        </ECSchema>)xml")}))
        << "Class with ReservedPropertyNames reject its on properties";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
                                                               R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P1</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>ClassA</BaseClass>
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
            </ECEntityClass>
            <ECEntityClass typeName="ClassC" modifier="None" >
                <BaseClass>ClassB</BaseClass>
                <ECProperty propertyName="P1" typeName="int" />     
                <ECProperty propertyName="P2" typeName="int" />     
                <ECProperty propertyName="P3" typeName="int" />     
            </ECEntityClass>

        </ECSchema>)xml")}))
        << "Class with ReservedPropertyNames reject its on properties";
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan.Khan                    09/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, ReservedPropertyNames_LinkTableRelationships)
    {
        auto bisCore = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <!-- Applied to an ECClass to reserve list properties that cannot be used by that class or its dervied classes. -->
            <ECCustomAttributeClass typeName="ReservedPropertyNames" modifier="Sealed" appliesTo="EntityClass, RelationshipClass"
                                    description="Declare a list of properties as reserved. The property name listed would be forbidden from use in that class context.">
                <ECArrayProperty propertyName="PropertyNames" typeName="string" minOccurs="0" maxOccurs="unbounded" 
                                description="Name of the property. System will do case insensitive comparison."/>
            </ECCustomAttributeClass>
        </ECSchema>)xml");

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
                                                                   R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>p1</string>
                            <string>p2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
           </ECRelationshipClass>
        </ECSchema>)xml")}))
            << "LinkTable should reject its own declar property if its part of reserved property list";

        ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>Id</string>
                            <string>ClassName</string>
                            <string>ClassFullName</string>
                            <string>SourceId</string>
                            <string>TargetId</string>
                            <string>SourceClassId</string>
                            <string>TargetClassId</string>
                            <string>MyProp</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
           </ECRelationshipClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="OptingInSchema" version="01.00.00" alias="master"/>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>master:ClassA</BaseClass>
                <ECProperty propertyName="P3" typeName="string" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>
            <ECRelationshipClass typeName="MyLinkTableRelDrv" modifier="None" strength="Referencing">
                <BaseClass>master:MyLinkTableRel</BaseClass>
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
           </ECRelationshipClass>
        </ECSchema>)xml")}))
            << "New dervied class in new schema with reserved properties name";

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>Id</string>
                            <string>ClassName</string>
                            <string>ClassFullName</string>
                            <string>SourceId</string>
                            <string>TargetId</string>
                            <string>SourceClassId</string>
                            <string>TargetClassId</string>
                            <string>MyProp</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
           </ECRelationshipClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="OptingInSchema" version="01.00.00" alias="master"/>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>master:ClassA</BaseClass>
                <ECProperty propertyName="P3" typeName="string" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>
            <ECRelationshipClass typeName="MyLinkTableRelDrv" modifier="None" strength="Referencing">
                <BaseClass>master:MyLinkTableRel</BaseClass>
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECProperty propertyName="ClassFullName" typeName="int" />
                <ECProperty propertyName="MyProp" typeName="int" />
           </ECRelationshipClass>
        </ECSchema>)xml")}))
            << "New dervied class in new schema with reserved properties name";

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImportOneAtATime({bisCore, SchemaItem(
                                                                             R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>Id</string>
                            <string>ClassName</string>
                            <string>ClassFullName</string>
                            <string>SourceId</string>
                            <string>TargetId</string>
                            <string>SourceClassId</string>
                            <string>TargetClassId</string>
                            <string>MyProp</string>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
           </ECRelationshipClass>
        </ECSchema>)xml"),
                                                                SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>Id</string>
                            <string>ClassName</string>
                            <string>ClassFullName</string>
                            <string>SourceId</string>
                            <string>TargetId</string>
                            <string>SourceClassId</string>
                            <string>TargetClassId</string>
                            <string>MyProp</string>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassA"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string"/>
                <ECProperty propertyName="MyProp" typeName="string"/>
                <ECProperty propertyName="P2" typeName="string"/>
           </ECRelationshipClass>
        </ECSchema>)xml")}))
            << "Updating schema add add a reserved property";

    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Affan.Khan                    09/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, ReservedPropertyNames_EntitiyClasses)
    {
        auto bisCore = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <!-- Applied to an ECClass to reserve list properties that cannot be used by that class or its dervied classes. -->
            <ECCustomAttributeClass typeName="ReservedPropertyNames" modifier="Sealed" appliesTo="EntityClass, RelationshipClass"
                                    description="Declare a list of properties as reserved. The property name listed would be forbidden from use in that class context.">
                <ECArrayProperty propertyName="PropertyNames" typeName="string" minOccurs="0" maxOccurs="unbounded" 
                                description="Name of the property. System will do case insensitive comparison."/>
            </ECCustomAttributeClass>
        </ECSchema>)xml");
        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
                                                                   R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P1</string>
                            <string>P2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")}))
            << "Class with ReservedPropertyNames reject its on properties";

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>p1</string>
                            <string>p2</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")})) << "Class with ReservedPropertyNames reject its on properties -Case-insensitive";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="OptingInSchema" version="01.00.00" alias="master"/>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>master:ClassA</BaseClass>
                <ECProperty propertyName="P3" typeName="string" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")})) << "New dervied class in new schema with reserved properties name";

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")})) << "add new property to existing class";

        ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")})) << "add new property to existing class and update reserved property policy";
        
        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({bisCore, SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECCustomAttributes>
                    <ReservedPropertyNames xmlns="BisCore.01.00.01">
                        <PropertyNames>
                            <string>P3</string>
                            <string>p4</string>
                        </PropertyNames>
                    </ReservedPropertyNames>
                </ECCustomAttributes>            
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>ClassA</BaseClass>
                <ECProperty propertyName="P3" typeName="string" />
                <ECProperty propertyName="P4" typeName="int" />
            </ECEntityClass>            
        </ECSchema>)xml")})) << "New dervied class in current schema using schema upgrade";
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, NoAdditionalRootEntityClasses)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="ClassB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")}));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00">
                    <Exceptions>
                        <string>Schema1:ClassB</string>
                    </Exceptions>
                </NoAdditionalRootEntityClasses>
            </ECCustomAttributes>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="ClassB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="ClassC" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")}));

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemapolicies1.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00">
                    <Exceptions>
                        <string>Schema1:ClassB</string>
                    </Exceptions>
                </NoAdditionalRootEntityClasses>
            </ECCustomAttributes>
            <ECEntityClass typeName="ClassA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="ClassB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")}));
    ASSERT_TRUE(GetHelper().TableExists("master_ClassA"));
    ASSERT_TRUE(GetHelper().TableExists("s1_ClassB"));
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemapolicies2.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="MasterA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="OptingInSchema" version="01.00" alias="master"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>

            <ECCustomAttributeClass typeName="MyCA" modifier="Sealed" appliesTo="Schema" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECCustomAttributeClass>

            <ECStructClass typeName="MyStruct" modifier="Sealed" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECStructClass>

            <ECEntityClass typeName="MyMixin" modifier="Abstract" >
                    <ECCustomAttributes>
                        <IsMixin xmlns="CoreCustomAttributes.01.00">
                            <AppliesToEntityClass>master:MasterA</AppliesToEntityClass>
                        </IsMixin>
                    </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>

            <ECEntityClass typeName="ClassB" modifier="None" >
                <BaseClass>master:MasterA</BaseClass>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECNavigationProperty propertyName="B1" relationshipName="MyLogicalFkRel" direction="Backward"/>
                <ECNavigationProperty propertyName="B2" relationshipName="MyPhysicalFkRel" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Target>
           </ECRelationshipClass>

            <ECRelationshipClass typeName="MyLogicalFkRel" modifier="Sealed" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Target>
           </ECRelationshipClass>

            <ECRelationshipClass typeName="MyPhysicalFkRel" modifier="Sealed" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="ClassB"/>
                </Target>
           </ECRelationshipClass>

        </ECSchema>)xml")}));

    ASSERT_TRUE(GetHelper().TableExists("master_MasterA"));
    ASSERT_TRUE(GetHelper().TableExists("s1_ClassB"));
    ASSERT_TRUE(GetHelper().TableExists("s1_MyLinkTableRel"));
    ASSERT_FALSE(GetHelper().TableExists("s1_MyCA"));
    ASSERT_FALSE(GetHelper().TableExists("s1_MyStruct"));
    ASSERT_FALSE(GetHelper().TableExists("s1_MyMixin"));

    ASSERT_EQ(ExpectedColumns({ExpectedColumn("s1_ClassB", "B1Id"),
                              ExpectedColumn("s1_ClassB","B1RelECClassId", Virtual::Yes)}),
              GetHelper().GetPropertyMapColumns(AccessString("s1", "ClassB", "B1")));

    ASSERT_EQ(ExpectedColumns({ExpectedColumn("s1_ClassB", "B2Id"),
                              ExpectedColumn("s1_ClassB","B2RelECClassId", Virtual::Yes)}),
              GetHelper().GetPropertyMapColumns(AccessString("s1", "ClassB", "B2")));
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemapolicies3.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SchemaBeforeOptingInSchemaIsImported" alias="pre" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="PreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="MasterA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="SchemaBeforeOptingInSchemaIsImported" version="01.00" alias="pre"/>
            <ECSchemaReference name="OptingInSchema" version="01.00" alias="master"/>
            <ECEntityClass typeName="S1A" modifier="None" >
                <BaseClass>pre:PreA</BaseClass>
                <ECProperty propertyName="P10" typeName="string" />
                <ECProperty propertyName="P20" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="S1B" modifier="None" >
                <BaseClass>master:MasterA</BaseClass>
                <ECProperty propertyName="P10" typeName="string" />
                <ECProperty propertyName="P20" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="S2A" modifier="None" >
                <ECProperty propertyName="P10" typeName="string" />
                <ECProperty propertyName="P20" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemapolicies3.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SchemaBeforeOptingInSchemaIsImported" alias="pre" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="PreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="MasterA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SchemaBeforeOptingInSchemaIsImported" alias="pre" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="PreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECProperty propertyName="P3" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Adding a property to an existing class should not violate with the policy";
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SchemaBeforeOptingInSchemaIsImported" alias="pre" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="PreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECProperty propertyName="P3" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="PreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "Schema update does not allow any schemas to violate the policy, even if they were imported before the opting in schema";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, NoAdditionalLinkTables)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
        </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")}));

        ASSERT_EQ(ERROR, TestHelper::RunSchemaImport({SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00">
                    <Exceptions>
                        <string>Schema1:MyLinkTableRel</string>
                    </Exceptions>
                </NoAdditionalLinkTables>
            </ECCustomAttributes>
            </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
               </ECRelationshipClass>
               <ECRelationshipClass typeName="MyLinkTableRel2" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
               </ECRelationshipClass>
        </ECSchema>)xml")}));

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemapolicies1.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00">
                    <Exceptions>
                        <string>Schema1:MyLinkTableRel</string>
                    </Exceptions>
                </NoAdditionalLinkTables>
            </ECCustomAttributes>
            </ECSchema>)xml"),
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="Core" version="01.00" alias="core"/>
           <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
           </ECRelationshipClass>
          </ECSchema>)xml")}));

    ASSERT_TRUE(GetHelper().TableExists("core_CoreA"));
    ASSERT_TRUE(GetHelper().TableExists("core_CoreB"));
    ASSERT_TRUE(GetHelper().TableExists("s1_MyLinkTableRel"));
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemapolicies2.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
           <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="CoreB"/>
                </Target>
           </ECRelationshipClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
            <ECSchemaReference name="Core" version="01.00" alias="core"/>

            <ECCustomAttributeClass typeName="MyCA" modifier="Sealed" appliesTo="Schema" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECCustomAttributeClass>

            <ECStructClass typeName="MyStruct" modifier="Sealed" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECStructClass>

            <ECEntityClass typeName="MyMixin" modifier="Abstract" >
                    <ECCustomAttributes>
                        <IsMixin xmlns="CoreCustomAttributes.01.00">
                            <AppliesToEntityClass>core:CoreA</AppliesToEntityClass>
                        </IsMixin>
                    </ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>

            <ECEntityClass typeName="MyEntity" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
                <ECNavigationProperty propertyName="B1" relationshipName="MyLogicalFkRel" direction="Backward"/>
                <ECNavigationProperty propertyName="B2" relationshipName="MyPhysicalFkRel" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>

            <ECRelationshipClass typeName="MyLinkTableSubRel" modifier="Sealed" strength="Referencing">
                <BaseClass>core:MyLinkTableRel</BaseClass>
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
           </ECRelationshipClass>

            <ECRelationshipClass typeName="MyLogicalFkRel" modifier="Sealed" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
                    <Class class="MyEntity"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="MyEntity"/>
                </Target>
           </ECRelationshipClass>

            <ECRelationshipClass typeName="MyPhysicalFkRel" modifier="Sealed" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
                    <Class class="MyEntity"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="MyEntity"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ASSERT_TRUE(GetHelper().TableExists("core_CoreA"));
    ASSERT_TRUE(GetHelper().TableExists("core_CoreB"));
    ASSERT_TRUE(GetHelper().TableExists("core_MyLinkTableRel"));
    ASSERT_TRUE(GetHelper().TableExists("s1_MyEntity"));
    ASSERT_FALSE(GetHelper().TableExists("s1_MyLinkTableSubRel"));
    }

        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemapolicies3.ecdb"));
        ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            </ECSchema>)xml"),
            SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            </ECSchema>)xml")}));

        ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1" >
                <ECSchemaReference name="Core" version="01.00" alias="core"/>
                
                <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreB"/>
                </Target>
                </ECRelationshipClass>       
            </ECSchema>)xml")));
        }

        {
        ASSERT_EQ(SUCCESS, SetupECDb("schemapolicies3.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            </ECSchema>)xml")));

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            </ECSchema>)xml")));
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

        ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="CoreB"/>
                </Target>
             </ECRelationshipClass>
        </ECSchema>)xml"))) << "Schema update does not allow any schemas, even if they were imported before the opting in schema";
        }

        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemapolicies4.ecdb"));
        ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
              </ECEntityClass>
              <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
              </ECEntityClass>
            </ECSchema>)xml"),
            SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core2" alias="core2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="core:CoreB"/>
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml")}));

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
            </ECSchema>)xml")));

        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Core2" alias="core2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECRelationshipClass typeName="MyLinkTableRel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                  <Class class="core:CoreB"/>
                </Target>
                <ECProperty propertyName="P1" typeName="string" />
              </ECRelationshipClass>
            </ECSchema>)xml"))) << "Adding a property to a link table relationship that already existed should not violate the policy";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, NoAdditionalForeignKeyConstraints)
    {
        {
            ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaPolicies_NoAdditionalForeignKeys.ecdb"));
            ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
           SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalForeignKeyConstraints xmlns="ECDbSchemaPolicies.01.00"/>
            </ECCustomAttributes>
        </ECSchema>)xml")}));

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECEntityClass typeName="SubB" modifier="None" >
                <BaseClass>core:CoreB</BaseClass>
                <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward" />
              </ECEntityClass>
              <ECRelationshipClass typeName="Rel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="SubB"/>
                </Target>
             </ECRelationshipClass>
          </ECSchema>)xml"))) << "Excepted to succeed as nav prop doesn't have ForeignKeyConstraint";

        ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema2" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECEntityClass typeName="Sub2B" modifier="None" >
                <BaseClass>core:CoreB</BaseClass>
                <ECNavigationProperty propertyName="A" relationshipName="Rel2" direction="Backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
              </ECEntityClass>
              <ECRelationshipClass typeName="Rel2" modifier="None" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="Sub2B"/>
                </Target>
             </ECRelationshipClass>
          </ECSchema>)xml"))) << "Excepted to fail as nav prop has ForeignKeyConstraint";
          }

        {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaPolicies_NoAdditionalForeignKeys.ecdb"));
        ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="CoreA" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="CoreB" modifier="None" >
                <ECProperty propertyName="P1" typeName="string" />
                <ECProperty propertyName="P2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"),
            SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="OptingInSchema" alias="master" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
            <ECCustomAttributes>
                <NoAdditionalForeignKeyConstraints xmlns="ECDbSchemaPolicies.01.00">
                    <Exceptions>
                        <string>Schema2:Sub2B.A</string>
                    </Exceptions>
                </NoAdditionalForeignKeyConstraints>
            </ECCustomAttributes>
        </ECSchema>)xml")}));

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECEntityClass typeName="SubB" modifier="None" >
                <BaseClass>core:CoreB</BaseClass>
                <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward" />
              </ECEntityClass>
              <ECRelationshipClass typeName="Rel" modifier="None" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="SubB"/>
                </Target>
             </ECRelationshipClass>
          </ECSchema>)xml"))) << "Excepted to succeed as nav prop doesn't have ForeignKeyConstraint";

        ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECEntityClass typeName="Sub2B" modifier="None" >
                <BaseClass>core:CoreB</BaseClass>
                <ECNavigationProperty propertyName="A" relationshipName="Rel2" direction="Backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
              </ECEntityClass>
              <ECRelationshipClass typeName="Rel2" modifier="None" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="Sub2B"/>
                </Target>
             </ECRelationshipClass>
          </ECSchema>)xml"))) << "Excepted to succeed as ForeignKeyConstraint is on exception list";

        ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Schema3" alias="s3" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
              <ECSchemaReference name="Core" version="01.00" alias="core"/>
              <ECEntityClass typeName="Sub3B" modifier="None" >
                <BaseClass>core:CoreB</BaseClass>
                <ECNavigationProperty propertyName="A" relationshipName="Rel3" direction="Backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
              </ECEntityClass>
              <ECRelationshipClass typeName="Rel3" modifier="None" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="core:CoreA"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="Sub3B"/>
                </Target>
             </ECRelationshipClass>
          </ECSchema>)xml"))) << "Excepted to fail as ForeignKeyConstraint is not on exception list";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiTest                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, ExceptionTests)
    {
    Utf8CP coreSchemaXmlTemplate = R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="Core" alias="core" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
                    <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
                    <ECCustomAttributes>
                        <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00">
                            <Exceptions>
                                <string>%s</string>
                                <string>Schema1.Child</string>
                            </Exceptions>
                        </NoAdditionalRootEntityClasses>
                        <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00">
                            <Exceptions>
                                <string>%s</string>
                            </Exceptions>
                        </NoAdditionalLinkTables>
                        <NoAdditionalForeignKeyConstraints xmlns="ECDbSchemaPolicies.01.00">
                            <Exceptions>
                                <string>%s</string>
                            </Exceptions>
                        </NoAdditionalForeignKeyConstraints>
                    </ECCustomAttributes>
                </ECSchema>)xml";

    Utf8CP testSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Foo" modifier="None" >
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="None" strength="Referencing">
                        <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                            <Class class="Foo"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                            <Class class="Foo"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECEntityClass typeName="Child" modifier="None" >
                        <ECNavigationProperty propertyName="Parent" relationshipName="FooHasChild" direction="Backward" >
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="FooHasChild" modifier="None" strength="Referencing">
                        <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                            <Class class="Foo"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                            <Class class="Child"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml";

    auto assertImport = [&] (Utf8CP rootEntityException, Utf8CP linkTableException, Utf8CP fkException)
        {
        Utf8String coreSchemaXml;
        coreSchemaXml.Sprintf(coreSchemaXmlTemplate, rootEntityException, linkTableException, fkException);
        return TestHelper::RunSchemaImport({SchemaItem(coreSchemaXml), SchemaItem(Utf8String(testSchemaXml))});
        };

    ASSERT_EQ(SUCCESS, assertImport("Schema1:Foo","Schema1:Rel","Schema1:Child.Parent"));
    ASSERT_EQ(SUCCESS, assertImport("Schema1.Foo", "Schema1.Rel", "Schema1.Child.Parent"));
    ASSERT_EQ(SUCCESS, assertImport("Schema1:Foo", "Schema1:Rel", "Schema1:Child:Parent"));
    ASSERT_EQ(ERROR, assertImport("Schema1,Foo", "Schema1,Rel", "Schema1,Child,Parent")) << "Comma is not a valid separator";
    ASSERT_EQ(SUCCESS, assertImport("schema1:foo", "schema1:rel", "schema1:child.parent")) << "Exceptions are expected to be case insensitive";
    ASSERT_EQ(SUCCESS, assertImport("schema1:fOo", "schema1:rEl", "schemA1:chiLd.parEnt")) << "Exceptions are expected to be case insensitive";
    ASSERT_EQ(SUCCESS, assertImport("s1:Foo", "s1:Rel", "s1:chiLd.Parent")) << "Exceptions are expected to accept schema name or schema alias";
    ASSERT_EQ(ERROR, assertImport("Schema2:Foo", "Schema2:Rel", "Schema2:Child.Parent")) << "Wrong schema name";
    ASSERT_EQ(ERROR, assertImport("s2:Foo", "s2:Rel", "s2:Child.Parent")) << "Wrong schema alias";
    ASSERT_EQ(ERROR, assertImport("Schema1:Goo", "Schema1:Rel", "Schema1:Child.Parent")) << "Wrong class name";
    ASSERT_EQ(ERROR, assertImport("Schema1:Foo", "Schema1:Rel2", "Schema1:Child.Parent")) << "Wrong rel class name";
    ASSERT_EQ(ERROR, assertImport("Schema1:Foo", "Schema1:Rel", "Schema1:Child2.Parent")) << "Wrong nav prop class name";
    ASSERT_EQ(ERROR, assertImport("Schema1:Foo", "Schema1:Rel", "Schema1:Child.Parent2")) << "Wrong nav prop class name";
    }

//---------------------------------------------------------------------------------------
// @bsiTest                                      Krischan.Eberle                06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaPolicyTestFixture, UpdatingECDbSchemas)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemapolicy_updatingecdbschemas.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
                    <ECSchemaReference name="ECDbSchemaPolicies" version="01.00" alias="ecdbpol"/>
                    <ECCustomAttributes>
                        <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00"/>
                        <NoAdditionalLinkTables xmlns="ECDbSchemaPolicies.01.00"/>
                        <NoAdditionalForeignKeyConstraints xmlns="ECDbSchemaPolicies.01.00"/>
                    </ECCustomAttributes>
                </ECSchema>)xml")));

    
    for (Utf8CP ecdbSchema : {"ECDbFileInfo", "ECDbMap", "ECDbMeta", "ECDbSchemaPolicies", "ECDbSystem"})
        {
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
        ECSchemaCP schema = m_ecdb.Schemas().GetSchema(ecdbSchema, true);
        ASSERT_TRUE(schema != nullptr);

        //Add to the ECDb schema new root entity classes, a link table rel, and a nav prop relationship with FK constraint
        ECSchemaR schemaR = const_cast<ECSchemaR>(*schema);
        ASSERT_EQ(ECObjectsStatus::Success, schemaR.SetVersionMinor(schemaR.GetVersionMinor() + 1));

        ECEntityClassP parentClass, childClass = nullptr;
        ECRelationshipClassP navPropRelClass, linkTableRelClass = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schemaR.CreateEntityClass(parentClass, "Parent"));
        ASSERT_EQ(ECObjectsStatus::Success, schemaR.CreateEntityClass(childClass, "Child"));

        ASSERT_EQ(ECObjectsStatus::Success, schemaR.CreateRelationshipClass(linkTableRelClass, "LinkTableRelClass"));
        linkTableRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
        ASSERT_EQ(ECObjectsStatus::Success, linkTableRelClass->GetSource().AddClass(*parentClass));
        linkTableRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
        ASSERT_EQ(ECObjectsStatus::Success, linkTableRelClass->GetTarget().AddClass(*childClass));

        ASSERT_EQ(ECObjectsStatus::Success, schemaR.CreateRelationshipClass(navPropRelClass, "NavPropRelClass"));
        navPropRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
        ASSERT_EQ(ECObjectsStatus::Success, navPropRelClass->GetSource().AddClass(*parentClass));
        navPropRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
        ASSERT_EQ(ECObjectsStatus::Success, navPropRelClass->GetTarget().AddClass(*childClass));

        NavigationECPropertyP navProp = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, childClass->CreateNavigationProperty(navProp, "Parent", *navPropRelClass, ECRelatedInstanceDirection::Backward));
        
        ECClassCP fkConstraintCaClass = m_ecdb.Schemas().GetClass("ECDbMap", "ForeignKeyConstraint");
        ASSERT_TRUE(fkConstraintCaClass != nullptr);
        if (schema->GetReferencedSchemas().find(SchemaKey("ECDbMap", 2, 0)) == schema->GetReferencedSchemas().end())
            schemaR.AddReferencedSchema(const_cast<ECSchemaR> (fkConstraintCaClass->GetSchema()));

        IECInstancePtr fkConstraintCa = fkConstraintCaClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(fkConstraintCa != nullptr);
        navProp->SetCustomAttribute(*fkConstraintCa);

        bvector<ECSchemaCP> modifiedSchemas;
        modifiedSchemas.push_back(schema);
        ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(modifiedSchemas));
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
        }
    }
END_ECDBUNITTESTS_NAMESPACE