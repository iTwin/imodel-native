/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************** SubqueryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryExp::SubqueryExp (std::unique_ptr<SelectStatementExp> selectExp)
: QueryExp ()
    {
    AddChild (std::move (selectExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SubqueryExp::_FindProperty (Utf8StringCR propertyName) const 
    {
    return GetQuery ()->FindProperty (propertyName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectClauseExp const* SubqueryExp::_GetSelection () const 
    {
    return GetQuery ()->GetSelection ();
    }

//*************************** AllOrAnyExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
AllOrAnyExp::AllOrAnyExp(unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, unique_ptr<SubqueryExp> subquery)
    : BooleanExp (), m_type(type), m_operator(op)
    {
    m_operandExpIndex = AddChild (move (operand));
    m_subqueryExpIndex = AddChild (move (subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
Utf8String AllOrAnyExp::ToECSql() const
    {
    auto tmp = Utf8String ();
    tmp.append(GetOperand ()->ToECSql()).append (" ");
    tmp.append(ExpHelper::ToString (m_operator)).append (" ").append (ExpHelper::ToString (m_type)).append (" ");
    tmp.append(GetSubquery ()->ToECSql());
    return tmp;       
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AllOrAnyExp::_ToString() const 
    {
    Utf8String str ("AllOrAny [Type: ");
    str.append (ExpHelper::ToString (m_type)).append (", Operator: ").append (ExpHelper::ToString (m_operator)).append ("]");
    return str;
    }

//************************* DerivedPropertyExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp::DerivedPropertyExp(unique_ptr<ValueExp> valueExp, Utf8CP columnAlias)
    : Exp (), m_columnAlias (columnAlias)
    {
    AddChild (move (valueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::GetName () const
    {     
    if (!m_columnAlias.empty())
        return m_columnAlias;

    if (GetExpression()->GetType() == Type::PropertyName)
        return static_cast<PropertyNameExp const*>(GetExpression())->GetPropertyPath ().ToString ();

    auto expr = GetExpression()->ToECSql();
    expr.ReplaceAll("[","");
    expr.ReplaceAll("]","");
    return expr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DerivedPropertyExp::ToECSql () const
    {
    if (m_columnAlias.empty())
        return GetExpression ()->ToECSql();

    return "(" + GetExpression ()->ToECSql() + ") AS " + m_columnAlias;
    }


//****************************** FromExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FromExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    RangeClassRefList classExpList;
    FindRangeClassRefs(classExpList);

    RangeClassRefExp const* classExpComparand = nullptr;
    for (RangeClassRefExp const* classExp : classExpList)
        {
        if (classExpComparand == nullptr)
            {
            classExpComparand = classExp;
            continue;
            }

        if (classExp->GetId ().EqualsI (classExpComparand->GetId ()))
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Multiple occurrences of ECClass expression '%s' in the ECSQL statement. Use different aliases to distinguish them.", classExp->ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(RangeClassRefList& classRefs) const
    {
    for(auto classRef : GetChildren ())
        FindRangeClassRefs (classRefs, static_cast<ClassRefExp const*> (classRef));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FromExp::FindRangeClassRefs(RangeClassRefList& classRefs, ClassRefExp const* classRef) const
    {
    switch(classRef->GetType())
        {
        case Type::ClassName:
        case Type::SubqueryRef:
            classRefs.push_back(static_cast<RangeClassRefExp const*>(classRef)); break;
        case Type::QualifiedJoin:
        case Type::NaturalJoin:
        case Type::CrossJoin:
        case Type::RelationshipJoin:
            {
            auto join = static_cast<JoinExp const*>(classRef);
            FindRangeClassRefs(classRefs, join->GetFromClassRef());
            FindRangeClassRefs(classRefs, join->GetToClassRef());
            if (classRef->GetType() == Type::RelationshipJoin)
                FindRangeClassRefs(classRefs, static_cast<RelationshipJoinExp const*>(join)->GetRelationshipClass());
            break;
            }
        default:
            BeAssert(false && "Case not handled");
        };
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus FromExp::TryAddClassRef(ECSqlParseContext&ctx, ClassRefExp* classRef)
    {
    if (classRef == nullptr)
        {
        BeAssert (false);
        return ctx.SetError (ECSqlStatus::ProgrammerError, "classRef is not expected to be nullptr");
        }

    RangeClassRefList existingRangeClassRefs;
    FindRangeClassRefs(existingRangeClassRefs);

    RangeClassRefList newRangeClassRefs;

    FindRangeClassRefs (newRangeClassRefs, classRef);
    for (auto newRangeCRef : newRangeClassRefs)
        for (auto existingRangeCRef : existingRangeClassRefs)
            if (existingRangeCRef->GetId().Equals(newRangeCRef->GetId()))
                {
                //e.g. SELECT * FROM FOO a, GOO a
                return ctx.SetError (ECSqlStatus::InvalidECSql, "Duplicate class name / alias '%s' in FROM or JOIN clause", newRangeCRef->GetId().c_str ());
                }

    AddChild (std::unique_ptr<ClassRefExp>(classRef));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
RangeClassRefExp const* FromExp::FindRangeClassRefById (Utf8StringCR id) const
    {
    if (!id.empty())           
        {
        RangeClassRefList rangeClassRefs;
        FindRangeClassRefs(rangeClassRefs);
        for(auto rangeClassRef : rangeClassRefs)
            {
            auto classRefId = rangeClassRef->GetId();
            if (classRefId.empty())
                continue;
            if (id.Equals(classRefId))
                return rangeClassRef;
            }
        }
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<RangeClassRefList> FromExp::FindRangeClassRefExpressions () const
    {
    auto rangeClassRefs = unique_ptr<RangeClassRefList> (new RangeClassRefList ());
    FindRangeClassRefs(*rangeClassRefs);
    return rangeClassRefs;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FromExp::ToECSql() const
    {
    Utf8String tmp = "FROM ";
    bool isFirstItem = true;
    for(auto classRefExp : GetChildren ())
        {
        if (!isFirstItem)
            tmp.append (", ");

        tmp.append (classRefExp->ToECSql ());
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
        if (expType == Exp::Type::Parameter || expType == Exp::Type::ConstantValue || !typeInfo.IsPrimitive() || typeInfo.IsPoint())
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Invalid expression '%s' in GROUP BY: Parameters, constants, points, structs and arrays are not supported.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GroupByExp::ToECSql() const
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
Utf8String HavingExp::ToECSql() const
    {
    Utf8String str("HAVING ");
    str.append(GetSearchConditionExp()->ToECSql());
    return str;
    }


//****************************** LimitOffsetExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::LimitOffsetExp (std::unique_ptr<ValueExp> limitExp, std::unique_ptr<ValueExp> offsetExp)
: Exp (), m_offsetExpIndex (UNSET_CHILDINDEX)
    {
    BeAssert (limitExp != nullptr);
    m_limitExpIndex = AddChild (std::move (limitExp));
    
    if (offsetExp != nullptr)
        m_offsetExpIndex = static_cast<int> (AddChild (std::move (offsetExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String LimitOffsetExp::ToECSql () const 
    {
    Utf8String str ("LIMIT ");
    str.append (GetLimitExp ()->ToECSql ());
    if (HasOffset ())
        str.append (" OFFSET ").append (GetOffsetExp ()->ToECSql ());

    return str; 
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
LimitOffsetExp::FinalizeParseStatus LimitOffsetExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    BeAssert (GetLimitExp () != nullptr);

    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;
    
    if (!IsValidChildExp (*GetLimitExp ()))
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid expression '%s'. LIMIT expression must be constant numeric expression which may have parameters.", ToECSql ().c_str ());
        return FinalizeParseStatus::Error;
        }

    if (HasOffset () && !IsValidChildExp (*GetOffsetExp ()))
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid expression '%s'. OFFSET expression must be constant numeric expression which may have parameters.", ToECSql ().c_str ());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool LimitOffsetExp::IsValidChildExp (ValueExp const& exp)
    {
    auto const& typeInfo = exp.GetTypeInfo ();
    if (!typeInfo.IsNumeric ())
        return false;

    switch (exp.GetType ())
        {
            case Exp::Type::ConstantValue:
            case Exp::Type::Parameter:
            case Exp::Type::BinaryValue:
            case Exp::Type::UnaryValue:
                return true;

            default:
                return false;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* LimitOffsetExp::GetLimitExp () const
    {
    return GetChild<ValueExp> (m_limitExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool LimitOffsetExp::HasOffset () const
    {
    return m_offsetExpIndex >= 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExp const* LimitOffsetExp::GetOffsetExp () const
    {
    if (!HasOffset ())
        return nullptr;

    return GetChild<ValueExp> ((size_t) m_offsetExpIndex);
    }





//************************* OrderBySpecExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
OrderBySpecExp::OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction)
    : m_direction(direction)
    {
    AddChild (move(expr));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
OrderBySpecExp::FinalizeParseStatus OrderBySpecExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto const& typeInfo = GetSortExpression ()->GetTypeInfo ();
    if (!typeInfo.IsPrimitive () || typeInfo.IsPoint () || typeInfo.IsGeometry ())
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid expression '%s' in ORDER BY: Points, Geometries, structs and arrays are not supported.", ToECSql().c_str()); 
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
Utf8String OrderBySpecExp::ToECSql () const
    {
    auto ecsql = GetSortExpression()->ToECSql();
    AppendSortDirection (ecsql, true);
    return ecsql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String OrderBySpecExp::_ToString() const 
    {
    Utf8String str ("OrderBySpec [SortDirection: ");
    AppendSortDirection (str, false);
    str.append ("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void OrderBySpecExp::AppendSortDirection (Utf8String& str, bool addLeadingBlank) const
    {
    if (addLeadingBlank && m_direction != SortDirection::NotSpecified)
        str.append (" ");

    switch (m_direction)
        {
        case SortDirection::Ascending:
            str.append ("ASC");
            break;

        case SortDirection::Descending:
            str.append ("DESC");
            break;

        default:
            break;
        }
    }


//*************************** SelectClauseExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus SelectClauseExp::ReplaceAsteriskExpressions (ECSqlParseContext& ctx, RangeClassRefList const& rangeClassRefs)
    {
    vector<DerivedPropertyExp const*> propertyNameExpList;
    for(auto childExp : GetChildren())
        {
        auto derivedPropExp = static_cast<DerivedPropertyExp const*> (childExp);
        if (derivedPropExp->GetExpression()->GetType() == Exp::Type::PropertyName)
            propertyNameExpList.push_back(derivedPropExp);
        }

    for(auto propertyNameExp : propertyNameExpList)
        {
        auto innerExp = static_cast<PropertyNameExp const*>(propertyNameExp->GetExpression());
        if (Exp::IsAsteriskToken (innerExp->GetPropertyName ().c_str ()))
            {
            auto stat = ReplaceAsteriskExpression (ctx, *propertyNameExp, rangeClassRefs);
            if (stat != ECSqlStatus::Success)
                return stat;

            continue;
            }

        //WIP_ECSQL: Why is the alias the first entry in the prop path? The alias should be the root class, but not an entry in the prop path
        //WIP_ECSQL: What about SELECT structProp.* from FOO?
        auto const& propertyPath = innerExp->GetPropertyPath ();
        //case: SELECT a.* from FOO a
        if (propertyPath.Size() > 1 && Exp::IsAsteriskToken (propertyPath.At(1).GetPropertyName ().c_str ()))
            {
            auto alias = propertyPath.At(0).GetPropertyName();
            //Find class ref that matches the alias and replace the asterisk by just the props of that class ref
            for(auto classRef : rangeClassRefs)
                {
                if (classRef->GetId().Equals(alias))
                    {
                    RangeClassRefList classRefList;
                    classRefList.push_back (classRef);
                    auto stat = ReplaceAsteriskExpression (ctx, *propertyNameExp, classRefList);
                    if (stat != ECSqlStatus::Success)
                        return stat;

                    break;
                    }
                }
            }
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus SelectClauseExp::ReplaceAsteriskExpression (ECSqlParseContext& ctx, DerivedPropertyExp const& asteriskExp, RangeClassRefList const& rangeClassRefs)
    {
    vector<unique_ptr<Exp>> derivedPropExpList;
    auto addDelegate = [&derivedPropExpList] (unique_ptr<PropertyNameExp>& propNameExp)
        {
        derivedPropExpList.push_back (unique_ptr<Exp> (new DerivedPropertyExp (move (propNameExp), nullptr)));
        };

    for (auto classRef : rangeClassRefs)
        classRef->CreatePropertyNameExpList (ctx, addDelegate);

    if (!GetChildrenR ().Replace (asteriskExp, derivedPropExpList))
        {
        BeAssert (false && "SelectClauseExp::ReplaceAsteriskExpression did not find an asterisk expression unexpectedly.");
        return ECSqlStatus::ProgrammerError;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SelectClauseExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto finalizeParseArgs = ctx.GetFinalizeParseArg ();
        BeAssert (finalizeParseArgs != nullptr && "SelectClauseExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
        RangeClassRefList const* rangeClassRefList = static_cast<RangeClassRefList const*> (finalizeParseArgs);
        BeAssert (rangeClassRefList != nullptr);
        const auto stat = ReplaceAsteriskExpressions (ctx, *rangeClassRefList);
        if (stat != ECSqlStatus::Success)
            {
            ctx.SetError (stat, "Asterisk replacement in select clause failed unexpectedly.");
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//*************************** SingleSelectStatementExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
SingleSelectStatementExp::SingleSelectStatementExp (SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp> selection, std::unique_ptr<FromExp> from, std::unique_ptr<WhereExp> where, std::unique_ptr<OrderByExp> orderby, std::unique_ptr<GroupByExp> groupby, std::unique_ptr<HavingExp> having, std::unique_ptr<LimitOffsetExp> limitOffsetExp ) 
    : QueryExp (), m_selectionType (selectionType), m_whereClauseIndex (UNSET_CHILDINDEX),  m_orderByClauseIndex (UNSET_CHILDINDEX), m_groupByClauseIndex (UNSET_CHILDINDEX), m_havingClauseIndex (UNSET_CHILDINDEX), m_limitOffsetClauseIndex (UNSET_CHILDINDEX), m_finalizeParsingArgCache (nullptr)
    {
    //WARNING: Do not change the order of following
    m_fromClauseIndex = AddChild (std::move (from));
    m_selectClauseIndex = AddChild (std::move (selection));
    if (where != nullptr) 
        m_whereClauseIndex = static_cast<int> (AddChild (std::move (where)));

    if (orderby != nullptr) 
        m_orderByClauseIndex = static_cast<int> (AddChild (std::move (orderby)));

    if (groupby != nullptr) 
        m_groupByClauseIndex = static_cast<int> (AddChild (std::move (groupby)));

    if (having != nullptr) 
        m_havingClauseIndex = static_cast<int> (AddChild (std::move (having)));

    if (limitOffsetExp != nullptr) 
        m_limitOffsetClauseIndex = static_cast<int> (AddChild (std::move (limitOffsetExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const* SingleSelectStatementExp::_FindProperty(Utf8StringCR propertyName) const
    {
    for(auto selectClauseExp : GetSelection ()->GetChildren ())
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
                auto property = static_cast<PropertyNameExp const*>(expr);
                if (property->GetPropertyName().Equals(propertyName))
                    return derivedPropertyExp;
                }
            }
        }
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SingleSelectStatementExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        m_finalizeParsingArgCache = move (GetFrom()->FindRangeClassRefExpressions ());
        ctx.PushFinalizeParseArg (m_finalizeParsingArgCache.get ());
        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        ctx.PopFinalizeParseArg ();
        m_finalizeParsingArgCache = nullptr;
        return FinalizeParseStatus::Completed;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SingleSelectStatementExp::_ToString() const 
    {
    Utf8String str ("Select [Modifier: ");
    str.append (ExpHelper::ToString (m_selectionType)).append ("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String SingleSelectStatementExp::ToECSql() const 
    {
    Utf8String tmp ("SELECT ");

    Utf8String selectionType = ExpHelper::ToString(GetSelectionType());
    if (!selectionType.empty())
        tmp.append (selectionType).append (" ");

    tmp+= GetSelection()->ToECSql();
    tmp+= " ";
    tmp+= GetFrom()->ToECSql();

    if (GetOptWhere() != nullptr)
        {
        tmp+= " ";
        tmp+= GetOptWhere()->ToECSql();
        }

    if (GetOptGroupBy() != nullptr)
        {
        tmp+= " ";
        tmp+= GetOptGroupBy()->ToECSql();
        }

    if (GetOptOrderBy() != nullptr)
        {
        tmp+= " ";
        tmp+= GetOptOrderBy()->ToECSql();
        }

    if (GetOptHaving() != nullptr)
        {
        tmp+= " ";
        tmp+= GetOptHaving()->ToECSql();
        }

    if (GetLimitOffset() != nullptr)
        tmp.append (" ").append (GetLimitOffset ()->ToECSql ());

    return tmp;
    }

//****************************** SubqueryExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus SubqueryExp::_FinalizeParsing( ECSqlParseContext& ctx, FinalizeParseMode mode )
    {
    //ctx.SetError (ECSqlStatus::InvalidECSql, "Nested SELECT statements are not yet supported.");i
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryExp::ToECSql() const
    {
    return "(" + GetQuery ()->ToECSql() + ")";
    }


//****************************** SubqueryRefExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus SubqueryRefExp::_CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const 
    {
    for(auto expr : GetSubquery ()->GetSelection()->GetChildren ())
        {
        auto selectClauseItemExp = static_cast<DerivedPropertyExp const*> (expr);
        auto propertyNameExp = unique_ptr<PropertyNameExp> (new PropertyNameExp(*this, *selectClauseItemExp));

        addDelegate (propertyNameExp);
        }

    return ECSqlStatus::Success;
    }


//****************************** SubqueryTestExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryTestExp::SubqueryTestExp (SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery) 
    : BooleanExp (), m_op (op)
    {
    AddChild (move (subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryTestExp::ToECSql() const 
    {
    return ExpHelper::ToString(m_op) + Utf8String(" ") + GetQuery ()->ToECSql();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SubqueryTestExp::_ToString() const 
    {
    Utf8String str ("SubqueryTest [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }



//****************************** SubqueryValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
SubqueryValueExp::SubqueryValueExp(std::unique_ptr<SubqueryExp> subquery)
    : ValueExp ()
    {
    AddChild (move (subquery));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SubqueryValueExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto selectClauseExp = GetQuery ()->GetSelection ();

    if (selectClauseExp->GetChildren ().size() != 1)
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Subquery must return exactly one column %s.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    SetTypeInfo (selectClauseExp->GetChildren ().Get<DerivedPropertyExp> (0)->GetExpression ()->GetTypeInfo ());

    return FinalizeParseStatus::Completed;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE

