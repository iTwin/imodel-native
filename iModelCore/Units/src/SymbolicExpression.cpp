
/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SymbolicExpression.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

using namespace std;

USING_NAMESPACE_BENTLEY_UNITS

static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';
static const int maxRecursionDepth = 42;

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
bool Expression::Match(UnitsSymbolCR a, UnitsSymbolCR b)
    {
    return a.GetName().EqualsI(b.GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
bool Expression::PhenomenonMatch(UnitsSymbolCR a, UnitsSymbolCR b)
    {
    return a.GetPhenomenon()->GetName().EqualsI(b.GetPhenomenon()->GetName().c_str());
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

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
void Expression::LogExpression(NativeLogging::SEVERITY loggingLevel, Utf8CP name) const
    {
    if (!LOG.isSeverityEnabled(loggingLevel))
        return;

    LOG.messagev(loggingLevel, "Expression for: %s", name);
    LOG.message(loggingLevel, ToString().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
Utf8String Expression::ToString(bool includeFactors) const
    {
    bvector <Utf8String> result;

    std::for_each(m_symbolExpression.begin(), m_symbolExpression.end(), [&result, includeFactors] (ExpressionSymbolCR symbol) 
        {
        result.push_back(symbol.ToString(includeFactors));
        });

    return BeStringUtilities::Join(result, " * ");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
void Expression::CreateExpressionWithOnlyBaseSymbols(ExpressionCR source, ExpressionR target)
    {
    for (auto symbolExp : source)
        if (symbolExp.GetSymbol()->IsBase() && !symbolExp.GetSymbol()->IsNumber())
            target.Add(symbolExp);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
BentleyStatus Expression::GenerateConversionExpression(UnitCR from, UnitCR to, ExpressionR conversionExpression)
    {
    // TODO: Now that we are not checking if they are dimensionally compatible we will need to add some dimensionality checking 
    // somewhere in the API when adding dynamic units.
    if (!from.GetPhenomenon()->GetName().EqualsI(to.GetPhenomenon()->GetName().c_str()))
        {
        LOG.errorv("Cannot convert from %s: (%s) to %s: (%s) because they are not belong to the same Phenomenon.",
                   from.GetName().c_str(), from.GetDefinition().c_str(), to.GetName().c_str(), to.GetDefinition().c_str());
        return BentleyStatus::ERROR;
        }

    // TODO: USING A MAX RECRUSION DEPTH TO CATCH CYCLES IS SUPER HACKY AND MAKES CHRIS T. MAD.  Replace with something that actually detects cycles.
    Expression fromExpression = from.Evaluate();
    Expression toExpression = to.Evaluate();

    fromExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, from.GetName().c_str());
    toExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, to.GetName().c_str());

    if (!Expression::SignaturesCompatible(fromExpression, toExpression))
        {
        LOG.errorv("Cannot convert from %s: (%s) to %s: (%s) because they are not dimensionally compatible.",
                   from.GetName().c_str(), from.GetDefinition().c_str(), to.GetName().c_str(), to.GetDefinition().c_str());
        return BentleyStatus::ERROR;
        }

    Expression::Copy(fromExpression, conversionExpression);
    Expression::MergeExpressions(from.GetDefinition().c_str(), conversionExpression, to.GetDefinition().c_str(), toExpression, -1);
    if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_DEBUG))
        {
        Utf8PrintfString combinedName("%s/%s", from.GetName().c_str(), to.GetName().c_str());
        conversionExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, combinedName.c_str());
        }
    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
bool Expression::ShareSignatures(PhenomenonCR phenomenon, UnitCR unit)
    {
    return SignaturesCompatible(phenomenon.Evaluate(), unit.Evaluate(), PhenomenonMatch);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
bool Expression::ShareSignatures(PhenomenonCR phenomenon, ExpressionCR expression)
    {
    return SignaturesCompatible(phenomenon.Evaluate(), expression, PhenomenonMatch);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
bool Expression::SignaturesCompatible(ExpressionCR fromExpression, ExpressionCR toExpression)
    {
    return SignaturesCompatible(fromExpression, toExpression, Match);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
// TODO: Consider how this could be combined with the merge step so we don't have to make two copies of the from expression
bool Expression::SignaturesCompatible(ExpressionCR fromExpression, ExpressionCR toExpression, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual)
    {
    Expression fromBaseSymbols;
    CreateExpressionWithOnlyBaseSymbols(fromExpression, fromBaseSymbols);
    Expression toBaseSymbols;
    CreateExpressionWithOnlyBaseSymbols(toExpression, toBaseSymbols);

    MergeExpressions("", fromBaseSymbols, "", toBaseSymbols, -1, areEqual);
    for (auto const& unitExp : fromBaseSymbols)
        {
        if (unitExp.GetExponent() == 0)
            continue;
        else if (unitExp.GetExponent() > 0)
            LOG.errorv("Cannot convert - from Expression has component of '%s' not in the to unit", unitExp.GetName());
        else
            LOG.errorv("Cannot convert - to Expression has component of '%s' not in the from Unit", unitExp.GetName());

        fromExpression.LogExpression(NativeLogging::LOG_ERROR, "From");
        toExpression.LogExpression(NativeLogging::LOG_ERROR, "To");
        return false;
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
void Expression::MergeSymbol(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, UnitsSymbolCP symbol, int symbolExponent, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual)
    {
    auto it = find_if(targetExpression.begin(), targetExpression.end(), [&symbol, &areEqual] (ExpressionSymbolCR a) { return areEqual(*a.GetSymbol(), *symbol); });
    if (it == targetExpression.end())
        {
        // TODO: We should ensure we are not adding an inverted unit in an expression because this will fail to generate a conversion
        LOG.debugv("%s --> %s - Adding Unit for %s with Exponent: %d", sourceDefinition, targetDefinition, symbol->GetName().c_str(), symbolExponent);
        targetExpression.Add(symbol, symbolExponent);
        return;
        }

    LOG.debugv("%s --> %s - Merging existing Unit %s. with Exponent: %d", sourceDefinition, targetDefinition, (*it).GetName(), symbolExponent);
    (*it).AddToExponent(symbolExponent);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
void Expression::MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent)
    {
    MergeExpressions(targetDefinition, targetExpression, sourceDefinition, sourceExpression, startingExponent, Match);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
void Expression::MergeExpressions(Utf8CP targetDefinition, ExpressionR targetExpression, Utf8CP sourceDefinition, ExpressionR sourceExpression, int startingExponent, std::function<bool(UnitsSymbolCR, UnitsSymbolCR)> areEqual)
    {
    LOG.debugv("Merging Expressions %s --> %s", sourceDefinition, targetDefinition);
    for (auto it = sourceExpression.rbegin(); it != sourceExpression.rend(); ++it)
        {
        int     mergedExponent = (*it).GetExponent() * startingExponent;
        MergeSymbol(targetDefinition, targetExpression, sourceDefinition, (*it).GetSymbol(), mergedExponent, areEqual);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
BentleyStatus Expression::HandleToken(UnitsSymbolCR owner, int& depth, ExpressionR expression, 
    Utf8CP definition, TokenCR token, int startingExponent, std::function<UnitsSymbolCP(Utf8CP, IUnitsContextCP)> getSymbolByName)
    {
    LOG.debugv("%s - Handle Token: %s  TokenExp: %d  StartExp: %d", definition, token.GetName(), token.GetExponent(), startingExponent);
    int mergedExponent = token.GetExponent() * startingExponent;

    UnitsSymbolCP symbol = getSymbolByName(token.GetName(), owner.m_unitsContext);
    if (nullptr == symbol)
        {
        LOG.errorv("Failed to parse %s because the unit %s could not be found", definition, token.GetName());
        return BentleyStatus::ERROR;
        }

    if (owner.GetName().EqualsI(symbol->GetName().c_str()))
        return BentleyStatus::SUCCESS;

    if (symbol->IsBase())
        {
        MergeSymbol(definition, expression, symbol->GetDefinition().c_str(), symbol, mergedExponent, Match);
        }
    else
        {
        if (depth > maxRecursionDepth)
            {
            LOG.errorv("Stopping parse, hit Max RecursionDepth.  Equation at this point is: %s", expression.ToString().c_str());
            return BentleyStatus::ERROR;
            }

        LOG.debugv("Evaluating %s", symbol->GetName().c_str());
        Expression sourceExpression = symbol->Evaluate(depth, getSymbolByName);
        MergeExpressions(definition, expression, symbol->GetDefinition().c_str(), sourceExpression, mergedExponent);
        }

    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Colin.Kerr         02/16
+---------------+---------------+---------------+---------------+---------------+------*/
// static
BentleyStatus Expression::ParseDefinition(UnitsSymbolCR owner, int& depth, Utf8CP definition,
    ExpressionR expression, int startingExponent, std::function<UnitsSymbolCP(Utf8CP, IUnitsContextCP)> getSymbolByName)
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

            HandleToken(owner, depth, expression, definition, currentToken, startingExponent, getSymbolByName);
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
        HandleToken(owner, depth, expression, definition, currentToken, startingExponent, getSymbolByName);
        ++numTokens;
        }

    // TODO: Need some validation that folks aren't defining units with offsets in ways we don't support.
    //if (owner.HasOffset() && numTokens > 1)
    //    {
    //    LOG.errorv("%s has an invalid symbol expression (%s) because it has more than one symbol and an offset", owner.GetName(), definition);
    //    return BentleyStatus::ERROR;
    //    }

    auto new_iter = remove_if(expression.begin(), expression.end(), [](ExpressionSymbolCR a) { return a.GetExponent() == 0; });
    expression.erase(new_iter, expression.end());

    LOG.debugv("%s - DONE", definition);

    return expression.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
void Expression::Add(UnitsSymbolCP symbol, int exponent)
    {
    m_symbolExpression.push_back(ExpressionSymbol(symbol, exponent));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
// static
void Expression::Copy(ExpressionR source, ExpressionR target)
    {
    for (auto const& sourceSymbol : source)
        target.Add(sourceSymbol);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
bool Expression::Contains(ExpressionSymbolCR symbol) const
    {
    auto it = find_if(begin(), end(), [&symbol] (ExpressionSymbolCR a) { return a.GetSymbol()->GetName().EqualsI(symbol.GetSymbol()->GetName().c_str()); });
    if (it == end() || 0 == (*it).GetExponent())
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Colin.Kerr         02/16
//--------------------------------------------------------------------------------------
Utf8String ExpressionSymbol::ToString(bool includeFactors) const
    {
    if (!includeFactors)
        return Utf8PrintfString("%s^%d", GetName(), GetExponent());

    if (GetSymbol()->HasOffset())
        {
        if (GetSymbol()->GetNumerator() == 1.0 && GetSymbol()->GetDenominator() == 1.0)
            return Utf8PrintfString("(%s + %.15g)^%d", GetSymbol()->GetName().c_str(), GetSymbol()->GetOffset(), GetExponent());

        if (GetSymbol()->GetDenominator() == 1.0)
            return Utf8PrintfString("%.15g(%s + %.15g)^%d", GetSymbol()->GetNumerator(), GetSymbol()->GetName().c_str(), GetSymbol()->GetOffset(), GetExponent());

        return Utf8PrintfString("(%.15g / %.15g)(%s + %.15g)^%d", GetSymbol()->GetNumerator(), GetSymbol()->GetDenominator(), GetSymbol()->GetName().c_str(), GetSymbol()->GetOffset(), GetExponent());
        }

    if (GetSymbol()->GetNumerator() == 1.0 && GetSymbol()->GetDenominator() == 1.0)
        return Utf8PrintfString("%s^%d", GetSymbol()->GetName().c_str(), GetExponent());

    if (GetSymbol()->GetDenominator() == 1.0)
        return Utf8PrintfString("%.15g[%s]^%d", GetSymbol()->GetNumerator(), GetSymbol()->GetName().c_str(), GetExponent());

    return Utf8PrintfString("(%.15g / %.15g)[%s]^%d", GetSymbol()->GetNumerator(), GetSymbol()->GetDenominator(), GetSymbol()->GetName().c_str(), GetExponent());
    }
