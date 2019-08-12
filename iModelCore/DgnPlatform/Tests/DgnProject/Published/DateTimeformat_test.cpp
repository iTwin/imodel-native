/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"

USING_NAMESPACE_BENTLEY_DGN
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define LOCS(str) str

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     02/2016
//=======================================================================================
struct  DateTimeFormatterTestData
    {
    WString         m_expectedString;
    DateTimeCR      m_inputValue;
    uint8_t         m_fractionalPrecision;
    bool            m_fractionalTrailingZeros;
    bool            m_convertToLocalTime;
    Utf8Char        m_dateSeparator;
    Utf8Char        m_timeSeparator;
    Utf8Char        m_decimalSeparator;
    bool            m_usePart;
    DateTimeFormatPart m_dateTimePart;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatDateTimeTest(DateTimeFormatterTestData & testData)
    {
#if defined (NOT_NOW)
    static int s_count = 0;
    printf ("%d\n", s_count++);
#endif

    DateTimeFormatterPtr   formatter = DateTimeFormatter::Create();

    formatter->SetFractionalSecondPrecision(testData.m_fractionalTrailingZeros);
    formatter->SetTrailingZeros(testData.m_fractionalTrailingZeros);
    formatter->SetConvertToLocalTime    (testData.m_convertToLocalTime);
    formatter->SetDateSeparator  (testData.m_dateSeparator);
    formatter->SetTimeSeparator  (testData.m_timeSeparator);
    formatter->SetDecimalSeparator(testData.m_decimalSeparator);
    if (testData.m_usePart)
        formatter->AppendFormatPart(testData.m_dateTimePart);

    Utf8String outputStr = formatter->ToString (testData.m_inputValue);
    Utf8String expectStr(testData.m_expectedString);
    ASSERT_TRUE (expectStr.Equals(outputStr));
    }

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     02/2016
//=======================================================================================
struct DateTimeFormatterTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DateTimeFormatterTest, TestGeneral)
    {
    DateTime testTime(DateTime::Kind::Utc, 2023, 9, 4, 5, 12, 45,143);
    DateTimeFormatterTestData testDataArray[] =
        {
        // expected                     value       precision   zeros   toLocal     
        { L"9/4/2023 5:12:45 AM",       testTime,   2,          true,    false,      '/',   ':',   '.'   , false},
        { L"9-4-2023 5:12:45 AM",       testTime,   2,          true,    false,      '-',   ':',   '.'   , false},
        { L"9 4 2023 5:12:45 AM",       testTime,   2,          true,    false,      ' ',   ':',   '.'   , false},
        { L"9-4-2023 5-12-45 AM",       testTime,   2,          true,    false,      '-',   '-',   '.'   , false},
        { L"9-4-2023 5-12-45 AM",       testTime,   2,          true,    false,      '-',   '-',   ' '   , false},
        { L"9-4-2023 5-12-45 AM",       testTime,   2,          true,    false,      '-',   '-',   ' '   , false}
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDateTimeTest(testDataArray[iTest]);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DateTimeFormatterTest, AM_PM)
    {
    DateTimeFormatterTestData testDataArray[] =
        {
        { L"9/4/2023 5:12:45 AM",          DateTime(DateTime::Kind::Utc, 2023, 9, 4, 5 , 12, 45)    , 1, true,   false, '/', ':', '.', false},
        { L"9/4/2023 5:12:45 PM",          DateTime(DateTime::Kind::Utc, 2023, 9, 4, 17, 12, 45)    , 1, true,   false, '/', ':', '.', false}
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDateTimeTest(testDataArray[iTest]);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat    02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DateTimeFormatterTest, IndividualParts)
    {
    DateTime testTime(DateTime::Kind::Utc, 2023, 8, 5, 5, 2, 7, 43); 
    DateTime testTime2(DateTime::Kind::Utc, 2023, 8, 15, 15, 11, 17,43);
    DateTimeFormatterTestData testDataArray[] =
        {
        //                                                                          Date     Time   Decimal
        // expected                     value       precision   zeros   toLocal              Separter         usepart   part
        { L"Saturday",      testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_DayOfWeek},
        { L"Sat",           testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_DoW},
        { L"5",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_D},
        { L"05",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_DD},
        { L"August",        testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Month},
        { L"Aug",           testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Mon},
        { L"8",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_M},
        { L"08",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_MM},
        { L"217",           testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_d},
        { L"217",           testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_ddd},
        { L"2023",          testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_YYYY},
        { L"23",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_YY},
        { L"5",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h},
        { L"05",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_hh},
        { L"3",             testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h},
        { L"03",            testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_hh},
        { L"5",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_H},
        { L"05",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_HH},
        { L"15",            testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_H},
        { L"15",            testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_HH},
        { L"2",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_m},
        { L"02",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_mm},
        { L"7",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_s},
        { L"07",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_ss},
        { L"0",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_FractionalSeconds},
        { L",",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Comma},
        { L"/",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_DateSeparator},
        { L":",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_TimeSeparator},
        { L".",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_DecimalSeparator},
        { L" ",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Space},
        { L"AM",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_AMPM},
        { L"PM",            testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_AMPM},
        { L"A",             testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_AP},
        { L"P",             testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_AP},

        { L"5:02 AM",               testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h_mm_AMPM},
        { L"3:11 PM",               testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h_mm_AMPM},
        { L"5:02:07 AM",            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h_mm_ss_AMPM},
        { L"3:11:17 PM",            testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_h_mm_ss_AMPM},

        { L"8/5/2023",                              testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_M_D_YYYY},
        { L"08/05/2023",                            testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_MM_DD_YYYY},
        { L"Saturday, 5 August, 2023",              testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Day_D_Month_YYYY},
        { L"Saturday, August 5, 2023",              testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Day_Month_D_YYYY},
        { L"Saturday, August 5, 2023, 5:02 AM",     testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Full},
        { L"Tuesday, August 15, 2023, 3:11 PM",     testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_Full},
        { L"8/5/2023 5:02:07 AM",                   testTime,   2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_General},
        { L"8/15/2023 3:11:17 PM",                  testTime2,  2,          true,    false,      '/',   ':',   '.'   , true,     DATETIME_PART_General}
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDateTimeTest(testDataArray[iTest]);
    }
