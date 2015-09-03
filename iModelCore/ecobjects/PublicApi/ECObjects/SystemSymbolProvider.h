/*----------------------------------------------------------------------+
|
|   $Source: PublicApi/ECObjects/SystemSymbolProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct SystemSymbolProvider : ECN::IECSymbolProvider
    {
private:
    mutable ECN::SymbolPtr  m_systemNamespaceSymbol;

    virtual Utf8CP          _GetName() const override { return "SystemSymbolProvider"; }
    virtual void            _PublishSymbols (ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;
public:
    SystemSymbolProvider();

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

