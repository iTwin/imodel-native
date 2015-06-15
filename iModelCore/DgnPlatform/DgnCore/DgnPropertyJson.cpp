/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnPropertyJson.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include "JsonUtils.h"

static Utf8CP DGNPROPERTYJSON_LinearUnitMode        = "linMode";
static Utf8CP DGNPROPERTYJSON_LinearPrecType        = "linType";
static Utf8CP DGNPROPERTYJSON_LinearPrecision       = "linPrec";
static Utf8CP DGNPROPERTYJSON_AngularMode           = "angMode";
static Utf8CP DGNPROPERTYJSON_AngularPrecision      = "angPrec";
static Utf8CP DGNPROPERTYJSON_DirectionMode         = "dirMode";
static Utf8CP DGNPROPERTYJSON_DirectionClockwise    = "clockwise";
static Utf8CP DGNPROPERTYJSON_FormatterFlags        = "fmtFlags";
static Utf8CP DGNPROPERTYJSON_MasterUnit            = "mastUnit";
static Utf8CP DGNPROPERTYJSON_SubUnit               = "subUnit";
static Utf8CP DGNPROPERTYJSON_RoundoffUnit          = "rndUnit";
static Utf8CP DGNPROPERTYJSON_RoundoffRatio         = "rndRatio";
static Utf8CP DGNPROPERTYJSON_FormatterBaseDir      = "fmtDir";
static Utf8CP DGNPROPERTYJSON_Base                  = "base";
static Utf8CP DGNPROPERTYJSON_System                = "sys";
static Utf8CP DGNPROPERTYJSON_Numerator             = "num";
static Utf8CP DGNPROPERTYJSON_Denominator           = "den";
static Utf8CP DGNPROPERTYJSON_Label                 = "label";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::FromJson(JsonValueCR inValue)
    {
    UnitBase base      = (UnitBase) inValue.get(DGNPROPERTYJSON_Base,(uint32_t)UnitBase::Meter).asUInt();
    UnitSystem system  = (UnitSystem) inValue.get(DGNPROPERTYJSON_System,(uint32_t)UnitSystem::Metric).asUInt();
    double numerator   = JsonUtils::GetDouble(inValue[DGNPROPERTYJSON_Numerator], 1.0);
    double denominator = JsonUtils::GetDouble(inValue[DGNPROPERTYJSON_Denominator], 1.0);

    WString label(BeJsonUtilities::CStringFromStringValue(inValue[DGNPROPERTYJSON_Label], "m"), true);

    Init(base,  system, numerator, denominator, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::ToJson(JsonValueR outValue) const
    {
    outValue[DGNPROPERTYJSON_Base] = (uint32_t) GetBase();
    outValue[DGNPROPERTYJSON_System] = (uint32_t) GetSystem();
    outValue[DGNPROPERTYJSON_Label] = Utf8String(GetLabel());
    outValue[DGNPROPERTYJSON_Numerator] = GetNumerator();
    outValue[DGNPROPERTYJSON_Denominator] = GetDenominator();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void DgnModel::Properties::FormatterFlags::FromJson(JsonValueCR inValue)
    {
    m_linearUnitMode     = inValue[DGNPROPERTYJSON_LinearUnitMode].asUInt();
    m_linearPrecType     = inValue[DGNPROPERTYJSON_LinearPrecType].asUInt();
    m_linearPrecision    = inValue[DGNPROPERTYJSON_LinearPrecision].asUInt();
    m_angularMode        = inValue[DGNPROPERTYJSON_AngularMode].asUInt();
    m_angularPrecision   = inValue[DGNPROPERTYJSON_AngularPrecision].asUInt();
    m_directionMode      = inValue[DGNPROPERTYJSON_DirectionMode].asUInt();
    m_directionClockwise = inValue[DGNPROPERTYJSON_DirectionClockwise].asBool();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void DgnModel::Properties::FormatterFlags::ToJson(JsonValueR outValue) const
    {
    outValue[DGNPROPERTYJSON_LinearUnitMode]     = (uint32_t) m_linearUnitMode;
    outValue[DGNPROPERTYJSON_LinearPrecType]     = (uint32_t) m_linearPrecType;
    outValue[DGNPROPERTYJSON_LinearPrecision]    = (uint32_t) m_linearPrecision;
    outValue[DGNPROPERTYJSON_AngularMode]        = (uint32_t) m_angularMode;
    outValue[DGNPROPERTYJSON_AngularPrecision]   = (uint32_t) m_angularPrecision;
    outValue[DGNPROPERTYJSON_DirectionMode]      = (uint32_t) m_directionMode;
    outValue[DGNPROPERTYJSON_DirectionClockwise] = m_directionClockwise;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void DgnModel::Properties::FromJson(JsonValueCR inValue)
    {
    m_formatterFlags.FromJson(inValue[DGNPROPERTYJSON_FormatterFlags]);

    m_masterUnit.FromJson(inValue[DGNPROPERTYJSON_MasterUnit]);
    m_subUnit.FromJson(inValue[DGNPROPERTYJSON_SubUnit]);

    m_roundoffUnit = inValue[DGNPROPERTYJSON_RoundoffUnit].asDouble();
    m_roundoffRatio    = inValue[DGNPROPERTYJSON_RoundoffRatio].asDouble();
    m_formatterBaseDir = inValue[DGNPROPERTYJSON_FormatterBaseDir].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void DgnModel::Properties::ToJson(JsonValueR outValue) const
    {
    m_formatterFlags.ToJson(outValue[DGNPROPERTYJSON_FormatterFlags]);

    m_masterUnit.ToJson(outValue[DGNPROPERTYJSON_MasterUnit]);
    m_subUnit.ToJson(outValue[DGNPROPERTYJSON_SubUnit]);

    outValue[DGNPROPERTYJSON_RoundoffUnit] = m_roundoffUnit;
    outValue[DGNPROPERTYJSON_RoundoffRatio]    = m_roundoffRatio;
    outValue[DGNPROPERTYJSON_FormatterBaseDir] = m_formatterBaseDir;
    }
