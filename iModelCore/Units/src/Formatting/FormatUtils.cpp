/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <map>
#include <Formatting/FormattingApi.h>
#include <iostream>
#include <locale>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
bmap<SignOption, Utf8String> UIUtils::GetAvailableSignOption(SignOption& defaultVal)
    {
    bmap<SignOption, Utf8String> signOptions;
    signOptions[SignOption::NoSign] = FormatConstant::FPN_NoSign().c_str();
    signOptions[SignOption::OnlyNegative] = FormatConstant::FPN_OnlyNegative().c_str();
    signOptions[SignOption::SignAlways] = FormatConstant::FPN_SignAlways().c_str();
    signOptions[SignOption::NegativeParentheses] = FormatConstant::FPN_NegativeParenths().c_str();

    defaultVal = FormatConstant::DefaultSignOption();
    return signOptions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
bmap<PresentationType, Utf8String> UIUtils::GetAvailablePresentationTypes(PresentationType& defaultVal)
    {
    bmap<PresentationType, Utf8String> presentationTypes;

    presentationTypes[PresentationType::Decimal] = Utils::GetPresentationTypeString(PresentationType::Decimal).c_str();
    presentationTypes[PresentationType::Fractional] = Utils::GetPresentationTypeString(PresentationType::Fractional).c_str();
    presentationTypes[PresentationType::Scientific] = Utils::GetPresentationTypeString(PresentationType::Scientific).c_str();
    presentationTypes[PresentationType::Station] = Utils::GetPresentationTypeString(PresentationType::Station).c_str();

    defaultVal = FormatConstant::DefaultPresentaitonType();
    return presentationTypes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
bmap<FormatTraits, Utf8String> UIUtils::GetAvailableTraits(FormatTraits& defaultVal)
    {
    bmap<FormatTraits, Utf8String> traits;

    traits[FormatTraits::ShowUnitLabel] = FormatConstant::FPN_ShowUnitLabel().c_str();
    traits[FormatTraits::Use1000Separator] = FormatConstant::FPN_Use1000Separator().c_str();
    traits[FormatTraits::TrailingZeroes] = FormatConstant::FPN_TrailZeroes().c_str();
    traits[FormatTraits::PrependUnitLabel] = FormatConstant::FPN_PrependUnitLabel().c_str();
    traits[FormatTraits::KeepSingleZero] = FormatConstant::FPN_KeepSingleZero().c_str();
    traits[FormatTraits::ZeroEmpty] = FormatConstant::FPN_ZeroEmpty().c_str();
    traits[FormatTraits::KeepDecimalPoint] = FormatConstant::FPN_KeepDecimalPoint().c_str();
    traits[FormatTraits::ApplyRounding] = FormatConstant::FPN_ApplyRounding().c_str();
    traits[FormatTraits::FractionDash] = FormatConstant::FPN_FractionDash().c_str();
    traits[FormatTraits::ExponenentOnlyNegative] = FormatConstant::FPN_ExponentOnlyNegative().c_str();

    defaultVal = FormatConstant::DefaultFormatTraits();
    return traits;
    }

//===================================================
// FormatConstant
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
const size_t FormatConstant::GetSequenceLength(unsigned char c)
    {
    if (0 == (c & UTF_TrailingByteMark())) // ASCII - single byte
        return 1;
    if ((c & UTF_2ByteMask()) == UTF_2ByteMark())
        return 2;
    if ((c & UTF_3ByteMask()) == UTF_3ByteMark())
        return 3;
    if ((c & UTF_4ByteMask()) == UTF_4ByteMark())
        return 4;
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
bool FormatConstant::GetTrailingBits(unsigned char c, Utf8P outBits)
    {
    if (nullptr != outBits)
        {
        *outBits = 0;
        if (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask()))
            *outBits = c & ~UTF_TrailingByteMask();
        return true;
        }
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
const unsigned char FormatConstant::TriadBitMask(size_t threeBit)
    {
    static unsigned char mask[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
    threeBit &= 0x7;
    return mask[threeBit];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
const FormatSpecialCodes FormatConstant::ParsingPatternCode(Utf8CP name)
    {
    if(Utf8String::IsNullOrEmpty(name)) return FormatSpecialCodes::SignatureNull;
    if (strcmp("N", name) == 0) return FormatSpecialCodes::SignatureN;
    if (strcmp("F", name) == 0) return FormatSpecialCodes::SignatureF;
    if (strcmp("NF", name) == 0) return FormatSpecialCodes::SignatureNF;
    if (strcmp("NU", name) == 0) return FormatSpecialCodes::SignatureNU;
    if (strcmp("FU", name) == 0) return FormatSpecialCodes::SignatureNU;
    if (strcmp("NFU", name) == 0) return FormatSpecialCodes::SignatureNFU;
    if (strcmp("NUNU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("FUNU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("NUFU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("NUNFU", name) == 0) return FormatSpecialCodes::SignatureNUNFU;
    if (strcmp("NUNUNU", name) == 0) return FormatSpecialCodes::SignatureNUNUNU;
    if (strcmp("NUNUNFU", name) == 0) return FormatSpecialCodes::SignatureNUNUNFU;
    if (strcmp("NUNUFU", name) == 0) return FormatSpecialCodes::SignatureNUNUNU;
    if (strcmp("NCNCN", name) == 0) return FormatSpecialCodes::SignatureNCNCN;  // 0:0:0
    if (strcmp("NCCN", name) == 0) return FormatSpecialCodes::SignatureNCCN;    // 0::0
    if (strcmp("NCC", name) == 0) return FormatSpecialCodes::SignatureNCC;      // 0::
    if (strcmp("CNCN", name) == 0) return FormatSpecialCodes::SignatureCNCN;    // :0:0
    if (strcmp("CNC", name) == 0) return FormatSpecialCodes::SignatureCNC;      // :0:
    if (strcmp("CCN", name) == 0) return FormatSpecialCodes::SignatureCCN;      // ::0
    if (strcmp("NCNC", name) == 0) return FormatSpecialCodes::SignatureNCNC;    // 0:0:
    if (strcmp("NCN", name) == 0) return FormatSpecialCodes::SignatureNCN;      // 0:0
    if (strcmp("CN", name) == 0) return FormatSpecialCodes::SignatureCN;        // :0
    if (strcmp("NC", name) == 0) return FormatSpecialCodes::SignatureNC;        // 0:
    if (strcmp("MCNCN", name) == 0) return FormatSpecialCodes::SignatureMCNCN;    // :0:0
    if (strcmp("MCNC", name) == 0) return FormatSpecialCodes::SignatureMCNCN;    // :0:0
    if (strcmp("MCCN", name) == 0) return FormatSpecialCodes::SignatureMCCN;      // ::0
    if (strcmp("MCN", name) == 0) return FormatSpecialCodes::SignatureMCN;        // :0

    return FormatSpecialCodes::SignatureInvalid;
    }

//===================================================
// Utils
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                               Kyle.Abramowitz      03/2018
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetScientificTypeString(ScientificType type)
    {
    switch(type)
        {
        case ScientificType::ZeroNormalized:
            return FormatConstant::FPN_ScientificZeroNormalized();
        case ScientificType::Normalized:
            return FormatConstant::FPN_ScientificNormalized();
        }
    return "";
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz       03/2018
//----------------------------------------------------------------------------------------
// static
bool Utils::ParseScientificType(ScientificType& out, Utf8StringCR name)
    {
    if (BeStringUtilities::StricmpAscii(name.c_str(), FormatConstant::FPN_ScientificNormalized().c_str()) == 0)
        out = ScientificType::Normalized;
    else if (BeStringUtilities::StricmpAscii(name.c_str(), FormatConstant::FPN_ScientificZeroNormalized().c_str()) == 0)
        out = ScientificType::ZeroNormalized;
    else
        return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetSignOptionString(SignOption opt)
    {
    switch (opt)
        {
        case SignOption::NoSign: return FormatConstant::FPN_NoSign();
        case SignOption::SignAlways: return FormatConstant::FPN_SignAlways();
        case SignOption::NegativeParentheses: return FormatConstant::FPN_NegativeParenths();
        default:
        case SignOption::OnlyNegative: return FormatConstant::FPN_OnlyNegative();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
// static
bool Utils::ParseSignOption(SignOption& out, Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_NoSign().c_str()) == 0) out = SignOption::NoSign;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_OnlyNegative().c_str()) == 0) out = SignOption::OnlyNegative;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_SignAlways().c_str()) == 0) out = SignOption::SignAlways;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_NegativeParenths().c_str()) == 0) out = SignOption::NegativeParentheses;
    else
        return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetCurrentDecimalSeparator()
    {
    struct lconv *lc = localeconv();
    Utf8String sep;
    if (*(lc->decimal_point) != '\0')
        sep.assign(lc->decimal_point);
    return sep;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetCurrentGrouping()
    {
#define UGCG_BUFLEN 10
    struct lconv *lc = localeconv();
    Utf8Char buf[UGCG_BUFLEN + 2];
    memset(buf, 0, sizeof(buf));
    Utf8CP p = (lc->grouping);
    for (int i = 0; i < UGCG_BUFLEN && *p != '\0'; i++, p++)
        {
        if ((*p & 0x40) != 0)
            {
            buf[i] = '0';
            break;
            }
        else
            buf[i] = '0' + *p;
        }
    Utf8String sep;
    if (buf[0] != '\0')
        sep.assign(buf);
    return sep;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetCurrentThousandSeparator()
    {
    struct lconv *lc = localeconv();
    Utf8String sep;
    if (*(lc->thousands_sep) != '\0')
        sep.assign(lc->thousands_sep);
    return sep;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
bool Utils::GetDecimalPrecisionByInt(DecimalPrecision& out, int32_t num)
    {
    if (num <= static_cast<uint32_t>(DecimalPrecision::Max))
        {
        out = static_cast<DecimalPrecision>(num);
        return true;
        }
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
double Utils::DecimalPrecisionFactor(DecimalPrecision decP)
    {
    static double FactorSet[13] = { 1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12 };
    return FactorSet[DecimalPrecisionToInt(decP)];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::GetPresentationTypeString(PresentationType type)
    {
    switch (type)
        {
        case PresentationType::Fractional: return FormatConstant::FPN_Fractional();
        case PresentationType::Scientific: return FormatConstant::FPN_Scientific();
        case PresentationType::Station: return FormatConstant::FPN_Station();
        default:
        case PresentationType::Decimal: return FormatConstant::FPN_Decimal();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
bool Utils::ParsePresentationType(PresentationType& type, Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Decimal().c_str()) == 0) 
        type = PresentationType::Decimal;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Fractional().c_str()) == 0) 
        type = PresentationType::Fractional;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Scientific().c_str()) == 0) 
        type = PresentationType::Scientific;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Station().c_str()) == 0) 
        type = PresentationType::Station;
    else
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
bool Utils::FractionalPrecisionByDenominator(FractionalPrecision& out, int32_t prec)
    {
    switch (prec)
        {
        case 1: out = FractionalPrecision::Whole; break;
        case 2: out = FractionalPrecision::Half; break;
        case 4: out = FractionalPrecision::Quarter; break;
        case 8: out = FractionalPrecision::Eighth; break;
        case 16: out = FractionalPrecision::Sixteenth; break;
        case 32: out = FractionalPrecision::Over_32; break;
        case 64: out = FractionalPrecision::Over_64; break;
        case 128: out = FractionalPrecision::Over_128; break;
        case 256: out = FractionalPrecision::Over_256; break;
        default: return false;
        }
    return true;
    }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 // static
int32_t Utils::FractionalPrecisionDenominator(FractionalPrecision prec)
    {
    switch (prec)
        {
        case FractionalPrecision::Half: return 2;
        case FractionalPrecision::Quarter: return 4;
        case FractionalPrecision::Eighth: return 8;
        case FractionalPrecision::Sixteenth: return 16;
        case FractionalPrecision::Over_32: return 32;
        case FractionalPrecision::Over_64: return 64;
        case FractionalPrecision::Over_128: return 128;
        case FractionalPrecision::Over_256: return 256;
        default:
        case FractionalPrecision::Whole: return 1;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::AppendUnitName(Utf8CP txtValue, Utf8CP unitName, Utf8CP space)
    {
    size_t txtL = Utf8String::IsNullOrEmpty(txtValue) ? 0 : strlen(txtValue);
    size_t uomL = Utf8String::IsNullOrEmpty(unitName) ? 0 : strlen(unitName);
    size_t spcL = Utf8String::IsNullOrEmpty(space) ? 0 : strlen(space);
    if (0 == txtL)
        txtValue = "";
    if (0 == uomL)
        return txtValue;
    size_t totL = txtL + uomL + spcL + 2;
    Utf8P buf = (char*)alloca(totL);
    BeStringUtilities::Snprintf(buf, totL, "%s%s%s", txtValue, ((0 == spcL)? "":space), unitName);
    return buf;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
// static
Utf8Char Utils::MatchingDivider(Utf8Char div)
    {
    Utf8CP fd= FormatConstant::Dividers();
    Utf8CP df = FormatConstant::DividerMatch();
    for (int i = 0; fd[i] != FormatConstant::EndOfLine(); i++)
        {
        if (div == fd[i])
            return df[i];
        }
    return FormatConstant::EndOfLine();
    }

//===================================================
// LocaleProperties
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
LocaleProperties::LocaleProperties(Json::Value jval)
    {
    if (!jval.empty())
        {
        Utf8CP paramName;
        Utf8String str;
        Utf8String jStr = jval.ToString();
        for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
            {
            paramName = iter.memberName();
            JsonValueCR val = *iter;
            if (BeStringUtilities::StricmpAscii(paramName, json_decimalSeparator()) == 0)
                {
                str = val.asString();
                m_decimalSeparator = str.c_str()[0];
                }
            else if (BeStringUtilities::StricmpAscii(paramName, json_thousandSeparator()) == 0)
                {
                str = val.asString();
                m_thousandsSeparator = str.c_str()[0];
                }
            }
        }
    else
        {
        m_decimalSeparator = '.';
        m_thousandsSeparator = '\0';
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
LocaleProperties::LocaleProperties(Utf8CP localeName)
    {
    std::locale loc = std::locale(localeName);
    const std::numpunct<char>& myfacet(std::use_facet < std::numpunct<char> >(loc));

    m_decimalSeparator = myfacet.decimal_point();
    m_thousandsSeparator = myfacet.thousands_sep();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
Json::Value LocaleProperties::ToJson()
    {
    Json::Value jval;
    jval[json_decimalSeparator()] = &m_decimalSeparator;
    jval[json_thousandSeparator()] = &m_thousandsSeparator;
    return jval;
    }

//===================================================
// FormattingDividers
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormattingDividers::FormattingDividers(Utf8CP div)
    {
    if (nullptr == div)
        div = " !\"#$%&\'()*+,-./:;<=>? [\\]^{|}";
    memset(m_markers, 0, sizeof(m_markers));
    Utf8CP p = div;
    while (*p != 0)
        {
        m_markers[(*p & 0x78) >> 3] |= FormatConstant::TriadBitMask(*p);
        ++p;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormattingDividers::FormattingDividers(FormattingDividersCR other)
    {
    memcpy(m_markers, other.m_markers, sizeof(m_markers));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool FormattingDividers::IsDivider(char c)
    {
    if (0 == c)
        return true;
    return (0 != ((m_markers[(c & 0x78) >> 3]) & (FormatConstant::TriadBitMask(c))));
    }

//===================================================
// FormatProblemDetail
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatProblemDetail::GetProblemDescription() const
    {
    switch (m_code)
        {
        case FormatProblemCode::NoProblems: return "No problems";
        case FormatProblemCode::UnknownStdFormatName: return "Unknown name of the standard format";
        case FormatProblemCode::UnknownUnitName: return "Unknown name of the unit";
        case FormatProblemCode::NotInitialized: return "Object not initialized";
        case FormatProblemCode::CVS_InconsistentFactorSet: return "Inconsistent set of factors";
        case FormatProblemCode::CVS_InconsistentUnitSet: return "Inconsistent set of units";
        case FormatProblemCode::CVS_UncomparableUnits: return "Units are not comparable";
        case FormatProblemCode::CVS_InvalidUnitName: return "Unknown name of the Unit";
        case FormatProblemCode::CVS_InvalidMajorUnit: return "Unknown name of the Major Unit";
        case FormatProblemCode::QT_PhenomenonNotDefined: return "Unknown name of the Phenomenon";
        case FormatProblemCode::QT_PhenomenaNotSame: return "Different Phenomena";
        case FormatProblemCode::QT_InvalidTopMidUnits: return "Top and Middle units are not comparable";
        case FormatProblemCode::QT_InvalidMidLowUnits: return "Middle and Low units are not comparable";
        case FormatProblemCode::QT_InvalidUnitCombination: return "Invalid Unit combination ";
        case FormatProblemCode::QT_InvalidSyntax: return "Invalid syntax of KOQ"; 
        case FormatProblemCode::FUS_InvalidSyntax: return "Invalid syntax of FUS";
        case FormatProblemCode::NFS_Undefined: return "Numeric Format is not defined";
        case FormatProblemCode::NFS_InvalidSpecName: return "Invalid Numeric Format name";
        case FormatProblemCode::NFS_DuplicateSpecName: return "Duplicate Numeric Format name";
        case FormatProblemCode::NFS_DuplicateSpecNameOrAlias: return "Duplicate Numeric Format name or alias";
        case FormatProblemCode::NFS_InvalidJsonObject: return "Invalid Numeric Format JSON-descriptor";
        case FormatProblemCode::DIV_UnknownDivider: return "Unknown Divider";
        case FormatProblemCode::NA_InvalidSign: return "Invalid or duplicate sign in numeric definition";         // Numeric Accumulator problems
        case FormatProblemCode::NA_InvalidPoint: return "Invalid or duplicate decimal point in numeric definition";
        case FormatProblemCode::NA_InvalidExponent: return "Invalid or duplicate exponent in numeric definition";
        case FormatProblemCode::NA_InvalidSyntax: return "Invalid symtax of numeric expression";
        case FormatProblemCode::PS_InvalidColonizedExpression: return "Invalid expression";
        case FormatProblemCode::PS_MissingFUS: return "Cannot parse expression without FUS";
        case FormatProblemCode::PS_MissingCompositeSpec: return "Cannot parse expression without Composite Format Spec";
        case FormatProblemCode::PS_MismatchingFUS: return "FUS does not match expression";
        case FormatProblemCode::SFS_InsertingNamelessFUS: return "FUS name is required for registering";
        case FormatProblemCode::SFS_DuplicateFUSName: return "FUS with htis name is already has been registered";
        case FormatProblemCode::SFS_FailedToMakeFUS: return "Invalid defintion for the FUS to be registered";
        case FormatProblemCode::NMQ_InvalidUnitName: return "Invalid unit name in the Named Quantity";
        case FormatProblemCode::NMQ_MissingName: return "Named Quantity requires a not-empty name";
        default: return "Unknown problem code";
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool FormatProblemDetail::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_code == FormatProblemCode::NoProblems)
        m_code = code;
    return IsProblem();
    }

END_BENTLEY_FORMATTING_NAMESPACE
