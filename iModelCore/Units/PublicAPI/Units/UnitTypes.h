/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Units/Units.h>


UNITS_TYPEDEFS(UnitsSymbol)
UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(InverseUnit)
UNITS_TYPEDEFS(Phenomenon)
UNITS_TYPEDEFS(Expression)
UNITS_TYPEDEFS(SpecificAccuracy)
UNITS_TYPEDEFS(UnitSynonymMap)
BEGIN_BENTLEY_UNITS_NAMESPACE

BE_JSON_NAME(unitName)
BE_JSON_NAME(synonym)


enum class UnitsProblemCode
    {
    NoProblem = 0,
    UncomparableUnits = 31,     //!< Units provided on the argument list are not comparable
    InvalidUnitName = 32,       //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    InvertingZero = 33
    };

enum class ComparisonCode
    {
    Indistinguishable = 0,
    Lesser = 1, 
    Greater = 2, 
    Uncomparable  = 100    //!< Units provided on the argument list are not comparable
    };

typedef bvector<Utf8String> Utf8Vector;

struct SpecificAccuracy
    {
private:
    double m_minResoluton;
    UnitCP m_minResolutionUnit;
    double m_minErrorMargin;
    size_t m_maxDecimalPrecision;  // i.e. number of reliable (significant) digits
public:
    SpecificAccuracy(double minRes, UnitCP resUnit, double errMargin, size_t prec): m_minResoluton(minRes),
        m_minResolutionUnit(resUnit), m_minErrorMargin(errMargin), m_maxDecimalPrecision(prec){}
    double GetMinResolution() { return m_minResoluton; }
    double GetMinErrorMargin() { return m_minErrorMargin; }
    UnitCP GetResolutionUnit() { return m_minResolutionUnit; }
    size_t GetMaxDecimalPrecision() { return m_maxDecimalPrecision; }

    //UNITS_EXPORT Quantity GetResolutionQuantity(){ return Quantity(m_minResoluton, m_minResolutionUnit); }

    //UNITS_EXPORT static bool IsIndistinguishable(QuantityCR q1, QuantityCR q2);
    //UNITS_EXPORT static ComparisonCode Compare(QuantityCR q1, QuantityCR q2);

    };

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
    UNITS_EXPORT UnitsSymbol();
    UnitsSymbol(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id, double factor, double offset);   
    ExpressionCR Evaluate(int depth, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName) const;
    virtual ~UnitsSymbol();
      
public:
    Utf8CP  GetName() const { return m_name.c_str(); }
    Utf8StringCP GetNameSP() const { return &m_name; }
    Utf8CP  GetDefinition() const { return m_definition.c_str(); }
    double  GetFactor() const { return m_factor; }
    bool    HasOffset() const { return 0.0 != m_offset; }
    double  GetOffset() const { return m_offset; }
    void SetName(Utf8CP name) { m_name = name; }
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
    Unit() :UnitsSymbol(), m_system(""), m_phenomenon(nullptr), m_parent(nullptr), m_isConstant(true) {}
    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.

    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    ExpressionCR Evaluate() const;

    uint32_t GetPhenomenonId() const override;
    UnitCP  CombineWithUnit(UnitCR rhs, int factor) const;
    bool    IsInverseUnit() const { return nullptr != m_parent; }
    
    UnitsProblemCode  DoNumericConversion(double& converted, double value, UnitCR toUnit) const;
    bool    GenerateConversion(UnitCR toUnit, Conversion& conversion) const;

public:
    UNITS_EXPORT Utf8String GetUnitSignature() const;
    UNITS_EXPORT Utf8String GetParsedUnitExpression() const;
    UNITS_EXPORT UnitsProblemCode Convert(double& converted, double value, UnitCP toUnit) const;

    bool IsRegistered()    const;
    bool IsConstant() const { return m_isConstant; }
    Utf8CP GetUnitSystem() const { return m_system.c_str(); }

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }

    UnitCP MultiplyUnit (UnitCR rhs) const;
    UnitCP DivideUnit(UnitCR rhs) const;
    static bool IsNegligible(double dval) { return (1.0e-16 > dval); }
    //! Returns true if the input units have the same id, false if not or if one or both are null.
    static bool AreEqual(UnitCP unitA, UnitCP unitB)
        { return nullptr == unitA || nullptr == unitB ? false : unitA->GetId() == unitB->GetId(); }
    //! Returns true if the input units belong to the same phenomenon, false if not or one or both are null.
    UNITS_EXPORT static bool AreCompatible(UnitCP unitA, UnitCP unitB);
    };

struct UnitSynonymMap
    {
private:
    UnitCP m_unit;
    Utf8String m_synonym;

    UNITS_EXPORT void Init(Utf8CP unitName, Utf8CP synonym);
public:
    UnitSynonymMap() { m_unit = nullptr; m_synonym.clear(); }
    UNITS_EXPORT UnitSynonymMap(Utf8CP unitName, Utf8CP synonym); 
    //! two comma separated names. The first name must be a registered Unit name.
    UNITS_EXPORT UnitSynonymMap(Utf8CP descriptor);
    UNITS_EXPORT UnitSynonymMap(Json::Value jval);
    bool IsMapEmpty() { return (nullptr == m_unit) && m_synonym.empty(); }
    Utf8CP GetSynonym() const { return m_synonym.c_str(); }
    UnitCP GetUnit() const { return m_unit; }
    PhenomenonCP GetPhenomenon() const{ return (nullptr == m_unit) ? nullptr : m_unit->GetPhenomenon(); }
    UNITS_EXPORT Json::Value ToJson();
    UNITS_EXPORT bool IsIdentical(UnitSynonymMapCR other);
    UNITS_EXPORT static bool AreVectorsIdentical(bvector<UnitSynonymMap>& v1, bvector<UnitSynonymMap>& v2);
    UNITS_EXPORT static bvector<UnitSynonymMap> MakeUnitSynonymVector(Json::Value jval);
    UNITS_EXPORT static size_t AugmentUnitSynonymVector(bvector<UnitSynonymMap>& mapV, Utf8CP unitName, Utf8CP synonym);

    };

struct Phenomenon final : UnitsSymbol
    {
DEFINE_T_SUPER(UnitsSymbol)
friend struct Unit;
friend struct UnitRegistry;
friend struct Expression;

private:
    bvector<UnitCP> m_units;
    mutable bvector<UnitSynonymMap> m_altNames;

    void AddUnit(UnitCR unit);
    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) : UnitsSymbol(name, definition, baseSymbol, id, 0.0, 0) {}
    void AddMap(UnitSynonymMapCR map);
    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

    ExpressionCR Evaluate() const;

    uint32_t GetPhenomenonId() const override { return GetId(); }

public:
    UNITS_EXPORT Utf8String GetPhenomenonSignature() const;

    bool HasUnits() const { return m_units.size() > 0; }
    bvector<UnitCP> const GetUnits() const { return m_units; }

    UNITS_EXPORT bool IsCompatible(UnitCR unit) const;
    bool Equals(PhenomenonCR comparePhenomenon) const {return GetPhenomenonId() == comparePhenomenon.GetPhenomenonId();}
    static bool AreEqual(PhenomenonCP phenA, PhenomenonCP phenB) 
        { return nullptr == phenA || nullptr == phenB ? false : phenA->GetId() == phenB->GetId(); }

    UNITS_EXPORT bool IsLength() const;
    UNITS_EXPORT bool IsTime() const;
    UNITS_EXPORT bool IsAngle() const;
    UNITS_EXPORT UnitCP LookupUnit(Utf8CP unitName);
    UNITS_EXPORT UnitCP FindSynonym(Utf8CP synonym) const;
    UNITS_EXPORT void AddSynonym(Utf8CP unitName, Utf8CP synonym);
    UNITS_EXPORT void AddSynonymMap(UnitSynonymMapCR map) const;
    UNITS_EXPORT void AddSynonymMaps(Json::Value jval) const;
    UNITS_EXPORT Json::Value SynonymMapToJson();
    UNITS_EXPORT static Json::Value SynonymMapVectorToJson(bvector<UnitSynonymMap> mapV);
};
END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
