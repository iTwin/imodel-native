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
m_parentColumnInfo (nullptr), m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false), m_joinTableClassId(0)
    {}

ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECClassId joinTableClassId)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(nullptr), m_parentArrayProperty(nullptr),
    m_parentColumnInfo(nullptr), m_nativeStatementIsNoop(false), m_nativeNothingToUpdate(false), m_joinTableClassId(joinTableClassId)
    {}
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(&parentCtx), m_parentArrayProperty(nullptr),
    m_parentColumnInfo (nullptr), m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false), m_joinTableClassId(0)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx, ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo)
: m_ecdb (ecdb), m_ecsqlStatement (preparedStatment), m_parentCtx (&parentCtx), 
 m_parentArrayProperty (&parentArrayProperty), m_parentColumnInfo (parentColumnInfo), 
 m_nativeStatementIsNoop (false), m_nativeNothingToUpdate (false), m_joinTableClassId(0)
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

//static 
ECSqlPrepareContext::JoinTableInfo::Ptr ECSqlPrepareContext::JoinTableInfo::TrySetupJoinTableContextForInsert(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    Ptr ptr = Ptr(new JoinTableInfo());
    auto const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (!classMap.IsJoinedTable())
        return false;

    NativeSqlBuilder parentOfJoinedTableECSQL;
    NativeSqlBuilder joinedTableECSQL;
    auto rootClassMap = classMap.FindRootOfJoinedTable();
    auto tables = exp.GetReferencedTables();

    NativeSqlBuilder::List joinedTableProperties;
    NativeSqlBuilder::List joinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableProperties;

    joinedTableECSQL.Append("INSERT INTO ").Append(classMap.GetECSqlName().c_str());
    parentOfJoinedTableECSQL.Append("INSERT INTO ").Append(rootClassMap->GetECSqlName().c_str());

    auto propertyList = exp.GetPropertyNameListExp();
    auto valueList = exp.GetValuesExp();
    ptr->m_userProvidedECInstanceId = false;
    ptr->m_primaryECInstanceIdParameterIndex = 0;

    for (size_t i = 0; i < propertyList->GetChildrenCount(); i++)
        {
        auto property = propertyList->GetPropertyNameExp(i);
        auto value = valueList->GetValueExp(i);
        std::vector<Parameter const*> thisValueParams;
        for (auto exp : value->Find(Exp::Type::Parameter, true /* recusive*/))
            {
            auto param = static_cast<ParameterExp const*>(exp);
            if (param->IsNamedParameter() && ptr->m_parameterMap.GetOrignal().Find(param->GetParameterName()))
                {
                //do nothing
                }
            else
                thisValueParams.push_back(ptr->m_parameterMap.GetOrignalR().Add(*param));
            }

        if (property->GetPropertyMap().IsSystemPropertyMap())
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));

            ptr->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            ptr->m_parameterMap.GetSecondaryR().Add(thisValueParams);

            if (!ptr->m_userProvidedECInstanceId  && property->GetPropertyMap().GetFirstColumn())
                {
                ptr->m_userProvidedECInstanceId = Enum::Contains(property->GetPropertyMap().GetFirstColumn()->GetKnownColumnId(), ECDbKnownColumns::ECInstanceId);
                BeAssert(thisValueParams.size() <= 1);
                if (thisValueParams.size() == 1)
                    ptr->m_primaryECInstanceIdParameterIndex = ptr->m_parameterMap.GetPrimaryR().Last();
                else if (thisValueParams.size() > 1)
                    {
                    BeAssert(false && "This case is not handled where e.g. (ECInstanceId ) VALUES ( ? + ? ) has more then one parameter");
                    return nullptr;
                    }
                }
            }
        else if (property->GetPropertyMap().IsMappedToPrimaryTable())
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            ptr->m_parameterMap.GetSecondaryR().Add(thisValueParams);
            }
        else
            {
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            ptr->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            }
        }

    if (!ptr->m_userProvidedECInstanceId)
        {
        parentOfJoinedTableProperties.push_back(NativeSqlBuilder("ECInstanceId"));
        parentOfJoinedTableValues.push_back(NativeSqlBuilder("?"));
        ptr->m_parameterMap.GetPrimaryR().Add();
        ptr->m_primaryECInstanceIdParameterIndex = ptr->m_parameterMap.GetPrimaryR().Last();
        }

    if (joinedTableProperties.empty())
        {
        joinedTableProperties.push_back(NativeSqlBuilder("ECInstanceId"));
        joinedTableValues.push_back(NativeSqlBuilder("NULL"));
        }

    joinedTableECSQL.AppendParenLeft().Append(joinedTableProperties).Append(") VALUES (").Append(joinedTableValues).AppendParenRight();
    parentOfJoinedTableECSQL.AppendParenLeft().Append(parentOfJoinedTableProperties).Append(") VALUES (").Append(parentOfJoinedTableValues).AppendParenRight();

    ptr->m_statement = joinedTableECSQL.ToString();
    ptr->m_parentStatement = parentOfJoinedTableECSQL.ToString();
    return std::move(ptr);
    }
//static 
ECSqlPrepareContext::JoinTableInfo::Ptr ECSqlPrepareContext::JoinTableInfo::TrySetupJoinTableContextForUpdate(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    return nullptr;
    }
//static 
ECSqlPrepareContext::JoinTableInfo::Ptr ECSqlPrepareContext::JoinTableInfo::TrySetupJoinTableContextIfAny(ECSqlPrepareContext& ctx, ECSqlParseTreeCR const& exp, Utf8CP orignalECSQL)
    {
    Ptr ptr;
    if (exp.GetType() == Exp::Type::Insert)
        ptr = TrySetupJoinTableContextForInsert(ctx, static_cast<InsertStatementExp const&>(exp));
    else if (exp.GetType() == Exp::Type::Update)
        ptr = TrySetupJoinTableContextForUpdate(ctx, static_cast<UpdateStatementExp const&>(exp));

    if (ptr != nullptr)
        {
        ptr->m_orginalStatement = orignalECSQL;
        return std::move(ptr);
        }

    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


