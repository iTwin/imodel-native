/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/CompositeValueSpec.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
// CompositeValueSpec
//===================================================

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValueSpec(BEU::UnitCP majorUnit, BEU::UnitCP middleUnit, BEU::UnitCP minorUnit, BEU::UnitCP subUnit)
    : m_includeZero(true)
    , m_explicitlyDefinedSpacer(false)
    , m_spacer(FormatConstant::DefaultSpacer())
    , m_ratio {0}
    {
    size_t unitCount = (nullptr != majorUnit)
        + (nullptr != middleUnit)
        + (nullptr != minorUnit)
        + (nullptr != subUnit);
    m_proxys.reserve(4);
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
CompositeValueSpec::CompositeValueSpec(bvector<BEU::UnitCP> const& units)
    : m_includeZero(true)
    , m_explicitlyDefinedSpacer(false)
    , m_spacer(FormatConstant::DefaultSpacer())
    , m_ratio {0}
    {
    m_proxys.reserve(4);
    m_proxys.resize(units.size());

    int i = 0;
    for(auto const& unit : units)
        {
        m_proxys[i] = unit;
        i++;
        }

    if (units.size() > 0)
        CalculateUnitRatios();
    else
        m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
CompositeValueSpec::CompositeValueSpec(CompositeValueSpecCR other)
    : m_includeZero(other.m_includeZero)
    , m_explicitlyDefinedSpacer(other.m_explicitlyDefinedSpacer)
    , m_spacer(other.m_spacer)
    , m_problem(other.m_problem)
    , m_proxys(other.m_proxys)
    {
    memcpy(m_ratio, other.m_ratio, sizeof(m_ratio));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//---------------------------------------------------------------------------------------
double CompositeValueSpec::CalculateUnitRatio(BEU::UnitCP upper, BEU::UnitCP lower)
    {
    if (nullptr == lower)
        {
        // Lower unit is not defined - which is OK regardless of whether the
        // upper is defined.
        return 0.0;
        }

    if (nullptr == upper)
        {
        // This should never occur, since units should always be defined in the
        // order of major, middle, minor, sub, which means the ratios
        // major/middle, middle/minor, and minor/sub should always have a
        // defined numerator.
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_InconsistentUnitSet);
        return 0.0;
        }

    if (upper->GetPhenomenon() != lower->GetPhenomenon())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_UncomparableUnits);
        return 0.0;
        }

    double ratio;
    auto code = upper->Convert(ratio, 1.0, lower);
    if (BEU::UnitsProblemCode::NoProblem != code)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidUnitCombination);
        return 0.0;
        }
    return ratio;
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
        if (m_ratio[indx] <= 1.0)
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
CompositeValue CompositeValueSpec::DecomposeValue(double dval, BEU::UnitCP uom) const
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
        if (!BEU::Unit::AreCompatible(uom, smallest))
            {
            cv.UpdateProblemCode(FormatProblemCode::CVS_UncomparableUnits);
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
// @bsimethod                                   Kyle.Abramowitz                 04/2018
//--------------------------------------------------------------------------------------
//static
bool CompositeValueSpec::CreateCompositeSpec(CompositeValueSpecR out, bvector<BEU::UnitCP> const& units)
    {
    if (units.size() > 4)
        {
        LOG.errorv("Cannot create a composite spec with more than 4 units");
        return false;
        }
    for (auto unit : units)
        {
        if (nullptr == unit)
            return false;
        }
    out = CompositeValueSpec(units);
    return true;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool CompositeValueSpec::IsIdentical(CompositeValueSpecCR other) const
    {
    auto compareProxies = [&]() -> bool
        {
        if (m_proxys.size() != other.m_proxys.size())
            return false;
        int i = 0;
        for (auto const& p : m_proxys)
            {
            if (!p.IsIdentical(other.m_proxys[i]))
                return false;
            i++;
            }
        return true;
        };
    return m_problem.GetProblemCode() == other.m_problem.GetProblemCode()
        && GetUnitCount() == other.GetUnitCount()
        && m_includeZero == other.m_includeZero
        && m_spacer.Equals(other.m_spacer)
        && compareProxies();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Json::Value CompositeValueSpec::ToJson() const
    {
    if (IsProblem()) // TODO log error;
        return Json::Value();
    Json::Value jCVS;
    bool valid = false;
    UnitProxyCP proxP;
    jCVS[json_units()] = Json::arrayValue;
    for (int i = 0; i < GetUnitCount(); i++)
        {
        proxP = GetProxy(i);
        if(!proxP->IsEmpty())
            {
            if (0 == i) // Major unit
                valid = true;
            jCVS[json_units()].append(proxP->ToJson());
            }
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
BentleyStatus CompositeValueSpec::FromJson(CompositeValueSpecR out, JsonValueCR jval, BEU::IUnitsContextCP context)
    {
    Utf8CP paramName;
    Utf8String str;
    if (jval.empty())
        return ERROR;
    bool includeZero = false;
    Utf8String spacer;
    Utf8String input;
    bvector<Units::UnitCP> units;
    bvector<Utf8String> labels;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        str = val.ToString();
        if (BeStringUtilities::StricmpAscii(paramName, "units") == 0)
            {
            for (Json::ValueIterator iter = val.begin(); iter != val.end(); iter++)
                {
                UnitProxy upp;
                upp.LoadJson(*iter, context);
                units.push_back(upp.GetUnit());
                labels.push_back(upp.GetLabel());
                }
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_includeZero()) == 0)
            includeZero = val.asBool();
        else if (BeStringUtilities::StricmpAscii(paramName, json_spacer()) == 0)
            spacer = val.asString();
        }
    out = CompositeValueSpec(units);
    out.SetIncludeZero(includeZero);
    out.SetSpacer(spacer.c_str());
    switch (labels.size())
        {
        case 4:
            out.SetSubLabel(labels[3]);
        case 3:
            out.SetMinorLabel(labels[2]);
        case 2:
            out.SetMiddleLabel(labels[1]);
        case 1:
            out.SetMajorLabel(labels[0]);
        }
    return SUCCESS;
    }

//===================================================
// UnitProxy Methods
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value UnitProxy::ToJson() const
    {
    Json::Value jUP;

    if(nullptr != m_unit)
        jUP[json_name()] = m_unit->GetName().c_str();
    if (!m_unitLabel.empty())
        jUP[json_label()] = m_unitLabel.c_str();
    return jUP;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void UnitProxy::LoadJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    m_unitLabel.clear();
    m_unit = nullptr;
    if (jval.empty())
        return;

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_name()) == 0)
            {
            Utf8CP str = val.asCString();
            if (nullptr != str)
                m_unit = context->LookupUnit(str);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_label()) == 0)
            m_unitLabel = val.asString().c_str();
        }
    }

//===================================================
// QuantityFormatting Methods
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
// static
Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, double* persist, BEU::UnitCP outputUnit, FormatCR inputFormat, FormatProblemCode* problemCode)
    {
    *problemCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = Formatting::FormatParsingSet(input, inputFormat.GetCompositeMajorUnit()).GetQuantity(problemCode, &inputFormat);
    if (*problemCode == Formatting::FormatProblemCode::NoProblems)
        {
        if (nullptr != persist)
            {
            BEU::Quantity persQty = qty.ConvertTo(outputUnit);
            *persist = persQty.GetMagnitude();
            }
        }
    else if (nullptr != persist)
        *persist = 0.0;

    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
// static
BEU::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, FormatCR inputFormat, FormatProblemCode* problemCode)
    {
    return Formatting::FormatParsingSet(input, inputFormat.GetCompositeMajorUnit()).GetQuantity(problemCode, &inputFormat);
    }

END_BENTLEY_FORMATTING_NAMESPACE
