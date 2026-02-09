/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementNoopImpls.h"
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
NoopECSqlBinder* NoopECSqlBinder::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
NoopECSqlBinder& NoopECSqlBinder::Get()
    {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlBinder();

    return *s_singleton;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
NoopECSqlValue const* NoopECSqlValue::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
NoopECSqlValue const& NoopECSqlValue::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlValue();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void const* NoopECSqlValue::_GetBlob(int* blobSize) const
    {
    if (blobSize != nullptr)
        *blobSize = -1;

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

