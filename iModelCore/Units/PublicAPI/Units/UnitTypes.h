/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(Symbol)
UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(InverseUnit)
UNITS_TYPEDEFS(Phenomenon)
UNITS_TYPEDEFS(Expression)
BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct UnitRegistry;
struct Expression;

struct Symbol
    {
    friend struct ExpressionSymbol;
    friend struct Expression;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    int         m_id;
    Utf8Char    m_dimensionSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_dimensionless;
    
    mutable bool        m_evaluated;
    Expression * m_symbolExpression;

protected:
    Symbol(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id, double factor, double offset);
    
    ExpressionCR Evaluate(int depth, std::function<SymbolCP(Utf8CP)> getSymbolByName) const;
protected:
    virtual ~Symbol();
      
public:
    Utf8CP  GetName() const { return m_name.c_str(); }
    // TODO: Consider making private because it will changed depending on load order.
    int     GetId()   const { return m_id; }
    Utf8CP  GetDefinition() const { return m_definition.c_str(); }
    double  GetFactor() const { return m_factor; }
    bool    HasOffset() const { return 0.0 != m_offset; }
    double  GetOffset() const { return m_offset; }
    bool    IsBaseSymbol() const { return ' ' != m_dimensionSymbol; }
    bool    IsDimensionless() const { return m_dimensionless; }

    virtual int GetPhenomenonId() const = 0;
    };

//struct InverseUnit : Unit
//    {
//DEFINE_T_SUPER(Unit)
//private:
//    UnitCP m_parentUnit;
//    Utf8String m_parentUnitName;
//    
//    static UnitP Create(Utf8CP parentUnitName, Utf8CP unitName, int id);
//    InverseUnit(UnitCR parentUnit, Utf8CP name, int id);
//
//    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
//    InverseUnit() = delete;
//    InverseUnit(InverseUnitCR unit) = delete;
//    InverseUnitR operator=(InverseUnitCR unit) = delete;
//
//    UnitCP GetParentUnit() const { return m_parentUnit; }
//
//    };

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;
friend struct Expression;

private:
    Utf8String      m_system;
    // TODO: Should this be a reference because it must be set?
    PhenomenonCP    m_phenomenon;

    static UnitP Create(Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset);

    Unit (Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit() = delete;
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    ExpressionCR Evaluate() const;

    int GetPhenomenonId() const;
    UnitCP CombineWithUnit(UnitCR rhs, int factor) const;

public:
    UNITS_EXPORT Utf8String GetUnitDimension() const;
    UNITS_EXPORT double Convert(double value, UnitCP toUnit) const;

    bool IsRegistered()    const;

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }

    UnitCP MultiplyUnit (UnitCR rhs) const;
    UnitCP DivideUnit(UnitCR rhs) const;
};

struct Phenomenon final : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct Unit;
friend struct UnitRegistry;
friend struct Expression;

private:
    bvector<UnitCP> m_units;

    void AddUnit(UnitCR unit);

    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id) : Symbol(name, definition, dimensionSymbol, id, 0.0, 0) {}

    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

    ExpressionCR Evaluate() const;

    int GetPhenomenonId() const { return GetId(); }

public:
    UNITS_EXPORT Utf8String GetPhenomenonDimension() const;

    bool HasUnits() const { return m_units.size() > 0; }
    bvector<UnitCP> const GetUnits() const { return m_units; }

    UNITS_EXPORT bool IsCompatible(UnitCR unit) const;
};
END_BENTLEY_UNITS_NAMESPACE