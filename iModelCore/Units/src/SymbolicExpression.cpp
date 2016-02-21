
/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

USING_NAMESPACE_BENTLEY_UNITS

void Token::Clear()
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

void SymbolicExpression::LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const
    {
    if (!LOG.isSeverityEnabled(loggingLevel))
        return;

    LOG.messagev(loggingLevel, "Expression for: %s", name);
    Utf8String output;
    for (auto const& sWE : m_symbolExpression)
        {
        if (sWE->GetSymbol()->GetFactor() == 0.0)
            {
            Utf8PrintfString sWEString ("%s^$d",sWE->GetName(), sWE->GetExponent());
            output.append(sWEString.c_str());
            }
        else
            {
            Utf8PrintfString sWEString("%lf%s^%d * ", sWE->GetSymbol()->GetFactor(), sWE->GetName(), sWE->GetExponent());
            output.append(sWEString.c_str());
            }
        }
    LOG.message(loggingLevel, output.c_str());
    }

// TODO: Consider how this could be combined with the merge step so we don't have to make two copies of the from expression
bool SymbolicExpression::DimensionallyCompatible(SymbolicExpressionR fromExpression, SymbolicExpressionR toExpression)
    {
    SymbolicExpression fromBaseSymbols;
    for (auto const& symbolExp : fromExpression)
        {
        if (symbolExp->GetSymbol()->IsBaseSymbol() && !symbolExp->GetSymbol()->IsDimensionless())
            {
            fromBaseSymbols.AddCopy(*symbolExp);
            }
        }
    SymbolicExpression toBaseSymbols;
    for (auto const& symbolExp : toExpression)
        {
        if (symbolExp->GetSymbol()->IsBaseSymbol() && !symbolExp->GetSymbol()->IsDimensionless())
            toBaseSymbols.Add(*symbolExp);
        }
    MergeExpressions("", fromBaseSymbols, "", toBaseSymbols, -1);
    for (auto const& unitExp : fromBaseSymbols)
        {
        if (unitExp->GetExponent() == 0)
            continue;
        else if (unitExp->GetExponent() > 0)
            LOG.errorv("Cannot convert - from Expression has component of '%s' not in the to unit", unitExp->GetName());
        else
            LOG.errorv("Cannot convert - to Expression has component of '%s' not in the from Unit", unitExp->GetName());

        fromExpression.LogExpression(NativeLogging::LOG_ERROR, "From");
        toExpression.LogExpression(NativeLogging::LOG_ERROR, "To");
        return false;
        }
    return true;
    }

void SymbolicExpression::MergeExpressions(Utf8CP targetDefinition, SymbolicExpressionR targetExpression, Utf8CP sourceDefinition, SymbolicExpressionR sourceExpression, int startingExponent)
    {
    LOG.debugv("Merging Expressions %s --> %s", sourceDefinition, targetDefinition);
    for (const auto& uWE : sourceExpression)
        {
        int     mergedExponent = uWE->GetExponent() * startingExponent;

        auto it = find_if(targetExpression.begin(), targetExpression.end(), [&uWE] (SymbolWithExponentCP a) { return uWE->GetSymbol()->GetId() == a->GetSymbol()->GetId(); });
        if (it != targetExpression.end())
            {
            LOG.debugv("%s --> %s - Merging existing Unit %s. with Exponent: %d", sourceDefinition, targetDefinition, (*it)->GetName(), mergedExponent);
            (*it)->AddToExponent(mergedExponent);
            }
        else
            {
            LOG.debugv("%s --> %s - Adding Unit for %s with Exponent: %d", sourceDefinition, targetDefinition, uWE->GetName(), mergedExponent);
            targetExpression.Add(uWE->GetSymbol(), mergedExponent);
            }
        }
    }

BentleyStatus SymbolicExpression::HandleToken(SymbolicExpressionR expression, Utf8CP definition, TokenCR token, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName)
    {
    LOG.debugv("%s - Handle Token: %s  TokenExp: %d  StartExp: %d", definition, token.GetName(), token.GetExponent(), startingExponent);
    int mergedExponent = token.GetExponent() * startingExponent;

    SymbolCP symbol = nullptr;
    auto it = find_if(expression.begin(), expression.end(), [&token] (SymbolWithExponentCP a) { return strcmp(token.GetName(), a->GetName()) == 0; });
    if (it != expression.end())
        {
        LOG.debugv("%s - Merging existing Unit %s with Exponent: %d", definition, (*it)->GetName(), mergedExponent);
        (*it)->AddToExponent(mergedExponent);
        symbol = (*it)->GetSymbol();
        }
    else
        {
        LOG.debugv("%s - Adding Unit %s with Exponent: %d", definition, token, mergedExponent);
        symbol = getSymbolByName(token.GetName());
        if (nullptr != symbol)
            {
            // Could probably skip this block if is base unit ... would result in smaller expression but would not fully expand dimension
            expression.Add(symbol, mergedExponent);
            }
        }

    if (nullptr == symbol)
        {
        LOG.errorv("Failed to parse %s because the unit %s could not be found", definition, token);
        return BentleyStatus::ERROR;
        }

    if (!symbol->IsBaseSymbol())
        {
        LOG.debugv("Evaluating %s", symbol->GetName());
        SymbolicExpression sourceExpression = symbol->Evaluate(getSymbolByName);
        MergeExpressions(definition, expression, symbol->GetDefinition(), sourceExpression, mergedExponent);
        }
    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SymbolicExpression::ParseDefinition(Utf8CP definition, SymbolicExpressionR expression, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName)
    {
    if (Utf8String::IsNullOrEmpty(definition))
        return BentleyStatus::ERROR;
 
    Utf8String definitionString = definition;
    Token currentToken;
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

            HandleToken(expression, definition, currentToken, startingExponent, getSymbolByName);
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
        HandleToken(expression, definition, currentToken, startingExponent, getSymbolByName);
        }

    auto new_iter = remove_if(expression.begin(), expression.end(), [](SymbolWithExponent* a) { return a->GetExponent() == 0; });
    expression.erase(new_iter, expression.end());

    // TODO: Decide if we want to sort or keep in generated order.
    //sort(expression.begin(), expression.end(), [&] (UnitExponent* a, UnitExponent* b) { return strcmp(a->m_unit->GetPhenomenon(), b->m_unit->GetPhenomenon()); });

    LOG.debugv("%s - DONE", definition);

    return expression.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

void SymbolicExpression::AddCopy(SymbolWithExponentCR sWE) { m_symbolExpression.push_back(new SymbolWithExponent(sWE)); }

void SymbolicExpression::Add(SymbolCP symbol, int exponent) { m_symbolExpression.push_back(new SymbolWithExponent(symbol, exponent)); }

void SymbolicExpression::Copy(SymbolicExpressionR source, SymbolicExpressionR target)
    {
    for (auto const& sourceSymbol : source)
        target.AddCopy(*sourceSymbol);
    }

//SymbolicExpression::~SymbolicExpression()
//    {
//    for (auto const& sWE : m_symbolExpression)
//        {
//        delete sWE;
//        }
//    // TODO: Necessary to clear?
//    m_symbolExpression.clear();
//    }

