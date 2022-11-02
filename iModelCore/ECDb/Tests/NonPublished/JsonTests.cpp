/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonTests : public ECDbTestFixture {
    // Json test
};

TEST_F(JsonTests, json_tree) {
    auto test_schema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema 
            schemaName="test_schema"
            alias="ts"
            version="1.0.0" 
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="json_docs" >
            <ECProperty propertyName="doc" typeName="string" />
        </ECEntityClass>
    </ECSchema>)xml");
    auto test_data = R"({
        "planet": "mars",
        "gravity": "3.721 m/s²",
        "surface_area": "144800000 km²",
        "distance_from_son":"227900000 km",
        "radius" : "3389.5 km",
        "orbital_period" : "687 days",
        "moons": ["Phobos", "Deimos"]
    })";
    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", test_schema));
    if(true){
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "insert into ts.json_docs(doc) values (?)"));
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ECInstanceKey inserted_row_key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(inserted_row_key));
    }
  if("join") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select s.* from ts.json_docs, json1.json_tree(doc) s where s.key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM (SELECT [Id] ECInstanceId,73 ECClassId,[doc] FROM [main].[ts_json_docs]) [json_docs],json_tree([json_docs].[doc]) s WHERE s.key='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"gravity");
        ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
        ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
    if("sub query") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select * from (select * from json1.json_tree(?)) WHERE key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT [K0],[K1],[K2],[K3],[K4],[K5],[K6] FROM (SELECT key [K0],value [K1],type [K2],atom [K3],parent [K4],fullkey [K5],path [K6] FROM json_tree(:_ecdb_sqlparam_ix1_col1)) WHERE [K0]='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
       ASSERT_STREQ(stmt.GetValueText(0),"gravity");
       ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
       ASSERT_STREQ(stmt.GetValueText(2),"text");
       ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
       ASSERT_EQ(stmt.GetValueInt(4), 0);
       ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
       ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
    if("wild card select + alias") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select s.* from json1.json_tree(?) s where s.key='planet'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL, "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM json_tree(:_ecdb_sqlparam_ix1_col1) s WHERE s.key='planet'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key    value type atom id parent fullkey  path 
            ------ ----- ---- ---- -- ------ -------- ---- 
            planet mars  text mars 2  0      $.planet $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"planet");
        ASSERT_STREQ(stmt.GetValueText(1),"mars");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"mars");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.planet");
        ASSERT_STREQ(stmt.GetValueText(6),"$");

    }
    if("wild card select") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select * from json1.json_tree(?) s where s.key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM json_tree(:_ecdb_sqlparam_ix1_col1) s WHERE s.key='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_tree");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"gravity");
        ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
        ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
}
TEST_F(JsonTests, json_each) {
    auto test_schema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema 
            schemaName="test_schema"
            alias="ts"
            version="1.0.0" 
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="json_docs" >
            <ECProperty propertyName="doc" typeName="string" />
        </ECEntityClass>
    </ECSchema>)xml");
    auto test_data = R"({
        "planet": "mars",
        "gravity": "3.721 m/s²",
        "surface_area": "144800000 km²",
        "distance_from_son":"227900000 km",
        "radius" : "3389.5 km",
        "orbital_period" : "687 days",
        "moons": ["Phobos", "Deimos"]
    })";
    ASSERT_EQ(SUCCESS, SetupECDb("test.ecdb", test_schema));
    if(true){
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "insert into ts.json_docs(doc) values (?)"));
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ECInstanceKey inserted_row_key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(inserted_row_key));
    }
  if("join") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select s.* from ts.json_docs, json1.json_each(doc) s where s.key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM (SELECT [Id] ECInstanceId,73 ECClassId,[doc] FROM [main].[ts_json_docs]) [json_docs],json_each([json_docs].[doc]) s WHERE s.key='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"gravity");
        ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
        ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
    if("sub query") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select * from (select * from json1.json_each(?)) WHERE key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT [K0],[K1],[K2],[K3],[K4],[K5],[K6] FROM (SELECT key [K0],value [K1],type [K2],atom [K3],parent [K4],fullkey [K5],path [K6] FROM json_each(:_ecdb_sqlparam_ix1_col1)) WHERE [K0]='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
       ASSERT_STREQ(stmt.GetValueText(0),"gravity");
       ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
       ASSERT_STREQ(stmt.GetValueText(2),"text");
       ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
       ASSERT_EQ(stmt.GetValueInt(4), 0);
       ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
       ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
    if("wild card select + alias") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select s.* from json1.json_each(?) s where s.key='planet'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL, "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM json_each(:_ecdb_sqlparam_ix1_col1) s WHERE s.key='planet'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key    value type atom id parent fullkey  path 
            ------ ----- ---- ---- -- ------ -------- ---- 
            planet mars  text mars 2  0      $.planet $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"planet");
        ASSERT_STREQ(stmt.GetValueText(1),"mars");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"mars");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.planet");
        ASSERT_STREQ(stmt.GetValueText(6),"$");

    }
    if("wild card select") {
        ECSqlStatement stmt; 
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "select * from json1.json_each(?) s where s.key='gravity'"));
        auto nativeSQL = stmt.GetNativeSql();
        ASSERT_STREQ(nativeSQL,  "SELECT s.key,s.value,s.type,s.atom,s.parent,s.fullkey,s.path FROM json_each(:_ecdb_sqlparam_ix1_col1) s WHERE s.key='gravity'");
        stmt.BindText(1, test_data, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        auto nCol = 0;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "key");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString(ECSqlPropertyPath::FormatOptions::WithArrayIndex).c_str(), "key");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 1;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "value");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "value");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        nCol = 2;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "type");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "type");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);   

        nCol = 3;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "atom");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "atom");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);          

        nCol = 4;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "parent");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "parent");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_Integer);     

        nCol = 5;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "fullkey");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "fullkey");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);       

        nCol = 6;
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).IsValid());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsGeneratedProperty());
        ASSERT_FALSE(stmt.GetColumnInfo(nCol).IsSystemProperty());
        ASSERT_TRUE(stmt.GetColumnInfo(nCol).GetRootClass().IsValid());
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetRootClass().GetClass().GetFullName(), "json1:json_each");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetProperty()->GetName().c_str(), "path");
        ASSERT_STREQ(stmt.GetColumnInfo(nCol).GetPropertyPath().ToString().c_str(), "path");
        ASSERT_EQ(stmt.GetColumnInfo(nCol).GetDataType().GetPrimitiveType(), ECN::PrimitiveType::PRIMITIVETYPE_String);

        // test first row
        /*
            key     value      type atom       id parent fullkey   path 
            ------- ---------- ---- ---------- -- ------ --------- ---- 
            gravity 3.721 m/s² text 3.721 m/s² 4  (null) $.gravity $    
        */
        ASSERT_STREQ(stmt.GetValueText(0),"gravity");
        ASSERT_STREQ(stmt.GetValueText(1),"3.721 m/s²");
        ASSERT_STREQ(stmt.GetValueText(2),"text");
        ASSERT_STREQ(stmt.GetValueText(3),"3.721 m/s²");
        ASSERT_EQ   (stmt.GetValueInt(4), 0);
        ASSERT_STREQ(stmt.GetValueText(5),"$.gravity");
        ASSERT_STREQ(stmt.GetValueText(6),"$");
    }
}
END_ECDBUNITTESTS_NAMESPACE