/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DateTimeTestDataset.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/DateTime.h>
#include "DateTimeTestHelpers.h"

//=======================================================================================    
//! @bsiclass                                           Krischan.Eberle            10/12
//=======================================================================================    
struct DateTimeTestDataset
    {
private:
    //static class
    DateTimeTestDataset ();
    ~DateTimeTestDataset ();

    static BentleyStatus ComputeExpectedJD (DateTimeTestItem& testItem, uint64_t jdDateFraction);
    static BentleyStatus ComputeExpectedUnixMillisecs (DateTimeTestItem& testItem);
    static BentleyStatus ComputeExpectedUnixMillisecs (DateTimeTestItem& testItem, int64_t rawUnixMillisecs);
                
    static BentleyStatus ComputeExpectedLocalTimezoneOffset (int64_t& localTimezoneOffsetInHns, const DateTime& dateTime);

public:
    static void CreateBaseTestDataset (DateTimeTestItemList& testItemList, DateTime::Kind targetKind);
    static void CreateBeforeGregorianCalendarReformTestDataset (DateTimeTestItemList& testDataset);

    static void CreateAccuracyTestDataset (DateTimeTestItemList& testItemList);

    static void CreateBrimOfUnixEpochCETTestDataset (DateTimeTestItemList& testItemList);
    static void CreateBrimOfUnixEpochESTTestDataset (DateTimeTestItemList& testItemList);

    static void CreateDstUKTestDataset (DateTimeTestItemList& testItemList);
    static void CreateDstCETTestDataset (DateTimeTestItemList& testItemList);
    static void CreateDstUSESTTestDataset (DateTimeTestItemList& testItemList);
    };