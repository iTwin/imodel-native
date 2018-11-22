/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/Nullable.h>
#include <Logging/bentleylogging.h>
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

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct ECDbIssue
    {
    private:
        Utf8String m_issue;

    public:
        explicit ECDbIssue(Utf8CP issue = nullptr) : m_issue(issue) {}

        bool IsIssue() const { return !m_issue.empty(); }
        Utf8CP GetMessage() const { return m_issue.c_str(); }
    };

//=======================================================================================
//!On construction registers itself with the ECDb, so that it starts listening right away.
//!On destruction unregisters itself. This allows for a fine-grained listening restricted
//!to the scope of the object
//!Example:
//! {
//! ECDbIssueListener issueListener(ecdb);
//! int val = statement.GetValueInt(1);
//! if (issueListener.GetIssue().IsIssue())
//!   return ERROR;
//! }
// @bsiclass                          Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    private:
        ECDbR m_ecdb;
        mutable ECDbIssue m_issue;
        void _OnIssueReported(Utf8CP message) const override { m_issue = ECDbIssue(message); }

    public:
        explicit ECDbIssueListener(ECDbR ecdb) : BeSQLite::EC::ECDb::IIssueListener(), m_ecdb(ecdb) { m_ecdb.AddIssueListener(*this); }
        ~ECDbIssueListener() { m_ecdb.RemoveIssueListener(); }

        //Can only be called once for a given issue. A second call will report whatever issue has occurred (or not) since the first call
        ECDbIssue GetIssue() const;

        void Reset() { m_issue = ECDbIssue(); }
    };

//=======================================================================================
//! Provides the logger for the ECDb ATP logging category
// @bsiclass                                                Krischan.Eberle      03/2014
//+===============+===============+===============+===============+===============+======
struct ECDbTestLogger
    {
    private:
        static BentleyApi::NativeLogging::ILogger* s_logger;

        ECDbTestLogger();
        ~ECDbTestLogger();

    public:
        static BentleyApi::NativeLogging::ILogger& Get();
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

