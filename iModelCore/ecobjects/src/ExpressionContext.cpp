/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ExpressionContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>
#include <Bentley/BeThreadLocalStorage.h>

BeThreadLocalStorage g_externalSymbolPublisher;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2015
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
* @bsimethod                                    Bill.Steinbock                  04/2015
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
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::RegisterSymbolProvider (IECSymbolProviderCR provider)
    {
    BeMutexHolder lock(m_mutex);
    if (m_symbolProviders.end() == std::find_if (m_symbolProviders.begin(), m_symbolProviders.end(), [&](IECSymbolProviderCP existing) { return existing == &provider; }))
        m_symbolProviders.push_back (&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::UnregisterSymbolProvider (IECSymbolProviderCR provider)
    {
    BeMutexHolder lock(m_mutex);
    auto found = std::find_if (m_symbolProviders.begin(), m_symbolProviders.end(), [&](IECSymbolProviderCP existing) { return existing == &provider; });
    if (found != m_symbolProviders.end())
        m_symbolProviders.erase (found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InternalECSymbolProviderManager::PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets)
    {
    BeMutexHolder lock(m_mutex);
    for (IECSymbolProviderCP const& provider: m_symbolProviders)
        provider->PublishSymbols (context, requestedSymbolSets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex, IECInstanceCR rootInstance)
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
                    ExpressionStatus exprStatus = accessorNode->GetValue (accessorResult, globalContext);
                    if (ExpressionStatus::Success != exprStatus)
                        {
                        evalResult.Clear();
                        return exprStatus;
                        }
                    else if (!accessorResult.IsECValue())
                        {
                        evalResult.Clear();
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
                return ExpressionStatus::PrimitiveRequired;
                }

            ECN::StructECPropertyCP  structProperty = currentProperty->GetAsStructProperty();
            BeAssert (NULL != structProperty);

            ecClass = &structProperty->GetType();
            }

        if (TOKEN_None == nextOperation)
            {
            if (NULL == currentProperty)
                return ExpressionStatus::UnknownSymbol;

            ECN::PrimitiveECPropertyCP   primProp = currentProperty->GetAsPrimitiveProperty();
            if (NULL == primProp)
                {
                evalResult.Clear();
                return ExpressionStatus::PrimitiveRequired;
                }

            ::uint32_t   propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != ECN::ECObjectsStatus::Success)
                {
                evalResult.Clear();
                return ExpressionStatus::UnknownError;
                }


            refResult.m_accessString = accessString;
            refResult.m_memberSelector = -1;
            refResult.m_property = currentProperty;

            return ExpressionStatus::Success;
            }

        if (TOKEN_LParen == nextOperation)
            {
            //  Might want to allow a method to setting properties on an object returned from a method.  For now, just return an error.
            return ExpressionStatus::UnknownError;
            }

        if (TOKEN_LeftBracket == nextOperation)
            {
            if (NULL == currentProperty)
                {
                evalResult.Clear();
                return ExpressionStatus::ArrayRequired;
                }

            ECN::ArrayECPropertyCP   arrayProp = currentProperty->GetAsArrayProperty();

            if (NULL == arrayProp)
                {
                evalResult.Clear();
                return ExpressionStatus::ArrayRequired;
                }

            LBracketNodeCP       lBracketNode = static_cast<LBracketNodeCP>(primaryList.GetOperatorNode(index++));

            //  Will be TOKEN_None if index is at the end
            nextOperation = primaryList.GetOperation(index);

            EvaluationResult    indexResult;
            NodePtr             indexNode = lBracketNode->GetIndexNode();

            ExpressionStatus exprStatus = indexNode->GetValue(indexResult, globalContext);
            if (ExpressionStatus::Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }
            else if (!indexResult.IsECValue())
                {
                evalResult.Clear();
                return ExpressionStatus::PrimitiveRequired;
                }

            //  Need to convert this to an int if it is not already an int
            uint32_t       arrayIndex = (uint32_t)indexResult.GetECValue()->GetInteger();

            //  May need to get an instance or primitive value.
            if (arrayProp->GetKind() == ECN::ARRAYKIND_Primitive)
                {
                if (TOKEN_None != nextOperation)
                    {
                    evalResult.Clear();
                    return ExpressionStatus::StructRequired;  //  might be method required.  Should check next node
                    }

                refResult.m_accessString = accessString;
                refResult.m_arrayIndex = arrayIndex;
                refResult.m_property = currentProperty;

                return ExpressionStatus::Success;
                }

            BeAssert(ECN::ARRAYKIND_Struct == arrayProp->GetKind());

            ::uint32_t   propertyIndex;
            if (enabler->GetPropertyIndex(propertyIndex, accessString.c_str()) != ECN::ECObjectsStatus::Success)
                {
                evalResult.Clear();
                return ExpressionStatus::UnknownError;
                }

            ECN::ECValue         ecValue;
            ECN::ECObjectsStatus  ecStatus = instance->GetValue(ecValue, propertyIndex, arrayIndex);

            if (ECN::ECObjectsStatus::Success != ecStatus)
                {
                evalResult.Clear();
                return ExpressionStatus::UnknownError;             //  This should return a good translation of the ECObjectsStatus:: value
                }

            if (ecValue.IsNull())
                {
                evalResult = ecValue;
                //  Should we initialize the array?
                return ExpressionStatus::UnknownError;
                }

            if (TOKEN_None == nextOperation)
                {
                //  Returning a struct instance.
                evalResult.Clear();
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

    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t getComponentIndex (NodeCP node, bool is2d)
    {
    uint32_t componentIndex = IECTypeAdapterContext::COMPONENT_INDEX_None;
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
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointProperty (bool& is2d, PrimitiveType primType)
    {
    switch (primType)
        {
        case PRIMITIVETYPE_Point3D:
            is2d = false;
            return true;
        case PRIMITIVETYPE_Point2D:
            is2d = true;
            return true;
        default:
            return false;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointProperty (bool& is2d, ECPropertyCP prop)
    {
    PrimitiveECPropertyCP primProp = nullptr != prop ? prop->GetAsPrimitiveProperty() : nullptr;
    return nullptr != primProp ? isPointProperty (is2d, primProp->GetType()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPointArrayProperty (bool& is2d, ECPropertyCP prop)
    {
    ArrayECPropertyCP arrayProp = nullptr != prop ? prop->GetAsArrayProperty() : nullptr;
    return nullptr != arrayProp && ARRAYKIND_Primitive == arrayProp->GetKind() ? isPointProperty (is2d, arrayProp->GetPrimitiveElementType()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::GetInstanceValue (EvaluationResultR evalResult, size_t& index, PrimaryListNodeR primaryList, ExpressionContextR globalContext, IECInstanceCR instance)
    {
    BeAssert(m_initialized);

    ExpressionToken nextOperation = primaryList.GetOperation (index);
    if (TOKEN_Dot != nextOperation && TOKEN_Ident != nextOperation && TOKEN_LeftBracket != nextOperation)
        {
        BeAssert (false);
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
            ExpressionStatus exprStatus = accessorNode->GetValue (accessorResult, globalContext);
            if (ExpressionStatus::Success != exprStatus)
                { evalResult.Clear(); return exprStatus; }
            else if (accessorResult.GetValueType() != ValType_ECValue || !accessorResult.GetECValue()->IsString())
                { evalResult.Clear(); return ExpressionStatus::WrongType; }

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

        currentProperty = ecClass->GetPropertyP (memberName);
        if (NULL == currentProperty)
            { evalResult.Clear(); return ExpressionStatus::UnknownMember; }
        else if (!currentProperty->GetIsStruct())
            break;

        if (TOKEN_None == nextOperation)
            { evalResult.Clear(); return ExpressionStatus::PrimitiveRequired; }

        ecClass = &currentProperty->GetAsStructProperty()->GetType();
        }

    bool is2d = false;
    uint32_t componentIndex = IECTypeAdapterContext::COMPONENT_INDEX_None;
    if (TOKEN_Dot == nextOperation && isPointProperty (is2d, currentProperty))
        {
        // handle point member access
        componentIndex = getComponentIndex (primaryList.GetOperatorNode (index), is2d);
        if (IECTypeAdapterContext::COMPONENT_INDEX_None == componentIndex)
            return ExpressionStatus::UnknownMember;

        // we'll get the component value below if no more operations
        nextOperation = primaryList.GetOperation (++index);
        }

    if (TOKEN_None == nextOperation)
        {
        evalResult.Clear();
        if (NULL == currentProperty)
            return ExpressionStatus::UnknownSymbol;
        else if (currentProperty->GetIsArray())
            {
            uint32_t propIdx;
            if (ECObjectsStatus::Success != instance.GetEnabler().GetPropertyIndex (propIdx, accessString.c_str()))
                return ExpressionStatus::UnknownSymbol;

            IValueListResultPtr list = IValueListResult::Create (const_cast<IECInstanceR>(instance), propIdx);
            evalResult.SetValueList (*list);
            return ExpressionStatus::Success;
            }

        UnitSpec units;
        ECValue ecval;
        PrimitiveECPropertyCP primProp = currentProperty->GetAsPrimitiveProperty();
        if (NULL == primProp || ECObjectsStatus::Success != instance.GetValue (ecval, accessString.c_str()))
            return ExpressionStatus::UnknownError;

        if (IECTypeAdapterContext::COMPONENT_INDEX_None != componentIndex)
            {
            if (ecval.IsNull() || (is2d && !ecval.IsPoint2D()) || (!is2d && !ecval.IsPoint3D()))
                return ExpressionStatus::DotNotSupported;

            DPoint3d pt = !is2d ? ecval.GetPoint3D() : DPoint3d::From (ecval.GetPoint2D().x, ecval.GetPoint2D().y, 0.0);
            double* component = (&pt.x) + componentIndex;
            ecval.SetDouble (*component);
            }

        if (globalContext.AllowsTypeConversion () || globalContext.EnforceGlobalRepresentation () || globalContext.EnforcesUnits ())
            {
            IECTypeAdapter* typeAdapter = primProp->GetTypeAdapter();
            if (nullptr != typeAdapter)
                {
                IECTypeAdapterContextPtr context = IECTypeAdapterContext::Create (*primProp, instance);
                if (!context.IsValid())
                    return ExpressionStatus::UnknownError;
                context->SetEvaluationOptions (globalContext.GetEvaluationOptions ());
                if (globalContext.AllowsTypeConversion () || globalContext.EnforceGlobalRepresentation ())
                    {
                    if (typeAdapter->RequiresExpressionTypeConversion (globalContext.GetEvaluationOptions ()) && !typeAdapter->ConvertToExpressionType (ecval, *context))
                        return ExpressionStatus::UnknownError;
                    }
                else if (typeAdapter->SupportsUnits () && !typeAdapter->GetUnits (units, *context))
                    return ExpressionStatus::UnknownError;
                }
            }

        evalResult = ecval;
        evalResult.SetUnits (units);
        return ExpressionStatus::Success;
        }
    else if (TOKEN_LeftBracket == nextOperation)
        {
        ArrayECPropertyCP arrayProp = NULL;
        if (NULL == currentProperty || NULL == (arrayProp = currentProperty->GetAsArrayProperty()))
            { evalResult.Clear(); return ExpressionStatus::ArrayRequired; }

        LBracketNodeCP lBracketNode = static_cast<LBracketNodeCP> (primaryList.GetOperatorNode (index++));
        nextOperation = primaryList.GetOperation (index);

        bool isPrimitive = (ARRAYKIND_Primitive == arrayProp->GetKind());
        bool is2d = false;
        uint32_t componentIndex = IECTypeAdapterContext::COMPONENT_INDEX_None;
        if (isPrimitive && TOKEN_Dot == nextOperation && isPointArrayProperty (is2d, currentProperty))
            {
            componentIndex = getComponentIndex (primaryList.GetOperatorNode (index), is2d);
            if (IECTypeAdapterContext::COMPONENT_INDEX_None == componentIndex)
                return ExpressionStatus::UnknownMember;

            // we'll get the component value below if no more operations
            nextOperation = primaryList.GetOperation (++index);
            }

        if (isPrimitive && TOKEN_None != nextOperation)
            { evalResult.Clear(); return ExpressionStatus::StructRequired; }

        EvaluationResult indexResult;
        NodePtr indexNode = lBracketNode->GetIndexNode();

        ExpressionStatus exprStatus = indexNode->GetValue (indexResult, globalContext);
        if (ExpressionStatus::Success != exprStatus)
            { evalResult.Clear(); return exprStatus; }
        else if (indexResult.GetValueType() != ValType_ECValue || !indexResult.GetECValue()->ConvertToPrimitiveType (PRIMITIVETYPE_Integer) || indexResult.GetECValue()->IsNull())
            { evalResult.Clear(); return ExpressionStatus::PrimitiveRequired; }

        ECValue arrayVal;
        UnitSpec units;
        if (ECObjectsStatus::Success != instance.GetValue (arrayVal, accessString.c_str(), (uint32_t)indexResult.GetECValue()->GetInteger()))
            { evalResult.Clear(); return ExpressionStatus::UnknownError; }

        if (isPrimitive && IECTypeAdapterContext::COMPONENT_INDEX_None != componentIndex)
            {
            if (arrayVal.IsNull() || (is2d && !arrayVal.IsPoint3D()) || (!is2d && !arrayVal.IsPoint3D()))
                return ExpressionStatus::DotNotSupported;

            DPoint3d pt = !is2d ? arrayVal.GetPoint3D() : DPoint3d::From (arrayVal.GetPoint2D().x, arrayVal.GetPoint2D().y, 0.0);
            double* component = (&pt.x) + componentIndex;
            arrayVal.SetDouble (*component);
            }

        if (isPrimitive && (globalContext.AllowsTypeConversion () || globalContext.EnforceGlobalRepresentation () || globalContext.EnforcesUnits ()))
            {
            IECTypeAdapter* adapter = arrayProp->GetMemberTypeAdapter();
            if (nullptr != adapter)
                {
                IECTypeAdapterContextPtr context = IECTypeAdapterContext::Create (*arrayProp, instance);
                context->SetEvaluationOptions (globalContext.GetEvaluationOptions ());
                if (globalContext.AllowsTypeConversion () || globalContext.EnforceGlobalRepresentation ())
                    {
                    if (adapter->RequiresExpressionTypeConversion (globalContext.GetEvaluationOptions ()) && !adapter->ConvertToExpressionType (arrayVal, *context))
                        {
                        evalResult.Clear();
                        return ExpressionStatus::UnknownError;
                        }
                    }
                else if (adapter->SupportsUnits () && !adapter->GetUnits (units, *context))
                    {
                    evalResult.Clear();
                    return ExpressionStatus::UnknownError;
                    }
                }
            }

        if (TOKEN_None == nextOperation)
            {
            evalResult = arrayVal;
            evalResult.SetUnits (units);
            return ExpressionStatus::Success;
            }

        BeAssert (ARRAYKIND_Struct == arrayProp->GetKind());

        if (arrayVal.IsNull())
            {
            evalResult = arrayVal;
            return ExpressionStatus::Success;
            }
        else if (!arrayVal.IsStruct())
            { evalResult.Clear(); return ExpressionStatus::StructRequired; }

        if (TOKEN_LParen == nextOperation)
            {
            --index;    // place the LParen back in queue, caller will handle it.
            evalResult = arrayVal;
            return ExpressionStatus::Success;
            }
        else
            return GetInstanceValue (evalResult, index, primaryList, globalContext, *arrayVal.GetStruct());
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

    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::GetInstanceValue (EvaluationResultR evalResult, size_t& startIndex, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ECInstanceListCR instanceList)
    {
    BeAssert(m_initialized);

    ExpressionStatus status = ExpressionStatus::StructRequired;
    for (IECInstancePtr const& instance: instanceList)
        {
        size_t index = startIndex;
        if (instance.IsNull())
            {
            BeAssert (false); continue;
            }

        status = GetInstanceValue (evalResult, index, primaryList, globalContext, *instance);
        if (ExpressionStatus::Success == status)
            {
            startIndex = index;
            break;
            }
        }

    return status;
    }
            
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
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
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    Initialize();

    evalResult.SetInstanceList (m_instances, false);

    size_t      numberOfOperators = primaryList.GetNumberOfOperators();
    size_t      index = startIndex;

    if (index == numberOfOperators)
        {
        //  This is an instance expression
        return ExpressionStatus::Success;
        }

    ExpressionToken     nextOperation = primaryList.GetOperation(index);

    while (TOKEN_None != nextOperation)
        {
        if (TOKEN_Dot == nextOperation || TOKEN_Ident == nextOperation || TOKEN_LeftBracket == nextOperation)
            {
            EvaluationResult valueResult;
            ExpressionStatus instanceStatus = GetInstanceValue (valueResult, index, primaryList, globalContext, *evalResult.GetInstanceList());
            if (ExpressionStatus::Success != instanceStatus)
                {
                if (0 == startIndex && 0 == index)
                    {
                    ExpressionContextP  outer = GetOuterP();
                    if (NULL != outer)
                        return outer->GetValue(evalResult, primaryList, globalContext, startIndex);
                    }

                evalResult.Clear();
                return instanceStatus;
                }

            evalResult = valueResult;
            nextOperation = primaryList.GetOperation (++index);
            if (TOKEN_None == nextOperation)
                {
                convertResultToInstanceList (evalResult, false);
                //  -- will we allow passing an struct t a method?
                return ExpressionStatus::Success;
                }
            else if (!evalResult.IsValueList() && !convertResultToInstanceList (evalResult, true))
                {
                evalResult.Clear();
                return ExpressionStatus::StructRequired;
                }
            }

        if (TOKEN_LParen == nextOperation)
            {
            CallNodeP               callNode = static_cast<CallNodeP>(primaryList.GetOperatorNode(index++));
            nextOperation           = primaryList.GetOperation(index);

            EvaluationResult        methodResult;
            ExpressionStatus exprStatus = ExpressionStatus::UnknownError;
            switch (evalResult.GetValueType())
                {
                case ValType_InstanceList:
                    exprStatus = callNode->InvokeInstanceMethod (methodResult, *evalResult.GetInstanceList(), globalContext);
                    break;
                case ValType_ValueList:
                    exprStatus = callNode->InvokeValueListMethod (methodResult, *evalResult.GetValueList(), globalContext);
                    break;
                }

            if (ExpressionStatus::Success != exprStatus)
                {
                evalResult.Clear();
                return exprStatus;
                }

            evalResult = methodResult;
            if (TOKEN_None == nextOperation)
                {
                convertResultToInstanceList (evalResult, false);
                return ExpressionStatus::Success;
                }
            else if (!convertResultToInstanceList (evalResult, true))
                {
                evalResult.Clear();
                return ExpressionStatus::StructRequired;
                }
            }
        }

    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferenceStandard::MethodReferenceStandard(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    : m_staticMethod(staticMethod), m_instanceMethod(instanceMethod), m_valueListMethod(NULL), m_context(context)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferenceStandard::MethodReferenceStandard(ExpressionValueListMethod_t valueListMethod, void* context)
    : m_staticMethod(NULL), m_instanceMethod(NULL), m_valueListMethod(valueListMethod), m_context(context)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments)
    {
    if (NULL == m_staticMethod)
        return ExpressionStatus::StaticMethodRequired;

    return (*m_staticMethod)(evalResult, m_context, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return NULL != m_valueListMethod ? (*m_valueListMethod)(evalResult, m_context, valueList, args) : ExpressionStatus::InstanceMethodRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodReferenceStandard::_InvokeInstanceMethod (EvaluationResultR evalResult, ECInstanceListCR instanceData, EvaluationResultVector& arguments)
    {
    if (NULL == m_instanceMethod)
        return ExpressionStatus::InstanceMethodRequired;

    return (*m_instanceMethod)(evalResult, m_context, instanceData, arguments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferencePtr MethodReferenceStandard::Create(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    {
    return new MethodReferenceStandard(staticMethod, instanceMethod, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
MethodReferencePtr MethodReferenceStandard::Create (ExpressionValueListMethod_t method, void* context)
    {
    return new MethodReferenceStandard (method, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbol::MethodSymbol(Utf8CP name, MethodReferenceR methodReference)
    : Symbol(name)
    {
    m_methodReference = &methodReference;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbol::MethodSymbol (Utf8CP name, ExpressionValueListMethod_t method)
    : Symbol (name)
    {
    m_methodReference = MethodReferenceStandard::Create (method);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbolPtr MethodSymbol::Create(Utf8CP name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context)
    {
    MethodReferencePtr  ref = MethodReferenceStandard::Create(staticMethod, instanceMethod, context);
    return new MethodSymbol(name, *ref);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
MethodSymbolPtr MethodSymbol::Create (Utf8CP name, ExpressionValueListMethod_t method)
    {
    return new MethodSymbol (name, method);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus MethodSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    //  We only get here if the symbol was resolved directly from a context. The context may be a global context, or a namespace context. If the
    //  context is a global context then startIndex is 1 and the callNode should be 0. If the context is a namespace context, then startIndex - 1
    //  should be the context used to find this symbol. It must be a CallNode.
    //
    //  For now assuming this has to be a static method; may want to support having a MethodSymbol in a context. That would require that this
    //  method receive a pointer to the context used to invoke it.

    BeAssert(1 <= startIndex);
    ::uint32_t      callNodeIndex = startIndex - 1;

    if (primaryList.GetOperation(callNodeIndex) != TOKEN_LParen)
        {
        evalResult.InitECValue().Clear();
        return ExpressionStatus::UnknownError;    // Invalid operation on method
        }

    // NEEDSWORK: If a static method returns an ECInstanceList, we can never access properties of those instances??
    if (primaryList.GetNumberOfOperators() != startIndex)
        {
        evalResult.InitECValue().Clear();
        return ExpressionStatus::UnknownError;    // Invalid operation on method
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
    if (NULL != FindCP (symbol.GetName ()))
        return BSIERROR;

    m_symbols.push_back (SymbolPtr (&symbol));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
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
* @bsimethod                                    Grigas.Petraitis                12/2013
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
* @bsimethod                                    Grigas.Petraitis                12/2013
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
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolExpressionContextPtr SymbolExpressionContext::Create(ExpressionContextP outer)
    {
    return new SymbolExpressionContext (outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    Utf8CP ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        return ExpressionStatus::UnknownSymbol;

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()))
            return symbol->GetValue(evalResult, primaryList, globalContext, startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, globalContext, startIndex);

    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_GetReference(EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    Utf8CP ident = primaryList.GetName(startIndex);
    if (NULL == ident)
        return ExpressionStatus::UnknownSymbol;

    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()))
            return symbol->GetReference(evalResult, refResult, primaryList, globalContext, startIndex + 1);
        }

    ExpressionContextP  outer = GetOuterP();
    if (NULL != outer)
        return outer->GetValue(evalResult, primaryList, globalContext, startIndex);

    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus SymbolExpressionContext::_ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary)
    {
    for (bvector<SymbolPtr>::iterator curr = m_symbols.begin(); curr != m_symbols.end(); curr++)
        {
        SymbolP symbol = (*curr).get();
        if (0 == strcmp(ident, symbol->GetName()))
            return symbol->CreateMethodResult(result);
        }

    if (useOuterIfNecessary)
        {
        ExpressionContextP  outer = GetOuterP();
        if (NULL != outer)
            return outer->ResolveMethod(result, ident, useOuterIfNecessary);
        }

    return ExpressionStatus::UnknownSymbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    return m_context->GetValue(evalResult, primaryList, globalContext, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ContextSymbol::_GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    return m_context->GetReference(evalResult, refResult, primaryList, globalContext, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContextSymbolPtr ContextSymbol::CreateContextSymbol(Utf8CP name, ExpressionContextR context) 
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
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ValueSymbol::ValueSymbol (Utf8CP name, EvaluationResultCR value) : Symbol(name)
    {
    m_expressionValue = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ValueSymbol::_GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    if (primaryList.GetNumberOfOperators() > startIndex)
        {
        ExpressionToken token = primaryList.GetOperation(startIndex);
        if (ECN::TOKEN_Dot == token)
            return ExpressionStatus::StructRequired;

        if (ECN::TOKEN_LParen == token)
            return ExpressionStatus::MethodRequired;

        if (ECN::TOKEN_LeftBracket == token)
            return ExpressionStatus::ArrayRequired;

        return ExpressionStatus::UnknownError;
        }

    evalResult = m_expressionValue;
    return ExpressionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2012
//--------------+------------------------------------------------------------------------
ValueSymbolPtr  ValueSymbol::Create(Utf8CP name, ECN::ECValueCR exprValue)
    {
    EvaluationResult evalResult;
    evalResult = exprValue;
    return new ValueSymbol (name, evalResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ValueSymbolPtr ValueSymbol::Create (Utf8CP name, EvaluationResultCR value) { return new ValueSymbol (name, value); }
EvaluationResultCR ValueSymbol::GetValue() const { return m_expressionValue; }
void ValueSymbol::SetValue (EvaluationResultCR value) { m_expressionValue = value; }
void ValueSymbol::SetValue (ECValueCR value) { m_expressionValue = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySymbol::PropertySymbol(Utf8CP name, PropertySymbol::PropertyEvaluator& evaluator) 
    : Symbol(name), m_valueEvaluator(&evaluator) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySymbol::PropertySymbol(Utf8CP name, PropertySymbol::ContextEvaluator& evaluator) 
    : Symbol(name), m_contextEvaluator(&evaluator) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertySymbol> PropertySymbol::Create(Utf8CP name, PropertySymbol::PropertyEvaluator& evaluator)
    {
    return new PropertySymbol(name, evaluator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PropertySymbol> PropertySymbol::Create(Utf8CP name, PropertySymbol::ContextEvaluator& evaluator)
    {
    return new PropertySymbol(name, evaluator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus PropertySymbol::_GetValue (EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::uint32_t startIndex)
    {
    if (m_valueEvaluator.IsValid())
        {
        if (primaryList.GetNumberOfOperators() > startIndex)
            {
            ExpressionToken token = primaryList.GetOperation(startIndex);
            if (ECN::TOKEN_Dot == token)
                return ExpressionStatus::StructRequired;

            if (ECN::TOKEN_LParen == token)
                return ExpressionStatus::MethodRequired;

            if (ECN::TOKEN_LeftBracket == token)
                return ExpressionStatus::ArrayRequired;

            return ExpressionStatus::UnknownError;
            }

        evalResult = m_valueEvaluator->_EvaluateProperty();
        return ExpressionStatus::Success;
        }

    BeAssert(m_contextEvaluator.IsValid());
    BeAssert(m_contextEvaluator->_GetContext().IsValid());
    return m_contextEvaluator->_GetContext()->GetValue(evalResult, primaryList, globalContext, startIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void IECSymbolProvider::RegisterExternalSymbolPublisher (ExternalSymbolPublisher publisher)
    {
    g_externalSymbolPublisher.SetValueAsPointer((void*)publisher);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void IECSymbolProvider::UnRegisterExternalSymbolPublisher ()
    {
    g_externalSymbolPublisher.SetValueAsPointer(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
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
* @bsimethod                                    Bill.Steinbock                  04/2015
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
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceExpressionContextPtr InstanceExpressionContext::Create (ExpressionContextP outer)
    {
    return new InstanceExpressionContext (outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceExpressionContext::SetInstance (IECInstanceCR instance)
    {
    ECInstanceList list;
    list.push_back (const_cast<IECInstanceP>(&instance));
    SetInstanceList (list);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceExpressionContext::SetInstances (ECInstanceListCR instances)
    {
    SetInstanceList (instances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContext::InstanceListExpressionContext (ExpressionContextP outer)
  : ExpressionContext (outer), m_initialized (false)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContext::InstanceListExpressionContext (ECInstanceListCR instances, ExpressionContextP outer)
  : ExpressionContext(outer), m_instances(instances), m_initialized(true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
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
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus InstanceListExpressionContext::_GetReference (EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, uint32_t startIndex)
    {
    Initialize();

    if (startIndex == primaryList.GetNumberOfOperators())
        {
        // This is an instance expression
        return ExpressionStatus::UnknownError;
        }

    ExpressionStatus status = ExpressionStatus::UnknownSymbol;
    Utf8CP name = primaryList.GetName (startIndex);
    if (NULL != name)
        {
        for (IECInstancePtr const& instance: m_instances)
            {
            if (instance.IsValid() && ExpressionStatus::Success == (status = GetReference (evalResult, refResult, primaryList, globalContext, startIndex, *instance)))
                break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceListExpressionContextPtr InstanceListExpressionContext::Create (ECInstanceListCR instances, ExpressionContextP outer)
    {
    return new InstanceListExpressionContext (instances, outer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionContext::AllowsTypeConversion() const
    {
    return 0 == (GetEvaluationOptions() & EVALOPT_SuppressTypeConversions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpressionContext::SetEvaluationOptions (EvaluationOptions opts)
    {
    m_options = opts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionContext::EnforcesUnits() const
    {
    return 0 != (GetEvaluationOptions() & EVALOPT_EnforceUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eligijus.Mauragas 01/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionContext::EnforceGlobalRepresentation () const
    {
    return 0 != (GetEvaluationOptions () & EVALOPT_EnforceGlobalRepresentation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
EvaluationOptions ExpressionContext::GetEvaluationOptions() const
    {
    return nullptr != GetOuterP() ? GetOuterP()->GetEvaluationOptions() : m_options;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
