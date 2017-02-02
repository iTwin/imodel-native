/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSqlFunctions.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/Base64Utilities.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
BlobToBase64SqlFunction* BlobToBase64SqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
BlobToBase64SqlFunction& BlobToBase64SqlFunction::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new BlobToBase64SqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
void BlobToBase64SqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& blobArg = args[0];
    if (blobArg.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    Byte const* blob = static_cast<Byte const*> (blobArg.GetValueBlob());
    const int byteCount = blobArg.GetValueBytes();
    BeAssert(byteCount >= 0);
    Utf8String str;
    Base64Utilities::Encode(str, blob, (size_t) byteCount);
    ctx.SetResultText(str.c_str(), (int) str.length(), Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Base64ToBlobSqlFunction* Base64ToBlobSqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Base64ToBlobSqlFunction& Base64ToBlobSqlFunction::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new Base64ToBlobSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
void Base64ToBlobSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& base64Arg = args[0];

    if (base64Arg.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP base64Str = base64Arg.GetValueText();
    if (!Base64Utilities::MatchesAlphabet(base64Str))
        {
        Utf8String error;
        error.Sprintf("Invalid argument for SQL function %s: '%s' is not a valid base64 string.", GetName(), base64Str);
        ctx.SetResultError(error.c_str());
        return;
        }

    int base64StrLen = base64Arg.GetValueBytes();
    if (base64StrLen < 0)
        base64StrLen = (int) strlen(base64Str);
    
    ByteStream blob;
    Base64Utilities::Decode(blob, base64Str, (size_t) base64StrLen);
    ctx.SetResultBlob(blob.data(), (int) blob.size(), DbFunction::Context::CopyData::Yes);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE