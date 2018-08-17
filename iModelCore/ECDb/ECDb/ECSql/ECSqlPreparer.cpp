/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************** ECSqlPreparer *******************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlPreparer::Prepare(Utf8StringR nativeSql, ECSqlPrepareContext& context, Exp const& exp)
    {
    ECSqlStatus status = ECSqlStatus::Error;
    if (&context.GetDataSourceConnection() != &context.GetECDb() && Exp::Type::Select != exp.GetType())
        {
        context.Issues().Report("Only SELECT queries can be executed against a separate data source connection.");
        return ECSqlStatus::Error;
        }

    switch (exp.GetType())
        {
            case Exp::Type::Select:
            {
            status = ECSqlSelectPreparer::Prepare(context, exp.GetAs<SelectStatementExp>());
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Insert:
            {
            status = ECSqlInsertPreparer::Prepare(context, exp.GetAs<InsertStatementExp>());
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Update:
            {
            status = ECSqlUpdatePreparer::Prepare(context, exp.GetAs<UpdateStatementExp>());
            if (!status.IsSuccess())
                return status;

            break;
            }

            case Exp::Type::Delete:
            {
            status = ECSqlDeletePreparer::Prepare(context, exp.GetAs<DeleteStatementExp>());
            if (!status.IsSuccess())
                return status;

            break;
            }

            default:
                BeAssert(false && "Programmer error in ECSqlPreparer::Preparer.");
                return ECSqlStatus::Error;
        }

    nativeSql.assign(context.GetSqlBuilder().GetSql());
    return ECSqlStatus::Success;
    }

//************** ECSqlExpPreparer *******************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareAllOrAnyExp(ECSqlPrepareContext& ctx, AllOrAnyExp const& exp)
    {
    ctx.Issues().Report("ALL or ANY expression not yet supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBetweenRangeValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BetweenRangeValueExp const& exp)
    {
    NativeSqlBuilder::List lowerBoundSqlTokens;
    ECSqlStatus status = PrepareValueExp(lowerBoundSqlTokens, ctx, *exp.GetLowerBoundOperand());
    if (!status.IsSuccess())
        return status;

    NativeSqlBuilder::List upperBoundSqlTokens;
    status = PrepareValueExp(upperBoundSqlTokens, ctx, *exp.GetUpperBoundOperand());
    if (!status.IsSuccess())
        return status;

    const size_t tokenCount = lowerBoundSqlTokens.size();
    if (tokenCount != upperBoundSqlTokens.size())
        {
        ctx.Issues().Report("Type mismatch between lower bound operand and upper bound operand in BETWEEN expression.");
        return ECSqlStatus::InvalidECSql;
        }

    for (size_t i = 0; i < tokenCount; i++)
        {
        NativeSqlBuilder sql;
        if (exp.HasParentheses())
            sql.AppendParenLeft();

        sql.Append(lowerBoundSqlTokens[i]).Append(" AND ").Append(upperBoundSqlTokens[i]);
        if (exp.HasParentheses())
            sql.AppendParenRight();

        nativeSqlSnippets.push_back(sql);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBinaryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryValueExp const& exp)
    {
    NativeSqlBuilder::List lhsSqlTokens;
    ECSqlStatus status = PrepareValueExp(lhsSqlTokens, ctx, *exp.GetLeftOperand());
    if (!status.IsSuccess())
        return status;

    NativeSqlBuilder::List rhsSqlTokens;
    status = PrepareValueExp(rhsSqlTokens, ctx, *exp.GetRightOperand());
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
        if (exp.HasParentheses())
            nativeSqlBuilder.AppendParenLeft();

        nativeSqlBuilder.Append(lhsSqlTokens[i]).AppendSpace().Append(ExpHelper::ToSql(exp.GetOperator())).AppendSpace().Append(rhsSqlTokens[i]);

        if (exp.HasParentheses())
            nativeSqlBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(nativeSqlBuilder);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBinaryBooleanExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BinaryBooleanExp const& exp)
    {
    const BooleanSqlOperator op = exp.GetOperator();
    ComputedExp const* lhsOperand = exp.GetLeftOperand();
    ComputedExp const* rhsOperand = exp.GetRightOperand();

    const bool lhsIsNullExp = !lhsOperand->IsParameterExp() && lhsOperand->GetTypeInfo().IsNull();
    const bool rhsIsNullExp = !rhsOperand->IsParameterExp() && rhsOperand->GetTypeInfo().IsNull();

    NativeSqlBuilder::List lhsNativeSqlSnippets;
    NativeSqlBuilder::List rhsNativeSqlSnippets;
    if (!lhsIsNullExp)
        {
        ECSqlStatus status = PrepareComputedExp(lhsNativeSqlSnippets, ctx, *lhsOperand);
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
        ECSqlStatus status = PrepareComputedExp(rhsNativeSqlSnippets, ctx, *rhsOperand);
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
        PrepareNullExp(lhsNativeSqlSnippets, ctx, *lhsOperand, targetSqliteSnippetCount);
        }

    if (rhsIsNullExp)
        PrepareNullExp(rhsNativeSqlSnippets, ctx, *rhsOperand, lhsNativeSqlSnippets.size());

    const size_t nativeSqlSnippetCount = lhsNativeSqlSnippets.size();
    if (nativeSqlSnippetCount != rhsNativeSqlSnippets.size())
        {
        BeAssert(false && "Expression could not be translated into SQLite SQL. Operands yielded different number of SQLite SQL expressions.");
        return ECSqlStatus::Error;
        }

    const BooleanSqlOperator logicalCompoundOp = DetermineCompoundLogicalOpForCompoundExpressions(op);
    const bool wrapInParens = exp.HasParentheses() || nativeSqlSnippetCount > 1;

    NativeSqlBuilder sqlBuilder;
    if (wrapInParens)
        sqlBuilder.AppendParenLeft();

    const bool wrapOpInSpaces = op != BooleanSqlOperator::EqualTo &&
        op != BooleanSqlOperator::GreaterThan &&
        op != BooleanSqlOperator::GreaterThanOrEqualTo &&
        op != BooleanSqlOperator::LessThan &&
        op != BooleanSqlOperator::LessThanOrEqualTo;

    bool isFirstSnippet = true;
    for (size_t i = 0; i < nativeSqlSnippetCount; i++)
        {
        if (!isFirstSnippet)
            sqlBuilder.AppendSpace().Append(ExpHelper::ToSql(logicalCompoundOp)).AppendSpace();

        sqlBuilder.Append(lhsNativeSqlSnippets[i]);
        if (wrapOpInSpaces)
            sqlBuilder.AppendSpace();

        sqlBuilder.Append(ExpHelper::ToSql(op));
        if (wrapOpInSpaces)
            sqlBuilder.AppendSpace();

        sqlBuilder.Append(rhsNativeSqlSnippets[i]);
        isFirstSnippet = false;
        }

    if (wrapInParens)
        sqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(sqlBuilder);
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
                return PrepareAllOrAnyExp(ctx, exp.GetAs<AllOrAnyExp>());
            case Exp::Type::BinaryBoolean:
                return PrepareBinaryBooleanExp(nativeSqlSnippets, ctx, exp.GetAs<BinaryBooleanExp>());
            case Exp::Type::BooleanFactor:
                return PrepareBooleanFactorExp(nativeSqlSnippets, ctx, exp.GetAs<BooleanFactorExp>());
            case Exp::Type::SubqueryTest:
                return PrepareSubqueryTestExp(ctx, exp.GetAs<SubqueryTestExp>());
            case Exp::Type::UnaryPredicate:
                return PrepareUnaryPredicateExp(nativeSqlSnippets, ctx, exp.GetAs<UnaryPredicateExp>());

            default:
                BeAssert(false && "ECSqlPreparer::PrepareBooleanExp> Case not handled");
                return ECSqlStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareBooleanFactorExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, BooleanFactorExp const& exp)
    {
    NativeSqlBuilder::List operandSqlSnippets;
    ECSqlStatus status = PrepareBooleanExp(operandSqlSnippets, ctx, *exp.GetOperand());
    if (!status.IsSuccess())
        return status;

    for (NativeSqlBuilder const& operandSqlSnippet : operandSqlSnippets)
        {
        NativeSqlBuilder sqlBuilder;
        if (exp.HasParentheses())
            sqlBuilder.AppendParenLeft();

        if (exp.HasNotOperator())
            sqlBuilder.Append("NOT ");

        sqlBuilder.Append(operandSqlSnippet);

        if (exp.HasParentheses())
            sqlBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(sqlBuilder);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareCastExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, CastExp const& exp)
    {
    ValueExp const* castOperand = exp.GetCastOperand();
    if (!exp.NeedsCasting())
        return PrepareValueExp(nativeSqlSnippets, ctx, *castOperand);

    if (!castOperand->IsParameterExp() && castOperand->GetTypeInfo().IsNull())
        return PrepareNullCastExp(nativeSqlSnippets, ctx, exp);

    NativeSqlBuilder::List operandNativeSqlSnippets;
    const ECSqlStatus stat = PrepareValueExp(operandNativeSqlSnippets, ctx, *castOperand);
    if (!stat.IsSuccess())
        return stat;

    if (operandNativeSqlSnippets.empty())
        {
        BeAssert(false && "Preparing CAST operand did not return a SQLite SQL expression.");
        return ECSqlStatus::Error;
        }

    if (!exp.GetTypeInfo().IsPrimitive())
        {
        ctx.Issues().ReportV("Invalid ECSQL expression '%s': Only primitive types are supported as CAST target type",
                                                               exp.ToECSql().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    const PrimitiveType targetType = exp.GetTypeInfo().GetPrimitiveType();

    for (size_t i = 0; i < operandNativeSqlSnippets.size(); i++)
        {
        NativeSqlBuilder nativeSqlBuilder;
        if (exp.HasParentheses())
            nativeSqlBuilder.AppendParenLeft();

        Utf8String castExpSnippet;
        if (SUCCESS != PrepareCastExpForPrimitive(castExpSnippet, targetType, operandNativeSqlSnippets[i].GetSql()))
            return ECSqlStatus::Error;

        nativeSqlBuilder.Append(castExpSnippet);

        if (exp.HasParentheses())
            nativeSqlBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(nativeSqlBuilder);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareNullCastExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, CastExp const& nullCastExp)
    {
    int sqliteSnippetCount = 0;
    ECSqlTypeInfo const& castTargetTypeInfo = nullCastExp.GetTypeInfo();
    if (castTargetTypeInfo.IsPrimitive())
        {
        switch (castTargetTypeInfo.GetPrimitiveType())
            {
                case PRIMITIVETYPE_Point2d:
                    sqliteSnippetCount = 2;
                    break;

                case PRIMITIVETYPE_Point3d:
                    sqliteSnippetCount = 3;
                    break;

                default:
                    sqliteSnippetCount = 1;
                    break;
            }
        }
    else if (castTargetTypeInfo.IsStruct())
        {
        std::function<void(int& colCount, ECStructClassCR structType)> countColumns;
        countColumns = [&countColumns] (int& colCount, ECStructClassCR structType)
            {
            for (ECPropertyCP prop : structType.GetProperties())
                {
                if (prop->GetIsPrimitive())
                    {
                    switch (prop->GetAsPrimitiveProperty()->GetType())
                        {
                            case PRIMITIVETYPE_Point2d:
                                colCount += 2;
                                continue;

                            case PRIMITIVETYPE_Point3d:
                                colCount += 3;
                                continue;

                            default:
                                colCount++;
                                continue;
                        }
                    }

                if (prop->GetIsStruct())
                    {
                    countColumns(colCount, prop->GetAsStructProperty()->GetType());
                    continue;
                    }

                colCount++;
                }
            };

        countColumns(sqliteSnippetCount, castTargetTypeInfo.GetStructType());
        }
    else //all other types are mapped to a single column
        sqliteSnippetCount = 1;

    for (int i = 0; i < sqliteSnippetCount; i++)
        {
        nativeSqlSnippets.push_back(NativeSqlBuilder("NULL"));
        }

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECSqlExpPreparer::PrepareCastExpForPrimitive(Utf8StringR sqlSnippet, ECN::PrimitiveType primTargetType, Utf8StringCR castOperandSnippet)
    {
    Utf8CP castFormat = nullptr;
    switch (primTargetType)
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
            case PRIMITIVETYPE_Point2d:
            case PRIMITIVETYPE_Point3d:
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
                return ERROR;
        }

    sqlSnippet.Sprintf(castFormat, castOperandSnippet.c_str());
    return SUCCESS;
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareClassNameExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ClassNameExp const& exp)
    {
    const ECSqlType currentScopeECSqlType = ctx.GetCurrentScope().GetECSqlType();
    ClassMap const& classMap = exp.GetInfo().GetMap();

    Policy policy = PolicyManager::GetPolicy(ClassIsValidInECSqlPolicyAssertion(classMap, currentScopeECSqlType, exp.IsPolymorphic()));
    if (!policy.IsSupported())
        {
        ctx.Issues().ReportV("Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    DbTable const* table = nullptr;
    switch (currentScopeECSqlType)
        {
            case ECSqlType::Select:
            {
            NativeSqlBuilder classViewSql;
            if (SUCCESS != ViewGenerator::GenerateSelectFromViewSql(classViewSql, ctx, classMap, exp.IsPolymorphic(), exp.GetMemberFunctionCallExp()))
                return ECSqlStatus::InvalidECSql;

            classViewSql.AppendSpace().AppendEscaped(exp.GetId());
            nativeSqlSnippets.push_back(classViewSql);
            return ECSqlStatus::Success;
            }

            case ECSqlType::Insert:
            {
            BeAssert(!exp.IsPolymorphic());
            SingleContextTableECSqlPreparedStatement& preparedStmt = ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>();
            table = &preparedStmt.GetContextTable();
            break;
            }

            case ECSqlType::Update:
            {
            SingleContextTableECSqlPreparedStatement& preparedStmt = ctx.GetPreparedStatement<SingleContextTableECSqlPreparedStatement>();
            table = &preparedStmt.GetContextTable();
            break;
            }

            case ECSqlType::Delete:
            {
            table = &classMap.GetPrimaryTable();
            break;
            }

            default:
                BeAssert(false);
                return ECSqlStatus::Error;
        }

    BeAssert(table != nullptr);
    BeAssert(table->GetType() != DbTable::Type::Virtual);
    NativeSqlBuilder nativeSqlSnippet;
    nativeSqlSnippet.AppendEscaped(table->GetName());
    nativeSqlSnippets.push_back(nativeSqlSnippet);
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
                return PrepareClassNameExp(nativeSqlSnippets, ctx, exp.GetAs<ClassNameExp>());
            case Exp::Type::CrossJoin:
                return PrepareCrossJoinExp(ctx, exp.GetAs<CrossJoinExp>());
            case Exp::Type::ECRelationshipJoin:
                return PrepareRelationshipJoinExp(ctx, exp.GetAs<ECRelationshipJoinExp>());
            case Exp::Type::NaturalJoin:
                return PrepareNaturalJoinExp(ctx, exp.GetAs<NaturalJoinExp>());
            case Exp::Type::QualifiedJoin:
                return PrepareQualifiedJoinExp(ctx, exp.GetAs<QualifiedJoinExp>());
            case Exp::Type::SubqueryRef:
                return PrepareSubqueryRefExp(ctx, exp.GetAs<SubqueryRefExp>());
        }

    BeAssert(false && "Unhandled ClassRef expression case");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareComputedExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ComputedExp const& exp)
    {
    //all subclasses of BooleanExp are handled by PrepareBooleanExp
    BooleanExp const* booleanExp = dynamic_cast<BooleanExp const*> (&exp);
    if (booleanExp != nullptr)
        return PrepareBooleanExp(nativeSqlSnippets, ctx, *booleanExp);

    //all subclasses of ValueExp are handled by PrepareValueExp
    ValueExp const* valueExp = dynamic_cast<ValueExp const*> (&exp);
    if (valueExp != nullptr)
        return PrepareValueExp(nativeSqlSnippets, ctx, *valueExp);

    if (exp.GetType() == Exp::Type::ValueExpList)
        return PrepareValueExpListExp(nativeSqlSnippets, ctx, exp.GetAs<ValueExpListExp>(), /* encloseInParentheses = */ true);

    BeAssert(false && "ECSqlPreparer::PrepareComputedExp: Unhandled ComputedExp subclass.");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareEnumValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, EnumValueExp const& exp)
    {
    NativeSqlBuilder nativeSqlBuilder;

    if (exp.HasParentheses())
        nativeSqlBuilder.AppendParenLeft();

    ECEnumeratorCR enumerator = exp.GetEnumerator();
    if (enumerator.IsInteger())
        nativeSqlBuilder.AppendFormatted("%" PRId32, enumerator.GetInteger());
    else if (enumerator.IsString())
        nativeSqlBuilder.AppendQuoted(enumerator.GetString().c_str());
    else
        {
        ctx.Issues().ReportV("Unsupported ECEnumeration %s. Only integer and string enumerations are supported.", enumerator.GetEnumeration().GetFullName().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    if (exp.HasParentheses())
        nativeSqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(nativeSqlBuilder);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareLiteralValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LiteralValueExp const& exp)
    {
    NativeSqlBuilder nativeSqlBuilder;
    ECSqlTypeInfo const& typeInfo = exp.GetTypeInfo();
    if (typeInfo.IsNull())
        {
        BeAssert(false && "NULL exp should never be called indirectly but always in context of its RHS exp");
        return PrepareNullExp(nativeSqlSnippets, ctx, exp, 1);
        }

    Utf8StringCR expRawValue = exp.GetRawValue();

    if (exp.HasParentheses())
        nativeSqlBuilder.AppendParenLeft();
    
    if (typeInfo.IsPrimitive())
        {
        switch (typeInfo.GetPrimitiveType())
            {
                case PRIMITIVETYPE_Boolean:
                {
                ECValue val;
                if (SUCCESS != exp.TryParse(val))
                    {
                    ctx.Issues().ReportV("Invalid boolean literal in expression '%s'", exp.ToECSql().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }

                Utf8CP nativeSqlBooleanVal = val.GetBoolean() ? SQLVAL_True : SQLVAL_False;
                nativeSqlBuilder.Append(nativeSqlBooleanVal);
                break;
                }

                case PRIMITIVETYPE_DateTime:
                {
                nativeSqlBuilder.Append("JULIANDAY(").AppendQuoted(expRawValue.c_str()).AppendParenRight();
                break;
                }

                case PRIMITIVETYPE_String:
                    nativeSqlBuilder.AppendQuoted(LiteralValueExp::EscapeStringLiteral(expRawValue).c_str());
                    break;

                default:
                    nativeSqlBuilder.Append(expRawValue.c_str());
                    break;
            }
        }
    else
        nativeSqlBuilder.Append(expRawValue.c_str());

    if (exp.HasParentheses())
        nativeSqlBuilder.AppendParenRight();

    nativeSqlSnippets.push_back(nativeSqlBuilder);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareNullExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ComputedExp const& exp, size_t targetExpNativeSqlSnippetCount)
    {
    if (targetExpNativeSqlSnippetCount == 0)
        {
        BeAssert(false && "NULL expression could not be translated into SQLite SQL. Target operand yielded empty SQLite SQL expression.");
        return ECSqlStatus::Error;
        }

    for (size_t i = 0; i < targetExpNativeSqlSnippetCount; i++)
        {
        if (exp.HasParentheses())
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
    ctx.Issues().Report("Cross join expression not yet supported.");
    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFromExp(ECSqlPrepareContext& ctx, FromExp const& fromClause)
    {
    NativeSqlBuilder& sqlGenerator = ctx.GetSqlBuilder();

    sqlGenerator.Append("FROM ");
    bool isFirstItem = true;
    for (Exp const* classRefExp : fromClause.GetChildren())
        {
        if (!isFirstItem)
            sqlGenerator.AppendComma();

        ECSqlStatus status = PrepareClassRefExp(sqlGenerator, ctx, classRefExp->GetAs<ClassRefExp>());
        if (!status.IsSuccess())
            return status;

        isFirstItem = false;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareGroupByExp(ECSqlPrepareContext& ctx, GroupByExp const& exp)
    {
    ctx.GetSqlBuilder().Append(" GROUP BY ");

    NativeSqlBuilder::List groupingValuesSnippetList;
    const ECSqlStatus stat = PrepareValueExpListExp(groupingValuesSnippetList, ctx, *exp.GetGroupingValueListExp(), /* encloseInParentheses = */ false);
    if (!stat.IsSuccess())
        return stat;

    ctx.GetSqlBuilder().Append(groupingValuesSnippetList);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareHavingExp(ECSqlPrepareContext& ctx, HavingExp const& exp)
    {
    ctx.GetSqlBuilder().Append(" HAVING ");
    return PrepareSearchConditionExp(ctx.GetSqlBuilder(), ctx, *exp.GetSearchConditionExp());
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlExpPreparer::PrepareLikeRhsValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, LikeRhsValueExp const& exp)
    {
    ECSqlStatus stat = PrepareValueExp(nativeSqlSnippets, ctx, *exp.GetRhsExp());
    if (!stat.IsSuccess())
        return stat;

    if (nativeSqlSnippets.size() != 1)
        {
        //This is a programmer error as the parse step should already check that the like expression is of string type
        BeAssert(false && "LIKE RHS expression is expected to result in a single SQLite SQL snippet as LIKE only works with string operands.");
        return ECSqlStatus::Error;
        }

    if (exp.HasEscapeExp())
        {
        NativeSqlBuilder::List escapeExpSqlSnippets;
        ECSqlStatus stat = PrepareValueExp(escapeExpSqlSnippets, ctx, *exp.GetEscapeExp());
        if (!stat.IsSuccess())
            return stat;

        if (escapeExpSqlSnippets.size() != 1)
            {
            ctx.Issues().Report("Invalid type in LIKE ESCAPE expression. ESCAPE only works with a string value.");
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
ECSqlStatus ECSqlExpPreparer::PrepareLimitOffsetExp(ECSqlPrepareContext& ctx, LimitOffsetExp const& exp)
    {
    ctx.GetSqlBuilder().Append(" LIMIT ");

    NativeSqlBuilder::List sqlSnippets;
    ECSqlStatus stat = PrepareValueExp(sqlSnippets, ctx, *exp.GetLimitExp());
    if (!stat.IsSuccess())
        return stat;

    ctx.GetSqlBuilder().Append(sqlSnippets[0]);

    if (exp.HasOffset())
        {
        ctx.GetSqlBuilder().Append(" OFFSET ");
        sqlSnippets.clear();
        ECSqlStatus stat = PrepareValueExp(sqlSnippets, ctx, *exp.GetOffsetExp());
        if (!stat.IsSuccess())
            return stat;

        ctx.GetSqlBuilder().Append(sqlSnippets[0]);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareNaturalJoinExp(ECSqlPrepareContext& ctx, NaturalJoinExp const& exp)
    {
    ctx.Issues().Report("Natural join expression not yet supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareOrderByExp(ECSqlPrepareContext& ctx, OrderByExp const& exp)
    {
    ctx.PushScope(exp);

    NativeSqlBuilder orderBySqlBuilder;
    bool isFirstSpec = true;
    for (Exp const* child : exp.GetChildren())
        {
        OrderBySpecExp const& specification =child->GetAs<OrderBySpecExp>();

        ComputedExp const* sortExp = specification.GetSortExpression();
        NativeSqlBuilder::List sqlSnippets;
        bool isPredicate = dynamic_cast<BooleanExp const*>(sortExp) != nullptr;
        //can validly return empty snippets (e.g. if prop ref maps to virtual column
        ECSqlStatus r = PrepareComputedExp(sqlSnippets, ctx, *sortExp);
        if (!r.IsSuccess())
            return r;

        bool isFirstSnippet = true;
        for (NativeSqlBuilder const& sqlSnippet : sqlSnippets)
            {
            if (!isFirstSpec)
                {
                if (!isFirstSnippet && isPredicate)
                    orderBySqlBuilder.Append(" AND ");
                else
                    orderBySqlBuilder.AppendComma();
                }

            orderBySqlBuilder.Append(sqlSnippet);
            switch (specification.GetSortDirection())
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
        ctx.GetSqlBuilder().Append("ORDER BY ").Append(orderBySqlBuilder);

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareParameterExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ParameterExp const& exp)
    {
    BeAssert(exp.GetTypeInfo().GetKind() != ECSqlTypeInfo::Kind::Unset);

    BeAssert((ctx.GetPreparedStatement().GetType() != ECSqlType::Insert && ctx.GetPreparedStatement().GetType() != ECSqlType::Update) || exp.IsNamedParameter() && "For INSERT and UPDATE parameters ECDb converts all params to named ones");

    ECSqlParameterMap& ecsqlParameterMap = ctx.GetPreparedStatement().GetParameterMapR();
    ECSqlBinder* binder = nullptr;
    const bool binderAlreadyExists = exp.IsNamedParameter() && ecsqlParameterMap.TryGetBinder(binder, exp.GetParameterName());
    if (!binderAlreadyExists)
        {
        binder = ecsqlParameterMap.AddBinder(ctx, exp);
        if (binder == nullptr)
            return ECSqlStatus::Error;
        }

    for (Utf8StringCR sqlParamName : binder->GetMappedSqlParameterNames())
        {
        NativeSqlBuilder parameterBuilder;
        if (exp.HasParentheses())
            parameterBuilder.AppendParenLeft();

        parameterBuilder.Append(sqlParamName.c_str());

        if (exp.HasParentheses())
            parameterBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(parameterBuilder);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareQualifiedJoinExp(ECSqlPrepareContext& ctx, QualifiedJoinExp const& exp)
    {
    NativeSqlBuilder& sqlBuilder = ctx.GetSqlBuilder();
    ECSqlStatus r = PrepareClassRefExp(sqlBuilder, ctx, exp.GetFromClassRef());
    if (!r.IsSuccess())
        return r;

    //ECSQL_LIMITATION: 
    //https://www.sqlite.org/omitted.html
    //RIGHT and FULL OUTER JOIN  LEFT OUTER JOIN is implemented, but not RIGHT OUTER JOIN or FULL OUTER JOIN.
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
            ctx.Issues().Report("'RIGHT OUTER JOIN' is currently not supported");
            return ECSqlStatus::InvalidECSql;
            }
            case ECSqlJoinType::FullOuterJoin:
            {
            //ECSQL_TODO: way around full outer join 
            //http://stackoverflow.com/questions/1923259/full-outer-join-with-sqlite
            ctx.Issues().Report("'FULL OUTER JOIN' is currently not supported");
            return ECSqlStatus::InvalidECSql;
            }
        }

    r = PrepareClassRefExp(sqlBuilder, ctx, exp.GetToClassRef());
    if (!r.IsSuccess())
        return r;

    if (exp.GetJoinSpec()->GetType() == Exp::Type::JoinCondition)
        {
        JoinConditionExp const& joinCondition = exp.GetJoinSpec()->GetAs<JoinConditionExp>();
        sqlBuilder.Append(" ON ");

        NativeSqlBuilder::List sqlSnippets;
        r = PrepareBooleanExp(sqlSnippets, ctx, *joinCondition.GetSearchCondition());
        if (!r.IsSuccess())
            return r;

        bool isFirstSnippet = true;
        for (NativeSqlBuilder const& sqlSnippet : sqlSnippets)
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
        ctx.Issues().Report("JOIN <class/subquery> USING (property,...) is not supported yet.");
        return ECSqlStatus::InvalidECSql;
        }

    BeAssert(false && "Invalid case");
    return ECSqlStatus::Error;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareQueryExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, QueryExp const& exp)
    {
    ctx.Issues().Report("Query expression not yet supported.");
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

    NativeSqlBuilder& sql = ctx.GetSqlBuilder();

    ///Resolve direction of the relationship
    ECRelationshipJoinExp::ResolvedEndPoint const& fromEP = exp.GetResolvedFromEndPoint();
    ECRelationshipJoinExp::ResolvedEndPoint const& toEP = exp.GetResolvedToEndPoint();
    JoinDirection direction = exp.GetDirection();

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
                    } 
                    break;
                    case JoinDirection::Backward:
                    {
                    fromIsSource = TriState::False;
                    } 
                    break;
                };
            break;
            }
            case ECRelationshipJoinExp::ClassLocation::ExistInSource:
            {
            if (direction != JoinDirection::Implied)
                {
                if (direction != JoinDirection::Forward)
                    {
                    ctx.Issues().ReportV("Invalid join direction BACKWARD in %s. Either specify FORWARD or omit the direction as the direction can be unambiguously implied in this ECSQL.", exp.ToString().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }
                }

            fromIsSource = TriState::True;
            } 
            break;
            case ECRelationshipJoinExp::ClassLocation::ExistInTarget:
            {
            if (direction != JoinDirection::Implied)
                {
                if (direction != JoinDirection::Backward)
                    {
                    ctx.Issues().ReportV("Invalid join direction FORWARD in %s. Either specify BACKWARD or omit the direction as the direction can be unambiguously implied in this ECSQL.", exp.ToString().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }
                }

            fromIsSource = TriState::False;
            }
            break;
        }

    PRECONDITION(fromIsSource != TriState::None, ECSqlStatus::Error);
    ////Determine the from/to related keys
    Utf8CP fromRelatedKey = nullptr;
    Utf8CP toRelatedKey = nullptr;
    if (fromIsSource == TriState::True)
        {
        fromRelatedKey = ECDBSYS_PROP_SourceECInstanceId;
        toRelatedKey = ECDBSYS_PROP_TargetECInstanceId;
        }
    else
        {
        fromRelatedKey = ECDBSYS_PROP_TargetECInstanceId;
        toRelatedKey = ECDBSYS_PROP_SourceECInstanceId;
        }

    ClassNameExp const& relationshipClassNameExp = exp.GetRelationshipClassNameExp();

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
    if (SUCCESS != ViewGenerator::GenerateSelectFromViewSql(relationshipView, ctx, relationshipClassNameExp.GetInfo().GetMap(), true))
        {
        BeAssert(false && "Generating class view during preparation of relationship class name expression failed.");
        return ECSqlStatus::Error;
        }

    sql.Append(relationshipView);
    sql.AppendSpace();
    sql.AppendEscaped(relationshipClassNameExp.GetId().c_str());

    sql.Append(" ON ");
    {
    ECInstanceIdPropertyMap const* fromECInstanceIdPropMap = fromEP.GetClassNameRef()->GetInfo().GetMap().GetECInstanceIdPropertyMap();
    if (fromECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    ToSqlPropertyMapVisitor fromECInstanceIdSqlVisitor(*fromECInstanceIdPropMap->GetTables().front(), ToSqlPropertyMapVisitor::ECSqlScope::Select, fromEP.GetClassNameRef()->GetId());
    fromECInstanceIdPropMap->AcceptVisitor(fromECInstanceIdSqlVisitor);
    if (fromECInstanceIdSqlVisitor.GetResultSet().size() != 1)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    sql.Append(fromECInstanceIdSqlVisitor.GetResultSet().front().GetSqlBuilder().GetSql());
    sql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo));
    }

    {
    PropertyMap const* fromRelatedIdPropMap = relationshipClassNameExp.GetInfo().GetMap().GetPropertyMaps().Find(fromRelatedKey);
    if (fromRelatedIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    
    ToSqlPropertyMapVisitor fromRelatedIdSqlVisitor(*fromRelatedIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>().GetTables().front(), ToSqlPropertyMapVisitor::ECSqlScope::Select, relationshipClassNameExp.GetId());
    fromRelatedIdPropMap->AcceptVisitor(fromRelatedIdSqlVisitor);
    if (fromRelatedIdSqlVisitor.GetResultSet().size() != 1)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    sql.Append(fromRelatedIdSqlVisitor.GetResultSet().front().GetSqlBuilder().GetSql());

    //RelationView To ToECClass
    sql.Append(" INNER JOIN ");
    r = PrepareClassRefExp(sql, ctx, exp.GetToClassRef());
    if (!r.IsSuccess())
        return r;

    sql.Append(" ON ");
    }

    {
    ECInstanceIdPropertyMap const*  toECInstanceIdPropMap = toEP.GetClassNameRef()->GetInfo().GetMap().GetECInstanceIdPropertyMap();
    if (toECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    ToSqlPropertyMapVisitor toECInstanceIdSqlVisitor(*toECInstanceIdPropMap->GetTables().front(), ToSqlPropertyMapVisitor::ECSqlScope::Select, toEP.GetClassNameRef()->GetId());
    toECInstanceIdPropMap->AcceptVisitor(toECInstanceIdSqlVisitor);
    if (toECInstanceIdSqlVisitor.GetResultSet().size() != 1)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    sql.Append(toECInstanceIdSqlVisitor.GetResultSet().front().GetSqlBuilder().GetSql());
    sql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo));
    }

    {
    PropertyMap const* toRelatedIdPropMap = relationshipClassNameExp.GetInfo().GetMap().GetPropertyMaps().Find(toRelatedKey);
    if (toRelatedIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    ToSqlPropertyMapVisitor toRelatedIdSqlVisitor(*toRelatedIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>().GetTables().front(), ToSqlPropertyMapVisitor::ECSqlScope::Select, relationshipClassNameExp.GetId());
    toRelatedIdPropMap->AcceptVisitor(toRelatedIdSqlVisitor);
    if (toRelatedIdSqlVisitor.GetResultSet().size() != 1)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }
    sql.Append(toRelatedIdSqlVisitor.GetResultSet().front().GetSqlBuilder().GetSql());
    }

    return ECSqlStatus::Success;
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFunctionCallExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, FunctionCallExp const& exp)
    {
    Utf8StringCR functionName = exp.GetFunctionName();
    NativeSqlBuilder nativeSql;
    if (exp.HasParentheses())
        nativeSql.AppendParenLeft();

    const bool isAnyOrSomeFunction = functionName.EqualsIAscii("any") || functionName.EqualsIAscii("some");
    const bool isEveryFunction = !isAnyOrSomeFunction && functionName.EqualsIAscii("every");
    const bool isAnyEveryOrSomeFunction = isAnyOrSomeFunction || isEveryFunction;
    const bool isCurrentDateTimeFunction = functionName.EqualsIAscii(FunctionCallExp::CURRENT_DATE()) || functionName.EqualsIAscii(FunctionCallExp::CURRENT_TIMESTAMP());
    if (isAnyEveryOrSomeFunction)
        {
        BeAssert(exp.GetChildrenCount() == 1 && "ANY, SOME, EVERY functions expect a single arg");

        //ANY, EVERY, SOME is not directly supported by SQLite. But they can be expressed by standard functions
        //ANY,SOME: checks whether at least one row in the specified BOOLEAN column is TRUE -> MAX(Col) <> 0
        //EVERY: checks whether all rows in the specified BOOLEAN column are TRUE -> MIN(Col) <> 0
        nativeSql.Append(isEveryFunction ? "MIN" : "MAX");
        }
    else if (isCurrentDateTimeFunction)
        {
        BeAssert(exp.IsGetter() && "CURRENT_DATE or CURRENT_TIMESTAMP are expected to not have args and parentheses");
        //Note: CURRENT_TIMESTAMP in SQLite returns a UTC timestamp. ECSQL specifies CURRENT_TIMESTAMP as UTC, too,
        //so no conversion needed.
        nativeSql.Append("JULIANDAY(").Append(functionName).AppendParenRight();
        }
    else
        nativeSql.Append(functionName);

    if (!exp.IsGetter())
        {
        nativeSql.AppendParenLeft();

        if (exp.GetSetQuantifier() != SqlSetQuantifier::NotSpecified)
            nativeSql.Append(ExpHelper::ToSql(exp.GetSetQuantifier())).AppendSpace();

        NativeSqlBuilder::List argSqlSnippets;
        ECSqlStatus stat = PrepareFunctionArgList(argSqlSnippets, ctx, exp);
        if (stat != ECSqlStatus::Success)
            return stat;

        nativeSql.Append(argSqlSnippets, ",");

        if (isAnyEveryOrSomeFunction)
            {
            BeAssert(argSqlSnippets.size() == 1);
            nativeSql.Append(" <> 0");
            }

        nativeSql.AppendParenRight(); //function arg list parent
        }

    if (exp.HasParentheses())
        nativeSql.AppendParenRight();

    nativeSqlSnippets.push_back(nativeSql);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareFunctionArgList(NativeSqlBuilder::List& argSqlSnippets, ECSqlPrepareContext& ctx, ValueExp const& functionCallExp)
    {
    if (functionCallExp.GetType() != Exp::Type::FunctionCall && functionCallExp.GetType() != Exp::Type::MemberFunctionCall)
        {
        BeAssert(false && "May not call PrepareFunctionArgList on an expression which is neither a FunctionCallExp nor a MemberFunctionCallExp");
        return ECSqlStatus::InvalidECSql;
        }

    for (Exp const* childExp : functionCallExp.GetChildren())
        {
        ValueExp const& argExp = childExp->GetAs<ValueExp>();

        NativeSqlBuilder::List nativeSqlArgumentList;
        ECSqlStatus status;
        if (!argExp.IsParameterExp() && argExp.GetTypeInfo().IsNull())
            {
            //for functions we only support args of single column primitive types so far, therefore an ECSQL NULL
            //always means a single SQLite NULL
            status = PrepareNullExp(nativeSqlArgumentList, ctx, argExp, 1);
            }
        else
            status = PrepareValueExp(nativeSqlArgumentList, ctx, argExp);

        if (!status.IsSuccess())
            return status;

        if (nativeSqlArgumentList.size() != 1)
            {
            ctx.Issues().ReportV("Failed to prepare function expression '%s': Functions in ECSQL can only accept primitive scalar arguments (i.e. excluding Point2d/Point3d).", functionCallExp.ToECSql().c_str());
            return ECSqlStatus::InvalidECSql;
            }

        argSqlSnippets.push_back(nativeSqlArgumentList[0]);
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
        if (searchConditionExp.GetAs<BinaryBooleanExp>().GetOperator() == BooleanSqlOperator::Or)
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
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryExp(ECSqlPrepareContext& ctx, SubqueryExp const& exp)
    {
    ctx.GetSqlBuilder().AppendParenLeft();
    ECSqlStatus stat = ECSqlSelectPreparer::Prepare(ctx, *exp.GetQuery());
    ctx.GetSqlBuilder().AppendParenRight();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryRefExp(ECSqlPrepareContext& ctx, SubqueryRefExp const& exp)
    {
    ECSqlStatus status = PrepareSubqueryExp(ctx, *exp.GetSubquery());
    if (!status.IsSuccess())
        return status;

    if (!exp.GetAlias().empty())
        ctx.GetSqlBuilder().AppendSpace().AppendQuoted(exp.GetAlias().c_str());

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryTestExp(ECSqlPrepareContext& ctx, SubqueryTestExp const& exp)
    {
    ctx.Issues().Report("SubqueryTest expression not supported.");
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareSubqueryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, SubqueryValueExp const& exp)
    {
    ctx.GetSqlBuilder().Push();
    ECSqlStatus st = PrepareSubqueryExp(ctx, *exp.GetQuery());
    nativeSqlSnippets.push_back(NativeSqlBuilder(ctx.GetSqlBuilder().Pop()));
    return st;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareUnaryPredicateExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryPredicateExp const& exp)
    {
    return PrepareValueExp(nativeSqlSnippets, ctx, *exp.GetValueExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareUnaryValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, UnaryValueExp const& exp)
    {
    NativeSqlBuilder::List unaryOperandSqlBuilders;
    ECSqlStatus status = PrepareValueExp(unaryOperandSqlBuilders, ctx, *exp.GetOperand());
    if (!status.IsSuccess())
        return status;

    BeAssert(unaryOperandSqlBuilders.size() <= 1 && "UnaryExp with Points and non-primitive types not supported yet.");

    for (NativeSqlBuilder const& unaryOperandSqlBuilder : unaryOperandSqlBuilders)
        {
        NativeSqlBuilder unaryExpBuilder;
        if (exp.HasParentheses())
            unaryExpBuilder.AppendParenLeft();

        unaryExpBuilder.Append(ExpHelper::ToSql(exp.GetOperator())).Append(unaryOperandSqlBuilder);

        if (exp.HasParentheses())
            unaryExpBuilder.AppendParenRight();

        nativeSqlSnippets.push_back(unaryExpBuilder);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::PrepareValueExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::BetweenRangeValue:
                return PrepareBetweenRangeValueExp(nativeSqlSnippets, ctx, exp.GetAs<BetweenRangeValueExp>());
            case Exp::Type::BinaryValue:
                return PrepareBinaryValueExp(nativeSqlSnippets, ctx, exp.GetAs<BinaryValueExp>());
            case Exp::Type::Cast:
                return PrepareCastExp(nativeSqlSnippets, ctx, exp.GetAs<CastExp>());
            case Exp::Type::LiteralValue:
                return PrepareLiteralValueExp(nativeSqlSnippets, ctx, exp.GetAs<LiteralValueExp>());
            case Exp::Type::FunctionCall:
                return PrepareFunctionCallExp(nativeSqlSnippets, ctx, exp.GetAs<FunctionCallExp>());
            case Exp::Type::LikeRhsValue:
                return PrepareLikeRhsValueExp(nativeSqlSnippets, ctx, exp.GetAs<LikeRhsValueExp>());
            case Exp::Type::Parameter:
                return PrepareParameterExp(nativeSqlSnippets, ctx, exp.GetAs<ParameterExp>());
            case Exp::Type::PropertyName:
                return ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, exp.GetAs<PropertyNameExp>());
            case Exp::Type::SubqueryValue:
                return PrepareSubqueryValueExp(nativeSqlSnippets, ctx, exp.GetAs<SubqueryValueExp>());
            case Exp::Type::UnaryValue:
                return PrepareUnaryValueExp(nativeSqlSnippets, ctx, exp.GetAs<UnaryValueExp>());
            case Exp::Type::EnumValue:
                return PrepareEnumValueExp(nativeSqlSnippets, ctx, exp.GetAs<EnumValueExp>());
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
ECSqlStatus ECSqlExpPreparer::PrepareValueExpListExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, ValueExpListExp const& exp, bool encloseInParentheses)
    {
    BeAssert(nativeSqlSnippets.empty());
    bool isFirstExp = true;
    for (Exp const* valueExp : exp.GetChildren())
        {
        NativeSqlBuilder::List listItemExpBuilders;
        auto stat = PrepareValueExp(listItemExpBuilders, ctx, valueExp->GetAs<ValueExp>());
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
                nativeSqlSnippets.push_back(builder);
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
ECSqlStatus ECSqlExpPreparer::PrepareValueExpListExp(NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, ValueExpListExp const& exp, NativeSqlBuilder::ListOfLists& targetNativeSqlSnippetLists)
    {
    BeAssert(nativeSqlSnippetLists.empty());
    size_t index = 0;
    for (Exp const* childExp : exp.GetChildren())
        {
        BeAssert(childExp != nullptr);
        ValueExp const& valueExp = childExp->GetAs<ValueExp>();
        const size_t targetNativeSqlSnippetCount = targetNativeSqlSnippetLists[index].size();

        NativeSqlBuilder::List nativeSqlSnippets;

        //If target expression does not have any SQL snippets, it means the expression is not necessary in SQLite SQL (e.g. for source/target class id props)
        //In that case the respective value exp does not need to be prepared either.
        ECSqlStatus stat = ECSqlStatus::Success;
        if (valueExp.IsParameterExp())
            stat = PrepareParameterExp(nativeSqlSnippets, ctx, valueExp.GetAs<ParameterExp>());
        else if (valueExp.GetTypeInfo().IsNull())
            {
            if (targetNativeSqlSnippetCount > 0)
                {
                //if value is null exp, we need to pass target operand snippets
                stat = PrepareNullExp(nativeSqlSnippets, ctx, valueExp, targetNativeSqlSnippetCount);
                }
            }
        else if (targetNativeSqlSnippetCount > 0)
            stat = PrepareValueExp(nativeSqlSnippets, ctx, valueExp);

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
ECSqlStatus ECSqlExpPreparer::PrepareWhereExp(NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, WhereExp const& exp)
    {
    nativeSqlSnippet.Append("WHERE ");
    return PrepareSearchConditionExp(nativeSqlSnippet, ctx, *exp.GetSearchConditionExp());
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlExpPreparer::GenerateECClassIdFilter(Utf8StringR filterSqlExpression, ClassNameExp const& exp)
    {
    ClassMap const& classMap = exp.GetInfo().GetMap();
    DbTable const& contextTable = classMap.GetPrimaryTable();
    DbColumn const& classIdColumn = contextTable.GetECClassIdColumn();

    //if no class id column exists -> no system where clause
    if (classIdColumn.GetPersistenceType() == PersistenceType::Virtual)
        return ECSqlStatus::Success;

    StorageDescription const& desc = classMap.GetStorageDescription();
    Partition const* partition = desc.GetHorizontalPartition(contextTable);
    if (partition == nullptr)
        {
        if (!desc.GetVerticalPartitions().empty())
            partition = desc.GetVerticalPartition(contextTable);

        if (partition == nullptr)
            {
            BeAssert(false && "Should always find a partition for the given table");
            return ECSqlStatus::Error;
            }
        }

    Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
    classMap.GetClass().GetId().ToString(classIdStr);

    Utf8String classIdColSql(classIdColumn.GetName());

    if (!exp.IsPolymorphic())
        {
        if (partition->IsSharedTable())
            filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);

        return ECSqlStatus::Success;
        }

    if (partition->NeedsECClassIdFilter())
        filterSqlExpression.append(classIdColSql).append(" IN (SELECT ClassId FROM [").append(classMap.GetSchemaManager().GetTableSpace().GetName()).append("]." TABLE_ClassHierarchyCache " WHERE BaseClassId=").append(classIdStr).append(")");

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

END_BENTLEY_SQLITE_EC_NAMESPACE
