/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Formatting.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include "RefCounted.h"
#include "ValueFormat.r.h"

BENTLEY_NAMESPACE_TYPEDEFS(Formatting);

BEGIN_BENTLEY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormat)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameterSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)

typedef RefCountedPtr<NumericFormat>   NumericFormatBPtr;

enum class ParameterCode
    {
    NoSign = 101,
    OnlyNegative = 102,
    SignAlways = 103,
    NegativeParenths = 104,
    Decimal = 151,
    Fractional = 152,
    Sientific = 153,
    ScientificNorm = 154,
    NoZeroes = 201,
    LeadingZeroes = 202,
    TrailingZeroes = 203,
    BothZeroes = 204,
    ShowDotZero = 205, // with precision 0 by default only integer part will be shows. This option will force .0 after the int part 
    Replace0Empty = 206,  // formatter will return the empy string if the result is 0
    DecPrec0 =  300,
    DecPrec1 =  301,
    DecPrec2 =  302,
    DecPrec3 =  303,
    DecPrec4 =  304,
    DecPrec5 =  305,
    DecPrec6 =  306,
    DecPrec7 =  307,
    DecPrec8 =  308,
    DecPrec9 =  309,
    DecPrec10 = 310,
    DecPrec11 = 311,
    DecPrec12 = 312,
    FractPrec1 = 331,
    FractPrec2 = 332,
    FractPrec4 = 333,
    FractPrec8 = 334,
    FractPrec16 = 335,
    FractPrec32 = 336,
    FractPrec64 = 337,
    FractPrec128 = 338,
    FractPrec256 = 339,
    DecimalComma = 351,
    DecimalPoint = 352,
    DecimalSepar = 353,
    ThousandSepComma = 354,
    ThousandSepPoint = 355,
    ThousandsSepar = 356,
    RoundUp = 401,
    RoundDown = 402,
    RoundToward0 = 403,
    RoundAwayFrom0 = 404,
    FractBarHoriz = 451,
    FractBarOblique = 452,
    FractBarDiagonal = 453,
    AngleRegular = 501,
    AngleDegMin = 502,
    AngleDegMinSec = 503,
    PaddingSymbol = 504,
    BoudaryLen = 601,
    CenterAlign = 620,
    LeftAlign = 621,
    RightAlign = 622,
    MapName = 651,
    };

enum class ParameterDataType
{
    Flag = 0,
    Integer = 1,
    Double = 2,
    Symbol = 3,
    String = 4
};

enum class RoundingType
{
    RoundUp = 1,
    RoundDown = 2,
    RoundToward0 = 3,
    RoundAwayFrom0 = 4
};

enum class FractionBarType
{
    Oblique = 0,
    Horizontal = 1,
    FractBarDiagonal = 2
};

enum class AngleFormatType
{
    AngleRegular = 0,
    AngleDegMin = 1,
    AngleDegMinSec = 2
};

enum class FieldAlignment
{
    Center = 0,
    Left = 1,
    Right = 2
};

enum class ShowSignOption
{
    NoSign = 0,
    OnlyNegative = 1,
    SignAlways = 2,
    NegativeParentheses = 3
};

enum class PresentationType
{
    Decimal = 1,
    Fractional = 2,
    Scientific = 3,      // scientific with 1 digit before the decimal point
    ScientificNorm = 4   // normalized scientific when Mantissa is < 1
};

enum class ZeroControl
{
    NoZeroes = 0,
    LeadingZeroes = 1,
    TrailingZeroes = 2,
    BothZeroes = 3
};

enum class DecimalPrecision
{
    Precision0 = 0,
    Precision1 = 1,
    Precision2 = 2,
    Precision3 = 3,
    Precision4 = 4,
    Precision5 = 5,
    Precision6 = 6,
    Precision7 = 7,
    Precision8 = 8,
    Precision9 = 9,
    Precision10 = 10,
    Precision11 = 11,
    Precision12 = 12
};

enum class FractionalPrecision
{
    Whole = 0,       //!< Ex. 30
    Half = 1,        //!< Ex. 30 1/2
    Quarter = 2,      //!< Ex. 30 1/4
    Eighth = 3,      //!< Ex. 30 1/8
    Sixteenth = 4,      //!< Ex. 30 1/16
    Over_32 = 5,      //!< Ex. 30 1/32
    Over_64 = 6,      //!< Ex. 30 1/64
    Over_128 = 7,      //!< Ex. 30 1/128
    Over_256 = 8,      //!< Ex. 30 1/256
};

enum class ParameterCategory
{
    DataType = 1,
    Sign = 2,
    Presentation = 3,
    Zeroes = 4,
    DecPrecision = 6,
    FractPrecision = 7,
    RoundType = 8,
    FractionBar = 9,
    AngleFormat = 10,
    Alignment = 11,
    Separator = 12,
    Padding = 13,
    Mapping = 14
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NumericFormat
{
private:
    PresentationType    m_presentationType;      // Decimal, Fractional, Sientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParenths
    ZeroControl         m_ZeroControl;           // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    bool                m_showDotZero;
    bool                m_replace0Empty;
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    bool                m_useThousandsSeparator; // UseThousandSeparator
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor

public:

    int PrecisionValue() const;
    double PrecisionFactor() const;
    DecimalPrecision ConvertToPrecision(int num);

    NumericFormat()
        {
        m_presentationType = PresentationType::Decimal;
        m_signOption = ShowSignOption::OnlyNegative;
        m_ZeroControl = ZeroControl::NoZeroes;
        m_showDotZero = false;
        m_replace0Empty = false;
        m_decPrecision = DecimalPrecision::Precision6;
        m_fractPrecision = FractionalPrecision::Sixteenth;
        m_useThousandsSeparator = false;
        m_decimalSeparator = '.';
        m_thousandsSeparator = ',';
        }

BENTLEYDLL_EXPORT void SetPresentationType(PresentationType type) { m_presentationType = type; }
BENTLEYDLL_EXPORT PresentationType GetPresentationType() const { return m_presentationType; }
BENTLEYDLL_EXPORT void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
BENTLEYDLL_EXPORT ShowSignOption GetSignOption() const { return m_signOption; }
BENTLEYDLL_EXPORT void setZeroControl(ZeroControl opt) { m_ZeroControl = opt; }
BENTLEYDLL_EXPORT ZeroControl getZeroControl() const { return m_ZeroControl; }
BENTLEYDLL_EXPORT bool getShowDotZero() { return m_showDotZero; }
BENTLEYDLL_EXPORT bool getReplace0Empty() const { return m_replace0Empty; }
BENTLEYDLL_EXPORT bool setShowDotZero(bool set) { return m_showDotZero = set; }
BENTLEYDLL_EXPORT bool setReplace0Empty(bool set) { return m_replace0Empty = set; }
BENTLEYDLL_EXPORT FractionalPrecision SetfractionaPrecision(FractionalPrecision precision) { return m_fractPrecision = precision; }
BENTLEYDLL_EXPORT FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }
BENTLEYDLL_EXPORT Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
BENTLEYDLL_EXPORT Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
BENTLEYDLL_EXPORT Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
BENTLEYDLL_EXPORT Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
BENTLEYDLL_EXPORT bool IfUseSeparator() { return m_useThousandsSeparator; }
BENTLEYDLL_EXPORT bool SetUseSeparator(bool set) { return m_useThousandsSeparator = set; }
BENTLEYDLL_EXPORT int GetDecimalPrecision()  { return PrecisionValue(); }
BENTLEYDLL_EXPORT void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
BENTLEYDLL_EXPORT bool IsPrecisionZero() {    return (m_decPrecision == DecimalPrecision::Precision0);}
BENTLEYDLL_EXPORT int IntPartToText (double n, char * bufOut, int bufLen, bool useSeparator);
BENTLEYDLL_EXPORT int FormatInteger (int n, char* bufOut, int bufLen);
BENTLEYDLL_EXPORT int FormatDouble(double dval, char* buf, int bufLen);
BENTLEYDLL_EXPORT Utf8String FormatDouble(double dval);
BENTLEYDLL_EXPORT Utf8String FormatInteger(int ival);
};


struct FormatParameterSet : NumericFormat
{
private:
    RoundingType        m_roundType;
    double              m_roundFactor;
    Utf8Char            m_leftPadding;
    Utf8Char            m_rightPadding;
    int                 m_boundaryLength;
    FieldAlignment      m_alignment;

public:

    FormatParameterSet()
    {
        m_roundType = RoundingType::RoundAwayFrom0;
        m_roundFactor = 0.5;
        m_leftPadding;
        m_rightPadding;
        m_boundaryLength;
        m_alignment;
    }
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
private:
    double m_dval;            // we keep the originally submitted value for its sign and possible reverse operations
    double m_topValue;        
    double m_midValue;
    double m_lowValue;
    DecimalPrecision m_decPrecision;
    int    m_topToMid;
    int    m_midToLow;
    bool   m_init;
    bool   m_midAssigned;
    bool   m_lowAssigned;
    bool   m_negative;

    void Convert()
    {
        if (!m_init)
            return;
        m_midAssigned = false;
        m_lowAssigned = false;

        double topmid = (double)m_topToMid;
        double midlow = (double)m_midToLow;
        double toplow = topmid * midlow;
        double rem = 0.0;
        if (m_decPrecision == DecimalPrecision::Precision0)
            m_dval = floor(m_dval + 0.501);
        m_topValue = 0.0;
        m_midValue = 0.0;
        m_lowValue = 0.0;
        int convType = 0;
        if (m_topToMid > 1)
            convType |= 0x1;
        if (m_midToLow > 1)
            convType |= 0x2;
        // there are only three allowed combinations of the factors:
        //  0 - when topMid < 1  top value is set to the initial value regardless of midToLow factor value
        //  1 - when topMid > 1 and midlow < 1 only top and middle values will be calculated
        //  3 - when both factors are > 1

        switch (convType)
        {
        case 1:
            m_topValue = floor(m_dval / topmid);
            m_midValue = m_dval - m_topValue * topmid;
            m_midAssigned = true;
            break;

        case 3:
            m_topValue = floor(m_dval / toplow);
            rem = m_dval - m_topValue * toplow;
            m_midValue = floor(rem/ midlow);
            m_lowValue = rem - m_midValue * midlow;
            m_midAssigned = true;
            m_lowAssigned = true;
            break;

        default:
            m_topValue = GetWhole();
            break;
        }
    }

    NumericTriad()
    {
        m_dval = 0.0;
        m_topValue = 0.0;
        m_midValue = 0.0;
        m_lowValue = 0.0;
        m_topToMid = 0;
        m_midToLow = 0;
        m_init = false;
        m_midAssigned = false;
        m_lowAssigned = false;
        m_negative = false;
        m_decPrecision = DecimalPrecision::Precision0;
    }

    void SetValue(double dval, DecimalPrecision prec)
    {
        m_dval = dval;
        m_negative = false;
        if (m_dval < 0.0)
        {
            m_negative = true;
            m_dval = -m_dval;
        }
        m_decPrecision = prec;
    }

public:
    
    NumericTriad(double dval, int topMid, int midLow, DecimalPrecision prec)
    {
        SetValue(dval, prec);
        m_topToMid = topMid;
        m_midToLow = midLow;
        m_init = true;
        Convert();
    }

    double GetWhole() { return m_negative ? -m_dval: m_dval; }

    void ProcessValue(double dval, DecimalPrecision prec) 
    { 
        SetValue(dval, prec);
        Convert();
    }

    void SetRatio(int topToMid, int midToLow) { m_topToMid = topToMid; m_midToLow = midToLow; }
    void SetPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    double GetTopValue() { return m_topValue; }
    double GetMidValue() { return m_midValue; }
    double GetlowValue() { return m_lowValue; }

    Utf8String FormatWhole(DecimalPrecision prec)
    {
        NumericFormat fmt;
        NumericFormat* fmt1 = new NumericFormat();
        fmt.SetDecimalPrecision(prec);
        return fmt.FormatDouble(GetWhole());
    }

    Utf8String FormatTriad(Utf8StringCP topName, Utf8StringCP midName, Utf8StringCP lowName, bool includeZero)
    {
        NumericFormat fmt;
        Utf8String blank = Utf8String(" ");
        if (nullptr == topName || topName->size() < 1)
            topName = &blank;
        if (nullptr == midName || midName->size() < 1)
            midName = &blank;
        if (nullptr == lowName || lowName->size() < 1)
            lowName = &blank;

        if (!m_midAssigned)
        {
            fmt.SetDecimalPrecision(m_decPrecision);
            return fmt.FormatDouble(GetWhole());
        }

        fmt.SetDecimalPrecision(DecimalPrecision::Precision0);
        Utf8String top = fmt.FormatDouble(m_negative ? -m_topValue : m_topValue);
        top.append(blank);
        top.append(*topName);
        Utf8String mid = "";
        Utf8String low = "";
        if (m_lowAssigned)
        {
            if(m_midValue > 0.0 || includeZero)
                mid = fmt.FormatDouble(m_midValue);
            if (m_lowValue > 0.0 || includeZero)
            {
                fmt.SetDecimalPrecision(m_decPrecision);
                low = fmt.FormatDouble(m_lowValue);
            }
        }
        else if(m_midValue > 0.0 || includeZero)
        {
            fmt.SetDecimalPrecision(m_decPrecision);
            mid = fmt.FormatDouble(m_midValue);
        }

       if ("" != mid)
        {
            top.append(blank);
            top.append(mid);
            top.append(blank);
            top.append(*midName);
        }
        if ("" != low)
        {
            top.append(blank);
            top.append(low);
            top.append(blank);
            top.append(*lowName);
        }
        return top;
    }
};

struct FormatParameter
{
private:
    Utf8StringCP m_paramName;
    ParameterCategory m_category;
    ParameterCode m_paramCode;
    ParameterDataType m_paramType;

public:

    FormatParameter(Utf8StringCP name, ParameterCategory cat, ParameterCode code, ParameterDataType type)
    {
        m_paramName = name;
        m_category = cat;
        m_paramCode = code;
        m_paramType = type;
    }

    Utf8StringCP GetName() { return m_paramName; }
    int CompareName(Utf8StringCP other) { return  (m_paramName == other); }
};


struct FormatDictionary
{
private:
    bvector<FormatParameterP> m_paramList;

    FormatDictionary();

public:

    int InitLoad()
    {
        AddParameter(new FormatParameter(new Utf8String("NoSign"),           ParameterCategory::Sign,           ParameterCode::NoSign,           ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("OnlyNegative"),     ParameterCategory::Sign,           ParameterCode::OnlyNegative,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("SignAlways"),       ParameterCategory::Sign,           ParameterCode::SignAlways,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("NegativeParenths"), ParameterCategory::Sign,           ParameterCode::NegativeParenths, ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Decimal"),          ParameterCategory::Presentation,   ParameterCode::Decimal,          ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Fractional"),       ParameterCategory::Presentation,   ParameterCode::Fractional,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Scientific"),       ParameterCategory::Presentation,   ParameterCode::Sientific,        ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("ScientificNorm"),   ParameterCategory::Presentation,   ParameterCode::ScientificNorm,   ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("NoZeroes"),         ParameterCategory::Zeroes,         ParameterCode::NoZeroes,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("LeadingZeroes"),    ParameterCategory::Zeroes,         ParameterCode::LeadingZeroes,    ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("TrailingZeroes"),   ParameterCategory::Zeroes,         ParameterCode::TrailingZeroes,   ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("ShowDotZero"),      ParameterCategory::Zeroes,         ParameterCode::ShowDotZero,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Replace0Empty"),    ParameterCategory::Zeroes,         ParameterCode::Replace0Empty,    ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("BothZeroes"),       ParameterCategory::Zeroes,         ParameterCode::BothZeroes,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision0"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec0,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision1"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec1,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision2"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec2,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision3"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec3,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision4"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec4,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision5"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec5,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision6"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec6,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision7"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec7,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision8"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec8,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision9"),       ParameterCategory::DecPrecision,   ParameterCode::DecPrec9,         ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision10"),      ParameterCategory::DecPrecision,   ParameterCode::DecPrec10,        ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision11"),      ParameterCategory::DecPrecision,   ParameterCode::DecPrec11,        ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("Precision12"),      ParameterCategory::DecPrecision,   ParameterCode::DecPrec12,        ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec1"),       ParameterCategory::FractPrecision, ParameterCode::FractPrec1,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec2"),       ParameterCategory::FractPrecision, ParameterCode::FractPrec2,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec4"),       ParameterCategory::FractPrecision, ParameterCode::FractPrec4,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec8"),       ParameterCategory::FractPrecision, ParameterCode::FractPrec8,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec16"),      ParameterCategory::FractPrecision, ParameterCode::FractPrec16,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec32"),      ParameterCategory::FractPrecision, ParameterCode::FractPrec32,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec64"),      ParameterCategory::FractPrecision, ParameterCode::FractPrec64,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec128"),     ParameterCategory::FractPrecision, ParameterCode::FractPrec128,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractPrec256"),     ParameterCategory::FractPrecision, ParameterCode::FractPrec256,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("DecimalComma"),     ParameterCategory::Separator,      ParameterCode::DecimalComma,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("DecimalPoint"),     ParameterCategory::Separator,      ParameterCode::DecimalPoint,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("DecimalSepar"),     ParameterCategory::Separator,      ParameterCode::DecimalSepar,     ParameterDataType::Symbol));
        AddParameter(new FormatParameter(new Utf8String("ThousandSepComma"), ParameterCategory::Separator,      ParameterCode::ThousandSepComma, ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("ThousandSepPoint"), ParameterCategory::Separator,      ParameterCode::ThousandSepPoint, ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("ThousandsSepar"),   ParameterCategory::Separator,      ParameterCode::ThousandsSepar,   ParameterDataType::Symbol));
        AddParameter(new FormatParameter(new Utf8String("RoundUp"),          ParameterCategory::RoundType,      ParameterCode::RoundUp,          ParameterDataType::Double));
        AddParameter(new FormatParameter(new Utf8String("RoundDown"),        ParameterCategory::RoundType,      ParameterCode::RoundDown,        ParameterDataType::Double));
        AddParameter(new FormatParameter(new Utf8String("RoundToward0"),     ParameterCategory::RoundType,      ParameterCode::RoundToward0,     ParameterDataType::Double));
        AddParameter(new FormatParameter(new Utf8String("RoundAwayFrom0"),   ParameterCategory::RoundType,      ParameterCode::RoundAwayFrom0,   ParameterDataType::Double));
        AddParameter(new FormatParameter(new Utf8String("FractBarHoriz"),    ParameterCategory::FractionBar,    ParameterCode::FractBarHoriz,    ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractBarOblique"),  ParameterCategory::FractionBar,    ParameterCode::FractBarHoriz,    ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("FractBarDiagonal"), ParameterCategory::FractionBar,    ParameterCode::FractBarHoriz,    ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("AngleRegular"),     ParameterCategory::AngleFormat,    ParameterCode::AngleRegular,     ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("AngleDegMin"),      ParameterCategory::AngleFormat,    ParameterCode::AngleDegMin,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("AngleDegMinSec"),   ParameterCategory::AngleFormat,    ParameterCode::AngleDegMinSec,   ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("PaddingSymbol"),    ParameterCategory::Padding,        ParameterCode::PaddingSymbol,    ParameterDataType::Symbol));
        AddParameter(new FormatParameter(new Utf8String("CenterAlign"),      ParameterCategory::Alignment,      ParameterCode::CenterAlign,      ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("LeftAlign"),        ParameterCategory::Alignment,      ParameterCode::LeftAlign,        ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("RightAlign"),       ParameterCategory::Alignment,      ParameterCode::RightAlign,       ParameterDataType::Flag));
        AddParameter(new FormatParameter(new Utf8String("MapName"),          ParameterCategory::Mapping,        ParameterCode::MapName,          ParameterDataType::String));
    }

    size_t GetCount() { return m_paramList.size(); }

    void AddParameter(FormatParameterP par)
    {
        if (nullptr != par)
            m_paramList.push_back(par);
        return;
    }

    FormatParameterP FindParameter(Utf8StringCP paramName)
    {
        FormatParameterP par;
        for (auto curr = m_paramList.begin(), end = m_paramList.end(); curr != end; curr++)
        {
            par = *curr;
            if (0 == par->CompareName(paramName))
                return par;
        }
        return nullptr;
    }
};


END_BENTLEY_NAMESPACE

