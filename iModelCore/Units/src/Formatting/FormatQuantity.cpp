/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatQuantity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/Formatting.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// ComboNumberSpec Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// The Ratio between Units must be a positive integer number. Otherwise forming a triad is not
//   possible (within the current triad concept). This function will return -1 if Units do not qualify:
//    1. Units do not belong to the same Phenomenon
//    2. Ratio of major/minor < 1
//    3. Ratio of major/minor is not an integer (within intrinsically defined tolerance)
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
size_t ComboNumberSpec::UnitRatio(UnitCP major, UnitCP minor)
    {
    if (nullptr != major && nullptr != minor && (major->GetPhenomenon() == minor->GetPhenomenon()))
        {
        double rat = major->Convert(1.0, minor);
        if (FormatConstant::IsNegligible(fabs(rat - floor(rat))))
            return static_cast<size_t>(rat);
        }
    return 0;
    }

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
    NumericFormatSpec fmt("FW");
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
    NumericFormatSpec fmt = NumericFormatSpec("Triad", presType, signOpt, formatTraits, (size_t)prec);

    Utf8CP blank = FormatConstant::BlankString();
    if (IsNameNullOrEmpty(topName))
        topName = blank;
    if (IsNameNullOrEmpty(midName))
        midName = blank;
    if (IsNameNullOrEmpty(lowName))
        lowName = blank;

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
    m_problemCode = FormatProblemCode::NoProblems;
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
size_t QuantityTriadSpec::UnitRatio(UnitCP major, UnitCP minor)
    {
    if (nullptr != major && nullptr != minor && (major->GetPhenomenon() == minor->GetPhenomenon()))
        {
        double rat = major->Convert(1.0, minor);
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
bool QuantityTriadSpec::ValidatePhenomenaPair(PhenomenonCP srcPhen, PhenomenonCP targPhen)
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
bool QuantityTriadSpec::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_problemCode == FormatProblemCode::NoProblems)
        m_problemCode = code;
    return IsProblem();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
QuantityTriadSpec::QuantityTriadSpec(QuantityCR qty, UnitCP topUnit, UnitCP midUnit, UnitCP lowUnit, bool incl0)
    {
    Init(incl0);
    int caseBits = 0;
    // before populating the structure - check validity of arguments combination  
    UnitCP unitQ = qty.GetUnit();
    PhenomenonCP phQ = nullptr;
    PhenomenonCP phTop = nullptr;
    PhenomenonCP phMid = nullptr;
    PhenomenonCP phLow = lowUnit->GetPhenomenon();

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
                QuantityPtr temp = qty.ConvertTo(GetTopUOM());
                SetValue(temp->GetMagnitude());
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
                    QuantityPtr temp = qty.ConvertTo(GetMidUOM());
                    SetValue(temp->GetMagnitude());
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
                QuantityPtr temp = qty.ConvertTo(GetLowUOM());
                SetValue(temp->GetMagnitude());
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