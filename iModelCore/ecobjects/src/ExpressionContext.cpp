/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ExpressionContext.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceExpressionContextPtr InstanceExpressionContext::Create(ExpressionContextP outer)
    { 
    return new InstanceExpressionContext(outer); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceExpressionContext::_GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    size_t      numberOfOperators = primaryList.GetNumberOfOperators();
    size_t      index = startIndex;
    bool        appendDot = false;
    WString     accessString;
    EC::IECInstancePtr  instance = const_cast<EC::IECInstanceP>(GetInstanceCP());
    EC::ECEnablerCP     enabler = &instance->GetEnabler();
    EC::ECClassCP       ecClass = &enabler->GetClass();


    accessString.reserve(50);

    evalResult.GetECValueR().SetStruct(instance.get());

    if (index == numberOfOperators)
        {
        //  This is an instance expression
        return ExprStatus_UnknownError;    // Report invalid reference
        }

    ExpressionToken        nextOperation = primaryList.GetOperation(index);
    EC::ECPropertyP             currentProperty = NULL;
    
    while (TOKEN_None != nextOperation)
        {
        while (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation)
            {
            assert(NULL != dynamic_cast<IdentNodeCP>(primaryList.GetOperatorNode(index)));

            IdentNodeCP       identNode = static_cast<IdentNodeCP>(primaryList.GetOperatorNode(index++));
            
            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            if (appendDot)
                accessString.append(L".");

            appendDot = true;

            wchar_t const*  memberName = identNode->GetName();
            accessString.append(memberName);

            currentProperty = ecClass->GetPropertyP(memberName);
            if (NULL == currentProperty)
                {
                evalResult.Clear();
                return ExprStatus_UnknownMember;
                }

            if (!currentProperty->GetIsStruct())
                break;

            if (TOKEN_None == nextOperation)
                {
                //  Since we don't allow struct values there must be another operator
                //  We might want to require DOT here. We need to decide
                //  -- will we allow indexing via a property name?
                //  -- will we allow passing an struct to a method?
                evalResult.Clear();
                return ExprStatus_PrimitiveRequired;
                }

            EC::StructECPropertyCP  structProperty = currentProperty->GetAsStructProperty();
            assert (NULL != structProperty);

            ecClass = &structProperty->GetType();
            }

        if (TOKEN_None == nextOperation)
            {
            if (NULL == currentProperty)
                return ExprStatus_UnknownSymbol;

            EC::PrimitiveECPropertyCP   primProp = currentProperty->GetAsPrimitiveProperty();
            if (NULL == primProp)
                {
                evalResult.Clear();
                return ExprStatus_PrimitiveRequired;
                }

            ::UInt32     propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }


            refResult.m_accessString = accessString;
            refResult.m_memberSelector = -1;
            refResult.m_property = currentProperty;

            return ExprStatus_Success;
            }

        if (TOKEN_LParen == nextOperation)
            {
            //  Might want to allow a method to setting properties on an object returned from a method.  For now, just return an error.
            return ExprStatus_UnknownError;
            }

        if (TOKEN_LeftBracket == nextOperation)
            {
            EC::ArrayECPropertyCP   arrayProp = currentProperty->GetAsArrayProperty();

            if (NULL == arrayProp)
                {
                evalResult.Clear();
                return ExprStatus_ArrayRequired;
                }

            LBracketNodeCP       lBracketNode = static_cast<LBracketNodeCP>(primaryList.GetOperatorNode(index++));

            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            EvaluationResult    indexResult;
            NodePtr             indexNode = lBracketNode->GetIndexNode();

            ExpressionStatus exprStatus = indexNode->GetValue(indexResult, globalContext, false, true);
            if (ExprStatus_Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }

            //  Need to convert this to an int if it is not already an int
            UInt32         arrayIndex = (UInt32)indexResult.GetECValue().GetInteger();

            //  May need to get an instance or primitive value.
            if (arrayProp->GetKind() == EC::ARRAYKIND_Primitive)
                {
                if (TOKEN_None != nextOperation)
                    {
                    evalResult.Clear();
                    return ExprStatus_StructRequired;  //  might be method required.  Should check next node
                    }

                accessString.append(L"[]");

                refResult.m_accessString = accessString;
                refResult.m_arrayIndex = arrayIndex;
                refResult.m_property = currentProperty;

                return ExprStatus_Success;
                }

            assert(EC::ARRAYKIND_Struct == arrayProp->GetKind());

            accessString.append(L"[]");
            ::UInt32     propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }

            EC::ECValue         ecValue;
            EC::ECObjectsStatus  ecStatus = instance->GetValue(ecValue, propertyIndex, arrayIndex);

            if (EC::ECOBJECTS_STATUS_Success != ecStatus)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;             //  This should return a good translation of the ECOBJECTS_STATUS_ value
                }

            if (ecValue.IsNull())
                {
                evalResult = ecValue;
                //  Should we initialize the array?
                return ExprStatus_UnknownError;
                }

            if (TOKEN_None == nextOperation)
                {
                //  Returning a struct instance.
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }

            appendDot   = false;
            accessString.clear();
            instance = ecValue.GetStruct().get();
            enabler = &instance->GetEnabler();
            ecClass = &enabler->GetClass();
            evalResult.GetECValueR().SetStruct(instance.get());
            }
        }

    return ExprStatus_UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    size_t      numberOfOperators = primaryList.GetNumberOfOperators();
    size_t      index = startIndex;
    bool        appendDot = false;
    WString     accessString;
    EC::IECInstancePtr  instance = const_cast<EC::IECInstanceP>(GetInstanceCP());
    EC::ECEnablerCP     enabler = &instance->GetEnabler();
    EC::ECClassCP       ecClass = &enabler->GetClass();

    accessString.reserve(50);

    evalResult.GetECValueR().SetStruct(instance.get());

    if (index == numberOfOperators)
        {
        //  This is an instance expression
        return ExprStatus_Success;
        }

    ExpressionToken        nextOperation = primaryList.GetOperation(index);

    while (TOKEN_None != nextOperation)
        {
        EC::ECPropertyP             currentProperty = NULL;

        while (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation)
            {
            assert(NULL != dynamic_cast<IdentNodeCP>(primaryList.GetOperatorNode(index)));

            IdentNodeCP       identNode = static_cast<IdentNodeCP>(primaryList.GetOperatorNode(index++));
            
            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            if (appendDot)
                accessString.append(L".");

            appendDot = true;

            wchar_t const*  memberName = identNode->GetName();
            accessString.append(memberName);

            currentProperty = ecClass->GetPropertyP(memberName);
            if (NULL == currentProperty)
                {
                evalResult.Clear();
                return ExprStatus_UnknownMember;
                }

            if (!currentProperty->GetIsStruct())
                break;

            if (TOKEN_None == nextOperation)
                {
                //  Since we don't allow struct values there must be another operator
                //  We might want to require DOT here. We need to decide
                //  -- will we allow indexing via a property name?
                //  -- will we allow passing an struct t a method?
                evalResult.Clear();
                return ExprStatus_PrimitiveRequired;
                }

            EC::StructECPropertyCP  structProperty = currentProperty->GetAsStructProperty();
            assert (NULL != structProperty);

            ecClass = &structProperty->GetType();
            }

        if (TOKEN_None == nextOperation)
            {
            if (NULL == currentProperty)
                return ExprStatus_UnknownSymbol;

            EC::PrimitiveECPropertyCP   primProp = currentProperty->GetAsPrimitiveProperty();
            if (NULL == primProp)
                {
                evalResult.Clear();
                return ExprStatus_PrimitiveRequired;
                }

            ::UInt32     propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }

            EC::ECValue         ecValue;
            EC::ECObjectsStatus  status = instance->GetValue(ecValue, propertyIndex);

            assert (EC::ECOBJECTS_STATUS_Success == status);

            if (EC::ECOBJECTS_STATUS_Success != status)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }

            evalResult = ecValue;
            return ExprStatus_Success;
            }

        if (TOKEN_LParen == nextOperation)
            {
            if (accessString.size() != 0)
                {
                //  Does not seem worth trying this. There is no way to access a struct independently.
                ::UInt32     propertyIndex;
                if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                    {
                    evalResult.Clear();
                    return ExprStatus_UnknownError;
                    }

                EC::ECValue         ecValue;
                EC::ECObjectsStatus  status = instance->GetValue(ecValue, propertyIndex);

                if (EC::ECOBJECTS_STATUS_Success != status)
                    {
                    evalResult.Clear();
                    return ExprStatus_UnknownError;
                    }

                if (ecValue.GetKind () !=- EC::VALUEKIND_Struct)
                    {
                    evalResult.Clear();
                    return ExprStatus_StructRequired;;
                    }

                EC::IECInstancePtr  valueInstance = ecValue.GetStruct();
                evalResult.GetECValueR().SetStruct(valueInstance.get());
                }

            CallNodeP               callNode = static_cast<CallNodeP>(primaryList.GetOperatorNode(index++));
            nextOperation           = primaryList.GetOperation(index);

            EvaluationResult        methodResult;
            ExpressionStatus exprStatus = callNode->InvokeInstanceMethod(methodResult, evalResult, globalContext);
            if (ExprStatus_Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }

            evalResult = methodResult.GetECValue();

            if (TOKEN_None == nextOperation)
                {
                //  Do we require a primitive here?
                return ExprStatus_Success;
                }

            if (!evalResult.GetECValue().IsStruct())
                {
                evalResult.Clear();
                return ExprStatus_StructRequired;
                }

            appendDot = false;
            accessString.clear();
            instance = evalResult.GetECValue().GetStruct().get();
            enabler = &instance->GetEnabler();
            ecClass = &enabler->GetClass();
            evalResult.GetECValueR().SetStruct(instance.get());
            }

        if (TOKEN_LeftBracket == nextOperation)
            {
            EC::ArrayECPropertyCP   arrayProp = currentProperty->GetAsArrayProperty();

            if (NULL == arrayProp)
                {
                evalResult.Clear();
                return ExprStatus_ArrayRequired;
                }

            LBracketNodeCP       lBracketNode = static_cast<LBracketNodeCP>(primaryList.GetOperatorNode(index++));

            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            EvaluationResult    indexResult;
            NodePtr             indexNode = lBracketNode->GetIndexNode();

            ExpressionStatus exprStatus = indexNode->GetValue(indexResult, globalContext, false, true);
            if (ExprStatus_Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }

            //  Need to convert this to an int if it is not already an int
            UInt32         arrayIndex = (UInt32)indexResult.GetECValue().GetInteger();

            //  May need to get an instance or primitive value.
            if (arrayProp->GetKind() == EC::ARRAYKIND_Primitive)
                {
                if (index < numberOfOperators)
                    {
                    evalResult.Clear();
                    return ExprStatus_StructRequired;  //  might be method required.  Should check next node
                    }

                accessString.append(L"[]");
                ::UInt32     propertyIndex;
                if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                    {
                    evalResult.Clear();
                    return ExprStatus_UnknownError;
                    }

                EC::ECValue         ecValue;
                EC::ECObjectsStatus  ecStatus = instance->GetValue(ecValue, propertyIndex, arrayIndex);

                assert (EC::ECOBJECTS_STATUS_Success == ecStatus);

                if (EC::ECOBJECTS_STATUS_Success != ecStatus)
                    {
                    evalResult.Clear();
                    return ExprStatus_UnknownError;
                    }

                evalResult = ecValue;
                return ExprStatus_Success;
                }

            assert(EC::ARRAYKIND_Struct == arrayProp->GetKind());

            accessString.append(L"[]");
            ::UInt32     propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != EC::ECOBJECTS_STATUS_Success)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;
                }

            EC::ECValue         ecValue;
            EC::ECObjectsStatus  ecStatus = instance->GetValue(ecValue, propertyIndex, arrayIndex);

            if (EC::ECOBJECTS_STATUS_Success != ecStatus)
                {
                evalResult.Clear();
                return ExprStatus_UnknownError;             //  This should return a good translation of the ECOBJECTS_STATUS_ value
                }

            if (ecValue.IsNull())
                {
                evalResult = ecValue;
                return ExprStatus_Success;
                }

            if (TOKEN_None == nextOperation)
                {
                //  Returning a struct instance.
                evalResult = ecValue;
                return ExprStatus_Success;
                }

            appendDot   = false;
            accessString.clear();
            instance = ecValue.GetStruct().get();
            enabler = &instance->GetEnabler();
            ecClass = &enabler->GetClass();
            evalResult.GetECValueR().SetStruct(instance.get());
            }
        }

    return ExprStatus_UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
EC::IECInstanceCP   InstanceExpressionContext::GetInstanceCP() const 
    { 
    return m_instance.get(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            InstanceExpressionContext::SetInstance(EC::IECInstanceCR instance)
    { 
    m_instance = &const_cast<EC::IECInstanceR>(instance); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                MethodReferenceStandard::MethodReferenceStandard(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod)
                : m_staticMethod(staticMethod), m_instanceMethod(instanceMethod)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments)
    {
    if (NULL == m_staticMethod)
        return ExprStatus_InstanceMethodRequired;

    return (*m_staticMethod)(evalResult, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeInstanceMethod (EvaluationResultR evalResult, EvaluationResultCR instanceData, EvaluationResultVector& arguments)
    {
    if (NULL == m_instanceMethod)
        return ExprStatus_StaticMethodRequired;

    return (*m_instanceMethod)(evalResult, instanceData, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferencePtr MethodReferenceStandard::Create(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod)
    {
    return new MethodReferenceStandard(staticMethod, instanceMethod);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                MethodSymbol::MethodSymbol(wchar_t const* name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod)
                : Symbol(name)
    {
    m_methodReference = MethodReferenceStandard::Create(staticMethod, instanceMethod);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbolPtr MethodSymbol::Create(wchar_t const* name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod)
    {
    return new MethodSymbol(name, staticMethod, instanceMethod);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    //  We only get here if the symbol was resolved directly from a context. The context may be a global context, or a namespace context. If the
    //  context is a global context then startIndex is 1 and the callNode should be 0. If the context is a namespace context, then startIndex - 1
    //  should be the context used to find this symbol. It must be a CallNode.
    //
    //  For now assuming this has to be a static method; may want to support having a MethodSymbol in a context. That would require that this
    //  method receive a pointer to the context used to invoke it.

    assert(1 <= startIndex);
    ::UInt32        callNodeIndex = startIndex - 1;

    if (primaryList.GetOperation(callNodeIndex) != TOKEN_LParen)
        {
        evalResult.GetECValueR().Clear();
        return ExprStatus_UnknownError;    // Invalid operation on method
        }

    if (primaryList.GetNumberOfOperators() != startIndex)
        {
        evalResult.GetECValueR().Clear();
        return ExprStatus_UnknownError;    // Invalid operation on method
        }

    CallNodeP   callNode = static_cast<CallNodeP>(primaryList.GetOperatorNode(callNodeIndex));

    return callNode->InvokeStaticMethod(evalResult, *m_methodReference, globalContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SymbolExpressionContext::AddSymbol (SymbolR symbol)
    {
    //  Check for duplicates in context
    m_symbols.push_back(SymbolPtr(&symbol));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolExpressionContextPtr SymbolExpressionContext::Create(ExpressionContextP outer)
    {
    return new SymbolExpressionContext (outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    wchar_t const* ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        return ExprStatus_UnknownSymbol;

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == wcscmp(ident, symbol->GetName()))
            return symbol->GetValue(evalResult, primaryList, globalContext, startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, globalContext, startIndex);

    return ExprStatus_UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetReference(EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    wchar_t const* ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        return ExprStatus_UnknownSymbol;

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == wcscmp(ident, symbol->GetName()))
            return symbol->GetReference(evalResult, refResult, primaryList, globalContext, startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, globalContext, startIndex);

    return ExprStatus_UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_ResolveMethod(MethodReferencePtr& result, wchar_t const* ident, bool useOuterIfNecessary)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == wcscmp(ident, symbol->GetName()))
            return symbol->CreateMethodResult(result);
        }

    if (useOuterIfNecessary)
        {
        ExpressionContextP  outer = GetOuterP();
        if (NULL != outer)
            return outer->ResolveMethod(result, ident, useOuterIfNecessary);
        }

    return ExprStatus_UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    return m_context->GetValue(evalResult, primaryList, globalContext, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    return m_context->GetReference(evalResult, refResult, primaryList, globalContext, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContextSymbolPtr ContextSymbol::CreateContextSymbol(wchar_t const* name, ExpressionContextR context) 
    { 
    return new ContextSymbol(name, context); 
    }

/*---------------------------------------------------------------------------------**//**
* Without this the compiler generates a destructor that invokes the ECValue. That
* can happen in code that does not use any of this stuff and does not link
* with the ECObjects DLL.
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ValueSymbol::~ValueSymbol()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ValueSymbol::ValueSymbol (wchar_t const* name, EC::ECValue& value) : Symbol(name)
    {
    m_expressionValue = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ValueSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
    {
    if (primaryList.GetNumberOfOperators() > startIndex)
        {
        ExpressionToken token = primaryList.GetOperation(startIndex);
        if (EC::TOKEN_Dot == token)
            return ExprStatus_StructRequired;

        if (EC::TOKEN_LParen == token)
            return ExprStatus_MethodRequired;

        if (EC::TOKEN_LeftBracket == token)
            return ExprStatus_ArrayRequired;

        return ExprStatus_UnknownError;
        }

    evalResult = m_expressionValue;
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ValueSymbol::GetValue(EC::ECValueR exprValue)
    {
    exprValue = m_expressionValue; 
    }

END_BENTLEY_EC_NAMESPACE
