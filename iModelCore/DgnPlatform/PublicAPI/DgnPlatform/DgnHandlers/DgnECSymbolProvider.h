/*----------------------------------------------------------------------+
|
|   $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnECSymbolProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Provides method symbols for IECInstances.
* May also provide wrappers for managed symbol providers, or native implementations of
* common symbols like System.Math.* and System.String.*
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECSymbolProvider : ECN::IECSymbolProvider, DgnHost::IHostObject
    {
private:
    DgnECSymbolProvider();

    ECN::ECSchemaPtr                    m_schema;
    bvector<ECN::MethodSymbolPtr>       m_defaultMethods;
    bvector<ECN::IECSymbolProviderCP>   m_symbolProviders;

    void                    InitDefaultMethods();

    // IECSymbolProvider
    virtual WCharCP         _GetName() const override           { return L"DgnECSymbolProvider"; }
    virtual void            _PublishSymbols (ECN::SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets) const override;

    // ECInstance methods for use in ECExpressions
    static ECN::ExpressionStatus GetInstanceId (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetInstanceLabel (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetClass (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus IsOfClass (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);

#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    static ECN::ExpressionStatus GetRelatedInstance (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);

    static ECN::ExpressionStatus IsPropertyValueSet (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus ResolveSymbology (ECN::EvaluationResult& evalResult, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
#endif
    // IValueListResult methods for use in ECExpressions
    static ECN::ExpressionStatus AnyMatches (ECN::EvaluationResult& evalResult, ECN::IValueListResultCR valueList, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus AllMatch (ECN::EvaluationResult& evalResult, ECN::IValueListResultCR valueList, ECN::EvaluationResultVector& args);

    // Helpers for ECInstance methods
    ECN::ECSchemaR                   GetSchema();
#ifdef DETERMINE_NEED_TO_SUPPORT_IN_GRAPHITE
    static ECN::IECInstancePtr       CreatePseudoRelatedInstance (WCharCP);
    static ECN::ECClassCP            GetRelatedClassDefinition (ECN::IECRelationshipInstanceCR relationship, ECN::ECRelatedInstanceDirection dir, WCharCP relatedClassName);
#endif

public:
    // This method is injected into ECObjects to provide symbols.
    static void             ExternalSymbolPublisher (ECN::SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets);

    DGNPLATFORM_EXPORT static DgnECSymbolProvider& GetProvider();

    // Register a provider. DgnECSymbolProvider::_PublishSymbols() will invoke each registered provider's _PublishSymbols() method.
    DGNPLATFORM_EXPORT void RegisterSymbolProvider (ECN::IECSymbolProviderCR provider);
    // Unregister a provider.
    DGNPLATFORM_EXPORT void UnregisterSymbolProvider (ECN::IECSymbolProviderCR provider);
    };

//=======================================================================================
// Context that defines the DgnDb to use.
// @bsiclass                                                     Bill.Steinbock  05/2015
//=======================================================================================
struct DgnDbExpressionContext : ECN::SymbolExpressionContext
    {
    private:
        DgnDbR m_db;

        Utf8CP  GetPath() const;
        ECN::ECValue GetName() const;

    protected:
        // Create an ECExpression context that allows access to DgnDb.
        DGNPLATFORM_EXPORT DgnDbExpressionContext(DgnDbR db);
        // Get the DgnDb associated with the Context.
        DGNPLATFORM_EXPORT DgnDbR GetDgnDb() const;

    public:
        // Create an ECExpression context that allows access to DgnDb.
        DGNPLATFORM_EXPORT static DgnDbExpressionContextPtr Create(DgnDbR db);
    };

//=======================================================================================
// Context that defines the DgnElement to use.
// @bsiclass                                                     Bill.Steinbock  05/2015
//=======================================================================================
struct DgnElementExpressionContext : DgnDbExpressionContext
    {
    private:
        DgnElementCR m_element;

        ECN::ECValue  GetClassName() const;
        ECN::ECValue  GetFullClassName() const;

    protected:
        // Create an ECExpression context that allows access to DgnDb.
        DGNPLATFORM_EXPORT DgnElementExpressionContext(DgnElementCR element);
        // Get the DgnDb associated with the Context.
        DGNPLATFORM_EXPORT DgnElementCR GetElement() const;

    public:
        // Create an ECExpression context that allows access to DgnDb.
        DGNPLATFORM_EXPORT static DgnElementExpressionContextPtr Create(DgnElementCR element);
    };


END_BENTLEY_DGNPLATFORM_NAMESPACE

