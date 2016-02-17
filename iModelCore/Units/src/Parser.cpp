
#include <UnitsPCH.h>
#include <Parser.h>

USING_NAMESPACE_BENTLEY_UNITS

BentleyStatus Parser::ParseDefinition(Utf8CP definition, bvector<Utf8String>& numerator, bvector<Utf8String>& denominator)
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
 
    return numerator.size() > 0 || denominator.size() > 0 ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }