/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreValueFormat.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

#define  ROUNDOFF           (0.5 - DBL_EPSILON)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::Init()
    {
    m_angleMode         = AngleMode::Degrees;
    m_precision         = AnglePrecision::Use4Places;
    m_leadingZero       = true;
    m_trailingZeros     = false;
    m_allowNegative     = true;
    m_allowUnclamped    = false;
    m_decimalSeparator  = L'.';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AngleFormatter::AngleFormatter(AngleFormatterCR source)
    {
    m_angleMode         = source.m_angleMode;
    m_precision         = source.m_precision;
    m_leadingZero       = source.m_leadingZero;
    m_trailingZeros     = source.m_trailingZeros;
    m_allowNegative     = source.m_allowNegative;
    m_allowUnclamped    = source.m_allowUnclamped;
    m_decimalSeparator  = source.m_decimalSeparator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::InitModelSettings(GeometricModelCR model)
    {
    GeometricModel::DisplayInfo const& displayInfo = model.GetDisplayInfo();

    SetAngleMode(displayInfo.GetAngularMode());
    SetAnglePrecision(displayInfo.GetAngularPrecision());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t AngleFormatter::GetLegacyFormat() const
    {
    AngleFormatVals angleFormat;
    StatusInt       status = SUCCESS;

    switch (m_angleMode)
        {
        default:
            {
            //  Ray noticed this occurs a lot in petrobas.  John G encountered it in SiteLayout.  Josh confirmed that it is
            //  bad data and treating it as AngleFormatVals::Degrees is appropriate.
            //  BeAssert(0);
            status = ERROR;
            // FALLTHROUGH
            }
        case AngleMode::Degrees:
            angleFormat = AngleFormatVals::Degrees;
            break;
        case AngleMode::DegMinSec:
            angleFormat = AngleFormatVals::DegMinSec;
            break;
        case AngleMode::Centesimal:
            angleFormat = AngleFormatVals::Centesimal;
            break;
        case AngleMode::Radians:
            angleFormat = AngleFormatVals::Radians;
            break;
        case AngleMode::DegMin:
            angleFormat =  AngleFormatVals::DegMin;
            break;
        }

    return static_cast<uint16_t>(angleFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AngleFormatter::SetAngleModeFromLegacy(AngleFormatVals legacyValue)
    {
    StatusInt   status = SUCCESS;

    switch (legacyValue)
        {
        default:
            {
//          BeDataAssert(0);
            status = ERROR;
            // FALLTHROUGH
            }
        case AngleFormatVals::Degrees:
            m_angleMode = AngleMode::Degrees;
            break;
        case AngleFormatVals::DegMinSec:
            m_angleMode =  AngleMode::DegMinSec;
            break;
        case AngleFormatVals::Centesimal:
            m_angleMode =  AngleMode::Centesimal;
            break;
        case AngleFormatVals::Radians:
            m_angleMode =  AngleMode::Radians;
            break;
        case AngleFormatVals::DegMin:
            m_angleMode =  AngleMode::DegMin;
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AngleFormatter::SetAnglePrecisionFromLegacy(int legacyValue)
    {
    if (0 <= legacyValue && 8 >= legacyValue)
        {
        m_precision = (AnglePrecision) legacyValue;
        return SUCCESS;
        }

    BeDataAssert(0);
    m_precision = AnglePrecision::Use4Places;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleFormatter::UseTwoDigitMinWidth() const
    {
    if (m_angleMode == AngleMode::DegMinSec ||
        m_angleMode == AngleMode::DegMin)
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::ConcatUnitLabel(Utf8StringR inString, AngleFormatter::AngleUnit unit) const
    {
    switch (unit)
        {
        case ANGLE_UNIT_Degrees:
            {
            WChar degStr[] = { 0x00b0 /*degree*/, 0 };
            inString.append(Utf8String(degStr));
            break;
            }
        case ANGLE_UNIT_Minutes:
            {
            inString.append("'");
            break;
            }
        case ANGLE_UNIT_Seconds:
            {
            inString.append("\"");
            break;
            }
        case ANGLE_UNIT_Grads:
            {
            inString.append("g");
            break;
            }
        case ANGLE_UNIT_Radians:
            {
            inString.append("r");
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::PrependLeadingZeroIfNeeded(Utf8StringR inString, double value) const
    {
    if ( ! UseTwoDigitMinWidth() || value >= 10.0 || ! m_leadingZero)
        return;

    inString.append("0");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::ConcatIntegerString(Utf8StringR inString, int value, AngleFormatter::AngleUnit unit) const
    {
    PrependLeadingZeroIfNeeded(inString, value);

    Utf8PrintfString tmpString("%d", value);
    inString.append(tmpString);

    ConcatUnitLabel(inString, unit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatter::ConcatPrecisionString(Utf8StringR inString, double value, AngleFormatter::AngleUnit unit, double delta) const
    {
    if (AnglePrecision::Whole == m_precision)
        {
        ConcatIntegerString(inString, (int)value, unit);
        return;
        }

    /*-----------------------------------------------------------
    Value will be rounded by sprintf %f so we need to remove the
    rounding constant added above. It could go negative - extremely
    close to zero so check and set to zero if negative.
    -----------------------------------------------------------*/
    value -= delta;
    if (value < 0.0)
        value = 0.0;

    PrependLeadingZeroIfNeeded(inString, value);

    Utf8PrintfString    fmtString("%%.%dlf", m_precision);
    Utf8PrintfString    tmpString(fmtString.c_str(), value);

    if (!m_trailingZeros)
        DoubleFormatterBase::StripTrailingZeros(tmpString);

    if (! m_leadingZero && '0' == tmpString[(size_t)0])
        {
        if (1 < tmpString.size() && ! UseTwoDigitMinWidth())
            tmpString.erase(0, 1);
        }

    DoubleFormatterBase::ReplaceDecimalSeparator(tmpString, m_decimalSeparator);

    inString.append(tmpString);
    ConcatUnitLabel(inString, unit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AngleFormatter::ToString(double angle) const
    {
    double          rmin, seconds, delta;
    int             degrees, minutes;
    Utf8String      angleString;

    if (m_allowNegative && 0.0 > angle)
        {
        angle = fabs(angle);

        angleString.assign("-");
        }

    if (fabs(angle) > 1.0e+10)
        {
        BeAssert(0);   // if the angle is too big, subtracting 360 doesn't cause any change
        angle = 0.0;
        }

    if (!m_allowUnclamped)
        {
        while (angle < 0.0)   angle += 360.0;
        while (angle > 360.0) angle -= 360.0;
        }

    delta = 0.5 + DBL_EPSILON;
    double  factor = 1.0;
    for (int i = 0; i < static_cast<int>(m_precision); i++)
        factor *= 10.0;
    delta /= factor; // avoid successive divides

    switch (m_angleMode)
        {
        case AngleMode::DegMin:
            {
            degrees = (int)angle;
            rmin    = delta + (angle - (double) degrees)*60.0;

            if (rmin >= 60)
                {
                rmin -= 60;
                degrees ++;
                }

            ConcatIntegerString(angleString, degrees, ANGLE_UNIT_Degrees);
            ConcatPrecisionString(angleString, rmin, ANGLE_UNIT_Minutes, delta);

            break;
            }
        case AngleMode::DegMinSec:
            {
            degrees = (int)angle;
            rmin    = (angle - (double) degrees)*60.0;
            minutes = (int)rmin;
            seconds = delta + (rmin - (double) minutes)*60.0;

            if (seconds >= 60.0)
                {
                seconds -= 60.0;
                minutes++;
                if (minutes >= 60)
                    {
                    minutes -= 60;
                    degrees ++;
                    }
                }

            ConcatIntegerString(angleString, degrees, ANGLE_UNIT_Degrees);
            ConcatIntegerString(angleString, minutes, ANGLE_UNIT_Minutes);

            ConcatPrecisionString(angleString, seconds, ANGLE_UNIT_Seconds, delta);

            break;
            }
        default:    /* decimal degrees, grads, radians  */
            {
            AngleUnit   angleUnit;

            switch (m_angleMode)
                {
                case AngleMode::Centesimal:   // (Gradians)
                    angle *= 100.0 / 90.0;
                    angleUnit = ANGLE_UNIT_Grads;
                    break;

                case AngleMode::Radians:
                    angle *= msGeomConst_radiansPerDegree;
                    angleUnit = ANGLE_UNIT_Radians;
                    break;

                default:
                    angleUnit = ANGLE_UNIT_Degrees;
                }

            ConcatPrecisionString(angleString, angle + delta, angleUnit, delta);

            break;
            }
        }

    return angleString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectionFormatter::Init()
    {
    m_angleFormatter = AngleFormatter::Create();
    m_angleFormatter->SetAllowNegative(false);

    m_mode              = DirectionMode::Azimuth;
    m_addTrueNorth      = false;
    m_trueNorth         = 0.0;
    m_baseDirection     = 0.0;
    m_clockwise         = false;
    m_bearingSpaces     = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DirectionFormatter::DirectionFormatter(DirectionFormatterCR source)
    {
    if (source.m_angleFormatter.IsValid())
        m_angleFormatter = source.m_angleFormatter->Clone();

    m_mode              = source.m_mode;
    m_addTrueNorth      = source.m_addTrueNorth;
    m_trueNorth         = source.m_trueNorth;
    m_baseDirection     = source.m_baseDirection;
    m_clockwise         = source.m_clockwise;
    m_bearingSpaces     = source.m_bearingSpaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectionFormatter::InitModelSettings(GeometricModelCR model)
    {
    m_angleFormatter->InitModelSettings(model);

    GeometricModel::DisplayInfo const& displayInfo = model.GetDisplayInfo();

    SetDirectionMode(displayInfo.GetDirectionMode());
    SetClockwise(displayInfo.GetDirectionClockwise());
    SetBaseDirection(displayInfo.GetDirectionBaseDir());

    DgnGCS* dgnGCS = model.GetDgnDb().Units().GetDgnGCS();
    double azimuth = (dgnGCS != nullptr) ? dgnGCS->GetAzimuth() : 0.0;
    SetTrueNorthValue(azimuth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DirectionFormatter::SetDirectionModeFromLegacy(int legacyValue)
    {
    StatusInt   status = SUCCESS;

    switch (legacyValue)
        {
        default:
            {
            BeAssert(0);
            status = ERROR;

            // FALLTHROUGH
            }
        case ANGLE_MODE_Standard:
            {
            m_mode          = DirectionMode::Azimuth;
            m_baseDirection = 0.0;
            m_clockwise     = false;
            break;
            }
        case ANGLE_MODE_Azimuth:
            {
            m_mode          = DirectionMode::Azimuth;
            m_baseDirection = 90.0;
            m_clockwise     = true;
            break;
            }
        case ANGLE_MODE_Bearing:
            {
            m_mode = DirectionMode::Bearing;
            break;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
int DirectionFormatter::GetLegacyAngleMode() const
    {
    switch (m_mode)
        {
        case DirectionMode::Bearing:
            {
            return static_cast<int>(ANGLE_MODE_Bearing);
            }
        case DirectionMode::Azimuth:
            {
            if (m_clockwise && 90.0 == m_baseDirection)
                return static_cast<int>(ANGLE_MODE_Azimuth);

            if ( ! m_clockwise && 0.0 == m_baseDirection)
                return static_cast<int>(ANGLE_MODE_Standard);

            break;
            }
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/07
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DirectionFormatter::ToString(double value) const
    {
    Utf8String  directionString;

    while (value < 0.0)   value += 360.0;
    while (value > 360.0) value -= 360.0;

    if (m_addTrueNorth)
        value -= m_trueNorth;

    switch (m_mode)
        {
        case DirectionMode::Azimuth:    /* azimuth angle readout */
            {
            if (m_clockwise)
                value = m_baseDirection - value;
            else
                value = value - m_baseDirection;

            if (m_angleFormatter->GetAllowNegative())
                {
                while (value <= - 180.0) value += 360.0;
                while (value >    180.0) value -= 360.0;
                }

            directionString = m_angleFormatter->ToString(value);

            break;
            }

        case DirectionMode::Bearing:    /* bearing angle readout */
            {
            if (360.0 == value)
                value = 0;

            Utf8String prefixChar;
            Utf8String suffixChar;

            // Previously, we used the following code to determine the quadrant:
            //     int  quadrant = (int) (value/90.0);
            // This gives the wrong answer for 90==value and 180==value.
            //
            // quadrant = (int) (90/90.0)  = 1; results in N0.0W;  desired N0.0E
            // quadrant = (int) (180/90.0) = 2; results in S90.0W; desired N90.0W

            if (0.0 <= value && 90.0 >= value)
                {
                value = 90.0 - value;
                prefixChar = "N";
                suffixChar = "E";
                }
            else
            if (90.0 < value && 180.0 >= value)
                {
                value = value - 90.0;
                prefixChar = "N";
                suffixChar = "W";
                }
            else
            if (180.0 < value && 270.0 > value)
                {
                value = 270.0 - value;
                prefixChar = "S";
                suffixChar = "W";
                }
            else
                {
                value = value - 270.0;
                prefixChar = "S";
                suffixChar = "E";
                }

            directionString.append(prefixChar);

            if (m_bearingSpaces)
                directionString.append(" ");

            directionString.append(m_angleFormatter->ToString(value));

            if (m_bearingSpaces)
                directionString.append(" ");

            directionString.append(suffixChar);

            break;
            }
        }

    return directionString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
int DirectionFormatter::DirFormatToLegacyAngleMode(DirFormat const& dirFormat)
    {
    DirectionFormatter formatter;

    formatter.SetDirectionMode((DirectionMode) dirFormat.mode);
    formatter.SetClockwise(TO_BOOL (dirFormat.flags.clockwise));
    formatter.SetBaseDirection(dirFormat.baseDir);

    int tentsubmode = formatter.GetLegacyAngleMode();

    if (tentsubmode < 0)
        tentsubmode = static_cast<int>(ANGLE_MODE_Standard);

    return tentsubmode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectionFormatter::DirFormatFromLegacyAngleMode(DirFormat& dirFormat, int tentsubmode)
    {
    DirectionFormatter  formatter;

    formatter.SetDirectionModeFromLegacy(tentsubmode);

    memset(&dirFormat, 0, sizeof(DirFormat));

    dirFormat.mode             = static_cast<uint16_t>(formatter.GetDirectionMode());
    dirFormat.baseDir          = formatter.GetBaseDirection();
    dirFormat.flags.clockwise  = formatter.GetClockwise();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/04
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DistanceFormatter::ToDwgUnitFormat(DwgUnitFormat& dwgUnitFormatOut, DgnUnitFormat dgnUnitFormat, PrecisionFormat dgnPrecision, UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit)
    {
    int             lunits = -1;
    PrecisionType   precisionType = DoubleFormatter::GetTypeFromPrecision(dgnPrecision);

    /* map DGN unit format values to DWG */
    if (PrecisionType::Scientific == precisionType)
        {
        /* set scientific format */
        lunits = static_cast<int>(DwgUnitFormat::Scientific);
        }
    else if (DgnUnitFormat::MU != dgnUnitFormat &&
             StandardUnit::EnglishFeet               == masterUnit.IsStandardUnit() &&
             StandardUnit::EnglishInches             == subUnit.IsStandardUnit())
        {
        /* map feet-inches to Architectural/Engineering */
        lunits = static_cast<int>(PrecisionType::Fractional == precisionType ? DwgUnitFormat::Architectural : DwgUnitFormat::Engineering);
        }
    else
        {
        /* map all other units to Fractional/Decimal */
        lunits = static_cast<int>(PrecisionType::Fractional == precisionType ? DwgUnitFormat::Fractional : DwgUnitFormat::Decimal);
        }

    if  (-1 != lunits)
        {
        dwgUnitFormatOut = (DwgUnitFormat)lunits;

        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceFormatter::DistanceFormatter(DistanceFormatterCR source)
    {
    T_Super::InitFrom(source);

    m_unitFlag                      = source.m_unitFlag;
    m_suppressZeroMasterUnits       = source.m_suppressZeroMasterUnits;
    m_suppressZeroSubUnits          = source.m_suppressZeroSubUnits;
    m_isDgnCoordReadOutCapable      = source.m_isDgnCoordReadOutCapable;
    m_unitFormat                    = source.m_unitFormat;
    m_masterUnit                    = source.m_masterUnit;
    m_subUnit                       = source.m_subUnit;
    m_scaleFactor                   = source.m_scaleFactor;
    m_useDWGFormattingLogic         = source.m_useDWGFormattingLogic;
    m_dwgUnitFormat                 = source.m_dwgUnitFormat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoeZbuchalski   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceFormatter::Init()
    {
    T_Super::Init();

    m_unitFlag                      = false;
    m_suppressZeroMasterUnits       = false;
    m_suppressZeroSubUnits          = false;
    m_isDgnCoordReadOutCapable      = true;
    m_unitFormat                    = DgnUnitFormat::MUSU;
    m_scaleFactor                   = 1.0;
    m_useDWGFormattingLogic         = false;
    m_dwgUnitFormat                 = DwgUnitFormat::Decimal;

    m_masterUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, NULL);
    m_subUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceFormatter::InitModelSettings(GeometricModelCR model)
    {
    GeometricModel::DisplayInfo const& displayInfo = model.GetDisplayInfo();

    SetUnitFormat(displayInfo.GetLinearUnitMode());
    SetPrecision(displayInfo.GetLinearPrecision());

    SetIsDgnCoordReadOutCapable(T_HOST.GetFormatterAdmin()._AllowDgnCoordinateReadout());

    m_masterUnit = displayInfo.GetMasterUnits();
    m_subUnit = displayInfo.GetSubUnits();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KyleDeeds                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DistanceFormatter::SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCP newSubUnit)
    {
    if (! newMasterUnit.IsValid())
        return ERROR;

    /* if Sub units are NULL make subunits are the same as master units */
    if (NULL == newSubUnit)
        {
        newSubUnit = &newMasterUnit;
        }
    else
        {
        if (! newSubUnit->IsValid())
            return ERROR;

        /* if subUnits passed in, validate that they are smaller than master */
        if ( ! newMasterUnit.AreComparable(*newSubUnit))
            return ERROR;

        int comparison = newMasterUnit.CompareByScale(*newSubUnit);

        if (0 < comparison)
            return ERROR;
        }

    m_masterUnit    = newMasterUnit;
    m_subUnit       = *newSubUnit;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceFormatterPtr    DistanceFormatter::Create(DgnViewportR viewport)
    {
    GeometricModelP targetModel = viewport.GetViewController().GetTargetModel();
    DistanceFormatterPtr formatter = DistanceFormatter::Create(*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;

    if (targetModel->GetDisplayInfo().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive(viewport);

    if (NULL == acs)
        return formatter;

    formatter->SetScaleFactor(1 / acs->GetScale());
#endif

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    05/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static void metersToMasterSub(int64_t& mu, int64_t& su, double& masterUnits, double& subUnits, bool& negFlag, double inUors, double minResolution, 
                              double subPerMast, double posPerSub)
    {
    if (inUors < 0)
        {
        negFlag = true;
        inUors  = -inUors;
        }
    else
        negFlag = false;

    inUors += minResolution * ROUNDOFF;

    double posUnits      = inUors;

    masterUnits = inUors / (subPerMast * posPerSub);
    mu          = (int64_t) masterUnits;
    posUnits   -= (mu * (subPerMast * posPerSub));

    subUnits   =  posUnits / posPerSub;
    su         = (int64_t) subUnits;
    posUnits  -= (su * posPerSub);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    05/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static double getMinimumResolutionForUnitInfo(DgnUnitFormat unitFormat, Byte precisionByte, PrecisionType precisionType, UnitDefinitionCR masterUnits, UnitDefinitionCR subUnits)
    {
    //  Now multiply by UORs per unit.  units may be NULL for MU SU PU
    double  minimumResolution = DoubleFormatterBase::MinimumResolutionForType(precisionByte, precisionType);

    switch (unitFormat)
        {
        case DgnUnitFormat::MUSU:      /* Master-Sub Units */
        case DgnUnitFormat::SU:        /* Sub Units */
            {
            minimumResolution *= subUnits.ToMeters();
            break;
            }

        case DgnUnitFormat::MU:        /* Master Units */
            {
            minimumResolution *= masterUnits.ToMeters();
            break;
            }

        }

    return minimumResolution;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
double  DistanceFormatter::GetMinimumResolution() const
    {
    return getMinimumResolutionForUnitInfo(m_unitFormat, m_precisionByte, m_precisionType, m_masterUnit, m_subUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoeZbuchalski   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
double DistanceFormatter::GetSubPerMaster() const
    {
    return m_subUnit.ConvertDistanceFrom(1.0, m_masterUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static PrecisionType precisionTypeForDWGUnitFormat(DwgUnitFormat format)
    {
    switch (format)
        {
        case DwgUnitFormat::Scientific:    return PrecisionType::Scientific;           break;
        case DwgUnitFormat::Architectural: return PrecisionType::Fractional;           break;
        case DwgUnitFormat::Fractional:    return PrecisionType::Fractional;           break;
        default:                           return PrecisionType::Decimal;              break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceFormatter::SetDWGUnitFormat(DwgUnitFormat newVal)
    {
    m_dwgUnitFormat = newVal;
    m_precisionType = precisionTypeForDWGUnitFormat(newVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoeZbuchalski   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DistanceFormatter::ToString(double meters) const
    {
    Utf8String      outString;
    Utf8Char        connector;
    double          masterUnits, subUnits;
    bool            isNegative;
    int64_t         iMasterUnits, iSubUnits;
    double          subPerMaster;
    double          posPerSub;
    bool            unitFlag = m_unitFlag;
    Utf8String      masterUnitsLabel, subUnitsLabel;

    masterUnitsLabel = m_masterUnit.GetLabel();
    subUnitsLabel = m_subUnit.GetLabel();

    subPerMaster = GetSubPerMaster();
    posPerSub = m_subUnit.ToMeters(); // Input value is always in meters

    if (m_scaleFactor != 0 && m_scaleFactor != 1)
         meters *= m_scaleFactor;

    metersToMasterSub(iMasterUnits, iSubUnits, masterUnits, subUnits, isNegative, meters, GetMinimumResolution(), subPerMaster, posPerSub);

    connector = ' ';
    if (!m_isDgnCoordReadOutCapable)
        {
        /* display '-" for Architectural/Engineering in DWG workmode */  
        if (StandardUnit::EnglishFeet == m_masterUnit.IsStandardUnit() && StandardUnit::EnglishInches == m_subUnit.IsStandardUnit())
            connector = '-';
        }

    switch (m_unitFormat)
        {
        case DgnUnitFormat::MUSU:          /* Master-Sub Units */
            {
            Utf8PrintfString masterUnitString("%lld", iMasterUnits);
            if (m_suppressZeroMasterUnits)
                {
                if (iMasterUnits == 0 && subUnits != 0)
                    {
                    masterUnitString.clear();
                    masterUnitsLabel.clear();
                    }
                }

            if (m_insertThousandsSeparator)
                InsertThousandsSeparator(masterUnitString, m_thousandsSeparator);

            Utf8String  subUnitString;
            double      limit = pow(10.0, -m_precisionByte);
            if (m_suppressZeroSubUnits && limit > subUnits)
                {
                subUnitsLabel.clear();
                }
            else
                {
                GetPrecisionString(subUnitString, subUnits, m_precisionType, m_precisionByte, m_leadingZero, m_trailingZeros);
                ReplaceDecimalSeparator(subUnitString, m_decimalSeparator);
                }

            if (unitFlag || !m_isDgnCoordReadOutCapable)
                outString.Sprintf("%s%s%c%s%s", masterUnitString.c_str(), masterUnitsLabel.c_str(), connector, subUnitString.c_str(), subUnitsLabel.c_str());
            else
                outString.Sprintf("%s:%s", masterUnitString.c_str(), subUnitString.c_str());

            break;
            }
        case DgnUnitFormat::MU:            /* Master Unit */
            {
            // Can't call ToStringBasic because UorsToMasterSubPositional already added in minRes*ROUNDOFF
            outString = T_Super::ToStringFromElevatedValue(masterUnits);
            if (unitFlag)
                outString.append(masterUnitsLabel);

            break;
            }
        case DgnUnitFormat::SU:
            {   
            double  totalSuUnits = subUnits + subPerMaster * iMasterUnits;

            // Can't call ToStringBasic because UorsToMasterSubPositional already added in minRes*ROUNDOFF
            outString = T_Super::ToStringFromElevatedValue(totalSuUnits);
            if (unitFlag)
                outString.append(subUnitsLabel);

            break;
            }
        }

    if (isNegative)
        outString.insert(0, "-");

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum LegacyAccuracyFlags
    {
    LEGACY_ACCURACYFLAGS_Default         = 0,
    LEGACY_ACCURACYFLAGS_Scientific      = 1,
    LEGACY_ACCURACYFLAGS_FractionalZero  = 2,
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LegacyDecFracAccuracy
    {
    union
        {
        Byte b;
        struct
            {
            unsigned dec:3;
            unsigned fract:5;
            } u;
        } adres2;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointFormatterPtr   PointFormatter::Create()           { return new PointFormatter(); }
PointFormatterPtr   PointFormatter::Create(DistanceFormatterCR distanceFormatter) { return new PointFormatter(distanceFormatter); }
/* ctor */          PointFormatter::PointFormatter()   { Init(); }
void                PointFormatter::SetAuxCoordSys(IAuxCoordSysCR acs)    { m_acs = acs.Clone(); }
void                PointFormatter::SetDistanceFormatter(DistanceFormatterCR f)    { m_distanceFormatter = f.Clone(); }
PointFormatterPtr   PointFormatter::Clone() const       { return new PointFormatter(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointFormatter::Init()
    {
    m_is3d = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        PointFormatter::PointFormatter(PointFormatterCR source)
    {
    if (source.m_acs.IsValid())
        m_acs = source.m_acs->Clone();

    if (source.m_distanceFormatter.IsValid())
        m_distanceFormatter = source.m_distanceFormatter->Clone();

    m_is3d = source.m_is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        PointFormatter::PointFormatter(DistanceFormatterCR distanceFormatter)
    {
    Init();
    m_distanceFormatter = distanceFormatter.Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceFormatterR  PointFormatter::GetDistanceFormatter()
    {
    if ( ! m_distanceFormatter.IsValid())
        m_distanceFormatter = DistanceFormatter::Create();

    return *m_distanceFormatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysR   PointFormatter::GetAuxCoordSys()
    {
    if ( ! m_acs.IsValid())
        m_acs = IACSManager::GetManager().CreateACS ();

    return *m_acs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointFormatter::InitModelSettings(GeometricModelCR model, bool addGlobalOrigin)
    {
    m_distanceFormatter = DistanceFormatter::Create(model);

    auto geomModel = model.ToGeometricModel();
    if (addGlobalOrigin && nullptr != geomModel)
        {
        // Create a un-rotated, un-scaled, rectangular ACS at the model's global origin.
        m_acs = IACSManager::GetManager().CreateACS ();
        m_acs->SetOrigin(geomModel->GetDgnDb().Units().GetGlobalOrigin());
        }

    m_is3d = nullptr != geomModel && geomModel->Is3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointFormatterPtr   PointFormatter::Create(GeometricModelCR model, bool addGlobalOrigin)
    {
    PointFormatterPtr   formatter = Create();

    formatter->InitModelSettings(model, addGlobalOrigin);

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointFormatterPtr   PointFormatter::Create(DgnViewportR viewport)
    {
    GeometricModelP targetModel = viewport.GetViewController().GetTargetModel();

#ifdef WIP_V10_MODEL_ACS
    bool            useViewACS  = targetModel->GetDisplayInfo().GetIsAcsLocked();
#else
    bool            useViewACS  = false;
#endif

    PointFormatterPtr   formatter = Create();
    formatter->InitModelSettings(*targetModel, !useViewACS);

#ifdef WIP_V10_MODEL_ACS
    if (useViewACS)
        formatter->SetAuxCoordSys(*IACSManager::GetManager().GetActive(viewport));
#endif

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void transformByACS(DPoint3d point, IAuxCoordSysCR acs)
    {
    double      scale = acs.GetScale();
    DPoint3d    origin;
    RotMatrix   rotation;

    acs.GetOrigin(origin);
    acs.GetRotation(rotation);

    point.Subtract(origin);
    rotation.Multiply(point);

    if (0 < scale)
        point.Scale(1/scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PointFormatter::ToString(DPoint3dCR point) const
    {
    // Ensure that these are initialized
    (const_cast <PointFormatterP> (this))->GetAuxCoordSys();
    (const_cast <PointFormatterP> (this))->GetDistanceFormatter();

    DPoint3d offpnt = point;
    transformByACS (offpnt, *m_acs);

    Utf8String outputString;

    outputString.append(m_distanceFormatter->ToString(offpnt.x));
    outputString.append(", ");
    outputString.append(m_distanceFormatter->ToString(offpnt.y));

    if (m_is3d)
        {
        outputString.append(", ");
        outputString.append(m_distanceFormatter->ToString(offpnt.z));
        }

    return outputString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeFormatterBase::InitFrom(AreaOrVolumeFormatterBase const& source)
    {
    T_Super::InitFrom(source);

    m_showUnitLabel                 = source.m_showUnitLabel;
    m_masterUnit                    = source.m_masterUnit;
    m_scaleFactor                   = source.m_scaleFactor;
    m_useDWGFormattingLogic         = source.m_useDWGFormattingLogic;
    m_labelDecoratorAsSuffix         = source.m_labelDecoratorAsSuffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeFormatterBase::Init()
    {
    T_Super::Init();

    m_showUnitLabel                 = false;
    m_scaleFactor                   = 1.0;
    m_useDWGFormattingLogic         = false;
    m_labelDecoratorAsSuffix        = true;

    m_masterUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeFormatterBase::InitModelSettings(GeometricModelCR model)
    {
    GeometricModel::DisplayInfo const& displayInfo = model.GetDisplayInfo();

    SetPrecision(displayInfo.GetLinearPrecision());

    m_masterUnit = displayInfo.GetMasterUnits();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeFormatterBase::SetDWGUnitFormat(DwgUnitFormat newVal)
    {
    m_dwgUnitFormat = newVal;
    m_precisionType = precisionTypeForDWGUnitFormat(newVal);
    m_insertThousandsSeparator = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */          AreaFormatter::AreaFormatter() { Init(); }
/*ctor*/            AreaFormatter::AreaFormatter(AreaFormatterCR source) { T_Super::InitFrom(source); }
void                AreaFormatter::Init()  { T_Super::Init(); }
AreaFormatterPtr    AreaFormatter::Create()            { return new AreaFormatter(); }
AreaFormatterPtr    AreaFormatter::Clone() const        { return new AreaFormatter(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AreaFormatterPtr    AreaFormatter::Create(GeometricModelCR model)
    {
    AreaFormatterPtr   formatter = Create();

    formatter->InitModelSettings(model);

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AreaFormatterPtr    AreaFormatter::Create(DgnViewportR viewport)
    {
    GeometricModelP targetModel = viewport.GetViewController().GetTargetModel();
    AreaFormatterPtr formatter = AreaFormatter::Create(*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetDisplayInfo().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive(viewport);

    if (NULL == acs)
        return formatter;

    formatter->SetScaleFactor(1 / acs->GetScale());
#endif

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    appendUnitLabel(Utf8StringR str, Utf8CP label, L10N::StringId decoratorId, bool asSuffix)
    {
    if ('\0' == *label)
        return;

    Utf8String decorator = DgnCoreL10N::GetString(decoratorId);

    if (iswupper(*label))
        decorator.ToUpper();

    if (asSuffix)
        {
        str.append(label);
        str.append(decorator);
        }
    else
        {
        str.append(decorator);
        str.append(label);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AreaFormatter::ToString(double squareMeters) const
    {
    squareMeters /= m_scaleFactor * m_scaleFactor;

    double mastMeters = m_masterUnit.ToMeters();
    double area = squareMeters / (mastMeters*mastMeters);

    Utf8String outString = T_Super::ToStringBasic(area);

    if (m_showUnitLabel)
        appendUnitLabel(outString, m_masterUnit.GetLabelCP(), m_labelDecoratorAsSuffix ? DgnCoreL10N::UNIT_LABEL_SUFFIX_Area() : DgnCoreL10N::UNIT_LABEL_PREFIX_Area(), m_labelDecoratorAsSuffix);

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */          VolumeFormatter::VolumeFormatter() { Init(); }
/*ctor*/            VolumeFormatter::VolumeFormatter(VolumeFormatterCR source) { T_Super::InitFrom(source); }
void                VolumeFormatter::Init()  { T_Super::Init(); }
VolumeFormatterPtr  VolumeFormatter::Create()            { return new VolumeFormatter(); }
VolumeFormatterPtr  VolumeFormatter::Clone() const        { return new VolumeFormatter(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeFormatterPtr VolumeFormatter::Create(GeometricModelCR model)
    {
    VolumeFormatterPtr   formatter = Create();

    formatter->InitModelSettings(model);

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeFormatterPtr VolumeFormatter::Create(DgnViewportR viewport)
    {
    GeometricModelP targetModel = viewport.GetViewController().GetTargetModel();
    VolumeFormatterPtr formatter = VolumeFormatter::Create(*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetDisplayInfo().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive(viewport);

    if (NULL == acs)
        return formatter;

    formatter->SetScaleFactor(1 / acs->GetScale());
#endif

    return formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String VolumeFormatter::ToString(double cubicMeters) const
    {
        cubicMeters /= m_scaleFactor * m_scaleFactor * m_scaleFactor;

    double mastMeters = m_masterUnit.ToMeters();
    double cube = cubicMeters / (mastMeters*mastMeters*mastMeters);

    Utf8String outString = T_Super::ToStringBasic(cube);

    if (m_showUnitLabel)
        appendUnitLabel(outString, m_masterUnit.GetLabelCP(), m_labelDecoratorAsSuffix ? DgnCoreL10N::UNIT_LABEL_SUFFIX_Volume() : DgnCoreL10N::UNIT_LABEL_PREFIX_Volume(), m_labelDecoratorAsSuffix);

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeFormatter::Reset()
    {
    ClearFormatParts();
    m_fractionalPrecision       = 3;
    m_fractionalTrailingZeros   = true;
    m_dateSeparator             = '/';
    m_timeSeparator             = ':';
    m_decimalSeparator          = '.';
    m_convertToLocalTime        = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeFormatter::DateTimeFormatter(DateTimeFormatterCR other)
    {
    m_partList              = other.m_partList;
    m_fractionalPrecision   = other.m_fractionalPrecision;
    m_fractionalTrailingZeros = other.m_fractionalTrailingZeros;
    m_dateSeparator         = other.m_dateSeparator;
    m_timeSeparator         = other.m_timeSeparator;
    m_decimalSeparator      = other.m_decimalSeparator;
    m_convertToLocalTime    = other.m_convertToLocalTime;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/

static DateTimeFormatPart s_dateTimeComposites[DATETIME_PART_COMPOSITE_END-DATETIME_PART_COMPOSITE_BASE][17] =
    {
        { DATETIME_PART_h, DATETIME_PART_TimeSeparator, DATETIME_PART_mm, DATETIME_PART_Space, DATETIME_PART_AMPM, DATETIME_PART_END },
        { DATETIME_PART_h, DATETIME_PART_TimeSeparator, DATETIME_PART_mm, DATETIME_PART_TimeSeparator, DATETIME_PART_ss, DATETIME_PART_Space, DATETIME_PART_AMPM, DATETIME_PART_END },
        { DATETIME_PART_M, DATETIME_PART_DateSeparator, DATETIME_PART_D, DATETIME_PART_DateSeparator, DATETIME_PART_YYYY, DATETIME_PART_END },
        { DATETIME_PART_MM, DATETIME_PART_DateSeparator, DATETIME_PART_DD, DATETIME_PART_DateSeparator, DATETIME_PART_YYYY, DATETIME_PART_END },
        { DATETIME_PART_DayOfWeek, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_D, DATETIME_PART_Space, DATETIME_PART_Month, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_YYYY, DATETIME_PART_END },
        { DATETIME_PART_DayOfWeek, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_Month, DATETIME_PART_Space, DATETIME_PART_D, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_YYYY, DATETIME_PART_END },
        { DATETIME_PART_DayOfWeek, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_Month, DATETIME_PART_Space, DATETIME_PART_D, DATETIME_PART_Comma, DATETIME_PART_Space, DATETIME_PART_YYYY, DATETIME_PART_Comma,
          DATETIME_PART_Space, DATETIME_PART_h, DATETIME_PART_TimeSeparator, DATETIME_PART_mm, DATETIME_PART_Space, DATETIME_PART_AMPM, DATETIME_PART_END },
        { DATETIME_PART_M, DATETIME_PART_DateSeparator, DATETIME_PART_D, DATETIME_PART_DateSeparator, DATETIME_PART_YYYY, DATETIME_PART_Space,
          DATETIME_PART_h, DATETIME_PART_TimeSeparator, DATETIME_PART_mm, DATETIME_PART_TimeSeparator, DATETIME_PART_ss, DATETIME_PART_Space, DATETIME_PART_AMPM, DATETIME_PART_END },
    };
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendFormatPart(bvector<DateTimeFormatPart>& partList, DateTimeFormatPart part)
    {
    if (DATETIME_PART_COMPOSITE_BASE <= part)
        {
        BeAssert(DATETIME_PART_COMPOSITE_END > part);
        DateTimeFormatPart* subParts = s_dateTimeComposites[part - DATETIME_PART_COMPOSITE_BASE];
        while (DATETIME_PART_END != *subParts)
            partList.push_back(*subParts++);
        }
    else
        partList.push_back(part);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeFormatter::AppendFormatPart(DateTimeFormatPart part)
    {
    appendFormatPart(m_partList, part);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getDayName(int d, bool useShort)
    {
    BeAssert(7 > d);
    DgnCoreL10N::StringId longNames[]={
        DgnCoreL10N::DATETIME_DAY_0(),
        DgnCoreL10N::DATETIME_DAY_1(),
        DgnCoreL10N::DATETIME_DAY_2(),
        DgnCoreL10N::DATETIME_DAY_3(),
        DgnCoreL10N::DATETIME_DAY_4(),
        DgnCoreL10N::DATETIME_DAY_5(),
        DgnCoreL10N::DATETIME_DAY_6(),
        };
    DgnCoreL10N::StringId shortNames[]={
        DgnCoreL10N::DATETIME_DAY_SHORT_0(),
        DgnCoreL10N::DATETIME_DAY_SHORT_1(),
        DgnCoreL10N::DATETIME_DAY_SHORT_2(),
        DgnCoreL10N::DATETIME_DAY_SHORT_3(),
        DgnCoreL10N::DATETIME_DAY_SHORT_4(),
        DgnCoreL10N::DATETIME_DAY_SHORT_5(),
        DgnCoreL10N::DATETIME_DAY_SHORT_6(),
        };

    DgnCoreL10N::StringId* base = useShort ? shortNames : longNames;
    return DgnCoreL10N::GetString(*(base + d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getMonthName(int m, bool useShort)
    {
    BeAssert(12 > m);

    DgnCoreL10N::StringId longNames[]={
        DgnCoreL10N::DATETIME_MONTH_0(),
        DgnCoreL10N::DATETIME_MONTH_1(),
        DgnCoreL10N::DATETIME_MONTH_2(),
        DgnCoreL10N::DATETIME_MONTH_3(),
        DgnCoreL10N::DATETIME_MONTH_4(),
        DgnCoreL10N::DATETIME_MONTH_5(),
        DgnCoreL10N::DATETIME_MONTH_6(),
        DgnCoreL10N::DATETIME_MONTH_7(),
        DgnCoreL10N::DATETIME_MONTH_8(),
        DgnCoreL10N::DATETIME_MONTH_9(),
        DgnCoreL10N::DATETIME_MONTH_10(),
        DgnCoreL10N::DATETIME_MONTH_11(),
        };

    DgnCoreL10N::StringId shortNames[]={
        DgnCoreL10N::DATETIME_MONTH_SHORT_0(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_1(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_2(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_3(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_4(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_5(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_6(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_7(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_8(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_9(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_10(),
        DgnCoreL10N::DATETIME_MONTH_SHORT_11(),
        };

    DgnCoreL10N::StringId* base = useShort ? shortNames : longNames;
    return DgnCoreL10N::GetString(*(base + m));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getAmPmName(bool pm, bool useShort)
    {
    DgnCoreL10N::StringId longNames[]={
        DgnCoreL10N::DATETIME_AM(),
        DgnCoreL10N::DATETIME_PM(),
        };
    DgnCoreL10N::StringId shortNames[]={
        DgnCoreL10N::DATETIME_AM_SHORT(),
        DgnCoreL10N::DATETIME_PM_SHORT(),
        };

    DgnCoreL10N::StringId* base = useShort ? shortNames : longNames;
    return DgnCoreL10N::GetString(*(base + (pm ? 1 : 0)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void formatUtc(Utf8StringR str, DateTimeCR local, DateTimeFormatterCR formatter, DateTimeFormatPart part)
    {
    // The point of this function is to coerce ToString into formatting a time span (vs. a time) representing offset from UTC.
    
    int64_t offsetHns;
    local.ComputeOffsetToUtcInHns(offsetHns);
    
    bool isOffsetNegative = (offsetHns < 0);
    uint64_t offsetHnsUnsigned = (isOffsetNegative ? (uint64_t)-offsetHns : (uint64_t)offsetHns);

    // We only care about the hours and minutes. Julian Date begins at 12:00, so to get a 0-based format, we need to advance 12 hours.
    // Note that we only deal in unsigned during the conversion (the sign is pre-pended separately below), so we don't need to worry about underflow.
    // Note that our representation of Julian Date is in HNS (hectonanoseconds, or 1e-7).
    const uint64_t HECTONANOSECS_IN_DAY = (86400ULL * 10000000ULL); // seconds in a day * HNS in a second
    offsetHnsUnsigned += (HECTONANOSECS_IN_DAY / 2);
    
    // It is critical to construct the offset DateTime as NOT DateTime::Kind::Local; otherwise it will apply the local time zone offset, defeating the purpose.
    DateTime offset;
    DateTime::FromJulianDay(offset, offsetHnsUnsigned, DateTime::Info(DateTime::Kind::Unspecified, DateTime::Component::DateAndTime));
    
    DateTimeFormatterPtr utcFmtr = formatter.Clone();
    utcFmtr->ClearFormatParts();
    utcFmtr->SetConvertToLocalTime(false);

    if (DATETIME_PART_U == part || DATETIME_PART_U_UU == part)
        utcFmtr->AppendFormatPart(DATETIME_PART_H);
    else
        utcFmtr->AppendFormatPart(DATETIME_PART_HH);

    if (DATETIME_PART_U_UU == part || DATETIME_PART_UU_UU == part)
        {
        utcFmtr->AppendFormatPart(DATETIME_PART_TimeSeparator);
        utcFmtr->AppendFormatPart(DATETIME_PART_mm);
        }

    str.append(1, isOffsetNegative ? '-' : '+'); 
    str.append(utcFmtr->ToString(offset));
    }

/*---------------------------------------------------------------------------------**//**
* Note I would have liked to use boost::date_time here, but it does not support all of the
* formatting we need, and relies on stringstream, which is apparently not supported on
* Android.
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DateTimeFormatter::ToString(DateTimeCR dtIn) const
    {
    DateTime dt = dtIn;
    if (m_convertToLocalTime)
        dtIn.ToLocalTime(dt);
    
    // If we have no parts, use a default format without modifying this formatter
    bvector<DateTimeFormatPart> localPartList;
    bvector<DateTimeFormatPart> const& partList = m_partList.empty() ? localPartList : m_partList;
    if (m_partList.empty())
        appendFormatPart(localPartList, DATETIME_PART_General);

    Utf8String  result;
    Utf8String  tmp;
    for (DateTimeFormatPart part: partList)
        {
        switch (part)
            {
        case DATETIME_PART_DayOfWeek:           result.append(getDayName((int)dt.GetDayOfWeek(), false)); break;
        case DATETIME_PART_DoW:                 result.append(getDayName((int)dt.GetDayOfWeek(), true)); break;
        case DATETIME_PART_D:                   tmp.Sprintf("%d", dt.GetDay()); result.append(tmp); break;
        case DATETIME_PART_DD:                  tmp.Sprintf("%02d", dt.GetDay()); result.append(tmp); break;
        case DATETIME_PART_Month:               result.append(getMonthName(dt.GetMonth() - 1, false)); break;
        case DATETIME_PART_Mon:                 result.append(getMonthName(dt.GetMonth() - 1, true)); break;
        case DATETIME_PART_M:                   tmp.Sprintf("%d", dt.GetMonth()); result.append(tmp); break;
        case DATETIME_PART_MM:                  tmp.Sprintf("%02d", dt.GetMonth()); result.append(tmp); break;
        case DATETIME_PART_d:                   tmp.Sprintf("%d", dt.GetDayOfYear()); result.append(tmp); break;
        case DATETIME_PART_ddd:                 tmp.Sprintf("%03d", dt.GetDayOfYear()); result.append(tmp); break;
        case DATETIME_PART_YYYY:                tmp.Sprintf("%d", dt.GetYear()); result.append(tmp); break;
        case DATETIME_PART_h:
        case DATETIME_PART_hh:
            {
            int hour = dt.GetHour();
            if (0 == hour)      hour = 12;
            else if (12 < hour) hour -= 12;
            tmp.Sprintf((DATETIME_PART_h == part ? "%d" : "%02d"), hour);
            result.append(tmp);
            }
            break;
        case DATETIME_PART_H:                   tmp.Sprintf("%d", dt.GetHour()); result.append(tmp); break;
        case DATETIME_PART_HH:                  tmp.Sprintf("%02d", dt.GetHour()); result.append(tmp); break;
        case DATETIME_PART_m:                   tmp.Sprintf("%d", dt.GetMinute()); result.append(tmp); break;
        case DATETIME_PART_mm:                  tmp.Sprintf("%02d", dt.GetMinute()); result.append(tmp); break;
        case DATETIME_PART_s:                   tmp.Sprintf("%d", dt.GetSecond()); result.append(tmp); break;
        case DATETIME_PART_ss:                  tmp.Sprintf("%02d", dt.GetSecond()); result.append(tmp); break;
        case DATETIME_PART_Comma:               result.append(1, ','); break;
        case DATETIME_PART_DateSeparator:       result.append(1, m_dateSeparator); break;
        case DATETIME_PART_TimeSeparator:       result.append(1, m_timeSeparator); break;
        case DATETIME_PART_DecimalSeparator:    result.append(1, m_decimalSeparator); break;
        case DATETIME_PART_Space:               result.append(1, ' '); break;
        case DATETIME_PART_AMPM:                result.append(getAmPmName(12 <= dt.GetHour(), false)); break;
        case DATETIME_PART_AP:                  result.append(getAmPmName(12 <= dt.GetHour(), true)); break;
        case DATETIME_PART_UTC:                 result.append(DgnCoreL10N::GetString(DgnCoreL10N::DATETIME_UTC()).c_str()); break;
        case DATETIME_PART_Y:                   tmp.Sprintf("%d", dt.GetYear() % 100); result.append(tmp); break;
        case DATETIME_PART_YY:                  tmp.Sprintf("%02d", dt.GetYear() % 100); result.append(tmp); break;
        case DATETIME_PART_YYY:                 tmp.Sprintf("%03d", dt.GetYear()); result.append(tmp); break;
        case DATETIME_PART_YYYYY:               tmp.Sprintf("%05d", dt.GetYear()); result.append(tmp); break;
        case DATETIME_PART_FractionalSeconds:
            {
            // Note input value gives us a precision of 3 - no more, no less.
            uint64_t ms = dt.GetMillisecond();
            double dms = static_cast<double>(ms);
            tmp.clear();
            switch (m_fractionalPrecision)
                {
            case 0:     break;
            case 1:     tmp.Sprintf("%d", (int32_t)(dms / 100.0 + 0.5)); break;
            case 2:     tmp.Sprintf("%02d", (int32_t)(dms / 10.0 + 0.5)); break;
            default:    tmp.Sprintf("%03d", ms); break;
                }
            
            if (!m_fractionalTrailingZeros)
                {
                while (0 < tmp.size() && '0' == tmp[tmp.size()-1])
                    tmp.erase(tmp.size()-1);
                }
            else
                {
                size_t nZeros = m_fractionalPrecision - tmp.size();
                if (0 < nZeros)
                    tmp.append(nZeros, '0');
                }

            result.append(tmp);
            }
            break;
        case DATETIME_PART_U:
        case DATETIME_PART_UU:
        case DATETIME_PART_U_UU:
        case DATETIME_PART_UU_UU:
            formatUtc(result, dt, *this, part);
            break;
        default:
            BeAssert(false); break;
            }
        }

    return result;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
