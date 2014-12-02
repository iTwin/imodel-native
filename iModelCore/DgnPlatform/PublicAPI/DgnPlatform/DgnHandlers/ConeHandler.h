/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ConeHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup 3DElements
/// @beginGroup

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* The default type handler for the CONE_ELM type that corresponds to the 
* Cone_3d structure. This is a 3d only element type, it has no 2d representation
* and can not be added to a 2d model.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ConeHandler : DisplayHandler, ISolidPrimitiveEdit
{
    DEFINE_T_SUPER(DisplayHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (ConeHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void        _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt   _ApplyTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

// DisplayHandler
virtual bool                           _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void        _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void        _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual void        _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;
virtual void                           _GetSnapOrigin (ElementHandleCR el, DPoint3dR origin) override {_GetTransformOrigin(el, origin);}
DGNPLATFORM_EXPORT virtual SnapStatus  _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual StatusInt   _EvaluateCustomKeypoint (ElementHandleCR elHandle, DPoint3dP outPointP, byte* customKeypointData) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;
virtual bool                           _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}

// ISolidPrimitiveEdit
DGNPLATFORM_EXPORT virtual BentleyStatus _GetSolidPrimitive (ElementHandleCR eh, ISolidPrimitivePtr& primitive) override;
DGNPLATFORM_EXPORT virtual BentleyStatus _SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive) override;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT BentleyStatus GetConeData (ElementHandleCR source, RotMatrixP rMatrix, DPoint3dP center0, DPoint3dP center1, double* r0, double* r1, bool* capped);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT BentleyStatus SetConeData (EditElementHandleR eeh, RotMatrixCR rMatrix, DPoint3dCR center0, DPoint3dCR center1, double r0, double r1, bool capped);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's top radius.
* @param[in]  eh        The element to extract from.
* @return cone top radius.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT double ExtractTopRadius (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's base radius.
* @param[in]  eh            The element to extract from.
* @return cone bottom radius.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT double ExtractBottomRadius (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's top center point.
* @param[in]  eh        The element to extract from.
* @return cone top center.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT DPoint3dCP ExtractTopCenter (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's base center point.
* @param[in]  eh            The element to extract from.
* @return cone bottom center.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT DPoint3dCP ExtractBottomCenter (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's rotation.
* @param[out] rMatrix   The cone's rotation.
* @param[in]  eh        The element to extract from.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Return the given cone element's capped flag.
* @param[in]  eh        The element to extract from.
* @return true if element is a solid cone.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool ExtractCapFlag (ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new CONE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  topRadius     cone top radius
* @param[in]  bottomRadius  cone base radius
* @param[in]  topCenter     center point of top
* @param[in]  bottomCenter  center point of base
* @param[in]  rotation      specifies the skew rotation to apply to top/base circles.
* @param[in]  isCapped      true for solid cone.
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateConeElement (EditElementHandleR eeh, ElementHandleCP templateEh, double topRadius, double bottomRadius, DPoint3dCR topCenter, DPoint3dCR bottomCenter, RotMatrixCR rotation, bool isCapped, DgnModelR modelRef);

}; // ConeHandler
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
