/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementBase.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
//static
ECSqlField::Collection ECSqlField::s_emptyChildCollection;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::Init () 
    {
    auto stat = _Init ();
    if (!stat.IsSuccess())
        return stat;

    for (unique_ptr<ECSqlField> const& child : GetChildren ())
        {
        stat = child->Init ();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfoCR ECSqlField::_GetColumnInfo () const
    {
    return m_ecsqlColumnInfo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::Reset () 
    {
    auto stat = _Reset ();
    if (!stat.IsSuccess())
        return stat;

    for (unique_ptr<ECSqlField> const& child : GetChildren ())
        {
        stat = child->Reset ();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatementBase& ECSqlField::GetECSqlStatementR () const
    { 
    return m_ecsqlStatement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlField::GetSqliteStatement () const
    { 
    BeAssert (GetECSqlStatementR ().IsPrepared ());
    return GetECSqlStatementR ().GetPreparedStatementP ()->GetSqliteStatementR ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::ReportError (ECSqlStatus status, Utf8CP errorMessage) const
    {
    ECDbCP ecdb = m_ecsqlStatement.GetECDb();
    if (ecdb != nullptr)
        ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, errorMessage);

    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
