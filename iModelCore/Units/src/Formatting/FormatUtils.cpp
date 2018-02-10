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
#include "../../Localization/xliffs/Units.xliff.h"
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_BENTLEY_FORMATTING_NAMESPACE

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

    // See Utils::FractionalPrecisionByDenominator
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
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::ScientificNorm, UNITSL10N_GETSTRING(PresentationType_ScientificNorm).c_str(), Utils::PresentationTypeName(PresentationType::ScientificNorm).c_str()));
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Stop100, UNITSL10N_GETSTRING(PresentationType_Stop100).c_str(), Utils::PresentationTypeName(PresentationType::Stop100).c_str()));
    presentationTypes.AddListEntry(UIListEntry((int) PresentationType::Stop1000, UNITSL10N_GETSTRING(PresentationType_Stop1000).c_str(), Utils::PresentationTypeName(PresentationType::Stop1000).c_str()));

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

    traits.AddListEntry(UIListEntry((int) FormatTraits::AppendUnitName, UNITSL10N_GETSTRING(FormatTraits_AppendUnitName).c_str(), json_AppendUnitName()));
    traits.AddListEntry(UIListEntry((int)FormatTraits::Use1000Separator, UNITSL10N_GETSTRING(FormatTraits_Use1000Separator).c_str(), json_Use1000Separator()));
    traits.AddListEntry(UIListEntry((int)FormatTraits::LeadingZeroes, UNITSL10N_GETSTRING(FormatTraits_LeadingZeroes).c_str(), json_LeadZeroes()));
    traits.AddListEntry(UIListEntry((int)FormatTraits::TrailingZeroes, UNITSL10N_GETSTRING(FormatTraits_TrailingZeroes).c_str(), json_TrailZeroes()));

   // TraitsBitToJson(jTraits, json_TrailZeroes(), FormatTraits::TrailingZeroes, &ref, verbose);
   // TraitsBitToJson(jTraits, json_LeadZeroes(), FormatTraits::LeadingZeroes, &ref, verbose);
   // TraitsBitToJson(jTraits, json_KeepDecPnt(), FormatTraits::KeepDecimalPoint, &ref, verbose);
   // TraitsBitToJson(jTraits, json_KeepSingleZero(), FormatTraits::KeepSingleZero, &ref, verbose);
   // TraitsBitToJson(jTraits, json_ExponentZero(), FormatTraits::ExponentZero, &ref, verbose);
   // TraitsBitToJson(jTraits, json_ZeroEmpty(), FormatTraits::ZeroEmpty, &ref, verbose);
   // TraitsBitToJson(jTraits, json_Use1000Separator(), FormatTraits::Use1000Separator, &ref, verbose);
   // TraitsBitToJson(jTraits, json_ApplyRounding(), FormatTraits::ApplyRounding, &ref, verbose);
   // TraitsBitToJson(jTraits, json_AppendUnitName(), FormatTraits::AppendUnitName, &ref, verbose);
   // TraitsBitToJson(jTraits, json_UseFractSymbol(), FormatTraits::UseFractSymbol, &ref, verbose);
   // TraitsBitToJson(jTraits, json_FractionDash(), FormatTraits::FractionDash, &ref, verbose);

    return traits;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
Json::Value UIUtils::GetAvailableUnitLabels(Utf8CP unitName)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        return Json::Value(Json::nullValue);

    BEU::UnitCP unit = BEU::UnitRegistry::Instance().LookupUnit(unitName);
    if (nullptr == unit)
        return Json::Value(Json::nullValue);

    Json::Value labels(Json::arrayValue);
    bvector<Utf8CP> synonyms;
    if (unit->GetSynonymList(synonyms) > 0)
        {
        for (size_t i = 0; i < synonyms.size(); i++)
            {
            labels.append(synonyms[i]);
            }
        }

    return labels;
    }

//===================================================
//
// FormatConstant Methods
//
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

size_t  FormatConstant::ExtractTrailingBits(unsigned char c, size_t shift) 
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

const Utf8CP FormatConstant::SpecialAngleSymbol(Utf8String name)
    {
    if (name.Equals("^")) return "ARC_DEG";
    //if (name.Equals(u8"°")) return "ARC_DEG";   
    if (name.Equals(u8"\xC2\xB0"))return "ARC_DEG";
    if (name.Equals(u8"\xB0"))return "ARC_DEG";
    if (name.Equals("'")) return "ARC_MINUTE";
    if (name.Equals("\"")) return "ARC_SECOND";     
    return nullptr;
    }

const Utf8CP FormatConstant::SpecialLengthSymbol(Utf8String name)
    {
    if (name.EqualsI("'")) return "FT";
    if (name.EqualsI("\"")) return "IN";
    return nullptr;
    }


//===================================================
//
// Utils Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String Utils::ShowSignOptionName(ShowSignOption opt)
    {
    switch (opt)
        {
        case ShowSignOption::OnlyNegative: return FormatConstant::FPN_OnlyNegative();
        case ShowSignOption::SignAlways: return FormatConstant::FPN_SignAlways();
        case ShowSignOption::NegativeParentheses: FormatConstant::FPN_NegativeParenths();
        default: return FormatConstant::FPN_NoSign();
        }
    }

ShowSignOption Utils::NameToSignOption(Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_OnlyNegative().c_str()) == 0) return ShowSignOption::OnlyNegative;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_SignAlways().c_str()) == 0) return ShowSignOption::SignAlways;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_NegativeParenths().c_str()) == 0) return ShowSignOption::NegativeParentheses;
    return ShowSignOption::NoSign;
    }

Utf8String Utils::FormatSpecTypeToName(FormatSpecType type)
    {
    switch (type)
        {
        case FormatSpecType::Numeric: return FormatConstant::FPN_Numeric();
        case FormatSpecType::Composite: return FormatConstant::FPN_Composite();
        default: return FormatConstant::FPN_Undefined();
        }
    }
FormatSpecType Utils::NameToFormatSpecType(Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Numeric().c_str()) == 0) return FormatSpecType::Numeric;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Composite().c_str()) == 0) return FormatSpecType::Composite;
    return FormatSpecType::Undefined;
    }

Utf8CP Utils::SkipBlanks(Utf8CP str)
    {
    while (isspace(*str))
        {
        str++;
        }
    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
Utf8Char Utils::GetFirstSignificantChar(Utf8CP str)
    {
    if (Utils::IsNameNullOrEmpty(str))
        return '\0';
    while (isspace(*str))
        {
        str++;
        }
    return *str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
Utf8Char Utils::GetLastSignificantChar(Utf8CP str)
    {
    size_t len = Utils::IsNameNullOrEmpty(str)? 0 : strlen(str);
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

bool Utils::IsJsonCandidate(Utf8CP str)
    {
    if ((GetFirstSignificantChar(str) == '{') &&  (GetLastSignificantChar(str) == '}')  )
        return true;
    return false;
    }

Utf8String Utils::AccumulatorStateName(AccumulatorState state)
    {
    switch (state)
        {
        case AccumulatorState::Init: return "init";
        case AccumulatorState::Complete: return "complete";
        case AccumulatorState::RejectedSymbol: return "rejectSymbol";
        case AccumulatorState::Exponent: return "exponent";
        case AccumulatorState::Fraction: return "fraction";
        case AccumulatorState::Integer: return "integer";
        case AccumulatorState::Failure: return "failure";
        default: return "???";
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String Utils::CharToString(Utf8Char c)
    {
    Utf8Char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return Utf8String(buf);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
DecimalPrecision Utils::DecimalPrecisionByIndex(size_t num)
    {
    if (num > FormatConstant::MaxDecimalPrecisionIndex())
        return FormatConstant::DefaultDecimalPrecision();
    switch (num)
        {
        case 0: return DecimalPrecision::Precision0;
        case 1: return DecimalPrecision::Precision1;
        case 2: return DecimalPrecision::Precision2;
        case 3: return DecimalPrecision::Precision3;
        case 4: return DecimalPrecision::Precision4;
        case 5: return DecimalPrecision::Precision5;
        case 6: return DecimalPrecision::Precision6;
        case 7: return DecimalPrecision::Precision7;
        case 8: return DecimalPrecision::Precision8;
        case 9: return DecimalPrecision::Precision9;
        case 10: return DecimalPrecision::Precision10;
        case 11: return DecimalPrecision::Precision11;
        case 12: return DecimalPrecision::Precision12;
        default: return DecimalPrecision::Precision0;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
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
Utf8CP Utils::GetParameterCategoryName(ParameterCategory parcat)
    {
    static Utf8CP CategoryNames[] = { "DataType", "Sign", "Presentation", "Zeroes", "DecPrecision", "FractPrecision", "RoundType",
        "FractionBar", "AngleFormat", "Alignment", "Separator", "Padding", "Mapping" };
    return CategoryNames[static_cast<int>(parcat)];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String  Utils::PresentationTypeName(PresentationType type)
    {
    switch (type)
        {
        case PresentationType::Fractional: return FormatConstant::FPN_Fractional();
        case PresentationType::Scientific: return FormatConstant::FPN_Scientific();
        case PresentationType::ScientificNorm: return FormatConstant::FPN_ScientificNorm();
        case PresentationType::Stop100: return FormatConstant::FPN_Stop100();
        case PresentationType::Stop1000: return FormatConstant::FPN_Stop1000();
        default:
        case PresentationType::Decimal: return FormatConstant::FPN_Decimal();
        }
    }

PresentationType Utils::NameToPresentationType(Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Fractional().c_str()) == 0) return PresentationType::Fractional;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Scientific().c_str()) == 0) return PresentationType::Scientific;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_ScientificNorm().c_str()) == 0) return PresentationType::ScientificNorm;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Stop100().c_str()) == 0) return PresentationType::Stop100;
    if (BeStringUtilities::StricmpAscii(name, FormatConstant::FPN_Stop1000().c_str()) == 0) return PresentationType::Stop1000;
    return PresentationType::Decimal;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
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

Utf8String  Utils::FractionBarName(FractionBarType bar)
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

FractionBarType Utils::NameToFractionBarType(Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, "diagonal") == 0) return FractionBarType::Diagonal;
    if (BeStringUtilities::StricmpAscii(name, "oblique") == 0) return FractionBarType::Oblique;
    if (BeStringUtilities::StricmpAscii(name, "horizontal") == 0) return FractionBarType::Horizontal;
    return FractionBarType::None;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String Utils::DecimalPrecisionName(DecimalPrecision prec)
    {
    switch (prec)
        {
        case DecimalPrecision::Precision1: return FormatConstant::FPN_Precision1();
        case DecimalPrecision::Precision2: return FormatConstant::FPN_Precision2();
        case DecimalPrecision::Precision3: return FormatConstant::FPN_Precision3();
        case DecimalPrecision::Precision4: return FormatConstant::FPN_Precision4();
        case DecimalPrecision::Precision5: return FormatConstant::FPN_Precision5();
        case DecimalPrecision::Precision6: return FormatConstant::FPN_Precision6();
        case DecimalPrecision::Precision7: return FormatConstant::FPN_Precision7();
        case DecimalPrecision::Precision8: return FormatConstant::FPN_Precision8();
        case DecimalPrecision::Precision9: return FormatConstant::FPN_Precision9();
        case DecimalPrecision::Precision10: return FormatConstant::FPN_Precision10();
        case DecimalPrecision::Precision11: return FormatConstant::FPN_Precision11();
        case DecimalPrecision::Precision12: return FormatConstant::FPN_Precision12();
        default:
        case DecimalPrecision::Precision0: return FormatConstant::FPN_Precision0();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
 Utf8String Utils::FractionallPrecisionName(FractionalPrecision prec)
    {
    switch (prec)
        {
        case FractionalPrecision::Half: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Quarter: return FormatConstant::FPN_FractPrec4();
        case FractionalPrecision::Eighth: return FormatConstant::FPN_FractPrec8();
        case FractionalPrecision::Sixteenth: return FormatConstant::FPN_FractPrec16();
        case FractionalPrecision::Over_32: return FormatConstant::FPN_FractPrec32();
        case FractionalPrecision::Over_64: return FormatConstant::FPN_FractPrec64();
        case FractionalPrecision::Over_128: return FormatConstant::FPN_FractPrec128();
        case FractionalPrecision::Over_256: return FormatConstant::FPN_FractPrec256();
        default:
        case FractionalPrecision::Whole: return FormatConstant::FPN_FractPrec1();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
FractionalPrecision Utils::FractionalPrecisionByDenominator(size_t prec)
    {
    switch (prec)
        {
        case 2: return FractionalPrecision::Half;
        case 4: return FractionalPrecision::Quarter;
        case 8: return FractionalPrecision::Eighth;
        case 16: return FractionalPrecision::Sixteenth;
        case 32: return FractionalPrecision::Over_32;
        case 64: return FractionalPrecision::Over_64;
        case 128: return FractionalPrecision::Over_128;
        case 256: return FractionalPrecision::Over_256;
        case 1:
        default:return FractionalPrecision::Whole;
        }
    }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
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




//Utf8String Utils::FormatProblemDescription(FormatProblemCode code)
//    {
//    switch (code)
//        {
//        case FormatProblemCode::UnknownStdFormatName: return "Unknown name of the standard format";
//        case FormatProblemCode::UnknownUnitName: return "Unknown name of the unit";
//        case FormatProblemCode::CNS_InconsistentFactorSet: return "Inconsistent set of factors";
//        case FormatProblemCode::CNS_InconsistentUnitSet: return "Inconsistent set of units";
//        case FormatProblemCode::CNS_UncomparableUnits: return "Units are not comparable";
//        case FormatProblemCode::CNS_InvalidUnitName: return "Unknown name of the Unit";
//        case FormatProblemCode::CNS_InvalidMajorUnit: return "Unknown name of the Major Unit";
//        case FormatProblemCode::QT_PhenomenonNotDefined: return "Unknown name of the Phenomenon";
//        case FormatProblemCode::QT_PhenomenaNotSame: return "Different Phenomena";
//        case FormatProblemCode::QT_InvalidTopMidUnits: return "Top and Middle units are not comparable";
//        case FormatProblemCode::QT_InvalidMidLowUnits: return "Middle and Low units are not comparable";
//        case FormatProblemCode::QT_InvalidUnitCombination: return "Invalid Unit combination ";
//        case FormatProblemCode::FUS_InvalidSyntax: return "Invalid syntax of FUS";
//        case FormatProblemCode::NoProblems:
//        default: return "No problems";
//        }
//    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
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
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
bool Utils::AreUnitsComparable(BEU::UnitCP un1, BEU::UnitCP un2)
    {
    BEU::PhenomenonCP ph1 = (nullptr == un1) ? nullptr : un1->GetPhenomenon();
    BEU::PhenomenonCP ph2 = (nullptr == un2) ? nullptr : un2->GetPhenomenon();
    return (ph1 == ph2);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------
Utf8String Utils::AppendUnitName(Utf8CP txtValue, Utf8CP unitName, Utf8CP space)
    {
    size_t txtL = Utils::IsNameNullOrEmpty(txtValue) ? 0 : strlen(txtValue);
    size_t uomL = Utils::IsNameNullOrEmpty(unitName) ? 0 : strlen(unitName);
    size_t spcL = Utils::IsNameNullOrEmpty(space) ? 0 : strlen(space);
    if (0 == txtL)
        txtValue = "";
    if (0 == uomL)
        return txtValue;
    size_t totL = txtL + uomL + spcL + 2;
    Utf8P buf = (char*)alloca(totL);
    BeStringUtilities::Snprintf(buf, totL, "%s%s%s", txtValue, ((0 == spcL)? "":space), unitName);
    return buf;
    }

Utf8CP Utils::HexByte(Utf8Char c, Utf8P buf, size_t bufLen)
    {
    static Utf8CP s = FormatConstant::HexSymbols();
    if (bufLen > 2)
        {
        size_t h = (size_t)(c & 0xF);
        buf[1] = s[h];
        h = (size_t)((c >> 4) & 0xF);
        buf[0] = s[h];
        buf[2] = FormatConstant::EndOfLine();
        }
    else if (nullptr != buf)
        buf[0] = FormatConstant::EndOfLine();
    return buf;
    }

size_t Utils::NumberOfUtf8Bytes(size_t code)
    {
    if (code < 0x80) return 1;
    if (code <  0x800) return 2;
    if (code < 0x10000) return 3;
    if (code < 0x200000) return 4;
    if (code < 0x4000000) return 5;
    return 6;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8String Utils::HexDump(Utf8CP txt, int len)
    {
    Utf8String str;
    int totL = 16;
    Utf8P buf = (char*)alloca(totL);
    buf[0] = ' ';
    buf[1] = '0';
    buf[2] = 'x';
    if (!Utils::IsNameNullOrEmpty(txt))
        {
        size_t actLen = strlen(txt);
        if (len < actLen)
            actLen = len;
        for (int i = 0; i < actLen; i++)
            {
            HexByte(txt[i], buf + 3, 10);
            str += Utf8String(buf);
            }
        }
    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
bool Utils::IsNameNullOrEmpty(Utf8CP name) 
    { 
    size_t len = (nullptr == name) ? 0 : strlen(name);
     return (len == 0); 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
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
int Utils::IndexOf(Utf8Char c, Utf8CP text)
    {
    int indx = -1;
    if (Utils::IsNameNullOrEmpty(text))
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
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
// this function scans through the given text string and returns a specially coded text string
//  representing types of the detected charactes using ASCII letters:
//    a - ASCII symbol occupying 1 byte
//    d - ASCII digit occupying 1 byte
//    . - ASCII point 
//    , - ASCII comma
//    e - 
//    l - Latin Char occupying 2 bytes
//    u - Unicode occupying 3 bytes
//    g - Unicode glyph occupying 4 bytes
//    v - Unicode char occupying 5 bytes  (reserved)
//    w - Unicode char occupying 6 bytes  (reserved)
//----------------------------------------------------------------------------------------
//Utf8String Utils::GetSignature(Utf8CP text)
//    {
//
//    }

 //===================================================
 //
 // FormatStopWatchMethods
 //
 //===================================================
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FormatStopWatch::FormatStopWatch()
     {
     m_start = std::chrono::steady_clock::now();
     m_lastInterval = 0.0;
     m_totalElapsed = 0.0;
     m_lastAmount = 0;
     m_totalAmount = 0;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FormatStopWatch::LastIntervalMetrics(size_t amount)
     {
     //m_lastInterval = GetElapsedSeconds();
     std::chrono::steady_clock::time_point moment = std::chrono::steady_clock::now();
     m_lastInterval = (double)std::chrono::duration_cast<std::chrono::microseconds>(moment - m_start).count();
     m_totalElapsed += m_lastInterval;
     m_lastAmount = amount;
     m_totalAmount += amount;
     NumericFormatSpec nfmt = NumericFormatSpec(6);
     nfmt.SetUse1000Separator(true);
     Utf8String amTxt = nfmt.FormatInteger((int)amount);
     Utf8String duraTxt = (amount > 0) ? nfmt.FormatDouble(m_lastInterval / (double)amount) : "n/a";
     Utf8String perfTxt = (m_lastInterval > 0.0) ? nfmt.FormatRoundedDouble((double)amount * 1.0e6 / m_lastInterval, 0.5) : "n/a";

     char buf[256];
     sprintf(buf, "Completed %s op's average duration %s mksec performance: %s op/sec", amTxt.c_str(), duraTxt.c_str(), perfTxt.c_str());
     return buf;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FormatStopWatch::LastInterval(double factor)
     {
     std::chrono::steady_clock::time_point moment = std::chrono::steady_clock::now();
     //std::chrono::duration<double> diff = moment - m_start;
     m_lastInterval = (double)std::chrono::duration_cast<std::chrono::microseconds>(moment - m_start).count();
     char buf[256];
     //m_lastInterval = GetElapsedSeconds();
     sprintf(buf, "%.4f UOR", m_lastInterval * factor);
     return Utf8String(buf);
     }

 //===================================================
 //
 // FractionalNumeric
 //
 //===================================================

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 size_t FractionalNumeric::GCF(size_t numer, size_t denom)
     {
     FactorizedNumber numF = FactorizedNumber(numer);
     FactorizedNumber denF = FactorizedNumber(denom);
     return numF.GetGreatestCommonFactor(denF);
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 void FractionalNumeric::Calculate(double dval, size_t denominator)
     {
     m_denominator = denominator;
     double integral = 0.0;
     double fract = modf(fabs(dval), &integral);
     double numer = floor(fract * m_denominator + FormatConstant::FPV_RoundFactor());
     m_numerator = static_cast<int>(numer);
     m_integral = static_cast<size_t>(integral);
     m_gcf = 1;
     if (0 != denominator && (m_numerator / m_denominator) == 1)
         {
         m_numerator = 0;
         m_integral += 1;
         }
     else
         m_gcf = GCF(m_numerator, m_denominator);
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FractionalNumeric::FractionalNumeric(double dval, FractionalPrecision fprec)
     {
     Calculate(dval, Utils::FractionalPrecisionDenominator(fprec));
     }
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FractionalNumeric::FractionalNumeric(double dval, int denominator)
     {
     Calculate(dval, denominator);
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FractionalNumeric::FractionalNumeric(double dval, int denominatorBase, double precision)
     {
     double maxPrec = 1.0;
     denominatorBase = abs(denominatorBase); // it must be positive
     double absDenom = (double)denominatorBase;
     int denominator = denominatorBase;
     if (denominator > 0)
        maxPrec /= absDenom;

     if (!FormatConstant::IsNegligible(precision))
         {
         while (maxPrec > precision)
             {
             maxPrec /= absDenom;
             denominator *= denominatorBase;
             }
         }
     Calculate(dval, denominator);
     }


 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
  void FractionalNumeric::FormTextParts(bool reduce)
     {
     NumericFormatSpec fmt =  NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, FormatTraits::DefaultTraits, 0);
     size_t numer = m_numerator;
     size_t denom = m_denominator;
     if (reduce && m_gcf > 1)
         {
         numer /= m_gcf;
         denom /= m_gcf;
         }
     m_textParts.push_back(fmt.FormatInteger((int)m_integral));
     if (numer > 0)
         {
         m_textParts.push_back(fmt.FormatInteger((int)numer));
         m_textParts.push_back(fmt.FormatInteger((int)denom));
         }
     }

  //----------------------------------------------------------------------------------------
  // @bsimethod                                                   David Fox-Rabinovitz 11/16
  //----------------------------------------------------------------------------------------
 Utf8String FractionalNumeric::GetIntegralString()
     {
     Utf8String strP;
     if (0 < m_textParts.size())
         strP = m_textParts.at(0);
     return strP ;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8String FractionalNumeric::GetNumeratorString()
     {
     Utf8String strP;
     if (3 <= m_textParts.size())
         strP = m_textParts.at(1);
     return strP;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8String FractionalNumeric::GetDenominatorString()
     {
     Utf8String strP;
     if (3 <= m_textParts.size())
         strP = m_textParts.at(2);
     return strP;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetIntegralText()
     {
     Utf8CP p = GetIntegralString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetDenominatorText()
     {
     Utf8CP p = GetDenominatorString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetNumeratorText()
     {
     Utf8CP p = GetNumeratorString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8String FractionalNumeric::ToTextDefault(bool reduce)
     {
     FormTextParts(reduce);
     if (m_numerator > 0)
        return GetIntegralString() + " " + GetNumeratorString() + "/" + GetDenominatorString();
     else
        return GetIntegralString();
     }

 //===================================================
 //
 // FactorPower
 //
 //===================================================
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 const size_t FactorPower::GetFactor()
     {
     size_t res = 1;
     
     for(size_t i = 0; i < m_power; i++)
         {
         res *= m_divisor;
         }
     return res;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 void FactorPower::CopyValues(FactorPowerP other)
     {
     if (nullptr == other)
         {
         m_divisor = 0;
         m_power = 0;
         m_index = 0;
         }
     else
         {
         m_divisor = other->m_divisor;
         m_power = other->m_power;
         m_index = other->m_index;
         }
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 void FactorPower::Merge(FactorPowerP fp1, FactorPowerP fp2)
     {
     m_divisor = 0;
     m_power = 0;
     m_index = -1;
     if (nullptr == fp1 || nullptr == fp2)
         return;
     if (fp1->m_index == fp2->m_index )
         m_index = fp1->m_index;
     if (fp1->m_divisor == fp2->m_divisor)
         {
         m_divisor = fp1->m_divisor;
         m_power = GetMin(fp1->m_power, fp2->m_power);
         }
     }
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorPower::ToText(Utf8Char pref)
     {
     char buf[128];
     memset(buf, 0, sizeof(buf));
     int i = 0;
     if (0 != pref)
         buf[i++] = pref;
     for (size_t n = 0; n < m_power; n++)
         {
         if (n > 0)
             buf[i++] = 'x';
         i += NumericFormatSpec::FormatIntegerSimple (static_cast<int>(m_divisor), buf + i, static_cast<int>(sizeof(buf)) - i, false, false);
         }
     return Utf8String(buf);
     }
 //===================================================
 //
 // FactorizedNumber
 //
 //===================================================
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 const size_t* FactorizedNumber::GetPrimes(int* length)
     {
     static const size_t prim[]{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
         79, 83, 89, 97, 101, 0}; // NOTE: the last number in  this sequence always must be 0
     if (nullptr != length)
         *length = static_cast<int>((sizeof(prim) -1)/ sizeof(size_t));
     return prim;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
size_t FactorizedNumber::GetPrimeCount()
     {
      int primN;
      GetPrimes(&primN);
      return static_cast<size_t>(primN);
     }
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 size_t FactorizedNumber::PowerOfPrime(size_t ival, size_t prim, size_t* result)
     {
     size_t rem = ival % prim;
     size_t pwr = 0;
     while (0 == rem && ival > 1)
         {
         pwr++;
         ival = ival / prim;
         rem = ival % prim;
         }
     if (nullptr != result)
         *result = ival;
     return pwr;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FactorizedNumber::FactorizedNumber(size_t ival)
     {
      m_ival = ival;
      int primM;
      const size_t* primN = GetPrimes(&primM);
      size_t pwr = 0;
      size_t num = ival;
      FactorPower fp;
      for (int i = 0; primN[i] > 0; i++)
          {
          pwr = PowerOfPrime(num, primN[i], &num);
          if (pwr > 0)
              {
              fp = FactorPower(primN[i], pwr, i);
              m_factors.push_back(fp);
              }
          }
      if(num > 1)
          {
          fp = FactorPower(num, 1, primM+1); // the last factor has index exceeding the number of primes in the base set
          m_factors.push_back(fp);
          }
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 size_t FactorizedNumber::RestoreNumber()
     {
     m_ival = 0;
     FactorPowerP fpp;
     if (m_factors.size() > 0)
         {
         m_ival = 1;
         for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
            {
             fpp = curr;
             m_ival *= fpp->GetFactor();
             //div = fpp->GetDivisor();
             //for (int i = 0; i < fpp->GetPower(); i++)
             //    {
             //    m_ival *= div;
             //    }
            }
         }
     return m_ival;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FactorPowerP FactorizedNumber::FindDivisor(int div)
     {
     FactorPowerP fpp;
     for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
         {
         fpp = curr;
         if (div == fpp->GetDivisor())
             return fpp;
         }
     return nullptr;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 void FactorizedNumber::ResetFactors(bvector<FactorPower> fact)
     {
     m_factors.clear();
     if (fact.size() < 1)
         return;
     FactorPowerP fpp;
     for (auto curr = fact.begin(), end = fact.end(); curr != end; curr++)
         {
         fpp = curr;
         if (0 < fpp->GetPower())
             m_factors.push_back(*curr);
         }
     RestoreNumber();
     return;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorizedNumber::ToText()
     {
     Utf8String txt = "1";
     FactorPowerP fpp;
     if (m_factors.size() > 0)
         {
         for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
             {
             fpp = curr;
             txt += fpp->ToText('x');
             }
         }
     return txt;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385) // static analysis thinks we exceed the bounds of fact3... I don't see how.
 size_t FactorizedNumber::GetGreatestCommonFactor(FactorizedNumber other)
     {
     size_t primN = GetPrimeCount();
     size_t primS = (primN + 2)* sizeof(FactorPower);
     FactorPowerP fact1 = (FactorPowerP)alloca(primS);
     FactorPowerP fact2 = (FactorPowerP)alloca(primS);
     memset(fact1, 0, primS);
     memset(fact2, 0, primS);
     FactorPower temp = FactorPower();

     FactorPowerP fpp;
     for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
         {
         fpp = curr;
         fact1[fpp->GetIndex()].CopyValues(fpp);
         }
     bvector<FactorPower> otherPwr = other.GetFactors();
     for (auto curr = otherPwr.begin(), end = otherPwr.end(); curr != end; curr++)
         {
         fpp = curr;
         fact2[fpp->GetIndex()].CopyValues(fpp);
         }
     size_t fact = 1;
     for (size_t i = 0; i < primN; i++)
         {
         temp.Merge(&fact1[i], &fact2[i]);
         fact *= temp.GetFactor();
         }

     return fact;
     }
POP_MSVC_IGNORE

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorizedNumber::DebugText()
     {
     char buf[256];
     sprintf(buf, "Value %d  (factors) %s ", static_cast<int>(m_ival), ToText().c_str());
     return Utf8String(buf);
     }
 
//===================================================
//
// FormatUnitSet
//
//===================================================

void FormatUnitSet::Init()
    {
    m_formatSpec = nullptr;
    m_unitName.clear();
    m_unit = nullptr;
    m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
    m_localCopy.Init();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit, bool cloneData)
    {
    m_formatSpec = format;
    m_unitName = Utf8String(unit->GetName());
    m_unit = unit;
    if (nullptr == m_formatSpec)
        m_formatSpec = StdFormatSet::FindFormatSpec(FormatConstant::DefaultFormatName());
    if (nullptr == m_formatSpec)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
    else if (nullptr == m_unit)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
    else
        {
        if (cloneData)
            {
            m_localCopy.Clone(m_formatSpec);
            m_formatSpec = &m_localCopy;
            }
        m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(Utf8CP formatName, Utf8CP unitName,  bool cloneData)
    {
    m_problem = FormatProblemDetail();
    m_unit = nullptr;
    if (Utils::IsNameNullOrEmpty(formatName))
        formatName = FormatConstant::DefaultFormatName();

	m_formatSpec = StdFormatSet::FindFormatSpec(formatName);
    if (nullptr == m_formatSpec)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
    else
        {
        m_unit = BEU::UnitRegistry::Instance().LookupUnitCI(unitName);
        if (nullptr == m_unit)
            m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
        else
            {
            m_unitName = unitName;
            if (cloneData)
                {
                m_localCopy.Clone(m_formatSpec);
                m_formatSpec = &m_localCopy;
                }
            m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
            }
        }
    }

FormatUnitSet& FormatUnitSet::operator=(const FormatUnitSet& other)
    {
    if (this != &other)
        {
        if (other.m_formatSpec == &other.m_localCopy)
            {
            m_localCopy.Clone(other.m_localCopy);
            m_formatSpec = &m_localCopy;
            }
        else
            {
            m_formatSpec = other.m_formatSpec;
            }

        m_unitName = other.m_unitName;
        m_unit = other.m_unit;
        m_problem = FormatProblemDetail(other.m_problem);
        }
    return *this;
    }

FormatUnitSet::FormatUnitSet(FormatUnitSetCR other)
{
	m_formatSpec = other.m_formatSpec;
	m_unitName = other.m_unitName;
	m_fusName.clear();
	m_unit = other.m_unit;
	m_problem = FormatProblemDetail(other.m_problem);
}

FormatUnitSet::FormatUnitSet(FormatUnitSetCP other)
{
	m_formatSpec = other->m_formatSpec;
	m_unitName = other->m_unitName;
	m_fusName.clear();
	m_unit = other->m_unit;
	m_problem = FormatProblemDetail(other->m_problem);
}


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void FormatUnitSet::LoadJson(Json::Value jval)
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
    //str = jval.ToString();
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_unitName()) == 0)
            {
            m_unitName = val.asString();
            m_unit = BEU::UnitRegistry::Instance().LookupUnitCI(m_unitName.c_str());
            if (nullptr == m_unit)
                {
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
                return;
                }
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatName()) == 0)
            {
            format = val.asString();
            if (format.empty())
                m_formatSpec = StdFormatSet::FindFormatSpec(FormatConstant::DefaultFormatName());
            else
                m_formatSpec = StdFormatSet::FindFormatSpec(format.c_str());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_cloneData()) == 0)
            cloneData = val.asBool();
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatSpec()) == 0)
            {
            m_localCopy.LoadJson(val);
            m_formatSpec = &m_localCopy;
            local = true;
            }
        }
    // last check if the referenced data is to be copied locally
    if(nullptr == m_formatSpec)
        m_formatSpec = StdFormatSet::FindFormatSpec(FormatConstant::DefaultFormatName());
    if (cloneData && !local && (nullptr != m_formatSpec))
        {
        m_localCopy.Clone(m_formatSpec);
        m_formatSpec = &m_localCopy;
        }
    }

//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(Utf8CP description)
    {
    m_problem = FormatProblemDetail();
    description = Utils::SkipBlanks(description);
    if (Utils::IsJsonCandidate(description))
        {
        Json::Value jval (Json::objectValue);
        Json::Reader::Parse(description, jval);
        LoadJson(jval);
        }
    else
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

        if (Utf8String::IsNullOrEmpty(fnam.GetText()))
            m_formatSpec = StdFormatSet::FindFormatSpec("DefaultReal");
        else
            m_formatSpec = StdFormatSet::FindFormatSpec(fnam.GetText());
        m_unit = BEU::UnitRegistry::Instance().LookupUnitCI(unit.GetText());
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
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToText(bool useAlias) const
    {
    if (HasProblem())
        return "";
    char buf[256];
    sprintf(buf, "%s(%s)", m_unit->GetName(), (useAlias? m_formatSpec->GetAlias() : m_formatSpec->GetName()));
    return buf;
    }

Utf8CP FormatUnitSet::GetDefaultDisplayLabel() const
{
	Utf8CP fnP = (nullptr == m_formatSpec) ? "#" : m_formatSpec->GetName();
	Utf8PrintfString lab("FUS_%s_%s", fnP, m_unitName.c_str());
	Utf8String dispLabel = BeSQLite::L10N::GetString(UnitsL10N::GetNameSpace(), BeSQLite::L10N::StringId(lab.c_str()));
	return dispLabel.c_str();
}

Utf8CP FormatUnitSet::GetDisplayLabel(bool useDefault) const
{
	if (m_fusName.empty() || useDefault)
		return GetDefaultDisplayLabel();

	Utf8PrintfString nam("FUS_%s", m_fusName.c_str());
	Utf8String dispLabel = BeSQLite::L10N::GetString(UnitsL10N::GetNameSpace(), BeSQLite::L10N::StringId(nam.c_str()));

	return dispLabel.c_str();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool FormatUnitSet::IsComparable(BEU::QuantityCR qty) const
    {
    return Utils::AreUnitsComparable(qty.GetUnit(), m_unit);
    }

bool FormatUnitSet::IsUnitComparable(Utf8CP unitName) const
    {
     BEU::UnitCP unit =  BEU::UnitRegistry::Instance().LookupUnitCI(unitName);
     return Utils::AreUnitsComparable(unit, m_unit);
    }


BEU::UnitCP FormatUnitSet::ResetUnit()
    {
    m_unit = BEU::UnitRegistry::Instance().LookupUnitCI(m_unitName.c_str());
    return m_unit;
    }

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
    Utf8String txt = NumericFormatSpec::StdFormatQuantity(*m_formatSpec, qty.ConvertTo(m_unit), nullptr, space);
    return txt;
    }


Json::Value FormatUnitSet::FormatQuantityJson(BEU::QuantityCR qty, bool useAlias, Utf8CP space) const
    {
    Utf8String str;
    Json::Value jval = ToJson(useAlias);
    BEU::Quantity conv = qty.ConvertTo(m_unit);
    Utf8String txt = NumericFormatSpec::StdFormatQuantity(*m_formatSpec, conv, nullptr, space);
    jval[FormatConstant::FUSJsonValue()] = conv.GetMagnitude();
    jval[FormatConstant::FUSJsonDispValue()] = txt.c_str();
    return jval;
    }


//===================================================
//
// FormatUnitGroup
//
//===================================================
//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------
FormatUnitGroup::FormatUnitGroup(Utf8CP name, Utf8CP description)
    {
    Utf8CP div = " ,;(){}";
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1, div);
    m_problem = FormatProblemDetail();
    m_name = name;
    curs.SkipBlanks();
    FormattingWord unit = curs.ExtractWord();
    FormattingWord fnam = curs.ExtractWord();
    Utf8Char unitDelim = unit.GetDelim();
    Utf8Char fnamDelim = fnam.GetDelim();
    while (m_problem.NoProblem())
        {
        if ('(' == unitDelim && ')' == fnamDelim)
            {
            FormatUnitSet fus = FormatUnitSet(fnam.GetText(), unit.GetText());
            if (fus.HasProblem())
                m_problem.UpdateProblemCode(fus.GetProblemCode());
            else
                {
                m_group.push_back(fus);
                unit = curs.ExtractWord();
                if (unit.IsEndLine())
                    break;
                if (unit.IsSeparator())
                    {
                    curs.SkipBlanks();
                    unit = curs.ExtractWord();
                    fnam = curs.ExtractWord();
                    unitDelim = unit.GetDelim();
                    fnamDelim = fnam.GetDelim();
                    }
                }
            }
        else
            m_problem.UpdateProblemCode(FormatProblemCode::FUS_InvalidSyntax);
        }
    }



Utf8String FormatUnitGroup::ToText(bool useAlias)
    {
    if (HasProblem())
        return "";
    Utf8String txt = Utils::SubstituteEmptyOrNull(m_name.c_str(), "unnamed");
    int i = 0;

    for (FormatUnitSetP fus = m_group.begin(); fus != m_group.end(); ++fus)
        {
        if (0 < i++)
            txt += ",";
        else
            txt += " ";
        txt += fus->ToText(useAlias);
        }

    return txt;
    }

FormatUnitSetCP FormatUnitGroup::GetPersistenceFUS() const
    {
    return (m_group.size() == 0) ? nullptr : &m_group[0];
    }

size_t FormatUnitGroup::GetPresentationFUSCount()
    {
    return m_group.size()-1;
    }

FormatUnitSetCP FormatUnitGroup::GetPresentationFUS(size_t index) const
    {
    index++;
    if (index < 1 || index >= m_group.size())
        return nullptr;
    FormatUnitSetCP fusP = &m_group[index];
    return fusP;
    }

bool FormatUnitGroup::IsIdentical(FormatUnitGroupCR other) const
    {
    if (strcmp(m_name.c_str(), other.m_name.c_str()) != 0) return false;
    if (m_group.size() != other.m_group.size()) return false;
    if (m_group.size() > 0)
        {
        for (size_t i = 0; i < m_group.size(); i++)
            {
            if (!m_group[i].IsIdentical(other.m_group[i])) return false;
            }
        }
    if (m_problem.GetProblemCode() != other.m_problem.GetProblemCode()) return false;
    return true;
    }


BEU::T_UnitSynonymVector* FormatUnitGroup::GetSynonymVector() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? nullptr : php->GetSynonymVector();
    }

size_t FormatUnitGroup::GetSynonymCount() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? 0 : php->GetSynonymCount();
    }

BEU::PhenomenonCP FormatUnitGroup::GetPhenomenon() const
    {
    Formatting::FormatUnitSet fus = GetPersistenceFUS();
    BEU::UnitCP un = fus.GetUnit();
    BEU::PhenomenonCP php = (nullptr == un) ? nullptr : un->GetPhenomenon();
    return php;
    }




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

FormattingDividers::FormattingDividers(FormattingDividersCR other)
    {
    memcpy(m_markers, other.m_markers, sizeof(m_markers));
    }

bool FormattingDividers::IsDivider(char c)
    {
    if (0 == c)
        return true;
    return (0 != ((m_markers[(c & 0x78) >> 3]) & (FormatConstant::TriadBitMask(c))));
    }


//===================================================
//
// FormattingWord
//
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
//
// NamedFormatSpec
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  01/2018
//---------------------------------------------------------------------------------------
void NamedFormatSpec::SetSuppressUnitLabel()
    {
    m_numericSpec.SetAppendUnit(false);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::Clone(NamedFormatSpecCR other)
    {
    m_name = other.m_name;
    m_alias = other.m_alias;
    m_description = other.m_description;
    m_displayLabel = other.m_displayLabel;
    m_numericSpec.Clone(other.m_numericSpec);
    m_compositeSpec.Clone(other.m_compositeSpec);
    m_specType = other.m_specType;
    m_problem.UpdateProblemCode(other.m_problem.GetProblemCode());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::Clone(NamedFormatSpecCP other)
    {
    m_name = other->m_name;
    m_alias = other->m_alias;
    m_description = other->m_description;
    m_displayLabel = other->m_displayLabel;
    m_numericSpec.Clone(other->m_numericSpec);
    m_compositeSpec.Clone(other->m_compositeSpec);
    m_specType = other->m_specType;
    m_problem.UpdateProblemCode(other->m_problem.GetProblemCode());
    }

NamedFormatSpec& NamedFormatSpec::operator=(const NamedFormatSpec& other)
    {
    if (this != &other)
        Clone(other);
        
    return *this;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec()
    {
    m_specType = FormatSpecType::Undefined;
    m_problem.UpdateProblemCode(FormatProblemCode::NFS_Undefined);
    }


 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
 
 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr);
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 02/17
 //----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias)
    {
    m_specType = FormatSpecType::Undefined;
    m_alias = alias;
    m_name = name;
    m_numericSpec = NumericFormatSpec(numSpec);
    m_compositeSpec.Clone(compSpec);
    m_specType = FormatSpecType::Composite;
    m_problem = FormatProblemDetail();
    if (Utils::IsNameNullOrEmpty(name))
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias)
    {
    m_specType = FormatSpecType::Undefined;
    m_alias = alias;
    m_name = name;
    m_numericSpec = NumericFormatSpec(numSpec);
    m_specType = FormatSpecType::Numeric;
    //m_compositeSpec(); // = CompositeValueSpec();
    m_problem = FormatProblemDetail();
    if (Utils::IsNameNullOrEmpty(name))
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool NamedFormatSpec::IsIdentical(NamedFormatSpecCR other) const
    {
    int cod = 0;
    while (0 == cod)
        {
        if (!m_name.Equals(other.m_name)) { cod = 1; break; }
        if (!m_alias.Equals(other.m_alias)) { cod = 2; break; }
        if (!m_numericSpec.IsIdentical(other.m_numericSpec)) { cod = 3; break; }
        if (!m_compositeSpec.IsIdentical(other.m_compositeSpec)) { cod = 4; break; }
        if (m_specType != other.m_specType) { cod = 5; break; }
        if (m_problem.GetProblemCode() != other.m_problem.GetProblemCode()) { cod = 6; break; }
        break;
        }
    if(0 == cod)
       return true;
    return false;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool NamedFormatSpec::HasName(Utf8CP name) const 
    {
    if (Utils::IsNameNullOrEmpty(name))
        return false;
    return (0 == BeStringUtilities::StricmpAscii(name, m_name.c_str())); 
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool NamedFormatSpec::HasAlias(Utf8CP name) const
    {
    if (Utils::IsNameNullOrEmpty(name))
        return false;
    return (0 == BeStringUtilities::StricmpAscii(name, m_alias.c_str()));
    }

//===================================================
//
// FormatProblemDetail
//
//===================================================
Utf8String FormatProblemDetail::GetProblemDescription() const
    {
    switch (m_code)
        {
        case FormatProblemCode::UnknownStdFormatName: return "Unknown name of the standard format";
        case FormatProblemCode::UnknownUnitName: return "Unknown name of the unit";
        case FormatProblemCode::CNS_InconsistentFactorSet: return "Inconsistent set of factors";
        case FormatProblemCode::CNS_InconsistentUnitSet: return "Inconsistent set of units";
        case FormatProblemCode::CNS_UncomparableUnits: return "Units are not comparable";
        case FormatProblemCode::CNS_InvalidUnitName: return "Unknown name of the Unit";
        case FormatProblemCode::CNS_InvalidMajorUnit: return "Unknown name of the Major Unit";
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
        case FormatProblemCode::NoProblems:
        default: return "No problems";
        }
    }


/*

FUS_InvalidSyntax = 20151,
NFS_InvalidSpecName = 20161,
NFS_DuplicateSpecName = 20162,
DIV_UnknownDivider = 25001,
NA_InvalidSign = 25101,             // Numeric Accumulator problems
NA_InvalidPoint = 25102,
NA_InvalidExponent = 25103
};

*/

bool FormatProblemDetail::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_code == FormatProblemCode::NoProblems)
        m_code = code;
    return IsProblem();
    }

//===================================================
//
// QuantityFraction
//
//===================================================
//void QuantityFraction::SetSigned(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Signed);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Signed);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::SetDecPoint(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::DecPoint);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::DecPoint);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//void QuantityFraction::SetExponent(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Exponent);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Exponent);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::SetUom(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Uom);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Uom);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::Detect(Utf8CP txt)
//    {
//    Init();
//    int phase = 0;
//    while (m_problem.NoProblem())
//        {
//        switch (phase)
//            {
//            case 0:
//                break;
//            case 1:
//                break;
//            case 2:
//                break;
//            case 3:
//                break;
//            case 4:
//                break;
//            default:
//                break;
//            }
//        }
//    }


END_BENTLEY_FORMATTING_NAMESPACE


