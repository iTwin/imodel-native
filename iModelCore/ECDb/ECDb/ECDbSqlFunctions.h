/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSqlFunctions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSQLFUNC_ChangeSummary "ChangeSummary"
#define SQLFUNC_ChangedValue "ChangedValue"
#define SQLFUNC_ToInstanceOp "ToInstanceOp"
//=======================================================================================
// TEXT BlobToBase64(BLOB blob)
// @bsiclass                                                   Krischan.Eberle   11/16
//=======================================================================================
struct BlobToBase64SqlFunction final : ScalarFunction
    {
    private:
        static BlobToBase64SqlFunction* s_singleton; //no need to release a static non-POD variable (Bentley C++ coding standards)

        BlobToBase64SqlFunction() : ScalarFunction("BlobToBase64", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        static BlobToBase64SqlFunction& GetSingleton();
    };


//=======================================================================================
// BLOB Base64ToBlob(TEXT base64Str)
// @bsiclass                                                   Krischan.Eberle   11/16
//=======================================================================================
struct Base64ToBlobSqlFunction final : ScalarFunction
    {
    private:
        static Base64ToBlobSqlFunction* s_singleton; //no need to release a static non-POD variable (Bentley C++ coding standards)

        Base64ToBlobSqlFunction() : ScalarFunction("Base64ToBlob", 1, DbValueType::BlobVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        static Base64ToBlobSqlFunction& GetSingleton();
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan         11/17
//=======================================================================================
struct ChangeSummaryHelper final
    {
    enum class InstanceOp
        {
        Inserted = 1,
        Deleted = 2,
        Updated = 4
        };

    enum class PropertyValueOp
        {
        Inserted = 1,     //'inserted'
        Deleted = 2,      //'deleted'
        UpdatedOld = 3,   //'updated.old'
        UpdatedNew = 4,   //'updated.new'
        };

    public:
        static std::set<PropertyValueOp> s_validPropertyValueOp;
        static std::set<InstanceOp> s_validInstanceOp;
        static std::map<Utf8CP, PropertyValueOp, CompareIUtf8Ascii> s_stringToPropertyValueOp;
        static std::map<PropertyValueOp, InstanceOp> s_propertyValueToInstanceOp;

    public:
        static bool IsValidInstanceOp(int i);
        static bool IsValidPropertyValueOp(int i);
        static bool TryParsePropertyValueOp(PropertyValueOp& op, Utf8CP v);
        static bool OpReferToOldValue(PropertyValueOp op);
        static InstanceOp ToInstanceOp(PropertyValueOp op);
    };

//=======================================================================================
// INT ToInstanceOpFuntion(TEXT|INT)
// @bsiclass                                                   Affan.Khan        11/17
//=======================================================================================
struct ToInstanceOpFuntion final : ScalarFunction
    {
    private:
        static ToInstanceOpFuntion* s_singleton;
        ToInstanceOpFuntion() : ScalarFunction("ExtractChangeOp", 1, DbValueType::IntegerVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        static ToInstanceOpFuntion& GetSingleton();
    };

//=======================================================================================
// BaseClass for scalar system funtion that require ECDb as context and use cache stmts
// @bsiclass                                                   Affan.Khan         11/17
//=======================================================================================
struct ECDbSystemScalarFunction : ScalarFunction
    {
    private:
        ECDbCR m_ecdb;
        ECSqlStatementCache m_stmtCache;

    protected:
        CachedECSqlStatementPtr GetPreparedStatement(Utf8CP ecsql) const;
        ECDbCR GetECDb() const { return m_ecdb; }

    public:
        ECDbSystemScalarFunction(ECDbCR, Utf8CP, int, DbValueType);
        ~ECDbSystemScalarFunction() {}
    };

//=======================================================================================
// Helper SQL function for ChangeSummary not intended for public use
// @bsiclass                                                   Affan.Khan         11/17
//=======================================================================================
struct ChangedValueFunction final : ECDbSystemScalarFunction
    {
    private:
        //Note: This must have the exact same definition as the respective ECEnumeration in the ECDbChangeSummaries schema
        static std::map<Utf8CP, std::function<void(Context&, ECSqlStatement&)>, CompareIUtf8Ascii> s_setValueMap;
        void SetValue(Context& ctx, ECSqlStatement& stmt);
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        explicit ChangedValueFunction(ECDbCR);
        ~ChangedValueFunction() {}
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
