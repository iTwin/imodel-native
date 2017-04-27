/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR

//****************************** ECSqlPrepareContext ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(IECSqlPreparedStatement& stmt) : m_ecdb(stmt.GetECDb()) 
    {
    if (!stmt.IsCompoundStatement())
        {
        BeAssert(dynamic_cast<SingleECSqlPreparedStatement*> (&stmt) != nullptr);
        m_singlePreparedStatement = static_cast<SingleECSqlPreparedStatement*> (&stmt);
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::Reset(SingleECSqlPreparedStatement& preparedStmt)
    {
    m_singlePreparedStatement = &preparedStmt;

    m_nativeSqlBuilder.Clear();
    m_scopes.Clear();
    m_selectionOptions.Clear();
    }

#else

//****************************** ECSqlPrepareContext ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECCrudWriteToken const* writeToken)
    : m_ecdb(ecdb), m_writeToken(writeToken), m_ecsqlStatement(preparedStatment)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECClassId joinedTableClassId, ECCrudWriteToken const* writeToken)
    : m_ecdb(ecdb), m_writeToken(writeToken), m_ecsqlStatement(preparedStatment), m_joinedTableClassId(joinedTableClassId)
    {}
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatementBase& ECSqlPrepareContext::GetECSqlStatementR() const { return m_ecsqlStatement; }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlPrepareContext::GetNativeSql() const { return m_nativeSqlBuilder.ToString(); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo const* ECSqlPrepareContext::TrySetupJoinedTableInfo(Exp const& exp, Utf8CP orignalECSQL)
    {
    m_joinedTableInfo = JoinedTableInfo::Create(*this, exp, orignalECSQL);
    return GetJoinedTableInfo();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::CreateForInsert(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    ClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (!classMap.GetMapStrategy().IsTablePerHierarchy() || !classMap.GetTphHelper()->HasJoinedTable())
        return nullptr;

    ECClassId parentOfJoinedTableClassId = classMap.GetTphHelper()->DetermineParentOfJoinedTableECClassId();
    if (!parentOfJoinedTableClassId.IsValid())
        {
        BeAssert(false && "Root class for joined table must exist.");
        return nullptr;
        }

    ECClassCP parentOfJoinedTableClass = ctx.GetECDb().Schemas().GetClass(parentOfJoinedTableClassId);
    if (parentOfJoinedTableClass == nullptr)
        {
        BeAssert(false && "Root class for joined table must exist.");
        return nullptr;
        }

    NativeSqlBuilder parentOfJoinedTableECSQL;
    NativeSqlBuilder joinedTableECSQL;

    DbTable const& primaryTable = classMap.GetPrimaryTable();
    DbTable const& joinedTable = classMap.GetJoinedOrPrimaryTable();

    NativeSqlBuilder::List joinedTableProperties;
    NativeSqlBuilder::List joinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableProperties;

    joinedTableECSQL.Append("INSERT INTO ").Append(classMap.GetClass().GetECSqlName().c_str());
    parentOfJoinedTableECSQL.Append("INSERT INTO ").Append(parentOfJoinedTableClass->GetECSqlName().c_str());

    std::unique_ptr<JoinedTableInfo> info(new JoinedTableInfo(classMap.GetClass()));
    PropertyNameListExp const* propertyList = exp.GetPropertyNameListExp();
    ValueExpListExp const* valueList = exp.GetValuesExp();
    for (size_t i = 0; i < propertyList->GetChildrenCount(); i++)
        {
        PropertyNameExp const* property = propertyList->GetPropertyNameExp(i);
        ValueExp const* value = valueList->GetValueExp(i);
        bvector<Parameter const*> thisValueParams;
        for (Exp const* exp : value->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            if (!paramExp.IsNamedParameter() || info->m_parameterMap.GetOrignal().Find(paramExp.GetParameterName()) == nullptr)
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(paramExp));
            }


        if (property->GetPropertyMap().IsSystem())
            {
            SystemPropertyMap const& systemPropertyMap = property->GetPropertyMap().GetAs<SystemPropertyMap>();
            joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
            parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
            parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));

            info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
            info->m_parameterMap.GetSecondaryR().Add(thisValueParams);

            if (!info->m_ecinstanceIdIsUserProvided  && systemPropertyMap.IsMappedToSingleTable())
                {
                DbTable const* contextTable = systemPropertyMap.GetTables().front();
                SingleColumnDataPropertyMap const* vmap = systemPropertyMap.FindDataPropertyMap(*contextTable);
                if (vmap == nullptr)
                    {
                    BeAssert(vmap != nullptr);
                    return nullptr;
                    }

                info->m_ecinstanceIdIsUserProvided = Enum::Contains(vmap->GetColumn().GetKind(), DbColumn::Kind::ECInstanceId);
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
        else if (property->GetPropertyMap().IsData())
            {
            DataPropertyMap const& businessPropertyMap = property->GetPropertyMap().GetAs<DataPropertyMap>();
            if (&businessPropertyMap.GetTable() == &joinedTable)
                {
                joinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
                joinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
                info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
                }
            else
                {
                BeAssert(&businessPropertyMap.GetTable() == &primaryTable);
                parentOfJoinedTableProperties.push_back(NativeSqlBuilder(property->ToECSql().c_str()));
                parentOfJoinedTableValues.push_back(NativeSqlBuilder(value->ToECSql().c_str()));
                info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
                }
            }
        else
            {
            BeAssert(false && "Case not handled. This should never happen");
            return nullptr;
            }
        }

    if (!info->m_ecinstanceIdIsUserProvided)
        {
        parentOfJoinedTableProperties.push_back(NativeSqlBuilder(ECDBSYS_PROP_ECInstanceId));
        parentOfJoinedTableValues.push_back(NativeSqlBuilder("?"));
        info->m_parameterMap.GetPrimaryR().Add();
        info->m_primaryECInstanceIdParameterIndex = info->m_parameterMap.GetPrimaryR().Last();
        }

    if (joinedTableProperties.empty())
        {
        joinedTableProperties.push_back(NativeSqlBuilder(ECDBSYS_PROP_ECInstanceId));
        joinedTableValues.push_back(NativeSqlBuilder("NULL"));
        }

    joinedTableECSQL.AppendParenLeft().Append(joinedTableProperties).Append(") VALUES (").Append(joinedTableValues).AppendParenRight();
    parentOfJoinedTableECSQL.AppendParenLeft().Append(parentOfJoinedTableProperties).Append(") VALUES (").Append(parentOfJoinedTableValues).AppendParenRight();

    info->m_joinedTableECSql = joinedTableECSQL.ToString();
    info->m_parentOfJoinedTableECSql = parentOfJoinedTableECSQL.ToString();
    return info;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::CreateForUpdate(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    ClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (!classMap.GetMapStrategy().IsTablePerHierarchy() || !classMap.GetTphHelper()->HasJoinedTable())
        return nullptr;

    ECClassId parentOfJoinedTableClassId = classMap.GetTphHelper()->DetermineParentOfJoinedTableECClassId();
    if (!parentOfJoinedTableClassId.IsValid())
        {
        BeAssert(false && "Root class for joined table must exist.");
        return nullptr;
        }

    ECClassCP parentOfJoinedTableClass = ctx.GetECDb().Schemas().GetClass(parentOfJoinedTableClassId);
    if (parentOfJoinedTableClass == nullptr)
        {
        BeAssert(false && "Root class for joined table must exist.");
        return nullptr;
        }

    NativeSqlBuilder parentOfJoinedTableECSQL;
    NativeSqlBuilder joinedTableECSQL;

    DbTable const& primaryTable = classMap.GetPrimaryTable();
    DbTable const& joinedTable = classMap.GetJoinedOrPrimaryTable();

    NativeSqlBuilder::List joinedTableProperties;
    NativeSqlBuilder::List joinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableValues;
    NativeSqlBuilder::List parentOfJoinedTableProperties;
    bool isPolymorphic = exp.GetClassNameExp()->IsPolymorphic();
    joinedTableECSQL.Append("UPDATE ").AppendIf(!isPolymorphic, "ONLY ").Append(classMap.GetClass().GetECSqlName().c_str()).Append(" SET ");
    parentOfJoinedTableECSQL.Append("UPDATE ").Append(parentOfJoinedTableClass->GetECSqlName().c_str()).Append(" SET ");

    std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> info = std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo>(new JoinedTableInfo(classMap.GetClass()));
    AssignmentListExp const* assignmentListExp = exp.GetAssignmentListExp();
    for (size_t i = 0; i < assignmentListExp->GetChildrenCount(); i++)
        {
        AssignmentExp const* assignmentExp = assignmentListExp->GetAssignmentExp(i);
        PropertyNameExp const* propertyNameExp = assignmentExp->GetPropertyNameExp();
        ValueExp const* assignmentValueExp = assignmentExp->GetValueExp();

        bvector<Parameter const*> thisValueParams;
        for (Exp const* exp : assignmentValueExp->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            if (!paramExp.IsNamedParameter() || info->m_parameterMap.GetOrignal().Find(paramExp.GetParameterName()) == nullptr)
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(paramExp));
            }

        if (propertyNameExp->GetPropertyMap().IsSystem())
            {
            BeAssert(false && "Updating system properties are not supported");
            return nullptr;
            }
        else if (propertyNameExp->GetPropertyMap().IsData())
            {
            DataPropertyMap const& businessPropertyMap = propertyNameExp->GetPropertyMap().GetAs<DataPropertyMap>();
            if (&businessPropertyMap.GetTable() == &joinedTable)
                {
                joinedTableProperties.push_back(NativeSqlBuilder(propertyNameExp->ToECSql().c_str()));
                joinedTableValues.push_back(NativeSqlBuilder(assignmentValueExp->ToECSql().c_str()));
                info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
                }
            else
                {
                BeAssert(&businessPropertyMap.GetTable() == &primaryTable);
                parentOfJoinedTableProperties.push_back(NativeSqlBuilder(propertyNameExp->ToECSql().c_str()));
                parentOfJoinedTableValues.push_back(NativeSqlBuilder(assignmentValueExp->ToECSql().c_str()));
                info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
                }
            }
        }

    joinedTableECSQL.Append(BuildAssignmentExpression(joinedTableProperties, joinedTableValues));
    parentOfJoinedTableECSQL.Append(BuildAssignmentExpression(parentOfJoinedTableProperties, parentOfJoinedTableValues));

    if (WhereExp const* bwhere = exp.GetWhereClauseExp())
        {
        bvector<Parameter const*> thisValueParams;
        for (Exp const* exp : bwhere->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            if (!(paramExp.IsNamedParameter() && info->m_parameterMap.GetOrignal().Find(paramExp.GetParameterName())))
                thisValueParams.push_back(info->m_parameterMap.GetOrignalR().Add(paramExp));
            }

        info->m_parameterMap.GetSecondaryR().Add(thisValueParams);
        info->m_parameterMap.GetPrimaryR().Add(thisValueParams);
        joinedTableECSQL.AppendSpace().Append(bwhere->ToECSql().c_str());
        parentOfJoinedTableECSQL.AppendSpace().Append(bwhere->ToECSql().c_str());
        }

    OptionsExp const* optionsExp = exp.GetOptionsClauseExp();
    if (optionsExp != nullptr)
        {
        joinedTableECSQL.AppendSpace().Append(optionsExp->ToECSql().c_str());
        parentOfJoinedTableECSQL.AppendSpace().Append(optionsExp->ToECSql().c_str());
        }

    if (!joinedTableProperties.empty())
        info->m_joinedTableECSql = joinedTableECSQL.ToString();

    if (!parentOfJoinedTableProperties.empty())
        info->m_parentOfJoinedTableECSql = parentOfJoinedTableECSQL.ToString();

    return info;
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

    return out;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<ECSqlPrepareContext::JoinedTableInfo> ECSqlPrepareContext::JoinedTableInfo::Create(ECSqlPrepareContext& ctx, Exp const& exp, Utf8CP orignalECSQL)
    {
    std::unique_ptr<JoinedTableInfo> info = nullptr;
    if (exp.GetType() == Exp::Type::Insert)
        info = CreateForInsert(ctx, exp.GetAs<InsertStatementExp>());
    else if (exp.GetType() == Exp::Type::Update)
        info = CreateForUpdate(ctx, exp.GetAs<UpdateStatementExp>());

    if (info != nullptr)
        {
        if (info->m_joinedTableECSql.empty() && info->m_parentOfJoinedTableECSql.empty())
            {
            BeAssert(false);
            return nullptr;
            }

        info->m_originalECSql = orignalECSQL;

        ParameterSet& primary = info->m_parameterMap.GetPrimaryR();
        ParameterSet& secondary = info->m_parameterMap.GetSecondaryR();
        for (size_t i = primary.First(); i > 0 && i <= primary.Last(); ++i)
            {
            Parameter* p = const_cast<Parameter*>(primary.Find(i));
            if (p != nullptr && p->GetOrignalParameter() != nullptr)
                {
                for (size_t j = secondary.First(); j > 0 && j <= secondary.Last(); ++j)
                    {
                    Parameter* s = const_cast<Parameter*>(secondary.Find(j));
                    if (s != nullptr && p->GetOrignalParameter() == s->GetOrignalParameter())
                        {
                        p->m_isShared = s->m_isShared = true;
                        }
                    }
                }
            }

        return info;
        }

    return nullptr;
    }

//********************** ECSqlPrepareContext::JoinedTableInfo::Parameter *****************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo::Parameter const* ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Find(size_t index) const
    {
    if (index > m_parameters.size() || index == 0)
        return nullptr;

    return m_parameters.at(index - 1).get();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo::Parameter const* ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Find(Utf8StringCR name) const
    {
    for (std::unique_ptr<Parameter> const& param : m_parameters)
        {
        if (param->IsNamed() && param->GetName().EqualsIAscii(name))
            return param.get();
        }

    return nullptr;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo::Parameter const* ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Add(ParameterExp const& exp)
    {
    if (exp.IsNamedParameter())
        {
        if (ECSqlPrepareContext::JoinedTableInfo::Parameter const* r = Find(exp.GetParameterName()))
            {
            BeAssert(r->GetIndex() == exp.GetParameterIndex());
            return r;
            }
        }

    std::unique_ptr<Parameter> param = std::make_unique<Parameter>(exp.GetParameterIndex(), exp.GetParameterName(), nullptr);
    Parameter const* paramCP = param.get();
    m_parameters.push_back(std::move(param));
    return paramCP;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo::Parameter const* ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Add(Parameter const& orignalParam)
    {
    std::unique_ptr<Parameter> param = std::make_unique<Parameter>(m_parameters.size() + 1, orignalParam.GetName(), &orignalParam);
    Parameter const* paramCP = param.get();
    m_parameters.push_back(std::move(param));
    return paramCP;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::JoinedTableInfo::Parameter const* ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Add()
    {
    std::unique_ptr<Parameter> param = std::make_unique<Parameter>(m_parameters.size() + 1, Utf8String(), nullptr);
    Parameter const* paramCP = param.get();
    m_parameters.push_back(std::move(param));
    return paramCP;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::JoinedTableInfo::ParameterSet::Add(bvector<Parameter const*> const& params)
    {
    for (Parameter const* param : params)
        {
        BeAssert(param != nullptr);
        Add(*param);
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECSqlPrepareContext::JoinedTableInfo::ParameterSet::First() const
    {
    if (m_parameters.size() > 0)
        return 1;

    return 0;
    }

#endif

//****************************** ECSqlPrepareContext::ExpScope ********************
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
ECSqlType ECSqlPrepareContext::ExpScope::DetermineECSqlType(ExpCR exp) const
    {
    switch (exp.GetType())
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
            BeAssert(m_parent != nullptr && "DetermineECSqlType");
            return m_parent->GetECSqlType();
            }
        }
    }

//****************************** ECSqlPrepareContext::ExpScopeStack ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::ExpScopeStack::Push(ExpCR exp, OptionsExp const* options)
    {
    ExpScope const* parent = nullptr;
    if (Depth() > 0)
        parent = &Current();

    m_scopes.push_back(ExpScope(exp, parent, options));
    }

//************************ ECSqlPrepareContext::SelectClauseInfo **************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::SelectClauseInfo::AddProperty(PropertyMap const& propertyMap)
    {
    SearchPropertyMapVisitor typeVisitor(PropertyMap::Type::System | PropertyMap::Type::SingleColumnData);
    propertyMap.AcceptVisitor(typeVisitor);
    for (PropertyMap const* propMap : typeVisitor.Results())
        {
        Utf8String path;
        for (Utf8StringCR subPath : Split(propMap->GetAccessString(), '.'))
            {
            if (path.empty())
                path.assign(subPath);
            else
                path.append(".").append(subPath);

            m_selectClause.insert(path);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlPrepareContext::SelectClauseInfo::IsSelected(Utf8StringCR accessString) const
    {
    if (m_selectClause.find(accessString) != m_selectClause.end())
        return true;

    //these system properties are always selected (? is this true??)
    return accessString.EqualsIAscii(ECDBSYS_PROP_ECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_ECClassId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_SourceECClassId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_TargetECClassId);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
bvector<Utf8String> ECSqlPrepareContext::SelectClauseInfo::Split(Utf8StringCR accessString, Utf8Char separator)
    {
    bvector<Utf8String> output;
    Utf8String::size_type prev_pos = 0, pos = 0;
    while ((pos = accessString.find(separator, pos)) != Utf8String::npos)
        {
        output.push_back(accessString.substr(prev_pos, pos - prev_pos));
        prev_pos = ++pos;
        }

    output.push_back(accessString.substr(prev_pos, pos - prev_pos)); // Last word
    return output;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


