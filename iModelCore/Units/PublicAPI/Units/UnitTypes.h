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
struct SymbolicExpression;

struct Symbol
    {
    friend struct SymbolWithExponent;
    friend struct SymbolicExpression;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    int         m_id;
    Utf8Char    m_dimensionSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_dimensionless;
    
    mutable bool        m_evaluated;
    SymbolicExpression * m_symbolExpression;   

protected:
    Symbol(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id, double factor, double offset);
    
    SymbolicExpression& Evaluate(std::function<SymbolCP(Utf8CP)> getSymbolByName) const;
protected:
    virtual ~Symbol();

    virtual Utf8CP _GetName() const;
    virtual int    _GetId()   const;
    virtual Utf8CP _GetDefinition() const;
    virtual double _GetFactor() const;
    virtual bool _IsBaseSymbol() const;
    virtual bool _IsDimensionless() const;
    //virtual bool _IsCompatibleWith(SymbolCR rhs) const { return SymbolicExpression::DimensionallyCompatible(*this, rhs); }

    // Binary comparison operators.
    bool operator== (SymbolCR rhs) const { return m_id == rhs.m_id; }
    bool operator!= (SymbolCR rhs) const { return m_id != rhs.m_id; }
    };

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;

private:
    Utf8String      m_system;
    PhenomenonCP    m_phenomenon;
    bool            m_isConstant;

    static UnitP Create (Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset, bool isConstant);

    Unit (Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

public:
    virtual ~Unit() { }

    UNITS_EXPORT double GetConversionTo(UnitCP unit) const;

    UNITS_EXPORT Utf8CP GetName() const { return _GetName(); }
    UNITS_EXPORT Utf8CP GetDefinition() const { return _GetDefinition(); }
    UNITS_EXPORT double GetFactor() const { return _GetFactor(); }

    bool IsRegistered()    const;
    bool IsConstant() const { return m_isConstant; }

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }
};

struct Phenomenon : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;

private:
    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id) : Symbol(name, definition, dimensionSymbol, id, 0.0, 0) {}

public:
    virtual ~Phenomenon() { };

    UNITS_EXPORT Utf8String GetPhenomenonDimension() const;

    UNITS_EXPORT Utf8CP GetName() const { return _GetName(); }
    UNITS_EXPORT Utf8CP GetDefinition() const { return _GetDefinition(); }
    UNITS_EXPORT double GetFactor() const { return _GetFactor(); }
    };

END_BENTLEY_UNITS_NAMESPACE