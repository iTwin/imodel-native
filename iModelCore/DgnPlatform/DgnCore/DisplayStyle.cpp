/*---------------------------------------------------------------------+
|
|   $Source: DgnCore/DisplayStyle.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/

#include <DgnPlatformInternal.h>
#include <BeXml/BeXml.h>

// static double       s_defaultHLineTransparencyThreshold             = .3;
// static RgbFactor    s_defaultZenithColor        = { .50, .80, 1.0};
// static RgbFactor    s_defaultSkyColor           = { .52, .68, 1.0};
// static RgbFactor    s_defaultNadirColor         = { .9, .9, .9};
static RgbFactor    s_defaultGroundPlaneColor   = { .6, .74, .56}; 

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------
Serialized XML format
- Note that name is no longer included; get it from the table row.
-----------------------------------------------------------------------------------------------------------------------------------------------------
                                            Required?   Default     Comments
<DisplayStyle
    Usages=""                                           00          Serialized bit mask of usages (e.g. view and clip volume)
    >
    <Flags
        DisplayVisibleEdges=""                          0           [0|1] to enable visible edge display
        DisplayHiddenEdges=""                           0           [0|1] to enable hidden edge display
        DisplayShadows=""                               0           [0|1] to enable display of shadows
        BackgroundColor=""                              0           [0|1] to override background color
        LegacyDrawOrder=""                              0           [0|1] to enable legacy draw order (a.k.a. file-based order)
        ApplyEdgeStyleToLines=""                        0           [0|1] applies edges style to wires / open elements (8.11.7)
        IgnoreGeometryMaps=""                           0           [0|1] (8.11.9)
        IgnoreImageMaps=""                              0           [0|1] (8.11.9)
        HideInPickers=""                                0           [0|1] hides this display style in pickers, but shows in the display style dialog for modification, or usage through APIs (8.11.9)

        
        VisibleEdgeColor=""                             0           [0|1] to override edge color
        VisibleEdgeStyle=""                             1           [0|1] to override visible edge style (0 => from-element (new), 1 => solid (old))            (8.11.7)
        VisibleEdgeWeight=""                            1           [0|1] to override visible edge weight (0 => from-element (new), 1 => value (old))           (8.11.7, effectively not used 8.11.5)
        HiddenEdgeStyle=""                              0           [0|1] to override hidden edge style (0 => from-element (new), 1 => value (old))             (8.11.7)
        HiddenEdgeWeight=""                             1           [0|1] to override hidden edge weight (0 => same-as-visible-edge (old), 1 => 0 (solid, new)) (8.11.7)
        Transparency=""                                 0           [0|1] to override transpaency
        FillColor=""                                    0           [0|1] to override fill color
        LineStyle=""                                    0           [0|1] to override line style
        LineWeight=""                                   0           [0|1] to override line weight
        Material=""                                     0           [0|1] to override material
        />
    <Overrides
        DisplayMode=""                      x                       [0|3|4|6] display mode (wireframe | hidden line | filled hidden line | shaded)
        HiddenEdgeLineStyle=""                          0           [0-7] hidden edge line style
        VisibleEdgeColor=""                             0           ID of override visible edge color
        VisibleEdgeWeight=""                            0           [0-31] override line weight
        Transparency=""                                 0           [0.0-1.0] override transparency
        BackgroundColor=""                              0           ID of override background color
        FillColor=""                                    0           ID of override fill color
        LineStyle=""                                    0           [0-7] override line style
        LineWeight=""                                   0           [0-31] override line weight
        Material=""                                     0           ID of override material
        />
</DisplayStyle>
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Utf8CP   XML_ELEMENT_DisplayStyle            = "DisplayStyle";
static Utf8CP   XML_ELEMENT_Flags                   = "Flags";
static Utf8CP   XML_ELEMENT_Overrides               = "Overrides";
static Utf8CP   XML_ATTRIBUTE_Usages                = "Usages";
static Utf8CP   XML_ATTRIBUTE_DisplayMode           = "DisplayMode";
static Utf8CP   XML_ATTRIBUTE_DisplayVisibleEdges   = "DisplayVisibleEdges";
static Utf8CP   XML_ATTRIBUTE_DisplayHiddenEdges    = "DisplayHiddenEdges";
static Utf8CP   XML_ATTRIBUTE_HiddenEdgeLineStyle   = "HiddenEdgeLineStyle";
static Utf8CP   XML_ATTRIBUTE_DisplayShadows        = "DisplayShadows";
static Utf8CP   XML_ATTRIBUTE_LegacyDrawOrder       = "LegacyDrawOrder";
static Utf8CP   XML_ATTRIBUTE_ApplyEdgeStyleToLines = "ApplyEdgeStyleToLines";
static Utf8CP   XML_ATTRIBUTE_IgnoreGeometryMaps    = "IgnoreGeometryMaps";
static Utf8CP   XML_ATTRIBUTE_IgnoreImageMaps       = "IgnoreImageMaps";
static Utf8CP   XML_ATTRIBUTE_HideInPickers         = "HideInPickers";
static Utf8CP   XML_ATTRIBUTE_VisibleEdgeColor      = "VisibleEdgeColor";
static Utf8CP   XML_ATTRIBUTE_VisibleEdgeLineStyle  = "VisibleEdgeStyle";
static Utf8CP   XML_ATTRIBUTE_VisibleEdgeLineWeight = "VisibleEdgeWeight";
static Utf8CP   XML_ATTRIBUTE_HiddenEdgeLineWeight  = "HiddenEdgeWeight";
static Utf8CP   XML_ATTRIBUTE_Transparency          = "Transparency";
static Utf8CP   XML_ATTRIBUTE_BackgroundColor       = "BackgroundColor";
static Utf8CP   XML_ATTRIBUTE_Color                 = "FillColor";
static Utf8CP   XML_ATTRIBUTE_LineStyle             = "LineStyle";
static Utf8CP   XML_ATTRIBUTE_LineWeight            = "LineWeight";
static Utf8CP   XML_ATTRIBUTE_Material              = "Material";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
static ElementId remapMaterial (ElementId sourceMaterialId, DgnProjectR sourceProject, DgnProjectR destinationProject)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP_MATERIALS
    ElementId           remappedMaterialId  = 0;
    MaterialManagerR    materialMgr         = MaterialManager::GetManagerR ();
    MaterialCP          sourceMaterial      = materialMgr.FindMaterial (NULL, MaterialId (sourceMaterialId), sourceDgnFile, *sourceDgnFile.GetDictionaryModel (), true);

    if (NULL != sourceMaterial)
        {
        MaterialPtr newMaterial = Material::Create (*sourceMaterial, *destinationDgnFile.GetDictionaryModel ());
        newMaterial->GetPaletteR ().SetSource (destinationDgnFile.GetDocument ().GetMoniker ());
        newMaterial->GetPaletteR ().SetLibrary (destinationDgnFile.GetDocument ().GetMoniker ());

        if (SUCCESS == materialMgr.SaveMaterial (NULL, *newMaterial.get (), &destinationDgnFile))
            remappedMaterialId = materialMgr.FindCachedIdFromMaterialNameAndSource (newMaterial->GetName ().c_str (), destinationDgnFile, destinationDgnFile);
        }

    return remappedMaterialId;
#else
    return ElementId();
#endif
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
static bool getXmlAttributeValueAsBoolean (BeXmlNodeR xmlElement, Utf8CP attributeName, bool defaultValue)
    {
    bool val;
    if (BEXML_Success != xmlElement.GetAttributeBooleanValue (val, attributeName))
        return defaultValue;

    return (val ? true : false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
static void addBooleanAsXmlAttribute (BeXmlNodeR xmlElement, Utf8CP attributeName, bool boolValue, bool defaultValue)
    {
    if (TO_BOOL (defaultValue) != TO_BOOL (boolValue))
        xmlElement.AddAttributeBooleanValue (attributeName, TO_BOOL (boolValue));
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleFlags::DisplayStyleFlags ()
    {
    memset (this, 0, sizeof (*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleFlags::~DisplayStyleFlags ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
bool DisplayStyleFlags::Equals (DisplayStyleFlagsCR rhs) const
    {
    return
        (
        (m_displayMode              == rhs.m_displayMode)               &&
        (m_displayVisibleEdges      == rhs.m_displayVisibleEdges)       &&
        (m_displayHiddenEdges       == rhs.m_displayHiddenEdges)        &&
        (m_hiddenEdgeLineStyle      == rhs.m_hiddenEdgeLineStyle)       &&
        (m_displayShadows           == rhs.m_displayShadows)            &&
        (m_legacyDrawOrder          == rhs.m_legacyDrawOrder)           &&
        (m_overrideBackgroundColor  == rhs.m_overrideBackgroundColor)   &&
        (m_applyEdgeStyleToLines    == rhs.m_applyEdgeStyleToLines)     &&
        (m_ignoreGeometryMaps       == rhs.m_ignoreGeometryMaps)        &&
        (m_ignoreImageMaps          == rhs.m_ignoreImageMaps)           &&
        m_invisibleToCamera         == rhs.m_invisibleToCamera          &&
        m_displayGroundPlane        == rhs.m_displayGroundPlane         &&
        (m_hideInPickers            == rhs.m_hideInPickers)
        );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
ViewDisplayOverrides::ViewDisplayOverrides ()
    {
    memset (this, 0, sizeof (*this));

#ifdef DGNV10FORMAT_CHANGES_WIP
    m_hLineTransparencyThreshold = s_defaultHLineTransparencyThreshold;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
ViewDisplayOverrides::~ViewDisplayOverrides ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
static BentleyStatus importElementColor (DgnProjectR targetProject, UInt32& newColorIndex, UInt32 sourceColorIndex, DgnColors const& sourceColorTable)
    {
    if (COLOR_BYLEVEL == sourceColorIndex)
        {
        newColorIndex = COLOR_BYLEVEL;
        return SUCCESS;
        }
    
    if (COLOR_BYCELL == sourceColorIndex)
        {
        newColorIndex = COLOR_BYCELL;
        return SUCCESS;
        }
    
    IntColorDef sourceColorDef;
    Utf8String  bookName;
    Utf8String  colorName;
    bool        isTrueColor;
    
    if (SUCCESS != sourceColorTable.Extract (&sourceColorDef, NULL, &isTrueColor, &bookName, &colorName, sourceColorIndex))
        { 
        BeAssert (false); 
        return ERROR; 
        }
    
    if (isTrueColor)
        {
        newColorIndex = targetProject.Colors().CreateElementColor (sourceColorDef, bookName.c_str (), colorName.c_str ());
        return SUCCESS;
        }
    
    DgnColorMapCP ourColorMap = targetProject.Colors().GetDgnColorMap();
    if (NULL == ourColorMap)
        { 
        BeAssert (false); 
        return ERROR; 
        }
    
    newColorIndex = ourColorMap->FindClosestMatch (sourceColorDef, NULL);
    return SUCCESS;    
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
ViewDisplayOverrides ViewDisplayOverrides::Clone (DgnProjectR sourceProject, DgnProjectR destinationProject) const
    {
    ViewDisplayOverrides clonedOverrides = *this;

    if (&sourceProject != &destinationProject)
        {
        DgnColors& sourceProjectColorTable = sourceProject.Colors();

        if (SUCCESS != importElementColor (destinationProject, clonedOverrides.m_visibleEdgeColor, m_visibleEdgeColor, sourceProjectColorTable))
            { BeAssert (false); }
        
        if (SUCCESS != importElementColor (destinationProject, clonedOverrides.m_backgroundColor, m_backgroundColor, sourceProjectColorTable))
            { BeAssert (false); }
        
        if (SUCCESS != importElementColor (destinationProject, clonedOverrides.m_elementColor, m_elementColor, sourceProjectColorTable))
            { BeAssert (false); }
        
        clonedOverrides.m_material = remapMaterial (m_material, sourceProject, destinationProject);
        }
    
    return clonedOverrides;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
bool ViewDisplayOverrides::Equals (ViewDisplayOverridesCR rhs) const
    {
    return
        (
        (m_flags.m_visibleEdgeColor     == rhs.m_flags.m_visibleEdgeColor)  &&
        (m_flags.m_visibleEdgeWeight    == rhs.m_flags.m_visibleEdgeWeight) &&
        (m_flags.m_useTransparency      == rhs.m_flags.m_useTransparency)   &&
        (m_flags.m_elementColor         == rhs.m_flags.m_elementColor)      &&
        (m_flags.m_lineStyle            == rhs.m_flags.m_lineStyle)         &&
        (m_flags.m_lineWeight           == rhs.m_flags.m_lineWeight)        &&
        (m_flags.m_material             == rhs.m_flags.m_material)          &&
        (m_flags.m_visibleEdgeStyle     == rhs.m_flags.m_visibleEdgeStyle)  &&
        (m_flags.m_hiddenEdgeStyle      == rhs.m_flags.m_hiddenEdgeStyle)   &&
        (m_flags.m_hiddenEdgeWeightZero == rhs.m_flags.m_hiddenEdgeWeightZero)  &&

        (m_visibleEdgeColor             == rhs.m_visibleEdgeColor)          &&
        (m_visibleEdgeWeight            == rhs.m_visibleEdgeWeight)         &&
        (m_hiddenEdgeWeight             == rhs.m_hiddenEdgeWeight)          && 
        (m_transparency                 == rhs.m_transparency)              &&
        (m_backgroundColor              == rhs.m_backgroundColor)           &&
        (m_elementColor                 == rhs.m_elementColor)              &&
        (m_lineStyle                    == rhs.m_lineStyle)                 &&
        (m_lineWeight                   == rhs.m_lineWeight)                &&
        (m_material                     == rhs.m_material)
        );
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&    DisplayStyleGroundPlane::GetGroundColor         () const                    { return m_color; }
void                DisplayStyleGroundPlane::SetGroundColor         (RgbFactor const& color)    { m_color = color; }
double              DisplayStyleGroundPlane::GetHeight              () const                    { return m_height; }
void                DisplayStyleGroundPlane::SetHeight              (double height)             { m_height = height; }
double              DisplayStyleGroundPlane::GetTransparency        () const                    { return m_transparency; }
void                DisplayStyleGroundPlane::SetTransparency        (double transparency)       { m_transparency = transparency; }
bool                DisplayStyleGroundPlane::ShowGroundFromBelow    () const                    { return m_showGroundFromBelow; }
void                DisplayStyleGroundPlane::SetShowGroundFromBelow (bool showGroundFromBelow)  { m_showGroundFromBelow = showGroundFromBelow; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleGroundPlane::DisplayStyleGroundPlane ()
    {
    m_color                 = s_defaultGroundPlaneColor;
    m_height                = 0.0;
    m_transparency          = 0.0;
    m_showGroundFromBelow   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyleGroundPlane::Clone (DisplayStyleGroundPlane const& rhs)
    {
    m_color                 = rhs.m_color;
    m_height                = rhs.m_height;
    m_transparency          = rhs.m_transparency;
    m_showGroundFromBelow   = rhs.m_showGroundFromBelow;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool valuesEqual (double value1, double value2)
    {
    return fabs (value1 - value2) < 1.0E-4;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool colorsEqual (const RgbFactor& color1, const RgbFactor& color2)
    {
    return valuesEqual (color1.red,   color2.red) &&
           valuesEqual (color1.green, color2.green) &&
           valuesEqual (color1.blue,  color2.blue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayStyleGroundPlane::Equals (DisplayStyleGroundPlane const& rhs) const
    {
    if (m_showGroundFromBelow == rhs.m_showGroundFromBelow && 
        colorsEqual (m_color, rhs.m_color) && 
        valuesEqual (m_height, rhs.m_height) && 
        valuesEqual (m_transparency, rhs.m_transparency))
        {
        return true;
        }
    
    return false;
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyleId                  DisplayStyle::GetId                         () const                        { return m_id; }
void                        DisplayStyle::SetId                         (DgnStyleId value)              { m_id = value; }
WStringCR                   DisplayStyle::GetName                       () const                        { return m_name; }
void                        DisplayStyle::SetName                       (WCharCP value)                 { m_name = value; }
DisplayStyleSource          DisplayStyle::GetSource                     () const                        { return m_source; }
bool                        DisplayStyle::IsFromFile                    () const                        { return (DisplayStyleSource::File == m_source); }
bool                        DisplayStyle::IsFromHardCodedDefault        () const                        { return (DisplayStyleSource::HardCodedDefault == m_source); }
void                        DisplayStyle::SetSource                     (DisplayStyleSource value)      { m_source = value; }
DgnProjectR                 DisplayStyle::GetProjectR                   () const                        { return m_project; }
DisplayStyleFlagsCR         DisplayStyle::GetFlags                      () const                        { return m_flags; }
DisplayStyleFlagsR          DisplayStyle::GetFlagsR                     ()                              { return m_flags; }
void                        DisplayStyle::SetFlags                      (DisplayStyleFlagsCR value)     { m_flags = value; }
ViewDisplayOverridesCR      DisplayStyle::GetOverrides                  () const                        { return m_overrides; }
ViewDisplayOverridesR       DisplayStyle::GetOverridesR                 ()                              { return m_overrides; }
void                        DisplayStyle::SetOverrides                  (ViewDisplayOverridesCR value)  { m_overrides = value; }
BitMask const&              DisplayStyle::GetUsages                     () const                        { return *m_usages; }
BitMask&                    DisplayStyle::GetUsagesR                    ()                              { return *m_usages; }
bool                        DisplayStyle::IsUsableForViews              () const                        { return m_usages->Test ((UInt32)DisplayStyleBuiltInUsage::View); }
bool                        DisplayStyle::IsUsableForClipVolumes        () const                        { return m_usages->Test ((UInt32)DisplayStyleBuiltInUsage::ClipVolume); }
void                        DisplayStyle::SetUsages                     (BitMask const& value)          { m_usages = value; }
void                        DisplayStyle::SetIsUsableForViews           (bool value)                    { m_usages->SetBit ((UInt32)DisplayStyleBuiltInUsage::View, value); }
void                        DisplayStyle::SetIsUsableForClipVolumes     (bool value)                    { m_usages->SetBit ((UInt32)DisplayStyleBuiltInUsage::ClipVolume, value); }
WStringCR                   DisplayStyle::GetEnvironmentName            () const                        { return m_environmentName; }
void                        DisplayStyle::SetEnvironmentName            (WCharCP environmentName)       { m_environmentName = environmentName; }
EnvironmentDisplay          DisplayStyle::GetEnvironmentTypeDisplayed   () const                        { return m_environmentTypeDisplayed; }
void                        DisplayStyle::SetEnvironmentTypeDisplayed   (EnvironmentDisplay typeDisplayed) { m_environmentTypeDisplayed = typeDisplayed; }
DisplayStyleGroundPlaneCR   DisplayStyle::GetGroundPlane                () const                        { return m_groundPlane; }
DisplayStyleGroundPlaneP    DisplayStyle::GetGroundPlaneP               ()                              { return &m_groundPlane; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyle::DisplayStyle (WCharCP name, DgnProjectR project) :
    m_name      (name),
    m_source    (DisplayStyleSource::File),
    m_project   (project),
    m_usages    (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStylePtr DisplayStyle::CreateFromXMLString (WCharCP name, Utf8CP xmlString, DgnProjectR project)
    {
    if (Utf8String::IsNullOrEmpty (xmlString))
        { BeAssert (false); return NULL; }
    
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString);
    if (xmlDom.IsNull () || (BEXML_Success != xmlStatus))
        { BeAssert (false); return NULL; }

    // Only display mode is required; all others can assume defaults.
    BeXmlNodeP rootElement = xmlDom->GetRootElement ();
    if (NULL == rootElement)
        { BeAssert (false); return NULL; }

    BeXmlNodeP overrideElement = rootElement->SelectSingleNode (XML_ELEMENT_Overrides);
    if (NULL == overrideElement)
        { BeAssert (false); return NULL; }

    UInt32 displayMode;
    if (BEXML_Success != overrideElement->GetAttributeUInt32Value (displayMode, XML_ATTRIBUTE_DisplayMode))
        { 
        BeAssert (false); 
        return NULL; 
        }
    
    // Safe to allocate the display style object.
    DisplayStyleP displayStyle = new DisplayStyle (name, project);

    // Flags element
    BeXmlNodeP flagElement = rootElement->SelectSingleNode (XML_ELEMENT_Flags);
    if (NULL != flagElement)
        {
        displayStyle->m_flags.m_displayVisibleEdges             = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_DisplayVisibleEdges,    false);
        displayStyle->m_flags.m_displayHiddenEdges              = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_DisplayHiddenEdges,     false);
        displayStyle->m_flags.m_displayShadows                  = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_DisplayShadows,         false);
        displayStyle->m_flags.m_overrideBackgroundColor         = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_BackgroundColor,        false);
        displayStyle->m_flags.m_legacyDrawOrder                 = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_LegacyDrawOrder,        false);
        displayStyle->m_flags.m_applyEdgeStyleToLines           = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_ApplyEdgeStyleToLines,  false); // 8.11.7
        displayStyle->m_flags.m_ignoreGeometryMaps              = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_IgnoreGeometryMaps,     true);  // 8.11.9
        displayStyle->m_flags.m_ignoreImageMaps                 = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_IgnoreImageMaps,        false); // 8.11.9
        displayStyle->m_flags.m_hideInPickers                   = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_HideInPickers,          false); // 8.11.9
    
        displayStyle->m_overrides.m_flags.m_visibleEdgeColor    = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_VisibleEdgeColor,       false);
        displayStyle->m_overrides.m_flags.m_visibleEdgeWeight   = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_VisibleEdgeLineWeight,  true);  // 8.11.7
        displayStyle->m_overrides.m_flags.m_hiddenEdgeStyle     = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_HiddenEdgeLineStyle,    true);  // 8.11.7
        displayStyle->m_overrides.m_flags.m_hiddenEdgeWeightZero= getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_HiddenEdgeLineWeight,   false);
        displayStyle->m_overrides.m_flags.m_useTransparency     = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_Transparency,           false);
        displayStyle->m_overrides.m_flags.m_elementColor        = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_Color,                  false);
        displayStyle->m_overrides.m_flags.m_lineStyle           = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_LineStyle,              false);
        displayStyle->m_overrides.m_flags.m_lineWeight          = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_LineWeight,             false);
        displayStyle->m_overrides.m_flags.m_material            = getXmlAttributeValueAsBoolean (*flagElement, XML_ATTRIBUTE_Material,               false);
        }

    // Overrides element
    displayStyle->m_flags.m_displayMode = displayMode;
            
    UInt32 hiddenEdgeLineStyle;
    if (BEXML_Success == overrideElement->GetAttributeUInt32Value (hiddenEdgeLineStyle, XML_ATTRIBUTE_HiddenEdgeLineStyle))
        displayStyle->GetFlagsR ().m_hiddenEdgeLineStyle = hiddenEdgeLineStyle;
        
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_visibleEdgeColor,     XML_ATTRIBUTE_VisibleEdgeColor);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_visibleEdgeWeight,    XML_ATTRIBUTE_VisibleEdgeLineWeight);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_hiddenEdgeWeight,     XML_ATTRIBUTE_HiddenEdgeLineWeight);
    overrideElement->GetAttributeDoubleValue (displayStyle->m_overrides.m_transparency,         XML_ATTRIBUTE_Transparency);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_backgroundColor,      XML_ATTRIBUTE_BackgroundColor);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_elementColor,         XML_ATTRIBUTE_Color);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_lineStyle,            XML_ATTRIBUTE_LineStyle);
    overrideElement->GetAttributeUInt32Value (displayStyle->m_overrides.m_lineWeight,           XML_ATTRIBUTE_LineWeight);

    UInt64 val;
    overrideElement->GetAttributeUInt64Value (val,  XML_ATTRIBUTE_Material);
    displayStyle->m_overrides.m_material = ElementId(val);

    // Usages element
    WString usagesAttributeValue;
    if (BEXML_Success == rootElement->GetAttributeStringValue (usagesAttributeValue, XML_ATTRIBUTE_Usages))
        {
        Utf8String str(usagesAttributeValue);
        displayStyle->m_usages->SetFromString (str.c_str (), 0, -1);
        }

    return displayStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStylePtr DisplayStyle::CreateHardCodedDefault (MSRenderMode displayMode, DgnProjectR project)
    {
    Utf8String name;
    
    switch (displayMode)
        {
        case MSRenderMode::Wireframe:     name = DgnCoreL10N::GetString(DgnCoreL10N::IDS_DISPLAYMODE_WIREFRAME);        BeAssert (name.length () > 0); break;
        case MSRenderMode::HiddenLine:    name = DgnCoreL10N::GetString(DgnCoreL10N::IDS_DISPLAYMODE_HIDDENLINE);       BeAssert (name.length () > 0); break;
        case MSRenderMode::SolidFill:     name = DgnCoreL10N::GetString(DgnCoreL10N::IDS_DISPLAYMODE_FILLEDHIDDENLINE); BeAssert (name.length () > 0); break;
        case MSRenderMode::SmoothShade:   name = DgnCoreL10N::GetString(DgnCoreL10N::IDS_DISPLAYMODE_SMOOTH);           BeAssert (name.length () > 0); break;
        
        default:
            BeAssert (false);
            return NULL;
        }
    
    DisplayStyleP displayStyle = new DisplayStyle (WString(name.c_str(), BentleyCharEncoding::Utf8).c_str(), project);
    
    displayStyle->GetFlagsR ().m_displayMode = static_cast<UInt32>(displayMode);
    displayStyle->SetSource (DisplayStyleSource::HardCodedDefault);
    displayStyle->SetIsUsableForViews (true);
    
    return displayStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
Utf8String DisplayStyle::CreateXMLString () const
    {
    Utf8String xmlString;

    BeXmlDomPtr xmlDom      = BeXmlDom::CreateEmpty ();
    BeXmlNodeP  rootElement = xmlDom->AddNewElement (XML_ELEMENT_DisplayStyle, NULL, NULL);

    // Flags element
    BeXmlNodeP flagElement = rootElement->AddEmptyElement (XML_ELEMENT_Flags);
    if (NULL == flagElement)
        { BeAssert (false); return xmlString; }

    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_DisplayVisibleEdges,     m_flags.m_displayVisibleEdges,            false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_DisplayHiddenEdges,      m_flags.m_displayHiddenEdges,             false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_DisplayShadows,          m_flags.m_displayShadows,                 false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_BackgroundColor,         m_flags.m_overrideBackgroundColor,        false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_LegacyDrawOrder,         m_flags.m_legacyDrawOrder,                false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_ApplyEdgeStyleToLines,   m_flags.m_applyEdgeStyleToLines,          false); // 8.11.7
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_IgnoreGeometryMaps,      m_flags.m_ignoreGeometryMaps,             true);  // 8.11.9
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_IgnoreImageMaps,         m_flags.m_ignoreImageMaps,                false); // 8.11.9
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_HideInPickers,           m_flags.m_hideInPickers,                  false); // 8.11.9

    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_VisibleEdgeColor,        m_overrides.m_flags.m_visibleEdgeColor,   false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_VisibleEdgeLineStyle,    m_overrides.m_flags.m_visibleEdgeStyle,   true);  // 8.11.7
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_VisibleEdgeLineWeight,   m_overrides.m_flags.m_visibleEdgeWeight,  true);  // 8.11.7 (effectively not used in 8.11.5)
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_HiddenEdgeLineStyle,     m_overrides.m_flags.m_hiddenEdgeStyle,    true);  // 8.11.7
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_HiddenEdgeLineWeight,    m_overrides.m_flags.m_hiddenEdgeWeightZero,   false); // 8.11.7
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_Transparency,            m_overrides.m_flags.m_useTransparency,    false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_Color,                   m_overrides.m_flags.m_elementColor,       false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_LineStyle,               m_overrides.m_flags.m_lineStyle,          false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_LineWeight,              m_overrides.m_flags.m_lineWeight,         false);
    addBooleanAsXmlAttribute (*flagElement,    XML_ATTRIBUTE_Material,                m_overrides.m_flags.m_material,           false);

    // Overrides element
    BeXmlNodeP overridesElement = rootElement->AddEmptyElement (XML_ELEMENT_Overrides);
    if (NULL == overridesElement)
        { BeAssert (false); return xmlString; }

    UInt32 displayMode = m_flags.m_displayMode;
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_DisplayMode, displayMode);

    UInt32 hiddenEdgeLineStyle = m_flags.m_hiddenEdgeLineStyle;
    if (0 != hiddenEdgeLineStyle)
        overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_HiddenEdgeLineStyle, hiddenEdgeLineStyle);

    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_VisibleEdgeColor,      m_overrides.m_visibleEdgeColor);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_VisibleEdgeLineWeight, m_overrides.m_visibleEdgeWeight);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_HiddenEdgeLineWeight,  m_overrides.m_hiddenEdgeWeight);
    overridesElement->AddAttributeDoubleValue (XML_ATTRIBUTE_Transparency,          m_overrides.m_transparency);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_BackgroundColor,       m_overrides.m_backgroundColor);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_Color,                 m_overrides.m_elementColor);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_LineStyle,             m_overrides.m_lineStyle);
    overridesElement->AddAttributeUInt32Value (XML_ATTRIBUTE_LineWeight,            m_overrides.m_lineWeight);
    overridesElement->AddAttributeUInt64Value (XML_ATTRIBUTE_Material,              m_overrides.m_material.GetValueUnchecked());

    // Usages element
    //  To allow for future growth, attempt to find an acceptable upper bound for the string. The bit mask generates a delimited series of ON bit indices; it also uses ranges of on bits. Therefore, the worst-case scenario is every-other bit turned on.
    UInt32 numUsagesMaskValidBits = m_usages->GetCapacity ();
    if (numUsagesMaskValidBits > 0)
        {
        UInt32  usagesMaskSizeLog10     = std::max<UInt32> (1, (UInt32)ceil (log10 ((double)numUsagesMaskValidBits)));
        UInt32  usagesMaskStringLength  = 1;
        
        for (UInt32 i = 0; i < usagesMaskSizeLog10; i++)
            usagesMaskStringLength += ((i + 2) * (5 * (UInt32)pow (10.0, (double)i)));
        
        Utf8String usagesMaskString;
        m_usages->ToString (usagesMaskString, 0);

        rootElement->AddAttributeStringValue (XML_ATTRIBUTE_Usages, WString(usagesMaskString.c_str(), true).c_str ());
        }
    
    if (BEXML_Success != rootElement->GetXmlString (xmlString))
        BeAssert (false);

    return xmlString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStylePtr DisplayStyle::Create (WCharCP name, DgnProjectR project)
    {
    return new DisplayStyle (name, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStylePtr DisplayStyle::Clone () const
    {
    return Clone (m_project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStylePtr DisplayStyle::Clone (DgnProjectR destinationProject) const
    {
    DisplayStyleP rhs = new DisplayStyle (m_name.c_str (), destinationProject);
    
    rhs->SetId (m_id);
    rhs->SetSource (m_source);
    rhs->SetFlags (m_flags);
    rhs->SetOverrides (m_overrides.Clone (m_project, destinationProject));
    rhs->SetUsages (*m_usages);
    

#ifdef NEEDS_WORK_PAUL_CHATER
    if (GetEnvironmentName ().length () > 0 && destinationDgnFile.GetMSDgnFileP () != GetFile ().GetMSDgnFileP () && DisplayStyleSource::File == GetSource ())
        mdlLxo_importEnvironmentSetup (destinationDgnFile, GetEnvironmentName ().c_str ());
#endif

    rhs->SetEnvironmentName (GetEnvironmentName ().c_str ());
    rhs->SetEnvironmentTypeDisplayed (GetEnvironmentTypeDisplayed ());
    *rhs->GetGroundPlaneP () = GetGroundPlane (); 

    return rhs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
bool DisplayStyle::Equals (DisplayStyleCR rhs, bool testSettingsOnly) const
    {
    if (!testSettingsOnly)
        {
        if (
            (rhs.m_id       != m_id)        ||
            (rhs.m_source   != m_source)    ||
            (&rhs.m_project != &m_project)
            )
            return false;
        }
    
    if (
        (0 != BeStringUtilities::Wcsicmp (rhs.m_name.c_str (), m_name.c_str ()))    ||
        !rhs.m_usages->IsEqual (m_usages.get ())                                    ||
        !rhs.m_flags.Equals (m_flags)                                               ||
        !rhs.m_overrides.Equals (m_overrides)
        )
        return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
bool DisplayStyle::IsValidForViewport (ViewportCR viewport, DisplayStyleApplyValidity* applyValidity) const
    {
    // Can only apply wireframe-based display styles to sheet models.
    if (viewport.IsSheetView () && (MSRenderMode::Wireframe != m_flags.m_displayMode))
        {
        if (applyValidity)
            *applyValidity = DisplayStyleApplyValidity::NonWireframeForSheet;
        
        return false;
        }
    
    // Can not apply shadow-based display styles to 2D models.
    if (!viewport.Is3dView () && m_flags.m_displayShadows)
        {
        if (applyValidity)
            *applyValidity = DisplayStyleApplyValidity::ShadowedOn2DModel;
        
        return false;
        }

    DisplayStyleHandlerCP handler = m_overrides.GetDisplayStyleHandlerCP ();

    // Give the display style handler a chance to check against its own criteria.
    if (m_overrides.m_flags.m_useDisplayHandler &&
        NULL != handler &&
        !handler->IsValidForViewport (viewport))
        {
        if (applyValidity)
            *applyValidity = DisplayStyleApplyValidity::NotValidForHandler;

        return false;
        }


    if (applyValidity)
        *applyValidity = DisplayStyleApplyValidity::CanBeApplied;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
void DisplayStyle::ApplyTo (ViewFlagsR viewFlags) const
    {
    viewFlags.renderMode                = m_flags.m_displayMode;
    viewFlags.renderDisplayEdges        = m_flags.m_displayVisibleEdges;           
    viewFlags.renderDisplayHidden       = m_flags.m_displayHiddenEdges;      
    viewFlags.hiddenLineStyle           = m_flags.m_hiddenEdgeLineStyle;
    viewFlags.renderDisplayShadows      = m_flags.m_displayShadows;          
    viewFlags.overrideBackground           = m_flags.m_overrideBackgroundColor;
    viewFlags.inhibitRenderMaterials    = (m_overrides.m_flags.m_material && (m_overrides.m_material.IsValid()));
    viewFlags.textureMaps               = !GetFlags().m_ignoreImageMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
void DisplayStyle::ApplyTo (ViewportR vp) const
    {
    PhysicalViewControllerP viewController = vp.GetPhysicalViewControllerP();
    if (NULL == viewController)
        return;

    ApplyTo (*viewController);
    vp.SynchWithViewController (true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
void DisplayStyle::ApplyTo (PhysicalViewControllerR viewController) const
    {
    ApplyTo (viewController.GetViewFlagsR ());
    
    if (m_flags.m_overrideBackgroundColor)
        {
        IntColorDef bgColorDef;
        if (SUCCESS == GetProjectR ().Colors ().Extract (&bgColorDef, NULL, NULL, NULL, NULL, m_overrides.m_backgroundColor))
            viewController.SetBackgroundColor (bgColorDef.m_rgb);
        }
    
    viewController.SetDisplayStyle (m_id);
    }

//=======================================================================================
// Because display styles are accessed frequently during updates, cache on read.
// @bsiclass                                                    Jeff.Marker     05/2012
//=======================================================================================
struct DisplayStyleAppData : public DbAppData
    {
    typedef bmap<DgnStyleId, DisplayStylePtr> DisplayStyleMap;
private:
    DisplayStyleMap m_displayStyles;
    virtual void _OnCleanup (BeSQLiteDbR) override {delete this;}

public:
    DisplayStyleMap& GetDisplayStyles() {return m_displayStyles;}

    static DisplayStyleAppData& GetForProject (DgnProject& project)
        {
        static DbAppData::Key   s_projectKey;
        DisplayStyleAppData* appData = static_cast<DisplayStyleAppData*>(project.AppData().Find(s_projectKey));
        if (NULL == appData)
            {
            appData = new DisplayStyleAppData();
            project.AppData().Add(s_projectKey, appData);
            }

        return *appData;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleCP DgnDisplayStyles::QueryById (DgnStyleId id) const
    {
    if (!id.IsValid())
        return  NULL;

    // Check cache first.
    DisplayStyleAppData& appData = DisplayStyleAppData::GetForProject (m_project);
    DisplayStyleAppData::DisplayStyleMap::const_iterator cachedStyleIter = appData.GetDisplayStyles().find (id);
    if (appData.GetDisplayStyles().end() != cachedStyleIter)
        return cachedStyleIter->second.get();

    // Query for it.
    DgnStyles::Style foundStyle = m_project.Styles().QueryStyleById (DgnStyleType::Display, id);
    if (!foundStyle.GetId().IsValid())
        {
        appData.GetDisplayStyles()[id] = NULL;
        return NULL;
        }

    WString foundStyleW (foundStyle.GetName(), true); // string conversion
    DisplayStylePtr displayStyle = DisplayStyle::CreateFromXMLString (foundStyleW.c_str(), (Utf8CP)foundStyle.GetData(), m_project);
    if (!displayStyle.IsValid())
        {
        BeAssert (false);
        return NULL;
        }

    displayStyle->SetId (id);
    displayStyle->SetSource (DisplayStyleSource::File);

    // Cache it for the next read.
    appData.GetDisplayStyles()[displayStyle->GetId()] = displayStyle;
    return displayStyle.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleCP DgnDisplayStyles::QueryByName (Utf8CP name) const
    {
    DgnStyleId foundStyleId = m_project.Styles().QueryStyleId (DgnStyleType::Display, name);
    if (!foundStyleId.IsValid())
        return NULL;

    // Rely on QueryStyleById for cache performance.
    return QueryById (foundStyleId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleCP DgnDisplayStyles::Insert (DisplayStyleCR originalDisplayStyle)
    {
    DisplayStylePtr displayStyle = originalDisplayStyle.Clone (m_project);

    Utf8String displayStyleData = displayStyle->CreateXMLString();
    if (displayStyleData.empty())
        {
        BeAssert (false);
        return NULL;
        }

    size_t displayStyleDataSize = (sizeof (displayStyleData[0]) * (displayStyleData.size() + 1)); // NULL terminator
    Utf8String displayStyleNameUtf8(displayStyle->GetName()); // string conversion
    DgnStyles::Style styleRow (DgnStyleId(), DgnStyleType::Display, displayStyleNameUtf8.c_str(), NULL, &displayStyleData[0], displayStyleDataSize);

    if (BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow))
        {
        BeAssert (false);
        return NULL;
        }

    displayStyle->SetId (styleRow.GetId());

    // Update cache.
    DisplayStyleAppData& appData = DisplayStyleAppData::GetForProject (m_project);
    appData.GetDisplayStyles()[displayStyle->GetId()] = displayStyle;

    return displayStyle.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
BentleyStatus DgnDisplayStyles::InsertWithId (DisplayStyleCR displayStyle)
    {
    Utf8String displayStyleData = displayStyle.CreateXMLString();
    if (displayStyleData.empty())
        {
        BeAssert (false); // Couldn't create a persistent representation of a display style.
        return ERROR;
        }

    size_t displayStyleDataSize = (sizeof (displayStyleData[0]) * (displayStyleData.size() + 1)); // NULL terminator
    Utf8String displayStyleNameUtf8(displayStyle.GetName()); // string conversion
    DgnStyles::Style styleRow (displayStyle.GetId(), DgnStyleType::Display, displayStyleNameUtf8.c_str(), NULL, &displayStyleData[0], displayStyleDataSize);

    if (BE_SQLITE_DONE != m_project.Styles().InsertStyleWithId(styleRow))
        {
        BeAssert (false);
        return ERROR;
        }

    // Update cache.
    DisplayStyleAppData& appData = DisplayStyleAppData::GetForProject (m_project);
    appData.GetDisplayStyles()[displayStyle.GetId()] = displayStyle.Clone();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DisplayStyleCP DgnDisplayStyles::Update (DisplayStyleCR displayStyle)
    {
    DisplayStyleCP existingDisplayStyle = QueryById (displayStyle.GetId());
    if (NULL == existingDisplayStyle)
        return NULL;

    Utf8String displayStyleData = displayStyle.CreateXMLString();
    if (displayStyleData.empty())
        {
        BeAssert (false);
        return NULL;
        }

    size_t displayStyleDataSize = (sizeof (displayStyleData[0]) * (displayStyleData.size() + 1)); // NULL terminator
    Utf8String displayStyleNameUtf8(displayStyle.GetName()); // string conversion
    DgnStyles::Style styleRow (displayStyle.GetId(), DgnStyleType::Display, displayStyleNameUtf8.c_str(), NULL, &displayStyleData[0], displayStyleDataSize);

    if (BE_SQLITE_DONE != m_project.Styles().UpdateStyle (styleRow))
        {
        BeAssert (false);
        return NULL;
        }

    // Update cache.
    DisplayStyleAppData& appData = DisplayStyleAppData::GetForProject (m_project);
    DisplayStyleAppData::DisplayStyleMap::iterator cachedStyleIter = appData.GetDisplayStyles().find (displayStyle.GetId());
    if (appData.GetDisplayStyles().end() != cachedStyleIter)
        {
        cachedStyleIter->second = displayStyle.Clone();
        }
    else
        {
        BeAssert (false);
        return NULL;
        }

    return appData.GetDisplayStyles()[displayStyle.GetId()].get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
void DgnDisplayStyles::Delete (DisplayStyleCR originalDisplayStyle)
    {
    DisplayStyleCP existingDisplayStyle = QueryById (originalDisplayStyle.GetId());
    if (NULL == existingDisplayStyle)
        return;

    m_project.Styles().DeleteStyle (DgnStyleType::Display, existingDisplayStyle->GetId());

    // Update cache.
    DisplayStyleAppData& appData = DisplayStyleAppData::GetForProject (m_project);
    DisplayStyleAppData::DisplayStyleMap::iterator cachedStyleIter = appData.GetDisplayStyles().find (existingDisplayStyle->GetId());
    if (appData.GetDisplayStyles().end() != cachedStyleIter)
        appData.GetDisplayStyles().erase (cachedStyleIter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnDisplayStyles::MakeIterator (DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf ("WHERE Type=%d", DgnStyleType::Display);

    switch (sortOrder)
        {
        case DgnStyleSort::None:       break;
        case DgnStyleSort::NameAsc:    queryModifierClauses += " ORDER BY Name ASC";   break;
        case DgnStyleSort::NameDsc:    queryModifierClauses += " ORDER BY Name DESC";   break;

        default:
            BeAssert (false);// && L"Unknown DgnStyleSort");
            break;
        }

    DgnStyles::Iterator it (m_project);
    it.Params().SetWhere(queryModifierClauses.c_str());
    return it;
    }
