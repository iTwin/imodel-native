/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatQuantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    m_includeZero = true;
    m_spacer = "";
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
        m_unitProx.SetUnitLabel(index, label);   // m_unitLabel[index] = label;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitLabels(Utf8CP MajorLab, Utf8CP MiddleLab, Utf8CP MinorLab, Utf8CP SubLab)
{
    m_unitProx.SetUnitLabel(indxMajor,MajorLab);
    m_unitProx.SetUnitLabel(indxMiddle,MiddleLab);
    m_unitProx.SetUnitLabel(indxMinor,MinorLab);
    m_unitProx.SetUnitLabel(indxSub,SubLab);
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
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::Clone(CompositeValueSpecCR other)
    {
    m_unitProx.Copy(other.m_unitProx);
    memcpy(m_ratio, other.m_ratio, sizeof(m_ratio));
    m_problem = other.m_problem;
    m_type = other.m_type;
    m_includeZero = other.m_includeZero;
    m_spacer = Utf8String(other.m_spacer);
    }

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
   // memcpy(m_unitLabel, other->m_unitLabel, sizeof(m_unitLabel));
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
    //memcpy(m_unitLabel, other.m_unitLabel, sizeof(m_unitLabel));
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
    if (dval < 0.0)
        {
        cv.SetNegative();
        dval = -dval;
        }

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
                    cv.SetMiddle(floor((rem + FormatConstant::FPV_RoundFactor()) / (double)m_ratio[indxMiddle]));
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

bool CompositeValueSpec::IsIdentical(CompositeValueSpecCR other) const
    {
    int cod = 0;
    while (cod == 0)
        {
        if (!m_unitProx.IsIdentical(other.m_unitProx)) { cod = 1; break;}
        if (m_problem.GetProblemCode() != other.m_problem.GetProblemCode()) { cod = 2; break; }
        if (m_type != other.m_type) { cod = 3; break; }
        if (m_includeZero != other.m_includeZero) { cod = 4; break; }
        if (!m_spacer.Equals(other.m_spacer)) { cod = 5; break; }
        break;
        }
    if (cod == 0) return true;
    return false;
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
    m_negative = false;
    }

CompositeValue::CompositeValue()
    {
    Init();
    }

END_BENTLEY_FORMATTING_NAMESPACE