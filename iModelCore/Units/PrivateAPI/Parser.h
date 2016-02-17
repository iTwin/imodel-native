/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/Parser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
// Not used yet
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';
 
 
//=======================================================================================
// @bsiclass                                                    Colin.Kerr  02/16
//=======================================================================================
struct UnitToken
    {
private:
    Utf8String  m_tolken;
    int         m_exponent = 1;

public:
    bool IsValid() { return !Utf8String::IsNullOrEmpty(m_tolken.c_str()) && abs(m_exponent) >= 1; }
    void AddChar(Utf8Char character) { m_tolken.append(1, character); }
    void SetExponent(int exponent) { m_exponent = exponent; }
    bool IsNumerator() { return m_exponent > 0; }
    
    void AddToNumeratorOrDenominator(Utf8Vector& numerator, Utf8Vector& denominator);
    void AddToVector(Utf8Vector& ator);
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