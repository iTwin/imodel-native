/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnPropertyJson.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include "JsonUtils.h"

static Utf8CP DGNPROPERTYJSON_LinearUnitMode        = "linearUnitMode";
static Utf8CP DGNPROPERTYJSON_LinearPrecType        = "linearPrecType";
static Utf8CP DGNPROPERTYJSON_LinearPrecision       = "linearPrecision";
static Utf8CP DGNPROPERTYJSON_AngularMode           = "angularMode";
static Utf8CP DGNPROPERTYJSON_AngularPrecision      = "angularPrecision";
static Utf8CP DGNPROPERTYJSON_DirectionMode         = "directionMode";
static Utf8CP DGNPROPERTYJSON_DirectionClockwise    = "directionClockwise";
static Utf8CP DGNPROPERTYJSON_UnitLock              = "unitLock";
static Utf8CP DGNPROPERTYJSON_Locked                = "locked";
static Utf8CP DGNPROPERTYJSON_SettingFlags          = "settingFlags";
static Utf8CP DGNPROPERTYJSON_FormatterFlags        = "formatterFlags";
static Utf8CP DGNPROPERTYJSON_MasterUnit            = "masterUnit";
static Utf8CP DGNPROPERTYJSON_SubUnit               = "subUnit";
static Utf8CP DGNPROPERTYJSON_RoundoffUnit          = "roundoffUnit";
static Utf8CP DGNPROPERTYJSON_RoundoffRatio         = "roundoffRatio";
static Utf8CP DGNPROPERTYJSON_FormatterBaseDir      = "formatterBaseDir";
static Utf8CP DGNPROPERTYJSON_Base                  = "base";
static Utf8CP DGNPROPERTYJSON_System                = "system";
static Utf8CP DGNPROPERTYJSON_Numerator             = "numerator";
static Utf8CP DGNPROPERTYJSON_Denominator           = "denominator";
static Utf8CP DGNPROPERTYJSON_Label                 = "label";
static Utf8CP DGNPROPERTYJSON_UseBgColor            = "useBgColor";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::FromJson (JsonValueCR inValue)
    {
    UnitBase base      = (UnitBase) inValue.get(DGNPROPERTYJSON_Base, (UInt32)UnitBase::Meter).asUInt();
    UnitSystem system  = (UnitSystem) inValue.get(DGNPROPERTYJSON_System, (UInt32)UnitSystem::Metric).asUInt();
    double numerator   = JsonUtils::GetDouble(inValue[DGNPROPERTYJSON_Numerator], 1.0);
    double denominator = JsonUtils::GetDouble(inValue[DGNPROPERTYJSON_Denominator], 1.0);

    WString label (BeJsonUtilities::CStringFromStringValue (inValue[DGNPROPERTYJSON_Label], "m"), true);

    Init (base,  system, numerator, denominator, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void UnitDefinition::ToJson (JsonValueR outValue) const
    {
    outValue[DGNPROPERTYJSON_Base] = (UInt32) GetBase();
    outValue[DGNPROPERTYJSON_System] = (UInt32) GetSystem();
    outValue[DGNPROPERTYJSON_Label] = Utf8String (GetLabel());
    outValue[DGNPROPERTYJSON_Numerator] = GetNumerator();
    outValue[DGNPROPERTYJSON_Denominator] = GetDenominator();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
BentleyStatus ModelInfo::FromPropertiesJson (JsonValueCR inValue)
    {
    m_flags.m_locked              = inValue[DGNPROPERTYJSON_Locked].asBool();
    m_flags.m_useBackgroundColor  = inValue[DGNPROPERTYJSON_UseBgColor].asBool();

    return  SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ModelInfo::ToPropertiesJson (JsonValueR outValue) const
    {
    if (m_flags.m_locked)
        outValue[DGNPROPERTYJSON_Locked]    = m_flags.m_locked;

    if (m_flags.m_useBackgroundColor)
        outValue[DGNPROPERTYJSON_UseBgColor]= m_flags.m_useBackgroundColor;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void modelSettingFlagsFromJson (ModelSettingFlags& flags, JsonValueCR inValue)
    {
    flags.m_unitLock      = inValue[DGNPROPERTYJSON_UnitLock].asBool();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void modelSettingFlagsToJson (JsonValueR outValue, ModelSettingFlags const& flags)
    {
    outValue[DGNPROPERTYJSON_UnitLock]      = flags.m_unitLock;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void formatterFlagsFromJson (FormatterFlags& flags, JsonValueCR inValue)
    {
    flags.m_linearUnitMode     = inValue[DGNPROPERTYJSON_LinearUnitMode].asUInt();
    flags.m_linearPrecType     = inValue[DGNPROPERTYJSON_LinearPrecType].asUInt();
    flags.m_linearPrecision    = inValue[DGNPROPERTYJSON_LinearPrecision].asUInt();
    flags.m_angularMode        = inValue[DGNPROPERTYJSON_AngularMode].asUInt();
    flags.m_angularPrecision   = inValue[DGNPROPERTYJSON_AngularPrecision].asUInt();
    flags.m_directionMode      = inValue[DGNPROPERTYJSON_DirectionMode].asUInt();
    flags.m_directionClockwise = inValue[DGNPROPERTYJSON_DirectionClockwise].asBool();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void formatterFlagsToJson (JsonValueR outValue, FormatterFlags const& flags)
    {
    outValue[DGNPROPERTYJSON_LinearUnitMode]     = (UInt32) flags.m_linearUnitMode;
    outValue[DGNPROPERTYJSON_LinearPrecType]     = (UInt32) flags.m_linearPrecType;
    outValue[DGNPROPERTYJSON_LinearPrecision]    = (UInt32) flags.m_linearPrecision;
    outValue[DGNPROPERTYJSON_AngularMode]        = (UInt32) flags.m_angularMode;
    outValue[DGNPROPERTYJSON_AngularPrecision]   = (UInt32) flags.m_angularPrecision;
    outValue[DGNPROPERTYJSON_DirectionMode]      = (UInt32) flags.m_directionMode;
    outValue[DGNPROPERTYJSON_DirectionClockwise] = flags.m_directionClockwise;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
BentleyStatus ModelInfo::FromSettingsJson(JsonValueCR inValue)
    {
    modelSettingFlagsFromJson (m_settingFlags, inValue[DGNPROPERTYJSON_SettingFlags]);
    formatterFlagsFromJson (m_formatterFlags, inValue[DGNPROPERTYJSON_FormatterFlags]);

    m_masterUnit.FromJson(inValue[DGNPROPERTYJSON_MasterUnit]);
    m_subUnit.FromJson(inValue[DGNPROPERTYJSON_SubUnit]);

    m_roundoffUnit = inValue[DGNPROPERTYJSON_RoundoffUnit].asDouble();
    m_roundoffRatio    = inValue[DGNPROPERTYJSON_RoundoffRatio].asDouble();
    m_formatterBaseDir = inValue[DGNPROPERTYJSON_FormatterBaseDir].asDouble();
    return  SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
void ModelInfo::ToSettingsJson (JsonValueR outValue) const
    {
    modelSettingFlagsToJson (outValue[DGNPROPERTYJSON_SettingFlags], m_settingFlags);
    formatterFlagsToJson (outValue[DGNPROPERTYJSON_FormatterFlags], m_formatterFlags);

    m_masterUnit.ToJson (outValue[DGNPROPERTYJSON_MasterUnit]);
    m_subUnit.ToJson (outValue[DGNPROPERTYJSON_SubUnit]);

    outValue[DGNPROPERTYJSON_RoundoffUnit] = m_roundoffUnit;
    outValue[DGNPROPERTYJSON_RoundoffRatio]    = m_roundoffRatio;
    outValue[DGNPROPERTYJSON_FormatterBaseDir] = m_formatterBaseDir;
    }


