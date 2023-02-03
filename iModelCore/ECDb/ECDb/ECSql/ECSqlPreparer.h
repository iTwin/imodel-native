/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlParser.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlPreparer final
    {
private:
    //static class
    ECSqlPreparer ();
    ~ECSqlPreparer ();

public:
    static ECSqlStatus Prepare (Utf8StringR nativeSql, ECSqlPrepareContext&, Exp const&);
    };


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlExpPreparer final
    {
    private:
        //static class
        ECSqlExpPreparer();
        ~ECSqlExpPreparer();

        static ECSqlStatus PrepareNullCastExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, CastExp const&);
        static BentleyStatus PrepareCastExpForPrimitive(Utf8StringR, ECN::PrimitiveType, Utf8StringCR castOperandSnippet);
        static ECSqlStatus PrepareSearchConditionExp(NativeSqlBuilder&, ECSqlPrepareContext&, BooleanExp const& searchConditionExp);
        static void RemovePropertyRefs(ECSqlPrepareContext&, ClassRefExp const&, ClassMap const&);

    public:
        static ECSqlStatus PrepareAllOrAnyExp(ECSqlPrepareContext&, AllOrAnyExp const&);
        static ECSqlStatus PrepareBetweenRangeValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BetweenRangeValueExp const&);
        static ECSqlStatus PrepareBinaryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BinaryValueExp const&);
        static ECSqlStatus PrepareBinaryBooleanExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BinaryBooleanExp const&);
        static ECSqlStatus PrepareBooleanExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BooleanExp const&);
        static ECSqlStatus PrepareBooleanFactorExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, BooleanFactorExp const&);
        static ECSqlStatus PrepareCastExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, CastExp const&);
        static ECSqlStatus PrepareClassNameExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ClassNameExp const&);
        static ECSqlStatus PrepareClassRefExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ClassRefExp const&);
        static ECSqlStatus PrepareClassRefExp(NativeSqlBuilder&, ECSqlPrepareContext&, ClassRefExp const&);
        static ECSqlStatus PrepareComputedExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ComputedExp const&);
        static ECSqlStatus PrepareCrossJoinExp(ECSqlPrepareContext&, CrossJoinExp const&);
        static ECSqlStatus PrepareFromExp(ECSqlPrepareContext&, FromExp const&);
        static ECSqlStatus PrepareFunctionCallExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, FunctionCallExp const&);
        static ECSqlStatus PrepareFunctionArgList(NativeSqlBuilder::List&, ECSqlPrepareContext&, ValueExp const& functionExp);
        static ECSqlStatus PrepareGroupByExp(ECSqlPrepareContext&, GroupByExp const&);
        static ECSqlStatus PrepareHavingExp(ECSqlPrepareContext&, HavingExp const&);
        static ECSqlStatus PrepareLikeRhsValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, LikeRhsValueExp const&);
        static ECSqlStatus PrepareLimitOffsetExp(ECSqlPrepareContext&, LimitOffsetExp const&);
        static ECSqlStatus PrepareLiteralValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, LiteralValueExp const&);
        static ECSqlStatus PrepareEnumValueExp(NativeSqlBuilder::List& , ECSqlPrepareContext&, EnumValueExp const&);
        static ECSqlStatus PrepareNaturalJoinExp(ECSqlPrepareContext&, NaturalJoinExp const&);
        static ECSqlStatus PrepareNullExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ComputedExp const&, size_t targetExpNativeSqlSnippetCount);
        static ECSqlStatus PrepareOrderByExp(ECSqlPrepareContext&, OrderByExp const&);
        static ECSqlStatus PrepareParameterExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ParameterExp const&);
        static ECSqlStatus PrepareQualifiedJoinExp(ECSqlPrepareContext&, QualifiedJoinExp const&);
        static ECSqlStatus PrepareQueryExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, QueryExp const&);
        static ECSqlStatus PrepareRelationshipJoinExp(ECSqlPrepareContext&, ECRelationshipJoinExp const&);
        static ECSqlStatus PrepareSubqueryExp(ECSqlPrepareContext&, SubqueryExp const&);
        static ECSqlStatus PrepareSubqueryRefExp(ECSqlPrepareContext&, SubqueryRefExp const&);
        static ECSqlStatus PrepareSubqueryTestExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, SubqueryTestExp const&);
        static ECSqlStatus PrepareSubqueryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, SubqueryValueExp const&);
        static ECSqlStatus PrepareUnaryPredicateExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, UnaryPredicateExp const&);
        static ECSqlStatus PrepareUnaryValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, UnaryValueExp const&);
        static ECSqlStatus PrepareValueExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ValueExp const&);
        static ECSqlStatus PrepareValueExpListExp(NativeSqlBuilder::List&, ECSqlPrepareContext&, ValueExpListExp const&, bool encloseInParentheses);
        static ECSqlStatus PrepareValueExpListExp(NativeSqlBuilder::ListOfLists&, ECSqlPrepareContext&, ValueExpListExp const&, NativeSqlBuilder::ListOfLists& targetNativeSqlSnippetLists);
        static ECSqlStatus PrepareWhereExp(NativeSqlBuilder&, ECSqlPrepareContext&, WhereExp const&);
        static ECSqlStatus PrepareCaseExp(NativeSqlBuilder::List &nativeSqlSnippets, ECSqlPrepareContext &ctx, SearchCaseValueExp const &exp);
        static ECSqlStatus PrepareIIFExp(NativeSqlBuilder::List &nativeSqlSnippets, ECSqlPrepareContext &ctx, IIFExp const &iiFexp);
        static ECSqlStatus GenerateECClassIdFilter(Utf8StringR filterSqlExpression, ClassNameExp const&);
        static ECSqlStatus PrepareTypeListExp(NativeSqlBuilder::List &nativeSqlSnippets, ECSqlPrepareContext &ctx, TypeListExp const &exp);
        static ECSqlStatus PrepareTableValuedFunctionExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, TableValuedFunctionExp const& exp);
        static ECSqlStatus PrepareExtractPropertyExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ExtractPropertyValueExp const& exp);
        static ECSqlStatus PrepareExtractInstanceExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ExtractInstanceValueExp const& exp);
        static BooleanSqlOperator DetermineCompoundLogicalOpForCompoundExpressions(BooleanSqlOperator);

    };


END_BENTLEY_SQLITE_EC_NAMESPACE