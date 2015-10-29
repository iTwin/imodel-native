/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    static ECSqlStatus Prepare (Utf8StringR nativeSql, ECSqlPrepareContext& context, ECSqlParseTreeCR ecsqlParseTree);
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    11/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlExpPreparer
    {
private:
    //static class
    ECSqlExpPreparer ();
    ~ECSqlExpPreparer ();

    static ECSqlStatus PrepareFunctionArgExpList(NativeSqlBuilder& nativeSqlSnippets, ECSqlPrepareContext& ctx, FunctionCallExp const& exp);
    static ECSqlStatus PrepareSearchConditionExp(NativeSqlBuilder& nativeSqlBuilder, ECSqlPrepareContext& ctx, BooleanExp const& searchConditionExp);
    static ECSqlStatus PrepareSetFunctionCallExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, SetFunctionCallExp const& exp);

public:
    static ECSqlStatus PrepareAllOrAnyExp (ECSqlPrepareContext& ctx, AllOrAnyExp const* exp);
    static ECSqlStatus PrepareBetweenRangeValueExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BetweenRangeValueExp const* exp);
    static ECSqlStatus PrepareBinaryValueExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryValueExp const* exp);
    static ECSqlStatus PrepareBinaryBooleanExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryBooleanExp const* exp);
    static ECSqlStatus PrepareBooleanExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BooleanExp const& exp);
    static ECSqlStatus PrepareBooleanFactorExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BooleanFactorExp const* exp);
    static ECSqlStatus PrepareCastExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, CastExp const* exp);
    static ECSqlStatus PrepareClassNameExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ClassNameExp const&);
    static ECSqlStatus PrepareClassRefExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ClassRefExp const&);
    static ECSqlStatus PrepareClassRefExp (NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, ClassRefExp const&);
    static ECSqlStatus PrepareComputedExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ComputedExp const* exp);
    static ECSqlStatus PrepareCrossJoinExp (ECSqlPrepareContext& ctx, CrossJoinExp const&);
    static ECSqlStatus PrepareDerivedPropertyExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, DerivedPropertyExp const* exp);
    static ECSqlStatus PrepareFromExp (ECSqlPrepareContext& ctx, FromExp const* exp);
    static ECSqlStatus PrepareFunctionCallExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, FunctionCallExp const* exp);
    static ECSqlStatus PrepareECClassIdFunctionExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ECClassIdFunctionExp const* exp);
    static ECSqlStatus PrepareGroupByExp (ECSqlPrepareContext& ctx, GroupByExp const* exp);
    static ECSqlStatus PrepareHavingExp (ECSqlPrepareContext& ctx, HavingExp const* exp);
    static ECSqlStatus PrepareLikeRhsValueExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LikeRhsValueExp const* exp);
    static ECSqlStatus PrepareLimitOffsetExp (ECSqlPrepareContext& ctx, LimitOffsetExp const*);
    static ECSqlStatus PrepareLiteralValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LiteralValueExp const* exp);
    static ECSqlStatus PrepareNaturalJoinExp (ECSqlPrepareContext& ctx, NaturalJoinExp const&);
    static ECSqlStatus PrepareNullLiteralValueExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LiteralValueExp const* exp, size_t targetExpNativeSqlSnippetCount);
    static ECSqlStatus PrepareOrderByExp (ECSqlPrepareContext& ctx, OrderByExp const* exp);
    static ECSqlStatus PrepareParameterExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ParameterExp const* exp, bool targetIsVirtual, bool enforceConstraints);
    static ECSqlStatus PreparePropertyNameListExp (NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, PropertyNameListExp const* exp);
    //! Prepares the PropertyNameListExp if each PropertyNameExp in the list will be prepared into a single SQL snippet.
    //! Returns an error otherwise
    static ECSqlStatus PreparePropertyNameListExp(NativeSqlBuilder::List& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, PropertyNameListExp const* exp);
    static ECSqlStatus PrepareQualifiedJoinExp(ECSqlPrepareContext& ctx, QualifiedJoinExp const&);
    static ECSqlStatus PrepareQueryExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, QueryExp const* exp);
    static ECSqlStatus PrepareRelationshipJoinExp (ECSqlPrepareContext& ctx, RelationshipJoinExp const&);
    static ECSqlStatus PrepareSelectClauseExp (ECSqlPrepareContext& ctx, SelectClauseExp const* exp);
    static ECSqlStatus PrepareSubqueryExp (ECSqlPrepareContext& ctx, SubqueryExp const*);
    static ECSqlStatus PrepareSubqueryRefExp (ECSqlPrepareContext& ctx, SubqueryRefExp const*);
    static ECSqlStatus PrepareSubqueryTestExp (ECSqlPrepareContext& ctx, SubqueryTestExp const* exp);
    static ECSqlStatus PrepareSubqueryValueExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, SubqueryValueExp const* exp);
    static ECSqlStatus PrepareUnaryPredicateExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryPredicateExp const* exp);
    static ECSqlStatus PrepareUnaryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryValueExp const* exp);
    static ECSqlStatus PrepareValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExp const* exp);
    static ECSqlStatus PrepareValueExpListExp (NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExpListExp const* exp, bool encloseInParentheses);
    static ECSqlStatus PrepareValueExpListExp(NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, ValueExpListExp const* exp, PropertyNameListExp const* targetExp, NativeSqlBuilder::ListOfLists& targetNativeSqlSnippetLists);
    static ECSqlStatus PrepareWhereExp(NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, WhereExp const* exp);
 
    static ECSqlStatus ResolveChildStatementsBinding (ECSqlPrepareContext& ctx);
    static ECSqlStatus ResolveParameterMappings (ECSqlPrepareContext& ctx);

    static BooleanSqlOperator DetermineCompoundLogicalOpForCompoundExpressions(BooleanSqlOperator op);

    static bool IsNullExp (ExpCR exp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE