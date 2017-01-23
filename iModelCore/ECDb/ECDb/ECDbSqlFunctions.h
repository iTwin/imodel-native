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
struct BlobToBase64 final : ScalarFunction
    {
    private:
        static BlobToBase64* s_singleton; //no need to release a static non-POD variable (Bentley C++ coding standards)

        BlobToBase64();
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        static BlobToBase64& GetSingleton();
    };


//=======================================================================================
// BLOB Base64ToBlob(TEXT base64Str)
// @bsiclass                                                   Krischan.Eberle   11/16
//=======================================================================================
struct Base64ToBlob final : ScalarFunction
    {
    private:
        static Base64ToBlob* s_singleton; //no need to release a static non-POD variable (Bentley C++ coding standards)

        Base64ToBlob();
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        static Base64ToBlob& GetSingleton();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
