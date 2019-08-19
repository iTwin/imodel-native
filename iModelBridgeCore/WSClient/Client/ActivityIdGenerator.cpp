/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ActivityIdGenerator.h"
#include <BeSQLite/BeSQLite.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Mantas.Smicius           10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ActivityIdGenerator::GenerateNextId() const
    {
    BeSQLite::BeGuid id = BeSQLite::BeGuid(true);
    return id.ToString();
    }