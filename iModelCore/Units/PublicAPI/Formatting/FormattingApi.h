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
#include <Formatting/FormattingParsing.h>
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxySet)
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
    UNITS_EXPORT static Json::Value GetAvailableUnitLabels(Utf8CP unitName);
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

    void Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    double RoundedValue(double dval, double round) const;
    int TrimTrailingZeroes(Utf8P buf, int index) const;
    size_t InsertChar(Utf8P buf, size_t index, char c, int num) const;
    void LoadJson(Json::Value jval);

public:
    NumericFormatSpec() { DefaultInit(FormatConstant::DefaultDecimalPrecisionIndex()); }
    NumericFormatSpec(NumericFormatSpecCR other) = default;
    NumericFormatSpec(NumericFormatSpecCP other) : NumericFormatSpec(*other) {};
    NumericFormatSpec(size_t precision) { DefaultInit(precision); }
    ~NumericFormatSpec() = default;
    UNITS_EXPORT static const NumericFormatSpecCP DefaultFormat();

    NumericFormatSpec& operator=(NumericFormatSpecCR other) = default;
    UNITS_EXPORT void Clone(NumericFormatSpecCR other) { *this = other; }

    UNITS_EXPORT bool IsIdentical(NumericFormatSpecCR other) const;

    /* TODO: These are untested. */
    UNITS_EXPORT void DefaultInit(size_t precision);
    UNITS_EXPORT NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision, Utf8CP uomSeparator=nullptr);
    UNITS_EXPORT NumericFormatSpec(Json::Value jval);
    UNITS_EXPORT NumericFormatSpec(Utf8CP jsonString);
    /* !TODO */

    UNITS_EXPORT static int RightAlignedCopy(Utf8P dest, int destLen, bool termZero, CharCP src, int srcLen);
    UNITS_EXPORT static bool AcceptableDifference(double dval1, double dval2, double maxDiff);
    UNITS_EXPORT static double RoundDouble(double dval, double roundTo);

    UNITS_EXPORT int GetDecimalPrecisionIndex(int prec) const;
    UNITS_EXPORT double GetDecimalPrecisionFactor(int prec) const;
    UNITS_EXPORT void SetPrecisionByValue(int prec);

    UNITS_EXPORT bool ImbueLocale(Utf8CP localeName);
    UNITS_EXPORT bool ImbueLocaleProperties(LocalePropertiesCR locProp);

    //======================================
    // Data Member Setters/Getters
    //======================================
    double SetRoundingFactor(double round) { return m_roundFactor = round; }
    double GetRoundingFactor() const { return m_roundFactor; }

    void SetPresentationType(PresentationType type) { m_presentationType = type; }
    PresentationType GetPresentationType() const { return m_presentationType; }

    void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
    ShowSignOption GetSignOption() const { return m_signOption; }

    void SetFormatTraits(FormatTraits opt) { m_formatTraits = opt; }
    FormatTraits GetFormatTraits() const { return m_formatTraits; }

    void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    DecimalPrecision GetDecimalPrecision() const { return m_decPrecision; }

    FractionalPrecision SetFractionaPrecision(FractionalPrecision precision) { return m_fractPrecision = precision; }
    FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }

    // SetFractionalBarType missing?
    FractionBarType GetFractionalBarType() const { return m_barType; }

    Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }

    Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    size_t GetThousandSeparatorSize() const { return sizeof(m_thousandsSeparator); }

    Utf8CP SetUomSeparator(Utf8CP sep) { m_uomSeparator = Utf8String(sep); return m_uomSeparator.c_str(); }
    Utf8CP GetUomSeparator(Utf8CP def = nullptr) const { return (nullptr == def)?  m_uomSeparator.c_str() : def; }

    int SetMinWidth(int wid) { return m_minWidth = wid; }
    int GetMinWidth() { return m_minWidth; }

    Utf8Char SetStopSeparator(Utf8Char sep) { return m_statSeparator = sep; }
    Utf8Char GetStopSeparator() const { return m_statSeparator; }

    //======================================
    // Presentation Type Setters/Getters
    //======================================
    // IsDecimal missing?
    bool IsFractional() const { return m_presentationType == PresentationType::Fractional; }
    bool IsScientific() const { return (m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm); }
    // Not sure if IsStop/IsStop100/IsStop1000 getters would be needed.


    //======================================
    // Show Sign Option Setters/Getters
    //======================================
    // SetNoSign missing?
    // IsNotSign missing?

    // SetOnlyNegative missing?
    bool IsOnlyNegative() const { return (m_signOption == ShowSignOption::OnlyNegative); }

    // SetSignAlways missing?
    bool IsSignAlways() const { return (m_signOption == ShowSignOption::SignAlways); }

    void SetNegativeParentheses() { m_signOption = ShowSignOption::NegativeParentheses; }
    bool IsNegativeParentheses() const { return (m_signOption == ShowSignOption::NegativeParentheses); }

    //======================================
    // Format Traits Bit Setters/Getters
    //======================================
    bool CheckTraitsBit(FormatTraits word, FormatTraits bit) const { return ((static_cast<int>(word) & static_cast<int>(bit)) != 0); }
    bool CheckTraitsBit(FormatTraits bit) const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(bit)) != 0); }
    //UNITS_EXPORT void SetTraitsBit(bool set, FormatTraits bit, FormatTraits* word = nullptr);
    UNITS_EXPORT void TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose=false) const;
    UNITS_EXPORT static FormatTraits SetTraitsBit(FormatTraits bit, FormatTraits traits, bool set);
    UNITS_EXPORT static void TraitsBitToJsonKey(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits traits);

    void SetUseLeadingZeroes(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::LeadingZeroes, m_formatTraits, use); }
    bool IsUseLeadingZeroes() const { return CheckTraitsBit(FormatTraits::LeadingZeroes); }

    void SetKeepTrailingZeroes(bool keep) {m_formatTraits = SetTraitsBit(FormatTraits::TrailingZeroes, m_formatTraits, keep);}
    bool IsKeepTrailingZeroes() const { return CheckTraitsBit(FormatTraits::TrailingZeroes);}

    void SetKeepDecimalPoint(bool use) {m_formatTraits = SetTraitsBit(FormatTraits::KeepDecimalPoint, m_formatTraits, use);}
    bool IsKeepDecimalPoint() const { return CheckTraitsBit(FormatTraits::KeepDecimalPoint); }

    void SetKeepSingleZero(bool use) {m_formatTraits = SetTraitsBit(FormatTraits::KeepSingleZero, m_formatTraits, use);}
    bool IsKeepSingleZero() const { return CheckTraitsBit(FormatTraits::KeepSingleZero); }

    void SetExponentZero(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ExponentZero, m_formatTraits, use); }
    bool IsExponentZero() const { return CheckTraitsBit(FormatTraits::ExponentZero); }

    void SetZeroEmpty(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ZeroEmpty, m_formatTraits, use); }
    bool IsZeroEmpty() const { return CheckTraitsBit(FormatTraits::ZeroEmpty); }

    void SetUse1000Separator(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::Use1000Separator, m_formatTraits, use); }
    bool IsUse1000Separator() const { return CheckTraitsBit(FormatTraits::Use1000Separator); }

    void SetApplyRounding(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ApplyRounding, m_formatTraits, use); }
    bool IsApplyRounding() const { return CheckTraitsBit(FormatTraits::ApplyRounding); }

    void SetFractionDash(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::FractionDash, m_formatTraits, use); }
    bool IsFractionDash() const { return CheckTraitsBit(FormatTraits::FractionDash); }

    void SetUseFractSymbol(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::UseFractSymbol, m_formatTraits, use); }
    bool IsUseFractSymbol() const { return CheckTraitsBit(FormatTraits::UseFractSymbol); }

    void SetAppendUnit(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::AppendUnitName, m_formatTraits, use); }
    bool IsAppendUnit() const { return CheckTraitsBit(FormatTraits::AppendUnitName); }

    bool IsInsertSeparator(bool confirm) const { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }

    //======================================
    // DecimalPrecision Setters/Getters
    //======================================
    bool IsPrecisionZero() const { return (m_decPrecision == DecimalPrecision::Precision0); }

    //======================================
    // Formatting Methods
    //======================================
    // The caller provided buffer must be at least 9 byte long with the 9th byte for the null
    // terminator.Returns the number of bytes that were not populated; in case of success
    // the function will return 0.
    UNITS_EXPORT int FormatBinaryByte(unsigned char n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT int FormatBinaryShort(short int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryInt(int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryDouble(double x, Utf8P bufOut, int bufLen, bool useSeparator);

    UNITS_EXPORT int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const;

    // Returns the number of bytes written.
    UNITS_EXPORT int FormatInteger(int n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT Utf8String FormatInteger(int ival);
    // Formats an integer and zero pads the formatted string with zeros up to length == minSize.
    UNITS_EXPORT Utf8String FormatIntegerToString(int n, int minSize) const;
    UNITS_EXPORT int static FormatIntegerSimple(int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero);

    UNITS_EXPORT size_t FormatDoubleBuf(double dval, Utf8P buf, size_t bufLen, int prec = -1, double round = -1.0) const;
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0) const;
    UNITS_EXPORT Utf8String FormatRoundedDouble(double dval, double round);
    UNITS_EXPORT static Utf8String StdFormatDouble(Utf8CP stdName, double dval, int prec = -1, double round = -1.0);

    UNITS_EXPORT static Utf8String StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit = nullptr, Utf8CP space = nullptr, Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space="", int prec = -1, double round = -1.0);

    UNITS_EXPORT Utf8String ByteToBinaryText(unsigned char n);
    UNITS_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
    UNITS_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
    UNITS_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);

    UNITS_EXPORT Json::Value ToJson(bool verbose) const;
    UNITS_EXPORT Json::Value JsonFormatTraits(bool verbose) const;
    UNITS_EXPORT FormatTraits TraitsFromJson(JsonValueCR jval);
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  06/2017
//=======================================================================================
struct UnitProxy
    {
private:
    BEU::UnitCP mutable m_unit;
    Utf8String mutable m_unitName;
    Utf8String mutable m_unitLabel;

public:
    void Clear() { m_unit = nullptr; m_unitName = Utf8String((Utf8CP)nullptr); m_unitLabel = Utf8String((Utf8CP)nullptr); }
    UnitProxy():m_unit(nullptr), m_unitName((Utf8CP)nullptr), m_unitLabel((Utf8CP)nullptr){}       
    UNITS_EXPORT UnitProxy(Utf8CP name, Utf8CP label = nullptr);
    UnitProxy(UnitProxyCR other)
        {
        m_unit = other.m_unit;
        m_unitName = Utf8String(other.m_unitName.c_str());
        m_unitLabel = Utf8String(other.m_unitLabel.c_str());
        }
    void Copy(UnitProxyCP other)
        {
        if (nullptr == other)
            Clear();
        else
            {
            m_unit = other->m_unit;
            m_unitName = Utf8String(other->m_unitName.c_str());
            m_unitLabel = Utf8String(other->m_unitLabel.c_str());
            }
        }
    UNITS_EXPORT UnitProxy(Json::Value jval);
    UNITS_EXPORT void LoadJson(Json::Value jval) const;
    UNITS_EXPORT bool Reset() const;
    UNITS_EXPORT bool SetName(Utf8CP name);
    UNITS_EXPORT bool SetUnit(BEU::UnitCP unit);
    Utf8CP GetLabel() const { return m_unitLabel.c_str(); }
    Utf8CP SetLabel(Utf8CP lab) { m_unitLabel = Utf8String(lab);  return m_unitLabel.c_str(); }
    Utf8CP GetName() const { return m_unitName.c_str(); }
    BEU::UnitCP GetUnit() const { return m_unit; }
    UNITS_EXPORT Json::Value ToJson() const;
    bool IsEmpty() const { return m_unitName.empty(); }
    UNITS_EXPORT bool IsIdentical(UnitProxyCR other) const;
    };


//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  06/2017
//=======================================================================================
struct UnitProxySet
{
private:
    bvector<UnitProxy> mutable m_proxys;
    BEU::UnitRegistry* m_unitReg = &BEU::UnitRegistry::Get();
    int mutable m_resetCount;

    UNITS_EXPORT int Validate() const;
    bool IsConsistent();
    
    size_t GetSize() const { return m_proxys.size(); }

public:
    UnitProxySet(int size)
        {
        m_proxys.resize(size);
        m_proxys.insert(m_proxys.begin(), size, UnitProxy());
        m_resetCount = 0;
        }
    UnitProxySet (UnitProxySetCP other)
        {
        m_proxys.resize(other->GetSize());
        m_resetCount = 0;
        for (size_t i = 0; i < m_proxys.size(); i++)
            {
            m_proxys[i].Copy(other->GetProxy(i));
            }
        }
    void Copy(UnitProxySetCR other)
        {
        m_proxys.resize(other.GetSize());
        m_resetCount = 0;
        for (size_t i = 0; i < m_proxys.size(); i++)
            {
            m_proxys[i].Copy(other.GetProxy(i));
            }
        }
    UNITS_EXPORT size_t UnitCount() const;
    int GetResetCount() const { return m_resetCount; }
    bool IsIndexCorrect(size_t indx) const { return indx < m_proxys.size(); }
    UnitProxyCP GetProxy(size_t indx) const { return (indx < m_proxys.size()) ? &m_proxys[indx] : nullptr; }
    void Clear() 
        { 
        for (int i = 0; IsIndexCorrect(i); ++i)
            {
            m_proxys[i].Clear();
            }
        m_unitReg = &BEU::UnitRegistry::Get(); 
        }
    BEU::UnitCP GetUnit(size_t indx) const { Validate();  return IsIndexCorrect(indx) ? m_proxys[indx].GetUnit() : nullptr; }
    Utf8CP GetUnitName(size_t indx, Utf8CP subst=nullptr) const { return  IsIndexCorrect(indx) ? m_proxys[indx].GetName() : subst; }
    Utf8CP GetUnitLabel(size_t indx, Utf8CP subst = nullptr) const { return IsIndexCorrect(indx) ? m_proxys[indx].GetLabel() : subst; }
    Utf8CP SetUnitLabel(size_t indx, Utf8CP unitLabel) { return IsIndexCorrect(indx) ? m_proxys[indx].SetLabel(unitLabel) : nullptr; }
    bool SetUnit(size_t indx, BEU::UnitCP unitP) { return IsIndexCorrect(indx) ?  m_proxys[indx].SetUnit(unitP) : false; }
    bool SetUnitName(size_t indx, Utf8CP unitName) const { return IsIndexCorrect(indx) ? m_proxys[indx].SetName(unitName) : false; }
    UNITS_EXPORT Json::Value ToJson(bvector<Utf8CP> keyNames) const;
    UNITS_EXPORT bool IsIdentical(UnitProxySetCR other) const;
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
protected:
    static const size_t  indxMajor  = 0;
    static const size_t  indxMiddle = 1;
    static const size_t  indxMinor  = 2;
    static const size_t  indxSub    = 3;
    static const size_t  indxInput  = 4;
    static const size_t  indxLimit  = 5;
    size_t m_ratio[indxSub];
    //BEU::UnitCP m_units[indxLimit];
    UnitProxySet m_unitProx = UnitProxySet(indxLimit);
    //Utf8CP m_unitLabel[indxLimit];
    FormatProblemDetail m_problem;
    CompositeSpecType m_type;
    bool m_includeZero; // Not currently used in the formatting code.
    Utf8String m_spacer;
    void SetUnitLabel(int index, Utf8CP label);
    size_t UnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    size_t UnitRatio(size_t uppIndx, size_t lowIndx);
    void ResetType() { m_type = CompositeSpecType::Undefined; }
    BEU::UnitCP GetUnit(size_t indx) const { return m_unitProx.GetUnit(indx); }
    bool SetInputUnit(BEU::UnitCP inputUnit) {return m_unitProx.SetUnit(indxInput, inputUnit); }
    void SetUnitRatios();
    bool SetUnitNames(Utf8CP MajorUnit, Utf8CP MiddleUnit=nullptr, Utf8CP MinorUnit = nullptr, Utf8CP SubUnit = nullptr);
    Utf8CP GetUnitName(size_t indx, Utf8CP substitute) const { return m_unitProx.GetUnitName(indx, substitute); }
    Utf8String GetEffectiveLabel(size_t indx, Utf8CP substitute) const 
        { 
        return  m_unitProx.GetUnitLabel(indx, m_unitProx.GetUnitName(indx));
        }
    //size_t GetRightmostRatioIndex();
    BEU::UnitCP GetSmallestUnit() const;

public:
    UNITS_EXPORT void Init();
    UNITS_EXPORT void Clone(CompositeValueSpecCR other);
   // UNITS_EXPORT CompositeValueSpec(size_t MajorToMiddle, size_t MiddleToMinor=0, size_t MinorToSub=0);
    CompositeValueSpec() { Init(); };
    UNITS_EXPORT CompositeValueSpec(CompositeValueSpecCP other);
    UNITS_EXPORT CompositeValueSpec(CompositeValueSpecCR other);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCP MajorUnit, BEU::UnitCP MiddleUnit=nullptr, BEU::UnitCP MinorUnit=nullptr, BEU::UnitCP subUnit = nullptr);
    UNITS_EXPORT CompositeValueSpec(Utf8CP MajorUnit, Utf8CP MiddleUnit = nullptr, Utf8CP MinorUni = nullptr, Utf8CP subUnit = nullptr);
    UNITS_EXPORT void SetUnitLabels(Utf8CP MajorLab, Utf8CP MiddleLab = nullptr, Utf8CP MinorLab = nullptr, Utf8CP SubLab = nullptr);
    UNITS_EXPORT Utf8String GetMajorLabel(Utf8CP substitute) const { return GetEffectiveLabel(indxMajor, substitute); }
    UNITS_EXPORT Utf8String GetMiddleLabel(Utf8CP substitute) const { return GetEffectiveLabel(indxMiddle, substitute); }
    UNITS_EXPORT Utf8String GetMinorLabel(Utf8CP substitute) const { return GetEffectiveLabel(indxMinor, substitute); }
    UNITS_EXPORT Utf8String GetSubLabel(Utf8CP substitute) const { return GetEffectiveLabel(indxSub, substitute); }
    BEU::UnitCP GetMajorUnit() const { return m_unitProx.GetUnit(indxMajor); }
    BEU::UnitCP GetMiddleUnit() const { return m_unitProx.GetUnit(indxMiddle); }
    BEU::UnitCP GetMinorUnit() const { return m_unitProx.GetUnit(indxMinor); }
    BEU::UnitCP GetSubUnit() const { return m_unitProx.GetUnit(indxSub); }
    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
    bool IsProblem() const { return m_problem.IsProblem(); }
    bool NoProblem() const { return m_problem.NoProblem(); }
    size_t GetMajorToMiddleRatio() { return m_ratio[indxMajor]; }
    size_t GetMiddleToMinorRatio() { return m_ratio[indxMiddle]; }
    size_t GetMinorToSubRatio() { return m_ratio[indxMinor]; }
    UNITS_EXPORT size_t GetUnitCount() const { return m_unitProx.UnitCount(); }
    UNITS_EXPORT Utf8CP GetProblemDescription() const { return m_problem.GetProblemDescription().c_str(); }
    UNITS_EXPORT CompositeValue DecomposeValue(double dval, BEU::UnitCP uom = nullptr);
    CompositeSpecType GetType() const { return m_type; }
    Utf8String GetSpacer() const { return m_spacer; }
    Utf8String SetSpacer(Utf8CP spacer) { return m_spacer = spacer; }
    bool IsIncludeZero() const { return m_includeZero; }
    bool SetIncludeZero(bool incl) { return m_includeZero = incl; }
    UNITS_EXPORT Json::Value ToJson() const;
    UNITS_EXPORT void LoadJsonData(JsonValueCR jval);
    UNITS_EXPORT bool IsIdentical(CompositeValueSpecCR other) const;
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct CompositeValue
    {
private:
    bool m_negative;
    double m_parts[CompositeValueSpec::indxLimit];
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
    double SetInput(double dval)  { return m_parts[CompositeValueSpec::indxInput] = dval; }

    double GetMajor()  { return m_parts[CompositeValueSpec::indxMajor]; }
    double GetMiddle() { return m_parts[CompositeValueSpec::indxMiddle]; }
    double GetMinor()  { return m_parts[CompositeValueSpec::indxMinor]; }
    double GetSub()    { return m_parts[CompositeValueSpec::indxSub]; }
    double GetInput()  { return m_parts[CompositeValueSpec::indxInput]; }
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

        UNITS_EXPORT void LoadJson(Json::Value jval);
        UNITS_EXPORT void LoadJson(Utf8CP jsonString);

        //! Creates a new NamedFormatSpec with default values.
        NamedFormatSpec() : m_specType(FormatSpecType::Undefined) {m_problem.UpdateProblemCode(FormatProblemCode::NFS_Undefined);}
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Json::Value jval);
        UNITS_EXPORT NamedFormatSpec(Utf8CP jsonString);

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
        UNITS_EXPORT FormatUnitSet():m_formatSpec(nullptr), m_unit(nullptr), m_localCopy(false), m_problem(FormatProblemDetail(FormatProblemCode::NotInitialized)) {}
        UNITS_EXPORT FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit, bool cloneData = false);
        UNITS_EXPORT FormatUnitSet(Utf8CP formatName, Utf8CP unitName, bool cloneData = false);
        UNITS_EXPORT FormatUnitSet(BEU::UnitCP unit, Utf8CP formatName = nullptr);
        UNITS_EXPORT FormatUnitSet(FormatUnitSetCR other);
        UNITS_EXPORT FormatUnitSet(FormatUnitSetCP other);
        UNITS_EXPORT FormatUnitSet& operator=(const FormatUnitSet& other);

        UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const;

        //!The 'descriptor' argument is a text string in several formats as follows:
        //! for compatibility with the obsolete KOQ def's it may consist of only a unit name, e.g. FT
        //!    since FUS consists of two components: the reference to the Unit and a reference to a format specification,
        //!      the DefaultReal format will be used in this case
        //! the most commont descriptor consists of two names: the Unit Name and the Format Name, e.g. FT(real6)
        //! For supporting the usgae of comples Unit Names a "vertical bar" can be used as a separator
        //!   between the Unit Name and the Format Name as in FT|real6 The closing vertical bar delimiting
        //!    the Format Name is not required if the Format Name is terminated with the "end-of-line"
        //!     However, the Fomat Name can be also delimited by the "vertical bar" as in: FT|real6|
        //! The descriptor can be also a JSON-string enclosed in the "curvy brackets"
        //! There are two types of this JSON-string which are currently supported:
        //!  A short one consists of the unitName and formatName. The optional cloneData boolean value 
        //!    indicates that the format spec should be cloned into the newly created FUS. The default 
        //!      value of the cloneData parameter if "false"
        //! the long one consists of the unitName and formatSpec that contains the full description of this Spec
        UNITS_EXPORT FormatUnitSet(Utf8CP descriptor);

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
        UNITS_EXPORT void LoadJson(Json::Value jval);

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
        UNITS_EXPORT bool IsComparable(BEU::QuantityCR qty) const;
        UNITS_EXPORT bool IsUnitComparable(Utf8CP unitName) const;

        UNITS_EXPORT Json::Value ToJson(bool useAlias = true, bool verbose = false) const;
        UNITS_EXPORT Utf8String ToJsonString(bool useAlias = true, bool verbose = false) const;

        UNITS_EXPORT Json::Value FormatQuantityJson(BEU::QuantityCR qty, bool useAlias, Utf8CP space="") const;
        UNITS_EXPORT BEU::UnitCP ResetUnit();
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
    //UNITS_EXPORT static size_t AddFormatDef(bvector<NamedFormatSpecCP> *vec, NamedFormatSpecCP fmtP);

    static FormatUnitSet DefaultFUS(BEU::QuantityCR qty) { return FormatUnitSet(DefaultFormatSpec(), qty.GetUnit()); }

    //! Creates a new FormatUnitSet with the provided name and FUS, and adds it to the set.
    //! @remark There are no duplicate named FUSes allowed in a StdFormatSet.
    //! @param[in] fusR The FUS to copy the data for the to be created FUS
    //! @param[in] fusName Name of the FUS to create
    //! @return A pointer to the newly created FormatUnitSet if successful; otherwise, nullptr.
    UNITS_EXPORT static FormatUnitSetCP AddFUS(FormatUnitSetCR fusR, Utf8CP fusName);

    //! Creates a new FormatUnitSet with the provided name using the format and unit.
    //! @param[in] formatName Name of the NamedFormatSpec to add to the FormatUnitSet to be created.
    //! @param[in] unitName Name of the Unit to add to the FormatUnitSet to be created.
    //! @param[in] fusName Name of the FormatUnitSet to be created.
    //! @return A pointer to the newly created FormatUnitSet if successful; otherwise, nullptr.
    UNITS_EXPORT static FormatUnitSetCP AddFUS(Utf8CP formatName, Utf8CP unitName, Utf8CP fusName);

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

END_BENTLEY_FORMATTING_NAMESPACE
