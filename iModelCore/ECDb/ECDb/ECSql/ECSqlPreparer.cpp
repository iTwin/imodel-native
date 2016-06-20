/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPreparer.h"
#include "ECSqlSelectPreparer.h"
#include "ECSqlInsertPreparer.h"
#include "ECSqlUpdatePreparer.h"
#include "ECSqlDeletePreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"
#include "ECSqlPreparedStatement.h"
#include "ECSqlFieldFactory.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define DEFAULT_POLYMORPHIC_QUERY true

//************** ECSqlPreparer *******************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlPreparer::Prepare(Utf8StringR nativeSql, ECSqlPrepareContext& context, Exp const& exp)
    {
    ECSqlStatus status = ECSqlStatus::Error;
    switch (exp.GetType())
        {
            case Exp::Type::Select:
            {
            status = ECSqlSelectPreparer::Prepare(context, static_cast<SelectStatementExp const&> (exp));
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Insert:
            {
            status = ECSqlInsertPreparer::Prepare(context, static_cast<InsertStatementExp const&> (exp));
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Update:
            {
            status = ECSqlUpdatePreparer::Prepare(context, static_cast<UpdateStatementExp const&> (exp));
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Delete:
            {
            status = ECSqlDeletePreparer::Prepare(context, static_cast<DeleteStatementExp const&> (exp));
            if (!status.IsSuccess())
                return status;

            break;
            }

            default:
                BeAssert(false && "Programmer error in ECSqlPreparer::Preparer.");
                return ECSqlStatus::Error;
        }

    nativeSql = context.GetNativeSql();

    return ECSqlExpPreparer::ResolveParameterMappings(context);
    }

//************** ECSqlExpPreparer *******************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareAllOrAnyExp(ECSqlPrepareContext& ctx, AllOrAnyExp const* exp)
    {
    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ALL or ANY expression not yet supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBetweenRangeValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BetweenRangeValueExp const* exp)
    {
    NativeSqlBuilder::List lowerBoundSqlTokens;
    auto status = PrepareValueExp(lowerBoundSqlTokens, ctx, exp->GetLowerBoundOperand());
    if (!status.IsSuccess())
        return status;

    NativeSqlBuilder::List upperBoundSqlTokens;
    status = PrepareValueExp(upperBoundSqlTokens, ctx, exp->GetUpperBoundOperand());
    if (!status.IsSuccess())
        return status;

    const size_t tokenCount = lowerBoundSqlTokens.size();
    if (tokenCount != upperBoundSqlTokens.size())
        {
        BeAssert(false && "Type mismatch between lower bound operand and upper bound operand in BETWEEN expression.");
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch between lower bound operand and upper bound operand in BETWEEN expression.");
        return ECSqlStatus::InvalidECSql;
        }

    for (size_t i = 0; i < tokenCount; i++)
        {
        NativeSqlBuilder sql;
        if (exp->HasParentheses())
            sql.AppendParenLeft();

        sql.Append(lowerBoundSqlTokens[i]).Append(" AND ").Append(upperBoundSqlTokens[i]);
        if (exp->HasParentheses())
            sql.AppendParenRight();

        nativeSqlSnippets.push_back(sql);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBinaryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryValueExp const* exp)
    {
    NativeSqlBuilder::List lhsSqlTokens;
    auto status = PrepareValueExp(lhsSqlTokens, ctx, exp->GetLeftOperand());
    if (!status.IsSuccess())
        return status;

    NativeSqlBuilder::List rhsSqlTokens;
    status = PrepareValueExp(rhsSqlTokens, ctx, exp->GetRightOperand());
    if (!status.IsSuccess())
        return status;

    const size_t tokenCount = lhsSqlTokens.size();
    if (tokenCount != rhsSqlTokens.size())
        {
        BeAssert(false && "Expression could not be translated into SQLite SQL. Operands yielded different number of SQLite SQL expressions.");
        return ECSqlStatus::Error;
        }

    for (size_t i = 0; i < tokenCount; i++)
        {
        NativeSqlBuilder nativeSqlBuilder;
        if (exp->HasParentheses())
            nativeSqlBuilder.AppendParenLeft();

        nativeSqlBuilder.Append(lhsSqlTokens[i]).AppendSpace().Append(exp->GetOperator()).AppendSpace().Append(rhsSqlTokens[i]);

        if (exp->HasParentheses())
            nativeSqlBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(move(nativeSqlBuilder));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBinaryBooleanExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryBooleanExp const* exp)
    {
    const BooleanSqlOperator op = exp->GetOperator();
    ComputedExp const* lhsOperand = exp->GetLeftOperand();
    ComputedExp const* rhsOperand = exp->GetRightOperand();

    const bool lhsIsNullExp = IsNullExp(*lhsOperand);
    const bool rhsIsNullExp = IsNullExp(*rhsOperand);

    NativeSqlBuilder::List lhsNativeSqlSnippets;
    NativeSqlBuilder::List rhsNativeSqlSnippets;
    if (!lhsIsNullExp)
        {
        auto status = PrepareComputedExp(lhsNativeSqlSnippets, ctx, lhsOperand);
        if (!status.IsSuccess())
            return status;

        if (lhsNativeSqlSnippets.empty())
            {
            BeAssert(false && "Expression was translated into an empty SQLite SQL expression.");
            return ECSqlStatus::Error;
            }

        }

    if (!rhsIsNullExp)
        {
        auto status = PrepareComputedExp(rhsNativeSqlSnippets, ctx, rhsOperand);
        if (!status.IsSuccess())
            return status;

        if (rhsNativeSqlSnippets.empty())
            {
            BeAssert(false && "Expression was translated into an empty SQLite SQL expression.");
            return ECSqlStatus::Error;
            }
        }

    if (lhsIsNullExp)
        {
        //if both operands are NULL, pass 1 as sql snippet count
        size_t targetSqliteSnippetCount = rhsIsNullExp ? 1 : rhsNativeSqlSnippets.size();
        PrepareNullLiteralValueExp(lhsNativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (lhsOperand), targetSqliteSnippetCount);
        }

    if (rhsIsNullExp)
        PrepareNullLiteralValueExp(rhsNativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (rhsOperand), lhsNativeSqlSnippets.size());

    const auto nativeSqlSnippetCount = lhsNativeSqlSnippets.size();
    if (nativeSqlSnippetCount != rhsNativeSqlSnippets.size())
        {
        BeAssert(false && "Expression could not be translated into SQLite SQL. Operands yielded different number of SQLite SQL expressions.");
        return ECSqlStatus::Error;
        }

    const BooleanSqlOperator logicalCompoundOp = DetermineCompoundLogicalOpForCompoundExpressions(op);
    const bool wrapInParens = exp->HasParentheses() || nativeSqlSnippetCount > 1;

    NativeSqlBuilder sqlBuilder;
    if (wrapInParens)
        sqlBuilder.AppendParenLeft();

    bool isFirstSnippet = true;
    for (size_t i = 0; i < nativeSqlSnippetCount; i++)
        {
        if (!isFirstSnippet)
            sqlBuilder.AppendSpace().Append(logicalCompoundOp).AppendSpace();

        sqlBuilder.Append(lhsNativeSqlSnippets[i]).AppendSpace().Append(op).AppendSpace().Append(rhsNativeSqlSnippets[i]);

        isFirstSnippet = false;
        }

    if (wrapInParens)
        sqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(move(sqlBuilder));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBooleanExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BooleanExp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::AllOrAny:
                return PrepareAllOrAnyExp(ctx, static_cast<AllOrAnyExp const*> (&exp));
            case Exp::Type::BinaryBoolean:
                return PrepareBinaryBooleanExp(nativeSqlSnippets, ctx, static_cast<BinaryBooleanExp const*> (&exp));
            case Exp::Type::BooleanFactor:
                return PrepareBooleanFactorExp(nativeSqlSnippets, ctx, static_cast<BooleanFactorExp const*> (&exp));
            case Exp::Type::SubqueryTest:
                return PrepareSubqueryTestExp(ctx, static_cast<SubqueryTestExp const*> (&exp));
            case Exp::Type::UnaryPredicate:
                return PrepareUnaryPredicateExp(nativeSqlSnippets, ctx, static_cast<UnaryPredicateExp const*> (&exp));

            default:
                BeAssert(false && "ECSqlPreparer::PrepareBooleanExp> Case not handled");
                return ECSqlStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBooleanFactorExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BooleanFactorExp const* exp)
    {
    NativeSqlBuilder::List operandSqlSnippets;
    auto status = PrepareBooleanExp(operandSqlSnippets, ctx, *exp->GetOperand());
    if (!status.IsSuccess())
        return status;

    for (auto const& operandSqlSnippet : operandSqlSnippets)
        {
        NativeSqlBuilder sqlBuilder;
        if (exp->HasParentheses())
            sqlBuilder.AppendParenLeft();

        if (exp->HasNotOperator())
            sqlBuilder.Append("NOT ");

        sqlBuilder.Append(operandSqlSnippet);

        if (exp->HasParentheses())
            sqlBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(move(sqlBuilder));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareCastExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, CastExp const* exp)
    {
    auto castOperand = exp->GetCastOperand();
    if (!exp->NeedsCasting())
        return PrepareValueExp(nativeSqlSnippets, ctx, castOperand);

    const bool castOperandIsNull = IsNullExp(*castOperand);
    NativeSqlBuilder::List operandNativeSqlSnippets;
    if (castOperandIsNull)
        PrepareNullLiteralValueExp(operandNativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (castOperand), 1);
    else
        {
        auto stat = PrepareValueExp(operandNativeSqlSnippets, ctx, castOperand);
        if (!stat.IsSuccess())
            return stat;
        }

    if (operandNativeSqlSnippets.empty())
        {
        BeAssert(false && "Preparing CAST operand did not return a SQLite SQL expression.");
        return ECSqlStatus::Error;
        }

    BeAssert(exp->GetTypeInfo().IsPrimitive() && "For now only primitive types supported as CAST target type.");
    const auto targetType = exp->GetTypeInfo().GetPrimitiveType();

    if (targetType == PRIMITIVETYPE_Point2D || targetType == PRIMITIVETYPE_Point3D)
        {
        size_t expectedOperandSnippetCount = targetType == PRIMITIVETYPE_Point2D ? 2 : 3;
        for (size_t i = 0; i < expectedOperandSnippetCount; i++)
            {
            NativeSqlBuilder nativeSqlBuilder;
            if (exp->HasParentheses())
                nativeSqlBuilder.AppendParenLeft();

            //if cast operand is null, the snippet list contains a single NULL snippet. In this case we simply
            //reuse the same snippet for all coordinates of the point.
            auto const& operandSqlSnippet = castOperandIsNull ? operandNativeSqlSnippets[0] : operandNativeSqlSnippets[i];
            nativeSqlBuilder.Append("CAST(").Append(operandSqlSnippet).Append(" AS DOUBLE)");

            if (exp->HasParentheses())
                nativeSqlBuilder.AppendParenRight();

            nativeSqlSnippets.push_back(move(nativeSqlBuilder));
            }
        return ECSqlStatus::Success;
        }


    NativeSqlBuilder nativeSqlBuilder;
    if (exp->HasParentheses())
        nativeSqlBuilder.AppendParenLeft();

    Utf8CP castFormat = nullptr;
    switch (targetType)
        {
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                castFormat = "CAST(%s AS BLOB)";
                break;
            case PRIMITIVETYPE_Boolean:
                castFormat = "CASE WHEN %s <> 0 THEN 1 ELSE 0 END";
                break;
            case PRIMITIVETYPE_DateTime:
                castFormat = "CAST(%s AS TIMESTAMP)";
                break;
            case PRIMITIVETYPE_Double:
                castFormat = "CAST(%s AS REAL)";
                break;
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Integer:
                castFormat = "CAST(%s AS INTEGER)";
                break;
            case PRIMITIVETYPE_String:
                castFormat = "CAST(%s AS TEXT)";
                break;
            default:
                BeAssert(false && "Unexpected cast target type during preparation");
                return ECSqlStatus::Error;
        }

    Utf8String castExpStr;
    castExpStr.Sprintf(castFormat, operandNativeSqlSnippets[0].ToString());
    nativeSqlBuilder.Append(castExpStr.c_str(), false);

    if (exp->HasParentheses())
        nativeSqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(move(nativeSqlBuilder));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareClassNameExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ClassNameExp const& exp)
    {
    const auto currentScopeECSqlType = ctx.GetCurrentScope().GetECSqlType();
    auto const& classMap = exp.GetInfo().GetMap();
    if (ctx.IsPrimaryStatement() && !ctx.IsParentOfJoinedTable() /*Disable abstract class test for joinedTable*/)
        {
        auto policy = ECDbPolicyManager::GetClassPolicy(classMap, IsValidInECSqlPolicyAssertion::Get(currentScopeECSqlType, exp.IsPolymorphic()));
        if (!policy.IsSupported())
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage());
            return ECSqlStatus::InvalidECSql;
            }
        }

    if (currentScopeECSqlType == ECSqlType::Select)
        {
        NativeSqlBuilder classViewSql;
        if (classMap.GenerateSelectViewSql(classViewSql, exp.IsPolymorphic(), ctx) != SUCCESS)
            {
            BeAssert(false && "Class view generation failed during preparation of class name expression.");
            return ECSqlStatus::Error;
            }

        classViewSql.AppendSpace().AppendEscaped(exp.GetId().c_str());
        nativeSqlSnippets.push_back(move(classViewSql));

        return ECSqlStatus::Success;
        }

    DbTable const* table = nullptr;
    if (currentScopeECSqlType == ECSqlType::Insert)
        {
        //don't compute storage description for INSERT as it is slow, and not needed for INSERT (which is always non-polymorphic)
        BeAssert(!exp.IsPolymorphic());
        table = &classMap.GetJoinedTable();
        }
    else
        {
        if (classMap.HasJoinedTable() && currentScopeECSqlType == ECSqlType::Delete)
            {
            table = &classMap.GetPrimaryTable();
            }
        else if (classMap.HasJoinedTable() && currentScopeECSqlType == ECSqlType::Update)
            {
            table = &classMap.GetJoinedTable();
            }
        else
            {
            StorageDescription const& desc = classMap.GetStorageDescription();
            if (exp.IsPolymorphic() && desc.HierarchyMapsToMultipleTables())
                {
                BeAssert(desc.HierarchyMapsToMultipleTables() && exp.IsPolymorphic() && "Returned partition is null only for a polymorphic ECSQL where subclasses are in a separate table");
                NativeSqlBuilder nativeSqlSnippet;
                nativeSqlSnippet.AppendEscaped(classMap.GetUpdatableViewName().c_str());
                nativeSqlSnippets.push_back(move(nativeSqlSnippet));
                return ECSqlStatus::Success;
                }


            Partition const* partition = desc.GetHorizontalPartition(exp.IsPolymorphic());
            table = &partition->GetTable();
            }
        }

    BeAssert(table != nullptr);

    //if table is virtual, i.e. does not exist in db, the ECSQL is still valid, but will result
    //in a no-op in SQLite. Continue preparation as clients must continue to be able to call the bind
    //API, even if it is a no-op. If we stopped preparation, clients would see index out of range errors when 
    //calling the bind API.
    if (table->GetPersistenceType() == PersistenceType::Virtual)
        ctx.SetNativeStatementIsNoop(true);

    NativeSqlBuilder nativeSqlSnippet;
    nativeSqlSnippet.AppendEscaped(table->GetName().c_str());
    nativeSqlSnippets.push_back(move(nativeSqlSnippet));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareClassRefExp(NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, ClassRefExp const& exp)
    {
    NativeSqlBuilder::List singleItemSnippetList;
    auto stat = PrepareClassRefExp(singleItemSnippetList, ctx, exp);
    if (!stat.IsSuccess() || singleItemSnippetList.empty())
        return stat;

    if (singleItemSnippetList.size() != 1)
        {
        BeAssert(false && "PrepareClassRefExp (NativeSqlBuilder&) overload must not be called for delete and update statements.");
        return ECSqlStatus::Error;
        }

    nativeSqlSnippet.Append(singleItemSnippetList[0]);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareClassRefExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ClassRefExp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::ClassName:
                return PrepareClassNameExp(nativeSqlSnippets, ctx, static_cast<ClassNameExp const&>(exp));
            case Exp::Type::CrossJoin:
                return PrepareCrossJoinExp(ctx, static_cast<CrossJoinExp const&>(exp));
            case Exp::Type::ECRelationshipJoin:
                return PrepareRelationshipJoinExp(ctx, static_cast<ECRelationshipJoinExp const&>(exp));
            case Exp::Type::NaturalJoin:
                return PrepareNaturalJoinExp(ctx, static_cast<NaturalJoinExp const&>(exp));
            case Exp::Type::QualifiedJoin:
                return PrepareQualifiedJoinExp(ctx, static_cast<QualifiedJoinExp const&>(exp));
            case Exp::Type::SubqueryRef:
                return PrepareSubqueryRefExp(ctx, static_cast<SubqueryRefExp const*>(&exp));
        }

    BeAssert(false && "Unhandled ClassRef expression case");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareComputedExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ComputedExp const* exp)
    {
    //all subclasses of BooleanExp are handled by PrepareBooleanExp
    auto booleanExp = dynamic_cast<BooleanExp const*> (exp);
    if (booleanExp != nullptr)
        return PrepareBooleanExp(nativeSqlSnippets, ctx, *booleanExp);

    //all subclasses of ValueExp are handled by PrepareValueExp
    auto valueExp = dynamic_cast<ValueExp const*> (exp);
    if (valueExp != nullptr)
        return PrepareValueExp(nativeSqlSnippets, ctx, valueExp);

    if (exp->GetType() == Exp::Type::ValueExpList)
        return PrepareValueExpListExp(nativeSqlSnippets, ctx, static_cast<ValueExpListExp const*> (exp), /* encloseInParentheses = */ true);

    BeAssert(false && "ECSqlPreparer::PrepareComputedExp: Unhandled ComputedExp subclass.");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareLiteralValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LiteralValueExp const* exp)
    {
    //WIP_ECSQL: Add support for PointXD
    auto const& typeInfo = exp->GetTypeInfo();
    if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::Null)
        {
        BeAssert(false && "Preparation of NULL expression must be called in context of the target expression.");
        return ECSqlStatus::Error;
        }

    auto expValue = exp->GetValue().c_str();

    NativeSqlBuilder nativeSqlBuilder;
    if (exp->HasParentheses())
        nativeSqlBuilder.AppendParenLeft();

    if (typeInfo.IsPrimitive())
        {
        switch (typeInfo.GetPrimitiveType())
            {
                case PRIMITIVETYPE_Binary:
                    nativeSqlBuilder.Append("X").Append(expValue);
                    break;

                case PRIMITIVETYPE_Boolean:
                {
                auto nativeSqlBooleanVal = exp->GetValueAsBoolean() ? "1" : "0";
                nativeSqlBuilder.Append(nativeSqlBooleanVal);
                break;
                }

                case PRIMITIVETYPE_DateTime:
                {
                //Note: CURRENT_TIMESTAMP in SQLite returns a UTC timestamp. ECSQL specifies CURRENT_TIMESTAMP as UTC, too,
                //so no conversion needed.
                nativeSqlBuilder.Append("JULIANDAY (");
                if (BeStringUtilities::Strnicmp(expValue, "CURRENT", 7) == 0)
                    nativeSqlBuilder.Append(expValue);
                else
                    nativeSqlBuilder.AppendQuoted(expValue);

                nativeSqlBuilder.AppendParenRight();
                break;
                }

                case PRIMITIVETYPE_String:
                    nativeSqlBuilder.AppendQuoted(LiteralValueExp::EscapeStringLiteral(expValue).c_str());
                    break;

                default:
                    nativeSqlBuilder.Append(expValue);
                    break;
            }
        }
    else
        nativeSqlBuilder.Append(expValue);

    if (exp->HasParentheses())
        nativeSqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(move(nativeSqlBuilder));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareNullLiteralValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LiteralValueExp const* exp, size_t targetExpNativeSqlSnippetCount)
    {
    if (targetExpNativeSqlSnippetCount == 0)
        {
        BeAssert(false && "NULL expression could not be translated into SQLite SQL. Target operand yielded empty SQLite SQL expression.");
        return ECSqlStatus::Error;
        }

    for (size_t i = 0; i < targetExpNativeSqlSnippetCount; i++)
        {
        if (exp->HasParentheses())
            nativeSqlSnippets.push_back(NativeSqlBuilder("(NULL)"));
        else
            nativeSqlSnippets.push_back(NativeSqlBuilder("NULL"));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareCrossJoinExp(ECSqlPrepareContext& ctx, CrossJoinExp const& exp)
    {
    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cross join expression not yet supported.");
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareDerivedPropertyExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, DerivedPropertyExp const* exp)
    {
    auto innerExp = exp->GetExpression();
    if (innerExp == nullptr)
        {
        BeAssert(false && "DerivedPropertyExp::GetExpression is not expected to return null during preparation.");
        return ECSqlStatus::Error;
        }

    const auto startColumnIndex = ctx.GetCurrentScope().GetNativeSqlSelectClauseColumnCount();

    auto snippetCountBefore = nativeSqlSnippets.size();
    if (IsNullExp(*innerExp))
        //pass 1 here as NULL in select clause item is always a primitive null and maps to the default type of the EC system (string).
        //E.g. The NULL expression in  'SELECT NULL as Something FROM Foo' will always be of type string
        PrepareNullLiteralValueExp(nativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (innerExp), 1);
    else
        {
        auto status = PrepareValueExp(nativeSqlSnippets, ctx, innerExp);
        if (!status.IsSuccess())
            return status;
        }

    if (!ctx.GetCurrentScope().IsRootScope())
        {
        auto alias = exp->GetColumnAlias();
        if (alias.empty())
            alias = exp->GetNestedAlias();

        if (!alias.empty())
            {
            if (nativeSqlSnippets.size() == 1LL)
                {
                nativeSqlSnippets.front().AppendSpace().AppendEscaped(alias.c_str());
                }
            else
                {
                int idx = 0;
                Utf8String postfix;
                for (auto& snippet : nativeSqlSnippets)
                    {
                    postfix.clear();
                    postfix.Sprintf("%s_%d", alias.c_str(), idx);
                    idx++;
                    snippet.AppendSpace().AppendEscaped(postfix.c_str());
                    }
                }
            }
        }
    else if (ctx.GetCurrentScope().IsRootScope())
        {
        ctx.GetCurrentScopeR().IncrementNativeSqlSelectClauseColumnCount(nativeSqlSnippets.size() - snippetCountBefore);
        auto status = ECSqlFieldFactory::CreateField(ctx, exp, startColumnIndex);
        if (!status.IsSuccess())
            return status;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFromExp(ECSqlPrepareContext& ctx, FromExp const* fromClause)
    {
    auto& sqlGenerator = ctx.GetSqlBuilderR();

    sqlGenerator.Append("FROM ");
    bool isFirstItem = true;
    for (auto classRefExp : fromClause->GetChildren())
        {
        if (!isFirstItem)
            sqlGenerator.AppendComma();

        auto status = PrepareClassRefExp(sqlGenerator, ctx, *static_cast<ClassRefExp const*> (classRefExp));
        if (!status.IsSuccess())
            return status;

        isFirstItem = false;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareECClassIdFunctionExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ECClassIdFunctionExp const& exp)
    {
    RangeClassRefExp const* classRefExp = exp.GetClassRefExp();
    BeAssert(classRefExp != nullptr && "ECClassIdFunctionExp::GetClassRefExp is expected to never return null during preparation.");
    if (classRefExp->GetType() != Exp::Type::ClassName)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "%s only supported for simple class references. Subqueries are not yet supported.", classRefExp->ToECSql().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    BeAssert(dynamic_cast<ClassNameExp const*> (classRefExp) != nullptr);
    ClassNameExp const* classNameExp = static_cast<ClassNameExp const*> (classRefExp);

    NativeSqlBuilder nativeSqlSnippet;

    if (exp.HasParentheses())
        nativeSqlSnippet.AppendParenLeft();

    ClassMap const& classMap = classNameExp->GetInfo().GetMap();
    DbColumn const* classIdColumn = nullptr;
    if (classMap.GetJoinedTable().TryGetECClassIdColumn(classIdColumn))
        {
        auto classRefId = classRefExp->GetId().c_str();
        auto classIdColumnName = classIdColumn->GetName().c_str();
        nativeSqlSnippet.Append(classRefId, classIdColumnName);
        }
    else
        {
        if (ctx.GetCurrentScope().GetECSqlType() == ECSqlType::Select)
            {
            //for select statements we need to use the view's ecclass id column to avoid
            //that a constant class id number shows up in order by etc
            auto classRefId = classRefExp->GetId().c_str();
            nativeSqlSnippet.Append(classRefId, ECDB_COL_ECClassId);
            }
        else
            {
            //no class id column -> class id is constant
            nativeSqlSnippet.Append(classMap.GetClass().GetId().ToString().c_str());
            }
        }

    if (exp.HasParentheses())
        nativeSqlSnippet.AppendParenRight();

    nativeSqlSnippets.push_back(move(nativeSqlSnippet));
    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareGetPointCoordinateFunctionExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, GetPointCoordinateFunctionExp const& exp)
    {
    NativeSqlBuilder::List pointSqlSnippets;
    ValueExp const& argExp = exp.GetArgument();
    if (!argExp.GetTypeInfo().IsPoint())
        {
        BeAssert(argExp.GetTypeInfo().IsPoint() && "Invalid syntax for GetX/GetY/GetZ should have been caught by parser already.");
        return ECSqlStatus::InvalidECSql;
        }

    ECSqlStatus stat = PrepareValueExp(pointSqlSnippets, ctx, &argExp);
    if (ECSqlStatus::Success != stat)
        return stat;

    size_t snippetIndex;
    switch (exp.GetCoordinate())
        {
            case GetPointCoordinateFunctionExp::Coordinate::X:
                snippetIndex = 0;
                break;
            case GetPointCoordinateFunctionExp::Coordinate::Y:
                snippetIndex = 1;
                break;
            case GetPointCoordinateFunctionExp::Coordinate::Z:
                snippetIndex = 2;
                break;

            default:
                BeAssert(false);
                return ECSqlStatus::InvalidECSql;
        }

    if (pointSqlSnippets.size() < (snippetIndex + 1))
        {
        BeAssert(false && "Point SQL snippet count is less than the GetPointCoordinate function expects. Invalid syntax for GetX / GetY / GetZ should have been caught by parser already.");
        return ECSqlStatus::InvalidECSql;
        }

    nativeSqlSnippets.push_back(pointSqlSnippets[snippetIndex]);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareGroupByExp(ECSqlPrepareContext& ctx, GroupByExp const* exp)
    {
    if (exp == nullptr)
        return ECSqlStatus::Success;

    ctx.GetSqlBuilderR().Append(" GROUP BY ");

    NativeSqlBuilder::List groupingValuesSnippetList;
    const ECSqlStatus stat = PrepareValueExpListExp(groupingValuesSnippetList, ctx, exp->GetGroupingValueListExp(), /* encloseInParentheses = */ false);
    if (!stat.IsSuccess())
        return stat;

    ctx.GetSqlBuilderR().Append(groupingValuesSnippetList);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareHavingExp(ECSqlPrepareContext& ctx, HavingExp const* exp)
    {
    if (exp == nullptr)
        return ECSqlStatus::Success;

    ctx.GetSqlBuilderR().Append(" HAVING ");
    return PrepareSearchConditionExp(ctx.GetSqlBuilderR(), ctx, *exp->GetSearchConditionExp());
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareLikeRhsValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LikeRhsValueExp const* exp)
    {
    auto stat = PrepareValueExp(nativeSqlSnippets, ctx, exp->GetRhsExp());
    if (!stat.IsSuccess())
        return stat;

    if (nativeSqlSnippets.size() != 1)
        {
        //This is a programmer error as the parse step should already check that the like expression is of string type
        BeAssert(false && "LIKE RHS expression is expected to result in a single SQLite SQL snippet as LIKE only works with string operands.");
        return ECSqlStatus::Error;
        }

    if (exp->HasEscapeExp())
        {
        NativeSqlBuilder::List escapeExpSqlSnippets;
        auto stat = PrepareValueExp(escapeExpSqlSnippets, ctx, exp->GetEscapeExp());
        if (!stat.IsSuccess())
            return stat;

        if (escapeExpSqlSnippets.size() != 1)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid type in LIKE ESCAPE expression. ESCAPE only works with a string value.");
            return ECSqlStatus::InvalidECSql;
            }

        NativeSqlBuilder& builder = nativeSqlSnippets[0];
        builder.Append(" ESCAPE ").Append(escapeExpSqlSnippets[0]);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareLimitOffsetExp(ECSqlPrepareContext& ctx, LimitOffsetExp const* exp)
    {
    if (exp == nullptr)
        return ECSqlStatus::Success;

    ctx.GetSqlBuilderR().Append(" LIMIT ");

    NativeSqlBuilder::List sqlSnippets;
    auto stat = PrepareValueExp(sqlSnippets, ctx, exp->GetLimitExp());
    if (!stat.IsSuccess())
        return stat;

    ctx.GetSqlBuilderR().Append(sqlSnippets[0]);

    if (exp->HasOffset())
        {
        ctx.GetSqlBuilderR().Append(" OFFSET ");
        sqlSnippets.clear();
        auto stat = PrepareValueExp(sqlSnippets, ctx, exp->GetOffsetExp());
        if (!stat.IsSuccess())
            return stat;

        ctx.GetSqlBuilderR().Append(sqlSnippets[0]);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareNaturalJoinExp(ECSqlPrepareContext& ctx, NaturalJoinExp const& exp)
    {
    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Natural join expression not yet supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareOrderByExp(ECSqlPrepareContext& ctx, OrderByExp const* exp)
    {
    ctx.PushScope(*exp);

    NativeSqlBuilder orderBySqlBuilder;
    bool isFirstSpec = true;
    for (auto const child : exp->GetChildren())
        {
        auto specification = static_cast<OrderBySpecExp const*> (child);

        auto sortExp = specification->GetSortExpression();
        NativeSqlBuilder::List sqlSnippets;
        bool isPredicate = dynamic_cast<BooleanExp const*>(sortExp) != nullptr;
        //can validly return empty snippets (e.g. if prop ref maps to virtual column
        auto r = PrepareComputedExp(sqlSnippets, ctx, sortExp);
        if (!r.IsSuccess())
            return r;

        bool isFirstSnippet = true;
        for (auto const& sqlSnippet : sqlSnippets)
            {
            if (!isFirstSpec)
                {
                if (!isFirstSnippet && isPredicate)
                    orderBySqlBuilder.Append(" AND ");
                else
                    orderBySqlBuilder.AppendComma();
                }

            orderBySqlBuilder.Append(sqlSnippet);
            switch (specification->GetSortDirection())
                {
                    case OrderBySpecExp::SortDirection::Ascending:
                    {
                    orderBySqlBuilder.Append(" ASC");
                    break;
                    }
                    case OrderBySpecExp::SortDirection::Descending:
                    {
                    orderBySqlBuilder.Append(" DESC");
                    break;
                    }
                    case OrderBySpecExp::SortDirection::NotSpecified:
                        break; //default direction is ASCENDING
                }
            isFirstSnippet = false;
            isFirstSpec = false; //needs to be inside the inner loop so that empty sqlSnippets are handled correctly
            }
        }

    if (!orderBySqlBuilder.IsEmpty())
        ctx.GetSqlBuilderR().Append("ORDER BY ").Append(orderBySqlBuilder);

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareParameterExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ParameterExp const* exp, bool targetIsVirtual, bool enforceConstraints)
    {
    BeAssert(exp->GetTypeInfo().GetKind() != ECSqlTypeInfo::Kind::Unset);

    Utf8CP parameterName = exp->GetParameterName();
    auto& ecsqlParameterMap = ctx.GetECSqlStatementR().GetPreparedStatementP()->GetParameterMapR();

    int nativeSqlParameterCount = -1;
    ECSqlBinder* binder = nullptr;
    const auto binderAlreadyExists = exp->IsNamedParameter() && ecsqlParameterMap.TryGetBinder(binder, parameterName);
    if (binderAlreadyExists)
        nativeSqlParameterCount = binder->GetMappedSqlParameterCount();
    else
        {
        binder = ecsqlParameterMap.AddBinder(ctx.GetECSqlStatementR(), *exp, targetIsVirtual, enforceConstraints);
        if (binder == nullptr)
            return ECSqlStatus::Error;

        nativeSqlParameterCount = binder->GetMappedSqlParameterCount();
        }

    for (int i = 0; i < nativeSqlParameterCount; i++)
        {
        NativeSqlBuilder parameterBuilder;
        if (exp->HasParentheses())
            parameterBuilder.AppendParenLeft();

        if (binderAlreadyExists)
            parameterBuilder.AppendParameter(parameterName, i);
        else
            {
            parameterBuilder.AppendParameter(parameterName, exp->GetParameterIndex(), i);
            }

        if (exp->HasParentheses())
            parameterBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(move(parameterBuilder));
        }

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                11/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PreparePropertyNameListExp(NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, PropertyNameListExp const* exp)
    {
    BeAssert(nativeSqlSnippetLists.empty());
    for (Exp const* childExp : exp->GetChildren())
        {
        PropertyNameExp const* propNameExp = static_cast<PropertyNameExp const*> (childExp);

        NativeSqlBuilder::List nativeSqlSnippets;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, propNameExp);
        if (!stat.IsSuccess())
            return stat;

        nativeSqlSnippetLists.push_back(move(nativeSqlSnippets));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                11/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PreparePropertyNameListExp(NativeSqlBuilder::List& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, PropertyNameListExp const* exp)
    {
    BeAssert(nativeSqlSnippetLists.empty());
    for (Exp const* childExp : exp->GetChildren())
        {
        PropertyNameExp const* propNameExp = static_cast<PropertyNameExp const*> (childExp);

        NativeSqlBuilder::List nativeSqlSnippets;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, propNameExp);
        if (!stat.IsSuccess())
            return stat;

        if (nativeSqlSnippets.size() > 1)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Property Name Expression '%s' with invalid type in Property Name List Expression '%s'. Only Property Name Expressions with numeric, string or blob types are supported.",
                                                                   propNameExp->ToECSql().c_str(), exp->ToECSql().c_str());
            return ECSqlStatus::InvalidECSql;
            }

        nativeSqlSnippetLists.push_back(move(nativeSqlSnippets[0]));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareQualifiedJoinExp(ECSqlPrepareContext& ctx, QualifiedJoinExp const& exp)
    {
    auto& sqlBuilder = ctx.GetSqlBuilderR();
    ECSqlStatus r = PrepareClassRefExp(sqlBuilder, ctx, exp.GetFromClassRef());
    if (!r.IsSuccess())
        return r;

    //ECSQL_LIMITATION: 
    //https://www.sqlite.org/omitted.html
    //RIGHT and FULL OUTER JOIN	 	 LEFT OUTER JOIN is implemented, but not RIGHT OUTER JOIN or FULL OUTER JOIN.
    //
    switch (exp.GetJoinType())
        {
            case ECSqlJoinType::InnerJoin:
            {
            sqlBuilder.Append(" INNER JOIN ");
            break;
            };
            case ECSqlJoinType::LeftOuterJoin:
            {
            sqlBuilder.Append(" LEFT OUTER JOIN ");
            break;
            }
            case ECSqlJoinType::RightOuterJoin:
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "'RIGHT OUTER JOIN' is currently not supported");
            return ECSqlStatus::InvalidECSql;
            }
            case ECSqlJoinType::FullOuterJoin:
            {
            //ECSQL_TODO: way around full outer join 
            //http://stackoverflow.com/questions/1923259/full-outer-join-with-sqlite
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "'FULL OUTER JOIN' is currently not supported");
            return ECSqlStatus::InvalidECSql;
            }
        }

    r = PrepareClassRefExp(sqlBuilder, ctx, exp.GetToClassRef());
    if (!r.IsSuccess())
        return r;

    if (exp.GetJoinSpec()->GetType() == Exp::Type::JoinCondition)
        {
        auto joinCondition = static_cast<JoinConditionExp const*>(exp.GetJoinSpec());
        sqlBuilder.Append(" ON ");

        NativeSqlBuilder::List sqlSnippets;
        r = PrepareBooleanExp(sqlSnippets, ctx, *joinCondition->GetSearchCondition());
        if (!r.IsSuccess())
            return r;

        bool isFirstSnippet = true;
        for (auto const& sqlSnippet : sqlSnippets)
            {
            if (!isFirstSnippet)
                sqlBuilder.Append("AND ");

            sqlBuilder.Append(sqlSnippet).AppendSpace();
            isFirstSnippet = false;
            }

        return ECSqlStatus::Success;
        }
    else if (exp.GetJoinSpec()->GetType() == Exp::Type::NamedPropertiesJoin)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "JOIN <class/subquery> USING (property,...) is not supported yet.");
        return ECSqlStatus::InvalidECSql;
        }

    BeAssert(false && "Invalid case");
    return ECSqlStatus::Error;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareQueryExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, QueryExp const* exp)
    {
    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Query expression not yet supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareRelationshipJoinExp(ECSqlPrepareContext& ctx, ECRelationshipJoinExp const& exp)
    {
    // (from) INNER JOIN (to) ON (from.ECInstanceId = to.ECInstanceId)
    // (from) INNER JOIN (view) ON view.SourceECInstanceId = from.ECInstanceId INNER JOIN to ON view.TargetECInstanceId=to.ECInstanceId

    ECSqlStatus r;

    auto& sql = ctx.GetSqlBuilderR();
    const auto ecsqlType = ctx.GetCurrentScope().GetECSqlType();

    ///Resolve direction of the relationship
    auto& fromEP = exp.GetResolvedFromEndPoint();
    auto& toEP = exp.GetResolvedToEndPoint();
    auto direction = exp.GetDirection();

    enum class TriState
        {
        True,
        False,
        None,
        } fromIsSource = TriState::None;

    switch (fromEP.GetLocation())
        {
            case ECRelationshipJoinExp::ClassLocation::ExistInBoth:
            {
            switch (direction)
                {
                    case JoinDirection::Implied:
                    case JoinDirection::Forward:
                    {
                    fromIsSource = TriState::True;
                    } break;
                    case JoinDirection::Backward:
                    {
                    fromIsSource = TriState::False;
                    } break;
                };
            break;
            }
            case ECRelationshipJoinExp::ClassLocation::ExistInSource:
            {
            if (direction != JoinDirection::Implied)
                {
                if (direction != JoinDirection::Forward)
                    {
                    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid join direction BACKWARD in %s. Either specify FORWARD or omit the direction as the direction can be unambiguously implied in this ECSQL.", exp.ToString().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }
                }

            fromIsSource = TriState::True;
            } break;
            case ECRelationshipJoinExp::ClassLocation::ExistInTarget:
            {
            if (direction != JoinDirection::Implied)
                {
                if (direction != JoinDirection::Backward)
                    {
                    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid join direction FORWARD in %s. Either specify BACKWARD or omit the direction as the direction can be unambiguously implied in this ECSQL.", exp.ToString().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }
                }

            fromIsSource = TriState::False;
            } break;
        }

    PRECONDITION(fromIsSource != TriState::None, ECSqlStatus::Error);
    ////Determine the from/to related keys
    Utf8CP fromRelatedKey = nullptr;
    Utf8CP toRelatedKey = nullptr;
    if (fromIsSource == TriState::True)
        {
        fromRelatedKey = ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME;
        toRelatedKey = ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;
        }
    else
        {
        fromRelatedKey = ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;
        toRelatedKey = ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME;
        }

    auto const& relationshipClassNameExp = exp.GetRelationshipClass();
    auto ecInstanceIdKey = ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME;

    //Render previous sql part as is
    r = PrepareClassRefExp(sql, ctx, exp.GetFromClassRef());
    if (!r.IsSuccess())
        return r;

    //FromECClass To RelationView
    sql.Append(" INNER JOIN ");

    //Append relationship view. 
    //ECSQL_TODO: we need to keep a list of view we add as we don't need to append them again and again. Instead use there alias everyWhere else
    //            The PrepareContext scope can manage that and keep a list of already defined classes and there alias/name

    //Generate view for relationship
    NativeSqlBuilder relationshipView;

    if (relationshipClassNameExp.GetInfo().GetMap().GenerateSelectViewSql(relationshipView, DEFAULT_POLYMORPHIC_QUERY, ctx) != SUCCESS)
        {
        BeAssert(false && "Generating class view during preparation of relationship class name expression failed.");
        return ECSqlStatus::Error;
        }

    sql.Append(relationshipView);
    sql.AppendSpace();
    sql.AppendEscaped(relationshipClassNameExp.GetId().c_str());

    sql.Append(" ON ");
    PropertyMapCP fromECInstanceIdPropMap = fromEP.GetClassNameRef()->GetInfo().GetMap().GetPropertyMap(ecInstanceIdKey);
    PRECONDITION(fromECInstanceIdPropMap != nullptr, ECSqlStatus::Error);
    auto fromECInstanceIdNativeSqlSnippets = fromECInstanceIdPropMap->ToNativeSql(fromEP.GetClassNameRef()->GetId().c_str(), ecsqlType, false);
    if (fromECInstanceIdNativeSqlSnippets.size() > 1)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported in ECSQL.");
        return ECSqlStatus::InvalidECSql;
        }

    sql.Append(fromECInstanceIdNativeSqlSnippets);

    sql.Append(" = ");

    auto fromRelatedIdPropMap = relationshipClassNameExp.GetInfo().GetMap().GetPropertyMap(fromRelatedKey);
    PRECONDITION(fromRelatedIdPropMap != nullptr, ECSqlStatus::Error);
    auto fromRelatedIdNativeSqlSnippets = fromRelatedIdPropMap->ToNativeSql(relationshipClassNameExp.GetId().c_str(), ecsqlType, false);
    if (fromRelatedIdNativeSqlSnippets.size() > 1)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported in ECSQL.");
        return ECSqlStatus::InvalidECSql;
        }

    sql.Append(fromRelatedIdNativeSqlSnippets);

    //RelationView To ToECClass
    sql.Append(" INNER JOIN ");
    r = PrepareClassRefExp(sql, ctx, exp.GetToClassRef());
    if (!r.IsSuccess())
        return r;

    sql.Append(" ON ");
    auto toECInstanceIdPropMap = toEP.GetClassNameRef()->GetInfo().GetMap().GetPropertyMap(ecInstanceIdKey);
    PRECONDITION(toECInstanceIdPropMap != nullptr, ECSqlStatus::Error);
    auto toECInstanceIdSqlSnippets = toECInstanceIdPropMap->ToNativeSql(toEP.GetClassNameRef()->GetId().c_str(), ecsqlType, false);
    if (toECInstanceIdSqlSnippets.size() > 1)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported in ECSQL.");
        return ECSqlStatus::InvalidECSql;
        }

    sql.Append(toECInstanceIdSqlSnippets);

    sql.Append(" = ");
    auto toRelatedIdPropMap = relationshipClassNameExp.GetInfo().GetMap().GetPropertyMap(toRelatedKey);
    PRECONDITION(toRelatedIdPropMap != nullptr, ECSqlStatus::Error);
    auto toRelatedIdSqlSnippets = toRelatedIdPropMap->ToNativeSql(relationshipClassNameExp.GetId().c_str(), ecsqlType, false);
    if (toRelatedIdSqlSnippets.size() > 1)
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported in ECSQL.");
        return ECSqlStatus::InvalidECSql;
        }

    sql.Append(toRelatedIdSqlSnippets);

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSelectClauseExp(ECSqlPrepareContext& ctx, SelectClauseExp const* selectClause)
    {
    std::vector<Exp const*> selection(selectClause->GetChildren().begin(), selectClause->GetChildren().end());
    NativeSqlBuilder::List selectClauseNativeSqlSnippets;
    bool first = true;
    for (Exp const* derivedPropExp : selection)
        {
        selectClauseNativeSqlSnippets.clear();
        ECSqlStatus status = PrepareDerivedPropertyExp(selectClauseNativeSqlSnippets, ctx, static_cast<DerivedPropertyExp const*> (derivedPropExp));
        if (!status.IsSuccess())
            return status;

        if (selectClauseNativeSqlSnippets.empty())
            continue;

        if (first)
            first = false;
        else
            ctx.GetSqlBuilderR().AppendComma();

        ctx.GetSqlBuilderR().Append(selectClauseNativeSqlSnippets);
        }

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFunctionCallExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, FunctionCallExp const& exp)
    {
    Utf8CP functionName = exp.GetFunctionName();
    NativeSqlBuilder nativeSql;
    if (exp.HasParentheses())
        nativeSql.AppendParenLeft();

    const bool isAnyOrSomeFunction = BeStringUtilities::StricmpAscii(functionName, "any") == 0 || BeStringUtilities::StricmpAscii(functionName, "some") == 0;
    const bool isEveryFunction = !isAnyOrSomeFunction && BeStringUtilities::StricmpAscii(functionName, "every") == 0;
    const bool isAnyEveryOrSomeFunction = isAnyOrSomeFunction || isEveryFunction;
    if (isAnyEveryOrSomeFunction)
        {
        //ANY, EVERY, SOME is not directly supported by SQLite. But they can be expressed by standard functions
        //ANY,SOME: checks whether at least one row in the specified BOOLEAN column is TRUE -> MAX(Col) <> 0
        //EVERY: checks whether all rows in the specified BOOLEAN column are TRUE -> MIN(Col) <> 0
        Utf8CP sqlFunctionName = isEveryFunction ? "MIN" : "MAX";
        nativeSql.Append(sqlFunctionName);
        }
    else
        nativeSql.Append(functionName);

    nativeSql.AppendParenLeft().Append(exp.GetSetQuantifier()).AppendSpace();

    ECSqlStatus stat = PrepareFunctionArgExpList(nativeSql, ctx, exp);
    if (!stat.IsSuccess())
        return stat;

    if (isAnyEveryOrSomeFunction)
        nativeSql.Append(" <> 0");

    nativeSql.AppendParenRight(); //function arg list parent

    if (exp.HasParentheses())
        nativeSql.AppendParenLeft();

    nativeSqlSnippets.push_back(move(nativeSql));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFunctionArgExpList(NativeSqlBuilder& nativeSql, ECSqlPrepareContext& ctx, FunctionCallExp const& exp)
    {
    bool isFirstItem = true;
    for (Exp const* argExp : exp.GetChildren())
        {
        if (!isFirstItem)
            nativeSql.AppendComma();

        NativeSqlBuilder::List nativeSqlArgumentList;
        ECSqlStatus status;
        if (IsNullExp(*argExp))
            {
            //for functions we only support args of single column primitive types so far, therefore an ECSQL NULL
            //always means a single SQLite NULL
            status = PrepareNullLiteralValueExp(nativeSqlArgumentList, ctx, static_cast<LiteralValueExp const*> (argExp), 1);
            }
        else
            status = PrepareValueExp(nativeSqlArgumentList, ctx, static_cast<ValueExp const*> (argExp));

        if (!status.IsSuccess())
            return status;

        nativeSql.Append(nativeSqlArgumentList);
        isFirstItem = false;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSearchConditionExp(NativeSqlBuilder& nativeSqlBuilder, ECSqlPrepareContext& ctx, BooleanExp const& searchConditionExp)
    {
    NativeSqlBuilder::List sqlSnippets;
    const ECSqlStatus stat = PrepareBooleanExp(sqlSnippets, ctx, searchConditionExp);
    if (!stat.IsSuccess())
        return stat;

    //If the top level search condition has an OR we wrap the entire exp in parentheses to ensure
    //precedence for the case where a system expression is added to the where clause.
    bool wrapInParens = false;
    if (searchConditionExp.GetType() == Exp::Type::BinaryBoolean)
        {
        BinaryBooleanExp const* binaryBoolExp = static_cast<BinaryBooleanExp const*> (&searchConditionExp);
        if (binaryBoolExp->GetOperator() == BooleanSqlOperator::Or)
            wrapInParens = true;
        }

    if (wrapInParens)
        nativeSqlBuilder.AppendParenLeft();

    nativeSqlBuilder.Append(sqlSnippets, " AND ");

    if (wrapInParens)
        nativeSqlBuilder.AppendParenRight();

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryExp(ECSqlPrepareContext& ctx, SubqueryExp const* exp)
    {
    ctx.GetSqlBuilderR().AppendParenLeft();
    auto stat = ECSqlSelectPreparer::Prepare(ctx, *exp->GetQuery());
    ctx.GetSqlBuilderR().AppendParenRight();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryRefExp(ECSqlPrepareContext& ctx, SubqueryRefExp const* exp)
    {
    auto status = PrepareSubqueryExp(ctx, exp->GetSubquery());
    if (!status.IsSuccess())
        return status;

    if (!exp->GetAlias().empty())
        ctx.GetSqlBuilderR().AppendSpace().AppendQuoted(exp->GetAlias().c_str());

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryTestExp(ECSqlPrepareContext& ctx, SubqueryTestExp const* exp)
    {
    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "SubqueryTest expression not supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, SubqueryValueExp const* exp)
    {
    Utf8String a = ctx.GetSqlBuilder().ToString();
    ctx.GetSqlBuilderR().Push(true);
    auto st = PrepareSubqueryExp(ctx, exp->GetQuery());
    nativeSqlSnippets.push_back(NativeSqlBuilder(ctx.GetSqlBuilderR().Pop().c_str()));
    return st;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareUnaryPredicateExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryPredicateExp const* exp)
    {
    return PrepareValueExp(nativeSqlSnippets, ctx, exp->GetValueExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareUnaryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryValueExp const* exp)
    {
    NativeSqlBuilder::List unaryOperandSqlBuilders;
    auto status = PrepareValueExp(unaryOperandSqlBuilders, ctx, exp->GetOperand());
    if (!status.IsSuccess())
        return status;

    BeAssert(unaryOperandSqlBuilders.size() <= 1 && "UnaryExp with Points and non-primitive types not supported yet.");

    const UnarySqlOperator unaryOp = exp->GetOperator();
    for (NativeSqlBuilder const& unaryOperandSqlBuilder : unaryOperandSqlBuilders)
        {
        NativeSqlBuilder unaryExpBuilder;
        if (exp->HasParentheses())
            unaryExpBuilder.AppendParenLeft();

        unaryExpBuilder.Append(unaryOp).Append(unaryOperandSqlBuilder);

        if (exp->HasParentheses())
            unaryExpBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(move(unaryExpBuilder));
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExp const* exp)
    {
    switch (exp->GetType())
        {
            case Exp::Type::BetweenRangeValue:
                return PrepareBetweenRangeValueExp(nativeSqlSnippets, ctx, static_cast<BetweenRangeValueExp const*> (exp));
            case Exp::Type::BinaryValue:
                return PrepareBinaryValueExp(nativeSqlSnippets, ctx, static_cast<BinaryValueExp const*> (exp));
            case Exp::Type::Cast:
                return PrepareCastExp(nativeSqlSnippets, ctx, static_cast<CastExp const*> (exp));
            case Exp::Type::LiteralValue:
                return PrepareLiteralValueExp(nativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (exp));
            case Exp::Type::ECClassIdFunction:
                return PrepareECClassIdFunctionExp(nativeSqlSnippets, ctx, *static_cast<ECClassIdFunctionExp const*> (exp));
            case Exp::Type::FunctionCall:
                return PrepareFunctionCallExp(nativeSqlSnippets, ctx, *static_cast<FunctionCallExp const*> (exp));
            case Exp::Type::GetPointCoordinateFunction:
                return PrepareGetPointCoordinateFunctionExp(nativeSqlSnippets, ctx, *static_cast<GetPointCoordinateFunctionExp const*> (exp));
            case Exp::Type::LikeRhsValue:
                return PrepareLikeRhsValueExp(nativeSqlSnippets, ctx, static_cast<LikeRhsValueExp const*> (exp));
            case Exp::Type::Parameter:
                return PrepareParameterExp(nativeSqlSnippets, ctx, static_cast<ParameterExp const*> (exp), false, false);
            case Exp::Type::PropertyName:
                return ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, static_cast<PropertyNameExp const*>(exp));
            case Exp::Type::SubqueryValue:
                return PrepareSubqueryValueExp(nativeSqlSnippets, ctx, static_cast<SubqueryValueExp const*> (exp));
            case Exp::Type::UnaryValue:
                return PrepareUnaryValueExp(nativeSqlSnippets, ctx, static_cast<UnaryValueExp const*> (exp));
            default:
                break;
        }

    BeAssert(false && "ECSqlPreparer::PrepareValueExp> Unhandled ValueExp subclass.");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareValueExpListExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExpListExp const* exp, bool encloseInParentheses)
    {
    BeAssert(nativeSqlSnippets.empty());
    auto isFirstExp = true;
    for (auto valueExp : exp->GetChildren())
        {
        NativeSqlBuilder::List listItemExpBuilders;
        auto stat = PrepareValueExp(listItemExpBuilders, ctx, static_cast<ValueExp const*> (valueExp));
        if (!stat.IsSuccess())
            return stat;

        for (size_t i = 0; i < listItemExpBuilders.size(); i++)
            {
            if (isFirstExp)
                {
                NativeSqlBuilder builder;
                if (encloseInParentheses)
                    builder.AppendParenLeft();

                builder.Append(listItemExpBuilders[i]);
                nativeSqlSnippets.push_back(move(builder));
                }
            else
                {
                auto& builder = nativeSqlSnippets[i];
                builder.AppendComma().Append(listItemExpBuilders[i]);
                }
            }

        isFirstExp = false;
        }

    //finally add closing parenthesis to all list snippets we created
    if (encloseInParentheses)
        for_each(nativeSqlSnippets.begin(), nativeSqlSnippets.end(), [] (NativeSqlBuilder& builder) { builder.AppendParenRight(); });

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                11/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareValueExpListExp(NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, ValueExpListExp const* exp, PropertyNameListExp const* targetExp, NativeSqlBuilder::ListOfLists& targetNativeSqlSnippetLists)
    {
    BeAssert(nativeSqlSnippetLists.empty());
    size_t index = 0;
    for (auto valueExp : exp->GetChildren())
        {
        ECSqlStatus stat = ECSqlStatus::Success;
        BeAssert(valueExp != nullptr);

        const auto targetNativeSqlSnippetCount = targetNativeSqlSnippetLists[index].size();
        //virtual props result in 0 native sql snippets. 
        const bool targetIsVirtual = targetNativeSqlSnippetCount == 0;

        NativeSqlBuilder::List nativeSqlSnippets;

        //If target expression does not have any SQL snippets, it means the expression is not necessary in SQLite SQL (e.g. for source/target class id props)
        //In that case the respective value exp does not need to be prepared either.

        if (valueExp->IsParameterExp())
            {
            //Parameter exp needs to be prepared even if target exp is virtual, i.e. doesn't have a column in the SQLite SQL
            //because we need a (noop) binder for it so that the binding API corresponds to the parameters in the incoming
            //ECSQL.
            BeAssert(dynamic_cast<ParameterExp const*> (valueExp) != nullptr);
            stat = PrepareParameterExp(nativeSqlSnippets, ctx, static_cast<ParameterExp const*> (valueExp), targetIsVirtual, true);
            }
        else if (IsNullExp(*valueExp))
            {
            if (targetNativeSqlSnippetCount > 0)
                {
                //if value is null exp, we need to pass target operand snippets
                BeAssert(dynamic_cast<LiteralValueExp const*> (valueExp) != nullptr);
                stat = PrepareNullLiteralValueExp(nativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (valueExp), targetNativeSqlSnippetCount);
                }
            }
        else if (targetNativeSqlSnippetCount > 0)
            stat = PrepareValueExp(nativeSqlSnippets, ctx, static_cast<ValueExp const*> (valueExp));

        if (!stat.IsSuccess())
            return stat;

        nativeSqlSnippetLists.push_back(move(nativeSqlSnippets));
        index++;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareWhereExp(NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, WhereExp const* exp)
    {
    if (exp == nullptr)
        return ECSqlStatus::Success;

    nativeSqlSnippet.Append(" WHERE ");
    return PrepareSearchConditionExp(nativeSqlSnippet, ctx, *exp->GetSearchConditionExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::ResolveParameterMappings(ECSqlPrepareContext& context)
    {
    //resolve parameter mappings
    //Parameter index mapping: Vector index = SQLite parameter index (minus 1)
    //                         Vector value = pair of ECSQL parameter index and index of parameter component (for types that map to more than one SQLite parameter)
    //                                        the SQLite parameter maps to
    // Ex: ECSQL: SELECT * FROM Foo WHERE IntProp = ? AND Point3DProp = ? would yield these mappings:
    //     { {1, 0}, // First entry: SQLite index 1 -> Maps to first ECSQL parameter of type Integer. Only one component.
    //       {2, 0}, // Second entry: SQLite index 2 -> Maps to second ECSQL parameter's first component (X column)
    //       {2, 1}, // Third entry: SQLite index 3 -> Maps to second ECSQL parameter's second component (Y column)
    //       {2, 2} } // Fourth entry: SQLite index 4 -> Maps to second ECSQL parameter's third component (Z column)
    auto const& parameterIndexMappings = context.GetSqlBuilder().GetParameterIndexMappings();
    auto& parameterMap = context.GetECSqlStatementR().GetPreparedStatementP()->GetParameterMapR();
    const auto nativeSqlParameterCount = parameterIndexMappings.size();
    if (nativeSqlParameterCount == 0)
        return ECSqlStatus::Success;

    int previousECSqlParameterIndex = 0;
    ECSqlBinder* parameterBinder = nullptr;
    bool isFirstItem = true;
    for (size_t i = 0; i < nativeSqlParameterCount; i++)
        {
        //Parameter indices are 1-based
        const auto nativeSqlIndex = i + 1;
        //corresponding ECSQL parameter index
        const auto ecsqlParameterIndex = parameterIndexMappings[i].GetIndex();
        //If ECSQL parameter maps to more than one SQLite parameters, the component index 
        const auto ecsqlParameterComponentIndex = parameterIndexMappings[i].GetComponentIndex();

        //if ECSQL parameter index is same as before, we don't need to look up the parameter binder again
        if (isFirstItem || ecsqlParameterIndex != previousECSqlParameterIndex)
            {
            ECSqlBinder* binder = nullptr;
            ECSqlStatus stat = ECSqlStatus::Success;
            if (ecsqlParameterIndex > 0)
                stat = parameterMap.TryGetBinder(binder, ecsqlParameterIndex);
            else
                stat = parameterMap.TryGetInternalBinder(binder, (size_t) ((-1) * ecsqlParameterIndex));

            if (!stat.IsSuccess())
                {
                BeAssert(false && "Resolution of parameter mappings failed. Index mismatches.");
                return ECSqlStatus::Error;
                }

            parameterBinder = binder;
            }
        else
            BeAssert(parameterBinder != nullptr);

        parameterBinder->SetSqliteIndex(ecsqlParameterComponentIndex, nativeSqlIndex);
        previousECSqlParameterIndex = ecsqlParameterIndex;
        isFirstItem = false;
        }

    parameterMap.RemapForJoinTable(context);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BooleanSqlOperator ECSqlExpPreparer::DetermineCompoundLogicalOpForCompoundExpressions(BooleanSqlOperator op)
    {
    //for positive operators the elements of the compound exp are ANDed together. For negative ones they are ORed together
    //Ex: ECSQL: MyPoint = MyOtherPoint -> SQL: MyPoint_x = MyOtherPoint_x AND MyPoint_y = MyOtherPoint_y
    //    ECSQL: MyPoint <> MyOtherPoint -> SQL: MyPoint_x <> MyOtherPoint_x OR MyPoint_y <> MyOtherPoint_y

    switch (op)
        {
            case BooleanSqlOperator::IsNot:
            case BooleanSqlOperator::NotBetween:
            case BooleanSqlOperator::NotEqualTo:
            case BooleanSqlOperator::NotIn:
            case BooleanSqlOperator::NotLike:
            case BooleanSqlOperator::NotMatch:
                return BooleanSqlOperator::Or;

            default:
                return BooleanSqlOperator::And;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECSqlExpPreparer::IsNullExp(ExpCR exp)
    {
    return exp.GetType() == Exp::Type::LiteralValue &&
        (static_cast<LiteralValueExp const&> (exp).GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Null);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
