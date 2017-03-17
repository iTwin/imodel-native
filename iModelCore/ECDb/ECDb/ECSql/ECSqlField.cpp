/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlField::GetSqliteStatement() const
    {
#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR
    return m_preparedECSqlStatement.GetSqliteStatement();
#else
    return m_preparedECSqlStatement.GetSqliteStatementR();
#endif
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
