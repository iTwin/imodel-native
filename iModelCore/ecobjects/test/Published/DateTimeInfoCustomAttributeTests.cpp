/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/DateTimeInfoCustomAttributeTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//********************** DateTimeInfoTestFixture base classes *************************************
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct DateTimeInfoTestFixture : ECTestFixture
    {
    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static void AssertDateTime(DateTimeCR expected, DateTimeCR actual, bool ignoreDateTimeInfo)
            {
            Utf8String expectedActualStr;
            expectedActualStr.Sprintf("Expected: %s - Actual: %s", expected.ToString().c_str(), actual.ToString().c_str());

            ASSERT_EQ(expected.GetYear(), actual.GetYear()) << expectedActualStr.c_str();
            ASSERT_EQ(expected.GetMonth(), actual.GetMonth()) << expectedActualStr.c_str();
            ASSERT_EQ(expected.GetDay(), actual.GetDay()) << expectedActualStr.c_str();

            if (actual.GetInfo().GetComponent() != DateTime::Component::Date &&
                expected.GetInfo().GetComponent() != DateTime::Component::Date)
                {

                if (!ignoreDateTimeInfo)
                    ASSERT_EQ(expected.GetInfo(), actual.GetInfo()) << expectedActualStr.c_str();

                ASSERT_EQ(expected.GetHour(), actual.GetHour()) << expectedActualStr.c_str();
                ASSERT_EQ(expected.GetMinute(), actual.GetMinute()) << expectedActualStr.c_str();
                ASSERT_EQ(expected.GetSecond(), actual.GetSecond()) << expectedActualStr.c_str();
                ASSERT_EQ(expected.GetMillisecond(), actual.GetMillisecond()) << expectedActualStr.c_str();
                }
            }


        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr DeserializeSchema(ECSchemaReadContextPtr& context, Utf8CP schemaXml)
            {
            EXPECT_FALSE(Utf8String::IsNullOrEmpty(schemaXml));

            context = ECSchemaReadContext::CreateContext();

            ECSchemaPtr schema;
            SchemaReadStatus stat = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
            EXPECT_EQ(SchemaReadStatus::Success, stat);
            EXPECT_TRUE(schema.IsValid());

            return schema;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchema(ECSchemaReadContextPtr& context)
            {
            Utf8CP testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<ECSchema schemaName=\"StandardClassesHelperTest\" alias=\"t\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                "   <ECSchemaReference name=\"CoreCustomAttributes\" version=\"01.00\" alias=\"CoreCA\" />"
                "   <ECEntityClass typeName=\"TestClass\">"
                "        <ECProperty propertyName=\"nodatetimeinfo\" typeName=\"dateTime\" />"
                "        <ECProperty propertyName=\"emptydatetimeinfo\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"utc\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Utc</DateTimeKind>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"unspecified\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Unspecified</DateTimeKind>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"local\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Local</DateTimeKind>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"garbagekind\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Garbage</DateTimeKind>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"dateonly\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeComponent>Date</DateTimeComponent>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"garbagecomponent\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeComponent>Garbage</DateTimeComponent>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName=\"garbagekindgarbagecomponent\" typeName=\"dateTime\" >"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Garbage</DateTimeKind>"
                "                   <DateTimeComponent>Garbage</DateTimeComponent>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECArrayProperty propertyName=\"dateTimeArrayProp\" typeName=\"dateTime\" minOccurs=\"0\" maxOccurs=\"unbounded\">"
                "           <ECCustomAttributes>"
                "               <DateTimeInfo xmlns=\"CoreCustomAttributes.01.00\">"
                "                   <DateTimeKind>Utc</DateTimeKind>"
                "               </DateTimeInfo>"
                "           </ECCustomAttributes>"
                "        </ECArrayProperty>"
                "        <ECProperty propertyName=\"intProp\" typeName=\"int\" />"
                "        <ECArrayProperty propertyName=\"intArrayProp\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                "    </ECEntityClass>"
                "</ECSchema>";

            return DeserializeSchema(context, testSchemaXml);
            }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct StandardCustomAttributeHelperTestFixture : public DateTimeInfoTestFixture
    {
    public:
        typedef bpair<Utf8String, DateTime::Info> ExpectedResultPerProperty;
        typedef bvector<ExpectedResultPerProperty> ExpectedResults;

    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static void Assert(ECPropertyCR dateTimeProperty, DateTime::Info const& expected)
            {
            DateTime::Info actual;
            const ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo(actual, dateTimeProperty);
            if (stat != ECObjectsStatus::Success)
                ASSERT_FALSE(expected.IsValid());

            ASSERT_EQ(expected, actual);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchema(ECSchemaReadContextPtr& context, ExpectedResults& expectedResults)
            {
            expectedResults.clear();
            expectedResults.push_back(ExpectedResultPerProperty("nodatetimeinfo", DateTime::Info()));
            expectedResults.push_back(ExpectedResultPerProperty("emptydatetimeinfo", DateTime::Info()));
            expectedResults.push_back(ExpectedResultPerProperty("utc", DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)));
            expectedResults.push_back(ExpectedResultPerProperty("unspecified", DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified)));
            expectedResults.push_back(ExpectedResultPerProperty("local", DateTime::Info::CreateForDateTime(DateTime::Kind::Local)));
            expectedResults.push_back(ExpectedResultPerProperty("garbagekind", DateTime::Info()));
            expectedResults.push_back(ExpectedResultPerProperty("dateonly", DateTime::Info::CreateForDate()));
            expectedResults.push_back(ExpectedResultPerProperty("garbagecomponent", DateTime::Info()));
            expectedResults.push_back(ExpectedResultPerProperty("garbagekindgarbagecomponent", DateTime::Info()));
            expectedResults.push_back(ExpectedResultPerProperty("dateTimeArrayProp", DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)));

            return DateTimeInfoTestFixture::CreateTestSchema(context);
            }


        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchemaNotReferencingBSCA(ECSchemaReadContextPtr& context)
            {
            Utf8CP testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<ECSchema schemaName=\"StandardClassesHelperTest\" nameSpacePrefix=\"t\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                "   <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"prop1\" typeName=\"dateTime\" />"
                "        <ECProperty propertyName=\"prop2\" typeName=\"int\" />"
                "    </ECClass>"
                "</ECSchema>";

            return DeserializeSchema(context, testSchemaXml);
            }

    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceGetSetDateTimeTestFixture : DateTimeInfoTestFixture
    {
    protected:
        

        static void AssertGetValue(IECInstancePtr instance, Utf8CP propertyName, DateTimeCR expectedDateTime, bool expectedMatch)
            {
                    {
                    ECValue value(expectedDateTime);
                    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue(propertyName, value)) << "AssertGetValue:Setup> Setting datetime value in ECInstance";
                    }

                    uint64_t expectedJdMsec = 0LL;
                    expectedDateTime.ToJulianDay(expectedJdMsec);
                    int64_t expectedTicks = DateTime::JulianDayToCommonEraMilliseconds(expectedJdMsec) * 10000;

                    ECValue value;
                    ECObjectsStatus stat = instance->GetValue(value, propertyName);
                    ASSERT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::GetValue";

                    DateTime actualDateTime = value.GetDateTime();

                    DateTime::Info actualMetadata;
                    int64_t actualTicks = value.GetDateTimeTicks(actualMetadata);

                    if (expectedMatch)
                        {
                        EXPECT_TRUE(expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned mismatching dateTime. Expected: " << expectedDateTime.ToString().c_str() << ". Actual: " << actualDateTime.ToString().c_str();
                        if (actualMetadata.IsValid())
                            {
                            EXPECT_TRUE(expectedDateTime.GetInfo() == actualMetadata);
                            }

                        EXPECT_EQ(expectedTicks, actualTicks);
                        }
                    else
                        {
                        EXPECT_FALSE(expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned matching dateTime. Expected: " << expectedDateTime.ToString().c_str() << ". Actual: " << actualDateTime.ToString().c_str();
                        if (actualMetadata.IsValid())
                            {
                            EXPECT_FALSE(expectedDateTime.GetInfo() == actualMetadata);
                            }

                        EXPECT_EQ(expectedTicks, actualTicks);
                        }
            }
    };

//********************** StandardCustomAttributeHelper Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeInfo)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context, expectedResults);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    for (ExpectedResultPerProperty const& result : expectedResults)
        {
        ECPropertyP prop = testClass->GetPropertyP(result.first.c_str());
        ASSERT_TRUE(prop != NULL);
        Assert(*prop, result.second);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeInfoInSchemaNotReferencingBSCA)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchemaNotReferencingBSCA(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP("prop1");
    DateTime::Info dti;
    const ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo(dti, *prop);
    EXPECT_TRUE(stat == ECObjectsStatus::Success && !dti.IsValid()) << "No DateTimeInfo CA expected on property that doesn't have the DateTimeInfo CA";
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeForNonDateTimeProperties)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context, expectedResults);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP("intProp");
    ASSERT_TRUE(prop != NULL);
    DISABLE_ASSERTS

    DateTime::Info dti;
    ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo(dti, *prop);
    ASSERT_NE(ECObjectsStatus::Success, stat);

    prop = testClass->GetPropertyP("intArrayProp");
    ASSERT_TRUE(prop != NULL);
    stat = StandardCustomAttributeHelper::GetDateTimeInfo(dti, *prop);
    ASSERT_NE(ECObjectsStatus::Success, stat);
    };

//********************** ECInstance SetValue / GetValue Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    auto assertSetValue = [] (IECInstancePtr instance, Utf8CP propertyName, DateTimeCR testDateTime, bool expectedToSucceed)
        {
        ECValue value;
        EXPECT_EQ(SUCCESS, value.SetDateTime(testDateTime)) << "Return value of ECValue::SetDateTime";
        if (expectedToSucceed)
            {
            ECObjectsStatus stat = instance->SetValue(propertyName, value);
            EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue with a matching DateTimeKind> Property: " << propertyName << " DateTime: " << testDateTime.ToString().c_str();
            }
        else
            {
            DISABLE_ASSERTS
                ECObjectsStatus stat = instance->SetValue(propertyName, value);
            EXPECT_NE(ECObjectsStatus::Success, stat) << "IECInstance::SetValue with a mismatching DateTimeKind> Property: " << propertyName << " DateTime: " << testDateTime.ToString().c_str();
            }
        };

    bvector<DateTime> testDateTimes;
    testDateTimes.push_back(DateTime::GetCurrentTimeUtc());
    testDateTimes.push_back(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22));
    testDateTimes.push_back(DateTime(2013, 2, 18));

    assertSetValue(instance, "nodatetimeinfo", DateTime::GetCurrentTimeUtc(), true);
    assertSetValue(instance, "nodatetimeinfo", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), true);
    assertSetValue(instance, "nodatetimeinfo", DateTime(2013, 2, 18), true);

    assertSetValue(instance, "emptydatetimeinfo", DateTime::GetCurrentTimeUtc(), true);
    assertSetValue(instance, "emptydatetimeinfo", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), true);
    assertSetValue(instance, "emptydatetimeinfo", DateTime(2013, 2, 18), true);

    assertSetValue(instance, "utc", DateTime::GetCurrentTimeUtc(), true);
    assertSetValue(instance, "utc", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), false);
    assertSetValue(instance, "utc", DateTime(2013, 2, 18), true);

    assertSetValue(instance, "unspecified", DateTime::GetCurrentTimeUtc(), false);
    assertSetValue(instance, "unspecified", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), true);
    assertSetValue(instance, "unspecified", DateTime(2013, 2, 18), true);

    assertSetValue(instance, "dateonly", DateTime::GetCurrentTimeUtc(), true);
    assertSetValue(instance, "dateonly", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), true);
    assertSetValue(instance, "dateonly", DateTime(2013, 2, 18), true);

    assertSetValue(instance, "garbagekind", DateTime::GetCurrentTimeUtc(), false);
    assertSetValue(instance, "garbagekind", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), false);
    assertSetValue(instance, "garbagekind", DateTime(2013, 2, 18), false);

    assertSetValue(instance, "garbagecomponent", DateTime::GetCurrentTimeUtc(), false);
    assertSetValue(instance, "garbagecomponent", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), false);
    assertSetValue(instance, "garbagecomponent", DateTime(2013, 2, 18), false);

    assertSetValue(instance, "garbagekindgarbagecomponent", DateTime::GetCurrentTimeUtc(), false);
    assertSetValue(instance, "garbagekindgarbagecomponent", DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22), false);
    assertSetValue(instance, "garbagekindgarbagecomponent", DateTime(2013, 2, 18), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeTicks)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    DateTime expectedDate = DateTime::GetCurrentTimeUtc();
    uint64_t jdMsec = 0LL;
    ASSERT_EQ(SUCCESS, expectedDate.ToJulianDay(jdMsec));

    ECValue ticksOnlyValue;
    ticksOnlyValue.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000);
    ECValue ticksWithUtc;
    ticksWithUtc.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));

    ECValue ticksWithDateOnly;
    ticksWithDateOnly.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000, DateTime::Info::CreateForDate());

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    ECObjectsStatus stat = instance->SetValue("nodatetimeinfo", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("emptydatetimeinfo", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> Empty DateTimeInfo - ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("unspecified", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("dateonly", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("garbagekind", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::ParseError, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks) is expected to fail because of invalid DateTimeInfo CA";

    stat = instance->SetValue("garbagekindgarbagecomponent", ticksOnlyValue);
    EXPECT_EQ(ECObjectsStatus::ParseError, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks) is expected to fail because of invalid DateTimeInfo CA";


    stat = instance->SetValue("nodatetimeinfo", ticksWithUtc);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks, Info): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("nodatetimeinfo", ticksWithDateOnly);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks, Info): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("utc", ticksWithUtc);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> DateTimeInfo (Utc)  - ECValue::SetDateTimeTicks (ticks, Utc): Expected to match with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("utc", ticksWithDateOnly);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> DateTimeInfo (Utc)  - ECValue::SetDateTimeTicks (ticks, Date): Expected to match with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("dateonly", ticksWithUtc);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> DateTimeInfo (Date)  - ECValue::SetDateTimeTicks (ticks, Utc): Expected to match with the DateTimeInfo custom attribute.";

    stat = instance->SetValue("dateonly", ticksWithDateOnly);
    EXPECT_EQ(ECObjectsStatus::Success, stat) << "IECInstance::SetValue> DateTimeInfo (Date)  - ECValue::SetDateTimeTicks (ticks, Date): Expected to match with the DateTimeInfo custom attribute.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeTicksGetAsDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);
    Utf8CP propertyName = "nodatetimeinfo";

    //test 1: Original date is UTC
    DateTime expectedDate = DateTime::GetCurrentTimeUtc();
    uint64_t jdMsec = 0LL;
    ASSERT_EQ(SUCCESS, expectedDate.ToJulianDay(jdMsec));

    ECValue v;
    BentleyStatus stat = v.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000);
    EXPECT_EQ(SUCCESS, stat);

    DateTime actualDate = v.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, true);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECObjectsStatus ecstat = instance->SetValue(propertyName, v);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    ECValue actualValue;
    ecstat = instance->GetValue(actualValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    actualDate = actualValue.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, true);

    //test 2: Original date is Unspecified
    expectedDate = DateTime(DateTime::Kind::Unspecified, 2013, 2, 2, 13, 14);
    jdMsec = 0LL;
    ASSERT_EQ(SUCCESS, expectedDate.ToJulianDay(jdMsec));

    v.Clear();
    stat = v.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000);
    EXPECT_EQ(SUCCESS, stat);

    actualDate = v.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, false);

    instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecstat = instance->SetValue(propertyName, v);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    actualValue.Clear();
    ecstat = instance->GetValue(actualValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    actualDate = actualValue.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, false);


    //test 3: Original date has component Date
    expectedDate = DateTime(2013, 2, 2);
    jdMsec = 0LL;
    ASSERT_EQ(SUCCESS, expectedDate.ToJulianDay(jdMsec));

    v.Clear();
    stat = v.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000);
    EXPECT_EQ(SUCCESS, stat);

    actualDate = v.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, true);

    instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecstat = instance->SetValue(propertyName, v);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    actualValue.Clear();
    ecstat = instance->GetValue(actualValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, ecstat);

    actualDate = actualValue.GetDateTime();
    EXPECT_EQ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo().GetKind());
    AssertDateTime(expectedDate, actualDate, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeWithLocalDateTimeKind)
    {
    bvector<DateTime> testDateTimes;
    testDateTimes.push_back(DateTime(DateTime::Kind::Utc, 2013, 2, 18, 14, 22));
    testDateTimes.push_back(DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22));
    testDateTimes.push_back(DateTime(DateTime::Kind::Local, 2013, 2, 18, 14, 22));
    testDateTimes.push_back(DateTime(2013, 2, 18));

    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);
    Utf8CP localDateTimePropName = "local";

    DISABLE_ASSERTS

        for (DateTimeCR testDateTime : testDateTimes)
            {
            const bool isLocal = testDateTime.GetInfo().GetKind() == DateTime::Kind::Local;
            ECValue value;
            const BentleyStatus expectedStat = isLocal ? ERROR : SUCCESS;
            EXPECT_EQ(expectedStat, value.SetDateTime(testDateTime)) << "Return value of ECValue::SetDateTime ('" << testDateTime.ToString().c_str() << "')";
            uint64_t jdMsec = 0LL;
            testDateTime.ToJulianDay(jdMsec);
            EXPECT_EQ(expectedStat, value.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000, testDateTime.GetInfo())) << "Return value of ECValue::SetDateTimeTicks () with '" << value.ToString().c_str();

            value = ECValue(testDateTime);
            if (!isLocal)
                {
                EXPECT_FALSE(value.IsNull()) << "ECValue (DateTime) is expected to return an ECValue which is not IsNull if the passed DateTime was not local.";
                EXPECT_TRUE(testDateTime == value.GetDateTime()) << "ECValue::GetDateTime () is expected to return a non-empty DateTime if the passed DateTime was not local.";
                }
            else
                {
                EXPECT_TRUE(value.IsNull()) << "ECValue (local DateTime) is expected to return an ECValue which is IsNull.";
                EXPECT_TRUE(DateTime() == value.GetDateTime()) << "ECValue (local DateTime)::GetDateTime () is expected to return an empty DateTime.";
                }

            IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
            ASSERT_TRUE(instance.IsValid());

            if (!isLocal)
                {
                EXPECT_NE(ECObjectsStatus::Success, instance->SetValue(localDateTimePropName, value)) << "IECInstance::SetValue> Property name: " << localDateTimePropName << " DateTime: " << testDateTime.ToString().c_str();
                }
            else
                {
                EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue(localDateTimePropName, value)) << "IECInstance::SetValue> Property name: " << localDateTimePropName << " with a local date time is expected to succeed, but the inserted value is null.";
                }
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, GetDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    DateTime expectedDateTime(DateTime::Kind::Utc, 2013, 2, 18, 14, 28, 34, 123);

    AssertGetValue(instance, "utc", expectedDateTime, true);
    AssertGetValue(instance, "nodatetimeinfo", expectedDateTime, false);
    AssertGetValue(instance, "emptydatetimeinfo", expectedDateTime, false);

    //Unspecified is the default kind, therefore any property that doesn't have a kind CA value (or an invalid value) return
    //a matching date time
    instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    expectedDateTime = DateTime(DateTime::Kind::Unspecified, 2013, 2, 18, 14, 28, 34, 123);

    AssertGetValue(instance, "unspecified", expectedDateTime, true);
    AssertGetValue(instance, "nodatetimeinfo", expectedDateTime, true);
    AssertGetValue(instance, "emptydatetimeinfo", expectedDateTime, true);

    //DateTime is the default component. Therefore any property that 
    //doesn't have a component CA value (or an invalid value) returns a mismatching date time
    instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    expectedDateTime = DateTime(2013, 2, 18);

    AssertGetValue(instance, "dateonly", expectedDateTime, true);
    AssertGetValue(instance, "nodatetimeinfo", expectedDateTime, false);
    AssertGetValue(instance, "emptydatetimeinfo", expectedDateTime, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, DateTimeArrayRoundtrip)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);
    Utf8CP propertyName = "dateTimeArrayProp";
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    const uint32_t expectedArraySize = 3;
    DateTime expected[expectedArraySize];
    expected[0] = DateTime::GetCurrentTimeUtc();
    expected[1] = DateTime(DateTime::Kind::Utc, 2012, 1, 1, 13, 14);
    expected[2] = DateTime(DateTime::Kind::Utc, 2011, 1, 1, 13, 14);

    instance->AddArrayElements(propertyName, expectedArraySize);
    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v(expected[i]);
        ECObjectsStatus stat = instance->SetValue(propertyName, v, i);
        EXPECT_EQ(ECObjectsStatus::Success, stat);
        }


    ECValue actualArrayInfoValue;
    ECObjectsStatus stat = instance->GetValue(actualArrayInfoValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    EXPECT_TRUE(actualArrayInfoValue.IsArray());
    ArrayInfo actualArrayInfo = actualArrayInfoValue.GetArrayInfo();
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == actualArrayInfo.GetElementPrimitiveType());
    EXPECT_EQ(expectedArraySize, actualArrayInfo.GetCount());

    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v;
        ECObjectsStatus stat = instance->GetValue(v, propertyName, i);
        EXPECT_EQ(ECObjectsStatus::Success, stat);

        DateTime actual = v.GetDateTime();
        AssertDateTime(expected[i], actual, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeArrayWithMismatchingArrayElements)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema(context);

    ECClassP testClass = testSchema->GetClassP("TestClass");
    ASSERT_TRUE(testClass != NULL);
    Utf8CP propertyName = "dateTimeArrayProp";
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    const uint32_t expectedArraySize = 3;
    DateTime expected[expectedArraySize];
    ECObjectsStatus expectedSuccess[expectedArraySize];
    expected[0] = DateTime::GetCurrentTimeUtc();
    expectedSuccess[0] = ECObjectsStatus::Success;
    expected[1] = DateTime(DateTime::Kind::Unspecified, 2012, 1, 1, 13, 14);
    expectedSuccess[1] = ECObjectsStatus::DataTypeMismatch;
    expected[2] = DateTime(2011, 1, 1);
    expectedSuccess[2] = ECObjectsStatus::Success;

    instance->AddArrayElements(propertyName, expectedArraySize);
    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v(expected[i]);
        ECObjectsStatus stat = instance->SetValue(propertyName, v, i);
        EXPECT_EQ(expectedSuccess[i], stat);
        }

    //retrieving the date time from the one array element that was inserted successfully
    ECValue actualArrayInfoValue;
    ECObjectsStatus stat = instance->GetValue(actualArrayInfoValue, propertyName);
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    EXPECT_TRUE(actualArrayInfoValue.IsArray());
    ArrayInfo actualArrayInfo = actualArrayInfoValue.GetArrayInfo();
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == actualArrayInfo.GetElementPrimitiveType());
    //array was initialized with the full size, although only one element was actually successfully inserted.
    EXPECT_EQ(expectedArraySize, actualArrayInfo.GetCount());

    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v;
        ECObjectsStatus stat = instance->GetValue(v, propertyName, i);
        EXPECT_EQ(ECObjectsStatus::Success, stat);

        if (expectedSuccess[i] == ECObjectsStatus::Success)
            {
            DateTime actual = v.GetDateTime();
            AssertDateTime(expected[i], actual, false);
            }
        else
            {
            EXPECT_TRUE(v.IsDateTime() && v.IsNull());
            }
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
