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

    bool constant = false;
    for (const auto& p : m_proxys)
        {
        if (p.GetUnit()->IsConstant())
            {
            LOG.errorv("Cannot add '%s' as a composite unit because it is a constant", p.GetUnit()->GetName().c_str());
            constant = true;
            }
        }

    if (unitCount > 0 && !constant)
        CalculateUnitRatios();
    else if (constant)
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_ConstantAsUnit);
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
    {
    m_proxys.reserve(4);
    m_proxys.resize(units.size());

    bool constant = false;
    int i = 0;
    for(auto const& unit : units)
        {
        if (unit->IsConstant())
            constant = true;

        m_proxys[i] = unit;
        i++;
        }

    if (units.size() > 0 && !constant)
        CalculateUnitRatios();
    else if (constant)
        m_problem.UpdateProblemCode(FormatProblemCode::CVS_ConstantAsUnit);
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

    return proxy->HasLabel() ? proxy->GetLabel().c_str() : proxy->GetUnit()->GetDisplayLabel().c_str();
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

//--------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                    06/2018
//--------------------------------------------------------------------------------------
Utf8String CompositeValueSpec::GetMajorLabel() const 
    {
    return GetUnitLabel(indxMajor);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                    06/2018
//--------------------------------------------------------------------------------------
Utf8String CompositeValueSpec::GetMiddleLabel() const 
    {   
    return GetUnitLabel(indxMiddle);
    }
//--------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                    06/2018
//--------------------------------------------------------------------------------------
Utf8String CompositeValueSpec::GetMinorLabel() const 
    {
    return GetUnitLabel(indxMinor);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                    06/2018
//--------------------------------------------------------------------------------------
Utf8String CompositeValueSpec::GetSubLabel() const 
    {
    return GetUnitLabel(indxSub);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
CompositeValueSpec::CompositeValue CompositeValueSpec::DecomposeValue(double dval, BEU::UnitCP uom) const
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

    if (out.IsProblem())
        return false;

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
// @bsimethod                                   Kyle.Abramowitz                 06/2018
//--------------------------------------------------------------------------------------
const bvector<BEU::UnitCP> CompositeValueSpec::GetUnits() const
    {
    bvector<BEU::UnitCP> units;
    units.reserve(GetUnitCount());
    if (HasMajorUnit())
        units.push_back(GetMajorUnit());
    if (HasMiddleUnit())
        units.push_back(GetMiddleUnit());
    if (HasMinorUnit())
        units.push_back(GetMinorUnit());
    if (HasSubUnit())
        units.push_back(GetSubUnit());
    return units;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool CompositeValueSpec::ToJson(Json::Value& out, bool verbose, bool excludeUnits) const
    {
    if (IsProblem())
        return false;

    bool valid = true;
    if (!excludeUnits)
        {
        valid = false;
        out[json_units()] = Json::arrayValue;
        UnitProxyCP proxP = nullptr;
        for (int i = 0; i < GetUnitCount(); i++)
            {
            proxP = GetProxy(i);
            if (!proxP->IsEmpty())
                {
                if (0 == i) // Major unit
                    valid = true;
                Json::Value jval;
                if (!proxP->ToJson(out[json_units()], verbose))
                    return false;
                }
            }
        }

    if (valid)
        {
        out[json_includeZero()] = IsIncludeZero();
        // since FormatConstant::DefaultSpacer() is a single blank character, the only way to not have a spacer is to set 
        // the spacer to "", so we must preserved that in the json. This means we have to save m_space even if it is empty.
        out[json_spacer()] = m_spacer.c_str();
        }

    return valid;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool CompositeValueSpec::FromJson(CompositeValueSpecR out, JsonValueCR jval, BEU::IUnitsContextCP context)
    {
    if (jval.empty())
        return false;

    bvector<Units::UnitCP> units;
    bvector<Utf8String> labels;
    if (jval.isMember(json_units()))
        {
        JsonValueCR unitsJson = jval[json_units()];
        for (Json::ValueIterator iter = unitsJson.begin(); iter != unitsJson.end(); iter++)
            {
            UnitProxy upp;
            if (!upp.FromJson(*iter, context))
                return false;
            units.push_back(upp.GetUnit());
            labels.push_back(upp.GetLabel());
            }
        }

    return FromJson(out, jval, units, labels);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool CompositeValueSpec::FromJson(CompositeValueSpecR out, JsonValueCR jval, bvector<Units::UnitCP> const& units, bvector<Utf8String> const& unitLabels)
    {
    if (jval.empty())
        return false;

    out = CompositeValueSpec(units);

    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        Utf8CP paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_includeZero()) == 0)
            out.SetIncludeZero(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_spacer()) == 0)
            out.SetSpacer(val.asCString());
        }
    // Fallthrough intentional
    switch (unitLabels.size())
        {
        case 4:
            out.SetSubLabel(unitLabels[3]);
        case 3:
            out.SetMinorLabel(unitLabels[2]);
        case 2:
            out.SetMiddleLabel(unitLabels[1]);
        case 1:
            out.SetMajorLabel(unitLabels[0]);
        }

    return true;
    }

//===================================================
// UnitProxy Methods
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool CompositeValueSpec::UnitProxy::ToJson(Json::Value& jUP, bool verbose) const
    {
    if (nullptr == m_unit)
        return false;
    auto& val = jUP.append(Json::Value());
    val[json_name()] = m_unit->GetName().c_str();
    if (!m_unitLabel.empty())
        val[json_label()] = m_unitLabel.c_str();
    else if (verbose)
        val[json_label()] = m_unit->GetDisplayLabel().c_str();
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool CompositeValueSpec::UnitProxy::FromJson(Json::Value const& jval, BEU::IUnitsContextCP context)
    {
    m_unitLabel.clear();
    m_unit = nullptr;
    if (jval.empty())
        return false;

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_name()) == 0)
            {
            Utf8String str = val.asString();
            if (!str.empty() && !str.Contains(":"))
                {
                str.ReplaceAll(".", ":"); // To handle the json case where . is used instead of :
                m_unit = context->LookupUnit(str.c_str(), true);
                }
            if (nullptr == m_unit)
                return false;
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_label()) == 0)
            m_unitLabel = val.asString().c_str();
        }
    return true;
    }

//===================================================
// QuantityFormatting Methods
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
// static
Units::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, double* persist, BEU::UnitCP outputUnit, FormatCR inputFormat, FormatProblemCode* problemCode, QuantityFormatting::UnitResolver* resolver)
    {
    *problemCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = Formatting::FormatParsingSet(input, inputFormat.GetCompositeMajorUnit(), &inputFormat, resolver).GetQuantity(problemCode, &inputFormat);
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
BEU::Quantity QuantityFormatting::CreateQuantity(Utf8CP input, FormatCR inputFormat, FormatProblemCode* problemCode, QuantityFormatting::UnitResolver* resolver)
    {
    return Formatting::FormatParsingSet(input, inputFormat.GetCompositeMajorUnit(), &inputFormat).GetQuantity(problemCode, &inputFormat);
    }

END_BENTLEY_FORMATTING_NAMESPACE
