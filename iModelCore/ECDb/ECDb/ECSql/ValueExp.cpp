/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
                ctx.Issues().ReportV("Invalid BETWEEN expression '%s'. Operands must be of numeric, date/timestamp, or string type.", ToECSql().c_str());
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
                        SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double));
                        break;
                    case BinarySqlOperator::ShiftLeft:
                    case BinarySqlOperator::ShiftRight:
                    case BinarySqlOperator::BitwiseOr:
                    case BinarySqlOperator::BitwiseAnd:
                    case BinarySqlOperator::BitwiseXOr:
                        SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long));
                        break;

                    case BinarySqlOperator::Concat:
                        SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String));
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
                    ctx.Issues().ReportV("Expecting a primitive value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }

                ECSqlTypeInfo const& expectedType = GetTypeInfo();
                /*
                if (expectedType.IsExactNumeric() && !typeInfo.IsExactNumeric())
                    {
                    ctx.Issues().ReportV("Expecting an integral value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }
                else */
                if (expectedType.IsNumeric() && !typeInfo.IsNumeric())
                    {
                    ctx.Issues().ReportV("Expecting a numeric value expression as operand. '%s'", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
                    }
                else if (expectedType.GetPrimitiveType() == PRIMITIVETYPE_String && typeInfo.GetPrimitiveType() != PRIMITIVETYPE_String)
                    {
                    ctx.Issues().ReportV("Expecting value expression of type String as operand. '%s'", ToECSql().c_str());
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
            ctx.Issues().ReportV("Parameters are not supported in a CAST expression ('%s').", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }

        if (m_castTargetSchemaName.empty())
            {
            ECN::PrimitiveType targetType;
            if (ExpHelper::ToPrimitiveType(targetType, GetCastTargetPrimitiveType()) != SUCCESS)
                {
                ctx.Issues().ReportV("Invalid CAST target type '%s'. Valid target types are the EC primitive types, a fully qualified EC struct type or arrays of those.", GetCastTargetPrimitiveType().c_str());
                return FinalizeParseStatus::Error;
                }

            SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(targetType, m_castTargetIsArray));
            return FinalizeParseStatus::NotCompleted;
            }

        //WIP: Need to change the grammar to allow specifying cast targets from other table spaces
        ECEnumerationCP targetEnumType = ctx.Schemas().GetEnumeration(m_castTargetSchemaName, GetCastTargetTypeName(), SchemaLookupMode::AutoDetect);
        if (targetEnumType != nullptr)
            {
            SetTypeInfo(ECSqlTypeInfo::CreateEnum(*targetEnumType, m_castTargetIsArray));
            return FinalizeParseStatus::NotCompleted;
            }

        ECClassCP targetClassType = ctx.Schemas().GetClass(m_castTargetSchemaName, GetCastTargetTypeName(), SchemaLookupMode::AutoDetect);
        if (targetClassType == nullptr)
            {
            ctx.Issues().ReportV("Invalid CAST target type '%s.%s'. The type does not exist.", m_castTargetSchemaName.c_str(), GetCastTargetTypeName().c_str());
            return FinalizeParseStatus::Error;
            }

        if (!targetClassType->IsStructClass())
            {
            ctx.Issues().ReportV("Invalid CAST target type '%s.%s'. Only enumeration and struct types are supported.", m_castTargetSchemaName.c_str(), GetCastTargetTypeName().c_str());
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo(ECSqlTypeInfo::CreateStruct(*targetClassType->GetStructClassCP(), m_castTargetIsArray));
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

    ECSqlTypeInfo const& targetTypeInfo = GetTypeInfo();
    if (!targetTypeInfo.IsPrimitive())
        {
        ctx.Issues().ReportV("Invalid target type in CAST expression '%s'. Non-primitive target type can only be used when casting NULL.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    if (castOperandTypeInfo.GetPrimitiveType() != targetTypeInfo.GetPrimitiveType())
        {
        //primitives can be cast except for points because they map to multiple columns
        std::vector<ECSqlTypeInfo const*> typeInfos {&targetTypeInfo, &castOperandTypeInfo};
        for (ECSqlTypeInfo const* typeInfo : typeInfos)
            {
            if (typeInfo->IsPoint())
                {
                ctx.Issues().ReportV("Casting from '%s' to '%s' is not supported", ExpHelper::ToString(castOperandTypeInfo.GetPrimitiveType()), ExpHelper::ToString(targetTypeInfo.GetPrimitiveType()));
                return FinalizeParseStatus::Error;
                }
            }
        }

    return FinalizeParseStatus::Completed;
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

//****************************** MemberFunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MemberFunctionCallExp::ValidateArgument(ValueExp const& arg, Utf8StringR msg)
    {
    std::vector<Exp const*> expList;
    expList = arg.Find (Exp::Type::PropertyName, true);
    if (!expList.empty())
        {
        msg.Sprintf("Invalid MemberFunctionCall expression '%s'. Argument %s is invalid. A MemberFunctionCall expression cannot not contain a PropertyName expression. ", 
                    m_functionName.c_str(), expList.front()->ToString().c_str());
        
        return ERROR;
        }

    expList = arg.Find(Exp::Type::Select, true);
    if (!expList.empty())
        {
        msg.Sprintf("Invalid MemberFunctionCall expression '%s'. Argument %s is invalid. MemberFunctionCall expression cannot not contain SubQueries.",
                    m_functionName.c_str(), expList.front()->ToString().c_str());

        return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus MemberFunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren)
        {
        FunctionSignature const* funcSig = FunctionSignatureSet::GetInstance().Find(m_functionName.c_str());
        if (funcSig == nullptr)
            {
            ctx.Issues().ReportV("Unknown member function '%s'", m_functionName.c_str());
            return Exp::FinalizeParseStatus::Error;
            }
        
        if (funcSig->SetParameterType(GetChildrenR()) != SUCCESS)
            {
            ctx.Issues().ReportV("Error in function call '%s' - Varying argument list cannot be parameterized.", m_functionName.c_str());
            return Exp::FinalizeParseStatus::Error;
            }

        Utf8String err;
        if (funcSig->Verify(err, GetChildren()) != SUCCESS)
            {
            ctx.Issues().ReportV("Error in function call '%s' - %s", m_functionName.c_str(), err.c_str());
            return Exp::FinalizeParseStatus::Error;
            }

        SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
        return FinalizeParseStatus::Completed;
        }

    return FinalizeParseStatus::NotCompleted;
    }
    
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
void MemberFunctionCallExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(".");
    ctx.AppendToECSql(m_functionName).AppendToECSql("(");
    bool isFirstItem = true;
    for (Exp const* argExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(",");

        ctx.AppendToECSql(*argExp);
        isFirstItem = false;
        }

    ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String MemberFunctionCallExp::_ToString() const
    {
    return Utf8String("MemberFunctionCall [Function: ").append(m_functionName).append("]");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool MemberFunctionCallExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //we don't have metadata about function args, so use a default type if the arg is a parameter
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MemberFunctionCallExp::AddArgument(std::unique_ptr<ValueExp> argument, Utf8StringR error)
    {
    if (ValidateArgument(*argument, error) != SUCCESS)
        return ERROR;

    AddChild(move(argument));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
ValueExp const* MemberFunctionCallExp::GetArgument(size_t index) const
    {
    if (index >= GetChildren().size())
        {
        BeAssert(false);
        return nullptr;
        }

    Exp const* childExp = GetChildren()[index];
    if (childExp == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }
    BeAssert(dynamic_cast<ValueExp const*> (childExp) != nullptr);
    return childExp->GetAsCP<ValueExp>();
    }

//****************************** FunctionCallExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bmap<Utf8CP, ECSqlTypeInfo, CompareIUtf8Ascii>* FunctionCallExp::s_builtInFunctionReturnTypes = nullptr;

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        DetermineReturnType(ctx.GetECDb());
        return FinalizeParseStatus::NotCompleted;
        }

    //verify that args are all primitive and handle parameter args
    const size_t argCount = GetChildrenCount();
    if (m_setQuantifier != SqlSetQuantifier::NotSpecified && argCount != 1)
        {
        ctx.Issues().ReportV("Function '%s' can only have one argument if used with the %s operator.",
                                      m_functionName.c_str(), ExpHelper::ToSql(m_setQuantifier));
        return FinalizeParseStatus::Error;
        }

    if (m_isGetter && argCount != 0)
        {
        ctx.Issues().ReportV("Function '%s' is a getter function any may not have any arguments.",
                             m_functionName.c_str());
        return FinalizeParseStatus::Error;
        }

    //no validation needed for count as it accepts everything
    if (m_isStandardSetFunction && m_functionName.EqualsI("count"))
        return FinalizeParseStatus::Completed;

    for (size_t i = 0; i < argCount; i++)
        {
        ValueExp* argExp = GetChildP<ValueExp>(i);
        if (argExp->IsParameterExp())
            continue;

        ECSqlTypeInfo::Kind typeKind = argExp->GetTypeInfo().GetKind();
        if (typeKind != ECSqlTypeInfo::Kind::Primitive && typeKind != ECSqlTypeInfo::Kind::Null)
            {
            ctx.Issues().ReportV("Function '%s' can only be called with primitive arguments. Argument #%" PRIu64 " is not primitive.",
                                          m_functionName.c_str(), (uint64_t) (i + 1));
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool FunctionCallExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    //we don't have metadata about function args, so use a default type if the arg is a parameter
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
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
void FunctionCallExp::DetermineReturnType(ECDbCR ecdb)
    {
    DbFunction* func = nullptr;
    const bool isCustomFunction = ecdb.GetImpl().TryGetSqlFunction(func, m_functionName.c_str(), (int) GetChildrenCount());
    if (isCustomFunction)
        {
        switch (func->GetReturnType())
            {
                case DbValueType::BlobVal: SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Binary)); return;
                case DbValueType::IntegerVal: SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Long)); return;
                case DbValueType::TextVal: SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String)); return;

                case DbValueType::FloatVal:
                //NullVal means that no return type was specified. In that case we use Double to be as generic as possible
                case DbValueType::NullVal:
                default:
                    SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
                    return;
            }
        }

    //return type for standard SQL and SQLite built-in functions
    auto it = GetBuiltInFunctionReturnTypes().find(m_functionName.c_str());
    if (it == GetBuiltInFunctionReturnTypes().end())
        {
        //all other functions get the default return type 
        SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
        return;
        }

    SetTypeInfo(it->second);
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

    ctx.AppendToECSql(m_functionName);
    
    if (!m_isGetter)
        {
        ctx.AppendToECSql("(");

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
        }

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bmap<Utf8CP, ECSqlTypeInfo, CompareIUtf8Ascii> const& FunctionCallExp::GetBuiltInFunctionReturnTypes()
    {
    //return type for standard SQL and SQLite built-in functions
    //TODO: This is partially SQLite specific and therefore should be moved out of the parser. Maybe an external file
    //for better maintainability?

    if (s_builtInFunctionReturnTypes == nullptr)
        {
        s_builtInFunctionReturnTypes = new bmap<Utf8CP, ECSqlTypeInfo, CompareIUtf8Ascii>();
        bmap<Utf8CP, ECSqlTypeInfo, CompareIUtf8Ascii>& map = *s_builtInFunctionReturnTypes;
        map["any"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["changes"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["char"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["count"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map[CURRENT_DATE()] = ECSqlTypeInfo::CreateDateTime(DateTime::Info::CreateForDate());
        map[CURRENT_TIMESTAMP()] = ECSqlTypeInfo::CreateDateTime(DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
        map[CURRENT_TIME()] = ECSqlTypeInfo::CreateDateTime(DateTime::Info::CreateForTimeOfDay());
        map["date"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["datetime"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["every"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["glob"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["group_concat"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["hex"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["instr"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Integer);
        map["invirtualset"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["json"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_array"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_array_length"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["json_extract"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String); // must be type 'Any'
        map["json_group_array"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_group_object"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_insert"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_object"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_quote"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_remove"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_replace"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_set"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_type"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["json_valid"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["julianday"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double);
        map["last_insert_rowid"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long, false, EXTENDEDTYPENAME_Id);
        map["length"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["like"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["lower"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["ltrim"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["match"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["quote"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["printf"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["random"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["randomblob"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Binary);
        map["regexp"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["replace"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["rtrim"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["round"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double);
        map["some"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["soundex"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["sqlite_compileoption_get"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["sqlite_compileoption_used"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        map["sqlite_source_id"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["sqlite_version"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["strftime"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["substr"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["time"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["total_changes"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["trim"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["typeof"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["unicode"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        map["upper"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        map["zeroblob"] = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Binary);
        }

    return *s_builtInFunctionReturnTypes;
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
    parameterExp.SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String));
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

//*************************** EnumValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
EnumValueExp::EnumValueExp(ECEnumeratorCR value, PropertyPath const& expPath) : ValueExp(Type::EnumValue, true), m_enumerator(value), m_expPath(expPath)
    {    
    SetTypeInfo(ECSqlTypeInfo::CreateEnum(value.GetEnumeration()));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
void EnumValueExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(m_expPath.ToString(true, false));

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String EnumValueExp::_ToString() const
    {
    Utf8String str("EnumValue [");
    str.append(m_expPath.ToString()).append("]");
    return str;
    }

//*************************** LiteralValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus LiteralValueExp::Create(std::unique_ptr<ValueExp>& exp, ECSqlParseContext& ctx, Utf8CP value, ECSqlTypeInfo const& typeInfo)
    {
    exp = nullptr;

    std::unique_ptr<LiteralValueExp> valueExp(new LiteralValueExp(value, typeInfo));

    if (typeInfo.IsDateTime())
        {
        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, value))
            {
            ctx.Issues().ReportV("Invalid format for DATE/TIMESTAMP/TIME in expression '%s'.", valueExp->ToECSql().c_str());
            return ERROR;
            }

        valueExp->SetTypeInfo(ECSqlTypeInfo::CreateDateTime(dt.GetInfo()));
        }

    exp = std::move(valueExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
LiteralValueExp::LiteralValueExp(Utf8CP value, ECSqlTypeInfo const& typeInfo) : ValueExp(Type::LiteralValue, true), m_rawValue(value) { SetTypeInfo(typeInfo); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LiteralValueExp::ResolveDataType(ECSqlParseContext& ctx)
    {
    if (!GetTypeInfo().IsNull() && GetTypeInfo().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, m_rawValue.c_str()))
            {
            ctx.Issues().ReportV("Invalid format for DATE/TIMESTAMP/TIME in expression '%s'.", ToECSql().c_str());
            return ERROR;
            }

        SetTypeInfo(ECSqlTypeInfo::CreateDateTime(dt.GetInfo()));
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LiteralValueExp::TryParse(ECN::ECValue& val) const
    {
    if (GetTypeInfo().IsNull())
        {
        val.SetToNull();
        return SUCCESS;
        }

    switch (GetTypeInfo().GetPrimitiveType())
        {
            case PRIMITIVETYPE_Boolean:
            {
            if (m_rawValue.EqualsIAscii("TRUE"))
                val.SetBoolean(true);
            else if (m_rawValue.EqualsIAscii("FALSE"))
                val.SetBoolean(false);
            else
                return ERROR;

            return SUCCESS;
            }

            case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, m_rawValue.c_str()))
                return ERROR;

            return val.SetDateTime(dt);
            }

            case PRIMITIVETYPE_Integer:
            {
            int32_t v = 0;
            sscanf(m_rawValue.c_str(), "%" SCNd32, &v);
            return val.SetInteger(v);
            }

            case PRIMITIVETYPE_Long:
            {
            int64_t v = 0;
            sscanf(m_rawValue.c_str(), "%" SCNd64, &v);
            return val.SetLong(v);
            }

            case PRIMITIVETYPE_String:
            return val.SetUtf8CP(m_rawValue.c_str(), false);

            default:
                // other types are not supported
                return ERROR;
        }
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
            Utf8String escapedLiteral(m_rawValue);
            escapedLiteral.ReplaceAll("'", "''");

            ctx.AppendToECSql("'").AppendToECSql(escapedLiteral).AppendToECSql("'");

            if (HasParentheses())
                ctx.AppendToECSql(")");

            return;
            }

        if (primType == PRIMITIVETYPE_DateTime)
            {
            DateTime::Info const& dtInfo = typeInfo.GetDateTimeInfo();
            DateTime::Component comp = dtInfo.IsValid() ? dtInfo.GetComponent() : DateTime::Component::DateAndTime;
            switch (comp)
                {
                    case DateTime::Component::Date:
                        ctx.AppendToECSql("DATE '");
                        break;
                    case DateTime::Component::TimeOfDay:
                        ctx.AppendToECSql("TIME '");
                        break;
                    default:
                    case DateTime::Component::DateAndTime:
                        ctx.AppendToECSql("TIMESTAMP '");
                        break;
                }

            ctx.AppendToECSql(m_rawValue).AppendToECSql("'");
            if (HasParentheses())
                ctx.AppendToECSql(")");

            return;
            }
        }

    ctx.AppendToECSql(m_rawValue);

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String LiteralValueExp::_ToString() const
    {
    Utf8String str("LiteralValue [Value: ");
    str.append(m_rawValue.c_str());

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
    SetTargetExpInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));

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
                    SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Double));
                    break;
                case Operator::BitwiseNot:
                    SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_Long));
                    break;

                default:
                    ctx.Issues().ReportV("Invalid unary operator in expression %s.", ToECSql().c_str());
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
                ctx.Issues().ReportV("Invalid type in expression %s: Unary operator expects a numeric type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }
            case Operator::BitwiseNot:
            {
            if (!operandTypeInfo.IsExactNumeric())
                {
                ctx.Issues().ReportV("Invalid type in expression %s: Unary operator expects an integral type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }

            default:
            {
            ctx.Issues().ReportV("Invalid unary operator in expression %s.", ToECSql().c_str());
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

//****************************** FunctionSignatureSet *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionSignatureSet::Declare(Utf8CP signature, Utf8CP description)
    {
    std::unique_ptr<FunctionSignature> sig = FunctionSignature::Parse(signature, description);
    if (sig == nullptr)
        {
        BeAssert(false && "Fail to parse FunctionSignature");
        return;
        }

    if (Find(sig->Name().c_str()) != nullptr)
        {
        BeAssert(false && "Function with same name already exist");
        return;
        }

    m_funtionSigs.insert(std::make_pair(sig->Name().c_str(), std::move(sig)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
FunctionSignature const* FunctionSignatureSet::Find(Utf8CP name) const
    {
    auto itor = m_funtionSigs.find(name);
    if (itor != m_funtionSigs.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
FunctionSignatureSet& FunctionSignatureSet::GetInstance()
    {

    static FunctionSignatureSet funtionSet;
    if (funtionSet.m_funtionSigs.empty())
        {
        funtionSet.LoadDefinitions();
        }

    return funtionSet;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionSignatureSet::LoadDefinitions()
    {
    //scoped funtion
    Declare("::" ECSQLFUNC_Changes "(changesetId:integer, operation:any):resultset");
    ////global funtion
    Declare("guidtostr(value:blob):string");
    Declare("strtoguid(value:string):string");
    Declare("hextoid(value:string):integer");
    Declare("idtohex(value:integer):string");
    Declare("abs(value:numeric):numeric");
    Declare("hex(value:blob):string");
    Declare("ifnull(x:any,y:any):any");
    Declare("length(v:any):any");
    Declare("max(x:any,y:any,...):any");
    Declare("random():integer");
    Declare("randomblob(n:integer):blob");
    Declare("soundex(x:string):string");
    Declare("coalesce(x:any,y:any,...):any");
    Declare("instr(x:string,y:string):string");
    Declare("like(x:string,y:string, optional z:string):integer");
    Declare("lower(x:string):string");
    Declare("nullif(x:any,y:any):any");
    Declare("quote(x:string):string");
    Declare("round(x:double,optional y:double):double");
    }


//****************************** FunctionSignature *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
FunctionSignature::Arg const* FunctionSignature::FindArg(Utf8CP name) const
    {
    for (std::unique_ptr<Arg>const & existingArg : m_args)
        if (existingArg->Name().EqualsI(name))
            return existingArg.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionSignature::SetName(Utf8CP name)
    {
    if (Utf8String::IsNullOrEmpty(name))
        return ERROR;

    m_name = name;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionSignature::SetDescription(Utf8CP name)
    {
    m_description = name;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<FunctionSignature::Arg const*> FunctionSignature::Args() const
    {
    std::vector<Arg const*> args;
    for (std::unique_ptr<Arg> const& arg : m_args)
        args.push_back(arg.get());

    return args;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionSignature::SetReturnType(ValueType type, bool member)
    {
    if (type == ValueType::Resultset && !member)
        return ERROR;

    if(member && type == ValueType::Resultset)
       m_scope = FunctionScope::Class;
    else if (member && type != ValueType::Resultset)
        m_scope = FunctionScope::Property;
    else 
        m_scope = FunctionScope::Global;

    m_returnType = type;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionSignature::Append(Utf8CP name, ValueType type, bool optional)
    {
    if (Utf8String::IsNullOrEmpty(name))
        return ERROR;

    if (Enum::Contains(type, ValueType::Resultset))
        return ERROR;

    std::unique_ptr<Arg> arg(new Arg(name, type, optional));
    Arg* last = !m_args.empty() ? m_args.back().get() : nullptr;
    if (last != nullptr && last->IsOptional() && !optional)
        return ERROR;

    if (last != nullptr && last->IsVariadic())
        return ERROR;
    
    if (arg->IsVariadic() && optional)
        return ERROR;

    //arg with same name should not exist
    if (FindArg(name) != nullptr)
        return ERROR;

    //optional and varying cannot be together
    if (OptionalArgCount() > 0 && arg->IsVariadic())
        return ERROR;

    m_args.push_back(std::move(arg));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8CP FunctionSignature::ValueTypeToString(ValueType type)
    {
    if (type == ValueType::String) return "string";
    if (type == ValueType::Integer) return "integer";
    if (type == ValueType::Float) return "float";
    if (type == ValueType::Blob) return "blob";
    if (type == ValueType::Resultset) return "resultset";
    if (type == ValueType::Numeric) return "numeric";
    if (type == ValueType::Any) return "any";
    BeAssert(false);
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static  
BentleyStatus FunctionSignature::Parse(std::unique_ptr<FunctionSignature>& funcSig, Utf8CP signature, Utf8CP description)
    {
    Utf8String temp = Utf8String(signature).Trim();   
    bool member = temp.StartsWith("::");
    if (member)
        temp = temp.substr(2);

    if (temp.StartsWith(":"))
        return ERROR;

    const size_t posOpenParenthesis = temp.find("(");
    const size_t posCloseParenthesis = temp.find(")");
    if (posOpenParenthesis == Utf8String::npos || posCloseParenthesis == Utf8String::npos)
        return ERROR;

    if (posCloseParenthesis < posOpenParenthesis)
        return ERROR;

    Utf8String functionName = temp.substr(0, posOpenParenthesis);
    funcSig = std::unique_ptr<FunctionSignature>(new FunctionSignature());
    if (funcSig->SetName(functionName.c_str()) != SUCCESS)
        return ERROR;

    funcSig->SetDescription(description);

    //check for return type
    if (temp.size() > posCloseParenthesis)
        {
        Utf8String funTypeStr = temp.substr(posCloseParenthesis + 1);
        funTypeStr.Trim();
        if (funTypeStr.size() > 0)
            {
            if (funTypeStr[0] != ':')
                return ERROR;

            Utf8String funcType = funTypeStr.substr(1);
            funcType.Trim();

            ValueType vtype;
            if (!ParseValueType(vtype, funcType.c_str()))
                return ERROR;

            if (funcSig->SetReturnType(vtype, member) != SUCCESS)
                return ERROR;
            }
        }

    const Utf8String argListStr = temp.substr(posOpenParenthesis + 1, posCloseParenthesis - posOpenParenthesis - 1);
    bvector<Utf8String> argList;
    BeStringUtilities::Split(argListStr.c_str(), ",", argList);
    for (Utf8StringR argStr : argList)
        {
        argStr.Trim();
        if (argStr.empty())
            return ERROR;

        bvector<Utf8String> argFmt;
        BeStringUtilities::Split(argStr.c_str(), " ", argFmt);
        bool optional = false;
        if (argFmt.empty())
            return ERROR;

        int paramIdx = 0;
        if (argFmt.size() == 2)
            {
            Utf8StringR optionalStr = argFmt.at(0).Trim();
            if (!optionalStr.EqualsIAscii("optional"))
                return ERROR;

            paramIdx = 1;
            optional = true;
            }

        Utf8StringR nameAndTypeStr = argFmt.at(paramIdx).Trim();
        if (nameAndTypeStr == "...")
            {
            if (funcSig->Append(nameAndTypeStr.c_str(), ValueType::Any, optional) != SUCCESS)
                return ERROR;
            }
        else
            {
            bvector<Utf8String> argType;
            BeStringUtilities::Split(nameAndTypeStr.c_str(), ":", argType);
            if (argType.size() != 2)
                return ERROR;

            Utf8StringR argNameStr = argType.at(0).Trim();
            Utf8StringR argTypeStr = argType.at(1).Trim();
            ValueType vtype;
            if (!ParseValueType(vtype, argTypeStr.c_str()))
                return ERROR;

            if (funcSig->Append(argNameStr.c_str(), vtype, optional) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FunctionSignature::ToString() const
    {
    Utf8String str;
    if (m_scope != FunctionScope::Global)
        str.append("::");

    str.append(m_name);
    str.append(" (");
    bool first = true;
    for (Arg const* arg : Args())
        {
        if (!first)
            str.append(", ");
        else
            first = false;

        if (arg->IsOptional())
            str.append("optional ");

        str.append(arg->Name());
        if (!arg->IsVariadic())
            {
            str.append(":");
            str.append(ValueTypeToString(arg->Type()));
            }
        }

    str.append("):");
    str.append(ValueTypeToString(m_returnType));
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bool FunctionSignature::ParseValueType(ValueType& type, Utf8CP str)
    {
    if (BeStringUtilities::StricmpAscii("string", str) == 0)
        {
        type = ValueType::String;
        return true;
        }

    else if (BeStringUtilities::StricmpAscii("integer", str) == 0 ||
        BeStringUtilities::StricmpAscii("int", str) == 0)
        {
        type = ValueType::Integer;
        return true;
        }
    else if (BeStringUtilities::StricmpAscii("float", str) == 0 ||
        BeStringUtilities::StricmpAscii("double", str) == 0 ||
        BeStringUtilities::StricmpAscii("real", str) == 0)
        {
        type = ValueType::Float;
        return true;
        }
    else if (BeStringUtilities::StricmpAscii("blob", str) == 0)
        {
        type = ValueType::Blob;
        return true;
        }
    else if (BeStringUtilities::StricmpAscii("resultset", str) == 0)
        {
        type = ValueType::Resultset;
        return true;
        }
    else if (BeStringUtilities::StricmpAscii("numeric", str) == 0)
        {
        type = ValueType::Numeric;
        return true;
        }
    else if (BeStringUtilities::StricmpAscii("any", str) == 0)
        {
        type = ValueType::Any;
        return true;
        }

    return false;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionSignature::SetParameterType(Exp::Collection& argExps) const
    {
    const std::vector<Arg const*> args = Args();
    int i = 0;
    int j = 0;
    do
        {
        Arg const* arg = args[i];
        ValueExp& test = const_cast<ValueExp&>(argExps[j]->GetAs<ValueExp>());          
        if (test.GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Unset && test.IsParameterExp())
            {
            if (arg->IsVariadic())
                {
                BeAssert(false);
                return ERROR;
                }

            if (arg->Type() == FunctionSignature::ValueType::String)
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String));
            else if (arg->Type() == FunctionSignature::ValueType::Blob)
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Binary));
            else if (arg->Type() == FunctionSignature::ValueType::Float ||
                     arg->Type() == FunctionSignature::ValueType::Numeric)
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double));
            else if (arg->Type() == FunctionSignature::ValueType::Integer)
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long));
            else if (arg->Type() == FunctionSignature::ValueType::Any)
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long));
            else
                test.SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Binary));
            }

        j++;
        i++;
        } while (j < argExps.size() && i < args.size());

        return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FunctionSignature::Verify(Utf8StringR err, Exp::Collection const& argExps) const
    {
    const std::vector<Arg const*> args = Args();
    const int requiredArgCount = RequiredArgCount();
    const int optionalArgCount = OptionalArgCount();
    const bool varying = HasVariadicArg();

    if (args.empty() && !argExps.empty())
        {
        err = "Function take no argument.";
        return ERROR;
        }

    if (requiredArgCount > argExps.size())
        {
        err = "Not enough arguments supplied.";
        return ERROR;
        }

    if (!varying)
        {
        if (argExps.size() > optionalArgCount + requiredArgCount)
            {
            err = "Too many arguments.";
            return ERROR;
            }

        }

    if (argExps.empty())
        return SUCCESS;

    int i = 0;
    int j = 0;
    do
        {
        Arg const* arg = args[i];
        ValueExp const* test = argExps[j]->GetAsCP<ValueExp>();
        ECSqlTypeInfo const& testType = test->GetTypeInfo();
        if (!testType.IsNull() && !testType.IsPrimitive())
            {
            err = "Expecting primitive argument to function.";
            return ERROR;
            }

        if (arg->IsVariadic())
            {
            j++;
            continue;
            }

        if (arg->Type() == ValueType::String)
            if (!testType.IsString())
                {
                err.Sprintf("Argument '%s' Expecting primitive argument to function.", arg->Name().c_str());
                return ERROR;
                }

        if (arg->Type() == ValueType::Blob)
            if (!testType.IsBinary() || !testType.IsGeometry())
                {
                err.Sprintf("Argument '%s' Expecting binary argument to function.", arg->Name().c_str());
                return ERROR;
                }

        if (arg->Type() == ValueType::Integer)
            if (!testType.IsExactNumeric())
                {
                err.Sprintf("Argument '%s' Expecting integer argument to function.", arg->Name().c_str());
                return ERROR;
                }

        if (arg->Type() == ValueType::Float)
            if (!testType.IsApproximateNumeric())
                {
                err.Sprintf("Argument '%s' Expecting float argument to function.", arg->Name().c_str());
                return ERROR;
                }

        if (arg->Type() == ValueType::Numeric)
            if (!testType.IsNumeric())
                {
                err.Sprintf("Argument '%s' Expecting numeric argument to function.", arg->Name().c_str());
                return ERROR;
                }

        j++;
        i++;
        } while (j < argExps.size() && i < args.size());

        return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
int FunctionSignature::RequiredArgCount() const
    {
    int i = 0;
    for (std::unique_ptr<Arg> const& arg : m_args)
        if (arg->IsOptional() || arg->IsVariadic())
            return i;
        else
            i++;

    return i;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
int FunctionSignature::OptionalArgCount() const
    {
    int i = 0;
    for (std::unique_ptr<Arg> const& arg : m_args)
        if (arg->IsOptional())
            i++;

    return i;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool FunctionSignature::HasVariadicArg() const
    {
    for (std::unique_ptr<Arg> const& arg : m_args)
        if (arg->IsVariadic())
            return true;

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::unique_ptr<FunctionSignature> FunctionSignature::Parse(Utf8CP signature, Utf8CP description)
    {
    std::unique_ptr<FunctionSignature> sig;
    BentleyStatus bs = Parse(sig, signature, description);
    BeAssert(bs == SUCCESS);
    if (bs != SUCCESS)
        return nullptr;

    return sig;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
