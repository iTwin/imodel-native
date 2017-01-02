/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementNoopImpls.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementNoopImpls.h"
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
//static
NoopECSqlBinder* NoopECSqlBinder::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
//static
NoopECSqlBinder& NoopECSqlBinder::Get()
    {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlBinder();

    return *s_singleton;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
//static
NoopECSqlValue const* NoopECSqlValue::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
//static
NoopECSqlValue const& NoopECSqlValue::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlValue();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
void const* NoopECSqlValue::_GetBlob(int* blobSize) const
    {
    if (blobSize != nullptr)
        *blobSize = -1;

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

