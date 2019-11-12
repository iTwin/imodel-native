/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSQLFUNC_Changes "Changes"
#define SQLFUNC_ChangedValue "ChangedValue"
#define SQLFUNC_ChangedValueStateToOpCode "ChangedValueStateToOpCode"
#define SQLFUNC_StrToGuid "StrToGuid"
#define SQLFUNC_GuidToStr "GuidToStr"
#define SQLFUNC_IdToHex "IdToHex"
#define SQLFUNC_HexToId "HexToId"
//=======================================================================================
//! B StrToGuid(S)
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct StrToGuid final : ScalarFunction
    {
    private:
        StrToGuid() : ScalarFunction(SQLFUNC_StrToGuid, 1, DbValueType::BlobVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~StrToGuid() {}
        static StrToGuid& GetSingleton();
    };
//=======================================================================================
//! S GuidToStr(B)
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct GuidToStr final : ScalarFunction
    {
    private:
        GuidToStr() : ScalarFunction(SQLFUNC_GuidToStr, 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~GuidToStr() {}

        static GuidToStr& GetSingleton();
    };
//=======================================================================================
//! S IdToHex(I)
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct IdToHex final : ScalarFunction
    {
    private:
        IdToHex() : ScalarFunction(SQLFUNC_IdToHex, 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~IdToHex() {}

        static IdToHex& GetSingleton();
    };
//=======================================================================================
//! I HexToId(S)
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct HexToId final : ScalarFunction
    {
    private:
        HexToId() : ScalarFunction(SQLFUNC_HexToId, 1, DbValueType::IntegerVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~HexToId() {}

        static HexToId& GetSingleton();
    };


//=======================================================================================
//! INT ChangedValueStateToOpCode(ChangedValueState TEXT|INT)
//!
//! @note Internal helper SQL function for Changes ECSQL function. Not intended for public use
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct ChangedValueStateToOpCodeSqlFunction final : ScalarFunction
    {
    private:
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
