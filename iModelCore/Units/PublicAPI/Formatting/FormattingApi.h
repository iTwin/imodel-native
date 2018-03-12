/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingApi.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Formatting/FormattingDefinitions.h>
#include <Formatting/FormattingEnum.h>
#include <Units/Units.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE
struct StdFormatSet;

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormatSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValue)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValueSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FactorPower)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NamedFormatSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxy)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UIListEntry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LocaleProperties)

// Json presentation
BE_JSON_NAME(roundFactor)
BE_JSON_NAME(presentType)
BE_JSON_NAME(signOpt)
BE_JSON_NAME(formatTraits)
BE_JSON_NAME(LeadZeroes)
BE_JSON_NAME(TrailZeroes)
BE_JSON_NAME(KeepDecPnt)
BE_JSON_NAME(KeepSingleZero)
BE_JSON_NAME(ExponentZero)
BE_JSON_NAME(ZeroEmpty)
BE_JSON_NAME(Use1000Separator)
BE_JSON_NAME(ApplyRounding)
BE_JSON_NAME(FractionDash)
BE_JSON_NAME(UseFractSymbol)
BE_JSON_NAME(AppendUnitName)
//BE_JSON_NAME(UseLocale)
BE_JSON_NAME(decPrec)
BE_JSON_NAME(fractPrec)
BE_JSON_NAME(barType)
BE_JSON_NAME(decimalSeparator)
BE_JSON_NAME(thousandSeparator)
BE_JSON_NAME(uomSeparator)
BE_JSON_NAME(statSeparator)
BE_JSON_NAME(minWidth)

// NamedFormatSpec
BE_JSON_NAME(SpecName)
BE_JSON_NAME(SpecAlias)
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

// KOQ
BE_JSON_NAME(KOQName)
BE_JSON_NAME(persistFUS)
BE_JSON_NAME(presentFUS)
BE_JSON_NAME(relativeErr)
BE_JSON_NAME(schemaName)


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
    UNITS_EXPORT static Json::Value GetAvailableUnitLabels(BEU::UnitCP unit);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FactorPower
    {
private:
    size_t m_divisor;  // the value of divisor
    size_t m_power;    // the degree of the divisor
    int m_index;    // the index of the divisor in the prime set
    static size_t GetMin(size_t i1, size_t i2) { return (i1 <= i2) ? i1 : i2; }
public:
    FactorPower() { m_divisor = 0; m_power = 0; m_index = -1; }
    FactorPower(size_t div, size_t pow, int ind) : m_divisor(div), m_power(pow), m_index(ind) {}
    UNITS_EXPORT void CopyValues(FactorPowerP other);
    UNITS_EXPORT void Merge(FactorPowerP fp1, FactorPowerP fp2);
    const int GetDivisor() { return static_cast<int>(m_divisor); }
    const size_t GetPower() { return m_power; }
    const int GetIndex() { return m_index; }
    UNITS_EXPORT const size_t GetFactor();
    UNITS_EXPORT Utf8String ToText(Utf8Char pref);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FactorizedNumber
    {
private:
    size_t m_ival;
    bvector<FactorPower> m_factors;
    static const size_t* GetPrimes(int* length);
    static size_t PowerOfPrime(size_t ival, size_t prim, size_t* result);
    size_t RestoreNumber();

public:
    UNITS_EXPORT static size_t GetPrimeCount();
    UNITS_EXPORT FactorizedNumber(size_t ival);
    bvector<FactorPower> GetFactors() { return m_factors; }
    void ResetFactors(bvector<FactorPower> fact);
    UNITS_EXPORT FactorPowerP FindDivisor(int div);
    UNITS_EXPORT size_t GetGreatestCommonFactor(FactorizedNumber other);
    size_t GetValue() { return m_ival; }
    UNITS_EXPORT Utf8String ToText();
    UNITS_EXPORT Utf8String DebugText();
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct FractionalNumeric
    {
private:
    int64_t m_integral;
    size_t m_numerator;
    size_t m_denominator;
    size_t m_gcf;
    bvector<Utf8String> m_textParts;

    void Calculate(double dval, size_t denominator);
    size_t GCF(size_t numer, size_t denom); 
public:
    size_t GetDenominator() { return m_denominator; }
    size_t GetNumerator() { return m_numerator; }
    int64_t GetIntegral() { return m_integral; }
    UNITS_EXPORT FractionalNumeric(double dval, FractionalPrecision fprec);
    UNITS_EXPORT FractionalNumeric(double dval, int denominator);
    UNITS_EXPORT FractionalNumeric(double dval, int denominatorBase, double precision);
    UNITS_EXPORT Utf8String ToTextDefault(bool reduce);
    UNITS_EXPORT Utf8String GetIntegralString();
    UNITS_EXPORT Utf8String GetDenominatorString();
    UNITS_EXPORT Utf8String GetNumeratorString();
    UNITS_EXPORT Utf8CP GetIntegralText();
    UNITS_EXPORT Utf8CP GetDenominatorText();
    UNITS_EXPORT Utf8CP GetNumeratorText();
    UNITS_EXPORT void FormTextParts(bool reduce);
    bool HasFractionPart() { return 1 < m_textParts.size(); }
    bool IsZero() { return (0 == m_numerator); }
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  02/2018
// implemented in Formatting.cpp
//=======================================================================================
struct LocaleProperties
{
private:
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
public:

    UNITS_EXPORT LocaleProperties(Json::Value jval);
    UNITS_EXPORT LocaleProperties(Utf8Char decimal, Utf8Char thousand) : m_decimalSeparator(decimal), m_thousandsSeparator(thousand) {}
    UNITS_EXPORT LocaleProperties(Utf8CP localeName = nullptr);
    UNITS_EXPORT static LocaleProperties DefaultAmerican();
    UNITS_EXPORT static LocaleProperties DefaultEuropean(bool useBlank = false);

    Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
    Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    UNITS_EXPORT Json::Value ToJson();
    UNITS_EXPORT Utf8String ToText();
};

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericFormatSpec
    {
private:
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Scientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParentheses
    FormatTraits        m_formatTraits;          // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    FractionBarType     m_barType;               // Oblique, Horizontal, Diagonal

    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
    Utf8String          m_uomSeparator;          // Default separator between the number and UOM.
    Utf8Char            m_statSeparator;         // Default separator between parts of the stopping format.
    int                 m_minWidth;              // The minimum width of the field. It will be taken into account
                                                 // only if the overall length (width) of the text representing integer
                                                 // a number of or integer part of a real is shorter and needs to be augmented by
                                                 // insignificant zeroes. Blanks are not considered because aligning text
                                                 // with the boundaries of a virtual box is the responsibility of annotation layer.

    double EffectiveRoundFactor(double rnd) const { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }

    double RoundedValue(double dval, double round) const;
    int TrimTrailingZeroes(Utf8P buf, int index) const;
    size_t InsertChar(Utf8P buf, size_t index, char c, int num) const;

    // TODO: Attempt to remove these methods from the private API===============
    int FormatInteger(int n, Utf8P bufOut, int bufLen);
    size_t FormatDoubleBuf(double dval, Utf8P buf, size_t bufLen, int prec = -1, double round = -1.0) const;
    Utf8String FormatRoundedDouble(double dval, double round);
    static double RoundDouble(double dval, double roundTo);
    int GetDecimalPrecisionIndex(int prec) const;
    double GetDecimalPrecisionFactor(int prec) const;
    void SetPrecisionByValue(int prec);
    static int RightAlignedCopy(Utf8P dest, int destLen, bool termZero, CharCP src, int srcLen);
    static bool AcceptableDifference(double dval1, double dval2, double maxDiff);
    int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const;
    int FormatBinaryByte(unsigned char n, Utf8P bufOut, int bufLen);
    int FormatBinaryShort(short int n, Utf8P bufOut, int bufLen, bool useSeparator);
    int FormatBinaryInt(int n, Utf8P bufOut, int bufLen, bool useSeparator);
    int FormatBinaryDouble(double x, Utf8P bufOut, int bufLen, bool useSeparator);
    NumericFormatSpec(size_t precision) { DefaultInit(precision); }
    NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision, Utf8CP uomSeparator=nullptr);
    Utf8String FormatIntegerToString(int n, int minSize) const;
    void Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    void DefaultInit(size_t precision);
    bool IsInsertSeparator(bool confirm) const { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    static void TraitsBitToJsonKey(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits traits);
    // !TODO====================================================================

public:
    // TODO: Attempt to remove these methods from the public API================
    UNITS_EXPORT int static FormatIntegerSimple(int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero);
    // !TODO====================================================================

    UNITS_EXPORT NumericFormatSpec();
    UNITS_EXPORT NumericFormatSpec(DecimalPrecision decimalPrecision);
    UNITS_EXPORT NumericFormatSpec(Json::Value jval);
    NumericFormatSpec(NumericFormatSpecCR other) = default;
    ~NumericFormatSpec() = default;

    NumericFormatSpecR operator=(NumericFormatSpecCR other) = default;

    UNITS_EXPORT bool IsIdentical(NumericFormatSpecCR other) const;

    UNITS_EXPORT bool ImbueLocale(Utf8CP localeName);
    UNITS_EXPORT bool ImbueLocaleProperties(LocalePropertiesCR locProp);

    UNITS_EXPORT static NumericFormatSpecCR DefaultFormat();

    //======================================
    // Data Member Setters/Getters
    //======================================
    void SetRoundingFactor(double roundingFactor) { m_roundFactor = roundingFactor; }
    double GetRoundingFactor() const { return m_roundFactor; }

    void SetPresentationType(PresentationType type) { m_presentationType = type; }
    PresentationType GetPresentationType() const { return m_presentationType; }
    bool IsDecimal() const { return PresentationType::Decimal == m_presentationType; }
    bool IsFractional() const { return PresentationType::Fractional == m_presentationType; }
    bool IsScientific() const { return PresentationType::Scientific == m_presentationType || PresentationType::ScientificNorm == m_presentationType; }

    void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
    ShowSignOption GetSignOption() const { return m_signOption; }
    bool IsNoSign() const { return ShowSignOption::NoSign == m_signOption; }
    bool IsOnlyNegative() const { return ShowSignOption::OnlyNegative == m_signOption; }
    bool IsSignAlways() const { return ShowSignOption::SignAlways == m_signOption; }
    bool IsNegativeParentheses() const { return ShowSignOption::NegativeParentheses == m_signOption; }

    void SetFormatTraits(FormatTraits traits) { m_formatTraits = traits; }
    FormatTraits GetFormatTraits() const { return m_formatTraits; }

    void SetDecimalPrecision(DecimalPrecision precision) { m_decPrecision = precision; }
    DecimalPrecision GetDecimalPrecision() const { return m_decPrecision; }
    bool IsPrecisionZero() const { return m_decPrecision == DecimalPrecision::Precision0; }

    void SetFractionaPrecision(FractionalPrecision precision) { m_fractPrecision = precision; }
    FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }

    void SetFractionalBarType(FractionBarType barType) { m_barType = barType; }
    FractionBarType GetFractionalBarType() const { return m_barType; }

    void SetDecimalSeparator(Utf8Char sep) { m_decimalSeparator = sep; }
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }

    void SetThousandSeparator(char sep) { m_thousandsSeparator = sep; }
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }

    void SetUomSeparator(Utf8CP sep) { m_uomSeparator = Utf8String(sep); }
    Utf8CP GetUomSeparator(Utf8CP def = nullptr) const { return (nullptr == def)?  m_uomSeparator.c_str() : def; }

    void SetMinWidth(int wid) { m_minWidth = wid; }
    int GetMinWidth() { return m_minWidth; }

    void SetStopSeparator(Utf8Char sep) { m_statSeparator = sep; }
    Utf8Char GetStopSeparator() const { return m_statSeparator; }

    //======================================
    // Format Traits Bit Setters/Getters
    //======================================
    UNITS_EXPORT static FormatTraits SetTraitsBit(FormatTraits traits, FormatTraits bit, bool setTo);
    UNITS_EXPORT static bool GetTraitsBit(FormatTraits traits, FormatTraits bit);
    UNITS_EXPORT void SetTraitsBit(FormatTraits bit, bool setTo);
    UNITS_EXPORT bool GetTraitsBit(FormatTraits bit) const;
    UNITS_EXPORT void TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose=false) const;
    
    UNITS_EXPORT void SetFormatTraitsFromJson(JsonValueCR jval);
    UNITS_EXPORT Json::Value FormatTraitsToJson(bool verbose) const;

    void SetUseLeadingZeroes(bool setTo) { SetTraitsBit(FormatTraits::LeadingZeroes, setTo); }
    bool IsUseLeadingZeroes() const { return GetTraitsBit(FormatTraits::LeadingZeroes); }

    void SetKeepTrailingZeroes(bool setTo) {SetTraitsBit(FormatTraits::TrailingZeroes, setTo);}
    bool IsKeepTrailingZeroes() const { return GetTraitsBit(FormatTraits::TrailingZeroes);}

    void SetKeepDecimalPoint(bool setTo) { SetTraitsBit(FormatTraits::KeepDecimalPoint, setTo);}
    bool IsKeepDecimalPoint() const { return GetTraitsBit(FormatTraits::KeepDecimalPoint); }

    void SetKeepSingleZero(bool setTo) { SetTraitsBit(FormatTraits::KeepSingleZero, setTo);}
    bool IsKeepSingleZero() const { return GetTraitsBit(FormatTraits::KeepSingleZero); }

    void SetExponentZero(bool setTo) { SetTraitsBit(FormatTraits::ExponentZero, setTo); }
    bool IsExponentZero() const { return GetTraitsBit(FormatTraits::ExponentZero); }

    void SetZeroEmpty(bool setTo) { SetTraitsBit(FormatTraits::ZeroEmpty, setTo); }
    bool IsZeroEmpty() const { return GetTraitsBit(FormatTraits::ZeroEmpty); }

    void SetUse1000Separator(bool setTo) { SetTraitsBit(FormatTraits::Use1000Separator, setTo); }
    bool IsUse1000Separator() const { return GetTraitsBit(FormatTraits::Use1000Separator); }

    void SetApplyRounding(bool setTo) { SetTraitsBit(FormatTraits::ApplyRounding, setTo); }
    bool IsApplyRounding() const { return GetTraitsBit(FormatTraits::ApplyRounding); }

    void SetFractionDash(bool setTo) { SetTraitsBit(FormatTraits::FractionDash, setTo); }
    bool IsFractionDash() const { return GetTraitsBit(FormatTraits::FractionDash); }

    void SetUseFractSymbol(bool setTo) { SetTraitsBit(FormatTraits::UseFractSymbol, setTo); }
    bool IsUseFractSymbol() const { return GetTraitsBit(FormatTraits::UseFractSymbol); }

    void SetAppendUnit(bool setTo) { SetTraitsBit(FormatTraits::AppendUnitName, setTo); }
    bool IsAppendUnit() const { return GetTraitsBit(FormatTraits::AppendUnitName); }

    //======================================
    // Formatting Methods
    //======================================
    UNITS_EXPORT Utf8String ByteToBinaryText(Byte n);
    UNITS_EXPORT Utf8String Int16ToBinaryText(int16_t n, bool useSeparator);
    UNITS_EXPORT Utf8String Int32ToBinaryText(int32_t n, bool useSeparator);
    UNITS_EXPORT Utf8String DoubleToBinaryText(double n, bool useSeparator);

    UNITS_EXPORT Utf8String FormatInteger(int32_t ival);
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0) const;

    UNITS_EXPORT static Utf8String StdFormatDouble(Utf8CP stdName, double dval, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit = nullptr, Utf8CP space = nullptr, Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space="", int prec = -1, double round = -1.0);

    UNITS_EXPORT Json::Value ToJson(bool verbose) const;
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  06/2017
//=======================================================================================
struct UnitProxy
    {
private:
    BEU::UnitCP mutable m_unit;
    Utf8String mutable m_unitLabel;

public:
    UnitProxy() : m_unit(nullptr) {}
    UNITS_EXPORT UnitProxy(BEU::UnitCP unit, Utf8CP label = nullptr);
    UnitProxy(UnitProxyCR other)
        {
        m_unit = other.m_unit;
        m_unitLabel = Utf8String(other.m_unitLabel.c_str());
        }
    void Copy(UnitProxyCP other)
        {
        if (nullptr == other)
            {
            m_unit = nullptr;
            }
        else
            {
            m_unit = other->m_unit;
            m_unitLabel = Utf8String(other->m_unitLabel.c_str());
            }
        }

    UNITS_EXPORT void LoadJson(Json::Value jval, BEU::IUnitsContextCP context);
    bool SetUnit(BEU::UnitCP unit) {m_unit = unit; return true;}
    Utf8CP GetLabel() const { return m_unitLabel.c_str(); }
    Utf8CP SetLabel(Utf8CP lab) { m_unitLabel = Utf8String(lab);  return m_unitLabel.c_str(); }

    //! Returns the name of Unit in this if one is available
    Utf8CP GetName() const {if (nullptr == m_unit) return nullptr; return m_unit->GetName().c_str();}
    BEU::UnitCP GetUnit() const { return m_unit; }
    UNITS_EXPORT Json::Value ToJson() const;
    bool IsEmpty() const {return nullptr == m_unit;}
    bool IsIdentical(UnitProxyCR other) const {return BEU::Unit::AreEqual(m_unit, other.m_unit) && m_unitLabel.Equals(other.m_unitLabel);}
    };

//=======================================================================================
// We recognize combined numbers (combo-numbers) that represent some quantity as a sum of 
//   subquantities expressed in lesser UOM's. For example, a given length could be representes
//    as a sum of M + Y + F + I where M- is a number of miles, Y - a number of yards, 
//   F - a number of feet and I a number of inches. The operation of expressing the given length 
//  in this form will require 3 ratios: M/Y, Y/F, F/I where M/Y is a number of Yards in a Mile,
//   Y/F - is a number of feet in one yard, F/I is a number of inches in one foot.
//  Obviously, the combo-presentation of length is just one example and in some special cases 
//   this system of ratios can be anything the application needs. Ratios could be set explicitly
// or automatically via names of the UOM's. The only condition is that all integer ratios must be > 1.
//   let's define terms for UOM of different levels: 0 - majorUOM, 1 - midlleUOM, 2 - minorUOM, 3 - subUOM
//    accordingly there are 3 ratios: major/middle, middle/minor. minor/sub and their indexes
// in the ratio array are associated with the upper UOM
// @bsiclass                                                    David.Fox-Rabinovitz  01/2017
//=======================================================================================
struct CompositeValueSpec
    {
    friend struct CompositeValue;
private:
    static const size_t  indxMajor  = 0;
    static const size_t  indxMiddle = 1;
    static const size_t  indxMinor  = 2;
    static const size_t  indxSub    = 3;
    size_t m_ratio[indxSub];
    bvector<UnitProxy> mutable m_proxys;
    FormatProblemDetail m_problem;
    CompositeSpecType m_type;
    bool m_includeZero; // Not currently used in the formatting code.
    Utf8String m_spacer;

    size_t CalculateUnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    size_t CalculateUnitRatio(size_t uppIndx, size_t lowIndx) {return  CalculateUnitRatio(GetUnit(uppIndx), GetUnit(lowIndx));}
    void ResetType() { m_type = CompositeSpecType::Undefined; }
    void CalculateUnitRatios();

    Utf8CP GetUnitName(size_t indx, Utf8CP substitute = nullptr) const;
    void SetUnitLabel(size_t index, Utf8CP label);
    Utf8CP GetUnitLabel(size_t index, Utf8CP substitute = nullptr) const;
    Utf8String GetEffectiveLabel(size_t indx) const;
    BEU::UnitCP GetSmallestUnit() const;

    UnitProxyP GetProxyP(size_t indx) const {return IsIndexCorrect(indx) ? &m_proxys[indx] : nullptr;}
    UnitProxyCP GetProxy(size_t indx) const {return GetProxyP(indx);}
    BEU::UnitCP GetUnit(size_t indx) const
        {
        UnitProxyCP proxy = GetProxy(indx);
        if (nullptr == proxy)
            return nullptr;
        return proxy->GetUnit();
        }
    bool SetUnit(size_t indx, BEU::UnitCP unitP) {return IsIndexCorrect(indx) ? m_proxys[indx].SetUnit(unitP) : false;}
    bool IsIndexCorrect(size_t indx) const { return indx < m_proxys.size(); }

public:
    UNITS_EXPORT void Init();
    UNITS_EXPORT void Clone(CompositeValueSpecCR other);
    CompositeValueSpec() { Init(); }

    CompositeValueSpec(CompositeValueSpecCR other) {Clone(other);}
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit=nullptr, BEU::UnitCP minorUnit=nullptr, BEU::UnitCP subUnit = nullptr);

    BEU::UnitCP GetMajorUnit() const {return GetUnit(indxMajor);}
    BEU::UnitCP GetMiddleUnit() const {return GetUnit(indxMiddle);}
    BEU::UnitCP GetMinorUnit() const {return GetUnit(indxMinor);}
    BEU::UnitCP GetSubUnit() const {return GetUnit(indxSub);}

    UNITS_EXPORT void SetUnitLabels(Utf8CP majorLabel, Utf8CP middleLabel = nullptr, Utf8CP minorLabel = nullptr, Utf8CP subLabel = nullptr);
    UNITS_EXPORT Utf8String GetMajorLabel() const { return GetEffectiveLabel(indxMajor); }
    UNITS_EXPORT Utf8String GetMiddleLabel() const { return GetEffectiveLabel(indxMiddle); }
    UNITS_EXPORT Utf8String GetMinorLabel() const { return GetEffectiveLabel(indxMinor); }
    UNITS_EXPORT Utf8String GetSubLabel() const { return GetEffectiveLabel(indxSub); }

    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
    bool IsProblem() const { return m_problem.IsProblem(); }
    bool NoProblem() const { return m_problem.NoProblem(); }
    size_t GetMajorToMiddleRatio() const {return m_ratio[indxMajor];}
    size_t GetMiddleToMinorRatio() const  {return m_ratio[indxMiddle];}
    size_t GetMinorToSubRatio() const {return m_ratio[indxMinor];}
    size_t GetUnitCount() const {return m_proxys.size();}
    Utf8CP GetProblemDescription() const {return m_problem.GetProblemDescription().c_str();}
    UNITS_EXPORT CompositeValue DecomposeValue(double dval, BEU::UnitCP uom = nullptr);
    CompositeSpecType GetType() const { return m_type; }
    Utf8String GetSpacer() const { return m_spacer; }
    Utf8String SetSpacer(Utf8CP spacer) { return m_spacer = spacer; }
    bool IsIncludeZero() const { return m_includeZero; }
    bool SetIncludeZero(bool incl) { return m_includeZero = incl; }
    UNITS_EXPORT Json::Value ToJson() const;
    UNITS_EXPORT void LoadJsonData(JsonValueCR jval, BEU::IUnitsContextCP context);
    UNITS_EXPORT bool IsIdentical(CompositeValueSpecCR other) const;
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct CompositeValue
    {
private:
    static const size_t  indxInput = 4;
    bool m_negative;
    double m_parts[5];
    FormatProblemDetail m_problem;
    double GetSignFactor() { return m_negative ? -1.0 : 1.0; }
    void Init();
public:
    UNITS_EXPORT CompositeValue();

    void SetNegative() { m_negative = true; }
    void SetPositive() { m_negative = false; }

    Utf8String GetSignPrefix(bool useParenth = false) { return m_negative?  (useParenth ? "(" : "-") : ""; }
    Utf8String GetSignSuffix(bool useParenth = false) { return m_negative ? (useParenth ? ")" : "") : ""; }

    double SetMajor(double dval)  { return m_parts[CompositeValueSpec::indxMajor] = dval; }
    double SetMiddle(double dval) { return m_parts[CompositeValueSpec::indxMiddle] = dval; }
    double SetMinor(double dval)  { return m_parts[CompositeValueSpec::indxMinor] = dval; }
    double SetSub(double dval)    { return m_parts[CompositeValueSpec::indxSub] = dval; }
    double SetInput(double dval)  { return m_parts[indxInput] = dval; }

    double GetMajor()  { return m_parts[CompositeValueSpec::indxMajor]; }
    double GetMiddle() { return m_parts[CompositeValueSpec::indxMiddle]; }
    double GetMinor()  { return m_parts[CompositeValueSpec::indxMinor]; }
    double GetSub()    { return m_parts[CompositeValueSpec::indxSub]; }
    double GetInput()  { return m_parts[indxInput]; }

    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
    bool IsProblem() { return m_problem.IsProblem(); }
    };

//=======================================================================================
//! Container for keeping together primary numeric, composite and other types of specs
//! and referring to them by the unique name. Name and a valid numeric spec are required
//! for creating a valid instance of this class. Alias and composite spec are optional
//! at the moment of creation but can be added at any time.
//!
//! @bsistruct                                             David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct NamedFormatSpec
    {
private:
        Utf8String         m_name;                  // name or ID of the format
        Utf8String         m_alias;                 // short alternative name (alias)
        Utf8String         m_description;           // @units_msg:descr_Real8@
        Utf8String         m_displayLabel;          // @units_msg:label_Real8@
        NumericFormatSpec  m_numericSpec;
        CompositeValueSpec m_compositeSpec;
        FormatSpecType     m_specType;
        FormatProblemDetail m_problem;

public:
        UNITS_EXPORT void Init(FormatProblemCode prob = FormatProblemCode::NoProblems);
        UNITS_EXPORT void Clone(NamedFormatSpecCR other);
        UNITS_EXPORT void Clone(NamedFormatSpecCP other);
        UNITS_EXPORT NamedFormatSpec& operator=(const NamedFormatSpec& other);

        UNITS_EXPORT void LoadJson(Json::Value jval, BEU::IUnitsContextCP context);
        UNITS_EXPORT void LoadJson(Utf8CP jsonString, BEU::IUnitsContextCP context);

        //! Creates a new NamedFormatSpec with default values.
        NamedFormatSpec() : m_specType(FormatSpecType::Undefined) {m_problem.UpdateProblemCode(FormatProblemCode::NFS_Undefined);}
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Json::Value jval, BEU::IUnitsContextCP context = nullptr);
        UNITS_EXPORT NamedFormatSpec(Utf8CP jsonString, BEU::IUnitsContextCP context = nullptr);

        Utf8CP SetAlias(Utf8CP alias) { m_alias = alias;  return m_alias.c_str(); }
        Utf8CP GetAlias() const { return m_alias.c_str(); }
        UNITS_EXPORT bool HasName(Utf8CP name) const;
        UNITS_EXPORT bool HasAlias(Utf8CP name) const;
        void SetSuppressUnitLabel() {m_numericSpec.SetAppendUnit(false);}

        Utf8CP GetName() const { return m_name.c_str(); };
        Utf8CP GetDescription() const { return m_description.c_str(); };
        Utf8CP SetDescription(Utf8CP descr) { m_description = descr;  return m_description.c_str(); };
        Utf8CP GetLabel() const { return m_displayLabel.c_str(); };
        Utf8CP SetLabel(Utf8CP label) { m_displayLabel = label;  return m_displayLabel.c_str(); };

        FormatSpecType  GetSpecType(){return m_specType;}
        bool HasComposite() const { return FormatSpecType::Composite == m_specType; }
        size_t GetCompositeUnitCount() const { return HasComposite() ? m_compositeSpec.GetUnitCount() : 0; }
        NumericFormatSpecCP GetNumericSpec() const { return &(this->m_numericSpec); }
        CompositeValueSpecCP GetCompositeSpec() const { return  (HasComposite() ? &m_compositeSpec : nullptr); }
        bool IsProblem() const { return m_problem.IsProblem(); }
        Utf8String GetProblemDescription() { return m_problem.GetProblemDescription(); }
        Utf8String GetNameAndAlias() const { return Utf8String(m_name) + Utf8String("(") + Utf8String(m_alias) + Utf8String(")"); };
        PresentationType GetPresentationType() const { return m_numericSpec.GetPresentationType(); }
        UNITS_EXPORT Json::Value ToJson(bool verbose) const;
        UNITS_EXPORT bool IsIdentical(NamedFormatSpecCR other) const;
        BEU::UnitCP GetCompositeMajorUnit() const { return HasComposite() ? m_compositeSpec.GetMajorUnit() : nullptr; }
        BEU::UnitCP GetCompositeMiddleUnit() const { return HasComposite() ? m_compositeSpec.GetMiddleUnit() : nullptr; }
        BEU::UnitCP GetCompositeMinorUnit() const { return HasComposite() ? m_compositeSpec.GetMinorUnit() : nullptr; }
        BEU::UnitCP GetCompositeSubUnit() const { return HasComposite() ? m_compositeSpec.GetSubUnit() : nullptr; }
    };

//=======================================================================================
//! The Format-Unit Set(FUS) has two parts describing how a Quantity transformation 
//! between an internal form and presentation form should be handled.
//! 
//! The two parts:
//! - The Unit member defines what Unit of Measurement(UOM) should be used when the format 
//!      does not provide this information. When format refers to a Composite Format that 
//!      provides at least one UOM, the format definition takes preference. Another 
//!      important role of the Unit member is to help in converting a pure numeric text 
//!      expression provided by the user input into a Quantity.
//! - The Format member provides formatting parameters when the Quantity needs to be 
//!      presented in the UI and it can be used as a reference for validating the complex user input
//!
// @bsiclass                                                    David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct FormatUnitSet
    {
    friend struct StdFormatSet;
    private:
        NamedFormatSpecCP m_formatSpec;
        Utf8String  m_unitName;
        BEU::UnitCP m_unit;
        FormatProblemDetail m_problem;
        NamedFormatSpec m_localCopy;
        mutable Utf8String  m_fusName;

        Utf8CP GetDefaultDisplayLabel() const;
        Utf8CP SetFusName(Utf8CP name) const { m_fusName.assign(name);  return m_fusName.c_str(); }

    public:
        UNITS_EXPORT void Init();
        FormatUnitSet() : m_formatSpec(nullptr), m_unit(nullptr), m_problem(FormatProblemCode::NotInitialized) {}
        FormatUnitSet(BEU::UnitCP unit) : FormatUnitSet(nullptr, unit) {}
        FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit) : FormatUnitSet(format, unit, false) {}
        FormatUnitSet(FormatUnitSetCR other) : m_formatSpec(other.m_formatSpec), m_unitName(other.m_unitName), m_unit(other.m_unit), m_problem(other.m_problem) {}
        // UNITS_EXPORT FormatUnitSet(Utf8CP formatName, BEU::UnitCP unit);
        UNITS_EXPORT FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit, bool cloneData);
        
        UNITS_EXPORT FormatUnitSet& operator=(const FormatUnitSet& other);

        UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const;

        //! The 'descriptor' argument is a text string in one of several formats as follows:
        //! For compatibility with the obsolete KOQ def's it may consist of only a unit name, e.g.
        //! FT. Since FUS consists of two components: the reference to the Unit and a reference to
        //! a format specification, the DefaultReal format will be used in this case. The most
        //! common descriptor consists of two names: the Unit Name and the Format Name, e.g.
        //! FT(real6). For supporting the usage of comples Unit Names a "vertical bar" can be used
        //! as a separator between the Unit Name and the Format Name as in "FT|real6" The closing
        //! vertical bar delimiting the Format Name is not required if the Format Name is
        //! terminated with the "end-of-line". However, the Fomat Name can be also delimited by
        //! the "vertical bar" as in: "FT|real6|". The descriptor can be also a JSON-string enclosed
        //! in the "curvy brackets". There are two types of this JSON-string which are currently
        //! supported: A short one consists of the unitName and formatName. The optional cloneData
        //! boolean value indicates that the format spec should be cloned into the newly created
        //! FUS. The default value of the cloneData parameter if "false" the long one consists of
        //! the unitName and formatSpec that contains the full description of this Spec.
        UNITS_EXPORT FormatUnitSet(Utf8CP descriptor, BEU::IUnitsContextCP context);

        //! Resets this FUS to its initial state and populates it will the json value.
        //!
        //! Supported JSON format:
        //!     <code>
        //!     {
        //!         unitName: "",
        //!         formatName: "",
        //!         cloneData: true,
        //!         formatSpec: { }
        //!     }
        //!     </code>
        //!
        //! Potential problem codes:
        //!     - FormatProblemCode::NFS_InvalidJsonObject,
        //!         - When Json value is empty
        //!     - FormatProblemCode::UnknownUnitName
        //!         - If the unitName is provided but cannot be found in the UnitRegistry
        //!
        //! @note If a formatName or formatSpec is not defined in the JSON value the FormatConstant::DefaultFormatName is used.
        //! @note If a formatName and formatSpec are provided in the JSON the last will be used.
        //!
        //! @param[in] jval Json to use to populate this.
        UNITS_EXPORT void LoadJson(Json::Value jval, BEU::IUnitsContextCP context);

        //! Returns whether this FormatUnitSet has a problem.
        bool HasProblem() const { return m_problem.IsProblem(); }

        //! Returns the problem code for this FUS.
        FormatProblemCode GetProblemCode() const { return m_problem.GetProblemCode(); }
        Utf8String GetProblemDescription() const { return m_problem.GetProblemDescription(); }
        Utf8String GetUnitName() const { return m_unitName; }
        Utf8CP GetFusName() const { return m_fusName.c_str(); }

        UNITS_EXPORT Utf8CP GetDisplayLabel(bool useDefault=false) const;
        UNITS_EXPORT Utf8String ToText(bool useAlias = true) const;
        BEU::UnitCP GetUnit() const { return m_unit; }
        NamedFormatSpecCP GetNamedFormatSpec() const { return m_formatSpec; }
        bool IsComparable(BEU::QuantityCR qty) const {return IsComparable(qty.GetUnit());}
        bool IsComparable(BEU::UnitCP unit) const {return BEU::Unit::AreCompatible(unit, m_unit);}

        UNITS_EXPORT Json::Value ToJson(bool useAlias = true, bool verbose = false) const;
        UNITS_EXPORT Utf8String ToJsonString(bool useAlias = true, bool verbose = false) const;

        UNITS_EXPORT Json::Value FormatQuantityJson(BEU::QuantityCR qty, bool useAlias, Utf8CP space="") const;
        BEU::PhenomenonCP GetPhenomenon() { return (nullptr == m_unit) ? nullptr : m_unit->GetPhenomenon(); }

        //! Populates this FormatUnitSet with the provided Json data. 
        //! 
        //! Supported JSON format is:
        //!     <code>
        //!     {
        //!         fusName: "SampleFUS"
        //!         unitName: "SampleUnit"
        //!         formatName: "SampleFormat"
        //!     }
        //!     </code>
        //!
        //! Potential problem code:
        //!     - FormatProblemCode::UnknownUnitName
        //!         - If an unitName is provided but cannot be found in the UnitRegistry
        //!     - FormatProblemCode::UnknownStdFormatName
        //!         - If a formatName is provided but cannot be found in the StdFormatSet
        //!
        //! @param[in] jval Json to use to populate this.
        UNITS_EXPORT void LoadJsonData(Json::Value jval);
        UNITS_EXPORT bool IsIdentical(FormatUnitSetCR other) const;
        
        bool IsFullySpecified() { return (m_formatSpec == &m_localCopy); }
        bool IsSetRegistered() { return !m_fusName.empty(); }
        bool HasComposite() const { return nullptr != m_formatSpec && m_formatSpec->HasComposite(); }
        size_t GetCompositeUnitCount() const { return HasComposite() ? m_formatSpec->GetCompositeUnitCount() : 0; }
        BEU::UnitCP GetCompositeMajorUnit() const { return HasComposite() ? m_formatSpec->GetCompositeMajorUnit() : nullptr; }
        BEU::UnitCP GetCompositeMiddleUnit() const { return HasComposite() ? m_formatSpec->GetCompositeMiddleUnit() : nullptr; }
        BEU::UnitCP GetCompositeMinorUnit() const { return HasComposite() ? m_formatSpec->GetCompositeMinorUnit() : nullptr; }
        BEU::UnitCP GetCompositeSubUnit() const { return HasComposite() ? m_formatSpec->GetCompositeSubUnit() : nullptr; }

        UNITS_EXPORT static void ParseUnitFormatDescriptor(Utf8StringR unitName, Utf8StringR formatString, Utf8CP description);
    };

//=======================================================================================
//! Singleton container for known NamedFormatSpecs and FormatUnitSets.
//!
//! @bsistruct
//=======================================================================================
struct StdFormatSet
    {
private:
    // This is going to be used only during testing
    BEU::IUnitsContextCP m_unitsRegistry;

    bvector<NamedFormatSpecCP> m_formatSet;    // core + app
    bvector<FormatUnitSetCP> m_fusSet;
    FormatProblemDetail m_problem; 

    NumericFormatSpecCP AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, Utf8CP alias = nullptr);
    NumericFormatSpecCP AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, CompositeValueSpecCR compS, Utf8CP alias = nullptr); 
    NumericFormatSpecCP AddFormat(Utf8CP jsonString);
    NamedFormatSpecCP AddNamedFormat(Utf8CP jsonString);

    size_t StdInit();
    void CustomInit();
    StdFormatSet() { m_problem = FormatProblemDetail(); }
    static StdFormatSetP Set();
    UNITS_EXPORT static bool IsFormatDefined(Utf8CP name, Utf8CP alias);
    UNITS_EXPORT FormatUnitSetCP FindFUS(Utf8CP fusName) const;
    bool HasDuplicate(Utf8CP name, FormatUnitSetCP * fusP);

public:
    //static StdFormatSetP GetStdSet() { return Set(); }
    UNITS_EXPORT static NumericFormatSpecCP DefaultDecimal();
    static NamedFormatSpecCP DefaultFormatSpec() { return FindFormatSpec(FormatConstant::DefaultFormatName()); }
    static size_t GetFormatSetSize() { return Set()->m_formatSet.size(); }
    UNITS_EXPORT static NumericFormatSpecCP GetNumericFormat(Utf8CP name);
    UNITS_EXPORT static NamedFormatSpecCP FindFormatSpec(Utf8CP name);
    UNITS_EXPORT static bvector<Utf8CP> StdFormatNames(bool useAlias);
    UNITS_EXPORT static Utf8String StdFormatNameList(bool useAlias);
    size_t GetFormatCount() { return m_formatSet.size(); }
    bool HasProblem() const { return m_problem.IsProblem(); }
    FormatProblemCode GetProblemCode() { return m_problem.GetProblemCode(); }
    void ResetProblemCode() { m_problem.Reset(); }

    static FormatUnitSet DefaultFUS(BEU::QuantityCR qty) { return FormatUnitSet(DefaultFormatSpec(), qty.GetUnit()); }

    //! Creates a new FormatUnitSet with the provided name and FUS, and adds it to the set.
    //! @remark There are no duplicate named FUSes allowed in a StdFormatSet.
    //! @param[in] fusR The FUS to copy the data for the to be created FUS
    //! @param[in] fusName Name of the FUS to create
    //! @return A pointer to the newly created FormatUnitSet if successful; otherwise, nullptr.
    UNITS_EXPORT static FormatUnitSetCP AddFUS(FormatUnitSetCR fusR, Utf8CP fusName);

    //! Creates a new FormatUnitSet with the provided name using the format and unit.
    //! @param[in] formatName Name of the NamedFormatSpec to add to the FormatUnitSet to be created.
    //! @param[in] unit Unit to add to the FormatUnitSet to be created.
    //! @param[in] fusName Name of the FormatUnitSet to be created.
    //! @return A pointer to the newly created FormatUnitSet if successful; otherwise, nullptr.
    UNITS_EXPORT static FormatUnitSetCP AddFUS(Utf8CP formatName, BEU::UnitCP unit, Utf8CP fusName);

    //! Creates a new FormatUnitSet with the provided name and json string describing the FUS to be
    //! created.
    //! @param[in] descriptor string representation of the FUS to be created.
    //! @param[in] fusName Name of the FormatUnitSet to be created.
    //! @return A pointer to the newly created FormatUnitSet if successful; otherwise, nullptr.
    UNITS_EXPORT static FormatUnitSetCP AddFUS(Utf8CP descriptor, Utf8CP fusName);

    //! Creates a new NamedFormatSpec with the provided JSON string representation of a NamedFormatSpec.
    //! @param[in] jsonString string representation of the NamedFormatSpec to be created.
    //! @param[out] problem Contains the problem code from adding the format defined in the JSON string.
    //! @return pointer to the newly created NamedFormatSpec if successful; otherwise, nullptr.
    UNITS_EXPORT static NamedFormatSpecCP AddFormat(Utf8CP jsonString, FormatProblemDetailR problem);

    //! Gets the FUS from this StdFormatSet.
    //! @param[in] name Name of the FUS to retrieve.
    //! @return A pointer to the FormatUnitSet if found in this set, nullptr otherwise.
    UNITS_EXPORT static FormatUnitSetCP LookupFUS(Utf8CP name);

    //! Whether or not the StdFormatSet has a problem.
    //! @return true if the Set has a problem; false, otherwise.
    UNITS_EXPORT static bool FusRegistrationHasProblem() {return Set()->HasProblem();}
    };

struct QuantityFormatting
    {
    UNITS_EXPORT static Units::Quantity CreateQuantity(Utf8CP input, size_t start, double* persist, FormatUnitSetCR outputFUS, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode);
    UNITS_EXPORT static Units::Quantity CreateQuantity(Utf8CP input, size_t start, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode);
    };

END_BENTLEY_FORMATTING_NAMESPACE
