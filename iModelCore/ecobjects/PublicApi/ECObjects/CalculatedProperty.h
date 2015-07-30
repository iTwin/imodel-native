/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/CalculatedProperty.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertySpecification : RefCountedBase
    {
private:
    NodePtr                     m_expression;
    ParserRegexP                m_parserRegex;
    ECValue                     m_failureValue;
    bool                        m_isDefaultOnly;
    bool                        m_useLastValidOnFailure;
    PrimitiveType               m_propertyType;
    ExpressionContextPtr        m_context;
    InstanceExpressionContextP  m_thisContext;
    EvaluationOptions           m_evaluationOptions;

    CalculatedPropertySpecification (NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue);
    ~CalculatedPropertySpecification();
public:
    // Attempts to evaluate the property value. Regardless of the return value, newValue will contain the proper result based on flags like UseLastValidOnFailure, FailureValue, and IsDefaultValueOnly
    ECOBJECTS_EXPORT ECObjectsStatus    Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance, Utf8CP accessString) const;

    // Attempts to apply the ParserRegularExpression attribute to dependent properties when setting the calculated property to a new value
    ECOBJECTS_EXPORT ECObjectsStatus    UpdateDependentProperties (ECValueCR calculatedValue, IECInstanceR instance) const;

    bool                    IsReadOnly() const { return NULL == m_parserRegex && !m_isDefaultOnly; }

    ECOBJECTS_EXPORT static RefCountedPtr<CalculatedPropertySpecification> Create (ECPropertyCR hostProperty, PrimitiveType propertyType);
    };

typedef RefCountedPtr<CalculatedPropertySpecification> CalculatedPropertySpecificationPtr;

END_BENTLEY_ECOBJECT_NAMESPACE


