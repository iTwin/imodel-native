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

    virtual WCharCP         _GetName() const override { return L"SystemSymbolProvider"; }
    virtual void            _PublishSymbols (ECN::SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets) const override;
public:
    SystemSymbolProvider();

public:
    // Helper routines for extracting arguments. Performs appropriate conversions e.g. int => double
    // Extract a single argument
    ECOBJECTS_EXPORT static bool ExtractArg (WCharCP& str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (WStringR str, ECN::EvaluationResultCR ev, bool allowNull = false);
    ECOBJECTS_EXPORT static bool ExtractArg (double& d, ECN::EvaluationResultCR ev);
    ECOBJECTS_EXPORT static bool ExtractArg (int32_t& i, ECN::EvaluationResultCR ev);
    ECOBJECTS_EXPORT static bool ExtractArg (ECN::LambdaValueCP& lambda, ECN::EvaluationResultCR ev);

    // Extract a single argument from the list at the specified index
    template<typename T>
    static bool ExtractArg (T& str, ECN::EvaluationResultVector const& args, size_t index, bool allowNull)
        {
        return index < args.size() ? ExtractArg (str, args[index], allowNull) : false;
        }
    template<typename T>
    static bool ExtractArg (T& extractedValue, ECN::EvaluationResultVector const& args, size_t index)
        {
        return index < args.size() ? ExtractArg (extractedValue, args[index]) : false;
        }

    // Extract a DPoint3d from 3 numeric arguments beginning at startIndex
    static bool ExtractArg (DPoint3d& p, ECN::EvaluationResultVector const& args, size_t startIndex)
        {
        return ExtractArg (p.x, args, startIndex) && ExtractArg (p.y, args, startIndex+1) && ExtractArg (p.z, args, startIndex+2);
        }

    };


END_BENTLEY_ECOBJECT_NAMESPACE

