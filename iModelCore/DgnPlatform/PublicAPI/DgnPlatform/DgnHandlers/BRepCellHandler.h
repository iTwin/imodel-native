/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/BRepCellHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include "CellHeaderHandler.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup 3DElements
/// @beginGroup
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* The Brep cell is used to store boundary representation surfaces and solids created 
* using either the Parasolid or ACIS kernels as elements. The cell's displayable children
* are the edge and face iso-param curves that will be displayed in a wireframe view. 
* The brep data is persisted in non-graphical DgnStore elements, either as children of the 
* cell or as an external element dependency. This is a 3d only element type.
* @note The brep data is stored scaled by a solid kernel to uor scale factor. This scale
*       can be set for each DgnModel and affects all BRep elements from that cache.
*       Elements from the dictionary model use the same scale as the default model.
* @see IBRepQuery::GetSolidKernelToUORScale
* @bsiclass                                                     Sam.Wilson      10/2004
+===============+===============+===============+===============+===============+======*/
struct          BrepCellHeaderHandler : Type2Handler,
                                        IBRepEdit
                                        //__PUBLISH_SECTION_END__
                                        ,ISubTypeHandlerQuery
                                        //__PUBLISH_SECTION_START__
{
    DEFINE_T_SUPER(Type2Handler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (BrepCellHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void         _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void         _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool         _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual void         _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnChangeOfUnits (EditElementHandleR, DgnModelP source, DgnModelP dest) override;
DGNPLATFORM_EXPORT virtual void         _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;
                                        
// DisplayHandler
virtual bool                            _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void         _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;
DGNPLATFORM_EXPORT virtual bool         _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass) override;
DGNPLATFORM_EXPORT virtual void         _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void         _DrawDecorations (ElementHandleCR, ViewContextR, UInt32 displayInfo, QvElem* qvElem=NULL) {}
DGNPLATFORM_EXPORT virtual StatusInt    _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ComplexHeaderDisplayHandler
DGNPLATFORM_EXPORT virtual void         _QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void         _EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context) override;

// IBRepEdit
DGNPLATFORM_EXPORT virtual BentleyStatus _GetBRepDataEntity (ElementHandleCR source, ISolidKernelEntityPtr& entity, bool useCache) override;
DGNPLATFORM_EXPORT virtual BentleyStatus _SetBRepDataEntity (EditElementHandleR eeh, ISolidKernelEntityR entity) override;

// ISubTypeHandlerQuery
DGNPLATFORM_EXPORT virtual bool _ClaimElement (ElementHandleCR) override;

public:

DGNPLATFORM_EXPORT static bool   AdjustBRepDataScale (EditElementHandleR eeh, double scale, bool allowBRepLinkageScale=true);

/*---------------------------------------------------------------------------------**//**
* Create a new BRep cell header.
* After creating the header element the application needs to add the edge geometry and brep data
* to complete the element.
* @param[out] eeh       The new element.
* @param[in]  isSolid   Whether the brep data represents surface or solid geometry.
* @param[in]  modelRef  Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                       and later used by AddChildComplete to update the cell's range. 
* @see NormalCellHeaderHandler::AddChildElement NormalCellHeaderHandler::AddChildComplete DgnStoreHdrHandler::AppendToCell
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void   CreateBRepCellHeaderElement (EditElementHandleR eeh, bool isSolid, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Check the dirty flag on the embedded brep linkage.
* @param[in]  eh        The element to test.
* @return true if dirty flag is set on embedded brep linkage.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool  IsBRepDataValid (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Check the face material attachment flag on the embedded brep linkage.
* @param[in]  eh        The element to test.
* @return true if materials have been attached to faces of body.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool  HasMaterialAttachmentToFace (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Update brep data dgnstore cell components from the supplied ISolidKernelEntity.
* @param[out] eeh           The element to update.
* @param[in]  entity        The brep data.
* @note Does not update edge/face geometry! Should only use for a "non-graphic" change ex. add/remove a face material attachment.
* @remarks Requires host implementation of SolidsKernelAdmin.
* @return SUCCESS if element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus UpdateBRepDataDgnStore (EditElementHandleR eeh, ISolidKernelEntityCR entity);

/*---------------------------------------------------------------------------------**//**
* Get the solid kernel (either Parasolid or ACIS) that the brep data was created in.
* @param[out] kernel        Either SolidKernel_PSolid or SolidKernel_ACIS.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  GetBRepDataKernel (ISolidKernelEntity::SolidKernelType& kernel, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Get the brep data version
* @param[out] version       brep data version.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  GetBRepDataVersion (int& version, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Get the brep data scale factor. This scale is in addition to the implied solid to uor
* scale stored for each DgnModel (dgnCache_getSolidExtent).
* @param[out] scale         brep data scale.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  GetBRepDataScale (double& scale, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Set the brep data scale factor. This scale is in addition to the implied solid to uor
* scale stored for each DgnModel (dgnCache_getSolidExtent).
* @param[out] eeh           The element to update.
* @param[in]  scale         new brep data scale.
* @return SUCCESS if eh is the correct type and the requested information was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  SetBRepDataScale (EditElementHandleR eeh, double scale);

/*---------------------------------------------------------------------------------**//**
* Get the element id for an external brep data DgnStore. If ERROR then the brep data is
* stored as child elements.
* @note FeatureSolids use both external and internal brep DgnStores. SmartSolids and SmartSurface
*       always use an internal brep storing the DgnStore as child elements.
* @param[out] elemId        brep data DgnStore element id.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  GetExternalBRepDataElementID (ElementId& elemId, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Set the element id for an external brep data DgnStore.
* @note FeatureSolids use both external and internal brep DgnStores. SmartSolids and SmartSurface
*       always use an internal brep storing the DgnStore as child elements.
* @param[out] eeh           The element to update.
* @param[in]  elemId        new DgnStore element id.
* @return SUCCESS if eh is the correct type and the requested information was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  SetExternalBRepDataElementID (EditElementHandleR eeh, ElementId elemId);

/*---------------------------------------------------------------------------------**//**
* Get the DgnStore id that should be used to create/extract a brep data DgnStore element.
* @return Brep data DgnStore id.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static UInt32 GetBRepDataDgnStoreId () {return DGN_STORE_ID;}

/*---------------------------------------------------------------------------------**//**
* Get the DgnStore application id that should be used to create/extract a brep data DgnStore element.
* @return Brep data DgnStore application id.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static UInt32 GetBRepDataDgnStoreApplicationId () {return EMBEDDED_BREP_ID;}

/*---------------------------------------------------------------------------------**//**
* Extract the brep data from a BRep cell type either from child element DgnStores or an
* external DgnStore dependency.
* @param[out] dataPP        pointer to buffer to hold extract brep data bytes. Caller is expected to 
*                           free using DgnStoreHdrHandler::FreeExtractedData.
* @param[out] dataSizeP     Size of dataPP in bytes. 
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  ExtractDgnStoreBRepData (void** dataPP, UInt32* dataSizeP, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Helper function when round-tripping an element though the solid kernel. Filtered copy of linkages from
* template to prevent known non-applicable linkages finding their way onto the new element.
* @param[out] eeh           The new element to append appropriate template linkages to.
* @param[in]  templateEh    The existing element and linkage source.
* @note Called automatically by CreateBRepCellElement when a non-NULL templateEh is supplied. This
*       method is exported for use with PSolidUtil::CreateElement when output is simple open curve
*       and region elements via CurveVector.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void PropagateTemplateLinkages (EditElementHandleR eeh, ElementHandleCR templateEh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new BRep cell from the supplied ISolidKernelEntity.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Optional template element.
* @param[in]  entity        The brep data.
* @param[in]  modelRef      Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                           and later used by AddChildComplete to update the cell's range. 
* @remarks Requires host implementation of SolidsKernelAdmin.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateBRepCellElement (EditElementHandleR eeh, ElementHandleCP templateEh, ISolidKernelEntityCR entity, DgnModelR modelRef);

}; // BrepCellHeaderHandler

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* Class used for internal feature solid nodes when feature solid application is not
* delivered (products like Draft/View). Prefer this over NormalCellHeaderHandler but
* not over BRepCellHeaderHandler by setting handler priority to low. Prevents the
* senario where an element that does not expose public children has a child
* that wants to expose public children...which is problematic for child iterators.
* @bsiclass                                                     Brien.Bastings  09/2008
+===============+===============+===============+===============+===============+======*/
struct          MissingFeatureSolidAppHandler : Type2Handler,
                                                ISubTypeHandlerQuery
{
    DEFINE_T_SUPER(Type2Handler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (MissingFeatureSolidAppHandler, DGNPLATFORM_EXPORT)
protected:

// Handler
DGNPLATFORM_EXPORT virtual void         _GetTypeName (WStringR string, UInt32 desiredLength) override;

// ISubTypeHandlerQuery
DGNPLATFORM_EXPORT virtual bool _ClaimElement (ElementHandleCR) override;

virtual bool                            _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) {return false;}

}; // MissingFeatureSolidAppHandler

//__PUBLISH_SECTION_START__

/** @endGroup */
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
