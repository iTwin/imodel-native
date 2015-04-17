/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/thematicdisplay.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#if defined (NEEDS_WORK_DGNITEM)

#include    "../DgnPlatform.h"
#include    "../DgnCore/DisplayStyleHandler.h"

DGNPLATFORM_TYPEDEFS (ThematicDisplaySettings)
DGNPLATFORM_TYPEDEFS (ThematicDisplayStyleHandler)
DGNPLATFORM_TYPEDEFS (ThematicDisplayViewFlags)
DGNPLATFORM_TYPEDEFS (ThematicLegend)
DGNPLATFORM_TYPEDEFS (ThematicMeshDisplayStyleHandler)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector <int32_t>            ThematicMeshIndexArray;
typedef bvector <double>           ThematicMeshDoubleArray;
typedef bvector <DPoint3d>         ThematicMeshPointArray;

typedef uint16_t                       ThematicDisplayMode;

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
+===============+===============+===============+===============+===============+======*/
struct ThematicDisplayStyleHandlerKey : public DisplayStyleHandlerKey
{
private:
    unsigned    m_cacheOnDisplayModeChange:1;
    unsigned    m_accurateSteppedDisplay:1;
    unsigned    m_edgeDisplayOverride:2;
    unsigned    m_fixedMaximum:1;
    unsigned    m_fixedMinimum:1;
    unsigned    m_isolinesSteppedDisplay:1;
    unsigned    m_flatShading:1;
    unsigned    m_transparentMarginContribution:1;
    int32_t     m_displayStyleIndex;
    uint16_t    m_displayModeContribution;

protected:
    DGNPLATFORM_EXPORT ThematicDisplayStyleHandlerKey (ThematicDisplaySettingsCR settings, ThematicDisplayStyleHandlerCR handler);

public:
    DGNPLATFORM_EXPORT static  DisplayStyleHandlerKeyPtr    Create (ThematicDisplaySettingsCR settings, ThematicDisplayStyleHandlerCR handler) { return new ThematicDisplayStyleHandlerKey (settings, handler); }
    DGNPLATFORM_EXPORT virtual bool                         Matches (DisplayStyleHandlerKey const& other) const override;
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011   
*  Base class for display style handlers that use color to thematically represent an 
*  scalar property.  This class would typically  not be extended directly.                                                                          
+===============+===============+===============+===============+===============+======*/
struct      ThematicDisplayStyleHandler  :  DisplayStyleHandler
{
private:
    void    _DrawLegendForView (ViewContextR context, DgnViewportR vp, ThematicDisplaySettingsR settings) const;
protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Format a "raw" value to a string.  "Raw" values are the unformatted, unscaled
*   values extracted directly and most efficiently from elements.
* @param[in]    value   Extracted value
* @param[in]    settings   
* @param[in]    modelRef
* @param[in]    includeUnits - true to include units in string.
* @return the formatted string. 
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual WString _GetStringFromRawValue (double value, ThematicDisplaySettingsCR, DgnModelR modelRef, bool includeUnits) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Extract a "raw" value from a string.  "Raw" values are the unformatted, unscaled
*   values extracted directly and most efficiently from elements.
* @param[out]   value   Extracted value
* @param[in]    settings   
* @param[in]    string
* @param[in]    modelRef 
* @return SUCCESS if the value is extracted successfully.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual StatusInt _GetRawValueFromString (double& value, ThematicDisplaySettingsCR settings, WStringCR string, DgnModelR modelRef) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Initialize default settings  (optional).
* @param[out]  settings.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void   _InitDefaultSettings (ThematicDisplaySettingsR settings) const { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Get display mode. - Optional, necessary only if a single handler has more than 
*  one display mode.
* @param[out]   label  Translatable string identifying this display mode.
* @param[in]    mode   
* @param[in]    modelRef 
* @return   SUCCESS if the display mode is valid.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const { return ERROR; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Return true if the thematic key needs to be changed when the display mode is changed.
*  The solar handlers, for example, don't need need this because the actual display
*  won't change, but the slope handler will because the display does change.
*  - Optional, necessary only if a single handler has more than one display mode.
* @return   true if the thematic key needs to be changed when the display mode is changed.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _CacheOnDisplayModeChange () const { return false; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  If this method returns true then the user will be allowed to define value range. 
* param[in]  settings.
* @return  true if fixed range supported.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _FixedRangeSupported (ThematicDisplaySettingsR settings) const { return true; }


    DGNPLATFORM_EXPORT virtual StatusInt                        _GetHitInfoStringFromRawValue (WStringR string, double value, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const;
    DGNPLATFORM_EXPORT virtual StatusInt                        _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const { return ERROR; }
    DGNPLATFORM_EXPORT virtual struct IViewHandlerHitInfo*      _GetViewHandlerHitInfo (DisplayStyleHandlerSettingsCP settings, DPoint3dCR hitPoint) const override { return NULL; } // NEEDS_WORK_HITINFO
    DGNPLATFORM_EXPORT virtual bool                             _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override = 0;
    DGNPLATFORM_EXPORT virtual void                             _DrawLegend (ViewContextR viewContext) const;

    DGNPLATFORM_EXPORT virtual DisplayStyleHandlerSettingsPtr   _GetSettings () const override;
    DGNPLATFORM_EXPORT virtual DisplayStyleHandlerKeyPtr       _GetCacheKey (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const override;
    DGNPLATFORM_EXPORT virtual void                             _OnFrustumChange (DisplayStyleHandlerSettingsR settings, ViewContextR viewContext, DgnModelR modelRef) const override;
    DGNPLATFORM_EXPORT virtual void                             _CookRange (ThematicDisplaySettingsR settings, DgnViewportR viewport, DgnModelR modelRef) const; 
    DGNPLATFORM_EXPORT virtual bool                             _IsValidForDisplayStyle (DisplayStyleCR style) const override;
    DGNPLATFORM_EXPORT virtual bool                             _IsPeriodic () const { return false; }
    DGNPLATFORM_EXPORT virtual bool                             _SupportsPointColors () const { return false; }
    DGNPLATFORM_EXPORT virtual StatusInt                        _GetPointColors (RgbColorDef* colors, DPoint3dCP points, int nPoints, ThematicDisplaySettingsCR settings, ViewContextR contest) const { return ERROR; }

public:
    static void                                                 RegisterHandlers ();
    void                                                        InitDefaultSettings (ThematicDisplaySettingsR settings) const;
    StatusInt                                                   GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const { return _GetHitInfoString (string, hitPath, hitPoint, settings, viewport, el, delimiter); }
    StatusInt                                                   GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const { return _GetDisplayMode (label, mode, model); }
    bool                                                        CacheOnDisplayModeChange () const { return _CacheOnDisplayModeChange(); }
    virtual void                                                CookRange (ThematicDisplaySettingsR settings, DgnViewportR viewport, DgnModelR modelRef) const          { _CookRange (settings, viewport, modelRef); }
    StatusInt                                                   GetRawValueFromString (double& value,  ThematicDisplaySettingsCR  settings, WStringCR string, DgnModelR modelRef) const  { return _GetRawValueFromString (value, settings, string, modelRef); }
    WString                                                     GetStringFromRawValue (double value,  ThematicDisplaySettingsCR  settings, DgnModelR modelRef,  bool includeUnits = true) const    { return _GetStringFromRawValue (value, settings, modelRef, includeUnits); }
    bool                                                        FixedRangeSupported (ThematicDisplaySettingsR settings) const { return _FixedRangeSupported (settings); }
    DGNPLATFORM_EXPORT WString                                  GetStringFromNormalizedValue (double value, ThematicDisplaySettingsCR  settings, DgnModelR modelRef, bool includeUnits = true) const;
    DGNPLATFORM_EXPORT void                                     DrawLegend (ViewContextR context, double maxHeight, bool exactHeight, uint8_t* transparency)  const;
    DGNPLATFORM_EXPORT void                                     ComputeLabelsAndSize (ThematicDisplaySettingsCR settings, double maxHeight, DgnModelR modelRef, bvector <WString>* outLabels, double* outMaxChars, size_t* outNValues, bool* outMaxHeightExceeded) const;
    DGNPLATFORM_EXPORT static bool                              NeverApplyThematic (DisplayHandlerCP handler);
    DGNPLATFORM_EXPORT bool                                     IsPeriodic () const { return _IsPeriodic (); }
    DGNPLATFORM_EXPORT bool                                     SupportsPointColors () const { return _SupportsPointColors(); }
    DGNPLATFORM_EXPORT virtual StatusInt                        GetPointColors (RgbColorDef* colors, DPoint3dCP points, int nPoints, ThematicDisplaySettingsCR settings, ViewContextR context) const { return _GetPointColors (colors, points, nPoints, settings, context); }

};   // ThematicDisplayStyleHandler    



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
*  Base class for display style handlers that use color to thematically represent a
*  scalar element property.
* @see ThematicDisplayStyleHandler, DisplayStyleHandler
+===============+===============+===============+===============+===============+======*/
struct      ThematicElementPropertyDisplayHandler : ThematicDisplayStyleHandler
{
    
protected:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Get thematic value for an element.  If this method returns true then the element
*   will be displayed in a color determined by this value and the current ThematicDisplaySettings.
*  @param[out]  value   
*  @param[in]   el
*  @param[in]   viewport
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool                                        _GetElementValue (double& value, ElementHandleCR el, DgnViewportR viewport) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Return the value range for this model.  This range will be used to map the legend 
*
*  colors unless a fixed range is specified by the user.
* @param[out]  min
* @param[out]  max
* @param[in]   modelRef
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool                                        _GetModelRange (double& min, double& max, DgnModelR modelRef) const = 0;

DGNPLATFORM_EXPORT virtual bool                     _IsValidForViewport (DgnViewportCR viewport) const override;
DGNPLATFORM_EXPORT virtual void                     _CookRange (ThematicDisplaySettingsR settings, DgnViewportR viewport, DgnModelR modelRef) const;
DGNPLATFORM_EXPORT virtual bool                     _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override;
DGNPLATFORM_EXPORT virtual StatusInt                _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const;

};



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct      IThematicMeshStroker
{
    virtual void                    _StrokeThematicMesh (int polySize, size_t nIndices, int32_t const* pointIndices, int32_t* valueIndices, size_t nPoints, DPoint3dCP points, double const* values, bool peroidic, double periodicMin, double periodicMax, ViewContextR context) = 0;

};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct      ThematicMeshDisplayStyleHandler  :  ThematicDisplayStyleHandler
{

protected:
    virtual StatusInt                     _ProcessMesh (IThematicMeshStroker& meshStroker, ViewContextR context, int polySize, size_t nIndices, int32_t const* pointIndices, int32_t const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool isTwoSided, ThematicDisplaySettingsCR settings) const = 0;
    DGNPLATFORM_EXPORT virtual StatusInt  _GetHitInfoStringFromFacet (WStringR string, double value, size_t facetIndex, size_t facetNumber, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const;
    DGNPLATFORM_EXPORT virtual StatusInt  _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const;

public:
    StatusInt                             ProcessMesh (IThematicMeshStroker& stroker, ViewContextR context, int polySize, size_t nIndices, int32_t const* pointIndices, int32_t const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool twoSided, ThematicDisplaySettingsCR settings) const { return _ProcessMesh (stroker, context, polySize, nIndices, pointIndices, normalIndices, nPoints, points, normals, twoSided, settings); }
    static DGNPLATFORM_EXPORT WString     GetStringFromUors (double uors, DgnModelR modelRef, bool includeUnits);
    static DGNPLATFORM_EXPORT StatusInt   GetUorsFromString (double& uors, WStringCR string, DgnModelR modelRef); 
    static DGNPLATFORM_EXPORT StatusInt   CalculateMeshNormals (ThematicMeshPointArray& normals, ThematicMeshDoubleArray* areas, ThematicMeshIndexArray& normalIndices, int polySize, size_t nIndices, int32_t const*  pointIndices, DPoint3dCP points);
 
};  // ThematicMeshDisplayStyleHandler

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct      GeometricThematicMeshDisplayStyleHandler : ThematicMeshDisplayStyleHandler
{
    bool        m_stroked;
    bool        m_curvedGeometryStroked;

protected:                          
                                        GeometricThematicMeshDisplayStyleHandler() { }
    DGNPLATFORM_EXPORT virtual bool     _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override;
    DGNPLATFORM_EXPORT virtual bool     _StrokeForCache (CachedDrawHandleCR dh, ViewContextR viewContext, IStrokeForCache& stroker, double pixelSize) const override;
                  virtual void          _OverrideSizeDependence (double& sizeDependentRatio) const override          { sizeDependentRatio = 3.0; }


};  //      GeometricThematicMeshDisplayStyleHandler


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
*   Base class for a display style handler that thematically displays mesh data with 
*   an associated (non-geometric) scalar value.  
+===============+===============+===============+===============+===============+======*/
struct      ThematicMeshValueDisplayStyleHandler  : ThematicMeshDisplayStyleHandler
{

protected:
                  virtual bool      _EmbeddedDataPresent (ElementHandleCR el) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return a mesh with scalar values for thematic display of the input element.
*   @param[in]  el           
*   @param[out] polySize        positive value (3) if fixed size mesh. else 0 for variable sized.
*   @param[out] pointIndices    point indices. 1 based indices into points array with 0 seperators.
*   @param[out] valueIndices    value indices.  1 based indices into values array with 0 seperators.
*   @param[out] points          vertex locations.
*   @param[out] values          normalized (0-1) values.  
*   @param[in]  settings.     
*   @return  true if mesh is provided.  false to display element normally.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool      _GetMesh 
(
ElementHandleCR                el, 
int&                        polySize, 
ThematicMeshIndexArray&     pointIndices, 
ThematicMeshIndexArray&     valueIndices, 
ThematicMeshPointArray&     points, 
ThematicMeshDoubleArray&    values, 
ThematicDisplaySettingsCR   settings
) const = 0;

                   virtual bool      _IsValidForViewport (DgnViewportCR viewport) const override= 0;
DGNPLATFORM_EXPORT virtual StatusInt _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el, WCharCP delimiter) const override;
DGNPLATFORM_EXPORT virtual bool      _DrawElement (ElementHandleCR el, ViewContextR viewContext) const override;


public:
                    bool            GetMesh (ElementHandleCR el, int& polySize, ThematicMeshIndexArray& pointIndices, ThematicMeshIndexArray& valueIndices, ThematicMeshPointArray& points, ThematicMeshDoubleArray& values, ThematicDisplaySettingsCR settings) const
                                      { return _GetMesh (el, polySize, pointIndices, valueIndices, points, values, settings); }

                    
};  //      ThematicMeshValueDisplayStyleHandler


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011   
*  A Display Style Handler base classes that produces a thematic colored mesh based
*   on a property based on only mesh vertex location.

+===============+===============+===============+===============+===============+======*/
struct     PointThematicMeshDisplayStyleHandler  :  GeometricThematicMeshDisplayStyleHandler
{
protected:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return a normalized value based on the input point.  Note the "cooked" range
*   must be initialized correctly in the _cookRange method.
* 
* @param[in]    point location in root model UORs.
* @param[in]    settings
* @return  Raw thematic value  that will be used to determine thematic color.
* @see  _GetCookedRange
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double  _GetPointValue (DPoint3dCR point, ThematicDisplaySettingsCR settings) const = 0;

    virtual StatusInt                       _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR settings, DgnViewportR viewport, ElementHandleR el,  WCharCP delimiter) const override = 0;
    virtual bool                            _IsValidForViewport (DgnViewportCR viewport) const override = 0;

    virtual bool                            _SupportsPointColors () const override { return true; }
    virtual DGNPLATFORM_EXPORT StatusInt    _GetPointColors (RgbColorDef* colors, DPoint3dCP points, int nPoints, ThematicDisplaySettingsCR settings, ViewContextR context) const override;

DGNPLATFORM_EXPORT virtual StatusInt        _ProcessMesh (IThematicMeshStroker& meshStroker, ViewContextR context, int polySize, size_t nIndices, int32_t const* pointIndices, int32_t const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool twoSided, ThematicDisplaySettingsCR settings) const override;

};  // PointThematicMeshDisplayStyleHandler

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct     NormalThematicMeshDisplayStyleHandler  :  GeometricThematicMeshDisplayStyleHandler
{
protected:
    virtual double                          _GetNormalValue (DVec3dCR normal, bool twoSided, ThematicDisplaySettingsCR settings) const = 0;
    virtual bool                            _IsValidForViewport (DgnViewportCR viewport) const override = 0;;
    virtual double                          _GetMinValue (ThematicDisplayMode) const = 0;
    virtual double                          _GetMaxValue (ThematicDisplayMode) const = 0;

DGNPLATFORM_EXPORT virtual StatusInt       _ProcessMesh (IThematicMeshStroker& meshStroker, ViewContextR context, int polySize, size_t nIndices, int32_t const* pointIndices, int32_t const* normalIndices, size_t nPoints, DPoint3dCP points, DVec3dCP normals, bool twoSided, ThematicDisplaySettingsCR settings) const override;
};  // PointThematicMeshDisplayStyleHandler


/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   09/2010
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
* @bsiclass                                                     BrandonBohrer   08/2011
* This is based on GradientKey in igdsext.h, but we created a new struct instead of
* including that file so this header can be used from verticals more easily.
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
* @bsiclass                                                     BrandonBohrer   09/2010
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
enum    ThematicEdgeDisplayOverride
    {
    ThematicEdgeOverrideDisplay_None           = 0,
    ThematicEdgeOverrideDisplay_AllOn          = 1,
    ThematicEdgeOverrideDisplay_AllOff         = 2,
    };

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

                    ThematicCookedRange ()                                      { m_minimum = 0.0; m_delta = 1.0; }
    void            Set (double min, double max)                                { m_minimum = min, m_delta =  (max > min) ? (max - min) : 1.0; }
    void            Set (ThematicRange const& range)                            { Set (range.GetMin(), range.GetMax()); }
    inline double   GetNormalizedValueFromRaw (double value)  const             { return (value - m_minimum) / (m_delta); }
    inline double   GetRawValueFromNormalized (double normalizedValue) const    { return m_minimum + normalizedValue * (m_delta); }

 };  // ThematicCookedRange

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
    RgbColorDef             m_color;
    ThematicLegendKeyFlags  m_flags;
 };  // ThematicLegendKey

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   12/2010
*   Used to represent the legend for a thematic display handler.
+===============+===============+===============+===============+===============+======*/
struct  ThematicLegend
 {
 private:
    bvector <ThematicLegendKey>                     m_keys;
    ThematicDisplaySettingsP                        m_parent;
    
    DGNPLATFORM_EXPORT size_t                       _OrderIndex (size_t i) const;

 public:
                                                    ThematicLegend () : m_keys (), m_parent (NULL) {}
                                                    ThematicLegend (ThematicDisplaySettingsP parent) : m_keys (), m_parent (parent) {}
                                                    ThematicLegend (bvector <ThematicLegendKey> keys, ThematicDisplaySettingsP parent) : m_keys (keys), m_parent (parent) {}

    DGNPLATFORM_EXPORT bvector <double> const&      GetAllValues (bool ordered) const;
    DGNPLATFORM_EXPORT bvector <RgbColorDef> const& GetAllColors (bool ordered) const;
    DGNPLATFORM_EXPORT bvector <bool> const&        GetAllVisible (bool ordered) const;
    DGNPLATFORM_EXPORT size_t                       GetNEntries () const;

    DGNPLATFORM_EXPORT RgbColorDef const&           GetRgbColor (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT double                       GetMinValue (size_t i, bool ordered) const;
    uint32_t                                        GetIntColor (size_t i, bool ordered) const                       { RgbColorDef color = GetRgbColor (i, ordered); return color.red + (color.green << 8) + (color.blue << 16); }
    DGNPLATFORM_EXPORT void                         SetRgbColor (size_t i, RgbColorDef const& color, bool ordered);
    DGNPLATFORM_EXPORT void                         SetMinValue (size_t i, double value, bool ordered);               
    DGNPLATFORM_EXPORT double                       GetMaxValue (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT void                         SetMaxValue (size_t i, double value, bool ordered);
    DGNPLATFORM_EXPORT bool                         GetVisible (size_t i, bool ordered) const;
    DGNPLATFORM_EXPORT void                         SetVisible (size_t i, bool visible, bool ordered);
    // Returns ONLY the stored data. Will NOT return a valid legend if the range is not fixed or m_keys has not been initialized.
    bvector <ThematicLegendKey> const&              GetInternalKeys () const                                        { return m_keys; }
    DGNPLATFORM_EXPORT bool                         IsStatic () const;
    bool                                            Equals (ThematicLegendCR rhs) const                             { return rhs.m_keys.size () == m_keys.size () && (0 == m_keys.size () || 0 == memcmp (&m_keys[0], &rhs.m_keys[0], m_keys.size () * sizeof (m_keys[0]))); }

    static HandlerR                                 GetElementHandler (); 

    DGNPLATFORM_EXPORT static StatusInt             CreateElement (EditElementHandleR eh, DPoint3dCR origin, double width, double height, RotMatrixCR rotation, DgnModelP modelRef);

 };


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
* @bsiclass                                                     RayBentley      10/2010                         
+===============+===============+===============+===============+===============+======*/
struct ThematicMeshColorMap 
{
    bvector<uint32_t>         m_colors;

    void        Init (ThematicDisplaySettingsCR settings, size_t nColors);
    uint32_t    Get (double value, double min, double max) const;
    void        Get (RgbColorDef& color, double value);

};  // ThematicMeshColorMap;

#define     THEMATIC_TEXTURE_SIZE       1024

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   12/2011
+===============+===============+===============+===============+===============+======*/
enum ThematicError
{
    ThematicError_ImportHandlerMismatch    = 0x7fff,
    ThematicError_ImportInconsistentValues = 0x7ffe,
    ThematicError_ImportOpenFailed         = 0x7ffd,
    ThematicError_Success                  = 0x7ffc,
};  // ThematicError

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct	ThematicDisplaySettings : DisplayStyleHandlerSettings
{                             
    // Directly persisted.
    ThematicDisplaySettingsData                 m_data;

    // Indirectly persisted.
    bvector <ThematicGradientKey>               m_gradientKeys;
    ThematicLegend                              m_legend;
    uint32_t                                    m_marginColor;
    uint32_t                                    m_legendTransparency;
    double                                      m_legendValueStep;

    // Not persisted.
    MaterialPtr                                 m_pMaterial;
    ThematicCookedRange                         m_cookedRange;
    ThematicMeshColorMap                        m_colorMap;
    uint32_t                                    m_texturePixels[THEMATIC_TEXTURE_SIZE];
    DgnModelP                                m_frustumModel;

    static int32_t const                          s_defaultNLegendEntries = 10;
    static uint32_t const                         s_defaultMarginColor = 0x003f3f3f;

    DGNPLATFORM_EXPORT                          ThematicDisplaySettings ();
    DGNPLATFORM_EXPORT                          ThematicDisplaySettings (ThematicDisplaySettingsCR other);

    DGNPLATFORM_EXPORT virtual StatusInt        _Read (DgnElementP DgnElementP, int styleIndex) override;
    DGNPLATFORM_EXPORT virtual StatusInt        _Save (DgnElementP DgnElementP, int styleIndex) override;

    DGNPLATFORM_EXPORT                          ~ThematicDisplaySettings ();
    ThematicColorScheme                         GetColorScheme () const                                     { return m_data.m_colorScheme; }
    DGNPLATFORM_EXPORT void                     SetColorScheme (ThematicColorScheme scheme);
    ThematicDisplayMode                         GetDisplayMode () const                                     { return m_data.m_mode; }
    void                                        SetDisplayMode (ThematicDisplayMode Mode)                   { m_data.m_mode = Mode; }
    ThematicDisplayViewFlagsCR                  GetFlags () const                                           { return m_data.m_flags; }
    ThematicDisplayViewFlagsR                   GetFlagsR ()                                                { return m_data.m_flags; }
    void                                        SetFlags (ThematicDisplayViewFlagsCR flags)                 { m_data.m_flags = flags; }
    MaterialCP                                  GetMaterial (ThematicDisplayStyleHandlerCR handler, ViewContextR);
    DGNPLATFORM_EXPORT void                     ClearMaterial () { m_pMaterial = NULL;  }   // Needs work... Free qv material and texture? 
    DGNPLATFORM_EXPORT void                     GetGradient (ThematicGradientKey* gradientKeys, size_t maxKeys, size_t* outNKeys) const;
    DGNPLATFORM_EXPORT void                     SetGradient (ThematicGradientKey* gradientKeys, size_t maxKeys);
    DGNPLATFORM_EXPORT void                     GetRange (double& min, double& max) const;
    DGNPLATFORM_EXPORT ThematicRange const&     GetRange () const;
    void                                        GetRawRange (double&min, double& max) const                 { m_data.m_range.Get (min, max); }
    void                                        SetRange (double min, double max)                           { m_data.m_range.Set (min, max); }
    void                                        SetRangeMin (double min)                                    { m_data.m_range.m_minimum = min; }
    void                                        SetRangeMax (double max)                                    { m_data.m_range.m_maximum = max; } 
    double                                      GetNormalizedValueFromRaw (double value) const              { return m_cookedRange.GetNormalizedValueFromRaw (value); }
    double                                      GetRawValueFromNormalized (double normalizedValue) const    { return m_cookedRange.GetRawValueFromNormalized (normalizedValue); }
    bool                                        IsMinFixed () const                                         { return m_data.m_flags.m_fixedMinimum; }
    bool                                        IsMaxFixed () const                                         { return m_data.m_flags.m_fixedMaximum; }
    void                                        SetFixedRange (double min, double max)                      { m_data.m_range.Set (min, max); m_data.m_flags.m_fixedMinimum = m_data.m_flags.m_fixedMaximum = true; }   
    uint32_t                                    GetMarginColor () const                                     { return m_marginColor; }
    void                                        SetMarginColor (uint32_t color)                               { m_marginColor = color; }
    void                                        SetMarginColorRGB (RgbColorDef color)                       { m_marginColor = color.red + (color.green << 8) + (color.green << 16); }
    DGNPLATFORM_EXPORT uint32_t                 GetColor (double value, double min=0.0, double max=1.0) const;
    DGNPLATFORM_EXPORT void                     GetColor (RgbColorDef& color, double rawValue) const;
    DGNPLATFORM_EXPORT uint32_t                 GetLegendColor (double value) const;
    void                                        ResizeLegend (uint32_t nLegendEntries)                        { m_data.m_flags.m_nLegendEntries = nLegendEntries;  RefreshLegend (); }
    void                                        ResizeLegendByStep (double step)                            { SetLegendValueStep (step); RefreshLegend (); }
    bool                                        MaterialOverrideRequired () const                           { return ThematicSteppedDisplay_Isolines != GetFlags().m_steppedDisplay; }
    DGNPLATFORM_EXPORT void                     RefreshLegend ();
    uint32_t                                    GetSeedNLegendEntries () const                              { return (0 != m_data.m_flags.m_nLegendEntries) ? m_data.m_flags.m_nLegendEntries : s_defaultNLegendEntries; }
    uint32_t                                    GetLegendOpacity () const                                   { return 255 - m_legendTransparency; }
    void                                        SetLegendOpacity (uint32_t opacity)                           { m_legendTransparency = 255 - opacity; }
    uint32_t                                    GetLegendTransparency () const                              { return m_legendTransparency; }
    void                                        SetLegendTransparency (uint32_t transparency)                 { m_legendTransparency = transparency; }

    ThematicLegendCR                            GetLegend () const                                          { return m_legend;   }
    ThematicLegendR                             GetLegendR ()                                               { return m_legend;   }
    void                                        SetLegend (ThematicLegendCR legend)                         { m_legend = legend; }
    ThematicError                               ReadDescartes (WCharCP filename, DisplayStyleHandlerCP handler);
    void                                        WriteDescartes (WCharCP  filename, DisplayStyleHandlerCP handler);
    double                                      GetLegendValueStep () const                                 { double min, max; GetRange (min, max); return m_legendValueStep > 0.0 ? m_legendValueStep : (max - min) / GetSeedNLegendEntries (); }
    void                                        SetLegendValueStep (double legendValueStep)                 { m_legendValueStep = legendValueStep; }
    DgnModelP                                GetFrustumModel () const                                    { return m_frustumModel; }
    static DisplayStyleHandlerSettingsPtr       Create ()                                                   { return new ThematicDisplaySettings (); }
    virtual DisplayStyleHandlerSettingsPtr      _Clone() const override                                     { return new ThematicDisplaySettings (*this); }
    DGNPLATFORM_EXPORT virtual bool             _Equals (DisplayStyleHandlerSettingsCP rhs) const override;

};   // ThematicDisplaySettings




/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2010
+===============+===============+===============+===============+===============+======*/
struct ThematicLegendElementData
{
    ThematicLegendElementData () { }
    ThematicLegendElementData (DPoint3dCR origin, double width, double height, RotMatrixCR rotation) { memset (this, 0, sizeof (*this)); m_origin = origin; m_width = width; m_height = height; m_rotation = rotation;}
    
    DPoint3d            m_origin;
    double              m_width;
    double              m_height;
    RotMatrix           m_rotation;
    double              m_unused[8];

    struct
        {
        unsigned        m_unused1:32;
        unsigned        m_unused2:32;
        }               m_flags;
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011
+===============+===============+===============+===============+===============+======*/
struct HillShadeDisplaySettingsData
{
    double  m_azimuth;
    double  m_altitude;
};  //  HillShadeDisplaySettingsData


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      03/2011
+===============+===============+===============+===============+===============+======*/
struct HillShadeDisplaySettings : ThematicDisplaySettings
{
    HillShadeDisplaySettingsData    m_data;

    DVec3d                          m_direction;

                    HillShadeDisplaySettings ();

    virtual StatusInt       _Save (DgnElementP DgnElementP, int styleIndex) override;
    virtual StatusInt       _Read (DgnElementP DgnElementP, int styleIndex) override;
    virtual bool            _Equals (DisplayStyleHandlerSettingsCP rhs) const override;


    double          GetAzimuthAngle () const        { return m_data.m_azimuth; }
    double          GetAltitudeAngle () const            { return m_data.m_altitude; }

    void            SetAzimuthAngle (double angle)  { m_data.m_azimuth  = angle; }
    void            SetAltitudeAngle (double angle) { m_data.m_altitude = angle; }

    static DisplayStyleHandlerSettingsPtr Create ()  { return new HillShadeDisplaySettings (); }

};  // HillShadeDisplaySettings

DisplayHandlerP thematicDisplay_getLegendElementHandler ();


/*=================================================================================**//**
*   Standard Thematic Handlers
+===============+===============+===============+===============+===============+======*/

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011                                                                              
+===============+===============+===============+===============+===============+======*/
struct      HeightDisplayHandler  :  PointThematicMeshDisplayStyleHandler
{
    
    virtual WString                     _GetName () const override;
    virtual XAttributeHandlerId         _GetHandlerId () const override; 
    virtual bool                        _IsValidForViewport (DgnViewportCR viewport) const override;
    virtual double                      _GetPointValue (DPoint3dCR point, ThematicDisplaySettingsCR settings) const override;
    virtual StatusInt                   _GetDisplayMode (WStringR label, ThematicDisplayMode mode, DgnModelR model) const override;
    virtual bool                        _CacheOnDisplayModeChange () const override { return true; }
    virtual DisplayStyleHandlerKeyPtr   _GetCacheKey  (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const override;
    virtual void                        _CookRange (ThematicDisplaySettingsR settings, DgnViewportR viewport, DgnModelR modelRef) const override;
    virtual StatusInt                   _GetRawValueFromString (double& value, ThematicDisplaySettingsCR settings, WStringCR string, DgnModelR modelRef) const  override;
    virtual WString                     _GetStringFromRawValue (double value, ThematicDisplaySettingsCR settings, DgnModelR modelRef, bool includeUnits) const override;                   
    virtual StatusInt                   _GetHitInfoString (WStringR string, HitPathCR hitPath, DPoint3dCR hitPoint, ThematicDisplaySettingsCR   settings, DgnViewportR viewport, ElementHandleR el,  WCharCP delimiter) const override;

};  // HeightDisplayHandler


END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif














