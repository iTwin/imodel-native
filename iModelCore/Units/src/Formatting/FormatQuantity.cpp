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
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit, BEU::UnitCP minorUnit, BEU::UnitCP subUnit)
    : m_includeZero(true)
    , m_spacer("")
    , m_ratio {0}
    {
    size_t unitCount = (nullptr != majorUnit)
        + (nullptr != middleUnit)
        + (nullptr != minorUnit)
        + (nullptr != subUnit);
    m_proxys.resize(unitCount);

    if (nullptr != majorUnit)
        m_proxys[indxMajor].SetUnit(majorUnit);
    if (nullptr != middleUnit)
        m_proxys[indxMiddle].SetUnit(middleUnit);
    if (nullptr != minorUnit)
        m_proxys[indxMinor].SetUnit(minorUnit);
    if (nullptr != subUnit)
        m_proxys[indxSub].SetUnit(subUnit);

    if (unitCount > 0)
        CalculateUnitRatios();
    else
        m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec()
    : CompositeValueSpec(nullptr, nullptr, nullptr, nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCR majorUnit)
    : CompositeValueSpec(&majorUnit, nullptr, nullptr, nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit)
    : CompositeValueSpec(&majorUnit, &middleUnit, nullptr, nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit)
    : CompositeValueSpec(&majorUnit, &middleUnit, &minorUnit, nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCR majorUnit, BEU::UnitCR middleUnit, BEU::UnitCR minorUnit, BEU::UnitCR subUnit)
    : CompositeValueSpec(&majorUnit, &middleUnit, &minorUnit, &subUnit)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(CompositeValueSpecCR other)
    : m_includeZero(other.m_includeZero)
    , m_spacer(other.m_spacer)
    , m_problem(other.m_problem)
    , m_proxys(other.m_proxys)
    {
    memcpy(m_ratio, other.m_ratio, sizeof(m_ratio));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
size_t CompositeValueSpec::CalculateUnitRatio(BEU::UnitCP upper, BEU::UnitCP lower)
    {
    if (nullptr == lower)
        {
        // Lower unit is not defined - which is OK regardless of whether the
        // upper is defined.
        return 0;
        }

    if (nullptr == upper)
        {
        // This should never occur, since units should always be defined in the
        // order of major, middle, minor, sub, which means the ratios
        // major/middle, middle/minor, and minor/sub should always have a
        // defined numerator.
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_InconsistentUnitSet);
        return 0;
        }

    if (upper->GetPhenomenon() != lower->GetPhenomenon())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_UncomparableUnits);
        return 0;
        }

    double ratio;
    upper->Convert(ratio, 1.0, lower);
    if (FormatConstant::IsNegligible(fabs(ratio - floor(ratio))))
        return static_cast<size_t>(ratio);
    m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
    return 0;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
void CompositeValueSpec::CalculateUnitRatios()
    {
    memset(m_ratio, 0, sizeof(m_ratio));
    for (size_t indx = indxMajor; indx < GetUnitCount()-1; ++indx)
        {
        m_ratio[indx] = CalculateUnitRatio(GetUnit(indx), GetUnit(indx+1));
        if (IsProblem())
            break;
        if (m_ratio[indx] < 1)
            m_problem.UpdateProblemCode(FormatProblemCode::CVS_InconsistentFactorSet);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
void CompositeValueSpec::SetUnitLabel(size_t index, Utf8CP label)
    {
    if (!IsIndexValid(index))
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
    if (!IsIndexValid(index))
        return substitute;

    auto proxy = GetProxyP(index);
    if (nullptr == proxy)
        return substitute;

    return proxy->HasLabel() ? proxy->GetLabel().c_str() : substitute;
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
    switch (GetUnitCount())
        {
        case 1: return GetUnit(indxMajor);
        case 2: return GetUnit(indxMiddle);
        case 3: return GetUnit(indxMinor);
        case 4: return GetUnit(indxSub);
        default: return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8CP CompositeValueSpec::GetUnitName(size_t indx, Utf8CP substitute) const
    {
    auto proxy = GetProxy(indx);
    if (nullptr == proxy)
        return substitute;

    Utf8CP name = proxy->GetName();
    return Utf8String::IsNullOrEmpty(name) ? substitute : name;
    }

//---------------------------------------------------------------------------------------
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

    if (!IsProblem())  // don't try to decompose if the spec is not valid
        {
        if (BEU::Unit::AreCompatible(uom, smallest))
            {
            m_problem.UpdateProblemCode(FormatProblemCode::CVS_UncomparableUnits);
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

            switch (GetUnitCount())
                {
                case 1: // smallQ already has the converted value
                    cv.SetMajor(smallQ.GetMagnitude());
                    break;
                case 2:
                    cv.SetMajor(floor(smallQ.GetMagnitude()/ (double)m_ratio[indxMajor]));
                    cv.SetMiddle(smallQ.GetMagnitude() - cv.GetMajor() * (double)m_ratio[indxMajor]);
                    break;
                case 3:
                    majorMinor = (double)(m_ratio[indxMajor] * m_ratio[indxMiddle]);
                    cv.SetMajor(floor((smallQ.GetMagnitude() + FormatConstant::FPV_RoundFactor()) / majorMinor));
                    rem = smallQ.GetMagnitude() - cv.GetMajor() * majorMinor;
                    cv.SetMiddle(floor((rem + FormatConstant::FPV_RoundFactor()) / (double)m_ratio[indxMiddle]));
                    cv.SetMinor(rem - cv.GetMiddle() * (double)m_ratio[indxMiddle]);
                    break;
                case 4:
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
    return m_problem.GetProblemCode() == other.m_problem.GetProblemCode()
        && GetUnitCount() == other.GetUnitCount()
        && m_includeZero == other.m_includeZero
        && m_spacer.Equals(other.m_spacer);
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
    switch (GetUnitCount())
        {
        case 4:
            proxP = GetProxy(indxSub);
            if(!proxP->IsEmpty())
                jCVS[json_SubUnit()] = proxP->ToJson();
        case 3:
            proxP = GetProxy(indxMinor);
            if (!proxP->IsEmpty())
                jCVS[json_MinorUnit()] = proxP->ToJson();
        case 2:
            proxP = GetProxy(indxMiddle);
            if (!proxP->IsEmpty())
                jCVS[json_MiddleUnit()] = proxP->ToJson();
        case 1: // smallQ already has the converted value
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

    m_proxys.resize(4);
    
    Utf8String input;
    UnitProxyP upp;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        str = val.ToString();
        if (BeStringUtilities::StricmpAscii(paramName, json_MajorUnit()) == 0)
            {
            upp = GetProxyP(indxMajor);
            upp->LoadJson(val, context);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_MiddleUnit()) == 0)
            {
            upp = GetProxyP(indxMiddle);
            upp->LoadJson(val, context);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_MinorUnit()) == 0)
            {
            upp = GetProxyP(indxMinor);
            upp->LoadJson(val, context);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_SubUnit()) == 0)
            {
            upp = GetProxyP(indxSub);
            upp->LoadJson(val, context);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_includeZero()) == 0)
            m_includeZero = val.asBool();
        else if (BeStringUtilities::StricmpAscii(paramName, json_spacer()) == 0)
            m_spacer = val.asString();
        }

    CalculateUnitRatios();
    }

//===================================================
//
// QuantityFormatting Methods
//
//===================================================

Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, double* persist, FormatUnitSetCR outputFUS, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode)
    {
    BEU::UnitCP persUnit = outputFUS.GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, inputFUS.GetUnit());
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

Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, FormatUnitSetCR inputFUS, FormatProblemCode* problemCode)
    {
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, inputFUS.GetUnit());
    BEU::Quantity qty = fps.GetQuantity(problemCode, &inputFUS);
    return qty;
    }

END_BENTLEY_FORMATTING_NAMESPACE
