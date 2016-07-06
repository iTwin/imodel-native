/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>
#include <stdlib.h>

UNITS_TYPEDEFS(ExpressionSymbol)
UNITS_TYPEDEFS(Token)

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct Token
    {
private:
    Utf8String  m_token;
    int         m_exponent = 1;

public:
    bool    IsValid() const { return !Utf8String::IsNullOrEmpty(m_token.c_str()) && abs(m_exponent) >= 1; }
    void    AddChar(Utf8Char character) { m_token.append(1, character); }
    void    SetExponent(int exponent) { m_exponent = exponent; }
    Utf8CP  GetName() const { return m_token.c_str(); }
    int     GetExponent() const { return m_exponent; }
    
    void Clear();
    };

//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct Expression
    {
    typedef bvector<ExpressionSymbol> SymbolList;
    typedef SymbolList::iterator iterator;
    typedef SymbolList::const_iterator const_iterator;
    typedef SymbolList::reverse_iterator reverse_iterator;
    typedef SymbolList::const_reverse_iterator const_reverse_iterator;
    
    friend struct UnitsSymbol;
    friend struct Unit;
    friend struct Phenomenon;

private:
    SymbolList m_symbolExpression;

    iterator begin() { return m_symbolExpression.begin(); }
    iterator end() { return m_symbolExpression.end(); }
    const_iterator begin() const { return m_symbolExpression.begin(); }
    const_iterator end() const { return m_symbolExpression.end(); }

    reverse_iterator rbegin() { return m_symbolExpression.rbegin(); }
    reverse_iterator rend() { return m_symbolExpression.rend(); }
    const_reverse_iterator rbegin() const { return m_symbolExpression.rbegin(); }
    const_reverse_iterator rend() const { return m_symbolExpression.rend(); }

    ExpressionSymbolCR FirstSymbol() const { return m_symbolExpression.front(); }

    void erase(iterator deleteIterator, iterator end) { m_symbolExpression.erase(deleteIterator, end); }
    size_t size() const { return m_symbolExpression.size(); }

    void Add(UnitsSymbolCP symbol, int exponent);
    void Add(ExpressionSymbolCR sWE) { m_symbolExpression.push_back(sWE); }
    static void Copy(ExpressionR source, ExpressionR target);

    void LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const;
    Utf8String ToString(bool includeFactors = true) const;
    bool Contains(ExpressionSymbolCR symbol) const;

    static BentleyStatus ParseDefinition(UnitsSymbolCR owner, int& depth, Utf8CP definition, ExpressionR expression, int startingExponent, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName);
    static BentleyStatus HandleToken(UnitsSymbolCR owner, int& depth, ExpressionR expression, Utf8CP definition, TokenCR token, int startingExponent, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName);
    static void MergeSymbol(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, UnitsSymbolCP symbol, int symbolExponent, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual);
    static void MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent);
    static void MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression,
                                 Utf8CP sourceDefinition, ExpressionR sourceExpression,
                                 int startingExponent, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual);
    static bool ShareSignatures(PhenomenonCR phenomenon, UnitCR unit);
    static bool ShareSignatures(PhenomenonCR phenomenon, ExpressionCR expression);
    static bool SignaturesCompatible(ExpressionCR expressionA, ExpressionCR expressionB);
    static bool SignaturesCompatible(ExpressionCR expressionA, ExpressionCR expressionB, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual);
    static BentleyStatus GenerateConversionExpression(UnitCR from, UnitCR to, ExpressionR conversionExpression);
    static void CreateExpressionWithOnlyBaseSymbols(ExpressionCR source, ExpressionR target);

    static bool IdsMatch(UnitsSymbolCR a, UnitsSymbolCR b);
    static bool PhenomenonIdsMatch(UnitsSymbolCR a, UnitsSymbolCR b);
    };

//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct ExpressionSymbol
    {
private:
    UnitsSymbolCP m_symbol;
    int m_exponent;

public:
    ExpressionSymbol(UnitsSymbolCP symbol, int exponent) : m_exponent(exponent) { m_symbol = symbol; }
    ExpressionSymbol(ExpressionSymbol const& swE) : ExpressionSymbol(swE.m_symbol, swE.m_exponent) {};

    Utf8CP GetName() const { return m_symbol->GetName(); }
    UnitsSymbolCP GetSymbol() const { return m_symbol; }
    double GetSymbolFactor() const { return m_symbol->GetFactor(); }
    
    int GetExponent() const { return m_exponent; }
    void AddToExponent(int toAdd) { m_exponent += toAdd; }
    void SetExponent(int exponent) { m_exponent = exponent; }
    Utf8String ToString(bool includeFactors = true) const;
    };

//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct Exponent
    {
private:
    Utf8String m_exponentChars;

public:
    bool IsValid() { return !Utf8String::IsNullOrEmpty(m_exponentChars.c_str()); }
    void AddChar(Utf8Char character) { m_exponentChars.append(1, character); }
    int GetExponent();
    };

END_BENTLEY_UNITS_NAMESPACE