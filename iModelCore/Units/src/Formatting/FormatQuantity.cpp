/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatQuantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"

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
        {
        UpdateProblemCode(FormatProblemCode::CNS_InconsistentUnitSet);
        return 0;
        }

    if (unit->GetPhenomenon() != subunit->GetPhenomenon())
        {
        UpdateProblemCode(FormatProblemCode::CNS_UncomparableUnits);
        return 0;
        }

    double rat;
    unit->Convert(rat, 1.0, subunit);
    if (FormatConstant::IsNegligible(fabs(rat - floor(rat))))
        return static_cast<size_t>(rat);
    else
        UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
    
    return 0;
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
    m_ratio[indxMajor] = UnitRatio(GetUnit(indxMajor), GetUnit(indxMiddle));

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
void CompositeValueSpec::SetUnitLabel(size_t index, Utf8CP label)
    {
    if (IsIndexCorrect(index))
        return;

    auto proxy = GetProxyP(index);
    if (nullptr == proxy)
        return;
    proxy->SetLabel(label);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
Utf8CP CompositeValueSpec::GetUnitLabel(size_t index, Utf8CP substitute) const
    {
    if (IsIndexCorrect(index))
        return substitute;

    auto proxy = GetProxyP(index);
    if (nullptr == proxy)
        return substitute;
    
    return proxy->GetLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitLabels(Utf8CP majorLabel, Utf8CP middleLabel, Utf8CP minorLabel, Utf8CP subLabel)
    {
    SetUnitLabel(indxMajor, majorLabel);
    SetUnitLabel(indxMiddle, middleLabel);
    SetUnitLabel(indxMinor, minorLabel);
    SetUnitLabel(indxSub, subLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8String CompositeValueSpec::GetEffectiveLabel(size_t indx) const
    {
    return GetUnitLabel(indx, GetUnitName(indx));
    }

//---------------------------------------------------------------------------------------
// returns the smallest partial unit or null if no units were defined
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
BEU::UnitCP CompositeValueSpec::GetSmallestUnit() const
    {
    switch (m_type)
        {
        case CompositeSpecType::Single: return GetUnit(indxMajor);
        case CompositeSpecType::Double: return GetUnit(indxMiddle);
        case CompositeSpecType::Triple: return GetUnit(indxMinor);
        case CompositeSpecType::Quatro: return GetUnit(indxSub);
        default: return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8CP CompositeValueSpec::GetUnitName(size_t indx, Utf8CP substitute) const
    {
    auto proxy = GetProxy(indx);
    if (nullptr != proxy)
        return substitute;

    Utf8CP name = proxy->GetName();
    return Utf8String::IsNullOrEmpty(name) ? substitute : name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::Clone(CompositeValueSpecCR other)
    {
    // m_unitProxy.Copy(other.m_unitProxy);
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
CompositeValueSpec::CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit, BEU::UnitCP minorUnit, BEU::UnitCP subUnit)
    {
    Init();
    SetUnit(indxMajor, majorUnit);
    SetUnit(indxMiddle, middleUnit);
    SetUnit(indxMinor, minorUnit);
    SetUnit(indxSub, subUnit);
    SetUnitRatios();
    }

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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool CompositeValueSpec::IsIdentical(CompositeValueSpecCR other) const
    {
    // TODO compare UnitProxies
    return m_problem.GetProblemCode() == other.m_problem.GetProblemCode() &&
        m_type == other.m_type && m_includeZero != other.m_includeZero &&
        m_spacer.Equals(other.m_spacer);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Json::Value CompositeValueSpec::ToJson() const
    {
    Json::Value jCVS;
    bool valid = false;
    bvector<Utf8CP> keyNames;
    UnitProxyCP proxP;
    switch (m_type)
        {
        case CompositeSpecType::Quatro:
            proxP = GetProxy(indxSub);
            if(!proxP->IsEmpty())
                jCVS[json_SubUnit()] = proxP->ToJson();
        case CompositeSpecType::Triple:
            proxP = GetProxy(indxMinor);
            if (!proxP->IsEmpty())
                jCVS[json_MinorUnit()] = proxP->ToJson();
        case CompositeSpecType::Double:
            proxP = GetProxy(indxMiddle);
            if (!proxP->IsEmpty())
                jCVS[json_MiddleUnit()] = proxP->ToJson();
        case CompositeSpecType::Single: // smallQ already has the converted value
            proxP = GetProxy(indxMajor);
            if (!proxP->IsEmpty())
                {
                jCVS[json_MajorUnit()] = proxP->ToJson();
                valid = true;
                }
            break;
        }

    if (valid)
        {
        jCVS[json_includeZero()] = IsIncludeZero();
        if (m_spacer.length() > 0)
            jCVS[json_spacer()] = m_spacer.c_str();
        }

    return jCVS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void CompositeValueSpec::LoadJsonData(JsonValueCR jval, BEU::IUnitsContextCP context)
    {
    Utf8CP paramName;
    Utf8String str;
    if (jval.empty())
        return;
    
    Utf8String input;
    UnitProxyP upp;
    int typeCount = 0;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        str = val.ToString();
        if (BeStringUtilities::StricmpAscii(paramName, json_MajorUnit()) == 0)
            {
            upp = GetProxyP(indxMajor);
            upp->LoadJson(val, context);
            typeCount++;
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_MiddleUnit()) == 0)
            {
            upp = GetProxyP(indxMiddle);
            upp->LoadJson(val, context);
            typeCount++;
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_MinorUnit()) == 0)
            {
            upp = GetProxyP(indxMinor);
            upp->LoadJson(val, context);
            typeCount++;
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_SubUnit()) == 0)
            {
            upp = GetProxyP(indxSub);
            upp->LoadJson(val, context);
            typeCount++;
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_InputUnit()) == 0)
            {
            input = val.asString();
            if (input.empty())
                continue;
            SetUnit(indxInput, context->LookupUnit(input.c_str()));
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_includeZero()) == 0)
            m_includeZero = val.asBool();
        else if (BeStringUtilities::StricmpAscii(paramName, json_spacer()) == 0)
            m_spacer = val.asString();
        }

    if (typeCount == 1)
        m_type = CompositeSpecType::Single;
    else if (typeCount == 2)
        m_type = CompositeSpecType::Double;
    else if (typeCount == 3)
        m_type = CompositeSpecType::Triple;
    else if (typeCount == 4)
        m_type = CompositeSpecType::Quatro;

    
    SetUnitRatios();
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

Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, size_t start, double* persist, FormatUnitSetCR outputFUS, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode)
    {
    BEU::UnitCP persUnit = outputFUS.GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, inputFUS.GetUnit());
    Formatting::FormatProblemCode locCode;
    if (nullptr == problemCode) problemCode = &locCode;
    *problemCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = fps.GetQuantity(problemCode, &inputFUS);
    if (*problemCode == Formatting::FormatProblemCode::NoProblems)
        {
        if (nullptr != persist)
            {
            BEU::Quantity persQty = qty.ConvertTo(persUnit);
            *persist = persQty.GetMagnitude();
            }
        }
    else if (nullptr != persist)
        *persist = 0.0;

    return qty;
    }

Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, size_t start, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode)
    {
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, inputFUS.GetUnit());
    BEU::Quantity qty = fps.GetQuantity(problemCode, &inputFUS);
    return qty;
    }


END_BENTLEY_FORMATTING_NAMESPACE
