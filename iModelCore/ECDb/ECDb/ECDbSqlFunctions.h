/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSqlFunctions.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSQLFUNC_Changes "Changes"
#define SQLFUNC_ChangedValue "ChangedValue"
#define SQLFUNC_ChangedValueStateToOpCode "ChangedValueStateToOpCode"

//=======================================================================================
//! INT ChangedValueStateToOpCode(ChangedValueState TEXT|INT)
//!
//! @note Internal helper SQL function for Changes ECSQL function. Not intended for public use
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct ChangedValueStateToOpCodeSqlFunction final : ScalarFunction
    {
    private:
        static ChangedValueStateToOpCodeSqlFunction* s_singleton;  //no need to release a static non-POD variable (Bentley C++ coding standards)

        ChangedValueStateToOpCodeSqlFunction() : ScalarFunction(SQLFUNC_ChangedValueStateToOpCode, 1, DbValueType::IntegerVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~ChangedValueStateToOpCodeSqlFunction() {}

        static ChangedValueStateToOpCodeSqlFunction& GetSingleton();
    };

//=======================================================================================
//! BLOB ChangedValue(InstanceChangeId INT, PropertyAccessString TEXT, ChangedValueState INT|TEXT, FallbackValue BLOB)
//!
//! @note Internal helper SQL function for Changes ECSQL function. Not intended for public use
// @bsiclass                                                   Affan.Khan         11/17
//=======================================================================================
struct ChangedValueSqlFunction final : ScalarFunction
    {
    private:
        ECDbCR m_ecdb;
        ECSqlStatementCache m_statementCache;

        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        explicit ChangedValueSqlFunction(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ChangedValue, 4, DbValueType::BlobVal), m_ecdb(ecdb), m_statementCache(5) {}
        ~ChangedValueSqlFunction() {}

        void ClearCache() { m_statementCache.Empty(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
