/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbDateTime_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle                      01/16
//---------------+---------------+---------------+---------------+---------------+-------
struct ECDbDateTimeTests : ECDbTestFixture
    {
    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Krischan.Eberle                  02/13
        //+---------------+---------------+---------------+---------------+---------------+------
        void AssertDateTime(DateTimeCR expected, DateTimeCR actual, bool ignoreDateTimeInfo, Utf8CP assertMessage)
            {
            //to compare expected and actual date times, compare the JD as comparing the DateTimes might not work because
            //of floating point arithmetics inaccuracies occurring when roundtripping a date time through ECDb
            double expectedJd = 0.0;
            expected.ToJulianDay(expectedJd);
            double actualJd = 0.0;
            actual.ToJulianDay(actualJd);

            ASSERT_DOUBLE_EQ(expectedJd, actualJd) << "Unexpected Julian Day. " << assertMessage;

            if (!ignoreDateTimeInfo)
                ASSERT_TRUE(expected.GetInfo() == actual.GetInfo()) << assertMessage << " Unexpected DateTime info. Expected: " << expected.ToUtf8String().c_str() << " Actual: " << actual.ToUtf8String().c_str();
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
            DateTime dt(DateTime::Kind::Unspecified, utc.GetYear(), utc.GetMonth(), utc.GetDay(), utc.GetHour(), utc.GetMinute(), utc.GetSecond(), utc.GetHectoNanosecond());
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
            ECClassCP structType = arrayProp->GetStructElementType();
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
            PopulateTestInstanceWithDateTimeArray(testInstance, expectedDateTimeArrayElements, "utcarray", DateTime::Info(DateTime::Kind::Utc, DateTime::Component::DateAndTime));
            PopulateTestInstanceWithStructArray(testInstance, expectedStructArrayElements, testClass);

            ECInstanceInserter inserter(ecdb, *testClass);
            if (!inserter.IsValid())
                return ECInstanceKey();

            ECInstanceKey instanceKey;
            EXPECT_EQ(SUCCESS, inserter.Insert(instanceKey, *testInstance)) << "Inserting ECInstance with DateTime values failed.";
            return instanceKey;
            }
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbDateTimeTests, InsertDateTimeValues)
    {
    ECDbCR ecdb = SetupECDb("ecdbdatetime.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    Utf8CP const testClassName = "PSADateTime";
    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", testClassName);

    bvector<Utf8String> nonArrayPropertyAccessStringList;
    bvector<DateTime> expectedNonArrayDateTimeList;
    bvector<bool> expectedNonArrayDateTimeInfoMatchesList;
    bvector<DateTime> expectedArrayDateTimeElements;
    bvector<IECInstancePtr> expectedStructArrayElements;
    InsertTestInstance(nonArrayPropertyAccessStringList, expectedNonArrayDateTimeList, expectedNonArrayDateTimeInfoMatchesList, expectedArrayDateTimeElements, expectedStructArrayElements, ecdb, testClass);
    //retrieving values and comparing them to original values is implicitly done in ECSqlStatementGetValueDateTime test

    //Insert local time which should fail as local time is not supported
    BeTest::SetFailOnAssert(false);
    {
    //option 1:
    ECValue value;
    BentleyStatus stat = value.SetDateTime(DateTime(DateTime::Kind::Local, 2012, 2, 18, 13, 14, 0));
    EXPECT_NE(SUCCESS, stat) << "ECValue::SetDateTime is expected to fail for a local DateTime.";

    //option 2:
    //when going via ctor, the value remains IsNull and no date time value is set
    value = ECValue(DateTime(DateTime::Kind::Local, 2012, 2, 18, 13, 14, 0));
    EXPECT_TRUE(value.IsNull()) << "ECValue::IsNull is expected to be true if passed a local time";
    EXPECT_TRUE(DateTime() == value.GetDateTime()) << "ECValue::GetDateTime is expected to return default date time if passed a local time"; ;
    }
    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbDateTimeTests, ECSqlStatementGetValueDateTime)
    {
    ECDbCR ecdb = SetupECDb("ecdbdatetime.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    Utf8CP const testClassName = "PSADateTime";
    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", testClassName);

    //test set up -> insert test data
    bvector<Utf8String> nonArrayPropertyAccessStringList;
    bvector<DateTime> expectedNonArrayDateTimeList;
    bvector<bool> expectedNonArrayDateTimeInfoMatchesList;
    bvector<DateTime> expectedArrayDateTimeElements;
    bvector<IECInstancePtr> expectedStructArrayElements;
    const ECInstanceKey ecInstanceKey = InsertTestInstance(nonArrayPropertyAccessStringList, expectedNonArrayDateTimeList, expectedNonArrayDateTimeInfoMatchesList, expectedArrayDateTimeElements, expectedStructArrayElements, ecdb, testClass);

    //actual test
    Utf8String ecsql("SELECT nodatetimeinfo, emptydatetimeinfo, utc, unspecified, dateonly, structwithdatetimes.nodatetimeinfo, structwithdatetimes.utc, structwithdatetimes.dateonly, utcarray, arrayofstructwithdatetimes FROM ONLY ");
    ecsql.append(testClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql.c_str())) << "Preparing ECSQL '" << ecsql.c_str() << "' failed.";
    statement.BindId(1, ecInstanceKey.GetECInstanceId());

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Executing ECSQL '" << ecsql.c_str() << "' didn't return the expected row.";

    const size_t nonArrayDataCount = nonArrayPropertyAccessStringList.size();
    for (size_t i = 0; i < nonArrayDataCount; i++)
        {
        DateTimeCR expected = expectedNonArrayDateTimeList[i];
        DateTime actual = statement.GetValueDateTime((int) i);
        Utf8String assertMessage;
        assertMessage.Sprintf("Unexpected date time for property access string '%s'", Utf8String(nonArrayPropertyAccessStringList[i].c_str()).c_str());
        AssertDateTime(expected, actual, !expectedNonArrayDateTimeInfoMatchesList[i], assertMessage.c_str());
        }


    //*** check array property
    const int expectedArraySize = (int) expectedArrayDateTimeElements.size();
    const int arrayPropertyIndex = (int) nonArrayDataCount;

    IECSqlArrayValue const& arrayValue = statement.GetValueArray(arrayPropertyIndex);
    int actualArraySize = arrayValue.GetArrayLength();

    EXPECT_EQ(expectedArraySize, actualArraySize) << "Unexpected size of returned DateTime array";
    uint32_t expectedIndex = 0;
    for (IECSqlValue const* arrayElementValue : arrayValue)
        {
        DateTimeCR expected = expectedArrayDateTimeElements[expectedIndex];
        DateTime actual = arrayElementValue->GetDateTime();
        AssertDateTime(expected, actual, false, "Unexpected date time for property 'utcarray'");
        expectedIndex++;
        }

    //*** check struct array property
    const size_t expectedStructArraySize = expectedStructArrayElements.size();
    const int structArrayPropertyIndex = arrayPropertyIndex + 1;

    IECSqlArrayValue const& structArrayValue = statement.GetValueArray(structArrayPropertyIndex);
    size_t actualArrayElementCount = 0;
    for (IECSqlValue const* arrayElement : structArrayValue)
        {
        IECSqlStructValue const& structArrayElement = arrayElement->GetStruct();

        IECInstancePtr expectedStructInstance = expectedStructArrayElements[actualArrayElementCount];
        ECValue v;
        expectedStructInstance->GetValue(v, "nodatetimeinfo");
        DateTime expected = v.GetDateTime();

        DateTime actual = structArrayElement.GetValue(0).GetDateTime();
        Utf8String assertMessage;
        assertMessage.Sprintf("Unexpected date time for struct property 'nodatetimeinfo' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, true, assertMessage.c_str());

        expectedStructInstance->GetValue(v, "utc");
        expected = v.GetDateTime();
        actual = structArrayElement.GetValue(1).GetDateTime();

        assertMessage.Sprintf("Unexpected date time for struct property 'utc' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, false, assertMessage.c_str());

        expectedStructInstance->GetValue(v, "dateonly");
        expected = v.GetDateTime();
        actual = structArrayElement.GetValue(2).GetDateTime();
        assertMessage.Sprintf("Unexpected date time for struct property 'dateonly' for array element %d.", actualArrayElementCount);
        AssertDateTime(expected, actual, false, assertMessage.c_str());

        actualArrayElementCount++;
        }

    ASSERT_EQ(expectedStructArraySize, actualArrayElementCount) << "Unexpected size of struct array";
    ASSERT_EQ(expectedStructArraySize, structArrayValue.GetArrayLength()) << "Unexpected size of struct array as returned from IECSqlArrayReader::GetArrayLength";
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "Only one row was expected to be returned from the test query " << ecsql.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbDateTimeTests, DateTimeStorageAccuracyTest)
    {
    ECDbCR ecdb = SetupECDb("ecdbdatetime.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    Utf8CP const testClassName = "AAA";
    Utf8CP const dateTimePropertyName = "t";

    ECClassCP testClass = ecdb.Schemas().GetECClass("StartupCompany", testClassName);
    ASSERT_TRUE(testClass != nullptr);

    bvector<ECValue> testDateList;
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, 2012, 10, 31, 13, 43, 53, 1237777)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, 2012, 10, 31, 13, 43, 53, 1239999)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, 2012, 10, 31, 13, 43, 53, 5550000)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, 1904, 2, 29, 1, 4, 13, 5555555)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, 213, 7, 14, 1, 4, 13, 5555555)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, -753, 7, 1, 15, 55, 13, 6666665)));
    testDateList.push_back(ECValue(DateTime(DateTime::Kind::Utc, -1000, 2, 28, 4, 4, 13, 5555555)));

    bmap<ECInstanceId, ECValue> testDataset;
    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_TRUE(inserter.IsValid());
    for (ECValueCR testECValue : testDateList)
        {
        IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

        testInstance->SetValue(dateTimePropertyName, testECValue);
        ECInstanceKey instanceKey;
        ASSERT_EQ(SUCCESS, inserter.Insert(instanceKey, *testInstance));
        testDataset[instanceKey.GetECInstanceId()] = testECValue;
        }

    Utf8String ecsql;
    ecsql.Sprintf("SELECT %s,%s FROM ONLY %s", ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY, dateTimePropertyName,
                  testClass->GetECSqlName().c_str());

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql.c_str()));

    size_t rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        ++rowCount;
        const ECInstanceId instanceId = statement.GetValueId<ECInstanceId>(0);
        const DateTime actualDateTime = statement.GetValueDateTime(1);
        double actualJd = 0.0;
        actualDateTime.ToJulianDay(actualJd);

        ECValue expectedValue = testDataset[instanceId];
        ECDbTestUtility::AssertECDateTime(expectedValue, ecdb, actualJd);
        }

    //only one instance was inserted, so row count is expected to be one.
    EXPECT_EQ(testDataset.size(), rowCount);
    }

END_ECDBUNITTESTS_NAMESPACE
