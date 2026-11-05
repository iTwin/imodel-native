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
#define SQLFUNC_ECSqlCast "_ecsql_cast"
#define SQLFUNC_ClassName "ec_classname"
#define SQLFUNC_ClassId "ec_classid"
#define SQLFUNC_InstanceOf "ec_instanceof"
#define SQLFUNC_XmlCAToJson "XmlCAToJson"

#define SQLFUNC_ExtractInst "extract_inst"
#define SQLFUNC_ExtractProp "extract_prop"
#define SQLFUNC_PropExists "prop_exists"
#define SQLFUNC_SupportInstanceQuery "supports_instance_query"

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
//! Internal conversion functions used by ECSQL cast lowering.
// @bsiclass
//=======================================================================================
struct ECSqlCastFunc final : ScalarFunction
    {
    private:
        using Compute = void (*)(Context&, int, DbValue*);
        Compute m_compute;
        bool m_parsesText;
        bool m_handlesNull;
        ECSqlCastFunc(Utf8CP name, int nArgs, Compute compute, bool parsesText = false, bool handlesNull = false)
            : ScalarFunction(name, nArgs, DbValueType::NullVal), m_compute(compute), m_parsesText(parsesText), m_handlesNull(handlesNull) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;
        static void ComputeStructText(Context&, int, DbValue*);
        static void ComputePointText(Context&, int, DbValue*);
        static void ComputeNavigationText(Context&, int, DbValue*);
        static void ComputeArrayText(Context&, int, DbValue*);
        static void ComputeId(Context&, int, DbValue*);
        static void ComputeJson(Context&, int, DbValue*);
        static void ComputeGuid(Context&, int, DbValue*);
        static void ComputeBase64(Context&, int, DbValue*);
        static void ComputeBooleanText(Context&, int, DbValue*);
        static void ComputeTextBoolean(Context&, int, DbValue*);
        static void ComputeDateTimeText(Context&, int, DbValue*);
        static void ComputeTextDateTime(Context&, int, DbValue*);
        static void ComputeGeometryText(Context&, int, DbValue*);
        static void ComputeTextPoint(Context&, int, DbValue*);
        static void ComputeEnum(Context&, int, DbValue*);

    public:
        ~ECSqlCastFunc() {}
        static bvector<ECSqlCastFunc*> const& GetFunctions();

        //! Layout codes describing how SQL values or internal array JSON are rendered by struct_text/arr_text
        static constexpr Utf8CP LayoutInt() { return "int"; }
        static constexpr Utf8CP LayoutLong() { return "long"; }
        static constexpr Utf8CP LayoutId() { return "id"; }
        static constexpr Utf8CP LayoutDouble() { return "dbl"; }
        static constexpr Utf8CP LayoutBool() { return "bool"; }
        static constexpr Utf8CP LayoutString() { return "str"; }
        static constexpr Utf8CP LayoutBinary() { return "bin"; }
        static constexpr Utf8CP LayoutGuid() { return "guid"; }
        static constexpr Utf8CP LayoutGeometry() { return "geom"; }
        static constexpr Utf8CP LayoutPoint2d() { return "p2"; }
        static constexpr Utf8CP LayoutPoint3d() { return "p3"; }
        static Utf8CP LayoutDateTime(DateTime::Info const&);
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
        explicit ExtractInstFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ExtractInst, 3, DbValueType::TextVal), m_ecdb(ecdb) {}
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
        explicit ExtractPropFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ExtractProp, -1, DbValueType::TextVal), m_ecdb(ecdb) {}
        ~ExtractPropFunc() {}
         static std::unique_ptr<ExtractPropFunc> Create(ECDbCR);
};

//=======================================================================================
//! S XmlCAToJson(S)
// @bsiclass
//=======================================================================================
struct XmlCAToJson final : ScalarFunction
    {
    private:
        SchemaManager const* m_schemaManager;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        XmlCAToJson(SchemaManager const& schemaManager) : ScalarFunction(SQLFUNC_XmlCAToJson, 2, DbValueType::TextVal), m_schemaManager(&schemaManager) {}
        ~XmlCAToJson() {}
        static std::unique_ptr<XmlCAToJson> Create(SchemaManager const& schemaManager);
    };

//=======================================================================================
//! I supports_instance_query(S)
//! Returns 1 if the given class (by name e.g. 'BisCore.Element' or by classId) supports
//! instance queries (SELECT $), 0 otherwise.
// @bsiclass
//=======================================================================================
struct SupportInstanceQueryFunc final : ScalarFunction {
    private:
        ECDbCR m_ecdb;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        explicit SupportInstanceQueryFunc(ECDbCR ecdb) : ScalarFunction(SQLFUNC_SupportInstanceQuery, 1, DbValueType::IntegerVal), m_ecdb(ecdb) {}
        ~SupportInstanceQueryFunc() {}
        static std::unique_ptr<SupportInstanceQueryFunc> Create(ECDbCR);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
