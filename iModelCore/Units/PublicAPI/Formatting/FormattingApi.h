/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingApi.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Units/Units.h>
#include <Formatting/FormattingDefinitions.h>
#include <Formatting/FormattingEnum.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormatSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValue)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValueSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Format)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxy)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UIListEntry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LocaleProperties)

// Json presentation
BE_JSON_NAME(roundFactor)
BE_JSON_NAME(presentType)
BE_JSON_NAME(signOpt)
BE_JSON_NAME(formatTraits)
BE_JSON_NAME(trailZeroes)
BE_JSON_NAME(leadZeroes)
BE_JSON_NAME(keepSingleZero)
BE_JSON_NAME(zeroEmpty)
BE_JSON_NAME(keepDecimalPrecision)
BE_JSON_NAME(applyRounding)
BE_JSON_NAME(fractionDash)
BE_JSON_NAME(showUnitLabel)
BE_JSON_NAME(prependUnitLabel)
BE_JSON_NAME(use1000Separator)
BE_JSON_NAME(exponentOnlyNegative)
//BE_JSON_NAME(UseLocale)
BE_JSON_NAME(decPrec)
BE_JSON_NAME(fractPrec)
BE_JSON_NAME(barType)
BE_JSON_NAME(decimalSeparator)
BE_JSON_NAME(thousandSeparator)
BE_JSON_NAME(uomSeparator)
BE_JSON_NAME(statSeparator)
BE_JSON_NAME(minWidth)

// Format
BE_JSON_NAME(SpecName)
BE_JSON_NAME(SpecDescript)
BE_JSON_NAME(SpecLabel)
BE_JSON_NAME(SpecType)
BE_JSON_NAME(CompositeFormat)
BE_JSON_NAME(NumericFormat)

//UnitProxy & FUS
BE_JSON_NAME(unitName)
BE_JSON_NAME(unitLabel)
BE_JSON_NAME(formatName)
BE_JSON_NAME(fusName)
BE_JSON_NAME(formatSpec)
BE_JSON_NAME(cloneData)
BE_JSON_NAME(synonym)

//CompositeValueSpec
BE_JSON_NAME(MajorUnit)
BE_JSON_NAME(MiddleUnit)
BE_JSON_NAME(MinorUnit)
BE_JSON_NAME(SubUnit)
BE_JSON_NAME(InputUnit)
BE_JSON_NAME(includeZero)
BE_JSON_NAME(spacer)

//=======================================================================================
//
// Class used to pass available values and their localized labels for use in UI.
//
// @bsistruct                                            Bill.Steinbock  11/2017
//=======================================================================================
struct UIListEntry
{
protected:
    Json::Value m_json;

public:
    UIListEntry() : m_json(Json::objectValue) {}
    UIListEntry(Json::Value const& j) : m_json(j) {}
    UIListEntry& operator=(UIListEntry const& rhs) { m_json = rhs.m_json; return *this; }

    void SetLabel(Utf8CP n) { m_json["label"] = n; }
    Utf8CP GetLabel() const { return m_json["label"].asCString(); }

    void SetStringValue(Utf8CP n) { m_json["stringValue"] = n; }
    Utf8CP GetStringValue() const { return m_json["stringValue"].asCString(); }

    void SetValue(int n) { m_json["value"] = n; }
    int GetValue() const { return m_json["value"].asInt(); }

    UIListEntry(int value, Utf8CP label, Utf8CP stringValue=nullptr) : m_json(Json::objectValue)
        {
        SetLabel(label);
        SetValue(value);

        // string value is optional
        if (stringValue)
            SetStringValue(stringValue);
        }

    Json::Value const& GetJson() const { return m_json; }
};

//=======================================================================================
//
// Class used to pass available values and their localized labels for use in UI.
//
// @bsistruct                                            Bill.Steinbock  11/2017
//=======================================================================================
struct UIList
{
protected:
    Json::Value m_json;

public:
    UIList() : m_json(Json::arrayValue) {}
    UIList(Json::Value const& j) : m_json(j) {}
    UIList& operator=(UIList const& rhs) { m_json = rhs.m_json; return *this; }

    Json::ArrayIndex GetSize() { return m_json.size(); }
    void AddListEntry(UIListEntryCR n) { m_json.append(n.GetJson()); }
    UIListEntry GetListEntry(Json::ArrayIndex i) { return UIListEntry(m_json[i]); }

    bool IsNull() const { return m_json.isNull(); }
    Json::Value const& GetJson() const { return m_json; }
};

//=======================================================================================
//
// Class containing static methods used to populate UI.
//
// @bsistruct                                            Bill.Steinbock  11/2017
//=======================================================================================
struct UIUtils
{
    UNITS_EXPORT static UIList GetAvailableDecimalPercisions();
    UNITS_EXPORT static UIList GetAvailableFractionalPercisions();
    UNITS_EXPORT static UIList GetAvailableSignOption();
    UNITS_EXPORT static UIList GetAvailablePresentationTypes();
    UNITS_EXPORT static UIList GetAvailableDecimalSeparators();
    UNITS_EXPORT static UIList GetAvailableThousandSeparators();
    UNITS_EXPORT static UIList GetAvailableUnitLabelSeparators();
    UNITS_EXPORT static UIList GetAvailableTraits();
};

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  02/2018
//=======================================================================================
struct LocaleProperties
{
private:
    Utf8Char m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
public:

    UNITS_EXPORT LocaleProperties(Json::Value jval);
    LocaleProperties(Utf8Char decimal, Utf8Char thousand) : m_decimalSeparator(decimal), m_thousandsSeparator(thousand) {}
    UNITS_EXPORT LocaleProperties(Utf8CP localeName = nullptr);
    static LocaleProperties DefaultAmerican() {return LocaleProperties('.',',');}
    static LocaleProperties DefaultEuropean(bool useBlank = false) {return LocaleProperties(',', (useBlank? ' ' : '.'));}

    Utf8Char SetDecimalSeparator(Utf8Char sep) {return m_decimalSeparator = sep;}
    Utf8Char GetDecimalSeparator() const {return m_decimalSeparator;}
    Utf8Char SetThousandSeparator(char sep) {return m_thousandsSeparator = sep;}
    Utf8Char GetThousandSeparator() const {return m_thousandsSeparator; }
    UNITS_EXPORT Json::Value ToJson();
};

//=======================================================================================
// @bsistruct                                              David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericFormatSpec
{
private:
    bool                m_explicitlyDefinedMinWidth;
    bool                m_explicitlyDefinedPrefixPadChar;
    bool                m_explicitlyDefinedRoundFactor;
    bool                m_explicitlyDefinedShowSign;
    bool                m_explicitlyDefinedPrecision;
    bool                m_explicitlyDefinedDecimalSeparator;
    bool                m_explicitlyDefinedThousandsSeparator;
    bool                m_explicitlyDefinedUOMSeparator;
    bool                m_explicitlyDefinedStatSeparator;
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Scientific, Station
    ScientificType      m_scientificType;
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParentheses
    FormatTraits        m_formatTraits;          // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    uint32_t            m_stationSize;           // 10*stationSize = distance between stations
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
    Utf8String          m_uomSeparator;          // Default separator between the number and UOM.
    Utf8Char            m_statSeparator;         // Default separator between parts of the stopping format.
    Utf8Char            m_prefixPadChar;         // Character to pad with to reach minimum width
    int                 m_minWidth;              // The minimum width of the field. It will be taken into account
                                                 // only if the overall length (width) of the text representing integer
                                                 // a number of or integer part of a real is shorter and needs to be augmented by
                                                 // insignificant zeroes. Blanks are not considered because aligning text
                                                 // with the boundaries of a virtual box is the responsibility of annotation layer.

    double EffectiveRoundFactor(double rnd) const { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }

    int TrimTrailingZeroes(Utf8P buf, int index) const;
    size_t InsertChar(Utf8P buf, size_t index, char c, int num) const;

    // TODO: Attempt to remove these methods from the private API===============
    int FormatInteger(int n, Utf8P bufOut, int bufLen);
    size_t FormatDoubleBuf(double dval, Utf8P buf, size_t bufLen, int prec = -1, double round = -1.0) const;
    static double RoundDouble(double dval, double roundTo);
    int GetDecimalPrecisionIndex(int prec = -1) const;
    double GetDecimalPrecisionFactor(int prec = -1) const {return Utils::DecimalPrecisionFactor(m_decPrecision, prec);}
    int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const;
    Utf8String FormatIntegerToString(int n, int minSize) const;
    bool IsInsertSeparator(bool confirm) const { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    // !TODO====================================================================

public:
    // TODO: Attempt to remove these methods from the public API================
    //! The following methind does not perform buffer related checks and does not use
    //! parenthesis for indicating negative numbers However it uses other ShowSign options
    //! the calling function.
    //! The main purpose of this methind is to form exponent value.
    UNITS_EXPORT int static FormatIntegerSimple(int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero);
    // !TODO====================================================================

    UNITS_EXPORT NumericFormatSpec();
    NumericFormatSpec(NumericFormatSpecCR other) = default;
    UNITS_EXPORT NumericFormatSpec(Json::Value jval);
    ~NumericFormatSpec() = default;

    NumericFormatSpecR operator=(NumericFormatSpecCR other) = default;

    //! Returns true if the all formatting properties of *this and other are equivalent.
    UNITS_EXPORT bool IsIdentical(NumericFormatSpecCR other) const;

    UNITS_EXPORT bool ImbueLocale(Utf8CP localeName);
    UNITS_EXPORT bool ImbueLocaleProperties(LocalePropertiesCR locProp);

    UNITS_EXPORT static NumericFormatSpecCR DefaultFormat();

    UNITS_EXPORT Json::Value ToJson(bool verbose) const;

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

    void SetSignOption(ShowSignOption opt) {m_explicitlyDefinedShowSign = true; m_signOption = opt;}
    ShowSignOption GetSignOption() const {return m_signOption;}
    bool HasSignOption() const {return m_explicitlyDefinedShowSign;}
    bool IsNoSign() const { return ShowSignOption::NoSign == m_signOption; }
    bool IsOnlyNegative() const { return ShowSignOption::OnlyNegative == m_signOption; }
    bool IsSignAlways() const { return ShowSignOption::SignAlways == m_signOption; }
    bool IsNegativeParentheses() const { return ShowSignOption::NegativeParentheses == m_signOption; }

    UNITS_EXPORT Utf8String GetFormatTraitsString() const;
    void SetFormatTraits(FormatTraits traits) { m_formatTraits = traits; }
    UNITS_EXPORT bool SetFormatTraitsFromString(Utf8StringCR input);
    FormatTraits GetFormatTraits() const { return m_formatTraits; }
    bool HasFormatTraits() const {return m_formatTraits != FormatTraits::None;}

    void SetStationOffsetSize(uint32_t size) {m_stationSize = size;}
    uint32_t GetStationOffsetSize() const {return m_stationSize;}

    void SetScientificType(ScientificType type) {m_scientificType = type;}
    ScientificType GetScientificType() const {return m_scientificType;}

    void SetDecimalPrecision(DecimalPrecision precision) {m_explicitlyDefinedPrecision = true; m_decPrecision = precision;}
    DecimalPrecision GetDecimalPrecision() const { return m_decPrecision; }
    bool IsPrecisionZero() const { return m_decPrecision == DecimalPrecision::Precision0; }

    void SetFractionalPrecision(FractionalPrecision precision) {m_explicitlyDefinedPrecision = true; m_fractPrecision = precision;}
    FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }
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

    void SetMinWidth(int wid) {m_explicitlyDefinedMinWidth = true; m_minWidth = wid;}
    int GetMinWidth() const {return m_minWidth;}
    bool HasMinWidth() const {return m_explicitlyDefinedMinWidth;}

    void SetStationSeparator(Utf8Char sep) {m_explicitlyDefinedStatSeparator = true; m_statSeparator = sep;}
    Utf8Char GetStationSeparator() const {return m_statSeparator;}
    bool HasStationSeparator() const {return m_explicitlyDefinedStatSeparator;}

    void SetPrefixPadChar(Utf8Char pad) {m_explicitlyDefinedPrefixPadChar = true; m_prefixPadChar = pad;}
    Utf8Char GetPrefixPadChar() const {return m_prefixPadChar;}
    bool HasPrefixPadChar() const {return m_explicitlyDefinedPrefixPadChar;}

    //======================================
    // Format Traits Bit Setters/Getters
    //======================================
    UNITS_EXPORT static FormatTraits SetTraitsBit(FormatTraits traits, FormatTraits bit, bool setTo);
    UNITS_EXPORT static bool GetTraitsBit(FormatTraits traits, FormatTraits bit);
    void SetTraitsBit(FormatTraits bit, bool setTo) {m_formatTraits = NumericFormatSpec::SetTraitsBit(m_formatTraits, bit, setTo);}
    bool GetTraitsBit(FormatTraits bit) const {return NumericFormatSpec::GetTraitsBit(m_formatTraits, bit);}
    UNITS_EXPORT void TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose=false) const;
    
    UNITS_EXPORT void SetFormatTraitsFromJson(JsonValueCR jval);
    UNITS_EXPORT Json::Value FormatTraitsToJson(bool verbose) const;
    
    void SetExponentZero(bool setTo) {SetTraitsBit(FormatTraits::ExponentZero, setTo);}
    bool IsExponentZero() const {return GetTraitsBit(FormatTraits::ExponentZero);}

    void SetUseLeadingZeroes(bool setTo) {SetTraitsBit(FormatTraits::LeadingZeroes, setTo);}
    bool IsUseLeadingZeroes() const {return GetTraitsBit(FormatTraits::LeadingZeroes);}

    void SetKeepTrailingZeroes(bool setTo) {SetTraitsBit(FormatTraits::TrailingZeroes, setTo);}
    bool IsKeepTrailingZeroes() const { return GetTraitsBit(FormatTraits::TrailingZeroes);}

    void SetKeepDecimalPoint(bool setTo) { SetTraitsBit(FormatTraits::KeepDecimalPoint, setTo);}
    bool IsKeepDecimalPoint() const { return GetTraitsBit(FormatTraits::KeepDecimalPoint); }

    void SetKeepSingleZero(bool setTo) { SetTraitsBit(FormatTraits::KeepSingleZero, setTo);}
    bool IsKeepSingleZero() const { return GetTraitsBit(FormatTraits::KeepSingleZero); }

    void SetExponentOnlyNegative(bool setTo) { SetTraitsBit(FormatTraits::ExponenentOnlyNegative, setTo); }
    bool IsExponenentOnlyNegative() const { return GetTraitsBit(FormatTraits::ExponenentOnlyNegative); }

    void SetZeroEmpty(bool setTo) { SetTraitsBit(FormatTraits::ZeroEmpty, setTo); }
    bool IsZeroEmpty() const { return GetTraitsBit(FormatTraits::ZeroEmpty); }

    void SetUse1000Separator(bool setTo) { SetTraitsBit(FormatTraits::Use1000Separator, setTo); }
    bool IsUse1000Separator() const { return GetTraitsBit(FormatTraits::Use1000Separator); }

    void SetApplyRounding(bool setTo) { SetTraitsBit(FormatTraits::ApplyRounding, setTo); }
    bool IsApplyRounding() const { return GetTraitsBit(FormatTraits::ApplyRounding); }

    void SetFractionDash(bool setTo) { SetTraitsBit(FormatTraits::FractionDash, setTo); }
    bool IsFractionDash() const { return GetTraitsBit(FormatTraits::FractionDash); }

    void SetShowUnitLabel(bool setTo) { SetTraitsBit(FormatTraits::ShowUnitLabel, setTo); }
    bool IsShowUnitLabel() const { return GetTraitsBit(FormatTraits::ShowUnitLabel); }

    void SetPrependUnitLabel(bool setTo) { SetTraitsBit(FormatTraits::ShowUnitLabel, setTo); }
    bool IsPrependUnitLabel() const { return GetTraitsBit(FormatTraits::ShowUnitLabel); }
    //======================================
    // Formatting Methods
    //======================================
    //! Format an integer using this NumericFormatSpec's format settings.
    //! @param[in] ival Integer to format.
    //! @return ival as a formatted string.
    UNITS_EXPORT Utf8String FormatInteger(int32_t ival);
    //! Format a double using this NumericFormatSpec's format settings.
    //! @param[in] dval Double to format.
    //! @return dval as a formatted string.
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0) const;
    //! Format a double using the format settings of numericFormatSpec.
    //! @param[in] nfs  NumericFormatSpec used for formatting.
    //! @param[in] dval Double to format.
    //! @return dval as a formatted string.
    UNITS_EXPORT static Utf8String FormatDouble(NumericFormatSpecCR nfs, double dval, int prec = -1, double round = -1.0);
};

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  06/2017
//=======================================================================================
struct UnitProxy
{
private:
    BEU::UnitCP m_unit;
    Utf8String m_unitLabel;

public:
    UnitProxy() : m_unit(nullptr) {}
    UnitProxy(BEU::UnitCP unit, Utf8CP label = nullptr) : m_unit(unit), m_unitLabel(label) {}
    UnitProxy(UnitProxyCR other)
        {
        if (nullptr != other.m_unit)
            m_unit = other.m_unit;
        
        m_unitLabel = other.m_unitLabel.c_str();
        }
    void Copy(UnitProxyCP other)
        {
        if (nullptr == other)
            {
            m_unit = nullptr;
            return;
            }

        if (nullptr != other->m_unit)
            m_unit = other->m_unit;
        
        m_unitLabel = other->m_unitLabel.c_str();
        }

    UNITS_EXPORT void LoadJson(Json::Value jval, BEU::IUnitsContextCP context);
    bool SetUnit(BEU::UnitCP unit) {m_unit = unit; return true;}
    Utf8StringCR GetLabel() const { return m_unitLabel; }
    bool HasLabel() const {return !m_unitLabel.empty();}
    void SetLabel(Utf8CP lab) {if (nullptr == lab) return; m_unitLabel = lab;}

    //! Returns the name of Unit in this if one is available
    Utf8CP GetName() const {if (nullptr == m_unit) return nullptr; return m_unit->GetName().c_str();}
    BEU::UnitCP GetUnit() const {return m_unit;}
    UNITS_EXPORT Json::Value ToJson() const;
    bool IsEmpty() const {return nullptr == m_unit;}
    bool IsIdentical(UnitProxyCR other) const {return BEU::Unit::AreEqual(m_unit, other.m_unit) && m_unitLabel.Equals(other.m_unitLabel);}
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
friend struct CompositeValue;
private:
    static size_t const indxMajor  = 0;
    static size_t const indxMiddle = 1;
    static size_t const indxMinor  = 2;
    static size_t const indxSub    = 3;

    size_t m_ratio[indxSub];
    bool m_includeZero; // Not currently used in the formatting code.
    bool m_explicitlyDefinedSpacer;
    bool m_explicitlyDefinedMajorLabel;
    bool m_explicitlyDefinedMiddleLabel;
    bool m_explicitlyDefinedMinorLabel;
    bool m_explicitlyDefinedSubLabel;
    Utf8String m_spacer;
    Utf8String m_labelOverride;
    FormatProblemDetail m_problem;
    bvector<UnitProxy> mutable m_proxys;
    BEU::UnitCP m_inputUnit;

    //! Returns the unit ratio of upper/lower.
    //! Lower may be set to nullptr, indicating the lower unit is not set on the CVS.
    //! This function will set an error FormatProblemCode if:
    //!     1. Units do not belong to the same Phenomenon
    //!     2. The unit ratios major/middle, middle/minor, and minor/sub are not positive integers.
    //!     3. Upper is a null pointer (invalid CVS).
    size_t CalculateUnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    //! Calculate/set all unit ratios within this CVS.
    void CalculateUnitRatios();

    Utf8CP GetUnitName(size_t indx, Utf8CP substitute = nullptr) const;
    Utf8CP GetUnitLabel(size_t index, Utf8CP substitute = nullptr) const;
    UNITS_EXPORT Utf8String GetEffectiveLabel(size_t indx) const;
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

    UNITS_EXPORT CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit, BEU::UnitCP minorUnit, BEU::UnitCP subUnit);
    UNITS_EXPORT void SetUnitLabel(size_t index, Utf8CP label);
public:
    // TODO: Attempt to remove these methods from the public API================
    UNITS_EXPORT void LoadJsonData(JsonValueCR jval, BEU::IUnitsContextCP context);
    // !TODO====================================================================

    UNITS_EXPORT CompositeValueSpec();
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit, BEU::UnitCR subUnit);
    UNITS_EXPORT CompositeValueSpec(bvector<BEU::UnitCP> const& units);
    UNITS_EXPORT CompositeValueSpec(CompositeValueSpecCR other);

    UNITS_EXPORT bool IsIdentical(CompositeValueSpecCR other) const;

    size_t GetUnitCount() const {return m_proxys.size();}

    BEU::UnitCP GetMajorUnit()  const {return GetUnit(indxMajor);}
    BEU::UnitCP GetMiddleUnit() const {return GetUnit(indxMiddle);}
    BEU::UnitCP GetMinorUnit()  const {return GetUnit(indxMinor);}
    BEU::UnitCP GetSubUnit()    const {return GetUnit(indxSub);}
    BEU::UnitCP GetInputUnit()  const {return m_inputUnit;}
    UNITS_EXPORT void SetInputUnit(BEU::UnitCP unit);
    void SetInputUnitLabel(Utf8CP label) {m_labelOverride = label;}
    bool HasInputUnit()  const {return nullptr != m_inputUnit;}
    bool HasMajorUnit()  const {return nullptr != GetUnit(indxMajor);}
    bool HasMiddleUnit() const {return nullptr != GetUnit(indxMiddle);}
    bool HasMinorUnit()  const {return nullptr != GetUnit(indxMinor);}
    bool HasSubUnit()    const {return nullptr != GetUnit(indxSub);}

    UNITS_EXPORT void SetUnitLabels(Utf8CP majorLabel, Utf8CP middleLabel = nullptr, Utf8CP minorLabel = nullptr, Utf8CP subLabel = nullptr);
    Utf8String GetMajorLabel()  const {return GetEffectiveLabel(indxMajor);}
    Utf8String GetMiddleLabel() const {return GetEffectiveLabel(indxMiddle);}
    Utf8String GetMinorLabel()  const {return GetEffectiveLabel(indxMinor);}
    Utf8String GetSubLabel()    const {return GetEffectiveLabel(indxSub);}

    void SetMajorLabel(Utf8StringCR label) {m_explicitlyDefinedMajorLabel = true; SetUnitLabel(indxMajor, label.c_str());}
    void SetMiddleLabel(Utf8StringCR label) {m_explicitlyDefinedMiddleLabel = true; SetUnitLabel(indxMiddle, label.c_str());}
    void SetMinorLabel(Utf8StringCR label) {m_explicitlyDefinedMinorLabel = true; SetUnitLabel(indxMinor, label.c_str());}
    void SetSubLabel(Utf8StringCR label) {m_explicitlyDefinedSubLabel = true; SetUnitLabel(indxSub, label.c_str());}

    bool HasMajorLabel()    const {return m_explicitlyDefinedMajorLabel;}
    bool HasMiddleLabel()   const {return m_explicitlyDefinedMiddleLabel;}
    bool HasMinorLabel()    const {return m_explicitlyDefinedMinorLabel;}
    bool HasSubLabel()      const {return m_explicitlyDefinedSubLabel;}

    size_t GetMajorToMiddleRatio() const {return m_ratio[indxMajor];}
    size_t GetMiddleToMinorRatio() const {return m_ratio[indxMiddle];}
    size_t GetMinorToSubRatio()    const {return m_ratio[indxMinor];}

    bool IsProblem() const {return m_problem.IsProblem();}
    Utf8String GetProblemDescription() const {return m_problem.GetProblemDescription();}

    Utf8String SetSpacer(Utf8CP spacer) {m_explicitlyDefinedSpacer = true; return m_spacer = spacer;}
    Utf8String GetSpacer() const {return m_spacer;}
    bool HasSpacer() const {return m_explicitlyDefinedSpacer;}

    bool SetIncludeZero(bool incl) {return m_includeZero = incl;}
    bool IsIncludeZero() const {return m_includeZero;}

    //! If uom is not provided we assume that the value is defined in the smallest units defined
    //! in the current spec.
    UNITS_EXPORT CompositeValue DecomposeValue(double dval, BEU::UnitCP uom = nullptr);

    UNITS_EXPORT Json::Value ToJson() const;
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct CompositeValue
{
private:
    static size_t const indxInput = 4;
    bool m_negative;
    double m_parts[5];
    FormatProblemDetail m_problem;
    double GetSignFactor() { return m_negative ? -1.0 : 1.0; }

public:
    CompositeValue() : m_negative(false) {memset(m_parts, 0, sizeof(m_parts));}

    void SetNegative() { m_negative = true; }
    void SetPositive() { m_negative = false; }

    Utf8String GetSignPrefix(bool useParenth = false) const { return m_negative?  (useParenth ? "(" : "-") : ""; }
    Utf8String GetSignSuffix(bool useParenth = false) const { return m_negative ? (useParenth ? ")" : "") : ""; }

    double SetMajor(double dval)  { return m_parts[CompositeValueSpec::indxMajor] = dval; }
    double SetMiddle(double dval) { return m_parts[CompositeValueSpec::indxMiddle] = dval; }
    double SetMinor(double dval)  { return m_parts[CompositeValueSpec::indxMinor] = dval; }
    double SetSub(double dval)    { return m_parts[CompositeValueSpec::indxSub] = dval; }
    double SetInput(double dval)  { return m_parts[indxInput] = dval; }

    double GetMajor()  const { return m_parts[CompositeValueSpec::indxMajor]; }
    double GetMiddle() const { return m_parts[CompositeValueSpec::indxMiddle]; }
    double GetMinor()  const { return m_parts[CompositeValueSpec::indxMinor]; }
    double GetSub()    const { return m_parts[CompositeValueSpec::indxSub]; }
    double GetInput()  const { return m_parts[indxInput]; }

    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
    bool IsProblem() const { return m_problem.IsProblem(); }
};

//=======================================================================================
//! Container for keeping together primary, numeric, composite and other types of specs.
//! Name and a valid numeric spec are required for creating a valid instance of this class.
//! Alias and composite spec are optional at the moment of creation but can be added at
//! any time.
//!
//! @bsistruct                                          David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct Format
{
private:
    FormatSpecType      m_specType;
    NumericFormatSpec   m_numericSpec;
    CompositeValueSpec  m_compositeSpec;
    FormatProblemDetail m_problem;

public:
    // TODO: Attempt to remove these methods from the public API================
    UNITS_EXPORT void FromJson(Utf8CP jsonString, BEU::IUnitsContextCP context = nullptr);
    UNITS_EXPORT void FromJson(Json::Value jval, BEU::IUnitsContextCP context = nullptr);
    // !TODO====================================================================

    Format() : m_specType(FormatSpecType::None), m_problem(FormatProblemCode::NotInitialized) {};
    UNITS_EXPORT Format(FormatCR other);
    UNITS_EXPORT Format(NumericFormatSpecCR numSpec);
    UNITS_EXPORT Format(NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec);

    FormatR operator=(const Format& other) = default;

    //! Returns true if the name, NumericFormatSpec, CompositeValueSpec, and problem codes of *this
    //! and other are identical.
    UNITS_EXPORT bool IsIdentical(FormatCR other) const;

    //! Creates a Json::Value representing this.
    UNITS_EXPORT Json::Value ToJson(bool verbose) const;

    FormatSpecType GetSpecType() const { return m_specType; }

    //! Returns true if this Format contains a NumericFormatSpec.
    bool HasNumeric() const {return !IsProblem() || (m_problem.GetProblemCode() != FormatProblemCode::NotInitialized);}
    //! Returns true if this Format contains a CompositeFormatSpec.
    //! A Format that contains a CompositeValueSpec will also contain a NumericFormatSpec.
    bool HasComposite() const {return static_cast<std::underlying_type<FormatSpecType>::type>(m_specType) > 0 ;}
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
    //! Returns a const pointer to the input unit of this Format's CompositeValueSpec.
    //! Returns nullptr if no CompositeValueSpec is defined.
    BEU::UnitCP GetCompositeInputUnit() const {return m_compositeSpec.GetInputUnit();}
    bool HasCompositeMajorUnit() const {return nullptr != GetCompositeMajorUnit();}
    bool HasCompositeMiddleUnit() const {return nullptr != GetCompositeMiddleUnit();}
    bool HasCompositeMinorUnit() const {return nullptr != GetCompositeMinorUnit();}
    bool HasCompositeSubUnit() const {return nullptr != GetCompositeSubUnit();}
    bool HasCompositeInputUnit() const {return nullptr != GetCompositeInputUnit();}
    bool HasCompositeSpacer() const {return GetCompositeSpec()->HasSpacer();}

    void SetSuppressUnitLabel() { m_numericSpec.SetShowUnitLabel(false); }

    bool IsProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblem() const {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() const {return m_problem.GetProblemDescription();}
    PresentationType GetPresentationType() const { return m_numericSpec.GetPresentationType(); }

    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space="", int prec = -1, double round = -1.0);
    Utf8String FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const {return Format::StdFormatQuantity(*this, qty.ConvertTo(m_compositeSpec.GetInputUnit()), nullptr, space);}
    UNITS_EXPORT static Utf8String StdFormatQuantity(FormatCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit = nullptr, Utf8CP space = nullptr, Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);

    //! Parse a Format from the provided format string. A format string takes the form
    //! <code>
    //! FORMAT_NAME<PRECISION_OVERRIDE>
    //! </code>
    //! where FORMAT_NAME is a defined named format that will be located using the defaultFormatMapper parameter and PRECISION_OVERRIDE is
    //! a decimal/fractional precision override for the Format.
    //! Examples:
    //! <code>
    //! "Real<2>"
    //! "Fractional<64>"
    //! "Scientific<12>"
    //! </code>
    //! @param[out] nfs                          Format to be populated with the settings parsed from the format string.
    //! @param[in]  formatString                 String to be parsed.
    //! @param[in]  defaultFormatMapper Functor that maps a format name to a NumericformatSpec containing default settings for the parsed,
    //!                                          NumericFormatSpec, overrides specified within the format string will override these defaults.
    //!                                          defaultFormatMapper should return a pointer to some Format if any such mapping
    //!                                          exists or nullptr if no such mapping exists. For example the mapping "Real" --> <default real spec>
    //!                                          is a supported within EC, so an EC mapping function should map "Real" to the DefaultReal format
    //!                                          spec. However the mapping "BlaBlaBla" does not exist within EC by default, so the mapping function
    //!                                          would return nullptr for a format string with name "BlaBlaBla".
    //! @returns BentleyStatus::SUCCESS if the string was successfully parsed.
    UNITS_EXPORT static BentleyStatus ParseFormatString(FormatR nfs, Utf8StringCR formatString, std::function<FormatCP(Utf8StringCR)> defaultFormatMapper);

    // Legacy Descriptor string
    UNITS_EXPORT static void ParseUnitFormatDescriptor(Utf8StringR unitName, Utf8StringR formatString, Utf8CP description);
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct QuantityFormatting
{
    UNITS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, double* persist, BEU::UnitCP outputUnit, FormatCR inputFUS, FormatProblemCode* problemCode);
    UNITS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, FormatCR inputFUS, FormatProblemCode* problemCode);
};

END_BENTLEY_FORMATTING_NAMESPACE
