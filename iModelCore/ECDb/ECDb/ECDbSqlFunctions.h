/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
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
#define SQLFUNC_ClassName "ec_classname"
#define SQLFUNC_ClassId "ec_classid"
#define SQLFUNC_InstanceOf "ec_instanceof"

#define SQLFUNC_ExtractInst "extract_inst"
#define SQLFUNC_ExtractProp "extract_prop"
#define SQLFUNC_PropExists "prop_exists"

//=======================================================================================
//! S ClassName(I)
// @bsiclass
//=======================================================================================
struct ClassNameFunc final : ScalarFunction
    {
    enum class FormatOptions
        {
        s_semicolon_c = 0,    // SchemaName:ClassName
        a_semicolon_c = 1,    // SchemaAlias:ClassName
        s = 2,                // SchemaName
        a = 3,                // SchemaAlias
        c = 4,                // ClassName
        s_dot_c = 5,          // SchemaName.ClassName
        a_dot_c = 6,          // SchemaAlias.ClassName
        default_fmt = s_semicolon_c,
        };

    private:
        Db const* m_db;
        std::map<Utf8CP, FormatOptions, CompareIUtf8Ascii> m_options;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ClassNameFunc(DbCR db);
        ~ClassNameFunc() {}
        static std::unique_ptr<ClassNameFunc> Create(DbCR db);
    };
//=======================================================================================
//! I ClassId(S,S)
// @bsiclass
//=======================================================================================
struct ClassIdFunc final : ScalarFunction
    {
    private:
        Db const* m_db;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ClassIdFunc(DbCR db) : ScalarFunction(SQLFUNC_ClassId, -1, DbValueType::IntegerVal), m_db(&db) {}
        ~ClassIdFunc() {}
        static std::unique_ptr<ClassIdFunc> Create(DbCR db);
    };
//=======================================================================================
//! I InstanceOf(S,S)
// @bsiclass
//=======================================================================================
struct InstanceOfFunc final : ScalarFunction
    {
    private:
        Db const* m_db;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;
         std::map<Utf8String, ECN::ECClassId, CompareIUtf8Ascii> m_cache;
    public:
        InstanceOfFunc(DbCR db) : ScalarFunction(SQLFUNC_InstanceOf, -1, DbValueType::IntegerVal), m_db(&db) {}
        ~InstanceOfFunc() {}
        static std::unique_ptr<InstanceOfFunc> Create(DbCR db);
    };

//=======================================================================================
//! B StrToGuid(S)
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
//=======================================================================================
// @bsiclass
//=======================================================================================
struct ExtractInstFunc final : ScalarFunction {
    private:
        ECDbCR m_ecdb;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        explicit ExtractInstFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ExtractInst, 2, DbValueType::TextVal), m_ecdb(ecdb) {}
        ~ExtractInstFunc() {}
        static std::unique_ptr<ExtractInstFunc> Create(ECDbCR);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ExtractPropFunc final : ScalarFunction {
    private:
        ECDbCR m_ecdb;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        explicit ExtractPropFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ExtractProp, 3, DbValueType::TextVal), m_ecdb(ecdb) {}
        ~ExtractPropFunc() {}
         static std::unique_ptr<ExtractPropFunc> Create(ECDbCR);
};
//=======================================================================================
// @bsiclass
//=======================================================================================
struct PropExistsFunc final : ScalarFunction , ECDb::IECDbCacheClearListener {
    private:
        ECDbCR m_ecdb;
        InMemoryPropertyExistMap m_propMap;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;
        void _OnBeforeClearECDbCache() override {}
        void _OnAfterClearECDbCache() override { m_propMap.Build(m_ecdb, true); }
    public:
        explicit PropExistsFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_PropExists, 2, DbValueType::IntegerVal), m_ecdb(ecdb) {
            const_cast<ECDbR>(m_ecdb).AddECDbCacheClearListener(*this);
        }
        ~PropExistsFunc() { const_cast<ECDbR>(m_ecdb).RemoveECDbCacheClearListener(*this); }
        static std::unique_ptr<PropExistsFunc> Create(ECDbCR);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
