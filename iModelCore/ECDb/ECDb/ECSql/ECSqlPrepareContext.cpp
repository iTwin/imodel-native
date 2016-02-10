/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlPrepareContext::ExpScope::ExpScope(ExpCR exp, ExpScope const* parent, OptionsExp const* options)
    : m_exp(exp), m_parent(parent), m_options(options), m_nativeSqlSelectClauseColumnCount(0), m_extendedOptions(ExtendedOptions::None)
    {
    m_ecsqlType = DetermineECSqlType(exp);
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
void ECSqlPrepareContext::ExpScopeStack::Push (ExpCR exp, OptionsExp const* options)
    {
    ExpScope const* parent = nullptr;
    if (Depth() > 0)
        parent = & Current();

    m_scopes.push_back (ExpScope (exp, parent, options));
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
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(nullptr), m_parentArrayProperty(nullptr),
    m_parentColumnInfo(nullptr), m_nativeStatementIsNoop(false), m_nativeNothingToUpdate(false), m_joinTableClassId(ECClass::UNSET_ECCLASSID), m_joinTableInfo(nullptr)
    {}

ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECClassId joinTableClassId)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(nullptr), m_parentArrayProperty(nullptr),
    m_parentColumnInfo(nullptr), m_nativeStatementIsNoop(false), m_nativeNothingToUpdate(false), m_joinTableClassId(joinTableClassId), m_joinTableInfo(nullptr)
    {}
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(&parentCtx), m_parentArrayProperty(nullptr),
    m_parentColumnInfo(nullptr), m_nativeStatementIsNoop(false), m_nativeNothingToUpdate(false), m_joinTableClassId(ECClass::UNSET_ECCLASSID), m_joinTableInfo(nullptr)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx, ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo)
    : m_ecdb(ecdb), m_ecsqlStatement(preparedStatment), m_parentCtx(&parentCtx),
    m_parentArrayProperty(&parentArrayProperty), m_parentColumnInfo(parentColumnInfo),
    m_nativeStatementIsNoop(false), m_nativeNothingToUpdate(false), m_joinTableClassId(ECClass::UNSET_ECCLASSID), m_joinTableInfo(nullptr)
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
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo const* ECSqlPrepareContext::TrySetupJoinTableContext(ECSqlParseTreeCR exp, Utf8CP orignalECSQL)
    {
    m_joinTableInfo = JoinedTableInfo::TrySetupJoinTableContext(*this, exp, orignalECSQL);
    return GetJoinTableInfo();
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
Utf8String ECSqlPrepareContext::CreateECInstanceIdSelectionQuery(ECSqlPrepareContext& ctx, ClassNameExp const& classNameExpr, WhereExp const* whereExp)
    {
    NativeSqlBuilder selectBuilder;
    selectBuilder.Append("SELECT DISTINCT ");
    if (!classNameExpr.GetAlias().empty())
        {
        selectBuilder.AppendEscaped(classNameExpr.GetAlias().c_str());
        selectBuilder.AppendDot();
        }

    selectBuilder.Append("ECInstanceId, GetECClassId() FROM ");

    if (!classNameExpr.IsPolymorphic())
        selectBuilder.Append("ONLY ");

    selectBuilder.AppendEscaped(classNameExpr.GetSchemaName().c_str());
    selectBuilder.AppendDot();
    selectBuilder.AppendEscaped(classNameExpr.GetClassName().c_str());
    selectBuilder.AppendSpace();

    if (!classNameExpr.GetAlias().empty())
        {
        selectBuilder.AppendEscaped(classNameExpr.GetAlias().c_str());
        selectBuilder.AppendSpace();
        }

    if (whereExp != nullptr)
        selectBuilder.Append(whereExp->ToECSql().c_str());

    return selectBuilder.ToString();
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

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::TrySetupJoinTableContextForInsert(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    IClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (!classMap.HasJoinedTable())
        return nullptr;

    NativeSqlBuilder parentOfJoinedTableECSQL;
    NativeSqlBuilder joinedTableECSQL;
    IClassMap const* parentOfJoinedTableClassMap = classMap.FindClassMapOfParentOfJoinedTable();
    if (parentOfJoinedTableClassMap == nullptr)
        {
        BeAssert(parentOfJoinedTableClassMap != nullptr && "Root class for joined table must exist.");
        return nullptr;
        }

    ECDbSqlTable const& primaryTable = classMap.GetPrimaryTable();
    ECDbSqlTable const& joinedTable = classMap.GetJoinedTable();
    auto tables = exp.GetReferencedTables();
    if (tables.size() < 2)
        {
        if (&classMap.GetJoinedTable() == &parentOfJoinedTableClassMap->GetJoinedTable())
            return nullptr;
        }

    NativeSqlBuilder::List joinedTableProperties;
    NativeSqlBuilder::List joinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableProperties;

    joinedTableECSQL.Append("INSERT INTO ").Append(classMap.GetClass().GetECSqlName().c_str());
    parentOfJoinedTableECSQL.Append("INSERT INTO ").Append(parentOfJoinedTableClassMap->GetClass().GetECSqlName().c_str());

    std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> info = std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo>( new JoinedTableInfo(classMap.GetClass()));
    auto propertyList = exp.GetPropertyNameListExp();
    auto valueList = exp.GetValuesExp();
    for (size_t i = 0; i < propertyList->GetChildrenCount(); i++)
        {
        PropertyNameExp const* property = propertyList->GetPropertyNameExp(i);
        ValueExp const* value = valueList->GetValueExp(i);
        std::vector<Parameter const*> thisValueParams;
        for (Exp const* exp : value->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const* param = static_cast<ParameterExp const*>(exp);
            if (!param->IsNamedParameter() || info->m_parameterMap.GetOrignal().Find(param->GetParameterName()) == nullptr)
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(*param));
            }

        if (property->GetPropertyMap().IsSystemPropertyMap())
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));

            info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);

            if (!info->m_ecinstanceIdIsUserProvided  && property->GetPropertyMap().GetSingleColumn())
                {
                info->m_ecinstanceIdIsUserProvided = Enum::Contains(property->GetPropertyMap().GetSingleColumn()->GetKind(), ColumnKind::ECInstanceId);
                BeAssert(thisValueParams.size() <= 1);
                if (thisValueParams.size() == 1)
                    info->m_primaryECInstanceIdParameterIndex = info->m_parameterMap.GetPrimaryR().Last();
                else if (thisValueParams.size() > 1)
                    {
                    BeAssert(false && "This case is not handled where e.g. (ECInstanceId ) VALUES ( ? + ? ) has more then one parameter");
                    return nullptr;
                    }
                }
            }
        else if (property->GetPropertyMap().MapsToTable(joinedTable))
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
            }
        else if (property->GetPropertyMap().GetAsPropertyMapStructArray())
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
            }
        else
            {
            BeAssert(property->GetPropertyMap().MapsToTable(primaryTable));
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            }
        }

    if (!info->m_ecinstanceIdIsUserProvided)
        {
        parentOfJoinedTableProperties.push_back(NativeSqlBuilder("ECInstanceId"));
        parentOfJoinedTableValues.push_back(NativeSqlBuilder("?"));
        info->m_parameterMap.GetPrimaryR().Add();
        info->m_primaryECInstanceIdParameterIndex = info->m_parameterMap.GetPrimaryR().Last();
        }

    if (joinedTableProperties.empty())
        {
        joinedTableProperties.push_back(NativeSqlBuilder("ECInstanceId"));
        joinedTableValues.push_back(NativeSqlBuilder("NULL"));
        }

    joinedTableECSQL.AppendParenLeft().Append(joinedTableProperties).Append(") VALUES (").Append(joinedTableValues).AppendParenRight();
    parentOfJoinedTableECSQL.AppendParenLeft().Append(parentOfJoinedTableProperties).Append(") VALUES (").Append(parentOfJoinedTableValues).AppendParenRight();

    info->m_joinedTableECSql = joinedTableECSQL.ToString();
    info->m_parentOfJoinedTableECSql = parentOfJoinedTableECSQL.ToString();
    return std::move(info);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::TrySetupJoinTableContextForUpdate(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    IClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (!classMap.HasJoinedTable())
        return nullptr;

    NativeSqlBuilder parentOfJoinedTableECSQL;
    NativeSqlBuilder joinedTableECSQL;
    IClassMap const* parentOfJoinedTableClassMap = classMap.FindClassMapOfParentOfJoinedTable();

    ECDbSqlTable const& primaryTable = classMap.GetPrimaryTable();
    ECDbSqlTable const& joinedTable = classMap.GetJoinedTable();
    auto tables = exp.GetReferencedTables();
    if (tables.size() <= 2)
        {
        if (&classMap.GetJoinedTable() == &parentOfJoinedTableClassMap->GetJoinedTable())
            return nullptr;
        }


    NativeSqlBuilder::List joinedTableProperties;
    NativeSqlBuilder::List joinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableProperties;
    bool isPolymorphic = exp.GetClassNameExp()->IsPolymorphic();
    joinedTableECSQL.Append("UPDATE ").AppendIf(!isPolymorphic, "ONLY ").Append(classMap.GetClass().GetECSqlName().c_str()).Append(" SET ");
    parentOfJoinedTableECSQL.Append("UPDATE ").Append(parentOfJoinedTableClassMap->GetClass().GetECSqlName().c_str()).Append(" SET ");

    std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> info = std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo>(new JoinedTableInfo(classMap.GetClass()));
    auto assignmentList = exp.GetAssignmentListExp();
    for (size_t i = 0; i < assignmentList->GetChildrenCount(); i++)
        {
        auto assignmentExp = assignmentList->GetAssignmentExp(i);
        auto property = assignmentExp->GetPropertyNameExp();
        auto value = assignmentExp->GetValueExp();

        std::vector<Parameter const*> thisValueParams;
        for (auto exp : value->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            auto param = static_cast<ParameterExp const*>(exp);
            if (param->IsNamedParameter() && info->m_parameterMap.GetOrignal().Find(param->GetParameterName()))
                {
                //do nothing
                }
            else
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(*param));
            }

        if (property->GetPropertyMap().IsSystemPropertyMap())
            {
            BeAssert(false && "Updating system properties are not supported");
            return nullptr;
            }
        else if (property->GetPropertyMap().MapsToTable(joinedTable))
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
            }
        else if (property->GetPropertyMap().GetAsPropertyMapStructArray())
            {
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
            }
        else
            {
            BeAssert(property->GetPropertyMap().MapsToTable(primaryTable));
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            }
        }

    joinedTableECSQL.Append(BuildAssignmentExpression(joinedTableProperties, joinedTableValues));
    parentOfJoinedTableECSQL.Append(BuildAssignmentExpression(parentOfJoinedTableProperties, parentOfJoinedTableValues));

    if (auto bwhere = exp.GetWhereClauseExp())
        {
        std::vector<Parameter const*> thisValueParams;
        for (auto exp : bwhere->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            auto param = static_cast<ParameterExp const*>(exp);
            if (param->IsNamedParameter() && info->m_parameterMap.GetOrignal().Find(param->GetParameterName()))
                {
                //do nothing
                }
            else
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(*param));
            }

        info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
        info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
        joinedTableECSQL.AppendSpace().Append(bwhere->ToECSql().c_str());
        parentOfJoinedTableECSQL.AppendSpace().Append(bwhere->ToECSql().c_str());
        }


    if (!joinedTableProperties.empty())
        {
        info->m_joinedTableECSql = joinedTableECSQL.ToString();
        }

    if (!parentOfJoinedTableProperties.empty())
        {
        info->m_parentOfJoinedTableECSql = parentOfJoinedTableECSQL.ToString();
        }

    return std::move(info);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeSqlBuilder ECSqlPrepareContext::JoinedTableInfo::BuildAssignmentExpression(NativeSqlBuilder::List const& prop, NativeSqlBuilder::List const& values)
    {
    BeAssert(prop.size() == values.size());
    NativeSqlBuilder out;
    for (auto propI = prop.begin(), valueI = values.begin(); propI != prop.end() && valueI != values.end(); ++propI, ++valueI)
        {
        if (propI != prop.begin())
            out.AppendComma();

        out.Append(*propI).Append("=").Append(*valueI);      
        }

    return std::move(out);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::TrySetupJoinTableContext(ECSqlPrepareContext& ctx, ECSqlParseTreeCR exp, Utf8CP orignalECSQL)
    {
    std::unique_ptr<JoinedTableInfo> info = nullptr;
    if (exp.GetType() == Exp::Type::Insert)
        info = TrySetupJoinTableContextForInsert(ctx, static_cast<InsertStatementExp const&>(exp));
    else if (exp.GetType() == Exp::Type::Update)
        info = TrySetupJoinTableContextForUpdate(ctx, static_cast<UpdateStatementExp const&>(exp));

    if (info != nullptr)
        {
        bool isValidJoinTableContext = !(info->m_joinedTableECSql.empty() && info->m_parentOfJoinedTableECSql.empty());
        if (!isValidJoinTableContext)
            {
            BeAssert(isValidJoinTableContext == true);
            return nullptr;
            }

        info->m_originalECSql = orignalECSQL;

        auto& primary = info->m_parameterMap.GetPrimaryR();
        auto& secondary = info->m_parameterMap.GetSecondaryR();
        for (auto i = primary.First(); i > 0 && i <= primary.Last(); ++i)
            {
            auto p = const_cast<Parameter*>(primary.Find(i));
            if (p && p->GetOrignalParameter())
                {
                for (auto j = secondary.First(); j > 0 && j <= secondary.Last(); ++j)
                    {
                    auto s = const_cast<Parameter*>(secondary.Find(j));
                    if (s && p->GetOrignalParameter() == s->GetOrignalParameter())
                        {
                        p->m_shared = s->m_shared = true;
                        }
                    }
                }
            }

        return std::move(info);
        }

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


