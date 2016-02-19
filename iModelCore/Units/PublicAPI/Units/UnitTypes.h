/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>
//#include <Units/UnitRegistry.h>

UNITS_TYPEDEFS(Unit);
UNITS_TYPEDEFS(Phenomenon);
UNITS_TYPEDEFS(SymbolicFraction);

BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

// TODO: Still needed?
struct SymbolicFraction
    {
friend struct Unit;
friend struct Phenomenon;
private:
    Utf8Vector m_numerator;
    Utf8Vector m_denominator;
    
    SymbolicFraction(Utf8CP definition);
    SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator);

protected:
    static void SimplifySubTypes(Utf8Vector &n, Utf8Vector &d);
    
    Utf8Vector& GetNumerator() { return m_numerator; }
    Utf8Vector& GetDenominator() { return m_denominator; }

public:
    SymbolicFraction operator*(SymbolicFractionCR rhs) const;
    SymbolicFraction operator/(SymbolicFractionCR rhs) const;
    
    Utf8Vector const& Numerator() const { return m_numerator; }
    Utf8Vector const& Denominator() const { return m_denominator; }

    bool operator== (const SymbolicFraction& rhs) const;
    bool operator!= (const SymbolicFraction& rhs) const;
};


struct UnitRegistry;

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit
    {
friend struct UnitRegistry;

    struct UnitExponent
        {
        UnitCP  m_unit;
        int     m_exponent;

        UnitExponent(UnitCP unit, int exponent) : m_exponent(exponent) { m_unit = unit; }
        };

private:
    Utf8String  m_system;
    Utf8String  m_phenomenon;
    Utf8String  m_name;
    Utf8String  m_definition;
    Utf8Char    m_dimensionSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_isConstant;

    mutable bvector<UnitExponent*>   m_unitFormula;
    mutable bool                     m_evaluated;

    static UnitP Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset, bool isConstant);

    static BentleyStatus ParseDefinition(Utf8CP definition, bvector<UnitExponent*>& unitFormula, int startingExponent);
    static void MergeExpressions(bvector<UnitExponent*>& targetExpression, bvector<UnitExponent*>& sourceExpression, int startingExponent);
    static BentleyStatus AddUFEToExpression(bvector<UnitExponent*>& unitExpression, Utf8CP definition, Utf8CP token, int mergedExponent);
    static BentleyStatus HandleToken(bvector<UnitExponent*>& unitExpression, Utf8CP definition, Utf8CP constToken, Utf8CP token, int tokenExponent, int startingExponent);

    // TODO: Create a better definition of an "unknown" unit
    Unit (Utf8Vector& numerator, Utf8Vector& denominator);
    Unit (Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    bvector<Unit::UnitExponent*>& Evaluate() const;
    void PrintForumula(UnitCP unit, const bvector<UnitExponent*> expression) const;

protected:
    virtual Utf8CP _GetName() const { return m_name.c_str(); }
    
public:
    virtual ~Unit() { }

    UNITS_EXPORT double GetConversionTo(UnitCP unit) const;

    bool   IsRegistered()    const;
    Utf8CP GetName()         const { return _GetName(); }
    Utf8CP GetPhenomenon()   const { return m_phenomenon.c_str(); }
    Utf8CP GetDefinition()   const { return m_definition.c_str(); }
    double GetFactor()       const { return m_factor; }
    //Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

    bool IsBaseUnit() const { return ' ' != m_dimensionSymbol; }
    bool IsConstant() const { return m_isConstant; }

    // Binary comparison operators.
    bool operator== (UnitCR rhs) const;
    bool operator!= (UnitCR rhs) const;

    // Arithmetic operators.
    UnitCR operator*(UnitCR rhs) const;
    UnitCR operator/(UnitCR rhs) const;
};


struct Phenomenon : SymbolicFraction
{
friend struct UnitRegistry;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    Utf8Char    m_dimensionSymbol;

    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionalSymbol) : SymbolicFraction(definition), m_name(name) {}

public:
    bool IsBasePhenomena() const { return ' ' != m_dimensionSymbol; }
};
END_BENTLEY_UNITS_NAMESPACE