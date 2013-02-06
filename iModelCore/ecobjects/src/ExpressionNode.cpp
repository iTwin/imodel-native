/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ExpressionNode.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2012
//--------------+------------------------------------------------------------------------
static void performConcatenation(EvaluationResultR evalResult, ECValueCR left, ECValueCR right)
    {
    WString     resultString;
    wchar_t const* leftString   = left.GetString();
    wchar_t const* rightString  = right.GetString();
    resultString.reserve(wcslen(leftString) + wcslen(rightString) + 1);
    resultString.append(leftString);
    resultString.append(rightString);
    evalResult.GetECValueR().SetString(resultString.c_str(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToInt32(EvaluationResultR evalResult) 
    {
    ECN::ECValueR    ecValue = evalResult.GetECValueR();
    BeAssert (!ecValue.IsUninitialized() && ecValue.IsPrimitive());

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetInteger(ecValue.GetBoolean() ? 1 : 0);
            return ExprStatus_Success;

        case PRIMITIVETYPE_Integer:
            return ExprStatus_Success;

        case PRIMITIVETYPE_Long:
            ecValue.SetInteger((int)ecValue.GetLong());
            return ExprStatus_Success;

        case PRIMITIVETYPE_Double:
            ecValue.SetInteger((int)ecValue.GetDouble());
            return ExprStatus_Success;

        case PRIMITIVETYPE_String:
            return ExprStatus_NotImpl;
        }

    return ExprStatus_WrongType; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToString(EvaluationResultR evalResult) 
    {
    ECN::ECValueR    ecValue = evalResult.GetECValueR();
    BeAssert (!ecValue.IsUninitialized());
        
    if (!ecValue.IsPrimitive() || ecValue.IsNull())
        return ExprStatus_WrongType;

    if (ecValue.IsString())
        return ExprStatus_Success;

    // --  TODO -- should this involve extended types
    wchar_t     buffer [80];
    switch(ecValue.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            BeStringUtilities::Snwprintf(buffer, _countof(buffer), L"%d", ecValue.GetInteger());
            break;
        case PRIMITIVETYPE_Boolean:
            //  TODO -- should this be locale specific?
            wcscpy (buffer, ecValue.GetBoolean() ?  L"true"  : L"false");
            break;
        case PRIMITIVETYPE_Long:
            BeStringUtilities::Snwprintf(buffer, _countof(buffer), L"%ld", ecValue.GetLong());
            break;
        case PRIMITIVETYPE_Double:
            //  TODO -- needs locale, extended type.
            BeStringUtilities::Snwprintf(buffer, _countof(buffer), L"%f", ecValue.GetDouble());
            break;
        default:
            return ExprStatus_NotImpl;
        }

    evalResult.GetECValueR().SetString(buffer);
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToInt64(EvaluationResultR evalResult) 
    {
    ECN::ECValueR    ecValue = evalResult.GetECValueR();
    BeAssert (!ecValue.IsUninitialized() && ecValue.IsPrimitive());

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetLong(ecValue.GetBoolean() ? 1 : 0);
            return ExprStatus_Success;

        case PRIMITIVETYPE_Integer:
            ecValue.SetLong(ecValue.GetInteger());
            return ExprStatus_Success;

        case PRIMITIVETYPE_Long:
            return ExprStatus_Success;

        case PRIMITIVETYPE_Double:
            ecValue.SetLong((Int64)ecValue.GetDouble());
            return ExprStatus_Success;

        case PRIMITIVETYPE_String:
            return ExprStatus_NotImpl;
        }

    return ExprStatus_WrongType; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToDouble(EvaluationResultR evalResult) 
    {
    ECN::ECValueR    ecValue = evalResult.GetECValueR();
    BeAssert (!ecValue.IsUninitialized() && ecValue.IsPrimitive());

    ECN::PrimitiveType  primType = ecValue.GetPrimitiveType();
    switch(primType)
        {
        case PRIMITIVETYPE_Boolean:
            ecValue.SetDouble(ecValue.GetBoolean() ? 1 : 0);
            return ExprStatus_Success;

        case PRIMITIVETYPE_Integer:
            ecValue.SetDouble(ecValue.GetInteger());
            return ExprStatus_Success;

        case PRIMITIVETYPE_Long:
            return ExprStatus_WrongType; 

        case PRIMITIVETYPE_Double:
            return ExprStatus_Success;

        case PRIMITIVETYPE_String:
            return ExprStatus_NotImpl;
        }

    return ExprStatus_WrongType; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToDateTime(EvaluationResultR evalResult) {return ExprStatus_NotImpl; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertToArithmeticOrBooleanOperand (EvaluationResultR evalResult) {return ExprStatus_WrongType; }
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

    if (ecValue.NativeValue is Int32)
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
    ECValueR    ecValue = evalResult.GetECValueR();

    BeAssert(!ecValue.IsUninitialized());
    BeAssert(!ecValue.IsNull());
    BeAssert(ecValue.IsPrimitive());

    if (ecValue.IsBoolean())
        return ExprStatus_Success;

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
            wchar_t const* strValue = ecValue.GetString();
            if (!wcscmp(L"0", strValue) || !wcscmp(L"false", strValue))
                boolValue = false;
            else if (!wcscmp(L"1", strValue) || !wcscmp(L"true", strValue))
                boolValue = true;
            else
                return ExprStatus_WrongType;
            }

        default:
            return ExprStatus_WrongType;
        }

    ecValue.SetBoolean(boolValue);

    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::ConvertStringToArithmeticOperand (EvaluationResultR ecValue) {return ExprStatus_WrongType; }
#ifdef NOTNOW
internal static void ConvertStringToArithmeticOperand
(
ref EvaluationResultCR     ecValue
)
    {
    if (ecValue.NativeValue is Int32)
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
ExpressionStatus Operations::ConvertStringToArithmeticOrBooleanOperand (EvaluationResultR ecValue) {return ExprStatus_WrongType; }

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

    if (ecValue.NativeValue is Int32)
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
ExpressionStatus Operations::PerformArithmeticPromotion(EvaluationResult& leftResult, EvaluationResult& rightResult, bool allowStrings)
    {
    ECN::ECValueR    left    = leftResult.GetECValueR();
    ECN::ECValueR    right   = rightResult.GetECValueR();

    if (!left.IsPrimitive() || !right.IsPrimitive())
        return ExprStatus_PrimitiveRequired;

    ECN::PrimitiveType   leftCode    = left.GetPrimitiveType();
    ECN::PrimitiveType   rightCode   = right.GetPrimitiveType();

    //  PRIMITIVETYPE_DateTime, point types, and Boolean are all missing from this
    //  We may also want to provide a way for structs with extended types to perform the conversion to 
    //  string
    //
    //  Managed ECExpressions always converted to numeric, requiring & for concatenation

    if (PRIMITIVETYPE_String == leftCode || PRIMITIVETYPE_String == rightCode)
        {
        if (!allowStrings)
            return ExprStatus_IncompatibleTypes;

        if (leftCode == rightCode)
            return ExprStatus_Success;

        //  Convert to strings; may want to involve the extended type in that
        return ExprStatus_IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        {
        if (leftCode == rightCode)
            return ExprStatus_Success;

        ECValueR            target = PRIMITIVETYPE_Long == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Long == leftCode ? rightCode : leftCode;
        if (PRIMITIVETYPE_Double == targetCode)
            {
            target.SetLong((Int64)target.GetDouble());
            return ExprStatus_Success;
            }

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetLong(target.GetInteger());
            return ExprStatus_Success;
            }

        return ExprStatus_IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Double == leftCode || PRIMITIVETYPE_Double == rightCode)
        {
        //  Both must be doubles
        if (leftCode == rightCode)
            return ExprStatus_Success;

        ECValueR            target = PRIMITIVETYPE_Double == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Double == leftCode ? rightCode : leftCode;

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetDouble(target.GetInteger());
            return ExprStatus_Success;
            }

        return ExprStatus_IncompatibleTypes;
        }

    //  Both must be int
    if (PRIMITIVETYPE_Integer == leftCode && PRIMITIVETYPE_Integer == rightCode)
        return ExprStatus_Success;

    return ExprStatus_WrongType;
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
    if (!left.GetECValue().IsPrimitive() || !right.GetECValue().IsPrimitive())
        return ExprStatus_WrongType;

    ExpressionStatus     status = ExprStatus_Success;

    if (left.GetECValue().IsString())
        {
        status = ConvertStringToArithmeticOrBooleanOperand (left);
        if (ExprStatus_Success != status)
            return status;
        }

    if (right.GetECValue().IsString())
        {
        status = ConvertStringToArithmeticOrBooleanOperand (right);
        if (ExprStatus_Success != status)
            return status;
        }

    if (left.GetECValue().IsBoolean() || right.GetECValue().IsBoolean())
        {
        status = ConvertToBooleanOperand (left);
        if (ExprStatus_Success == status)
            status = ConvertToBooleanOperand (right);
        return status;
        }

    return PerformArithmeticPromotion (left, right, false);
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
    ECN::ECValueR        ecLeft = left.GetECValueR();

    if (!ecLeft.IsPrimitive())
        return ExprStatus_IncompatibleTypes;

    ECN::PrimitiveType   primType = ecLeft.GetPrimitiveType();

    if (PRIMITIVETYPE_String == primType)
        {
        ExpressionStatus status = ConvertStringToArithmeticOperand(left);
        if (ExprStatus_Success != status)
            return status;

        primType = ecLeft.GetPrimitiveType();
        }

    switch (primType)
        {
        case PRIMITIVETYPE_Double:
            {
            resultOut.GetECValueR().SetDouble(-left.GetECValue().GetDouble());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Integer:
            {
            resultOut.GetECValueR().SetInteger(-left.GetECValue().GetInteger());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Long:
            {
            resultOut.GetECValueR().SetLong(-left.GetECValue().GetLong());
            return ExprStatus_Success;
            }

        }

    return ExprStatus_WrongType;
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
    ECN::ECValueR        ecLeft = left.GetECValueR();

    if (!ecLeft.IsPrimitive())
        return ExprStatus_IncompatibleTypes;

    ECN::PrimitiveType   primType = ecLeft.GetPrimitiveType();

    if (PRIMITIVETYPE_String == primType)
        {
        ExpressionStatus status = ConvertStringToArithmeticOperand(left);
        if (ExprStatus_Success != status)
            return status;

        primType = ecLeft.GetPrimitiveType();
        }

    switch (primType)
        {
        case PRIMITIVETYPE_Boolean:
            {
            resultOut.GetECValueR().SetBoolean(!left.GetECValue().GetBoolean());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Integer:
            {
            resultOut.GetECValueR().SetInteger(~left.GetECValue().GetInteger());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Long:
            {
            resultOut.GetECValueR().SetLong(~left.GetECValue().GetLong());
            return ExprStatus_Success;
            }

        }

    return ExprStatus_WrongType;
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
    if (ExprStatus_Success != status)
        return status;

    if (!left.GetECValue().IsPrimitive())
        return ExprStatus_WrongType;

    ECN::PrimitiveType   primType = left.GetECValue().GetPrimitiveType();
    int                 count = right.GetECValue().GetInteger();

    //  If string, we may want to try to convert to int.
    switch (primType)
        {
        case PRIMITIVETYPE_Integer:
            {
            int     value = left.GetECValue().GetInteger();
            if (shiftOp == TOKEN_ShiftLeft)
                {
                resultOut.GetECValueR().SetInteger(value << count);
                return ExprStatus_Success;
                }
            else if (shiftOp == TOKEN_ShiftRight)
                {
                resultOut.GetECValueR().SetInteger(value >> count);
                return ExprStatus_Success;
                }
            else if (shiftOp == TOKEN_UnsignedShiftRight)
                {
                UInt32  uvalue = (UInt32)value;
                resultOut.GetECValueR().SetInteger((int)(uvalue >> count));
                return ExprStatus_Success;
                }
            }
            break;

        case PRIMITIVETYPE_Long:
            {
            Int64         value = left.GetECValue().GetInteger();
            if (shiftOp == TOKEN_ShiftLeft)
                {
                resultOut.GetECValueR().SetLong(value << count);
                return ExprStatus_Success;
                }
            else if (shiftOp == TOKEN_ShiftRight)
                {
                resultOut.GetECValueR().SetLong(value >> count);
                return ExprStatus_Success;
                }
            else if (shiftOp == TOKEN_UnsignedShiftRight)
                {
                UInt64  uvalue = (UInt64)value;
                resultOut.GetECValueR().SetLong((Int64)(uvalue >> count));
                return ExprStatus_Success;
                }
            }
            break;
        }

    return ExprStatus_WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformMultiplication
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right
)
    {
    ExpressionStatus status = PerformArithmeticPromotion (left, right);

    if (ExprStatus_Success != status)
        return status;

    ECN::PrimitiveType   primType = left.GetECValue().GetPrimitiveType();

    switch (primType)
        {
        case PRIMITIVETYPE_Integer:
            {
            resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() * right.GetECValue().GetInteger());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Long:
            {
            resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() * right.GetECValue().GetLong());
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Double:
            {
            resultOut.GetECValueR().SetDouble(left.GetECValue().GetDouble() * right.GetECValue().GetDouble());
            return ExprStatus_Success;
            }
        }

    return ExprStatus_WrongType;
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
    return ExprStatus_NotImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformIntegerDivision
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right
)
    {
    ExpressionStatus status = PerformArithmeticPromotion (left, right);

    if (ExprStatus_Success != status)
        return status;

    switch (left.GetECValue().GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            int     divisor = right.GetECValue().GetInteger();
            if (0 == divisor)
                return ExprStatus_DivideByZero;

            resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() / divisor);

            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64     divisor = right.GetECValue().GetLong();
            if (0 == divisor)
                return ExprStatus_DivideByZero;

            resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() / divisor);

            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Double:
            {
            double     divisor = right.GetECValue().GetDouble();
            if (0 == divisor)
                return ExprStatus_DivideByZero;

            resultOut.GetECValueR().SetDouble(floor(left.GetECValue().GetDouble() / divisor));

            return ExprStatus_Success;
            }

        }

    return ExprStatus_InvalidTypesForDivision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Operations::PerformDivision
(
EvaluationResultR           resultOut,
EvaluationResultR           left,
EvaluationResultR           right
)
    {
    ExpressionStatus status = ConvertToDouble (left);
    if (ExprStatus_Success != status)
        return status;

    status = ConvertToDouble (right);
    if (ExprStatus_Success != status)
        return status;

    double  divisor = right.GetECValue().GetDouble();
    if (0 == divisor)
        return ExprStatus_DivideByZero;

    resultOut.GetECValueR().SetDouble(left.GetECValueR().GetDouble() / divisor);
    return ExprStatus_Success;
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

    if (ExprStatus_Success != status)
        return status;

    switch (left.GetECValue().GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            int     divisor = right.GetECValue().GetInteger();
            if (0 == divisor)
                return ExprStatus_DivideByZero;

            resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() % divisor);
            return ExprStatus_Success;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64     divisor = right.GetECValue().GetLong();
            if (0 == divisor)
                return ExprStatus_DivideByZero;

            resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() % divisor);
            return ExprStatus_Success;
            }
        }

    return ExprStatus_InvalidTypesForDivision;
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

    if (ExprStatus_Success != status)
        return status;

    if (left.GetECValue().IsBoolean())
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.GetECValueR().SetBoolean(left.GetECValue().GetBoolean() | right.GetECValue().GetBoolean());
                return ExprStatus_Success;

            case TOKEN_And:
                resultOut.GetECValueR().SetBoolean(left.GetECValue().GetBoolean() & right.GetECValue().GetBoolean());
                return ExprStatus_Success;

            case TOKEN_Xor:
                resultOut.GetECValueR().SetBoolean(left.GetECValue().GetBoolean() ^ right.GetECValue().GetBoolean());
                return ExprStatus_Success;
            }
        }

    ECN::PrimitiveType  primType = left.GetECValue().GetPrimitiveType();
    if (PRIMITIVETYPE_Integer == primType)
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() | right.GetECValue().GetInteger());
                return ExprStatus_Success;

            case TOKEN_And:
                resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() & right.GetECValue().GetInteger());
                return ExprStatus_Success;

            case TOKEN_Xor:
                resultOut.GetECValueR().SetInteger(left.GetECValue().GetInteger() ^ right.GetECValue().GetInteger());
                return ExprStatus_Success;
            }
        }

    if (PRIMITIVETYPE_Long == primType)
        {
        switch (junctionOperator)
            {
            case TOKEN_Or:
                resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() | right.GetECValue().GetLong());
                return ExprStatus_Success;

            case TOKEN_And:
                resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() & right.GetECValue().GetLong());
                return ExprStatus_Success;

            case TOKEN_Xor:
                resultOut.GetECValueR().SetLong(left.GetECValue().GetLong() ^ right.GetECValue().GetLong());
                return ExprStatus_Success;
            }
        }


    //  It should be impossible to get here. All errors should be caught in the promotion functions.
    return ExprStatus_WrongType;
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
    
    if (ExprStatus_Success != status)
        return status;

    if (boolValue)
        {
        resultOut = leftValue.GetECValue();
        return ExprStatus_Success;
        }

    status = rightValue.GetBoolean(boolValue, false);
    if (ExprStatus_Success != status)
        return status;

    resultOut = rightValue.GetECValue();
    return ExprStatus_Success;
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
    
    if (ExprStatus_Success != status)
        return status;

    if (!boolValue)
        {
        resultOut = leftValue.GetECValue();
        return ExprStatus_Success;
        }

    status = rightValue.GetBoolean(boolValue, false);
    if (ExprStatus_Success != status)
        return status;

    resultOut = rightValue.GetECValue();
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                EvaluationResult::EvaluationResult () : 
                        m_valueType(ValType_None), m_unitsOrder(UO_Unknown) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                EvaluationResult::~EvaluationResult () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetInteger(Int32& result)
    {
#if defined (NOTNOW)
    //  Enable this is we disable direct access to ECValue
    //  Only we don't allow direct access to ECValue
    if (ValType_ECValue != m_valueType)
        return ExprStatus_WrongType;
#endif

    result = m_ecValue.GetInteger();
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetECValue(ECN::ECValueR result)
    {
#if defined (NOTNOW)
    //  Enable this is we disable direct access to ECValue
    if (ValType_ECValue != m_valueType)
        return ExprStatus_WrongType;
#endif

    result.Clear();
    result = m_ecValue;
    return ExprStatus_Success;
    }

//  Still undecided on wrapping ECValue vs. extending it.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECValueR    EvaluationResult::GetECValueR() 
    { 
    return m_ecValue; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECValueCR   EvaluationResult::GetECValue() const 
    { 
    return m_ecValue; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                EvaluationResult::EvaluationResult(EvaluationResultCR rhs)
    {
    m_ecValue.Clear();
    m_valueType = rhs.m_valueType;
    m_unitsOrder = rhs.m_unitsOrder;
    m_ecValue = rhs.m_ecValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResultR EvaluationResult::operator=(EvaluationResultCR rhs)
    {
    m_ecValue.Clear();
    m_valueType = rhs.m_valueType;
    m_unitsOrder = rhs.m_unitsOrder;
    m_ecValue = rhs.m_ecValue;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            EvaluationResult::Clear()
    {
    m_ecValue.Clear();
    m_valueType = ValType_None;
    m_unitsOrder = UO_Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus EvaluationResult::GetBoolean(bool& result, bool requireBoolean)
    {
#ifdef NOTNOW   //  Only we don't allow direct access to ECValue
    if (ValType_ECValue != m_valueType)
        return ExprStatus_WrongType;
#endif
    if (m_ecValue.IsNull())
        {
        result = false;
        return requireBoolean ? ExprStatus_WrongType : ExprStatus_Success;
        }

    if (!m_ecValue.IsPrimitive())
        return ExprStatus_WrongType;

    switch(m_ecValue.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            result = m_ecValue.GetBoolean();
            return ExprStatus_Success;

        case PRIMITIVETYPE_Integer:
            result = m_ecValue.GetInteger() != 0;
            return ExprStatus_Success;

        case PRIMITIVETYPE_Long:
            result = m_ecValue.GetLong() != 0;
            return ExprStatus_Success;

        case PRIMITIVETYPE_Double:
            result = m_ecValue.GetDouble() != 0.0;
            return ExprStatus_Success;

        case PRIMITIVETYPE_String:
            {
            wchar_t const* value = m_ecValue.GetString();
            if (!wcscmp(L"1", value) || !BeStringUtilities::Wcsicmp(L"true", value))
                {
                result = true;
                return ExprStatus_Success;
                }

            if (!wcscmp(L"0", value) || !BeStringUtilities::Wcsicmp(L"false", value))
                {
                result = false;
                return ExprStatus_Success;
                }

            return ExprStatus_WrongType;
            }
        }

    return ExprStatus_WrongType;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ValueResult::ValueResult (EvaluationResultR result) : 
                            m_evalResult(result) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ValueResult::~ValueResult()
    {
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
void            NodeHelpers::GetAdditiveNodes(NodeVector& nodes, NodeR rightMost)
    {
    BeAssert (rightMost.IsAdditive());

    BinaryNodeCP    current = dynamic_cast<BinaryNodeCP>(&rightMost);
    BeAssert(NULL != current);

    NodeP   left = current->GetLeftP();
    if (left->IsAdditive())
        GetAdditiveNodes(nodes, *left);

    nodes.push_back(&rightMost);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            NodeHelpers::DetermineKnownUnitsSame(UnitsTypeR units, NodeR rightMost)
    {
    NodeVector  nodes;

    nodes.push_back(rightMost.GetLeftP());
    nodes.push_back(&rightMost);
    NodeHelpers::DetermineKnownUnitsSame(units, nodes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            NodeHelpers::DetermineKnownUnitsSame(UnitsTypeR units, NodeVector& nodes)
    {
    UnitsType   leftUnits;
    bool        powerPromotionRequired = false;

    NodeVectorIterator  nodeIterator = nodes.begin();
    (*nodeIterator)->GetLeftP()->DetermineKnownUnits(leftUnits);
    for (; nodeIterator != nodes.end(); nodeIterator++)
        {
        UnitsType   rightUnits;
        NodeP       right = (*nodeIterator)->GetRightP();
        right->DetermineKnownUnits(rightUnits);
        if (leftUnits.m_unitsOrder == rightUnits.m_unitsOrder)
            {
            if (!rightUnits.m_powerCanIncrease)
                leftUnits.m_powerCanIncrease = false;

            if (!rightUnits.m_powerCanDecrease)
                leftUnits.m_powerCanDecrease = false;
            }

        if (leftUnits.m_unitsOrder != rightUnits.m_unitsOrder)
            {
            if (leftUnits.m_unitsOrder == UO_Unknown)
                {
                //  This should only happen when a symbol can't be resolved or this is a method 
                leftUnits = rightUnits;
                continue;
                }

            if (leftUnits.m_unitsOrder < rightUnits.m_unitsOrder)
                {
#if defined (NOTNOW)
                if (!leftUnits.m_powerFixed)
                    {
                    powerPromotionRequired = true;
                    leftUnits = rightUnits;
                    continue;
                    }
#else
                leftUnits = rightUnits;
                continue;
#endif
                //  Place an error on the node.
                }
            }
        }

    if (powerPromotionRequired)
        {
        NodeVectorIterator  nodeIterator = nodes.begin();
        (*nodeIterator)->GetLeftP()->ForceUnitsOrder(leftUnits);
        for (; nodeIterator != nodes.end(); nodeIterator++)
            {
            NodeP       right = (*nodeIterator)->GetRightP();
            right->ForceUnitsOrder(leftUnits);
            }
        }

    units = leftUnits;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString         Node::ToString() const
    {
    return _ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Node::Traverse(NodeVisitorR visitor)
    {
    if (this->GetHasParens() && !visitor.OpenParens())
        return false;
    
    if (!_Traverse (visitor))
        return false;

    if (this->GetHasParens() && !visitor.CloseParens())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateBooleanLiteral(bool literalValue)
    {
    return new BooleanLiteralNode(literalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateNullLiteral()                   { return new NullLiteralNode(); }
NodePtr         Node::CreatePoint2DLiteral (DPoint2dCR pt)  { return new Point2DLiteralNode (pt); }
NodePtr         Node::CreatePoint3DLiteral (DPoint3dCR pt)  { return new Point3DLiteralNode (pt); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateStringLiteral (wchar_t const* value)
    {
    size_t      origLen = wcslen(value);
    BeAssert(origLen > 1);
    wchar_t*    buffer = (wchar_t*)_alloca(2 *(origLen+1));

    BeStringUtilities::Wcsncpy(buffer, origLen, value+1);
    buffer[origLen-2] = 0;

    return new StringLiteralNode(buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateIntegerLiteral (int value)
    {
    return new IntegerLiteralNode(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateInt64Literal(Int64 value)
    {
    return new Int64LiteralNode(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         Node::CreateFloatLiteral(double value)
    {
    return new DoubleLiteralNode(value);
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

    return ErrorNode::Create(L"internal error: unexpected arithmetic token", NULL, NULL).get();
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
NodePtr         Node::CreateBitWise(ExpressionToken tokenId, NodeR left, NodeR right)
    {
    return new BitWiseNode(tokenId, left, right);
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
            if (propertyCustomAttribute.IsValid() &&  (ECOBJECTS_STATUS_Success == propertyCustomAttribute->GetValue (value, L"DefaultValue")))
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PrimaryListNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    return context.GetValue(evalResult, *this, context, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*  PrimaryListNode::GetName(size_t index) const
    {
    if (m_operators.size() <= index)
        return NULL;

    NodeP   node = m_operators[index].get();
    ExpressionToken    nodeId = node->GetOperation();
    if (TOKEN_Ident == nodeId || TOKEN_Dot == nodeId)
        {
        IdentNodeP identNode = static_cast<IdentNodeP>(node);
        return identNode->GetName();
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
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrimaryListNode::AppendArrayNode(LBracketNodeR lbracketNode)
    {
    m_operators.push_back(&lbracketNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus UnaryArithmeticNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    inputValue;
    ExpressionStatus    status = _GetLeftP()->GetValue(inputValue, context, allowUnknown, allowOverrides);
    if (ExprStatus_Success != status)
        return status;

    if (_GetOperation() == TOKEN_Minus)
        return Operations::PerformUnaryMinus(evalResult, inputValue);

    if (_GetOperation() == TOKEN_Plus)
        {
        evalResult = inputValue;
        return ExprStatus_Success;
        }

    return Operations::PerformUnaryNot(evalResult, inputValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeInstanceMethod(EvaluationResult& evalResult, EvaluationResultCR instanceData, ExpressionContextR context)
    {
    MethodReferencePtr  methodReference;

    //  The lookup should include the instance data since that would be the most logical place to find the method reference
    ExpressionStatus    exprStatus = context.ResolveMethod(methodReference, this->GetMethodName(), true);
    if (ExprStatus_Success != exprStatus)
        {
        evalResult = ECN::ECValue();
        return exprStatus;
        }

    EvaluationResultVector  argsVector;

    ExpressionStatus status = m_arguments->EvaluateArguments(argsVector, context);
    if (NULL != status)
        return status;

    return methodReference->InvokeInstanceMethod(evalResult, instanceData, argsVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeStaticMethod(EvaluationResult& evalResult, MethodReferenceR methodReference, ExpressionContextR context)
    {
    EvaluationResultVector  argsVector;

    ExpressionStatus status = m_arguments->EvaluateArguments(argsVector, context);
    if (NULL != status)
        return status;

    return methodReference.InvokeStaticMethod(evalResult, argsVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus CallNode::InvokeStaticMethod(EvaluationResult& evalResult, ExpressionContextR context)
    {
    MethodReferencePtr  methodReference;

    //  The lookup should include the instance data since that would be the most logical place to find the method reference
    ExpressionStatus    exprStatus = context.ResolveMethod(methodReference, this->GetMethodName(), true);
    if (ExprStatus_Success != exprStatus)
        {
        evalResult = ECN::ECValue();
        return exprStatus;
        }

    return InvokeStaticMethod(evalResult, *methodReference, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus AssignmentNode::PerformModifier (ExpressionToken  modifier, EvaluationResultR left, EvaluationResultR right)
    {
    return ExprStatus_NotImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus AssignmentNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    // Modifiers not implemented
    BeAssert (_GetOperation() == TOKEN_None);

#if defined (NOTNOW)
    ExpressionToken    operation = _GetOperation();
    if (TOKEN_None != operation)
        {
        EvaluationResult    leftResult;
        status = _GetLeftP()->GetValue(leftResult, context, allowUnknown, allowOverrides);
        if (ExprStatus_Success != status)
            return status;

        status = PerformModifier(operation, leftResult, evalResult);
        if (ExprStatus_Success != status)
            return status;
        }
#endif

    NodeP   leftNode = _GetLeftP();
    if (leftNode->GetOperation() != TOKEN_PrimaryList)
        return ExprStatus_UnknownError;

    PrimaryListNodeP    primaryList = static_cast<PrimaryListNodeP>(leftNode);
    EvaluationResult    instanceResult;
    ReferenceResult     refResult;

    ExpressionStatus    exprStatus = context.GetReference(instanceResult, refResult, *primaryList, context, 0);
    if (ExprStatus_Success != exprStatus)
        return exprStatus;

    exprStatus = _GetRightP()->GetValue(evalResult, context, allowUnknown, allowOverrides);
    if (ExprStatus_Success != exprStatus)
        return exprStatus;

    ECN::PrimitiveECPropertyCP   primProperty = refResult.m_property->GetAsPrimitiveProperty();
    if (NULL != primProperty)
        {
        ECN::IECInstanceP    instance = instanceResult.GetECValueR().GetStruct().get();
        ECN::ECEnablerCR     enabler = instance->GetEnabler();

        ::UInt32     propertyIndex;
        if (enabler.GetPropertyIndex(propertyIndex, refResult.m_accessString.c_str()) != ECN::ECOBJECTS_STATUS_Success)
            {
            evalResult.Clear();
            return ExprStatus_UnknownError;
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
        ECN::ECObjectsStatus  ecStatus = instance->SetValue(propertyIndex, evalResult.GetECValue());
        if (ECN::ECOBJECTS_STATUS_Success != ecStatus)
            {
            evalResult.Clear();
            return ExprStatus_UnknownError;
            }

        return ExprStatus_Success;
        }  //  End of processing for primitive property


    //  For now only support setting primitive properties; should add support for arrays here
    return ExprStatus_PrimitiveRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ArgumentTreeNode::EvaluateArguments(EvaluationResultVector& results, ExpressionContextR context)
    {
    ExpressionStatus    status = ExprStatus_Success;
    BeAssert (results.size() == 0);
    results.reserve(m_arguments.size());

    for (NodePtrVectorIterator curr = m_arguments.begin(); curr != m_arguments.end(); ++curr)
        {
        results.push_back(EvaluationResult());
        EvaluationResultR currValue = results.back();
        status = (*curr)->GetValue(currValue, context, false, false);
        if (ExprStatus_Success != status)
            return status;
        }

    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus IIfNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    local;

    ExpressionStatus    status = m_condition->GetValue(local, context, allowUnknown, allowOverrides);
    if (ExprStatus_Success != status)
        return status;

    bool    condition;
    status = local.GetBoolean(condition, false);
    if (ExprStatus_Success != status)
        return status;

    if (condition)
        return m_true->GetValue(evalResult, context, allowUnknown, allowOverrides);

    return m_false->GetValue(evalResult, context, allowUnknown, allowOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArithmeticNode::_ForceUnitsOrder
(
UnitsTypeCR     requiredType
)
    {
    if (this->IsAdditive())
        {
        GetLeftP()->ForceUnitsOrder(requiredType);
        GetRightP()->ForceUnitsOrder(requiredType);
        return;
        }

    UnitsType   leftUnits;
    UnitsType   rightUnits;

    GetLeftP()->DetermineKnownUnits(leftUnits);
    GetRightP()->DetermineKnownUnits(rightUnits);

    if (UO_Unknown == leftUnits.m_unitsOrder ||  UO_Unknown == rightUnits.m_unitsOrder)
        return;  //  generate an error?

    if (TOKEN_Star == m_operatorCode)
        {
        int     unitsOrder = leftUnits.m_unitsOrder * rightUnits.m_unitsOrder;

        int     diff = requiredType.m_unitsOrder - unitsOrder;
        if (0 == diff)
            return;

        if (diff < 0)
            {
            if (leftUnits.m_powerCanDecrease)
                {
                leftUnits.m_unitsOrder = (UnitsOrder)(leftUnits.m_unitsOrder/UO_Linear);
                GetLeftP()->ForceUnitsOrder(leftUnits);
                ForceUnitsOrder(requiredType);
                return;
                }

            if (rightUnits.m_powerCanDecrease)
                {
                rightUnits.m_unitsOrder = (UnitsOrder)(rightUnits.m_unitsOrder/UO_Linear);
                GetRightP()->ForceUnitsOrder(rightUnits);
                ForceUnitsOrder(requiredType);
                return;
                }

            //  Can't get it low enough. Need to report an error
            return;
            }

        BeAssert (diff > 0);
        if (!leftUnits.m_powerCanIncrease)
            {
            leftUnits.m_unitsOrder = (UnitsOrder)(leftUnits.m_unitsOrder*UO_Linear);
            GetLeftP()->ForceUnitsOrder(leftUnits);
            ForceUnitsOrder(requiredType);
            return;
            }

        if (!rightUnits.m_powerCanIncrease)
            {
            rightUnits.m_unitsOrder = (UnitsOrder)(rightUnits.m_unitsOrder*UO_Linear);
            GetRightP()->ForceUnitsOrder(rightUnits);
            ForceUnitsOrder(requiredType);
            return;
            }

        //  Can't get it high enough -- error
        return;
        }

    if (TOKEN_Mod == m_operatorCode || TOKEN_Slash == m_operatorCode)
        {

        int     unitsOrder = leftUnits.m_unitsOrder / rightUnits.m_unitsOrder;
        int     diff = requiredType.m_unitsOrder - unitsOrder;

        if (0 == diff)
            return;

        if (diff < 0)
            {
            if (leftUnits.m_powerCanDecrease)
                {
                leftUnits.m_unitsOrder = (UnitsOrder)(leftUnits.m_unitsOrder/UO_Linear);
                GetLeftP()->ForceUnitsOrder(leftUnits);
                ForceUnitsOrder(requiredType);
                return;
                }

            if (rightUnits.m_powerCanIncrease)
                {
                rightUnits.m_unitsOrder = (UnitsOrder)(rightUnits.m_unitsOrder*UO_Linear);
                GetRightP()->ForceUnitsOrder(rightUnits);
                ForceUnitsOrder(requiredType);
                return;
                }

            //  Can't get it low enough. Need to report an error
            return;
            }

        BeAssert (diff > 0);
        if (!leftUnits.m_powerCanIncrease)
            {
            leftUnits.m_unitsOrder = (UnitsOrder)(leftUnits.m_unitsOrder*UO_Linear);
            GetLeftP()->ForceUnitsOrder(leftUnits);
            ForceUnitsOrder(requiredType);
            return;
            }

        if (!rightUnits.m_powerCanDecrease)
            {
            rightUnits.m_unitsOrder = (UnitsOrder)(rightUnits.m_unitsOrder/UO_Linear);
            GetRightP()->ForceUnitsOrder(rightUnits);
            ForceUnitsOrder(requiredType);
            return;
            }

        //  Can't get it high enough -- error
        return;
        }

    //  Have Shift, etc. 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArithmeticNode::_DetermineKnownUnits
(
UnitsTypeR      unitsType
)
    {
    if (IsAdditive())
        {
        NodeVector  additiveNodes;
        //  Puts this node and all of the additive left children into the list.  The first node
        //  in the list 
        NodeHelpers::GetAdditiveNodes(additiveNodes, *this);
        NodeHelpers::DetermineKnownUnitsSame(unitsType, additiveNodes);
        return;
        }

    //  If additive node, then generate the list of additive nodes.
    //  If any entry in the list can provide a known type then that is the required order for the entire
    //  list.

    //  If a multiplicative node, 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ArithmeticNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExprStatus_Success != status)
        return status;

    status = _Promote(leftResult, rightResult, context);
    if (ExprStatus_Success != status)
        return status;

    return _PerformOperation(evalResult, leftResult, rightResult, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ConcatenateNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExprStatus_Success != status)
        return status;
    
    if (((status = Operations::ConvertToString(leftResult)) != ExprStatus_Success) || ((status = Operations::ConvertToString(rightResult)) != ExprStatus_Success))
        return status;

    performConcatenation (evalResult, leftResult.GetECValue(), rightResult.GetECValue());

    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  ShiftNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExprStatus_Success != status)
        return status;

    return Operations::PerformShift(evalResult, m_operatorCode, leftResult, rightResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus  LogicalNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = ExprStatus_UnknownError;

    switch (m_operatorCode)
        {
    case TOKEN_And:
    case TOKEN_Or:
    case TOKEN_Xor:
        status = GetOperandValues(leftResult, rightResult, context);
        if (ExprStatus_Success == status)
            status = Operations::PerformJunctionOperator(evalResult, _GetOperation(), leftResult, rightResult);
        break;
    case TOKEN_AndAlso:
    case TOKEN_OrElse:
        {
        // Short-circuit operators do not evaluate righthand expression unless required.
        status = GetLeftP()->GetValue (leftResult, context, false, true);
        bool leftBool;
        if (ExprStatus_Success == status && ExprStatus_Success == (status = leftResult.GetBoolean (leftBool, false)))
            {
            if (leftBool == (TOKEN_AndAlso == m_operatorCode))
                {
                // OrElse and lefthand expr is false, or AndAlso and righthand expr is true.
                status = GetRightP()->GetValue (rightResult, context, false, true);
                }
            }

        if (ExprStatus_Success == status)
            {
            leftResult.GetECValueR().SetBoolean (leftBool);
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
ExpressionStatus  BitWiseNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;
    ExpressionStatus    status = GetOperandValues(leftResult, rightResult, context);

    if (ExprStatus_Success != status)
        return status;

    status = Operations::PerformArithmeticPromotion(leftResult, rightResult, false);
    if (ExprStatus_Success != status)
        return status;

    return Operations::PerformJunctionOperator(evalResult, _GetOperation(), leftResult, rightResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus BinaryNode::GetOperandValues(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context)
    {
    ExpressionStatus    status = m_left->GetValue(leftResult, context, false, true);
    if (ExprStatus_Success != status)
        return status;

    return m_right->GetValue(rightResult, context, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus BinaryNode::PromoteCommon(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context, bool allowStrings)
    {
    ECN::ECValueR    left    = leftResult.GetECValueR();
    ECN::ECValueR    right   = rightResult.GetECValueR();

    if (!left.IsPrimitive() || !right.IsPrimitive())
        return ExprStatus_PrimitiveRequired;

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
                return ExprStatus_Success;
            case PRIMITIVETYPE_String:
                return allowStrings ? ExprStatus_Success : ExprStatus_WrongType;
            }

        return ExprStatus_WrongType;
        }

    if (PRIMITIVETYPE_String == leftCode || PRIMITIVETYPE_String == rightCode)
        {
        if (!allowStrings)
            return ExprStatus_IncompatibleTypes;

        if (TOKEN_Plus != m_operatorCode)
            return ExprStatus_IncompatibleTypes;

        if (leftCode == rightCode)
            return ExprStatus_Success;

        //  Convert to strings; may want to involve the extended type in that
        return ExprStatus_IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Long == leftCode || PRIMITIVETYPE_Long == rightCode)
        {
        if (leftCode == rightCode)
            return ExprStatus_Success;

        ECValueR            target = PRIMITIVETYPE_Long == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Long == leftCode ? rightCode : leftCode;
        if (PRIMITIVETYPE_Double == targetCode)
            {
            target.SetLong((Int64)target.GetDouble());
            return ExprStatus_Success;
            }

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetLong(target.GetInteger());
            return ExprStatus_Success;
            }

        return ExprStatus_IncompatibleTypes;
        }

    if (PRIMITIVETYPE_Double == leftCode || PRIMITIVETYPE_Double == rightCode)
        {
        //  Both must be doubles
        if (leftCode == rightCode)
            return ExprStatus_Success;

        ECValueR            target = PRIMITIVETYPE_Double == leftCode ? right : left;
        ECN::PrimitiveType   targetCode = PRIMITIVETYPE_Double == leftCode ? rightCode : leftCode;

        if (PRIMITIVETYPE_Integer == targetCode)
            {
            target.SetDouble(target.GetInteger());
            return ExprStatus_Success;
            }

        return ExprStatus_IncompatibleTypes;
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
        {
        return ExprStatus_Success;
        }

    return ExprStatus_WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ExponentNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExprStatus_Success != status)
        return status;

    return Operations::PerformExponentiation(evalResult, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MultiplyNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExprStatus_Success != status)
        return status;

    return Operations::PerformMultiplication(evalResult, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DivideNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    left;
    EvaluationResult    right;
    ExpressionStatus status = GetOperandValues(left, right, context);

    if (ExprStatus_Success != status)
        return status;

    switch(m_operatorCode)
        {
        case TOKEN_IntegerDivide:
            return Operations::PerformIntegerDivision(evalResult, left, right);
        case TOKEN_Slash:
            return Operations::PerformDivision(evalResult, left, right);
        case TOKEN_Mod:
            return Operations::PerformMod(evalResult, left, right);
        }

    BeAssert (false && L"bad divide operator");
    return ExprStatus_UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PlusMinusNode::_Promote(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context)
    {
    return PromoteCommon(leftResult, rightResult, context, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+-----*/
ExpressionStatus PlusMinusNode::_PerformOperation(EvaluationResultR evalResult, EvaluationResultCR leftResult, EvaluationResultCR rightResult, ExpressionContextR context)
    {
    ECN::ECValueCR   left    = leftResult.GetECValue();
    ECN::ECValueCR   right   = rightResult.GetECValue();
    ECN::ECValueR    result  = evalResult.GetECValueR();

    if (m_operatorCode == TOKEN_Plus)
        {
        switch(left.GetPrimitiveType())
            {
            case PRIMITIVETYPE_String:
                performConcatenation (evalResult, left, right);
                return ExprStatus_Success;

            case PRIMITIVETYPE_Long:
                result.SetLong(left.GetLong() + right.GetLong());
                return ExprStatus_Success;

            case PRIMITIVETYPE_Integer:
                result.SetInteger(left.GetInteger() + right.GetInteger());
                return ExprStatus_Success;

            case PRIMITIVETYPE_Double:
                result.SetDouble(left.GetDouble() + right.GetDouble());
                return ExprStatus_Success;
            }
        BeAssert (false && L"unexpected types for addition");
        return ExprStatus_UnknownError;
        }

    switch(left.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Long:
            result.SetLong(left.GetLong() - right.GetLong());
            return ExprStatus_Success;

        case PRIMITIVETYPE_Integer:
            result.SetInteger(left.GetInteger() - right.GetInteger());
            return ExprStatus_Success;

        case PRIMITIVETYPE_Double:
            result.SetDouble(left.GetDouble() - right.GetDouble());
            return ExprStatus_Success;
        }

    BeAssert (false && L"unexpected types for subtraction");

    return ExprStatus_UnknownError;
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
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ComparisonNode::_GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    leftResult;
    EvaluationResult    rightResult;

    ExpressionStatus status = GetOperandValues(leftResult, rightResult, context);
    
    if (ExprStatus_Success != status)
        return status;

    status = PromoteCommon(leftResult, rightResult, context, true);

    if (ExprStatus_Success != status)
        return status;

    if (TOKEN_Like == m_operatorCode)
        return ExprStatus_NotImpl;

    ECValueCR   ecLeft      = leftResult.GetECValue();
    ECValueCR   ecRight     = rightResult.GetECValue();
    if (ecLeft.IsNull() || ecRight.IsNull())
        {
        //  Maybe the not's should be true for this
        evalResult.GetECValueR().SetBoolean(false);
        return ExprStatus_Success;
        }

    //   Promoted, so one string => both strings
    if (ecLeft.IsString())
        {
        wchar_t const*  leftString   = ecLeft.GetString();
        wchar_t const*  rightString  = ecRight.GetString();
        int             intResult = wcscmp(leftString, rightString);
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

        evalResult.GetECValueR().SetBoolean(boolResult);
        return ExprStatus_Success;
        }

    switch (ecLeft.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            evalResult.GetECValueR().SetBoolean(PerformCompare(ecLeft.GetBoolean(), m_operatorCode, ecRight.GetBoolean()));
            return ExprStatus_Success;
        case PRIMITIVETYPE_Double:
            evalResult.GetECValueR().SetBoolean(PerformCompare(ecLeft.GetDouble(), m_operatorCode, ecRight.GetDouble()));
            return ExprStatus_Success;
        case PRIMITIVETYPE_Integer:
            evalResult.GetECValueR().SetBoolean(PerformCompare(ecLeft.GetInteger(), m_operatorCode, ecRight.GetInteger()));
            return ExprStatus_Success;
        case PRIMITIVETYPE_Long:
            evalResult.GetECValueR().SetBoolean(PerformCompare(ecLeft.GetLong(), m_operatorCode, ecRight.GetLong()));
            return ExprStatus_Success;
        }
    
    return ExprStatus_WrongType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Node::GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                    bool allowUnknown, bool allowOverrides)
    { 
    return _GetValue(evalResult, context, allowUnknown, allowOverrides); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus Node::GetValue(ValueResultPtr& valueResult, ExpressionContextR context, 
                                    bool allowUnknown, bool allowOverrides)
    {
    EvaluationResult    evalResult;

    ExpressionStatus    status = GetValue(evalResult, context, allowUnknown, allowOverrides);
    valueResult = ValueResult::Create(evalResult);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationResultR EvaluationResult::operator= (ECN::ECValueCR rhs)
    {
    m_ecValue.Clear();
    m_ecValue = rhs;
    m_valueType = ValType_ECValue;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ValueResult::GetECValue (ECN::ECValueR ecValue)
    {
    return m_evalResult.GetECValue(ecValue);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
