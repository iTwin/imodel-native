/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlStatementBase.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** ECSqlPrepareContext::StatementScope ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ExpScope::ExpScope (ExpCR exp, ExpScope const* parent) 
: m_exp (exp), m_parent (parent), m_nativeSqlSelectClauseColumnCount (0)
    {
    m_ecsqlType = DetermineECSqlType (exp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ExpScope const* ECSqlPrepareContext::ExpScope::GetParent() const
    {
    return m_parent;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ExpCR ECSqlPrepareContext::ExpScope::GetExp() const
    {
    return m_exp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlPrepareContext::ExpScope::IsRootScope() const
    {
    return GetParent() == nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlType ECSqlPrepareContext::ExpScope::DetermineECSqlType (ExpCR exp) const
    {
    switch (exp.GetType ())
        {
        case Exp::Type::SingleSelect:
        case Exp::Type::Select:
                return ECSqlType::Select;
            case Exp::Type::Insert:
                return ECSqlType::Insert;
            case Exp::Type::Update:
                return ECSqlType::Update;
            case Exp::Type::Delete:
                return ECSqlType::Delete;
            default:
                {
                BeAssert (m_parent != nullptr && "DetermineECSqlType");
                return m_parent->GetECSqlType ();
                }
        }
    }

//****************************** ECSqlPrepareContext::StatementScopeStack ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::ExpScopeStack::Push (ExpCR exp)
    {
    ExpScope const* parent = nullptr;
    if (Depth() > 0)
        parent = & Current();

    m_scopes.push_back (ExpScope (exp, parent));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::ExpScopeStack::Pop ()
    {
    m_scopes.pop_back();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECSqlPrepareContext::ExpScopeStack::Depth () const 
    { 
    return m_scopes.size ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ExpScope const& ECSqlPrepareContext::ExpScopeStack::Current () const
    {
    BeAssert (!m_scopes.empty ());
    return m_scopes.back ();
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ExpScope& ECSqlPrepareContext::ExpScopeStack::CurrentR () 
    {
    BeAssert (!m_scopes.empty ());
    return m_scopes.back ();
    }

//****************************** ECSqlPrepareContext ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext (ECDbCR ecdb, ECSqlStatementBase& preparedStatment)
: m_ecdb (ecdb), m_ecsqlStatement (preparedStatment), m_parentCtx (nullptr), m_parentArrayProperty (nullptr), 
m_parentColumnInfo (nullptr), m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(&parentCtx), m_parentArrayProperty(nullptr),
    m_parentColumnInfo (nullptr), m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx, ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo)
: m_ecdb (ecdb), m_ecsqlStatement (preparedStatment), m_parentCtx (&parentCtx), 
 m_parentArrayProperty (&parentArrayProperty), m_parentColumnInfo (parentColumnInfo), 
 m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IClassMap::View ECSqlPrepareContext::GetClassMapViewMode () const
    {
    if (m_parentArrayProperty == nullptr)
        return IClassMap::View::DomainClass;
    else
        return IClassMap::View::EmbeddedType;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatementBase& ECSqlPrepareContext::GetECSqlStatementR () const
    {
    return m_ecsqlStatement;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlPrepareContext::GetNativeSql () const 
    { 
    return m_nativeSqlBuilder.ToString ();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlPrepareContext::ExpScope::GetNativeSqlSelectClauseColumnCount () const 
    {
    return m_nativeSqlSelectClauseColumnCount;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::ExpScope::IncrementNativeSqlSelectClauseColumnCount (size_t value)
    {
    m_nativeSqlSelectClauseColumnCount += static_cast<int> (value);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8String ECSqlPrepareContext::CreateECInstanceIdSelectionQuery (ECSqlPrepareContext& ctx, ClassNameExp const& classNameExpr, WhereExp const* whereExp)
    {
    NativeSqlBuilder selectBuilder;
    selectBuilder.Append ("SELECT DISTINCT ");
    if (!classNameExpr.GetAlias ().empty ())
        {
        selectBuilder.AppendEscaped (classNameExpr.GetAlias ().c_str ());
        selectBuilder.AppendDot ();
        }

    selectBuilder.Append ("ECInstanceId, GetECClassId() FROM");

    selectBuilder.AppendSpace ();
    if (!classNameExpr.IsPolymorphic ())
        {
        selectBuilder.Append ("ONLY");
        selectBuilder.AppendSpace ();
        }

    selectBuilder.AppendEscaped (classNameExpr.GetSchemaName ().c_str ());
    selectBuilder.AppendDot ();
    selectBuilder.AppendEscaped (classNameExpr.GetClassName ().c_str ());
    selectBuilder.AppendSpace ();

    if (!classNameExpr.GetAlias ().empty ())
        {
        selectBuilder.AppendEscaped (classNameExpr.GetAlias ().c_str ());
        }

    if (whereExp)
        {
        selectBuilder.AppendSpace ();
        selectBuilder.Append (whereExp->ToECSql ().c_str ());
        }

    return selectBuilder.ToString ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static 
int ECSqlPrepareContext::FindLastParameterIndexBeforeWhereClause (Exp const& statementExp, WhereExp const* whereExp)
    {
    int index = 0;
    FindLastParameterIndexBeforeWhereClause (index, statementExp, whereExp);
    return index;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bool ECSqlPrepareContext::FindLastParameterIndexBeforeWhereClause (int& index, Exp const& statementExp, WhereExp const* whereExp)
    {
    for (auto exp : statementExp.GetChildren ())
        {
        if (exp == whereExp)
            return true;

        else if (exp->IsParameterExp ())
            index++;

        else if (FindLastParameterIndexBeforeWhereClause (index, *exp, whereExp))
            return true;
        }

    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

