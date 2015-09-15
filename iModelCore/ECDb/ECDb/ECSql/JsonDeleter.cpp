/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonDeleter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonDeleter::JsonDeleter (ECDbCR ecdb, ECN::ECClassCR ecClass)
: m_ecinstanceDeleter (ecdb, ecClass)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonDeleter::IsValid () const
    {
    return m_ecinstanceDeleter.IsValid ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonDeleter::Delete (ECInstanceId const& ecInstanceId) const
    {
    return m_ecinstanceDeleter.Delete (ecInstanceId);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
