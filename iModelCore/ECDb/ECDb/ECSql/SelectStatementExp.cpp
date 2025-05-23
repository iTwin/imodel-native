/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** AllOrAnyExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
AllOrAnyExp::AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery)
    : BooleanExp(Type::AllOrAny), m_type(type), m_operator(op)
    {
    m_operandExpIndex = AddChild(std::move(operand));
    m_subqueryExpIndex = AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void AllOrAnyExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: AllOrAnyExp
    val["id"] = "AllOrAnyExp";
    val["op"] = Utf8String(ExpHelper::ToSql(m_type));
    GetOperand()->ToJson(val["exp"], fmt);
    GetSubquery()->ToJson(val["query"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AllOrAnyExp::_ToString() const
    {
    Utf8String str("AllOrAny [Type: ");
    str.append(ExpHelper::ToSql(m_type)).append(", Operator: ").append(ExpHelper::ToSql(m_operator)).append("]");
    return str;
    }

//************************* DerivedPropertyExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp::DerivedPropertyExp(std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias)
    : Exp(Type::DerivedProperty), m_columnAlias(columnAlias)
    {
    AddChild(std::move(valueExp));
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DerivedPropertyExp::IsComputed() const
    {
    if (GetExpression()->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef())
            {
            return propertyNameExp.GetPropertyRef()->IsComputedExp();
            }
        return false;
        }
    return true;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::GetName() const {
    Utf8String columnAlias = GetAliasRecursively();
    if (!columnAlias.empty())
        return columnAlias;

    if (GetExpression()->GetType() == Exp::Type::PropertyName) {
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        return propertyNameExp.GetResolvedPropertyPath().ToString();
    }

    if (GetExpression()->GetType() == Exp::Type::NavValueCreationFunc) {
        NavValueCreationFuncExp const& navValueCreationFuncExp = GetExpression()->GetAs<NavValueCreationFuncExp>();
        return navValueCreationFuncExp.GetPropertyNameExp()->GetResolvedPropertyPath().ToString();
    }

    return GetExpression()->ToECSql();
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::GetAliasRecursively() const {
    Utf8StringCR columnAlias = m_columnAlias;
    if (!columnAlias.empty())
        return columnAlias;

    if (GetExpression()->GetType() == Exp::Type::PropertyName) {
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef()) {
            auto alias = propertyNameExp.GetPropertyRef()->LinkedTo().GetAliasRecursively();
            if (!alias.empty())
                return alias;
        }
    }
    return columnAlias;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
Utf8StringCR DerivedPropertyExp::GetColumnAlias() const
    {
    if (!m_columnAlias.empty())
        return m_columnAlias;

    if (m_subQueryAlias.empty() && GetExpression()->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const& propertyNameExp = GetExpression()->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef())
            m_subQueryAlias =  propertyNameExp.GetResolvedPropertyPath().ToString();
        }

    return m_columnAlias;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void DerivedPropertyExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: DerivedPropertyExp
    val.SetEmptyObject();
    val["id"] = "DerivedPropertyExp";
    GetExpression()->ToJson(val["exp"], fmt);
    if (!m_columnAlias.empty())
        val["alias"] = m_columnAlias;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ExtractPropertyValueExp const* DerivedPropertyExp::TryGetExtractPropExp() const {
    const Exp* exp = GetExpression();
    while(exp) {
        if (exp->GetType() == Exp::Type::ExtractProperty)
            return exp->GetAsCP<ExtractPropertyValueExp>();
        if (exp->GetType() == Exp::Type::PropertyName) {
            auto prop = exp->GetAsCP<PropertyNameExp>();
            if (!prop->IsPropertyRef())
                return nullptr;
            exp = prop->GetPropertyRef()->GetEndPointDerivedProperty().GetExpression();
        } else if (exp->GetChildrenCount() == 1)
            exp = exp->GetChildren()[0];
        else
            return nullptr;
    }
    return (ExtractPropertyValueExp const*)exp;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool DerivedPropertyExp::IsWildCard() const {
    if (GetExpression()->GetType() == Exp::Type::PropertyName) {
        return GetExpression()->GetAsCP<PropertyNameExp>()->IsWildCard();
    }
    return false;
}
//****************************** FromExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
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
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0567,
                "Multiple occurrences of ECClass expression '%s' in the ECSQL statement. Use different aliases to distinguish them.", classExp.GetExp().ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(std::vector<RangeClassInfo>& classRefs, RangeClassInfo::Scope scope) const
    {
    for (Exp const* classRef : GetChildren())
        FindRangeClassRefs(classRefs, classRef->GetAs<ClassRefExp>(), scope);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(std::vector<RangeClassInfo>& classRefs, ClassRefExp const& classRef, RangeClassInfo::Scope scope) const
    {
    switch (classRef.GetType())
        {
            case Type::ClassName:
            case Type::SubqueryRef:
            case Type::CommonTableBlockName: {
                classRefs.push_back(RangeClassInfo(classRef.GetAs<RangeClassRefExp>(), scope));
                break;
            }
            case Type::QualifiedJoin:
            case Type::NaturalJoin:
            case Type::CrossJoin:
            case Type::ECRelationshipJoin:
                {
                JoinExp const& join = classRef.GetAs<JoinExp>();
                FindRangeClassRefs(classRefs, join.GetFromClassRef(), scope);
                FindRangeClassRefs(classRefs, join.GetToClassRef(), scope);
                if (classRef.GetType() == Type::ECRelationshipJoin)
                    FindRangeClassRefs(classRefs, join.GetAs<UsingRelationshipJoinExp>().GetRelationshipClassNameExp(), scope);

                break;
                }
            case Type::TableValuedFunction: {
                // printf
                classRefs.push_back(RangeClassInfo(classRef.GetAs<RangeClassRefExp>(), scope));
                break;
            }
            default:
                BeAssert(false && "Case not handled");
        };
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0568,
                    "Duplicate class name / alias '%s' in FROM or JOIN clause", newRangeCRef.GetExp().GetId().c_str());
                return ERROR;
                }
            }
        }

    AddChild(std::move(classRefExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
            FromExp const* fromExp = cur->GetFrom();
            if (fromExp != nullptr && fromExp != this)
                fromExp->FindRangeClassRefs(rangeClassRefs, RangeClassInfo::Scope::Inherited);
            }
        }

    return rangeClassRefs;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: FromExp
    val.SetEmptyArray();
    for (Exp const* classRefExp : GetChildren())
        classRefExp->ToJson(val.appendValue(), fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0569,
                "Invalid expression '%s' in GROUP BY: Parameters, constants, and navigation properties are not supported.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//****************************** LimitOffsetExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::LimitOffsetExp(std::unique_ptr<ValueExp> limitExp, std::unique_ptr<ValueExp> offsetExp) : Exp(Type::LimitOffset)
    {
    BeAssert(limitExp != nullptr);
    m_limitExpIndex = AddChild(std::move(limitExp));

    if (offsetExp != nullptr)
        m_offsetExpIndex =(int) AddChild(std::move(offsetExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void LimitOffsetExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: LimitOffsetExp
    val.SetEmptyObject();
    val["id"] = "LimitOffsetExp";
    GetLimitExp()->ToJson(val["exp"], fmt);
    if (HasOffset())
        GetOffsetExp()->ToJson(val["offset"], fmt);
}


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void LimitOffsetExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("LIMIT ").AppendToECSql(*GetLimitExp());
    if (HasOffset())
        ctx.AppendToECSql(" OFFSET ").AppendToECSql(*GetOffsetExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0570,
                    "Invalid expression '%s'. LIMIT expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            if (HasOffset() && !IsValidChildExp(*GetOffsetExp()))
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0571,
                    "Invalid expression '%s'. OFFSET expression must be constant numeric expression which may have parameters.", ToECSql().c_str());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitOffsetExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //limit offset operands are always integral
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Long));
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* LimitOffsetExp::GetOffsetExp() const
    {
    if (!HasOffset())
        return nullptr;

    return GetChild<ValueExp>((size_t) m_offsetExpIndex);
    }

//************************* OrderByExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OrderByExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: OrderByExp
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
}
//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
            if (ComputedExp const* incompatibleExp = FindIncompatibleOrderBySpecExpForUnion())
                {
                parseContext.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0572,
                    "'%s' ORDER BY term does not match any column in the result set.", incompatibleExp->ToECSql().c_str());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
OrderBySpecExp::FinalizeParseStatus OrderBySpecExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    ECSqlTypeInfo const& typeInfo = GetSortExpression()->GetTypeInfo();
    if (!typeInfo.IsPrimitive() || typeInfo.IsPoint() || typeInfo.IsGeometry())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0573,
            "Invalid expression '%s' in ORDER BY: Points, Geometries, navigation properties, structs and arrays are not supported.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void OrderBySpecExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: OrderBySpecExp
    val.SetEmptyObject();
    GetSortExpression()->ToJson(val["exp"], fmt);
    if (m_direction != SortDirection::NotSpecified) {
        if (m_direction == SortDirection::Ascending) {
            val["direction"] = "ASC";
        } else {
           val["direction"] = "DESC";
        }
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
        switch (m_nullsOrder)
        {
            case NullsOrder::First:
                ctx.AppendToECSql(" NULLS FIRST");
                break;

            case NullsOrder::Last:
                ctx.AppendToECSql(" NULLS LAST");
                break;

            default:
                break;
        }

    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
        PropertyPath const& propertyPath = innerExp.GetResolvedPropertyPath();
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SelectClauseExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto& sel = GetParent()->GetAs<SingleSelectStatementExp>();
        if (!sel.IsRowConstructor())
            {
            BeAssert(ctx.CurrentArg() != nullptr && "SelectClauseExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
            BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::RangeClass && "Expecting range class");
            std::vector<RangeClassInfo> rangeClassRefs;
            sel.GetFrom()->FindRangeClassRefs(rangeClassRefs, RangeClassInfo::Scope::Local);
            if (SUCCESS != ReplaceAsteriskExpressions(ctx, rangeClassRefs))
                {
                ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0574, "Asterisk replacement in select clause failed unexpectedly.");
                return FinalizeParseStatus::Error;
                }
            }
        }
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SelectClauseExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SelectClauseExp
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
SingleSelectStatementExp::SingleSelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp> selection, std::unique_ptr<FromExp> from, std::unique_ptr<WhereExp> where, std::unique_ptr<OrderByExp> orderby, std::unique_ptr<WindowFunctionClauseExp> windowExp, std::unique_ptr<GroupByExp> groupby, std::unique_ptr<HavingExp> having, std::unique_ptr<LimitOffsetExp> limitOffsetExp, std::unique_ptr<OptionsExp> optionsExp)
    : QueryExp(Type::SingleSelect), m_selectionType(selectionType)
    {
    //WARNING: Do not change the order of following
    if (from != nullptr)
        m_fromClauseIndex = (int) AddChild(std::move(from));

    m_selectClauseIndex = AddChild(std::move(selection));

    if (where != nullptr)
        m_whereClauseIndex = (int) AddChild(std::move(where));

    if (orderby != nullptr)
        m_orderByClauseIndex = (int) AddChild(std::move(orderby));

    if (windowExp != nullptr)
        m_WindowFunctionClauseExpExpIndex = (int) AddChild(std::move(windowExp));

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SingleSelectStatementExp::SingleSelectStatementExp(std::vector<std::unique_ptr<ValueExp>>& valueExpList) : QueryExp(Type::SingleSelect)
    {
    std::unique_ptr<SelectClauseExp> selectClauseExp = std::make_unique<SelectClauseExp>();
    int expIx = 0;
    UNUSED_VARIABLE(expIx);
    for (std::unique_ptr<ValueExp>& valueExp : valueExpList)
        {
        expIx++;
        std::unique_ptr<DerivedPropertyExp> derivedPropertyExp = std::make_unique<DerivedPropertyExp>(std::move(valueExp), nullptr);
        selectClauseExp->AddProperty(std::move(derivedPropertyExp));
        }

    m_selectClauseIndex = AddChild(std::move(selectClauseExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMatchResult SingleSelectStatementExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const {
    if (propertyPath.IsEmpty()) {
        BeAssert(false);
        return PropertyMatchResult::NotFound();
    }

    auto findProperty = [&](PropertyPath const& effectivePath) {
        for (Exp const* selectClauseExp : GetSelection()->GetChildren()) {
            DerivedPropertyExp const& derivedPropertyExp = selectClauseExp->GetAs<DerivedPropertyExp>();
            PropertyNameExp const *propertyNameExp = derivedPropertyExp.GetExpression()->GetType() == Exp::Type::PropertyName ? derivedPropertyExp.GetExpression()->GetAsCP<PropertyNameExp>() : nullptr;

            if (propertyNameExp == nullptr)
                propertyNameExp = derivedPropertyExp.GetExpression()->GetType() == Exp::Type::NavValueCreationFunc ? derivedPropertyExp.GetExpression()->
                    GetAs<NavValueCreationFuncExp>().GetPropertyNameExp() : nullptr;
            // Match alias or indirect
            const auto matchUserAlias = !derivedPropertyExp.GetColumnAlias().empty() && derivedPropertyExp.GetColumnAlias().EqualsIAscii(effectivePath.First().GetName());
            const auto matchIndirect = derivedPropertyExp.GetColumnAlias().empty() && propertyNameExp != nullptr &&  propertyNameExp->GetResolvedPropertyPath().ToString().EqualsIAscii(effectivePath.First().GetName());

            if (matchUserAlias || matchIndirect) {
                if (effectivePath.Size() == 1) {
                    const auto isMatchIndirect = !matchUserAlias && matchIndirect;
                    if (isMatchIndirect && propertyNameExp->HasUserDefinedAlias()) {
                        derivedPropertyExp.OverrideAlias(propertyNameExp->GetResolvedPropertyPath().ToString().c_str());
                    }
                    return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, isMatchIndirect ? -1 : 0);
                } else if (propertyNameExp != nullptr && propertyNameExp->GetClassRefExp() != nullptr) {
                    if (CompoundDataPropertyMap const *compoundProp = dynamic_cast<CompoundDataPropertyMap const*>(propertyNameExp->GetPropertyMap())) {
                        PropertyPath restOfAccessString = effectivePath.Skip(1);
                        auto endMap = compoundProp->Find(restOfAccessString.ToString().c_str());
                        if (endMap != nullptr) {
                            return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, 0);
                        }
                    }
                }
            }
            if (propertyNameExp != nullptr && propertyNameExp->GetClassRefExp() != nullptr) {
                if (propertyNameExp->GetResolvedPropertyPath().First().GetName().EqualsIAscii(effectivePath.First().GetName())) {
                    if (effectivePath.Size() == 1) {
                        return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, 0);
                    } else if (!propertyNameExp->IsPropertyFromCommonTableBlock() && propertyNameExp->GetPropertyMap() != nullptr) {
                        if (CompoundDataPropertyMap const *compoundProp = dynamic_cast<CompoundDataPropertyMap const*>(propertyNameExp->GetPropertyMap())) {
                            PropertyPath restOfAccessString = effectivePath.Skip(1);
                            auto endMap = compoundProp->Find(restOfAccessString.ToString().c_str());
                            if (endMap != nullptr) {
                                return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, 0);
                            }
                        }
                    }
                    if (propertyNameExp->GetResolvedPropertyPath().ToString().EqualsIAscii(effectivePath.ToString().c_str())) {
                        return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, -4);
                    }
                }
            }
            if (propertyNameExp == nullptr) {
                if (derivedPropertyExp.GetName().EqualsIAscii(effectivePath.ToString()))
                    return PropertyMatchResult(options, propertyPath, effectivePath, derivedPropertyExp, 0);
            }
         }
         return PropertyMatchResult::NotFound();
    };

    auto const &firstComp = propertyPath.First();
    if (propertyPath.Size() > 1 && !options.GetAlias().empty() && options.GetAlias().EqualsIAscii(firstComp.GetName())) {
        PropertyPath effectivePath = propertyPath.Skip(1);
        PropertyMatchResult rs = findProperty(effectivePath);
        if (rs.isValid()) {
            return rs;
        }
    }
    return findProperty(propertyPath);
}


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SingleSelectStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (!IsRowConstructor())
            {
            m_rangeClassRefExpCache = GetFrom()->FindRangeClassRefExpressions();

            if (FindParent(Exp::Type::SubqueryValue) != nullptr)
                {
                Exp const* parent = FindParent(Exp::Type::SingleSelect);
                SingleSelectStatementExp const* cur = parent == nullptr ? nullptr : parent->GetAsCP<SingleSelectStatementExp>();

                while (cur != nullptr)
                    {
                    parent = cur->FindParent(Exp::Type::SingleSelect);
                    cur = parent == nullptr ? nullptr : parent->GetAsCP<SingleSelectStatementExp>();
                    if (cur != nullptr)
                        {
                        cur->GetFrom()->FindRangeClassRefs(m_rangeClassRefExpCache, RangeClassInfo::Scope::Inherited);
                        }
                    }
                }
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SingleSelectStatementExp::_ToString() const
    {
    Utf8String str("Select [Modifier: ");
    str.append(ExpHelper::ToSql(m_selectionType)).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SingleSelectStatementExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SingleSelectStatementExp
    val.SetEmptyObject();
    if (IsRowConstructor()) {
        val["id"] = "RowConstructor";
        GetSelection()->ToJson(val["values"], fmt);
    } else {
        val["id"] = "SingleSelectStatementExp";
        Utf8String selectionType = ExpHelper::ToSql(GetSelectionType());
        if (!selectionType.empty())
            val["selectionType"] = selectionType;

        GetSelection()->ToJson(val["selection"], fmt);
        if (GetFrom() != nullptr)
            GetFrom()->ToJson(val["from"], fmt);

        if (GetWhere() != nullptr)
            GetWhere()->ToJson(val["where"], fmt);

        if (GetGroupBy() != nullptr)
            GetGroupBy()->ToJson(val["groupBy"], fmt);

        if (GetOrderBy() != nullptr)
            GetOrderBy()->ToJson(val["orderBy"], fmt);

        if (GetHaving() != nullptr)
            GetHaving()->ToJson(val["having"], fmt);

        if (GetLimitOffset() != nullptr)
            GetLimitOffset()->ToJson(val["limit"], fmt);

        if (GetOptions() != nullptr)
            GetOptions()->ToJson(val["options"], fmt);
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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

    ctx.AppendToECSql(*GetSelection());

    if (GetFrom() != nullptr) {
        ctx.AppendToECSql(" ").AppendToECSql(*GetFrom());
    }
    if (GetWhere() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetWhere());

    if (GetGroupBy() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetGroupBy());

    if (GetOrderBy() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetOrderBy());

    if (GetWindowFunctionClause() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetWindowFunctionClause());

    if (GetHaving() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetHaving());

    if (GetLimitOffset() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetLimitOffset());

    if (GetOptions() != nullptr)
        ctx.AppendToECSql(" ").AppendToECSql(*GetOptions());
    }


//*************************** SubqueryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryExp::SubqueryExp(std::unique_ptr<SelectStatementExp> exp) : QueryExp(Type::Subquery)
    {
    AddChild(std::move(exp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryExp::SubqueryExp(std::unique_ptr<CommonTableExp> exp) : QueryExp(Type::Subquery)
    {
    AddChild(std::move(exp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMatchResult SubqueryExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const {
    SelectStatementExp const* stm = GetQuery<SelectStatementExp>();
    if(stm != nullptr)
        return stm->FindProperty(ctx, propertyPath, options);
    CommonTableExp const* stmcte = GetQuery<CommonTableExp>();
    if(stmcte != nullptr){
        auto selectStatementInsideCTE = stmcte->GetQuery();
        return selectStatementInsideCTE->FindProperty(ctx,propertyPath,options);
    }   
    return PropertyMatchResult::NotFound();
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SelectClauseExp const* SubqueryExp::_GetSelection() const { 
    SelectStatementExp const* stm = GetQuery<SelectStatementExp>();
    if(stm != nullptr)
        return stm->GetSelection();
    CommonTableExp const* stmcte = GetQuery<CommonTableExp>();
    if(stmcte != nullptr){
        auto selectStatementInsideCTE = stmcte->GetQuery();
        return selectStatementInsideCTE->GetSelection();
    }
    // This below code should never be reached by the control. Otherwise unexpected behaviour/ undefined behaviour/ crashes may occur
    BeAssert(false && "SubqueryExp::_GetSelection> Reaching this code may lead to unexpected behaviour/ undefined behaviour/ crashes because child of SubqueryExp is expected to be either of type SelectStatementExp or CommonTableExp");
    return nullptr;  
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// SubqueryExp constructor allows either SelectStatementExp or CommonTableExp both of which are children of Exp so GetQuery<Exp> will always give a valid pointer
template<typename T>
T const* SubqueryExp::GetQuery() const { 
    auto child = GetChild<Exp>(0);
    if(child != nullptr && dynamic_cast<T const*> (child) != nullptr)
        return static_cast<T const*>(child);
    return nullptr; 
    }
// As SubqueryExp can have a child of one of these two types CommonTableExp or SelectStatementExp so only GetQuery<Exp> or GetQuery<SelectStatementExp> or GetQuery<CommonTableExp> is allowed. DON'T CALL GetQuery<>() WITH ANY OTHER TYPE.
template Exp const* SubqueryExp::GetQuery<Exp>() const;
template CommonTableExp const* SubqueryExp::GetQuery<CommonTableExp>() const;
template SelectStatementExp const* SubqueryExp::GetQuery<SelectStatementExp>() const;
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SubqueryExp
    val.SetEmptyObject();
    val["id"] = "SubqueryExp";
    Exp const* query = GetQuery<Exp>();
    query->ToJson(val["query"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryExp::_ToECSql(ECSqlRenderContext& ctx) const {
    Exp const* query = GetQuery<Exp>(); 
    ctx.AppendToECSql(*query); 
    }

//****************************** SubqueryRefExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassNameExp const* SubqueryRefExp::GetViewClass() const {
    if (GetChildrenCount() < 2) {
        return nullptr;
    }
    return GetChild<ClassNameExp>(1);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassNameExp * SubqueryRefExp::GetViewClassP() {
    if (GetChildrenCount() < 2) {
        return nullptr;
    }
    return GetChildP<ClassNameExp>(1);
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryRefExp::SubqueryRefExp(std::unique_ptr<SubqueryExp> subquery, Utf8CP alias, PolymorphicInfo polymorphic, std::unique_ptr<ClassNameExp> viewClass)
    : RangeClassRefExp(Type::SubqueryRef, polymorphic)
    {
    if (!Utf8String::IsNullOrEmpty(alias))
        SetAlias(alias);

    AddChild(std::move(subquery));
    if (viewClass != nullptr)
        AddChild(std::move(viewClass));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMatchResult SubqueryRefExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const
    {
        PropertyMatchOptions overrideOptions = options;
        if (!Utf8String::IsNullOrEmpty(GetAlias().c_str()))
            overrideOptions.SetAlias(GetAlias().c_str());
        
        return GetSubquery()->FindProperty(ctx, propertyPath, overrideOptions);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryRefExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SubqueryRefExp
    val.SetEmptyObject();
    auto viewClass = GetViewClass();
    if (viewClass == nullptr) {
    val["id"] = "SubqueryRefExp";
        if (!GetAlias().empty())
            val["alias"] = GetAlias();

        GetSubquery()->ToJson(val["query"], fmt);
        auto polymorphicInfo = GetPolymorphicInfo().ToECSql();
        if (!polymorphicInfo.empty())
            GetPolymorphicInfo().ToJson(val["polymorphicInfo"]);

        if(!GetAlias().empty())
            val["alias"] = GetAlias();
    } else {
        viewClass->ToJson(val, fmt);
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryRefExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (GetPolymorphicInfo().IsOnly())
        ctx.AppendToECSql("ONLY ");


    ctx.AppendToECSql(*GetSubquery());

    if (!GetAlias().empty())
        ctx.AppendToECSql(" AS ").AppendToECSql(GetAlias());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryRefExp::_ToString() const
    {
    Utf8String str("SubqueryRef [");
    str.append(GetId()).append("]");
    return str;
    }


//****************************** SubqueryTestExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryTestExp::SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery)
    : BooleanExp(Type::SubqueryTest), m_op(op)
    {
    AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryTestExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SubqueryTestExp
    val.SetEmptyObject();
    val["id"] = "SubqueryTestExp";
    val["op"] = ExpHelper::ToSql(m_op);
    GetSubquery()->ToJson(val["query"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryTestExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(ExpHelper::ToSql(m_op)).AppendToECSql(" ").AppendToECSql(*GetSubquery());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryTestExp::_ToString() const
    {
    Utf8String str("SubqueryTest [Operator: ");
    str.append(ExpHelper::ToSql(m_op)).append("]");
    return str;
    }



//****************************** SubqueryValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryValueExp::SubqueryValueExp(std::unique_ptr<SubqueryExp> subquery)
    : ValueExp(Type::SubqueryValue)
    {
    SetHasParentheses(); //subquery value exp always wrapped in parentheses
    AddChild(std::move(subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SubqueryValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto selectClauseExp = GetQuery()->GetSelection();

    if (selectClauseExp->GetChildren().size() != 1)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0575, "Subquery must return exactly one column %s.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    SetTypeInfo(selectClauseExp->GetChildren().Get<DerivedPropertyExp>(0)->GetExpression()->GetTypeInfo());
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SubqueryValueExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SubqueryValueExp
    GetQuery()->ToJson(val, fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SelectStatementExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: SelectStatementExp
    val.SetEmptyObject();
    val["id"] = "SelectStatementExp";
    GetFirstStatement().ToJson(val["select"], fmt);
    if (!IsCompound())
        return;

    auto nextBlock = val["nextBlock"];
    nextBlock.SetEmptyObject();
    nextBlock["combineOp"] = Utf8String(OperatorToString(m_operator)) + (m_isAll ? " ALL" : "");
    GetRhsStatement()->ToJson(nextBlock["select"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs)
    : QueryExp(Type::Select), m_isAll(false), m_operator(CompoundOperator::None), m_rhsSelectStatementExpIndex(UNSET_CHILDINDEX)
    {
    BeAssert(lhs != nullptr);
    m_firstSingleSelectStatementExpIndex = AddChild(std::move(lhs));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp const* SelectStatementExp::GetRhsStatement() const
    {
    if (IsCompound())
        return GetChild<SelectStatementExp>((size_t) m_rhsSelectStatementExpIndex);

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SelectStatementExp::_FinalizeParsing(ECSqlParseContext& parseContext, FinalizeParseMode parseMode)
    {
    if (GetRhsStatement() != nullptr && GetFirstStatement().GetOrderBy() != nullptr)
        {
        parseContext.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0576, "ORDER BY clause must not be followed by UNION clause.");
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void GroupByExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql("GROUP BY ").AppendToECSql(*GetGroupingValueListExp());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void GroupByExp::_ToJson(BeJsValue val, JsonFormat const& fmt ) const {
    //! ITWINJS_PARSE_TREE: GroupByExp
    GetGroupingValueListExp()->ToJson(val, fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void HavingExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql("HAVING ").AppendToECSql(*GetSearchConditionExp());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void HavingExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const {
    //! ITWINJS_PARSE_TREE: HavingExp
    GetSearchConditionExp()->ToJson(val, fmt);
}

END_BENTLEY_SQLITE_EC_NAMESPACE

