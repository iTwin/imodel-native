/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(Expression)
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
    typedef bvector<ExpressionSymbolP> SymbolList;
    typedef SymbolList::iterator iterator;
    typedef SymbolList::const_iterator const_iterator;
    
    friend struct Symbol;
    friend struct Unit;
    friend struct Phenomenon;

private:
    SymbolList m_symbolExpression;

    iterator begin() { return m_symbolExpression.begin(); }
    iterator end() { return m_symbolExpression.end(); }
    const_iterator begin() const { return m_symbolExpression.begin(); }
    const_iterator end() const { return m_symbolExpression.end(); }

    ExpressionSymbolCP FirstSymbol() const { return m_symbolExpression.front(); }

    void erase(iterator deleteIterator, iterator end) { m_symbolExpression.erase(deleteIterator, end); }
    size_t size() const { return m_symbolExpression.size(); }

    void AddCopy(ExpressionSymbolCR sWE);
    void Add(SymbolCP symbol, int exponent);
    void Add(ExpressionSymbolR sWE) { m_symbolExpression.push_back(&sWE); }
    static void Copy(ExpressionR source, ExpressionR target);

    void LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const;
    Utf8String ToString(bool includeFactors = true) const;
    bool Contains(ExpressionSymbolCR symbol) const;

    static BentleyStatus ParseDefinition(SymbolCR owner, int& depth, Utf8CP definition, ExpressionR expression, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static BentleyStatus HandleToken(int& depth, ExpressionR expression, Utf8CP definition, TokenCR token, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static void MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent);
    static void MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression,
                                 Utf8CP sourceDefinition, ExpressionR sourceExpression,
                                 int startingExponent, std::function<bool(SymbolCR, SymbolCR)> areEqual);
    static bool ShareDimensions(PhenomenonCR phenomenon, UnitCR unit);
    static bool DimensionallyCompatible(ExpressionCR expressionA, ExpressionCR expressionB);
    static bool DimensionallyCompatible(ExpressionCR expressionA, ExpressionCR expressionB, std::function<bool(SymbolCR, SymbolCR)> areEqual);
    static void CreateExpressionWithOnlyBaseSymbols(ExpressionCR source, ExpressionR target, bool copySymbols);
    };

//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct ExpressionSymbol
    {
private:
    SymbolCP m_symbol;
    int m_exponent;

public:
    ExpressionSymbol(SymbolCP symbol, int exponent) : m_exponent(exponent) { m_symbol = symbol; }
    ExpressionSymbol(ExpressionSymbol const& swE) : ExpressionSymbol(swE.m_symbol, swE.m_exponent) {};

    Utf8CP GetName() const { return m_symbol->GetName(); }
    SymbolCP GetSymbol() const { return m_symbol; }
    double GetSymbolFactor() const { return m_symbol->GetFactor(); }
    
    int GetExponent() { return m_exponent; }
    void AddToExponent(int toAdd) { m_exponent += toAdd; }
    void SetExponent(int exponent) { m_exponent = exponent; }
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