/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/SymbolicExpression.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(SymbolicExpression)
UNITS_TYPEDEFS(SymbolWithExponent)
UNITS_TYPEDEFS(Token)
UNITS_TYPEDEFS(Symbol)

BEGIN_BENTLEY_UNITS_NAMESPACE

static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';

struct Unit;
struct Phenomenon;
struct Symbol;

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
private:
    bvector<SymbolWithExponentP> m_symbolExpression;

    // TODO: Need this?
    //~SymbolicExpression();

    SymbolWithExponentP * begin() { return m_symbolExpression.begin(); }
    SymbolWithExponentP * end() { return m_symbolExpression.end(); }
    void erase(SymbolWithExponentP* deleteIterator, SymbolWithExponentP* end) { m_symbolExpression.erase(deleteIterator, end); }
    size_t size() const { return m_symbolExpression.size(); }

    void AddCopy(SymbolWithExponentCR sWE);
    void Add(SymbolCP symbol, int exponent);
    void Add(SymbolWithExponentR sWE) { m_symbolExpression.push_back(&sWE); }
    static void Copy(SymbolicExpressionR source, SymbolicExpressionR target);

    void LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const;

    static BentleyStatus ParseDefinition(Utf8CP definition, SymbolicExpressionR expression, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static BentleyStatus HandleToken(SymbolicExpressionR expression, Utf8CP definition, TokenCR token, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName);
    static void MergeExpressions(Utf8CP targetDefinition, SymbolicExpressionR targetExpression, Utf8CP sourceDefinition, SymbolicExpressionR sourceExpression, int startingExponent);
    static bool DimensionallyCompatible(SymbolicExpressionR expressionA, SymbolicExpressionR expressionB);
    };

struct Symbol
    {
friend struct SymbolicExpression;
friend struct Unit;
friend struct Phenomenon;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    int         m_id;
    Utf8Char    m_dimensionSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_dimensionless;
    mutable bool m_evaluated;
    mutable SymbolicExpression m_symbolExpression;


    SymbolicExpression& Evaluate(std::function<SymbolCP(Utf8CP)> getSymbolByName) const;

protected:
    Symbol(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id, double factor, double offset) : 
        m_name(name), m_definition(definition), m_dimensionSymbol(dimensionSymbol), m_id(id), m_factor(factor), m_offset(offset), m_evaluated(false)
        {
        m_dimensionless = strcmp("ONE", m_definition.c_str()) == 0;
        }

public:
    Utf8CP GetName() const { return m_name.c_str(); }
    int     GetId() const { return m_id; }
    Utf8CP GetDefinition() const { return m_definition.c_str(); }
    double GetFactor() const { return m_factor; }
    bool IsBaseSymbol() const { return ' ' != m_dimensionSymbol; }
    bool IsDimensionless() const { return m_dimensionless; }

    // Binary comparison operators.
    bool operator== (SymbolCR rhs) const { return m_id == rhs.m_id; }
    bool operator!= (SymbolCR rhs) const { return m_id != rhs.m_id; }
    };

struct SymbolWithExponent
    {
private:
    SymbolCP m_symbol;
    int m_exponent;

public:
    SymbolWithExponent(SymbolCP symbol, int exponent) : m_exponent(exponent) { m_symbol = symbol; }
    SymbolWithExponent(SymbolWithExponent const& swE) : SymbolWithExponent(swE.m_symbol, swE.m_exponent) {};

    Utf8CP GetName() const { return m_symbol->GetName(); }
    SymbolCP GetSymbol() const { return m_symbol; }

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