/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingApi.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameterSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(QuantityTriadSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatName)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatNameMap)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValue)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValueSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatDictionary)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FactorPower)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatUnitSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatUnitGroup)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NamedFormatSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxySet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnitProxy)

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
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericFormatSpec
    {
    

private:
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Sientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParenths
    FormatTraits        m_formatTraits;          // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    FractionBarType     m_barType;               // oblique, horizontal, diagonal

    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
    Utf8String          m_uomSeparator;          // default separator between the number and UOM
    Utf8Char            m_statSeparator;         // default separator between parts of the stopping format
    int                 m_minWIdth;              // the minimum width of the field. It will be taken into account
                                                 // only if the overall length (width) of the text representing integer
                                                 // a number of or integer part of a real is shorter and needs to be augmented by
                                                 // insignificant zeroes. Blanks are not considered because aligning text
                                                 // with the boundaries of a virtual box is the responsibility of annotation layer

    double EffectiveRoundFactor(double rnd) const { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }
  
    UNITS_EXPORT void DefaultInit(size_t precision);
    UNITS_EXPORT void Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    UNITS_EXPORT double RoundedValue(double dval, double round) const;
    UNITS_EXPORT int TrimTrailingZeroes(Utf8P buf, int index) const;
    UNITS_EXPORT size_t InsertChar(Utf8P buf, size_t index, char c, int num) const;

public:
    NumericFormatSpec() { DefaultInit( FormatConstant::DefaultDecimalPrecisionIndex()); }
    NumericFormatSpec(size_t precision) { DefaultInit(precision); }
    NumericFormatSpec(NumericFormatSpecCP other) :
        m_roundFactor(other->m_roundFactor), m_presentationType(other->m_presentationType),
        m_signOption(other->m_signOption), m_formatTraits(other->m_formatTraits), m_decPrecision(other->m_decPrecision),
        m_fractPrecision(other->m_fractPrecision), m_decimalSeparator(other->m_decimalSeparator),
        m_thousandsSeparator(other->m_thousandsSeparator), m_barType(other->m_barType), m_uomSeparator(other->m_uomSeparator),
        m_statSeparator(other->m_statSeparator), m_minWIdth(other->m_minWIdth){}
    NumericFormatSpec(NumericFormatSpecCR other) :
        m_roundFactor(other.m_roundFactor), m_presentationType(other.m_presentationType),
        m_signOption(other.m_signOption), m_formatTraits(other.m_formatTraits), m_decPrecision(other.m_decPrecision),
        m_fractPrecision(other.m_fractPrecision), m_decimalSeparator(other.m_decimalSeparator),
        m_thousandsSeparator(other.m_thousandsSeparator), m_barType(other.m_barType), m_uomSeparator(other.m_uomSeparator),
        m_statSeparator(other.m_statSeparator), m_minWIdth(other.m_minWIdth) {}
    UNITS_EXPORT NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision);
    UNITS_EXPORT NumericFormatSpec(Json::Value jval);

    void SetFormatTraits(FormatTraits opt) { m_formatTraits = opt; }
    FormatTraits GetFormatTraits() const { return m_formatTraits; }   
    bool CheckTraitsBit(FormatTraits word, FormatTraits bit) const { return ((static_cast<int>(word) & static_cast<int>(bit)) != 0); }
    bool CheckTraitsBit(FormatTraits bit) const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(bit)) != 0); }
    //UNITS_EXPORT void SetTraitsBit(bool set, FormatTraits bit, FormatTraits* word = nullptr);
    UNITS_EXPORT void TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose=false) const;
    UNITS_EXPORT static FormatTraits SetTraitsBit(FormatTraits bit, FormatTraits traits, bool set);
    UNITS_EXPORT static void TraitsBitToJsonKey(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits traits);

    void SetKeepTrailingZeroes(bool keep) { m_formatTraits = SetTraitsBit(FormatTraits::TrailingZeroes, m_formatTraits, keep); }//SetTraitsBit(keep, FormatTraits::TrailingZeroes); }
    bool IsKeepTrailingZeroes() const { return CheckTraitsBit(FormatTraits::TrailingZeroes);}

    void SetUseLeadingZeroes(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::LeadingZeroes, m_formatTraits, use); }
    bool IsUseLeadingZeroes() const { return CheckTraitsBit(FormatTraits::LeadingZeroes); }

    void SetKeepDecimalPoint(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::KeepDecimalPoint, m_formatTraits, use);
        }
    bool IsKeepDecimalPoint() const { return CheckTraitsBit(FormatTraits::KeepDecimalPoint); }

    void SetKeepSingleZero(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::KeepSingleZero, m_formatTraits, use);
        }
    bool IsKeepSingleZero() const { return CheckTraitsBit(FormatTraits::KeepSingleZero); }

    void SetExponentZero(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ExponentZero, m_formatTraits, use); }
    bool IsExponentZero() const { return CheckTraitsBit(FormatTraits::ExponentZero); }

    void SetZeroEmpty(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ZeroEmpty, m_formatTraits, use); }
    bool IsZeroEmpty() const { return CheckTraitsBit(FormatTraits::ZeroEmpty); }

    void SetUse1000Separator(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::Use1000Separator, m_formatTraits, use); }
    bool IsUse1000Separator() const { return CheckTraitsBit(FormatTraits::Use1000Separator); }

    void SetApplyRounding(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::ApplyRounding, m_formatTraits, use); }
    bool IsApplyRounding() const { return CheckTraitsBit(FormatTraits::ApplyRounding); }

    void SetAppendUnit(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::AppendUnitName, m_formatTraits, use); }
    bool IsAppendUnit() const { return CheckTraitsBit(FormatTraits::AppendUnitName); }

    void SetFractionDash(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::FractionDash, m_formatTraits, use); }
    bool IsFractionDash() const { return CheckTraitsBit(FormatTraits::FractionDash); }

    void SetUseFractSymbol(bool use) { m_formatTraits = SetTraitsBit(FormatTraits::UseFractSymbol, m_formatTraits, use); }
    bool IsUseFractSymbol() const { return CheckTraitsBit(FormatTraits::UseFractSymbol); }

    bool IsInsertSeparator(bool confirm) const { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    void SetNegativeParentheses() { m_signOption = ShowSignOption::NegativeParentheses; }
    bool IsNegativeParentheses() const { return (m_signOption == ShowSignOption::NegativeParentheses); }
    bool IsOnlyNegative() const { return (m_signOption == ShowSignOption::OnlyNegative); }
    bool IsSignAlways() const { return (m_signOption == ShowSignOption::SignAlways); }
    bool IsFractional() const { return m_presentationType == PresentationType::Fractional; }
    bool IsScientific() const { return (m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm); }
    void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    DecimalPrecision GetDecimalPrecision() const { return m_decPrecision; }
    UNITS_EXPORT int GetDecimalPrecisionIndex(int prec) const;
    UNITS_EXPORT double GetDecimalPrecisionFactor(int prec) const;
    FractionalPrecision SetFractionaPrecision(FractionalPrecision precision) { return m_fractPrecision = precision; }
    FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }
    double SetRoundingFactor(double round) { return m_roundFactor = round; }
    double GetRoundingFactor() const { return m_roundFactor; }
    UNITS_EXPORT void SetPrecisionByValue(int prec);
    void SetPresentationType(PresentationType type) { m_presentationType = type; }
    PresentationType GetPresentationType() const { return m_presentationType; }
    void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
    int SetMinWidth(int wid) { return m_minWIdth = wid; }
    int GetMinWidth() { return m_minWIdth; }
    ShowSignOption GetSignOption() const { return m_signOption; }
    Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
    Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    Utf8CP   SetUomSeparator(Utf8CP sep) { m_uomSeparator = Utf8String(sep); return m_uomSeparator.c_str(); }
    Utf8CP   GetUomSeparator(Utf8CP def=nullptr) { return (nullptr == def)?  m_uomSeparator.c_str() : def; }
    Utf8Char SetStopSeparator(Utf8Char sep) { return m_statSeparator = sep; }
    Utf8Char GetStopSeparator() const { return m_statSeparator; }
    FractionBarType  GetFractionalBarType() const { return m_barType; }
    UNITS_EXPORT static double RoundDouble(double dval, double roundTo);
    UNITS_EXPORT static bool AcceptableDifference(double dval1, double dval2, double maxDiff); 
    bool IsPrecisionZero() const {    return (m_decPrecision == DecimalPrecision::Precision0);}
    UNITS_EXPORT int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const;
    
    UNITS_EXPORT int FormatInteger (int n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT int static FormatIntegerSimple (int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero);
    UNITS_EXPORT Utf8String FormatIntegerToString(int n, int minSize, bool padSpace) const;
    UNITS_EXPORT size_t FormatDoubleBuf(double dval, Utf8P buf, size_t bufLen, int prec = -1, double round = -1.0) const;
    
    UNITS_EXPORT static Utf8String StdFormatDouble(Utf8CP stdName, double dval, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantity(Utf8CP stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit=nullptr, Utf8CP space = "", Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0); 
    UNITS_EXPORT static Utf8String StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit = nullptr, Utf8CP space = "", Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantity(FormatUnitSetCR fus, BEU::QuantityCR qty);

    UNITS_EXPORT static Utf8String StdFormatQuantityTriad(Utf8CP stdName, QuantityTriadSpecP qtr,Utf8CP space, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0) const;
    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space="", int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatPhysValue(Utf8CP stdName, double dval, Utf8CP fromUOM, Utf8CP toUOM, Utf8CP toLabel, Utf8CP space, int prec = -1, double round = -1.0);

    UNITS_EXPORT static const NumericFormatSpecCP DefaultFormat();

    //FormatDoubleStd

    UNITS_EXPORT int FormatBinaryByte (unsigned char n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT int FormatBinaryShort (short int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryInt (int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryDouble (double x, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT static int RightAlignedCopy(Utf8P dest, int destLen, bool termZero, CharCP src, int srcLen);
    UNITS_EXPORT Utf8String FormatRoundedDouble(double dval, double round);
    UNITS_EXPORT Utf8String FormatInteger(int ival);
    UNITS_EXPORT Utf8String ByteToBinaryText(unsigned char n);
    UNITS_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
    UNITS_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
    UNITS_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);

    UNITS_EXPORT Json::Value ToJson(bool verbose) const;
    UNITS_EXPORT Json::Value JsonFormatTraits(bool verbose) const;
    UNITS_EXPORT FormatTraits TraitsFromJson(JsonValueCR jval);
    UNITS_EXPORT bool IsIdentical(NumericFormatSpecCR other) const;
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
    BEU::UnitRegistry* m_unitReg = &BEU::UnitRegistry::Instance();
    int mutable m_resetCount;

    UNITS_EXPORT int Validate() const;
    bool IsConsistent();
    UNITS_EXPORT size_t UnitCount() const;
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
    int GetResetCount() const { return m_resetCount; }
    bool IsIndexCorrect(size_t indx) const { return indx < m_proxys.size(); }
    UnitProxyCP GetProxy(size_t indx) const { return (indx < m_proxys.size()) ? &m_proxys[indx] : nullptr; }
    void Clear() 
        { 
        for (int i = 0; IsIndexCorrect(i); ++i)
            {
            m_proxys[i].Clear();
            }
        m_unitReg = &BEU::UnitRegistry::Instance(); 
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
    bool m_includeZero;
    Utf8String m_spacer;

    void SetUnitLabel(int index, Utf8CP label);
    size_t UnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    size_t UnitRatio(size_t uppIndx, size_t lowIndx);
    void ResetType() { m_type = CompositeSpecType::Undefined; }
    void Init();
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
    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
    bool IsProblem() const { return m_problem.IsProblem(); }
    bool NoProblem() const { return m_problem.NoProblem(); }
    size_t GetMajorToMiddleRatio() { return m_ratio[indxMajor]; }
    size_t GetMiddleToMinorRatio() { return m_ratio[indxMiddle]; }
    size_t GetMinorToSubRatio() { return m_ratio[indxMinor]; }
    UNITS_EXPORT Utf8CP GetProblemDescription() const;
    UNITS_EXPORT CompositeValue DecomposeValue(double dval, BEU::UnitCP uom = nullptr);
    UNITS_EXPORT Utf8String FormatValue(double dval, NumericFormatSpecP fmtP, Utf8CP uomName = nullptr);
    CompositeSpecType GetType() const { return m_type; }
    Utf8String GetSpacer() const { return m_spacer; }
    Utf8String SetSpacer(Utf8CP spacer) { return m_spacer = spacer; }
    bool IsIncludeZero() const { return m_includeZero; }
    bool SetIncludeZero(bool incl) { return m_includeZero = incl; }
    UNITS_EXPORT Json::Value ToJson() const;
    UNITS_EXPORT void LoadJsonData(JsonValueCR jval);
    UNITS_EXPORT bool IsIdentical(CompositeValueSpecCR other) const;
    };

struct CompositeValue
    {
private:
    double m_parts[CompositeValueSpec::indxLimit];
    FormatProblemDetail m_problem;

    void Init();
public:
    UNITS_EXPORT CompositeValue();
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
// Container for keeping together primary numeric, composite and other types of specs
//  and referrring them by the unique name. Name and at the valid numeric spec are required
//   for creating a valid instance of this class. Alias and composite spec are optional at the
// moment of creation but can be added later
// @bsiclass                                                    David.Fox-Rabinovitz  03/2017
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
        //UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr, CompositeValueSpecP compSpec = nullptr);
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
        UNITS_EXPORT NamedFormatSpec(Json::Value jval);
        Utf8CP SetAlias(Utf8CP alias) { m_alias = alias;  return m_alias.c_str(); }
        Utf8CP GetAlias() const { return m_alias.c_str(); }
        bool HasName(Utf8CP name) const { return 0 == BeStringUtilities::StricmpAscii(name, m_name.c_str()); }
        bool HasAlias(Utf8CP name) const { return 0 == BeStringUtilities::StricmpAscii(name, m_alias.c_str()); }
        
        Utf8CP GetName() const { return m_name.c_str(); };
        Utf8CP GetDescription() const { return m_description.c_str(); };
        Utf8CP SetDescription(Utf8CP descr) { m_description = descr;  return m_description.c_str(); };
        Utf8CP GetLabel() const { return m_displayLabel.c_str(); };
        Utf8CP SetLabel(Utf8CP label) { m_displayLabel = label;  return m_displayLabel.c_str(); };

        FormatSpecType  GetSpecType(){return m_specType;}
        bool HasComposite() const { return FormatSpecType::Composite == m_specType; }
        NumericFormatSpecCP GetNumericSpec() const { return &(this->m_numericSpec); }
        CompositeValueSpecCP GetCompositeSpec() const { return  (HasComposite() ? &m_compositeSpec : nullptr); }
        bool IsProblem() const { return m_problem.IsProblem(); }
        Utf8String GetProblemDescription() { return m_problem.GetProblemDescription(); }
        Utf8String GetNameAndAlias() const { return Utf8String(m_name) + Utf8String("(") + Utf8String(m_alias) + Utf8String(")"); };
        PresentationType GetPresentationType() const { return m_numericSpec.GetPresentationType(); }
        UNITS_EXPORT Json::Value ToJson(bool verbose) const;
        UNITS_EXPORT bool IsIdentical(NamedFormatSpec other) const;
    };

//=======================================================================================
// A pair of the unit reference and the format spec 
// @bsiclass                                                    David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct FormatUnitSet
    {
    private:
        NamedFormatSpecCP m_formatSpec;
        Utf8String  m_unitName;
        BEU::UnitCP m_unit;
        FormatProblemDetail m_problem;

    public:
        FormatUnitSet():m_formatSpec(nullptr), m_unit(nullptr), m_problem(FormatProblemDetail()) {}
        UNITS_EXPORT FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit);
        UNITS_EXPORT FormatUnitSet(Utf8CP formatName, Utf8CP unitName);
        FormatUnitSet(FormatUnitSetCR other)
            {
            m_formatSpec = other.m_formatSpec;
            m_unitName = other.m_unitName;
            m_unit = other.m_unit;
            m_problem = FormatProblemDetail(other.m_problem);
            }
        FormatUnitSet(FormatUnitSetCP other)
            {
            m_formatSpec = other->m_formatSpec;
            m_unitName = other->m_unitName;
            m_unit = other->m_unit;
            m_problem = FormatProblemDetail(other->m_problem);
            }

        UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const;
        UNITS_EXPORT FormatUnitSet(Utf8CP description);
        bool HasProblem() const { return m_problem.IsProblem(); }
        FormatProblemCode GetProblemCode() { return m_problem.GetProblemCode(); }
        Utf8String GetProblemDescription() const { return m_problem.GetProblemDescription(); }
        Utf8String GetUnitName() { return m_unitName; }
        UNITS_EXPORT Utf8String ToText(bool useAlias) const;
        BEU::UnitCP GetUnit() const { return m_unit; }
        UNITS_EXPORT bool IsComparable(BEU::QuantityCR qty);
        UNITS_EXPORT bool IsUnitComparable(Utf8CP unitName);
        UNITS_EXPORT Json::Value ToJson(bool useAlias) const;
        UNITS_EXPORT Utf8String ToJsonString(bool useAlias) const;
        UNITS_EXPORT Json::Value FormatQuantityJson(BEU::QuantityCR qty, bool useAlias) const;
        UNITS_EXPORT BEU::UnitCP ResetUnit();
        UNITS_EXPORT void LoadJsonData(Json::Value jval);
        UNITS_EXPORT bool IsIdentical(FormatUnitSetCR other) const;
    };


struct FormatUnitGroup
    {
    private:
        Utf8String m_name;
        bvector<FormatUnitSet> m_group;    // the first member is a persistence FUS and the following N-1 are presentation FUS's
        FormatProblemDetail m_problem;
    public:
        UNITS_EXPORT FormatUnitGroup(Utf8CP name, Utf8CP description);
        UNITS_EXPORT FormatUnitGroup(JsonValueCR jval);
        UNITS_EXPORT Json::Value ToJson(bool useAlias);
        UNITS_EXPORT Utf8String ToText(bool useAlias);
        bool HasProblem() const { return m_problem.IsProblem(); }
        FormatProblemCode GetProblemCode() { return m_problem.GetProblemCode(); }
        UNITS_EXPORT FormatUnitSetCP GetPersistenceFUS() const;
        UNITS_EXPORT size_t GetPresentationFUSCount();
        UNITS_EXPORT FormatUnitSetCP GetPresentationFUS(size_t index) const;
        UNITS_EXPORT bool IsIdentical(FormatUnitGroupCR other) const;
    };

struct StdFormatSet
    {
private:
    bvector<NamedFormatSpecCP> m_formatSet;    // core + app
    bvector<NamedFormatSpecCP> m_customSet;    // user

    NumericFormatSpecCP AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, Utf8CP alias = nullptr);
    NumericFormatSpecCP AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, CompositeValueSpecCR compS, Utf8CP alias = nullptr);
  

    UNITS_EXPORT size_t StdInit();
    UNITS_EXPORT size_t CustomInit();
    StdFormatSet() { StdInit(); CustomInit(); }
    static StdFormatSetP Set();

public:
    NumericFormatSpecCP AddCustomFormat(Utf8CP jsonString);
    //static StdFormatSetP GetStdSet() { return Set(); }
    UNITS_EXPORT static NumericFormatSpecCP DefaultDecimal();
    static NamedFormatSpecCP DefaultFormatSpec() { return FindFormatSpec(FormatConstant::DefaultFormatName()); }
    static size_t GetFormatSetSize() { return Set()->m_formatSet.size(); }
    UNITS_EXPORT static NumericFormatSpecCP GetNumericFormat(Utf8CP name);
    UNITS_EXPORT static NamedFormatSpecCP FindFormatSpec(Utf8CP name);
    UNITS_EXPORT static bvector<Utf8CP> StdFormatNames(bool useAlias);
    UNITS_EXPORT static Utf8String StdFormatNameList(bool useAlias);
    UNITS_EXPORT static Utf8String CustomNameList(bool useAlias);
    UNITS_EXPORT static bool AreSetsIdentical();
    UNITS_EXPORT size_t GetFormatCount() { return m_formatSet.size(); }
    UNITS_EXPORT size_t GetCustomCount() { return m_customSet.size(); }
    //UNITS_EXPORT static size_t AddFormatDef(bvector<NamedFormatSpecCP> *vec, NamedFormatSpecCP fmtP);

    static FormatUnitSet DefaultFUS(BEU::QuantityCR qty) { return FormatUnitSet(DefaultFormatSpec(), qty.GetUnit()); }
   // UNITS_EXPORT bvector<Json::Value> ToJson();
    };

//=======================================================================================
//! A class for breaking a given double precision number into 2 or 3 sub-parts defined by their ratios
//! Can be used for presenting angular measurement in the form of Degrees, Minutes and Seconds or 
//!   length in a form of yards, feet and inches. The parts are called top, middle and low and two ratios between
//!     top and middle and middle and low define the breakup details. The top is always supposed to be an integer
//!  represented as double precision number. Both ratios are assumed to be positive integer numbers for avoiding incorrect
//!   results. If a negative number is provided for either of two ratios its sign will be dropped. 
//!  
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericTriad
    {
    protected:
        double m_dval;            // we keep the originally submitted value for its sign and possible reverse operations
        double m_topValue;
        double m_midValue;
        double m_lowValue;
        //DecimalPrecision m_decPrecision;
        size_t    m_topToMid;
        size_t    m_midToLow;
        bool   m_init;
        bool   m_midAssigned;
        bool   m_lowAssigned;
        bool   m_negative;
        UNITS_EXPORT void Convert();
        UNITS_EXPORT void SetValue(double dval);
        UNITS_EXPORT NumericTriad();
        UNITS_EXPORT static bool IsNameNullOrEmpty(Utf8CP name) { return (nullptr == name || strlen(name) == 0); }
        size_t SetTopToMid(size_t value) { return  m_topToMid = value; }
        size_t SetMidToLow(size_t value) { return  m_midToLow = value; }
        void SetInit() { m_init = true; }
    public:
        UNITS_EXPORT NumericTriad(double dval, size_t topMid, size_t midLow);

        double GetWhole() { return m_negative ? -m_dval : m_dval; }
        void ProcessValue(double dval) { SetValue(dval);  Convert(); }
        void SetRatio(int topToMid, int midToLow) { m_topToMid = topToMid; m_midToLow = midToLow; }
        //void SetPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
        double GetTopValue() { return m_topValue; }
        double GetMidValue() { return m_midValue; }
        double GetlowValue() { return m_lowValue; }
        UNITS_EXPORT Utf8String FormatWhole(DecimalPrecision prec);
        UNITS_EXPORT Utf8String FormatTriad(Utf8CP topName, Utf8CP midName, Utf8CP lowName, Utf8CP space, int prec, bool fract = false, bool includeZero = false);
    };

struct QuantityTriadSpec : NumericTriad
    {
    private:
        BEU::QuantityCP m_quant;
        BEU::UnitCP m_topUnit;
        BEU::UnitCP m_midUnit;
        BEU::UnitCP m_lowUnit;
        Utf8CP m_topUnitLabel;
        Utf8CP m_midUnitLabel;
        Utf8CP m_lowUnitLabel;
        bool m_includeZero;
        FormatProblemDetail m_problem;

        void Init(bool incl0);
        QuantityTriadSpec();
        bool ValidatePhenomenaPair(BEU::PhenomenonCP srcPhen, BEU::PhenomenonCP targPhen);
        // UNITS_EXPORT static size_t UnitRatio(BEU::UnitCP un1, BEU::UnitCP un2);
    public:

        Utf8CP SetTopUnitLabel(Utf8CP symbol) { return m_topUnitLabel = symbol; }
        Utf8CP GetTopUnitLabel() { return m_topUnitLabel; }
        Utf8CP SetMidUnitLabel(Utf8CP symbol) { return m_midUnitLabel = symbol; }
        Utf8CP GetMidUnitLabel() { return m_midUnitLabel; }
        Utf8CP SetLowUnitLabel(Utf8CP symbol) { return m_lowUnitLabel = symbol; }
        Utf8CP GetLowUnitLabel() { return m_lowUnitLabel; }
        bool SetIncludeZero(bool val) { return m_includeZero = val; }
        bool GetIncludeZero() const { return m_includeZero; }
        UNITS_EXPORT static size_t UnitRatio(BEU::UnitCP un1, BEU::UnitCP un2);
        UNITS_EXPORT QuantityTriadSpec(BEU::QuantityCR qty, BEU::UnitCP topUnit, BEU::UnitCP midUnit = nullptr, BEU::UnitCP lowUnit = nullptr, bool incl0 = false);

        bool IsProblem() { return m_problem.IsProblem(); }
        FormatProblemCode GetProblemCode() { return m_problem.GetProblemCode(); }
        bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
        UNITS_EXPORT Utf8String FormatQuantTriad(Utf8CP space, int prec, bool fract = false, bool includeZero = false);
        Utf8CP GetTopUOM() { return (nullptr == m_topUnit) ? FormatConstant::EmptyString() : m_topUnit->GetName(); }
        Utf8CP GetMidUOM() { return (nullptr == m_midUnit) ? FormatConstant::EmptyString() : m_midUnit->GetName(); }
        Utf8CP GetLowUOM() { return (nullptr == m_lowUnit) ? FormatConstant::EmptyString() : m_lowUnit->GetName(); }
        BEU::UnitCP GetTopUnit() { return m_topUnit; }
        BEU::UnitCP GetMidUnit() { return m_midUnit; }
        BEU::UnitCP GetLowUnit() { return m_lowUnit; }
    };

// Format parameter traits
//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatParameter
{
private:
    Utf8String m_paramName;
    ParameterCategory m_category;
    ParameterCode m_paramCode;
    ParameterDataType m_paramType;
    int m_intValue;            // for binary flags and other integer values

public:
    FormatParameter(Utf8StringCR name, ParameterCategory cat, ParameterCode code, ParameterDataType type) :
        m_paramName(name), m_category(cat), m_paramCode(code), m_paramType(type), m_intValue(0) {}

    FormatParameter(Utf8StringCR name, ParameterCategory cat, ParameterCode code, int bitFlag):
        m_paramName(name), m_category(cat), m_paramCode(code), m_paramType(ParameterDataType::BitFlag), m_intValue(bitFlag) {}

    Utf8StringCR GetName() { return m_paramName; }
    int CompareName(Utf8StringCR other) { return strcmp(m_paramName.c_str(), other.c_str()); }
    ParameterCategory GetCategory() { return m_category; }
    CharCP GetCategoryName() { return Utils::GetParameterCategoryName(m_category); }
    ParameterCode GetParameterCode() { return m_paramCode; }
    size_t GetParameterCodeValue() { return (size_t)m_paramCode; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatDictionary //: FormatConstant
    {
private:
    bvector<FormatParameter> m_paramList;

    UNITS_EXPORT void InitLoad();
    Utf8StringP ParameterValuePair(Utf8StringCR name, Utf8StringCR value, char quote, Utf8StringCR prefix);

public:

    FormatDictionary() { InitLoad(); }
    int GetCount() { return (int)m_paramList.size(); }
    void AddParameter(FormatParameterCR par) { m_paramList.push_back(par); return; }
    UNITS_EXPORT FormatParameterP FindParameterByName(Utf8StringCR paramName);
    UNITS_EXPORT FormatParameterP FindParameterByCode(ParameterCode paramCode);
    UNITS_EXPORT FormatParameterP GetParameterByIndex(int index);
    UNITS_EXPORT Utf8StringCR CodeToName(ParameterCode paramCode);
    UNITS_EXPORT Utf8String SerializeFormatDefinition(NamedFormatSpecCP format);
    };

struct FormatStopWatch
    {
private:
    std::chrono::steady_clock::time_point m_start;
    double m_lastInterval;
    double m_totalElapsed;
    size_t m_lastAmount;
    size_t m_totalAmount;
   
public:
    UNITS_EXPORT FormatStopWatch();
    UNITS_EXPORT Utf8String LastIntervalMetrics(size_t amount);
    UNITS_EXPORT Utf8String LastInterval(double factor);
    };

//QuantityFraction
//struct QuantityFraction 
//    {
//private:
//    NumSequenceTraits m_traits;
//    double m_dval;    // collected value
//    int m_ival;
//    int m_startIndx;  // indicates the location of the first char in the numeric sequence (after skipping blanks)
//    int m_len;        // the length of the inspected numeric sequence
//    Utf8String m_uomName;
//    BEU::UnitCP m_unit;
//    FormatProblemDetail m_problem;
//    void Init()
//        {
//        m_traits = NumSequenceTraits::None;
//        m_dval = 0.0;
//        m_ival = 0;
//        m_startIndx = -1;
//        m_len = 0;
//        m_uomName = "";
//        m_unit = nullptr;
//        m_problem = FormatProblemDetail();
//        }
//
//public:
//    QuantityFraction() { Init();  }
//    UNITS_EXPORT void SetSigned(bool use);
//    bool IsSigned() const { return ((static_cast<int>(m_traits) & static_cast<int>(NumSequenceTraits::Signed)) != 0); }
//    UNITS_EXPORT void SetDecPoint(bool use);
//    bool HasDecPoint() const { return ((static_cast<int>(m_traits) & static_cast<int>(NumSequenceTraits::DecPoint)) != 0); }
//    UNITS_EXPORT void SetExponent(bool use);
//    bool HasExponent() const { return ((static_cast<int>(m_traits) & static_cast<int>(NumSequenceTraits::Exponent)) != 0); }
//    UNITS_EXPORT void SetUom(bool use);
//    bool HasUom() const { return ((static_cast<int>(m_traits) & static_cast<int>(NumSequenceTraits::Uom)) != 0); }
//    Utf8String SetUomName(Utf8String name) { return m_uomName = name; }
//    Utf8String GetUomName() { return m_uomName; }
//    BEU::UnitCP GetUnit() { return BEU::UnitRegistry::Instance().LookupUnit(m_uomName.c_str()); }
//    UNITS_EXPORT void Detect(Utf8CP txt);
//    bool IsProblem() { return m_problem.IsProblem(); }
//    FormatProblemCode GetProblemCode() { return m_problem.GetProblemCode(); }
//    bool UpdateProblemCode(FormatProblemCode code) { return m_problem.UpdateProblemCode(code); }
//    Utf8String GetProblemDescription() { return m_problem.GetProblemDescription(); }
//    double GetRealValue() { return m_dval; }
//    int GetIntValue() { return m_ival; }
//    int GetStartIndex() { return m_startIndx; }
//    int GetLength() { return m_len; }
//    int GetNextindex() { return m_startIndx + m_len; }
//
//    };


END_BENTLEY_FORMATTING_NAMESPACE
