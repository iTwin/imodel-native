/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlField::ECSqlField (ECSqlStatementBase& owner, ECSqlColumnInfo&& ecsqlColumnInfo) 
    : m_ecsqlStatement (owner), m_ecsqlColumnInfo (move (ecsqlColumnInfo))
    {
    }

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
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::_Init () 
    {
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
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::_Reset () 
    {
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


//****************** ECSqlPrimitiveBinder ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlPrimitiveBinder::ECSqlPrimitiveBinder()
    : m_sourceColumnIndex(-1), m_targetParameterIndex(-1), m_sourceStmtType(StatementType::Unknown)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void ECSqlPrimitiveBinder::SetSourcePropertyPath(Utf8CP column)
    {
    m_sourcePropertyPath = column;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void ECSqlPrimitiveBinder::SetSourceStatementType(StatementType type)
    {
    m_sourceStmtType = type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void ECSqlPrimitiveBinder::SetSourceColumnIndex(int columnIndex)
    {
    m_sourceColumnIndex = columnIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void ECSqlPrimitiveBinder::SetTargetParamterIndex(int parameterIndex)
    {
    m_targetParameterIndex = parameterIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
bool ECSqlPrimitiveBinder::IsResolved() const
    {
    return m_sourceColumnIndex > -1 && m_targetParameterIndex > -1 && m_sourceStmtType != StatementType::Unknown;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
Utf8StringCR ECSqlPrimitiveBinder::GetSourcePropertyPath()const  { return m_sourcePropertyPath;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
int ECSqlPrimitiveBinder::GetSourceColumnIndex() const { return m_sourceColumnIndex;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
int ECSqlPrimitiveBinder::GetTargetParameterIndex() const {return m_targetParameterIndex;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlPrimitiveBinder::StatementType ECSqlPrimitiveBinder::GetSourceStatementType () const {return m_sourceStmtType;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPrimitiveBinder::Execute(ECSqlStatementBase& sourceStmt, ECSqlStatementBase& targetStmt, IECSqlBinder::MakeCopy makeCopy)
    {
    if (!IsResolved())
        return ECSqlStatus::Error;

    BeAssert(sourceStmt.GetPreparedStatementP() != nullptr);

    IECSqlValue const* ecsqlValue = nullptr;
    if (m_sourceStmtType == StatementType::ECSql)
        ecsqlValue = &sourceStmt.GetValue(m_sourceColumnIndex);

    Statement& sourceSqliteStatement = sourceStmt.GetPreparedStatementP()->GetSqliteStatementR();

    IECSqlBinder& targetBinder = targetStmt.GetBinder(m_targetParameterIndex);
    int64_t value = (m_sourceStmtType == StatementType::ECSql) ?
        ecsqlValue->GetInt64() : sourceSqliteStatement.GetValueInt64(m_sourceColumnIndex);
    return targetBinder.BindInt64(value);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
