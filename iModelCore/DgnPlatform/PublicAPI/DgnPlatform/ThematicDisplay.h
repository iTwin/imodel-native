/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ThematicDisplay.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__


BEGIN_BENTLEY_RENDER_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicDisplaySettings)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicDisplayStyleHandler)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicDisplayViewFlags)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicLegend)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicRange)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ThematicCookedRange)

#define     THEMATIC_TEXTURE_SIZE       8192

typedef bvector <int32_t>           ThematicMeshIndexArray;
typedef bvector <double>            ThematicMeshDoubleArray;
typedef bvector <DPoint3d>          ThematicMeshPointArray;

typedef uint16_t                    ThematicDisplayMode;


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
enum ThematicColorScheme
{
    ThematicColorScheme_BlueRed     = 0,
    ThematicColorScheme_RedBlue     = 1,
    ThematicColorScheme_Monochrome  = 2,
    ThematicColorScheme_Topographic = 3,
    ThematicColorScheme_SeaMountain = 4,
    ThematicColorScheme_Max         = 5,
    ThematicColorScheme_Custom      = 0xffff,
};  // ThematicColorScheme

    
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
enum    ThematicSteppedDisplay
    {
    ThematicSteppedDisplay_None               = 0,
    ThematicSteppedDisplay_Accurate           = 1,
    ThematicSteppedDisplay_Isolines           = 2,
    ThematicSteppedDisplay_Fast               = 3,
    ThematicSteppedDisplay_FastWithIsolines   = 4,
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct      ThematicGradientKey
    {
    double      value;
    unsigned    red:8;
    unsigned    blue:8;
    unsigned    green:8;
    unsigned    unused1:8;
    unsigned    unused2:32;
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct ThematicColorSchemeProvider
{
    ThematicColorScheme m_icse;

    DGNPLATFORM_EXPORT   ThematicColorSchemeProvider (ThematicColorScheme icse);
    
public:
    DGNPLATFORM_EXPORT   StatusInt   GradientKeys    (size_t maxKeys, size_t* fetchedKeys, ThematicGradientKey* outKeys);
    DGNPLATFORM_EXPORT   StatusInt   GradientArrays  (size_t maxKeys, size_t* fetchedKeys, double* outValues, RgbFactor* outColors);
    
    DGNPLATFORM_EXPORT   static void ArraysToKeys    (size_t nKeys, const double* values, const RgbFactor* colors, ThematicGradientKey* keys);
    DGNPLATFORM_EXPORT   static void KeysToArrays    (size_t nKeys, const ThematicGradientKey* keys, double* values, RgbFactor* colors);

private:
    static void GetGradientKeys (ThematicGradientKey* keys, size_t nKeys);

};  //  ThematicColorSchemeProvider

/*=================================================================================**//**
* @bsiclass                                                     Will.Bentley     5/10
+===============+===============+===============+===============+===============+======*/
struct  ThematicDisplayViewFlags
{
    unsigned                            m_noLegend:1;
    unsigned                            m_noSmoothing:1;
    unsigned                            m_steppedDisplay:3;
    unsigned                            m_unused:10;
    unsigned                            m_fixedMinimum:1;
    unsigned                            m_fixedMaximum:1;
    unsigned                            m_edgeDisplayOverride:2;
    unsigned                            m_valuesByStep:1;
    unsigned                            m_unused1:12;
    unsigned                            m_nLegendEntries:8;
    unsigned                            m_invertLegend:1;
    unsigned                            m_outOfRangeTransparent:1;
    unsigned                            m_customLegend:1;
    unsigned                            m_unused2:21;
    
};   // ThematicDisplayViewFlags


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
 struct  ThematicRange
 {
    double                              m_minimum;  
    double                              m_maximum;  

                    ThematicRange ()                                        { m_minimum = 0.0; m_maximum = 1.0; }
    double          GetMin () const                                         { return m_minimum; }
    double          GetMax() const                                          { return m_maximum; }
    void            SetMin (double min)                                     { m_minimum = min; }
    void            SetMax (double max)                                     { m_maximum = max; }
    void            Get (double& min, double& max) const                    { min = m_minimum; max = m_maximum; }
    void            Set (double min, double max)                            { m_minimum = min, m_maximum = (max > min) ? max : (min + 1.0); }

 };  // ThematicRange

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct  ThematicCookedRange 
 {
    double                              m_minimum;
    double                              m_delta;

                    ThematicCookedRange(ThematicRangeCR range)                  { Set(range); }
                    ThematicCookedRange(double min, double max)                 { Set(min, max); }    
    void            Set (double min, double max)                                { m_minimum = min, m_delta =  (max > min) ? (max - min) : 1.0; }
    void            Set (ThematicRange const& range)                            { Set (range.GetMin(), range.GetMax()); }
    inline double   GetNormalizedValueFromRaw (double value)  const             { return (value - m_minimum) / (m_delta); }
    inline double   GetRawValueFromNormalized (double normalizedValue) const    { return m_minimum + normalizedValue * (m_delta); }

 };  // ThematicCookedRange

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010                         
+===============+===============+===============+===============+===============+======*/
struct ThematicMeshColorMap 
{
    bvector<ColorDef>         m_colors;

    void        Init (ThematicDisplaySettingsCR settings, size_t nColors);
    ColorDef    Get (double value, double min, double max) const;
    void        Get (ColorDef& color, double value);

};  // ThematicMeshColorMap;


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
 struct  ThematicDisplaySettingsData
 {
    uint16_t                        m_version;
    uint16_t                        m_unused;
    uint16_t                        m_mode;
    ThematicColorScheme             m_colorScheme;
    ThematicDisplayViewFlags        m_flags;
    ThematicRange                   m_range;

    ThematicDisplaySettingsData () : m_version (0), m_mode (0), m_colorScheme (ThematicColorScheme_RedBlue)      { }

    // STOP!... Do not add more data to this structure.

 }; //  ThematicDisplaySettingsData


/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   12/2010
+===============+===============+===============+===============+===============+======*/
struct  ThematicLegendKeyFlags
 {
    unsigned        m_hidden:1;
    unsigned        m_unused:31;
 };  // ThematicLegendKeyFlags

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   12/2010
*   One key in a thematic legend. The value of key n is the nth minimum value and the (n-1)th
*   maximum value. Because of this the final key of a legend is special - only its value
*   is valid.
+===============+===============+===============+===============+===============+======*/
struct  ThematicLegendKey
 {
    double                  m_value;
    ColorDef                m_color;
    ThematicLegendKeyFlags  m_flags;
 };  // ThematicLegendKey

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   12/2010
+===============+===============+===============+===============+===============+======*/
struct  ThematicLegend
 {
 private:
    bvector <ThematicLegendKey>                     m_keys;
    ThematicDisplaySettingsP                        m_parent;
    
    size_t                                          OrderIndex (size_t i) const;

 public:
                                                    ThematicLegend () : m_keys (), m_parent (NULL) {}
                                                    ThematicLegend (ThematicDisplaySettingsP parent) : m_keys (), m_parent (parent) {}
                                                    ThematicLegend (bvector <ThematicLegendKey> keys, ThematicDisplaySettingsP parent) : m_keys (keys), m_parent (parent) {}

    DGNPLATFORM_EXPORT bvector <double> const&      GetAllValues (bool ordered) const;
    DGNPLATFORM_EXPORT bvector <ColorDef> const&    GetAllColors (bool ordered) const;
    DGNPLATFORM_EXPORT bvector <bool> const&        GetAllVisible (bool ordered) const;
    DGNPLATFORM_EXPORT size_t                       GetNEntries () const;

    DGNPLATFORM_EXPORT ColorDef                     GetColor (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT double                       GetMinValue (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT void                         SetColor (size_t i, ColorDef const& color, bool ordered);
    DGNPLATFORM_EXPORT void                         SetMinValue (size_t i, double value, bool ordered);               
    DGNPLATFORM_EXPORT double                       GetMaxValue (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT void                         SetMaxValue (size_t i, double value, bool ordered);
    DGNPLATFORM_EXPORT bool                         GetVisible (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT void                         SetVisible (size_t i, bool visible, bool ordered);
    // Returns ONLY the stored data. Will NOT return a valid legend if the range is not fixed or m_keys has not been initialized.
    bvector <ThematicLegendKey> const&              GetInternalKeys () const                                        { return m_keys; }
    DGNPLATFORM_EXPORT bool                         IsStatic () const;
    bool                                            Equals (ThematicLegendCR rhs) const                             { return rhs.m_keys.size () == m_keys.size () && (0 == m_keys.size () || 0 == memcmp (&m_keys[0], &rhs.m_keys[0], m_keys.size () * sizeof (m_keys[0]))); }

 };


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct	ThematicDisplaySettings 
{                           
private:  
    // Directly persisted.
    ThematicDisplaySettingsData                 m_data;

    // Indirectly persisted.
    bvector <ThematicGradientKey>               m_gradientKeys;
    ThematicLegend                              m_legend;
    ColorDef                                    m_marginColor;
    uint32_t                                    m_legendTransparency;
    double                                      m_legendValueStep;

    // Not persisted.
    MaterialPtr                                 m_pMaterial;
    ThematicMeshColorMap                        m_colorMap;
    mutable ColorDef                            m_texturePixels[THEMATIC_TEXTURE_SIZE];

    static int32_t const                        s_defaultNLegendEntries = 10;

public:
    DGNPLATFORM_EXPORT                          ThematicDisplaySettings();
    DGNPLATFORM_EXPORT                          ThematicDisplaySettings(ThematicDisplaySettingsCR other);
    ThematicColorScheme                         GetColorScheme() const                                      { return m_data.m_colorScheme; }
    DGNPLATFORM_EXPORT void                     SetColorScheme(ThematicColorScheme scheme);
    ThematicDisplayMode                         GetDisplayMode() const                                      { return m_data.m_mode; }
    void                                        SetDisplayMode(ThematicDisplayMode Mode)                    { m_data.m_mode = Mode; }
    ThematicDisplayViewFlagsCR                  GetFlags() const                                            { return m_data.m_flags; }
    ThematicDisplayViewFlagsR                   GetFlagsR()                                                 { return m_data.m_flags; }
    void                                        SetFlags(ThematicDisplayViewFlagsCR flags)                  { m_data.m_flags = flags; }
    void                                        SetSteppedDisplay(ThematicSteppedDisplay steppedDisplay)    { m_data.m_flags.m_steppedDisplay = (uint32_t) steppedDisplay; }
    MaterialCP                                  GetMaterial(ThematicDisplayStyleHandlerCR handler, ViewContextR);
    DGNPLATFORM_EXPORT void                     ClearMaterial() { m_pMaterial = NULL;  } 
    DGNPLATFORM_EXPORT void                     GetGradient(ThematicGradientKey* gradientKeys, size_t maxKeys, size_t* outNKeys) const;
    DGNPLATFORM_EXPORT void                     SetGradient(ThematicGradientKey* gradientKeys, size_t maxKeys);
    DGNPLATFORM_EXPORT void                     GetRange(double& min, double& max) const;
    DGNPLATFORM_EXPORT ThematicRange const&     GetRange() const;
    void                                        GetRawRange(double&min, double& max) const                  { m_data.m_range.Get(min, max); }
    void                                        SetRange(double min, double max)                            { m_data.m_range.Set(min, max); }
    void                                        SetRangeMin(double min)                                     { m_data.m_range.m_minimum = min; }
    void                                        SetRangeMax(double max)                                     { m_data.m_range.m_maximum = max; } 
    bool                                        IsMinFixed() const                                          { return m_data.m_flags.m_fixedMinimum; }
    bool                                        IsMaxFixed() const                                          { return m_data.m_flags.m_fixedMaximum; }
    void                                        SetFixedRange(double min, double max)                       { m_data.m_range.Set(min, max); m_data.m_flags.m_fixedMinimum = m_data.m_flags.m_fixedMaximum = true; }   
    ColorDef                                    GetMarginColor() const                                      { return m_marginColor; }
    void                                        SetMarginColor(ColorDef color)                              { m_marginColor = color; }
    DGNPLATFORM_EXPORT ColorDef                 GetColor(double value, double min=0.0, double max=1.0) const;
    DGNPLATFORM_EXPORT void                     GetColor(ColorDef& color, double rawValue) const;
    DGNPLATFORM_EXPORT ColorDef                 GetLegendColor(double value) const;
    void                                        ResizeLegend(uint32_t nLegendEntries)                       { m_data.m_flags.m_nLegendEntries = nLegendEntries;  RefreshLegend(); }
    void                                        ResizeLegendByStep(double step)                             { SetLegendValueStep(step); RefreshLegend(); }
    bool                                        MaterialOverrideRequired() const                            { return ThematicSteppedDisplay_Isolines != GetFlags().m_steppedDisplay; }
    DGNPLATFORM_EXPORT void                     RefreshLegend();
    uint32_t                                    GetSeedNLegendEntries() const                               { return(0 != m_data.m_flags.m_nLegendEntries) ? m_data.m_flags.m_nLegendEntries : s_defaultNLegendEntries; } 
    uint32_t                                    GetLegendOpacity() const                                    { return 255 - m_legendTransparency; }
    void                                        SetLegendOpacity(uint32_t opacity)                          { m_legendTransparency = 255 - opacity; }
    uint32_t                                    GetLegendTransparency() const                               { return m_legendTransparency; }
    void                                        SetLegendTransparency(uint32_t transparency)                { m_legendTransparency = transparency; }

    ThematicLegendCR                            GetLegend() const                                           { return m_legend;   }
    ThematicLegendR                             GetLegendR()                                                { return m_legend;   }
    void                                        SetLegend(ThematicLegendCR legend)                          { m_legend = legend; }
    double                                      GetLegendValueStep() const                                  { double min, max; GetRange(min, max); return m_legendValueStep > 0.0 ? m_legendValueStep :(max - min) / GetSeedNLegendEntries(); }
    void                                        SetLegendValueStep(double legendValueStep)                  { m_legendValueStep = legendValueStep; }
    Image                                       GetImage() const;

};   // ThematicDisplaySettings




END_BENTLEY_RENDER_NAMESPACE
