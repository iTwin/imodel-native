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
// CompositeValueSpec Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::Init()
    {
    memset(m_ratio, 0, sizeof(m_ratio));
    memset(m_units, 0, sizeof(m_units));
    m_problemCode = FormatProblemCode::NoProblems;
    m_type = CompositeSpecType::Undefined;
    m_formatSpec = nullptr;
    }

//---------------------------------------------------------------------------------------
// The Ratio between Units must be a positive integer number. Otherwise forming a triad is not
//   possible (within the current triad concept). This function will return -1 if Units do not qualify:
//    1. Units do not belong to the same Phenomenon
//    2. Ratio of major/minor < 1
//    3. Ratio of major/minor is not an integer (within intrinsically defined tolerance)
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
size_t CompositeValueSpec::UnitRatio(UnitCP unit, UnitCP subunit)
    {
    if (nullptr == subunit) // subunit is not defined - which is OK regardless of wheteh the unit is defined
        return 0;
    if (nullptr != unit)  // this is not allowed because defined subunit requires unit to be defined
        {
        if (unit->GetPhenomenon() != subunit->GetPhenomenon())
            {
            UpdateProblemCode(FormatProblemCode::CNS_UncomparableUnits);
            }
        double rat = unit->Convert(1.0, subunit);
        if (FormatConstant::IsNegligible(fabs(rat - floor(rat))))
            return static_cast<size_t>(rat);
        else
            UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
        }
    else
        UpdateProblemCode(FormatProblemCode::CNS_InconsistentUnitSet);
    return 0;
    }

void CompositeValueSpec::CheckRatios()
    {
    size_t ratioBits = 0; // the proper combinations are 0x1, 0x3, 0x7
    if (1 < m_ratio[indxMajor]) ratioBits |= 0x1;
    if (1 < m_ratio[indxMiddle]) ratioBits |= 0x2;
    if (1 < m_ratio[indxMinor]) ratioBits |= 0x4;
    m_type = CompositeSpecType::Undefined;

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

void CompositeValueSpec::SetRatios(size_t MajorToMiddle, size_t MiddleToMinor, size_t MinorToSub)
    {
    m_ratio[indxMajor] = MajorToMiddle;
    m_ratio[indxMiddle] = MiddleToMinor;
    m_ratio[indxMinor] = MinorToSub;
    }

void CompositeValueSpec::SetUnits(UnitCP MajorUnit, UnitCP MiddleUnit, UnitCP MinorUnit, UnitCP SubUnit)
    {
    m_units[indxMajor] = MajorUnit;
    m_units[indxMiddle] = MiddleUnit;
    m_units[indxMinor] = MinorUnit;
    m_units[indxSub] = SubUnit;
    }

void CompositeValueSpec::SetUnitRatios()
    {
    m_ratio[indxMajor] = UnitRatio(m_units[indxMajor], m_units[indxMiddle]);
    m_ratio[indxMiddle] = UnitRatio(m_units[indxMiddle], m_units[indxMinor]);
    m_ratio[indxMinor] = UnitRatio(m_units[indxMinor], m_units[indxSub]);
    }

//---------------------------------------------------------------------------------------
// returns the smallest partial unit or null if no units were defined
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
UnitCP CompositeValueSpec::GetSmallestUnit()
    {
    if (CompositeSpecType::Undefined != m_type)
        {
        for (int i = indxSub; i >= 0; i--)
            {
            if (nullptr != m_units[i])
                return m_units[i];
            }
        }
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// returns true if error is detected. Otherwise - false - same as IsError()
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
bool CompositeValueSpec::SetUnitNames(Utf8CP MajorUnit, Utf8CP MiddleUnit, Utf8CP MinorUnit, Utf8CP SubUnit)
    {
    UnitCP un = UnitRegistry::Instance().LookupUnit(MajorUnit);
    if (nullptr == un)
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidMajorUnit);

    m_units[indxMajor] = un;
    if (nullptr == (un = UnitRegistry::Instance().LookupUnit(MiddleUnit)))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);

    m_units[indxMiddle] = un;
    if (nullptr == (un = UnitRegistry::Instance().LookupUnit(MinorUnit)))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);

    m_units[indxMinor] = un;
    if (nullptr == (un = UnitRegistry::Instance().LookupUnit(SubUnit)))
        return UpdateProblemCode(FormatProblemCode::CNS_InvalidUnitName);
    m_units[indxSub] = un;
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
CompositeValueSpec::CompositeValueSpec(size_t MajorToMiddle, size_t MiddleToMinor, size_t MinorToSub)
    {
    Init();
    SetRatios(MajorToMiddle, MiddleToMinor, MinorToSub);
    CheckRatios();
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(UnitCP MajorUnit, UnitCP MiddleUnit, UnitCP MinorUnit, UnitCP SubUnit)
    {
    Init();
    SetUnits(MajorUnit, MiddleUnit, MinorUnit, SubUnit);
    SetUnitRatios();
    CheckRatios();
    }



//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(Utf8CP MajorUnit, Utf8CP MiddleUnit, Utf8CP MinorUnit, Utf8CP SubUnit)
    {
    Init();
    if (!SetUnitNames(MajorUnit, MiddleUnit, MinorUnit, SubUnit))
        {
        SetUnitRatios();
        CheckRatios();
        }
    }

//---------------------------------------------------------------------------------------
// Using CompositeValueSpec is possible only when the quantity of the source value is defined and at least
//   the top UOM is also defined. Regarding middle and low Units wqe may have three valid cases:
//    1. Both of them not defined
//    2. Middle is defined but low is not (null)
//    3. Both of them are not defined
//  if either of three "target" UOM are defined their associated phenomenon must be the same as
//    the phenomenon of the source quantity
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
bool CompositeValueSpec::ValidatePhenomenaPair(PhenomenonCP srcPhen, PhenomenonCP targPhen)
    {
    if (nullptr == srcPhen || nullptr == targPhen)
        return UpdateProblemCode(FormatProblemCode::QT_PhenomenonNotDefined);
    if (srcPhen != targPhen)
        return UpdateProblemCode(FormatProblemCode::QT_PhenomenaNotSame);
    return IsProblem();
    }

Utf8CP CompositeValueSpec::GetProblemDescription()
    {
    return Utils::GetFormatProblemDescription(m_problemCode);
    }
//---------------------------------------------------------------------------------------
// The problem code will be updated only if it was not already set to some non-zero value
//   this approach is not the best, but witout a standard method for collection multiple 
//    problems it's better than override earlier encountered problems
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
bool CompositeValueSpec::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_problemCode == FormatProblemCode::NoProblems)
        m_problemCode = code;
    return IsProblem();
    }

//---------------------------------------------------------------------------------------
// if uom is not provided we assume that the value is defined in the smallest units defined
//   in the current spec. 
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
//CompositeValue CompositeValueSpec::DecomposeValue(double dval, UnitCP uom)
//    {
//    CompositeValue cv = CompositeValue();
//    UnitCP smallest = GetSmallestUnit();
//    if (!Utils::AreUnitsComparable(uom, smallest))
//        {
//        UpdateProblemCode(FormatProblemCode::CNS_UncomparableUnits);
//        }
//    else
//        {
//        if (nullptr != uom) // we need to convert the given value to the smallest units
//            {
//            QuantityPtr qty = Quantity::Create(dval, uom);
//
//            }
//        }
//    }

//===================================================
//
// CompositeValue Methods
//
//===================================================
void CompositeValue::Init()
    {
    memset(m_parts, 0, sizeof(m_parts));
    m_problemCode = FormatProblemCode::NoProblems;
    }

CompositeValue::CompositeValue()
    {
    Init();
    }

bool CompositeValue::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_problemCode == FormatProblemCode::NoProblems)
        m_problemCode = code;
    return IsProblem();
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
                Quantity temp = qty.ConvertTo(GetTopUnit());
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
                    Quantity temp = qty.ConvertTo(GetMidUnit());
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
                Quantity temp = qty.ConvertTo(GetLowUnit());
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