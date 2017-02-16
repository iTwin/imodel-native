/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementBase.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatementBase& ECSqlField::GetECSqlStatementR() const { return m_ecsqlStatement; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlField::GetSqliteStatement() const
    {
    BeAssert(GetECSqlStatementR().IsPrepared());
    return GetECSqlStatementR().GetPreparedStatementP()->GetSqliteStatementR();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
