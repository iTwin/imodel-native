/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatQuantity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// CompositeValueSpec Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::Init()
    {
    memset(m_ratio, 0, sizeof(m_ratio));
    m_unitProx.Clear();
    m_problem = FormatProblemDetail();
    m_type = CompositeSpecType::Undefined;
    }

//---------------------------------------------------------------------------------------
// The Ratio between Units must be a positive integer number. Otherwise forming a triad is not
//   possible (within the current triad concept). This function will return -1 if Units do not qualify:
//    1. Units do not belong to the same Phenomenon
//    2. Ratio of major/minor < 1
//    3. Ratio of major/minor is not an integer (within intrinsically defined tolerance)
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
size_t CompositeValueSpec::UnitRatio(BEU::UnitCP unit, BEU::UnitCP subunit)
    {
    if (nullptr == subunit) // subunit is not defined - which is OK regardless of whether the unit is defined
        return 0;

    if (nullptr == unit)  // this is not allowed because defined subunit requires unit to be defined
        UpdateProblemCode(FormatProblemCode::CNS_InconsistentUnitSet);
    else
        {
        if (unit->GetPhenomenon() != subunit->GetPhenomenon())
            {
            UpdateProblemCode(FormatProblemCode::CNS_UncomparableUnits);
            }
        double rat;
        unit->Convert(rat, 1.0, subunit);
        if (FormatConstant::IsNegligible(fabs(rat - floor(rat))))
            return static_cast<size_t>(rat);
        else
            UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
        }
    return 0;
    }

size_t CompositeValueSpec::UnitRatio(size_t uppIndx, size_t lowIndx)
    {
    BEU::UnitCP unit = m_unitProx.GetUnit(uppIndx);
    BEU::UnitCP subunit = m_unitProx.GetUnit(lowIndx);
    return  UnitRatio(unit, subunit);
    }

//---------------------------------------------------------------------------------------
// Checks comparability and calculates ratios between UOM of the parts and checks their consistency
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitRatios()
    {
    m_type = CompositeSpecType::Undefined;
    size_t ratioBits = 0; // the proper combinations are 0x1, 0x3, 0x7
    memset(m_ratio, 0, sizeof(m_ratio));
    m_ratio[indxMajor] = UnitRatio(m_unitProx.GetUnit(indxMajor), m_unitProx.GetUnit(indxMiddle));

    if (NoProblem())
        {
        if (1 < m_ratio[indxMajor]) ratioBits |= 0x1;
        m_ratio[indxMiddle] = UnitRatio(indxMiddle, indxMinor);
        if (1 < m_ratio[indxMiddle]) ratioBits |= 0x2;
        if (NoProblem())
            {
            m_ratio[indxMinor] = UnitRatio(indxMinor, indxSub);
            if (1 < m_ratio[indxMinor]) ratioBits |= 0x4;
            switch (ratioBits)
                {
                case 0x3:
                    m_type = CompositeSpecType::Triple;
                    break;
                case 0x7:
                    m_type = CompositeSpecType::Quatro;
                    break;
                case 0x1:
                    m_type = CompositeSpecType::Double;
                    break;
                case 0:
                    m_type = CompositeSpecType::Single;
                    break;
                default:
                    UpdateProblemCode(FormatProblemCode::CNS_InconsistentFactorSet);
                    break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitLabel(int index, Utf8CP label)
    {
    if (indxMajor <= index && index < indxLimit)
        m_unitLabel[index] = label;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitLabels(Utf8CP MajorLab, Utf8CP MiddleLab, Utf8CP MinorLab, Utf8CP SubLab)
{
    m_unitLabel[indxMajor] = MajorLab;
    m_unitLabel[indxMiddle] = MiddleLab;
    m_unitLabel[indxMinor] = MinorLab;
    m_unitLabel[indxSub] = SubLab;
}


//---------------------------------------------------------------------------------------
// returns the smallest partial unit or null if no units were defined
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
BEU::UnitCP CompositeValueSpec::GetSmallestUnit() const
    {
    switch (m_type)
        {
        case CompositeSpecType::Single: return m_unitProx.GetUnit(indxMajor);
        case CompositeSpecType::Double: return m_unitProx.GetUnit(indxMiddle);
        case CompositeSpecType::Triple: return m_unitProx.GetUnit(indxMinor);
        case CompositeSpecType::Quatro: return m_unitProx.GetUnit(indxSub);
        default: return nullptr;
        }
    }
//---------------------------------------------------------------------------------------
// returns true if error is detected. Otherwise - false - same as IsError()
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
bool CompositeValueSpec::SetUnitNames(Utf8CP MajorUnit, Utf8CP MiddleUnit, Utf8CP MinorUnit, Utf8CP SubUnit)
    {
    m_unitProx.Clear();
    if(!m_unitProx.SetUnitName(indxMajor, MajorUnit))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidMajorUnit);
    if (Utf8String::IsNullOrEmpty(MiddleUnit))
        return false;
    if (!m_unitProx.SetUnitName(indxMiddle, MiddleUnit))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);
    if (Utf8String::IsNullOrEmpty(MinorUnit))
        return false;
    if (!m_unitProx.SetUnitName(indxMinor, MinorUnit))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);
    if (Utf8String::IsNullOrEmpty(SubUnit))
        return false;
    if (!m_unitProx.SetUnitName(indxSub, SubUnit))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);
    return false;
    }


//---------------------------------------------------------------------------------------
// Constructor has three call formats that could use default values of arguments
//  Value of MajorToMiddle lesser than 2 will be treated as error because this
//  is designed for helping in breaking a single given value into subvalues according
//   to ratios. The processing algorithm 
//  could be this approach - is not prohibited
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
//CompositeValueSpec::CompositeValueSpec(size_t MajorToMiddle, size_t MiddleToMinor, size_t MinorToSub)
//    {
//    Init();
//    SetRatios(MajorToMiddle, MiddleToMinor, MinorToSub);
//    CheckRatios();
//    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCP MajorUnit, BEU::UnitCP MiddleUnit, BEU::UnitCP MinorUnit, BEU::UnitCP SubUnit)
    {
    Init();
    m_unitProx.SetUnit(indxMajor, MajorUnit);
    m_unitProx.SetUnit(indxMiddle, MiddleUnit);
    m_unitProx.SetUnit(indxMinor, MinorUnit);
    m_unitProx.SetUnit(indxSub, SubUnit);
    SetUnitRatios();
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(Utf8CP MajorUnit, Utf8CP MiddleUnit, Utf8CP MinorUnit, Utf8CP SubUnit)
    {
    Init();
    if (!SetUnitNames(MajorUnit, MiddleUnit, MinorUnit, SubUnit))
        SetUnitRatios();
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(CompositeValueSpecCP other)
    {
    m_unitProx.Copy(other->m_unitProx);
    //memcpy(m_units, other->m_units, sizeof(m_units));
    memcpy(m_ratio, other->m_ratio, sizeof(m_ratio));
    memcpy(m_unitLabel, other->m_unitLabel, sizeof(m_unitLabel));
    m_problem = other->m_problem;
    m_type = other->m_type;
    m_includeZero = other->m_includeZero;
    m_spacer = Utf8String(other->m_spacer);
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(CompositeValueSpecCR other)
    {
    m_unitProx.Copy(other.m_unitProx);
    //memcpy(m_units, other.m_units, sizeof(m_units));
    memcpy(m_ratio, other.m_ratio, sizeof(m_ratio));
    memcpy(m_unitLabel, other.m_unitLabel, sizeof(m_unitLabel));
    m_problem = other.m_problem;
    m_type = other.m_type;
    m_includeZero = other.m_includeZero;
    m_spacer = Utf8String(other.m_spacer);
    }


//Utf8CP CompositeValueSpec::GetProblemDescription() const
//    {
//    return Utils::FormatProblemDescription(m_problemCode).c_str();
//    }
//---------------------------------------------------------------------------------------
// The problem code will be updated only if it was not already set to some non-zero value
//   this approach is not the best, but witout a standard method for collection multiple 
//    problems it's better than override earlier encountered problems
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
//bool CompositeValueSpec::UpdateProblemCode(FormatProblemCode code)
//    {
//    if (m_problemCode == FormatProblemCode::NoProblems)
//        m_problemCode = code;
//    return IsProblem();
//    }

//---------------------------------------------------------------------------------------
// if uom is not provided we assume that the value is defined in the smallest units defined
//   in the current spec. 
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
CompositeValue CompositeValueSpec::DecomposeValue(double dval, BEU::UnitCP uom)
    {
    CompositeValue cv = CompositeValue();
    BEU::UnitCP smallest = GetSmallestUnit();
    double majorMinor = 0.0;
    double rem = 0.0;;
    double majorSub = 0.0;
    double middleSub = 0.0;

    if (NoProblem())  // don't try to decompose if the spec is not valid
        {
        if (!Utils::AreUnitsComparable(uom, smallest))
            {
            UpdateProblemCode(FormatProblemCode::CNS_UncomparableUnits);
            }
        else
            {
            BEU::Quantity smallQ;
            if (nullptr != uom) // we need to convert the given value to the smallest units
                {
                BEU::Quantity qty = BEU::Quantity(dval, *uom);
                smallQ = qty.ConvertTo(smallest);
                }
            else
                smallQ = BEU::Quantity(dval, *smallest);

            switch (m_type)
                {
                case CompositeSpecType::Single: // smallQ already has the converted value
                    cv.SetMajor(smallQ.GetMagnitude());
                    break;
                case CompositeSpecType::Double:
                    cv.SetMajor(floor(smallQ.GetMagnitude()/ (double)m_ratio[indxMajor]));
                    cv.SetMiddle(smallQ.GetMagnitude() - cv.GetMajor() * (double)m_ratio[indxMajor]);
                    break;
                case CompositeSpecType::Triple:
                    majorMinor = (double)(m_ratio[indxMajor] * m_ratio[indxMiddle]);
                    cv.SetMajor(floor((smallQ.GetMagnitude() + FormatConstant::FPV_RoundFactor()) / majorMinor));
                    rem = smallQ.GetMagnitude() - cv.GetMajor() * majorMinor;
                    cv.SetMiddle(floor((rem + +FormatConstant::FPV_RoundFactor()) / (double)m_ratio[indxMiddle]));
                    cv.SetMinor(rem - cv.GetMiddle() * (double)m_ratio[indxMiddle]);
                    break;
                case CompositeSpecType::Quatro:
                    majorSub = (double)(m_ratio[indxMajor] * m_ratio[indxMiddle] * m_ratio[indxMinor]);
                    middleSub = (double)(m_ratio[indxMiddle] * m_ratio[indxMinor]);
                    cv.SetMajor(floor((smallQ.GetMagnitude() + FormatConstant::FPV_RoundFactor()) / majorSub));
                    rem = smallQ.GetMagnitude() - cv.GetMajor() * majorSub;
                    cv.SetMiddle(floor((rem + FormatConstant::FPV_RoundFactor()) / middleSub));
                    rem -= cv.GetMiddle() * middleSub;
                    cv.SetMinor(floor((rem + FormatConstant::FPV_RoundFactor()) /(double)m_ratio[indxMinor]));
                    cv.SetSub(rem - cv.GetMinor() * (double)m_ratio[indxMinor]);
                    break;
                default:
                    break;
                }
            }
        }
    return cv;
    }

Utf8String CompositeValueSpec::FormatValue(double dval, NumericFormatSpecP fmtP, Utf8CP uomName)
    {
    Utf8String txt;
    BEU::UnitCP uom = Utils::IsNameNullOrEmpty(uomName)? nullptr : BEU::UnitRegistry::Instance().LookupUnit(uomName);
    CompositeValue cv = DecomposeValue(dval, uom);
    NumericFormatSpec fmtI = NumericFormatSpec(*fmtP);
    fmtI.SetDecimalPrecision(DecimalPrecision::Precision0);  // cloning spec,  but setting precision to 0 for integer parts
    Utf8String majT, midT, minT, subT;
    //NumericFormatSpec fmtI = NumericFormatSpec(PresentationType::Decimal, FormatConstant::DefaultSignOption(),
    //    FormatConstant::DefaultFormatTraits(), 0);
    Utf8CP spacer = GetSpacer().c_str();
    switch (m_type)
        {
        case CompositeSpecType::Single: // there is only one value to report
            majT = fmtP->FormatDouble(cv.GetMajor());
            majT = Utils::AppendUnitName(majT.c_str(), GetMajorLabel(nullptr).c_str(), spacer);
            break;

        case CompositeSpecType::Double:
            majT = fmtI.FormatDouble(cv.GetMajor());
            majT = Utils::AppendUnitName(majT.c_str(), GetMajorLabel(nullptr).c_str(), spacer);
            midT = fmtP->FormatDouble(cv.GetMajor());
            midT = Utils::AppendUnitName(midT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            majT += midT;
            break;

        case CompositeSpecType::Triple:
            majT = fmtI.FormatDouble(cv.GetMajor());
            majT = Utils::AppendUnitName(majT.c_str(), GetMajorLabel(nullptr).c_str(), spacer);
            midT = fmtI.FormatDouble(cv.GetMajor());
            midT = Utils::AppendUnitName(midT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            minT = fmtP->FormatDouble(cv.GetMajor() );
            minT = Utils::AppendUnitName(minT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            majT += midT + " " + minT;
            break;

        case CompositeSpecType::Quatro:
            majT = fmtI.FormatDouble(cv.GetMajor());
            majT = Utils::AppendUnitName(majT.c_str(), GetMajorLabel(nullptr).c_str(), spacer);
            midT = fmtI.FormatDouble(cv.GetMajor());
            midT = Utils::AppendUnitName(midT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            minT = fmtI.FormatDouble(cv.GetMajor());
            minT = Utils::AppendUnitName(minT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            subT = fmtP->FormatDouble(cv.GetMajor());
            subT = Utils::AppendUnitName(subT.c_str(), GetMiddleLabel(nullptr).c_str(), spacer);
            majT += midT + " " + minT + " " + subT;
            break;
        }
    return txt;
    }
//===================================================
//
// CompositeValue Methods
//
//===================================================
void CompositeValue::Init()
    {
    memset(m_parts, 0, sizeof(m_parts));
    m_problem = FormatProblemDetail();
    }

CompositeValue::CompositeValue()
    {
    Init();
    }

//bool CompositeValue::UpdateProblemCode(FormatProblemCode code)
//    {
//    return m_problem.UpdateProblemCode(code);
//    }


//===================================================
//
// NumericTriad Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericTriad::Convert()
    {
    if (!m_init)
        return;
    m_midAssigned = false;
    m_lowAssigned = false;

    double topmid = (double)m_topToMid;
    double midlow = (double)m_midToLow;
    double toplow = topmid * midlow;
    double rem = 0.0;
    /* if (m_decPrecision == DecimalPrecision::Precision0)
    m_dval = floor(m_dval + FormatConstant::FPV_RoundFactor());*/
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
            m_topValue = floor((m_dval + FormatConstant::FPV_RoundFactor()) / toplow);
            rem = m_dval - m_topValue * toplow;
            m_midValue = floor((rem + +FormatConstant::FPV_RoundFactor()) / midlow);
            m_lowValue = rem - m_midValue * midlow;
            m_midAssigned = true;
            m_lowAssigned = true;
            break;

        default:
            m_topValue = GetWhole();
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericTriad::SetValue(double dval)
    {
    m_dval = dval;
    m_negative = false;
    if (m_dval < 0.0)
        {
        m_negative = true;
        m_dval = -m_dval;
        }
    //m_decPrecision = prec;
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
NumericTriad::NumericTriad()
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
    //m_decPrecision = DecimalPrecision::Precision0;
    }


//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericTriad::NumericTriad(double dval, size_t topMid, size_t midLow)
    {
    SetValue(dval);
    m_topToMid = topMid;
    m_midToLow = midLow;
    m_init = true;
    Convert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericTriad::FormatWhole(DecimalPrecision prec)
    {
    NumericFormatSpec fmt = NumericFormatSpec();
    fmt.SetDecimalPrecision(prec);
    return fmt.FormatDouble(GetWhole());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericTriad::FormatTriad(Utf8CP topName, Utf8CP midName, Utf8CP lowName, Utf8CP space, int prec, bool fract, bool includeZero)
    {
    //NumericFormat fmt =  NumericFormat("Triad");
    PresentationType presType = fract ? PresentationType::Fractional : PresentationType::Decimal;
    ShowSignOption signOpt = FormatConstant::DefaultSignOption();
    FormatTraits formatTraits = FormatConstant::DefaultFormatTraits();
    NumericFormatSpec fmt = NumericFormatSpec(presType, signOpt, formatTraits, (size_t)prec);

    Utf8CP blank = FormatConstant::BlankString();
    topName = Utils::SubstituteEmptyOrNull(topName,blank);
    midName = Utils::SubstituteEmptyOrNull(midName, blank);
    lowName = Utils::SubstituteEmptyOrNull(lowName, blank);

    if (!m_midAssigned)
        {
        fmt.SetPrecisionByValue(prec);
        return fmt.FormatDouble(GetWhole());
        }

    fmt.SetDecimalPrecision(DecimalPrecision::Precision0);
    fmt.SetFormatTraits(FormatTraits::DefaultZeroes);
    Utf8String top = fmt.FormatDouble(m_negative ? -m_topValue : m_topValue);
    top.append(space);
    top.append(topName);
    Utf8String mid = "";
    Utf8String low = "";
    if (m_lowAssigned)
        {
        if (m_midValue > 0.0 || includeZero)
            mid = fmt.FormatDouble(m_midValue);
        if (m_lowValue > 0.0 || includeZero)
            {
            fmt.SetPrecisionByValue(prec);
            low = fmt.FormatDouble(m_lowValue);
            }
        }
    else if (m_midValue > 0.0 || includeZero)
        {
        fmt.SetPrecisionByValue(prec);
        mid = fmt.FormatDouble(m_midValue);
        }

    if ("" != mid)
        {
        top.append(blank);
        top.append(mid);
        top.append(space);
        top.append(midName);
        }
    if ("" != low)
        {
        top.append(blank);
        top.append(low);
        top.append(space);
        top.append(lowName);
        }
    return top;
    }




//===================================================
//
// QuantityTriadSpec Methods
//
//===================================================

void QuantityTriadSpec::Init(bool incl0)
    {
    m_quant = nullptr;
    m_topUnit = nullptr;
    m_midUnit = nullptr;
    m_lowUnit = nullptr;
    m_topUnitLabel = nullptr;
    m_midUnitLabel = nullptr;
    m_lowUnitLabel = nullptr;
    m_problem = FormatProblemDetail();
    m_includeZero = incl0;
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
QuantityTriadSpec::QuantityTriadSpec()
    {
    Init(false);
    }
//---------------------------------------------------------------------------------------
// The Ratio between Units must be a positive integer number. Otherwise forming a triad is not
//   possible (within the current triad concept). This function will return -1 if Units do not qualify:
//    1. Units do not belong to the same Phenomenon
//    2. Ratio of major/minor < 1
//    3. Ratio of major/minor is not an integer (within intrinsically defined tolerance)
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
size_t QuantityTriadSpec::UnitRatio(BEU::UnitCP major, BEU::UnitCP minor)
    {
    if (nullptr != major && nullptr != minor && (major->GetPhenomenon() == minor->GetPhenomenon()))
        {
        double rat;
        major->Convert(rat, 1.0, minor);
        if (FormatConstant::IsNegligible(fabs(rat - floor(rat))))
            return static_cast<int>(rat);
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// Using QuantityTriad is possible only when the quantity of the source value is defined and at least
//   the top UOM is also defined. Regarding middle and low Units wqe may have three valid cases:
//    1. Both of them not defined
//    2. Middle is defined but low is not (null)
//    3. Both of them are not defined
//  if either of three "target" UOM are defined their associated phenomenon must be the same as
//    the phenomenon of the source quantity
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
bool QuantityTriadSpec::ValidatePhenomenaPair(BEU::PhenomenonCP srcPhen, BEU::PhenomenonCP targPhen)
    {
    if (nullptr == srcPhen || nullptr == targPhen)
        return UpdateProblemCode(FormatProblemCode::QT_PhenomenonNotDefined);
    if (srcPhen != targPhen)
        return UpdateProblemCode(FormatProblemCode::QT_PhenomenaNotSame);
    return IsProblem();
    }

//---------------------------------------------------------------------------------------
// The problem code will be updated only if it was not already set to some non-zero value
//   this approach is not the best, but witout a standard method for collection multiple 
//    problems it's better than override earlier encountered problems
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
//bool QuantityTriadSpec::UpdateProblemCode(FormatProblemCode code)
//    {
//    if (m_problemCode == FormatProblemCode::NoProblems)
//        m_problemCode = code;
//    return m_problem.UpdateProblemCode(code);
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
QuantityTriadSpec::QuantityTriadSpec(BEU::QuantityCR qty, BEU::UnitCP topUnit, BEU::UnitCP midUnit, BEU::UnitCP lowUnit, bool incl0)
    {
    Init(incl0);
    int caseBits = 0;
    // before populating the structure - check validity of arguments combination  
    BEU::UnitCP unitQ = qty.GetUnit();
    BEU::PhenomenonCP phQ = nullptr;
    BEU::PhenomenonCP phTop = nullptr;
    BEU::PhenomenonCP phMid = nullptr;
    BEU::PhenomenonCP phLow = lowUnit->GetPhenomenon();

    if (nullptr != unitQ)
        {
        caseBits |= 0x1;  // source unit is present
        phQ = unitQ->GetPhenomenon();
        }
    if (nullptr != topUnit)
        {
        caseBits |= 0x2;  // top unit is present
        phTop = topUnit->GetPhenomenon();
        }
    if (nullptr != midUnit)
        {
        caseBits |= 0x4; // mid unit is present
        phMid = midUnit->GetPhenomenon();
        }
    if (nullptr != lowUnit)
        {
        caseBits |= 0x8; // low unit is present
        phLow = lowUnit->GetPhenomenon();
        }
    // only three combinations of bits indicate that the set of argument is sufficient for further investigation
    // 0011, 0111, 1111, all other combinations - indicate an erroneous argument list
    size_t topToMid = 0;
    size_t midToLow = 0;
    //double temp;

    switch (caseBits)
        {
        case 0x3:
            ValidatePhenomenaPair(phQ, phTop);
            if (!IsProblem())
                {
                m_topUnit = topUnit;
                BEU::Quantity temp = qty.ConvertTo(GetTopUnit());
                SetValue(temp.GetMagnitude());
                }
            break;
        case 0x7:
            topToMid = UnitRatio(topUnit, midUnit);
            ValidatePhenomenaPair(phQ, phTop);
            ValidatePhenomenaPair(phTop, phMid);
            if (!IsProblem())
                {
                if (topToMid > 1)
                    {
                    SetTopToMid(topToMid);
                    m_topUnit = topUnit;
                    m_midUnit = midUnit;
                    BEU::Quantity temp = qty.ConvertTo(GetMidUnit());
                    SetValue(temp.GetMagnitude());
                    }
                else
                    UpdateProblemCode(FormatProblemCode::QT_InvalidTopMidUnits);
                }
            break;
        case 0xF:
            topToMid = UnitRatio(topUnit, midUnit);
            midToLow = UnitRatio(midUnit, lowUnit);
            ValidatePhenomenaPair(phQ, phTop);
            ValidatePhenomenaPair(phTop, phMid);
            if (!IsProblem())
                {
                if (topToMid > 1)
                    SetTopToMid(topToMid);
                else
                    UpdateProblemCode(FormatProblemCode::QT_InvalidTopMidUnits);
                if (midToLow > 1)
                    SetMidToLow(midToLow);
                else
                    UpdateProblemCode(FormatProblemCode::QT_InvalidMidLowUnits);
                }
            if (!IsProblem())
                {
                m_topUnit = topUnit;
                m_midUnit = midUnit;
                m_lowUnit = lowUnit;
                BEU::Quantity temp = qty.ConvertTo(GetLowUnit());
                SetValue(temp.GetMagnitude());
                }
            break;
        default:
            UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
            break;
        }
    if (!IsProblem())
        {
        SetInit();
        Convert();
        }
    }

Utf8String QuantityTriadSpec::FormatQuantTriad(Utf8CP space, int prec, bool fract, bool includeZero)
    {
    if (IsProblem())
        return FormatConstant::FailedOperation();

    Utf8CP topUOM = (nullptr == m_topUnitLabel) ? GetTopUOM() : m_topUnitLabel;
    Utf8CP midUOM = (nullptr == m_midUnitLabel) ? GetMidUOM() : m_midUnitLabel;
    Utf8CP lowUOM = (nullptr == m_lowUnitLabel) ? GetLowUOM() : m_lowUnitLabel;
    return FormatTriad(topUOM, midUOM, lowUOM, space, prec, fract, includeZero);
    }



END_BENTLEY_FORMATTING_NAMESPACE