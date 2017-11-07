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
// Helper funtion for changeSummary not intented for public use
// @bsiclass                                                   Affan.Khan         11/17
//=======================================================================================
struct ChangedValue final : ScalarFunction
    {
    private:
        ECDbR m_ecdb;
        ECSqlStatementCache m_stmtCache;
        static std::map<Utf8CP, std::function<void(Context&, ECSqlStatement&)>, CompareIUtf8Ascii> s_setValueMap;

        void SetValue(Context& ctx, ECSqlStatement& stmt);
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ChangedValue(ECDbR ecdb);
        ~ChangedValue() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
