/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Units/Units.h>
#include <Formatting/FormattingDefinitions.h>
#include <Formatting/FormattingEnum.h>
#include <Formatting/AliasMappings.h>
#include <Bentley/Nullable.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormatSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValueSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Format)

// Json presentation
BE_JSON_NAME(type)
BE_JSON_NAME(roundFactor)
BE_JSON_NAME(precision)
BE_JSON_NAME(scientificType)
BE_JSON_NAME(showSignOption)
BE_JSON_NAME(formatTraits)
BE_JSON_NAME(decimalSeparator)
BE_JSON_NAME(thousandSeparator)
BE_JSON_NAME(uomSeparator)
BE_JSON_NAME(stationSeparator)
BE_JSON_NAME(stationOffsetSize)
BE_JSON_NAME(minWidth)

// Format Traits
BE_JSON_NAME(trailZeroes)
BE_JSON_NAME(leadZeroes)
BE_JSON_NAME(keepSingleZero)
BE_JSON_NAME(zeroEmpty)
BE_JSON_NAME(keepDecimalPoint)
BE_JSON_NAME(applyRounding)
BE_JSON_NAME(fractionDash)
BE_JSON_NAME(showUnitLabel)
BE_JSON_NAME(prependUnitLabel)
BE_JSON_NAME(use1000Separator)
BE_JSON_NAME(exponentOnlyNegative)

// Format
BE_JSON_NAME(SpecType)
BE_JSON_NAME(composite)

//UnitProxy
BE_JSON_NAME(name)
BE_JSON_NAME(label)

//CompositeValueSpec
BE_JSON_NAME(units)
BE_JSON_NAME(includeZero)
BE_JSON_NAME(spacer)

//=======================================================================================
//
// Class containing static methods used to populate UI.
//
// @bsistruct                                            Bill.Steinbock  11/2017
//=======================================================================================
struct UIUtils
{
    UNITS_EXPORT static bmap<SignOption, Utf8String> GetAvailableSignOption(SignOption& defaultVal);
    UNITS_EXPORT static bmap<PresentationType, Utf8String> GetAvailablePresentationTypes(PresentationType& defaultVal);
    UNITS_EXPORT static bmap<FormatTraits, Utf8String> GetAvailableTraits(FormatTraits& defaultVal);
};

//=======================================================================================
// @bsistruct                                              David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericFormatSpec
{
private:
    uint8_t m_explicitlyDefinedMinWidth:1;
    uint8_t m_explicitlyDefinedRoundFactor:1;
    uint8_t m_explicitlyDefinedShowSign:1;
    uint8_t m_explicitlyDefinedPrecision:1;
    uint8_t m_explicitlyDefinedDecimalSeparator:1;
    uint8_t m_explicitlyDefinedThousandsSeparator:1;
    uint8_t m_explicitlyDefinedUOMSeparator:1;
    uint8_t m_explicitlyDefinedStatSeparator:1;
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Scientific, Station
    ScientificType      m_scientificType;
    SignOption          m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParentheses
    FormatTraits        m_formatTraits;          // NoZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    uint32_t            m_stationSize;           // 10*stationSize = distance between stations
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
    Utf8String          m_uomSeparator;          // Default separator between the number and UOM.
    Utf8Char            m_statSeparator;         // Default separator between parts of the stopping format.
    uint32_t            m_minWidth;              // The minimum width of the field. It will be taken into account
                                                 // only if the overall length (width) of the text representing integer
                                                 // a number of or integer part of a real is shorter and needs to be augmented by
                                                 // insignificant zeroes. Blanks are not considered because aligning text
                                                 // with the boundaries of a virtual box is the responsibility of annotation layer.

    double EffectiveRoundFactor(double rnd) const { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }

    int TrimTrailingZeroes(Utf8P buf, int index) const;
    size_t InsertChar(Utf8P buf, size_t index, char c, int num) const;

    //! Rounds the provided double value to round factor if it is within the threshold. The threshold is defined by FormatConstant::IsNegligible.
    static double RoundDouble(double dval, double roundTo);

    double GetDecimalPrecisionFactor() const {return Utils::DecimalPrecisionFactor(m_decPrecision);}
    int GetDecimalPrecisionIndex() const {return Utils::DecimalPrecisionToInt(m_decPrecision);}

    // TODO: Attempt to remove these methods from the private API===============
    int FormatInt(int n, Utf8P bufOut, int bufLen) const;
    size_t FormatDouble(double dval, Utf8P buf, size_t bufLen) const;
    int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const;
    Utf8String FormatToString(int n, int minSize) const;
    bool IsInsertSeparator(bool confirm) const { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    //! The following method does not perform buffer related checks and does not use
    //! parenthesis for indicating negative numbers However it uses other ShowSign options
    //! the calling function.
    //! The main purpose of this methind is to form exponent value.
    UNITS_EXPORT int static FormatSimple(int n, Utf8P bufOut, int bufLen, bool showSign);
    // !TODO====================================================================

public:
    //! Creates a new valid NumericFormatSpec with a 
    UNITS_EXPORT NumericFormatSpec();
    NumericFormatSpec(NumericFormatSpecCR other) = default;
    ~NumericFormatSpec() = default;
    NumericFormatSpecR operator=(NumericFormatSpecCR other) = default;

    //! Update this with the values from the provided JSON.
    //! @return Success if this NumericFormatSpec is successfully updated. Otherwise, false.
    UNITS_EXPORT static bool FromJson(NumericFormatSpecR out, JsonValueCR jval);
    //! Serializes this to JSON. The JSON will only contain values which differ from their initial state, or have been explicitly set
    //! to the current state.
    //!
    //! @param[out] out the json value populated with this spec's fields
    //! @param[in] verbose Specifies whether to include default values.
    //! @return false on error (if the spec has a problem)
    UNITS_EXPORT bool ToJson(Json::Value& out, bool verbose) const;

    UNITS_EXPORT static NumericFormatSpecCR DefaultFormat();

    //! Returns true if the all formatting properties of *this and other are equivalent.
    UNITS_EXPORT bool IsIdentical(NumericFormatSpecCR other) const;

    UNITS_EXPORT bool ImbueLocale(Utf8CP localeName);

    //======================================
    // Data Member Setters/Getters
    //======================================
    void SetRoundingFactor(double roundingFactor) {m_explicitlyDefinedRoundFactor = true; m_roundFactor = roundingFactor;}
    double GetRoundingFactor() const {return m_roundFactor;}
    bool HasRoundingFactor() const {return m_explicitlyDefinedRoundFactor;}

    void SetPresentationType(PresentationType type) {m_presentationType = type;}
    PresentationType GetPresentationType() const { return m_presentationType; }
    bool IsDecimal() const { return PresentationType::Decimal == m_presentationType; }
    bool IsFractional() const { return PresentationType::Fractional == m_presentationType; }
    bool IsScientific() const {return PresentationType::Scientific == m_presentationType;}

    void SetSignOption(SignOption opt) {m_explicitlyDefinedShowSign = true; m_signOption = opt;}
    SignOption GetSignOption() const {return m_signOption;}
    bool HasSignOption() const {return m_explicitlyDefinedShowSign;}
    bool IsNoSign() const {return SignOption::NoSign == m_signOption;}
    bool IsOnlyNegative() const {return SignOption::OnlyNegative == m_signOption;}
    bool IsSignAlways() const {return SignOption::SignAlways == m_signOption;}
    bool IsNegativeParentheses() const {return SignOption::NegativeParentheses == m_signOption;}

    UNITS_EXPORT Utf8String GetFormatTraitsString() const;
    void SetFormatTraits(FormatTraits traits) { m_formatTraits = traits; }
    UNITS_EXPORT bool SetFormatTraits(Utf8CP input);
    UNITS_EXPORT bool SetFormatTraits(JsonValueCR jval);
    UNITS_EXPORT Json::Value FormatTraitsToJson() const;

    FormatTraits GetFormatTraits() const { return m_formatTraits; }
    bool HasFormatTraits() const {return m_formatTraits != FormatTraits::None;}

    void SetStationOffsetSize(uint32_t size) {m_stationSize = size;}
    uint32_t GetStationOffsetSize() const {return m_stationSize;}

    void SetScientificType(ScientificType type) {m_scientificType = type;}
    ScientificType GetScientificType() const {return m_scientificType;}

    void SetPrecision(FractionalPrecision precision) {m_explicitlyDefinedPrecision = true; m_fractPrecision = precision; }
    void SetPrecision(DecimalPrecision precision) {m_explicitlyDefinedPrecision = true; m_decPrecision = precision;}
    DecimalPrecision GetDecimalPrecision() const {return m_decPrecision;}
    FractionalPrecision GetFractionalPrecision() const {return m_fractPrecision;}
    bool IsPrecisionZero() const {return m_decPrecision == DecimalPrecision::Precision0;}
    bool HasPrecision() const {return m_explicitlyDefinedPrecision;}

    void SetDecimalSeparator(Utf8Char sep) {m_explicitlyDefinedDecimalSeparator = true; m_decimalSeparator = sep;}
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
    bool HasDecimalSeparator() const {return m_explicitlyDefinedDecimalSeparator;}

    void SetThousandSeparator(char sep) {m_explicitlyDefinedThousandsSeparator = true; m_thousandsSeparator = sep;}
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    bool HasThousandsSeparator() const {return m_explicitlyDefinedThousandsSeparator;}

    void SetUomSeparator(Utf8CP sep) {m_explicitlyDefinedUOMSeparator = true; m_uomSeparator = Utf8String(sep);}
    Utf8CP GetUomSeparator(Utf8CP def = nullptr) const { return (nullptr == def)?  m_uomSeparator.c_str() : def; }
    bool HasUomSeparator() const {return m_explicitlyDefinedUOMSeparator;}

    void SetMinWidth(uint32_t wid) {m_explicitlyDefinedMinWidth = true; m_minWidth = wid;}
    uint32_t GetMinWidth() const {return m_minWidth;}
    bool HasMinWidth() const {return m_explicitlyDefinedMinWidth;}

    void SetStationSeparator(Utf8Char sep) {m_explicitlyDefinedStatSeparator = true; m_statSeparator = sep;}
    Utf8Char GetStationSeparator() const {return m_statSeparator;}
    bool HasStationSeparator() const {return m_explicitlyDefinedStatSeparator;}

    //======================================
    // Format Traits Bit Setters/Getters
    //======================================
    //! Given a starting traits can set/unset any of the format traits using the provided new traits.
    //! @param[in] startTraits The traits to update.
    //! @param[in] newTraits The traits to use to update the starting traits.
    //! @param[in] setTo Used to determine whether to set/unset the the startTraits with the newTraits.
    //! @return The FormatTraits that result from applying the new traits, with whether to set/unset, to the starting traits.
    UNITS_EXPORT static FormatTraits SetTraitsBit(FormatTraits startTraits, FormatTraits newTraits, bool setTo);

    //! Determine whether the supplied FormatTraits are set in the on this NumericFormatSpec.
    //! @param[in] traits The traits to compare against.
    //! @param[in] compareTraits The traits to check if they exist in the other set.
    //! @return True if all the format traits are from the compare traits exist. Otherwise, false.
    static bool AreTraitsSet(FormatTraits traits, FormatTraits compareTraits) {return 0 != (static_cast<std::underlying_type<FormatTraits>::type>(traits) & static_cast<std::underlying_type<FormatTraits>::type>(compareTraits));}

    //! Sets the provided traits as either off or on based on the supplied boolean.
    //! @param[in] traits The traits to set/unset on this NumericFormatSpec.
    //! @param[in] setTo Used to determine whether to set/unset the provided traits.
    void SetTraitsBit(FormatTraits traits, bool setTo) {m_formatTraits = NumericFormatSpec::SetTraitsBit(m_formatTraits, traits, setTo);}

    //! Determine whether the supplied FormatTraits are set on this NumericFormatSpec.
    //! @param[in] traitBit The traits to check if they are set.
    //! @return True if all the supplied FormatTraits are set. Otherwise, false.
    bool GetTraitBit(FormatTraits traitBit) const {return NumericFormatSpec::AreTraitsSet(m_formatTraits, traitBit);}

    void SetKeepTrailingZeroes(bool setTo) {SetTraitsBit(FormatTraits::TrailingZeroes, setTo);}
    bool IsKeepTrailingZeroes() const { return GetTraitBit(FormatTraits::TrailingZeroes);}

    void SetKeepDecimalPoint(bool setTo) { SetTraitsBit(FormatTraits::KeepDecimalPoint, setTo);}
    bool IsKeepDecimalPoint() const { return GetTraitBit(FormatTraits::KeepDecimalPoint); }

    void SetKeepSingleZero(bool setTo) { SetTraitsBit(FormatTraits::KeepSingleZero, setTo);}
    bool IsKeepSingleZero() const { return GetTraitBit(FormatTraits::KeepSingleZero); }

    void SetExponentOnlyNegative(bool setTo) { SetTraitsBit(FormatTraits::ExponenentOnlyNegative, setTo); }
    bool IsExponenentOnlyNegative() const { return GetTraitBit(FormatTraits::ExponenentOnlyNegative); }

    void SetZeroEmpty(bool setTo) { SetTraitsBit(FormatTraits::ZeroEmpty, setTo); }
    bool IsZeroEmpty() const { return GetTraitBit(FormatTraits::ZeroEmpty); }

    void SetUse1000Separator(bool setTo) { SetTraitsBit(FormatTraits::Use1000Separator, setTo); }
    bool IsUse1000Separator() const { return GetTraitBit(FormatTraits::Use1000Separator); }

    void SetApplyRounding(bool setTo) { SetTraitsBit(FormatTraits::ApplyRounding, setTo); }
    bool IsApplyRounding() const { return GetTraitBit(FormatTraits::ApplyRounding); }

    void SetFractionDash(bool setTo) { SetTraitsBit(FormatTraits::FractionDash, setTo); }
    bool IsFractionDash() const { return GetTraitBit(FormatTraits::FractionDash); }

    void SetShowUnitLabel(bool setTo) { SetTraitsBit(FormatTraits::ShowUnitLabel, setTo); }
    bool IsShowUnitLabel() const { return GetTraitBit(FormatTraits::ShowUnitLabel); }

    void SetPrependUnitLabel(bool setTo) { SetTraitsBit(FormatTraits::PrependUnitLabel, setTo); }
    bool IsPrependUnitLabel() const { return GetTraitBit(FormatTraits::PrependUnitLabel); }

    //======================================
    // Formatting Methods
    //======================================
    //! Format an integer using this NumericFormatSpec's format settings.
    //! @param[in] ival Integer to format.
    //! @return ival as a formatted string.
    UNITS_EXPORT Utf8String Format(int32_t ival) const;
    //! Format a double using this NumericFormatSpec's format settings.
    //! @param[in] dval Double to format.
    //! @return dval as a formatted string.
    UNITS_EXPORT Utf8String Format(double dval) const;
};

//=======================================================================================
//! We recognize combined numbers (combo-numbers) that represent some quantity as a sum of
//! subquantities expressed in lesser UOM's. For example, a given length could be represented as a
//! sum of M + Y + F + I where M is some number of miles, Y is some number of yards, F is some
//! number of feet and I is some number of inches. The operation of expressing a given length in
//! this form will require 3 ratios: M/Y, Y/F, and F/I where Y/M is the number of yards per mile,
//! F/Y is the number of feet per yard, I/F is a number of inches per foot.
//! Obviously, the combo-presentation of length is just one example and in some special cases. This
//! system of ratios can be anything the application needs. Ratios could be set explicitly or
//! automatically via names of the UOM's. The only condition is that all integer ratios must be > 1.
//! Let's define terms for UOM of different levels:
//!      0 - major UOM
//!      1 - middle UOM
//!      2 - minor UOM
//!      3 - sub UOM
//! Accordingly there are 3 ratios: major/middle, middle/minor, and minor/sub. Their indexes
//! in the ratio array are associated with the upper UOM.
//! @bsiclass                                                David.Fox-Rabinovitz  01/2017
//=======================================================================================
struct CompositeValueSpec
{
friend struct Format;
private:
    DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxy)
    struct UnitProxy
    {
    private:
        BEU::UnitCP m_unit;
        Utf8String m_unitLabel;
        bool m_explicitlyDefinedLabel;
    public:
        UnitProxy() : m_explicitlyDefinedLabel(false), m_unit(nullptr) {}
        UnitProxy(BEU::UnitCP unit, Utf8CP label = nullptr) 
            : m_unit(unit), m_explicitlyDefinedLabel(nullptr != label), m_unitLabel(label) {}
        UnitProxy(UnitProxyCR other)
            {
            if (nullptr != other.m_unit)
                m_unit = other.m_unit;
            m_explicitlyDefinedLabel = other.m_explicitlyDefinedLabel;
            m_unitLabel = other.m_unitLabel.c_str();
            }

        bool FromJson(Json::Value const& jval, BEU::IUnitsContextCP context);
        bool SetUnit(BEU::UnitCP unit) {m_unit = unit; return true;}
        Utf8StringCR GetLabel() const { return m_unitLabel; }
        bool HasLabel() const {return m_explicitlyDefinedLabel;}
        void SetLabel(Utf8CP lab) {if (nullptr == lab) return; m_explicitlyDefinedLabel = true; m_unitLabel = lab;}

        //! Returns the name of Unit in this if one is available
        Utf8CP GetName() const {if (nullptr == m_unit) return nullptr; return m_unit->GetName().c_str();}
        BEU::UnitCP GetUnit() const {return m_unit;}
        bool ToJson(Json::Value& jval, bool verbose = false) const;
        bool IsIdentical(UnitProxyCR other) const {return BEU::Unit::AreEqual(m_unit, other.m_unit) && m_unitLabel.Equals(other.m_unitLabel);}
        bool IsEmpty() const {return nullptr == m_unit;}
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValue)
    struct CompositeValue
    {
    private:
        bool m_negative;
        double m_parts[4];
        FormatProblemDetail m_problem;

    public:
        CompositeValue() : m_negative(false) {memset(m_parts, 0, sizeof(m_parts));}

        void SetNegative() {m_negative = true;}
        void SetPositive() {m_negative = false;}

        Utf8String GetSignPrefix(bool useParenth = false) const { return m_negative?  (useParenth ? "(" : "-") : ""; }
        Utf8String GetSignSuffix(bool useParenth = false) const { return m_negative ? (useParenth ? ")" : "") : ""; }

        double SetMajor(double dval)  {return m_parts[CompositeValueSpec::indxMajor] = dval;}
        double SetMiddle(double dval) {return m_parts[CompositeValueSpec::indxMiddle] = dval;}
        double SetMinor(double dval)  {return m_parts[CompositeValueSpec::indxMinor] = dval;}
        double SetSub(double dval)    {return m_parts[CompositeValueSpec::indxSub] = dval;}

        double GetMajor()  const {return m_parts[CompositeValueSpec::indxMajor];}
        double GetMiddle() const {return m_parts[CompositeValueSpec::indxMiddle];}
        double GetMinor()  const {return m_parts[CompositeValueSpec::indxMinor];}
        double GetSub()    const {return m_parts[CompositeValueSpec::indxSub];}

        bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
        bool IsProblem() const {return m_problem.IsProblem();}
    };

    static size_t const indxMajor  = 0;
    static size_t const indxMiddle = 1;
    static size_t const indxMinor  = 2;
    static size_t const indxSub    = 3;

    double m_ratio[indxSub] = {0};
    bool m_includeZero = true; // TODO: Not currently used in the formatting code, needs to be fixed.
    bool m_explicitlyDefinedSpacer = false;
    Utf8String m_spacer = FormatConstant::DefaultSpacer();
    FormatProblemDetail m_problem;
    bvector<UnitProxy> mutable m_proxys;

    //! Returns the unit ratio of upper/lower.
    //! Lower may be set to nullptr, indicating the lower unit is not set on the CVS.
    //! This function will set an error FormatProblemCode if:
    //!     1. Units do not belong to the same Phenomenon
    //!     2. The unit ratios major/middle, middle/minor, and minor/sub are not positive numbers.
    //!     3. Upper is a null pointer (invalid CVS).
    double CalculateUnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    //! Calculate/set all unit ratios within this CVS.
    void CalculateUnitRatios();

    UNITS_EXPORT Utf8CP GetUnitName(size_t indx, Utf8CP substitute = nullptr) const;
    UNITS_EXPORT Utf8CP GetUnitLabel(size_t index, Utf8CP substitute = nullptr) const;
    BEU::UnitCP GetSmallestUnit() const;

    UnitProxyP GetProxyP(size_t indx) const {return IsIndexValid(indx) ? &m_proxys[indx] : nullptr;}
    UnitProxyCP GetProxy(size_t indx) const {return GetProxyP(indx);}
    BEU::UnitCP GetUnit(size_t indx) const
        {
        UnitProxyCP proxy = GetProxy(indx);
        if (nullptr == proxy)
            return nullptr;
        return proxy->GetUnit();
        }
    bool IsIndexValid(size_t indx) const { return indx < m_proxys.size(); }

    UNITS_EXPORT void SetUnitLabel(size_t index, Utf8CP label);
    UNITS_EXPORT CompositeValueSpec(bvector<BEU::UnitCP> const& units);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit, BEU::UnitCP minorUnit, BEU::UnitCP subUnit);
    CompositeValue DecomposeValue(double value, BEU::UnitCP uom = nullptr) const;
public:
    UNITS_EXPORT CompositeValueSpec();
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit, BEU::UnitCR subUnit);
    UNITS_EXPORT CompositeValueSpec(CompositeValueSpecCR other);
    UNITS_EXPORT bool ToJson(Json::Value& out, bool verbose = false, bool excludeUnits = false) const;
    UNITS_EXPORT static bool FromJson(CompositeValueSpecR out, JsonValueCR jval, BEU::IUnitsContextCP context);
    UNITS_EXPORT static bool FromJson(CompositeValueSpecR out, JsonValueCR jsonWithoutUnits, bvector<BEU::UnitCP> const& units, bvector<Utf8String> const& unitLabels);

    UNITS_EXPORT bool IsIdentical(CompositeValueSpecCR other) const;

    size_t GetUnitCount() const {return m_proxys.size();}

    BEU::UnitCP GetMajorUnit()  const {return GetUnit(indxMajor);}
    BEU::UnitCP GetMiddleUnit() const {return GetUnit(indxMiddle);}
    BEU::UnitCP GetMinorUnit()  const {return GetUnit(indxMinor);}
    BEU::UnitCP GetSubUnit()    const {return GetUnit(indxSub);}
    
    UNITS_EXPORT const bvector<BEU::UnitCP> GetUnits() const; 
    
    bool HasMajorUnit()  const {return nullptr != GetUnit(indxMajor);} //!< Determine whether this composite value has a major unit.
    bool HasMiddleUnit() const {return nullptr != GetUnit(indxMiddle);} //!< Determine whether this composite value has a middle unit.
    bool HasMinorUnit()  const {return nullptr != GetUnit(indxMinor);} //!< Determine whether this composite value has a minor unit.
    bool HasSubUnit()    const {return nullptr != GetUnit(indxSub);} //!< Determine whether this composite value has a sub unit.

    UNITS_EXPORT void SetUnitLabels(Utf8CP majorLabel, Utf8CP middleLabel = nullptr, Utf8CP minorLabel = nullptr, Utf8CP subLabel = nullptr);
    UNITS_EXPORT Utf8String GetMajorLabel() const;
    UNITS_EXPORT Utf8String GetMiddleLabel() const;
    UNITS_EXPORT Utf8String GetMinorLabel() const;
    UNITS_EXPORT Utf8String GetSubLabel() const;

    void SetMajorLabel(Utf8StringCR label) {SetUnitLabel(indxMajor, label.c_str());}
    void SetMiddleLabel(Utf8StringCR label) {SetUnitLabel(indxMiddle, label.c_str());}
    void SetMinorLabel(Utf8StringCR label) {SetUnitLabel(indxMinor, label.c_str());}
    void SetSubLabel(Utf8StringCR label) {SetUnitLabel(indxSub, label.c_str());}

    bool HasMajorLabel()    const {return HasMajorUnit() && GetProxy(indxMajor)->HasLabel();} //!< Determine whether this composite value has a major unit label override.
    bool HasMiddleLabel()   const {return HasMiddleUnit() && GetProxy(indxMiddle)->HasLabel();} //!< Determine whether this composite value has a middle unit label override.
    bool HasMinorLabel()    const {return HasMinorUnit() && GetProxy(indxMinor)->HasLabel();} //!< Determine whether this composite value has a minor unit label override.
    bool HasSubLabel()      const {return HasSubUnit() && GetProxy(indxSub)->HasLabel();} //!< Determine whether this composite value has a sub unit label override.

    double GetMajorToMiddleRatio() const {return m_ratio[indxMajor];}
    double GetMiddleToMinorRatio() const {return m_ratio[indxMiddle];}
    double GetMinorToSubRatio()    const {return m_ratio[indxMinor];}

    bool IsProblem() const {return m_problem.IsProblem();}
    Utf8String GetProblemDescription() const {return m_problem.GetProblemDescription();}

    //! Set the string that will be used as a spacer in between each segment value and its uom label of the composite value string.
    Utf8String SetSpacer(Utf8CP spacer) {m_explicitlyDefinedSpacer = true; return m_spacer = spacer;}
    Utf8String GetSpacer() const {return m_spacer;} //!< Get the spacer used in between each segment value and its uom label of a composite value string.
    bool HasSpacer() const {return m_explicitlyDefinedSpacer;} //!< Returns whether a spacer has been explicitly set.

    //! Sets whether a segment of the composite value will be serialized to the resulting string if it evaluates to zero.
    bool SetIncludeZero(bool incl) {return m_includeZero = incl;}
    //! Determine whether a segment of the composite value will be serialized to the resulting string if it evaluates to zero.
    bool IsIncludeZero() const {return m_includeZero;}

    //! Given a vector of 0-4 units, create a composite spec from them. They must be non-null
    //! @param[out] out Output CompositeValueSpec
    //! @param[in]  units Vector of 0-4 non-null units to be added to the composite
    //! @return true on success false on error (too many units, null units, etc).
    UNITS_EXPORT static bool CreateCompositeSpec(CompositeValueSpecR out, bvector<BEU::UnitCP> const& units);
};


//=======================================================================================
//! Container for keeping together numeric and composite spec types.
//! A valid NumericFormatSpec is required for creating a valid instance of this class.
//! A CompositeValuespec are optional at the moment of creation but can be added at
//! any time.
//!
//! @bsistruct                                          David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct Format
{
private:
    bool                m_explicitlyDefinedComposite;
    FormatSpecType      m_specType;
    NumericFormatSpec   m_numericSpec;
    CompositeValueSpec  m_compositeSpec;
    FormatProblemDetail m_problem;

    UNITS_EXPORT virtual bool _ToJson(Json::Value& out, bool verbose) const;

public:

    Format() : m_specType(FormatSpecType::None), m_explicitlyDefinedComposite(false), m_problem(FormatProblemCode::NotInitialized) {};
    virtual ~Format(){}
    UNITS_EXPORT Format(FormatCR other);
    UNITS_EXPORT Format(NumericFormatSpecCR numSpec);
    UNITS_EXPORT Format(NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec);

    FormatR operator=(FormatCR other) = default;

    //! Returns true if the name, NumericFormatSpec, CompositeValueSpec, and problem codes of *this
    //! and other are identical.
    UNITS_EXPORT bool IsIdentical(FormatCR other) const;
    //! Creates a Format from a Json string
    //! @param[out] out         Output Format
    //! @param[in]  jsonString  Json string representing a format
    //! @param[in]  context     Context to resolve units in the composite spec of the format if there is one
    //! @return                 False if jsonString is empty or there are issues parsing/looking up units
    UNITS_EXPORT static bool FromJson(FormatR out, Utf8CP jsonString, BEU::IUnitsContextCP context = nullptr);
    //! Creates a Format from a Json object
    //! @param[out] out         Output Format
    //! @param[in]  jval        Json objects representing a format
    //! @param[in]  context     Context to resolve units in the composite spec of the format if there is one
    //! @return                 False if jval is empty or there are issues looking up units or the json is not valid.
    UNITS_EXPORT static bool FromJson(FormatR out, Json::Value jval, BEU::IUnitsContextCP context = nullptr);
    //! Creates a Json::Value representing this.
    virtual bool ToJson(Json::Value& out, bool verbose) const {return _ToJson(out, verbose);}

    FormatSpecType GetSpecType() const { return m_specType; }

    //! Returns true if this Format is explicitly defined as a CompositeFormat.
    bool HasExplicitlyDefinedComposite() const { return m_explicitlyDefinedComposite; }

    //! Returns true if this Format contains a NumericFormatSpec.
    bool HasNumeric() const {return !IsProblem() || (m_problem.GetProblemCode() != FormatProblemCode::NotInitialized);}
    //! Returns true if this Format contains a CompositeFormatSpec.
    //! A Format that contains a CompositeValueSpec will also contain a NumericFormatSpec.
    bool HasComposite() const {return m_explicitlyDefinedComposite || static_cast<std::underlying_type<FormatSpecType>::type>(m_specType) > 0;}
    //! Returns true if the spec has no problems and is set successfully. False otherwise
    UNITS_EXPORT bool SetCompositeSpec(CompositeValueSpec spec);
    //! Returns true if the spec is set successfully.
    UNITS_EXPORT bool SetNumericSpec(NumericFormatSpec spec);
    //! Returns a const pointer to this Format's NumericFormatSpec if it exists.
    //! Returns nullptr if no NumericFormatSpec is defined.
    NumericFormatSpecCP GetNumericSpec() const { return HasNumeric() ? &m_numericSpec : nullptr; }
    //! Returns a pointer to this Format's NumericFormatSpec if it exists.
    //! Returns nullptr if no NumericFormatSpec is defined.
    NumericFormatSpecP GetNumericSpecP() { return HasNumeric() ? &m_numericSpec : nullptr; }
    //! Returns a const pointer to this Format's CompositeValueSpec if it exists.
    //! Returns nullptr if no CompositeValueSpec is defined.
    CompositeValueSpecCP GetCompositeSpec() const { return HasComposite() ? &m_compositeSpec : nullptr; }
    //! Returns a pointer to this Format's CompositeValueSpec if it exists.
    //! Returns nullptr if no CompositeValueSpec is defined.
    CompositeValueSpecP GetCompositeSpecP() { return HasComposite() ? &m_compositeSpec : nullptr; }
    //! Returns the number of units defined on this Format's CompositeValueSpec.
    //! Returns 0 if the CompositeValueSpec has no units OR in the case that no CompositeValueSpec
    //! is defined on this NumericFormatSpec.
    size_t GetCompositeUnitCount() const { return HasComposite() ? m_compositeSpec.GetUnitCount() : 0; }
    //! Returns a const pointer to the major unit of this Format's CompositeValueSpec.
    //! Returns nullptr if no CompositeValueSpec is defined.
    BEU::UnitCP GetCompositeMajorUnit() const { return HasComposite() ? m_compositeSpec.GetMajorUnit() : nullptr; }
    //! Returns a const pointer to the middle unit of this Format's CompositeValueSpec.
    //! Returns nullptr if no CompositeValueSpec is defined.
    BEU::UnitCP GetCompositeMiddleUnit() const { return HasComposite() ? m_compositeSpec.GetMiddleUnit() : nullptr; }
    //! Returns a const pointer to the minor unit of this Format's CompositeValueSpec.
    //! Returns nullptr if no CompositeValueSpec is defined.
    BEU::UnitCP GetCompositeMinorUnit() const { return HasComposite() ? m_compositeSpec.GetMinorUnit() : nullptr; }
    //! Returns a const pointer to the sub unit of this Format's CompositeValueSpec.
    //! Returns nullptr if no CompositeValueSpec is defined.
    BEU::UnitCP GetCompositeSubUnit() const { return HasComposite() ? m_compositeSpec.GetSubUnit() : nullptr; }
    bool HasCompositeMajorUnit() const {return nullptr != GetCompositeMajorUnit();}
    bool HasCompositeMiddleUnit() const {return nullptr != GetCompositeMiddleUnit();}
    bool HasCompositeMinorUnit() const {return nullptr != GetCompositeMinorUnit();}
    bool HasCompositeSubUnit() const {return nullptr != GetCompositeSubUnit();}
    bool HasCompositeSpacer() const {return GetCompositeSpec()->HasSpacer();}

    void SetSuppressUnitLabel() { m_numericSpec.SetShowUnitLabel(false); }

    bool IsProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblem() const {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() const {return m_problem.GetProblemDescription();}
    PresentationType GetPresentationType() const { return m_numericSpec.GetPresentationType(); }

    Utf8String FormatQuantity(BEU::QuantityCR qty, Utf8StringCR space) const {return FormatQuantity(qty.ConvertTo(m_compositeSpec.GetMajorUnit()), nullptr, space.c_str());}
    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit = nullptr, Utf8CP space = nullptr, Utf8CP useLabel = nullptr) const;

    //! Parse a Format from the provided format string. A format string takes the form,
    //! <code>
    //! FORMAT_NAME(PRECISION_OVERRIDE)[INPUT_UNIT_NAME|UNIT_LABEL]
    //! </code>
    //! FORMAT_NAME is a defined named format that will be located using the defaultFormatMapper parameter.
    //! PRECISION_OVERRIDE is a decimal/fractional precision override for the Format.
    //! INPUT_UNIT_NAME is a defined BEU::Unit that overrides the input unit defined in the Format
    //! UNIT_LABEL is a string that overrides the unit label defined on the override input unit.
    //! Examples:
    //! <code>
    //! "Real(2)"
    //! "Fractional(64)"
    //! "Scientific(12)"
    //! "Real(2)[u:M|m]"
    //! </code>
    //! @param[out] nfs                 Format to be populated with the settings parsed from the format string.
    //! @param[in]  formatString        String to be parsed.
    //! @param[in]  defaultFormatMapper Functor that maps a Format name to a defined Format. The functor should return a pointer to some Format if any such mapping
    //!                                     exists or nullptr if no such mapping exists.
    //! @param[in]  unitContext         Units contex to be used to resolve 
    //! @returns BentleyStatus::SUCCESS if the string was successfully parsed.
    UNITS_EXPORT static BentleyStatus ParseFormatString(FormatR nfs, Utf8StringCR formatString, std::function<FormatCP(Utf8StringCR)> defaultFormatMapper, BEU::IUnitsContextCP unitContext = nullptr);

    UNITS_EXPORT static BentleyStatus ParseFormatString(Utf8StringR formatName, Nullable<int32_t>& precision, bvector<Utf8String>& unitNames, bvector<Nullable<Utf8String>>& labels, Utf8StringCR formatString);

    // Legacy Descriptor string
    UNITS_EXPORT static void ParseUnitFormatDescriptor(Utf8StringR unitName, Utf8StringR formatString, Utf8CP description);
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct QuantityFormatting
{
    typedef std::function<BEU::UnitCP(Utf8CP, BEU::PhenomenonCP)> UnitResolver;
    UNITS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, double* persist, BEU::UnitCP outputUnit, FormatCR inputFUS, FormatProblemCode* problemCode, UnitResolver* resolver = nullptr);
    UNITS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, FormatCR inputFUS, FormatProblemCode* problemCode, UnitResolver* resolver = nullptr);
};

END_BENTLEY_FORMATTING_NAMESPACE
