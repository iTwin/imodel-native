/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ValueExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//NOTE: The code in this file is sorted by the class names. This should make maintenance easier as there are too many types in this single file

//*************************** BetweenRangeValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus BetweenRangeValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        {
        //indicate that the exp per se doesn't have a single type info as it can vary across its children
        SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies));
        return FinalizeParseStatus::NotCompleted;
        }

    if (mode == FinalizeParseMode::AfterFinalizingChildren)
        {
        std::vector<ValueExp const*> operands {GetLowerBoundOperand(), GetUpperBoundOperand()};
        for (ValueExp const* operand : operands)
            {
            //parameter exp type is determined later, so do not check type for it here
            if (operand->IsParameterExp())
                continue;

            ECSqlTypeInfo const& typeInfo = operand->GetTypeInfo();
            if (!typeInfo.IsPrimitive() || typeInfo.IsGeometry() || typeInfo.IsPoint() || typeInfo.IsNavigation())
                {
                ctx.Issues().Report("Invalid BETWEEN expression '%s'. Operands must be of numeric, date/timestamp, or string type.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }
            }

        return FinalizeParseStatus::Completed;
        }

    BeAssert(false);
    return FinalizeParseStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void BetweenRangeValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetLowerBoundOperand()).AppendToECSql(" AND ").AppendToECSql(*GetUpperBoundOperand());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//*************************** BinaryValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus BinaryValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    switch (mode)
        {
            case Exp::FinalizeParseMode::BeforeFinalizingChildren:
            {
            switch (m_op)
                {
                    case BinarySqlOperator::Plus:
                    case BinarySqlOperator::Minus:
                    case BinarySqlOperator::Modulo:
                    case BinarySqlOperator::Divide:
                    case BinarySqlOperator::Multiply:
                        SetTypeInfo(ECSqlTypeInfo(PRIMITIVETYPE_Double));
                        break;
                    case BinarySqlOperator::ShiftLeft:
                    case BinarySqlOperator::ShiftRight:
                    case BinarySqlOperator::BitwiseOr:
                    case BinarySqlOperator::BitwiseAnd:
                    case BinarySqlOperator::BitwiseXOr:
                        SetTypeInfo(ECSqlTypeInfo(PRIMITIVETYPE_Long));
                        break;

                    case BinarySqlOperator::Concat:
                        SetTypeInfo(ECSqlTypeInfo(PRIMITIVETYPE_String));
                        break;

                    default:
                        BeAssert(false && "Adjust BinaryValueExp::_FinalizeParsing for new value of BinarySqlOperator enum.");
                        LOG.error("Adjust BinaryValueExp::_FinalizeParsing for new value of BinarySqlOperator enum.");
                        return FinalizeParseStatus::Error;
                }

            return FinalizeParseStatus::NotCompleted;
            }

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
            {
            std::vector<ValueExp const*> operands {GetLeftOperand(), GetRightOperand()};
            for (ValueExp const* operand : operands)
                {
                ECSqlTypeInfo const& typeInfo = operand->GetTypeInfo();
                //ignore parameter exp whose type will be resolved later
                if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::Unset)
                    {
                    BeAssert(operand->IsParameterExp());
                    continue;
                    }

                if (!typeInfo.IsPrimitive())
                    {
                    ctx.Issues().Report("Expecting a primitive value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }

                ECSqlTypeInfo const& expectedType = GetTypeInfo();
                if (expectedType.IsExactNumeric() && !typeInfo.IsExactNumeric())
                    {
                    ctx.Issues().Report("Expecting an integral value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }
                else if (expectedType.IsNumeric() && !typeInfo.IsNumeric())
                    {
                    ctx.Issues().Report("Expecting a numeric value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }
                else if (expectedType.GetPrimitiveType() == PRIMITIVETYPE_String && typeInfo.GetPrimitiveType() != PRIMITIVETYPE_String)
                    {
                    ctx.Issues().Report("Expecting value expression of type String as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }
                }

            return FinalizeParseStatus::Completed;
            }


            default:
                BeAssert(false);
                return FinalizeParseStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool BinaryValueExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    parameterExp.SetTargetExpInfo(GetTypeInfo());
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void BinaryValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetLeftOperand()).AppendToECSql(" ").AppendToECSql(ExpHelper::ToSql(m_op)).AppendToECSql(" ").AppendToECSql(*GetRightOperand());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BinaryValueExp::_ToString() const
    {
    Utf8String str("Binary [Operator: ");
    str.append(ExpHelper::ToSql(m_op)).append("]");
    return str;
    }


//*************************** CastExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus CastExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        //if operands are parameters set the target exp in those expressions
        if (GetCastOperand()->IsParameterExp())
            {
            ctx.Issues().Report("Parameters are not supported in a CAST expression ('%s').", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }

        if (m_castTargetSchemaName.empty())
            {
            ECN::PrimitiveType targetType;
            if (ExpHelper::ToPrimitiveType(targetType, GetCastTargetPrimitiveType()) != SUCCESS)
                {
                ctx.Issues().Report("Invalid CAST target type '%s'. Valid target types are the EC primitive types, a fully qualified EC struct type or arrays of those.", GetCastTargetPrimitiveType().c_str());
                return FinalizeParseStatus::Error;
                }

            SetTypeInfo(ECSqlTypeInfo(targetType, m_castTargetIsArray, nullptr));
            }
        else
            {
            ECClassCP targetType = ctx.Schemas().GetClass(m_castTargetSchemaName, GetCastTargetClassName(), SchemaLookupMode::AutoDetect);
            if (targetType == nullptr)
                {
                ctx.Issues().Report("Invalid CAST target type '%s.%s'. The type does not exist.", m_castTargetSchemaName.c_str(), GetCastTargetClassName().c_str());
                return FinalizeParseStatus::Error;
                }

            if (!targetType->IsStructClass())
                {
                ctx.Issues().Report("Invalid CAST target type '%s.%s'. The type is not an EC struct.", m_castTargetSchemaName.c_str(), GetCastTargetClassName().c_str());
                return FinalizeParseStatus::Error;
                }

            SetTypeInfo(ECSqlTypeInfo(*targetType->GetStructClassCP(), m_castTargetIsArray));
            }

        return FinalizeParseStatus::NotCompleted;
        }


    ECSqlTypeInfo const& castOperandTypeInfo = GetCastOperand()->GetTypeInfo();
    if (castOperandTypeInfo.IsNull()) //NULL can always be cast
        return FinalizeParseStatus::Completed;

    if (!castOperandTypeInfo.IsPrimitive())
        {
        ctx.Issues().Report("CAST expects operand to be a primitive value expression or the null constant.");
        return FinalizeParseStatus::Error;
        }

    ECSqlTypeInfo const& expectedTypeInfo = GetTypeInfo();

    if (castOperandTypeInfo.GetPrimitiveType() != expectedTypeInfo.GetPrimitiveType())
        {
        //primitives can be cast except for points because they map to multiple columns
        std::vector<ECSqlTypeInfo const*> typeInfos {&expectedTypeInfo, &castOperandTypeInfo};
        for (ECSqlTypeInfo const* typeInfo : typeInfos)
            {
            if (typeInfo->IsPoint())
                {
                ctx.Issues().Report("Casting from '%s' to '%s' is not supported", ExpHelper::ToString(castOperandTypeInfo.GetPrimitiveType()), ExpHelper::ToString(expectedTypeInfo.GetPrimitiveType()));
                return FinalizeParseStatus::Error;
                }
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool CastExp::NeedsCasting() const
    {
    BeAssert(IsComplete());
    return !GetCastOperand()->GetTypeInfo().Equals(GetTypeInfo());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void CastExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql("CAST (").AppendToECSql(*GetCastOperand()).AppendToECSql(" AS ");

    if (!m_castTargetSchemaName.empty())
        ctx.AppendToECSql(m_castTargetSchemaName).AppendToECSql(".");

    ctx.AppendToECSql(m_castTargetTypeName);

    if (m_castTargetIsArray)
        ctx.AppendToECSql("[]");

    ctx.AppendToECSql(")");

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CastExp::_ToString() const
    {
    Utf8String str("Cast [Target type: ");
    if (!m_castTargetSchemaName.empty())
        str.append(m_castTargetSchemaName).append(".");

    str.append(m_castTargetTypeName);

    if (m_castTargetIsArray)
        str.append("[] ");

    str.append("]");
    return str;
    }

//****************************** FunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        const ECN::PrimitiveType returnType = DetermineReturnType(ctx.GetECDb(), GetFunctionName(), (int) GetChildrenCount());
        SetTypeInfo(ECSqlTypeInfo(returnType));
        return FinalizeParseStatus::NotCompleted;
        }

    //verify that args are all primitive and handle parameter args
    const size_t argCount = GetChildrenCount();
    if (m_setQuantifier != SqlSetQuantifier::NotSpecified && argCount != 1)
        {
        ctx.Issues().Report("Function '%s' can only have one argument if used with the %s operator.",
                                      m_functionName.c_str(), ExpHelper::ToSql(m_setQuantifier));
        return FinalizeParseStatus::Error;
        }

    //no validation needed for count as it accepts everything
    if (m_isStandardSetFunction && m_functionName.EqualsI("count"))
        return FinalizeParseStatus::Completed;

    bool allArgsAreConstant = true;
    for (size_t i = 0; i < argCount; i++)
        {
        ValueExp* argExp = GetChildP<ValueExp>(i);
        if (argExp->IsParameterExp())
            continue;

        if (!argExp->IsConstant())
            allArgsAreConstant = false;

        ECSqlTypeInfo::Kind typeKind = argExp->GetTypeInfo().GetKind();
        if (typeKind != ECSqlTypeInfo::Kind::Primitive && typeKind != ECSqlTypeInfo::Kind::Null)
            {
            ctx.Issues().Report("Function '%s' can only be called with primitive arguments. Argument #%" PRIu64 " is not primitive.",
                                          m_functionName.c_str(), (uint64_t) (i + 1));
            return FinalizeParseStatus::Error;
            }
        }

    //set functions are never constant per row, as they aggregate. For custom set functions we'd have to do this too,
    //but we don't know which custom functions are aggregate and which not
    if (!m_isStandardSetFunction)
        SetIsConstant(allArgsAreConstant);

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool FunctionCallExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //we don't have metadata about function args, so use a default type if the arg is a parameter
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Double));
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionCallExp::AddArgument(std::unique_ptr<ValueExp> argument)
    {
    if (m_setQuantifier != SqlSetQuantifier::NotSpecified && GetChildrenCount() >= 1)
        {
        BeAssert(false && "Only one argument is allowed for aggregate functions that use an aggregate operator.");
        return ERROR;
        }

    AddChild(move(argument));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::PrimitiveType FunctionCallExp::DetermineReturnType(ECDbCR ecdb, Utf8StringCR functionName, int argCount)
    {
    DbFunction* func = nullptr;
    const bool isCustomFunction = ecdb.GetImpl().TryGetSqlFunction(func, functionName.c_str(), argCount);
    if (isCustomFunction)
        {
        switch (func->GetReturnType())
            {
                case DbValueType::BlobVal: return ECN::PRIMITIVETYPE_Binary;
                case DbValueType::IntegerVal: return ECN::PRIMITIVETYPE_Long;
                case DbValueType::TextVal: return ECN::PRIMITIVETYPE_String;

                case DbValueType::FloatVal:
                    //NullVal means that no return type was specified. In that case we use Double to be as generic as possible
                case DbValueType::NullVal:
                default:
                    return ECN::PRIMITIVETYPE_Double;
            }
        }

    //return type for SQLite built-in functions
    //TODO: This is SQLite specific and therefore should be moved out of the parser. Maybe an external file
    //for better maintainability?
    if (functionName.EqualsIAscii("any"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("count"))
        return PRIMITIVETYPE_Long;

    if (functionName.EqualsIAscii("every"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("glob"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("group_concat"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("hex"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("instr"))
        return PRIMITIVETYPE_Integer;

    if (functionName.EqualsIAscii("length"))
        return PRIMITIVETYPE_Long;

    if (functionName.EqualsIAscii("like"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("lower"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("ltrim"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("match"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("quote"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("randomblob"))
        return PRIMITIVETYPE_Binary;

    if (functionName.EqualsIAscii("regexp"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("replace"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("rtrim"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("some"))
        return PRIMITIVETYPE_Boolean;

    if (functionName.EqualsIAscii("soundex"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("substr"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("trim"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("unicode"))
        return PRIMITIVETYPE_Long;

    if (functionName.EqualsIAscii("upper"))
        return PRIMITIVETYPE_String;

    if (functionName.EqualsIAscii("zeroblob"))
        return PRIMITIVETYPE_Binary;

    //Functions built-into BeSQLite
    if (functionName.EqualsIAscii("invirtualset"))
        return PRIMITIVETYPE_Boolean;

    //all other functions get the default return type
    return ECN::PRIMITIVETYPE_Double;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FunctionCallExp::_ToString() const
    {
    Utf8String str("FunctionCall [Function: ");
    str.append(m_functionName);

    if (m_setQuantifier != SqlSetQuantifier::NotSpecified)
        str.append(" Aggregate operator: ").append(ExpHelper::ToSql(m_setQuantifier));

    str.append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionCallExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(m_functionName).AppendToECSql("(");

    if (m_setQuantifier != SqlSetQuantifier::NotSpecified)
        ctx.AppendToECSql(ExpHelper::ToSql(m_setQuantifier)).AppendToECSql(" ");

    bool isFirstItem = true;
    for (Exp const* argExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(",");

        ctx.AppendToECSql(*argExp);
        isFirstItem = false;
        }

    ctx.AppendToECSql(")");

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//****************************** LikeRhsValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus LikeRhsValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;
        
    SetTypeInfo(GetRhsExp()->GetTypeInfo());
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool LikeRhsValueExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_String));
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
ValueExp const* LikeRhsValueExp::GetEscapeExp() const
    {
    if (HasEscapeExp())
        return GetChild<ValueExp>((size_t) m_escapeExpIndex);
    else
        return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
void LikeRhsValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetRhsExp());

    if (HasEscapeExp())
        ctx.AppendToECSql(" ESCAPE ").AppendToECSql(*GetEscapeExp());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//*************************** LiteralValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const  LiteralValueExp::CURRENT_DATE = "CURRENT_DATE";
Utf8CP const  LiteralValueExp::CURRENT_TIMESTAMP = "CURRENT_TIMESTAMP";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus LiteralValueExp::Create(std::unique_ptr<ValueExp>& exp, ECSqlParseContext& ctx, Utf8CP value, ECSqlTypeInfo typeInfo)
    {
    exp = nullptr;

    std::unique_ptr<LiteralValueExp> valueExp(new LiteralValueExp(value, typeInfo));
    if (SUCCESS != valueExp->ResolveDataType(ctx))
        return ERROR;

    exp = std::move(valueExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
LiteralValueExp::LiteralValueExp(Utf8CP value, ECSqlTypeInfo typeInfo) : ValueExp(Type::LiteralValue, true), m_value(value)
    {
    SetTypeInfo(typeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t LiteralValueExp::GetValueAsInt64() const
    {
    int64_t v = 0;
    sscanf(m_value.c_str(), "%" PRId64, &v);
    return v;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool LiteralValueExp::GetValueAsBoolean() const
    {
    if (m_value.EqualsIAscii("TRUE"))
        return true;

    if (m_value.EqualsIAscii("FALSE"))
        return false;

    BeAssert(false && "value doesn't represent a boolean value of 'true' or 'false'");
    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LiteralValueExp::ResolveDataType(ECSqlParseContext& ctx)
    {
    if (GetTypeInfo().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        DateTime::Info dtInfo;
        if (m_value.EqualsI(CURRENT_DATE))
            dtInfo = DateTime::Info::CreateForDate();
        else if (m_value.EqualsI(CURRENT_TIMESTAMP))
            //deviating from SQL-99 ECSQL considers CURRENT_TIMESTAMP to return a UTC time stamp
            dtInfo = DateTime::Info::CreateForDateTime(DateTime::Kind::Utc);
        else
            {
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, m_value.c_str()))
                {
                ctx.Issues().Report("Invalid format for DATE/TIMESTAMP in expression '%s'.", ToECSql().c_str());
                return ERROR;
                }

            dtInfo = dt.GetInfo();
            }

        SetTypeInfo(ECSqlTypeInfo(PRIMITIVETYPE_DateTime, false, &dtInfo));
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void LiteralValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    if (typeInfo.IsNull())
        {
        ctx.AppendToECSql("NULL");
        if (HasParentheses())
            ctx.AppendToECSql(")");

        return;
        }

    if (typeInfo.IsPrimitive())
        {
        const PrimitiveType primType = typeInfo.GetPrimitiveType();
        if (primType == PRIMITIVETYPE_String)
            {
            //escape single quotes again
            Utf8String escapedLiteral(m_value);
            escapedLiteral.ReplaceAll("'", "''");

            ctx.AppendToECSql("'").AppendToECSql(escapedLiteral).AppendToECSql("'");

            if (HasParentheses())
                ctx.AppendToECSql(")");

            return;
            }

        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (BeStringUtilities::Strnicmp(m_value.c_str(), "CURRENT", 7) == 0)
                {
                ctx.AppendToECSql(m_value);
                if (HasParentheses())
                    ctx.AppendToECSql(")");

                return;
                }

            DateTime::Info const& dtInfo = typeInfo.GetDateTimeInfo();
            if (dtInfo.IsValid() && dtInfo.GetComponent() == DateTime::Component::Date)
                ctx.AppendToECSql("DATE '");
            else
                ctx.AppendToECSql("TIMESTAMP '");

            ctx.AppendToECSql(m_value).AppendToECSql("'");
            if (HasParentheses())
                ctx.AppendToECSql(")");

            return;
            }
        }

    ctx.AppendToECSql(m_value);

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String LiteralValueExp::_ToString() const
    {
    Utf8String str("LiteralValue [Value: ");
    str.append(m_value.c_str());

    if (GetTypeInfo().IsPrimitive())
        str.append(", Type: ").append(ExpHelper::ToString(GetTypeInfo().GetPrimitiveType()));

    str.append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String LiteralValueExp::EscapeStringLiteral(Utf8StringCR constantStringLiteral)
    {
    Utf8String tmp = constantStringLiteral;
    tmp.ReplaceAll("'", "''");
    return tmp;
    }


//****************************** ParameterExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ParameterExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        m_parameterIndex = ctx.TrackECSqlParameter(*this);

    return FinalizeParseStatus::NotCompleted;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::SetDefaultTargetExpInfo()
    {
    SetTargetExpInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Double));

#ifndef NDEBUG
    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        Exp const* parentExp = GetParent();
        BeAssert(parentExp != nullptr && "ParameterExp is expected to always have a parent exp");
        if (parentExp != nullptr)
            LOG.debugv("Using default parameter data type for parameter in exp %s", parentExp->ToECSql().c_str());
        }
#endif
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::SetTargetExpInfo(ComputedExp const& targetExp)
    {
    if (targetExp.IsParameterExp())
        {
        SetDefaultTargetExpInfo();
        return;
        }

    m_targetExp = &targetExp;
    SetTypeInfo(targetExp.GetTypeInfo());
    SetIsComplete();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::SetTargetExpInfo(ECSqlTypeInfo const& targetTypeInfo)
    {
    SetTypeInfo(targetTypeInfo);
    m_targetExp = nullptr;
    SetIsComplete();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    if (IsNamedParameter())
        {
        ctx.AppendToECSql(":[").AppendToECSql(m_parameterName).AppendToECSql("]");
        if (ctx.GetMode() == Exp::ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter)
            ctx.AddParameterIndexNameMapping(m_parameterIndex, m_parameterName, false);
        }
    else
        {
        switch (ctx.GetMode())
            {
                case ECSqlRenderContext::Mode::Default:
                    ctx.AppendToECSql("?");
                    break;

                case ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter:
                {
                Utf8String systemNamedParameter;
                systemNamedParameter.Sprintf(ECSQLSYS_PARAM_FORMAT, m_parameterIndex);
                ctx.AppendToECSql(":").AppendToECSql(systemNamedParameter);

                ctx.AddParameterIndexNameMapping(m_parameterIndex, systemNamedParameter, true);
                break;
                }
                default:
                    BeAssert(false && "ECSqlRenderContext::Mode enum has changed. This code needs to be adjusted");
                    ctx.AppendToECSql("?");
                    break;
            }
        }

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ParameterExp::_ToString() const
    {
    Utf8String str;
    str.Sprintf("Parameter [Index: %d, Name: %s]", m_parameterIndex, IsNamedParameter() ? m_parameterName.c_str() : "-");
    return str;
    }


//****************************** UnaryExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus UnaryValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        switch (m_op)
            {
                case Operator::Minus:
                case Operator::Plus:
                    SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Double));
                    break;
                case Operator::BitwiseNot:
                    SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Long));
                    break;

                default:
                    ctx.Issues().Report("Invalid unary operator in expression %s.", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
            }

        return FinalizeParseStatus::NotCompleted;
        }

    ValueExp const* operand = GetOperand();
    //parameter exp get their types later, so ignore them in this check
    if (operand->IsParameterExp())
        return FinalizeParseStatus::Completed;

    auto const& operandTypeInfo = operand->GetTypeInfo();
    switch (m_op)
        {
            case Operator::Minus:
            case Operator::Plus:
            {
            if (!operandTypeInfo.IsNumeric())
                {
                ctx.Issues().Report("Invalid type in expression %s: Unary operator expects a numeric type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }
            case Operator::BitwiseNot:
            {
            if (!operandTypeInfo.IsExactNumeric())
                {
                ctx.Issues().Report("Invalid type in expression %s: Unary operator expects an integral type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }

            default:
            {
            ctx.Issues().Report("Invalid unary operator in expression %s.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool UnaryValueExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    parameterExp.SetTargetExpInfo(GetTypeInfo());
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void UnaryValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(ExpHelper::ToSql(m_op)).AppendToECSql(*GetOperand());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnaryValueExp::_ToString() const
    {
    Utf8String str("UnaryValue [Operator: ");
    str.append(ExpHelper::ToSql(m_op)).append("]");
    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
