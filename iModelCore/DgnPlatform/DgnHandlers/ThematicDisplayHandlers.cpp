/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ThematicDisplayHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

DGNPLATFORM_TYPEDEFS (HeightDisplayHandlerKey)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/1993
+---------------+---------------+---------------+---------------+---------------+------*/
static void azimuthAnglesToDirection
(               
DPoint3dR       direction,                /* <= direction */
double          azimuth,                    /* => azimuth in radians */
double          altitude,                   /* => altitude in radians */
DgnModelCR   modelRef
)
    {
    direction.x = sin (azimuth) * cos (altitude);
    direction.y = cos (azimuth) * cos (altitude);
    direction.z = sin (altitude);

    /* Compensate for true North */
    RotMatrix   azimuthRotMatrix;
    double      azimuthAngle = modelRef.GetDgnProject().Units().GetAzimuth();

    azimuthRotMatrix.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (),  0.0,  0.0,  azimuthAngle*msGeomConst_radiansPerDegree);
    azimuthRotMatrix.Multiply(direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   getDgnModelRange (DRange3dR range, DgnModelR modelRef, ViewportP viewport = NULL)
    {
    modelRef.GetRange(range);

    return range.IsNull() ? ERROR : SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
+===============+===============+===============+===============+===============+======*/
enum        HeightDisplayMode
    {
    HeightDisplayMode_Z      = 0,
    HeightDisplayMode_Y,
    HeightDisplayMode_X,
    };


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
+===============+===============+===============+===============+===============+======*/
struct HeightDisplayHandlerKey : public ThematicDisplayStyleHandlerKey
{
private:
    DRange3d    m_range;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HeightDisplayHandlerKey (DgnModelR modelRef, ThematicDisplaySettingsCR settings, ThematicDisplayStyleHandlerCR handler)
    : ThematicDisplayStyleHandlerKey (settings, handler)
    {
    getDgnModelRange (m_range, modelRef);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    Matches (DisplayStyleHandlerKey const& other) const override
    {
    if (GetHandlerId () != other.GetHandlerId () ||
        !ThematicDisplayStyleHandlerKey::Matches (other))
        return false;
    
    HeightDisplayHandlerKeyCP     otherKey = static_cast <HeightDisplayHandlerKeyCP> (&other);

    return  0 == memcmp (&m_range, &otherKey->m_range, sizeof (m_range));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static  DisplayStyleHandlerKeyPtr  Create (DgnModelR modelRef, ThematicDisplaySettingsCR settings, ThematicDisplayStyleHandlerCR handler)
    {
    return new HeightDisplayHandlerKey (modelRef, settings, handler);
    }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString HeightDisplayHandler::_GetName () const
    {
    return DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_HeightName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId    HeightDisplayHandler::_GetHandlerId () const
    {
    return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_Height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool HeightDisplayHandler::_IsValidForViewport (ViewportCR viewport) const
    {
    return viewport.Is3dView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getRelevantCoord (DPoint3dCR point, ThematicDisplaySettingsCR settings)
    {
    switch (settings.GetDisplayMode ())
        {
        case HeightDisplayMode_X:
            return point.x;

        case HeightDisplayMode_Y:
            return point.y;

        case HeightDisplayMode_Z:
        default:
            return point.z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double  HeightDisplayHandler::_GetPointValue (DPoint3dCR point, ThematicDisplaySettingsCR settings) const
    {
    return getRelevantCoord (point, settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt HeightDisplayHandler::_GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const
    {
    switch (mode)
        {
        case HeightDisplayMode_X:
            label = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_HeightX);
            return SUCCESS;

        case HeightDisplayMode_Y:
            label = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_HeightY);
            return SUCCESS;

        case HeightDisplayMode_Z:
            label = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_HeightZ);
            return SUCCESS;
        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerKeyPtr HeightDisplayHandler::_GetCacheKey  (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const
    {
    ThematicDisplaySettingsCP thematicSettings = dynamic_cast <ThematicDisplaySettingsCP> (settings);

    if (NULL == thematicSettings)
        return NULL;

    return HeightDisplayHandlerKey::Create (modelRef, *thematicSettings, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    HeightDisplayHandler:: _CookRange (ThematicDisplaySettingsR settings, ViewportR viewport, DgnModelR modelRef) const
    {
    double uorPerMeter = 1000.;

    DPoint3d globalOrigin = modelRef.GetGlobalOrigin();

    if (!settings.IsMinFixed() || !settings.IsMaxFixed())
        {
        DRange3d        range;
        double const    EPSILON = 1E-5;

        if (SUCCESS == getDgnModelRange (range, modelRef))
            {
            double min = (getRelevantCoord (range.low, settings)  - getRelevantCoord (globalOrigin, settings))/ uorPerMeter,
                   max = (getRelevantCoord (range.high, settings) - getRelevantCoord (globalOrigin, settings))/ uorPerMeter,
                   oldMin, oldMax;

            settings.GetRawRange (oldMin, oldMax);

            // When updating the range, ensure max >= min
            if (!settings.IsMinFixed())
                {
                if (settings.IsMaxFixed () && min + EPSILON > oldMax)
                    settings.SetRangeMin (oldMax - EPSILON);
                else
                    settings.SetRangeMin (min);
                }

            if (!settings.IsMaxFixed())
                {
                if (settings.IsMinFixed () && max + EPSILON < oldMin)
                    settings.SetRangeMax (oldMin + EPSILON);
                else
                    settings.SetRangeMax (max);
                }
            }
        }

    double          min, max;

    settings.GetRange (min, max);
    settings.m_cookedRange.Set (min * uorPerMeter + getRelevantCoord (globalOrigin, settings), max * uorPerMeter + getRelevantCoord (globalOrigin, settings));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        HeightDisplayHandler::_GetRawValueFromString
(
double&                     value,
ThematicDisplaySettingsCR   settings,
WStringCR                   string,
DgnModelR                modelRef
) const
    {
    StatusInt   status;

    if (SUCCESS == (status = GetUorsFromString (value, string, modelRef)))
        value /= 1000.;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString         HeightDisplayHandler::_GetStringFromRawValue
(
double                      value,
ThematicDisplaySettingsCR   settings,
DgnModelR                modelRef,
bool                        includeUnits
) const
    {
    return GetStringFromUors (value * 1000., modelRef, includeUnits);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
*   This is a shortcut. As the height value can be derived from the hit point directly
*   without parsing the mesh to determine the value, we _GetHitInfoString and
*   return it direct here.
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       HeightDisplayHandler::_GetHitInfoString
(
WStringR                    string,
HitPathCR                   hitPath,
DPoint3dCR                  hitPoint,
ThematicDisplaySettingsCR   settings,
ViewportR                   viewport,
ElementHandleR              el,
WCharCP                     delimiter
) const
    {
    DgnModelP            frustumModel = settings.GetFrustumModel();

    DPoint3d globalOrigin = frustumModel->GetGlobalOrigin();

    double distance = getRelevantCoord (hitPoint, settings) - getRelevantCoord (globalOrigin, settings);

    string = _GetName() + WString(L": ") + GetStringFromUors (distance, *frustumModel, false);

    return SUCCESS;
    }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct      BaseNormalThematicMeshDisplayStyleHandler  :  NormalThematicMeshDisplayStyleHandler
{
    virtual bool _IsValidForViewport (ViewportCR viewport) const override { return viewport.Is3dView(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    _CookRange (ThematicDisplaySettingsR settings, ViewportR viewport, DgnModelR modelRef) const override
    {
    if (!settings.IsMinFixed())
        settings.SetRangeMin (_GetMinValue(settings.GetDisplayMode()));

    if (!settings.IsMaxFixed())
        settings.SetRangeMax (_GetMaxValue(settings.GetDisplayMode()));

    double          min, max;

    settings.GetRange (min, max);
    settings.m_cookedRange.Set (min, max);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _InitDefaultSettings (ThematicDisplaySettingsR settings ) const override
    {
    NormalThematicMeshDisplayStyleHandler::_InitDefaultSettings (settings);
    settings.SetFixedRange (_GetMinValue(settings.GetDisplayMode()), _GetMaxValue(settings.GetDisplayMode()));
    }

};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct  AngleThematicMeshDisplayStyleHandler :  BaseNormalThematicMeshDisplayStyleHandler
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   _GetRawValueFromString (double& value, ThematicDisplaySettingsCR settings, WStringCR string, DgnModelR modelRef) const override
    {
    AngleParserPtr   angleParser = AngleParser::Create(modelRef);

    return angleParser->ToValue (value, string.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString        _GetStringFromRawValue (double value, ThematicDisplaySettingsCR, DgnModelR modelRef, bool includeUnits) const override
    {
    return AngleFormatter::Create(modelRef)->ToString (value);
    }
};  //AngleThematicMeshDisplayStyleHandler

enum        SlopeDisplayMode
    {
    SlopeDisplayMode_Angle      = 0,
    SlopeDisplayMode_Percent,
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct      SlopeDisplayHandler  :  BaseNormalThematicMeshDisplayStyleHandler
{
    virtual XAttributeHandlerId _GetHandlerId () const override { return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_SlopeAngle); }
    virtual WString _GetName () const override { return DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_SlopeName); }
    virtual double _GetMinValue (ThematicDisplayMode mode) const override { return 0.0; }
    virtual double _GetMaxValue (ThematicDisplayMode mode) const override { return (SlopeDisplayMode_Angle == mode) ? 90.0 : 100.0; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double   _GetNormalValue (DVec3dCR normal, bool twoSided, ThematicDisplaySettingsCR settings) const override
    {
    DVec3d      normalized;

    normalized.normalize (&normal);

    if (SlopeDisplayMode_Angle == settings.GetDisplayMode())
        {
        return msGeomConst_degreesPerRadian * acos (fabs (normalized.z));
        }
    else
        {
        double      run = fabs (normalized.z), rise = normalized.magnitudeXY();

        return  100.0 * rise / MAX (1.0E-4, run);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const override
    {
    switch (mode)
        {
        case SlopeDisplayMode_Angle:
            label = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_Angle);
            return SUCCESS;

        case SlopeDisplayMode_Percent:
            label = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_Percent);
            return SUCCESS;

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _CacheOnDisplayModeChange () const override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   _GetRawValueFromString (double& value, ThematicDisplaySettingsCR settings, WStringCR string, DgnModelR modelRef) const override
    {
    if (SlopeDisplayMode_Angle == settings.GetDisplayMode())
        {
        AngleParserPtr   angleParser = AngleParser::Create(modelRef);

        return angleParser->ToValue (value, string.c_str());
        }
    else
        {
        return BaseNormalThematicMeshDisplayStyleHandler::_GetRawValueFromString (value, settings, string, modelRef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString        _GetStringFromRawValue (double value, ThematicDisplaySettingsCR settings, DgnModelR modelRef, bool includeUnits) const override
    {
    if (SlopeDisplayMode_Angle == settings.GetDisplayMode())
        {
        return AngleFormatter::Create(modelRef)->ToString (value);
        }
    else
        {
        char        chars[1024];

        sprintf (chars, "%.2f%%", value);

        return WString (chars, false);
        }
    }
};  // SlopeDisplayHandler


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct      AspectDisplayHandler  :  AngleThematicMeshDisplayStyleHandler
{
    virtual XAttributeHandlerId _GetHandlerId () const override { return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_Aspect); }
    virtual WString _GetName () const override { return DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_AspectAngleName); }
    virtual double _GetMinValue (ThematicDisplayMode) const override { return 0.0; }
    virtual double _GetMaxValue (ThematicDisplayMode) const override { return 360.0; }
    virtual bool _IsPeriodic () const override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double          _GetNormalValue (DVec3dCR normal, bool twoSided, ThematicDisplaySettingsCR settings) const override
    {
    double          angle = (twoSided && normal.z < 0.0) ? atan2 (-normal.x, -normal.y) : atan2 (normal.x, normal.y);

    if (angle < 0.0)
        angle += msGeomConst_2pi;

    return angle * msGeomConst_degreesPerRadian;
    }


};  // AspectDisplayHandler





#define     DEFAULT_AZIMUTH      315.0;
#define     DEFAULT_ALTITUDE     45.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HillShadeDisplaySettings::HillShadeDisplaySettings ()
    {
    m_data.m_azimuth  = DEFAULT_AZIMUTH;
    m_data.m_altitude = DEFAULT_ALTITUDE;
    ThematicDisplaySettings::m_data.m_colorScheme = ThematicColorScheme_Monochrome;
    ThematicDisplaySettings::m_data.m_flags.m_noLegend = true;
    m_colorMap.Init (*this, 512);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   HillShadeDisplaySettings::_Save (ElementRefP ElementRefP, int styleIndex)
    {
    ThematicDisplaySettings::_Save (ElementRefP, styleIndex);

    return SaveData (&m_data, sizeof (m_data), ElementRefP, DisplayStyleHandler_SettingsXAttributeSubId_HillShade, styleIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   HillShadeDisplaySettings::_Read (ElementRefP elementRefP, int styleIndex)
    {
    StatusInt           status;
    bvector<byte>       buffer;

    if (SUCCESS != (status = ThematicDisplaySettings::_Read (elementRefP, styleIndex)) ||
        SUCCESS != (status = ReadData (buffer, elementRefP, DisplayStyleHandler_SettingsXAttributeSubId_HillShade, styleIndex)))
        return status;

    if (buffer.size() < sizeof (m_data))
        return ERROR;

    memcpy (&m_data, &buffer[0], sizeof (m_data));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HillShadeDisplaySettings::_Equals (DisplayStyleHandlerSettingsCP rhs) const
    {
     HillShadeDisplaySettings const*   hillShadeSettings = NULL;

    return  ThematicDisplaySettings::_Equals (rhs) &&
            NULL != (hillShadeSettings = dynamic_cast <HillShadeDisplaySettings const*> (rhs)) &&
            0 == memcmp (&m_data, &hillShadeSettings->m_data, sizeof (m_data));
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011
+===============+===============+===============+===============+===============+======*/
struct      HillShadeDisplayHandler  :  BaseNormalThematicMeshDisplayStyleHandler
{
    virtual XAttributeHandlerId _GetHandlerId () const override { return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_HillShade); }
    virtual WString _GetName () const override { return DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_THEMATIC_HillShadeName); }
    virtual double _GetMinValue (ThematicDisplayMode mode) const override { return 0.0; }
    virtual double _GetMaxValue (ThematicDisplayMode mode) const override { return 100.0; }
    virtual StatusInt _GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const override { return ERROR; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnFrustumChange (DisplayStyleHandlerSettingsR settings, ViewContextR viewContext, DgnModelR modelRef) const override 
    { 
    HillShadeDisplaySettings*     hillshadeSettings;

    if (NULL == (hillshadeSettings = dynamic_cast <HillShadeDisplaySettings *> (&settings)))
        {
        BeAssert (false);
        return;
        }

    azimuthAnglesToDirection (hillshadeSettings->m_direction, msGeomConst_radiansPerDegree * hillshadeSettings->GetAzimuthAngle(), msGeomConst_radiansPerDegree * hillshadeSettings->GetAltitudeAngle(), modelRef);
    BaseNormalThematicMeshDisplayStyleHandler::_OnFrustumChange (settings, viewContext, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double   _GetNormalValue (DVec3dCR normal, bool twoSided, ThematicDisplaySettingsCR settings) const override
    {
    HillShadeDisplaySettings const*     hillshadeSettings;

    if (NULL == (hillshadeSettings = dynamic_cast <HillShadeDisplaySettings const*> (&settings)))
        {
        BeAssert (false);
        return 0.0;
        }
    double      dotProduct = normal.dotProduct (&hillshadeSettings->m_direction) / normal.magnitude();

    if (twoSided && normal.z < 0.0)
        dotProduct = -dotProduct;

    return 100.0 * MAX (0.0, dotProduct);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString        _GetStringFromRawValue (double value, ThematicDisplaySettingsCR settings, DgnModelR modelRef, bool includeUnits) const override
    {
    char        chars[1024];

    sprintf (chars, "%.2f%%", value);

    return WString (chars, false);
    }

};  // HillShadeDisplayHandler

#ifdef NOTNOW_EXAMPLE_HANDLERS

static char *s_timeFormat =    "%H:%M:%S %m/%d/%y";

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2010
+===============+===============+===============+===============+===============+======*/
struct  ElementModificationTimeDisplayHandler : ThematicElementPropertyDisplayHandler
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual WString                 _GetName () const override
    {
    return WString ("Element Change Time");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual XAttributeHandlerId     _GetHandlerId () const override
    {
    return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ElementModificationTime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual bool   _GetElementValue (double& value, ElementHandleCR el, ViewportR viewport) const override
    {
    value = el.GetElementCP()->ehdr.lastModified;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsValidForDisplayStyle (DisplayStyleCR style) const override
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
static void   GetModelModificationTimeRange (double& min, double& max, DgnModelP modelRef)
    {
    DgnCacheP           dgnCache;
    DgnElmList*         elmList;

    if (NULL == (dgnCache = mdlDgnModel_getCache(modelRef)) ||
        NULL == (elmList = dgnCache->GetGraphicElms()))
        return;

    DgnElmListIterator          iter;

    for (CacheElemRef elRef = iter.FirstCacheElm(elmList); NULL != elRef; elRef = iter.NextCacheElm ())
        {
        Elm_hdr     ehdr;

        ElementRefP_getElementHeader (elRef, &ehdr);

        if (.IsGraphic())
            {
            if (ehdr.lastModified < min)
                min = ehdr.lastModified;

            if (ehdr.lastModified > max)
                max = ehdr.lastModified;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual bool _GetModelRange (double& min, double& max, DgnModelP modelRef) const override
    {
    min = DBL_MAX;
    max = DBL_MIN;

    DgnModelIteratorP           modelIterator;

    mdlDgnModelIterator_create (&modelIterator, modelRef, MRITERATE_Root | MRITERATE_PrimaryChildRefs, -1);

    for (DgnModelP   iterateDgnModel = mdlDgnModelIterator_getFirst (modelIterator);
            NULL != iterateDgnModel;
                iterateDgnModel = mdlDgnModelIterator_getNext (modelIterator))
        GetModelModificationTimeRange (min, max, iterateDgnModel);

    mdlDgnModelIterator_free (&modelIterator);

    return max > min;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual WString   _GetStringFromRawValue
(
double                      value,
ThematicDisplaySettingsCR   settings,
DgnModelP                modelRef,
bool                        includeUnits
) const override
    {
    time_t          time = (time_t) (value/1000.0);
    struct tm       localTm  = *localtime (&time);
    char            chars[1024];

    strftime (chars, sizeof(chars), s_timeFormat, &localTm);

    return WString (chars);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetRawValueFromString
(
double&                     value,
ThematicDisplaySettingsCR   settings,
WStringCR                   string,
DgnModelP                modelRef
) const override
    {
    char            chars[1024];
    struct tm       tmStruct;

    if (NULL == strptime (string.ToChar (chars, sizeof(chars)), s_timeFormat, &tmStruct))
        return ERROR;

    value = 1000.0 * (double) mktime (&tmStruct);

    return SUCCESS;
    }

};  //  ElementModificationTimeDisplayHandler

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2010
+===============+===============+===============+===============+===============+======*/
struct  RangeDisplayStyleHandler : DisplayStyleHandler
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual WString _GetName () const override     { return ConfigurationManager::GetString (DGNHANDLERS_STRINGS, MSGID_THEMATIC_ElementRangeName); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual XAttributeHandlerId     _GetHandlerId () const override
    {
    return XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ElementRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt               _EditSettings (DisplayStyleR style, MdlDesc*& mdlDesc, UInt32& itemType, Int32& itemId) const override
    {
    return EditSettingsWithContainer (style, mdlDesc, itemType, itemId, CONTAINERID_DisplayStyleHandlerApply);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsValidForDisplayStyle (DisplayStyleCR style) const override
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+--------------+---------------+---------------+---------------+---------------+------*/
virtual bool   _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override
    {
    DgnElementCP                     element;
    DisplayStyleHandlerSettingsCP   settings;

    if (NULL != (settings = viewContext.GetDisplayStyleHandlerSettings()) &&
        !settings->DoFilter (*viewContext.GetCurrDisplayPath()) &&
        NULL != (element = el.GetElementCP()) && element->IsGraphic())
        {
        DRange3d    range;
        DPoint2d        box[4];

        DataConvert::ScanRangeToDRange3d (range, element->hdr.dhdr.range);

        box[0].x = box[3].x = range.low.x;
        box[1].x = box[2].x = range.high.x;
        box[0].y = box[1].y = range.low.y;
        box[2].y = box[3].y = range.high.y;

        for (int i=0; i<4; i++)
            {
            int             iNext = (i+1) % 4;
            DPoint3d        edge[2];

            edge[0].init (box[i].x, box[i].y, range.low.z);
            edge[1].init (box[iNext].x, box[iNext].y, range.low.z);
            viewContext.GetIDrawGeom().DrawLineString3d (2, edge, NULL);

            edge[0].z = edge[1].z = range.high.z;
            viewContext.GetIDrawGeom().DrawLineString3d (2, edge, NULL);

            edge[0].init (box[i].x, box[i].y, range.low.z);
            edge[1].init (box[i].x, box[i].y, range.high.z);
            viewContext.GetIDrawGeom().DrawLineString3d (2, edge, NULL);
            }
        return true;
        }
    return false;
    }

};  // RangeDisplayStyleHandler

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+--------------+---------------+---------------+---------------+---------------+------*/
void ThematicDisplayStyleHandler::RegisterHandlers ()
    {
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new HeightDisplayHandler());
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new SlopeDisplayHandler());
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new AspectDisplayHandler());
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new HillShadeDisplayHandler());


#ifdef NEEDS_WORK_LEGEND_DISPLAY
    IElementHandlerManager::RegisterElementHandler (Bentley::Ustn::Element::ElementHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_ThematicLegendElementHandler), *thematicDisplay_getLegendElementHandler ());
#endif

#ifdef NOTNOW_EXAMPLE_HANDLERS
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new RangeDisplayStyleHandler());
    DisplayStyleHandlerManager::GetManager().RegisterHandler (*new ElementModificationTimeDisplayHandler());
    ElementNeighborhoodDisplayHelper::RegisterDisplayStyleHandler();
#endif

    }

