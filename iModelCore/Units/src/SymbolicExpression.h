/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(SymbolicExpression)
UNITS_TYPEDEFS(SymbolWithExponent)
UNITS_TYPEDEFS(Token)

BEGIN_BENTLEY_UNITS_NAMESPACE

static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';


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

struct SymbolicExpression
    {
friend struct Symbol;
friend struct Unit;
friend struct Phenomenon;
private:
    bvector<SymbolWithExponentP> m_symbolExpression;

    SymbolWithExponentP * begin() { return m_symbolExpression.begin(); }
    SymbolWithExponentP * end() { return m_symbolExpression.end(); }
    void erase(SymbolWithExponentP* deleteIterator, SymbolWithExponentP* end) { m_symbolExpression.erase(deleteIterator, end); }
    size_t size() const { return m_symbolExpression.size(); }

    void AddCopy(SymbolWithExponentCR sWE);
    void Add(SymbolCP symbol, int exponent);
    void Add(SymbolWithExponentR sWE) { m_symbolExpression.push_back(&sWE); }
    static void Copy(SymbolicExpressionR source, SymbolicExpressionR target);

    void LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const;
    Utf8String ToString() const;

    static BentleyStatus ParseDefinition(int& depth, Utf8CP definition, SymbolicExpressionR expression, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static BentleyStatus HandleToken(int& depth, SymbolicExpressionR expression, Utf8CP definition, TokenCR token, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static void MergeExpressions(Utf8CP targetDefinition, SymbolicExpressionR targetExpression, Utf8CP sourceDefinition, SymbolicExpressionR sourceExpression, int startingExponent);
    static bool DimensionallyCompatible(SymbolicExpressionR expressionA, SymbolicExpressionR expressionB);
    static void CreateExpressionWithOnlyBaseSymbols(SymbolicExpressionR source, SymbolicExpressionR target, bool copySymbols);
    };

struct SymbolWithExponent
    {
private:
    SymbolCP m_symbol;
    int m_exponent;

public:
    SymbolWithExponent(SymbolCP symbol, int exponent) : m_exponent(exponent) { m_symbol = symbol; }
    SymbolWithExponent(SymbolWithExponent const& swE) : SymbolWithExponent(swE.m_symbol, swE.m_exponent) {};

    Utf8CP GetName() const { return m_symbol->_GetName(); }
    SymbolCP GetSymbol() const { return m_symbol; }
    double GetFactor() const { return m_symbol->_GetFactor(); }
    
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