
/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Parser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "Parser.h"

USING_NAMESPACE_BENTLEY_UNITS

void UnitToken::AddToNumeratorOrDenominator(Utf8Vector& numerator, Utf8Vector& denominator)
    {
    if (IsNumerator())
        AddToVector(numerator);
    else
        AddToVector(denominator);
    }

void UnitToken::AddToVector(Utf8Vector& ator)
    {
    for (int i = 0; i < abs(m_exponent); ++i)
        ator.push_back(Utf8String(m_tolken.c_str()));

    m_tolken.clear();
    m_exponent = 1;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
int Exponent::GetExponent()
    {
    int value = 0;
    BE_STRING_UTILITIES_UTF8_SSCANF(m_exponentChars.c_str(), "%d", &value);
    m_exponentChars.clear();
    return value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Unit::ParseDefinition(Utf8CP definition, Utf8Vector& numerator, Utf8Vector& denominator)
    {
    if (Utf8String::IsNullOrEmpty(definition))
        return BentleyStatus::ERROR;
 
    Utf8String definitionString = definition;
    UnitToken currentTolken;
    Exponent currentExponent;
    bool inExponent = false;
    for (auto const& character : definitionString)
        {
        if (Utf8String::IsAsciiWhiteSpace(character))
            continue;
 
        if (Multiply == character)
            {
            if (!currentTolken.IsValid())
                return BentleyStatus::ERROR;
 
            currentTolken.AddToNumeratorOrDenominator(numerator, denominator);
 
            continue;
            }
 
        if (OpenParen == character)
            {
            inExponent = true;
            continue;
            }
 
        if (CloseParen == character)
            {
            inExponent = false;
            if (!currentExponent.IsValid())
                return BentleyStatus::ERROR;
            currentTolken.SetExponent(currentExponent.GetExponent());
            continue;
            }
 
 
        if (inExponent)
            currentExponent.AddChar(character);
        else
            currentTolken.AddChar(character);
        }
 
    if (currentTolken.IsValid())
        currentTolken.AddToNumeratorOrDenominator(numerator, denominator);
 
    sort(numerator.begin(), numerator.end());
    sort(denominator.begin(), denominator.end());

    return numerator.size() > 0 || denominator.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }