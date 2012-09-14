/*--------------------------------------------------------------------------------------+
|
|     $Source: src/persistence/IECSchemaManager.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "ECPersistence/IECSchemaManager.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECSchemaManager::ImportSchema (ECSchemaCR ecSchema)
    {
    return _ImportSchema (ecSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECSchemaManager::GetClass (WCharCP schemaFullName, WCharCP className, ECClassP& ecClass) const
    {
    return _GetClass (schemaFullName, className, ecClass);
    }

END_BENTLEY_EC_NAMESPACE