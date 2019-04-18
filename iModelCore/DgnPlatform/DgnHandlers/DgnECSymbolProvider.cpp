/*--------------------------------------------------------------------------------------+ 
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnECSymbolProvider.h>
#include    <ECObjects/ECExpressionNode.h>
#include    <ECObjects/ECExpressions.h>
#include    <ECObjects/SystemSymbolProvider.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC


BEGIN_BENTLEY_DGN_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////
// DgnDbExpressionContext
////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
Utf8CP DgnDbExpressionContext::GetPath() const
    {
    return m_db.GetDbFileName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnDbExpressionContext::GetName() const
    {
    return ECValue(m_db.GetFileName().GetFileNameWithoutExtension().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnDbExpressionContext::DgnDbExpressionContext(DgnDbR db) : m_db(db), SymbolExpressionContext(NULL)
    {
    SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);

    methodContext->AddSymbol(*PropertySymbol::Create<DgnDbExpressionContext, Utf8CP>("Path", *this, &DgnDbExpressionContext::GetPath));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnDbExpressionContext, ECValue>("Name", *this, &DgnDbExpressionContext::GetName));

    AddSymbol (*ContextSymbol::CreateContextSymbol("DgnDb", *methodContext));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnDbR DgnDbExpressionContext::GetDgnDb() const
    {
    return m_db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnDbExpressionContextPtr DgnDbExpressionContext::Create(DgnDbR db) 
    { 
    return new DgnDbExpressionContext(db); 
    }

////////////////////////////////////////////////////////////////////////////////////////
// DgnElementExpressionContext
////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::GetClassName() const
    {
    return ECValue(m_element.GetElementClass()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::GetFullClassName() const
    {
    return ECValue(m_element.GetElementClass()->GetFullName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::GetUserLabel() const
    {
    return ECValue(m_element.GetUserLabel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::HasUserLabel() const
    {
    return ECValue(m_element.HasUserLabel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::GetDisplayLabel() const
    {
    return ECValue(m_element.GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::GetCodeValue() const
    {
    return ECValue(m_element.GetCode().GetValueUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  11/2015
//---------------------------------------------------------------------------------------
ECValue DgnElementExpressionContext::HasGeometry() const
    {
    GeometrySourceCP geometrySource = m_element.ToGeometrySource();
    if (nullptr == geometrySource)
        return ECValue(false);

    return ECValue(geometrySource->HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnElementCR DgnElementExpressionContext::GetElement() const
    {
    return m_element;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnElementExpressionContext::DgnElementExpressionContext(DgnElementCR element) : m_element(element), DgnDbExpressionContext(element.GetDgnDb())
    {
    SymbolExpressionContextPtr methodContext = SymbolExpressionContext::Create(NULL);

    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("ClassName", *this, &DgnElementExpressionContext::GetClassName));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("FullClassName", *this, &DgnElementExpressionContext::GetFullClassName));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("DisplayLabel", *this, &DgnElementExpressionContext::GetDisplayLabel));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("UserLabel", *this, &DgnElementExpressionContext::GetUserLabel));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("HasUserLabel", *this, &DgnElementExpressionContext::HasUserLabel));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("CodeValue", *this, &DgnElementExpressionContext::GetCodeValue));
    methodContext->AddSymbol(*PropertySymbol::Create<DgnElementExpressionContext, ECValue>("HasGeometry", *this, &DgnElementExpressionContext::HasGeometry));

    AddSymbol(*ContextSymbol::CreateContextSymbol("element", *methodContext));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  05/2015
//---------------------------------------------------------------------------------------
DgnElementExpressionContextPtr DgnElementExpressionContext::Create(DgnElementCR element)
    {
    return new DgnElementExpressionContext(element);
    }


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
void DgnECSymbolProvider::ExternalSymbolPublisher (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets)
    {
    return GetProvider().PublishSymbols (context, requestedSymbolSets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaR DgnECSymbolProvider::GetSchema()
    {
    if (m_schema.IsNull())
        {
        ECSchema::CreateSchema (m_schema, "PseudoSchema", "PseudoSchema", 1, 0, 0);
        ECEntityClassP ecClass;
        m_schema->CreateEntityClass (ecClass, "ECClass");
        PrimitiveECPropertyP ecProp;
        ecClass->CreatePrimitiveProperty (ecProp, "Name", PRIMITIVETYPE_String);
        ecClass->CreatePrimitiveProperty (ecProp, "DisplayLabel", PRIMITIVETYPE_String);
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
    m_defaultMethods.push_back (MethodSymbol::Create ("GetInstanceId", NULL, &GetInstanceId));
    m_defaultMethods.push_back (MethodSymbol::Create ("GetInstanceLabel", NULL, &GetInstanceLabel));
    m_defaultMethods.push_back (MethodSymbol::Create ("GetClass", NULL, &GetClass));
#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    m_defaultMethods.push_back (MethodSymbol::Create ("GetRelatedInstance", NULL, &GetRelatedInstance));
#endif

    m_defaultMethods.push_back (MethodSymbol::Create ("IsOfClass", NULL, &IsOfClass));

#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    m_defaultMethods.push_back (MethodSymbol::Create ("IsPropertyValueSet", NULL, &IsPropertyValueSet));
    m_defaultMethods.push_back (MethodSymbol::Create ("ResolveSymbology", NULL, &ResolveSymbology));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECSymbolProvider::_PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const
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
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetInstanceId (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::GetInstanceId: StructRequired. No instance data");
        return ExpressionStatus::StructRequired;
        }

    for (IECInstancePtr const& instance: instanceData)
        {
        Utf8String instanceId = instance->GetInstanceId();
        if (!Utf8String::IsNullOrEmpty (instanceId.c_str()))
            {
            evalResult.InitECValue().SetUtf8CP (instanceId.c_str());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("DgnECSymbolProvider::GetInstanceId: Result: %s", evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        }
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::GetInstanceId: UnknownError. Could not get instance Id");
    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetInstanceLabel (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::GetInstanceLabel: StructRequired. No instance data");
        return ExpressionStatus::StructRequired;
        }

    for (IECInstancePtr const& instance: instanceData)
        {
        Utf8String displayLabel;
        if (ECObjectsStatus::Success == instance->GetDisplayLabel (displayLabel))
            {
            evalResult.InitECValue().SetUtf8CP (displayLabel.c_str());
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("DgnECSymbolProvider::GetInstanceLabel: Result: %s", evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        }
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::GetInstanceLabel: UnknownError. Could not get instance label");
    return ExpressionStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus DgnECSymbolProvider::GetClass (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (0 == instanceData.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::GetClass: StructRequired. No instance data");
        return ExpressionStatus::StructRequired;
        }

    ECInstanceList classInstances;
    StandaloneECEnablerR classEnabler = *GetProvider().GetSchema().GetClassCP ("ECClass")->GetDefaultStandaloneEnabler();
    for (IECInstancePtr const& instance: instanceData)
        {
        ECClassCR ecClass = instance->GetEnabler().GetClass();
        IECInstancePtr classInstance = classEnabler.CreateInstance();

        ECValue v;
        v.SetUtf8CP (ecClass.GetName().c_str());
        classInstance->SetValue ("Name", v);
        v.SetUtf8CP (ecClass.GetDisplayLabel().c_str());
        classInstance->SetValue ("DisplayLabel", v);

        classInstances.push_back (classInstance);
        }

    evalResult.SetInstanceList (classInstances, true);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("DgnECSymbolProvider::GetClass: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
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
ExpressionStatus extractPropertyAccessor (Utf8CP& schemaName, Utf8CP& className, Utf8CP& accessString, EvaluationResultVector& args)
    {
    if (!ExtractArg(schemaName, args, 0, false) || !ExtractArg(className, args, 1, false) || !ExtractArg(accessString, args, 2, false))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "extractPropertyAccessor: UnknownError. Could not extract property accesssor");
        return ExpressionStatus::UnknownError;
        }
    else
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("extractPropertyAccessor: Result: %s, %s, %s",schemaName, className, accessString).c_str());
        return ExpressionStatus::Success;
        }
    }
typedef bpair<IECInstanceCP, ECPropertyCP> InstancePropertyPair;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
InstancePropertyPair extractProperty (ExpressionStatus& status, ECInstanceListCR instanceData, EvaluationResultVector& args, Utf8CP& accessString)
    {
    InstancePropertyPair pair (nullptr, nullptr);
    Utf8CP schemaName, className;
    status = extractPropertyAccessor (schemaName, className, accessString, args);
    if (ExpressionStatus::Success == status)
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
ExpressionStatus DgnECSymbolProvider::IsOfClass (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    IECInstancePtr instance;
    Utf8CP schemaname, classname;
    if (2 != args.size() || !SystemSymbolProvider::ExtractArg(classname, args[0]) || !SystemSymbolProvider::ExtractArg(schemaname, args[1]))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "DgnECSymbolProvider::IsOfClass: UnknownError (Number of arguments is not 2 or could not extract arguments)");
        return ExpressionStatus::UnknownError;
        }

    bool found = false;
    for (IECInstancePtr const& instance: instanceData)
        {
        found = instance->GetClass().Is (schemaname, classname);
        if (found)
            break;
        }

    evalResult.InitECValue().SetBoolean (found);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("DgnECSymbolProvider::IsOfClass: Result: ", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

END_BENTLEY_DGN_NAMESPACE



