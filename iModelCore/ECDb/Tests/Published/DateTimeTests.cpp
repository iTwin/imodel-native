/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle                      01/16
//---------------+---------------+---------------+---------------+---------------+-------
struct DateTimeTestFixture : ECDbTestFixture
    {
    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        void AssertDateTime(DateTimeCR expected, DateTimeCR actual, bool ignoreDateTimeInfo, Utf8CP assertMessage)
            {
            if (!ignoreDateTimeInfo)
                ASSERT_EQ(expected.GetInfo(), actual.GetInfo()) << assertMessage << " Expected DateTime: " << expected.ToString().c_str();

            ASSERT_EQ(expected.GetYear(), actual.GetYear()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetMonth(), actual.GetMonth()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetDay(), actual.GetDay()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetHour(), actual.GetHour()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetMinute(), actual.GetMinute()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetSecond(), actual.GetSecond()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            ASSERT_EQ(expected.GetMillisecond(), actual.GetMillisecond()) << assertMessage << "Expected DateTime: " << expected.ToString().c_str();
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        void PopulateTestInstanceWithNonArrayValues(IECInstancePtr& testInstance, bvector<Utf8String>& propertyAccessStringList, bvector<DateTime>& expectedDateTimeList,
                bvector<bool>& expectedMatchesDateTimeInfoList)
            {
            //primitive properties
            propertyAccessStringList.push_back("nodatetimeinfo");
            expectedDateTimeList.push_back(DateTime::GetCurrentTimeUtc());
            expectedMatchesDateTimeInfoList.push_back(false);

            propertyAccessStringList.push_back("emptydatetimeinfo");
            expectedDateTimeList.push_back(DateTime::GetCurrentTimeUtc());
            expectedMatchesDateTimeInfoList.push_back(false);

            propertyAccessStringList.push_back("utc");
            expectedDateTimeList.push_back(DateTime::GetCurrentTimeUtc());
            expectedMatchesDateTimeInfoList.push_back(true);

            propertyAccessStringList.push_back("unspecified");
            DateTime utc = DateTime::GetCurrentTimeUtc();
            DateTime dt(DateTime::Kind::Unspecified, utc.GetYear(), utc.GetMonth(), utc.GetDay(), utc.GetHour(), utc.GetMinute(), utc.GetSecond(), utc.GetMillisecond());
            expectedDateTimeList.push_back(dt);
            expectedMatchesDateTimeInfoList.push_back(true);

            propertyAccessStringList.push_back("dateonly");
            expectedDateTimeList.push_back(DateTime(2012, 10, 31));
            expectedMatchesDateTimeInfoList.push_back(true);

            //datetime props in struct member
            propertyAccessStringList.push_back("structwithdatetimes.nodatetimeinfo");
            expectedDateTimeList.push_back(DateTime::GetCurrentTimeUtc());
            expectedMatchesDateTimeInfoList.push_back(false);

            propertyAccessStringList.push_back("structwithdatetimes.utc");
            expectedDateTimeList.push_back(DateTime::GetCurrentTimeUtc());
            expectedMatchesDateTimeInfoList.push_back(true);

            propertyAccessStringList.push_back("structwithdatetimes.dateonly");
            expectedDateTimeList.push_back(DateTime(2013, 2, 22));
            expectedMatchesDateTimeInfoList.push_back(true);

            size_t testItemCount = propertyAccessStringList.size();
            for (size_t i = 0; i < testItemCount; i++)
                {
                const DateTime dateTime = expectedDateTimeList[i];
                ECValue value(dateTime);
                Utf8CP propAccessString = propertyAccessStringList[i].c_str();
                ECObjectsStatus stat = testInstance->SetValue(propAccessString, value);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue ('" << propAccessString << "', " << dateTime.ToString().c_str() << ")";
                }
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        void PopulateTestInstanceWithDateTimeArray (IECInstancePtr& testInstance, bvector<DateTime>& expectedDateTimeArrayElements, Utf8CP arrayPropertyName,
                DateTime::Info const& targetMetadata)
            {
            const uint32_t arraySize = 3;

            DateTime datetimeArray[arraySize];
            testInstance->AddArrayElements(arrayPropertyName, arraySize);

            for (uint32_t i = 0; i < arraySize; i++)
                {
                DateTime datetime;
                if (targetMetadata.GetComponent() == DateTime::Component::Date)
                    {
                    datetime = DateTime(static_cast <int16_t> (2000 + i), static_cast <uint8_t> (1 + i), static_cast <uint8_t> (10 + i));
                    }
                else
                    {
                    datetime = DateTime(targetMetadata.GetKind(), static_cast <int16_t> (2000 + i),
                                        static_cast <uint8_t> (1 + i),
                                        static_cast <uint8_t> (10 + i),
                                        static_cast <uint8_t> (12 + i),
                                        static_cast <uint8_t> (30 + i));
                    }

                expectedDateTimeArrayElements.push_back(datetime);

                ECValue arrayElementValue(datetime);
                ECObjectsStatus stat = testInstance->SetValue(arrayPropertyName, arrayElementValue, i);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue ('" << arrayPropertyName << "', " << arrayElementValue.ToString().c_str() << ", " << i << ") failed";
                }
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        void PopulateTestInstanceWithStructArray(IECInstancePtr& testInstance, bvector<IECInstancePtr>& expectedStructArrayElements, ECClassCP testClass)
            {
            Utf8CP structArrayPropName = "arrayofstructwithdatetimes";
            StructArrayECPropertyCP arrayProp = testClass->GetPropertyP(structArrayPropName)->GetAsStructArrayProperty();
            ECClassCP structType = &arrayProp->GetStructElementType();
            ASSERT_TRUE(structType != nullptr);

            const uint32_t arraySize = 3;

            testInstance->AddArrayElements(structArrayPropName, arraySize);
            for (uint32_t i = 0; i < arraySize; i++)
                {
                IECInstancePtr structInstance = structType->GetDefaultStandaloneEnabler()->CreateInstance();
                ECValue v;
                v.SetDateTime(DateTime::GetCurrentTimeUtc());
                ECObjectsStatus stat = structInstance->SetValue("nodatetimeinfo", v);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "StructInstance::SetValue failed";

                v.SetDateTime(DateTime::GetCurrentTimeUtc());
                stat = structInstance->SetValue("utc", v);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "StructInstance::SetValue failed";

                v.SetDateTime(DateTime(2013, 2, 22));
                stat = structInstance->SetValue("dateonly", v);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "StructInstance::SetValue failed";

                ECValue structValue;
                BentleyStatus bstat = structValue.SetStruct(structInstance.get());
                EXPECT_EQ(SUCCESS, bstat) << "ECValue::SetStruct failed";

                stat = testInstance->SetValue(structArrayPropName, structValue, i);
                EXPECT_EQ(ECObjectsStatus::Success, stat) << "ECInstance::SetValue (StructValue) failed";
                expectedStructArrayElements.push_back(structInstance);
                }
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        ECInstanceKey InsertTestInstance(bvector<Utf8String>& nonArrayPropertyAccessStringList, bvector<DateTime>& expectedNonArrayDateTimeList,
                                         bvector<bool>& expectedNonArrayDateTimeInfoMatchesList, bvector<DateTime>& expectedDateTimeArrayElements,
                                         bvector<IECInstancePtr>& expectedStructArrayElements, ECDbCR ecdb, ECClassCP testClass)
            {
            IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
            EXPECT_TRUE(testInstance.IsValid());

            PopulateTestInstanceWithNonArrayValues(testInstance, nonArrayPropertyAccessStringList, expectedNonArrayDateTimeList, expectedNonArrayDateTimeInfoMatchesList);
            PopulateTestInstanceWithDateTimeArray(testInstance, expectedDateTimeArrayElements, "utcarray", DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            PopulateTestInstanceWithStructArray(testInstance, expectedStructArrayElements, testClass);

            ECInstanceInserter inserter(ecdb, *testClass, nullptr);
            if (!inserter.IsValid())
                return ECInstanceKey();

            ECInstanceKey instanceKey;
            EXPECT_EQ(BE_SQLITE_OK, inserter.Insert(instanceKey, *testInstance)) << "Inserting ECInstance with DateTime values failed.";
            return instanceKey;
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DateTimeTestFixture, DifferingDateTimeInfos)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbdatetime.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ecsqltest.PSADateTime(nodatetimeinfo, emptydatetimeinfo, utc, unspecified, dateonly, structwithdatetimes.nodatetimeinfo, structwithdatetimes.utc, structwithdatetimes.dateonly) VALUES (?,?,?,?,?,?,?,?)"));
    DateTime utc = DateTime::GetCurrentTimeUtc();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(1, utc)) << "Property nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(2, utc)) << "Property emptydatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(3, utc)) << "Property utc";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindDateTime(4, utc)) << "Property unspecified";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(5, utc)) << "Property dateonly";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(6, utc)) << "Property structwithdatetimes.nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(7, utc)) << "Property structwithdatetimes.utc";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(8, utc)) << "Property structwithdatetimes.dateonly";
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    DateTime unspecified = DateTime(DateTime::Kind::Unspecified, 2016, 11, 28, 12, 13, 14);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(1, unspecified)) << "Property nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(2, unspecified)) << "Property emptydatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindDateTime(3, unspecified)) << "Property utc";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(4, unspecified)) << "Property unspecified";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(5, unspecified)) << "Property dateonly";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(6, unspecified)) << "Property structwithdatetimes.nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Error, statement.BindDateTime(7, unspecified)) << "Property structwithdatetimes.utc";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(8, unspecified)) << "Property structwithdatetimes.dateonly";
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    DateTime dateonly = DateTime(2016, 11, 28);
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(1, dateonly)) << "Property nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(2, dateonly)) << "Property emptydatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(3, dateonly)) << "Property utc";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(4, dateonly)) << "Property unspecified";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(5, dateonly)) << "Property dateonly";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(6, dateonly)) << "Property structwithdatetimes.nodatetimeinfo";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(7, dateonly)) << "Property structwithdatetimes.utc";
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDateTime(8, dateonly)) << "Property structwithdatetimes.dateonly";
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DateTimeTestFixture, ECSqlStatementGetValueDateTime)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbdatetime.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSADateTime");

    //test set up -> insert test data
    bvector<Utf8String> nonArrayPropertyAccessStringList;
    bvector<DateTime> expectedNonArrayDateTimeList;
    bvector<bool> expectedNonArrayDateTimeInfoMatchesList;
    bvector<DateTime> expectedArrayDateTimeElements;
    bvector<IECInstancePtr> expectedStructArrayElements;
    const ECInstanceKey ecInstanceKey = InsertTestInstance(nonArrayPropertyAccessStringList, expectedNonArrayDateTimeList, expectedNonArrayDateTimeInfoMatchesList, expectedArrayDateTimeElements, expectedStructArrayElements, m_ecdb, testClass);

    //actual test
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT nodatetimeinfo, emptydatetimeinfo, utc, unspecified, dateonly, structwithdatetimes.nodatetimeinfo, structwithdatetimes.utc, structwithdatetimes.dateonly, utcarray, arrayofstructwithdatetimes FROM ONLY ecsqltest.PSADateTime WHERE ECInstanceId=?"));
    statement.BindId(1, ecInstanceKey.GetInstanceId());

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Executing ECSQL '" << statement.GetECSql() << "' didn't return the expected row.";

    const size_t nonArrayDataCount = nonArrayPropertyAccessStringList.size();
    for (size_t i = 0; i < nonArrayDataCount; i++)
        {
        DateTimeCR expected = expectedNonArrayDateTimeList[i];
        DateTime actual = statement.GetValueDateTime((int) i);
        Utf8String assertMessage;
        assertMessage.Sprintf("Unexpected date time for property access string '%s'", nonArrayPropertyAccessStringList[i].c_str());
        AssertDateTime(expected, actual, !expectedNonArrayDateTimeInfoMatchesList[i], assertMessage.c_str());
        }


    //*** check array property
    const int expectedArraySize = (int) expectedArrayDateTimeElements.size();
    const int arrayPropertyIndex = (int) nonArrayDataCount;

    IECSqlValue const& arrayValue = statement.GetValue(arrayPropertyIndex);
    int actualArraySize = arrayValue.GetArrayLength();

    EXPECT_EQ(expectedArraySize, actualArraySize) << "Unexpected size of returned DateTime array";
    uint32_t expectedIndex = 0;
    for (IECSqlValue const& arrayElementValue : arrayValue.GetArrayIterable())
        {
        DateTimeCR expected = expectedArrayDateTimeElements[expectedIndex];
        DateTime actual = arrayElementValue.GetDateTime();
        AssertDateTime(expected, actual, false, "Unexpected date time for property 'utcarray'");
        expectedIndex++;
        }

    //*** check struct array property
    const size_t expectedStructArraySize = expectedStructArrayElements.size();
    const int structArrayPropertyIndex = arrayPropertyIndex + 1;

    IECSqlValue const& structArrayValue = statement.GetValue(structArrayPropertyIndex);
    size_t actualArrayElementCount = 0;
    for (IECSqlValue const& arrayElement : structArrayValue.GetArrayIterable())
        {
        IECInstancePtr expectedStructInstance = expectedStructArrayElements[actualArrayElementCount];
        ECValue v;
        expectedStructInstance->GetValue(v, "nodatetimeinfo");
        DateTime expected = v.GetDateTime();

        DateTime actual = arrayElement["nodatetimeinfo"].GetDateTime();
        Utf8String assertMessage;
        assertMessage.Sprintf("Unexpected date time for struct property 'nodatetimeinfo' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, true, assertMessage.c_str());

        expectedStructInstance->GetValue(v, "utc");
        expected = v.GetDateTime();
        actual = arrayElement["utc"].GetDateTime();

        assertMessage.Sprintf("Unexpected date time for struct property 'utc' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, false, assertMessage.c_str());

        expectedStructInstance->GetValue(v, "dateonly");
        expected = v.GetDateTime();
        actual = arrayElement["dateonly"].GetDateTime();
        assertMessage.Sprintf("Unexpected date time for struct property 'dateonly' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, false, assertMessage.c_str());

        actualArrayElementCount++;
        }

    ASSERT_EQ(expectedStructArraySize, actualArrayElementCount) << "Unexpected size of struct array";
    ASSERT_EQ(expectedStructArraySize, structArrayValue.GetArrayLength()) << "Unexpected size of struct array as returned from IECSqlArrayReader::GetArrayLength";
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "Only one row was expected to be returned from the test query " << statement.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DateTimeTestFixture, DateTimeStorageAccuracyTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbdatetime.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    //DateTime accuracy in ECDb(SQLite) is millisecs
    bvector<DateTime> testDateList;
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, 2012, 1, 1, 13, 43, 53, 124));
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, 2012, 1, 1, 13, 43, 53, 999));
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, 2012, 1, 4, 13, 43, 53, 555));
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, 2012, 1, 1, 13, 59, 59, 999));
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, -753, 1, 7, 15, 55, 13, 666));
    testDateList.push_back(DateTime(DateTime::Kind::Unspecified, -1000, 2, 28, 1, 4, 13, 555));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO stco.AAA(t) VALUES(?)"));

    bmap<ECInstanceId, DateTime> testDataset;
    for (DateTime const& testDate : testDateList)
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(1, testDate));
        ECInstanceKey instanceKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(instanceKey));
        stmt.Reset();
        stmt.ClearBindings();
        testDataset[instanceKey.GetInstanceId()] = testDate;
        }

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, t FROM stco.AAA"));
    size_t rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ++rowCount;
        const ECInstanceId instanceId = stmt.GetValueId<ECInstanceId>(0);
        const DateTime actualDateTime = stmt.GetValueDateTime(1);
        DateTime const& expectedDateTime = testDataset[instanceId];
        AssertDateTime(expectedDateTime, actualDateTime, false, "");
        }

    //only one instance was inserted, so row count is expected to be one.
    ASSERT_EQ(testDataset.size(), rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DateTimeTestFixture, TimeOfDay)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("TimeOfDay.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.01" alias="CoreCA"/>
            <ECEntityClass typeName="CalendarEntry" modifier="None">
                <ECProperty propertyName="Day" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>Date</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="StartTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="EndTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>

                <ECProperty propertyName="Stamp" typeName="dateTime"/>

            </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey calenderEntryKey, timestampOnOtherDayKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(calenderEntryKey, "INSERT INTO ts.CalendarEntry(Day,StartTime,EndTime,Stamp) VALUES(DATE '2018-11-09', TIME '08:00', TIME '08:30',TIMESTAMP '2000-01-01T08:00:00Z')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(timestampOnOtherDayKey, "INSERT INTO ts.CalendarEntry(Day,StartTime,EndTime,Stamp) VALUES(DATE '2018-11-09', TIME '08:00', TIME '08:30',TIMESTAMP '2018-11-01T08:00:00Z')"));

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.CalendarEntry WHERE StartTime = TIMESTAMP '2018-04-30T08:10:00'"));
    stmt.Finalize();
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.CalendarEntry WHERE StartTime = DATE '2018-04-30'"));
    stmt.Finalize();
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.CalendarEntry WHERE StartTime = Day"));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.CalendarEntry WHERE StartTime = Stamp ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(calenderEntryKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.CalendarEntry WHERE StartTime <= TIME '08:10:00' AND EndTime >= TIME '08:10:00' ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(calenderEntryKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(timestampOnOtherDayKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DateTimeTestFixture, CURRENT_XXX)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CurrentXXX.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CURRENT_DATE, CURRENT_TIMESTAMP, CURRENT_TIME FROM meta.ECSchemaDef LIMIT 1"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    DateTime currentDate = stmt.GetValueDateTime(0);
    ASSERT_TRUE(currentDate.IsValid());
    EXPECT_EQ(DateTime::Info::CreateForDate(), currentDate.GetInfo());
    EXPECT_GE(currentDate.GetYear(), 2018);

    DateTime currentTimeStamp = stmt.GetValueDateTime(1);
    ASSERT_TRUE(currentTimeStamp.IsValid());
    EXPECT_EQ(DateTime::Info::CreateForDateTime(DateTime::Kind::Utc), currentTimeStamp.GetInfo());
    EXPECT_GE(currentTimeStamp.GetYear(), 2018);

    DateTime currentTime = stmt.GetValueDateTime(2);
    ASSERT_TRUE(currentTime.IsValid());
    ASSERT_TRUE(currentTime.IsTimeOfDay());
    EXPECT_EQ(DateTime::Info::CreateForTimeOfDay(), currentTime.GetInfo());
    }
END_ECDBUNITTESTS_NAMESPACE
