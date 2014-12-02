/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IAreaFillProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    <DgnPlatform/DgnCore/Handler.h> // For MissingHandlerPermissions
#include    <DgnPlatform/DgnCore/AreaPattern.h>
#include    <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup ElementProperties
/// @beginGroup

/*=================================================================================**//**
* Provides methods for inspecting the current area properties of an element.
* Implemented by Element handlers that define a closed shape or region.
* @bsiclass                                                     Brien.Bastings  04/07
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IAreaFillPropertiesQuery
{
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual bool _GetAreaType (ElementHandleCR eh, bool* isHoleP) const;
DGNPLATFORM_EXPORT virtual bool _GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const;
DGNPLATFORM_EXPORT virtual bool _GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const;
DGNPLATFORM_EXPORT virtual bool _GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Query whether the area defined by a closed region represents a solid or hole.
* @param[in] eh The element to query.
* @param[out] isHoleP true if element denotes a hole type area, false for solid area.
* @return true if element supports the solid/hole property.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetAreaType (ElementHandleCR eh, bool* isHoleP) const;

/*---------------------------------------------------------------------------------**//**
* Query the current solid fill information for the supplied element.
* @param[in] eh The element to query.
* @param[out] fillColorP Color of solid fill, for an outline fill the fill color can be different than the element color.
* @param[out] alwaysFilledP true if area always displayed as filled even when fill view attribute is off.
* @return true if element currently has a solid fill.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const;

/*---------------------------------------------------------------------------------**//**
* Query the current gradient fill information for the supplied element.
* @param[in] eh The element to query.
* @param[out] symb Defintion for the gradient fill.
* @return true if element currently has a gradient fill.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const;

/*---------------------------------------------------------------------------------**//**
* Query the current hatch or area pattern information for the supplied element.
* @param[in] eh The element to query.
* @param[out] params The pattern parameters, holds information about angles, scales, pattern cell, and symbology.
* @param[out] hatchDefLinesP Information about DWG hatch definition. 
* @param[out] originP The pattern origin.
* @param[in] index Which pattern index to query (0 based).
* @return true if element is currently hatched or has an area pattern.
* @note The Multiline element is the only element type that currently supports having more than one pattern.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const;

}; // IAreaFillPropertiesQuery

/*=================================================================================**//**
* Provides methods for changing the area properties of an element.
* Implemented by Element handlers that define a closed shape or region.
* @bsiclass                                                     Brien.Bastings  04/07
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IAreaFillPropertiesEdit : public IAreaFillPropertiesQuery
{
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual bool _SetAreaType (EditElementHandleR eeh, bool isHole);
DGNPLATFORM_EXPORT virtual bool _RemoveAreaFill (EditElementHandleR eeh);
DGNPLATFORM_EXPORT virtual bool _RemovePattern (EditElementHandleR eeh, int index);
DGNPLATFORM_EXPORT virtual bool _AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP);
DGNPLATFORM_EXPORT virtual bool _AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb);
DGNPLATFORM_EXPORT virtual bool _AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Change the current solid/hole status for the supplied element.
* @param[in] eeh The element to modify.
* @param[in] isHole true to set area to hole type, false for solid.
* @return true if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool SetAreaType (EditElementHandleR eeh, bool isHole);

/*---------------------------------------------------------------------------------**//**
* Remove the current solid or gradient fill from the supplied element.
* @param[in] eeh The element to modify.
* @return true if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool RemoveAreaFill (EditElementHandleR eeh);

/*---------------------------------------------------------------------------------**//**
* Remove the current hatch or area pattern from the supplied element.
* @param[in] eeh The element to modify.
* @param[in] index which pattern to remove (0 based).
* @return true if element was updated.
* @note The Multiline element is the only element type that currently supports having more than one pattern.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool RemovePattern (EditElementHandleR eeh, int index);

/*---------------------------------------------------------------------------------**//**
* Add solid fill to the supplied element.
* @param[in] eeh The element to modify.
* @param[in] fillColorP fill color, pass NULL for an opaque fill using the same color as the edge color.
* @param[in] alwaysFilledP whether to honor the fill view attribute, pass NULL for per-view fill.
* @return true if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP = NULL, bool* alwaysFilledP = NULL);

/*---------------------------------------------------------------------------------**//**
* Add gradient fill to the supplied element.
* @param[in] eeh The element to modify.
* @param[in] symb The gradient fill settings.
* @return true if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb);

/*---------------------------------------------------------------------------------**//**
* Add a hatch or area pattern to the supplied element.
* @param[in] eeh The element to modify.
* @param[in] params The pattern settings.
* @param[in] hatchDefLinesP Settings for DWG hatch type. NOTE: Count must be set in params.GetDwgHatchDef ().nDefLines.
* @param[in] index Pattern index (only for multilines).
* @return true if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index = 0);

}; // IAreaFillPropertiesEdit

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
