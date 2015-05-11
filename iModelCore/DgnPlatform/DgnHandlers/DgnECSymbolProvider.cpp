/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnECSymbolProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DgnECSymbolProvider.h>
#include    <ECObjects/ECExpressionNode.h>
#include    <ECObjects/SystemSymbolProvider.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_EC


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

// Extract a single argument from the list at the specified index
template<typename T>
static bool ExtractArg (T& str, ECN::EvaluationResultVector const& args, size_t index, bool allowNull)
    {
    return index < args.size() ? SystemSymbolProvider::ExtractArg (str, args[index], allowNull) : false;
    }
template<typename T>
static bool ExtractArg (T& extractedValue, ECN::EvaluationResultVector const& args, size_t index)
    {
    return index < args.size() ? SystemSymbolProvider::ExtractArg (extractedValue, args[index]) : false;
    }

#ifdef NOT_USED
// Extract a DPoint3d from 3 numeric arguments beginning at startIndex
static bool ExtractArg (DPoint3d& p, ECN::EvaluationResultVector const& args, size_t startIndex)
    {
    return SystemSymbolProvider::ExtractArg (p.x, args, startIndex) && SystemSymbolProvider::ExtractArg (p.y, args, startIndex+1) && SystemSymbolProvider::ExtractArg (p.z, args, startIndex+2);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECSymbolProvider::DgnECSymbolProvider()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::RegisterSymbolProvider (IECSymbolProviderCR provider)
    {
    if (m_symbolProviders.end() == std::find_if(m_symbolProviders.begin(), m_symbolProviders.end(), [&] (IECSymbolProviderCP existing) { return existing == &provider; }))
        m_symbolProviders.push_back(&provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::UnregisterSymbolProvider (IECSymbolProviderCR provider)
    {
    auto found = std::find_if (m_symbolProviders.begin(), m_symbolProviders.end(), [&](IECSymbolProviderCP existing) { return existing == &provider; });
    if (found != m_symbolProviders.end())
        m_symbolProviders.erase(found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECSymbolProvider& DgnECSymbolProvider::GetProvider()
    {
    static DgnHost::Key s_hostKey;

    DgnECSymbolProvider* provider = static_cast<DgnECSymbolProvider*> (T_HOST.GetHostObject (s_hostKey));
    if (NULL == provider)
        {
        provider = new DgnECSymbolProvider();
        T_HOST.SetHostObject (s_hostKey, provider);
        }

    return *provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::ExternalSymbolPublisher (SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets)
    {
    return GetProvider().PublishSymbols (context, requestedSymbolSets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::_OnHostTermination (bool)         { delete this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaR DgnECSymbolProvider::GetSchema()
    {
    if (m_schema.IsNull())
        {
        ECSchema::CreateSchema (m_schema, L"PseudoSchema", 1, 0);
        ECClassP ecClass;
        m_schema->CreateClass (ecClass, L"ECClass");
        PrimitiveECPropertyP ecProp;
        ecClass->CreatePrimitiveProperty (ecProp, L"Name", PRIMITIVETYPE_String);
        ecClass->CreatePrimitiveProperty (ecProp, L"DisplayLabel", PRIMITIVETYPE_String);
        }

    BeAssert (m_schema.IsValid());
    return *m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::InitDefaultMethods()
    {
    m_defaultMethods.reserve (9);
    m_defaultMethods.push_back (MethodSymbol::Create (L"GetInstanceId", NULL, &GetInstanceId));
    m_defaultMethods.push_back (MethodSymbol::Create (L"GetInstanceLabel", NULL, &GetInstanceLabel));
    m_defaultMethods.push_back (MethodSymbol::Create (L"GetClass", NULL, &GetClass));
#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    m_defaultMethods.push_back (MethodSymbol::Create (L"GetRelatedInstance", NULL, &GetRelatedInstance));
#endif

    m_defaultMethods.push_back (MethodSymbol::Create (L"IsOfClass", NULL, &IsOfClass));

#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    m_defaultMethods.push_back (MethodSymbol::Create (L"IsPropertyValueSet", NULL, &IsPropertyValueSet));
    m_defaultMethods.push_back (MethodSymbol::Create (L"ResolveSymbology", NULL, &ResolveSymbology));
#endif

    m_defaultMethods.push_back (MethodSymbol::Create (L"AnyMatches", &AnyMatches));
    m_defaultMethods.push_back (MethodSymbol::Create (L"AllMatch", &AllMatch));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::_PublishSymbols (SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets) const
    {
    if (m_defaultMethods.empty())
        const_cast<DgnECSymbolProvider&>(*this).InitDefaultMethods();

    // Note these symbols are provided regardless of whether or not they are requested.
    for (MethodSymbolPtr const& method: m_defaultMethods)
        {
        context.AddSymbol (*method);
        }

    for (IECSymbolProviderCP const& provider: m_symbolProviders)
        {
        provider->PublishSymbols (context, requestedSymbolSets);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Handles AnyMatches() and AllMatch() list methods
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct PredicateProcessor : LambdaValue::IProcessor
    {
private:
    bool                    m_valueToStopOn;
    bool                    m_ignoreErrors;
    EvaluationResult&       m_result;
    ExpressionStatus        m_status;

    PredicateProcessor (bool valueToStopOn, bool ignoreErrors, EvaluationResult& result)
        : m_valueToStopOn(valueToStopOn), m_ignoreErrors(ignoreErrors), m_result(result), m_status(ExprStatus_Success)
        {
        m_result.InitECValue().SetBoolean (!valueToStopOn);
        }

    virtual bool ProcessResult (ExpressionStatus status, EvaluationResultCR member, EvaluationResultCR result) override
        {
        if (ExprStatus_Success != status)
            return m_ignoreErrors;  // stop iteration if we're not ignoring errors
        else if (!result.IsECValue() || !result.GetECValue()->IsBoolean())
            {
            m_status = ExprStatus_UnknownError;
            return false;   // stop iteration
            }
        else if (m_valueToStopOn == result.GetECValue()->GetBoolean())
            {
            m_result = result;
            return false;   // stop, we found our answer
            }
        else
            return true;    // continue looking
        }

    static ExpressionStatus     Execute (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args, bool target, bool ignoreErrors)
        {
        LambdaValueCP lambda;
        if (!ExtractArg (lambda, args, 0) || NULL == lambda)
            return ExprStatus_UnknownError;

        PredicateProcessor proc (target, ignoreErrors, evalResult);
        ExpressionStatus status = lambda->Evaluate (valueList, proc);
        if (ExprStatus_Success == status)
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
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::AnyMatches (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return PredicateProcessor::Any (evalResult, valueList, args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::AllMatch (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& args)
    {
    return PredicateProcessor::All (evalResult, valueList, args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetInstanceId (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        return ExprStatus_StructRequired;

    for (IECInstancePtr const& instance: instanceData)
        {
        WString instanceId = instance->GetInstanceId();
        if (!WString::IsNullOrEmpty (instanceId.c_str()))
            {
            evalResult.InitECValue().SetString (instanceId.c_str());
            return ExprStatus_Success;
            }
        }

    return ExprStatus_UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetInstanceLabel (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        return ExprStatus_StructRequired;

    for (IECInstancePtr const& instance: instanceData)
        {
        WString displayLabel;
        if (ECOBJECTS_STATUS_Success == instance->GetDisplayLabel (displayLabel))
            {
            evalResult.InitECValue().SetString (displayLabel.c_str());
            return ExprStatus_Success;
            }
        }

    return ExprStatus_UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetClass (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        return ExprStatus_StructRequired;

    ECInstanceList classInstances;
    StandaloneECEnablerR classEnabler = *GetProvider().GetSchema().GetClassCP (L"ECClass")->GetDefaultStandaloneEnabler();
    for (IECInstancePtr const& instance: instanceData)
        {
        ECClassCR ecClass = instance->GetEnabler().GetClass();
        IECInstancePtr classInstance = classEnabler.CreateInstance();

        ECValue v;
        v.SetString (ecClass.GetName().c_str());
        classInstance->SetValue (L"Name", v);
        v.SetString (ecClass.GetDisplayLabel().c_str());
        classInstance->SetValue (L"DisplayLabel", v);

        classInstances.push_back (classInstance);
        }

    evalResult.SetInstanceList (classInstances, true);
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassVisitor
    {
    virtual bool        Process (ECClassCR ecClass) = 0;    // return true to stop processing

    ECClassCP           Visit (ECClassCR ecClass)
        {
        if (Process (ecClass))
            return &ecClass;
        else
            {
            for (const ECClassP& baseClass: ecClass.GetBaseClasses())
                {
                if (Process (*baseClass))
                    return baseClass;
                }
            }

        return NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus extractPropertyAccessor (WCharCP& schemaName, WCharCP& className, WCharCP& accessString, EvaluationResultVector& args)
    {
    if (!ExtractArg (schemaName, args, 0, false) || !ExtractArg (className, args, 1, false) || !ExtractArg (accessString, args, 2, false))
        return ExprStatus_UnknownError;
    else
        return ExprStatus_Success;
    }

typedef bpair<IECInstanceCP, ECPropertyCP> InstancePropertyPair;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
InstancePropertyPair extractProperty (ExpressionStatus& status, ECInstanceListCR instanceData, EvaluationResultVector& args, WCharCP& accessString)
    {
    InstancePropertyPair pair (nullptr, nullptr);
    WCharCP schemaName, className;
    status = extractPropertyAccessor (schemaName, className, accessString, args);
    if (ExprStatus_Success == status)
        {
        for (auto const& instance : instanceData)
            {
            if (instance->GetClass().Is (schemaName, className))
                {
                auto prop = instance->GetEnabler().LookupECProperty (accessString);
                        
                if (nullptr != prop)
                    pair = InstancePropertyPair (instance.get(), prop);

                break;
                }
            }
        }

    return pair;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::IsOfClass (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    IECInstancePtr instance;
    WCharCP schemaname, classname;
    if (2 != args.size() || !SystemSymbolProvider::ExtractArg (classname, args[0]) || !SystemSymbolProvider::ExtractArg (schemaname, args[1]))
        return ExprStatus_UnknownError;

    bool found = false;
    for (IECInstancePtr const& instance: instanceData)
        {
        found = instance->GetClass().Is (schemaname, classname);
        if (found)
            break;
        }

    evalResult.InitECValue().SetBoolean (found);
    return ExprStatus_Success;
    }


#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::IsPropertyValueSet (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    ExpressionStatus status;
    WCharCP accessString;
    InstancePropertyPair pair = extractProperty (status, instanceData, args, accessString);
    if (ExprStatus_Success == status)
        {
        IECInstanceCP instance = pair.first;
        ECPropertyCP prop = pair.second;
            if (nullptr == prop)
                    return ExprStatus_UnknownError;
        
        bool isSet = false;
        IDgnECTypeAdapterP adapter = nullptr != prop && nullptr != instance ? &IDgnECTypeAdapter::GetForProperty (*prop) : nullptr;

        if (nullptr != adapter)
            {
            ECValue sigilValue;
            ECValue actualValue;
            bool sigilValueAvailable  = adapter->GetPropertyNotSetValue (sigilValue);
            bool actualValueAvailable = ECOBJECTS_STATUS_Success == instance->GetValue (actualValue, accessString);
            if (sigilValueAvailable && actualValueAvailable && !actualValue.IsNull () && !actualValue.Equals (sigilValue))
                isSet = true;
            else if (!sigilValueAvailable && actualValueAvailable && !actualValue.IsNull ())
                isSet = true;
            }
        evalResult.InitECValue().SetBoolean (isSet);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::ResolveSymbology (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    ExpressionStatus status;
    WCharCP accessString;
    InstancePropertyPair pair = extractProperty (status, instanceData, args, accessString);
    if (ExprStatus_Success != status)
        return status;

    IECInstanceCP instance = pair.first;
    ECPropertyCP prop = pair.second;
    ECValue v;
    if (nullptr == instance || nullptr == prop)
        return ExprStatus_UnknownMember;
    else if (ECOBJECTS_STATUS_Success != instance->GetValue (v, accessString))
        return ExprStatus_UnknownError;

    IDgnECTypeAdapterR adapter = IDgnECTypeAdapter::GetForProperty (*prop);
    auto resolver = dynamic_cast<IDgnECSymbologyResolver const*>(&adapter);
    if (nullptr != resolver)
        {
        auto context = IDgnECTypeAdapterContext::CreateStandalone (*prop, *instance, accessString);
        if (!resolver->ResolveSymbology (v, *context))
            return ExprStatus_UnknownError;

        // NEEDSWORK:
        //  We introduced this method for use in Condition Editor (display rules, ecreporting)
        //  We'll have an expression like "this.ResolveSymbology ("Color") = '1'"
        //  Importantly, the color on the right-hand side is going to be expressed AS A STRING, not the underlying type (e.g. int)
        //  That conversion is actually controlled by EvaluationOptions on the ExpressionContext. But we do not have access to the ExpressionContext from here.
        //  Sooo....
        //  We will convert the left-hand value to a string, until we find a use case in which that is not sufficient
        if (adapter.RequiresExpressionTypeConversion (EVALOPT_EnforceGlobalRepresentation))
            adapter.ConvertToExpressionType (v, *context);
        }

    evalResult.InitECValue() = v;
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetRelatedInstance (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    // This method takes a single string argument of the format:
    //  "relationshipClassName:dir:relatedClassName[, PropertyName:FailureValue]"
    // where dir: 0=forward, 1=backward
    // It doesn't specify schema names for either class, which makes our job more difficult and more time-consuming
    // If we fail to find any related instance, and PropertyName:FailureValue is specified, then we create a fake IECInstance
    // with a single property "PropertyName" with the value of FailureValue

    WCharCP rawArg;
    if (1 != args.size() || 0 == instanceData.size() || !SystemSymbolProvider::ExtractArg (rawArg, args[0]))
        return ExprStatus_UnknownError;

    bvector<WString> argTokens;
    BeStringUtilities::Split(rawArg, L",", NULL, argTokens);
    if (1 > argTokens.size() || 2 < argTokens.size())
        return ExprStatus_UnknownError;

    WString arg = argTokens[0];     // "relationship:dir:related"
    WString failureSpec = (argTokens.size() > 1) ? argTokens[1] : L"";  // " PropertyName:FailureValue
    argTokens.clear();

    BeStringUtilities::Split(arg.c_str(), L":", NULL, argTokens);
    if (argTokens.size() < 3 || argTokens.size() > 4 || argTokens[1].length() != 1)
        return ExprStatus_UnknownError;

    ECRelatedInstanceDirection direction;
    switch (argTokens[1][0])
        {
    case '0':       direction = STRENGTHDIRECTION_Forward; break;
    case '1':       direction = STRENGTHDIRECTION_Backward; break;
    default:        return ExprStatus_UnknownError;
        }

    WCharCP   relationshipName = argTokens[0].c_str(),
              relatedName      = argTokens[2].c_str();

    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& iecInstance: instanceData)
        {
        DgnECInstanceCP instance = iecInstance->AsDgnECInstanceCP();
        if (NULL == instance)
            continue;

        // We may get lucky and find the related/relationship classes in the same schema as this instance's class
        ECClassCP relatedClass = instance->GetEnabler().GetClass().GetSchema().GetClassCP (relationshipName);
        ECRelationshipClassCP relationshipClass = relatedClass ? relatedClass->GetRelationshipClassCP() : NULL;
        if (NULL != relatedClass)
            relatedClass = instance->GetEnabler().GetClass().GetSchema().GetClassCP (relatedName);

        QueryRelatedClassSpecifierPtr relClassSpec = QueryRelatedClassSpecifier::Create (relationshipClass, relatedClass, direction, true, true);   // allow polymorphism

        DgnECInstanceCreateContext context(SelectedProperties::Create (true).get(), DgnECInstanceCreateOptions (false, false));
        if (NULL != relationshipClass && NULL != relatedClass)
            {
            DgnECInstanceIterable relatedInstances = DgnECManager::GetManager().FindRelatedInstances (*instance, *relClassSpec, context);
            if (relatedInstances.begin() != relatedInstances.end())
                relatedInstance = *relatedInstances.begin();
            }
        else
            {
            // We have to get all of the relationships for this instance and compare the class names
            for (IDgnECRelationshipInstanceP const& relationship: DgnECManager::GetManager().FindRelationships (*instance, *relClassSpec, context))
                {
                if (!relationship->GetEnabler().GetClass().GetName().Equals (relationshipName))
                    continue;   // note this does not support polymorphism

                ECClassCP foundRelatedClass = GetRelatedClassDefinition (*relationship, direction, relatedName);
                if (NULL != foundRelatedClass)
                    {
                    relatedInstance = (STRENGTHDIRECTION_Forward == direction) ? relationship->GetTarget().get() : relationship->GetSource().get();
                    break;
                    }
                }
            }

        if (relatedInstance.IsValid())
            break;
        }

    if (relatedInstance.IsNull() && !failureSpec.empty())
        relatedInstance = CreatePseudoRelatedInstance (failureSpec.c_str());

    
    if (relatedInstance.IsValid())
        evalResult.SetInstance (*relatedInstance);
    else
        evalResult.InitECValue().SetToNull();

    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnECSymbolProvider::GetRelatedClassDefinition (ECN::IECRelationshipInstanceCR relationship, ECN::ECRelatedInstanceDirection dir, WCharCP relatedClassName)
    {
    // This supports polymorphism
    ECClassCR actualRelatedClass = (STRENGTHDIRECTION_Forward == dir) ? relationship.GetTarget()->GetEnabler().GetClass() : relationship.GetSource()->GetEnabler().GetClass();
    struct Visitor : ClassVisitor
        {
        WCharCP relatedClassName;
        Visitor (WCharCP rcn) : relatedClassName(rcn) { }
        virtual bool Process (ECClassCR ecclass) override
            {
            return ecclass.GetName().Equals (relatedClassName);
            }
        } visitor (relatedClassName);

    return visitor.Visit (actualRelatedClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr DgnECSymbolProvider::CreatePseudoRelatedInstance (WCharCP spec)
    {
    // Extract the property name and failure value
    while (*spec && ' ' == *spec)
        ++spec;

    bvector<WString> parts;
    BeStringUtilities::Split(spec, L":", NULL, parts);
    if (2 == parts.size() && parts[0].length() > 0 && parts[1].length() > 0)
        {
        WCharCP propName = parts[0].c_str(),
                propVal  = parts[1].c_str();

        // We have to create a separate ECClass for each distinct property name used.
        ECSchemaR schema = GetProvider().GetSchema();
        ECClassP pseudoClass = NULL;
        uint32_t classCount = 0;
        for (ECClassContainer::const_iterator iter = schema.GetClasses().begin(), end = schema.GetClasses().end(); iter != end; ++iter)
            {
            ECClassP curClass = *iter;
            if (0 == wcsncmp (curClass->GetName().c_str(), L"pseudo", _countof(L"pseudo") - 1))
                {
                if (NULL != curClass->GetPropertyP (propName))
                    {
                    pseudoClass = curClass;
                    break;
                    }
                else
                    ++classCount;
                }
            }

        if (NULL == pseudoClass)
            {
            WChar pseudoClassName[0x20];
            BeStringUtilities::Snwprintf (pseudoClassName, _countof(pseudoClassName), L"pseudo%d", classCount);
            schema.CreateClass (pseudoClass, pseudoClassName);
            PrimitiveECPropertyP ecprop;
            pseudoClass->CreatePrimitiveProperty (ecprop, propName, PRIMITIVETYPE_String);
            }

        BeAssert (NULL != pseudoClass);
        IECInstancePtr instance = pseudoClass->GetDefaultStandaloneEnabler()->CreateInstance();
        instance->SetValue (propName, ECValue (propVal));
        return instance;
        }

    return NULL;
    }
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE



