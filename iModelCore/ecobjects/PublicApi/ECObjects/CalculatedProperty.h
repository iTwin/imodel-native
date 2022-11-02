/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include <ECObjects/ECObjects.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertySpecification : RefCountedBase
    {
private:
    Utf8String                  m_expressionStr;
    NodePtr                     m_expression;
    ParserRegexP                m_parserRegex;
    ECValue                     m_failureValue;
    bool                        m_isDefaultOnly;
    bool                        m_useLastValidOnFailure;
    PrimitiveType               m_propertyType;
    ExpressionContextPtr        m_context;
    InstanceExpressionContextP  m_thisContext;

    CalculatedPropertySpecification(Utf8CP exprStr, NodeR expr, ParserRegexP regex, IECInstanceCR customAttr, PrimitiveType primType, ECValueCR failureValue);
    CalculatedPropertySpecification(Utf8CP exprStr, NodeR expr, ParserRegexP regex, PrimitiveType primType, ECValueCR failureValue);
    ~CalculatedPropertySpecification();
public:
    Utf8CP GetExpression() const {return m_expressionStr.c_str();}

    // Attempts to evaluate the property value. Regardless of the return value, newValue will contain the proper result based on flags like UseLastValidOnFailure, FailureValue, and IsDefaultValueOnly
    ECOBJECTS_EXPORT ECObjectsStatus    Evaluate (ECValueR newValue, ECValueCR existingValue, IECInstanceCR instance, Utf8CP accessString) const;

    // Attempts to apply the ParserRegularExpression attribute to dependent properties when setting the calculated property to a new value
    ECOBJECTS_EXPORT ECObjectsStatus    UpdateDependentProperties (ECValueCR calculatedValue, IECInstanceR instance) const;

    bool                    IsReadOnly() const { return NULL == m_parserRegex && !m_isDefaultOnly; }

    bool IsDefaultOnly() const { return m_isDefaultOnly; }

    ECOBJECTS_EXPORT static RefCountedPtr<CalculatedPropertySpecification> Create(ECPropertyCR hostProperty, PrimitiveType propertyType);

    // This was created for the DgnV8Converter to use.  It only ever needs to calculate a value for the this.GetRelatedInstance() expression and no longer
    // stores a custom attribute on the property.  Therefore, it needs a way to hand calculate the value.
    ECOBJECTS_EXPORT static RefCountedPtr<CalculatedPropertySpecification> Create(Utf8String expression, PrimitiveType primitiveType);
    };

typedef RefCountedPtr<CalculatedPropertySpecification> CalculatedPropertySpecificationPtr;

END_BENTLEY_ECOBJECT_NAMESPACE


