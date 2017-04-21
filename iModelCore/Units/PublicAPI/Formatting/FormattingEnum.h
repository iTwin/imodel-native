/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingEnum.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Formatting/FormattingDefinitions.h>
#include <Units/Units.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// Enumerations
//
//===================================================

enum class ParameterCode
    {
    FormatName = 50,
    NoSign = 101,
    OnlyNegative = 102,
    SignAlways = 103,
    NegativeParenths = 104,
    Decimal = 151,
    Fractional = 152,
    Scientific = 153,
    ScientificNorm = 154,
    Binary = 155,
    DefaultZeroes = 201,
    LeadingZeroes = 202,
    TrailingZeroes = 203,
    KeepSingleZero = 204,
    KeepDecimalPoint = 205,
    ExponentZero = 206,
    ZeroEmpty = 207,    // formatter will return the empy string if the result is 0
    DecPrec0 = 300,
    DecPrec1 = 301,
    DecPrec2 = 302,
    DecPrec3 = 303,
    DecPrec4 = 304,
    DecPrec5 = 305,
    DecPrec6 = 306,
    DecPrec7 = 307,
    DecPrec8 = 308,
    DecPrec9 = 309,
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
    BitFlag = 1,
    Integer = 2,
    Double = 3,
    Symbol = 4,
    String = 5
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
    None = 0,
    Oblique = 1,
    Horizontal = 2,
    Diagonal = 3
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

enum class FormatTraits : int
    {
    DefaultZeroes = 0,
    LeadingZeroes = 0x1,
    TrailingZeroes = 0x2,
    KeepDecimalPoint = 0x4,
    KeepSingleZero = 0x8,
    ExponentZero = 0x10,
    ZeroEmpty = 0x20,
    Use1000Separator = 0x40,
    ApplyRounding = 0x80
    };

enum class NumSequenceTraits
    {
    None = 0,
    Signed = 0x1,
    DecPoint = 0x2,
    Exponent = 0x4,
    Uom = 0x8
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

enum class ScannerCursorStatus
    {
    Success = 0,
    InvalidSymbol = 1,
    IncompleteSequence = 2,
    NoEnclosure = 3
    };

//! Codes of problems might help in finding the source of the problem
enum class FormatProblemCode
    {
    NoProblems = 0,
    UnknownStdFormatName = 20011,
    UnknownUnitName = 20012,
    CNS_InconsistentFactorSet = 20051,  //!< All ratio factors between units must be bigger than one
    CNS_InconsistentUnitSet = 20052,    //!< Each pair of UOM's for parts of combo-numbers should yeild a ratio > 1
    CNS_UncomparableUnits = 20053,      //!< Units provided on the argument list are not comparable
    CNS_InvalidUnitName = 20054,        //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    CNS_InvalidMajorUnit = 20055,       //!< The MajorUnit in ComboNumbers is null or invalid
    QT_PhenomenonNotDefined = 20101,
    QT_PhenomenaNotSame = 20102,
    QT_InvalidTopMidUnits = 20103,
    QT_InvalidMidLowUnits = 20104,
    QT_InvalidUnitCombination = 20105,
    FUS_InvalidSyntax = 20151,
    NFS_InvalidSpecName = 20161,
    NFS_DuplicateSpecName = 20162,
    DIV_UnknownDivider = 25001
    };

enum class FormatProblemLevel  // these levels should be used for assigning the Problem code
    {
    Undefined = 0,
    Notice = 1,
    Warning = 10000,
    Critical = 20000
    };

enum class CodepointSize
    {
    Zero = 0,
    Single = 1,
    Double = 2,
    Quatro = 4
    };

//! Type of the ComboSpec describes one of allowable value transformations
enum class CompositeSpecType
    {
    Undefined = 0, //!< program failes to infer the type (default)
    Single = 1,    //!< trivial case when Combo effectively is not used - not prohibited though
    Double = 2,    //!< indicates of using 2 levels: Major and Middle
    Triple = 3,    //!< indicates of using 3 levels: Major, Middle and Minor UOM's
    Quatro = 4     //!< indicates of using 4 levels: Major, Middle, Minor and SubUnit UOM's
    };

enum class FormatSpecType
    {
    Undefined = 0,
    Numeric = 1,   // a pure numeric Spec
    Composite = 2      // a composite spec is also defined (numeric spec is implied)
    };

enum class ScanSegmentType
    {
    Prefix = 0,
    Sign = 1,
    IntPart = 2,
    DecPoint = 3,
    FractPart = 4,
    ExponentSymbol = 5,
    ExponentSign = 6,
    ExponentValue = 7,
    Delimiter = 8,
    UnitName = 9,
    UnitDelim = 10,
    Suffix = 11,
    Total = 12,
    Undefined = 13
    };

struct FormatProblemDetail
    {
    private:
        FormatProblemCode  m_code;

    public:

        FormatProblemDetail() { m_code = FormatProblemCode::NoProblems; }

        FormatProblemDetail(FormatProblemCode code)
            {
            m_code = code;
            }
        bool IsCritical() const { return (static_cast<int>(m_code) > static_cast<int>(FormatProblemLevel::Critical)); }
        bool IsWarning() const {
            return (static_cast<int>(m_code) < static_cast<int>(FormatProblemLevel::Critical) &&
                static_cast<int>(m_code) > static_cast<int>(FormatProblemLevel::Warning));
            }
        bool IsProblem() const { return m_code != FormatProblemCode::NoProblems; }
        bool NoProblem() const { return m_code == FormatProblemCode::NoProblems; }

        FormatProblemCode const GetProblemCode() { return m_code; }
        UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
        UNITS_EXPORT Utf8String GetProblemDescription() const;
    };

END_BENTLEY_FORMATTING_NAMESPACE