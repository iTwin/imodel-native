/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Units/Units.h>

UNITS_TYPEDEFS(UnitsSymbol)
UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(InverseUnit)
UNITS_TYPEDEFS(Phenomenon)
UNITS_TYPEDEFS(Expression)
BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct UnitRegistry;
struct Expression;

struct Conversion
    {
    double Factor;
    double Offset;
    Conversion()
        {
        Factor = 0.0;
        Offset = 0.0;
        }
    };

struct UnitsSymbol
    {
friend struct ExpressionSymbol;
friend struct Expression;
friend struct Unit;
friend struct Phenomenon;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    uint32_t    m_id;
    Utf8Char    m_baseSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_dimensionless;
    
    mutable bool        m_evaluated;
    Expression * m_symbolExpression;

    uint32_t GetId()   const { return m_id; }
    bool    IsBaseSymbol() const { return ' ' != m_baseSymbol; }
    bool    IsDimensionless() const { return m_dimensionless; }
    
    Utf8Char GetBaseSymbol() const { return m_baseSymbol; }
    virtual uint32_t GetPhenomenonId() const = 0;

protected:
    UnitsSymbol(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id, double factor, double offset);
    
    ExpressionCR Evaluate(int depth, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName) const;
protected:
    virtual ~UnitsSymbol();
      
public:
    Utf8CP  GetName() const { return m_name.c_str(); }
    Utf8CP  GetDefinition() const { return m_definition.c_str(); }
    double  GetFactor() const { return m_factor; }
    bool    HasOffset() const { return 0.0 != m_offset; }
    double  GetOffset() const { return m_offset; }
    };


//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit final : UnitsSymbol
    {
DEFINE_T_SUPER(UnitsSymbol)
friend struct UnitRegistry;
friend struct Expression;
friend struct InverseUnit;

private:
    Utf8String      m_system;
    // TODO: Should this be a reference because it must be set?
    PhenomenonCP    m_phenomenon;
    UnitCP          m_parent;
    bool            m_isConstant;

    static UnitP Create(Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant);
    static UnitP Create(UnitCR parentUnit, Utf8CP unitName, uint32_t id);

    Unit (Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant);
    Unit(UnitCR parentUnit, Utf8CP name, uint32_t id);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit() = delete;
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    ExpressionCR Evaluate() const;

    uint32_t GetPhenomenonId() const;
    UnitCP  CombineWithUnit(UnitCR rhs, int factor) const;
    bool    IsInverseUnit() const { return nullptr != m_parent; }
    
    double  DoNumericConversion(double value, UnitCR toUnit) const;
    bool    GenerateConversion(UnitCR toUnit, Conversion& conversion) const;

public:
    UNITS_EXPORT Utf8String GetUnitSignature() const;
    UNITS_EXPORT Utf8String GetParsedUnitExpression() const;
    UNITS_EXPORT double Convert(double value, UnitCP toUnit) const;

    bool IsRegistered()    const;
    bool IsConstant() const { return m_isConstant; }
    Utf8CP GetUnitSystem() const { return m_system.c_str(); }

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }

    UnitCP MultiplyUnit (UnitCR rhs) const;
    UnitCP DivideUnit(UnitCR rhs) const;
};

struct Phenomenon final : UnitsSymbol
    {
DEFINE_T_SUPER(UnitsSymbol)
friend struct Unit;
friend struct UnitRegistry;
friend struct Expression;

private:
    bvector<UnitCP> m_units;

    void AddUnit(UnitCR unit);

    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) : UnitsSymbol(name, definition, baseSymbol, id, 0.0, 0) {}

    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

    ExpressionCR Evaluate() const;

    uint32_t GetPhenomenonId() const { return GetId(); }

public:
    UNITS_EXPORT Utf8String GetPhenomenonSignature() const;

    bool HasUnits() const { return m_units.size() > 0; }
    bvector<UnitCP> const GetUnits() const { return m_units; }

    UNITS_EXPORT bool IsCompatible(UnitCR unit) const;
};
END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
