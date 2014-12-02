/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IMaterialProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    <DgnPlatform/DgnCore/Handler.h> // For MissingHandlerPermissions
#include    <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup ElementProperties
/// @beginGroup


typedef bpair<WString, DgnMaterialId>  MaterialSubEntityAttachmentPair;
typedef bvector<MaterialSubEntityAttachmentPair>   MaterialSubEntityAttachments;

//=======================================================================================
//! Provides methods for inspecting the current material properties of an element.
//! Implemented by Element handlers that support material association.
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IMaterialPropertiesExtension : Handler::Extension
{
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__
protected:

virtual MaterialCP _FindMaterialAttachment (ElementHandleCR eh) const {return NULL;}

virtual BentleyStatus _AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id) {return ERROR;}

virtual bool _SupportsSubEntityAttachments (ElementHandleCR eh) const { return false;}

virtual bool _HasSubEntityAttachments (ElementHandleCR eh) const { return false;}

virtual BentleyStatus _GetSubEntityAttachments (ElementHandleCR eh, MaterialSubEntityAttachments& attachments) const {return ERROR;}

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS(IMaterialPropertiesExtension, DGNPLATFORM_EXPORT)

/*---------------------------------------------------------------------------------**//**
* Query whether the element has a material attached to it.  Since a material attachment has the
* highest priority for being associated with an element, this does not query whether a material is
* associated with the element in any other manner. This pointer is valid only as long as the element's
* design file remains loaded and the material is not modified by another caller.
* @param[in] eh The element to query.
* @param[in] renderDgnModel The model that this material will be used with - needed to determine the scaling for maps that make use of real world units.
* @return The material associated with the element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT MaterialCP FindMaterialAttachment (ElementHandleCR eh) const;

/*---------------------------------------------------------------------------------**//**
* Query whether the element has a material associated with it.  This will resolve the element's
* effective material, based on the priority of material assignments, attachments, overrides, etc.
* This pointer is valid only as long as the element's design file remains loaded and the material
* is not modified by another caller.
* @param[in] eh The element to query.
* @param[in] level The effective level of the element.  Because of overrides, this may differ from the level declared in the ElementHandle.
* @param[in] colorIndex The effective color of the element.  Because of overrides, this may differ from the color declared in the ElementHandle.
* @param[in] useSymbologyOverride Specifies whether the lookup will search for an override material or a by-level material.
* @return The material associated with the element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT MaterialCP FindMaterial (ElementHandleCR eh, LevelId level, UInt32 colorIndex, bool useSymbologyOverride) const;

/*---------------------------------------------------------------------------------**//**
* Associate a material with an element via a material attachment.
* @param[in] eeh The element attach the material to.
* @param[in] materialId The material associated with the attachment
* @return SUCCESS if the attachment was made
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT BentleyStatus AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id);

/*---------------------------------------------------------------------------------**//**
* Query whether the element has the ability to associate multiple materials with itself
* @param[in] eh The element to query.
* @return   true if multiple materials can be associated with this element, false otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool SupportsSubEntityAttachments (ElementHandleCR eh) const;

/*---------------------------------------------------------------------------------**//**
* Query if this element has multiple materials associated with it
* @param[in] eh The element to query.
* @return  true if there are multiple materials associated with this element, false otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool HasSubEntityAttachments (ElementHandleCR eh) const;

/*---------------------------------------------------------------------------------**//**
* Fill in a map of material attachments to sub components of this element.
* @param[in]    eh              The element to query.
* @param[in]    attachments     A vector containing WString, materialId pairs of attachments to sub elements
* @return       SUCCESS if the method is succeded
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT BentleyStatus GetSubEntityAttachments (ElementHandleCR eh, MaterialSubEntityAttachments& attachments) const;
}; // IMaterialPropertiesExtension

/// @endGroup
//__PUBLISH_SECTION_END__

//=======================================================================================
//! Provides methods for inspecting the current material properties of an element.
// @bsiclass                                                    PaulChater 09/11
//=======================================================================================
struct CommonMaterialPropertiesExtension : IMaterialPropertiesExtension
{
protected :

DGNPLATFORM_EXPORT virtual MaterialCP _FindMaterialAttachment (ElementHandleCR eh) const override;

DGNPLATFORM_EXPORT virtual BentleyStatus _AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id) override;

public :
static void Register ();
}; //CommonMaterialPropertiesExtension

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
