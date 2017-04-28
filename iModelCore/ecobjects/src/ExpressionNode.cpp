/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ExpressionNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2012
//--------------+------------------------------------------------------------------------
static void performConcatenation(ECValueR evalResult, ECValueCR left, ECValueCR right)
    {
    Utf8String     resultString;
    Utf8CP leftString   = left.GetUtf8CP();
    Utf8CP rightString  = right.GetUtf8CP();
    resultString.reserve(strlen(leftString) + strlen(rightString) + 1);
    resultString.append(leftString);
    resultString.append(rightString);
    evalResult.SetUtf8CP(resultString.c_str(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToInt32(EvaluationResultR evalResult) 
    {
    if (!evalResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt32: WrongType. (parameter is not ECValue)");
        return ExpressionStatus::WrongType;
        }

    ECN::ECValueR    ecValue = *evalResult.GetECValue();
    if (ecValue.IsNull() || !ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt32: WrongType. (parameter is not primitive or is NULL)");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetInteger(ecValue.GetBoolean() ? 1 : 0);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt32: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Integer:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt32: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Long:
            ecValue.SetInteger((int)ecValue.GetLong());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt32: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Double:
            ecValue.SetInteger((int)ecValue.GetDouble());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt32: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_String:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt32: Integer conversion to string is not implemented");
            return ExpressionStatus::NotImpl;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::ConvertToInt32: WrongType. EvaluationResult value is not primitive(%s)", ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToString(ECN::ECValueR ecValue) 
    {
    if (ecValue.IsNull() || !ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToString: WrongType. EvaluationResult value is not primitive or is null");
        return ExpressionStatus::WrongType;
        }

    if (ecValue.IsString())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToString: %s", ecValue.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    // --  TODO -- should this involve extended types
    Utf8Char     buffer [80];
    switch(ecValue.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            BeStringUtilities::Snprintf(buffer, "%d", ecValue.GetInteger());
            break;
        case PRIMITIVETYPE_Boolean:
            //  TODO -- should this be locale specific?
            strcpy (buffer, ecValue.GetBoolean() ?  "true"  : "false");
            break;
        case PRIMITIVETYPE_Long:
            BeStringUtilities::Snprintf(buffer, "%lld", ecValue.GetLong());
            break;
        case PRIMITIVETYPE_Double:
            //  TODO -- needs locale, extended type.
            BeStringUtilities::Snprintf(buffer, "%f", ecValue.GetDouble());
            break;
        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToString: NotImplemented string conversion");
            return ExpressionStatus::NotImpl;
        }

    ecValue.SetUtf8CP(buffer);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToString: %s", ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToString(EvaluationResultR evalResult) 
    {
    return evalResult.IsECValue() ? ConvertToString(*evalResult.GetECValue()) : ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToInt64(EvaluationResultR evalResult) 
    {
    if (!evalResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt64: WrongType. EvaluationResult is not ECValue");
        return ExpressionStatus::WrongType;
        }

    ECN::ECValueR    ecValue = *evalResult.GetECValue();
    if (ecValue.IsNull() || !ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt64: WrongType. EvaluationResult value is not primitive or is null");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetLong(ecValue.GetBoolean() ? 1 : 0);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt64: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Integer:
            ecValue.SetLong(ecValue.GetInteger());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt64: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Long:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt64: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Double:
            ecValue.SetLong((int64_t)ecValue.GetDouble());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToInt64: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_String:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToInt64: Can not convert int64 to string");
            return ExpressionStatus::NotImpl;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::ConvertToInt64: WrongType. Evaluation result is not primitive %s", ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::WrongType; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToDouble(EvaluationResultR evalResult) 
    {
    if (!evalResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToDouble: WrongType. Evaluation result is not ECValue");
        return ExpressionStatus::WrongType;
        }

    ECN::ECValueR    ecValue = *evalResult.GetECValue();
    if (ecValue.IsNull() || !ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToDouble: WrongType. Evaluation result value is not primitive or is null");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetDouble(ecValue.GetBoolean() ? 1 : 0);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToDouble: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Integer:
            ecValue.SetDouble(ecValue.GetInteger());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToDouble: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Long:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToDouble: Can not convert long to double");
            return ExpressionStatus::WrongType;

        case PRIMITIVETYPE_Double:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToDouble: %s", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_String:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToDouble: Can not convert string to double");
            return ExpressionStatus::NotImpl;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::ConvertToDouble: WrongType. Evaluation result value is not primitive: %s", ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToDateTime(EvaluationResultR evalResult) {return ExpressionStatus::NotImpl; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToArithmeticOrBooleanOperand (EvaluationResultR evalResult) {return ExpressionStatus::WrongType; }
#ifdef NOTNOW
(
ref EvaluationResultCR     ecValue
)
    {
    if (ecValue.NativeValue is Boolean)
        {
        //  Converting from a value that has an expression type of string but a
        //  native type of boolean
        ecValue = new EvaluationResultCR (ECObjects.BooleanType, ecValue.NativeValue);
        return;
        }

    if (ecValue.NativeValue is int32_t)
        {
        //  Converting from a value that has an expression type of string but a
        //  native type of int32
        ecValue = new EvaluationResultCR (ECObjects.IntegerType, ecValue.NativeValue);
        return;
        }

    try
        {
        int     intValue = System.Convert.ToInt32 (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.IntegerType, intValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        bool     boolValue = System.Convert.ToBoolean (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.BooleanType, boolValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        long     longValue = System.Convert.ToInt64 (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.LongType, longValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        double     doubleValue = System.Convert.ToDouble (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.DoubleType, doubleValue);
        return;
        }
    catch (System.Exception)
        {
        }
    //  This is the last one we will try. If it does not work. let the exception go
    DateTime     dtValue = System.Convert.ToDateTime (ecValue.ExpressionValue);
    ecValue = new EvaluationResultCR (ECObjects.DateTimeType, dtValue);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToBooleanOperand (EvaluationResultR evalResult)
    {
    if (!evalResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToBooleanOperand: WrongType. Evaluation result is not ECValue");
        return ExpressionStatus::WrongType;
        }
    ECValueR    ecValue = *evalResult.GetECValue();
    if (ecValue.IsNull() || !ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::ConvertToBooleanOperand: WrongType. Evaluation result value is not primitive or is null");
        return ExpressionStatus::WrongType;
        }

    if (ecValue.IsBoolean())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToBooleanOperand: %s", ecValue.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    ECN::PrimitiveType   primType = ecValue.GetPrimitiveType();
    bool    boolValue = false;
    switch(primType)
        {
        case PRIMITIVETYPE_Integer:
            boolValue = ecValue.GetInteger() != 0;
            break;

        case PRIMITIVETYPE_Double:
            boolValue = ecValue.GetDouble() != 0;
            break;

        case PRIMITIVETYPE_Long:
            boolValue = ecValue.GetLong() != 0;
            break;

        case PRIMITIVETYPE_String:
            {
            Utf8CP strValue = ecValue.GetUtf8CP();
            if (!strcmp("0", strValue) || !strcmp("false", strValue))
                boolValue = false;
            else if (!strcmp("1", strValue) || !strcmp("true", strValue))
                boolValue = true;
            else
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::ConvertToBooleanOperand: WrongType.(%s)", ecValue.ToString().c_str()).c_str());
                return ExpressionStatus::WrongType;
                }
            }

        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::ConvertToBooleanOperand: WrongType. (%s)", ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::WrongType;
        }

    ecValue.SetBoolean(boolValue);

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::ConvertToBooleanOperand: %s", ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertStringToArithmeticOperand (EvaluationResultR ecValue) {return ExpressionStatus::WrongType; }
#ifdef NOTNOW
internal static void ConvertStringToArithmeticOperand
(
ref EvaluationResultCR     ecValue
)
    {
    if (ecValue.NativeValue is int32_t)
        {
        ecValue = new EvaluationResultCR (ECObjects.IntegerType, ecValue.NativeValue);
        return;
        }

    if (ecValue.NativeValue is Boolean)
        {
        int     intValue = System.Convert.ToInt32 (ecValue.NativeValue);
        ecValue = new EvaluationResultCR (ECObjects.IntegerType, intValue);
        return;
        }

    try
        {
        int     intValue = System.Convert.ToInt32 (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.IntegerType, intValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        long     longValue = System.Convert.ToInt64 (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.LongType, longValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        double     doubleValue = System.Convert.ToDouble (ecValue.ExpressionValue);
        ecValue = new EvaluationResultCR (ECObjects.DoubleType, doubleValue);
        return;
        }
    catch (System.Exception)
        {
        throw new ECExpressionsException.EvaluatorException (ECObjects.GetLocalizedString ("CantConvertToNumeric"));
        }

#if NOTNOW
    //  TODO -- we don't know how to do arithmetic on DateTime.
    //  This is the last one we will try. If it does not work. let the exception go
    DateTime     dtValue = System.Convert.ToDateTime (ecValue.ExpressionValue);
    ecValue = new EvaluationResultCR (ECObjects.DateTimeType, dtValue);
#endif
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertStringToArithmeticOrBooleanOperand (EvaluationResultR ecValue) {return ExpressionStatus::WrongType; }

#ifdef NOTNOW
static void ConvertStringToArithmeticOrBooleanOperand
(
ref ECEvaluationResult     ecValue
)
    {
    if (ecValue.NativeValue is Boolean)
        {
        //  Converting from a value that has an expression type of string but a
        //  native type of boolean
        ecValue = new ECEvaluationResult (ECObjects.BooleanType, ecValue.NativeValue);
        return;
        }

    if (ecValue.NativeValue is int32_t)
        {
        //  Converting from a value that has an expression type of string but a
        //  native type of int32
        ecValue = new ECEvaluationResult (ECObjects.IntegerType, ecValue.NativeValue);
        return;
        }

    try
        {
        int     intValue = System.Convert.ToInt32 (ecValue.ExpressionValue);
        ecValue = new ECEvaluationResult (ECObjects.IntegerType, intValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        bool     boolValue = System.Convert.ToBoolean (ecValue.ExpressionValue);
        ecValue = new ECEvaluationResult (ECObjects.BooleanType, boolValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        long     longValue = System.Convert.ToInt64 (ecValue.ExpressionValue);
        ecValue = new ECEvaluationResult (ECObjects.LongType, longValue);
        return;
        }
    catch (System.Exception)
        {
        }
    try
        {
        double     doubleValue = System.Convert.ToDouble (ecValue.ExpressionValue);
        ecValue = new ECEvaluationResult (ECObjects.DoubleType, doubleValue);
        return;
        }
    catch (System.Exception)
        {
        }
    //  This is the last one we will try. If it does not work. let the exception go
    DateTime     dtValue = System.Convert.ToDateTime (ecValue.ExpressionValue);
    ecValue = new ECEvaluationResult (ECObjects.DateTimeType, dtValue);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformArithmeticPromotion(EvaluationResult& leftResult, EvaluationResult& rightResult)
    {
    if (!leftResult.IsECValue() || !rightResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformArithmeticPromotion: Primitive required (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::ECValueR    left    = *leftResult.GetECValue();
    ECN::ECValueR    right   = *rightResult.GetECValue();

    if (!left.IsPrimitive() || !right.IsPrimitive() || left.IsNull() || right.IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformArithmeticPromotion: Primitive required (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::PrimitiveType   leftCode    = left.GetPrimitiveType();
    ECN::PrimitiveType   rightCode   = right.GetPrimitiveType();

    //  PRIMITIVETYPE_DateTime, point types, and Boolean are all missing from this
    //  We may also want to provide a way for structs with extended types to perform the conversion to 
    //  string
    //
    //  PerformArithmeticPromotion for Managed ECExpressions always converts strings to numeric,native 
    //  ECExpressions has never supported automatic conversions of strings to numbers.

    if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        {
        if (leftCode == rightCode)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        ECValueR            target = PRIMITIVETYPE_Long == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Long == leftCode ? rightCode : leftCode;
        if (PRIMITIVETYPE_Double == targetCode)
            {
            target.SetLong((int64_t)target.GetDouble());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: %s", target.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetLong(target.GetInteger());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: %s", target.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformArithmeticPromotion: IncompatibleTypes. (Unavailable long conversion)");
        return ExpressionStatus::IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Double == leftCode || PRIMITIVETYPE_Double == rightCode)
        {
        //  Both must be doubles
        if (leftCode == rightCode)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        ECValueR            target = PRIMITIVETYPE_Double == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Double == leftCode ? rightCode : leftCode;

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetDouble(target.GetInteger());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: %s", target.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformArithmeticPromotion: IncompatibleTypes. (Unavailable double conversion)");
        return ExpressionStatus::IncompatibleTypes;
        }

    //  Both must be int
    if (PRIMITIVETYPE_Integer == leftCode && PRIMITIVETYPE_Integer == rightCode)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformArithmeticPromotion: (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformArithmeticPromotion: WrongType.");
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations:: PerformJunctionPromotion
(
EvaluationResultR     left,
EvaluationResultR     right
)
    {
    if (!left.IsECValue() || !right.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformJunctionPromotion: WrongType. Invalid operands (not ECValues)");
        return ExpressionStatus::WrongType;
        }

    ECValueR lv = *left.GetECValue(), rv = *right.GetECValue();
    if (!lv.IsPrimitive() || !rv.IsPrimitive() || lv.IsNull() || rv.IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformArithmeticPromotion: WrongType. NotPrimitive (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::WrongType;
        }

    ExpressionStatus     status = ExpressionStatus::Success;

    if (lv.IsString())
        {
        status = ConvertStringToArithmeticOrBooleanOperand (left);
        if (ExpressionStatus::Success != status)
            return status;
        }

    if (rv.IsString())
        {
        status = ConvertStringToArithmeticOrBooleanOperand (right);
        if (ExpressionStatus::Success != status)
            return status;
        }

    if (lv.IsBoolean() || rv.IsBoolean())
        {
        status = ConvertToBooleanOperand (left);
        if (ExpressionStatus::Success == status)
            status = ConvertToBooleanOperand (right);
        return status;
        }

    return PerformArithmeticPromotion (left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformUnaryMinus
(
EvaluationResultR           resultOut,
EvaluationResultR           left
)
    {
    if (!left.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformUnaryMinus: Primitive required. (left is not ECValue");
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::ECValueR        ecLeft = *left.GetECValue();

    if (!ecLeft.IsPrimitive() || ecLeft.IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformUnaryMinus: IncompatibleTypes. Left is not primitive (%s)", ecLeft.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    ECN::PrimitiveType   primType = ecLeft.GetPrimitiveType();

    if (PRIMITIVETYPE_String == primType)
        {
        ExpressionStatus status = ConvertStringToArithmeticOperand(left);
        if (ExpressionStatus::Success != status)
            return status;

        primType = ecLeft.GetPrimitiveType();
        }

    ECValue v;
    switch (primType)
        {
        case PRIMITIVETYPE_Double:
            {
            v.SetDouble(-left.GetECValue()->GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            v.SetInteger(-left.GetECValue()->GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            v.SetLong(-left.GetECValue()->GetLong());
            break;
            }
        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformUnaryMinus: WrongType. Is not primitive(%s)", v.ToString().c_str()).c_str());
            return ExpressionStatus::WrongType;
        }

    resultOut = v;
    resultOut.SetUnits (left.GetUnits());
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformUnaryMinus: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformUnaryNot
(
EvaluationResultR           resultOut,
EvaluationResultR           left
)
    {
    if (!left.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformUnaryNot: Primitive required (left is not ECValue)");
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::ECValueR        ecLeft = *left.GetECValue();

    if (!ecLeft.IsPrimitive() || ecLeft.IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformUnaryNot: IncompatibleTypes. Left is not primitive(%s)", ecLeft.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    ECN::PrimitiveType   primType = ecLeft.GetPrimitiveType();

    if (PRIMITIVETYPE_String == primType)
        {
        ExpressionStatus status = ConvertStringToArithmeticOperand(left);
        if (ExpressionStatus::Success != status)
            return status;

        primType = ecLeft.GetPrimitiveType();
        }

    ECValue v;
    switch (primType)
        {
        case PRIMITIVETYPE_Boolean:
            {
            v.SetBoolean(!left.GetECValue()->GetBoolean());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            v.SetInteger(~left.GetECValue()->GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            v.SetLong(~left.GetECValue()->GetLong());
            break;
            }

        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformUnaryNot: WrongType. Is not primitive(%s)", v.ToString().c_str()).c_str());
            return ExpressionStatus::WrongType;
        }

    resultOut = v;
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformUnaryNot: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

//  TODO Can we apply this to arrays? strings?
//  TODO 64 bit types
//  TODO unsigned right shift
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformShift
(
EvaluationResultR         resultOut,
ExpressionToken      shiftOp,
EvaluationResultR         left,
EvaluationResultR         right
)
    {
    ExpressionStatus    status = ConvertToInt32(right);
    if (ExpressionStatus::Success != status)
        return status;

    if (!left.IsECValue() || !left.GetECValue()->IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformShift: WrongType. Left is not primitive");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType   primType = left.GetECValue()->GetPrimitiveType();
    int                 count = right.GetECValue()->GetInteger();

    //  If string, we may want to try to convert to int.
    ECValue v;
    switch (primType)
        {
        case PRIMITIVETYPE_Integer:
            {
            int     value = left.GetECValue()->GetInteger();
            if (shiftOp == TOKEN_ShiftLeft)
                {
                v.SetInteger(value << count);
                break;
                }
            else if (shiftOp == TOKEN_ShiftRight)
                {
                v.SetInteger(value >> count);
                break;
                }
            else if (shiftOp == TOKEN_UnsignedShiftRight)
                {
                uint32_t uvalue = (uint32_t)value;
                v.SetInteger((int)(uvalue >> count));
                break;
                }
            }
            break;

        case PRIMITIVETYPE_Long:
            {
            int64_t       value = left.GetECValue()->GetInteger();
            if (shiftOp == TOKEN_ShiftLeft)
                {
                v.SetLong(value << count);
                break;
                }
            else if (shiftOp == TOKEN_ShiftRight)
                {
                v.SetLong(value >> count);
                break;
                }
            else if (shiftOp == TOKEN_UnsignedShiftRight)
                {
                uint64_t uvalue = (uint64_t)value;
                v.SetLong((int64_t)(uvalue >> count));
                break;
                }
            }
            break;
        }

    if (!v.IsNull())
        {
        resultOut = v;
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformShift: Result: %s", resultOut.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }
    else
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformShift: WrongType. Not primitive (%s)", v.ToString().c_str()).c_str());
        return ExpressionStatus::WrongType;
        }
    }

/*---------------------------------------------------------------------------------**//**
* For multiplication, exponenentiation, and division, at most one operand can have units.
* The other must be a scalar. (Our current units system has no way to do analysis to figure
* out the units of an operation involving two unitized quantities e.g. L * W = Area)
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::EnforceMultiplicativeUnits (UnitSpecR units, EvaluationResultR left, EvaluationResultR right)
    {
    UnitSpecCR lUnit = left.GetUnits(), rUnit = right.GetUnits();
    if (lUnit.IsUnspecified())
        units = rUnit;
    else if (rUnit.IsUnspecified())
        units = lUnit;
    else
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::EnforceMultiplicativeUnits: IncompatibleUnits");
        return ExpressionStatus::IncompatibleUnits;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::EnforceMultiplicativeUnits: (%s)", units.ToECExpressionString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::EnforceLikeUnits (EvaluationResultR left, EvaluationResultR right)
    {
    UnitSpecCR lUnit = left.GetUnits(), rUnit = right.GetUnits();
    if (lUnit.IsUnspecified() != rUnit.IsUnspecified())
        {
        // assume non-unitized operand uses same units as unitized operand
        if (lUnit.IsUnspecified())
            left.SetUnits (rUnit);
        else
            right.SetUnits (lUnit);

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::EnforceLikeUnits: (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }
    else if (lUnit.IsUnspecified())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::EnforceLikeUnits: (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::Success;  // neither has units
        }
    else if (!lUnit.IsCompatible(rUnit))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::EnforceLikeUnits: IncompatibleUnits (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleUnits;
        }
    else if (lUnit.IsEquivalent(rUnit))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::EnforceLikeUnits: (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::Success;  // units compatible and no conversion required
        }
    // Convert rhs to units of lhs
    ECValueR rv = *right.GetECValue();
    if (!rv.ConvertToPrimitiveType(PRIMITIVETYPE_Double))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::EnforceLikeUnits: UnknownError (Could not convert right units to units of left)");
         return ExpressionStatus::UnknownError;
       }

    double rd = rv.GetDouble();
    if (!rUnit.ConvertTo(rd, lUnit))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::EnforceLikeUnits: IncompatibleUnits. Could not convert right unit to left unit (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleUnits;
        }

    rv.SetDouble (rd);
    right.SetUnits (lUnit);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::EnforceLikeUnits: (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformMultiplication
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right,
bool                        enforceUnits
)
    {
    UnitSpec units;
    ExpressionStatus status = PerformArithmeticPromotion (left, right);
    if (enforceUnits && ExpressionStatus::Success == status)
        status = EnforceMultiplicativeUnits (units, left, right);

    if (ExpressionStatus::Success != status)
        return status;

    ECN::PrimitiveType   primType = left.GetECValue()->GetPrimitiveType();

    ECValue v;
    switch (primType)
        {
        case PRIMITIVETYPE_Integer:
            {
            v.SetInteger(left.GetECValue()->GetInteger() * right.GetECValue()->GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            v.SetLong(left.GetECValue()->GetLong() * right.GetECValue()->GetLong());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            v.SetDouble(left.GetECValue()->GetDouble() * right.GetECValue()->GetDouble());
            break;
            }

        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformMultiplication: WrongType. Is not primitive (%s)", v.ToString().c_str()).c_str());
            return ExpressionStatus::WrongType;
        }

    resultOut = v;
    resultOut.SetUnits (units);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformMultiplication: Result: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformExponentiation
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right
)
    {
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformExponentiation: NotImplemented (PerformExponentiation is not implemented)");
    return ExpressionStatus::NotImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformIntegerDivision
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right,
bool                        enforceUnits
)
    {
    ExpressionStatus status = PerformArithmeticPromotion (left, right);

    Unit units;
    if (enforceUnits && ExpressionStatus::Success == status)
        status = EnforceMultiplicativeUnits (units, left, right);

    if (ExpressionStatus::Success != status)
        return status;

    ECValue v;
    switch (left.GetECValue()->GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            int     divisor = right.GetECValue()->GetInteger();
            if (0 == divisor)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformIntegerDivision: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::DivideByZero;
                }

            v.SetInteger(left.GetECValue()->GetInteger() / divisor);

            break;
            }

        case PRIMITIVETYPE_Long:
            {
            int64_t   divisor = right.GetECValue()->GetLong();
            if (0 == divisor)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformIntegerDivision: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::DivideByZero;
                }

            v.SetLong(left.GetECValue()->GetLong() / divisor);

            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double     divisor = right.GetECValue()->GetDouble();
            if (0 == divisor)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformIntegerDivision: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::DivideByZero;
                }

            v.SetDouble(floor(left.GetECValue()->GetDouble() / divisor));

            break;
            }

        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformIntegerDivision: InvalidTypesForDivision (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
            return ExpressionStatus::InvalidTypesForDivision;
        }

    resultOut = v;
    resultOut.SetUnits (units);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformIntegerDivision: Result: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformDivision
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right,
bool                        enforceUnits
)
    {
    ExpressionStatus status = ConvertToDouble (left);
    if (ExpressionStatus::Success != status)
        return status;

    status = ConvertToDouble (right);
    if (ExpressionStatus::Success != status)
        return status;

    double  divisor = right.GetECValue()->GetDouble();
    if (0 == divisor)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformDivision: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::DivideByZero;
        }

    UnitSpec units;
    if (enforceUnits)
        {
        auto status = EnforceMultiplicativeUnits (units, left, right);
        if (ExpressionStatus::Success != status)
            return status;
        }

    resultOut = ECValue (left.GetECValue()->GetDouble() / divisor);
    resultOut.SetUnits (units);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformDivision: Result: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformMod
(
EvaluationResultR            resultOut,
EvaluationResultR            left,
EvaluationResultR            right
)
    {
    ExpressionStatus status = PerformArithmeticPromotion (left, right);

    if (ExpressionStatus::Success != status)
        return status;

    switch (left.GetECValue()->GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            int     divisor = right.GetECValue()->GetInteger();
            if (0 == divisor)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformMod: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::DivideByZero;
                }

            resultOut = ECValue (left.GetECValue()->GetInteger() % divisor);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformMod: Result: %s", resultOut.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        case PRIMITIVETYPE_Long:
            {
            int64_t   divisor = right.GetECValue()->GetLong();
            if (0 == divisor)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformMod: DivideByZero (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::DivideByZero;
                }

            resultOut = ECValue (left.GetECValue()->GetLong() % divisor);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformMod: Result: %s", resultOut.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("Operations::PerformMod: InvalidTypesForDivision (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
    return ExpressionStatus::InvalidTypesForDivision;
    }

#ifdef NOTNOW
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformLikeTest
(
EvaluationResultR            resultOut,
EvaluationResultR            left,
EvaluationResultR            right
)
//  Need to decide on standard pattern matching
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformJunctionOperator
(
EvaluationResultR            resultOut,
ExpressionToken         junctionOperator,
EvaluationResultR            left,
EvaluationResultR            right
)
    {
    ExpressionStatus status = PerformJunctionPromotion (left, right);

    if (ExpressionStatus::Success != status)
        return status;

    if (left.GetECValue()->IsBoolean())
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.InitECValue().SetBoolean(left.GetECValue()->GetBoolean() | right.GetECValue()->GetBoolean());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_And:
                resultOut.InitECValue().SetBoolean(left.GetECValue()->GetBoolean() & right.GetECValue()->GetBoolean());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_Xor:
                resultOut.InitECValue().SetBoolean(left.GetECValue()->GetBoolean() ^ right.GetECValue()->GetBoolean());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
            }
        }

    ECN::PrimitiveType  primType = left.GetECValue()->GetPrimitiveType();
    if (PRIMITIVETYPE_Integer == primType)
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.InitECValue().SetInteger(left.GetECValue()->GetInteger() | right.GetECValue()->GetInteger());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_And:
                resultOut.InitECValue().SetInteger(left.GetECValue()->GetInteger() & right.GetECValue()->GetInteger());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_Xor:
                resultOut.InitECValue().SetInteger(left.GetECValue()->GetInteger() ^ right.GetECValue()->GetInteger());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
            }
        }

    if (PRIMITIVETYPE_Long == primType)
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.InitECValue().SetLong(left.GetECValue()->GetLong() | right.GetECValue()->GetLong());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_And:
                resultOut.InitECValue().SetLong(left.GetECValue()->GetLong() & right.GetECValue()->GetLong());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;

            case TOKEN_Xor:
                resultOut.InitECValue().SetLong(left.GetECValue()->GetLong() ^ right.GetECValue()->GetLong());
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformJunctionOperator: Operator: %s left: %s, right: %s", Lexer::GetString(junctionOperator).c_str(), left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
            }
        }


    //  It should be impossible to get here. All errors should be caught in the promotion functions.
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "Operations::PerformJunctionOperator: WrongType");
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations:: PerformLogicalOr
(
EvaluationResultR            resultOut,
EvaluationResultR            leftValue,
EvaluationResultR            rightValue
)
    {
    bool        boolValue;
    
    ExpressionStatus status = leftValue.GetBoolean(boolValue, false);
    
    if (ExpressionStatus::Success != status)
        return status;

    if (boolValue)
        {
        resultOut = *leftValue.GetECValue();
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformLogicalOr: Result: %s", resultOut.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    status = rightValue.GetBoolean(boolValue, false);
    if (ExpressionStatus::Success != status)
        return status;

    resultOut = *rightValue.GetECValue();
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformLogicalOr: Result: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformLogicalAnd
(
EvaluationResultR            resultOut,
EvaluationResultR            leftValue,
EvaluationResultR            rightValue
)
    {
    bool        boolValue;
    
    ExpressionStatus status = leftValue.GetBoolean(boolValue, false);
    
    if (ExpressionStatus::Success != status)
        return status;

    if (!boolValue)
        {
        resultOut = *leftValue.GetECValue();
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformLogicalAnd: Result: %s", resultOut.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    status = rightValue.GetBoolean(boolValue, false);
    if (ExpressionStatus::Success != status)
        return status;

    resultOut = *rightValue.GetECValue();
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("Operations::PerformLogicalAnd: Result: %s", resultOut.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResult::EvaluationResult()
    : m_valueList (NULL), m_ownsInstanceList (false), m_valueType (ValType_None)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResult::~EvaluationResult()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetInteger(int32_t& result)
    {
    if (ValType_ECValue != m_valueType || !m_ecValue.IsInteger() || m_ecValue.IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("EvaluationResult::GetInteger: WrongType. Is not an integer (%s)", m_ecValue.ToString().c_str()).c_str());
        return ExpressionStatus::WrongType;
        }

    result = m_ecValue.GetInteger();
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluationResult::GetInteger: Result: %s", m_ecValue.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueR EvaluationResult::InitECValue()
    {
    Clear();
    m_valueType = ValType_ECValue;
    return m_ecValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetECValue(ECN::ECValueR result)
    {
    if (ValType_ECValue != m_valueType)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("EvaluationResult::GetECValue: WrongType. Result: %s", result.ToString().c_str()).c_str());
        return ExpressionStatus::WrongType;
        }

    result.Clear();
    result = m_ecValue;
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECValueP    EvaluationResult::GetECValue() 
    { 
    return ValType_ECValue == m_valueType ? &m_ecValue : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECValueCP   EvaluationResult::GetECValue() const 
    { 
    return ValType_ECValue == m_valueType ? &m_ecValue : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::UnitSpecCR EvaluationResult::GetUnits() const { return m_units; }
void EvaluationResult::SetUnits (UnitSpecCR units) { m_units = units; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResult::EvaluationResult (EvaluationResultCR rhs)
    : m_valueList(NULL), m_ownsInstanceList(false), m_valueType(rhs.m_valueType), m_units(rhs.m_units)
    {
    if (ValType_InstanceList == rhs.m_valueType && NULL != rhs.m_instanceList)
        {
        if (!rhs.m_ownsInstanceList)
            m_instanceList = rhs.m_instanceList;
        else
            {
            m_instanceList = new ECInstanceList (*rhs.m_instanceList);
            m_ownsInstanceList = true;
            }
        }
    else if (ValType_ECValue == rhs.m_valueType)
        m_ecValue = rhs.m_ecValue;
    else if (ValType_ValueList == rhs.m_valueType)
        {
        m_valueList = rhs.m_valueList;
        m_valueList->AddRef();
        }
    else if (ValType_Lambda == rhs.m_valueType)
        {
        m_lambda = rhs.m_lambda;
        m_lambda->AddRef();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResultR EvaluationResult::operator=(EvaluationResultCR rhs)
    {
    Clear();

    m_valueType = rhs.m_valueType;
    m_units = rhs.m_units;
    m_ecValue = rhs.m_ecValue;
    if (m_valueType == ValType_InstanceList)
        SetInstanceList (*rhs.m_instanceList, rhs.m_ownsInstanceList);
    else if (m_valueType == ValType_ValueList)
        SetValueList (*rhs.m_valueList);
    else if (m_valueType == ValType_Lambda)
        SetLambda (*rhs.m_lambda);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            EvaluationResult::Clear()
    {
    if (ValType_InstanceList == m_valueType)
        {
        if (m_ownsInstanceList && NULL != m_instanceList)
            delete m_instanceList;
        }
    else if (ValType_ValueList == m_valueType)
        {
        if (NULL != m_valueList)
            m_valueList->Release();
        }
    else if (ValType_Lambda == m_valueType)
        {
        if (NULL != m_lambda)
            m_lambda->Release();
        }

    m_ecValue.Clear();
    m_ownsInstanceList = false;
    m_instanceList = NULL;  // and m_valueList, and m_lambda...
    m_valueList = NULL;
    m_valueType = ValType_None;
    m_units = UnitSpec();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetBoolean(bool& result, bool requireBoolean)
    {
    if (ValType_ECValue != m_valueType)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "EvaluationResult::GetBoolean: WrongType");
        return ExpressionStatus::WrongType;
        }

    if (m_ecValue.IsNull())
        {
        result = false;
        if (requireBoolean)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "EvaluationResult::GetBoolean: WrongType (EC Value is Null)");
            return ExpressionStatus::WrongType;
            }
        else
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "EvaluationResult::GetBoolean: Result: false");
            return ExpressionStatus::Success;
            }
        }

    if (!m_ecValue.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "EvaluationResult::GetBoolean: WrongType (EC Value is not primitive)");
        return ExpressionStatus::WrongType;
        }

    switch(m_ecValue.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            result = m_ecValue.GetBoolean();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluationResult::GetBoolean: Result: %s", m_ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Integer:
            result = m_ecValue.GetInteger() != 0;
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluationResult::GetBoolean: Result: %s", m_ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Long:
            result = m_ecValue.GetLong() != 0;
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluationResult::GetBoolean: Result: %s", m_ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Double:
            result = m_ecValue.GetDouble() != 0.0;
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluationResult::GetBoolean: Result: %s", m_ecValue.ToString().c_str()).c_str());
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_String:
            {
            Utf8CP value = m_ecValue.GetUtf8CP();
            if (!strcmp("1", value) || !BeStringUtilities::StricmpAscii("true", value))
                {
                result = true;
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "EvaluationResult::GetBoolean: Result: true");
                return ExpressionStatus::Success;
                }

            if (!strcmp("0", value) || !BeStringUtilities::StricmpAscii("false", value))
                {
                result = false;
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "EvaluationResult::GetBoolean: Result: false");
                return ExpressionStatus::Success;
                }

            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "EvaluationResult::GetBoolean: WrongType (EC Value primitive type not convertible to bool)");
            return ExpressionStatus::WrongType;
            }
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "EvaluationResult::GetBoolean: WrongType");
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceListCP EvaluationResult::GetInstanceList() const
    {
    return ValType_InstanceList == m_valueType ? m_instanceList : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void EvaluationResult::SetInstance (IECInstanceCR instance)
    {
    Clear();
    IECInstancePtr pInstance = const_cast<IECInstanceP>(&instance);
    m_valueType = ValType_InstanceList;
    m_ownsInstanceList = true;
    m_instanceList = new ECInstanceList (1, pInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void EvaluationResult::SetInstanceList (ECInstanceListCR instanceList, bool makeACopy = false)
    {
    Clear();
    m_valueType = ValType_InstanceList;
    if (makeACopy)
        {
        m_ownsInstanceList = true;
        m_instanceList = new ECInstanceList (instanceList);
        }
    else
        m_instanceList = &instanceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
IValueListResultCP EvaluationResult::GetValueList() const
    {
    return ValType_ValueList == m_valueType ? m_valueList : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
IValueListResultP EvaluationResult::GetValueList()
    {
    return ValType_ValueList == m_valueType ? m_valueList  :NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void EvaluationResult::SetValueList (IValueListResultR valueList)
    {
    Clear();
    m_valueList = &valueList;
    m_valueList->AddRef();
    m_valueType = ValType_ValueList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
LambdaValueCP EvaluationResult::GetLambda() const
    {
    return ValType_Lambda == m_valueType ? m_lambda : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void EvaluationResult::SetLambda (LambdaValueR value)
    {
    Clear();
    m_valueType = ValType_Lambda;
    m_lambda = &value;
    m_lambda->AddRef();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Aidas.Vaiksnoras  04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String EvaluationResult::ToString() const
    {
    if (IsECValue())
        return GetECValue()->ToString();
    if (IsInstanceList())
        {
        Utf8String msg = "";
        for (IECInstancePtr instance : *(GetInstanceList()))
            {
            if (!msg.empty())
                msg.append(", ");
            msg.append(instance->GetClass().GetFullName());
            msg.append(":");
            msg.append(instance->GetInstanceId());
            }
        return msg;
        }
    if (IsLambda())
        return GetLambda()->GetNode().ToExpressionString();
    if (IsValueList())
        return GetValueList()->ToString();
    return "<None>";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ValueResult::ValueResult (EvaluationResultR result)
    : m_evalResult(result) 
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ValueResult::~ValueResult()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ValueResultPtr  ValueResult::Create(EvaluationResultR result) 
    { 
    return new ValueResult(result); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            NodeHelpers::GetAdditiveNodes(NodeCPVector& nodes, NodeCR rightMost)
    {
    BeAssert (rightMost.IsAdditive());

    BinaryNodeCP    current = dynamic_cast<BinaryNodeCP>(&rightMost);
    BeAssert(NULL != current);

    NodeCP  left = current->GetLeftCP();
    if (left->IsAdditive())
        GetAdditiveNodes(nodes, *left);

    nodes.push_back(&rightMost);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String         Node::ToString() const
    {
    return _ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DotNode::_ToString() const
    {
    Utf8String str (".");
    for (auto const& qualifier : GetQualifiers())
        str.append (qualifier).append (2, ':');

    str.append (GetName());
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr Node::GetResolvedTree(ExpressionResolverR context)
    {
    return _GetResolvedTree(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Node::Traverse(NodeVisitorR visitor) const
    {
    if (this->GetHasParens() && !visitor.OpenParens())
        return false;
    
    if (!_Traverse (visitor))
        return false;

    if (this->GetHasParens() && !visitor.CloseParens())
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr Node::_GetResolvedTree(ExpressionResolverR context)
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedTypeNodePtr Node::CreateBooleanLiteral(bool literalValue)   { return LiteralNode::CreateBoolean (literalValue); }
ResolvedTypeNodePtr Node::CreateNullLiteral()                       { return LiteralNode::CreateNull(); }
ResolvedTypeNodePtr Node::CreatePoint2dLiteral (DPoint2dCR pt)      { return LiteralNode::CreatePoint2d (pt); }
ResolvedTypeNodePtr Node::CreatePoint3dLiteral (DPoint3dCR pt)      { return LiteralNode::CreatePoint3d (pt); }
ResolvedTypeNodePtr Node::CreateDateTimeLiteral (int64_t ticks)       { return LiteralNode::CreateDateTime (ticks); }
ResolvedTypeNodePtr Node::CreateIntegerLiteral (int value)          { return LiteralNode::CreateInteger (value); }
ResolvedTypeNodePtr Node::CreateInt64Literal(int64_t value)           { return LiteralNode::CreateLong (value); }
ResolvedTypeNodePtr Node::CreateFloatLiteral(double value)          { return LiteralNode::CreateDouble (value); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedTypeNodePtr Node::CreateStringLiteral (Utf8CP value, bool quoted)
    {
    if (!quoted)
        return LiteralNode::CreateString (value);

    size_t      origLen = strlen(value);
    BeAssert(origLen > 1);
    Utf8Char*    buffer = (Utf8Char*)_alloca(sizeof(*buffer) *(origLen+1));

    BeStringUtilities::Strncpy(buffer, origLen, value+1);
    buffer[origLen-2] = 0;

    return LiteralNode::CreateString (buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateUnaryArithmetic(ExpressionToken tokenId, NodeR left)
    {
    return new UnaryArithmeticNode(tokenId, left);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateArithmetic(ExpressionToken  tokenId, NodeR left, NodeR right)
    {
    switch (tokenId)
        {
        case TOKEN_Exponentiation:
            return new ExponentNode(left, right);

        case TOKEN_Star:
            return new MultiplyNode(tokenId, left, right);

        case TOKEN_Slash:
        case TOKEN_IntegerDivide:
        case TOKEN_Mod:
            return new DivideNode(tokenId, left, right);

        case TOKEN_Plus:
        case TOKEN_Minus:
            return new PlusMinusNode(tokenId, left, right);

        case TOKEN_Concatenate:
            return new ConcatenateNode(left, right);
        }

    BeAssert (false && L"invalid arithmetic token");

    return ErrorNode::Create("internal error: unexpected arithmetic token", NULL, NULL).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateShift (ExpressionToken tokenId, NodeR left, NodeR right)
    {
    return new ShiftNode(tokenId, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateComparison(ExpressionToken   tokenId, NodeR left, NodeR right)
    {
    return new ComparisonNode(tokenId, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateLogical(ExpressionToken tokenId, NodeR left, NodeR right)
    {
    return new LogicalNode(tokenId, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateAssignment(NodeR left, NodeR rightSide, ExpressionToken assignmentSubtype)
    {
    return new AssignmentNode (left, rightSide, assignmentSubtype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ArgumentTreeNodePtr Node::CreateArgumentTree()
    {
    return new ArgumentTreeNode ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateIIf(NodeR conditional, NodeR trueNode, NodeR falseNode)
    {
    return new IIfNode (conditional, trueNode, falseNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ExpressionType::Init ()
    {
    m_unitsPower        = 0;
    m_valueKind         = ECN::VALUEKIND_Uninitialized;
    m_arrayKind         = ECN::ARRAYKIND_Primitive;
    m_structClass       = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ExpressionType::ExpressionType
(
ECN::PrimitiveECPropertyR primitiveProp
)
    {
    Init();

    m_valueKind = ECN::VALUEKIND_Primitive;
    m_primitiveType = primitiveProp.GetType();

#if defined (NOTNOW)
            //  Need something like this to get the extended type custom attribute
            IECInstancePtr  propertyCustomAttribute = ecProperty->GetCustomAttribute (*propertyInfoCAClass);
            if (propertyCustomAttribute.IsValid() &&  (ECObjectsStatus::Success == propertyCustomAttribute->GetValue (value, L"DefaultValue")))
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PrimaryListNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    return context.GetValue(evalResult, *this, {&context}, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP  PrimaryListNode::GetName(size_t index) const
    {
    if (m_operators.size() <= index)
        return NULL;

    NodeP   node = m_operators[index].get();
    if (!node)
        return NULL;

    ExpressionToken    nodeId = node->GetOperation();
    if (TOKEN_Ident == nodeId || TOKEN_Dot == nodeId)
        {
        IdentNodeP identNode = static_cast<IdentNodeP>(node);
        return identNode->GetName();
        }
    else if (TOKEN_Lambda == nodeId)
        {
        LambdaNodeP lambdaNode = static_cast<LambdaNodeP>(node);
        return lambdaNode->GetSymbolName();
        }

    BeAssert(TOKEN_LParen == nodeId);

    CallNodeP   callNode = static_cast<CallNodeP>(node);
    return callNode->GetMethodName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          PrimaryListNode::GetNumberOfOperators() const
    {
    return m_operators.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP           PrimaryListNode::GetOperatorNode(size_t index) const
    {
    if (index >= m_operators.size())
        return NULL;

    return m_operators[index].get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken PrimaryListNode::GetOperation(size_t index) const
    {
    if (index >= m_operators.size())
        return TOKEN_None;

    return m_operators[index]->GetOperation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrimaryListNode::AppendCallNode(CallNodeR callNode)
    {
    m_operators.push_back(&callNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrimaryListNode::AppendNameNode(IdentNodeR nameNode)
    {
    m_operators.push_back(&nameNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrimaryListNode::AppendLambdaNode (LambdaNodeR lambdaNode)
    {
    m_operators.push_back (&lambdaNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus LambdaNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    // Bind expression and context
    evalResult.SetLambda (*LambdaValue::Create (*this, context));
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrimaryListNode::AppendArrayNode(LBracketNodeR lbracketNode)
    {
    m_operators.push_back(&lbracketNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
void IdentNode::PushQualifier(Utf8CP rightName)
    {
    m_qualifiers.push_back(Utf8String(m_value));
    m_value = Utf8String(rightName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus UnaryArithmeticNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    inputValue;
    ExpressionStatus    status = _GetLeftP()->GetValue(inputValue, context);
    if (ExpressionStatus::Success != status)
        return status;

    if (_GetOperation() == TOKEN_Minus)
        return Operations::PerformUnaryMinus(evalResult, inputValue);

    if (_GetOperation() == TOKEN_Plus)
        {
        evalResult = inputValue;
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("UnaryArithmeticNode::_GetValue: %s", evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    return Operations::PerformUnaryNot(evalResult, inputValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus ResolveMethod(MethodReferencePtr& methodReference, Utf8CP name, bool useOuter, bvector<ExpressionContextP> contexts)
    {
    // note: start looking from the topmost context on the stack
    for (auto iter = contexts.rbegin(); iter != contexts.rend(); ++iter)
        {
        if (ExpressionStatus::Success == (*iter)->ResolveMethod(methodReference, name, useOuter))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ResolveMethod: %s", name).c_str());
            return ExpressionStatus::Success;
            }
        }
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ResolveMethod: MethodRequired (%s)", name).c_str());
    return ExpressionStatus::MethodRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, bvector<ExpressionContextP> const& contextsStack)
    {
    MethodReferencePtr methodRef;
    ExpressionStatus status = ResolveMethod(methodRef, GetMethodName(), true, contextsStack);
    if (ExpressionStatus::Success == status)
        {
        EvaluationResultVector argsList;
        status = m_arguments->EvaluateArguments (argsList, *contextsStack.front());
        if (ExpressionStatus::Success == status)
            status = methodRef->InvokeValueListMethod (evalResult, valueList, argsList);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeInstanceMethod(EvaluationResult& evalResult, ECInstanceListCR instanceData, bvector<ExpressionContextP> const& contextsStack)
    {
    MethodReferencePtr  methodReference;

    //  The lookup should include the instance data since that would be the most logical place to find the method reference
    ExpressionStatus    exprStatus = ResolveMethod(methodReference, GetMethodName(), true, contextsStack);
    if (ExpressionStatus::Success != exprStatus)
        {
        evalResult = ECN::ECValue();
        return exprStatus;
        }

    EvaluationResultVector  argsVector;

    ExpressionStatus status = m_arguments->EvaluateArguments(argsVector, *contextsStack.front());
    if (ExpressionStatus::Success != status)
        return status;

    return methodReference->InvokeInstanceMethod(evalResult, instanceData, argsVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeStaticMethod(EvaluationResult& evalResult, MethodReferenceR methodReference, bvector<ExpressionContextP> const& contextsStack)
    {
    EvaluationResultVector  argsVector;

    ExpressionStatus status = m_arguments->EvaluateArguments(argsVector, *contextsStack.front());
    if (ExpressionStatus::Success != status)
        return status;

    return methodReference.InvokeStaticMethod(evalResult, argsVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeStaticMethod(EvaluationResult& evalResult, bvector<ExpressionContextP> const& contextsStack)
    {
    MethodReferencePtr  methodReference;

    //  The lookup should include the instance data since that would be the most logical place to find the method reference
    ExpressionStatus    exprStatus = ResolveMethod(methodReference, GetMethodName(), true, contextsStack);
    if (ExpressionStatus::Success != exprStatus)
        {
        evalResult = ECN::ECValue();
        return exprStatus;
        }

    return InvokeStaticMethod(evalResult, *methodReference, contextsStack);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus AssignmentNode::PerformModifier (ExpressionToken  modifier, EvaluationResultR left, EvaluationResultR right)
    {
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "AssignmentNode::PerformModifier: Not implemented");
    return ExpressionStatus::NotImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static IECInstancePtr   getInstanceFromResult (EvaluationResultCR result)
    {
    if (result.IsInstanceList())
        return result.GetInstanceList()->size() == 1 ? *result.GetInstanceList()->begin() : NULL;
    else if (result.IsECValue() && result.GetECValue()->IsStruct())
        return result.GetECValue()->GetStruct();
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus AssignmentNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    // Modifiers not implemented
    BeAssert (_GetOperation() == TOKEN_None);

#if defined (NOTNOW)
    ExpressionToken    operation = _GetOperation();
    if (TOKEN_None != operation)
        {
        EvaluationResult    leftResult;
        status = _GetLeftP()->GetValue(leftResult, context);
        if (ExpressionStatus::Success != status)
            return status;

        status = PerformModifier(operation, leftResult, evalResult);
        if (ExpressionStatus::Success != status)
            return status;
        }
#endif

    NodeP   leftNode = _GetLeftP();
    if (leftNode->GetOperation() != TOKEN_PrimaryList)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("AssignmentNode::_GetValue: UnknownError. Invalid left node operation (%s)", Lexer::GetString(leftNode->GetOperation()).c_str()).c_str());
        return ExpressionStatus::UnknownError;
        }

    PrimaryListNodeP    primaryList = static_cast<PrimaryListNodeP>(leftNode);
    EvaluationResult    instanceResult;
    ReferenceResult     refResult;

    ExpressionStatus    exprStatus = context.GetReference(instanceResult, refResult, *primaryList, {&context}, 0);
    if (ExpressionStatus::Success != exprStatus)
        return exprStatus;

    exprStatus = _GetRightP()->GetValue(evalResult, context);
    if (ExpressionStatus::Success != exprStatus)
        return exprStatus;

    ECN::PrimitiveECPropertyCP   primProperty = refResult.m_property->GetAsPrimitiveProperty();
    if (NULL != primProperty)
        {
        ECN::IECInstancePtr  instance = getInstanceFromResult (instanceResult);
        ECN::ECEnablerCR     enabler = instance->GetEnabler();

        ::uint32_t   propertyIndex;
        if (enabler.GetPropertyIndex(propertyIndex, refResult.m_accessString.c_str()) != ECN::ECObjectsStatus::Success)
            {
            evalResult.Clear();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "AssignmentNode::_GetValue: UknownError. Could not get property index");
            return ExpressionStatus::UnknownError;
            }

        //  Need to add conversions, support for DateTime, points
#if defined (NOTNOW)
        switch(primProperty->GetType())
            {
            case ECN::PRIMITIVETYPE_String:
            case ECN::PRIMITIVETYPE_Long:
            case ECN::PRIMITIVETYPE_Integer:
            case ECN::PRIMITIVETYPE_Double:
#endif
        ECN::ECObjectsStatus  ecStatus = instance->SetValue(propertyIndex, *evalResult.GetECValue());
        if (ECN::ECObjectsStatus::Success != ecStatus)
            {
            evalResult.Clear();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "AssignmentNode::_GetValue: UknownError. Could not set value to instance ");
            return ExpressionStatus::UnknownError;
            }

        return ExpressionStatus::Success;
        }  //  End of processing for primitive property


    //  For now only support setting primitive properties; should add support for arrays here
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "AssignmentNode::_GetValue: PrimitiveRequired");
    return ExpressionStatus::PrimitiveRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ArgumentTreeNode::EvaluateArguments(EvaluationResultVector& results, ExpressionContextR context) const
    {
    ExpressionStatus    status = ExpressionStatus::Success;
    BeAssert (results.size() == 0);
    results.reserve(m_arguments.size());
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "EvaluateArguments: Started Evaluating arguments");

    for (NodePtrVector::const_iterator curr = m_arguments.begin(); curr != m_arguments.end(); ++curr)
        {
        results.push_back(EvaluationResult());
        EvaluationResultR currValue = results.back();
        status = (*curr)->GetValue(currValue, context);
        if (ExpressionStatus::Success != status)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("EvaluateArguments: Evaluated argument unsuccessfully: %s", currValue.ToString().c_str()).c_str());
            return status;
            }
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("EvaluateArguments: Evaluated argument: %s", currValue.ToString().c_str()).c_str());
        }
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "EvaluateArguments: Finished Evaluating arguments");
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus IIfNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    local;

    ExpressionStatus    status = m_condition->GetValue(local, context);
    if (ExpressionStatus::Success != status)
        return status;

    bool    condition;
    status = local.GetBoolean(condition, false);
    if (ExpressionStatus::Success != status)
        return status;

    if (condition)
        return m_true->GetValue(evalResult, context);

    return m_false->GetValue(evalResult, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ArithmeticNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExpressionStatus::Success != status)
        return status;

    status = _Promote(leftResult, rightResult, context);
    if (ExpressionStatus::Success != status)
        return status;

    return _PerformOperation(evalResult, leftResult, rightResult, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ConcatenateNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExpressionStatus::Success != status)
        return status;
    
    if (((status = Operations::ConvertToString(leftResult)) != ExpressionStatus::Success) || ((status = Operations::ConvertToString(rightResult)) != ExpressionStatus::Success))
        return status;

    performConcatenation (evalResult.InitECValue(), *leftResult.GetECValue(), *rightResult.GetECValue());

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ConcatenateNode::_GetValue: Result: ", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ShiftNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExpressionStatus::Success != status)
        return status;

    return Operations::PerformShift(evalResult, m_operatorCode, leftResult, rightResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  LogicalNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = ExpressionStatus::UnknownError;

    switch (m_operatorCode)
        {
    case TOKEN_And:
    case TOKEN_Or:
    case TOKEN_Xor:
        status = GetOperandValues(leftResult, rightResult, context);
        if (ExpressionStatus::Success == status)
            status = Operations::PerformJunctionOperator(evalResult, _GetOperation(), leftResult, rightResult);
        break;

    case TOKEN_AndAlso:
    case TOKEN_OrElse:
        {
        // Short-circuit operators do not evaluate righthand expression unless required.
        status = GetLeftP()->GetValue (leftResult, context);

        // Treat error as false evaluation value
        bool leftBool = false;
        if (ExpressionStatus::Success != status || ExpressionStatus::Success != (status = leftResult.GetBoolean (leftBool, false)))
            leftBool = false;

        if (leftBool == (TOKEN_AndAlso == m_operatorCode))
            {
            // OrElse and lefthand expr is false, or AndAlso and righthand expr is true.
            status = GetRightP()->GetValue (rightResult, context);
            }

        if (ExpressionStatus::Success == status)
            {
            leftResult.InitECValue().SetBoolean (leftBool);
            status = (TOKEN_AndAlso == m_operatorCode) ? Operations::PerformLogicalAnd (evalResult, leftResult, rightResult) : Operations::PerformLogicalOr (evalResult, leftResult, rightResult);
            }
        }
        break;
    default:
        BeAssert(false && L"bad LogicalNode operator");
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus BinaryNode::GetOperandValues(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context)
    {
    ExpressionStatus    status = m_left->GetValue(leftResult, context);
    if (ExpressionStatus::Success != status)
        return status;

    return m_right->GetValue(rightResult, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus BinaryNode::PromoteCommon(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context, bool allowStrings)
    {
    if (!leftResult.IsECValue() || !rightResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: PrimitiveRequired (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::ECValueR    left    = *leftResult.GetECValue();
    ECN::ECValueR    right   = *rightResult.GetECValue();

    if (left.IsNull() || right.IsNull())
        {
        // make sure null value does not have a defined type - it's just 'null'
        if (left.IsNull())
            left.Clear();
        if (right.IsNull())
            right.Clear();

        return ExpressionStatus::Success;
        }
    else if (!left.IsPrimitive() || !right.IsPrimitive())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: PrimitiveRequired (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::PrimitiveRequired;
        }

    ECN::PrimitiveType   leftCode    = left.GetPrimitiveType();
    ECN::PrimitiveType   rightCode   = right.GetPrimitiveType();

    //  PRIMITIVETYPE_DateTime, point types, and Boolean are all missing from this
    //  We may also want to provide a way for structs with extended types to perform the conversion to 
    //  string

    if (leftCode == rightCode)
        {
        switch (leftCode)
            {
            case PRIMITIVETYPE_Boolean:
            case PRIMITIVETYPE_Double:
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_DateTime:
                return ExpressionStatus::Success;
            case PRIMITIVETYPE_String:
                return allowStrings ? ExpressionStatus::Success : ExpressionStatus::WrongType;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "BinaryNode::PromoteCommon: WrongType");
        return ExpressionStatus::WrongType;
        }

    if (PRIMITIVETYPE_String == leftCode || PRIMITIVETYPE_String == rightCode)
        {
        if (!allowStrings)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: IncompatibleTypes. Strings are not allowed (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
            return ExpressionStatus::IncompatibleTypes;
            }

        if (TOKEN_Plus != m_operatorCode)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: IncompatibleTypes. Expected '+' operator ('%s')", Lexer::GetString(m_operatorCode).c_str()).c_str());
            return ExpressionStatus::IncompatibleTypes;
            }

        if (leftCode == rightCode)
            return ExpressionStatus::Success;

        //  Convert to strings; may want to involve the extended type in that
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "BinaryNode::PromoteCommon: IncompatibleTypes");
        return ExpressionStatus::IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        {
        if (leftCode == rightCode)
            return ExpressionStatus::Success;

        ECValueR            target = PRIMITIVETYPE_Long == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Long == leftCode ? rightCode : leftCode;
        if (PRIMITIVETYPE_Double == targetCode)
            {
            target.SetLong((int64_t)target.GetDouble());
            return ExpressionStatus::Success;
            }

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetLong(target.GetInteger());
            return ExpressionStatus::Success;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: IncompatibleTypes. (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Double == leftCode || PRIMITIVETYPE_Double == rightCode)
        {
        //  Both must be doubles
        if (leftCode == rightCode)
            return ExpressionStatus::Success;

        ECValueR            target = PRIMITIVETYPE_Double == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Double == leftCode ? rightCode : leftCode;

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetDouble(target.GetInteger());
            return ExpressionStatus::Success;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("BinaryNode::PromoteCommon: IncompatibleTypes. (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Boolean == leftCode)
        {
        left.SetInteger(left.GetBoolean() ? 1 : 0);
        leftCode = PRIMITIVETYPE_Integer;
        }

    if (PRIMITIVETYPE_Boolean == rightCode)
        {
        right.SetInteger(right.GetBoolean() ? 1 : 0);
        rightCode = PRIMITIVETYPE_Integer;
        }

    if (PRIMITIVETYPE_Integer == leftCode && PRIMITIVETYPE_Integer == rightCode)
        return ExpressionStatus::Success;

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "BinaryNode::PromoteCommon: WrongType");
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ExponentNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExpressionStatus::Success != status)
        return status;

    return Operations::PerformExponentiation(evalResult, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MultiplyNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExpressionStatus::Success != status)
        return status;

    return Operations::PerformMultiplication(evalResult, left, right, context.EnforcesUnits());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DivideNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExpressionStatus::Success != status)
        return status;

    switch(m_operatorCode)
        {
        case TOKEN_IntegerDivide:
            return Operations::PerformIntegerDivision(evalResult, left, right, context.EnforcesUnits());
        case TOKEN_Slash:
            return Operations::PerformDivision(evalResult, left, right, context.EnforcesUnits());
        case TOKEN_Mod:
            return Operations::PerformMod(evalResult, left, right);
        }

    BeAssert (false && L"bad divide operator");
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("DivideNode::_GetValue: UnknownError. Bad divide operator (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PlusMinusNode::_Promote(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context)
    {
    ExpressionStatus status =  PromoteCommon(leftResult, rightResult, context, true);
    if (ExpressionStatus::Success != status)
        return status;
    else if (leftResult.GetECValue()->IsNull() || rightResult.GetECValue()->IsNull())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PlusMinusNode::_Promote: PrimitiveRequired (left: %s, right: %s)", leftResult.ToString().c_str(), rightResult.ToString().c_str()).c_str());
        return ExpressionStatus::PrimitiveRequired;
        }

    if (context.EnforcesUnits())
        {
        status = Operations::EnforceLikeUnits (leftResult, rightResult);
        if (ExpressionStatus::Success == status)
            {
            // primitive types may have changed if we did unit conversion...make sure they are back in sync
            status = PromoteCommon (leftResult, rightResult, context, false);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+-----*/
ExpressionStatus PlusMinusNode::_PerformOperation(EvaluationResultR evalResult, EvaluationResultCR leftResult, EvaluationResultCR rightResult, ExpressionContextR context)
    {
    if (!leftResult.IsECValue() || !rightResult.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "PlusMinusNode::_PerformOperation: WrongType (left result or right result is not ECValue)");
        return ExpressionStatus::WrongType;
        }

    ECN::ECValueCR   left    = *leftResult.GetECValue();
    ECN::ECValueCR   right   = *rightResult.GetECValue();
    ECN::ECValueR    result  = evalResult.InitECValue();

    if (m_operatorCode == TOKEN_Plus)
        {
        switch(left.GetPrimitiveType())
            {
            case PRIMITIVETYPE_String:
                performConcatenation (result, left, right);
                break;

            case PRIMITIVETYPE_Long:
                result.SetLong(left.GetLong() + right.GetLong());
                break;

            case PRIMITIVETYPE_Integer:
                result.SetInteger(left.GetInteger() + right.GetInteger());
                break;

            case PRIMITIVETYPE_Double:
                result.SetDouble(left.GetDouble() + right.GetDouble());
                break;
            default:
                BeAssert (false && L"unexpected types for addition");
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PlusMinusNode::_PerformOperation: UnknownError. Unexpected types for audition (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::UnknownError;
            }
        }
    else
        {
        switch(left.GetPrimitiveType())
            {
            case PRIMITIVETYPE_Long:
                result.SetLong(left.GetLong() - right.GetLong());
                break;

            case PRIMITIVETYPE_Integer:
                result.SetInteger(left.GetInteger() - right.GetInteger());
                break;

            case PRIMITIVETYPE_Double:
                result.SetDouble(left.GetDouble() - right.GetDouble());
                break;
            default:
                BeAssert (false && L"unexpected types for subtraction");
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PlusMinusNode::_PerformOperation: UnknownError. Unexpected types for substraction (left: %s, right: %s)", left.ToString().c_str(), right.ToString().c_str()).c_str());
                return ExpressionStatus::UnknownError;
            }
        }

    evalResult.SetUnits (leftResult.GetUnits());
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("PlusMinusNode::_PerformOperation: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bool     PerformCompare (T l, ExpressionToken op, T r)    
    {
    switch (op)
        {
        case TOKEN_Equal:      return l == r;
        case TOKEN_NotEqual:   return l != r;
        case TOKEN_Less:       return l <  r;
        case TOKEN_LessEqual:  return l <= r;
        case TOKEN_Greater:    return l >  r;
        case TOKEN_GreaterEqual: return l >= r;
        }

    //  Does not handle string equality or Like operator
    BeAssert (false);
    return false;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
template<> /*static*/ bool     PerformCompare<double> (double l, ExpressionToken op, double r) // Android compiler gives error "explicit template specialization cannot have a storage class" with the static keyword
    {
    bool equal = DoubleOps::AlmostEqual (l, r);
    switch (op)
        {
        case TOKEN_Equal:           return equal;
        case TOKEN_NotEqual:        return !equal;
        case TOKEN_Less:            return !equal && l < r;
        case TOKEN_Greater:         return !equal && l > r;
        case TOKEN_LessEqual:       return equal || l < r;
        case TOKEN_GreaterEqual:    return equal || l > r;
        default:                    BeAssert (false); return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ComparisonNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;

    ExpressionStatus status = GetOperandValues(leftResult, rightResult, context);
    
    if (ExpressionStatus::Success != status)
        return status;

    status = PromoteCommon(leftResult, rightResult, context, true);

    if (ExpressionStatus::Success != status)
        return status;

    if (TOKEN_Like == m_operatorCode)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ComparisonNode::_GetValue: NotImplemented (operator: %s)", Lexer::GetString(m_operatorCode).c_str()).c_str());
        return ExpressionStatus::NotImpl;
        }

    ECValueCR   ecLeft      = *leftResult.GetECValue();
    ECValueCR   ecRight     = *rightResult.GetECValue();
    if (ecLeft.IsNull() || ecRight.IsNull())
        {
        bool leftnull = ecLeft.IsNull(), rightnull = ecRight.IsNull();
        bool boolResult = false;
        switch (m_operatorCode)
            {
        case TOKEN_Equal:       boolResult = leftnull && rightnull; break;
        case TOKEN_NotEqual:    boolResult = leftnull != rightnull; break;
        default:                break;  // less/greater operators nonsensical for null values
            }

        //  Maybe the not's should be true for this
        evalResult.InitECValue().SetBoolean(boolResult);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    //   Promoted, so one string => both strings
    if (ecLeft.IsString())
        {
        Utf8CP          leftString   = ecLeft.GetUtf8CP();
        Utf8CP          rightString  = ecRight.GetUtf8CP();
        int             intResult = strcmp(leftString, rightString);
        bool            boolResult = false;
        
        switch (m_operatorCode)
            {
            case TOKEN_Equal:      boolResult = intResult == 0;    break;
            case TOKEN_NotEqual:   boolResult = intResult != 0;    break;
            case TOKEN_Less:       boolResult = intResult <  0;    break;
            case TOKEN_LessEqual:  boolResult = intResult <= 0;    break;
            case TOKEN_Greater:    boolResult = intResult >  0;    break;
            case TOKEN_GreaterEqual: boolResult = intResult >= 0;  break;
            }

        evalResult.InitECValue().SetBoolean(boolResult);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    if (context.EnforcesUnits())
        {
        ExpressionStatus status = ExpressionStatus::Success;
        switch (ecLeft.GetPrimitiveType())
            {
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Double:
                {
                status = Operations::EnforceLikeUnits (leftResult, rightResult);
                if (ExpressionStatus::Success == status)
                    {
                    // primitive types may have changed if we did unit conversion...make sure they are back in sync
                    status = PromoteCommon (leftResult, rightResult, context, false);
                    }
                }
                break;
            }
        
        if (ExpressionStatus::Success != status)
            return status;
        }
                
    switch (ecLeft.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            evalResult.InitECValue().SetBoolean(PerformCompare(ecLeft.GetBoolean(), m_operatorCode, ecRight.GetBoolean()));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
        case PRIMITIVETYPE_Double:
            evalResult.InitECValue().SetBoolean(PerformCompare(ecLeft.GetDouble(), m_operatorCode, ecRight.GetDouble()));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
        case PRIMITIVETYPE_Integer:
            evalResult.InitECValue().SetBoolean(PerformCompare(ecLeft.GetInteger(), m_operatorCode, ecRight.GetInteger()));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
        case PRIMITIVETYPE_Long:
            evalResult.InitECValue().SetBoolean(PerformCompare(ecLeft.GetLong(), m_operatorCode, ecRight.GetLong()));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
        case PRIMITIVETYPE_DateTime:
            evalResult.InitECValue().SetBoolean(PerformCompare(ecLeft.GetDateTimeTicks(), m_operatorCode, ecRight.GetDateTimeTicks()));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ComparisonNode::_GetValue: (%s %s %s) = %s", ecLeft.ToString().c_str(), Lexer::GetString(m_operatorCode).c_str(), ecRight.ToString().c_str(), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
        }
    
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ComparisonNode::_GetValue: WrongType");
    return ExpressionStatus::WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Node::GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    { 
    return _GetValue(evalResult, context); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Node::GetValue(ValueResultPtr& valueResult, ExpressionContextR context)
    {
    EvaluationResult    evalResult;

    ExpressionStatus    status = GetValue(evalResult, context);
    valueResult = ValueResult::Create(evalResult);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResultR EvaluationResult::operator= (ECN::ECValueCR rhs)
    {
    Clear();
    m_valueType = ValType_ECValue;
    m_ecValue = rhs;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ValueResult::GetECValue (ECN::ECValueR ecValue)
    {
    return m_evalResult.GetECValue(ecValue);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    02/2011 
+---------------+---------------+---------------+---------------+---------------+------*/
struct          ParseTreeTraverser : NodeVisitor
    {
private:
    Utf8String  m_expression;
    bool        m_lastRequiredSpace;

protected:
    virtual bool OpenParens() override
        {
        m_lastRequiredSpace = false;
        m_expression += "(";
        return true;
        }

    virtual bool CloseParens() override
        {
        m_lastRequiredSpace = false;
        m_expression += ")";
        return true;
        }

    virtual bool StartArrayIndex(NodeCR node) override
        {
        m_lastRequiredSpace = false;
        m_expression += "[";
        return true;
        }

    virtual bool EndArrayIndex(NodeCR node) override
        {
        m_lastRequiredSpace = false;
        m_expression += "]";
        return true;
        }

    virtual bool StartArguments(NodeCR node) override
        {
        m_lastRequiredSpace = false;
        m_expression += "(";
        return true;
        }

    virtual bool EndArguments(NodeCR node) override
        {
        m_lastRequiredSpace = false;
        m_expression += ")";
        return true;
        }

    virtual bool Comma() override
        {
        m_lastRequiredSpace = false;
        m_expression += ",";
        return true;
        }

    virtual bool ProcessNode(NodeCR node) override
        {
        Utf8String     curr = node.ToString();
        if (m_lastRequiredSpace && 
                (isalnum(curr[0]) || curr[0] == '_'))
            m_expression += " ";

        size_t  stringLen = curr.size();
        if (stringLen > 0)
            m_lastRequiredSpace = (isalnum(curr[stringLen - 1]) || curr[stringLen - 1] == '_');

        m_expression += curr;
        return true;
        }

    virtual bool ProcessUnits (UnitSpecCR units) override
        {
        m_expression += units.ToECExpressionString();
        return true;
        }

public:
    ParseTreeTraverser () : m_lastRequiredSpace(false) {}
    Utf8String Traverse (NodeCR node)
        {
        m_lastRequiredSpace = false;
        m_expression = "";
        node.Traverse(*this);
        return m_expression;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::ToExpressionString() const
    {
    ParseTreeTraverser traverser;
    return traverser.Traverse (*this);
    }

/*---------------------------------------------------------------------------------**//**
* A little state machine which remaps any schema/class/property names within the
* ECExpression which may have been renamed.
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionRemapper : NodeVisitor
    {
private:
    enum State
        {
        kState_AwaitingAccessor,        // default state - awaiting a DotNode
        kState_ProcessingAccessors,     // processing a series of dot nodes
        kState_AwaitingRightBracket     // ignoring everything within [brackets]. It's possible we'll miss dot nodes in here - but not in our current use cases.
        };

    ECSchemaCR          m_preSchema;
    ECSchemaCR          m_postSchema;
    IECSchemaRemapperCR m_remapper;

    ECClassCP           m_currentClass;
    State               m_state;
    bool                m_anyRemapped;
    bool                m_schemaRenamed;

    ExpressionRemapper (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper)
      : m_preSchema (pre), m_postSchema (post), m_remapper (remapper), m_currentClass (nullptr),
        m_state (kState_AwaitingAccessor), m_anyRemapped (false), m_schemaRenamed (!pre.GetName().Equals (post.GetName()))
    {
    //
    }

    virtual bool    OpenParens() override                       { return ProcessOther(); }
    virtual bool    CloseParens() override                      { return ProcessOther(); }
    virtual bool    StartArguments(NodeCR node) override        { return ProcessOther(); }
    virtual bool    EndArguments(NodeCR node) override          { return ProcessOther(); }
    virtual bool    Comma() override                            { return ProcessOther(); }
    virtual bool    ProcessUnits (UnitSpecCR units) override    { return ProcessOther(); }
    virtual bool    StartArrayIndex(NodeCR node) override       { return ProcessArrayIndex (false); }
    virtual bool    EndArrayIndex(NodeCR node) override         { return ProcessArrayIndex (true); }

    virtual bool    ProcessNode(NodeCR node) override;

    bool            ProcessDot (DotNodeCR node);
    bool            ProcessMethod (CallNodeCR node);
    bool            ProcessArrayIndex (bool isEnd);
    bool            ProcessOther();

    void            ProcessIsOfClass (CallNodeR node);
    void            ProcessFullyQualifiedAccessStringArguments (CallNodeR node);

    template<size_t N> bool ExtractArguments (Utf8StringP (&args)[N], CallNodeCR callNode);
    template<size_t N> void ReplaceArguments (Utf8StringP (&args)[N], CallNodeCR callNode);

    void            Await();
public:
    static bool         Remap (NodeR node, ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR schemaRemapper)
        {
        ExpressionRemapper exprRemapper (pre, post, schemaRemapper);
        node.Traverse (exprRemapper);
        return exprRemapper.m_anyRemapped;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionRemapper::ProcessArrayIndex (bool isEnd)
    {
    BeAssert ((!isEnd && kState_ProcessingAccessors == m_state) || (isEnd && kState_AwaitingRightBracket == m_state));
    m_state = isEnd ? kState_AwaitingAccessor : kState_AwaitingRightBracket;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpressionRemapper::Await()
    {
    m_currentClass = nullptr;
    m_state = kState_AwaitingAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionRemapper::ProcessOther()
    {
    if (kState_ProcessingAccessors == m_state)
        {
        // finished with the current chain of member accessors
        Await();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionRemapper::ProcessNode (NodeCR node)
    {
    // Ignore everything inside [brackets]
    if (kState_AwaitingRightBracket == m_state)
        return true;

    auto dot = dynamic_cast<DotNodeCP> (&node);
    if (nullptr != dot)
        return ProcessDot (*dot);

    auto call = dynamic_cast<CallNodeCP> (&node);
    return nullptr != call ? ProcessMethod (*call) : ProcessOther();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionRemapper::ProcessDot (DotNodeCR node)
    {
    DotNodeR dot = const_cast<DotNodeR> (node);

    bvector<Utf8String>& qualifiers = dot.GetQualifiers();
    if (2 == qualifiers.size())
        {
        // The qualifiers give us a schema + class name. We require these on the first DotNode in a sequence;
        // the ECClasses for subsequent unqualified accessors can be looked up within the preceding ECClass.
        m_currentClass = nullptr;
        Utf8StringR schemaName = qualifiers[0];
        if (schemaName.Equals (m_preSchema.GetName()))
            {
            schemaName = m_postSchema.GetName();
            if (m_schemaRenamed)
                m_anyRemapped = true;

            Utf8StringR className = qualifiers[1];
            if (m_remapper.ResolveClassName (className, m_postSchema))
                m_anyRemapped = true;

            m_currentClass = m_postSchema.GetClassCP (className.c_str());
            }
        }

    m_state = kState_ProcessingAccessors;

    if (nullptr == m_currentClass)
        return true;

    // Look up the property by access string. Note the access string within a DotNode will always refer to a single ECProperty, not a member of an embedded struct.
    Utf8String propName (dot.GetName());
    if (m_remapper.ResolvePropertyName (propName, *m_currentClass))
        {
        dot.SetName (propName.c_str());
        m_anyRemapped = true;
        }

    // This dot node may be followed by:
    //  - another dot node, accessing a member of a struct property, or
    //  - an array index, followed by another dot node, accessing a member of a struct array member, or
    //  - anything else, in which case this is probably a primitive property
    // Need to identify the struct ECClass for the first two cases

    ECPropertyCP prop = m_currentClass->GetPropertyP (propName.c_str());
    m_currentClass = nullptr;
    if (nullptr != prop)
        {
        StructArrayECPropertyCP arrayProp = nullptr;
        auto structProp = prop->GetAsStructProperty();
        if (nullptr != structProp)
            m_currentClass = &structProp->GetType();
        else if (nullptr != (arrayProp = prop->GetAsStructArrayProperty()))
            m_currentClass = &arrayProp->GetStructElementType();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* A bit hard-codey...there are a handful of functions which take schema/class/property names
* as arguments which may need to be remapped. This could be generalized, and made to
* handle arguments that are not string literals - but our current use cases do not require
* that.
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionRemapper::ProcessMethod (CallNodeCR node)
    {
    auto call = const_cast<CallNodeR>(node);
    if (0 == strcmp (call.GetMethodName(), "IsPropertyValueSet") || 0 == strcmp (call.GetMethodName(), "ResolveSymbology"))
        ProcessFullyQualifiedAccessStringArguments (call);
    else if (0 == strcmp (call.GetMethodName(), "IsOfClass"))
        ProcessIsOfClass (call);

    // We will process the arguments immediately rather than await the StartArguments()/EndArguments() callbacks...
    return ProcessOther();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<size_t N> bool ExpressionRemapper::ExtractArguments (Utf8StringP (&args)[N], CallNodeCR callNode)
    {
    ArgumentTreeNode const* argNodes = callNode.GetArguments();
    size_t nArgs = nullptr != argNodes ? argNodes->GetArgumentCount() : 0;
    if (nArgs != N)
        return false;

    for (size_t i = 0; i < N; i++)
        {
        NodeCP argNode = argNodes->GetArgument (i);
        auto literalNode = dynamic_cast<LiteralNode const*> (argNode);
        if (nullptr == literalNode || !literalNode->GetInternalValue().IsString())
            return false;

        *args[i] = literalNode->GetInternalValue().GetUtf8CP();
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<size_t N> void ExpressionRemapper::ReplaceArguments (Utf8StringP (&args)[N], CallNodeCR callNode)
    {
    NodePtrVector& argNodes = const_cast<ArgumentTreeNode*> (callNode.GetArguments())->GetArguments();
    for (size_t i = 0; i < N; i++)
        {
        auto literalNode = dynamic_cast<LiteralNode*> (argNodes[i].get());
        ECValue v (args[i]->c_str());
        literalNode->SetInternalValue (v);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpressionRemapper::ProcessIsOfClass (CallNodeR node)
    {
    Utf8String className, schemaName;
    Utf8StringP args[2] = { &className, &schemaName };
    if (!ExtractArguments (args, node) || !schemaName.Equals (m_preSchema.GetName()))
        return;

    bool remapped = m_schemaRenamed;
    if (m_schemaRenamed)
        schemaName = m_postSchema.GetName();

    if (m_remapper.ResolveClassName (className, m_postSchema))
        remapped = true;

    if (remapped)
        {
        ReplaceArguments (args, node);
        m_anyRemapped = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpressionRemapper::ProcessFullyQualifiedAccessStringArguments (CallNodeR node)
    {
    QualifiedECAccessor accessor;
    Utf8StringP args[3] = { &accessor.GetSchemaNameR(), &accessor.GetClassNameR(), &accessor.GetAccessStringR() };
    if (ExtractArguments (args, node) && accessor.Remap (m_preSchema, m_postSchema, m_remapper))
        {
        ReplaceArguments (args, node);
        m_anyRemapped = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::Remap (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR schemaRemapper)
    {
    return ExpressionRemapper::Remap (*this, pre, post, schemaRemapper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Aidas.Vaiksnoras   04/25
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ErrorNode::_ToString() const
    {
    Utf8String msg = "ERROR (";
    msg.append(m_summary);
    if (!m_detail1.empty())
        {
        msg.append(", ");
        msg.append(m_detail1);
        if (!m_detail2.empty())
            {
            msg.append(", ");
            msg.append(m_detail2);
            }
        }
    msg.append(")");
    return msg;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ResolvedTypeNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context)
    {
    ExpressionStatus    status = ExpressionStatus::Success;;
    switch(m_primitiveType)
        {
        case PRIMITIVETYPE_Boolean:
            evalResult.InitECValue().SetBoolean(_GetBooleanValue(status, context));
            return status;
        case PRIMITIVETYPE_DateTime:
            //  evalResult.InitECValue().SetDateTime(_GetDateTimeValue(status, context));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedTypeNode::_GetValue: Method SetDateTime() is not implemented");
            return ExpressionStatus::NotImpl;
        case PRIMITIVETYPE_Double:
            evalResult.InitECValue().SetDouble(_GetDoubleValue(status, context));
            return status;
        case PRIMITIVETYPE_Integer:
            evalResult.InitECValue().SetInteger(_GetIntegerValue(status, context));
            return status;
        case PRIMITIVETYPE_Long:
            evalResult.InitECValue().SetLong(_GetLongValue(status, context));
            return status;
        case PRIMITIVETYPE_String:
            {
            ECValue  result;
            status = _GetStringValue(result, context);
            if (!result.IsUtf8())
                evalResult.InitECValue().SetWCharCP(result.GetWCharCP(), true);
            else
                evalResult.InitECValue().SetUtf8CP(result.GetUtf8CP(), true);
            }
            return status;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedTypeNode::_GetValue: IncompatibleTypes");
    return ExpressionStatus::IncompatibleTypes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ResolvedConcatenateNode::_GetStringValue(ECValueR result, ExpressionContextR context)
    {
    ECValue  left;
    ExpressionStatus status = m_left->_GetStringValue(left, context);
    if (ExpressionStatus::Success != status)
        {
        result.SetUtf8CP("", true);
        return status;
        }

    ECValue  right;
    status = m_right->_GetStringValue(right, context);
    if (ExpressionStatus::Success != status)
        {
        result.SetUtf8CP("", true);
        return status;
        }

    performConcatenation(result, left, right);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareIntegerNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int32_t leftValue = m_left->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    ::int32_t rightValue = m_right->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, rightValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareLongNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int64_t leftValue = m_left->_GetLongValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    ::int64_t rightValue = m_right->_GetLongValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, rightValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareDoubleNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    double leftValue = m_left->_GetDoubleValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    double rightValue = m_right->_GetDoubleValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, rightValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareBooleanNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    bool leftValue = m_left->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    bool rightValue = m_right->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, rightValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareIntegerToConstantNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int32_t leftValue = m_left->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, m_right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareLongToConstantNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int64_t leftValue = m_left->_GetLongValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, m_right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareDoubleToConstantNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    double leftValue = m_left->_GetDoubleValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, m_right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareBooleanToConstantNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    bool leftValue = m_left->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        return false;

    return PerformCompare(leftValue, m_operatorCode, m_right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedCompareStringNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ECValue   ecLeft;
    ECValue   ecRight;

    status = m_left->_GetStringValue(ecLeft, context);
    if (ExpressionStatus::Success != status)
        return false;

    status = m_right->_GetStringValue(ecLeft, context);
    if (ExpressionStatus::Success != status)
        return false;

    int             intResult = strcmp(ecLeft.GetUtf8CP(), ecRight.GetUtf8CP());
    bool            boolResult = false;
        
    switch (m_operatorCode)
        {
        case TOKEN_Equal:      boolResult = intResult == 0;    break;
        case TOKEN_NotEqual:   boolResult = intResult != 0;    break;
        case TOKEN_Less:       boolResult = intResult <  0;    break;
        case TOKEN_LessEqual:  boolResult = intResult <= 0;    break;
        case TOKEN_Greater:    boolResult = intResult >  0;    break;
        case TOKEN_GreaterEqual: boolResult = intResult >= 0;    break;
        }
    return boolResult;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ExpressionResolver::PromoteToType(ResolvedTypeNodePtr& node, ECN::PrimitiveType  targetType)
    {
    ECN::PrimitiveType  sourceType = node->GetResolvedPrimitiveType();
    if (sourceType == targetType)
        return ExpressionStatus::Success;

    switch (targetType)
        {
        case PRIMITIVETYPE_Boolean:
            {
            if (node->_SupportsGetBooleanValue())
                return ExpressionStatus::Success;

            if (PRIMITIVETYPE_Integer != sourceType && PRIMITIVETYPE_Long != sourceType && PRIMITIVETYPE_Double != sourceType)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ExpressionResolver::PromoteToType: IncompatibleTypes (can only convert integer, long or double to bool): %s", node->ToExpressionString().c_str()).c_str());
                return ExpressionStatus::IncompatibleTypes;
                }

            if (sourceType == PRIMITIVETYPE_Integer)
                {
                node = ResolvedConvertIntegerToBoolean::Create(*node);
                return ExpressionStatus::Success;
                }

            if (sourceType == PRIMITIVETYPE_Long)
                {
                node = ResolvedConvertLongToBoolean::Create(*node);
                return ExpressionStatus::Success;
                }

            if (sourceType == PRIMITIVETYPE_Double)
                {
                node = ResolvedConvertDoubleToBoolean::Create(*node);
                return ExpressionStatus::Success;
                }
            }
            break;

        case PRIMITIVETYPE_Integer:
            BeAssert(false && L"asked to promote to integer");
            break;  //  Doesn't make sense. PRIMITIVETYPE_Integer is the smallest

        case PRIMITIVETYPE_Long:
            BeAssert(sourceType == PRIMITIVETYPE_Integer);
            if (node->_SupportsGetLongValue())
                return ExpressionStatus::Success;

            node = ResolvedConvertIntegerToLong::Create(*node);
            return ExpressionStatus::Success;

        case PRIMITIVETYPE_Double:
            if (node->_SupportsGetDoubleValue())
                return ExpressionStatus::Success;
            if (PRIMITIVETYPE_Integer == sourceType)
                node = ResolvedConvertIntegerToDouble::Create(*node);
            else
                {
                BeAssert(PRIMITIVETYPE_Long == sourceType);
                node = ResolvedConvertLongToDouble::Create(*node);
                }
            return ExpressionStatus::Success;
        }

    BeAssert (targetType == PRIMITIVETYPE_Long);  //  or PRIMITIVETYPE_Double
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ExpressionResolver::PromoteToType: IncompatibleTypes (target type not supported)");
    return ExpressionStatus::IncompatibleTypes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ExpressionResolver::PromoteToString(ResolvedTypeNodePtr& node)
    {
    ECN::PrimitiveType  sourceType = node->GetResolvedPrimitiveType();
    if (sourceType == PRIMITIVETYPE_String)
        return ExpressionStatus::Success;

    if (node->IsConstant())
        {
        Utf8String     stringValue = node->ToString();
        node = Node::CreateStringLiteral(stringValue.c_str(), false);
        return ExpressionStatus::Success;
        }

    switch(sourceType)
        {
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
        case PRIMITIVETYPE_Double:
            node = ResolvedConvertToString::Create(*node);
            return ExpressionStatus::Success;
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ExpressionResolver::PromoteToString: Expression %s not convertible to string", node->ToExpressionString().c_str()).c_str());
    return ExpressionStatus::IncompatibleTypes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ExpressionResolver::PerformArithmeticPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right)
    {
    //  Check this here instead of making every caller verify that it successfully resolved both types.
    if (!left.IsValid() || !right.IsValid())
        {
        targetType = PRIMITIVETYPE_Binary;
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ExpressionResolver::PerformArithmeticPromotion: WrongTypes (left node or right node is invalid)");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType   leftCode    = left->GetResolvedPrimitiveType();
    ECN::PrimitiveType   rightCode   = right->GetResolvedPrimitiveType();
    targetType = leftCode;

    if (leftCode == rightCode)
        return ExpressionStatus::Success;

    if (PRIMITIVETYPE_Double == leftCode || PRIMITIVETYPE_Double == rightCode)
        targetType = PRIMITIVETYPE_Double;
    else if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        targetType = PRIMITIVETYPE_Long;
    else if (PRIMITIVETYPE_Integer != leftCode && PRIMITIVETYPE_Integer != rightCode)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ExpressionResolver::PerformArithmeticPromotion: IncompatibleTypes (left: %s, right: %s)", left->ToExpressionString().c_str(), right->ToExpressionString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    //  This calls one of the methods like _SupportsGetDoubleValue.  If the node returns true, then PromoteToType does
    //  not create a new node. This results in a node that is used to get a value of a type that is different than the
    //  nodes primary primitive type.
    if (ExpressionStatus::Success != PromoteToType(left, targetType) || ExpressionStatus::Success != PromoteToType(right, targetType))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ExpressionResolver::PerformArithmeticPromotion: IncompatibleTypes (left: %s, right: %s)", left->ToExpressionString().c_str(), right->ToExpressionString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    return ExpressionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ExpressionResolver::PerformJunctionPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right)
    {
    if (!left.IsValid() || !right.IsValid())
        {
        targetType = PRIMITIVETYPE_Binary;
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ExpressionResolver::PerformJunctionPromotion: WrongTypes (left node or right node is invalid)");
        return ExpressionStatus::WrongType;
        }

    ECN::PrimitiveType   leftCode    = left->GetResolvedPrimitiveType();
    ECN::PrimitiveType   rightCode   = right->GetResolvedPrimitiveType();
    targetType = leftCode;

    if (leftCode == rightCode)
        return ExpressionStatus::Success;

    if (PRIMITIVETYPE_Boolean == leftCode || PRIMITIVETYPE_Boolean == rightCode)
        targetType = PRIMITIVETYPE_Boolean;
    else if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        targetType = PRIMITIVETYPE_Long;
    else if (PRIMITIVETYPE_Integer != leftCode && PRIMITIVETYPE_Integer != rightCode)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ExpressionResolver::PerformJunctionPromotion: IncompatibleTypes (left: %s, right: %s)", left->ToExpressionString().c_str(), right->ToExpressionString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    if (ExpressionStatus::Success != PromoteToType(left, targetType) || ExpressionStatus::Success != PromoteToType(left, targetType))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ExpressionResolver::PerformJunctionPromotion: IncompatibleTypes (left: %s, right: %s)", left->ToExpressionString().c_str(), right->ToExpressionString().c_str()).c_str());
        return ExpressionStatus::IncompatibleTypes;
        }

    return ExpressionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
static bool checkConstants(ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right, bool allowReorder)
    {
    if (right->IsConstant())
        return true;
    if (!left->IsConstant())
        return false;

    if (!allowReorder)
        return true;    //  Resolver logic for commutative operands assumes constant is on right.

    ResolvedTypeNodePtr temp = right;
    right = left;
    left = temp;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolvePrimaryList (PrimaryListNodeR primaryList) { return NULL; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveUnaryArithmeticNode (UnaryArithmeticNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    switch(node.GetOperation())
        {
        case TOKEN_Plus:
            return left;

        case TOKEN_Minus:
            {
            ExpressionStatus status;
            if (left->IsConstant())
                {
                switch(left->GetResolvedPrimitiveType())
                    {
                    case PRIMITIVETYPE_Integer:
                        return Node::CreateIntegerLiteral(-left->_GetIntegerValue(status, *m_context));

                    case PRIMITIVETYPE_Long:
                        return Node::CreateInt64Literal(-left->_GetLongValue(status, *m_context));

                    case PRIMITIVETYPE_Double:
                        return Node::CreateFloatLiteral(-left->_GetDoubleValue(status, *m_context));
                    }
                }
            }

            return ResolvedUnaryMinusNode::Create(left->GetResolvedPrimitiveType(), *left);

        case TOKEN_Not:
            {
            ExpressionStatus status;
            if (left->IsConstant())
                {
                switch(left->GetResolvedPrimitiveType())
                    {
                    case PRIMITIVETYPE_Integer:
                        return Node::CreateIntegerLiteral(~left->_GetIntegerValue(status, *m_context));

                    case PRIMITIVETYPE_Long:
                        return Node::CreateInt64Literal(~left->_GetLongValue(status, *m_context));

                    case PRIMITIVETYPE_Boolean:
                        return Node::CreateBooleanLiteral(!left->_GetBooleanValue(status, *m_context));
                    }
                }
            }

            return ResolvedUnaryNotNode::Create(left->GetResolvedPrimitiveType(), *left);
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveMultiplyNode (MultiplyNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);

    //  Returns an error if unsupported types or either operand was not resolved.
    ECN::PrimitiveType  resultType;
    if (ExpressionStatus::Success != ExpressionResolver::PerformArithmeticPromotion(resultType, left, right))
        return NULL;

    if (checkConstants(left, right, true))
        {
        BeAssert(right->IsConstant());
        //  If just operand is constant, checkConstants moves it to the right side.  Therefore, if 
        //  left side is constant then we know that both side are constant.
        if (left->IsConstant())
            {
            ExpressionStatus    status = ExpressionStatus::Success;
            switch(resultType)
                {
                case PRIMITIVETYPE_Integer:
                    return Node::CreateIntegerLiteral(left->_GetIntegerValue(status, GetExpressionContextR()) * right->_GetIntegerValue(status, GetExpressionContextR()));
                case PRIMITIVETYPE_Long:
                    return Node::CreateInt64Literal(left->_GetLongValue(status, GetExpressionContextR()) * right->_GetLongValue(status, GetExpressionContextR()));
                case PRIMITIVETYPE_Double:
                    return Node::CreateFloatLiteral(left->_GetDoubleValue(status, GetExpressionContextR()) * right->_GetDoubleValue(status, GetExpressionContextR()));
                }
            }

        ExpressionStatus    status = ExpressionStatus::Success;
        switch(resultType)
            {
            case PRIMITIVETYPE_Integer:
                return ResolvedMultiplyConstantNode::CreateInteger(*left, right->_GetIntegerValue(status, GetExpressionContextR()));
            case PRIMITIVETYPE_Long:
                return ResolvedMultiplyConstantNode::CreateInt64(*left, right->_GetLongValue(status, GetExpressionContextR()));
            case PRIMITIVETYPE_Double:
                return ResolvedMultiplyConstantNode::CreateDouble(*left, right->_GetDoubleValue(status, GetExpressionContextR()));
            }
        BeAssert(false && L"multiplication constant handling failed to generate nodes");
        }

    return ResolvedMultiplyNode::Create(resultType, *left, *right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolvePlusMinusNode (PlusMinusNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);

    ECN::PrimitiveType  resultType;
    if (ExpressionStatus::Success != ExpressionResolver::PerformArithmeticPromotion(resultType, left, right))
        return NULL;

    ExpressionStatus status = ExpressionStatus::Success;
    ExpressionContextR  expContext = GetExpressionContextR();
    if (node.GetOperation() == TOKEN_Plus)
        {
        if (checkConstants(left, right, true))
            {
            if (left->IsConstant())
                {
                switch(resultType)
                    {
                    case PRIMITIVETYPE_Integer:
                        return Node::CreateIntegerLiteral(left->_GetIntegerValue(status, expContext) + right->_GetIntegerValue(status, expContext));
                    case PRIMITIVETYPE_Long:
                        return Node::CreateInt64Literal(left->_GetLongValue(status, expContext) + right->_GetLongValue(status, expContext));
                    case PRIMITIVETYPE_Double:
                        return Node::CreateFloatLiteral(left->_GetDoubleValue(status, expContext) + right->_GetDoubleValue(status, expContext));
                    }
                }

            EvaluationResult    evalResult;
            right->GetValue(evalResult, expContext);
            return ResolvedAddConstantNode::Create(resultType, *left, *evalResult.GetECValue());
            }

        return ResolvedAddNode::Create(resultType, *left, *right);
        }

    if (right->IsConstant())
        {
        if (left->IsConstant())
            {
            switch(resultType)
                {
                case PRIMITIVETYPE_Integer:
                    return Node::CreateIntegerLiteral(left->_GetIntegerValue(status, expContext) - right->_GetIntegerValue(status, expContext));
                case PRIMITIVETYPE_Long:
                    return Node::CreateInt64Literal(left->_GetLongValue(status, expContext) - right->_GetLongValue(status, expContext));
                case PRIMITIVETYPE_Double:
                    return Node::CreateFloatLiteral(left->_GetDoubleValue(status, expContext) - right->_GetDoubleValue(status, expContext));
                }
            }

        EvaluationResult    evalResult;
        right->GetValue(evalResult, expContext);
        ECValueR  ecValue = *evalResult.GetECValue();
        switch(resultType)
            {
            case PRIMITIVETYPE_Integer:
                ecValue.SetInteger(-1 * ecValue.GetInteger());
                break;
            case PRIMITIVETYPE_Long:
                ecValue.SetLong(-1 * ecValue.GetLong());
                break;
            case PRIMITIVETYPE_Double:
                ecValue.SetDouble(-1 * ecValue.GetDouble());
                break;
            default:
                BeAssert(false && L"encountered invalid primitive when creating constant subtract node");
                return NULL;
            }

        return ResolvedAddConstantNode::Create(resultType, *left, *evalResult.GetECValue());
        }

    BeAssert(node.GetOperation() == TOKEN_Minus);
    return ResolvedSubtractNode::Create(resultType, *left, *right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveConcatenateNode (ConcatenateNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);
    if (!right.IsValid())
        return NULL;

    if (ExpressionStatus::Success != PromoteToString(left) || ExpressionStatus::Success != PromoteToString(right))
        return NULL;

    if (checkConstants(left, right, false))
        {
        //  If both constant then do the work here. If just one constant then may want to create a 
        //  new node type to hold the constant value along with a pointer to the left operand.
        //  If either value is one just return the left node
        }

    return ResolvedConcatenateNode::Create(*left, *right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveShiftNode (ShiftNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);
    if (!right.IsValid())
        return NULL;

    if (ExpressionStatus::Success != PromoteToType(right, PRIMITIVETYPE_Integer))
        return NULL;

    switch (left->GetResolvedPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            return ResolvedShiftInteger::Create(node.GetOperation(), *left, *right);
        case PRIMITIVETYPE_Long:
            return ResolvedShiftLong::Create(node.GetOperation(), *left, *right);
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveIIfNode (IIfNodeCR node)
    {
    ResolvedTypeNodePtr condition = node.GetConditionP()->GetResolvedTree(*this);
    if (!condition.IsValid())
        return NULL;

    ResolvedTypeNodePtr trueNode = node.GetTrueP()->GetResolvedTree(*this);
    if (!trueNode.IsValid())
        return NULL;

    ResolvedTypeNodePtr falseNode = node.GetFalseP()->GetResolvedTree(*this);
    if (!falseNode.IsValid())
        return NULL;

    //  Promote arguments.  If either is string, do string - else if either is bool do bool else do arithmetic
    ECN::PrimitiveType  resultType;
    if (ExpressionStatus::Success != PerformArithmeticPromotion(resultType, trueNode, falseNode))
        return NULL;

    if (ExpressionStatus::Success != PromoteToType(condition, PRIMITIVETYPE_Boolean))
        return NULL;

    return ResolvedIIfNode::Create(resultType, *condition, *trueNode, *falseNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveComparisonNode (ComparisonNodeCR node)
    {
    ExpressionToken operation = node.GetOperation();
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);
    if (!right.IsValid())
        return NULL;

    if (left->IsConstant() && !right->IsConstant())
        {
        ResolvedTypeNodePtr temp = left;
        left = right;
        right = temp;
        switch(operation)
            {
            case TOKEN_Less:
                operation = TOKEN_Greater;
                break;
            case TOKEN_LessEqual:
                operation = TOKEN_GreaterEqual;
                break;
            case TOKEN_Greater:
                operation = TOKEN_Less;
                break;
            case TOKEN_GreaterEqual:
                operation = TOKEN_LessEqual;
                break;
            }
        }

    ExpressionStatus    status = ExpressionStatus::Success;
    if (left->GetResolvedPrimitiveType() == PRIMITIVETYPE_Boolean || right->GetResolvedPrimitiveType() == PRIMITIVETYPE_Boolean)
        {
        if (ExpressionStatus::Success != PromoteToType(left, PRIMITIVETYPE_Boolean) || ExpressionStatus::Success != PromoteToType(right, PRIMITIVETYPE_Boolean))
            return NULL;

        if (right->IsConstant())
            {
            bool  rightValue = right->_GetBooleanValue(status, GetExpressionContextR());
            if (left->IsConstant())
                {
                bool leftValue = left->_GetBooleanValue(status, GetExpressionContextR());
                return Node::CreateBooleanLiteral(PerformCompare(leftValue, operation, rightValue));
                }
            if (rightValue)
                return left;
            return ResolvedCompareBooleanToConstantNode::Create(operation, *left, rightValue);
            }

        return ResolvedCompareBooleanNode::Create(operation, *left, *right);
        }

    if (left->GetResolvedPrimitiveType() != PRIMITIVETYPE_String && right->GetResolvedPrimitiveType() != PRIMITIVETYPE_String)
        {
        ECN::PrimitiveType  resultType;
        ExpressionResolver::PerformArithmeticPromotion(resultType, left, right);
        if (right->IsConstant())
            {
            if (left->IsConstant())
                {
                bool computed = false;
                bool result = false;
                switch (resultType)
                    {
                    case PRIMITIVETYPE_Integer:
                        result = PerformCompare(left->_GetIntegerValue(status, GetExpressionContextR()), operation, right->_GetIntegerValue(status, GetExpressionContextR()));
                        computed = true;
                        break;
                    case PRIMITIVETYPE_Long:
                        result = PerformCompare(left->_GetLongValue(status, GetExpressionContextR()), operation, right->_GetLongValue(status, GetExpressionContextR()));
                        computed = true;
                        break;
                    case PRIMITIVETYPE_Double:
                        result = PerformCompare(left->_GetDoubleValue(status, GetExpressionContextR()), operation, right->_GetDoubleValue(status, GetExpressionContextR()));
                        computed = true;
                        break;
                    }

                if (computed)
                    return Node::CreateBooleanLiteral(result);
                }

            //  Only the right operand is constant
            switch(resultType)
                {
                case PRIMITIVETYPE_Integer:
                    return ResolvedCompareIntegerToConstantNode::Create(operation, *left, right->_GetIntegerValue(status, GetExpressionContextR()));
                case PRIMITIVETYPE_Long:
                    return ResolvedCompareLongToConstantNode::Create(operation, *left, right->_GetLongValue(status, GetExpressionContextR()));
                case PRIMITIVETYPE_Double:
                    return ResolvedCompareDoubleToConstantNode::Create(operation, *left, right->_GetDoubleValue(status, GetExpressionContextR()));
                }
            }

        switch(resultType)
            {
            case PRIMITIVETYPE_Integer:
                return ResolvedCompareIntegerNode::Create(operation, *left, *right);
            case PRIMITIVETYPE_Long:
                return ResolvedCompareLongNode::Create(operation, *left, *right);
            case PRIMITIVETYPE_Double:
                return ResolvedCompareDoubleNode::Create(operation, *left, *right);
            }
        }

    BeAssert(false && "_ResolveComparisonNode not implemented for this type");
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveLogicalNode (LogicalNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);
    if (!right.IsValid())
        return NULL;

    switch(node.GetOperation())
        {
        case TOKEN_And:
        case TOKEN_Or:
        case TOKEN_Xor:
            {
            //  Process bitwise operators.  If we knew the desired result was boolean we could 
            //  turn these into short circuiting operators that generate a boolean result
            ECN::PrimitiveType      targetType;
            ExpressionResolver::PerformJunctionPromotion(targetType, left, right);
            switch(targetType)
                {
                case PRIMITIVETYPE_Boolean:
                case PRIMITIVETYPE_Integer:
                case PRIMITIVETYPE_Long:
                    return ResolvedLogicalBitNode::Create(targetType, node.GetOperation(), *left, *right);
                }
            }
            break;

        case TOKEN_AndAlso:
        case TOKEN_OrElse:
            //  Force everything to Boolean.  Then use ResolvedLogicalBitNode::Create
            break;
        }

    BeAssert (false && L"_ResolveLogicalNode is incomplete");
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ResolvedTypeNodePtr ExpressionResolver::_ResolveDivideNode (DivideNodeCR node)
    {
    ResolvedTypeNodePtr left = node.GetLeftP()->GetResolvedTree(*this);
    if (!left.IsValid())
        return NULL;

    ResolvedTypeNodePtr right = node.GetRightP()->GetResolvedTree(*this);
    if (!right.IsValid())
        return NULL;

    if (node.GetOperation() == TOKEN_Mod)
        {
        ECN::PrimitiveType  resultType;
        if (ExpressionStatus::Success != PerformArithmeticPromotion(resultType, left, right))
            return NULL;
        if (resultType != PRIMITIVETYPE_Integer && resultType != PRIMITIVETYPE_Long)
            return NULL;
        return ResolvedModNode::Create(resultType, *left, *right);
        }

    if (node.GetOperation() == TOKEN_IntegerDivide)
        {
        ECN::PrimitiveType  resultType;
        if (ExpressionStatus::Success != PerformArithmeticPromotion(resultType, left, right))
            return NULL;
        switch(resultType)
            {
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Double:
                return ResolvedIntegerDivideNode::Create(resultType, *left, *right);
            default:
                return NULL;
            }
        }

    if (ExpressionStatus::Success != PromoteToType(left, PRIMITIVETYPE_Double) || ExpressionStatus::Success != PromoteToType(right, PRIMITIVETYPE_Double))
        return NULL;

    if (left->IsConstant())
        {
        ExpressionStatus status = ExpressionStatus::Success;
        double leftValue = left->_GetDoubleValue(status, GetExpressionContextR());
        if (right->IsConstant())
            {
            double divisor = right->_GetDoubleValue(status, GetExpressionContextR());
            if (0.0 == divisor)
                return NULL;
            double result = leftValue/divisor;
            if (result == ::int32_t(result))
                return Node::CreateIntegerLiteral(int32_t(result));
            return Node::CreateFloatLiteral(result);
            }
        return ResolvedDivideConstantNode::Create(leftValue, *right);
        }
    else if (right->IsConstant())
        {
        ExpressionStatus status = ExpressionStatus::Success;
        double divisor = right->_GetDoubleValue(status, GetExpressionContextR());
        if (0.0 == divisor)
            return NULL;
        return ResolvedDivideByConstantNode::Create(*left, divisor);
        }

    if (checkConstants(left, right, false))
        {
        //  If both constant then do the work here. If just one constant then may want to create a 
        //  new node type to hold the constant value along with a pointer to the left operand.
        //  If either value is one just return the left node
        }

    return ResolvedDivideNode::Create(*left, *right);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
::int32_t ResolvedShiftInteger::_GetIntegerValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int32_t left = m_left->_GetIntegerValue(status, context);
    ::int32_t right = m_right->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return 0;

    switch(m_operatorCode)
        {
        case TOKEN_ShiftLeft:
            return left << right;
        case TOKEN_ShiftRight:
            return left >> right;
        case TOKEN_UnsignedShiftRight:
            return (int32_t)((uint32_t)left >> right);
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedShiftInteger::_GetIntegerValue: bad shift operator");
    BeAssert(false && L"bad shift operator");
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
::int64_t ResolvedShiftLong::_GetLongValue(ExpressionStatus& status, ExpressionContextR context)
    {
    ::int64_t left = m_left->_GetLongValue(status, context);
    ::int32_t right = m_right->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return 0;

    switch(m_operatorCode)
        {
        case TOKEN_ShiftLeft:
            return left << right;
        case TOKEN_ShiftRight:
            return left >> right;
        case TOKEN_UnsignedShiftRight:
            return (int64_t)((uint64_t)left >> right);
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedShiftLong::_GetLongValue: bad shift operator");
    BeAssert(false && L"bad shift operator");
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
::int32_t ResolvedLogicalBitNode::_GetIntegerValue(ExpressionStatus& status, ExpressionContextR context)
    {
    //  Assume that on success status is not set but on error it is so it there is no need to check
    //  status between calls to _GetIntegerValue.
    ::int32_t left = m_left->_GetIntegerValue(status, context);
    ::int32_t right = m_right->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return 0;

    switch(m_operatorCode)
        {
        case TOKEN_Or:
            return left | right;

        case TOKEN_And:
            return left & right;

        case TOKEN_Xor:
            return left ^ right;
        }

    status = ExpressionStatus::NotImpl;
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedLogicalBitNode::_GetIntegerValue: NotImplemented");
    BeAssert (status != ExpressionStatus::NotImpl);
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
::int64_t ResolvedLogicalBitNode::_GetLongValue(ExpressionStatus& status, ExpressionContextR context)
    {
    //  Assume that on success status is not set but on error it is so it there is no need to check
    //  status between calls to _GetIntegerValue.
    ::int64_t left = m_left->_GetIntegerValue(status, context);
    ::int64_t right = m_right->_GetIntegerValue(status, context);
    if (ExpressionStatus::Success != status)
        return 0;

    switch(m_operatorCode)
        {
        case TOKEN_Or:
            return left | right;

        case TOKEN_And:
            return left & right;

        case TOKEN_Xor:
            return left ^ right;
        }

    status = ExpressionStatus::NotImpl;
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedLogicalBitNode::_GetLongValue: NotImplemented");
    BeAssert (status != ExpressionStatus::NotImpl);
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
bool ResolvedLogicalBitNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    {
    //  Assume that on success status is not set but on error it is so it there is no need to check
    //  status between calls to _GetIntegerValue.
    bool left = m_left->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        {
        BeAssert(!left);
        return false;
        }

    if (left && TOKEN_Or == m_operatorCode)
        return true;   //  maybe we need to evaluate right to check for error
    else if (!left && TOKEN_And == m_operatorCode)
        return false;

    bool right = m_right->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        {
        BeAssert(!right);
        return false;
        }

    switch(m_operatorCode)
        {
        case TOKEN_Or:
            return left | right;

        case TOKEN_And:
            return left & right;

        case TOKEN_Xor:
            return left ^ right;
        }

    status = ExpressionStatus::NotImpl;
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedLogicalBitNode::_GetBooleanValue: NotImplemented");
    BeAssert (status != ExpressionStatus::NotImpl);
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
::int32_t ResolvedIIfNode::_GetIntegerValue(ExpressionStatus& status, ExpressionContextR context)
    { return m_condition->_GetBooleanValue(status, context) ? m_true->_GetIntegerValue(status, context) : m_false->_GetIntegerValue(status, context); }

::int64_t ResolvedIIfNode::_GetLongValue(ExpressionStatus& status, ExpressionContextR context)
    { return m_condition->_GetBooleanValue(status, context) ? m_true->_GetLongValue(status, context) : m_false->_GetLongValue(status, context); }

double ResolvedIIfNode::_GetDoubleValue(ExpressionStatus& status, ExpressionContextR context)
    { return m_condition->_GetBooleanValue(status, context) ? m_true->_GetDoubleValue(status, context) : m_false->_GetDoubleValue(status, context); }

bool ResolvedIIfNode::_GetBooleanValue(ExpressionStatus& status, ExpressionContextR context)
    { return m_condition->_GetBooleanValue(status, context) ? m_true->_GetBooleanValue(status, context) : m_false->_GetBooleanValue(status, context); }

ExpressionStatus ResolvedIIfNode::_GetStringValue(ECValueR result, ExpressionContextR context)
    { 
    ExpressionStatus status;
    bool condition = m_condition->_GetBooleanValue(status, context);
    if (ExpressionStatus::Success != status)
        {
        result.SetUtf8CP("");
        return status;
        }

    return  condition ? m_true->_GetStringValue(result, context) : m_false->_GetStringValue(result, context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
    ResolvedAddConstantNode::ResolvedAddConstantNode(ECN::PrimitiveType resultType, ResolvedTypeNodeR left, ECValueCR right) 
            : ResolvedTypeNode(resultType), m_left(&left) 
    {
    switch(resultType)
        {
        case PRIMITIVETYPE_Integer:
            m_right.m_i = right.GetInteger();
            break;
        case PRIMITIVETYPE_Long:
            m_right.m_i64 = right.GetLong();
            break;
        case PRIMITIVETYPE_Double:
            m_right.m_d = right.GetDouble();
            break;
        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedAddConstantNode::ResolvedAddConstantNode: adding unknown constant type");
            BeAssert(false && L"adding unknown constant type");
            m_right.m_i64 = 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
ExpressionStatus ResolvedConvertToString::_GetStringValue(ECValueR result, ExpressionContextR context)
    {
    ExpressionStatus status = ExpressionStatus::Success;

    switch(m_left->GetResolvedPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            ::int32_t value = m_left->_GetIntegerValue(status, context);
            result.SetInteger(value);
            if (ExpressionStatus::Success == status)
                status = Operations::ConvertToString(result);
            return status;
            }

        case PRIMITIVETYPE_Long:
            {
            ::int64_t value = m_left->_GetLongValue(status, context);
            result.SetLong(value);
            if (ExpressionStatus::Success == status)
                status = Operations::ConvertToString(result);
            return status;
            }

        case PRIMITIVETYPE_Double:
            {
            double value = m_left->_GetDoubleValue(status, context);
            result.SetDouble(value);
            if (ExpressionStatus::Success == status)
                status = Operations::ConvertToString(result);
            return status;
            }
        }

    result.SetUtf8CP("");
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ResolvedConvertToString::_GetStringValue: IncompatibleTypes");
    return ExpressionStatus::IncompatibleTypes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t IValueListResult::GetCount() const { return _GetCount(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus IValueListResult::GetValueAt (EvaluationResultR result, uint32_t index) const
    {
    return index < GetCount() ? _GetValueAt (result, index) : ExpressionStatus::IndexOutOfRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArrayValueResult : IValueListResult
    {
private:
    IECInstancePtr      m_instance;
    uint32_t            m_propertyIndex;
    uint32_t            m_count;

    ArrayValueResult (IECInstanceR instance, uint32_t propIdx)
        : m_instance(&instance), m_propertyIndex(propIdx), m_count(0)
        {
        m_instance = &instance;
        m_propertyIndex = propIdx;

        ECValue v;
        if (ECObjectsStatus::Success == instance.GetValue (v, propIdx) && v.IsArray())
            m_count = v.GetArrayInfo().GetCount();
        else
            m_count = 0;
        }

    virtual uint32_t            _GetCount() const override
        {
        return m_count;
        }

    virtual ExpressionStatus    _GetValueAt (EvaluationResultR result, uint32_t index) const override
        {
        if (ECObjectsStatus::Success == m_instance->GetValue (result.InitECValue(), m_propertyIndex, index))
            {
            if (result.GetECValue()->IsStruct() && !result.GetECValue()->IsNull())
                result.SetInstance (*result.GetECValue()->GetStruct());

            return ExpressionStatus::Success;
            }
        else
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ArrayValueResult::_GetValueAt: UnknownError");
            return ExpressionStatus::UnknownError;
            }
        }
public:
    static IValueListResultPtr Create (IECInstanceR instance, uint32_t propIdx) { return new ArrayValueResult (instance, propIdx); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
IValueListResultPtr IValueListResult::Create (IECInstanceR instance, uint32_t propIdx)
    {
    return ArrayValueResult::Create (instance, propIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ValueListResult : IValueListResult
    {
private:
    EvaluationResultVector      m_values;

    ValueListResult (EvaluationResultVector const& values) : m_values(values) { }

    virtual uint32_t            _GetCount() const override
        {
        return (uint32_t)m_values.size();
        }

    virtual ExpressionStatus    _GetValueAt (EvaluationResultR result, uint32_t index) const override
        {
        if (index < _GetCount())
            {
            result = m_values[index];
            return ExpressionStatus::Success;
            }
        else
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ValueListResult::_GetValueAt(%" PRIu32 "): IndexOutOfRange", index).c_str());
            return ExpressionStatus::IndexOutOfRange;
            }
        }
public:
    static IValueListResultPtr Create (EvaluationResultVector const& values)
        {
        return new ValueListResult (values);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
IValueListResultPtr IValueListResult::Create (EvaluationResultVector const& values)
    {
    return ValueListResult::Create (values);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Aidas.Vaiksnoras 04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IValueListResult::ToString() const
    {
    Utf8String msg;
    uint32_t count = GetCount();
    for (uint32_t i = 0; i < count; i++)
        {
        EvaluationResult member;
        GetValueAt(member, i);
        if (0 != i)
            msg.append(",");
        msg.append(member.ToString());
        }    
    return msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArrayMemberSymbol : Symbol
    {
private:
    uint32_t m_arrayIndex;
    EvaluationResult                m_primitive;
    InstanceExpressionContextPtr    m_struct;

    ArrayMemberSymbol (Utf8CP name) : Symbol (name)
        {
        m_arrayIndex = 0;
        m_primitive = ECValue (/*null*/);
        }

    virtual ExpressionStatus        _GetValue (EvaluationResultR result, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, uint32_t index) override
        {
        if (m_struct.IsValid())
            return m_struct->GetValue(result, primaryList, contextsStack, index);
        else if (primaryList.GetNumberOfOperators() <= index)
            {
            result = m_primitive;
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ArrayMemberSymbol::_GetValue(%" PRIu32 ") Result: %s", m_arrayIndex, result.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        else
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ArrayMemberSymbol::_GetValue: UnknownError");
            return ExpressionStatus::UnknownError;
            }
        }
    virtual ExpressionStatus         _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override
        {
        // not applicable
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ArrayMemberSymbol::_GetValue: UnknownError");
        return ExpressionStatus::UnknownError;
        }

    static bool     ConvertToStruct (EvaluationResultR result)
        {
        if (result.IsInstanceList())
            return true;
        else if (result.IsECValue() && result.GetECValue()->IsStruct())
            {
            if (result.GetECValue()->IsNull())
                {
                ECInstanceList empty;
                result.SetInstanceList (empty, true);
                }
            else
                result.SetInstance (*result.GetECValue()->GetStruct());

            return true;
            }
        else
            return false;
        }
public:
    static RefCountedPtr<ArrayMemberSymbol> Create (Utf8CP name) { return new ArrayMemberSymbol (name); }

    void        Set (uint32_t arrayIndex, EvaluationResultR ev)
        {
        m_arrayIndex = arrayIndex;
        if (ConvertToStruct (ev) && m_struct.IsNull())
            m_struct = InstanceExpressionContext::Create();

        // once this is initialized we know we're dealing with a struct array (possible we encounter null entries first)
        if (m_struct.IsValid())
            {
            m_struct->Clear();
            if (ev.IsInstanceList())
                m_struct->SetInstances (*ev.GetInstanceList());
            }
        else
            m_primitive = ev;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus    LambdaValue::Evaluate (IValueListResultCR valueList, LambdaValue::IProcessor& processor) const
    {
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("LambdaValue::Evaluate: Started lambda evaluation. Input: %s", valueList.ToString().c_str()).c_str());
    uint32_t count = valueList.GetCount();
    EvaluationResult lambdaResult;
    if (0 < count)
        {
        // Set up a symbol context to map symbol name to member of list being processed
        SymbolExpressionContextPtr innerContext = SymbolExpressionContext::Create (m_context.get());
        RefCountedPtr<ArrayMemberSymbol> symbol = ArrayMemberSymbol::Create (m_node->GetSymbolName());
        innerContext->AddSymbol (*symbol);

        for (uint32_t i = 0; i < count; i++)
            {
            EvaluationResult member;
            ExpressionStatus status = valueList.GetValueAt (member, i);
            if (ExpressionStatus::Success != status)
                return status;
            symbol->Set (i, member);
            status = m_node->GetExpression().GetValue (lambdaResult, *innerContext);
            if (!processor.ProcessResult (status, member, lambdaResult))
                break;
            }
        }

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("LambdaValue::Evaluate: Finished lambda evaluation. Result: %s", lambdaResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitSpecNode::SetFactor (double factor)
    {
    m_units.SetConverter (UnitConverter (factor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitSpecNode::SetFactorAndOffset (double factor, double offset)
    {
    m_units.SetConverter (UnitConverter (factor, offset));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
UnitSpecNodePtr UnitSpecNode::Create (NodeR left, Utf8CP baseUnitName)
    {
    return new UnitSpecNode (left, baseUnitName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus UnitSpecNode::_GetValue (EvaluationResultR result, ExpressionContextR context)
    {
    ExpressionStatus    status = _GetLeftP()->GetValue (result, context);
    if (ExpressionStatus::Success != status || !context.EnforcesUnits())
        return status;
    
    UnitSpecCR operandUnits = result.GetUnits();
    if (operandUnits.IsUnspecified() || operandUnits.IsEquivalent (m_units))
        {
        // no conversion needed, set units directly
        result.SetUnits (m_units);
        return ExpressionStatus::Success;
        }
    else if (!operandUnits.IsCompatible (m_units))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UnitSpecNode::_GetValue: IncompatibleUnits");
        return ExpressionStatus::IncompatibleUnits;
        }

    if (!result.IsECValue())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UnitSpecNode::_GetValue: PrimitiveRequired");
        return ExpressionStatus::PrimitiveRequired;
        }

    ECValueR v = *result.GetECValue();
    if (!v.IsPrimitive() || v.IsNull() || !v.ConvertToPrimitiveType (PRIMITIVETYPE_Double))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UnitSpecNode::_GetValue: IncompatibleTypes");
        return ExpressionStatus::IncompatibleTypes;
        }

    // convert units
    double rd = v.GetDouble();
    if (!operandUnits.ConvertTo (rd, m_units))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UnitSpecNode::_GetValue: IncompatibleUnits");
        return ExpressionStatus::IncompatibleUnits;
        }

    v.SetDouble (rd);
    result.SetUnits (m_units);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("UnitSpecNode::_GetValue: Result: %s", result.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
