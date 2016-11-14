/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** SubqueryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryExp::SubqueryExp(std::unique_ptr<SelectStatementExp> selectExp) : QueryExp()
    {
    AddChild(std::move(selectExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SubqueryExp::_FindProperty(Utf8CP propertyName) const
    {
    return GetQuery()->FindProperty(propertyName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectClauseExp const* SubqueryExp::_GetSelection() const
    {
    return GetQuery()->GetSelection();
    }

//*************************** AllOrAnyExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
AllOrAnyExp::AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery)
    : BooleanExp(), m_type(type), m_operator(op)
    {
    m_operandExpIndex = AddChild(std::move(operand));
    m_subqueryExpIndex = AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void AllOrAnyExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(GetOperand()->ToECSql()).append(" ");
    ecsql.append(ExpHelper::ToSql(m_operator)).append(" ").append(ExpHelper::ToSql(m_type)).append(" ");
    ecsql.append(GetSubquery()->ToECSql());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AllOrAnyExp::_ToString() const
    {
    Utf8String str("AllOrAny [Type: ");
    str.append(ExpHelper::ToSql(m_type)).append(", Operator: ").append(ExpHelper::ToSql(m_operator)).append("]");
    return str;
    }

//************************* DerivedPropertyExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp::DerivedPropertyExp(std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias)
    : Exp(), m_columnAlias(columnAlias)
    {
    AddChild(std::move(valueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::GetName() const
    {
    Utf8StringCR columnAlias = GetColumnAlias();
    if (!columnAlias.empty())
        return columnAlias;

    if (GetExpression()->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const* propertyNameExp = static_cast<PropertyNameExp const*>(GetExpression());
        if (propertyNameExp->IsPropertyRef())
            return propertyNameExp->GetPropertyRef()->LinkedTo().GetName();

        return propertyNameExp->GetPropertyPath().ToString();
        }

    return GetExpression()->ToECSql();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+-----
Utf8StringCR DerivedPropertyExp::GetColumnAlias() const
    {
    if (!m_columnAlias.empty())
        return m_columnAlias;

    if (GetExpression()->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const* propertyNameExp = static_cast<PropertyNameExp const*>(GetExpression());
        if (propertyNameExp->IsPropertyRef())
            return propertyNameExp->GetPropertyRef()->LinkedTo().GetColumnAlias();
        }

    return m_columnAlias;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::_ToECSql() const
    {
    if (m_columnAlias.empty())
        return GetExpression()->ToECSql();

    return "(" + GetExpression()->ToECSql() + ") AS " + m_columnAlias;
    }


//****************************** FromExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FromExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    RangeClasssInfo::List classExpList;
    FindRangeClassRefs(classExpList);

    RangeClassRefExp const* classExpComparand = nullptr;
    for (RangeClasssInfo const& classExp : classExpList)
        {
        if (classExpComparand == nullptr)
            {
            classExpComparand = &classExp.GetExp();
            continue;
            }

        if (classExp.GetExp().GetId().EqualsI(classExpComparand->GetId()))
            {
            ctx.Issues().Report(ECDbIssueSeverity::Error, "Multiple occurrences of ECClass expression '%s' in the ECSQL statement. Use different aliases to distinguish them.", classExp.GetExp().ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(RangeClasssInfo::List& classRefs, RangeClasssInfo::Scope scope) const
    {
    for (auto classRef : GetChildren())
        FindRangeClassRefs(classRefs, *static_cast<ClassRefExp const*> (classRef), scope);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(RangeClasssInfo::List& classRefs, ClassRefExp const& classRef, RangeClasssInfo::Scope scope) const
    {
    switch (classRef.GetType())
        {
            case Type::ClassName:
            case Type::SubqueryRef:
                classRefs.push_back(RangeClasssInfo(static_cast<RangeClassRefExp const&>(classRef), scope)); break;
            case Type::QualifiedJoin:
            case Type::NaturalJoin:
            case Type::CrossJoin:
            case Type::ECRelationshipJoin:
            {
            JoinExp const& join = static_cast<JoinExp const&>(classRef);
            FindRangeClassRefs(classRefs, join.GetFromClassRef(), scope);
            FindRangeClassRefs(classRefs, join.GetToClassRef(), scope);
            if (classRef.GetType() == Type::ECRelationshipJoin)
                FindRangeClassRefs(classRefs, static_cast<ECRelationshipJoinExp const&>(join).GetRelationshipClass(), scope);
            break;
            }
            default:
                BeAssert(false && "Case not handled");
        };
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FromExp::TryAddClassRef(ECSqlParseContext& ctx, std::unique_ptr<ClassRefExp> classRefExp)
    {
    if (classRefExp == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RangeClasssInfo::List existingRangeClassRefs;
    FindRangeClassRefs(existingRangeClassRefs);

    RangeClasssInfo::List newRangeClassRefs;

    FindRangeClassRefs(newRangeClassRefs, *classRefExp, RangeClasssInfo::Scope::Local);
    for (RangeClasssInfo const& newRangeCRef : newRangeClassRefs)
        {
        for (auto existingRangeCRef : existingRangeClassRefs)
            {
            if (existingRangeCRef.GetExp().GetId().Equals(newRangeCRef.GetExp().GetId()))
                {
                //e.g. SELECT * FROM FOO a, GOO a
                ctx.Issues().Report(ECDbIssueSeverity::Error, "Duplicate class name / alias '%s' in FROM or JOIN clause", newRangeCRef.GetExp().GetId().c_str());
                return ERROR;
                }
            }
        }

    AddChild(move(classRefExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
RangeClasssInfo::List FromExp::FindRangeClassRefExpressions() const
    {
    RangeClasssInfo::List rangeClassRefs;
    FindRangeClassRefs(rangeClassRefs, RangeClasssInfo::Scope::Local);
    auto  isSubQuery = [] (Exp const& start, SingleSelectStatementExp const* end)
        {
        if (end == nullptr)
            return true;

        Exp const* c = &start;
        do
            {
            c = c->GetParent();
            if (c == nullptr || c == end)
                return false;

            if (c->GetType() == Exp::Type::FromClause)
                return true;

            } while (true);
        };


    SingleSelectStatementExp const* cur = static_cast<SingleSelectStatementExp const*>(FindParent(Exp::Type::SingleSelect));
    Exp const* old = this;
    bool isTableSubQuery = isSubQuery(*old, cur);
    while (cur != nullptr && !isTableSubQuery)
        {        
        old = cur;
        cur = static_cast<SingleSelectStatementExp const*>(cur->FindParent(Exp::Type::SingleSelect));
        isTableSubQuery = isSubQuery(*old, cur);
        if (cur != nullptr && !isTableSubQuery)
            {
            if (cur->GetFrom() != this)
                cur->GetFrom()->FindRangeClassRefs(rangeClassRefs, RangeClasssInfo::Scope::Inherited);
            }
        }

    return rangeClassRefs;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FromExp::_ToECSql() const
    {
    Utf8String tmp = "FROM ";
    bool isFirstItem = true;
    for (Exp const* classRefExp : GetChildren())
        {
        if (!isFirstItem)
            tmp.append(", ");

        tmp.append(classRefExp->ToECSql());
        isFirstItem = false;
        }

    return tmp;
    }


//****************************** GroupByExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
GroupByExp::GroupByExp(std::unique_ptr<ValueExpListExp> groupingValueListExp) : Exp()
    {
    m_groupingValueListExpIndex = AddChild(std::move(groupingValueListExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
ValueExpListExp const* GroupByExp::GetGroupingValueListExp() const
    {
    return GetChild<ValueExpListExp>(m_groupingValueListExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus GroupByExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    BeAssert(GetGroupingValueListExp() != nullptr);

    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    ValueExpListExp const* groupingValueListExp = GetGroupingValueListExp();
    const size_t listCount = groupingValueListExp->GetChildrenCount();
    for (size_t i = 0; i < listCount; i++)
        {
        ValueExp const* groupingValueExp = groupingValueListExp->GetValueExp(i);
        const Exp::Type expType = groupingValueExp->GetType();
        ECSqlTypeInfo const& typeInfo = groupingValueExp->GetTypeInfo();
        if (expType == Exp::Type::Parameter || groupingValueExp->IsConstant() || typeInfo.IsGeometry() || typeInfo.IsArray())
            {
            ctx.Issues().Report(ECDbIssueSeverity::Error, "Invalid expression '%s' in GROUP BY: Parameters, constants and geometry and array properties are not supported.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GroupByExp::_ToECSql() const
    {
    Utf8String str("GROUP BY ");
    str.append(GetGroupingValueListExp()->ToECSql());
    return str;
    }

//****************************** HavingExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
HavingExp::HavingExp(std::unique_ptr<BooleanExp> searchConditionExp) : Exp()
    {
    m_searchConditionExpIndex = AddChild(std::move(searchConditionExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
BooleanExp const* HavingExp::GetSearchConditionExp() const
    {
    return GetChild<BooleanExp>(m_searchConditionExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String HavingExp::_ToECSql() const
    {
    Utf8String str("HAVING ");
    str.append(GetSearchConditionExp()->ToECSql());
    return str;
    }


//****************************** LimitOffsetExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::LimitOffsetExp(std::unique_ptr<ValueExp> limitExp, std::unique_ptr<ValueExp> offsetExp)
    : Exp(), m_offsetExpIndex(UNSET_CHILDINDEX)
    {
    BeAssert(limitExp != nullptr);
    m_limitExpIndex = AddChild(std::move(limitExp));

    if (offsetExp != nullptr)
        m_offsetExpIndex = static_cast<int> (AddChild(std::move(offsetExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String LimitOffsetExp::_ToECSql() const
    {
    Utf8String str("LIMIT ");
    str.append(GetLimitExp()->ToECSql());
    if (HasOffset())
        str.append(" OFFSET ").append(GetOffsetExp()->ToECSql());

    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::FinalizeParseStatus LimitOffsetExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    BeAssert(GetLimitExp() != nullptr);

    switch (mode)
        {
            case Exp::FinalizeParseMode::BeforeFinalizingChildren:
                return FinalizeParseStatus::NotCompleted;

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
            {
            if (!IsValidChildExp(*GetLimitExp()))
                {
                ctx.Issues().Report(ECDbIssueSeverity::Error, "Invalid expression '%s'. LIMIT expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            if (HasOffset() && !IsValidChildExp(*GetOffsetExp()))
                {
                ctx.Issues().Report(ECDbIssueSeverity::Error, "Invalid expression '%s'. OFFSET expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            return FinalizeParseStatus::Completed;
            }

            default:
                BeAssert(false);
                return FinalizeParseStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitOffsetExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //limit offset operands are always integral
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Long));
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool LimitOffsetExp::IsValidChildExp(ValueExp const& exp)
    {
    //parameter exp get their type later, so ignore them here
    if (exp.IsParameterExp())
        return true;

    // we allow non-integral numeric expressions as well, assuming that the underlying db will handle casting
    return exp.GetTypeInfo().IsNumeric();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* LimitOffsetExp::GetLimitExp() const
    {
    return GetChild<ValueExp>(m_limitExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool LimitOffsetExp::HasOffset() const
    {
    return m_offsetExpIndex >= 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* LimitOffsetExp::GetOffsetExp() const
    {
    if (!HasOffset())
        return nullptr;

    return GetChild<ValueExp>((size_t) m_offsetExpIndex);
    }


//************************* OrderBySpecExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
OrderBySpecExp::OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction)
    : m_direction(direction)
    {
    AddChild(move(expr));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
OrderBySpecExp::FinalizeParseStatus OrderBySpecExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto const& typeInfo = GetSortExpression()->GetTypeInfo();
    if (!typeInfo.IsPrimitive() || typeInfo.IsPoint() || typeInfo.IsGeometry())
        {
        ctx.Issues().Report(ECDbIssueSeverity::Error, "Invalid expression '%s' in ORDER BY: Points, Geometries, structs and arrays are not supported.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
Utf8String OrderBySpecExp::_ToECSql() const
    {
    auto ecsql = GetSortExpression()->ToECSql();
    AppendSortDirection(ecsql, true);
    return ecsql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String OrderBySpecExp::_ToString() const
    {
    Utf8String str("OrderBySpec [SortDirection: ");
    AppendSortDirection(str, false);
    str.append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void OrderBySpecExp::AppendSortDirection(Utf8String& str, bool addLeadingBlank) const
    {
    if (addLeadingBlank && m_direction != SortDirection::NotSpecified)
        str.append(" ");

    switch (m_direction)
        {
            case SortDirection::Ascending:
                str.append("ASC");
                break;

            case SortDirection::Descending:
                str.append("DESC");
                break;

            default:
                break;
        }
    }


//*************************** SelectClauseExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SelectClauseExp::ReplaceAsteriskExpressions(RangeClasssInfo::List const& rangeClassRefs)
    {
    std::vector<DerivedPropertyExp const*> propertyNameExpList;
    for (auto childExp : GetChildren())
        {
        DerivedPropertyExp const* derivedPropExp = static_cast<DerivedPropertyExp const*> (childExp);
        if (derivedPropExp->GetExpression()->GetType() == Exp::Type::PropertyName)
            propertyNameExpList.push_back(derivedPropExp);
        }

    for (DerivedPropertyExp const* propertyNameExp : propertyNameExpList)
        {
        PropertyNameExp const* innerExp = static_cast<PropertyNameExp const*>(propertyNameExp->GetExpression());
        if (Exp::IsAsteriskToken(innerExp->GetPropertyName()))
            {
            if (SUCCESS != ReplaceAsteriskExpression(*propertyNameExp, rangeClassRefs))
                return ERROR;

            continue;
            }

        //WIP_ECSQL: Why is the alias the first entry in the prop path? The alias should be the root class, but not an entry in the prop path
        //WIP_ECSQL: What about SELECT structProp.* from FOO?
        PropertyPath const& propertyPath = innerExp->GetPropertyPath();
        //case: SELECT a.* from FOO a
        if (propertyPath.Size() > 1 && Exp::IsAsteriskToken(propertyPath[1].GetPropertyName()))
            {
            Utf8CP alias = propertyPath[0].GetPropertyName();
            //Find class ref that matches the alias and replace the asterisk by just the props of that class ref
            for (RangeClasssInfo const& classRef : rangeClassRefs)
                {
                if (classRef.GetExp().GetId().Equals(alias))
                    {
                    RangeClasssInfo::List classRefList;
                    classRefList.push_back(classRef);
                    auto stat = ReplaceAsteriskExpression(*propertyNameExp, classRefList);
                    if (stat != SUCCESS)
                        return stat;

                    break;
                    }
                }
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SelectClauseExp::ReplaceAsteriskExpression(DerivedPropertyExp const& asteriskExp, RangeClasssInfo::List const& rangeClassRefs)
    {
    std::vector<std::unique_ptr<Exp>> derivedPropExpList;
    auto addDelegate = [&derivedPropExpList] (std::unique_ptr<PropertyNameExp>& propNameExp)
        {
        derivedPropExpList.push_back(std::unique_ptr<Exp>(new DerivedPropertyExp(std::move(propNameExp), nullptr)));
        };

    for (RangeClasssInfo const& classRef : rangeClassRefs)
        classRef.GetExp().CreatePropertyNameExpList(addDelegate);

    if (!GetChildrenR().Replace(asteriskExp, derivedPropExpList))
        {
        BeAssert(false && "SelectClauseExp::ReplaceAsteriskExpression did not find an asterisk expression unexpectedly.");
        return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SelectClauseExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto finalizeParseArgs = ctx.GetFinalizeParseArg();
        BeAssert(finalizeParseArgs != nullptr && "SelectClauseExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
        RangeClasssInfo::List const* rangeClassRefList = static_cast<RangeClasssInfo::List const*> (finalizeParseArgs);
        BeAssert(rangeClassRefList != nullptr);
        const auto stat = ReplaceAsteriskExpressions(*rangeClassRefList);
        if (stat != SUCCESS)
            {
            ctx.Issues().Report(ECDbIssueSeverity::Error, "Asterisk replacement in select clause failed unexpectedly.");
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SelectClauseExp::_ToECSql() const
    {
    Utf8String tmp;
    bool isFirstItem = true;
    for (auto childExp : GetChildren())
        {
        if (!isFirstItem)
            tmp.append(", ");

        tmp.append(childExp->ToECSql());
        isFirstItem = false;
        }

    return tmp;
    }

//*************************** SingleSelectStatementExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
SingleSelectStatementExp::SingleSelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp> selection, std::unique_ptr<FromExp> from, std::unique_ptr<WhereExp> where, std::unique_ptr<OrderByExp> orderby, std::unique_ptr<GroupByExp> groupby, std::unique_ptr<HavingExp> having, std::unique_ptr<LimitOffsetExp> limitOffsetExp, std::unique_ptr<OptionsExp> optionsExp)
    : QueryExp(), m_selectionType(selectionType), m_whereClauseIndex(UNSET_CHILDINDEX), m_orderByClauseIndex(UNSET_CHILDINDEX), m_groupByClauseIndex(UNSET_CHILDINDEX), m_havingClauseIndex(UNSET_CHILDINDEX), m_limitOffsetClauseIndex(UNSET_CHILDINDEX), m_optionsClauseIndex(UNSET_CHILDINDEX)
    {
    //WARNING: Do not change the order of following
    m_fromClauseIndex = AddChild(std::move(from));
    m_selectClauseIndex = AddChild(std::move(selection));

    if (where != nullptr)
        m_whereClauseIndex = (int) AddChild(std::move(where));

    if (orderby != nullptr)
        m_orderByClauseIndex = (int) AddChild(std::move(orderby));

    if (groupby != nullptr)
        m_groupByClauseIndex = (int) AddChild(std::move(groupby));

    if (having != nullptr)
        m_havingClauseIndex = (int) AddChild(std::move(having));

    if (limitOffsetExp != nullptr)
        m_limitOffsetClauseIndex = (int) AddChild(std::move(limitOffsetExp));

    if (optionsExp != nullptr)
        m_optionsClauseIndex = (int) AddChild(std::move(optionsExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SingleSelectStatementExp::_FindProperty(Utf8CP propertyName) const
    {
    for (auto selectClauseExp : GetSelection()->GetChildren())
        {
        auto derivedPropertyExp = static_cast<DerivedPropertyExp const*> (selectClauseExp);
        if (!derivedPropertyExp->GetColumnAlias().empty())
            {
            if (derivedPropertyExp->GetColumnAlias().Equals(propertyName))
                return derivedPropertyExp;
            }
        else
            {
            auto expr = derivedPropertyExp->GetExpression();
            if (expr->GetType() == Type::PropertyName)
                {
                PropertyNameExp const* propertyNameExp = static_cast<PropertyNameExp const*>(expr);
                if (strcmp(propertyNameExp->GetPropertyName(), propertyName) == 0)
                    return derivedPropertyExp;
                }
            }
        }
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SingleSelectStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        m_finalizeParsingArgCache = GetFrom()->FindRangeClassRefExpressions();
        ctx.PushFinalizeParseArg(&m_finalizeParsingArgCache);
        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        ctx.PopFinalizeParseArg();
        m_finalizeParsingArgCache.clear();
        return FinalizeParseStatus::Completed;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SingleSelectStatementExp::_ToString() const
    {
    Utf8String str("Select [Modifier: ");
    str.append(ExpHelper::ToSql(m_selectionType)).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SingleSelectStatementExp::_ToECSql() const
    {
    Utf8String tmp("SELECT ");

    Utf8String selectionType = ExpHelper::ToSql(GetSelectionType());
    if (!selectionType.empty())
        tmp.append(selectionType).append(" ");

    tmp.append(GetSelection()->ToECSql()).append(" ").append(GetFrom()->ToECSql());

    if (GetWhere() != nullptr)
        tmp.append(" ").append(GetWhere()->ToECSql());

    if (GetGroupBy() != nullptr)
        tmp.append(" ").append(GetGroupBy()->ToECSql());

    if (GetOrderBy() != nullptr)
        tmp.append(" ").append(GetOrderBy()->ToECSql());

    if (GetHaving() != nullptr)
        tmp.append(" ").append(GetHaving()->ToECSql());

    if (GetLimitOffset() != nullptr)
        tmp.append(" ").append(GetLimitOffset()->ToECSql());

    if (GetOptions() != nullptr)
        tmp.append(" ").append(GetOptions()->ToECSql());

    return tmp;
    }

//****************************** SubqueryExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryExp::_ToECSql() const
    {
    return "(" + GetQuery()->ToECSql() + ")";
    }


//****************************** SubqueryRefExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryRefExp::SubqueryRefExp(std::unique_ptr<SubqueryExp> subquery, Utf8StringCR alias, bool isPolymorphic)
    : RangeClassRefExp(isPolymorphic)
    {
    SetAlias(alias);
    AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SubqueryRefExp::_CreatePropertyNameExpList(std::function<void(std::unique_ptr<PropertyNameExp>&)> addDelegate) const
    {
    for (Exp const* expr : GetSubquery()->GetSelection()->GetChildren())
        {
        DerivedPropertyExp const* selectClauseItemExp = static_cast<DerivedPropertyExp const*> (expr);
        std::unique_ptr<PropertyNameExp> propNameExp(new PropertyNameExp(*this, *selectClauseItemExp));
        addDelegate(propNameExp);
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryRefExp::_ToECSql() const
    {
    return (!IsPolymorphic() ? "ONLY " : "") + GetSubquery()->ToString() + (GetAlias().empty() ? "" : " AS " + GetAlias());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryRefExp::_ToString() const
    {
    Utf8String str("SubqueryRef [");
    str.append(GetId()).append("]");
    return str;
    }


//****************************** SubqueryTestExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryTestExp::SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery)
    : BooleanExp(), m_op(op)
    {
    AddChild(move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryTestExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(ExpHelper::ToSql(m_op)).append(" ").append(GetQuery()->ToECSql());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryTestExp::_ToString() const
    {
    Utf8String str("SubqueryTest [Operator: ");
    str.append(ExpHelper::ToSql(m_op)).append("]");
    return str;
    }



//****************************** SubqueryValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryValueExp::SubqueryValueExp(std::unique_ptr<SubqueryExp> subquery)
    : ValueExp()
    {
    SetHasParentheses(); //subquery value exp always wrapped in parentheses
    AddChild(move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SubqueryValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto selectClauseExp = GetQuery()->GetSelection();

    if (selectClauseExp->GetChildren().size() != 1)
        {
        ctx.Issues().Report(ECDbIssueSeverity::Error, "Subquery must return exactly one column %s.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    SetTypeInfo(selectClauseExp->GetChildren().Get<DerivedPropertyExp>(0)->GetExpression()->GetTypeInfo());

    return FinalizeParseStatus::Completed;
    }


//****************************** SelectStatementExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SelectStatementExp::_ToECSql() const
    {
    if (IsCompound())
        return GetFirstStatement().ToECSql() + " " + OperatorToString(m_operator) + (m_isAll ? " ALL " : " ") + GetRhsStatement()->ToECSql();

    return GetFirstStatement().ToECSql();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs)
    :m_isAll(false), m_operator(CompoundOperator::None), m_rhsSelectStatementExpIndex(UNSET_CHILDINDEX)
    {
    BeAssert(lhs != nullptr);
    m_firstSingleSelectStatementExpIndex = AddChild(std::move(lhs));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs, CompoundOperator op, bool isAll, std::unique_ptr<SelectStatementExp> rhs)
    :m_isAll(isAll), m_operator(op)
    {
    BeAssert(lhs != nullptr);
    BeAssert(rhs != nullptr);
    BeAssert(op != CompoundOperator::None);

    m_firstSingleSelectStatementExpIndex = AddChild(std::move(lhs));
    m_rhsSelectStatementExpIndex = (int) AddChild(std::move(rhs));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp const* SelectStatementExp::GetRhsStatement() const
    {
    if (IsCompound())
        return GetChild<SelectStatementExp>((size_t) m_rhsSelectStatementExpIndex);

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP SelectStatementExp::OperatorToString(CompoundOperator op)
    {
    switch (op)
        {
            case CompoundOperator::Union:
                return "UNION";
            case CompoundOperator::Intersect:
                return "INTERSECT";
            case CompoundOperator::Except:
                return "EXCEPT";
            default:
                BeAssert(false && "Programmer error");
                return nullptr;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

