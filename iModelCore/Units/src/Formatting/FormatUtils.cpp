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
#include "../../Localization/xliffs/Units.xliff.h"
#include <BeSQLite/L10N.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_BENTLEY_FORMATTING_NAMESPACE

static StdFormatSet const s_stdFmtSet;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableDecimalPercisions()
    {
    UIList decPercisionList;

    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision0, UNITSL10N_GETSTRING(DecimalPrecision_0).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision1, UNITSL10N_GETSTRING(DecimalPrecision_1).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision2, UNITSL10N_GETSTRING(DecimalPrecision_2).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision3, UNITSL10N_GETSTRING(DecimalPrecision_3).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision4, UNITSL10N_GETSTRING(DecimalPrecision_4).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision5, UNITSL10N_GETSTRING(DecimalPrecision_5).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision6, UNITSL10N_GETSTRING(DecimalPrecision_6).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision7, UNITSL10N_GETSTRING(DecimalPrecision_7).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision8, UNITSL10N_GETSTRING(DecimalPrecision_8).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision9, UNITSL10N_GETSTRING(DecimalPrecision_9).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision10, UNITSL10N_GETSTRING(DecimalPrecision_10).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision11, UNITSL10N_GETSTRING(DecimalPrecision_11).c_str()));
    decPercisionList.AddListEntry(UIListEntry((int)DecimalPrecision::Precision12, UNITSL10N_GETSTRING(DecimalPrecision_12).c_str()));

    return decPercisionList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableFractionalPercisions()
    {
    UIList fracPercisionList;

    fracPercisionList.AddListEntry(UIListEntry(1,    UNITSL10N_GETSTRING(FractionalPrecision_Whole).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(2,    UNITSL10N_GETSTRING(FractionalPrecision_Half).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(4,    UNITSL10N_GETSTRING(FractionalPrecision_Quarter).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(8,    UNITSL10N_GETSTRING(FractionalPrecision_Eighth).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(16,   UNITSL10N_GETSTRING(FractionalPrecision_Sixteenth).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(32,   UNITSL10N_GETSTRING(FractionalPrecision_Over_32).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(64,   UNITSL10N_GETSTRING(FractionalPrecision_Over_64).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(128,  UNITSL10N_GETSTRING(FractionalPrecision_Over_128).c_str()));
    fracPercisionList.AddListEntry(UIListEntry(256,  UNITSL10N_GETSTRING(FractionalPrecision_Over_256).c_str()));
    return fracPercisionList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableSignOption()
    {
    UIList signOptions;
    signOptions.AddListEntry(UIListEntry((int)ShowSignOption::NoSign,              UNITSL10N_GETSTRING(ShowSignOption_NoSign).c_str(), FormatConstant::FPN_NoSign().c_str()));
    signOptions.AddListEntry(UIListEntry((int)ShowSignOption::OnlyNegative,        UNITSL10N_GETSTRING(ShowSignOption_OnlyNegative).c_str(), FormatConstant::FPN_OnlyNegative().c_str()));
    signOptions.AddListEntry(UIListEntry((int)ShowSignOption::SignAlways,          UNITSL10N_GETSTRING(ShowSignOption_SignAlways).c_str(), FormatConstant::FPN_SignAlways().c_str()));
    signOptions.AddListEntry(UIListEntry((int)ShowSignOption::NegativeParentheses, UNITSL10N_GETSTRING(ShowSignOption_NegativeParentheses).c_str(), FormatConstant::FPN_NegativeParenths().c_str()));
    return signOptions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailablePresentationTypes()
    {
    UIList presentationTypes;

    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Decimal, UNITSL10N_GETSTRING(PresentationType_Decimal).c_str(), Utils::PresentationTypeName(PresentationType::Decimal).c_str()));
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Fractional, UNITSL10N_GETSTRING(PresentationType_Fractional).c_str(), Utils::PresentationTypeName(PresentationType::Fractional).c_str()));
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Scientific, UNITSL10N_GETSTRING(PresentationType_Scientific).c_str(), Utils::PresentationTypeName(PresentationType::Scientific).c_str()));
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Station, UNITSL10N_GETSTRING(PresentationType_Station).c_str(), Utils::PresentationTypeName(PresentationType::Station).c_str()));

    return presentationTypes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableDecimalSeparators()
    {
    UIList decimalSeparators;

    decimalSeparators.AddListEntry(UIListEntry(0, UNITSL10N_GETSTRING(DecimalSeparator_Comma).c_str(), ","));
    decimalSeparators.AddListEntry(UIListEntry(1, UNITSL10N_GETSTRING(DecimalSeparator_Point).c_str(), "."));

    return decimalSeparators;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableThousandSeparators()
    {
    UIList thousandSeparators;

    thousandSeparators.AddListEntry(UIListEntry(0, UNITSL10N_GETSTRING(ThousandSeparator_Comma).c_str(), ","));
    thousandSeparators.AddListEntry(UIListEntry(1, UNITSL10N_GETSTRING(ThousandSeparator_Point).c_str(), "."));

    return thousandSeparators;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableUnitLabelSeparators()
    {
    UIList unitLabelSeparators;

    unitLabelSeparators.AddListEntry(UIListEntry(0, UNITSL10N_GETSTRING(UnitLabelSeparator_None).c_str(), ""));
    unitLabelSeparators.AddListEntry(UIListEntry(1, UNITSL10N_GETSTRING(UnitLabelSeparator_Space).c_str(), " "));

    return unitLabelSeparators;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
UIList UIUtils::GetAvailableTraits()
    {
    UIList traits;

    traits.AddListEntry(UIListEntry((int) FormatTraits::ShowUnitLabel, UNITSL10N_GETSTRING(FormatTraits_ShowUnitLabel).c_str(), json_showUnitLabel()));
    traits.AddListEntry(UIListEntry((int)FormatTraits::Use1000Separator, UNITSL10N_GETSTRING(FormatTraits_Use1000Separator).c_str(), json_use1000Separator()));
    traits.AddListEntry(UIListEntry((int) FormatTraits::TrailingZeroes, UNITSL10N_GETSTRING(FormatTraits_TrailingZeroes).c_str(), json_trailZeroes()));
    // only used when width is set, so let's ignore for now since users won't likely be setting width.
    //traits.AddListEntry(UIListEntry((int)FormatTraits::LeadingZeroes, UNITSL10N_GETSTRING(FormatTraits_LeadingZeroes).c_str(), json_LeadZeroes()));

    // the following trait does not seem to do anything.
    //traits.AddListEntry(UIListEntry((int)FormatTraits::KeepDecimalPoint, UNITSL10N_GETSTRING(FormatTraits_KeepDecimalPoint).c_str(), json_KeepDecPnt()));

   // TraitsBitToJson(jTraits, json_KeepDecPnt(), FormatTraits::KeepDecimalPoint, &ref, verbose);
   // TraitsBitToJson(jTraits, json_KeepSingleZero(), FormatTraits::KeepSingleZero, &ref, verbose);
   // TraitsBitToJson(jTraits, json_ExponentZero(), FormatTraits::ExponentZero, &ref, verbose);
   // TraitsBitToJson(jTraits, json_ZeroEmpty(), FormatTraits::ZeroEmpty, &ref, verbose);

   // DFR said not to show Rounding option to user
   // TraitsBitToJson(jTraits, json_ApplyRounding(), FormatTraits::ApplyRounding, &ref, verbose);

   // TraitsBitToJson(jTraits, json_UseFractSymbol(), FormatTraits::UseFractSymbol, &ref, verbose);
   // TraitsBitToJson(jTraits, json_FractionDash(), FormatTraits::FractionDash, &ref, verbose);

    return traits;
    }

//===================================================
// FormatConstant
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
const bool FormatConstant::IsLittleEndian()
    {
    union { short int s; char b[4]; } un;
    un.s = 1;
    return (un.b[0] == (char)1);
    }

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

// the trailing byte should be properly marked for being processed. It always contains only
//  6 bits that should be shifted accordingly to the location of the byte in the sequence
//  there are only 3 possible numbers of bytes in sequences: 2, 3 and 4. Accordingly the 
//  the range of indexes is striclty governed by the sequence lenght. The minimum index
//  value is always 1 and the maximum is N-1 where N is the sequence length
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
bool FormatConstant::GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits)
    {
    if (nullptr != outBits)
        {
        // calculate the shift 
        *outBits = 0;
        int shift = ((int)seqLength - (int)index - 1);
        if (0 > shift || 2 < shift)
            return false;
        if (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask()))
            {
            size_t temp = c & ~UTF_TrailingByteMask();
            temp <<= shift * UTF_UpperBitShift();
            *outBits = temp;
            }
        return true;
        }
    return false;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
size_t FormatConstant::ExtractTrailingBits(unsigned char c, size_t shift) 
    { 
    size_t cod = c & UTF_TrailingBitsMask();
    if (shift > 0)
        cod <<= shift;
    return cod; 
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
const size_t* FormatConstant::FractionCodes()
    {
                          // 1/4  1/2  3/4  1/3   2/3   1/5   2/5   3/5   4/5   1/6   5/6   1/8   3/8   5/8   7/8
    static size_t cod[16] = {188, 189, 190, 8531, 8532, 8533, 8534, 8535, 8536, 8537, 8538, 8539, 8540, 8541, 8542 };
    return cod;
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
// @bsimethod                                                    Kyle.Abramowitz     12/17
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::ScientificTypeName(ScientificType type)
    {
    switch(type)
        {
        case ScientificType::Engineering:
            return FormatConstant::FPN_ScientificEngineering();
        case ScientificType::Normal:
            return FormatConstant::FPN_ScientificNormal();
        case ScientificType::Standard:
            return FormatConstant::FPN_ScientificStandard();
        }
    return "";
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                    Kyle.Abramowitz     12/17
//----------------------------------------------------------------------------------------
// static
bool Utils::NameToScientificType(Utf8StringCR name, ScientificType& out)
    {
    if (BeStringUtilities::StricmpAscii(name.c_str(), FormatConstant::FPN_ScientificStandard().c_str()) == 0) out = ScientificType::Standard;
    else if (BeStringUtilities::StricmpAscii(name.c_str(), FormatConstant::FPN_ScientificNormal().c_str()) == 0) out = ScientificType::Normal;
    else if (BeStringUtilities::StricmpAscii(name.c_str(), FormatConstant::FPN_ScientificEngineering().c_str()) == 0) out = ScientificType::Engineering;
    else
        return false;
    return true;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
// static
bool Utils::NameToSignOption(Utf8CP name, ShowSignOption& out)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_NoSign().c_str()) == 0) out = ShowSignOption::NoSign;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_OnlyNegative().c_str()) == 0) out = ShowSignOption::OnlyNegative;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_SignAlways().c_str()) == 0) out = ShowSignOption::SignAlways;
    else if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_NegativeParenths().c_str()) == 0) out = ShowSignOption::NegativeParentheses;
    else
        return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
// static
Utf8CP Utils::SkipBlanks(Utf8CP str)
    {
    while (isspace(*str))
        str++;
    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
// static
Utf8Char Utils::GetFirstSignificantChar(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return '\0';
    while (isspace(*str))
        str++;
    return *str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
Utf8Char Utils::GetLastSignificantChar(Utf8CP str)
    {
    size_t len = Utf8String::IsNullOrEmpty(str)? 0 : strlen(str);
    if(len == 0)
        return '\0';
    len--;
    while (isspace(str[len]))
        {
        if (len > 1) 
            len--;
        else
            return '\0';
        }
    return str[len];
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
bool Utils::DecimalPrecisionByIndex(const size_t num, DecimalPrecision& out)
    {
    if (num < static_cast<uint32_t>(DecimalPrecision::Max))
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
double Utils::DecimalPrecisionFactor(DecimalPrecision decP, int index = -1)
    {
    static double FactorSet[13] = { 1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12 };
    if (0 <= index && sizeof(FactorSet) / sizeof(double) > index)
        return FactorSet[index];
    return FactorSet[DecimalPrecisionToInt(decP)];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String  Utils::PresentationTypeName(PresentationType type)
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
bool Utils::NameToPresentationType(Utf8CP name, PresentationType& type)
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
Utf8String Utils::SignOptionName(ShowSignOption opt)
    {
    switch (opt)
        {
        case ShowSignOption::NoSign: return FormatConstant::FPN_NoSign();
        case ShowSignOption::SignAlways: return FormatConstant::FPN_SignAlways();
        case ShowSignOption::NegativeParentheses: return FormatConstant::FPN_NegativeParenths();
        default:
        case ShowSignOption::OnlyNegative: return FormatConstant::FPN_OnlyNegative();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
Utf8String Utils::FractionBarName(FractionBarType bar)
    {
    switch (bar)
        {
        case FractionBarType::Diagonal: return FormatConstant::FPN_FractBarDiagonal();
        case FractionBarType::Oblique: return FormatConstant::FPN_FractBarOblique();
        case FractionBarType::Horizontal: return FormatConstant::FPN_FractBarHoriz();
        case FractionBarType::None:
        default:  return "";
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
bool Utils::NameToFractionBarType(Utf8CP name, FractionBarType& type)
    {
    if (BeStringUtilities::StricmpAscii(name, "none") == 0) type = FractionBarType::None;
    else if (BeStringUtilities::StricmpAscii(name, "diagonal") == 0) type = FractionBarType::Diagonal;
    else if (BeStringUtilities::StricmpAscii(name, "oblique") == 0) type = FractionBarType::Oblique;
    else if (BeStringUtilities::StricmpAscii(name, "horizontal") == 0) type = FractionBarType::Horizontal;
    else
        return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
bool Utils::FractionalPrecisionByDenominator(const size_t prec, FractionalPrecision& out)
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
const size_t Utils::FractionalPrecisionDenominator(FractionalPrecision prec)
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
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
size_t Utils::AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str)
    {
    int cap = static_cast<int>(bufLen) - static_cast<int>(index) - 1;
    size_t strL = (nullptr == str) ? 0 : strlen(str);
    if (strL < 1 || cap < 1)
        return index;
    if (static_cast<int>(strL) > cap)
        strL = static_cast<size_t>(cap);
    memcpy(static_cast<void*>(buf + index), str, strL);
    index += strL;
    buf[index] = FormatConstant::EndOfLine();     
    return index;
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
    Utf8CP fd= FormatConstant::FUSDividers();
    Utf8CP df = FormatConstant::FUSDividerMatch();
    for (int i = 0; fd[i] != FormatConstant::EndOfLine(); i++)
        {
        if (div == fd[i])
            return df[i];
        }
    return FormatConstant::EndOfLine();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
// static
int Utils::IndexOf(Utf8Char c, Utf8CP text)
    {
    int indx = -1;
    if (Utf8String::IsNullOrEmpty(text))
        return indx;
    while (*text != FormatConstant::EndOfLine())
        {
        ++indx;
        if (c == *text)
            return indx;
        text++;
        }
    return -1;
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
//
// FormatUnitSet
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
void FormatUnitSet::Init()
    {
    m_formatSpec = nullptr;
    m_unitName.clear();
    m_unit = nullptr;
    m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
    m_localCopy = NamedFormatSpec();
    }

FormatUnitSet::FormatUnitSet(FormatUnitSetCR other)
    : m_formatSpec(other.m_formatSpec)
    , m_unitName(other.m_unitName)
    , m_unit(other.m_unit)
    , m_problem(other.m_problem)
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit, bool cloneData)
    {
    m_formatSpec = format;
    if (nullptr == m_formatSpec)
        m_formatSpec = s_stdFmtSet.FindNamedFormatSpec(FormatConstant::DefaultFormatName());
    
    m_unit = unit;
    if (nullptr == m_unit)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
    else
        {
        m_unitName = Utf8String(unit->GetName());
        if (cloneData)
            {
            m_localCopy = *m_formatSpec;
            m_formatSpec = &m_localCopy;
            }
        m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
FormatUnitSet& FormatUnitSet::operator=(const FormatUnitSet& other)
    {
    if (this != &other)
        {
        if (other.m_formatSpec == &other.m_localCopy)
            {
            m_localCopy = other.m_localCopy;
            m_formatSpec = &m_localCopy;
            }
        else
            m_formatSpec = other.m_formatSpec;

        m_unitName = other.m_unitName;
        m_unit = other.m_unit;
        m_problem = FormatProblemDetail(other.m_problem);
        }
    return *this;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void FormatUnitSet::LoadJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    Utf8CP paramName;
    Utf8String format;
    bool cloneData = false;
    bool local = false;
    m_formatSpec = nullptr;

    Init();
    if (jval.empty())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidJsonObject);
        return;
        }

    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_unitName()) == 0)
            {
            m_unitName = val.asString();
            if (nullptr == context)
                {
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
                continue;
                }

            m_unit = context->LookupUnit(m_unitName.c_str());
            if (nullptr == m_unit)
                {
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
                continue;
                }
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatName()) == 0)
            {
            format = val.asString();
            if (format.empty())
                m_formatSpec = s_stdFmtSet.FindNamedFormatSpec(FormatConstant::DefaultFormatName());
            else
                m_formatSpec = s_stdFmtSet.FindNamedFormatSpec(format.c_str());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_cloneData()) == 0)
            cloneData = val.asBool();
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatSpec()) == 0)
            {
            m_localCopy.FromJson(val, context);
            m_formatSpec = &m_localCopy;
            local = true;
            }
        }
    // last check if the referenced data is to be copied locally
    if(nullptr == m_formatSpec)
        m_formatSpec = s_stdFmtSet.FindNamedFormatSpec(FormatConstant::DefaultFormatName());
    if (cloneData && !local && (nullptr != m_formatSpec))
        {
        m_localCopy = *m_formatSpec;
        m_formatSpec = &m_localCopy;
        }
    }

//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(Utf8CP description, BEU::IUnitsContextCP context)
    {
    m_problem = FormatProblemDetail();
    description = Utils::SkipBlanks(description);
    if (Utils::IsJsonCandidate(description))
        {
        Json::Value jval (Json::objectValue);
        Json::Reader::Parse(description, jval);
        LoadJson(jval, context);
        }
    else
        {
        Utf8String fnam;
        Utf8String unit;
        ParseUnitFormatDescriptor(unit, fnam, description);
        if (Utf8String::IsNullOrEmpty(fnam.c_str()))
            m_formatSpec = s_stdFmtSet.FindNamedFormatSpec("DefaultReal");
        else
            m_formatSpec = s_stdFmtSet.FindNamedFormatSpec(fnam.c_str());
        m_unit = context->LookupUnit(unit.c_str());
        if (nullptr == m_formatSpec)
            m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
        else
            {
            if (nullptr == m_unit)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
            else
                m_unitName = Utf8String(m_unit->GetName());
            }
        }
    }

//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   Kyle.Abramowitz    02/2018
//----------------------------------------------------------------------------------------
void FormatUnitSet::ParseUnitFormatDescriptor(Utf8StringR unitName, Utf8StringR formatString, Utf8CP description)
    {
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1, FormatConstant::FUSDividers());
    FormattingWord fnam;
    FormattingWord unit;
    FormatDividerInstance fdt;
    FormatDividerInstance fdi = FormatDividerInstance(description, '|'); // check if this is a new format
    int n = fdi.GetDivCount();
    if (n == 2 && fdi.IsDivLast())
        {
        fnam = curs.ExtractLastEnclosure();
        unit = curs.ExtractBeforeEnclosure();
        }
    else if (n == 1 && !fdi.IsDivLast())
        {
        int loc = fdi.GetFirstLocation();
        if (loc > 0)
            {
            unit = curs.ExtractSegment(0, (size_t)loc - 1);
            fnam = curs.ExtractSegment((size_t)loc + 1, curs.GetTotalLength());
            }
        }
    else
        {
        fdi = FormatDividerInstance(description, '(');
        n = fdi.GetDivCount();
        if (n == 0)
            unit = curs.ExtractSegment(0, curs.GetTotalLength());
        if (fdi.BracketsMatched() && fdi.IsDivLast()) // there is a candidate for the format in parethesis
            {
            unit = curs.ExtractLastEnclosure();
            fdt = FormatDividerInstance(unit.GetText(), "/*");
            if (fdt.GetDivCount() == 0) // it can be a format name
                {
                fnam = unit;
                unit = curs.ExtractBeforeEnclosure();
                }
            else
                {
                unit = curs.ExtractSegment(0, curs.GetTotalLength());
                }
            }
        // dividers are not found - we assume a Unit name only
        }
    formatString = fnam.GetText();
    unitName = unit.GetText();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToText() const
    {
    if (HasProblem())
        return "";
    char buf[256];
    sprintf(buf, "%s(%s)", m_unit->GetName().c_str(), m_formatSpec->GetName().c_str());
    return buf;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool FormatUnitSet::IsIdentical(FormatUnitSetCR other) const
    {
    if(m_formatSpec != other.m_formatSpec) return false;
    if(strcmp(m_unitName.c_str(), other.m_unitName.c_str()) != 0) return false;
    if(m_unit != other.m_unit) return false;
    if(m_problem.GetProblemCode() != other.m_problem.GetProblemCode()) return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const
    {
    return NumericFormatSpec::StdFormatQuantity(*m_formatSpec, qty.ConvertTo(m_unit), nullptr, space);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Json::Value FormatUnitSet::FormatQuantityJson(BEU::QuantityCR qty, Utf8CP space) const
    {
    Utf8String str;
    Json::Value jval = ToJson();
    BEU::Quantity conv = qty.ConvertTo(m_unit);
    Utf8String txt = NumericFormatSpec::StdFormatQuantity(*m_formatSpec, conv, nullptr, space);
    jval[FormatConstant::FUSJsonValue()] = conv.GetMagnitude();
    jval[FormatConstant::FUSJsonDispValue()] = txt.c_str();
    return jval;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value FormatUnitSet::ToJson(bool verbose) const
    {
    Json::Value jval;
    jval[json_unitName()] = m_unit->GetName();
    if(verbose)
        jval[json_formatSpec()] = m_formatSpec->ToJson(true);
    else
        jval[json_formatName()] = m_formatSpec->GetName();
    return jval;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToJsonString(bool verbose) const
    {
    Utf8String str;
    Json::Value jval = ToJson(verbose);
    str = jval.ToString();
    return str;
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
// FormattingWord
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormattingWord::FormattingWord(FormattingScannerCursorP cursor, Utf8CP buffer, Utf8CP delim, bool isAscii)
    {
    m_cursor = cursor;
    m_word = "";
    size_t len = (nullptr == buffer) ? 0 : strlen(buffer);
    if(0 < len)
        m_word = buffer;
    len = (nullptr == delim) ? 0 : strlen(delim);
    memset(m_delim, 0, sizeof(m_delim));
    if (0 < len)
        memcpy(m_delim, delim, Utils::MinInt(maxDelim, len));
    m_isASCII = isAscii;
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
