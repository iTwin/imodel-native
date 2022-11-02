/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include    <regex>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

const static Utf8CP NestedCaptureGroup = "NestedCaptureGroup";

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ParserRegex
    {
private:
    std::regex             m_regex;
    bvector<Utf8String>        m_capturedPropertyNames;        // order indicates capture group number - 1

    ParserRegex() { }

    bool ConvertRegexString (Utf8StringR converted, Utf8CP in);
    bool ProcessCaptureGroup (Utf8StringR converted, Utf8CP& in, Utf8CP end, int32_t& depth);
    bool ProcessRegex (Utf8StringR converted, Utf8CP& in, Utf8CP end, int32_t& depth);
public:
    bool                Apply (IECInstanceR instance, Utf8CP calculatedValue) const;

    static ParserRegexP Create (Utf8CP regexStr, bool doNotUseECMA);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ParserRegexP ParserRegex::Create (Utf8CP regexStr, bool doNotUseECMA)
    {
    if (regexStr == NULL)
        return NULL;

    ParserRegexP parserRegex = new ParserRegex();
    Utf8String fixedRegexStr;
    if (parserRegex->ConvertRegexString (fixedRegexStr, regexStr))
        {
        try
            {
            parserRegex->m_regex = std::regex (fixedRegexStr.c_str(), doNotUseECMA ? std::regex_constants::extended : std::regex_constants::ECMAScript);
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ConvertRegexString (Utf8StringR converted, Utf8CP in)
    {
    PRECONDITION (in != NULL, false);

    Utf8CP end = in + strlen (in);
    int32_t depth = 0;
    return ProcessRegex (converted, in, end, depth) && 0 == depth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ProcessRegex (Utf8StringR converted, Utf8CP& in, Utf8CP end, int32_t& depth)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::ProcessCaptureGroup (Utf8StringR converted, Utf8CP& in, Utf8CP end, int32_t& depth)
    {
    if (in >= end)
        return false;

    Utf8Char c = *in;
    if (*in++ != '?')
        {
        in--;
        m_capturedPropertyNames.push_back(NestedCaptureGroup);
        return ProcessRegex(converted, in, end, depth);
        }

    c = *in++;
    if (c == ':')
        {
        // unnamed capture group e.g. "(?:nonCapturingRegex)"
        converted.append ("?:");
        return ProcessRegex (converted, in, end, depth);
        }
    else if (c == '<')
        {
        // named capture group e.g. "(?<propertyName>regex)"
        Utf8CP propNameStart = in;
        Utf8CP propNameEnd = strchr (propNameStart, '>');
        if (NULL != propNameEnd && propNameEnd > propNameStart)
            {
            m_capturedPropertyNames.push_back (Utf8String (propNameStart, propNameEnd));
            in = propNameEnd + 1;
            return ProcessRegex (converted, in, end, depth);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool SETVALUE_SUCCEEDED (ECObjectsStatus status)
    {
    // ###TODO: ECObjectsStatus::PropertyValueMatchesNoChange is always causing problems, because it is not an error but
    // we typically test against ECObjectsStatus::Success.
    // Can we get rid of it?
    return status == ECObjectsStatus::Success || status == ECObjectsStatus::PropertyValueMatchesNoChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ParserRegex::Apply (IECInstanceR instance, Utf8CP calculatedValue) const
    {
    std::match_results<Utf8CP> matches;
    if (!std::regex_match (calculatedValue, matches, m_regex) || matches.size() != m_capturedPropertyNames.size() + 1)
        return false;

    for (size_t i = 0; i < m_capturedPropertyNames.size(); i++)
        {
        if (m_capturedPropertyNames[i].Equals(NestedCaptureGroup))
            continue;

        // ###TODO: we may want to create this stuff once and cache it with the captured property names
        ECValueAccessor accessor;
        if (ECObjectsStatus::Success != ECValueAccessor::PopulateValueAccessor (accessor, instance, m_capturedPropertyNames[i].c_str()))
            return false;

        ECPropertyCP ecprop = accessor.GetECProperty();
        if (NULL == ecprop)
            return false;
        
        PrimitiveType primType;
        if (ecprop->GetIsPrimitive())
            primType = ecprop->GetAsPrimitiveProperty()->GetType();
        else if (ecprop->GetIsPrimitiveArray())
            primType = ecprop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::CalculatedPropertySpecification (Utf8CP exprStr, NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue)
  : m_expression(&expr), m_expressionStr(exprStr), m_parserRegex(regex), m_failureValue(failureValue), m_isDefaultOnly(false), m_useLastValidOnFailure(false), m_propertyType(primType)
    {
    ECValue v;
    if (ECObjectsStatus::Success == customAttr.GetValue (v, "IsDefaultValueOnly") && !v.IsNull())
        m_isDefaultOnly = v.GetBoolean();

    if (ECObjectsStatus::Success == customAttr.GetValue (v, "UseLastValidValueOnFailure") && !v.IsNull())
        m_useLastValidOnFailure = v.GetBoolean();

    uint32_t nSymbolSets;
    bvector<Utf8String> requiredSymbolSets;
    if (ECObjectsStatus::Success == customAttr.GetValue (v, "RequiredSymbolSets") && !v.IsNull() && 0 < ( nSymbolSets = v.GetArrayInfo().GetCount()))
        {
        requiredSymbolSets.reserve (nSymbolSets);
        for (uint32_t i = 0; i < nSymbolSets; i++)
            {
            if (ECObjectsStatus::Success == customAttr.GetValue (v, "RequiredSymbolSets", i) && !v.IsNull())
                requiredSymbolSets.push_back (v.GetUtf8CP());
            }
        }

    InstanceExpressionContextPtr thisContext = InstanceExpressionContext::Create (NULL);
    ContextSymbolPtr thisSymbol = ContextSymbol::CreateContextSymbol ("this", *thisContext);
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (requiredSymbolSets);
    symbolContext->AddSymbol (*thisSymbol);

    m_context = symbolContext.get();
    m_thisContext = thisContext.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::~CalculatedPropertySpecification()
    {
    delete m_parserRegex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationPtr CalculatedPropertySpecification::Create (ECPropertyCR ecprop, PrimitiveType primitiveType)
    {
    IECInstancePtr customAttr = ecprop.GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification");
    if (customAttr.IsNull())
        return NULL;

    ECValue v;
    if (ECObjectsStatus::Success != customAttr->GetValue (v, "ECExpression") || v.IsNull())
        { BeAssert (false && "CalculatedECPropertySpecification must contain an ECExpression"); return NULL; }

    Utf8String expression = v.GetUtf8CP();
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree (expression.c_str());
    if (node.IsNull())
        { BeAssert (false && "Could not parse ECExpression for CalculatedECPropertySpecification"); return NULL; }

    bool isDefaultOnly = (ECObjectsStatus::Success == customAttr->GetValue (v, "IsDefaultValueOnly") && !v.IsNull() && v.GetBoolean());

    // ###TODO: It seems to me that if the calculated property spec is for default value only, then setting the value should not affect dependent properties and we don't require ParserRegex...correct?
    // Note: ParserRegex only makes sense for string properties
    ParserRegexP parserRegex = NULL;
    bool wantParserRegex = !isDefaultOnly && PRIMITIVETYPE_String == primitiveType && !ecprop.IsReadOnlyFlagSet();
    if (wantParserRegex)
        {
        // ###TODO: there is also a configuration variable which can be used to control this...In System.Configuration.ConfigurationManager.AppSettings[] - relevant?
        bool doNotUseECMAScript =  (ECObjectsStatus::Success == customAttr->GetValue (v, "DoNotUseECMAScript") && !v.IsNull() && v.GetBoolean());
        if (ECObjectsStatus::Success == customAttr->GetValue (v, "ParserRegularExpression") && !v.IsNull())
            {
            Utf8CP parserExpression = v.GetUtf8CP();
            if (!Utf8String::IsNullOrEmpty(parserExpression))
                {
                parserRegex = ParserRegex::Create(parserExpression, doNotUseECMAScript);
                if (NULL == parserRegex)
                    {
                    LOG.errorv("A non-read-only non-default CalculatedECPropertySpecification must provide a valid ParserRegularExpression.  Failed to parse '%s' on ECProperty %s:%s", v.GetUtf8CP(), ecprop.GetClass().GetFullName(), ecprop.GetName().c_str());
                    return NULL;
                    }
                }
            }
        }

    ECValue failureValue;
    customAttr->GetValue (failureValue, "FailureValue");
    if (!failureValue.ConvertToPrimitiveType (primitiveType))
        {
        LOG.infov ("Unable to convert failure value %s to primitive type %s\n", failureValue.GetUtf8CP(), ecprop.GetTypeName().c_str());
        failureValue.SetToNull();
        }

    return new CalculatedPropertySpecification (expression.c_str(), *node, parserRegex, *customAttr, primitiveType, failureValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecification::CalculatedPropertySpecification(Utf8CP exprStr, NodeR expr, ParserRegexP regex, PrimitiveType primType, ECValueCR failureValue)
    : m_expression(&expr), m_expressionStr(exprStr), m_parserRegex(regex), m_failureValue(failureValue), m_isDefaultOnly(false), m_useLastValidOnFailure(false), m_propertyType(primType)
    {
    bvector<Utf8String> requiredSymbolSets;
    InstanceExpressionContextPtr thisContext = InstanceExpressionContext::Create(NULL);
    ContextSymbolPtr thisSymbol = ContextSymbol::CreateContextSymbol("this", *thisContext);
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create(requiredSymbolSets);
    symbolContext->AddSymbol(*thisSymbol);

    m_context = symbolContext.get();
    m_thisContext = thisContext.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
CalculatedPropertySpecificationPtr CalculatedPropertySpecification::Create(Utf8String expression, PrimitiveType primitiveType)
    {
    if (expression.empty())
        {
        BeAssert(false && "CalculatedECPropertySpecification must contain an ECExpression"); return NULL;
        }

    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree(expression.c_str());
    if (node.IsNull())
        {
        BeAssert(false && "Could not parse ECExpression for CalculatedECPropertySpecification"); return NULL;
        }

    ECValue failureValue;
    failureValue.SetToNull();
    return new CalculatedPropertySpecification(expression.c_str(), *node, NULL, primitiveType, failureValue);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CalculatedPropertySpecification::Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance, Utf8CP accessString) const
    {
    PRECONDITION (m_expression.IsValid(), ECObjectsStatus::Error);

    ECPropertyCP ecprop = instance.GetEnabler().LookupECProperty (accessString);
    if (nullptr == ecprop)
        return ECObjectsStatus::PropertyNotFound;

    ECObjectsStatus status = ECObjectsStatus::Success;

    if (m_isDefaultOnly && !existingValue.IsNull())
        newValue = existingValue;
    else
        {
        m_thisContext->SetInstance (instance);
        
        ValueResultPtr valueResult;
        ECValue exprValue;
        if (ExpressionStatus::Success == m_expression->GetValue (valueResult, *m_context) && ExpressionStatus::Success == valueResult->GetECValue (exprValue) && !exprValue.IsNull() && exprValue.ConvertToPrimitiveType (m_propertyType))
            {
            newValue = exprValue;
            }
        else
            {
            // Note that if we don't *have* a last valid value, we still return the failure value
            // Also note that we are returning ECObjectsStatus::Success even if expression evaluation failed, because we have successfully produced a value for the calculated property
            newValue = (m_useLastValidOnFailure && !existingValue.IsNull()) ? existingValue : m_failureValue;
            }
        }

    return status;   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CalculatedPropertySpecification::UpdateDependentProperties (ECValueCR v, IECInstanceR instance) const
    {
    if (m_isDefaultOnly)
        return ECObjectsStatus::Success;                        // doesn't apply to dependent properties
    else if (!v.IsString() || v.IsNull())
        return ECObjectsStatus::OperationNotSupported;          // only supported for strings
    else if (NULL == m_parserRegex)
        return ECObjectsStatus::UnableToSetReadOnlyProperty;
    else
        return m_parserRegex->Apply (instance, v.GetUtf8CP()) ? ECObjectsStatus::Success : ECObjectsStatus::ParseError;
    }


END_BENTLEY_ECOBJECT_NAMESPACE

