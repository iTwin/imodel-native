/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct MappingTests : SchemaUpdateTestFixture
    {
    //! Base uses all shared columns, Sub has nothing of its own, so Sub rows have no overflow row.
    static SchemaUpdateBaseline const& GetOverflowBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_Overflow", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="B1" typeName="string" />
                    <ECProperty propertyName="B2" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub">
                    <BaseClass>Base</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Base uses 3 of 4 shared columns, A.A1 and B.Bx share the fourth.
    static SchemaUpdateBaseline const& GetRemapOverflowBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_RemapOverflow", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="B1" typeName="string" />
                    <ECProperty propertyName="B2" typeName="string" />
                    <ECProperty propertyName="B3" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="A">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="A1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Bx" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! BIS layout, joined table per direct subclass with shared columns on the subclasses.
    static SchemaUpdateBaseline const& GetJoinedTableBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_JoinedTable", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Element">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow><ApplyToSubclassesOnly>True</ApplyToSubclassesOnly></ShareColumns>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Geometric">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="G1" typeName="string" />
                    <ECProperty propertyName="G2" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Pipe">
                    <BaseClass>Geometric</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! TPH without shared columns, siblings with a same-named, same-typed property.
    static SchemaUpdateBaseline const& GetPhysicalColumnsBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_PhysicalColumns", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="A">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Like B1, but the siblings' Code differs in type.
    static SchemaUpdateBaseline const& GetPhysicalColumnsMixedTypeBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_PhysicalColumnsMixedType", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="A">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="Code" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Root with physical columns, subclasses share.
    static SchemaUpdateBaseline const& GetSubclassesShareBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_SubclassesShare", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Root">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow><ApplyToSubclassesOnly>True</ApplyToSubclassesOnly></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub">
                    <BaseClass>Root</BaseClass>
                    <ECProperty propertyName="Code" typeName="string" />
                    <ECProperty propertyName="Other" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! TPH with a small hierarchy to delete from.
    static SchemaUpdateBaseline const& GetDeletionBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_Deletion", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Leaf">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="LeafOnly" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Mid">
                    <BaseClass>Base</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName="MidLeaf">
                    <BaseClass>Mid</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName="AParent">
                    <BaseClass>Base</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName="ZChild">
                    <BaseClass>AParent</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName="ZParent">
                    <BaseClass>Base</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName="AChild">
                    <BaseClass>ZParent</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Classes with their own tables.
    static SchemaUpdateBaseline const& GetOwnTableBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_OwnTable", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Keep">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Gone">
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! A struct with 64 members under a hierarchy that allows 200 shared columns.
    static SchemaUpdateBaseline const& GetWideStructBaseline()
        {
        static Utf8String s_xml = [] ()
            {
            Utf8String members;
            for (int i = 0; i < 64; i++)
                members.append(Utf8PrintfString(R"xml(<ECProperty propertyName="M%02d" typeName="int" />)xml", i));
            return Utf8String(Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECStructClass typeName="Big">%s</ECStructClass>
                    <ECEntityClass typeName="Base">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>200</MaxSharedColumnsBeforeOverflow></ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub">
                        <BaseClass>Base</BaseClass>
                        <ECStructProperty propertyName="S" typeName="Big" />
                    </ECEntityClass>
                </ECSchema>)xml", members.c_str()));
            }();
        static SchemaUpdateBaseline s_baseline("MappingGap_WideStruct", s_xml.c_str());
        return s_baseline;
        }

    //! Ten siblings with ten properties each on two shared columns, so most go to overflow.
    static SchemaUpdateBaseline const& GetManySiblingsBaseline()
        {
        static Utf8String s_xml = [] ()
            {
            Utf8String classes;
            for (int c = 0; c < 10; c++)
                {
                Utf8String props;
                for (int p = 0; p < 10; p++)
                    props.append(Utf8PrintfString(R"xml(<ECProperty propertyName="C%02dP%02d" typeName="string" />)xml", c, p));
                classes.append(Utf8PrintfString(R"xml(<ECEntityClass typeName="C%02d"><BaseClass>Base</BaseClass>%s</ECEntityClass>)xml", c, props.c_str()));
                }
            return Utf8String(Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Base">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    %s
                </ECSchema>)xml", classes.c_str()));
            }();
        static SchemaUpdateBaseline s_baseline("MappingGap_ManySiblings", s_xml.c_str(), 2);
        return s_baseline;
        }

    //! PropertyMap IsNullable/IsUnique on a property that lands in a shared column.
    static SchemaUpdateBaseline const& GetConstrainedSharedColumnBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingGap_ConstrainedSharedColumn", R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Base">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><ApplyToSubclassesOnly>True</ApplyToSubclassesOnly></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Name" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Sub">
                    <BaseClass>Base</BaseClass>
                    <ECProperty propertyName="SourceId" typeName="string">
                        <ECCustomAttributes>
                            <PropertyMap xmlns="ECDbMap.02.00.00"><IsNullable>False</IsNullable><IsUnique>True</IsUnique></PropertyMap>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Expects the import to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId, SchemaVersionChange versionChange = SchemaVersionChange::Minor)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    //! Message of the last error issue of the last update, for failure output and the log.
    Utf8String LastIssue() const
        {
        for (auto it = GetLastUpdateIssues().rbegin(); it != GetLastUpdateIssues().rend(); ++it)
            if (it->severity == IssueSeverity::Error)
                return Utf8String(it->id.m_issueId).append(": ").append(it->message);
        return GetLastUpdateIssues().empty() ? Utf8String() : GetLastUpdateIssues().back().message;
        }

    //! For tests whose outcome is open: if the last update was rejected, acknowledges whatever
    //! issue ECDb reported so VerifyData can run. Logs the outcome either way.
    void LogOutcomeAndAcknowledgeRejection(BentleyStatus stat)
        {
        if (stat == SUCCESS)
            return;
        for (auto it = GetLastUpdateIssues().rbegin(); it != GetLastUpdateIssues().rend(); ++it)
            if (it->severity == IssueSeverity::Error)
                {
                ExpectRejectionReason(it->id.m_issueId);
                return;
                }
        }

    //! Row count of a physical table, -1 if the table does not exist.
    int CountRows(Utf8CP tableName) const
        {
        if (!m_ecdb.TableExists(tableName))
            return -1;
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, SqlPrintfString("SELECT COUNT(*) FROM [%s]", tableName)))
            return -1;
        return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt(0) : -1;
        }

    //! Table and column a property is mapped to, as "table.column", for the log.
    Utf8String Mapping(Utf8CP className, Utf8CP accessString) const
        {
        Column col = GetHelper().GetPropertyMapColumn(AccessString("TestSchema", className, accessString));
        if (!col.Exists())
            return "<not mapped>";
        return Utf8PrintfString("%s.%s", col.GetTableName().c_str(), col.GetName().c_str());
        }

    //! Result of a SELECT as JSON text, for the log.
    Utf8String Dump(Utf8CP ecsql) const { return GetHelper().ExecuteSelectECSql(ecsql).ToString(); }

    //! Runs a non-SELECT ECSQL and returns the step result; -1 if it did not prepare.
    int Exec(Utf8CP ecsql) const
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
            return -1;
        return (int) stmt.Step();
        }

    //! Number of instances of exactly the class that match the condition, -1 if the ECSQL did not prepare.
    int CountWhere(Utf8CP className, Utf8CP condition) const
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT COUNT(*) FROM ONLY ts.%s WHERE %s", className, condition).c_str()))
            return -1;
        return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt(0) : -1;
        }
    };

//***************************************************************************************
// Group A: rows without an overflow row
//***************************************************************************************

//---------------------------------------------------------------------------------------
// A1. The update adds Sub.P1. Base already holds the two shared columns, so P1 can only go to
// the overflow table. The baseline Sub rows had no overflow row. RESULT: the import inserts the
// missing overflow rows (3 rows right after UpdateSchema), so the UPDATE reaches all instances.
// The harness write probe in VerifyData is the control for new instances.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, UpdateNewOverflowPropertyOnPreExistingRows)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetOverflowBaseline()));
    const int subCount = CountWhere("Sub", "ECInstanceId IS NOT NULL");
    ASSERT_LT(0, subCount);

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sub", "<BaseClass>Base</BaseClass>", R"xml(<BaseClass>Base</BaseClass>
        <ECProperty propertyName="P1" typeName="string" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml)) << LastIssue();

    EXPECT_EQ((int) BE_SQLITE_DONE, Exec("UPDATE ts.Sub SET P1='updated'"));
    EXPECT_EQ(subCount, CountWhere("Sub", "P1='updated'")) << "A1: UPDATE of a new overflow property on pre-existing rows is lost.";

    m_ecdb.AbandonChanges();
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// A2. The same mechanism through the RemapManager. A1 moves up to Base. Its shared column is
// taken by B.Bx in a sibling, so Base.A1 is placed in overflow and the values of A have to
// move ps4 -> overflow. RESULT: overflow rows are inserted for all 9 instances before the data
// move, A1 survives. MovePropertyToOverflow in SchemaRemapTests covers the already-in-overflow case.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, RemapIntoOverflowForClassWithoutOverflowRow)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetRemapOverflowBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Base", R"xml(<ECProperty propertyName="B3" typeName="string" />)xml", R"xml(<ECProperty propertyName="B3" typeName="string" />
        <ECProperty propertyName="A1" typeName="string" />)xml");
    ReplaceInClass(schemaXml, "A", R"xml(<ECProperty propertyName="A1" typeName="string" />)xml", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml)) << LastIssue();

    VerifyData();
    }

//---------------------------------------------------------------------------------------
// A4. A1 in the BIS layout: joined table per direct subclass, shared columns on the subclasses
// only, overflow hanging off the joined table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, UpdateNewOverflowPropertyOnPreExistingRows_JoinedTable)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetJoinedTableBaseline()));
    const int pipeCount = CountWhere("Pipe", "ECInstanceId IS NOT NULL");
    ASSERT_LT(0, pipeCount);

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>Geometric</BaseClass>", R"xml(<BaseClass>Geometric</BaseClass>
        <ECProperty propertyName="Diameter" typeName="double" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml)) << LastIssue();

    EXPECT_EQ((int) BE_SQLITE_DONE, Exec("UPDATE ts.Pipe SET Diameter=2.5"));
    EXPECT_EQ(pipeCount, CountWhere("Pipe", "Diameter=2.5")) << "A4: same as A1 with a joined table.";

    m_ecdb.AbandonChanges();
    VerifyData();
    }

//***************************************************************************************
// Group B: hierarchies with physical columns
//***************************************************************************************

//---------------------------------------------------------------------------------------
// B1. TPH without ShareColumns. A.Code and B.Code are physical columns (the same column, reused
// by name because the types match). Code moves up to Base. The remap cleanup skips physical
// columns, so the old mappings stay. Unknown: rejected, a second column, or values kept.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, MovePropertyUpWithoutSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetPhysicalColumnsBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Base", R"xml(<ECProperty propertyName="Name" typeName="string" />)xml", R"xml(<ECProperty propertyName="Name" typeName="string" />
        <ECProperty propertyName="Code" typeName="string" />)xml");
    ReplaceInClass(schemaXml, "A", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", "");
    ReplaceInClass(schemaXml, "B", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", "");
    const BentleyStatus stat = UpdateSchema(schemaXml);
    LogOutcomeAndAcknowledgeRejection(stat);
    EXPECT_EQ(SUCCESS, stat) << "B1: moving a property up in a hierarchy without shared columns is rejected.";
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// B1b. Same as B1 but the siblings' Code differs in type, so they sit in two physical columns
// (ts_Base.Code and ts_Base.Code1). Base.Code string has to reconcile with B whose Code was int.
// RESULT: accepted. Base.Code is mapped to ts_Base.Code, so A keeps its strings, but B's ints in
// Code1 are never moved and B.Code reads NULL for every existing instance. Data loss on an accepted
// import; a rejection would be correct. BUG.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, DISABLED_MovePropertyUpWithoutSharedColumns_SiblingsDifferInType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetPhysicalColumnsMixedTypeBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Base", R"xml(<ECProperty propertyName="Name" typeName="string" />)xml", R"xml(<ECProperty propertyName="Name" typeName="string" />
        <ECProperty propertyName="Code" typeName="string" />)xml");
    ReplaceInClass(schemaXml, "A", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", "");
    ReplaceInClass(schemaXml, "B", R"xml(<ECProperty propertyName="Code" typeName="int" />)xml", "");
    const BentleyStatus stat = UpdateSchema(schemaXml, SchemaVersionChange::Major);
    LogOutcomeAndAcknowledgeRejection(stat);
    if (stat != SUCCESS)
        {
        VerifyData();
        return;
        }

    EXPECT_EQ(0, CountWhere("B", "Code IS NULL")) << "B1b BUG: import accepted, B's Code values (int, column Code1) were dropped instead of moved to Base.Code.";
    // B.Code was int and is now string: it has to read back as the int's text, or the import should have refused.
    VerifyData(ExpectedDataChanges().ConvertValuesOf("TestSchema:B", "Code", [] (Utf8StringCR v) { return Utf8String("\"") + v + "\""; }));
    }

//---------------------------------------------------------------------------------------
// B2. BIS layout: the root's own properties have physical columns and the subclasses share.
// Code moves from Sub (shared js column) up to Root (new physical column). Check the values follow.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, MovePropertyFromSharedSubclassToPhysicalRoot)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetSubclassesShareBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Root", R"xml(<ECProperty propertyName="Name" typeName="string" />)xml", R"xml(<ECProperty propertyName="Name" typeName="string" />
        <ECProperty propertyName="Code" typeName="string" />)xml");
    ReplaceInClass(schemaXml, "Sub", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml", "");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml)) << LastIssue();

    VerifyData();
    }

//***************************************************************************************
// Group C: what deletions leave behind
//***************************************************************************************

//---------------------------------------------------------------------------------------
// C3. PurgeUnusedColumns is on. Deleting Leaf leaves its physical column LeafOnly unused.
// RESULT: the column is never dropped, neither by the deleting import nor by a later import that
// adds a column. Two reasons in DbSchemaPersistenceManager: AlterTable returns before the DROP
// loop when columnsToAdd is empty, and UpdateTable only lists columns that are missing from the
// in-memory DbTable, while DeleteProperty leaves the DbColumn in ec_Column. So the option only
// drops columns that vanished from ec_Column by other means. Looks at the table on purpose.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, DISABLED_PurgeUnusedColumnsOnlyRunsWhenAColumnIsAdded)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetDeletionBaseline()));
    m_ecdb.GetECSqlConfig().SetPurgeUnusedColumns(true);
    ASSERT_TRUE(GetHelper().ColumnExists("ts_Base", "LeafOnly"));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Leaf");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major)) << LastIssue();
    const bool droppedOnDelete = !GetHelper().ColumnExists("ts_Base", "LeafOnly");
    EXPECT_TRUE(droppedOnDelete) << "C3 BUG: unused column survives a pure deletion although PurgeUnusedColumns is on (AlterTable early return).";
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Leaf"));

    // Adding a column makes AlterTable reach its DROP loop. VerifyData reopened the file, so set the flag again.
    m_ecdb.GetECSqlConfig().SetPurgeUnusedColumns(true);
    ReplaceInClass(schemaXml, "Base", R"xml(<ECProperty propertyName="Name" typeName="string" />)xml", R"xml(<ECProperty propertyName="Name" typeName="string" />
        <ECProperty propertyName="Added" typeName="string" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml)) << LastIssue();
    const bool droppedOnAdd = !GetHelper().ColumnExists("ts_Base", "LeafOnly");
    EXPECT_TRUE(droppedOnAdd) << "C3 BUG: still not dropped; the orphaned DbColumn stays in ec_Column so UpdateTable does not consider it deleted.";
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Leaf"));
    }

//---------------------------------------------------------------------------------------
// C4. Deleting a class mapped to its own table with rows. DeleteClass purges instances only for
// TablePerHierarchy. If the table and rows stay, re-adding a class with the same name shows them again.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, DeleteOwnTableClassWithRows)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetOwnTableBaseline()));
    const int goneCount = CountRows("ts_Gone");
    ASSERT_LT(0, goneCount);

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Gone");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major)) << LastIssue();
    const int rows = CountRows("ts_Gone");
    EXPECT_NE(goneCount, rows) << "C4: OwnTable class deleted, its instances stay in an orphaned table.";
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Gone"));

    // Re-add the class. If the rows survived they are visible again through the new class.
    ASSERT_EQ(SUCCESS, UpdateSchema(GetBaselineSchemaXml())) << LastIssue();
    const int back = CountWhere("Gone", "ECInstanceId IS NOT NULL");
    EXPECT_EQ(0, back) << "C4: rows of the deleted class are visible through the re-added class.";
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Gone"));
    }

//***************************************************************************************
// Group D: incoming vs. stored state in the same import
//***************************************************************************************

//---------------------------------------------------------------------------------------
// D5. Mid gets sealed in the same import that deletes its only subclass MidLeaf. UpdateClass
// checks the incoming class's derived classes, so this should pass. Recorded because the
// class-deletion check (0378) looks at the stored schema instead.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, SealClassWhoseSubclassIsDeletedInSameImport)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetDeletionBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "MidLeaf");
    ReplaceInSchema(schemaXml, R"xml(<ECEntityClass typeName="Mid">)xml", R"xml(<ECEntityClass typeName="Mid" modifier="Sealed">)xml");
    const BentleyStatus stat = UpdateSchema(schemaXml, SchemaVersionChange::Major);
    LogOutcomeAndAcknowledgeRejection(stat);
    EXPECT_EQ(SUCCESS, stat) << "D5: sealing uses the stored subclass list.";
    VerifyData(stat == SUCCESS ? ExpectedDataChanges().DeleteInstancesOf("TestSchema:MidLeaf") : ExpectedDataChanges());
    }

//---------------------------------------------------------------------------------------
// D5b. Parent and child deleted together. DeleteClass asks the stored schema for subclasses, so the
// parent still "has subclasses" and the import is rejected with ECDb_0378 although the child goes
// away in the same import. Order of names does not matter (both AParent>ZChild and ZParent>AChild
// are in the baseline; AParent is reported first). A connector has to delete the child in one import
// and the parent in the next; the second half of the test shows that this works.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, DeleteParentAndChildInSameImport)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetDeletionBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "ZChild");
    RemoveClass(schemaXml, "AParent");
    RemoveClass(schemaXml, "AChild");
    RemoveClass(schemaXml, "ZParent");
    ExpectRejected(schemaXml, "ECDb_0378", SchemaVersionChange::Major);

    // Workaround: children first, parents in a second import.
    Utf8String step1 = GetBaselineSchemaXml();
    RemoveClass(step1, "ZChild");
    RemoveClass(step1, "AChild");
    ASSERT_EQ(SUCCESS, UpdateSchema(step1, SchemaVersionChange::Major)) << LastIssue();
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:ZChild").DeleteInstancesOf("TestSchema:AChild"));

    RemoveClass(step1, "AParent");
    RemoveClass(step1, "ZParent");
    ASSERT_EQ(SUCCESS, UpdateSchema(step1, SchemaVersionChange::Major)) << LastIssue();
    VerifyData(ExpectedDataChanges().DeleteInstancesOf("TestSchema:ZChild").DeleteInstancesOf("TestSchema:AChild")
               .DeleteInstancesOf("TestSchema:AParent").DeleteInstancesOf("TestSchema:ZParent"));
    }

//***************************************************************************************
// Group F: column limits
//***************************************************************************************

//---------------------------------------------------------------------------------------
// F4. ClassMapColumnFactory treats 63 columns as the most a base table holds for one property.
// A struct with 64 members goes to overflow regardless of MaxSharedColumnsBeforeOverflow=200.
// No update; the harness fills and reads the struct, VerifyData checks the round trip.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, StructWithMoreThan63MembersAlwaysGoesToOverflow)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetWideStructBaseline()));
    EXPECT_STREQ("ts_Base_Overflow", GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "Sub", "S.M00")).GetTableName().c_str()) << "F4: expected the 64-member struct in overflow.";
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// F2. CheckForPerTableColumnLimit groups per class and table, so a shared overflow table can
// collect more columns than any single class uses. Records the per-class vs. per-table counts
// for a small case to make the gap visible; reaching the 2000 limit takes thousands of properties.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, PerTableColumnLimitIsCheckedPerClassNotPerTable)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetManySiblingsBaseline()));
    const int overflowCols = GetHelper().GetColumnCount("ts_Base_Overflow");

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(
        SELECT [cl].[Name], COUNT(*) FROM [ec_PropertyMap] [pm]
          JOIN [ec_Column] [co] ON [co].[Id]=[pm].[ColumnId]
          JOIN [ec_Table] [tb] ON [tb].[Id]=[co].[TableId]
          JOIN [ec_Class] [cl] ON [cl].[Id]=[pm].[ClassId]
         WHERE [tb].[Name]='ts_Base_Overflow' AND [co].[IsVirtual]=0
         GROUP BY [cl].[Id] ORDER BY COUNT(*) DESC LIMIT 1)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_LE(stmt.GetValueInt(1), overflowCols);
    stmt.Finalize();
    VerifyData();
    }

//***************************************************************************************
// Group G: PropertyMap on shared columns
//***************************************************************************************

//---------------------------------------------------------------------------------------
// G4'. IsNullable=False and IsUnique=True on a property in a shared column are only logged as
// warnings by ClassMapColumnFactory::AllocateSharedColumn. CONFIRMED: neither is enforced,
// duplicates and NULLs go in with BE_SQLITE_DONE. The schema author gets no error, only a log
// warning during import. Documented behaviour; the test pins it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, PropertyMapNotNullAndUniqueOnSharedColumnAreNotEnforced)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetConstrainedSharedColumnBaseline()));

    const int dup1 = Exec("INSERT INTO ts.Sub(Name,SourceId) VALUES('a','same')");
    const int dup2 = Exec("INSERT INTO ts.Sub(Name,SourceId) VALUES('b','same')");
    const int nul = Exec("INSERT INTO ts.Sub(Name) VALUES('c')");
    EXPECT_EQ((int) BE_SQLITE_DONE, dup1);
    EXPECT_EQ((int) BE_SQLITE_DONE, dup2) << "IsUnique on a shared column is not enforced; if this starts failing, ECDb began enforcing it.";
    EXPECT_EQ((int) BE_SQLITE_DONE, nul) << "IsNullable=False on a shared column is not enforced; if this starts failing, ECDb began enforcing it.";
    EXPECT_EQ(2, CountWhere("Sub", "SourceId='same'"));
    EXPECT_EQ(1, CountWhere("Sub", "SourceId IS NULL"));

    m_ecdb.AbandonChanges();
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// G5. A new property with PropertyMap ColumnName in a class with shared columns is rejected. The
// shared-column check (ECDb_0041) is never reached: the earlier general check fires first and
// reports ECDb_0079 "Only ECClasses with map strategy 'ExistingTable' may specify a column name".
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, PropertyMapColumnNameOnSharedColumnIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetSubclassesShareBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sub", R"xml(<ECProperty propertyName="Other" typeName="string" />)xml", R"xml(<ECProperty propertyName="Other" typeName="string" />
        <ECProperty propertyName="Tag" typeName="string">
            <ECCustomAttributes><PropertyMap xmlns="ECDbMap.02.00.00"><ColumnName>MyTag</ColumnName></PropertyMap></ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0079");
    }

//---------------------------------------------------------------------------------------
// G5. Same with Collation: here the shared-column specific check does fire (ECDb_0042).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingTests, PropertyMapCollationOnSharedColumnIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetSubclassesShareBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sub", R"xml(<ECProperty propertyName="Other" typeName="string" />)xml", R"xml(<ECProperty propertyName="Other" typeName="string" />
        <ECProperty propertyName="Tag" typeName="string">
            <ECCustomAttributes><PropertyMap xmlns="ECDbMap.02.00.00"><Collation>NoCase</Collation></PropertyMap></ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0042");
    }

END_ECDBUNITTESTS_NAMESPACE
