/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/Logging.h>
#include <Bentley/Nullable.h>
#include <ECDb/ECDbApi.h>

#include <ostream>

// Macros to test actual values which can be nullable against their non-nullable counterpart type
// Example:
// Nullable<int> actualHeight = ...;
// ASSERT_EQ_NULLABLE(10, actualHeight);
#define ASSERT_EQ_NULLABLE(expectedNonNullable, actualNullable) ASSERT_TRUE((actualNullable) != nullptr && (expectedNonNullable) == (actualNullable).Value())
#define ASSERT_DOUBLE_EQ_NULLABLE(expectedNonNullable, actualNullable) ASSERT_TRUE((actualNullable) != nullptr && TestUtilities::Equals((expectedNonNullable), (actualNullable).Value()))
#define EXPECT_EQ_NULLABLE(expectedNonNullable, actualNullable) EXPECT_TRUE((actualNullable) != nullptr && (expectedNonNullable) == (actualNullable).Value())
#define EXPECT_DOUBLE_EQ_NULLABLE(expectedNonNullable, actualNullable) EXPECT_TRUE((actualNullable) != nullptr && TestUtilities::Equals((expectedNonNullable), (actualNullable).Value()))

#define ASSERT_TRUE_NULLABLE(actualNullable) ASSERT_EQ_NULLABLE(true, actualNullable)
#define EXPECT_TRUE_NULLABLE(actualNullable) EXPECT_EQ_NULLABLE(true, actualNullable)
#define ASSERT_FALSE_NULLABLE(actualNullable) ASSERT_EQ_NULLABLE(false, actualNullable)
#define EXPECT_FALSE_NULLABLE(actualNullable) EXPECT_EQ_NULLABLE(false, actualNullable)

#define BEGIN_ECDBUNITTESTS_NAMESPACE BEGIN_BENTLEY_SQLITE_EC_NAMESPACE namespace Tests {
#define END_ECDBUNITTESTS_NAMESPACE \
    }                               \
    END_BENTLEY_SQLITE_EC_NAMESPACE
#define USING_ECDBUNITTESTS_NAMESPACE using namespace BentleyApi::BeSQLite::EC::Tests;
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Provides the logger for the ECDb ATP logging category
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbTestLogger {
   private:
    ECDbTestLogger();
    ~ECDbTestLogger();

   public:
    static NativeLogging::CategoryLogger Get();
};

#define LOG (ECDbTestLogger::Get())

Utf8String ToString(JsonECSqlSelectAdapter::FormatOptions const&);
Utf8String ToString(JsonUpdater::Options const&);

END_ECDBUNITTESTS_NAMESPACE

// GTest Format customizations for types not handled by GTest

BEGIN_BENTLEY_NAMESPACE

void PrintTo(BentleyStatus, std::ostream*);
void PrintTo(BeInt64Id, std::ostream*);
void PrintTo(DateTime const&, std::ostream*);
void PrintTo(DateTime::Info const&, std::ostream*);
void PrintTo(std::vector<Utf8CP> const&, std::ostream*);
void PrintTo(std::vector<Utf8String> const&, std::ostream*);

END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

void PrintTo(ECClassId, std::ostream*);
void PrintTo(ECValue const&, std::ostream*);

END_BENTLEY_ECOBJECT_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

void PrintTo(DbResult, std::ostream*);
void PrintTo(BeBriefcaseBasedId, std::ostream*);
void PrintTo(ProfileState, std::ostream*);

END_BENTLEY_SQLITE_NAMESPACE

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ENUM_TOSTRING(value) #value

void PrintTo(ECInstanceId, std::ostream*);
void PrintTo(ECInstanceKey const&, std::ostream*);
void PrintTo(ECSqlStatus, std::ostream*);
END_BENTLEY_SQLITE_EC_NAMESPACE
