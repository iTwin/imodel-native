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
Exp::FinalizeParseStatus BetweenRangeValueExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto const& lowerBoundTypeInfo = GetLowerBoundOperand ()->GetTypeInfo ();

    if (!lowerBoundTypeInfo.IsPrimitive () || !GetUpperBoundOperand ()->GetTypeInfo ().IsPrimitive ())
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Invalid BETWEEN expression '%s'. Operands cannot be structs or arrays.", ToECSql().c_str()); 
        return FinalizeParseStatus::Error;
        }

    //indicate that the exp per se doesn't have a single type info as it can vary across its children
    SetTypeInfo (ECSqlTypeInfo (ECSqlTypeInfo::Kind::Varies));
    return FinalizeParseStatus::Completed;
    }

//*************************** BinaryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus BinaryExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    auto lhs = GetLeftOperand ();
    auto rhs = GetRightOperand ();

    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        //if operands are parameters set the target exp in those expressions
        if (DetermineOperandsTargetTypes (ctx, lhs, rhs) != ECSqlStatus::Success)
            return FinalizeParseStatus::Error;

        return FinalizeParseStatus::NotCompleted;
        }

    auto const& lhsTypeInfo = lhs->GetTypeInfo ();
    auto const& rhsTypeInfo = rhs->GetTypeInfo ();

    if (!lhsTypeInfo.IsPrimitive () || !rhsTypeInfo.IsPrimitive ())
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Expecting a primitive value expression as operand. '%s'", ToECSql().c_str()); 
        return FinalizeParseStatus::Error;
        }

    auto lhsType = lhsTypeInfo.GetPrimitiveType ();
    auto rhsType = rhsTypeInfo.GetPrimitiveType ();

    auto resolvedType = ECN::PRIMITIVETYPE_Integer;
    switch (m_op)
        {
        case SqlBinaryOperator::PLUS:
        case SqlBinaryOperator::MINUS:
        case SqlBinaryOperator::MODULUS:
        case SqlBinaryOperator::DIVIDE:
        case SqlBinaryOperator::MULTIPLY:
            {
            if (!lhsTypeInfo.IsNumeric () || !rhsTypeInfo.IsNumeric ())
                {
                ctx.SetError(ECSqlStatus::InvalidECSql, "Expecting a numeric value expression as operand. '%s'", ToECSql().c_str()); 
                return FinalizeParseStatus::Error;
                }

            resolvedType = lhsType;
            if (lhsType == ECN::PRIMITIVETYPE_Double || rhsType == ECN::PRIMITIVETYPE_Double )
                resolvedType = ECN::PRIMITIVETYPE_Double;
            if (lhsType == ECN::PRIMITIVETYPE_Long || rhsType == ECN::PRIMITIVETYPE_Long )
                resolvedType = ECN::PRIMITIVETYPE_Long;

            break;
            }
        case SqlBinaryOperator::SHIFT_LEFT:
        case SqlBinaryOperator::SHIFT_RIGHT:
        case SqlBinaryOperator::BITWISE_OR:
        case SqlBinaryOperator::BITWISE_AND:
        case SqlBinaryOperator::BITWISE_XOR:
            {
            if (!lhsTypeInfo.IsExactNumeric () || !rhsTypeInfo.IsExactNumeric ())
                {
                ctx.SetError(ECSqlStatus::InvalidECSql, "Expecting value expression of type Int or int64_t as operand to bitwise operation. '%s'", ToECSql().c_str()); 
                return FinalizeParseStatus::Error;
                }

            resolvedType = lhsType;
            if (lhsType == ECN::PRIMITIVETYPE_Long || rhsType == ECN::PRIMITIVETYPE_Long )
                resolvedType = ECN::PRIMITIVETYPE_Long;
            break;
            }

        case SqlBinaryOperator::CONCAT:
            {
            if (lhsType == ECN::PRIMITIVETYPE_String  && lhsType == ECN::PRIMITIVETYPE_String)
                resolvedType = ECN::PRIMITIVETYPE_String;
            else
                {
                ctx.SetError(ECSqlStatus::InvalidECSql, "Expecting value expression of type String as operand. '%s'", ToECSql().c_str());                 
                return FinalizeParseStatus::Error;
                }

            break;
            }

        default:
            BeAssert (false && "Unhandled Binary operator when parsing BinaryExp.");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Unhandled Binary operator when parsing BinaryExp.");
            return FinalizeParseStatus::Error;
        }

    SetTypeInfo (ECSqlTypeInfo (resolvedType));
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BinaryExp::ToECSql() const 
    {
    return "(" + GetLeftOperand ()->ToECSql() + " " + ExpHelper::ToString(m_op) + " " + GetRightOperand ()->ToECSql() +")";
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BinaryExp::_ToString() const 
    {
    Utf8String str ("Binary [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }


//*************************** CastExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus CastExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    auto castOperand = GetCastOperand ();
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        //if operands are parameters set the target exp in those expressions
        if (castOperand->GetType () == Exp::Type::Parameter)
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "Parameters are not supported in a CAST expression ('%s').", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }

        return FinalizeParseStatus::NotCompleted;
        }

    ECN::PrimitiveType targetType;
    if (ExpHelper::ToPrimitiveType (targetType, m_castTargetType) != ECSqlStatus::Success)
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid CAST target type '%s'.", m_castTargetType.c_str());
        return FinalizeParseStatus::Error;
        }         

    auto const& castOperandTypeInfo = GetCastOperand ()->GetTypeInfo ();
    const auto castOperandTypeKind = castOperandTypeInfo.GetKind ();
    if (!castOperandTypeInfo.IsPrimitive () && castOperandTypeKind != ECSqlTypeInfo::Kind::Null)
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "CAST expects operand to be a primitive value expression or the null constant.");
        return FinalizeParseStatus::Error;
        }

    //if operand is not null, now check that operand type and target type are compatible.
    if (castOperandTypeKind != ECSqlTypeInfo::Kind::Null)
        {
        bool canCast = true;
        const auto operandType = castOperandTypeInfo.GetPrimitiveType ();
        if (operandType != targetType)
            {
            switch (targetType)
                {
                case ECN::PRIMITIVETYPE_Double:
                case ECN::PRIMITIVETYPE_Long:
                case ECN::PRIMITIVETYPE_Integer:
                    switch (operandType)
                        {
                        case ECN::PRIMITIVETYPE_Integer:
                        case ECN::PRIMITIVETYPE_Double:
                        case ECN::PRIMITIVETYPE_String:
                        case ECN::PRIMITIVETYPE_Long:
                        case ECN::PRIMITIVETYPE_Boolean:
                            canCast = true;
                            break;
                        default:
                            canCast = false;
                            break;
                        }
                    break;

                case ECN::PRIMITIVETYPE_String:
                    switch (operandType)
                        {
                        case ECN::PRIMITIVETYPE_Integer:
                        case ECN::PRIMITIVETYPE_Double:
                        case ECN::PRIMITIVETYPE_String:
                        case ECN::PRIMITIVETYPE_Long:
                        case ECN::PRIMITIVETYPE_Boolean:
                        case ECN::PRIMITIVETYPE_Binary:
                            canCast = true;
                            break;
                        default:
                            canCast = false;
                            break;
                        }
                    break;

                case ECN::PRIMITIVETYPE_Boolean:
                    switch (operandType)
                        {
                        case ECN::PRIMITIVETYPE_Integer:
                        case ECN::PRIMITIVETYPE_Double:
                        case ECN::PRIMITIVETYPE_Long:
                        case ECN::PRIMITIVETYPE_Boolean:
                            canCast = true;
                            break;
                        default:
                            canCast = false;
                            break;
                        }
                    break;

                default:
                    canCast = false;
                    break;
                }      
            }

        if (!canCast)
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Casting from '%s' to '%s' is not supported", ExpHelper::ToString(operandType), ExpHelper::ToString(targetType));
            return FinalizeParseStatus::Error;
            }
        }

    SetTypeInfo (ECSqlTypeInfo (targetType));
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
Utf8String CastExp::ToECSql () const 
    {
    return "CAST (" + GetCastOperand ()->ToECSql() +" AS " + m_castTargetType + ")" ;
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

//*************************** ConstantValueExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const  ConstantValueExp::CURRENT_DATE = "CURRENT_DATE";
Utf8CP const  ConstantValueExp::CURRENT_TIMESTAMP = "CURRENT_TIMESTAMP";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ConstantValueExp> ConstantValueExp::Create (ECSqlParseContext& ctx, Utf8CP value, ECSqlTypeInfo typeInfo)
    {
    auto exp = unique_ptr<ConstantValueExp> (new ConstantValueExp (value, typeInfo));
    if (ECSqlStatus::Success != exp->ResolveDataType (ctx))
        return nullptr;

    return exp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   09/2013
//+---------------+---------------+---------------+---------------+---------------+------
ConstantValueExp::ConstantValueExp (Utf8CP value, ECSqlTypeInfo typeInfo)
   : ValueExp (), m_value (value)
    {
    SetTypeInfo (typeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ConstantValueExp::GetValue () const
    {
    return m_value;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ConstantValueExp::GetValueAsInt64 () const
    {
    int64_t v = 0;
    sscanf(m_value.c_str(), "%" PRId64, &v);
    return v;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ConstantValueExp::GetValueAsBoolean () const
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
ECSqlStatus ConstantValueExp::ResolveDataType (ECSqlParseContext& ctx)
    {
    if (GetTypeInfo ().GetPrimitiveType () == PRIMITIVETYPE_DateTime)
        {
        DateTime::Info dtInfo;
        if (m_value.EqualsI (CURRENT_DATE))
            dtInfo = DateTime::Info (DateTime::Kind::Unspecified, DateTime::Component::Date);
        else if (m_value.EqualsI (CURRENT_TIMESTAMP))
            //deviating from SQL-99 ECSQL considers CURRENT_TIMESTAMP to return a UTC time stamp
            dtInfo = DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime);
        else
            {
            DateTime dt;
            if (SUCCESS != DateTime::FromString (dt, m_value.c_str ()))
                return ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid format for DATE/TIMESTAMP in expression '%s'.", ToECSql ().c_str ());

            dtInfo = dt.GetInfo ();
            }

        DateTimeInfo dtMetadata (dtInfo);
        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_DateTime, &dtMetadata));
        }

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
Utf8String ConstantValueExp::ToECSql () const
    {
    auto const& typeInfo = GetTypeInfo ();

    if (typeInfo.GetKind () == ECSqlTypeInfo::Kind::Null)
        return "NULL";

    if (typeInfo.IsPrimitive ())
        {
        auto primType = typeInfo.GetPrimitiveType ();
        if (primType == PRIMITIVETYPE_String)
            return "'" + m_value + "'";

        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (BeStringUtilities::Strnicmp (m_value.c_str (), "CURRENT", 7) == 0)
                return m_value;

            auto const& dtInfo = typeInfo.GetDateTimeInfo ();
            if (!dtInfo.IsComponentNull () && dtInfo.GetInfo (false).GetComponent () == DateTime::Component::Date)
                return "DATE '" + m_value + "'";
            else
                return "TIMESTAMP '" + m_value + "'";
            }
        }

    return m_value;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ConstantValueExp::_ToString() const 
    {
    Utf8String str ("ConstantValue [Value: ");
    str.append (m_value.c_str ());
    
    if (GetTypeInfo ().IsPrimitive ())
        str.append (", Type: ").append (ExpHelper::ToString (GetTypeInfo ().GetPrimitiveType ()));

    str.append ("]");
    return str;
    }


//****************************** ECClassIdFunctionExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ECClassIdFunctionExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
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
                ctx.SetError (ECSqlStatus::InvalidECSql, "Undefined Class alias '%s' in expression '%s'.", GetClassAlias (), ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }
            else if (matchCount > 1)
                {
                ctx.SetError (ECSqlStatus::InvalidECSql, "Class alias '%s' used in expression '%s' is ambiguous.", GetClassAlias (), ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }

            m_classRefExp = matchingClassRefExpList[0];
            }
        else // no class alias
            {
            const auto classRefCount = rangeClassRefList.size ();
            if (classRefCount == 0)
                {
                ctx.SetError (ECSqlStatus::InvalidECSql, "No class reference found for '%s' in ECSQL statement.", ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }
            else if (classRefCount > 1)
                {
                ctx.SetError (ECSqlStatus::InvalidECSql, "Ambiguous call to '%s'. Class alias is missing.", ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
                }

            m_classRefExp = rangeClassRefList[0];
            }

        if (m_classRefExp == nullptr)
            {
            BeAssert (m_classRefExp != nullptr && "Resolved class reference expression is nullptr");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Resolved class reference expression is nullptr");
            return FinalizeParseStatus::Error;
            }

        if (m_classRefExp->GetType () != Exp::Type::ClassName)
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "%s only supported for simple class references. Subqueries are not yet supported.", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_Long));
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
Utf8String ECClassIdFunctionExp::ToECSql () const 
    {
    Utf8String ecsql;
    if (HasClassAlias ())
        ecsql.append (m_classAlias).append (".");

    ecsql.append ("GetECClassId()");
    return ecsql;
    }

//****************************** FunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus FunctionCallExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t argCount = GetChildrenCount();
    ECSqlFunction* func = nullptr;
    if (!ctx.GetECDb().GetECDbImplR().TryGetECSqlFunction(func, GetFunctionName(), (int) argCount))
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Unknown function '%s' with %d args. If this is a custom function, make sure to have it registered.", m_functionName.c_str(), argCount);
        return FinalizeParseStatus::Error;
        }

    //now verify that args are all primitive
    for (size_t i = 0; i < argCount; i++)
        {
        ValueExp const* argExp = GetChild<ValueExp>(i);
        ECSqlTypeInfo::Kind typeKind = argExp->GetTypeInfo().GetKind();
        if (typeKind != ECSqlTypeInfo::Kind::Primitive)
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Function '%s' can only be called with primitive arguments. Argument #%d is NULL or not primitive.",
                         m_functionName.c_str(), i + 1);
            return FinalizeParseStatus::Error;
            }
        }

    SetTypeInfo(ECSqlTypeInfo(func->GetReturnType ()));
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void FunctionCallExp::AddArgument (std::unique_ptr<ValueExp> argument)
    {
    AddChild (move (argument));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool FunctionCallExp::IsValidArgCount (ECSqlParseContext& ctx, size_t expectedArgCount, size_t actualArgCount) const
    {
    if (expectedArgCount != actualArgCount)
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid function call '%s'. Function %s expects %d argument(s)", ToECSql ().c_str (), GetFunctionName (), expectedArgCount);
        return false;
        }

    return true;
    }

//****************************** LikeRhsValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    09/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus LikeRhsValueExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    SetTypeInfo (GetRhsExp ()->GetTypeInfo ());
    return FinalizeParseStatus::Completed;
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
//virtual
Utf8String LikeRhsValueExp::ToECSql () const 
    {
    Utf8String ecsql (GetRhsExp ()->ToECSql());
    if (HasEscapeExp ())
        ecsql.append (" ESCAPE ").append (GetEscapeExp ()->ToECSql ()); 

    return ecsql;
    }



//****************************** ParameterExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ParameterExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto targetExp = GetTargetExp ();
    if (targetExp != nullptr)
        {
        //if target exp is another parameter, use a default type
        if (targetExp->GetType () == Type::Parameter)
            {
            auto const& targetTypeInfo = targetExp->GetTypeInfo ();
            if (targetTypeInfo.GetKind () == ECSqlTypeInfo::Kind::Unset)
                SetTypeInfoFromTarget (ECSqlTypeInfo (PRIMITIVETYPE_Long));
            else
                SetTypeInfoFromTarget (targetTypeInfo);
            }
        else
            {
            //Target expr is not a child of parameter expr, but a cross reference. If it hasn't been finalized yet, we do so now.
            auto stat = const_cast<ComputedExp*> (targetExp)->FinalizeParsing (ctx);
            if (stat != ECSqlStatus::Success)
                return FinalizeParseStatus::Error;

            SetTypeInfoFromTarget (targetExp->GetTypeInfo ());
            }
        }
    else
        //In some cases the parameter exp has no target type (e.g. in limit/offset clauses. In
        //that case just assume a numeric type.
        //TODO: assumption to choose numeric type in that case is error-prone. 
        SetTypeInfoFromTarget (ECSqlTypeInfo (PRIMITIVETYPE_Long));

    m_parameterIndex = ctx.TrackECSqlParameter (*this);
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::SetTargetExp (ComputedExp const& exp)
    {
    m_targetExp = &exp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ParameterExp::SetTypeInfoFromTarget (ECSqlTypeInfo const& targetTypeInfo)
    {
    SetTypeInfo (targetTypeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ParameterExp::ToECSql () const
    {
    if (!IsNamedParameter ())
        return "?";

    Utf8String str (":");
    str.append (m_parameterName);
    return str;
    }



//****************************** SetFunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus SetFunctionCallExp::_FinalizeParsing( ECSqlParseContext& ctx, FinalizeParseMode mode )
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const auto childCount = GetChildren ().size ();
    //all standard set functions require 1 arg
    if (!IsValidArgCount (ctx, 1, childCount))
        return FinalizeParseStatus::Error;

    //verify supported functions
    auto const& functionName = GetFunctionName ();
    if (BeStringUtilities::Stricmp (functionName, "COUNT") == 0)
        {
        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_Long));
        }
    else if (BeStringUtilities::Stricmp(functionName,"AVG") == 0)
        {
        if (!GetChild<ValueExp> (0)->GetTypeInfo ().IsNumeric ())
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid function call '%s'. Function %s expects numeric argument", ToECSql ().c_str (), functionName);
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_Double));
        }
    else if (BeStringUtilities::Stricmp(functionName,"MAX") == 0 ||
             BeStringUtilities::Stricmp(functionName,"MIN") == 0 ||
             BeStringUtilities::Stricmp(functionName,"SUM") == 0) //arg dependent
        {
        auto const& firstArgTypeInfo = GetChild<ValueExp> (0)->GetTypeInfo ();
        if (!firstArgTypeInfo.IsNumeric ())
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid function call '%s'. Function %s expects numeric argument", ToECSql ().c_str (), functionName);
            return FinalizeParseStatus::Error;
            }

        SetTypeInfo (firstArgTypeInfo);
        }
    else
        {
        BeAssert (false && "Unsupported standard set function. Check parse_general_set_fct()");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Unsupported standard set function '%s'. Check parse_general_set_fct()", functionName);
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SetFunctionCallExp::ToECSql() const 
    {
    Utf8String ecsql(GetFunctionName());
    ecsql.append ("(");
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
    return ecsql;
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


//****************************** StringFunctionCallExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus StringFunctionCallExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        SetTypeInfo (ECSqlTypeInfo (PRIMITIVETYPE_String));
        return FinalizeParseStatus::NotCompleted;
        }

    const auto childCount = GetChildren ().size ();
    auto const& functionName = GetFunctionName ();
    if (BeStringUtilities::Stricmp(functionName, "LOWER") == 0 || BeStringUtilities::Stricmp(functionName,"UPPER") == 0)
        {
        if (!IsValidArgCount (ctx, 1, childCount))
            return FinalizeParseStatus::Error;

        auto const& typeInfo = GetChild<ValueExp> (0)->GetTypeInfo ();
        if (!typeInfo.IsPrimitive () || typeInfo.GetPrimitiveType () != PRIMITIVETYPE_String)
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "Invalid function call '%s'. Function %s expects string argument", ToECSql ().c_str (), functionName);
            return FinalizeParseStatus::Error;
            }

        }
    else
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Function '%s' not yet supported.", functionName);
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StringFunctionCallExp::_ToString () const
    {
    Utf8String str ("StringFunctionCall [Function: ");
    str.append (GetFunctionName ()).append ("]");
    return str;
    }

//****************************** UnaryExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus UnaryExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    auto const& operandTypeInfo = GetOperand ()->GetTypeInfo ();
    if (!operandTypeInfo.IsNumeric ())
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Invalid type in expression %s: Unary operator expects a numeric type expression", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    switch (m_op)
        {
        case SqlUnaryOperator::MINUS:
        case SqlUnaryOperator::PLUS:
            {
            SetTypeInfo (operandTypeInfo);
            break;
            }
        case SqlUnaryOperator::BITWISE_NOT:
            {
            if (!operandTypeInfo.IsExactNumeric ())
                {
                ctx.SetError(ECSqlStatus::InvalidECSql, "Invalid type in expression %s: Unary operator BITWISE_NOT expects a scalar exact numeric type expression", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            SetTypeInfo (operandTypeInfo);
            break;
            }

        default:
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Invalid unary operator in expression %s.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnaryExp::ToECSql() const 
    {
    return ExpHelper::ToString(m_op) + Utf8String("(") + GetOperand ()->ToECSql() + ")";
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnaryExp::_ToString() const 
    {
    Utf8String str ("Unary [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
