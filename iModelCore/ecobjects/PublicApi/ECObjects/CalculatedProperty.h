/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/CalculatedProperty.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertySpecification : RefCountedBase
    {
private:
    NodePtr                     m_expression;
    ParserRegexP                m_parserRegex;
    bvector<WString>            m_requiredSymbolSets;
    ECValue                     m_failureValue;
    bool                        m_isDefaultOnly;
    bool                        m_useLastValidOnFailure;
    PrimitiveType               m_propertyType;
    ExpressionContextPtr        m_context;
    InstanceExpressionContextP  m_thisContext;

    CalculatedPropertySpecification (NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue);
    ~CalculatedPropertySpecification();
public:
    // Attempts to evaluate the property value. Regardless of the return value, newValue will contain the proper result based on flags like UseLastValidOnFailure, FailureValue, and IsDefaultValueOnly
    ECObjectsStatus         Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance) const;

    static RefCountedPtr<CalculatedPropertySpecification> Create (PrimitiveECPropertyCR ecprop);
    };

typedef RefCountedPtr<CalculatedPropertySpecification> CalculatedPropertySpecificationPtr;

END_BENTLEY_EC_NAMESPACE


