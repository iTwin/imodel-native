/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/Nullable.h>
#include <Bentley/Logging.h>
#include <ostream>

// Macros to test actual values which can be nullable against their non-nullable counterpart type
// Example:
// Nullable<int> actualHeight = ...;
// ASSERT_EQ_NULLABLE(10, actualHeight);
#define ASSERT_EQ_NULLABLE(expectedNonNullable,actualNullable)  ASSERT_TRUE((actualNullable) != nullptr && (expectedNonNullable) == (actualNullable).Value())
#define ASSERT_DOUBLE_EQ_NULLABLE(expectedNonNullable,actualNullable)  ASSERT_TRUE((actualNullable) != nullptr && TestUtilities::Equals((expectedNonNullable),(actualNullable).Value()))
#define EXPECT_EQ_NULLABLE(expectedNonNullable,actualNullable)  EXPECT_TRUE((actualNullable) != nullptr && (expectedNonNullable) == (actualNullable).Value())
#define EXPECT_DOUBLE_EQ_NULLABLE(expectedNonNullable,actualNullable)  EXPECT_TRUE((actualNullable) != nullptr && TestUtilities::Equals((expectedNonNullable),(actualNullable).Value()))

#define ASSERT_TRUE_NULLABLE(actualNullable)                    ASSERT_EQ_NULLABLE(true,actualNullable)
#define EXPECT_TRUE_NULLABLE(actualNullable)                    EXPECT_EQ_NULLABLE(true,actualNullable)
#define ASSERT_FALSE_NULLABLE(actualNullable)                   ASSERT_EQ_NULLABLE(false,actualNullable)
#define EXPECT_FALSE_NULLABLE(actualNullable)                   EXPECT_EQ_NULLABLE(false,actualNullable)

#define BEGIN_ECDBUNITTESTS_NAMESPACE BEGIN_BENTLEY_SQLITE_EC_NAMESPACE namespace Tests {
#define END_ECDBUNITTESTS_NAMESPACE } END_BENTLEY_SQLITE_EC_NAMESPACE
#define USING_ECDBUNITTESTS_NAMESPACE using namespace BentleyApi::BeSQLite::EC::Tests;
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// based on std::optional which won't be available until C++17 support
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbIssue
    {
    ECN::IssueSeverity severity;
    ECN::IssueCategory category;
    ECN::IssueType type;
    Utf8String message;

    ECDbIssue(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8String message): severity(severity), category(category), type(type), message(message) {}
    ECDbIssue(): severity(ECN::IssueSeverity::Info), category(ECN::IssueCategory::BusinessProperties), type("ECDbIssue"), message() {}

    bool has_value() const { return !message.empty(); }
    void reset() { message.clear(); }
    };

//=======================================================================================
//!On construction registers itself with the ECDb, so that it starts listening right away.
//!On destruction unregisters itself. This allows for a fine-grained listening restricted
//!to the scope of the object
//!Example:
//! {
//! ECIssueListener issueListener(ecdb);
//! int val = statement.GetValueInt(1);
//! if (issueListener.GetIssue().has_value())
//!   return ERROR;
//! }
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECIssueListener : ECN::IIssueListener
    {
    private:
        ECDbR m_ecdb;
        mutable ECDbIssue m_issue;
        void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override {m_issue = ECDbIssue(severity, category, type, message);}

    public:
        explicit ECIssueListener(ECDbR ecdb) : ECN::IIssueListener(), m_ecdb(ecdb) { m_ecdb.AddIssueListener(*this); }
        ~ECIssueListener() { m_ecdb.RemoveIssueListener(); }

        //Can only be called once for a given issue. A second call will report whatever issue has occurred (or not) since the first call
        ECDbIssue GetIssue() const;

        void Reset() { m_issue.reset(); }
    };

//=======================================================================================
//! Provides the logger for the ECDb ATP logging category
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbTestLogger
    {
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

