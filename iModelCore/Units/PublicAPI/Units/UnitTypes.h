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
UNITS_TYPEDEFS(Phenomenon)

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
    
    Expression& Evaluate(int depth, std::function<SymbolCP(Utf8CP)> getSymbolByName) const;
protected:
    virtual ~Symbol();

    virtual int GetPhenomenonId() const = 0;
  
public:
    UNITS_EXPORT Utf8CP GetName() const;
    // TODO: Consider making private because it will changed depending on load order.
    UNITS_EXPORT int    GetId()   const;
    UNITS_EXPORT Utf8CP GetDefinition() const;
    UNITS_EXPORT double GetFactor() const;
    UNITS_EXPORT bool IsBaseSymbol() const;
    UNITS_EXPORT bool IsDimensionless() const;

    bool IsCompatibleWith(SymbolCR rhs) const;

    // Binary comparison operators.
    bool operator== (SymbolCR rhs) const { return m_id == rhs.m_id; }
    bool operator!= (SymbolCR rhs) const { return m_id != rhs.m_id; }
    };

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit final: Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;
friend struct Expression;

private:
    Utf8String      m_system;
    // TODO: Should this be a reference because it must be set?
    PhenomenonCP    m_phenomenon;
    bool            m_isConstant;

    static UnitP Create (Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset, bool isConstant);

    Unit (Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit() = delete;
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    Expression& Evaluate() const;

    int GetPhenomenonId() const;

public:
    UNITS_EXPORT double GetConversionTo(UnitCP unit) const;

    // TODO: Should GetId be private?  Should probably only be used internally because id is not gaurantteed to be consistent between runs (though it is because units are added in our code)

    bool IsRegistered()    const;
    bool IsConstant() const { return m_isConstant; }

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }
};

struct Phenomenon final : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;
friend struct Expression;

private:
    bvector<UnitCP> m_units;

    void AddUnit(UnitCR unit);

    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id) : Symbol(name, definition, dimensionSymbol, id, 0.0, 0) {}

    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

    Expression& Evaluate() const;

    int GetPhenomenonId() const { return GetId(); }

public:
    UNITS_EXPORT Utf8String GetPhenomenonDimension() const;

    bool HasUnits() const { return m_units.size() > 0; }
    bvector<UnitCP> const GetUnits() const { return m_units; }

    UNITS_EXPORT bool IsCompatible(UnitCR unit) const;
};
END_BENTLEY_UNITS_NAMESPACE