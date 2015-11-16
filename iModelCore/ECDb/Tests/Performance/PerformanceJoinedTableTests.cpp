/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceJoinedTableTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan         10/15
//=======================================================================================
struct PerformanceJoinedTableTests: ECDbTestFixture
    {
    private:
        static BeFileName s_seedFilePath;

    protected:
        static const int64_t s_firstInstanceId = INT64_C(1);
        static const int s_initialInstanceCount = 1000000;
        static const int s_opCount = 500000;


        static int DetermineECInstanceIdIncrement()
            {
            return s_initialInstanceCount / s_opCount;
            }
        static Utf8String GenerateTestValue()
            {
            Utf8String val; val.Sprintf("%d", DateTime::GetCurrentTimeUtc().GetDayOfYear()); return val;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                      Affan.Khan                  10/15
        //+---------------+---------------+---------------+---------------+---------------+------
        BentleyStatus SetupTestECDb(ECDbR ecdb)
            {
            Utf8String seedFileName;
            bool mustCreateSeed = false;
            if (s_seedFilePath.empty())
                {
                seedFileName.Sprintf("joinedTableperformance_seed_%d.ecdb", DateTime::GetCurrentTimeUtc().GetDayOfYear());

                BeFileName seedPath = ECDbTestUtility::BuildECDbPath(seedFileName.c_str());
                //if seed file exists on disk, we reuse it. This is risky if other tests happen to create file with same name
                //but we add the current day of the year to the file name, to make sure it would never be reused after 24h.
                if (seedPath.DoesPathExist())
                    s_seedFilePath = seedPath;
                else
                    mustCreateSeed = true;
                }

            if (mustCreateSeed)
                {
                auto const schema_joinedTable =
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                    "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                    "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                    "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                    "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                    "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
                    "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
                    "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
                    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                    "    <ECClass typeName='Foo' isDomainClass='False' isStruct='False' isCustomAttributeClass='False'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.01.00'>"
                    "                <MapStrategy>"
                    "                    <Strategy>SharedTable</Strategy>"
                    "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
                    "                    <Options>JoinedTableForSubclasses</Options>"
                    "                </MapStrategy>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='F1l' typeName='long'/>"
                    "        <ECProperty propertyName='F2s' typeName='string'/>"
                    "        <ECProperty propertyName='F3l' typeName='long'/>"
                    "        <ECProperty propertyName='F4s' typeName='string'/>"
                    "    </ECClass>"
                    "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                    "        <BaseClass>Foo</BaseClass>"
                    "        <ECProperty propertyName='G1l' typeName='long'/>"
                    "        <ECProperty propertyName='G2s' typeName='string'/>"
                    "        <ECProperty propertyName='G3l' typeName='long'/>"
                    "        <ECProperty propertyName='G4s' typeName='string'/>"

                    "    </ECClass>"
                    "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                    "        <BaseClass>Goo</BaseClass>"
                    "        <ECProperty propertyName='B1l' typeName='long'/>"
                    "        <ECProperty propertyName='B2s' typeName='string'/>"
                    "        <ECProperty propertyName='B3l' typeName='long'/>"
                    "        <ECProperty propertyName='B4s' typeName='string'/>"
                    "    </ECClass>"
                    "</ECSchema>";
                    
                {
                ECDbTestProject seedProject;
                ECSchemaPtr seedSchema;
                seedProject.Create(seedFileName.c_str());
                s_seedFilePath.AssignUtf8(seedProject.GetECDbPath());
                auto readContext = ECSchemaReadContext::CreateContext();
                ECSchema::ReadFromXmlString(seedSchema, schema_joinedTable, *readContext);
                if (seedSchema.IsNull())
                    return ERROR;

                auto importStatus = seedProject.GetECDb().Schemas().ImportECSchemas(readContext->GetCache());
                if (importStatus != BentleyStatus::SUCCESS)
                    return ERROR;
                }

                ECDb seed;
                if (BE_SQLITE_OK != seed.OpenBeSQLiteDb(s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
                    return ERROR;

                ECSqlStatement stmt;
                if (ECSqlStatus::Success != stmt.Prepare(seed, "INSERT INTO dgn.Boo(ECInstanceId, F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"))
                    return ERROR;

                for (int i = 0; i < s_initialInstanceCount; i++)
                    {
                    Utf8String txt = Utf8PrintfString("test_%d", i).c_str();
                    auto k = s_firstInstanceId + i;
                    if (ECSqlStatus::Success != stmt.BindId(1, ECInstanceId(k)))
                        return ERROR;

                    if (ECSqlStatus::Success != stmt.BindInt(2, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(3, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(4, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(5, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(6, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(7, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(8, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(9, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(10, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(11, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(12, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(13, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (stmt.Step() != BE_SQLITE_DONE)
                        return ERROR;

                    stmt.Reset();
                    stmt.ClearBindings();
                    }

                seed.SaveChanges();
                }

            return CloneECDb(ecdb, "joinedTableperformance.ecdb", s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
            }


        //---------------------------------------------------------------------------------------
        // @bsimethod                                      Affan.Khan                  10/15
        //+---------------+---------------+---------------+---------------+---------------+------
        void LogTiming(StopWatch& timer, Utf8CP testDescription, int actualOpCount)
            {
            ASSERT_EQ(s_opCount, actualOpCount) << "Unexpected actual op count";
            Utf8String totalTestDescr;
            totalTestDescr.Sprintf("%s [initial instance count: %d]", testDescription, s_initialInstanceCount);
            LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), totalTestDescr, actualOpCount);
            }
    };

int const PerformanceJoinedTableTests::s_opCount;

BeFileName PerformanceJoinedTableTests::s_seedFilePath;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, CreateSeedFile)
    {
    //separate out code that creates and populates the seed files, so that multiple runs of the actual
    //perf timings can be done without influence of the heavy work to create the seed file.
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Insert)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECSqlStatement booInsert;
    const int instanceIdIncrement = DetermineECInstanceIdIncrement();
    ASSERT_EQ(booInsert.Prepare(ecdb, "INSERT INTO dgn.Boo(ECInstanceId, F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"), ECSqlStatus::Success);
    StopWatch timer(true);

    for (int i = 0; i < s_opCount; i++)
        {
        
        ECInstanceId id(s_firstInstanceId + s_initialInstanceCount + i);
        Utf8String txt = Utf8PrintfString("test_%d", i).c_str();
        ASSERT_EQ(booInsert.BindId(1, id), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(2, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(3, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(4, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(5, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(6, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(7, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(8, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(9, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(10, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(11, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(12, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(13, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.Step(), BE_SQLITE_DONE);
        ASSERT_EQ(booInsert.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.ClearBindings(), ECSqlStatus::Success);
        }
    timer.Stop();
    LogTiming(timer, "ECSQL INSERT (JOINEDTABLE) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Update)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECSqlStatement booUpdate;
    ASSERT_EQ(booUpdate.Prepare(ecdb, "UPDATE dgn.Boo SET F1l = ?,F2s = ?,F3l = ?,F4s = ?,G1l = ?,G2s = ?,G3l = ?,G4s = ?,B1l = ?,B2s = ?,B3l = ?,B4s = ? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        Utf8String txt = Utf8PrintfString("test_updated_%d", i).c_str();
        auto updatedInt = i + 2;
        ASSERT_EQ(booUpdate.BindInt(1, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(2, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(3, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(4, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(5, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(6, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(7, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(8, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(9, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(10, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(11, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(12, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindId(13, id), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.Step(), BE_SQLITE_DONE);
        ASSERT_EQ(booUpdate.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.ClearBindings(), ECSqlStatus::Success);
        }

    timer.Stop();
    LogTiming(timer, "ECSQL UPDATE (JOINEDTABLE) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Select)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECSqlStatement booSelect;
    ASSERT_EQ(booSelect.Prepare(ecdb, "SELECT F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s FROM dgn.Boo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        booSelect.ClearBindings();
        booSelect.Reset();
        booSelect.BindId(1, id);
        if (booSelect.Step() == BE_SQLITE_ROW)
            {
            booSelect.GetValueInt(0);
            booSelect.GetValueText(1);
            booSelect.GetValueInt(2);
            booSelect.GetValueText(3);
            booSelect.GetValueInt(4);
            booSelect.GetValueText(5);
            booSelect.GetValueInt(6);
            booSelect.GetValueText(7);
            booSelect.GetValueInt(8);
            booSelect.GetValueText(9);
            booSelect.GetValueInt(10);
            booSelect.GetValueText(11);
            }
        else
            {
            FAIL() << "Must find row";
            }
        }

    timer.Stop();
    LogTiming(timer, "ECSQL SELECT (JOINEDTABLE) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Delete)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECSqlStatement booDelete;
    ASSERT_EQ(booDelete.Prepare(ecdb, "DELETE FROM dgn.Boo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (auto i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        booDelete.ClearBindings();
        booDelete.Reset();
        booDelete.BindId(1, id);
        if (booDelete.Step() != BE_SQLITE_DONE)
            {
            FAIL() << "Must delete row";
            }
        }

    timer.Stop();
    LogTiming(timer, "ECSQL DELETE (JOINEDTABLE) ", s_opCount);

    }


END_ECDBUNITTESTS_NAMESPACE
