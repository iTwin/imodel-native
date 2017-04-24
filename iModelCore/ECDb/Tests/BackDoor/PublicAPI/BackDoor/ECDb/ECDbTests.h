/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <json/json.h>

#define BEGIN_ECDBUNITTESTS_NAMESPACE BEGIN_BENTLEY_SQLITE_EC_NAMESPACE namespace Tests {
#define END_ECDBUNITTESTS_NAMESPACE } END_BENTLEY_SQLITE_EC_NAMESPACE

//#define ECSQLPREPAREDSTATEMENT_REFACTOR 1

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
        void _OnIssueReported(Utf8CP message) const override;

    public:
        explicit ECDbIssueListener(ECDbR ecdb) : BeSQLite::EC::ECDb::IIssueListener(), m_ecdb(ecdb) { m_ecdb.AddIssueListener(*this); }
        ~ECDbIssueListener() { m_ecdb.RemoveIssueListener(); }

        //Can only be called once for a given issue. A second call will report whatever issue has occurred (or not) since the first call
        ECDbIssue GetIssue() const;

        void Reset() { m_issue = ECDbIssue(); }
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestUtility
    {
    typedef void(*PopulatePrimitiveValueCallback)(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecproperty);

    private:
        static bool     CompareJsonWithECValue(const Json::Value& jsonValue, ECN::ECValueCR referenceValue, ECN::IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString);
        static bool     CompareJsonWithECPrimitiveValue(const Json::Value& jsonValue, ECN::ECValueCR referenceValue);
        static bool     CompareJsonWithECArrayValue(const Json::Value& jsonValue, ECN::ECValueCR referenceValue, ECN::IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString);
        static bool     CompareJsonWithECStructValue(const Json::Value& jsonValue, ECN::ECValueCR referenceValue);

        static void     PopulateStructValue(ECN::ECValueR value, ECN::ECClassCR structType, PopulatePrimitiveValueCallback callback);
        static void     PopulatePrimitiveValue(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);
        static void     GenerateRandomValue(ECN::ECValueR value, ECN::PrimitiveType type, ECN::ECPropertyCP ecproperty = nullptr);

        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, Utf8CP baseAccessPath);
        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR target, ECN::IECInstanceCR structValue, Utf8CP propertyName);

    public:
        static bool     CompareECInstances(ECN::IECInstanceCR a, ECN::IECInstanceCR b);
        static int64_t  ReadCellValueAsInt64(BeSQLite::DbR db, Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause);
        static bool     CompareJsonWithECInstance(const Json::Value& json, ECN::IECInstanceCR referenceInstance);
        static void     DebugDumpJson(const Json::Value& jsonValue);
        
        static BentleyStatus ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);

        static void     AssertECDateTime(ECN::ECValueCR expectedECValue, const Db& db, double actualJd);

        static ECN::IECInstancePtr  CreateArbitraryECInstance(ECN::ECClassCR ecClass, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false, bool skipReadOnlyProps = false);
        static void                 PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false, bool skipReadOnlyProps = false);
        static void            AssignRandomValueToECInstance(ECN::ECValueP createdValue, ECN::IECInstancePtr instance, Utf8CP propertyName);
        static void PopulatePrimitiveValueWithRandomValues(ECN::ECValueR ecValue, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);

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
END_ECDBUNITTESTS_NAMESPACE