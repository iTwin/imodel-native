/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementProperties.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include <Bentley/RefCounted.h>
#include "PropertyContext.h"

DGNPLATFORM_REF_COUNTED_PTR (ElementPropertiesGetter);
DGNPLATFORM_REF_COUNTED_PTR (ElementPropertiesSetter);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup ElementProperties
/// @beginGroup

/*=================================================================================**//**
  ElementPropertiesGetter is a helper class to query element symbology and properties.
  This information is typically extracted from the "dhdr" section of the element header 
  and from user data.The following example illustrates using this class to get the 
  category and color of the supplied ElementHandle:
  \code
static void getCategoryAndColor (ElementHandleCR eh, DgnCategoryId& category, uint32_t& color)
    {
    ElementPropertiesGetterPtr propGetter = ElementPropertiesGetter::Create (eh);

    category = propGetter->GetCategory ();
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
DGNPLATFORM_EXPORT ColorDef GetColor () const;

//! Get element linestyle and modifiers (optional).
DGNPLATFORM_EXPORT int32_t GetLineStyle (LineStyleParams* lsParams = NULL) const;

//! Get element weight.
DGNPLATFORM_EXPORT uint32_t GetWeight () const;

//! Get element category.
DGNPLATFORM_EXPORT DgnCategoryId GetCategory () const;

//! Get element display priority (2d only).
DGNPLATFORM_EXPORT int32_t GetDisplayPriority () const;

//! Get element transparency.
DGNPLATFORM_EXPORT double GetTransparency () const;

}; // ElementPropertiesGetter

/*=================================================================================**//**
  ElementPropertiesSetter is a helper class for changing element symbology
  and properties. It provides an implementation of IEditProperties for setting
  an element's basic property values. The following example illustrates using 
  this class to set the category and color of the supplied EditElementHandle:
  \code
static bool setCategoryAndColor (EditElementHandleR eeh, DgnCategoryId newCategory, uint32_t newColor)
    {
    ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();

    remapper->SetCategory (newCategory);
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
    TEMPLATE_IGNORE_Category        = (0x01 << 7),
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

ColorDef            m_color;
ColorDef            m_fillColor;
int32_t             m_style;
uint32_t            m_weight;
DgnCategoryId       m_category;
int32_t             m_priority;
double              m_transparency;
DgnFontCP           m_font;

LineStyleParamsCP   m_lsParams;

protected:

virtual ElementProperties _GetEditPropertiesMask () override {return m_propMask;}

bool IsValidBaseID (EachPropertyBaseArg& arg);

DGNPLATFORM_EXPORT virtual void _EachColorCallback (EachColorArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachWeightCallback (EachWeightArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachCategoryCallback (EachCategoryArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachDisplayPriorityCallback (EachDisplayPriorityArg& arg) override;
DGNPLATFORM_EXPORT virtual void _EachTransparencyCallback (EachTransparencyArg& arg) override;
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
DGNPLATFORM_EXPORT void SetColor (ColorDef color);

//! Set remapper object to change just the solid fill color of an element.
//! @param[in] fillColor New fill color value.
//! @note Will not add fill to an element that is currently un-filled. Fill colors 
//! announced to color callback using PROPSCALLBACK_FLAGS_IsBackgroundID.
//! @see IAreaFillPropertiesEdit
DGNPLATFORM_EXPORT void SetFillColor (ColorDef fillColor);

//! Set remapper object to change the linestyle of an element.
//! @param[in] style New linestyle id.
//! @param[in] lsParams Modifiers for custom linestyle (or NULL).
DGNPLATFORM_EXPORT void SetLinestyle (int32_t style, LineStyleParamsCP lsParams);

//! Set remapper object to change the weight of an element.
//! @param[in] weight New weight value.
DGNPLATFORM_EXPORT void SetWeight (uint32_t weight);

//! Set remapper object to change the category of an element.
//! @param[in] category New category id.
DGNPLATFORM_EXPORT void SetCategory (DgnCategoryId category);

//! Set remapper object to change the display priority of an element.
//! @param[in] priority New display priority value.
//! @note Only applicable for 2d elements, will be ignored by 3d elements.
DGNPLATFORM_EXPORT void SetDisplayPriority (int32_t priority);

//! Set remapper object to change the transparency of an element.
//! @param[in] transparency New transparency value, 0.0 for fully opaque.
DGNPLATFORM_EXPORT void SetTransparency (double transparency);

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
DGNPLATFORM_EXPORT static void SetGraphicGroup (EditElementHandleR eeh, uint32_t gg);

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
