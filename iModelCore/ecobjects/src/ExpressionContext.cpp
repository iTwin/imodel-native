/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>
#include <Bentley/BeThreadLocalStorage.h>

BeThreadLocalStorage g_externalSymbolPublisher;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InternalECSymbolProviderManager::InternalECSymbolProviderManager ()
    {
    static ECN::SystemSymbolProvider* s_systemSymbolProvider = nullptr;

    if (nullptr == s_systemSymbolProvider)
        s_systemSymbolProvider = new SystemSymbolProvider ();

    if (nullptr != s_systemSymbolProvider)
        RegisterSymbolProvider (*s_systemSymbolProvider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InternalECSymbolProviderManager& InternalECSymbolProviderManager::GetManager ()
    {
    BeSystemMutexHolder lock;
    static ECN::InternalECSymbolProviderManager* s_manager = nullptr;

    if (nullptr == s_manager)
        s_manager = new InternalECSymbolProviderManager ();

    return *s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::RegisterSymbolProvider (IECSymbolProviderCR provider)
    {
    BeMutexHolder lock(m_mutex);
    if (m_symbolProviders.end() == std::find_if (m_symbolProviders.begin(), m_symbolProviders.end(), [&](IECSymbolProviderCP existing) { return existing == &provider; }))
        m_symbolProviders.push_back (&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::UnregisterSymbolProvider (IECSymbolProviderCR provider)
    {
    BeMutexHolder lock(m_mutex);
    auto found = std::find_if (m_symbolProviders.begin(), m_symbolProviders.end(), [&](IECSymbolProviderCP existing) { return existing == &provider; });
    if (found != m_symbolProviders.end())
        m_symbolProviders.erase (found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets)
    {
    BeMutexHolder lock(m_mutex);
    for (IECSymbolProviderCP const& provider: m_symbolProviders)
        provider->PublishSymbols (context, requestedSymbolSets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ExpressionContextP> Stack(bvector<ExpressionContextP> stack, ExpressionContextR ctx)
    {
    stack.push_back(&ctx);
    return stack;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex, IECInstanceCR rootInstance)
    {
    BeAssert(m_initialized);

    size_t      numberOfOperators = primaryList.GetNumberOfOperators();
    size_t      index = startIndex;
    bool        appendDot = false;
    Utf8String     accessString;
    ECN::IECInstancePtr  instance = const_cast<ECN::IECInstanceP>(&rootInstance);
    ECN::ECEnablerCP     enabler = &instance->GetEnabler();
    ECN::ECClassCP       ecClass = &enabler->GetClass();

    accessString.reserve(50);

    if (index == numberOfOperators)
        {
        //  This is an instance expression
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. InvalidReference (out of vector bounds)");
        return ExpressionStatus::UnknownError;    // Report invalid reference
        }

    evalResult.InitECValue().SetStruct (const_cast<IECInstanceP>(&rootInstance));

    ExpressionToken        nextOperation = primaryList.GetOperation(index);
    ECN::ECPropertyP             currentProperty = NULL;

    while (TOKEN_None != nextOperation)
        {
        while (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation || TOKEN_LeftBracket == nextOperation)    // Handle instance["AccessString"] and instance.AccessString property access
            {
            Utf8String memberName;
            if (TOKEN_LeftBracket == nextOperation)
                {
                // Property access or array indexer?
                if (NULL != currentProperty && currentProperty->GetIsArray())
                    break;
                else
                    {
                    LBracketNodeCP lBracketNode = static_cast<LBracketNodeCP>(primaryList.GetOperatorNode(index));
                    EvaluationResult accessorResult;
                    NodePtr accessorNode = lBracketNode->GetIndexNode();
                    ExpressionStatus exprStatus = accessorNode->GetValue (accessorResult, *contextsStack.back());
                    if (ExpressionStatus::Success != exprStatus)
                        {
                        evalResult.Clear();
                        return exprStatus;
                        }
                    else if (!accessorResult.IsECValue())
                        {
                        evalResult.Clear();
                        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: PrimitiveRequired (is not ECValue)");
                        return ExpressionStatus::PrimitiveRequired;
                        }

                    memberName = accessorResult.GetECValue()->GetUtf8CP();
                    }
                }
            else
                {
                BeAssert (NULL != dynamic_cast<IdentNodeCP>(primaryList.GetOperatorNode(index)));
                IdentNodeCP       identNode = static_cast<IdentNodeCP>(primaryList.GetOperatorNode(index));
                memberName = identNode->GetName();
                }

            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(++index);

            if (appendDot)
                accessString.append(".");

            appendDot = true;

            accessString.append(memberName);

            currentProperty = ecClass->GetPropertyP(memberName);
            if (NULL == currentProperty)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownMember (property is NULL)");
                return ExpressionStatus::UnknownMember;
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
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: PrimitiveRequired");
                return ExpressionStatus::PrimitiveRequired;
                }

            ECN::StructECPropertyCP  structProperty = currentProperty->GetAsStructProperty();
            BeAssert (NULL != structProperty);

            ecClass = &structProperty->GetType();
            }

        if (TOKEN_None == nextOperation)
            {
            if (NULL == currentProperty)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownSymbol (property is NULL)");
                return ExpressionStatus::UnknownSymbol;
                }

            ECN::PrimitiveECPropertyCP   primProp = currentProperty->GetAsPrimitiveProperty();
            if (NULL == primProp)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: PrimitiveRequired (property is not primitive)");
                return ExpressionStatus::PrimitiveRequired;
                }

            ::uint32_t   propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != ECN::ECObjectsStatus::Success)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Could not get property index");
                return ExpressionStatus::UnknownError;
                }


            refResult.m_accessString = accessString;
            refResult.m_memberSelector = -1;
            refResult.m_property = currentProperty;

            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("InstanceListExpressionContext::GetReference: %s = %s", primaryList.GetName(index), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }

        if (TOKEN_LParen == nextOperation)
            {
            //  Might want to allow a method to setting properties on an object returned from a method.  For now, just return an error.
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Unexpected left parenthesis");
            return ExpressionStatus::UnknownError;
            }

        if (TOKEN_LeftBracket == nextOperation)
            {
            if (NULL == currentProperty)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("InstanceListExpressionContext::GetReference: ArrayRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                return ExpressionStatus::ArrayRequired;
                }

            if (!currentProperty->GetIsArray())
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("InstanceListExpressionContext::GetReference: ArrayRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                return ExpressionStatus::ArrayRequired;
                }

            LBracketNodeCP       lBracketNode = static_cast<LBracketNodeCP>(primaryList.GetOperatorNode(index++));

            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            EvaluationResult    indexResult;
            NodePtr             indexNode = lBracketNode->GetIndexNode();

            ExpressionStatus exprStatus = indexNode->GetValue(indexResult, *contextsStack.back());
            if (ExpressionStatus::Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }
            else if (!indexResult.IsECValue())
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("InstanceListExpressionContext::GetReference: PrimitiveRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                evalResult.Clear();
                return ExpressionStatus::PrimitiveRequired;
                }

            //  Need to convert this to an int if it is not already an int
            uint32_t       arrayIndex = (uint32_t)indexResult.GetECValue()->GetInteger();

            //  May need to get an instance or primitive value.
            if (currentProperty->GetIsPrimitiveArray())
                {
                if (TOKEN_None != nextOperation)
                    {
                    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("InstanceListExpressionContext::GetReference: StructRequired. Result: %s = %s ", primaryList.GetName(index), evalResult.ToString().c_str()).c_str());
                    evalResult.Clear();
                    return ExpressionStatus::StructRequired;  //  might be method required.  Should check next node
                    }

                refResult.m_accessString = accessString;
                refResult.m_arrayIndex = arrayIndex;
                refResult.m_property = currentProperty;

                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("InstanceListExpressionContext::GetReference: Result: %s = %s ", primaryList.GetName(index), evalResult.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
                }

            BeAssert(currentProperty->GetIsStructArray());

            ::uint32_t   propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != ECN::ECObjectsStatus::Success)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Could not get property index");
                return ExpressionStatus::UnknownError;
                }

            ECN::ECValue         ecValue;
            ECN::ECObjectsStatus  ecStatus = instance->GetValue(ecValue, propertyIndex, arrayIndex);

            if (ECN::ECObjectsStatus::Success != ecStatus)
                {
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Could not get instance value");
                return ExpressionStatus::UnknownError;             //  This should return a good translation of the ECObjectsStatus:: value
                }

            if (ecValue.IsNull())
                {
                evalResult = ecValue;
                //  Should we initialize the array?
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Instance value is NULL");
                return ExpressionStatus::UnknownError;
                }

            if (TOKEN_None == nextOperation)
                {
                //  Returning a struct instance.
                evalResult.Clear();
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError. Expected next operation");
                return ExpressionStatus::UnknownError;
                }

            appendDot   = false;
            accessString.clear();
            instance = ecValue.GetStruct().get();
            enabler = &instance->GetEnabler();
            ecClass = &enabler->GetClass();
            evalResult.InitECValue().SetStruct(instance.get());
            }
        }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "InstanceListExpressionContext::GetReference: UnknownError");
        return ExpressionStatus::UnknownError;
    }

static const uint32_t COMPONENT_INDEX_None = -1;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t getComponentIndex (NodeCP node, bool is2d)
    {
    uint32_t componentIndex = COMPONENT_INDEX_None;
    IdentNodeCP ident = dynamic_cast<IdentNodeCP>(node);
    Utf8CP componentName = nullptr != ident ? ident->GetName() : nullptr;
    if (nullptr != componentName && 0 != *componentName && 0 == *(componentName + 1))
        {
        switch (towlower (*componentName))
            {
            case 'x':
                componentIndex = 0;
                break;
            case 'y':
                componentIndex = 1;
                break;
            case 'z':
                if (!is2d)
                    componentIndex = 2;
                break;
            }
        }

    return componentIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointProperty (bool& is2d, PrimitiveType primType)
    {
    switch (primType)
        {
        case PRIMITIVETYPE_Point3d:
            is2d = false;
            return true;
        case PRIMITIVETYPE_Point2d:
            is2d = true;
            return true;
        default:
            return false;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointProperty (bool& is2d, ECPropertyCP prop)
    {
    PrimitiveECPropertyCP primProp = nullptr != prop ? prop->GetAsPrimitiveProperty() : nullptr;
    return nullptr != primProp ? isPointProperty (is2d, primProp->GetType()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointArrayProperty (bool& is2d, ECPropertyCP prop)
    {
    PrimitiveArrayECPropertyCP arrayProp = nullptr != prop ? prop->GetAsPrimitiveArrayProperty() : nullptr;
    return nullptr != arrayProp ? isPointProperty (is2d, arrayProp->GetPrimitiveElementType()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus GetInstanceValue (EvaluationResultR evalResult, uint32_t& index, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, IECInstanceCR instance)
    {
    uint32_t startIndex = index;
    ExpressionToken nextOperation = primaryList.GetOperation (index);
    if (TOKEN_Dot != nextOperation && TOKEN_Ident != nextOperation && TOKEN_LeftBracket != nextOperation)
        {
        BeAssert (false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: UnknownError. Invalid next operation: %s", Lexer::GetString(nextOperation).c_str()).c_str());
        return ExpressionStatus::UnknownError;
        }

    bool appendDot = false;
    Utf8String accessString;
    ECClassCP ecClass = &instance.GetClass();

    accessString.reserve (50);

    ECPropertyCP currentProperty = NULL;
    while (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation || TOKEN_LeftBracket == nextOperation)
        {
        Utf8String memberName;
        if (TOKEN_LeftBracket == nextOperation)
            {
            if (NULL != currentProperty && currentProperty->GetIsArray())
                break;

            BeAssert (NULL != dynamic_cast<LBracketNodeCP> (primaryList.GetOperatorNode (index)));
            LBracketNodeCP lBracketNode = static_cast<LBracketNodeCP> (primaryList.GetOperatorNode (index));
            EvaluationResult accessorResult;
            NodePtr accessorNode = lBracketNode->GetIndexNode();
            ExpressionStatus exprStatus = accessorNode->GetValue (accessorResult, *contextsStack.back());
            if (ExpressionStatus::Success != exprStatus)
                { evalResult.Clear(); return exprStatus; }
            else if (accessorResult.GetValueType() != ValType_ECValue || !accessorResult.GetECValue()->IsString())
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: WrongType. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
                evalResult.Clear();
                return ExpressionStatus::WrongType;
                }

            // ###TODO: NEEDSWORK: this doesn't work for expressions like 'this["Struct.Member"]'
            memberName = accessorResult.GetECValue()->GetUtf8CP();
            }
        else
            {
            BeAssert (NULL != dynamic_cast<IdentNodeCP> (primaryList.GetOperatorNode (index)));
            IdentNodeCP identNode = static_cast<IdentNodeCP> (primaryList.GetOperatorNode (index));
            if (TOKEN_Dot == nextOperation)
                {
                // Handle fully-qualified property accessor. Expect two qualifiers: schema name and class name
                bvector<Utf8String> const& qualifiers = identNode->GetQualifiers();
                if (2 == qualifiers.size() && !instance.GetClass().Is (qualifiers[0].c_str(), qualifiers[1].c_str()))
                    {
                    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: UnknownMember. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
                    evalResult.Clear();
                    return ExpressionStatus::UnknownMember;
                    }
                }

            memberName = identNode->GetName();
            }

        nextOperation = primaryList.GetOperation (++index);
        if (appendDot)
            accessString.append (".");
        else
            appendDot = true;

        accessString.append (memberName);

        currentProperty = ecClass->GetPropertyP(memberName);
        if (!currentProperty || !currentProperty->GetIsStruct())
            break;

        if (TOKEN_None == nextOperation)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: PrimitiveRequired. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
            evalResult.Clear();
            return ExpressionStatus::PrimitiveRequired;
            }

        ecClass = &currentProperty->GetAsStructProperty()->GetType();
        }

    bool is2d = false;
    uint32_t componentIndex = COMPONENT_INDEX_None;
    if (TOKEN_Dot == nextOperation && isPointProperty (is2d, currentProperty))
        {
        // handle point member access
        componentIndex = getComponentIndex (primaryList.GetOperatorNode (index), is2d);
        if (COMPONENT_INDEX_None == componentIndex)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: UnknownMember. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::UnknownMember;
            }
        // we'll get the component value below if no more operations
        nextOperation = primaryList.GetOperation (++index);
        }

    if (TOKEN_None == nextOperation)
        {
        evalResult.Clear();
        if (NULL == currentProperty)
            {
            if (accessString.Equals("ECInstanceId"))
                {
                evalResult = ECValue(BeInt64Id::FromString(instance.GetInstanceId().c_str()).ToHexStr().c_str());
                return ExpressionStatus::Success;
                }
            else if (accessString.Equals("ECClassId"))
                {
                evalResult = ECValue(instance.GetClass().HasId() ? instance.GetClass().GetId().ToHexStr().c_str() : BeInt64Id().ToHexStr().c_str());
                return ExpressionStatus::Success;
                }

            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownSymbol. Property is NULL");
            return ExpressionStatus::UnknownSymbol;
            }
        else if (currentProperty->GetIsArray())
            {
            uint32_t propIdx;
            if (ECObjectsStatus::Success != instance.GetEnabler().GetPropertyIndex (propIdx, accessString.c_str()))
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownSymbol. Could not get property index");
                return ExpressionStatus::UnknownSymbol;
                }

            IValueListResultPtr list = IValueListResult::Create (const_cast<IECInstanceR>(instance), propIdx);
            evalResult.SetValueList (*list);
            return ExpressionStatus::Success;
            }

        ECValue ecval;
        PrimitiveECPropertyCP primProp = currentProperty->GetAsPrimitiveProperty();
        if (NULL == primProp || ECObjectsStatus::Success != instance.GetValue (ecval, accessString.c_str()))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownError. Current property is not primitive");
            return ExpressionStatus::UnknownError;
            }

        if (COMPONENT_INDEX_None != componentIndex)
            {
            if (ecval.IsNull() || (is2d && !ecval.IsPoint2d()) || (!is2d && !ecval.IsPoint3d()))
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: ECValue is not supported (%s)", ecval.ToString().c_str()).c_str());
                return ExpressionStatus::DotNotSupported;
                }

            DPoint3d pt = !is2d ? ecval.GetPoint3d() : DPoint3d::From (ecval.GetPoint2d().x, ecval.GetPoint2d().y, 0.0);
            double* component = (&pt.x) + componentIndex;
            ecval.SetDouble (*component);
            }

        evalResult = ecval;
        return ExpressionStatus::Success;
        }
    else if (TOKEN_LeftBracket == nextOperation)
        {
        if (NULL == currentProperty || !currentProperty->GetIsArray())
            {
            evalResult.Clear();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: ArrayRequired (Current property is not array)");
            return ExpressionStatus::ArrayRequired;
            }

        LBracketNodeCP lBracketNode = static_cast<LBracketNodeCP> (primaryList.GetOperatorNode (index++));
        nextOperation = primaryList.GetOperation (index);

        bool isPrimitive = currentProperty->GetIsPrimitiveArray();
        is2d = false;
        componentIndex = COMPONENT_INDEX_None;

        if (isPrimitive && TOKEN_Dot == nextOperation && isPointArrayProperty (is2d, currentProperty))
            {
            componentIndex = getComponentIndex (primaryList.GetOperatorNode (index), is2d);
            if (COMPONENT_INDEX_None == componentIndex)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: UnknownMember. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
                return ExpressionStatus::UnknownMember;
                }
            // we'll get the component value below if no more operations
            nextOperation = primaryList.GetOperation (++index);
            }

        if (isPrimitive && TOKEN_None != nextOperation)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: StructRequired. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
            evalResult.Clear();
            return ExpressionStatus::StructRequired;
            }

        EvaluationResult indexResult;
        NodePtr indexNode = lBracketNode->GetIndexNode();

        ExpressionStatus exprStatus = indexNode->GetValue (indexResult, *contextsStack.back());
        if (ExpressionStatus::Success != exprStatus)
            { evalResult.Clear(); return exprStatus; }
        else if (indexResult.GetValueType() != ValType_ECValue || !indexResult.GetECValue()->ConvertToPrimitiveType (PRIMITIVETYPE_Integer) || indexResult.GetECValue()->IsNull())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: StructRequired. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
            evalResult.Clear();
            return ExpressionStatus::PrimitiveRequired;
            }

        ECValue arrayVal;
        if (ECObjectsStatus::Success != instance.GetValue (arrayVal, accessString.c_str(), (uint32_t)indexResult.GetECValue()->GetInteger()))
            {
            evalResult.Clear();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownError. Could not get instance value");
            return ExpressionStatus::UnknownError;
            }

        if (isPrimitive && COMPONENT_INDEX_None != componentIndex)
            {
            if (arrayVal.IsNull() || (is2d && !arrayVal.IsPoint3d()) || (!is2d && !arrayVal.IsPoint3d()))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: ECValue is not supported. Expected to be 2D or 3D point (%s)", arrayVal.ToString().c_str()).c_str());
            return ExpressionStatus::DotNotSupported;
            }

            DPoint3d pt = !is2d ? arrayVal.GetPoint3d() : DPoint3d::From (arrayVal.GetPoint2d().x, arrayVal.GetPoint2d().y, 0.0);
            double* component = (&pt.x) + componentIndex;
            arrayVal.SetDouble (*component);
            }

        if (TOKEN_None == nextOperation)
            {
            evalResult = arrayVal;
            return ExpressionStatus::Success;
            }

        BeAssert (currentProperty->GetIsStructArray());
        if (arrayVal.IsNull())
            {
            evalResult = arrayVal;
            return ExpressionStatus::Success;
            }
        else if (!arrayVal.IsStruct())
            {
            evalResult.Clear();
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: StructRequired: Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::StructRequired;
            }

        if (TOKEN_LParen == nextOperation)
            {
            --index;    // place the LParen back in queue, caller will handle it.
            evalResult = arrayVal;
            return ExpressionStatus::Success;
            }
        else
            return GetInstanceValue (evalResult, index, primaryList, contextsStack, *arrayVal.GetStruct());
        }
    else if (TOKEN_Dot == nextOperation && currentProperty->GetIsNavigation())
        {
        NodeCP node = primaryList.GetOperatorNode(index);
        nextOperation = primaryList.GetOperation(++index);
        if (TOKEN_None != nextOperation || nullptr == dynamic_cast<IdentNodeCP>(node))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownError. Can only access 'id' value of navigation properties.");
            return ExpressionStatus::UnknownMember;
            }
        IdentNodeCP identNode = static_cast<IdentNodeCP>(node);
        if (0 != BeStringUtilities::Stricmp("id", identNode->GetName()))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownError. Can only access 'id' value of navigation properties.");
            return ExpressionStatus::UnknownMember;
            }
        ECValue v;
        ECObjectsStatus evalStatus;
        if (ECObjectsStatus::Success != (evalStatus = instance.GetValue(v, accessString.c_str())))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetInstanceValue: UnknownError. Failed to get value of property '%s' with status %d", accessString.c_str(), evalStatus).c_str());
            return ExpressionStatus::UnknownError;
            }
        if (v.IsNull())
            evalResult.InitECValue().SetIsNull(true);
        else
            evalResult = ECValue(v.GetNavigationInfo().GetId<BeInt64Id>().GetValue());
        return ExpressionStatus::Success;
        }
    else if (TOKEN_Dot == nextOperation || TOKEN_LParen == nextOperation)
        {
        // we get here if we find a dot following something that is not a struct. e.g., 'someArray.Count', 'someArray.Any (x => x < 5)', etc.
        if (NULL != currentProperty && currentProperty->GetIsArray())
            {
            uint32_t propIdx;
            if (ECObjectsStatus::Success == instance.GetEnabler().GetPropertyIndex (propIdx, accessString.c_str()))
                {
                IValueListResultPtr list = IValueListResult::Create (const_cast<IECInstanceR>(instance), propIdx);
                evalResult.SetValueList (*list);
                }
            }

        if (evalResult.IsValueList())
            {
            IValueListResultPtr list = evalResult.GetValueList();
            if (TOKEN_Dot == nextOperation)
                {
                IdentNodeCP ident = dynamic_cast<IdentNodeCP>(primaryList.GetOperatorNode (index));
                Utf8CP arrayPropName = NULL != ident ? ident->GetName() : NULL;
                uint32_t count = list->GetCount();
                if (NULL != arrayPropName)
                    {
                    if (0 == strcmp ("Count", arrayPropName))
                        {
                        evalResult.InitECValue().SetInteger ((int32_t)count);
                        return ExpressionStatus::Success;
                        }

                    bool wantFirst = 0 == strcmp ("First", arrayPropName);
                    if (wantFirst || 0 == strcmp ("Last", arrayPropName))
                        {
                        // if array is empty, we return null successfully. Otherwise get array member.
                        if (0 == count)
                            {
                            evalResult.InitECValue().SetToNull();
                            return ExpressionStatus::Success;
                            }
                        else
                            return list->GetValueAt (evalResult, wantFirst ? 0 : count - 1);
                        }
                    }
                }
            else
                {
                // put the LParen back in queue, caller will handle it.
                --index;
                return ExpressionStatus::Success;
                }
            }
        }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "GetInstanceValue: UnknownError");
        return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus GetInstanceValue (EvaluationResultR evalResult, uint32_t& startIndex, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ECInstanceListCR instanceList)
    {
    ExpressionStatus status = ExpressionStatus::StructRequired;
    for (IECInstancePtr const& instance: instanceList)
        {
        uint32_t index = startIndex;
        if (instance.IsNull())
            {
            BeAssert (false); continue;
            }

        status = GetInstanceValue (evalResult, index, primaryList, contextsStack, *instance);
        if (ExpressionStatus::Success == status)
            {
            startIndex = index;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool convertResultToInstanceList (EvaluationResultR result, bool requireInstance)
    {
    if (result.GetValueType() == ValType_InstanceList)
        return true;
    else if (result.GetValueType() == ValType_ECValue && result.GetECValue()->IsStruct() && !result.GetECValue()->IsNull())
        {
        result.SetInstance (*result.GetECValue()->GetStruct());
        return true;
        }
    else
        return !requireInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus GetPrimaryListResult(EvaluationResultR evalResult, PrimaryListNodeR primaryList, uint32_t startIndex, bvector<ExpressionContextP> const& contextsStack, ExpressionContextP outerContext)
    {
    uint32_t numberOfOperators = (uint32_t)primaryList.GetNumberOfOperators();
    uint32_t index = startIndex;

    if (index == numberOfOperators)
        {
        // already have the result
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("GetPrimaryListResult: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    ExpressionToken nextOperation = primaryList.GetOperation(index);
    while (TOKEN_None != nextOperation)
        {
        if (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation || TOKEN_LeftBracket == nextOperation)
            {
            if (!convertResultToInstanceList(evalResult, true))
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("GetPrimaryListResult: UnknownError. Could not convert result to instance list. Result: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
                return ExpressionStatus::UnknownError;
                }

            EvaluationResult valueResult;
            ExpressionStatus instanceStatus = GetInstanceValue(valueResult, index, primaryList, contextsStack, *evalResult.GetInstanceList());
            if (ExpressionStatus::Success != instanceStatus)
                {
                if (0 == startIndex && 0 == index)
                    {
                    if (NULL != outerContext)
                        return outerContext->GetValue(evalResult, primaryList, contextsStack, startIndex);
                    }
                evalResult.Clear();
                return instanceStatus;
                }

            evalResult = valueResult;
            nextOperation = primaryList.GetOperation (++index);
            }

        if (TOKEN_LParen == nextOperation)
            {
            CallNodeP callNode = static_cast<CallNodeP>(primaryList.GetOperatorNode(index++));
            nextOperation = primaryList.GetOperation(index);

            EvaluationResult methodResult;
            ExpressionStatus exprStatus = ExpressionStatus::UnknownError;
            switch (evalResult.GetValueType())
                {
                case ValType_None:
                    exprStatus = callNode->InvokeStaticMethod(methodResult, contextsStack);
                    break;
                case ValType_InstanceList:
                    exprStatus = callNode->InvokeInstanceMethod (methodResult, *evalResult.GetInstanceList(), contextsStack);
                    break;
                case ValType_ValueList:
                    exprStatus = callNode->InvokeValueListMethod (methodResult, *evalResult.GetValueList(), contextsStack);
                    break;
                }

            if (ExpressionStatus::Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }

            evalResult = methodResult;
            }
        }
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("GetPrimaryListResult: %s = %s", primaryList.GetName(startIndex), evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    Initialize();
    evalResult.SetInstanceList (m_instances, false);
    return GetPrimaryListResult(evalResult, primaryList, startIndex, contextsStack, GetOuterP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferenceStandard::MethodReferenceStandard(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    : m_staticMethod(staticMethod), m_instanceMethod(instanceMethod), m_valueListMethod(NULL), m_context(context)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferenceStandard::MethodReferenceStandard(ExpressionValueListMethod_t valueListMethod, void* context)
    : m_staticMethod(NULL), m_instanceMethod(NULL), m_valueListMethod(valueListMethod), m_context(context)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments)
    {
    if (NULL == m_staticMethod)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "MethodReferenceStandard::_InvokeStaticMethod StaticMethodRequired");
        return ExpressionStatus::StaticMethodRequired;
        }

    return (*m_staticMethod)(evalResult, m_context, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return NULL != m_valueListMethod ? (*m_valueListMethod)(evalResult, m_context, valueList, args) : ExpressionStatus::InstanceMethodRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeInstanceMethod (EvaluationResultR evalResult, ECInstanceListCR instanceData, EvaluationResultVector& arguments)
    {
    if (NULL == m_instanceMethod)
        return ExpressionStatus::InstanceMethodRequired;

    return (*m_instanceMethod)(evalResult, m_context, instanceData, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferencePtr MethodReferenceStandard::Create(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    {
    return new MethodReferenceStandard(staticMethod, instanceMethod, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferencePtr MethodReferenceStandard::Create (ExpressionValueListMethod_t method, void* context)
    {
    return new MethodReferenceStandard (method, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbol::MethodSymbol(Utf8CP name, MethodReferenceR methodReference)
    : Symbol(name)
    {
    m_methodReference = &methodReference;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbol::MethodSymbol (Utf8CP name, ExpressionValueListMethod_t method)
    : Symbol (name)
    {
    m_methodReference = MethodReferenceStandard::Create (method);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbolPtr MethodSymbol::Create(Utf8CP name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    {
    MethodReferencePtr  ref = MethodReferenceStandard::Create(staticMethod, instanceMethod, context);
    return new MethodSymbol(name, *ref);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbolPtr MethodSymbol::Create (Utf8CP name, ExpressionValueListMethod_t method)
    {
    return new MethodSymbol (name, method);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodSymbol::_CreateMethodResult(MethodReferencePtr & result, ExpressionMethodType methodType) const
    {
    if ((ExpressionMethodType::Instance == methodType && !m_methodReference->SupportsInstanceMethodCall()) ||
        (ExpressionMethodType::Static == methodType && !m_methodReference->SupportsStaticMethodCall()) ||
        (ExpressionMethodType::ValueList == methodType && !m_methodReference->SupportsValueListMethodCall()))
        return ExpressionStatus::WrongType;

    result = m_methodReference.get();
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    //  We only get here if the symbol was resolved directly from a context. The context may be a global context, or a namespace context. If the
    //  context is a global context then startIndex is 1 and the callNode should be 0. If the context is a namespace context, then startIndex - 1
    //  should be the context used to find this symbol. It must be a CallNode.
    //
    //  For now assuming this has to be a static method; may want to support having a MethodSymbol in a context. That would require that this
    //  method receive a pointer to the context used to invoke it.

    BeAssert(1 <= startIndex);
    uint32_t callNodeIndex = startIndex - 1;
    return GetPrimaryListResult(evalResult, primaryList, callNodeIndex, contextsStack, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SymbolExpressionContext::AddSymbol (SymbolR symbol)
    {
    //  Check for duplicates in context
    if (NULL != FindCP (symbol.GetName ()))
        return BSIERROR;

    m_symbols.push_back (SymbolPtr (&symbol));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolCP        SymbolExpressionContext::FindCP (Utf8CP ident)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin (); curr != m_symbols.end (); curr++)
        {
        SymbolP symbol = (*curr).get ();
        if (0 == strcmp (ident, symbol->GetName ()))
            return symbol;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SymbolExpressionContext::RemoveSymbol (Utf8CP ident)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin (); curr != m_symbols.end (); curr++)
        {
        SymbolP symbol = (*curr).get ();
        if (0 == strcmp (ident, symbol->GetName ()))
            {
            m_symbols.erase (curr);
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SymbolExpressionContext::RemoveSymbol (SymbolR symbol)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin (); curr != m_symbols.end (); curr++)
        {
        if ((*curr).get () == &symbol)
            {
            m_symbols.erase (curr);
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolExpressionContextPtr SymbolExpressionContext::Create(ExpressionContextP outer)
    {
    return new SymbolExpressionContext (outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    Utf8CP ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "SymbolExpressionContext::_GetValue: UnknownSymbol. Ident is invalid");
        return ExpressionStatus::UnknownSymbol;
        }

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()))
            return symbol->GetValue(evalResult, primaryList, Stack(contextsStack, *this), startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, Stack(contextsStack, *this), startIndex);

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UnknownSymbol: %s", ident).c_str());
    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetReference(EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    Utf8CP ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "SymbolExpressionContext::_GetReference: UnknownSymbol. Ident is invalid");
        return ExpressionStatus::UnknownSymbol;
        }

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()))
            return symbol->GetReference(evalResult, refResult, primaryList, Stack(contextsStack, *this), startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, Stack(contextsStack, *this), startIndex);

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UnknownSymbol: %s", ident).c_str());
    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary, ExpressionMethodType methodType)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()) && ExpressionStatus::Success == symbol->CreateMethodResult(result, methodType))
            return ExpressionStatus::Success;
        }

    if (useOuterIfNecessary)
        {
        ExpressionContextP  outer = GetOuterP();
        if (NULL != outer)
            return outer->ResolveMethod(result, ident, useOuterIfNecessary, methodType);
        }

    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    return m_context->GetValue(evalResult, primaryList, contextsStack, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    return m_context->GetReference(evalResult, refResult, primaryList, contextsStack, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContextSymbolPtr ContextSymbol::CreateContextSymbol(Utf8CP name, ExpressionContextR context)
    {
    return new ContextSymbol(name, context);
    }

/*---------------------------------------------------------------------------------**//**
* Without this the compiler generates a destructor that invokes the ECValue. That
* can happen in code that does not use any of this stuff and does not link
* with the ECObjects DLL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValueSymbol::~ValueSymbol()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValueSymbol::ValueSymbol (Utf8CP name, EvaluationResultCR value) : Symbol(name)
    {
    m_expressionValue = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ValueSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    if (primaryList.GetNumberOfOperators() > startIndex)
        {
        ExpressionToken token = primaryList.GetOperation(startIndex);
        if (ECN::TOKEN_Dot == token)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ValueSymbol::_GetValue: StructRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
            return ExpressionStatus::StructRequired;
            }

        if (ECN::TOKEN_LParen == token)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ValueSymbol::_GetValue: MethodRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
            return ExpressionStatus::MethodRequired;
            }

        if (ECN::TOKEN_LeftBracket == token)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ValueSymbol::_GetValue: ArrayRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
            return ExpressionStatus::ArrayRequired;
            }

        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ValueSymbol::_GetValue: UnknownError");
        return ExpressionStatus::UnknownError;
        }

    evalResult = m_expressionValue;
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ValueSymbol::_GetValue: Result: %s = %s", primaryList.ToExpressionString().c_str(), evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
ValueSymbolPtr  ValueSymbol::Create(Utf8CP name, ECN::ECValueCR exprValue)
    {
    EvaluationResult evalResult;
    evalResult = exprValue;
    return new ValueSymbol (name, evalResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValueSymbolPtr ValueSymbol::Create (Utf8CP name, EvaluationResultCR value) { return new ValueSymbol (name, value); }
EvaluationResultCR ValueSymbol::GetValue() const { return m_expressionValue; }
void ValueSymbol::SetValue (EvaluationResultCR value) { m_expressionValue = value; }
void ValueSymbol::SetValue (ECValueCR value) { m_expressionValue = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySymbol::PropertySymbol(Utf8CP name, PropertySymbol::ValueEvaluator& evaluator)
    : Symbol(name), m_valueEvaluator(&evaluator)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySymbol::PropertySymbol(Utf8CP name, PropertySymbol::ValueListEvaluator& evaluator)
    : Symbol(name), m_valueListEvaluator(&evaluator)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySymbol::PropertySymbol(Utf8CP name, PropertySymbol::ContextEvaluator& evaluator)
    : Symbol(name), m_contextEvaluator(&evaluator)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertySymbol> PropertySymbol::Create(Utf8CP name, PropertySymbol::ValueEvaluator& evaluator)
    {
    return new PropertySymbol(name, evaluator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertySymbol> PropertySymbol::Create(Utf8CP name, PropertySymbol::ValueListEvaluator& evaluator)
    {
    return new PropertySymbol(name, evaluator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertySymbol> PropertySymbol::Create(Utf8CP name, PropertySymbol::ContextEvaluator& evaluator)
    {
    return new PropertySymbol(name, evaluator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PropertySymbol::_GetValue (EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
    {
    if (m_valueEvaluator.IsValid())
        {
        if (primaryList.GetNumberOfOperators() > startIndex)
            {
            ExpressionToken token = primaryList.GetOperation(startIndex);
            if (ECN::TOKEN_Dot == token)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PropertySymbol::_GetValue: StructRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                return ExpressionStatus::StructRequired;
                }
            if (ECN::TOKEN_LParen == token)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PropertySymbol::_GetValue: MethodRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                return ExpressionStatus::MethodRequired;
                }
            if (ECN::TOKEN_LeftBracket == token)
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("PropertySymbol::_GetValue: ArrayRequired (%s)", primaryList.ToExpressionString().c_str()).c_str());
                return ExpressionStatus::ArrayRequired;
                }
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "PropertySymbol::_GetValue: UnknownError");
            return ExpressionStatus::UnknownError;
            }
        evalResult = m_valueEvaluator->_EvaluateValue();
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("PropertySymbol::_GetValue: Result: %s = %s", primaryList.ToExpressionString().c_str(), evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

    if (m_valueListEvaluator.IsValid())
        {
        IValueListResultPtr valueList = m_valueListEvaluator->_EvaluateValueList();
        if (!valueList.IsValid())
            {
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "PropertySymbol::_GetValue: UnknownError. Invalid value list");
            return ExpressionStatus::UnknownError;
            }
        evalResult.SetValueList(*valueList);
        return GetPrimaryListResult(evalResult, primaryList, startIndex, contextsStack, nullptr);
        }

    if (m_contextEvaluator.IsValid())
        {
        ExpressionContextPtr context = m_contextEvaluator->_GetContext();
        if (!context.IsValid())
            {
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "PropertySymbol::_GetValue: UnknownError. Invalid context");
            return ExpressionStatus::UnknownError;
            }
        return context->GetValue(evalResult, primaryList, Stack(contextsStack, *context), startIndex);
        }

    BeAssert(false);
    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IECSymbolProvider::RegisterExternalSymbolPublisher (ExternalSymbolPublisher publisher)
    {
    g_externalSymbolPublisher.SetValueAsPointer((void*)publisher);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IECSymbolProvider::UnRegisterExternalSymbolPublisher ()
    {
    g_externalSymbolPublisher.SetValueAsPointer(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolExpressionContextPtr SymbolExpressionContext::Create (bvector<Utf8String> const& requestedSymbolSets, ExpressionContextP outer)
    {
    SymbolExpressionContextPtr context = Create (outer);
    if (context.IsValid())
        {
        InternalECSymbolProviderManager::GetManager ().PublishSymbols (*context, requestedSymbolSets);
        ECN::IECSymbolProvider::ExternalSymbolPublisher externalSymbolPublisher = reinterpret_cast<ECN::IECSymbolProvider::ExternalSymbolPublisher>(g_externalSymbolPublisher.GetValueAsPointer());

        if (nullptr != externalSymbolPublisher)
            externalSymbolPublisher (*context, requestedSymbolSets);
        }

    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolExpressionContextPtr  SymbolExpressionContext::CreateWithThis (bvector<Utf8String> const& requestedSymbolSets, IECInstanceP instance)
    {
    SymbolExpressionContextPtr symbolContext = SymbolExpressionContext::Create (requestedSymbolSets, NULL);

    // If we also have an ECInstance add it as "this"
    if (symbolContext.IsValid() && NULL != instance)
        {
        InstanceExpressionContextPtr context = InstanceExpressionContext::Create (NULL);
        if (context.IsValid ())
            {
            context->SetInstance (*instance);
            ContextSymbolPtr instanceSymbol = ContextSymbol::CreateContextSymbol ("this", *context);
            symbolContext->AddSymbol (*instanceSymbol);
            }
        }

    return symbolContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceExpressionContextPtr InstanceExpressionContext::Create (ExpressionContextP outer)
    {
    return new InstanceExpressionContext (outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceExpressionContext::SetInstance (IECInstanceCR instance)
    {
    ECInstanceList list;
    list.push_back (const_cast<IECInstanceP>(&instance));
    SetInstanceList (list);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceExpressionContext::SetInstances (ECInstanceListCR instances)
    {
    SetInstanceList (instances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContext::InstanceListExpressionContext (ExpressionContextP outer)
  : ExpressionContext (outer), m_initialized (false)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContext::InstanceListExpressionContext (ECInstanceListCR instances, ExpressionContextP outer)
  : ExpressionContext(outer), m_instances(instances), m_initialized(true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceListExpressionContext::Initialize()
    {
    if (!m_initialized)
        {
        _GetInstances (m_instances);
        m_initialized = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::_GetReference (EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, uint32_t startIndex)
    {
    Initialize();

    if (startIndex == primaryList.GetNumberOfOperators())
        {
        // This is an instance expression
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("InstanceListExpressionContext::_GetReference: UnknownError (startIndex: %" PRIu32 " = numberOfOperators: %" PRIu64 ")", startIndex, (uint64_t)primaryList.GetNumberOfOperators()).c_str());
        return ExpressionStatus::UnknownError;
        }

    ExpressionStatus status = ExpressionStatus::UnknownSymbol;
    Utf8CP name = primaryList.GetName (startIndex);
    if (NULL != name)
        {
        for (IECInstancePtr const& instance: m_instances)
            {
            if (instance.IsValid() && ExpressionStatus::Success == (status = GetReference (evalResult, refResult, primaryList, contextsStack, startIndex, *instance)))
                break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContextPtr InstanceListExpressionContext::Create (ECInstanceListCR instances, ExpressionContextP outer)
    {
    return new InstanceListExpressionContext (instances, outer);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
