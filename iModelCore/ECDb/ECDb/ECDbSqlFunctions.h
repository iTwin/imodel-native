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
        enum class Operation
            {
            Inserted = 1,     //'inserted'
            Deleted = 2,
            UpdatedOld = 3,   //'updated.old'
            UpdatedNew = 4,   //'updated.new'
            };

        //inserted=1,deleted=2, updated.old=3, updated.new=4

        static std::map<Utf8CP, std::function<void(Context&, ECSqlStatement&)>, CompareIUtf8Ascii> s_setValueMap;
        static std::set<int> s_operationValidValues;
        static std::map<Utf8CP, int, CompareIUtf8Ascii> s_operationStrValues;
        void SetValue(Context& ctx, ECSqlStatement& stmt);
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

        //This is part of public API therefore following must not change else it would break user code
        static_assert((int) Operation::Inserted == 1, "Expecting Inserted==1");
        static_assert((int) Operation::Deleted == 2, "Expecting Deleted==2");
        static_assert((int) Operation::UpdatedOld == 3, "Expecting UpdatedOld==3");
        static_assert((int) Operation::UpdatedNew == 4, "Expecting UpdatedNew==4");

    public:
        explicit ChangedValueFunction(ECDbCR);
        ~ChangedValueFunction() {}
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
