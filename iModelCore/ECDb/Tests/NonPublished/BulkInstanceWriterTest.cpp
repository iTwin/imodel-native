/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/BulkInstanceWriter.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct BulkInstanceWriterFixture : ECDbTestFixture {
    ECN::ECClassId GetClassId(Utf8CP schemaName, Utf8CP className) {
        auto ecClass = m_ecdb.Schemas().GetClass(schemaName, className);
        return ecClass == nullptr ? ECN::ECClassId() : ecClass->GetId();
    }

    //! Reads the full instance back through InstanceReader (the read side mirror).
    std::optional<BeJsDocument> ReadInstance(ECInstanceKeyCR key) {
        BeJsDocument doc;
        InstanceReader::Position pos(key.GetInstanceId(), key.GetClassId());
        if (!m_ecdb.GetInstanceReader().Seek(pos, [&](InstanceReader::IRowContext const& row, auto) {
                doc.From(row.GetJson(JsReadOptions().SetAbbreviateBlobs(false)));
            })) {
            return std::nullopt;
        }
        return doc;
    }

    //! Reads a single column straight out of SQLite, bypassing every ECDb layer.
    std::optional<Utf8String> ReadRawColumn(Utf8CP table, Utf8CP column, ECInstanceId id) {
        Utf8String sql;
        sql.Sprintf("SELECT [%s] FROM [%s] WHERE [Id]=?", column, table);
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, sql.c_str()))
            return std::nullopt;

        stmt.BindId(1, id);
        if (BE_SQLITE_ROW != stmt.Step())
            return std::nullopt;

        if (stmt.IsColumnNull(0))
            return std::nullopt;

        return Utf8String(stmt.GetValueText(0));
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, InsertAndUpdateSimpleClass) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_simple.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="D" typeName="double"/>
                <ECProperty propertyName="S" typeName="string"/>
                <ECProperty propertyName="B" typeName="boolean"/>
                <ECProperty propertyName="P2" typeName="point2d"/>
                <ECProperty propertyName="P3" typeName="point3d"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");
    ASSERT_TRUE(classId.IsValid());

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(123);
        ctx.FindBinder("D")->BindDouble(3.5);
        ctx.FindBinder("S")->BindText("hello", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("B")->BindBoolean(true);
        ctx.FindBinder("P2")->BindPoint2d(DPoint2d::From(1.0, 2.0));
        ctx.FindBinder("P3")->BindPoint3d(DPoint3d::From(1.0, 2.0, 3.0));
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();
    ASSERT_TRUE(key.IsValid());

    auto inserted = ReadInstance(key);
    ASSERT_TRUE(inserted.has_value());
    EXPECT_EQ(123, (*inserted)["I"].GetInt());
    EXPECT_DOUBLE_EQ(3.5, (*inserted)["D"].GetDouble());
    EXPECT_STREQ("hello", (*inserted)["S"].asCString());
    EXPECT_TRUE((*inserted)["B"].GetBoolean());
    EXPECT_DOUBLE_EQ(2.0, (*inserted)["P3"]["Y"].GetDouble());

    // partial update: only S is written, everything else must stay as it is
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("S")->BindText("world", IECSqlBinder::MakeCopy::Yes);
    })) << writer.GetLastError().c_str();

    auto updated = ReadInstance(key);
    ASSERT_TRUE(updated.has_value());
    EXPECT_STREQ("world", (*updated)["S"].asCString());
    EXPECT_EQ(123, (*updated)["I"].GetInt()) << "I was not bound and must keep its value";
    EXPECT_DOUBLE_EQ(3.5, (*updated)["D"].GetDouble()) << "D was not bound and must keep its value";
    EXPECT_TRUE((*updated)["B"].GetBoolean()) << "B was not bound and must keep its value";
    EXPECT_DOUBLE_EQ(3.0, (*updated)["P3"]["Z"].GetDouble()) << "P3 was not bound and must keep its value";
}

//---------------------------------------------------------------------------------------
//! Requesting a binder without calling any Bind* is documented to behave like BindNull().
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, UpdateWithExplicitNullVersusUntouched) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_null.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="int"/>
                <ECProperty propertyName="B" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindInt(1);
        ctx.FindBinder("B")->BindInt(2);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    // A is explicitly nulled, B is not touched at all
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindNull();
    })) << writer.GetLastError().c_str();

    auto updated = ReadInstance(key);
    ASSERT_TRUE(updated.has_value());
    EXPECT_TRUE((*updated)["A"].isNull() || !updated->isMember("A")) << "A was explicitly set to null";
    EXPECT_EQ(2, (*updated)["B"].GetInt()) << "B was untouched";

    // obtaining a binder without binding anything is equivalent to BindNull()
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("B");
    })) << writer.GetLastError().c_str();

    auto updated2 = ReadInstance(key);
    ASSERT_TRUE(updated2.has_value());
    EXPECT_TRUE((*updated2)["B"].isNull() || !updated2->isMember("B"));
}

//---------------------------------------------------------------------------------------
//! TablePerHierarchy produces joined and overflow tables. Every table of the class map must
//! get a row on insert, and the partial update must only touch the tables that are dirty.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, TablePerHierarchyWithJoinedTable) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_tph.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.04" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.04"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.04"/>
                </ECCustomAttributes>
                <ECProperty propertyName="BaseProp" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Sub">
                <BaseClass>Base</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.04">
                        <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="S1" typeName="string"/>
                <ECProperty propertyName="S2" typeName="int"/>
                <ECProperty propertyName="S3" typeName="double"/>
                <ECProperty propertyName="S4" typeName="long"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Sub");
    ASSERT_TRUE(classId.IsValid());

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("BaseProp")->BindText("base", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("S1")->BindText("one", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("S2")->BindInt(2);
        ctx.FindBinder("S3")->BindDouble(3.0);
        ctx.FindBinder("S4")->BindInt64(4);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    auto inserted = ReadInstance(key);
    ASSERT_TRUE(inserted.has_value()) << "InstanceReader must be able to seek the row in every table";
    EXPECT_STREQ("base", (*inserted)["BaseProp"].asCString());
    EXPECT_STREQ("one", (*inserted)["S1"].asCString());
    EXPECT_EQ(2, (*inserted)["S2"].GetInt());
    EXPECT_DOUBLE_EQ(3.0, (*inserted)["S3"].GetDouble());
    EXPECT_EQ(4, (*inserted)["S4"].GetInt64());

    // only update a property in the joined/overflow table
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("S4")->BindInt64(44);
    })) << writer.GetLastError().c_str();

    auto updated = ReadInstance(key);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(44, (*updated)["S4"].GetInt64());
    EXPECT_STREQ("base", (*updated)["BaseProp"].asCString()) << "the primary table must not be touched";
    EXPECT_STREQ("one", (*updated)["S1"].asCString());
    EXPECT_EQ(2, (*updated)["S2"].GetInt());
    EXPECT_DOUBLE_EQ(3.0, (*updated)["S3"].GetDouble());

    // only update a property in the primary table
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("BaseProp")->BindText("base2", IECSqlBinder::MakeCopy::Yes);
    })) << writer.GetLastError().c_str();

    auto updated2 = ReadInstance(key);
    ASSERT_TRUE(updated2.has_value());
    EXPECT_STREQ("base2", (*updated2)["BaseProp"].asCString());
    EXPECT_EQ(44, (*updated2)["S4"].GetInt64());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, StructsAndArrays) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_struct.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="Inner">
                <ECProperty propertyName="X" typeName="int"/>
                <ECProperty propertyName="Y" typeName="string"/>
            </ECStructClass>
            <ECStructClass typeName="Outer">
                <ECProperty propertyName="Name" typeName="string"/>
                <ECStructProperty propertyName="In" typeName="Inner"/>
            </ECStructClass>
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Tag" typeName="string"/>
                <ECStructProperty propertyName="St" typeName="Outer"/>
                <ECArrayProperty propertyName="Ints" typeName="int" minOccurs="0" maxOccurs="unbounded"/>
                <ECStructArrayProperty propertyName="Structs" typeName="Inner" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Tag")->BindText("tag", IECSqlBinder::MakeCopy::Yes);

        auto& st = *ctx.FindBinder("St");
        st["Name"].BindText("outer", IECSqlBinder::MakeCopy::Yes);
        st["In"]["X"].BindInt(7);
        st["In"]["Y"].BindText("inner", IECSqlBinder::MakeCopy::Yes);

        auto& ints = *ctx.FindBinder("Ints");
        ints.AddArrayElement().BindInt(10);
        ints.AddArrayElement().BindInt(20);
        ints.AddArrayElement().BindInt(30);

        auto& structs = *ctx.FindBinder("Structs");
        auto& e0 = structs.AddArrayElement();
        e0["X"].BindInt(1);
        e0["Y"].BindText("a", IECSqlBinder::MakeCopy::Yes);
        auto& e1 = structs.AddArrayElement();
        e1["X"].BindInt(2);
        e1["Y"].BindText("b", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    auto inserted = ReadInstance(key);
    ASSERT_TRUE(inserted.has_value());
    EXPECT_STREQ("outer", (*inserted)["St"]["Name"].asCString());
    EXPECT_EQ(7, (*inserted)["St"]["In"]["X"].GetInt());
    EXPECT_STREQ("inner", (*inserted)["St"]["In"]["Y"].asCString());
    ASSERT_EQ(3u, (*inserted)["Ints"].size());
    EXPECT_EQ(30, (*inserted)["Ints"][2].GetInt());
    ASSERT_EQ(2u, (*inserted)["Structs"].size());
    EXPECT_STREQ("b", (*inserted)["Structs"][1]["Y"].asCString());

    // partial update of the arrays only
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        auto& ints = *ctx.FindBinder("Ints");
        ints.AddArrayElement().BindInt(99);
    })) << writer.GetLastError().c_str();

    auto updated = ReadInstance(key);
    ASSERT_TRUE(updated.has_value());
    ASSERT_EQ(1u, (*updated)["Ints"].size());
    EXPECT_EQ(99, (*updated)["Ints"][0].GetInt());
    EXPECT_STREQ("tag", (*updated)["Tag"].asCString()) << "Tag was untouched";
    EXPECT_EQ(7, (*updated)["St"]["In"]["X"].GetInt()) << "the struct was untouched";
    ASSERT_EQ(2u, (*updated)["Structs"].size()) << "the struct array was untouched";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, NavigationProperty) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_nav.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.04" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECProperty propertyName="Name" typeName="string"/>
                <ECNavigationProperty propertyName="MyParent" relationshipName="ParentHasChildren" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentHasChildren" strength="Referencing" modifier="Sealed">
                <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false"><Class class="Parent"/></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="false"><Class class="Child"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "Parent"), [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("p", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), parentKey)) << writer.GetLastError().c_str();

    const auto relClassId = GetClassId("TestSchema", "ParentHasChildren");
    ECInstanceKey childKey;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "Child"), [&](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("c", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("MyParent")->BindNavigation(parentKey.GetInstanceId(), relClassId);
    }, BulkInstanceWriter::InsertOptions(), childKey)) << writer.GetLastError().c_str();

    auto inserted = ReadInstance(childKey);
    ASSERT_TRUE(inserted.has_value());
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), (*inserted)["MyParent"]["Id"].asCString());

    // updating Name must leave the navigation property alone
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(childKey, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("c2", IECSqlBinder::MakeCopy::Yes);
    })) << writer.GetLastError().c_str();

    auto updated = ReadInstance(childKey);
    ASSERT_TRUE(updated.has_value());
    EXPECT_STREQ("c2", (*updated)["Name"].asCString());
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), (*updated)["MyParent"]["Id"].asCString());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, LinkTableRelationship) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_linktable.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="AToB" strength="Referencing" modifier="Sealed">
                <ECProperty propertyName="Order" typeName="int"/>
                <Source multiplicity="(0..*)" roleLabel="a" polymorphic="false"><Class class="A"/></Source>
                <Target multiplicity="(0..*)" roleLabel="b" polymorphic="false"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();

    ECInstanceKey aKey, bKey;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "A"), [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("a", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), aKey)) << writer.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "B"), [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("b", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), bKey)) << writer.GetLastError().c_str();

    ECInstanceKey relKey;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "AToB"), [&](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("SourceECInstanceId")->BindId(aKey.GetInstanceId());
        ctx.FindBinder("TargetECInstanceId")->BindId(bKey.GetInstanceId());
        ctx.FindBinder("Order")->BindInt(1);
    }, BulkInstanceWriter::InsertOptions(), relKey)) << writer.GetLastError().c_str();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, [Order] FROM ts.AToB WHERE ECInstanceId=?"));
    stmt.BindId(1, relKey.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(aKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    EXPECT_EQ(bKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue());
    EXPECT_EQ(1, stmt.GetValueInt(2));
    stmt.Finalize();

    // partial update of the relationship's own property
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(relKey, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Order")->BindInt(2);
    })) << writer.GetLastError().c_str();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, [Order] FROM ts.AToB WHERE ECInstanceId=?"));
    stmt.BindId(1, relKey.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(aKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << "the source must not have been touched";
    EXPECT_EQ(2, stmt.GetValueInt(1));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, RejectsForeignKeyRelationship) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_fkrel.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Parent">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECNavigationProperty propertyName="MyParent" relationshipName="ParentHasChildren" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentHasChildren" strength="Referencing" modifier="Sealed">
                <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false"><Class class="Parent"/></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="false"><Class class="Child"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_ERROR, writer.Insert(GetClassId("TestSchema", "ParentHasChildren"),
                                             [](BulkInstanceWriter::IBindContext const&) {},
                                             BulkInstanceWriter::InsertOptions(), key));
    EXPECT_FALSE(writer.GetLastError().empty());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, ManualInstanceId) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_manualid.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    const ECInstanceId wanted((uint64_t)4242);
    BulkInstanceWriter::InsertOptions options;
    options.UseInstanceId(wanted);

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(1);
    }, options, key)) << writer.GetLastError().c_str();
    EXPECT_EQ(wanted.GetValue(), key.GetInstanceId().GetValue());

    // inserting the same id again must fail with a constraint violation
    EXPECT_NE(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(2);
    }, options, key));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, UpdateOfMissingInstance) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_missing.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const ECInstanceKey missing(GetClassId("TestSchema", "Foo"), ECInstanceId((uint64_t)9999));

    // by default a missing instance is not an error
    EXPECT_EQ(BE_SQLITE_DONE, writer.Update(missing, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(1);
    }));

    BulkInstanceWriter::UpdateOptions options;
    options.FailIfNoRowChanged(true);
    EXPECT_EQ(BE_SQLITE_NOTFOUND, writer.Update(missing, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(1);
    }, options));
}

//---------------------------------------------------------------------------------------
//! An update that binds nothing is a no-op and must not touch the database.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, EmptyUpdateIsNoop) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_noop.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "Foo"), [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("I")->BindInt(5);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    EXPECT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const&) {}));

    auto after = ReadInstance(key);
    ASSERT_TRUE(after.has_value());
    EXPECT_EQ(5, (*after)["I"].GetInt());
}

//---------------------------------------------------------------------------------------
//! Property enumeration and index based access must be stable and case insensitive.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, BindContextPropertyAccess) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_ctx.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Alpha" typeName="int"/>
                <ECProperty propertyName="Beta" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(GetClassId("TestSchema", "Foo"), [](BulkInstanceWriter::IBindContext const& ctx) {
        EXPECT_GE(ctx.GetPropertyCount(), 2);
        EXPECT_EQ(nullptr, ctx.FindBinder("DoesNotExist"));
        EXPECT_EQ(-1, ctx.GetPropertyIndex("DoesNotExist"));
        EXPECT_FALSE(ctx.Find("DoesNotExist").has_value());

        const auto alphaIx = ctx.GetPropertyIndex("alpha");
        ASSERT_GE(alphaIx, 0) << "lookup must be case insensitive";
        ASSERT_NE(nullptr, ctx.GetProperty(alphaIx));
        EXPECT_STRCASEEQ("Alpha", ctx.GetProperty(alphaIx)->GetName().c_str());
        ctx.GetBinder(alphaIx).BindInt(11);

        auto beta = ctx.Find("BETA");
        ASSERT_TRUE(beta.has_value());
        beta->GetBinder().BindText("b", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    auto inserted = ReadInstance(key);
    ASSERT_TRUE(inserted.has_value());
    EXPECT_EQ(11, (*inserted)["Alpha"].GetInt());
    EXPECT_STREQ("b", (*inserted)["Beta"].asCString());
}

//---------------------------------------------------------------------------------------
//! The cached statements refer to class maps, so they must be dropped on a cache clear.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, SurvivesSchemaImport) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_schemaimport.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, m_ecdb.GetBulkInstanceWriter().Insert(GetClassId("TestSchema", "Foo"),
        [](BulkInstanceWriter::IBindContext const& ctx) { ctx.FindBinder("I")->BindInt(1); },
        BulkInstanceWriter::InsertOptions(), key));
    m_ecdb.SaveChanges();

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Bar">
                <ECProperty propertyName="S" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    // the cache was cleared, both the old and the new class must still be writable
    ECInstanceKey key2;
    ASSERT_EQ(BE_SQLITE_DONE, m_ecdb.GetBulkInstanceWriter().Insert(GetClassId("TestSchema", "Foo"),
        [](BulkInstanceWriter::IBindContext const& ctx) { ctx.FindBinder("I")->BindInt(2); },
        BulkInstanceWriter::InsertOptions(), key2))
        << m_ecdb.GetBulkInstanceWriter().GetLastError().c_str();

    ECInstanceKey key3;
    ASSERT_EQ(BE_SQLITE_DONE, m_ecdb.GetBulkInstanceWriter().Insert(GetClassId("TestSchema2", "Bar"),
        [](BulkInstanceWriter::IBindContext const& ctx) { ctx.FindBinder("S")->BindText("s", IECSqlBinder::MakeCopy::Yes); },
        BulkInstanceWriter::InsertOptions(), key3))
        << m_ecdb.GetBulkInstanceWriter().GetLastError().c_str();

    auto read = ReadInstance(key2);
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ(2, (*read)["I"].GetInt());
}

//---------------------------------------------------------------------------------------
//! Bulk loop: the writer must be reusable for many rows without leaking bindings between rows.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, BulkInsertDoesNotLeakBindingsBetweenRows) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_bulk.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="S" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    std::vector<ECInstanceKey> keys;
    for (int i = 0; i < 100; ++i) {
        ECInstanceKey key;
        // only every other row binds S, the others must end up with S = NULL
        ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [i](BulkInstanceWriter::IBindContext const& ctx) {
            ctx.FindBinder("I")->BindInt(i);
            if (i % 2 == 0)
                ctx.FindBinder("S")->BindText("even", IECSqlBinder::MakeCopy::Yes);
        }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();
        keys.push_back(key);
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Foo WHERE S IS NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(50, stmt.GetValueInt(0)) << "bindings must be cleared between rows";
    stmt.Finalize();

    auto last = ReadInstance(keys.back());
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(99, (*last)["I"].GetInt());
}

//---------------------------------------------------------------------------------------
//! UPDATE statements are specialized per written property set. Alternating the set between
//! calls must keep producing correct partial updates.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, PartialUpdateWithAlternatingPropertySets) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_alternating.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="string"/>
                <ECProperty propertyName="B" typeName="string"/>
                <ECProperty propertyName="C" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");
    ASSERT_TRUE(classId.IsValid());

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a0", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("B")->BindText("b0", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("C")->BindText("c0", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    auto updateOne = [&](Utf8CP prop, Utf8CP value) {
        ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
            ctx.FindBinder(prop)->BindText(value, IECSqlBinder::MakeCopy::Yes);
        }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();
    };

    // every switch of the written set invalidates the guessed statement
    updateOne("A", "a1");
    updateOne("B", "b1");
    updateOne("A", "a2");
    updateOne("C", "c1");
    updateOne("C", "c2");

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a2", (*inst)["A"].asCString());
    EXPECT_STREQ("b1", (*inst)["B"].asCString());
    EXPECT_STREQ("c2", (*inst)["C"].asCString());

    // two properties at once is yet another specialization
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a3", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("C")->BindText("c3", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();

    inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a3", (*inst)["A"].asCString());
    EXPECT_STREQ("b1", (*inst)["B"].asCString());
    EXPECT_STREQ("c3", (*inst)["C"].asCString());
}

//---------------------------------------------------------------------------------------
//! The property index space a caller sees must not depend on which specialization happens
//! to back the context, and it must be identical in the discovery and the binding pass.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, PropertyIndexSpaceIsStableAcrossSpecializations) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_indexspace.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="string"/>
                <ECProperty propertyName="B" typeName="int"/>
                <ECProperty propertyName="C" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    std::vector<int> counts, indexOfB;
    std::vector<Utf8String> nameOfB;
    auto record = [&](BulkInstanceWriter::IBindContext const& ctx) {
        counts.push_back(ctx.GetPropertyCount());
        const auto ix = ctx.GetPropertyIndex("B");
        indexOfB.push_back(ix);
        auto prop = ctx.GetProperty(ix);
        nameOfB.push_back(prop == nullptr ? "" : prop->GetName());
    };

    // narrow specialization
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
        record(ctx);
        ctx.FindBinder("A")->BindText("a1", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();

    // wide specialization
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
        record(ctx);
        ctx.FindBinder("A")->BindText("a2", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("B")->BindInt(7);
        ctx.FindBinder("C")->BindDouble(1.5);
    }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();

    ASSERT_FALSE(counts.empty());
    for (size_t i = 1; i < counts.size(); ++i) {
        EXPECT_EQ(counts[0], counts[i]) << "property count changed between passes";
        EXPECT_EQ(indexOfB[0], indexOfB[i]) << "property index changed between passes";
        EXPECT_STREQ(nameOfB[0].c_str(), nameOfB[i].c_str()) << "property lookup changed between passes";
    }
    EXPECT_LE(0, indexOfB[0]);

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a2", (*inst)["A"].asCString());
    EXPECT_EQ(7, (*inst)["B"].GetInt());
    EXPECT_DOUBLE_EQ(1.5, (*inst)["C"].GetDouble());
}

//---------------------------------------------------------------------------------------
//! A repeated update of the same property set must run the callback only once per call.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, RepeatedUpdateInvokesCallbackOncePerCall) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_callbackcount.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="int"/>
                <ECProperty propertyName="B" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindInt(0);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    int invocations = 0;
    auto update = [&](int v) {
        ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
            ++invocations;
            ctx.FindBinder("A")->BindInt(v);
        }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();
    };

    update(1);
    const auto afterFirst = invocations;
    EXPECT_EQ(2, afterFirst) << "the first update of a class needs a discovery pass";

    for (int i = 0; i < 10; ++i)
        update(i + 2);

    EXPECT_EQ(afterFirst + 10, invocations) << "subsequent updates of the same property set must not re-run the callback";

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_EQ(11, (*inst)["A"].GetInt());
}

//---------------------------------------------------------------------------------------
//! A partial update of a wide class must leave every column it does not write untouched,
//! including columns in the overflow table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, PartialUpdateOfWideClassLeavesOtherColumnsUntouched) {
    const int kProps = 24;
    Utf8String xml = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
        <ECEntityClass typeName="Base" modifier="Abstract">
          <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00">
              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
            </ShareColumns>
          </ECCustomAttributes>
          <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="Wide">
          <BaseClass>Base</BaseClass>)xml";
    for (int p = 0; p < kProps; ++p)
        xml.append(SqlPrintfString("<ECProperty propertyName=\"P%d\" typeName=\"string\" />", p).GetUtf8CP());
    xml.append("</ECEntityClass></ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_wide_partial.ecdb", SchemaItem(xml)));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Wide");
    ASSERT_TRUE(classId.IsValid());

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [&](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("original", IECSqlBinder::MakeCopy::Yes);
        for (int p = 0; p < kProps; ++p)
            ctx.FindBinder(Utf8PrintfString("P%d", p).c_str())->BindText(Utf8PrintfString("v%d", p).c_str(), IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    // P0 lives in the shared columns, P20 in the overflow table
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("P0")->BindText("changed0", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("P20")->BindText("changed20", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("original", (*inst)["Name"].asCString());
    for (int p = 0; p < kProps; ++p) {
        Utf8String name = Utf8PrintfString("P%d", p);
        Utf8String expected = p == 0 ? "changed0" : (p == 20 ? "changed20" : Utf8PrintfString("v%d", p).c_str());
        EXPECT_STREQ(expected.c_str(), (*inst)[name.c_str()].asCString()) << "property " << name.c_str();
    }
}

//---------------------------------------------------------------------------------------
//! Many distinct property sets must not corrupt anything when the statement cache evicts.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, ManyPropertySetsEvictSpecializations) {
    const int kProps = 12;
    Utf8String xml = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="Foo">)xml";
    for (int p = 0; p < kProps; ++p)
        xml.append(SqlPrintfString("<ECProperty propertyName=\"P%d\" typeName=\"int\" />", p).GetUtf8CP());
    xml.append("</ECEntityClass></ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_evict.ecdb", SchemaItem(xml)));

    // a tiny cache so that every specialization evicts the previous one
    BulkInstanceWriter writer(m_ecdb, 2);
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        for (int p = 0; p < 12; ++p)
            ctx.GetBinder(ctx.GetPropertyIndex(Utf8PrintfString("P%d", p).c_str())).BindInt(0);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    // each round writes a different single property, forcing a new specialization every time
    for (int round = 0; round < 3; ++round) {
        for (int p = 0; p < kProps; ++p) {
            const int value = round * 100 + p + 1;
            ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
                ctx.FindBinder(Utf8PrintfString("P%d", p).c_str())->BindInt(value);
            }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();
        }
    }

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    for (int p = 0; p < kProps; ++p)
        EXPECT_EQ(200 + p + 1, (*inst)[Utf8PrintfString("P%d", p).c_str()].GetInt()) << "property P" << p;
}

//---------------------------------------------------------------------------------------
//! A full update writes every property, so properties the callback does not bind are nulled.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, FullUpdateNullsUnboundProperties) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_fullupdate.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="string"/>
                <ECProperty propertyName="B" typeName="int"/>
                <ECProperty propertyName="C" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    auto insert = [&](ECInstanceKey& key) {
        ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
            ctx.FindBinder("A")->BindText("a", IECSqlBinder::MakeCopy::Yes);
            ctx.FindBinder("B")->BindInt(1);
            ctx.FindBinder("C")->BindDouble(2.5);
        }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();
    };

    ECInstanceKey partialKey, fullKey;
    insert(partialKey);
    insert(fullKey);

    // partial: only A is written, B and C survive
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(partialKey, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a2", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();

    auto inst = ReadInstance(partialKey);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a2", (*inst)["A"].asCString());
    EXPECT_EQ(1, (*inst)["B"].GetInt());
    EXPECT_DOUBLE_EQ(2.5, (*inst)["C"].GetDouble());

    // full: only A is bound, so B and C are nulled
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(fullKey, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a3", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions().UseFullUpdate())) << writer.GetLastError().c_str();

    inst = ReadInstance(fullKey);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a3", (*inst)["A"].asCString());
    EXPECT_TRUE((*inst)["B"].isNull()) << "a full update must null the properties it does not bind";
    EXPECT_TRUE((*inst)["C"].isNull()) << "a full update must null the properties it does not bind";

    // a full update that binds everything round trips
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(fullKey, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindText("a4", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("B")->BindInt(9);
        ctx.FindBinder("C")->BindDouble(8.25);
    }, BulkInstanceWriter::UpdateOptions().UseFullUpdate())) << writer.GetLastError().c_str();

    inst = ReadInstance(fullKey);
    ASSERT_TRUE(inst.has_value());
    EXPECT_STREQ("a4", (*inst)["A"].asCString());
    EXPECT_EQ(9, (*inst)["B"].GetInt());
    EXPECT_DOUBLE_EQ(8.25, (*inst)["C"].GetDouble());
}

//---------------------------------------------------------------------------------------
//! A full update knows its property set up-front, so it never needs a discovery pass. It also
//! must not disturb the guess the partial updates of the same class rely on.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, FullUpdateAlwaysInvokesCallbackOnce) {
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_fullupdate_cb.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="A" typeName="int"/>
                <ECProperty propertyName="B" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Foo");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("A")->BindInt(0);
        ctx.FindBinder("B")->BindInt(0);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    int invocations = 0;
    auto fullUpdate = [&](int v) {
        ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
            ++invocations;
            ctx.FindBinder("A")->BindInt(v);
            ctx.FindBinder("B")->BindInt(v * 2);
        }, BulkInstanceWriter::UpdateOptions().UseFullUpdate())) << writer.GetLastError().c_str();
    };

    for (int i = 1; i <= 5; ++i)
        fullUpdate(i);

    EXPECT_EQ(5, invocations) << "a full update never needs a discovery pass";

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_EQ(5, (*inst)["A"].GetInt());
    EXPECT_EQ(10, (*inst)["B"].GetInt());

    // interleaving a full update must not cost the partial updates their guess
    int partialInvocations = 0;
    auto partialUpdate = [&](int v) {
        ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [&](BulkInstanceWriter::IBindContext const& ctx) {
            ++partialInvocations;
            ctx.FindBinder("A")->BindInt(v);
        }, BulkInstanceWriter::UpdateOptions())) << writer.GetLastError().c_str();
    };

    partialUpdate(100);
    const auto afterFirstPartial = partialInvocations;
    fullUpdate(7);
    partialUpdate(200);
    EXPECT_EQ(afterFirstPartial + 1, partialInvocations) << "the full update must not invalidate the partial update guess";

    inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_EQ(200, (*inst)["A"].GetInt());
    EXPECT_EQ(14, (*inst)["B"].GetInt()) << "B keeps the value the full update gave it";
}

//---------------------------------------------------------------------------------------
//! A full update must reach every table of a class that spans an overflow table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkInstanceWriterFixture, FullUpdateSpansOverflowTable) {
    const int kProps = 24;
    Utf8String xml = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
        <ECEntityClass typeName="Base" modifier="Abstract">
          <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00">
              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
            </ShareColumns>
          </ECCustomAttributes>
          <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="Wide">
          <BaseClass>Base</BaseClass>)xml";
    for (int p = 0; p < kProps; ++p)
        xml.append(SqlPrintfString("<ECProperty propertyName=\"P%d\" typeName=\"string\" />", p).GetUtf8CP());
    xml.append("</ECEntityClass></ECSchema>");
    ASSERT_EQ(SUCCESS, SetupECDb("bulkwriter_fullupdate_overflow.ecdb", SchemaItem(xml)));

    auto& writer = m_ecdb.GetBulkInstanceWriter();
    const auto classId = GetClassId("TestSchema", "Wide");

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, writer.Insert(classId, [&](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("Name")->BindText("original", IECSqlBinder::MakeCopy::Yes);
        for (int p = 0; p < kProps; ++p)
            ctx.FindBinder(Utf8PrintfString("P%d", p).c_str())->BindText(Utf8PrintfString("v%d", p).c_str(), IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::InsertOptions(), key)) << writer.GetLastError().c_str();

    // binds only P0 (shared columns) and P20 (overflow table), everything else must be nulled
    ASSERT_EQ(BE_SQLITE_DONE, writer.Update(key, [](BulkInstanceWriter::IBindContext const& ctx) {
        ctx.FindBinder("P0")->BindText("changed0", IECSqlBinder::MakeCopy::Yes);
        ctx.FindBinder("P20")->BindText("changed20", IECSqlBinder::MakeCopy::Yes);
    }, BulkInstanceWriter::UpdateOptions().UseFullUpdate())) << writer.GetLastError().c_str();

    auto inst = ReadInstance(key);
    ASSERT_TRUE(inst.has_value());
    EXPECT_TRUE((*inst)["Name"].isNull());
    for (int p = 0; p < kProps; ++p) {
        Utf8String name = Utf8PrintfString("P%d", p);
        if (p == 0)
            EXPECT_STREQ("changed0", (*inst)[name.c_str()].asCString());
        else if (p == 20)
            EXPECT_STREQ("changed20", (*inst)[name.c_str()].asCString());
        else
            EXPECT_TRUE((*inst)[name.c_str()].isNull()) << "property " << name.c_str() << " should have been nulled";
    }
}

END_ECDBUNITTESTS_NAMESPACE
