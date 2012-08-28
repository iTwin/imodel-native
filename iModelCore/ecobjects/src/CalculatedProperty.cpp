/*--------------------------------------------------------------------------------------+
|
|     $Source: src/CalculatedProperty.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#if defined (_WIN32)    // WIP_NONPORT - regex
    #define HAVE_REGEX
    #include    <regex>
#elif defined (__unix__)
    // regex is coming in C++0x
#endif

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ParserRegex
    {
#if defined (HAVE_REGEX)
private:
    std::tr1::wregex        m_regex;

    ParserRegex (std::tr1::wregex const& regex) : m_regex (regex) { }
public:
    static ParserRegexP Create (WCharCP regexStr, bool doNotUseECMA)
        {
        ParserRegexP parserRegex = NULL;
        try
            {
            std::tr1::wregex regex (regexStr, doNotUseECMA ? std::tr1::regex_constants::basic : std::tr1::regex_constants::ECMAScript);
            parserRegex = new ParserRegex (regex);
            }
        catch (...)
            {
            BeAssert (false);
            }

        return parserRegex;
        }
#else
public:
    static ParserRegexP Create (WCharCP regexStr, bool doNotUseECMA)
        {
        return NULL;
        }
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::CalculatedPropertySpecification (NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue)
  : m_expression(&expr), m_parserRegex(regex), m_failureValue(failureValue), m_isDefaultOnly(false), m_useLastValidOnFailure(false), m_propertyType(primType)
    {
    ECValue v;
    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"IsDefaultValueOnly") && !v.IsNull())
        m_isDefaultOnly = v.GetBoolean();

    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"UseLastValidValueOnFailure") && !v.IsNull())
        m_useLastValidOnFailure = v.GetBoolean();

    UInt32 nSymbolSets;
    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"RequiredSymbolSets") && !v.IsNull() && 0 < ( nSymbolSets = v.GetArrayInfo().GetCount()))
        {
        m_requiredSymbolSets.reserve (nSymbolSets);
        for (UInt32 i = 0; i < nSymbolSets; i++)
            {
            if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"RequiredSymbolSets", i) && !v.IsNull())
                m_requiredSymbolSets.push_back (v.GetString());
            }
        }

    InstanceExpressionContextPtr thisContext = InstanceExpressionContext::Create (NULL);
    ContextSymbolPtr thisSymbol = ContextSymbol::CreateContextSymbol (L"this", *thisContext);
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (NULL);
    symbolContext->AddSymbol (*thisSymbol);

    m_context = symbolContext.get();
    m_thisContext = thisContext.get();
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
CalculatedPropertySpecificationPtr CalculatedPropertySpecification::Create (PrimitiveECPropertyCR ecprop)
    {
    IECInstancePtr customAttr = ecprop.GetCustomAttribute (L"CalculatedECPropertySpecification");
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
    ParserRegexP parserRegex = NULL;
    if (!ecprop.GetIsReadOnly() && !isDefaultOnly)
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
    if (SUCCESS != failureValue.ConvertToPrimitiveType (ecprop.GetType()))
        { BeAssert (false && "Invalid FailureValue in CalculatedECPropertySpecification"); return NULL; }

    return new CalculatedPropertySpecification (*node, parserRegex, *customAttr, ecprop.GetType(), failureValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CalculatedPropertySpecification::Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance) const
    {
    PRECONDITION (m_expression.IsValid(), ECOBJECTS_STATUS_Error);

    ECObjectsStatus status = ECOBJECTS_STATUS_Success;

    if (m_isDefaultOnly && !existingValue.IsNull())
        newValue = existingValue;
    else
        {
        // ###TODO: context, symbols
        m_thisContext->SetInstance (instance);
        
        ValueResultPtr valueResult;
        ECValue exprValue;
        if (ExprStatus_Success == m_expression->GetValue (valueResult, *m_context, false, true) && ExprStatus_Success == valueResult->GetECValue (exprValue) && SUCCESS == exprValue.ConvertToPrimitiveType (m_propertyType))
            newValue = exprValue;
        else
            {
            // Note that if we don't *have* a last valid value, we still return the failure value
            // Also note that we are returning ECOBJECTS_STATUS_Success even if expression evaluation failed, because we have successfully produced a value for the calculated property
            newValue = (m_useLastValidOnFailure && !existingValue.IsNull()) ? existingValue : m_failureValue;
            }
        }

    return status;   
    }

END_BENTLEY_EC_NAMESPACE

#undef HAVE_REGEX

