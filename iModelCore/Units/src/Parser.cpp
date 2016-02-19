
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
        ator.push_back(Utf8String(m_token.c_str()));

    m_token.clear();
    m_exponent = 1;
    }

void UnitToken::Clear()
    {
    m_token.clear();
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

// TODO: These static methods probably belong in some other class besides unit.  Was lazy and this was quickest place to plop
void Unit::MergeExpressions(Utf8CP definition, bvector<UnitExponent*>& targetExpression, bvector<UnitExponent*>& sourceExpression, int startingExponent)
    {
    for (const auto& uWE : sourceExpression)
        {
        int     mergedExponent = uWE->m_exponent * startingExponent;

        auto it = find_if(targetExpression.begin(), targetExpression.end(), [&] (UnitExponent* a) { return strcmp(uWE->m_unit->GetName(), a->m_unit->GetName()) == 0; });
        if (it != targetExpression.end())
            {
            LOG.debugv("%s - Merging existing UFE for %s.  With Exponent: %d", definition, (*it)->m_unit->GetName(), mergedExponent);
            (*it)->m_exponent += mergedExponent;
            }
        else
            {
            LOG.debugv("%s - Adding UFE for %s   With Exponent: %d", definition, uWE->m_unit->GetName(), mergedExponent);
            UnitExponent* copy = new UnitExponent(uWE->m_unit, mergedExponent);
            targetExpression.push_back(copy);
            }
        }
    }

BentleyStatus Unit::AddUFEToExpression(bvector<UnitExponent*>& unitExpression, Utf8CP definition, Utf8CP token, int mergedExponent)
    {
    UnitCP unit = nullptr;
    auto it = find_if(unitExpression.begin(), unitExpression.end(), [&] (UnitExponent* a) { return strcmp(token, a->m_unit->GetName()) == 0; });
    if (it != unitExpression.end())
        {
        (*it)->m_exponent += mergedExponent;
        unit = (*it)->m_unit;
        }
    else
        {
        unit = UnitRegistry::Instance().LookupUnit(token);
        UnitExponent* uWE = new UnitExponent(unit, mergedExponent);
        unitExpression.push_back(uWE);
        }
    
    if (nullptr == unit)
        {
        LOG.errorv("Failed to parse %s because the unit %s could not be found", definition, token);
        return BentleyStatus::ERROR;
        }
    
    // Could probably skip this block if is base unit ... would result in smaller expression but would not fully expand dimension
    if (!(unit->IsConstant() || unit->IsBaseUnit()))
        {
        LOG.debugv("%s - Merging sub expression for %s", definition, unit->GetName());
        bvector<UnitExponent*> subExpression = unit->Evaluate();
        MergeExpressions(definition, unitExpression, subExpression, mergedExponent);
        }
    return BentleyStatus::SUCCESS;
    }

BentleyStatus Unit::HandleToken(bvector<UnitExponent*>& unitExpression, Utf8CP definition, Utf8CP token, int tokenExponent, int startingExponent)
    {
    LOG.debugv("%s - Handle Token: %s  TokenExp: %d  StartExp: %d", definition, token, tokenExponent, startingExponent);
    int mergedExponent = tokenExponent * startingExponent;

    return AddUFEToExpression(unitExpression, definition, token, mergedExponent);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Unit::ParseDefinition(Utf8CP definition, bvector<UnitExponent*>& unitExpression, int startingExponent)
    {
    if (Utf8String::IsNullOrEmpty(definition))
        return BentleyStatus::ERROR;
 
    Utf8String definitionString = definition;
    UnitToken currentToken;
    Exponent currentExponent;
    bool inExponent = false;
    for (auto const& character : definitionString)
        {
        if (Utf8String::IsAsciiWhiteSpace(character))
            continue;
 
        if (Multiply == character)
            {
            if (!currentToken.IsValid())
                {
                LOG.errorv("Failed to parse %s because found invalid token", definition);
                return BentleyStatus::ERROR;
                }

            HandleToken(unitExpression, definition, currentToken.GetToken(), currentToken.GetExponent(), startingExponent);
            currentToken.Clear();
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
            currentToken.SetExponent(currentExponent.GetExponent());
            continue;
            }
 
        if (OpenBracket == character)
            {
            continue;
            }
        if (CloseBracket == character)
            {
            continue;
            }
 
        if (inExponent)
            currentExponent.AddChar(character);
        else
            currentToken.AddChar(character);
        }
 
    if (currentToken.IsValid())
        {
        HandleToken(unitExpression, definition, currentToken.GetToken(), currentToken.GetExponent(), startingExponent);
        }

    remove_if(unitExpression.begin(), unitExpression.end(), [&] (UnitExponent* a) { return a->m_exponent == 0; });

    return unitExpression.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

