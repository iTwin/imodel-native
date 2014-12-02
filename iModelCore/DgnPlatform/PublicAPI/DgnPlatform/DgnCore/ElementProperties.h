/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include <Bentley/RefCounted.h>
#include "PropertyContext.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup ElementProperties
/// @beginGroup

typedef RefCountedPtr<ElementPropertiesGetter> ElementPropertiesGetterPtr;
typedef RefCountedPtr<ElementPropertiesSetter> ElementPropertiesSetterPtr;

/*=================================================================================**//**
  ElementPropertiesGetter is a helper class to query element symbology and properties.
  This information is typically extracted from the "dhdr" section of the element header 
  and from user data.The following example illustrates using this class to get the 
  level and color of the supplied ElementHandle:
  \code
static void getLevelAndColor (ElementHandleCR eh, LevelId& level, UInt32& color)
    {
    ElementPropertiesGetterPtr propGetter = ElementPropertiesGetter::Create (eh);

    level = propGetter->GetLevel ();
    color = propGetter->GetColor ();
    }
  \endcode
  @see PropertyContext::QueryElementProperties IQueryProperties
  @see IAreaFillPropertiesQuery for getting information about fills and patterns.
  @note Some elements are not a single uniform color, style, weight, etc. so this class
        serves to return the "base" property value which may or may not actually be
        used by any displayed geometry. Applications should implement IQueryProperties
        if they need the complete collection of all used property values.
  @bsiclass                                                     Brien.Bastings  02/09
+===============+===============+===============+===============+===============+======*/
struct ElementPropertiesGetter : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

ElemDisplayParams   m_displayParams;

public:

DGNPLATFORM_EXPORT ElementPropertiesGetter (ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create an instance of an ElementPropertiesGetter for the purpose of querying
//! the common base property values of an element.
//! @return A reference counted pointer to an ElementPropertiesGetter.
DGNPLATFORM_EXPORT static ElementPropertiesGetterPtr Create (ElementHandleCR eh);

//! Get element color id.
DGNPLATFORM_EXPORT UInt32 GetColor () const;

//! Get element linestyle and modifiers (optional).
DGNPLATFORM_EXPORT Int32 GetLineStyle (LineStyleParams* lsParams = NULL) const;

//! Get element weight.
DGNPLATFORM_EXPORT UInt32 GetWeight () const;

//! Get element level.
DGNPLATFORM_EXPORT LevelId GetLevel () const;

//! Get element class.
DGNPLATFORM_EXPORT DgnElementClass GetElementClass () const;

//! Get element display priority (2d only).
DGNPLATFORM_EXPORT Int32 GetDisplayPriority () const;

//! Get element transparency.
DGNPLATFORM_EXPORT double GetTransparency () const;

//! Get element extrude thickness.
DGNPLATFORM_EXPORT DVec3dCP GetThickness (bool& isCapped) const;

}; // ElementPropertiesGetter

/*=================================================================================**//**
  ElementPropertiesSetter is a helper class for changing element symbology
  and properties. It provides an implementation of IEditProperties for setting
  an element's basic property values. The following example illustrates using 
  this class to set the level and color of the supplied EditElementHandle:
  \code
static bool setLevelAndColor (EditElementHandleR eeh, LevelId newLevel, UInt32 newColor)
    {
    ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();

    remapper->SetLevel (newLevel);
    remapper->SetColor (newColor);

    return remapper->Apply (eeh);
    }
  \endcode
  @see PropertyContext::EditElementProperties IEditProperties
  @see IAreaFillPropertiesEdit for applying fills and patterns.
  @note For elements like user defined cells with public children the Apply method
        also applies the changes to the child elements.
  @bsiclass                                                     Brien.Bastings  02/09
+===============+===============+===============+===============+===============+======*/
struct ElementPropertiesSetter : RefCountedBase
                                 //__PUBLISH_SECTION_END__
                                 ,IEditProperties
                                 //__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__
public:

enum TemplateIgnores
    {
    TEMPLATE_IGNORE_None            = (0),
    TEMPLATE_IGNORE_Color           = (0x01 << 0),
    TEMPLATE_IGNORE_Style           = (0x01 << 1),
    TEMPLATE_IGNORE_Weight          = (0x01 << 2),
    TEMPLATE_IGNORE_Fill            = (0x01 << 3),
    TEMPLATE_IGNORE_Material        = (0x01 << 4),
    TEMPLATE_IGNORE_ElemClass       = (0x01 << 5),
    TEMPLATE_IGNORE_Pattern         = (0x01 << 6),
    TEMPLATE_IGNORE_Level           = (0x01 << 7),
    TEMPLATE_IGNORE_Transparency    = (0x01 << 8),
    TEMPLATE_IGNORE_Priority        = (0x01 << 9),
    TEMPLATE_IGNORE_StyleModifiers  = (0x01 << 10),
    TEMPLATE_IGNORE_All             = (0xffff),
    };

private:

ElementProperties   m_propMask;
bool                m_changeAll;

bool                m_setElemColor;
bool                m_setFillColor;

UInt32              m_color;
UInt32              m_fillColor;
Int32               m_style;
UInt32              m_weight;
LevelId             m_level;
DgnElementClass     m_elmClass;
Int32               m_priority;
double              m_transparency;
DgnFontCP           m_font;

LineStyleParamsCP   m_lsParams;

double              m_thickness;
DVec3dCP            m_direction;
bool                m_isCapped;
bool                m_alwaysUseDir;

protected:

virtual ElementProperties _GetEditPropertiesMask () override {return m_propMask;}

bool IsValidBaseID (EachPropertyBaseArg& arg);

DGNPLATFORM_EXPORT virtual void _EachColorCallback (EachColorArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachWeightCallback (EachWeightArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachLevelCallback (EachLevelArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachElementClassCallback (EachElementClassArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachDisplayPriorityCallback (EachDisplayPriorityArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachTransparencyCallback (EachTransparencyArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachThicknessCallback (EachThicknessArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachFontCallback (EachFontArg& arg) override;

public:

DGNPLATFORM_EXPORT ElementPropertiesSetter ();

//! Set element properties using the supplied ElemDisplayParams.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] params The ElemDisplayParams to apply.
//! @param[in] options A mask of properties to ignore from template.
DGNPLATFORM_EXPORT static void ApplyElemDisplayParamsRestricted (EditElementHandleR eeh, ElemDisplayParamsCR params, TemplateIgnores options);

//! Set element properties to match properties of template element.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] templateEh The element to match the properties of.
//! @param[in] options A mask of properties to ignore from template.
//! @note The template element is expected to be from the same model as the element to change.
DGNPLATFORM_EXPORT static void ApplyTemplateRestricted (EditElementHandleR eeh, ElementHandleCR templateEh, TemplateIgnores options);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create an instance of an ElementPropertiesSetter for the purpose of changing
//! the common base property values of an element.
//! @return A reference counted pointer to an ElementPropertiesSetter.
DGNPLATFORM_EXPORT static ElementPropertiesSetterPtr Create ();

//! Apply the property values supplied through the various set methods to the input element.
//! @param[out] eeh The element to change the properties of.
//! @return true if the element was changed.
DGNPLATFORM_EXPORT bool Apply (EditElementHandleR eeh);

//! Set remapper object to change the colors of an element.
//! @param[in] color New color value.
//! @note For opaque filled elements changing the element color will also change
//!       the fill color so that it continues to match the outline color. The same
//!       behavior is also true for patterns/hatches.
DGNPLATFORM_EXPORT void SetColor (UInt32 color);

//! Set remapper object to change just the solid fill color of an element.
//! @param[in] fillColor New fill color value.
//! @note Will not add fill to an element that is currently un-filled. Fill colors 
//! announced to color callback using PROPSCALLBACK_FLAGS_IsBackgroundID.
//! @see IAreaFillPropertiesEdit
DGNPLATFORM_EXPORT void SetFillColor (UInt32 fillColor);

//! Set remapper object to change the linestyle of an element.
//! @param[in] style New linestyle id.
//! @param[in] lsParams Modifiers for custom linestyle (or NULL).
DGNPLATFORM_EXPORT void SetLinestyle (Int32 style, LineStyleParamsCP lsParams);

//! Set remapper object to change the weight of an element.
//! @param[in] weight New weight value.
DGNPLATFORM_EXPORT void SetWeight (UInt32 weight);

//! Set remapper object to change the level of an element.
//! @param[in] level New level id.
DGNPLATFORM_EXPORT void SetLevel (LevelId level);

//! Set remapper object to change the class type of an element.
//! @param[in] elmClass New element class (Typically just primary/construction).
DGNPLATFORM_EXPORT void SetElementClass (DgnElementClass elmClass);

//! Set remapper object to change the display priority of an element.
//! @param[in] priority New display priority value.
//! @note Only applicable for 2d elements, will be ignored by 3d elements.
DGNPLATFORM_EXPORT void SetDisplayPriority (Int32 priority);

//! Set remapper object to change the transparency of an element.
//! @param[in] transparency New transparency value, 0.0 for fully opaque.
DGNPLATFORM_EXPORT void SetTransparency (double transparency);

//! Set remapper object to change the thickness of an element. Thickness is used to
//! display planar elements as extrusions in 3d when the view is not aligned with the extrude
//! direction bvector.
//! @param[in] thickness New thickness value.
//! @param[in] direction Extrude direction, required for lines, optional for elements with well defined normal.
//! @param[in] isCapped true to display closed curves as capped solids instead of surface.
//! @param[in] alwaysUseDirection true to use direction even for elements with a well defined normal (skewed extrusions).
//! @note This property is supported by simple open and closed curves, text, and multilines.
DGNPLATFORM_EXPORT void SetThickness (double thickness, DVec3dCP direction, bool isCapped, bool alwaysUseDirection);

//! Set remapper object to change the font of an element.
//! @param[in] font the new font to use
//! @note If the element supports and has an SHX big font, it will be removed, and the provided font will be the sole font used by the element.
DGNPLATFORM_EXPORT void SetFont (DgnFontCR font);

//! Whether to change just the base property values or all property values for a specified property type.
//! Some elements like dimensions, text, and multilines store more than one color, style, etc. It is
//! also possible that no part of the element displays using the base property value.
//! @param[in]  changeAll     true to change all property values.
//! @note The default behavior is to just change the base property values, PROPSCALLBACK_FLAGS_IsBaseID.
//! @see PropsCallbackFlags 
DGNPLATFORM_EXPORT void SetChangeEntireElement (bool changeAll);

//! Assign or clear the graphic group number of the supplied element.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] gg the graphic group number to use, 0 to remove element from group.
DGNPLATFORM_EXPORT static void SetGraphicGroup (EditElementHandleR eeh, UInt32 gg);

//! Assign or clear the locked flag of the supplied element.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] locked the locked flag to use.
DGNPLATFORM_EXPORT static void SetLocked (EditElementHandleR eeh, bool locked);

//! Set element properties using the supplied ElemDisplayParams.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] params The ElemDisplayParams to apply.
DGNPLATFORM_EXPORT static void ApplyElemDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params);

//! Set element properties to match properties of the supplied template element.
//! @param[in,out] eeh The element to change the properties of.
//! @param[in] templateEh The element to match the properties of.
//! @note The template element is expected to be from the same model as the element to change.
DGNPLATFORM_EXPORT static void ApplyTemplate (EditElementHandleR eeh, ElementHandleCR templateEh);

}; // ElementPropertiesSetter

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
