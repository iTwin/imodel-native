/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** AllOrAnyExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
AllOrAnyExp::AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery)
    : BooleanExp(Type::AllOrAny), m_type(type), m_operator(op)
    {
    m_operandExpIndex = AddChild(std::move(operand));
    m_subqueryExpIndex = AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void AllOrAnyExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetOperand()).AppendToECSql(" ");
    ctx.AppendToECSql(ExpHelper::ToSql(m_operator)).AppendToECSql(" ").AppendToECSql(ExpHelper::ToSql(m_type)).AppendToECSql(" ");
    ctx.AppendToECSql(*GetSubquery());

    if (HasParentheses())
        ctx.AppendToECSql(")");
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
    : Exp(Type::DerivedProperty), m_columnAlias(columnAlias)
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
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef())
            return propertyNameExp.GetPropertyRef()->LinkedTo().GetName();

        return propertyNameExp.GetPropertyPath().ToString();
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
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef())
            return propertyNameExp.GetPropertyRef()->LinkedTo().GetColumnAlias();
        }

    return m_columnAlias;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void DerivedPropertyExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (m_columnAlias.empty())
        {
        ctx.AppendToECSql(*GetExpression());
        return;
        }

    ctx.AppendToECSql("(").AppendToECSql(*GetExpression()).AppendToECSql(") AS ").AppendToECSql(m_columnAlias);
    }


//****************************** FromExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FromExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    std::vector<RangeClassInfo> classExpList;
    FindRangeClassRefs(classExpList);

    RangeClassRefExp const* classExpComparand = nullptr;
    for (RangeClassInfo const& classExp : classExpList)
        {
        if (classExpComparand == nullptr)
            {
            classExpComparand = &classExp.GetExp();
            continue;
            }

        if (classExp.GetExp().GetId().EqualsI(classExpComparand->GetId()))
            {
            ctx.Issues().ReportV("Multiple occurrences of ECClass expression '%s' in the ECSQL statement. Use different aliases to distinguish them.", classExp.GetExp().ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(std::vector<RangeClassInfo>& classRefs, RangeClassInfo::Scope scope) const
    {
    for (Exp const* classRef : GetChildren())
        FindRangeClassRefs(classRefs, classRef->GetAs<ClassRefExp>(), scope);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(std::vector<RangeClassInfo>& classRefs, ClassRefExp const& classRef, RangeClassInfo::Scope scope) const
    {
    switch (classRef.GetType())
        {
            case Type::ClassName:
            case Type::SubqueryRef:
                classRefs.push_back(RangeClassInfo(classRef.GetAs<RangeClassRefExp>(), scope)); break;
            case Type::QualifiedJoin:
            case Type::NaturalJoin:
            case Type::CrossJoin:
            case Type::ECRelationshipJoin:
                {
                JoinExp const& join = classRef.GetAs<JoinExp>();
                FindRangeClassRefs(classRefs, join.GetFromClassRef(), scope);
                FindRangeClassRefs(classRefs, join.GetToClassRef(), scope);
                if (classRef.GetType() == Type::ECRelationshipJoin)
                    FindRangeClassRefs(classRefs, join.GetAs<ECRelationshipJoinExp>().GetRelationshipClassNameExp(), scope);

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

    std::vector<RangeClassInfo> existingRangeClassRefs;
    FindRangeClassRefs(existingRangeClassRefs);

    std::vector<RangeClassInfo> newRangeClassRefs;
    FindRangeClassRefs(newRangeClassRefs, *classRefExp, RangeClassInfo::Scope::Local);
    for (RangeClassInfo const& newRangeCRef : newRangeClassRefs)
        {
        for (auto existingRangeCRef : existingRangeClassRefs)
            {
            if (existingRangeCRef.GetExp().GetId().Equals(newRangeCRef.GetExp().GetId()))
                {
                //e.g. SELECT * FROM FOO a, GOO a
                ctx.Issues().ReportV("Duplicate class name / alias '%s' in FROM or JOIN clause", newRangeCRef.GetExp().GetId().c_str());
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
std::vector<RangeClassInfo> FromExp::FindRangeClassRefExpressions() const
    {
    std::vector<RangeClassInfo> rangeClassRefs;
    FindRangeClassRefs(rangeClassRefs, RangeClassInfo::Scope::Local);
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


    Exp const* parent = FindParent(Exp::Type::SingleSelect);
    SingleSelectStatementExp const* cur = parent == nullptr ? nullptr : parent->GetAsCP<SingleSelectStatementExp>();
    Exp const* old = this;
    bool isTableSubQuery = isSubQuery(*old, cur);
    while (cur != nullptr && !isTableSubQuery)
        {        
        old = cur;
        parent = cur->FindParent(Exp::Type::SingleSelect);
        cur = parent == nullptr ? nullptr : parent->GetAsCP<SingleSelectStatementExp>();
        isTableSubQuery = isSubQuery(*old, cur);
        if (cur != nullptr && !isTableSubQuery)
            {
            if (cur->GetFrom() != this)
                cur->GetFrom()->FindRangeClassRefs(rangeClassRefs, RangeClassInfo::Scope::Inherited);
            }
        }

    return rangeClassRefs;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("FROM ");
    bool isFirstItem = true;
    for (Exp const* classRefExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*classRefExp);
        isFirstItem = false;
        }
    }


//****************************** GroupByExp *****************************************
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
        if (expType == Exp::Type::Parameter || groupingValueExp->IsConstant() || typeInfo.IsNavigation())
            {
            ctx.Issues().ReportV("Invalid expression '%s' in GROUP BY: Parameters, constants, and navigation properties are not supported.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//****************************** LimitOffsetExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::LimitOffsetExp(std::unique_ptr<ValueExp> limitExp, std::unique_ptr<ValueExp> offsetExp) : Exp(Type::LimitOffset)
    {
    BeAssert(limitExp != nullptr);
    m_limitExpIndex = AddChild(std::move(limitExp));

    if (offsetExp != nullptr)
        m_offsetExpIndex =(int) AddChild(std::move(offsetExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void LimitOffsetExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("LIMIT ").AppendToECSql(*GetLimitExp());
    if (HasOffset())
        ctx.AppendToECSql(" OFFSET ").AppendToECSql(*GetOffsetExp());
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
                ctx.Issues().ReportV("Invalid expression '%s'. LIMIT expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            if (HasOffset() && !IsValidChildExp(*GetOffsetExp()))
                {
                ctx.Issues().ReportV("Invalid expression '%s'. OFFSET expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
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
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Long));
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
ValueExp const* LimitOffsetExp::GetOffsetExp() const
    {
    if (!HasOffset())
        return nullptr;

    return GetChild<ValueExp>((size_t) m_offsetExpIndex);
    }

//************************* OrderByExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void OrderByExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("ORDER BY ");
    bool isFirstItem = true;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*childExp);
        isFirstItem = false;
        }
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2017
//+---------------+---------------+---------------+---------------+---------------+------
ComputedExp const* OrderByExp::FindIncompatibleOrderBySpecExpForUnion() const
    {
    for (auto children : GetChildren())
        {
        ComputedExp const* exp = children->GetAs<OrderBySpecExp>().GetSortExpression();
        if (exp->GetType() != Exp::Type::PropertyName && exp->GetType() != Exp::Type::FunctionCall)
            return exp;
        }

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2017
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus OrderByExp::_FinalizeParsing(ECSqlParseContext& parseContext, FinalizeParseMode parseMode) 
    {
    if (parseMode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        Exp const* exp = this;
        while (exp->GetParent())
            {
            exp = exp->GetParent();

            if (exp->GetType() == Exp::Type::Select)
                m_unionClauses.push_back(&exp->GetAsCP<SelectStatementExp>()->GetFirstStatement());
            else if (exp->GetType() == Exp::Type::SingleSelect)
                {
                continue;
                }
            else //SubQuery or other parent should not included here
                {
                m_unionClauses.clear();
                break;
                }
            }

        if (m_unionClauses.size() > 1)
            {
            if (ComputedExp const* exp = FindIncompatibleOrderBySpecExpForUnion())
                {
                parseContext.Issues().ReportV("'%s' ORDER BY term does not match any column in the result set.", exp->ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            parseContext.PushArg(std::make_unique<ECSqlParseContext::UnionOrderByArg>(m_unionClauses));
            }
        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        if (m_unionClauses.size() > 1)
            {
            parseContext.PopArg();
            m_unionClauses.clear();
            }
        }

    return FinalizeParseStatus::Completed;   
    }

//************************* OrderBySpecExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
OrderBySpecExp::FinalizeParseStatus OrderBySpecExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    ECSqlTypeInfo const& typeInfo = GetSortExpression()->GetTypeInfo();
    if (!typeInfo.IsPrimitive() || typeInfo.IsPoint() || typeInfo.IsGeometry())
        {
        ctx.Issues().ReportV("Invalid expression '%s' in ORDER BY: Points, Geometries, navigation properties, structs and arrays are not supported.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void OrderBySpecExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(*GetSortExpression());

    switch (m_direction)
        {
            case SortDirection::Ascending:
                ctx.AppendToECSql(" ASC");
                break;

            case SortDirection::Descending:
                ctx.AppendToECSql(" DESC");
                break;

            default:
                break;
        }   
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String OrderBySpecExp::_ToString() const
    {
    Utf8String str("OrderBySpec [SortDirection: ");

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

    str.append("]");
    return str;
    }


//*************************** SelectClauseExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SelectClauseExp::ReplaceAsteriskExpressions(ECSqlParseContext const& ctx, std::vector<RangeClassInfo> const& rangeClassRefs)
    {
    std::vector<DerivedPropertyExp const*> propertyNameExpList;
    for (Exp const* childExp : GetChildren())
        {
        DerivedPropertyExp const& derivedPropExp = childExp->GetAs<DerivedPropertyExp> ();
        if (derivedPropExp.GetExpression()->GetType() == Exp::Type::PropertyName)
            propertyNameExpList.push_back(&derivedPropExp);
        }

    for (DerivedPropertyExp const* propertyNameExp : propertyNameExpList)
        {
        PropertyNameExp const& innerExp = propertyNameExp->GetExpression()->GetAs<PropertyNameExp>();
        if (Exp::IsAsteriskToken(innerExp.GetPropertyName()))
            {
            if (SUCCESS != ReplaceAsteriskExpression(ctx, *propertyNameExp, rangeClassRefs))
                return ERROR;

            continue;
            }

        //WIP_ECSQL: Why is the alias the first entry in the prop path? The alias should be the root class, but not an entry in the prop path
        //WIP_ECSQL: What about SELECT structProp.* from FOO?
        PropertyPath const& propertyPath = innerExp.GetPropertyPath();
        //case: SELECT a.* from FOO a
        if (propertyPath.Size() > 1 && Exp::IsAsteriskToken(propertyPath[1].GetName()))
            {
            Utf8StringCR alias = propertyPath[0].GetName();
            //Find class ref that matches the alias and replace the asterisk by just the props of that class ref
            for (RangeClassInfo const& classRef : rangeClassRefs)
                {
                if (classRef.GetExp().GetId().Equals(alias))
                    {
                    std::vector<RangeClassInfo> classRefList;
                    classRefList.push_back(classRef);
                    if (SUCCESS != ReplaceAsteriskExpression(ctx, *propertyNameExp, classRefList))
                        return ERROR;

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
BentleyStatus SelectClauseExp::ReplaceAsteriskExpression(ECSqlParseContext const& ctx, DerivedPropertyExp const& asteriskExp, std::vector<RangeClassInfo> const& rangeClassRefs)
    {
    std::vector<std::unique_ptr<DerivedPropertyExp>> derivedPropExpList;
    for (RangeClassInfo const& classRef : rangeClassRefs)
        classRef.GetExp().ExpandSelectAsterisk(derivedPropExpList, ctx);

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
        if (!GetParent()->GetAs<SingleSelectStatementExp>().IsRowConstructor())
            {
            BeAssert(ctx.CurrentArg() != nullptr && "SelectClauseExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
            BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::RangeClass && "Expecting range class");
            ECSqlParseContext::RangeClassArg const* arg = static_cast<ECSqlParseContext::RangeClassArg const*>(ctx.CurrentArg());
            if (SUCCESS != ReplaceAsteriskExpressions(ctx, arg->GetRangeClassInfos()))
                {
                ctx.Issues().Report("Asterisk replacement in select clause failed unexpectedly.");
                return FinalizeParseStatus::Error;
                }
            }
        }
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void SelectClauseExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    bool isFirstItem = true;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*childExp);
        isFirstItem = false;
        }
    }

//*************************** SingleSelectStatementExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
SingleSelectStatementExp::SingleSelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp> selection, std::unique_ptr<FromExp> from, std::unique_ptr<WhereExp> where, std::unique_ptr<OrderByExp> orderby, std::unique_ptr<GroupByExp> groupby, std::unique_ptr<HavingExp> having, std::unique_ptr<LimitOffsetExp> limitOffsetExp, std::unique_ptr<OptionsExp> optionsExp)
    : QueryExp(Type::SingleSelect), m_selectionType(selectionType)
    {
    //WARNING: Do not change the order of following
    m_fromClauseIndex = (int) AddChild(std::move(from));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2017
//+---------------+---------------+---------------+---------------+---------------+------
SingleSelectStatementExp::SingleSelectStatementExp(std::vector<std::unique_ptr<ValueExp>>& valueExpList) : QueryExp(Type::SingleSelect)
    {
    std::unique_ptr<SelectClauseExp> selectClauseExp = std::make_unique<SelectClauseExp>();
    int expIx = 0;
    for (std::unique_ptr<ValueExp>& valueExp : valueExpList)
        {
        expIx++;
        std::unique_ptr<DerivedPropertyExp> derivedPropertyExp = std::make_unique<DerivedPropertyExp>(std::move(valueExp), nullptr);
        selectClauseExp->AddProperty(std::move(derivedPropertyExp));
        }

    m_selectClauseIndex = AddChild(std::move(selectClauseExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SingleSelectStatementExp::_FindProperty(Utf8CP propertyName) const
    {
    for (Exp const* selectClauseExp : GetSelection()->GetChildren())
        {
        DerivedPropertyExp const& derivedPropertyExp = selectClauseExp->GetAs<DerivedPropertyExp>();
        if (!derivedPropertyExp.GetColumnAlias().empty())
            {
            if (derivedPropertyExp.GetColumnAlias().Equals(propertyName))
                return &derivedPropertyExp;
            }
        else
            {
            ValueExp const* expr = derivedPropertyExp.GetExpression();
            if (expr->GetType() == Type::PropertyName)
                {
                PropertyNameExp const& propertyNameExp = expr->GetAs<PropertyNameExp>();
                if (propertyNameExp.GetPropertyName().Equals(propertyName))
                    return &derivedPropertyExp;
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
        if (!IsRowConstructor())
            {
            m_rangeClassRefExpCache = GetFrom()->FindRangeClassRefExpressions();
            ctx.PushArg(std::make_unique<ECSqlParseContext::RangeClassArg>(m_rangeClassRefExpCache));
            }

        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        if (!IsRowConstructor())
            {
            ctx.PopArg();
            m_rangeClassRefExpCache.clear();
            }
        }

    return FinalizeParseStatus::Completed;
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
void SingleSelectStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (IsRowConstructor())
        {        
        ctx.AppendToECSql("VALUES (").AppendToECSql(*GetSelection()).AppendToECSql(")");
        return;
        }

    ctx.AppendToECSql("SELECT ");

    Utf8String selectionType = ExpHelper::ToSql(GetSelectionType());
    if (!selectionType.empty())
        ctx.AppendToECSql(selectionType).AppendToECSql(" ");

    ctx.AppendToECSql(*GetSelection()).AppendToECSql(" ").AppendToECSql(*GetFrom());

    if (GetWhere() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetWhere());

    if (GetGroupBy() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetGroupBy());

    if (GetOrderBy() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetOrderBy());

    if (GetHaving() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetHaving());

    if (GetLimitOffset() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetLimitOffset());

    if (GetOptions() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetOptions());
    }


//*************************** SubqueryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryExp::SubqueryExp(std::unique_ptr<SelectStatementExp> selectExp) : QueryExp(Type::Subquery)
    {
    AddChild(std::move(selectExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SubqueryExp::_FindProperty(Utf8CP propertyName) const { return GetQuery()->FindProperty(propertyName); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectClauseExp const* SubqueryExp::_GetSelection() const { return GetQuery()->GetSelection(); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp const* SubqueryExp::GetQuery() const { return GetChild<SelectStatementExp>(0); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryExp::_ToECSql(ECSqlRenderContext& ctx) const { ctx.AppendToECSql(*GetQuery()); }

//****************************** SubqueryRefExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryRefExp::SubqueryRefExp(std::unique_ptr<SubqueryExp> subquery, Utf8CP alias, bool isPolymorphic)
    : RangeClassRefExp(Type::SubqueryRef, isPolymorphic)
    {
    if (!Utf8String::IsNullOrEmpty(alias))
        SetAlias(alias);

    AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryRefExp::_ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const
    {
    for (Exp const* expr : GetSubquery()->GetSelection()->GetChildren())
        {
        DerivedPropertyExp const& selectClauseItemExp = expr->GetAs<DerivedPropertyExp>();
        std::unique_ptr<PropertyNameExp> propNameExp = std::make_unique<PropertyNameExp>(ctx, *this, selectClauseItemExp);
        expandedSelectClauseItemList.push_back(std::make_unique<DerivedPropertyExp>(std::move(propNameExp), nullptr));
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryRefExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (!IsPolymorphic())
        ctx.AppendToECSql("ONLY ");
    
    ctx.AppendToECSql(*GetSubquery());
    
    if (!GetAlias().empty())
        ctx.AppendToECSql(" AS ").AppendToECSql(GetAlias());
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
    : BooleanExp(Type::SubqueryTest), m_op(op)
    {
    AddChild(move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryTestExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(ExpHelper::ToSql(m_op)).AppendToECSql(" ").AppendToECSql(*GetQuery());

    if (HasParentheses())
        ctx.AppendToECSql(")");
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
    : ValueExp(Type::SubqueryValue)
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
        ctx.Issues().ReportV("Subquery must return exactly one column %s.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    SetTypeInfo(selectClauseExp->GetChildren().Get<DerivedPropertyExp>(0)->GetExpression()->GetTypeInfo());
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetQuery());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//****************************** SelectStatementExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void SelectStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(GetFirstStatement());

    if (!IsCompound())
        return;

    ctx.AppendToECSql(" ").AppendToECSql(OperatorToString(m_operator));

    if (m_isAll)
        ctx.AppendToECSql(" ALL ");
    
    ctx.AppendToECSql(*GetRhsStatement());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs)
    : QueryExp(Type::Select), m_isAll(false), m_operator(CompoundOperator::None), m_rhsSelectStatementExpIndex(UNSET_CHILDINDEX)
    {
    BeAssert(lhs != nullptr);
    m_firstSingleSelectStatementExpIndex = AddChild(std::move(lhs));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs, CompoundOperator op, bool isAll, std::unique_ptr<SelectStatementExp> rhs)
    :QueryExp(Type::Select), m_isAll(isAll), m_operator(op)
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
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2017
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SelectStatementExp::_FinalizeParsing(ECSqlParseContext& parseContext, FinalizeParseMode parseMode)
    {
    if (GetRhsStatement() != nullptr && GetFirstStatement().GetOrderBy() != nullptr)
        {
        parseContext.Issues().Report("ORDER BY clause must not be followed by UNION clause.");
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE

