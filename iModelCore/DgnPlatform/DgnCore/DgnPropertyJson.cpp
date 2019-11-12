/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJsonCpp/BeJsonUtilities.h>


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::FromJson(JsonValueCR inValue)
    {
    UnitBase base = (UnitBase) inValue[json_base()].asUInt((uint32_t)UnitBase::Meter);
    UnitSystem system  = (UnitSystem) inValue[json_sys()].asUInt((uint32_t)UnitSystem::Metric);

    double numerator   = inValue[json_num()].asDouble(1.0);
    double denominator = inValue[json_den()].asDouble(1.0);

    Init(base, system, numerator, denominator, BeJsonUtilities::CStringFromStringValue(inValue[json_label()], "m"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::ToJson(JsonValueR outValue) const
    {
    outValue.SetOrRemoveUInt(json_base(), (uint32_t) GetBase(), (uint32_t)UnitBase::Meter);
    outValue.SetOrRemoveUInt(json_sys(), (uint32_t) GetSystem(), (uint32_t)UnitSystem::Metric);
    outValue[json_label()] = Utf8String(GetLabel());
    outValue.SetOrRemoveDouble(json_num(), GetNumerator(), 1.0);
    outValue.SetOrRemoveDouble(json_den(), GetDenominator(), 1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void GeometricModel::Formatter::FormatterFlags::FromJson(JsonValueCR inValue)
    {
    m_linearUnitMode     = inValue[json_linMode()].asUInt();
    m_linearPrecType     = inValue[json_linType()].asUInt();
    m_linearPrecision    = inValue[json_linPrec()].asUInt();
    m_angularMode        = inValue[json_angMode()].asUInt();
    m_angularPrecision   = inValue[json_angPrec()].asUInt();
    m_directionMode      = inValue[json_dirMode()].asUInt();
    m_directionClockwise = inValue[json_clockwise()].asBool();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
Json::Value GeometricModel::Formatter::FormatterFlags::ToJson() const
    {
    Json::Value outValue;
    outValue.SetOrRemoveUInt(json_linMode(), m_linearUnitMode, 0);
    outValue.SetOrRemoveUInt(json_linType(), m_linearPrecType, 0);
    outValue.SetOrRemoveUInt(json_linPrec(), m_linearPrecision, 0);
    outValue.SetOrRemoveUInt(json_angMode(), m_angularMode, 0);
    outValue.SetOrRemoveUInt(json_angPrec(), m_angularPrecision, 0);
    outValue.SetOrRemoveUInt(json_dirMode(), m_directionMode, 0);
    outValue.SetOrRemoveBool(json_clockwise(), m_directionClockwise, false);
    return outValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void GeometricModel::Formatter::FromJson(JsonValueCR inValue)
    {
    m_formatterFlags.FromJson(inValue[json_fmtFlags()]);

    m_masterUnit.FromJson(inValue[json_mastUnit()]);
    m_subUnit.FromJson(inValue[json_subUnit()]);

    m_roundoffUnit      = inValue[json_rndUnit()].asDouble();
    m_roundoffRatio     = inValue[json_rndRatio()].asDouble();
    m_formatterBaseDir  = inValue[json_fmtDir()].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
Json::Value GeometricModel::Formatter::ToJson() const
    {
    Json::Value outValue;
    Json::Value fmtJson = m_formatterFlags.ToJson();
    if (!fmtJson.isNull())
        outValue[json_fmtFlags()] = fmtJson;
     
    m_masterUnit.ToJson(outValue[json_mastUnit()]);
    m_subUnit.ToJson(outValue[json_subUnit()]);

    outValue.SetOrRemoveDouble(json_rndUnit(), m_roundoffUnit, 0.0);
    outValue.SetOrRemoveDouble(json_rndRatio(), m_roundoffRatio, 0.0);
    outValue.SetOrRemoveDouble(json_fmtDir(), m_formatterBaseDir, 0.0);
    return outValue;
    }
