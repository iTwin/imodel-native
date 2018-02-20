/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Units/Units.h>
#include <type_traits>

UNITS_TYPEDEFS(UnitsSymbol)
UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(InverseUnit)
UNITS_TYPEDEFS(Phenomenon)
UNITS_TYPEDEFS(Expression)
UNITS_TYPEDEFS(SpecificAccuracy)
UNITS_TYPEDEFS(UnitSynonymMap)
UNITS_TYPEDEFS(UnitSystem)

BEGIN_BENTLEY_UNITS_NAMESPACE
//! @addtogroup UnitsGroup
//! @beginGroup
BE_JSON_NAME(synonymMap)
BE_JSON_NAME(unitName)
BE_JSON_NAME(synonym)

typedef bvector<UnitSynonymMap> T_UnitSynonymVector;

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
    double GetMinResolution() {return m_minResoluton;}
    double GetMinErrorMargin() {return m_minErrorMargin;}
    UnitCP GetResolutionUnit() {return m_minResolutionUnit;}
    size_t GetMaxDecimalPrecision() {return m_maxDecimalPrecision;}

    //UNITS_EXPORT Quantity GetResolutionQuantity(){ return Quantity(m_minResoluton, m_minResolutionUnit); }

    //UNITS_EXPORT static bool IsIndistinguishable(QuantityCR q1, QuantityCR q2);
    //UNITS_EXPORT static ComparisonCode Compare(QuantityCR q1, QuantityCR q2);

};

struct UnitRegistry;
struct Expression;

//=======================================================================================
// @bsistruct
//=======================================================================================
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

//=======================================================================================
// @bsiclass                                                    Colin.Kerr         01/18
//=======================================================================================
struct UnitSystem
{
friend struct StandardUnitSystems;
friend struct UnitRegistry;
private:
    Utf8String m_name;

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    UnitSystem() = delete;
    UnitSystem(UnitSystemCR system) = delete;
    UnitSystemR operator=(UnitSystemCR unit) = delete;

protected:
    static UnitSystemP _Create(Utf8CP name) {return new UnitSystem(name);}

    UnitSystem(Utf8CP name) : m_name(name) {}
    virtual ~UnitSystem() {}

public:
    Utf8StringCR GetName() const {return m_name;}
};

//=======================================================================================
// @bsistruct
//=======================================================================================
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

    mutable bool m_evaluated;
    Expression* m_symbolExpression;

    uint32_t GetId()   const {return m_id;}
    bool    IsBaseSymbol() const {return ' ' != m_baseSymbol;}
    bool    IsDimensionless() const {return m_dimensionless;}

    Utf8Char GetBaseSymbol() const {return m_baseSymbol;}
    virtual uint32_t GetPhenomenonId() const = 0;

protected:
    UNITS_EXPORT UnitsSymbol();
    UNITS_EXPORT UnitsSymbol(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id, double factor, double offset);
    ExpressionCR Evaluate(int depth, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName) const;
    UNITS_EXPORT virtual ~UnitsSymbol();

public:
    Utf8StringCR GetName() const {return m_name;}
    Utf8StringCP GetNameSP() const {return &m_name;}
    Utf8StringCR GetDefinition() const {return m_definition;}
    double GetFactor() const {return m_factor;}
    bool HasOffset() const {return 0.0 != m_offset;}
    double GetOffset() const {return m_offset;}
    void SetName(Utf8CP name) {m_name = name;}
};

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : UnitsSymbol
{
DEFINE_T_SUPER(UnitsSymbol)
friend struct UnitRegistry;
friend struct Expression;
friend struct InverseUnit;

private:
    // TODO: Should these be a reference because it must be set?
    UnitSystemCP    m_system;
    PhenomenonCP    m_phenomenon;

    UnitCP          m_parent;
    bool            m_isConstant;
    mutable Utf8String m_displayLabel;
    mutable Utf8String m_displayDescription;

    Unit() :UnitsSymbol(), m_system(nullptr), m_phenomenon(nullptr), m_parent(nullptr), m_isConstant(true) {}
    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.

    Unit(UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

    ExpressionCR Evaluate() const;

    UNITS_EXPORT uint32_t GetPhenomenonId() const override;
    UnitCP CombineWithUnit(UnitCR rhs, int factor) const;


    UnitsProblemCode DoNumericConversion(double& converted, double value, UnitCR toUnit) const;
    bool GenerateConversion(UnitCR toUnit, Conversion& conversion) const;

protected:
    // Needs to be overriden by any sub class
    static UnitP _Create(UnitSystemCR sysName, PhenomenonCR phenomenon, Utf8CP unitName, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
        {
        NativeLogging::LoggingManager::GetLogger(L"UnitsNative")->debugv("Creating unit %s  Factor: %.17g  Offset: %d", unitName, factor, offset);
        return new Unit(sysName, phenomenon, unitName, id, definition, baseSymbol, factor, offset, isConstant);
        }

    UNITS_EXPORT static UnitP _Create(UnitCR parentUnit, UnitSystemCR system, Utf8CP unitName, uint32_t id);

    Unit(UnitSystemCR system, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset, bool isConstant) 
        : UnitsSymbol(name, definition, dimensonSymbol, id, factor, offset), m_parent(nullptr), m_isConstant(isConstant), m_system(&system), m_phenomenon(&phenomenon)
        {}

    Unit(UnitCR parentUnit, UnitSystemCR system, Utf8CP name, uint32_t id)
        : Unit(system, *(parentUnit.GetPhenomenon()), name, id, parentUnit.GetDefinition().c_str(), parentUnit.GetBaseSymbol(), 0, 0, false)
        {
        m_parent = &parentUnit;
        }
    UnitCP GetParent() const {return m_parent;}
    UNITS_EXPORT void SetLabel(Utf8CP label) {m_displayLabel = label;}

public:
    UNITS_EXPORT Utf8String GetUnitSignature() const;
    UNITS_EXPORT Utf8String GetParsedUnitExpression() const;
    UNITS_EXPORT UnitsProblemCode Convert(double& converted, double value, UnitCP toUnit) const;
    UNITS_EXPORT Utf8StringCR GetLabel() const;
    UNITS_EXPORT Utf8StringCR GetInvariantLabel() const;
    UNITS_EXPORT Utf8CP GetDescription() const;

    bool IsSI() const {return 0 == strcmp(m_system->GetName().c_str(), "SI");} // TODO: Replace with something better ... SI is a known system

    UNITS_EXPORT bool IsInvertedUnit() const {return nullptr != m_parent;} //!< Indicates if this unit is an InverseUnit or not
    bool IsRegistered() const; //!< Indicates if this Unit is in the UnitRegistry singleton
    bool IsConstant() const {return m_isConstant;} //!< Indicates if this Unit is constant.
    UnitSystemCP GetUnitSystem() const {return m_system;} //!< Gets the UnitSystem for this Unit.
    PhenomenonCP GetPhenomenon() const {return m_phenomenon;} //!< Gets the Phenomenon for this Unit.

    UnitCP MultiplyUnit(UnitCR rhs) const;
    UnitCP DivideUnit(UnitCR rhs) const;

    UNITS_EXPORT void AddSynonym(Utf8CP synonym) const;
    UNITS_EXPORT size_t GetSynonymList(bvector<Utf8CP>& synonyms) const;

    static bool IsNegligible(double dval) {return (1.0e-16 > dval);}
    //! Returns true if the input units have the same id, false if not or if one or both are null.
    static bool AreEqual(UnitCP unitA, UnitCP unitB) {return nullptr == unitA || nullptr == unitB ? false : unitA->GetId() == unitB->GetId();}
    //! Returns true if the input units belong to the same phenomenon, false if not or one or both are null.
    UNITS_EXPORT static bool AreCompatible(UnitCP unitA, UnitCP unitB);
};

//=======================================================================================
//! UnitSynonymMap links a non-canonical unit name (synonym) to the specific Unit. Attaching 
//! "synonyms" to the specific Units helps in avoiding conflict between synonyms. However, 
//! each synonym must be unique among all synonyms of the specific Phenomenon. The canonical
//! Unit name must be unique among all units of all Phenomena
//!
// @bsistruct
//=======================================================================================
struct UnitSynonymMap
{
private:
    UnitCP m_unit;
    Utf8String m_synonym;

    UNITS_EXPORT void Init(Utf8CP unitName, Utf8CP synonym);
    UNITS_EXPORT void LoadJson(Json::Value jval);
public:
    UnitSynonymMap() {m_unit = nullptr; m_synonym.clear();}
    
    //! The first argument of the constructor is a required text string that can contain of of the 
    //! following:
    //!   a valid canonical Unit name
    //!   a valid canonical Unit name followed by comma and the synonym: UnitName,synonym
    //!   a Json string that containts a canonical Unit name and its synonym: {"synonym":"^","unitName":"ARC_DEG"}
    //! When the first argument contains only the name of the unit, the second argument must be a synonym
    //! Invalide names or their invalid combination will result in the empty Map
    UNITS_EXPORT UnitSynonymMap(Utf8CP unitName, Utf8CP synonym = nullptr);
    UNITS_EXPORT UnitSynonymMap(UnitCP unit, Utf8CP synonym) :m_unit(unit), m_synonym(synonym) {}

    //UNITS_EXPORT UnitSynonymMap(Utf8CP descriptor);
    UNITS_EXPORT UnitSynonymMap(Json::Value jval);

    bool IsMapEmpty() {return (nullptr == m_unit) && m_synonym.empty();}
    Utf8CP GetSynonym() const {return m_synonym.c_str();}
    Utf8CP GetUnitName() const {return m_unit->GetName().c_str();}
    UnitCP GetUnit() const {return m_unit;}
    PhenomenonCP GetPhenomenon() const{return (nullptr == m_unit) ? nullptr : m_unit->GetPhenomenon();}
    UNITS_EXPORT Json::Value ToJson();
    UNITS_EXPORT bool IsIdentical(UnitSynonymMapCR other);
    UNITS_EXPORT static bool AreVectorsIdentical(bvector<UnitSynonymMap>& v1, bvector<UnitSynonymMap>& v2);
    UNITS_EXPORT static bvector<UnitSynonymMap> MakeUnitSynonymVector(Json::Value jval);
    UNITS_EXPORT static size_t AugmentUnitSynonymVector(bvector<UnitSynonymMap>& mapV, Utf8CP unitName, Utf8CP synonym);
    UNITS_EXPORT static bool CompareSynonymMap(UnitSynonymMapCR map1, UnitSynonymMapCR map2);
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct Phenomenon : UnitsSymbol
{
DEFINE_T_SUPER(UnitsSymbol)
friend struct Unit;
friend struct UnitRegistry;
friend struct Expression;

private:
    bvector<UnitCP> m_units;
    mutable bvector<UnitSynonymMap> m_altNames;
    mutable Utf8String m_displayLabel;

    void AddUnit(UnitCR unit) 
        {
        auto it = std::find_if(m_units.begin(), m_units.end(), [&unit](UnitCP existingUnit) {return existingUnit->GetId() == unit.GetId();});
        if (it == m_units.end())
            m_units.push_back(&unit);
        }
    void AddMap(UnitSynonymMapCR map);
    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

    ExpressionCR Evaluate() const;

    UNITS_EXPORT uint32_t GetPhenomenonId() const override {return GetId();}

protected:
    UNITS_EXPORT static PhenomenonP _Create(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) {return new Phenomenon(name, definition, baseSymbol, id);}
    UNITS_EXPORT Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) : UnitsSymbol(name, definition, baseSymbol, id, 0.0, 0) {}
    UNITS_EXPORT void SetLabel(Utf8CP label) {m_displayLabel = label;}

public:
    UNITS_EXPORT Utf8String GetPhenomenonSignature() const;

    bool HasUnits() const {return m_units.size() > 0;}
    bool HasSynonyms() const {return m_altNames.size() > 0;}
    bvector<UnitCP> const GetUnits() const {return m_units;}
    UnitCP GetSIUnit() const {auto it = std::find_if(m_units.begin(), m_units.end(), [](UnitCP unit) {return unit->IsSI();});  return m_units.end() == it ? nullptr : *it;}

    UNITS_EXPORT bool IsCompatible(UnitCR unit) const;
    bool Equals(PhenomenonCR comparePhenomenon) const {return GetPhenomenonId() == comparePhenomenon.GetPhenomenonId();}
    static bool AreEqual(PhenomenonCP phenA, PhenomenonCP phenB)
        {return nullptr == phenA || nullptr == phenB ? false : phenA->GetId() == phenB->GetId();}

    UNITS_EXPORT bool IsLength() const;
    UNITS_EXPORT bool IsTime() const;
    UNITS_EXPORT bool IsAngle() const;
    UNITS_EXPORT UnitCP LookupUnit(Utf8CP unitName) const;
    UNITS_EXPORT UnitCP FindSynonym(Utf8CP synonym) const;
    UNITS_EXPORT void AddSynonym(Utf8CP unitName, Utf8CP synonym);
    UNITS_EXPORT void AddSynonym(UnitCP unitP, Utf8CP synonym) const;
    UNITS_EXPORT void AddSynonymMap(UnitSynonymMapCR map) const;
    UNITS_EXPORT void AddSynonymMaps(Json::Value jval) const;
    UNITS_EXPORT Json::Value SynonymMapToJson() const;
    UNITS_EXPORT static Json::Value SynonymMapVectorToJson(bvector<UnitSynonymMap> mapV);
    UNITS_EXPORT T_UnitSynonymVector* GetSynonymVector() const {return &m_altNames;}
    UNITS_EXPORT size_t GetSynonymCount() const {return m_altNames.size();}
    UNITS_EXPORT Utf8StringCR GetLabel() const;
    UNITS_EXPORT Utf8StringCR GetInvariantLabel() const;
};

/** @endGroup */
END_BENTLEY_UNITS_NAMESPACE
/*__PUBLISH_SECTION_END__*/
