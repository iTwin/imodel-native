/*--------------------------------------------------------------------------------------+
|
|     $Source: src/CalculatedProperty.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#if defined (_WIN32)    // WIP_NONPORT - regex
    #define HAVE_REGEX
    #include    <regex>
#elif defined (__unix__)
    // regex is coming in C++0x
#endif

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ParserRegex
    {
#if defined (HAVE_REGEX)
private:
    std::tr1::wregex        m_regex;
    bvector<WString>        m_capturedPropertyNames;        // order indicates capture group number - 1

    ParserRegex() { }

    bool ConvertRegexString (WStringR converted, WCharCP in);
    bool ProcessCaptureGroup (WStringR converted, WCharCP& in, WCharCP end, int32_t& depth);
    bool ProcessRegex (WStringR converted, WCharCP& in, WCharCP end, int32_t& depth);
#endif
public:
    bool                Apply (IECInstanceR instance, WCharCP calculatedValue) const;

    static ParserRegexP Create (WCharCP regexStr, bool doNotUseECMA);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ParserRegexP ParserRegex::Create (WCharCP regexStr, bool doNotUseECMA)
    {
#if defined (HAVE_REGEX)
    if (regexStr == NULL)
        return NULL;

    ParserRegexP parserRegex = new ParserRegex();
    WString fixedRegexStr;
    if (parserRegex->ConvertRegexString (fixedRegexStr, regexStr))
        {
        try
            {
            parserRegex->m_regex = std::tr1::wregex (fixedRegexStr.c_str(), doNotUseECMA ? std::tr1::regex_constants::extended : std::tr1::regex_constants::ECMAScript);
            }
        catch (...)
            {
            delete parserRegex;
            parserRegex = NULL;
            }
        }
    else
        {
        delete parserRegex;
        parserRegex = NULL;
        }

    return parserRegex;
#else
    return NULL;
#endif
    }

#if defined (HAVE_REGEX)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ConvertRegexString (WStringR converted, WCharCP in)
    {
    PRECONDITION (in != NULL, false);

    WCharCP end = in + wcslen (in);
    int32_t depth = 0;
    return ProcessRegex (converted, in, end, depth) && 0 == depth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ProcessRegex (WStringR converted, WCharCP& in, WCharCP end, int32_t& depth)
    {
    PRECONDITION (in != NULL && end != NULL, false);

    while (in < end)
        {
        switch (*in)
            {
        case '\\':
            converted.append (1, *in++);
            break;
        case '(':
            converted.append (1, '(');
            depth++;
            if (!ProcessCaptureGroup (converted, ++in, end, depth))
                return false;
            break;
        case ')':
            if (0 < --depth)
                return true;    // parsing an inner group
            }

        if (in < end)
            converted.append (1, *in++);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ProcessCaptureGroup (WStringR converted, WCharCP& in, WCharCP end, int32_t& depth)
    {
    if (in >= end || *in++ != '?')
        return false;
    else if (in >= end)
        return false;

    WChar c = *in++;
    if (c == ':')
        {
        // unnamed capture group e.g. "(?:nonCapturingRegex)"
        converted.append (L"?:");
        return ProcessRegex (converted, in, end, depth);
        }
    else if (c == '<')
        {
        // named capture group e.g. "(?<propertyName>regex)"
        WCharCP propNameStart = in;
        WCharCP propNameEnd = wcschr (propNameStart, '>');
        if (NULL != propNameEnd && propNameEnd > propNameStart)
            {
            m_capturedPropertyNames.push_back (WString (propNameStart, propNameEnd));
            in = propNameEnd + 1;
            return ProcessRegex (converted, in, end, depth);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool SETVALUE_SUCCEEDED (ECObjectsStatus status)
    {
    // ###TODO: ECOBJECTS_STATUS_PropertyValueMatchesNoChange is always causing problems, because it is not an error but
    // we typically test against ECOBJECTS_STATUS_Success.
    // Can we get rid of it?
    return status == ECOBJECTS_STATUS_Success || status == ECOBJECTS_STATUS_PropertyValueMatchesNoChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::Apply (IECInstanceR instance, WCharCP calculatedValue) const
    {
    std::tr1::match_results<WCharCP> matches;
    if (!std::tr1::regex_match (calculatedValue, matches, m_regex) || matches.size() != m_capturedPropertyNames.size() + 1)
        return false;

    for (size_t i = 0; i < m_capturedPropertyNames.size(); i++)
        {
        // ###TODO: we may want to create this stuff once and cache it with the captured property names
        ECValueAccessor accessor;
        if (ECOBJECTS_STATUS_Success != ECValueAccessor::PopulateValueAccessor (accessor, instance, m_capturedPropertyNames[i].c_str()))
            return false;

        ECPropertyCP ecprop = accessor.GetECProperty();
        if (NULL == ecprop)
            return false;
        
        PrimitiveType primType;
        if (ecprop->GetIsPrimitive())
            primType = ecprop->GetAsPrimitiveProperty()->GetType();
        else if (ecprop->GetIsArray())
            primType = ecprop->GetAsArrayProperty()->GetPrimitiveElementType();
        else
            return false;

        ECValue v (matches.str (i+1).c_str());
        if (!v.ConvertToPrimitiveType (primType))
            return false;

        if (!SETVALUE_SUCCEEDED (instance.SetValueUsingAccessor (accessor, v)))
            return false;
        }

    return true;
    }

#endif  // HAVE_REGEX

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::CalculatedPropertySpecification (NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue)
  : m_expression(&expr), m_parserRegex(regex), m_failureValue(failureValue), m_isDefaultOnly(false), m_useLastValidOnFailure(false), m_propertyType(primType), m_evaluationOptions (EVALOPT_Legacy)
    {
    ECValue v;
    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"IsDefaultValueOnly") && !v.IsNull())
        m_isDefaultOnly = v.GetBoolean();

    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"UseLastValidValueOnFailure") && !v.IsNull())
        m_useLastValidOnFailure = v.GetBoolean();

    uint32_t nSymbolSets;
    bvector<WString> requiredSymbolSets;
    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"RequiredSymbolSets") && !v.IsNull() && 0 < ( nSymbolSets = v.GetArrayInfo().GetCount()))
        {
        requiredSymbolSets.reserve (nSymbolSets);
        for (uint32_t i = 0; i < nSymbolSets; i++)
            {
            if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"RequiredSymbolSets", i) && !v.IsNull())
                requiredSymbolSets.push_back (v.GetString());
            }
        }

    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"EnforceUnits") && !v.IsNull() && v.GetBoolean())
        m_evaluationOptions = EVALOPT_EnforceUnits;
    else if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"SuppressTypeConversions") && !v.IsNull() && v.GetBoolean())
        m_evaluationOptions = EVALOPT_SuppressTypeConversions;

    InstanceExpressionContextPtr thisContext = InstanceExpressionContext::Create (NULL);
    ContextSymbolPtr thisSymbol = ContextSymbol::CreateContextSymbol (L"this", *thisContext);
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (requiredSymbolSets);
    symbolContext->AddSymbol (*thisSymbol);

    m_context = symbolContext.get();
    m_thisContext = thisContext.get();
    m_thisContext->SetEvaluationOptions (m_evaluationOptions);
    m_context->SetEvaluationOptions (m_evaluationOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::~CalculatedPropertySpecification()
    {
    delete m_parserRegex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationPtr CalculatedPropertySpecification::Create (ECPropertyCR ecprop, PrimitiveType primitiveType)
    {
    IECInstancePtr customAttr = ecprop.GetCustomAttribute (L"Bentley_Standard_CustomAttributes", L"CalculatedECPropertySpecification");
    if (customAttr.IsNull())
        return NULL;

    ECValue v;
    if (ECOBJECTS_STATUS_Success != customAttr->GetValue (v, L"ECExpression") || v.IsNull())
        { BeAssert (false && "CalculatedECPropertySpecification must contain an ECExpression"); return NULL; }

    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree (v.GetString());
    if (node.IsNull())
        { BeAssert (false && "Could not parse ECExpression for CalculatedECPropertySpecification"); return NULL; }

    bool isDefaultOnly = (ECOBJECTS_STATUS_Success == customAttr->GetValue (v, L"IsDefaultValueOnly") && !v.IsNull() && v.GetBoolean());

    // ###TODO: It seems to me that if the calculated property spec is for default value only, then setting the value should not affect dependent properties and we don't require ParserRegex...correct?
    // Note: ParserRegex only makes sense for string properties
    ParserRegexP parserRegex = NULL;
    bool wantParserRegex = !isDefaultOnly && PRIMITIVETYPE_String == primitiveType && !ecprop.IsReadOnlyFlagSet();
    if (wantParserRegex)
        {
        // ###TODO: there is also a configuration variable which can be used to control this...In System.Configuration.ConfigurationManager.AppSettings[] - relevant?
        bool doNotUseECMAScript =  (ECOBJECTS_STATUS_Success == customAttr->GetValue (v, L"DoNotUseECMAScript") && !v.IsNull() && v.GetBoolean());
        if (ECOBJECTS_STATUS_Success == customAttr->GetValue (v, L"ParserRegularExpression") && !v.IsNull())
            {
            parserRegex = ParserRegex::Create (v.GetString(), doNotUseECMAScript);
            if (NULL == parserRegex)
                { BeAssert (false && "A non-read-only non-default CalculatedECPropertySpecification must provide a valid ParserRegularExpression"); return NULL; }
            }
        }

    ECValue failureValue;
    customAttr->GetValue (failureValue, L"FailureValue");
    if (!failureValue.ConvertToPrimitiveType (primitiveType))
        {
        LOG.infov (L"Unable to convert failure value %ls to primitive type %ls\n", failureValue.GetString(), ecprop.GetTypeName().c_str());
        failureValue.SetToNull();
        }

    return new CalculatedPropertySpecification (*node, parserRegex, *customAttr, primitiveType, failureValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CalculatedPropertySpecification::Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance, WCharCP accessString) const
    {
    PRECONDITION (m_expression.IsValid(), ECOBJECTS_STATUS_Error);

    ECPropertyCP ecprop = instance.GetEnabler().LookupECProperty (accessString);
    if (nullptr == ecprop)
        return ECOBJECTS_STATUS_PropertyNotFound;

    ECObjectsStatus status = ECOBJECTS_STATUS_Success;

    if (m_isDefaultOnly && !existingValue.IsNull())
        newValue = existingValue;
    else
        {
        m_thisContext->SetInstance (instance);
        
        ValueResultPtr valueResult;
        ECValue exprValue;
        bool gotValue = false;
        if (ExprStatus_Success == m_expression->GetValue (valueResult, *m_context) && ExprStatus_Success == valueResult->GetECValue (exprValue) && !exprValue.IsNull() && exprValue.ConvertToPrimitiveType (m_propertyType))
            {
            gotValue = true;
            bool allowTypeConversions = (0 == (m_evaluationOptions & EVALOPT_SuppressTypeConversions));
            bool enforceGlobalRepresentation = (0 != (m_evaluationOptions & EVALOPT_EnforceGlobalRepresentation));
            bool convertUnits = !allowTypeConversions && (0 != (m_evaluationOptions & EVALOPT_EnforceUnits));
            if (allowTypeConversions || enforceGlobalRepresentation || convertUnits)
                {
                IECTypeAdapter* typeAdapter = nullptr;
                if (ecprop->GetIsPrimitive())
                    typeAdapter = ecprop->GetAsPrimitiveProperty()->GetTypeAdapter();
                else if (ecprop->GetIsArray())
                    typeAdapter = ecprop->GetAsArrayProperty()->GetMemberTypeAdapter();

                if (nullptr != typeAdapter)
                    {
                    IECTypeAdapterContextPtr context = IECTypeAdapterContext::Create (*ecprop, instance, accessString);
                    context->SetEvaluationOptions (m_evaluationOptions);

                    if ((allowTypeConversions || enforceGlobalRepresentation) && typeAdapter->RequiresExpressionTypeConversion (m_evaluationOptions) && !typeAdapter->ConvertFromExpressionType (exprValue, *context))
                        {
                        gotValue = false;
                        }
                    else if (convertUnits && typeAdapter->SupportsUnits())
                        {
                        gotValue = false;
                        UnitSpec units;

                        // We only care about units if the ECProperty is unitized...otherwise ignore units of expression result
                        if (typeAdapter->GetUnits (units, *context))
                            {
                            if (units.IsUnspecified())
                                gotValue = true;    // not unitized
                            else
                                {
                                // If expression result has no units, assume same units as ECProperty.
                                UnitSpecCR exprUnits = valueResult->GetResult().GetUnits();
                                if (exprUnits.IsUnspecified() || exprUnits.IsEquivalent (units))
                                    gotValue = true;
                                else if (units.IsCompatible (exprUnits) && exprValue.ConvertToPrimitiveType (PRIMITIVETYPE_Double))
                                    {
                                    double dv = exprValue.GetDouble();
                                    if (exprUnits.ConvertTo (dv, units))
                                        {
                                        exprValue.SetDouble (dv);
                                        gotValue = exprValue.ConvertToPrimitiveType (m_propertyType);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        if (gotValue)
            {
            newValue = exprValue;
            }
        else
            {
            // Note that if we don't *have* a last valid value, we still return the failure value
            // Also note that we are returning ECOBJECTS_STATUS_Success even if expression evaluation failed, because we have successfully produced a value for the calculated property
            newValue = (m_useLastValidOnFailure && !existingValue.IsNull()) ? existingValue : m_failureValue;
            }
        }

    return status;   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CalculatedPropertySpecification::UpdateDependentProperties (ECValueCR v, IECInstanceR instance) const
    {
    if (m_isDefaultOnly)
        return ECOBJECTS_STATUS_Success;                        // doesn't apply to dependent properties
    else if (!v.IsString() || v.IsNull())
        return ECOBJECTS_STATUS_OperationNotSupported;          // only supported for strings
    else if (NULL == m_parserRegex)
        return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;

    else
        return m_parserRegex->Apply (instance, v.GetString()) ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;
    }

#undef HAVE_REGEX

END_BENTLEY_ECOBJECT_NAMESPACE

