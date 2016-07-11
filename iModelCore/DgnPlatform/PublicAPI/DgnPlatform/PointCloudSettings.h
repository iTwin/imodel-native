/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/PointCloudSettings.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"

#define CLASSIFICATION_COUNT    32
#define CLASSIFICATION_STATES   256

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* View settings for point clouds
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PointCloudViewSettings
    {
    enum class DisplayStyle
        {
        None           = 0,
        Intensity      = 1,
        Classification = 2,
        Location       = 3,
        Custom         = 4, //New Display style in SS4
        };

    enum {
        VIEWSETTINGS_RGB_MASK        = (0x00000001),
        VIEWSETTINGS_INTENSITY_MASK  = (0x00000002),
        VIEWSETTINGS_LIGHTNING_MASK  = (0x00000004),
        VIEWSETTINGS_PLANE_MASK      = (0x00000008),
        VIEWSETTINGS_FRONTBIAS_MASK  = (0x00000010),
        };

protected:
    uint32_t        m_flags;
    double          m_contrast;
    double          m_brightness;
    double          m_distance;
    double          m_offset;
    int32_t         m_adaptivePointSize;
    uint32_t        m_intensityRampIdx;
    uint32_t        m_planeRampIdx;
    uint32_t        m_planeAxis; //{x,y,z} index
    DisplayStyle    m_displayStyle = DisplayStyle::None;
    Utf8String      m_planeRamp;
    Utf8String      m_intensityRamp;
    bool            m_useACSAsPlaneAxis;
    bool            m_clampIntensity;
    bool            m_needClassifBuffer;
    Utf8String      m_displayStyleName;
    int32_t         m_displayStyleIndex;

public:
    static float GetDefaultViewContrast() {return 50.0;}
    static float GetDefaultViewBrightness() {return 180.0;}
    
    PointCloudViewSettings() {InitDefaults();}
    void InitDefaults();
    bool AreSetToDefault() const;

    bool GetUseRgb() const {return TO_BOOL(m_flags & VIEWSETTINGS_RGB_MASK);}
    bool GetUseIntensity() const {return TO_BOOL(m_flags & VIEWSETTINGS_INTENSITY_MASK);}
    bool GetUseLightning() const {return TO_BOOL(m_flags & VIEWSETTINGS_LIGHTNING_MASK);}
    bool GetUsePlane() const {return TO_BOOL(m_flags & VIEWSETTINGS_PLANE_MASK);}
    bool GetUseFrontBias() const {return TO_BOOL(m_flags & VIEWSETTINGS_FRONTBIAS_MASK);}
    uint32_t GetFlags() const {return m_flags;}
    void SetFlags(uint32_t flags) {m_flags = flags;}
    double GetContrast() const {return m_contrast;}
    void SetContrast(double contrast) {m_contrast = contrast;}
    double GetBrightness() const {return m_brightness;}
    void SetBrightness(double brightness){m_brightness = brightness;}
    double GetDistance() const {return m_distance;}
    void SetDistance(double distance) {m_distance = distance;}
    double GetOffset() const {return m_offset;}
    void SetOffset(double offset) {m_offset = offset;}
    int32_t GetAdaptivePointSize() const {return m_adaptivePointSize;}
    void SetAdaptivePointSize(int32_t adaptivePointSize){m_adaptivePointSize = adaptivePointSize;}
    uint32_t GetIntensityRampIdx() const {return m_intensityRampIdx;}
    void SetIntensityRampIdx(uint32_t intensityRampIdx){m_intensityRampIdx = intensityRampIdx;}
    uint32_t GetPlaneRampIdx() const {return m_planeRampIdx;}
    void SetPlaneRampIdx(uint32_t planeRampIdx){m_planeRampIdx = planeRampIdx;}
    uint32_t GetPlaneAxis() const {return m_planeAxis;}
    void SetPlaneAxis(uint32_t planeAxis){m_planeAxis = planeAxis;}
    DisplayStyle GetDisplayStyle() const {return m_displayStyle;}
    void SetDisplayStyle(DisplayStyle const& displayStyle) {m_displayStyle = displayStyle;}
    Utf8StringCR GetPlaneRamp() const {return m_planeRamp;}
    void SetPlaneRamp(Utf8StringCR planeRamp){m_planeRamp = planeRamp;}
    Utf8StringCR GetIntensityRamp() const {return m_intensityRamp;}
    void SetIntensityRamp(Utf8StringCR intensityRamp){m_intensityRamp = intensityRamp;}
    bool GetUseACSAsPlaneAxis() const {return m_useACSAsPlaneAxis;}
    void SetUseACSAsPlaneAxis(bool useACSAsPlaneAxis){m_useACSAsPlaneAxis = useACSAsPlaneAxis;}
    bool GetClampIntensity() const {return m_clampIntensity;}
    void SetClampIntensity(bool clampIntensity){m_clampIntensity = clampIntensity;}
    bool GetNeedClassifBuffer() const {return m_needClassifBuffer;}
    void SetNeedClassifBuffer(bool needClassifBuffer){m_needClassifBuffer = needClassifBuffer;}
    Utf8String GetDisplayStyleName() const {return m_displayStyleName;}
    void SetDisplayStyleName(Utf8StringCR displayStyleName){m_displayStyleName = displayStyleName;}
    int32_t GetDisplayStyleIndex() const {return m_displayStyleIndex;}
    void SetDisplayStyleIndex(int32_t displayStyleIndex){m_displayStyleIndex = displayStyleIndex;}

    void FromJson(JsonValueCR);
    void ToJson(JsonValueR val) const;
   };

/*=================================================================================**//**
* Classification view settings for point clouds
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PointCloudClassificationSettings
    {
protected:
    ColorDef        m_classificationColor[CLASSIFICATION_COUNT];
    bool            m_blendColor;
    bool            m_useBaseColor;
    bool            m_visibleState[CLASSIFICATION_STATES];
    bool            m_activeState[CLASSIFICATION_STATES];
    ColorDef        m_unclassColor;
    bool            m_unclassVisible;

public:
    PointCloudClassificationSettings() {InitDefaults();}
    DGNPLATFORM_EXPORT void InitDefaults();
    bool AreSetToDefault() const;
    bool operator==(PointCloudClassificationSettings const& rhs) const;

    ColorDefCP      GetClassificationColors() const { return m_classificationColor; }
    ColorDef        GetClassificationColor(uint32_t idx) const { return m_classificationColor[idx]; }
    void            SetClassificationColor(ColorDefCR classificationColor, uint32_t idx) 
                        { 
                        if (idx >= CLASSIFICATION_COUNT)
                            return;
                        m_classificationColor[idx] = classificationColor;
                        }
    void            SetClassificationColors(ColorDefCP classificationColors) { memcpy (m_classificationColor, classificationColors, CLASSIFICATION_COUNT * sizeof(ColorDef)); }
    bool            GetBlendColor() const { return m_blendColor; }
    void            SetBlendColor(bool blendColor) { m_blendColor = blendColor; }
    bool            GetUseBaseColor() const { return m_useBaseColor; }
    void            SetUseBaseColor(bool useBaseColor) { m_useBaseColor = useBaseColor; }
    bool const *    GetVisibleStates() const { return m_visibleState; }
    bool            GetVisibleState(uint32_t idx) const { return m_visibleState[idx]; }
    void            SetVisibleStates(bool const * visibleStates) { memcpy (m_visibleState, visibleStates, CLASSIFICATION_STATES * sizeof(bool)); }
    void            SetVisibleState(bool visibleState, uint32_t idx) 
                        { 
                        if (idx >= CLASSIFICATION_STATES)
                            return;
                        m_visibleState[idx] = visibleState;
                        }
    bool const *    GetActiveStates() const { return m_activeState; }
    bool            GetActiveState(uint32_t idx) const { return m_activeState[idx]; }
    void            SetActiveStates(bool const * activeStates) { memcpy (m_activeState, activeStates, CLASSIFICATION_STATES * sizeof(bool)); }
    void            SetActiveState(bool activeState, uint32_t idx) 
                        { 
                        if (idx >= CLASSIFICATION_STATES)
                            return;
                        m_activeState[idx] = activeState;
                        }
    ColorDef        GetUnclassColor() const { return m_unclassColor; }
    void            SetUnclassColor(ColorDefCR unclassColor) { m_unclassColor = unclassColor; }
    bool            GetUnclassVisible() const { return m_unclassVisible; }
    void            SetUnclassVisible(bool unclassVisible) { m_unclassVisible = unclassVisible; }

    void            FromJson(JsonValueCR);
    void            ToJson(JsonValueR val) const;
   };

END_BENTLEY_DGN_NAMESPACE
