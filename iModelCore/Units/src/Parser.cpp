
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
void Unit::MergeExpressions(bvector<UnitFactorExponent*>& targetExpression, bvector<UnitFactorExponent*>& sourceExpression, int startingExponent)
    {
    for (const auto& uWE : sourceExpression)
        {
        int     mergedExponent = uWE->m_exponent * startingExponent;

        auto it = find_if(targetExpression.begin(), targetExpression.end(), [&] (UnitFactorExponent* a) { return strcmp(uWE->m_unit->GetName(), a->m_unit->GetName()) == 0; });
        if (it != targetExpression.end())
            {
            LOG.debugv("Merging existing UFE for %s.  With Exponent: %d", (*it)->m_unit->GetName(), mergedExponent);
            (*it)->m_exponent += mergedExponent;
            }
        else
            {
            LOG.debugv("Adding UFE for %s   With Exponent: %d", uWE->m_unit->GetName(), mergedExponent);
            UnitFactorExponent* copy = new UnitFactorExponent(uWE->m_unit, mergedExponent);
            targetExpression.push_back(copy);
            }
        }
    }

BentleyStatus Unit::AddUFEToExpression(bvector<UnitFactorExponent*>& unitExpression, Utf8CP definition, Utf8CP token, int mergedExponent)
    {
    auto it = find_if(unitExpression.begin(), unitExpression.end(), [&] (UnitFactorExponent* a) { return strcmp(token, a->m_unit->GetName()) == 0; });
    if (it != unitExpression.end())
        {
        (*it)->m_exponent += mergedExponent;
        }
    else
        {
        UnitCP unit = UnitRegistry::Instance().LookupUnit(token);
        if (nullptr == unit)
            {
            LOG.errorv("Failed to parse %s because the unit %s could not be found", definition, token);
            return BentleyStatus::ERROR;
            }
        if (!unit->IsBaseUnit())
            {
            UnitFactorExponent* uWE = new UnitFactorExponent(unit, mergedExponent);
            unitExpression.push_back(uWE);
            if (!unit->IsConstant())
                {
                LOG.debugv("Mergin sub expression for %s", unit->GetName());
                bvector<UnitFactorExponent*> subExpression = unit->Evaluate();
                MergeExpressions(unitExpression, subExpression, mergedExponent);
                }
            }
        }
    return BentleyStatus::SUCCESS;
    }

BentleyStatus Unit::HandleToken(bvector<UnitFactorExponent*>& unitExpression, Utf8CP definition, Utf8CP constToken, Utf8CP token, int tokenExponent, int startingExponent)
    {
    LOG.debugv("Handle Token: %s  ConstToken: %s  TokenExp: %d  StartExp: %d", token, Utf8String::IsNullOrEmpty(constToken)? "None": constToken, tokenExponent, startingExponent);
    int mergedExponent = tokenExponent * startingExponent;
    if (!Utf8String::IsNullOrEmpty(constToken))
        {
        AddUFEToExpression(unitExpression, definition, constToken, mergedExponent);
        LOG.debugv("%s - Merged Exponent: %d", definition, mergedExponent);
        }

    return AddUFEToExpression(unitExpression, definition, token, mergedExponent);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Unit::ParseDefinition(Utf8CP definition, bvector<UnitFactorExponent*>& unitExpression, int startingExponent)
    {
    if (Utf8String::IsNullOrEmpty(definition))
        return BentleyStatus::ERROR;
 
    Utf8String definitionString = definition;
    UnitToken currentToken;
    UnitToken currentConst;
    Exponent currentExponent;
    bool inExponent = false;
    bool inConstant = false;
    for (auto const& character : definitionString)
        {
        if (Utf8String::IsAsciiWhiteSpace(character))
            continue;
 
        if (Multiply == character)
            {
            if (inConstant && !currentConst.IsValid())
                {
                LOG.errorv("Failed to parse %s because found invalid constant", definition);
                return BentleyStatus::ERROR;
                }
            if (!inConstant && !currentToken.IsValid())
                {
                LOG.errorv("Failed to parse %s because found invalid token", definition);
                return BentleyStatus::ERROR;
                }

            if (!inConstant)
                {
                HandleToken(unitExpression, definition, currentConst.GetToken(), currentToken.GetToken(), currentToken.GetExponent(), startingExponent);
                currentToken.Clear();
                currentConst.Clear();
                }
            inConstant = false;
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
            inConstant = true;
            continue;
            }
        if (CloseBracket == character)
            {
            continue;
            }
 
        if (inExponent)
            currentExponent.AddChar(character);
        else if (inConstant)
            currentConst.AddChar(character);
        else
            currentToken.AddChar(character);
        }
 
    if (currentToken.IsValid())
        {
        HandleToken(unitExpression, definition, currentConst.GetToken(), currentToken.GetToken(), currentToken.GetExponent(), startingExponent);
        }

    remove_if(unitExpression.begin(), unitExpression.end(), [&] (UnitFactorExponent* a) { return a->m_exponent == 0; });
    sort(unitExpression.begin(), unitExpression.end(), [&] (UnitFactorExponent* a, UnitFactorExponent* b) { return strcmp(a->m_unit->GetPhenomenon(), b->m_unit->GetPhenomenon()); });

    return unitExpression.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

