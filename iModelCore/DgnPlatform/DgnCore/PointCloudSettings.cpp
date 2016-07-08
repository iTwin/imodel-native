/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PointCloudSettings.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/PointCloudSettings.h>

static Utf8CP SETTINGPOINTCLOUD_flags               = "flags";
static Utf8CP SETTINGPOINTCLOUD_contrast            = "contrast";
static Utf8CP SETTINGPOINTCLOUD_brightness          = "brightness";
static Utf8CP SETTINGPOINTCLOUD_distance            = "distance";
static Utf8CP SETTINGPOINTCLOUD_offset              = "offset";
static Utf8CP SETTINGPOINTCLOUD_adaptivePointSize   = "adaptivePointSize";
static Utf8CP SETTINGPOINTCLOUD_intensityRampIdx    = "intensityRampIdx";
static Utf8CP SETTINGPOINTCLOUD_planeRampIdx        = "planeRampIdx";
static Utf8CP SETTINGPOINTCLOUD_planeAxis           = "planeAxis";
static Utf8CP SETTINGPOINTCLOUD_displayStyle        = "displayStyle";
static Utf8CP SETTINGPOINTCLOUD_planeRamp           = "planeRamp";
static Utf8CP SETTINGPOINTCLOUD_intensityRamp       = "intensityRamp";
static Utf8CP SETTINGPOINTCLOUD_useACSAsPlaneAxis   = "useACSAsPlaneAxis";
static Utf8CP SETTINGPOINTCLOUD_clampIntensity      = "clampIntensity";
static Utf8CP SETTINGPOINTCLOUD_needClassifBuffer   = "needClassifBuffer";
static Utf8CP SETTINGPOINTCLOUD_displayStyleName    = "displayStyleName";
static Utf8CP SETTINGPOINTCLOUD_displayStyleIndex   = "displayStyleIndex";

static Utf8CP SETTINGCLASSIF_classifColor           = "classifColor";
static Utf8CP SETTINGCLASSIF_blendColor             = "blendColor";
static Utf8CP SETTINGCLASSIF_useBaseColor           = "useBaseColor";
static Utf8CP SETTINGCLASSIF_visibleState           = "visibleState";
static Utf8CP SETTINGCLASSIF_activeState            = "activeState";
static Utf8CP SETTINGCLASSIF_unclassColor           = "unclassColor";
static Utf8CP SETTINGCLASSIF_unclassVisible         = "unclassVisible";

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
void PointCloudViewSettings::InitDefaults()
    {
    m_flags = VIEWSETTINGS_FRONTBIAS_MASK;
    m_contrast = GetDefaultViewContrast();
    m_brightness = GetDefaultViewBrightness();
    m_distance = 10.0f;
    m_offset = 0;
    m_adaptivePointSize = 0;
    m_intensityRampIdx = 0;
    m_planeRampIdx = 1;
    m_planeAxis = 2;
    m_useACSAsPlaneAxis = false;
    m_displayStyle = DisplayStyle::None;
    m_planeRamp     = "";
    m_intensityRamp = "";
    m_clampIntensity = false;
    m_needClassifBuffer = false;
    m_displayStyleName = "";
    m_displayStyleIndex = -1; //Set it to -1 it means display style none
    }

//-----------------------------------------------------------------------------------------
// Verify if this PointCloudViewSettings is equal to default settings
// @bsimethod                                                   Eric.Paquet         5/2016
//-----------------------------------------------------------------------------------------
bool PointCloudViewSettings::AreSetToDefault() const
    {
    PointCloudViewSettings tmpViewSettings;

    if (m_flags != tmpViewSettings.m_flags  ||
        m_contrast != tmpViewSettings.m_contrast ||
        m_brightness != tmpViewSettings.m_brightness ||
        m_distance != tmpViewSettings.m_distance ||
        m_offset != tmpViewSettings.m_offset ||
        m_adaptivePointSize != tmpViewSettings.m_adaptivePointSize ||
        m_intensityRampIdx != tmpViewSettings.m_intensityRampIdx ||
        m_planeRampIdx != tmpViewSettings.m_planeRampIdx ||
        m_planeAxis != tmpViewSettings.m_planeAxis ||
        m_useACSAsPlaneAxis != tmpViewSettings.m_useACSAsPlaneAxis ||
        m_displayStyle != tmpViewSettings.m_displayStyle ||
        m_planeRamp != tmpViewSettings.m_planeRamp ||
        m_intensityRamp != tmpViewSettings.m_intensityRamp ||
        m_clampIntensity != tmpViewSettings.m_clampIntensity ||
        m_needClassifBuffer != tmpViewSettings.m_needClassifBuffer ||
        m_displayStyleName != tmpViewSettings.m_displayStyleName ||
        m_displayStyleIndex != tmpViewSettings.m_displayStyleIndex)
        {
        return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
void PointCloudViewSettings::FromJson(JsonValueCR val)
    {
    m_flags                 = val[SETTINGPOINTCLOUD_flags].asUInt();
    m_contrast              = val[SETTINGPOINTCLOUD_contrast].asDouble();
    m_brightness            = val[SETTINGPOINTCLOUD_brightness].asDouble();
    m_distance              = val[SETTINGPOINTCLOUD_distance].asDouble();
    m_offset                = val[SETTINGPOINTCLOUD_offset].asDouble();
    m_adaptivePointSize     = val[SETTINGPOINTCLOUD_adaptivePointSize].asInt();
    m_intensityRampIdx      = val[SETTINGPOINTCLOUD_intensityRampIdx].asUInt();
    m_planeRampIdx          = val[SETTINGPOINTCLOUD_planeRampIdx].asUInt();
    m_planeAxis             = val[SETTINGPOINTCLOUD_planeAxis].asUInt();
    m_displayStyle          = DisplayStyle(val[SETTINGPOINTCLOUD_displayStyle].asUInt());
    m_planeRamp             = val[SETTINGPOINTCLOUD_planeRamp].asString();
    m_intensityRamp         = val[SETTINGPOINTCLOUD_intensityRamp].asString();
    m_useACSAsPlaneAxis     = val[SETTINGPOINTCLOUD_useACSAsPlaneAxis].asBool();
    m_clampIntensity        = val[SETTINGPOINTCLOUD_clampIntensity].asBool();
    m_needClassifBuffer     = val[SETTINGPOINTCLOUD_needClassifBuffer].asBool();
    m_displayStyleName      = val[SETTINGPOINTCLOUD_displayStyleName].asString();
    m_displayStyleIndex     = val[SETTINGPOINTCLOUD_displayStyleIndex].asInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
void PointCloudViewSettings::ToJson(JsonValueR val) const
    {
    val[SETTINGPOINTCLOUD_flags]                = m_flags;
    val[SETTINGPOINTCLOUD_contrast]             = m_contrast;
    val[SETTINGPOINTCLOUD_brightness]           = m_brightness;
    val[SETTINGPOINTCLOUD_distance]             = m_distance;
    val[SETTINGPOINTCLOUD_offset]               = m_offset;
    val[SETTINGPOINTCLOUD_adaptivePointSize]    = m_adaptivePointSize;
    val[SETTINGPOINTCLOUD_intensityRampIdx]     = m_intensityRampIdx;
    val[SETTINGPOINTCLOUD_planeRampIdx]         = m_planeRampIdx;
    val[SETTINGPOINTCLOUD_planeAxis]            = m_planeAxis;
    val[SETTINGPOINTCLOUD_displayStyle]         = (uint32_t) m_displayStyle;
    val[SETTINGPOINTCLOUD_planeRamp]            = m_planeRamp.c_str();
    val[SETTINGPOINTCLOUD_intensityRamp]        = m_intensityRamp.c_str();
    val[SETTINGPOINTCLOUD_useACSAsPlaneAxis]    = m_useACSAsPlaneAxis;
    val[SETTINGPOINTCLOUD_clampIntensity]       = m_clampIntensity;
    val[SETTINGPOINTCLOUD_needClassifBuffer]    = m_needClassifBuffer;
    val[SETTINGPOINTCLOUD_displayStyleName]     = m_displayStyleName.c_str();
    val[SETTINGPOINTCLOUD_displayStyleIndex]    = m_displayStyleIndex;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         7/2016
//-----------------------------------------------------------------------------------------
void PointCloudClassificationSettings::InitDefaults()
    {
    m_blendColor = false;
    m_useBaseColor = false;

    // Default values for classification colors
    m_classificationColor[0]    = ColorDef(255, 255, 0);
    m_classificationColor[1]    = ColorDef(255, 255, 255);
    m_classificationColor[2]    = ColorDef(255, 0, 0);
    m_classificationColor[3]    = ColorDef(0, 200, 50);
    m_classificationColor[4]    = ColorDef(0, 200, 100);
    m_classificationColor[5]    = ColorDef(0, 200, 200);
    m_classificationColor[6]    = ColorDef(150, 150, 150);
    m_classificationColor[7]    = ColorDef(0, 50, 100);
    m_classificationColor[8]    = ColorDef(255, 255, 255);
    m_classificationColor[9]    = ColorDef(0, 0, 255);
    m_classificationColor[10]   = ColorDef(0, 0, 0);
    m_classificationColor[11]   = ColorDef(0, 0, 0);
    m_classificationColor[12]   = ColorDef(0, 0, 0);

    // Initialize other colors to white
    for (int i = 13; i < CLASSIFICATION_COUNT; i++)
        {
        m_classificationColor[i] = ColorDef::White();
        }

    for (int i = 0; i < CLASSIFICATION_STATES; i++)
        {
        // All visible; all active
        m_visibleState[i] = true;
        m_activeState[i] = true;
        }

    m_unclassColor = ColorDef(255, 0, 255);
    m_unclassVisible = true;
    }

//-----------------------------------------------------------------------------------------
// Verify if this PointCloudClassificationSettings is equal to default settings
// @bsimethod                                                   Eric.Paquet         7/2016
//-----------------------------------------------------------------------------------------
bool PointCloudClassificationSettings::AreSetToDefault() const
    {
    PointCloudClassificationSettings tmpClassifSettings;

    if (*this == tmpClassifSettings)
        return true;

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         7/2016
//-----------------------------------------------------------------------------------------
bool PointCloudClassificationSettings::operator==(PointCloudClassificationSettings const& rhs) const
    {
    if (m_blendColor != rhs.m_blendColor || 
        m_useBaseColor != rhs.m_useBaseColor ||
        m_unclassColor != rhs.m_unclassColor ||
        m_unclassVisible != rhs.m_unclassVisible
        )
        {
        return false;
        }

    if (!std::equal(std::begin(m_classificationColor), std::end(m_classificationColor), std::begin(rhs.m_classificationColor)) ||
        !std::equal(std::begin(m_visibleState), std::end(m_visibleState), std::begin(rhs.m_visibleState)) ||
        !std::equal(std::begin(m_activeState), std::end(m_activeState), std::begin(rhs.m_activeState)))
        {
        return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         7/2016
//-----------------------------------------------------------------------------------------
void PointCloudClassificationSettings::FromJson(JsonValueCR inValue)
    {
    for (int i = 0; i < CLASSIFICATION_COUNT; ++i)
        m_classificationColor[i] = ColorDef(inValue[SETTINGCLASSIF_classifColor][i].asUInt());
    m_blendColor = inValue[SETTINGCLASSIF_blendColor].asBool();
    m_useBaseColor = inValue[SETTINGCLASSIF_useBaseColor].asBool();

    for (int i = 0; i < CLASSIFICATION_STATES; ++i)
        m_visibleState[i] = inValue[SETTINGCLASSIF_visibleState][i].asBool();
    for (int i = 0; i < CLASSIFICATION_STATES; ++i)
        m_activeState[i] = inValue[SETTINGCLASSIF_activeState][i].asBool();

    m_unclassColor = ColorDef(inValue[SETTINGCLASSIF_unclassColor].asUInt());
    m_unclassVisible = inValue[SETTINGCLASSIF_unclassVisible].asBool();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         7/2016
//-----------------------------------------------------------------------------------------
void PointCloudClassificationSettings::ToJson(JsonValueR outValue) const
    {
    for (int i = 0; i < CLASSIFICATION_COUNT; ++i)
        outValue[SETTINGCLASSIF_classifColor][i] = m_classificationColor[i].GetValue();

    outValue[SETTINGCLASSIF_blendColor] = m_blendColor;
    outValue[SETTINGCLASSIF_useBaseColor] = m_useBaseColor;

    for (int i = 0; i < CLASSIFICATION_STATES; ++i)
        outValue[SETTINGCLASSIF_visibleState][i] = m_visibleState[i];
    for (int i = 0; i < CLASSIFICATION_STATES; ++i)
        outValue[SETTINGCLASSIF_activeState][i] = m_activeState[i];

    outValue[SETTINGCLASSIF_unclassColor] = m_unclassColor.GetValue();
    outValue[SETTINGCLASSIF_unclassVisible] = m_unclassVisible;
    }
