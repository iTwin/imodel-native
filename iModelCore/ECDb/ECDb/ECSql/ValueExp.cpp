/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ValueExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ValueExp.h"
#include "ExpHelper.h"

using namespace std;
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
        vector<ValueExp const*> operands {GetLowerBoundOperand(), GetUpperBoundOperand()};
        for (ValueExp const* operand : operands)
            {
            //parameter exp type is determined later, so do not check type for it here
            if (operand->IsParameterExp())
                continue;

            ECSqlTypeInfo const& typeInfo = operand->GetTypeInfo();
            if (!typeInfo.IsPrimitive() || typeInfo.IsGeometry() || typeInfo.IsPoint())
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid BETWEEN expression '%s'. Operands must be of numeric, date/timestamp, or string type.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }
            }

        return FinalizeParseStatus::Completed;
        }

    BeAssert(false);
    return FinalizeParseStatus::Error;
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
                            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Adjust BinaryValueExp::_FinalizeParsing for new value of BinarySqlOperator enum.");
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
                        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Expecting a primitive value expression as operand. '%s'", ToECSql().c_str());
                        return FinalizeParseStatus::Error;
                        }

                    ECSqlTypeInfo const& expectedType = GetTypeInfo();
                    if (expectedType.IsExactNumeric() && !typeInfo.IsExactNumeric())
                        {
                        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Expecting an integral value expression as operand. '%s'", ToECSql().c_str());
                        return FinalizeParseStatus::Error;
                        }
                    else if (expectedType.IsNumeric() && !typeInfo.IsNumeric())
                        {
                        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Expecting a numeric value expression as operand. '%s'", ToECSql().c_str());
                        return FinalizeParseStatus::Error;
                        }
                    else if (expectedType.GetPrimitiveType() == PRIMITIVETYPE_String && typeInfo.GetPrimitiveType() != PRIMITIVETYPE_String)
                        {
                        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Expecting value expression of type String as operand. '%s'", ToECSql().c_str());
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
void BinaryValueExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(GetLeftOperand()->ToECSql()).append(" ").append(ExpHelper::ToString(m_op)).append(" ").append(GetRightOperand()->ToECSql());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BinaryValueExp::_ToString() const 
    {
    Utf8String str ("Binary [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }


//*************************** CastExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus CastExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    auto castOperand = GetCastOperand();
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        //if operands are parameters set the target exp in those expressions
        if (castOperand->IsParameterExp())
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Parameters are not supported in a CAST expression ('%s').", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }

        ECN::PrimitiveType targetType;
        if (ExpHelper::ToPrimitiveType(targetType, m_castTargetType) != SUCCESS)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid CAST target type '%s'.", m_castTargetType.c_str());
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo(ECSqlTypeInfo(targetType));
        return FinalizeParseStatus::NotCompleted;
        }


    ECSqlTypeInfo const& castOperandTypeInfo = GetCastOperand()->GetTypeInfo();
    if (castOperandTypeInfo.GetKind() == ECSqlTypeInfo::Kind::Null) //NULL can always be cast
        return FinalizeParseStatus::Completed;

    if (!castOperandTypeInfo.IsPrimitive())
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "CAST expects operand to be a primitive value expression or the null constant.");
        return FinalizeParseStatus::Error;
        }

    ECSqlTypeInfo const& expectedTypeInfo = GetTypeInfo();

    if (castOperandTypeInfo.GetPrimitiveType() != expectedTypeInfo.GetPrimitiveType())
        {
        vector<ECSqlTypeInfo const*> typeInfos {&expectedTypeInfo, &castOperandTypeInfo};
        for (ECSqlTypeInfo const* typeInfo : typeInfos)
            {
            if (!typeInfo->IsNumeric() && !typeInfo->IsBoolean() && !typeInfo->IsBinary() && !typeInfo->IsString())
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Casting from '%s' to '%s' is not supported", ExpHelper::ToString(castOperandTypeInfo.GetPrimitiveType()), ExpHelper::ToString(expectedTypeInfo.GetPrimitiveType()));
                return FinalizeParseStatus::Error;
                }
            }
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool CastExp::NeedsCasting () const
    {
    BeAssert (IsComplete ());
    //DateTimeInfo is ignored as any date time value can be cast to another date time type
    return !GetCastOperand ()->GetTypeInfo ().Equals (GetTypeInfo (), true);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void CastExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append("CAST (").append(GetCastOperand ()->ToECSql()).append(" AS ").append(m_castTargetType).append (")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CastExp::_ToString() const 
    {
    Utf8String str ("Cast [Target type: ");
    str.append (m_castTargetType.c_str ()).append ("]");
    return str;
    }


//****************************** ECClassIdFunctionExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const ECClassIdFunctionExp::NAME = "GetECClassId";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ECClassIdFunctionExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto finalizeParseArgs = ctx.GetFinalizeParseArg ();
        BeAssert (finalizeParseArgs != nullptr && "ECClassIdFunctionExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
        RangeClassRefList const& rangeClassRefList = *static_cast<RangeClassRefList const*> (finalizeParseArgs);

        if (HasClassAlias ())
            {
            bvector<RangeClassRefExp const*> matchingClassRefExpList;
            //1. Assume that the first entry of the prop path is a property name and not a class alias and directly search it in classrefs
            for(auto rangeClassRef : rangeClassRefList)
                {
                if (rangeClassRef->GetId().Equals(m_classAlias))
                    matchingClassRefExpList.push_back(rangeClassRef);
                }

            const auto matchCount = matchingClassRefExpList.size ();
            if (matchCount == 0)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Undefined Class alias '%s' in expression '%s'.", GetClassAlias (), ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }
            else if (matchCount > 1)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Class alias '%s' used in expression '%s' is ambiguous.", GetClassAlias (), ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }

            m_classRefExp = matchingClassRefExpList[0];
            }
        else // no class alias
            {
            const auto classRefCount = rangeClassRefList.size ();
            if (classRefCount == 0)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "No class reference found for '%s' in ECSQL statement.", ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }
            else if (classRefCount > 1)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Ambiguous call to '%s'. Class alias is missing.", ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }

            m_classRefExp = rangeClassRefList[0];
            }

        if (m_classRefExp == nullptr)
            {
            BeAssert (m_classRefExp != nullptr && "Resolved class reference expression is nullptr");
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Resolved class reference expression is nullptr");
            return FinalizeParseStatus::Error;
            }

        if (m_classRefExp->GetType () != Exp::Type::ClassName)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "%s only supported for simple class references. Subqueries are not yet supported.", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo (ECSqlTypeInfo (FunctionCallExp::DetermineReturnType (ctx.GetECDb(), NAME, 0)));
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassIdFunctionExp::_ToString () const 
    {
    Utf8String str ("ECClassIdFunction [Alias: ");
    str.append (m_classAlias).append ("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECClassIdFunctionExp::_DoToECSql (Utf8StringR ecsql) const 
    {
    if (HasClassAlias ())
        ecsql.append (m_classAlias).append (".");

    ecsql.append (NAME).append ("()");
    }

//****************************** GetPointCoordinateFunctionExp *****************************************

#define GETPOINTCOORDINATEFUNCTION_NAMEROOT "Get"

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
const size_t GetPointCoordinateFunctionExp::s_functionNameRootLength = strlen(GETPOINTCOORDINATEFUNCTION_NAMEROOT);

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
GetPointCoordinateFunctionExp::GetPointCoordinateFunctionExp(Utf8StringCR functionName, std::unique_ptr<ValueExp> pointArgumentExp) 
    : ValueExp(), m_coordinate(CoordinateFromFunctionName(functionName))
    {
    m_argIndex = AddChild(std::move(pointArgumentExp));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus GetPointCoordinateFunctionExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t argCount = GetChildrenCount();
    if (argCount != 1)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function '%s' can only be called with a single arguments.",
                     ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    //now verify that args are all primitive and handle parameter args
    ValueExp* argExp = GetChildP<ValueExp>(0);
    ECSqlTypeInfo const& argTypeInfo = argExp->GetTypeInfo();
    if (!argTypeInfo.IsPoint() || argExp->GetType() == Exp::Type::Parameter)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function '%s' can only be called with Point2D or Point3D arguments.",
                     ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    if (m_coordinate == Coordinate::Z && argTypeInfo.GetPrimitiveType() != PRIMITIVETYPE_Point3D)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function '%s' can only be called with Point3D arguments.",
                     ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Double));
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
void GetPointCoordinateFunctionExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(GETPOINTCOORDINATEFUNCTION_NAMEROOT).append(CoordinateToString(m_coordinate)).append("(").append(GetArgument().ToECSql()).append(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool GetPointCoordinateFunctionExp::IsPointCoordinateFunction(Utf8StringCR functionName)
    {
    if (BeStringUtilities::Strnicmp(GETPOINTCOORDINATEFUNCTION_NAMEROOT, functionName.c_str(), s_functionNameRootLength) != 0 ||
        (s_functionNameRootLength + 1) != functionName.size())
        return false;

    Utf8Char const& lastChar = functionName[s_functionNameRootLength];
    return lastChar == 'X' || lastChar == 'Y' || lastChar == 'Z' || lastChar == 'x' || lastChar == 'y' || lastChar == 'z';
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GetPointCoordinateFunctionExp::_ToString() const
    {
    Utf8String str;
    str.Sprintf("GetPointCoordinateFunction [Coordinate: %s]", CoordinateToString(m_coordinate));
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
GetPointCoordinateFunctionExp::Coordinate GetPointCoordinateFunctionExp::CoordinateFromFunctionName(Utf8StringCR functionName)
    {
    BeAssert(IsPointCoordinateFunction(functionName));
    Utf8Char const& lastChar = functionName[functionName.size() - 1];
    switch (lastChar)
        {
            case 'X':
            case 'x':
                return Coordinate::X;

            case 'Y':
            case 'y':
                return Coordinate::Y;

            case 'Z':
            case 'z':
                return Coordinate::Z;

            default:
                BeAssert(false);
                return Coordinate::X;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP GetPointCoordinateFunctionExp::CoordinateToString(Coordinate coordinate)
    {
    switch (coordinate)
        {
            case Coordinate::X:
                return "X";
            case Coordinate::Y:
                return "Y";
            case Coordinate::Z:
                return "Z";

            default:
                BeAssert(false);
                return nullptr;
        }
    }

//****************************** FunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bmap<Utf8CP, ECN::PrimitiveType, CompareIUtf8> FunctionCallExp::s_builtinFunctionNonDefaultReturnTypes;

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    switch (mode)
        {
            case Exp::FinalizeParseMode::BeforeFinalizingChildren:
                {
                const ECN::PrimitiveType returnType = DetermineReturnType(ctx.GetECDb(), GetFunctionName(), (int) GetChildrenCount());
                SetTypeInfo(ECSqlTypeInfo(returnType));
                return FinalizeParseStatus::NotCompleted;
                }

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
                {
                //verify that args are all primitive and handle parameter args
                size_t argCount = GetChildrenCount();
                for (size_t i = 0; i < argCount; i++)
                    {
                    ValueExp* argExp = GetChildP<ValueExp>(i);
                    if (argExp->IsParameterExp())
                        continue;

                    ECSqlTypeInfo::Kind typeKind = argExp->GetTypeInfo().GetKind();
                    if (typeKind != ECSqlTypeInfo::Kind::Primitive && typeKind != ECSqlTypeInfo::Kind::Null)
                        {
                        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function '%s' can only be called with primitive arguments. Argument #%d is not primitive.",
                                     m_functionName.c_str(), i + 1);
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
void FunctionCallExp::AddArgument (std::unique_ptr<ValueExp> argument)
    {
    AddChild (move (argument));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::PrimitiveType FunctionCallExp::DetermineReturnType(ECDbCR ecdb, Utf8CP functionName, int argCount)
    {
    DbFunction* func = nullptr;
    const bool isCustomFunction = ecdb.GetECDbImplR().TryGetSqlFunction(func, functionName, argCount);
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
    if (s_builtinFunctionNonDefaultReturnTypes.empty())
        {
        s_builtinFunctionNonDefaultReturnTypes["ANY"] = ECN::PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["COUNT"] = ECN::PRIMITIVETYPE_Long;
        s_builtinFunctionNonDefaultReturnTypes["EVERY"] = ECN::PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["GETECCLASSID"] = ECN::PRIMITIVETYPE_Long;
        s_builtinFunctionNonDefaultReturnTypes["GLOB"] = ECN::PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["GROUP_CONCAT"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["HEX"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["INSTR"] = ECN::PRIMITIVETYPE_Integer;
        s_builtinFunctionNonDefaultReturnTypes["LENGTH"] = ECN::PRIMITIVETYPE_Long;
        s_builtinFunctionNonDefaultReturnTypes["LIKE"] = ECN::PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["LOWER"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["LTRIM"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["MATCH"] = PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["QUOTE"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["RANDOMBLOB"] = ECN::PRIMITIVETYPE_Binary;
        s_builtinFunctionNonDefaultReturnTypes["REGEXP"] = PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["REPLACE"] = PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["RTRIM"] = PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["SOME"] = ECN::PRIMITIVETYPE_Boolean;
        s_builtinFunctionNonDefaultReturnTypes["SOUNDEX"] = PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["SUBSTR"] = PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["TRIM"] = PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["UNICODE"] = ECN::PRIMITIVETYPE_Long;
        s_builtinFunctionNonDefaultReturnTypes["UPPER"] = ECN::PRIMITIVETYPE_String;
        s_builtinFunctionNonDefaultReturnTypes["ZEROBLOB"] = ECN::PRIMITIVETYPE_Binary;

        //Functions built-into BeSQLite
        s_builtinFunctionNonDefaultReturnTypes["INVIRTUALSET"] = ECN::PRIMITIVETYPE_Boolean;
        }

    auto it = s_builtinFunctionNonDefaultReturnTypes.find(functionName);
    if (it != s_builtinFunctionNonDefaultReturnTypes.end())
        return it->second;

    //all other functions get the default return type
    return ECN::PRIMITIVETYPE_Double;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FunctionCallExp::_ToString() const
    {
    Utf8String str("FunctionCall [Function: ");
    str.append(m_functionName).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionCallExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(m_functionName).append("(");
    bool isFirstItem = true;
    for (auto argExp : GetChildren())
        {
        if (!isFirstItem)
            ecsql.append(",");

        ecsql.append(argExp->ToECSql());
        isFirstItem = false;
        }
    ecsql.append(")");
    }


//****************************** LikeRhsValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus LikeRhsValueExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_String));

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
ValueExp const* LikeRhsValueExp::GetEscapeExp () const
    {
    if (HasEscapeExp ())
        return GetChild<ValueExp> (static_cast<size_t> (m_escapeExpIndex));
    else
        return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
void LikeRhsValueExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append (GetRhsExp ()->ToECSql());
    if (HasEscapeExp ())
        ecsql.append (" ESCAPE ").append (GetEscapeExp ()->ToECSql ()); 
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
BentleyStatus LiteralValueExp::Create(unique_ptr<ValueExp>& exp, ECSqlParseContext& ctx, Utf8CP value, ECSqlTypeInfo typeInfo)
    {
    exp = nullptr;

    unique_ptr<LiteralValueExp> valueExp = unique_ptr<LiteralValueExp>(new LiteralValueExp(value, typeInfo));
    BentleyStatus stat = valueExp->ResolveDataType(ctx);
    if (stat == SUCCESS)
        exp = move(valueExp);

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
LiteralValueExp::LiteralValueExp(Utf8CP value, ECSqlTypeInfo typeInfo)
    : ValueExp(true), m_value(value)
    {
    SetTypeInfo(typeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR LiteralValueExp::GetValue() const
    {
    return m_value;
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
    if (m_value.EqualsI("TRUE"))
        return true;
    if (m_value.EqualsI("FALSE"))
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
            dtInfo = DateTime::Info(DateTime::Kind::Unspecified, DateTime::Component::Date);
        else if (m_value.EqualsI(CURRENT_TIMESTAMP))
            //deviating from SQL-99 ECSQL considers CURRENT_TIMESTAMP to return a UTC time stamp
            dtInfo = DateTime::Info(DateTime::Kind::Utc, DateTime::Component::DateAndTime);
        else
            {
            DateTime dt;
            if (SUCCESS != DateTime::FromString(dt, m_value.c_str()))
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid format for DATE/TIMESTAMP in expression '%s'.", ToECSql().c_str());
                return ERROR;
                }

            dtInfo = dt.GetInfo();
            }

        DateTimeInfo dtMetadata(dtInfo);
        SetTypeInfo(ECSqlTypeInfo(PRIMITIVETYPE_DateTime, &dtMetadata));
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void LiteralValueExp::_DoToECSql(Utf8StringR ecsql) const
    {
    auto const& typeInfo = GetTypeInfo();
    if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::Null)
        {
        ecsql.append("NULL");
        return;
        }

    if (typeInfo.IsPrimitive())
        {
        auto primType = typeInfo.GetPrimitiveType();
        if (primType == PRIMITIVETYPE_String)
            {
            ecsql.append("'").append(m_value).append("'");
            return;
            }

        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (BeStringUtilities::Strnicmp(m_value.c_str(), "CURRENT", 7) == 0)
                {
                ecsql.append(m_value);
                return;
                }

            auto const& dtInfo = typeInfo.GetDateTimeInfo();
            if (!dtInfo.IsComponentNull() && dtInfo.GetInfo(false).GetComponent() == DateTime::Component::Date)
                ecsql.append("DATE '").append(m_value).append("'");
            else
                ecsql.append("TIMESTAMP '").append(m_value).append("'");

            return;
            }
        }

    ecsql.append(m_value);
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
void ParameterExp::_DoToECSql(Utf8StringR ecsql) const
    {
    if (!IsNamedParameter())
        {
        ecsql.append("?");
        return;
        }

    ecsql.append(":").append(m_parameterName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ParameterExp::_ToString() const
    {
    Utf8String str("Parameter [Name: ");
    str.append(m_parameterName).append("]");
    return str;
    }


//****************************** SetFunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SetFunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    Utf8CP functionName = GetFunctionName();

    const int childCount = (int) GetChildren().size();
    //all standard set functions require 1 arg
    if (childCount != 1)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid function call '%s'. Function %s expects 1 argument.", ToECSql().c_str(), functionName);
        return FinalizeParseStatus::Error;
        }

    ECN::PrimitiveType returnType = DetermineReturnType(ctx.GetECDb(), functionName, childCount);
    SetTypeInfo(ECSqlTypeInfo(returnType));

    //check arg type for all functions except COUNT (which can take any arg)
    Function function = GetFunction();
    if (function != Function::Count)
        {
        ECSqlTypeInfo const& argTypeInfo = GetChild<ValueExp>(0)->GetTypeInfo();

        if (Function::Any == function ||
            Function::Every == function ||
            Function::Some == function)
            {
            if (!argTypeInfo.IsPrimitive() || argTypeInfo.GetPrimitiveType() != ECN::PRIMITIVETYPE_Boolean)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid function call '%s'. Function %s expects boolean argument", ToECSql().c_str(), functionName);
                return FinalizeParseStatus::Error;
                }
            }
        else
            {
            if (!argTypeInfo.IsNumeric())
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid function call '%s'. Function %s expects numeric argument", ToECSql().c_str(), functionName);
                return FinalizeParseStatus::Error;
                }
            }
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void SetFunctionCallExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(GetFunctionName()).append ("(");
    Utf8String selectionType = ExpHelper::ToString(GetSetQuantifier());
    if (!selectionType.empty())
        ecsql.append (selectionType).append (" ");

    bool isFirstItem = true;
    for(auto argExp : GetChildren ())
        {
        if (!isFirstItem)
            ecsql.append(", ");

        ecsql.append(argExp->ToECSql());
        isFirstItem = false;
        }

    ecsql.append(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SetFunctionCallExp::_ToString() const 
    {
    Utf8String str ("SetFunctionCall [Function: ");
    str.append (GetFunctionName ()).append (", Type: ").append (ExpHelper::ToString (m_setQuantifier)).append ("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP SetFunctionCallExp::ToString(Function function)
    {
    switch (function)
        {
            case Function::Any: return "ANY";
            case Function::Avg: return "AVG";
            case Function::Count: return "COUNT";
            case Function::Every: return "EVERY";
            case Function::Max: return "MAX";
            case Function::Min: return "MIN";
            case Function::Some: return "SOME";
            case Function::Sum: return "SUM";

            default:
                BeAssert(false && "Enum Function has changed. Please update ToString");
                return nullptr;
        }
    }

//****************************** FoldFunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FoldFunctionCallExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_String));
        return FinalizeParseStatus::NotCompleted;
        }

    Utf8CP functionName = GetFunctionName();
    const int childCount = (int) GetChildren().size();
    
    ECN::PrimitiveType returnType = DetermineReturnType(ctx.GetECDb(), functionName, childCount);
    SetTypeInfo(ECSqlTypeInfo(returnType));

    ValueExp const* argExp = GetChild<ValueExp>(0);
    SetIsConstant(argExp->IsConstant());

    ECSqlTypeInfo const& typeInfo = argExp->GetTypeInfo ();
    if (!typeInfo.IsPrimitive () || typeInfo.GetPrimitiveType () != PRIMITIVETYPE_String)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid function call '%s'. Function %s expects string argument", ToECSql ().c_str (), functionName);
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FoldFunctionCallExp::_ToString () const
    {
    Utf8String str ("FoldFunctionCall [Function: ");
    str.append (GetFunctionName ()).append ("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP FoldFunctionCallExp::ToString(FoldFunction foldFunction)
    {
    switch (foldFunction)
        {
            case FoldFunction::Lower: return "LOWER";
            case FoldFunction::Upper: return "UPPER";
            default:
                BeAssert(false && "Programmer Error: new value added to FoldFunction enum. Please update ToString method.");
                return nullptr;
        }
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
                case UnarySqlOperator::Minus:
                case UnarySqlOperator::Plus:
                    SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Double));
                    break;
                case UnarySqlOperator::BitwiseNot:
                    SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Long));
                    break;

                default:
                    ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid unary operator in expression %s.", ToECSql().c_str());
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
        case UnarySqlOperator::Minus:
        case UnarySqlOperator::Plus:
            {
            if (!operandTypeInfo.IsNumeric())
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid type in expression %s: Unary operator expects a numeric type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }
        case UnarySqlOperator::BitwiseNot:
            {
            if (!operandTypeInfo.IsExactNumeric ())
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid type in expression %s: Unary operator expects an integral type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            break;
            }

        default:
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid unary operator in expression %s.", ToECSql().c_str());
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
void UnaryValueExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(ExpHelper::ToString(m_op)).append(GetOperand()->ToECSql());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnaryValueExp::_ToString() const 
    {
    Utf8String str ("UnaryValue [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
