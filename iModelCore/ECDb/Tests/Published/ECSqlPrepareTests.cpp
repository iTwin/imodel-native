/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlPrepareTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                    Krischan.Eberle                     10/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlPrepareTestFixture : public ECDbTestFixture
    {
    protected:
        ECSqlPrepareTestFixture() : ECDbTestFixture() {}

        ECSqlStatus Prepare(Utf8CP ecsql) const { return GetHelper().PrepareECSql(ecsql); }
    public:
        virtual ~ECSqlPrepareTestFixture() {}

        void SetUp() override
            {
            EXPECT_EQ(SUCCESS, SetupECDb("ecsqlpreparetests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));
            EXPECT_EQ(SUCCESS, PopulateECDb(10));
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                    09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlPrepareTestFixture, ReservedTokens)
    {
    //Class names with reserved tokens
    std::vector<SchemaItem> schemas;
    schemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="SELECT" />
                    <ECEntityClass typeName="FROM" />
                    <ECEntityClass typeName="WHERE" />
                    <ECEntityClass typeName="AND" />
                    <ECEntityClass typeName="OR" />
                    <ECEntityClass typeName="NOT" />
                    <ECEntityClass typeName="LIKE" />
                    <ECEntityClass typeName="ORDER" />
                    <ECEntityClass typeName="BY" />
                    <ECEntityClass typeName="GROUP" />
                    <ECEntityClass typeName="HAVING" />
                    <ECEntityClass typeName="LIMIT" />
                    <ECEntityClass typeName="OFFSET" />
                    <ECEntityClass typeName="INSERT" />
                    <ECEntityClass typeName="INTO" />
                    <ECEntityClass typeName="VALUES" />
                    <ECEntityClass typeName="UPDATE" />
                    <ECEntityClass typeName="SET" />
                    <ECEntityClass typeName="DELETE" />
                  </ECSchema>)xml"));

    schemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="select" />
                    <ECEntityClass typeName="from" />
                    <ECEntityClass typeName="where" />
                    <ECEntityClass typeName="and" />
                    <ECEntityClass typeName="or" />
                    <ECEntityClass typeName="not" />
                    <ECEntityClass typeName="like" />
                    <ECEntityClass typeName="order" />
                    <ECEntityClass typeName="by" />
                    <ECEntityClass typeName="group" />
                    <ECEntityClass typeName="having" />
                    <ECEntityClass typeName="limit" />
                    <ECEntityClass typeName="offset" />
                    <ECEntityClass typeName="insert" />
                    <ECEntityClass typeName="into" />
                    <ECEntityClass typeName="values" />
                    <ECEntityClass typeName="update" />
                    <ECEntityClass typeName="set" />
                    <ECEntityClass typeName="delete" />
                  </ECSchema>)xml"));

    
    for (SchemaItem const& schema : schemas)
        {
        ASSERT_EQ(SUCCESS, SetupECDb("ReservedECSQLTokens.ecdb", schema));
        ECN::ECSchemaCP schema = m_ecdb.Schemas().GetSchema("TestSchema");
        ASSERT_TRUE(schema != nullptr);

        for (ECN::ECClassCP cl : schema->GetClasses())
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT * FROM ts.%s", cl->GetName().c_str());
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
            }
        }

    //Property names with reserved tokens
    schemas.clear();
    schemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="SELECT" typeName="int"/>
                    <ECProperty propertyName="FROM" typeName="int"/>
                    <ECProperty propertyName="WHERE" typeName="int"/>
                    <ECProperty propertyName="AND" typeName="int"/>
                    <ECProperty propertyName="OR" typeName="int"/>
                    <ECProperty propertyName="NOT" typeName="int"/>
                    <ECProperty propertyName="LIKE" typeName="int"/>
                    <ECProperty propertyName="ORDER" typeName="int"/>
                    <ECProperty propertyName="BY" typeName="int"/>
                    <ECProperty propertyName="GROUP" typeName="int"/>
                    <ECProperty propertyName="HAVING" typeName="int"/>
                    <ECProperty propertyName="LIMIT" typeName="int"/>
                    <ECProperty propertyName="OFFSET" typeName="int"/>
                    <ECProperty propertyName="INSERT" typeName="int"/>
                    <ECProperty propertyName="INTO" typeName="int"/>
                    <ECProperty propertyName="VALUES" typeName="int"/>
                    <ECProperty propertyName="UPDATE" typeName="int"/>
                    <ECProperty propertyName="SET" typeName="int"/>
                    <ECProperty propertyName="DELETE" typeName="int"/>
                    </ECEntityClass>
                  </ECSchema>)xml"));

    schemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" >
                    <ECProperty propertyName="select" typeName="int"/>
                    <ECProperty propertyName="from" typeName="int"/>
                    <ECProperty propertyName="where" typeName="int"/>
                    <ECProperty propertyName="and" typeName="int"/>
                    <ECProperty propertyName="or" typeName="int"/>
                    <ECProperty propertyName="not" typeName="int"/>
                    <ECProperty propertyName="like" typeName="int"/>
                    <ECProperty propertyName="order" typeName="int"/>
                    <ECProperty propertyName="by" typeName="int"/>
                    <ECProperty propertyName="group" typeName="int"/>
                    <ECProperty propertyName="having" typeName="int"/>
                    <ECProperty propertyName="limit" typeName="int"/>
                    <ECProperty propertyName="offset" typeName="int"/>
                    <ECProperty propertyName="insert" typeName="int"/>
                    <ECProperty propertyName="into" typeName="int"/>
                    <ECProperty propertyName="values" typeName="int"/>
                    <ECProperty propertyName="update" typeName="int"/>
                    <ECProperty propertyName="set" typeName="int"/>
                    <ECProperty propertyName="delete" typeName="int"/>
                    </ECEntityClass>
                  </ECSchema>)xml"));

    for (SchemaItem const& schema : schemas)
        {
        ASSERT_EQ(SUCCESS, SetupECDb("ReservedECSQLTokens.ecdb", schema));
        ECN::ECClassCP cl = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        ASSERT_TRUE(cl != nullptr);

        for (ECN::ECPropertyCP prop : cl->GetProperties())
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT %s FROM ts.%s", prop->GetName().c_str(), cl->GetName().c_str());
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
            }
        }
    }


struct ECSqlSelectPrepareTests : ECSqlPrepareTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                      10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Alias)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, PStructProp A11 FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.ECInstanceId FROM ecsql.PSA S")) << "tests when class alias is same as a property name.This should work unless the property is a struct property";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.ECInstanceId FROM ecsql:PSA S")) << "tests when class alias is same as a property name.This should work unless the property is a struct property";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.S FROM ecsql.PSA S")) << "tests when class alias is same as a property name.This should work unless the property is a struct property";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.S FROM ecsql:PSA S")) << "tests when class alias is same as a property name.This should work unless the property is a struct property";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsql.PSA S")) << "tests when class alias is same as a property name.This should work unless the property is a struct property";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.I FROM ecsql.PSA S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, 0x332dff ffff FROM ecsql.PSA S WHERE I=0x2abdef+0x233+22"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, 0x332dffz kkk FROM ecsql.PSA S WHERE I > 0x2abdefz"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM (SELECT S FROM ecsql.PSA) S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.S FROM (SELECT S FROM ecsql.PSA) S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM (SELECT S, I FROM ecsql.PSA) S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.I FROM (SELECT S, I FROM ecsql.PSA) S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.S FROM ecsql.PSA, (SELECT ECInstanceId, I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S.Bla FROM ecsql.PSA, (SELECT ECInstanceId, I, 3.14 AS Bla FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT S.S FROM ecsql.PSA, (SELECT I, S FROM ecsql.PSA) S WHERE PSA.ECInstanceId=S.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp FROM ecsql.PSA PStructProp"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.ECInstanceId FROM ecsql.PSA PStructProp"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.s FROM ecsql.PSA PStructProp"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.I FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.PStructProp FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT PStructProp.i FROM ecsql.PSA, (SELECT ECInstanceId, I, PStructProp FROM ecsql.PSA) PStructProp WHERE PSA.ECInstanceId=PStructProp.ECInstanceId"));
    //EH TFS#521392 
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM meta.ECClassDef this WHERE ECInstanceId IN (SELECT Class.Id FROM meta.ECPropertyDef WHERE Class.Id = this.ECInstanceId)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM meta.ECClassDef this WHERE ECInstanceId = (SELECT Class.Id FROM meta.ECPropertyDef WHERE Class.Id = this.ECInstanceId)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, (SELECT Class.Id FROM meta.ECPropertyDef WHERE Class.Id = this.ECInstanceId) FROM meta.ECClassDef this"));

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, AndOrPrecedence)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.TH3 WHERE S1 IS NOT NULL OR S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.TH3 WHERE (S1 IS NOT NULL OR S2 IS NOT NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.TH3 WHERE S1 IS NULL AND S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.TH3 WHERE S1 IS NULL AND S2 IS NOT NULL OR 1=1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.TH3 WHERE S1 IS NULL AND (S2 IS NOT NULL OR 1=1)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Arrays)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT Dt_Array, B FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE Dt_Array = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE Dt_Array <> ?")) << "unbound parameters mean NULL and comparing NULL with NULL is always false";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE Dt_Array IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE CARDINALITY(Dt_Array) > 0")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, PStruct_Array FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStruct_Array = ?")) << "Struct arrays are not supported yet in where clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NULL")) << "Struct arrays are not supported yet in where clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStruct_Array IS NOT NULL")) << "Struct arrays are not supported yet in where clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE CARDINALITY(PStruct_Array) > 0")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT Dt_Array[1], B FROM ecsql.PSA")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT Dt_Array[100000], B FROM ecsql.PSA")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT Dt_Array[-1], B FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT UnknowProperty[1], B FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT UnknowProperty[-1], B FROM ecsql.PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, BetweenOperator)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 1 AND 3"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I BETWEEN 122 AND 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND 3"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'T'")) << "S always amounts to Sample String";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S BETWEEN 'Q' AND 'R'")) << "S always amounts to Sample String";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND 3"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN 1 AND ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT BETWEEN ? AND ?"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Cast)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS BINARY) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS BINARY) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS BINARY) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS BINARY) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS BINARY) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS BINARY[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Bi_Array AS BINARY[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (B AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (1 AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (I AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (True AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (False AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST ('1' AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Unknown AS BOOLEAN) FROM ecsql.PSA")) << "SQL-99 keyword UNKNOWN not supported in ECSQL as ECObjects doesn't have a counterpart for it.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (B AS BOOL) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS BOOLEAN) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS BOOLEAN[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (B_Array AS BOOLEAN[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Dt AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Dt AS DATETIME) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Dt AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (DtUtc AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (DtUtc AS DATETIME) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (DtUtc AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (TIMESTAMP '2013-02-09T12:00:00' AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (DATE '2013-02-09' AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (D AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (123425 AS DATETIME) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (123425.2343 AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (True AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (123425.123 AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS TIMESTAMP) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS TIMESTAMP[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (DtUtc_Array AS TIMESTAMP[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS DATE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS DATE[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Dt_Array AS DATE[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (L AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Dt AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS DOUBLE) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS DOUBLE[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (D_Array AS DOUBLE[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";
    
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (False AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS INT32) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS INT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS INT[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (I_Array AS INT[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (L AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (L AS INT64) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (True AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS LONG) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS LONG[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (L_Array AS LONG[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (B AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Bi AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (True AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (False AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (Dt AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (L AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (S AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (L AS TEXT) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS STRING) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS STRING[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (S_ARRAY AS STRING[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Bi AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (L AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (I AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (D AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (S AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS POINT2D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS POINT2D[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P2D_Array AS POINT2D[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Bi AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (L AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (I AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (D AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (S AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS POINT3D[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (P3D_Array AS POINT3D[]) FROM ecsql.PSA")) << "For arrays only CAST (NULL as Type[]) is supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (3.134 AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (L AS POINT3D) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (100000123 AS POINT3D) FROM ecsql.PSA"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (L AS ecsql.PStruct) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStructProp AS ecsql.PStruct) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (PStruct_Array AS PStruct[]) FROM ecsql.PSA")) << "For structs and arrays only CAST (NULL as <>) is supported";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS ecsql.PStruct) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS [ecsql].[PStruct]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS ecsql.PStruct[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS [ecsql].[PStruct][]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS PStruct) FROM ecsql.PSA")) << "CAST target struct must be fully qualified";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS PStruct[]) FROM ecsql.PSA")) << "CAST target struct must be fully qualified";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS ecsql.P) FROM ecsql.PSA")) << "CAST target is not a struct";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS ecsql.P[]) FROM ecsql.PSA")) << "CAST target is not a struct";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS Bla) FROM ecsql.PSA")) << "CAST target struct does not exist";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (NULL AS Bla[]) FROM ecsql.PSA")) << "CAST target struct does not exist";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (I AS IGeometry) FROM ecsql.PSA")) << "fails at step time";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (I AS Geometry) FROM ecsql.PSA")) << "fails at step time";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS IGeometry) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS Geometry) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT CAST (NULL AS IGeometry[]) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (Geometry_Array AS IGeometry[]) FROM ecsql.PASpatial")) << "For arrays only CAST (NULL as Type[]) is supported";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (? AS INT) FROM ecsql.PSA")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT CAST (I AS ?) FROM ecsql.PSA")) << "not yet supported";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Casing)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM EcSqltEst.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ECSQLTEST.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsqltest.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ECSqlTest.p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT s FROM ECSqlTest.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsQl.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsql.p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT s FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ecinsTanceiD FROM ECSqlTest.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ecCLassid FROM ECSqlTest.P"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, CommonGeometry)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Geometry, S FROM ecsql.PASpatial"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Geometry_Array, S FROM ecsql.PASpatial"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PASpatial ORDER BY Geometry")) << "Common Geometry properties cannot be ordered by ECSQL";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.PASpatial"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT p.* FROM ecsql.PASpatial p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(*) FROM ecsql.PASpatial p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.SSpatial"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SpatialStructProp FROM ecsql.SSpatial"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SpatialStructProp.Geometry FROM ecsql.SSpatial"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SpatialStructProp.Geometry_Array FROM ecsql.SSpatial"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.PASpatial WHERE Geometry_Array IS NOT NULL"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.SSpatial WHERE SpatialStructProp.Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.SSpatial WHERE SpatialStructProp.Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.SSpatial WHERE SpatialStructProp.Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ONLY ecsql.SSpatial WHERE SpatialStructProp.Geometry_Array IS NOT NULL"));
    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, DateTime)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2013-02-18T06:00:00.000'")) << "ECSQL supports the date and time component delimiter from both SQL-99 (space) and ISO 8601 ('T').";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt = DATE '2012-01-18'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt > DATE '2012-01-18' AND Dt <= TIMESTAMP '2014-01-01 12:00:00'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt = TIMESTAMP '2012-01-18 13:02:55.123'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt > TIMESTAMP '2012-01-18 13:02:55.123Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE Dt IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE DtUtc IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = :utc"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt > :dateonly"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00Z'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = TIMESTAMP '2013-02-18 06:00:00'")) << "DtUtc is UTC time stamp while RHS is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE DtUtc > :dateonly"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = DtUnspec")) << "DtUtc is UTC time stamp while DtUnspec has kind Unspecified.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc = Dt")) << "Dt has no DateTimeInfo, so it accepts any date time kind on the other side of the expression";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec = TIMESTAMP '2013-02-18 06:00:00Z'")) << "DtUnspec has DateTimeKind Unspecified while RHS has DateTimeKind UTC.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE DtUnspec > :dateonly"));

    //DateOnly has midnight as time fraction, DateTime columns as generated by test are not at midnight therefore they are not equal
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 00:00:00Z'")) << "Date-onlys (DateTime::Component::Date) can have any date time kind on other side of operation";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = TIMESTAMP '2013-02-18 00:00:00'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = Dt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly = DtUtc"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly <= DtUtc"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DateOnly < DtUtc"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2013-02-01')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', DATE '2013-02-01')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt IN (TIMESTAMP '2013-02-18 06:00:01', :utc)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00Z', DATE '2013-02-01')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')")) << "DtUtc is UTC time stamp while first item in list is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', DATE '2014-02-18')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec IN (TIMESTAMP '2013-02-18 06:00:00', TIMESTAMP '2014-02-18 06:00:00Z')")) << "Dt has date time kind Unspecified, but second item in list has kind Utc.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-02-18 06:00:00Z'")) << "DtUtc is UTC time stamp while lower bound operand is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2014-01-01 00:00:00Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00Z' AND TIMESTAMP '2013-02-01 12:00:00'")) << "DtUtc is UTC time stamp while upper bound operand is not.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUtc NOT BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND DATE '2014-01-01'")) << "DtUtc is UTC time stamp while lower bound operand is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE DtUtc BETWEEN TIMESTAMP '2013-02-01 00:00:00Z' AND :utc"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01 12:00:00' AND TIMESTAMP '2014-01-01 00:00:00'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN TIMESTAMP '2013-02-01T12:00:00' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND DATE '2014-01-01'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE DtUnspec BETWEEN DATE '2013-02-01' AND TIMESTAMP '2013-02-01 12:00:00Z'")) << "DtUnspec has date time kind Unspecified but upper bound has date time kind UTC.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16'")) << "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = TIME '13:35:16.123456'")) << "TIME constructor (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = LOCALTIME")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = ? OR Dt = ? OR S = ?"));

    //CURRENT_XXX functions
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_DATE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, CURRENT_DATE FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIMESTAMP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.P WHERE DtUtc = CURRENT_TIMESTAMP"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt FROM ecsql.P WHERE DtUnspec = CURRENT_TIMESTAMP")) << "In ECSQL CURRENT_TIMESTAMP returns a UTC time stamp.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, CURRENT_TIMESTAMP FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt FROM ecsql.P WHERE Dt = CURRENT_TIME")) << "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, CURRENT_TIME FROM ecsql.P")) << "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.";
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, ECInstanceId)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 AND I < 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 OR I > 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId ='123'")) << "ECSQL supports implicit conversion from string to number for ECInstanceId.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId <= '10000'")) << "ECSQL supports implicit conversion from string to number for ECInstanceId.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId IN ('123','334')")) << "ECSQL supports implicit conversion from string to number for ECInstanceId.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId BETWEEN '123' AND '223'")) << "ECSQL supports implicit conversion from string to number for ECInstanceId.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE ECInstanceId IN (123, (select ECInstanceId from ecsql.PSA where ECInstanceId = 223))"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = :id"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt FROM ecsql.PSA WHERE ECInstanceId = ?"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L FROM ecsql.PSA ECInstanceId")) << "Might become invalid because ECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA [ECInstanceId]"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS ECInstanceId FROM ecsql.PSA")) << "Might become invalid because ECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS [ECInstanceId] FROM ecsql.PSA"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L FROM ecsql.PSA SourceECInstanceId")) << "Might become invalid because SourceECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA [SourceECInstanceId]"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS SourceECInstanceId FROM ecsql.PSA")) << "Might become invalid because SourceECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS [SourceECInstanceId] FROM ecsql.PSA"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L FROM ecsql.PSA TargetECInstanceId")) << "Might become invalid because TargetECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA [TargetECInstanceId]"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS TargetECInstanceId FROM ecsql.PSA")) << "Might become invalid because TargetECInstanceId might become a reserved word.";
    //same test with escaping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L AS [TargetECInstanceId] FROM ecsql.PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, From)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql:PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql:PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ONLY ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM ecsql.PSAHasP"));

    //select from structs 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT i, s FROM ecsql.PStruct")) << "Structs are invalid in FROM clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT i, s FROM ONLY ecsql.PStruct")) << "Structs are invalid in FROM clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.PStruct")) << "Structs are invalid in FROM clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ONLY ecsql.PStruct")) << "Structs are invalid in FROM clause.";

    //select from CAs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM bsca.DateTimeInfo")) << "Custom Attributes are invalid in FROM clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ONLY bsca.DateTimeInfo")) << "Custom Attributes are invalid in FROM clause.";

    // Abstract classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ONLY ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ONLY ecsql.AbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.AbstractR")) << "Ttis should work and will be fixed.";

    // Unmapped classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM ecsql.PUnmapped")) << "Unmapped classes cannot be used in FROM clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM ONLY ecsql.PUnmapped")) << "Unmapped classes cannot be used in FROM clause.";

    // Unsupported classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM bsm.AnyClass"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ONLY bsm.AnyClass"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM bsm.InstanceCount"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ONLY bsm.InstanceCount"));

    // Missing schema alias / not existing ECClasses / not existing ECProperties
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM PSA")) << "Class name needs to be prefixed by schema alias.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM ecsql.BlaBla"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM blabla.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, blabla FROM ecsql.PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Functions)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(*) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT p.count(*) FROM ecsql.PSA p")) << "Class alias not allowed with count function.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(NULL) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT p.count(NULL) FROM ecsql.PSA p")) << "Class alias not allowed with count function.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(ECInstanceId) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(I) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT count(distinct I) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT count(distinct I, L) FROM ecsql.PSA p"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT count(distinct (I, L)) FROM ecsql.PSA p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT AVG (I) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT LENGTH (S) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsql.PSA WHERE LENGTH (S) = 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.PSA WHERE I = ROUND (122.8)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT group_concat(S) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT group_concat(S,'|') FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT group_concat(DISTINCT S) FROM ecsql.PSA"));

    //**** ECClassId
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT p.ECClassId FROM ecsql.PSA p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT p.ECClassId, c.ECClassId FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.PSA p WHERE ECClassId < 1000"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.PSA p WHERE p.ECClassId < 1000"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.PSA p ORDER BY ECClassId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.PSA p ORDER BY p.ECClassId"));

    //invalid expressions
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT GetClassId() FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT p.GetClassId() FROM ecsql.PSA p"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT p.GetClassId FROM ecsql.PSA p"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT GetClassId() FROM ecsql.PSA p JOIN ecsql.P c USING ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT a.GetClassId() FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT a.GetClassId() FROM ecsql.PSA p"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECClassId <> 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECClassId = 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE LOWER(S) = UPPER(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE LOWER(UPPER(S)) = LOWER (S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE LOWER(I)=I")) << "lower/upper only make sense with strings, but no failure if used for other data types (like in SQLite)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE UPPER(D)>0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE LOWER(S)=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE UPPER(?) = 'hello'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE InVirtualSet(?, ECInstanceId)"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId NOT MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE I MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (I + L) MATCH random()")) << "even though SQLite expects the LHS to be a column, we allow a value exp in the ECSQL grammar. Fails at step time only";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId MATCH '123'"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, GroupBy)
    {
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT I, count(*) FROM ecsql.PSA GROUP BY I"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT B, count(*) FROM ecsql.PSA GROUP BY B"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Bi"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Hex(Bi), count(*) FROM ecsql.PSA GROUP BY Bi"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT S, count(*) FROM ecsql.PSA GROUP BY S"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT DtUtc, count(*) FROM ecsql.PSA GROUP BY DtUtc"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Geometry, count(*) FROM ecsql.PASpatial GROUP BY Geometry"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT count(*) FROM ecsql.PSA GROUP BY S")) << "group by column not in select clause is supported (although against standard)";
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT S, count(*) FROM ecsql.PSA GROUP BY Length(S)")) << "functions in group by is supported (although against standard)";
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT count(*) FROM ecsql.PSA GROUP BY Length(S)"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Bi, count(*) FROM ecsql.PSA GROUP BY Hex(Bi)"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT I, L, count(*) FROM ecsql.PSA GROUP BY I + L"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT count(*) FROM ecsql.THBase GROUP BY ECClassId"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, count(*) FROM ecsql.PSA GROUP BY ?"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, count(*) FROM ecsql.PSA GROUP BY NULL"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, count(*) FROM ecsql.PSA GROUP BY 1"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT P2D, count(*) FROM ecsql.PSA GROUP BY P2D"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, count(*) FROM ecsql.PSA GROUP BY P3D"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp, count(*) FROM ecsql.PSA GROUP BY PStructProp"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Bi_Array, count(*) FROM ecsql.PSA GROUP BY Bi_Array"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT PStruct_Array, count(*) FROM ecsql.PSA GROUP BY PStruct_Array"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Geometry, count(*) FROM ecsql.PASpatial GROUP BY I HAVING Geometry IS NOT NULL"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT S, count(*) FROM ecsql.PSA GROUP BY S HAVING PStructProp IS NOT NULL"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT S, count(*) FROM ecsql.PSA GROUP BY S HAVING Length(S) > 1"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '0C0B%'"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT Hex(Bi), count(*) FROM ecsql.P GROUP BY Bi HAVING Hex(Bi) like '1C0B%'"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT S, count(*) FROM ecsql.PSA HAVING Length(S) > 1")) << "Although standard SQL allows, SQLite doesn't support HAVING without GROUP BY.";
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT MyPSA, count(*) FROM ecsql.P GROUP BY MyPSA"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT MyPSA.Id, count(*) FROM ecsql.P GROUP BY MyPSA.Id"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("SELECT MyPSA.RelECClassId, count(*) FROM ecsql.P GROUP BY MyPSA.RelECClassId"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, InOperator)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, 2, 3)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (1, 2, 3)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (L, I)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I NOT IN (L, I)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE S IN ('hello', 'Sample string')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (1, ?, 3)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I IN (?)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I IN ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE S NOT IN ('hello', 'world' )"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE I IN (1, 2, ROUND (122.9879))"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE L IN (1, AVG(L))"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE L IN SELECT L FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE IN (1, 2, 3)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE MyPSA IN (123)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE MyPSA.Id IN (-11)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE MyPSA.RelECClassId IN (-11)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Join)
    {
    //JOIN USING
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA BACKWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select parent.ECInstanceId, child.ECInstanceId FROM ecsql:PSA parent JOIN ecsql:PSA child USING ecsql:PSAHasPSA BACKWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select parent.ECInstanceId, child.ECInstanceId FROM ecsql.PSA parent JOIN ecsql.PSA child USING ecsql.PSAHasPSA FORWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PSA.*, P.* FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA BACKWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.I FROM ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA BACKWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ecsql.PSA end2 USING ecsql.PSAHasPSA BACKWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.I FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA FORWARD WHERE end2.I = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.PSA end2 USING ecsql.PSAHasPSA BACKWARD WHERE end2.I = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP WHERE end2.I = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP FORWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 JOIN ONLY ecsql.P end2 USING ecsql.PSAHasP BACKWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ONLY ecsql.PSA JOIN ONLY ecsql.PSA USING ecsql.PSAHasPSA")) << "ambiguous classes";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.PSA USING ecsql.PSAHasPSA BACKWARD")) << "ambiguous properties in select clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, L FROM ONLY ecsql.PSA JOIN ONLY ecsql.P USING ecsql.PSAHasP")) << "ambiguous properties in select clause";

    //JOIN ON
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "INNER JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "INNER JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end2.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasPSA rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.PSA end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT end1.I, end1.L FROM ONLY ecsql.PSA end1 "
        "JOIN ONLY ecsql.PSAHasP rel ON end1.ECInstanceId = rel.SourceECInstanceId "
        "JOIN ONLY ecsql.P end2 ON end2.ECInstanceId = rel.TargetECInstanceId "
        "WHERE end2.I = 123"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP FORWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select PSA.ECInstanceId, P.ECInstanceId FROM ecsql.PSA JOIN ecsql.P USING ecsql.PSAHasP BACKWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSA INNER JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId INNER JOIN ecsql.P ON P.ECInstanceId = PSAHasP.TargetECInstanceId"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSAHasPSA_1N"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSAHasPSA_11"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSAHasPSA_NN"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PHasSA_11P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PHasP_1NPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select REL.* FROM ONLY ecsql.PSAHasPSA_1N REL ORDER BY REL.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select REL.* FROM ONLY ecsql.PSAHasPSA_11 REL ORDER BY REL.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select REL.* FROM ONLY ecsql.PSAHasPSA_NN REL ORDER BY REL.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select REL.* FROM ONLY ecsql.PHasSA_11P REL ORDER BY REL.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select REL.* FROM ONLY ecsql.PHasP_1NPSA REL ORDER BY REL.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PHasP_1NPSA, ecsql.PSAHasPSA_1N, ecsql.PSAHasPSA_11, ecsql.PSAHasPSA_NN, ecsql.PHasSA_11P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select COUNT (*) FROM ONLY ecsql.PHasP_1NPSA, ONLY ecsql.PSAHasPSA_1N, ONLY ecsql.PSAHasPSA_11, ONLY ecsql.PSAHasPSA_NN, ONLY ecsql.PHasSA_11P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select REL.* FROM ecsql.PSA JOIN ecsql.PSA USING ecsql.PSAHasPSA_1N FORWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSA JOIN ecsql.PSA child USING ecsql.PSAHasPSA_1N FORWARD"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N RelationShipAliasNotAllowedYet FORWARD"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PSAHasPSA_1N.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_1N FORWARD ORDER BY PSAHasPSA_1N.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select  PSAHasPSA_11.*, PARENT.*, CHILD.*  FROM ONLY ecsql.PSA PARENT JOIN ecsql.PSA CHILD USING ecsql.PSAHasPSA_11 BACKWARD ORDER BY PSAHasPSA_11.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PSAHasPSA_NN.*, PARENT.*, CHILD.* FROM ONLY ecsql.PSA PARENT JOIN  ecsql.PSA CHILD USING ecsql.PSAHasPSA_NN FORWARD ORDER BY PSAHasPSA_NN.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PHasSA_11P.*, P.*, SA.* FROM ONLY ecsql.P JOIN ecsql.SA USING ecsql.PHasSA_11P ORDER BY PHasSA_11P.ECInstanceId DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select PHasP_1NPSA.*, PARENT.*, CHILD.* FROM ecsql.P PARENT JOIN ecsql.P CHILD USING ecsql.PHasP_1NPSA BACKWARD ORDER BY PHasP_1NPSA.ECInstanceId DESC"));

    //RIGHT JOIN
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select * FROM ecsql.PSA RIGHT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId")) << "RIGHT JOIN not supported (neither by SQLite nor by ECDb";

    //LEFT JOIN not a good example
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select * FROM ecsql.PSA LEFT JOIN ecsql.PSAHasP ON PSA.ECInstanceId = PSAHasP.SourceECInstanceId"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, LikeOperator)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE I LIKE 'Sam%'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 10"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam_le string'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam%' ESCAPE '\\'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\%' ESCAPE '\\'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S LIKE 'Sam\\_le string' ESCAPE '\\'"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE")) << "invalid escape clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE 10")) << "invalid escape clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE ?")) << "invalid escape clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam%' ESCAPE S")) << "invalid escape clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.PSA WHERE S NOT LIKE 'Sam\\%' {ESCAPE '\\'}")) << "invalid escape clause";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE S LIKE ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE S NOT LIKE ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, Dt, S FROM ecsql.P WHERE MyPSA LIKE '11'"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, LimitOffset)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 5"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 1+1+1+2"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET 3"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 10/2 OFFSET 3*1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 5 - ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 5 OFFSET ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT ? OFFSET ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * :pageno"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, I FROM ecsql.PSA LIMIT :pagesize OFFSET :pagesize * (:pageno - 1)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT S, I FROM ecsql.PSA LIMIT"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT S, I FROM ecsql.PSA LIMIT 10 OFFSET"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Misc)
    {
    // Statements where non-optional clauses are missing
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare(""));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("FROM ONLY ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT FROM ONLY ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S WHERE L > 109222"));

    // Select clause
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.PSA a"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.P WHERE ECInstanceId >= 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT a.* FROM ecsql.P a WHERE a.ECInstanceId >= 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.SA"));

    // Special tokens
    //These were reserved words in the original grammar introduced by some ODBC data time functions.
    //The ODBC stuff was removed from the ECSQL grammar, and the following tests serve as safeguards
    //against regressions when updating the grammar.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT D FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, S FROM ecsql.PSA d"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA t"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA ts"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT L, B FROM ecsql.PSA Z"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT DISTINCT Dt_Array FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT DISTINCT MyPSA FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT DISTINCT MyPSA.Id FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT DISTINCT MyPSA.RelECClassId FROM ecsql.P"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT _A_B_C,_ABC,_ABC_,A_B_C_,ABC_ FROM ecsql._UnderBar")) << "Select clause in which the class name and the properties name contain, start with or end with under bar";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT [_A_B_C],[_ABC],[_ABC_],[A_B_C_],[ABC_] FROM ecsql.[_UnderBar]")) << "Select clause in which the class name and the properties name contain, start with or end with under bar";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, NullLiteral)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL, I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL, NULL FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL as I FROM ecsql.P")) << "Alias in select clause is always interpreted literally even if it happens to be a property name.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL as P3D FROM ecsql.P")) << "Alias in select clause is always interpreted literally even if it happens to be a property name.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL as StructProp FROM ecsql.PSA")) << "Alias in select clause is always interpreted literally even if it happens to be a property name.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL as PStruct_Array FROM ecsql.SA")) << "Alias in select clause is always interpreted literally even if it happens to be a property name.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Options)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS")) << "OPTIONS clause without options";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS 123")) << "An option must be a name";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt=")) << "option value is missing";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt myOpt")) << "duplicate options not allowed (even if they differ by case)";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myOpt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt=1 myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt=1 myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P ECSQLOPTIONS myopt myotheropt=true onemoreopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt=1 myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId=? ORDER BY I ECSQLOPTIONS myopt=1 myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ECInstanceId=? GROUP BY I HAVING I=1 ECSQLOPTIONS myopt=1 myotheropt"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, OrderBy)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY L ASC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY L DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY L, S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY L ASC, S DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY Dt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY Dt ASC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY Dt DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA ORDER BY DtUtc DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE ECInstanceId >= 0 ORDER BY ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA a WHERE a.ECInstanceId >= 0 ORDER BY a.ECInstanceId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY S"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY LOWER (S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY UPPER (S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY P3D.X DESC"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY P3D.Z ASC"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY P2D.Z"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY 1")) << "constant value exp as order by->no-op";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE I < L ORDER BY I < 123")) << "boolean exp as order by";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY P2D DESC")) << "ORDER BY Point2D is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA tt ORDER BY tt.P2D ASC")) << "ORDER BY Point2D is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY P3D DESC")) << "ORDER BY Point3D is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY PStructProp DESC")) << "ORDER BY ECStruct is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY I_Array ASC")) << "ORDER BY arrays is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY P2D_Array DESC")) << "ORDER BY arrays is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA ORDER BY PStruct_Array ASC")) << "ORDER BY arrays is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.P ORDER BY MyPSA")) << "ORDER BY nav props is not supported by ECSQL.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.P ORDER BY MyPSA.Id"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.P ORDER BY MyPSA.RelECClassId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select I FROM ecsql.PSA ORDER BY NULLIF(I,123)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select I FROM ecsql.PSA ORDER BY COALESCE(I,L)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Parameters)
    {
    //This includes only advanced parameter tests that are not covered implicitly by the other test datasets

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = :i AND S = :s AND L = :i * 1000000 + 456789"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = ? AND S = :s AND L = :l"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = -?")) << "use unary operator (-) with parameter";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE ? = -?")) << "use parameters on both sides of an expression (should result in default parameter type)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE ? = ?")) << "use parameters on both sides of an expression (should result in default parameter type)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE :p1 > -:p1")) << "use parameters on both sides of an expression (should result in default parameter type)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE :p1 = -:p1")) << "use parameters on both sides of an expression (should result in default parameter type)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ?, S FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ? AS NewProp, S FROM ecsql.PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Points)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P2D FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, P2D FROM ecsql.PSA a"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, P2D FROM ecsql.PSA WHERE P2D = P2D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, P2D FROM ecsql.PSA WHERE P2D <> P2D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, P2D FROM ecsql.PSA WHERE P2D IN (P2D, P2D)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D, P2D FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D >= P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D <= P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D BETWEEN P2D AND P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D >= ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA a WHERE a.P2D <> ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D != ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P3D = ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D = P3D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D = P2D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P2D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P3D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE P3D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3, 45.134)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0, 0)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (0)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P2D FROM ecsql.PSA WHERE P2D = POINT2D (-1.3)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (-1.3, 45.134, 10)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0, 0)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (0, 0)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, P3D FROM ecsql.PSA WHERE P3D = POINT3D (1, -34.1)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D IN (P2D, P2D)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P3D IN (P3D, P3D)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D NOT IN (P2D, P2D)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P3D NOT IN (P3D, P3D)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D IN (?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D IN (:p1, :p2)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D = :p2 AND P2D != :p1)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR P2D = :p2 OR P2D = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P2D = :p1 OR (P2D != :p1 AND B = ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P3D IN (?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE P3D = :p1 OR P3D = :p2 OR P3D = ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I FROM ecsql.PSA WHERE P2D IN (POINT2D (1,1), POINT2D (2,2))")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I FROM ecsql.PSA WHERE P3D IN (POINT3D (1,1,1), POINT3D (2,2,2))")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I FROM ecsql.PSA WHERE P2D BETWEEN POINT2D (1,1) AND POINT2D (2,2)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I FROM ecsql.PSA WHERE P3D BETWEEN POINT3D (0,0,0) AND POINT3D (10,10,10)")) << "point literals not yet supported";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P2D.X, P2D.Y FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT P3D.X, P3D.Y, P3D.Z FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT P2D.Z FROM ecsql.PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Polymorphic)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, L FROM ONLY ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.THBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.TH1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.TH5"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.THBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.TH1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.TH5"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.TCBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.TC1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ONLY ecsql.TC5"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.TCBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.TC1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT * FROM ecsql.TC5"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select ECInstanceId FROM ecsql.Empty"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select ECInstanceId, I, S from ecsql.AbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select ECInstanceId from ecsql.EmptyAbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select ECInstanceId, I, S from only ecsql.AbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select ECInstanceId from only ecsql.EmptyAbstractNoSubclasses"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Primitives)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, L FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, L FROM ecsql.PSA a"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT a.I, a.L FROM ecsql.PSA a"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, ECInstanceId, S FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT Bi FROM ecsql.PSA a"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT 3.14 FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT 3.14 FROM ecsql.PSA WHERE L = 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT 1000 AS I FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT 3.14 AS BlaBla FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT b FROM ecsql.PSA")) << "Primitive Property with different case";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, d FROM ecsql.PSA")) << "Primitive Property with different case";
    
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE I = :p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE L = :p1 OR I = :p2"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR B = :p2 OR S = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE L = ? OR I = :p1 OR I > :p1 OR S = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.P WHERE I <> ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.P WHERE D <> ?"));

    //***** Boolean properties
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = false"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = False"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = Unknown")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE B = UNKNOWN")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.P WHERE B = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.P WHERE B <> ?"));

    //***** String literals
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample string'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE S = 'Sample \"string'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE S = \"Sample string\"")) << "String literals must be surrounded by single quotes. Double quotes are equivalent to square brackets in SQL.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE S_Array = 123")) << "DataType mismatch.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT I, S, B FROM ecsql.PSA WHERE S_Array = '123'")) << "Type mismatch in array.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Relationships)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE ECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE ECInstanceId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE TargetECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123 AND TargetECInstanceId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123 AND TargetECInstanceId <> 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE SourceECClassId = 123 AND TargetECClassId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasP WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE ECInstanceId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE ECInstanceId <>123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE TargetECInstanceId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE SourceECInstanceId =123 AND TargetECInstanceId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE SourceECInstanceId =123 AND TargetECInstanceId <> 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE SourceECClassId =123 AND TargetECClassId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPSA WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE B = false"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE B = false AND D = 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE TargetECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE SourceECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE SourceECClassId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE TargetECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE TargetECClassId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE ECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSAHasPWithPrimProps WHERE ECInstanceId > 123"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT SourceECClassId FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ecsql.PSAHasP_N1"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT TargetECClassId FROM ecsql.PSA"));
    //link table mapping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECClassId, TargetECClassId FROM ecsql.PSAHasPSA"));
    //end table mapping
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECClassId, TargetECClassId FROM ecsql.PSAHasP"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECClassId, TargetECClassId FROM ecsql.PSAHasP WHERE SourceECClassId = TargetECClassId AND ECClassId = 180"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel WHERE rel.SourceECClassId = rel.TargetECClassId AND rel.ECClassId = 180"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECClassId, TargetECClassId FROM ecsql.PSAHasP ORDER BY ECClassId, SourceECClassId, TargetECClassId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, SourceECClassId, TargetECClassId FROM ecsql.PSAHasP rel ORDER BY rel.ECClassId, rel.SourceECClassId, rel.TargetECClassId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT rel.ECClassId, rel.SourceECClassId, rel.TargetECClassId FROM ecsql.PSAHasP rel"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, SelectClause)
    {
    //tests with identically named select clause items. If one of them is an alias, preparation fails. Otherwise a unique name is generated

    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL, NULL bla FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL, NULL FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL, NULL, NULL FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULL bli, NULL bla FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select NULL bla, NULL bla FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select NULL I, I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select I, I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select I, L AS I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select L AS I, I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select I + L, I + L FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select I + L, I +L FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select L, ECClassId L FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select ECClassId S, S FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select NULLIF(I,123) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select NULLIF(I) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("select COALESCE(I,123,L) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select COALESCE(I) FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("select CASE WHEN I > 123 THEN 'Hello' ELSE 'World' END,S FROM ecsql.PSA")) << "CASE not yet supported";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Structs)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.i, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT PStructProp.j, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA")) << "Struct member property J does not exist";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp.I, PStructProp.dtUtc, PStructProp.b FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, PStructProp.B FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, PStructProp FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, PStructProp, P3D FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SAStructProp FROM ecsql.SA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SAStructProp.PStructProp FROM ecsql.SA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT SAStructProp.PStructProp, SAStructProp.PStructProp.p3d FROM ecsql.SA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA tt WHERE tt.PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NULL")) << "Structs that contain struct arrays are not supported in where clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp IS NOT NULL")) << "Structs that contain struct arrays are not supported in where clause.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE SAStructProp.PStructProp IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NULL")) << "Struct arrays are not supported in where clause.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM ecsql.SA WHERE PStruct_Array IS NOT NULL")) << "Struct arrays are not supported in where clause.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp.i IN (10, 123, 200)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I, S FROM ecsql.PSA WHERE PStructProp.i BETWEEN 10 AND 200"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp.i = 123 AND B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.PSA WHERE PStructProp.i = 123 AND PStructProp.dt <> DATE '2010-10-10' AND B = true"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp.PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp.PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp.PStructProp.i = 123 AND SAStructProp.PStructProp.dt <> DATE '2010-10-10'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp IS NULL")) << "Structs with struct array props are not supported in the where clause";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp IS NOT NULL")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp=?")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.SA WHERE SAStructProp<>?")) << "Structs with struct array props are not supported in the where clause";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Subquery)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.P WHERE ECInstanceId < (SELECT avg(ECInstanceId) FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM (SELECT * FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM (SELECT COUNT(*) FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM (SELECT * FROM P)")) << "Schema alias not mentioned before class name.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT ECInstanceId FROM (SELECT * FROM sql.P)")) << "Invalid Schema alias.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM (SELECT A FROM ecsql.P)")) << "Property 'A' does not match with any of the class properties.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.P WHERE (SELECT * FROM ecsql.P)")) << "SubQuery should return exactly 1 column.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT AVG(S) FROM (SELECT * FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM (SELECT I FROM ecsql.P HAVING COUNT(S)>1)")) << "A GROUP BY clause is mandatory before HAVING.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.PSA WHERE (SELECT ? FROM ecsql.PSA WHERE I=abc)")) << "Property 'I' doesn't take String values.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT L FROM (SELECT * FROM ecsql.P WHERE B <>)")) << "Property B is not assigned any value.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT L FROM ecsql.PSA WHERE(SELECT * FROM ecsql.P WHERE I BETWEEN 100)")) << "Syntax error in query.Expecting AND.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.P WHERE B = (SELECT * FROM ecsql.P)")) << "Outer clause expecting a single value whereas inner returns multiple.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE B = (SELECT DateOnly FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT I FROM ecsql.PSA WHERE B IN (SELECT DateOnly FROM ecsql.P)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM (SELECT I FROM ecsql.P WHERE COUNT(S)>1)")) << "WHERE clause can't be used with aggregate functions.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, Union)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.P UNION SELECT ECInstanceId FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECInstanceId FROM ecsql.P UNION ALL SELECT ECInstanceId FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.P UNION ALL SELECT B, Bi, I, L, S, P2D, P3D FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStructProp FROM ecsql.PSA UNION SELECT SAStructProp.PStructProp FROM ecsql.SA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PSA UNION ALL SELECT B_Array, Bi_Array, D_Array, Dt_Array, I_Array, S_Array, P2D_Array, P3D_Array FROM ecsql.PA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT PStruct_Array FROM ecsql.PSA UNION SELECT SAStructProp.PStruct_Array FROM ecsql.SA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, COUNT(*) FROM (SELECT ECClassId, ECInstanceId FROM ecsql.PSA UNION ALL SELECT ECClassId, ECInstanceId FROM ecsql.SA) GROUP BY ECClassId"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT ECClassId, Name, COUNT(*) FROM (SELECT ECClassId, ECInstanceId, 'PSA' Name FROM ecsql.PSA UNION ALL SELECT ECClassId, ECInstanceId, 'SA' Name FROM ecsql.SA) GROUP BY ECClassId, Name"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, NULL FROM ecsql.PSA UNION ALL SELECT NULL, I FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL, I FROM ecsql.PSA UNION ALL SELECT S, NULL FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S FROM ecsql.P UNION ALL SELECT Dt FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT S, 2.34 FROM ecsql.P UNION ALL SELECT I, S FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT Bi FROM ecsql.P UNION SELECT I_Array FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT Bi_Array FROM ecsql.PSA UNION SELECT I_Array FROM ecsql.PA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.P UNION SELECT * FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT S FROM ecsql.P UNION SELECT * FROM ecsql.PSA")) << "Number of properties should be same in all the select clauses.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT B FROM ecsql.P UNION SELECT B_Array FROM ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.P UNION ALL SELECT * FROM ecsql.PA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT * FROM ecsql.P UNION ALL SELECT * FROM ecsql.A")) << "'A' is not a valid class name.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectPrepareTests, WhereBasics)
    {
    //case insensitive tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B = NULL OR b = NULL"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE i>=:myParam"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE I IS 123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE B IS TRUE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L < 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (L < 3.14 AND I > 3) OR B = True AND D > 0.0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE 8 % 3 = 2"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE 8 % 2 = 0"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE (I&1)=1 AND ~(I|2=I)")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 5 + (4&1) = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 5 + 4 & 1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 5 + 4 | 1 = 9")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 4|1&1 = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE (4|1)&1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 4^1 = 0")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 5^4 = 4")) << "not yet supported";

    //unary predicates
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT B"));
    //SQLite function which ECDb knows to return a bool
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE Glob('*amp*',S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT Glob('*amp*',S)"));
    //Int/Long types are supported as unary predicate. They evalute to True if they are not 0.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NOT Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (I IS NOT NULL) AND L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (I IS NOT NULL) AND NOT L"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 3.14"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE 'hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE S"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P3D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P2D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D.X >= P3D.X AND P2D.Y >= P3D.Y"));
    //with parentheses around
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (P2D.X) >= (P3D.X) AND (P2D.Y) >= (P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (P2D.X >= P3D.X) AND (P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE (P2D.X >= P3D.X AND P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE Random()"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE Hex(Bi)"));
    //unary operator
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE -I = -123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE I == 10"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE Garbage = 'bla'"));
    //NULL tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL IS NULL")); // NULL IS NULL is always true
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL = NULL")); // NULL = NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL <> NULL")); // NULL <> NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL IS 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL IS NOT 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B = NULL")); // = NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B <> NULL"));  // <> NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L IS NULL OR I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE L IS NULL AND I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE S IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE S IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE B IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE I IS ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE I IS NOT ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ? = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE ? <> NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE NULL <> ?"));
    //points
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P2D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P2D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P2D = ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P3D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P3D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE P3D = ?"));
    //nav props
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA = L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA = MyPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA.Id = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("SELECT NULL FROM ecsql.P WHERE MyPSA.RelECClassId IS NULL"));

    //*******************************************************
    //  Unsupported literals
    //*******************************************************
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE B = UNKNOWN")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE Dt = TIME '13:35:16'")) << "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE Dt = LOCALTIME")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P2D = POINT2D (-1.3, 45.134)")) << "Point literal not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("SELECT NULL FROM ecsql.P WHERE P3D = POINT3D (-1.3, 45.134, 2)")) << "Point literal not yet supported";
    }




//********************* Insert **********************
struct ECSqlInsertPrepareTests : ECSqlPrepareTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Arrays)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (Dt_Array, B) VALUES (NULL, true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (PStruct_Array, B) VALUES (NULL, true)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Casing)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO EcSqltEst.P(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ECSQLTEST.P(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsqltest.P(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ECSqlTest.p(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsQl.P(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsQl.p(ECInstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsQl.p(EcinstanceId) VALUES(NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsQl.p(ecinstanceId) VALUES(NULL)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, CommonGeometry)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial VALUES (False, 3.14, 123, 'hello', NULL, NULL, NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial (I, Geometry) VALUES (123, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PASpatial (I, Geometry_Array) VALUES (123, ?)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, DateTime)
    {
    //Inserting into date time prop without DateTimeInfo CA
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55.123456')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2013-02-18T06:00:00.000')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (TIMESTAMP '2012-01-18 13:02:55Z')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (DATE '2012-01-18')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (NULL)"));

    //Inserting into UTC date time prop
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (DtUtc) VALUES (TIMESTAMP '2013-02-18 06:00:00')")) << "DtUtc is UTC time stamp while value is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUtc) VALUES (DATE '2012-01-18')"));

    //Inserting into date time prop with DateTimeInfo CA where kind is set to Unspecified
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (DtUnspec) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')")) << "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUnspec) VALUES (DATE '2012-01-18')"));

    //Inserting into date time props with DateTimeInfo CA where component is set to Date-only
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DateOnly) VALUES (DATE '2013-02-18')"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00Z')")) << "DateOnly can take time stamps, too";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DateOnly) VALUES (TIMESTAMP '2013-02-18 06:00:00')")) << "DateOnly can take time stamps, too";

    //CURRENT_XXX functions
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_DATE)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_DATE)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIMESTAMP)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (DtUtc) VALUES (CURRENT_TIMESTAMP)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (DtUnspec) VALUES (CURRENT_TIMESTAMP)")) << "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (Dt) VALUES (CURRENT_TIME)")) << "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.";

    //implicit conversions (supported what SQLite supports)
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (I, L) VALUES (123, DATE '2013-04-30')"));

    //*** Parameters ****
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (I, Dt, DtUtc, DtUnspec, DateOnly) VALUES (123, ?, ?, ?, ?)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Into)
    {


    // Inserting into classes which map to tables with ECClassId columns
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.THBase (S) VALUES ('hello')"));
    //inserting into classes with base classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH5 (S, S1, S3, S5) VALUES ('hello', 'hello1', 'hello3', 'hello5')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH3 VALUES ('hello', NULL, NULL, NULL, 'hello1', 'hello2', 'hello3')"));

    // Abstract classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.Abstract (I, S) VALUES (123, 'hello')")) <<"Abstract classes cannot be used in INSERT statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.AbstractNoSubclasses (I, S) VALUES (123, 'hello')")) << "Abstract classes cannot be used in INSERT statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasMyMixin(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)")) << "Abstract ECRels cannot be inserted.";
    // mixins
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.MyMixin(MixinCode) VALUES('new')")) << "Mixins are invalid in INSERT statements.";

    // Inserting into structs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PStruct (i, l, dt, b) VALUES (123, 1000000, DATE '2013-10-10', False)")) << "structs are not insertible";
    
    // Inserting into CAs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO bsca.DateTimeInfo (DateTimeKind) VALUES ('Utc')"));

    // Inserting into fk relationships
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP(SourceECInstanceId,TargetECInstanceId) VALUES (?,?)")) << "cannot insert into FK relationship - must use nav prop instead";

    // Unmapped classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PUnmapped (I, D) VALUES (123, 3.14)")) << "Unmapped classes cannot be used in INSERT statements.";

    // Subclasses of abstract class
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.Sub1 (I, S, Sub1I) VALUES (123, 'hello', 100123)"));

    // Empty classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.Empty (ECInstanceId) VALUES (NULL)"));

    // Unsupported classes
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an INSERT statement because of that in the first place
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO bsm.InstanceCount (ECSchemaName, ECClassName, Count) VALUES ('Foo', 'Goo', 103)")) << "InstanceCount class is not supported in ECSQL";

    // Missing schema alias / not existing ECClasses / not existing ECProperties
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO PSA (I, L, Dt) VALUES (123, 1000000, DATE '2013-10-10')")) << "Class name needs to be prefixed by schema alias.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.BlaBla VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO blabla.PSA VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (Garbage, I, L) VALUES ('bla', 123, 100000000)")) << "One of the properties does not exist in the target class.";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.Sub1 (I) VALUES (0xabcdef)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.Sub1 (I) VALUES (0xabcdef + 0x34d - 343)"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.Sub1 (I) VALUES (0xabcdefgih)"));

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Misc)
    {
    // Syntactically incorrect statements 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare(""));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT ecsql.P (I) VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (I)"));

    // Insert expressions 
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (I) VALUES (1 + 1)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (I) VALUES (5 * 4)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (L) VALUES (1 + ECClassId)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (L) VALUES (ECClassId * 4)"));

    // Insert ECInstanceId 
    //NULL for ECInstanceId means ECDb auto-generates the ECInstanceId.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId) VALUES (NULL)")) << "NULL for ECInstanceId means ECDb auto-generates the ECInstanceId.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId, I) VALUES (NULL, NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH2 (ECInstanceId, S2) VALUES (NULL, 'hello')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId) VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (4443412341)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId) VALUES (?)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP (ECInstanceId) VALUES (NULL)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasPSA (ECInstanceId) VALUES (NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId, I) VALUES (123, 123)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH2 (ECInstanceId, S1) VALUES (41241231231, 's1')"));

    //for link table mappings specifying the ECInstanceId is same as for regular classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPSA (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, 123, 345)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPSA(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(129, 123, 312)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPSA(ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES(?, 123, 312)"));


    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (NULL, 123, ?)")) << "cannot insert into fk relationship - nav prop must be used instead";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (?, 123, ?)")) << "cannot insert into fk relationship - nav prop must be used instead";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP (ECInstanceId, SourceECInstanceId, TargetECInstanceId) VALUES (14123123, 123, ?)")) << "cannot insert into fk relationship - nav prop must be used instead";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P (ECInstanceId) VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.TH2 (ECInstanceId) VALUES (412313)"));

    // Class aliases
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA t (t.Dt, t.L, t.S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')")) << "In SQLite class aliases are not allowed, but ECSQL allows them. So test that ECDb properly omits them during preparation";

    // Insert literal values
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (Dt, L, S) VALUES (DATE '2013-04-30', 100000000000, 'hello, world')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (L, S, I) VALUES (100000000000, 'hello, \" world', -1)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (L, I) VALUES (CAST (100000 AS INT64), 12 + 99)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (L, S, DtUtc) VALUES (?, ?, ?)"));
    

    // Insert without column clause
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL, NULL, NULL)"));

    //  VALUES clause mismatch
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (I, L) VALUES ('bla', 123, 100000000)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (I, L) VALUES (123)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P VALUES (123, 'bla bla')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P VALUES (True, NULL, 3.1415, TIMESTAMP '2013-10-14T12:00:00', TIMESTAMP '2013-10-14T12:00:00Z', TIMESTAMP '2013-10-14T12:00:00', DATE '2013-10-14', 123, 1234567890, 'bla bla', NULL)"));

    //  Literals
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (B) VALUES (true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (B) VALUES (True)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (B) VALUES (false)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (B) VALUES (UNKNOWN)")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (Dt) VALUES (DATE '2012-01-18')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (Dt) VALUES (TIMESTAMP '2012-01-18T13:02:55')"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (Dt) VALUES (TIME '13:35:16')")) << "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (Dt) VALUES (LOCALTIME)")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (P2D) VALUES (POINT2D (-1.3, 45.134))")) << "Point literals not supported yet";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSA (P3D) VALUES (POINT3D (-1.3, 45.134, 2))")) << "Point literals not supported yet";

    // Not yet supported flavors
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (I, L) SELECT I, L FROM ecsql.PSA")) << "not supported yet";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P (I, L) VALUES (1, 1234), (2, 32434)")) << "not supported yet";

    // Insert clause in which the class name and the properties name contain, start with or end with under bar
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.[_UnderBar] ([_A_B_C], [_ABC], [_ABC_], [A_B_C_], [ABC_]) VALUES ('_A_B_C', 2, '_ABC_', 4, 'ABC_')"));

    // Insert where string literal consists of Escaping single quotes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql._UnderBar (_A_B_C, _ABC, _ABC_, A_B_C_, ABC_) VALUES ('5''55''', 2, '''_ABC_', 4, 'ABC_''')"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Options)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt")) << "ECSQLOPTIONS not supported for INSERT";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt myotheropt")) << "ECSQLOPTIONS not supported for INSERT";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt=1 myotheropt")) << "ECSQLOPTIONS not supported for INSERT";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt=1 myotheropt=true")) << "ECSQLOPTIONS not supported for INSERT";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt myotheropt=true")) << "ECSQLOPTIONS not supported for INSERT";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt myotheropt=true onemoreopt")) << "ECSQLOPTIONS not supported for INSERT";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS")) << "OPTIONS clause without options";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS 123")) << "An option must be a name";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt=")) << "option value is missing";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt myOpt")) << "duplicate options not allowed (even if they differ by case)";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myopt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.P(I) VALUES(123) ECSQLOPTIONS myOpt=1 myopt")) << "duplicate options not allowed";
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Relationships)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("INSERT INTO ecsql.PSAHasP(SourceECInstanceId, TargetECInstanceId) VALUES (123, 321)")) << "Inserting into fk relationship is not supported - must insert via nav prop";

    // ECDb doesn't validate source/target class ids.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (123, 321)")) << "Target is non-polymorphic -> target class id not required";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasOnlyTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (123,?, 333, ?)")) << "Target is non-polymorphic -> target class id not required";

    //**** Case: target class id is mandatory as target constraint is ambiguous
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId) VALUES (123, 124);"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (123, 333, 124, 335)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (123, ?, 124, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasTHBase_NN (SourceECInstanceId, TargetECInstanceId, TargetECClassId) VALUES (123, 333, 124)"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, TargetECInstanceId) VALUES (123, 442)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, B, D, I, S) VALUES (123, 332, 124, 335, True, 3.14, 123, 'hello')"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSAHasPWithPrimProps (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (123, 332, 125, 334)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertPrepareTests, Structs)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (PStructProp, B) VALUES (NULL, true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (PStructProp, B) VALUES (?, true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (PStructProp.i, B) VALUES (123, true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.PSA (PStructProp.i, PStructProp.dt, B) VALUES (123, DATE '2010-10-10', true)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.SA (SAStructProp.PStructProp) VALUES (?)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("INSERT INTO ecsql.SA (SAStructProp.PStructProp.i, SAStructProp.PStructProp.dt) VALUES (123, DATE '2010-10-10')"));
    }


//********************* Update **********************
struct ECSqlUpdatePrepareTests : ECSqlPrepareTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, AndOrPrecedence)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TH3 SET S1=NULL WHERE S1 IS NOT NULL OR S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TH3 SET S1=NULL WHERE (S1 IS NOT NULL OR S2 IS NOT NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TH3 SET S1=NULL WHERE S1 IS NULL AND S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TH3 SET S1=NULL WHERE S1 IS NULL AND S2 IS NOT NULL OR 1=1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TH3 SET S1=NULL WHERE S1 IS NULL AND (S2 IS NOT NULL OR 1=1)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Arrays)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET Dt_Array = NULL, B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStruct_Array = NULL, B = true"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Casing)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ECSqlTest.P SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE EcSqltEst.P SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ECSQLTEST.P SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ECSqlTest.p SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.p SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsQl.P SET I=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET i=?"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, CommonGeometry)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET I = 123, Geometry = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET I = 123, Geometry_Array = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET I = 123 WHERE Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET I = 123 WHERE Geometry_Array IS NULL"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PASpatial SET Geometry=NULL WHERE Geometry_Array IS NOT NULL"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SSpatial SET SpatialStructProp.Geometry=NULL WHERE SpatialStructProp.Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SSpatial SET SpatialStructProp.Geometry=NULL WHERE SpatialStructProp.Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SSpatial SET SpatialStructProp.Geometry=NULL WHERE SpatialStructProp.Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SSpatial SET SpatialStructProp.Geometry=NULL WHERE SpatialStructProp.Geometry_Array IS NOT NULL"));
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, DateTime)
    {
    //updating date time prop without DateTimeInfo CA
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55.123'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55.123456'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2013-02-18T06:00:00.000'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = TIMESTAMP '2012-01-18 13:02:55Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = DATE '2012-01-18'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE Dt = TIMESTAMP '2012-01-18 13:02:55Z'"));

    //Updating UTC date time prop
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUtc = TIMESTAMP '2013-02-18 06:00:00Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = TIMESTAMP '2012-01-18 13:02:55Z'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET DtUtc = TIMESTAMP '2013-02-18 06:00:00'")) << "DtUtc is UTC time stamp while value is not.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = TIMESTAMP '2012-01-18 13:02:55'")) << "DtUtc is UTC time stamp while value is not.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUtc = DATE '2012-01-18'"));

    //Updating date time prop with DateTimeInfo CA where kind is set to Unspecified
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUnspec = TIMESTAMP '2013-02-18 06:00:00'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = TIMESTAMP '2012-01-18 13:02:55'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET DtUnspec = TIMESTAMP '2013-02-18 06:00:00Z'")) << "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = TIMESTAMP '2012-01-18 13:02:55Z'")) << "DtUnspec has DateTimeKind Unspecified while value has DateTimeKind UTC.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUnspec = DATE '2012-01-18'"));

    //Updating date time props with DateTimeInfo CA where component is set to Date-onlys
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DateOnly = DATE '2013-02-18'"));

    //DateOnly can take time stamps, too
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DateOnly = TIMESTAMP '2013-02-18 06:00:00Z'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DateOnly = TIMESTAMP '2012-01-18 13:02:55'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DateOnly = TIMESTAMP '2013-02-18 06:00:00'"));

    //CURRENT_XXX functions
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = CURRENT_DATE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUtc = CURRENT_DATE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = CURRENT_DATE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET Dt = CURRENT_TIMESTAMP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET DtUtc = CURRENT_TIMESTAMP"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUtc = CURRENT_TIMESTAMP"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET DtUnspec = CURRENT_TIMESTAMP")) << "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE DtUnspec = CURRENT_TIMESTAMP")) << "In ECSQL CURRENT_TIMESTAMP returns a UTC timestamp";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET Dt = CURRENT_TIME")) << "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.P SET I = 123 WHERE Dt = CURRENT_TIME")) << "CURRENT_TIME function (as specified in SQL-99) is not valid in ECSQL as the TIME type is not supported by ECObjects.";

    //*** Parameters ****
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.P SET I=123, Dt=?, DtUtc=?, DtUnspec=?, DateOnly=?"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Functions)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ECClassId <> 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ECClassId = 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE LOWER(S) = UPPER(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE LOWER(UPPER(S)) = LOWER (S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE LOWER(I)=I")) << "lower/upper only make sense with strings, but no failure if used for other data types (like in SQLite)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE UPPER(D)>0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE LOWER(S)=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE UPPER(?) = 'hello'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE InVirtualSet(?, ECInstanceId)"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ECInstanceId MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ECInstanceId NOT MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE I MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (I + L) MATCH random()")) << "even though SQLite expects the LHS to be a column, we allow a value exp in the ECSQL grammar. Fails at step time only";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE ECInstanceId MATCH '123'"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Misc)
    {
    // Syntactically incorrect statements 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare(""));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA WHERE I = 123"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET L = 0xabcdef"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET L = 0xabcdef + 0x3434fff+343"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET L = 0xabcdefgh"));

    // Typical updates
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET I = 124, L = 100000000000, D = -1.2345678, S = 'hello, world'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET Dt = ?, L = ?"));

    // Class aliases
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA t SET t.I = 124, t.L = 100000000000, t.D = -1.2345678, t.S = 'hello, world' WHERE t.D > 0.0")) << "Class alias are not allowed in SQLite, but ECSQL allows them. So test that ECDb properly omits them during preparation";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA t SET t.Dt = ?, t.L = ?")) << "Class alias are not allowed in SQLite, but ECSQL allows them. So test that ECDb properly omits them during preparation";

    // Update ECInstanceId 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET ECInstanceId = -3, I = 123")) << "Updating ECInstanceId is not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET [ECInstanceId] = -3, I = 123")) << "The bracketed property [ECInstanceId] refers to an ECProperty (and not to the system property ECInstanceId). Parsing [ECInstanceId] is not yet supported.";

    //  Literals
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET B = false"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET B = UNKNOWN")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET Dt = DATE '2012-01-18'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET Dt = TIMESTAMP '2012-01-18T13:02:55'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET Dt = TIME '13:35:16'")) << "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET Dt = LOCALTIME")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET P2D = POINT2D (-1.3, 45.134)")) << "Point literals not supported yet";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET P3D = POINT3D (-1.3, 45.134, 2)")) << "Point literals not supported yet";

    // Update clause in which the class name and the properties name contain, start with or end with under bar
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql._UnderBar u SET u._A_B_C = '1st Property', u._ABC = 22, u._ABC_ = '3rd Property', u.A_B_C_ = 44, u.ABC_= 'Last Property' WHERE u._ABC > 0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.[_UnderBar] u SET u.[_A_B_C] = '1st Property', u.[_ABC] = 22, u.[_ABC_] = '3rd Property', u.[A_B_C_] = 44, u.[ABC_]= 'Last Property' WHERE u.[_ABC] > 0"));

    // update clause where string literal consists of Escaping single quotes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql._UnderBar u SET u._A_B_C = '''', u._ABC = 22, u._ABC_ = '''5''', u.A_B_C_ = 44, u.ABC_= 'LAST''' WHERE u._ABC > 0"));

    // ECSQLOPTIONS ReadonlyPropertiesAreUpdatable
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.ClassWithLastModProp SET I=123, LastMod=? WHERE ECInstanceId=?")) << "readonly prop cannot be updated";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp.CreationDate=? WHERE ECInstanceId=?")) << "readonly prop cannot be updated";

    //cal props are always updatable
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.ClassWithLastModProp SET FullName='bla' WHERE ECInstanceId=?")) << "calc properties are always updatable";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.ClassWithLastModProp SET I=123, LastMod=? WHERE ECInstanceId=? ECSQLOPTIONS ReadonlyPropertiesAreUpdatable"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp.CreationDate=? WHERE ECInstanceId=? ECSQLOPTIONS ReadonlyPropertiesAreUpdatable"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.ClassWithLastModProp SET I=123, LastMod=? WHERE ECInstanceId=? ECSQLOPTIONS rEaDonlYPropertiEsAreupdaTable")) << "options are expected to be case insensitive";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.ClassWithLastModProp SET I=123 WHERE ECInstanceId=? ECSQLOPTIONS ReadonlyPropertiesAreUpdatable")) << "option has no affect if no readonly prop is in the SET clause";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Options)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS")) << "OPTIONS clause without options";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS 123")) << "An option must be a name";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt=")) << "option value is missing";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt myOpt")) << "duplicate options not allowed (even if they differ by case)";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myOpt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt=1 myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt=1 myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? ECSQLOPTIONS myopt myotheropt=true onemoreopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? WHERE ECInstanceId=? ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? WHERE ECInstanceId=? ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=? WHERE ECInstanceId=? ECSQLOPTIONS myopt=1 myotheropt"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Polymorphic)
    {
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I = 123"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.Abstract SET I = 123"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.Abstract SET I = 123"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.AbstractNoSubclasses SET I = 123"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.AbstractNoSubclasses SET I = 123"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.AbstractTablePerHierarchy SET I = 123"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.AbstractTablePerHierarchy SET I = 123"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.THBase SET S = 'hello'"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.THBase SET S = 'hello'"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.TCBase SET S = 'hello'"));
    EXPECT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.TCBase SET S = 'hello'"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.AbstractBaseWithSingleSubclass SET Prop1= 'hello'"));
    EXPECT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.AbstractBaseWithSingleSubclass SET Prop1= 'hello'"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Relationships)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE ECInstanceId = 123")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE ECInstanceId <> 123")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE SourceECInstanceId = 123")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE TargetECInstanceId = 123")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE SourceECInstanceId = 123 AND TargetECInstanceId = 124")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE SourceECInstanceId = 123 AND TargetECInstanceId <> 124")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE SourceECClassId = 123 AND TargetECClassId = 124")) << "Cannot update FK relationship - must update via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasP SET SourceECInstanceId=? WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124")) << "Cannot update FK relationship - must update via nav prop";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE ECInstanceId =123")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE ECInstanceId <>123")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE SourceECInstanceId = 123")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE TargetECInstanceId =123")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE SourceECInstanceId =123 AND TargetECInstanceId = 124")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE SourceECInstanceId =123 AND TargetECInstanceId <> 124")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE SourceECClassId =123 AND TargetECClassId = 124")) << "Cannot update Source/TargetECInstanceId";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasPSA SET SourceECInstanceId=? WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124")) << "Cannot update Source/TargetECInstanceId";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE B = false"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE B = false AND D = 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B = ? WHERE TargetECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE SourceECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE SourceECClassId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE TargetECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE TargetECClassId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE ECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSAHasPWithPrimProps SET B=? WHERE ECInstanceId > 123"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, Structs)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp = NULL, B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp = ?, B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp.i = 123, B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.PSA SET PStructProp.i = 123, PStructProp.dt = DATE '2010-10-10', B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.SA SET SAStructProp.PStructProp.i = 123, SAStructProp.PStructProp.dt = DATE '2010-10-10'"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp.i = 123 AND B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.PSA SET I=? WHERE PStructProp.i = 123 AND PStructProp.dt <> DATE '2010-10-10' AND B = true"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp.PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp.PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp.PStructProp.i = 123 AND SAStructProp.PStructProp.dt <> DATE '2010-10-10'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp IS NULL")) << "Structs with struct array props are not supported in the where clause";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp IS NOT NULL")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp=?")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.SA SET SAStructProp.PStructProp.i=? WHERE SAStructProp<>?")) << "Structs with struct array props are not supported in the where clause";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, TargetClass)
    {
    //Updating classes with base classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.TH5 SET S='hello', S1='hello1', S3='hello3', S5='hello5'"));

    // Abstract classes
    //by contract non-polymorphic updates on abstract classes are valid, but are a no-op
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.Abstract SET I=123, S='hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.Abstract SET I=123, S='hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.AbstractNoSubclasses SET I=123, S='hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.AbstractNoSubclasses SET I=123, S='hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.PSAHasMyMixin SET SourceECInstanceId=?")) << "ECRels cannot be updated.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSAHasMyMixin SET SourceECInstanceId=?")) << "ECRels cannot be updated.";
    // mixins
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.MyMixin SET MixinCode='new'")) << "Mixins are invalid in UPDATE statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.MyMixin SET MixinCode='new'")) << "Mixins are invalid in UPDATE statements.";

    // Updating structs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.SAStruct SET PStructProp.i=123, PStructProp.l=100000, PStructProp.dt=DATE '2013-10-10', PStructProp.b=False")) << "Structs are invalid in UPDATE statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PStruct SET i=123, l=10000, dt=DATE '2013-10-10', b=False")) << "Structs are invalid in UPDATE statements.";

    // Updating relationship
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSAHasP SET SourceECInstanceId=?")) << "FK relationships cannot be updated.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSAHasPSA SET SourceECInstanceId=?")) << "link table relationships cannot be updated.";

    // Updating CAs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY bsca.DateTimeInfo SET DateTimeKind='Utc'")) << "Custom Attributes classes are invalid in UPDATE statements.";

    // Unmapped classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PUnmapped SET I=123, D=3.14")) << "Unmapped classes cannot be used in UPDATE statements.";


    // Subclasses of abstract class
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ONLY ecsql.Sub1 SET I=123, S='hello', Sub1I=100123"));

    // Empty classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.Empty SET ECInstanceId = ?")) << "Empty classes cannot be used in UPDATE statements.";

    // Unsupported classes
    //AnyClass is unsupported, but doesn't have properties, so it cannot be used in an UPDATE statement because of that in the first place
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY bsm.InstanceCount SET ECSchemaName='Foo', ECClassName='Goo', Count=103"));

    // Missing schema alias / not existing ECClasses / not existing ECProperties
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY PSA SET I=123, L=100000")) << "Class name needs to be prefixed by schema alias.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.BlaBla SET I=123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY blabla.PSA SET I=123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ONLY ecsql.PSA SET Garbage='bla', I=123")) << "One of the properties does not exist in the target class.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdatePrepareTests, WhereBasics)
    {
    //case insensitive tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B = NULL OR b = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE i>=:myParam"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE I IS 123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE B IS TRUE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L < 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (L < 3.14 AND I > 3) OR B = True AND D > 0.0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE 8 % 3 = 2"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE 8 % 2 = 0"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE (I&1)=1 AND ~(I|2=I)")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 5 + (4&1) = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 5 + 4 & 1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 5 + 4 | 1 = 9")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 4|1&1 = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE (4|1)&1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 4^1 = 0")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 5^4 = 4")) << "not yet supported";

    //unary predicates
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT B"));
    //SQLite function which ECDb knows to return a bool
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE Glob('*amp*',S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT Glob('*amp*',S)"));
    //Int/Long types are supported as unary predicate. They evalute to True if they are not 0.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NOT Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (I IS NOT NULL) AND L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (I IS NOT NULL) AND NOT L"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 3.14"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE 'hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE S"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D.X >= P3D.X AND P2D.Y >= P3D.Y"));
    //with parentheses around
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (P2D.X) >= (P3D.X) AND (P2D.Y) >= (P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (P2D.X >= P3D.X) AND (P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE (P2D.X >= P3D.X AND P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE Random()"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE Hex(Bi)"));
    //unary operator
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE -I = -123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE I == 10"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE Garbage = 'bla'"));
    //NULL tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL IS NULL")); // NULL IS NULL is always true
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL = NULL")); // NULL = NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL <> NULL")); // NULL <> NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL IS 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL IS NOT 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B = NULL")); // = NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B <> NULL"));  // <> NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L IS NULL OR I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE L IS NULL AND I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE S IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE S IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE B IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE I IS ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE I IS NOT ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ? = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE ? <> NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE NULL <> ?"));
    //points
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D = ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D = ?"));
    //nav props
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA = L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA = MyPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA.Id = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("UPDATE ecsql.P SET I=10 WHERE MyPSA.RelECClassId IS NULL"));

    //  Unsupported literals
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE B = UNKNOWN")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE Dt = TIME '13:35:16'")) << "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE Dt = LOCALTIME")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P2D = POINT2D (-1.3, 45.134)")) << "Point literal not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("UPDATE ecsql.P SET I=10 WHERE P3D = POINT3D (-1.3, 45.134, 2)")) << "Point literal not yet supported";
    }


//********************* Delete **********************
struct ECSqlDeletePrepareTests : ECSqlPrepareTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, AndOrPrecedence)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TH3 WHERE S1 IS NOT NULL OR S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TH3 WHERE (S1 IS NOT NULL OR S2 IS NOT NULL)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TH3 WHERE S1 IS NULL AND S2 IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TH3 WHERE S1 IS NULL AND S2 IS NOT NULL OR 1=1"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TH3 WHERE S1 IS NULL AND (S2 IS NOT NULL OR 1=1)"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Casing)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY EcSqltEst.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ECSQLTEST.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsqltest.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ECSqlTest.p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ECSqlTest.P WHERE i<0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsqltest.p WHERE i<0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY Ecsql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecSql.P"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.p"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.P WHERE i<0"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, CommonGeometry)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PASpatial WHERE Geometry_Array IS NOT NULL"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SSpatial WHERE SpatialStructProp.Geometry IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SSpatial WHERE SpatialStructProp.Geometry IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SSpatial WHERE SpatialStructProp.Geometry_Array IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SSpatial WHERE SpatialStructProp.Geometry_Array IS NOT NULL"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, From)
    {
    //Delete classes with base classes
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.TH5"));

    // Delete abstract classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasMyMixin"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.PSAHasMyMixin"));
    // Delete mixins
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.MyMixin")) << "Mixins are invalid in DELETE statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.MyMixin")) << "Mixins are invalid in DELETE statements.";

    // Delete structs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.SAStruct")) << "Structs are invalid in DELETE statements.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.PStruct")) << "Structs are invalid in DELETE statements.";

    // Delete relationships
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.PSAHasP")) << "FK relationships are invalid in DELETE statements.";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.PSAHasPSA")) << "Link table relationships can be deleted.";

    // Deleting CAs
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY bsca.DateTimeInfo")) << "Custom Attributes classes are invalid in DELETE statements.";

    // Unmapped classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.PUnmapped")) << "Unmapped classes cannot be used in DELETE statements.";

    // Abstract classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.Abstract")) << "by contract non-polymorphic deletes on abstract classes are valid, but are a no-op";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.AbstractNoSubclasses"));

    // Subclasses of abstract class
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.Sub1"));

    // Unsupported classes
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY bsm.AnyClass")) << "Cannot delete from AnyClass";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY bsm.InstanceCount")) << "Cannot delete from InstanceCount class";

    // Missing schema alias / not existing ECClasses / not existing ECProperties
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY P SET I=123, L=100000")) << "Class name needs to be prefixed by schema alias.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.BlaBla SET I=123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY blabla.P SET I=123"));

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.P SET Garbage='bla', I=123")) << "One of the properties does not exist in the target class.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Functions)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECClassId <> 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECClassId = 145"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE LOWER(S) = UPPER(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE LOWER(UPPER(S)) = LOWER (S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE LOWER(I)=I")) << "lower/upper only make sense with strings, but no failure if used for other data types (like in SQLite)";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE UPPER(D)>0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE LOWER(S)=?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE UPPER(?) = 'hello'"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE InVirtualSet(?, ECInstanceId)"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId NOT MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE I MATCH random()")) << "fails at step time only";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (I + L) MATCH random()")) << "even though SQLite expects the LHS to be a column, we allow a value exp in the ECSQL grammar. Fails at step time only";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId MATCH '123'"));
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Misc)
    {
    // Syntactically incorrect statements 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare(""));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE ONLY ecsql.P"));

    // Class aliases
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.P t WHERE t.D > 0.0")) << "In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them during preparation";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.P t WHERE t.S = ?"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.P t WHERE t.L = 0xabcdef"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.P t WHERE t.L = 0xabcdefgh"));

    // Delete clause in which the class name and the properties name contain, start with or end with under bar
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql._UnderBar u WHERE u.ABC_ = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.[_UnderBar] u WHERE u.[ABC_] = ?"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Options)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS")) << "OPTIONS clause without options";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS 123")) << "An option must be a name";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt=")) << "option value is missing";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt myOpt")) << "duplicate options not allowed (even if they differ by case)";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myOpt=1 myopt")) << "duplicate options not allowed";
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt=1 myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt=1 myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt myotheropt=true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P ECSQLOPTIONS myopt myotheropt=true onemoreopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt myotheropt"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ECInstanceId=? ECSQLOPTIONS myopt=1 myotheropt"));
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Polymorphic)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.Abstract"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.AbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ONLY ecsql.AbstractNoSubclasses"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.AbstractTablePerHierarchy"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.AbstractTablePerHierarchy"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.THBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.THBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.TCBase"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ONLY ecsql.TCBase"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecdbf.FileInfo WHERE ECInstanceId=?")) << "Polymorphic delete not supported as subclass is mapped to existing table which means it is readonly";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Relationships)
    {
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE ECInstanceId = 123")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE ECInstanceId <> 123")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE TargetECInstanceId = 123")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123 AND TargetECInstanceId = 124")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE SourceECInstanceId = 123 AND TargetECInstanceId <> 124")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE SourceECClassId = 123 AND TargetECClassId = 124")) << "Cannot delete FK relationship - must delete via nav prop";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.PSAHasP WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124")) << "Cannot delete FK relationship - must delete via nav prop";

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE ECInstanceId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE ECInstanceId <>123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE TargetECInstanceId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE SourceECInstanceId =123 AND TargetECInstanceId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE SourceECInstanceId =123 AND TargetECInstanceId <> 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE SourceECClassId =123 AND TargetECClassId = 124"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPSA WHERE SourceECClassId = 123 + 1 AND TargetECClassId = 124"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE B = false"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE B = false AND D = 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE SourceECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE TargetECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE SourceECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE SourceECClassId =123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE TargetECClassId <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE TargetECClassId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE ECInstanceId = 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSAHasPWithPrimProps WHERE ECInstanceId > 123"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, Structs)
    {
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp<>?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp.i = 123 AND B = true"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.PSA WHERE PStructProp.i = 123 AND PStructProp.dt <> DATE '2010-10-10' AND B = true"));

    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp.PStructProp IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp.PStructProp = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp.PStructProp.i = 123 AND SAStructProp.PStructProp.dt <> DATE '2010-10-10'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp IS NULL")) << "Structs with struct array props are not supported in the where clause";

    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp IS NOT NULL")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp=?")) << "Structs with struct array props are not supported in the where clause";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.SA WHERE SAStructProp<>?")) << "Structs with struct array props are not supported in the where clause";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeletePrepareTests, WhereBasics)
    {
    //case insensitive tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B = NULL OR b = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE i>=:myParam"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE I IS 123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE B IS TRUE"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L < 3.14"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (L < 3.14 AND I > 3) OR B = True AND D > 0.0"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE 8 % 3 = 2"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE 8 % 2 = 0"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE (I&1)=1 AND ~(I|2=I)")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 5 + (4&1) = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 5 + 4 & 1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 5 + 4 | 1 = 9")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 4|1&1 = 5")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE (4|1)&1 = 1")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 4^1 = 0")) << "not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 5^4 = 4")) << "not yet supported";

    //unary predicates
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT True"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT B"));
    //SQLite function which ECDb knows to return a bool
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE Glob('*amp*',S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT Glob('*amp*',S)"));
    //Int/Long types are supported as unary predicate. They evalute to True if they are not 0.
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT I"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NOT Length(S)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (I IS NOT NULL) AND L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (I IS NOT NULL) AND NOT L"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 3.14"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE 'hello'"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE S"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P2D"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P3D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P2D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D.X >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D.Y >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D.Z >= -11.111"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D.X >= P3D.X AND P2D.Y >= P3D.Y"));
    //with parentheses around
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (P2D.X) >= (P3D.X) AND (P2D.Y) >= (P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (P2D.X >= P3D.X) AND (P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE (P2D.X >= P3D.X AND P2D.Y >= P3D.Y)"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE Random()"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE Hex(Bi)"));
    //unary operator
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE -I = -123"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE I == 10"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE Garbage = 'bla'"));
    //NULL tests
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL IS NULL")); // NULL IS NULL is always true
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL = NULL")); // NULL = NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL <> NULL")); // NULL <> NULL returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL IS 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL IS NOT 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL <> 123"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B = NULL")); // = NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B <> NULL"));  // <> NULL always returns NULL
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L IS NULL OR I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE L IS NULL AND I IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE S IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE S IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE B IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE I IS ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE I IS NOT ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ? = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE ? <> NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE NULL <> ?"));
    //points
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P2D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P2D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P2D = ?"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P3D = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P3D = D"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D IS NOT NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE P3D = ?"));
    //nav props
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE MyPSA = 11"));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE MyPSA = L"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA = MyPSA"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA = ?"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA.Id IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA.Id = NULL"));
    ASSERT_EQ(ECSqlStatus::Success, Prepare("DELETE FROM ecsql.P WHERE MyPSA.RelECClassId IS NULL"));

    //*******************************************************
    //  Unsupported literals
    //*******************************************************
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE B = UNKNOWN")) << "Boolean literal UNKNOWN (from SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE Dt = TIME '13:35:16'")) << "TIME literal (as specified in SQL-99) is not valid in ECSQL as it is not supported by ECObjects.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE Dt = LOCALTIME")) << "LOCALTIME function (as specified in SQL-99) is not valid in ECSQL as implicit time zone conversions will not be supported for now.";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P2D = POINT2D (-1.3, 45.134)")) << "Point literal not yet supported";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, Prepare("DELETE FROM ecsql.P WHERE P3D = POINT3D (-1.3, 45.134, 2)")) << "Point literal not yet supported";
    }

END_ECDBUNITTESTS_NAMESPACE