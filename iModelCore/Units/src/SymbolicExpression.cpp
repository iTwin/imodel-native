
/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "SymbolicExpression.h"

USING_NAMESPACE_BENTLEY_UNITS

static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';

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

void Expression::LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const
    {
    if (!LOG.isSeverityEnabled(loggingLevel))
        return;

    LOG.messagev(loggingLevel, "Expression for: %s", name);
    LOG.message(loggingLevel, ToString().c_str());
    }

Utf8String Expression::ToString(bool includeFactors) const
    {
    Utf8String output;
    for (auto const& sWE : m_symbolExpression)
        {
        if (!includeFactors || sWE->GetSymbol()->GetFactor() == 0.0)
            {
            Utf8PrintfString sWEString("%s^%d * ", sWE->GetName(), sWE->GetExponent());
            output.append(sWEString.c_str());
            }
        else
            {
            Utf8PrintfString sWEString("%lf%s^%d * ", sWE->GetSymbol()->GetFactor(), sWE->GetName(), sWE->GetExponent());
            output.append(sWEString.c_str());
            }
        }
    return output;
    }

void Expression::CreateExpressionWithOnlyBaseSymbols(ExpressionCR source, ExpressionR target, bool copySymbols)
    {
    for (auto symbolExp : source)
        {
        if (symbolExp->GetSymbol()->IsBaseSymbol() && !symbolExp->GetSymbol()->IsDimensionless())
            {
            if (copySymbols)
                target.AddCopy(*symbolExp);
            else
                target.Add(*symbolExp);
            }
        }
    }

bool Expression::ShareDimensions(PhenomenonCR phenomenon, UnitCR unit)
    {
    return DimensionallyCompatible(phenomenon.Evaluate(), unit.Evaluate(), [] (SymbolCR a, SymbolCR b) { return a.GetPhenomenonId() == b.GetPhenomenonId(); });
    }

bool Expression::DimensionallyCompatible(ExpressionCR fromExpression, ExpressionCR toExpression)
    {
    return DimensionallyCompatible(fromExpression, toExpression, [] (SymbolCR a, SymbolCR b) { return a.GetId() == b.GetId(); });
    }

// TODO: Consider how this could be combined with the merge step so we don't have to make two copies of the from expression
bool Expression::DimensionallyCompatible(ExpressionCR fromExpression, ExpressionCR toExpression, std::function<bool(SymbolCR, SymbolCR)> areEqual)
    {
    Expression fromBaseSymbols;
    CreateExpressionWithOnlyBaseSymbols(fromExpression, fromBaseSymbols, true);
    Expression toBaseSymbols;
    CreateExpressionWithOnlyBaseSymbols(toExpression, toBaseSymbols, false);

    MergeExpressions("", fromBaseSymbols, "", toBaseSymbols, -1, areEqual);
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

void Expression::MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent)
    {
    MergeExpressions(targetDefinition, targetExpression, sourceDefinition, sourceExpression, startingExponent, [] (SymbolCR a, SymbolCR b) { return a.GetId() == b.GetId(); });
    }

void Expression::MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent, std::function<bool(SymbolCR, SymbolCR)> areEqual)
    {
    LOG.debugv("Merging Expressions %s --> %s", sourceDefinition, targetDefinition);
    for (const auto& uWE : sourceExpression)
        {
        int     mergedExponent = uWE->GetExponent() * startingExponent;

        auto it = find_if(targetExpression.begin(), targetExpression.end(), [&uWE, &areEqual] (ExpressionSymbolCP a) { return areEqual(*a->GetSymbol(), *uWE->GetSymbol()); });
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

int maxRecursionDepth = 42;

BentleyStatus Expression::HandleToken(int& depth, ExpressionR expression, 
    Utf8CP definition, TokenCR token, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName)
    {
    LOG.debugv("%s - Handle Token: %s  TokenExp: %d  StartExp: %d", definition, token.GetName(), token.GetExponent(), startingExponent);
    int mergedExponent = token.GetExponent() * startingExponent;

    SymbolCP symbol = nullptr;
    auto it = find_if(expression.begin(), expression.end(), [&token] (ExpressionSymbolCP a) { return strcmp(token.GetName(), a->GetName()) == 0; });
    if (it != expression.end())
        {
        LOG.debugv("%s - Merging existing Unit %s with Exponent: %d", definition, (*it)->GetName(), mergedExponent);
        (*it)->AddToExponent(mergedExponent);
        symbol = (*it)->GetSymbol();
        }
    else
        {
        LOG.debugv("%s - Adding Unit %s with Exponent: %d", definition, token.GetName(), mergedExponent);
        symbol = getSymbolByName(token.GetName());
        if (nullptr != symbol)
            {
            // Could probably skip this block if is base unit ... would result in smaller expression 
            // but would not fully expand dimension
            expression.Add(symbol, mergedExponent);
            }
        }

    if (nullptr == symbol)
        {
        LOG.errorv("Failed to parse %s because the unit %s could not be found", definition, token.GetName());
        return BentleyStatus::ERROR;
        }

    if (!symbol->IsBaseSymbol())
        {
        if (depth > maxRecursionDepth)
            {
            LOG.errorv("Stopping parse, hit Max RecursionDepth.  Equation at this point is: %s", expression.ToString().c_str());
            return BentleyStatus::ERROR;
            }

        LOG.debugv("Evaluating %s", symbol->GetName());
        Expression sourceExpression = symbol->Evaluate(depth, getSymbolByName);
        MergeExpressions(definition, expression, symbol->GetDefinition(), sourceExpression, mergedExponent);
        }
    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Expression::ParseDefinition(SymbolCR owner, int& depth, Utf8CP definition,
    ExpressionR expression, int startingExponent, std::function<SymbolCP(Utf8CP)> getSymbolByName)
    {
    ++depth;
    if (Utf8String::IsNullOrEmpty(definition))
        return BentleyStatus::ERROR;
 
    Utf8String definitionString = definition;
    Token currentToken;
    Exponent currentExponent;
    bool inExponent = false;
    int numTokens = 0;
    for (auto const& character : definitionString)
        {
        if (Utf8String::IsAsciiWhiteSpace(character))
            continue;

        // TODO: Refactor this into a tight switch statement.
        if (Multiply == character)
            {
            if (!currentToken.IsValid())
                {
                LOG.errorv("Failed to parse %s because found invalid token", definition);
                return BentleyStatus::ERROR;
                }

            HandleToken(depth, expression, definition, currentToken, startingExponent, getSymbolByName);
            ++numTokens;
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
            continue;

        if (CloseBracket == character)
            continue;
 
        if (inExponent)
            currentExponent.AddChar(character);
        else
            currentToken.AddChar(character);
        }
 
    if (currentToken.IsValid())
        {
        HandleToken(depth, expression, definition, currentToken, startingExponent, getSymbolByName);
        ++numTokens;
        }

    // TODO: Need some validation that folks aren't defining units with offsets in ways we don't support.
    //if (owner.HasOffset() && numTokens > 1)
    //    {
    //    LOG.errorv("%s has an invalid symbol expression (%s) because it has more than one symbol and an offset", owner.GetName(), definition);
    //    return BentleyStatus::ERROR;
    //    }

    auto new_iter = remove_if(expression.begin(), expression.end(), [](ExpressionSymbol* a) { return a->GetExponent() == 0; });
    expression.erase(new_iter, expression.end());

    // TODO: Decide if we want to sort or keep in generated order.
    //sort(expression.begin(), expression.end(), [&] (UnitExponent* a, UnitExponent* b) { return strcmp(a->m_unit->GetPhenomenon(), b->m_unit->GetPhenomenon()); });

    LOG.debugv("%s - DONE", definition);

    return expression.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

bool Expression::Contains(ExpressionSymbolCR symbol) const
    {
    auto it = find_if(begin(), end(), [&symbol] (ExpressionSymbol* a) { return a->GetSymbol()->GetId() == symbol.GetSymbol()->GetId(); });
    if (it == end() || 0 == (*it)->GetExponent())
        return false;

    return true;
    }

void Expression::AddCopy(ExpressionSymbolCR sWE) 
    { 
    m_symbolExpression.push_back(new ExpressionSymbol(sWE)); 
    }

void Expression::Add(SymbolCP symbol, int exponent) 
    {
    m_symbolExpression.push_back(new ExpressionSymbol(symbol, exponent)); 
    }

void Expression::Copy(ExpressionR source, ExpressionR target)
    {
    for (auto const& sourceSymbol : source)
        target.AddCopy(*sourceSymbol);
    }
