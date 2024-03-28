/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifndef DOCUMENTATION_GENERATOR
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct SystemSymbolProvider : ECN::IECSymbolProvider
    {
private:
    mutable ECN::SymbolPtr  m_systemNamespaceSymbol;

private:
    virtual Utf8CP  _GetName() const override { return "SystemSymbolProvider"; }
    void            _PublishSymbols (ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;
    static ExpressionStatus AnyMatches(EvaluationResult& evalResult, void* context, IValueListResultCR valueList, EvaluationResultVector& args);
    static ExpressionStatus AllMatch(EvaluationResult& evalResult, void* context, IValueListResultCR valueList, EvaluationResultVector& args);

public:
    ECOBJECTS_EXPORT SystemSymbolProvider();

public:
    // Helper routines for extracting arguments. Performs appropriate conversions e.g. int => double
    // Extract a single argument
    ECOBJECTS_EXPORT static bool ExtractArg (WCharCP& str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (WStringR str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (Utf8CP& str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (Utf8StringR str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (double& d, ECN::EvaluationResultCR ev);
    ECOBJECTS_EXPORT static bool ExtractArg (int32_t& i, ECN::EvaluationResultCR ev);
    ECOBJECTS_EXPORT static bool ExtractArg (ECN::LambdaValueCP& lambda, ECN::EvaluationResultCR ev);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
#endif
