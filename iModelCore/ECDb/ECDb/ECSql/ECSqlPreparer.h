/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlParser.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                 Affan.Khan    06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPreparer
    {
private:
    //static class
    ECSqlPreparer ();
    ~ECSqlPreparer ();

public:
    static ECSqlStatus Prepare (Utf8StringR nativeSql, ECSqlPrepareContext&, Exp const&);
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    11/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlExpPreparer
    {
    private:
        //static class
        ECSqlExpPreparer();
        ~ECSqlExpPreparer();

        static ECSqlStatus PrepareFunctionArgExpList(NativeSqlBuilder&, ECSqlPrepareContext&, FunctionCallExp const&);
        static ECSqlStatus PrepareSearchConditionExp(NativeSqlBuilder&, ECSqlPrepareContext&, BooleanExp const& searchConditionExp);

    public:
        static ECSqlStatus PrepareAllOrAnyExp(ECSqlPrepareContext&, AllOrAnyExp const*);
        static ECSqlStatus PrepareBetweenRangeValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BetweenRangeValueExp const*);
        static ECSqlStatus PrepareBinaryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BinaryValueExp const*);
        static ECSqlStatus PrepareBinaryBooleanExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BinaryBooleanExp const*);
        static ECSqlStatus PrepareBooleanExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BooleanExp const&);
        static ECSqlStatus PrepareBooleanFactorExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BooleanFactorExp const*);
        static ECSqlStatus PrepareCastExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, CastExp const*);
        static ECSqlStatus PrepareClassNameExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ClassNameExp const&);
        static ECSqlStatus PrepareClassRefExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ClassRefExp const&);
        static ECSqlStatus PrepareClassRefExp(NativeSqlBuilder&, ECSqlPrepareContext&, ClassRefExp const&);
        static ECSqlStatus PrepareComputedExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ComputedExp const*);
        static ECSqlStatus PrepareCrossJoinExp(ECSqlPrepareContext&, CrossJoinExp const&);
        static ECSqlStatus PrepareDerivedPropertyExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, DerivedPropertyExp const*);
        static ECSqlStatus PrepareFromExp(ECSqlPrepareContext&, FromExp const*);
        static ECSqlStatus PrepareFunctionCallExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, FunctionCallExp const&);
        static ECSqlStatus PrepareECClassIdFunctionExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ECClassIdFunctionExp const&);
        static ECSqlStatus PrepareGetPointCoordinateFunctionExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, GetPointCoordinateFunctionExp const&);
        static ECSqlStatus PrepareGroupByExp(ECSqlPrepareContext&, GroupByExp const*);
        static ECSqlStatus PrepareHavingExp(ECSqlPrepareContext&, HavingExp const*);
        static ECSqlStatus PrepareLikeRhsValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, LikeRhsValueExp const*);
        static ECSqlStatus PrepareLimitOffsetExp(ECSqlPrepareContext&, LimitOffsetExp const*);
        static ECSqlStatus PrepareLiteralValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, LiteralValueExp const*);
        static ECSqlStatus PrepareNaturalJoinExp(ECSqlPrepareContext&, NaturalJoinExp const&);
        static ECSqlStatus PrepareNullLiteralValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, LiteralValueExp const*, size_t targetExpNativeSqlSnippetCount);
        static ECSqlStatus PrepareOrderByExp(ECSqlPrepareContext&, OrderByExp const*);
        static ECSqlStatus PrepareParameterExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ParameterExp const*, bool targetIsVirtual, bool enforceConstraints);
        static ECSqlStatus PrepareQualifiedJoinExp(ECSqlPrepareContext&, QualifiedJoinExp const&);
        static ECSqlStatus PrepareQueryExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, QueryExp const*);
        static ECSqlStatus PrepareRelationshipJoinExp(ECSqlPrepareContext&, ECRelationshipJoinExp const&);
        static ECSqlStatus PrepareSelectClauseExp(ECSqlPrepareContext&, SelectClauseExp const*);
        static ECSqlStatus PrepareSubqueryExp(ECSqlPrepareContext&, SubqueryExp const*);
        static ECSqlStatus PrepareSubqueryRefExp(ECSqlPrepareContext&, SubqueryRefExp const*);
        static ECSqlStatus PrepareSubqueryTestExp(ECSqlPrepareContext&, SubqueryTestExp const*);
        static ECSqlStatus PrepareSubqueryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, SubqueryValueExp const*);
        static ECSqlStatus PrepareUnaryPredicateExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, UnaryPredicateExp const*);
        static ECSqlStatus PrepareUnaryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, UnaryValueExp const*);
        static ECSqlStatus PrepareValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ValueExp const*);
        static ECSqlStatus PrepareValueExpListExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ValueExpListExp const*, bool encloseInParentheses);
        static ECSqlStatus PrepareValueExpListExp(NativeSqlBuilder::ListOfLists&, ECSqlPrepareContext&, ValueExpListExp const*, PropertyNameListExp const* targetExp, NativeSqlBuilder::ListOfLists& targetNativeSqlSnippetLists);
        static ECSqlStatus PrepareWhereExp(NativeSqlBuilder&, ECSqlPrepareContext&, WhereExp const*);

        static ECSqlStatus ResolveParameterMappings(ECSqlPrepareContext&);

        static BooleanSqlOperator DetermineCompoundLogicalOpForCompoundExpressions(BooleanSqlOperator);

        static bool IsNullExp(ExpCR);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE