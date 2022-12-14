/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>

// static ECN::IECSymbolProvider::ExternalSymbolPublisher   s_externalSymbolPublisher; *** unused variable

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// Extract a single argument from the list at the specified index
template<typename T>
static bool ExtractArg(T& str, ECN::EvaluationResultVector const& args, size_t index, bool allowNull)
    {
    return index < args.size() ? SystemSymbolProvider::ExtractArg(str, args[index], allowNull) : false;
    }

template<typename T>
static bool ExtractArg(T& extractedValue, ECN::EvaluationResultVector const& args, size_t index)
    {
    return index < args.size() ? SystemSymbolProvider::ExtractArg(extractedValue, args[index]) : false;
    }

#ifdef NOT_USED
// Extract a DPoint3d from 3 numeric arguments beginning at startIndex
static bool ExtractArg(DPoint3d& p, ECN::EvaluationResultVector const& args, size_t startIndex)
    {
    return ExtractArg(p.x, args, startIndex) && ExtractArg(p.y, args, startIndex + 1) && ExtractArg(p.z, args, startIndex + 2);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(WCharCP& str, EvaluationResultCR ev, bool allowNull)
    {
    str = NULL;
    ECValueCP v = ev.GetECValue();
    if (NULL == v || v->IsNull())
        return allowNull;
    else if (!v->IsString())
        return false;

    str = v->GetWCharCP();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(WStringR str, EvaluationResultCR ev, bool allowNull)
    {
    WCharCP strP;
    if (SystemSymbolProvider::ExtractArg(strP, ev, allowNull))
        {
        str = strP ? strP : L"";
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(Utf8CP& str, EvaluationResultCR ev, bool allowNull)
    {
    str = NULL;
    ECValueCP v = ev.GetECValue();
    if (NULL == v || v->IsNull())
        return allowNull;
    else if (!v->IsString())
        return false;

    str = v->GetUtf8CP();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(Utf8StringR str, EvaluationResultCR ev, bool allowNull)
    {
    Utf8CP strP;
    if (SystemSymbolProvider::ExtractArg(strP, ev, allowNull))
        {
        str = strP ? strP : "";
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(double& d, EvaluationResultCR ev)
    {
    ECValueCP v = ev.GetECValue();
    if (NULL == v || v->IsNull())
        return false;
    else if (v->IsDouble())
        {
        d = v->GetDouble();
        return true;
        }
    else if (v->IsInteger() || v->IsLong())
        {
        ECValue dv(*v);
        if (dv.ConvertToPrimitiveType(PRIMITIVETYPE_Double))
            {
            d = dv.GetDouble();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(int32_t& i, EvaluationResultCR ev)
    {
    if (!ev.IsECValue())
        return false;

    ECValueCR v = *ev.GetECValue();
    if (v.IsNull())
        return false;
    else if (v.IsInteger())
        {
        i = v.GetInteger();
        return true;
        }
    else if (v.IsDouble() || v.IsLong())
        {
        ECValue vv(v);
        if (vv.ConvertToPrimitiveType(PRIMITIVETYPE_Integer))
            {
            i = vv.GetInteger();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SystemSymbolProvider::ExtractArg(LambdaValueCP& lambda, EvaluationResultCR ev)
    {
    lambda = ev.GetLambda();
    return ev.IsLambda();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringMethods
    {
    static ExpressionStatus compare(EvaluationResult& evalResult, EvaluationResultVector& args, bool ignoreCase)
        {
        Utf8String a, b;
        if (ExtractArg(a, args, 0, true) && ExtractArg(b, args, 1, true))
            {
            evalResult.InitECValue().SetBoolean(ignoreCase ? a.EqualsI(b) : a.Equals(b));
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus Compare(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return compare(evalResult, args, false);
        }

    static ExpressionStatus CompareI(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return compare(evalResult, args, true);
        }

    static ExpressionStatus convertCase(EvaluationResult& evalResult, EvaluationResultVector& args, Utf8StringR (Utf8String::*transformFunc)())
        {
        Utf8String str;
        if (ExtractArg(str, args, 0, true))
            {
            (str.*transformFunc)();
            evalResult.InitECValue().SetUtf8CP(str.c_str());
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus ToUpper(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return convertCase(evalResult, args, &Utf8String::ToUpper);
        }

    static ExpressionStatus ToLower(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return convertCase(evalResult, args, &Utf8String::ToLower);
        }

    static Utf8CP findLastOccurrence(Utf8CP str, Utf8CP token)
        {
        Utf8String s(str);
        size_t foundPos = s.rfind(token);
        return -1 != foundPos ? str + foundPos : NULL;
        }

    static ExpressionStatus IndexOf(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8CP str, token;
        if (ExtractArg(str, args, 0, true) && ExtractArg(token, args, 1, true))
            {
            Utf8CP found;
            int32_t index = (str && token && NULL != (found = strstr(str, token))) ? (int32_t)(found - str) : -1;
            evalResult.InitECValue().SetInteger(index);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus LastIndexOf(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8CP str, token;
        if (ExtractArg(str, args, 0, true) && ExtractArg(token, args, 1, true))
            {
            Utf8CP found;
            int32_t index = (str && token && NULL != (found = findLastOccurrence(str, token))) ? (int32_t)(found - str) : -1;
            evalResult.InitECValue().SetInteger(index);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus Contains(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        ExpressionStatus status = IndexOf(evalResult, context, args);
        if (ExpressionStatus::Success == status)
            evalResult.GetECValue()->SetBoolean(evalResult.GetECValue()->GetInteger() >= 0);

        return status;
        }

    static ExpressionStatus ContainsI(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8String str, token;
        if (ExtractArg(str, args, 0, true) && ExtractArg(token, args, 1, true))
            {
            str.ToUpper();
            token.ToUpper();
            evalResult.InitECValue().SetBoolean(-1 != str.find(token));
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus ToString(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        if (args.size() == 1 && args[0].IsECValue())
            {
            evalResult.InitECValue().SetUtf8CP(args[0].GetECValue()->ToString().c_str());
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus Length(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8CP str;
        if (ExtractArg(str, args, 0, true))
            {
            evalResult.InitECValue().SetInteger(str ? (int32_t)strlen(str) : 0);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus SubString(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8String str;
        int32_t startIndex, length;
        if (ExtractArg(str, args, 0, true) && ExtractArg(startIndex, args, 1) && ExtractArg(length, args, 2))
            {
            if (str.empty() || startIndex < 0 || (size_t)startIndex >= str.length() || length < 0)
                evalResult.InitECValue().SetUtf8CP("");
            else
                evalResult.InitECValue().SetUtf8CP(str.substr(startIndex, length).c_str());

            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

#ifdef TODO
    static ExpressionStatus Format (EvaluationResult& evalResult, EvaluationResultVector& args)
        {
        struct ValueList : IECInteropStringFormatter::IECValueList
            {
            EvaluationResultVector const& m_values;
            ValueList (EvaluationResultVector const& values) : m_values(values) { }

            virtual uint32_t      GetCount() const override                   { return (uint32_t)m_values.size() - 1; }
            virtual ECValueCP   operator[](uint32_t index) const override     { return index < GetCount() ? m_values[index+1].GetECValue() : NULL; }
            };

        Utf8CP fmtString;
        if (SystemSymbolProvider::ExtractArg (fmtString, args, 0, true))
            {
            Utf8String formatted;
            if (DgnECManager::GetManager().GetInteropStringFormatter().Format (formatted, fmtString, ValueList (args)))
                {
                evalResult.InitECValue().SetString (formatted.c_str());
                return ExpressionStatus::Success;
                }
            }

        return ExpressionStatus::UnknownError;
        }
#endif

    static ExpressionStatus Trim(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        Utf8String str;
        if (ExtractArg(str, args, 0, true))
            {
            str.Trim();
            evalResult.InitECValue().SetUtf8CP(str.c_str());
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static void PublishSymbols(SymbolExpressionContextR systemContext)
        {
        SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);
        methodContext->AddSymbol(*MethodSymbol::Create("Length", Length, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Compare", Compare, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("CompareI", CompareI, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("ToUpper", ToUpper, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("ToLower", ToLower, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("IndexOf", IndexOf, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("LastIndexOf", LastIndexOf, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Contains", Contains, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("ContainsI", ContainsI, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("ToString", ToString, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("SubString", SubString, NULL));
#ifdef TODO
        methodContext->AddSymbol (*MethodSymbol::Create ("Format", Format, NULL));
#endif
        methodContext->AddSymbol(*MethodSymbol::Create("Trim", Trim, NULL));

        systemContext.AddSymbol(*ContextSymbol::CreateContextSymbol("String", *methodContext));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeMethods
    {
    static ExpressionStatus Now(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        uint64_t unixMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        DateTime dt;
        DateTime::FromUnixMilliseconds(dt, (int64_t)unixMillis);
        evalResult.InitECValue().SetDateTime(dt);
        return ExpressionStatus::Success;
        }

    static void PublishSymbols(SymbolExpressionContextR systemContext)
        {
        SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);
        methodContext->AddSymbol(*MethodSymbol::Create("Now", Now, NULL));

        systemContext.AddSymbol(*ContextSymbol::CreateContextSymbol("DateTime", *methodContext));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct PathMethods
    {
    // GetFileName
    // GetDirectoryName - should not include terminating separator
    // GetExtension - should include the '.'
    // GetFullPath - uses present working directory
    // GetFileNameWithoutExtension
    // Combine

    static ExpressionStatus getFileNamePart(EvaluationResult& evalResult, EvaluationResultVector& args, WString(*extractFunc)(WCharCP))
        {
        WCharCP arg;
        if (ExtractArg(arg, args, 0, false))
            {
            evalResult.InitECValue().SetWCharCP(extractFunc(arg).c_str());
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus GetFileName(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return getFileNamePart(evalResult, args, BeFileName::GetFileNameAndExtension);
        }

    static ExpressionStatus GetDirectoryName(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        ExpressionStatus status = getFileNamePart(evalResult, args, BeFileName::GetDirectoryName);
        if (ExpressionStatus::Success == status)
            {
            // Do not include terminating separator
            Utf8CP ret = evalResult.GetECValue()->GetUtf8CP();
            if (ret && *ret)
                {
                size_t len = strlen(ret);
                if (DIR_SEPARATOR_CHAR == ret[len - 1])
                    evalResult.GetECValue()->SetUtf8CP(Utf8String(ret, len - 1).c_str());
                }
            }

        return status;
        }

    static ExpressionStatus GetExtension(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        ExpressionStatus status = getFileNamePart(evalResult, args, BeFileName::GetExtension);
        if (ExpressionStatus::Success == status)
            {
            // Add the dot '.'
            Utf8CP ret = evalResult.GetECValue()->GetUtf8CP();
            if (ret && *ret)
                {
                Utf8String ext(".");

                ext.append(ret);
                evalResult.GetECValue()->SetUtf8CP(ext.c_str());
                }
            }

        return status;
        }

    static ExpressionStatus GetFileNameWithoutExtension(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        return getFileNamePart(evalResult, args, BeFileName::GetFileNameWithoutExtension);
        }

    static ExpressionStatus GetFullPath(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        WCharCP arg;
        WString fullPath;
        if (ExtractArg(arg, args, 0, false) && BeFileNameStatus::Success == BeFileName::BeGetFullPathName(fullPath, arg))
            {
            evalResult.InitECValue().SetWCharCP(fullPath.c_str());
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus Combine(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        WCharCP nextPart;
        if (!ExtractArg(nextPart, args, 0, false))
            return ExpressionStatus::UnknownError;

        BeFileName filename(nextPart);
        for (size_t i = 1; i < args.size(); i++)
            {
            if (!ExtractArg(nextPart, args, i, false))
                return ExpressionStatus::UnknownError;

            filename.AppendToPath(nextPart);
            }

        evalResult.InitECValue().SetWCharCP(filename);
        return ExpressionStatus::Success;
        }

    static void PublishSymbols(SymbolExpressionContextR systemContext)
        {
        SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);
        methodContext->AddSymbol(*MethodSymbol::Create("GetFileName", GetFileName, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("GetDirectoryName", GetDirectoryName, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("GetExtension", GetExtension, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("GetFileNameWithoutExtension", GetFileNameWithoutExtension, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("GetFullPath", GetFullPath, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Combine", Combine, NULL));

        systemContext.AddSymbol(*ContextSymbol::CreateContextSymbol("Path", *methodContext));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct MathMethods
    {
    // All methods take double(s) except where specified. All angles in radians.
    // E
    // PI
    // Acos
    // Asin
    // Atan
    // Atan2
    // BigMul(int32_t, int32_t)-->Int64
    // Cos
    // Cosh
    // Exp
    // IEEERemainder
    // Abs
    // Floor
    // Ceiling
    // Log
    // Max(T, T)-->T
    // Min(T, T)-->T
    // Round
    // Pow
    // Log10
    // Sin
    // Sinh
    // Sqrt
    // Tan
    // Tanh

    static ExpressionStatus eval1(EvaluationResult& evalResult, EvaluationResultVector& args, double(*func)(double))
        {
        double arg;
        if (ExtractArg(arg, args, 0))
            {
            evalResult.InitECValue().SetDouble(func(arg));
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus eval2(EvaluationResult& evalResult, EvaluationResultVector& args, double(*func)(double, double))
        {
        double arg1, arg2;
        if (ExtractArg(arg1, args, 0) && ExtractArg(arg2, args, 1))
            {
            evalResult.InitECValue().SetDouble(func(arg1, arg2));
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static double roundToEven(double val)
        {
        double sign = val < 0 ? -1.0 : 1.0;
        val += 0.5 * sign;
        double intpart;
        double fractionalpart = modf(val, &intpart);
        if (DoubleOps::AlmostEqual(0.0, fractionalpart))
            return 0.0 == fmod(intpart, 2.0) ? intpart : intpart + (sign * -1.0);
        else
            return intpart;
        }

    static double ieeeremainder(double dividend, double divisor)
        {
        return dividend - (divisor * roundToEven(dividend / divisor));
        }

    static ExpressionStatus Acos(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, acos); }
    static ExpressionStatus Asin(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, asin); }
    static ExpressionStatus Atan(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, atan); }
    static ExpressionStatus Atan2(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)            { return eval2(evalResult, args, atan2); }
    static ExpressionStatus Cos(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, cos); }
    static ExpressionStatus Cosh(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, cosh); }
    static ExpressionStatus Exp(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, exp); }
    static ExpressionStatus IEEERemainder(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)    { return eval2(evalResult, args, ieeeremainder); }
    static ExpressionStatus Abs(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, fabs); }
    static ExpressionStatus Floor(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)            { return eval1(evalResult, args, floor); }
    static ExpressionStatus Ceiling(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)          { return eval1(evalResult, args, ceil); }
    static ExpressionStatus Log(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, log); }
    static ExpressionStatus Pow(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval2(evalResult, args, pow); }
    static ExpressionStatus Log10(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)            { return eval1(evalResult, args, log10); }
    static ExpressionStatus Sin(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, sin); }
    static ExpressionStatus Sinh(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, sinh); }
    static ExpressionStatus Sqrt(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, sqrt); }
    static ExpressionStatus Tan(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return eval1(evalResult, args, tan); }
    static ExpressionStatus Tanh(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)             { return eval1(evalResult, args, tanh); }
    static ExpressionStatus Round(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)            { return eval1(evalResult, args, roundToEven); }

    static ExpressionStatus minOrMax(EvaluationResult& evalResult, EvaluationResultVector& args, bool doMax)
        {
        double a, b;
        if (ExtractArg(a, args, 0) && ExtractArg(b, args, 1))
            {
            double result = doMax ? (a > b ? a : b) : (a < b ? a : b);
            evalResult.InitECValue().SetDouble(result);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus Min(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return minOrMax(evalResult, args, false); }
    static ExpressionStatus Max(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)              { return minOrMax(evalResult, args, true); }

    static ExpressionStatus almostEqual(EvaluationResult& evalResult, EvaluationResultVector& args)
        {
        double a, b;
        if (ExtractArg(a, args, 0) && ExtractArg(b, args, 1))
            {
            bool result = DoubleOps::AlmostEqual(a, b);
            evalResult.InitECValue().SetBoolean(result);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static ExpressionStatus AlmostEqual(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)      { return almostEqual(evalResult, args); }

    static ExpressionStatus BigMul(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        int32_t a, b;
        if (ExtractArg(a, args, 0) && ExtractArg(b, args, 1))
            {
            evalResult.InitECValue().SetLong((int64_t)a * (int64_t)b);
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::UnknownError;
        }

    static void PublishSymbols(SymbolExpressionContextR systemContext)
        {
        SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);

        methodContext->AddSymbol(*MethodSymbol::Create("Acos", Acos, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Asin", Asin, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Atan", Atan, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Atan2", Atan2, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Cos", Cos, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Cosh", Cosh, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Exp", Exp, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("IEEERemainder", IEEERemainder, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Abs", Abs, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Floor", Floor, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Ceiling", Ceiling, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Log", Log, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Pow", Pow, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Log10", Log10, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Sin", Sin, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Sinh", Sinh, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Sqrt", Sqrt, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Tan", Tan, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Tanh", Tanh, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Min", Min, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Max", Max, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("AlmostEqual", AlmostEqual, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("Round", Round, NULL));
        methodContext->AddSymbol(*MethodSymbol::Create("BigMul", BigMul, NULL));

        methodContext->AddSymbol(*ValueSymbol::Create("PI", ECValue(msGeomConst_pi)));
        methodContext->AddSymbol(*ValueSymbol::Create("E", ECValue(exp(1.0))));

        systemContext.AddSymbol(*ContextSymbol::CreateContextSymbol("Math", *methodContext));
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SystemSymbolProvider::SystemSymbolProvider()
    {
    SymbolExpressionContextPtr systemContext = SymbolExpressionContext::Create(NULL);

    StringMethods::PublishSymbols(*systemContext);
    DateTimeMethods::PublishSymbols(*systemContext);
    PathMethods::PublishSymbols(*systemContext);
    MathMethods::PublishSymbols(*systemContext);
    m_systemNamespaceSymbol = ContextSymbol::CreateContextSymbol("System", *systemContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus IsNull(EvaluationResult& evalResult, void*, EvaluationResultVector& arguments)
    {
    evalResult.InitECValue().SetBoolean(false);
    for (EvaluationResultCR arg : arguments)
        {
        if (arg.IsECValue() && !arg.GetECValue()->IsNull())
            return ExpressionStatus::Success;
        }
    evalResult.GetECValue()->SetBoolean(true);
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SystemSymbolProvider::_PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const
    {
    // Note: managed impl publishes all symbols if requestedSymbolSets is empty, otherwise it publishes only those sets which are requested
    // There doesn't seem to be a reason to limit the set of published symbols, and we avoid having to do any further processing/allocation
    // by simply always publishing the full set of symbols.
    context.AddSymbol(*m_systemNamespaceSymbol);

    context.AddSymbol(*MethodSymbol::Create("IsNull", &IsNull, nullptr, nullptr));

    context.AddSymbol(*MethodSymbol::Create("AnyMatches", &SystemSymbolProvider::AnyMatches));
    context.AddSymbol(*MethodSymbol::Create("AllMatch", &SystemSymbolProvider::AllMatch));
    }

/*---------------------------------------------------------------------------------**//**
* Handles AnyMatches() and AllMatch() list methods
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct PredicateProcessor : LambdaValue::IProcessor
    {
private:
    bool                    m_valueToStopOn;
    bool                    m_ignoreErrors;
    EvaluationResult&       m_result;
    ExpressionStatus        m_status;

    PredicateProcessor (bool valueToStopOn, bool ignoreErrors, EvaluationResult& result)
        : m_valueToStopOn(valueToStopOn), m_ignoreErrors(ignoreErrors), m_result(result), m_status(ExpressionStatus::Success)
        {}

    bool ProcessResult (ExpressionStatus status, EvaluationResultCR member, EvaluationResultCR result) override
        {
        if (ExpressionStatus::Success != status)
            return m_ignoreErrors;  // stop iteration if we're not ignoring errors

        if (!result.IsECValue() || !result.GetECValue()->IsBoolean())
            {
            m_status = ExpressionStatus::UnknownError;
            return false;   // stop iteration
            }

        if (!m_result.GetECValue())
            m_result.InitECValue().SetBoolean(!m_valueToStopOn);

        if (m_valueToStopOn == result.GetECValue()->GetBoolean())
            {
            m_result = result;
            return false;   // stop, we found our answer
            }

        return true;    // continue looking
        }

    static ExpressionStatus     Execute (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args, bool target, bool ignoreErrors)
        {
        LambdaValueCP lambda;
        if (!ExtractArg (lambda, args, 0) || NULL == lambda)
            return ExpressionStatus::UnknownError;

        if (0 == valueList.GetCount())
            {
            evalResult.InitECValue().SetBoolean(!target);
            return ExpressionStatus::Success;
            }

        PredicateProcessor proc (target, ignoreErrors, evalResult);
        ExpressionStatus status = lambda->Evaluate (valueList, proc);
        if (ExpressionStatus::Success == status)
            status = proc.m_status;

        return status;
        }
public:
    static ExpressionStatus     Any (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
        {
        return Execute (evalResult, valueList, args, true, true);
        }
    static ExpressionStatus     All (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
        {
        return Execute (evalResult, valueList, args, false, false);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SystemSymbolProvider::AnyMatches(EvaluationResult& evalResult, void* context, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return PredicateProcessor::Any (evalResult, valueList, args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SystemSymbolProvider::AllMatch(EvaluationResult& evalResult, void* context, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return PredicateProcessor::All (evalResult, valueList, args);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
