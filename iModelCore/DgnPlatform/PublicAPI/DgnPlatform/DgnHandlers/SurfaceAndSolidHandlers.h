/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/SurfaceAndSolidHandlers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @addtogroup 3DElements
* 3D only surface and solid element types.
* @bsiclass
+===============+===============+===============+===============+===============+======*/

/// @addtogroup 3DElements
/// @beginGroup
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* Base class with behavior common to the SOLID_ELM and SURFACE_ELM types. Used to
* represent capped and uncapped surfaces of projection or revolution. These are
* complex element types that store the profile geometry as components along with
* rule lines for extruded types and rule arcs for revolved types.
* @note SurfaceOrSolidHandler is never the element handler for any element.
* @bsiclass                                                     BrienBastings   02/04
+===============+===============+===============+===============+===============+======*/
struct          SurfaceOrSolidHandler : ComplexHeaderDisplayHandler, 
                                        ISolidPrimitiveEdit
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SurfaceOrSolidHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual void        DrawSurface (ElementHandleCR, ViewContextR);

// Handler
DGNPLATFORM_EXPORT virtual StatusInt   _ApplyTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool        _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
virtual bool                           _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void        _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;
DGNPLATFORM_EXPORT virtual void        _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual SnapStatus  _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ISolidPrimitiveEdit
DGNPLATFORM_EXPORT virtual BentleyStatus _GetSolidPrimitive (ElementHandleCR eh, ISolidPrimitivePtr& primitive) override;
DGNPLATFORM_EXPORT virtual BentleyStatus _SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive) override;

public:

DGNPLATFORM_EXPORT static void CreateSurfaceOrSolidHeaderElement (EditElementHandleR eeh, ElementHandleCP templateEh, bool isExtrusion, bool isSolid, int nBoundElms, DgnModelR modelRef);
DGNPLATFORM_EXPORT static size_t GetNumProfileRules (ElementHandleCP eh);

//! Check the supplied element to see if it represents a surface of projection.
DGNPLATFORM_EXPORT static bool IsSurfaceOfProjection (ElementHandleCR eh, bool* isCapped = NULL);

//! Check the supplied element to see if it represents a surface of revolution.
DGNPLATFORM_EXPORT static bool IsSurfaceOfRevolution (ElementHandleCR eh, bool* isCapped = NULL);

//! Extract profile geometry as curve vectors.
DGNPLATFORM_EXPORT static BentleyStatus GetProfiles (ElementHandleCR source, bvector<CurveVectorPtr>& profiles);

//! Return information for element that represents a simple surface or solid of extrusion.
DGNPLATFORM_EXPORT static BentleyStatus GetProjectionParameters (ElementHandleCR source, CurveVectorPtr& profile, DVec3dR direction, double& distance, bool ignoreSkew);

//! Return information for element that represents a simple surface or solid of revolution.
DGNPLATFORM_EXPORT static BentleyStatus GetRevolutionParameters (ElementHandleCR source, CurveVectorPtr& profile, DPoint3dR center, DVec3dR axis, double& sweep);

//! Update profile/parameters for a simple surface or solid extrusion.
DGNPLATFORM_EXPORT static BentleyStatus SetProjectionParameters (EditElementHandleR eeh, CurveVectorCR profile, DVec3dCR direction, double distance);

//! Update profile/parameters for a simple surface or solid revolition.
DGNPLATFORM_EXPORT static BentleyStatus SetRevolutionParameters (EditElementHandleR eeh, CurveVectorCR profile, DPoint3dCR center, DVec3dCR axis, double sweep, size_t numProfileRules);

//! Check the supplied element to see if it represents a slab.
DGNPLATFORM_EXPORT static bool IsBlock (ElementHandleCR eh, double& length, double& width, double& height, DPoint3dR center, RotMatrixR rMatrix);

//! Check the supplied element to see if it represents a cone.
DGNPLATFORM_EXPORT static bool IsCone (ElementHandleCR eh, double& baseRadius, double& topRadius, DPoint3dR basePt, DPoint3dR topPt, RotMatrixR rMatrix);

//! Check the supplied element to see if it represents a sphere.
DGNPLATFORM_EXPORT static bool IsSphere (ElementHandleCR eh, double& radius, DPoint3dR center, RotMatrixR rMatrix);

//! Check the supplied element to see if it represents a torus.
DGNPLATFORM_EXPORT static bool IsTorus (ElementHandleCR eh, double& eRadius, double& tRadius, double& sweep, DPoint3dR center, RotMatrixR rMatrix);

//! Determine if a surface or solid element represents a simple extrusion of a single 
//! profile without any twist or scale applied and inter the parameters used to construct it.
//! @param[in]  eh            The element to extract from.
//! @param[out] profileEeh    Projected profile geometry.
//! @param[out] direction     The sweep direction for the extrusion.
//! @param[out] distance      The extrusion length.
//! @param[in]  ignoreSkew    true to return information for skewed extrusions (project direction not parrallel with profile normal).
//! @return true if eh is the correct type and the requested information is valid.
//! @bsimethod
DGNPLATFORM_EXPORT static BentleyStatus ExtractProjectionParameters (ElementHandleCR eh, EditElementHandleR profileEeh, DVec3dR direction, double& distance, bool ignoreSkew);

//! Determine if a surface or solid element represents a simple revolution and inter 
//! the parameters used to construct it.
//! @param[in]  eh            The element to extract from.
//! @param[out] profileEeh    Revolved profile geometry.
//! @param[out] center        The center of revolution.
//! @param[out] axis          The axis of revolution.
//! @param[out] sweep         The angle of revoltion in radians.
//! @param[in]  ignoreNonStandardForms  false to return information for revolutions created by applications that aren't one of the standard revolution types.
//! @return true if eh is the correct type and the requested information is valid.
//! @bsimethod
DGNPLATFORM_EXPORT static BentleyStatus ExtractRevolutionParameters (ElementHandleCR eh, EditElementHandleR profileEeh, DPoint3dR center, DVec3dR axis, double& sweep, bool ignoreNonStandardForms = true);

//! Extract the profile geometry for surfaces and solids of projection and revolution.
//! @param[in]  eh            The element to extract from.
//! @param[out] profiles      An element agenda containing the profile geometry.
//! @return SUCCESS if eh is the correct type and the requested information is valid.
//! @note For surfaces of revolution the number of profiles is always one. A surface of projection requires two profiles (common case) but is not limited to only two.
//! @bsimethod
DGNPLATFORM_EXPORT static BentleyStatus ExtractProfiles (ElementHandleCR eh, ElementAgendaR profiles);

//! Create a new SURFACE_ELM or SOLID_ELM that is a ruled surface from the supplied profiles.
//! @param[out] eeh           The new element.
//! @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
//! @param[in]  profiles      The profile sections (minimum of 2, profiles must have similar geometry).
//! @param[in]  preferSolid   Whether to create a capped surface (SOLID_ELM) for profiles that are closed curves or regions.
//! @param[in]  modelRef      Model to associate this element with. Required to compute range.
//! @return SUCCESS if a valid element is created and range was sucessfully calculated.
//! @bsimethod
DGNPLATFORM_EXPORT static BentleyStatus CreateProjectionElement (EditElementHandleR eeh, ElementHandleCP templateEh, ElementAgendaR profiles, bool preferSolid, DgnModelR modelRef);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public: 

/*---------------------------------------------------------------------------------**//**
* Check the supplied element to determine if it is an acceptable type for use
* as a projection or revolution profile. Valid elements are open curve, closed curves,
* and regions of the following types:
* \li LINE_ELM
* \li LINE_STRING_ELM
* \li SHAPE_ELM
* \li CURVE_ELM
* \li CMPLX_STRING_ELM
* \li CMPLX_SHAPE_ELM
* \li ELLIPSE_ELM
* \li ARC_ELM
* \li BSPLINE_CURVE_ELM
* \li CELL_HEADER_ELM that returns true from GroupedHoleHandler::IsGroupedHole.
* \li SURFACE_ELM when create method is called to continue an existing surface.
* \li SOLID_ELM when create method is called to continue an existing solid.
* <p>
* @param[in] eh The element to check.
* @return true if element is valid candidate.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool IsValidProfileType (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Create a new SURFACE_ELM or SOLID_ELM that is a surface of projection with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  profileEh     The element to project.
* @param[in]  origin        The anchor point for the extrusion.
* @param[in]  extrudeVector The start-to-finish sweep bvector.
* @param[in]  transform     Optional rotation and scaling to apply to the profile (can be NULL).
* @param[in]  preferSolid   Whether to create a capped surface (SOLID_ELM) for profiles that are closed curves or regions.
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateProjectionElement (EditElementHandleR eeh, ElementHandleCP templateEh, ElementHandleCR profileEh, DPoint3dCR origin, DVec3dCR extrudeVector, TransformCP transform, bool preferSolid, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new SURFACE_ELM or SOLID_ELM that is a surface of revolution with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  profileEh     The element to revolve.
* @param[in]  center        The center of revolution.
* @param[in]  axis          The axis of revolution.
* @param[in]  sweepAngle    The angle of revoltion in radians.
* @param[in]  preferSolid   Whether to create a capped surface (SOLID_ELM) for profiles that are closed curves or regions.
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @param[in]  numProfileRules Number of radial rule lines to display.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateRevolutionElement (EditElementHandleR eeh, ElementHandleCP templateEh, ElementHandleCR profileEh, DPoint3dCR center, DVec3dCR axis, double sweepAngle, bool preferSolid, DgnModelR modelRef, size_t numProfileRules = 1);

}; // SurfaceOrSolidHandler

/*=================================================================================**//**
* The default type handler for the SURFACE_ELM type. The element data is stored using
* the Surface structure. The SURFACE_ELM type is used for uncapped surfaces of
* projection and revolution. This is a 3d only element type, it has no 2d representation
* and can not be added to a 2d model.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          SurfaceHandler : SurfaceOrSolidHandler
{
    DEFINE_T_SUPER(SurfaceOrSolidHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SurfaceHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void _Draw (ElementHandleCR, ViewContextR) override;

public:

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // SurfaceHandler

/*=================================================================================**//**
* The default type handler for the SOLID_ELM type. The element data is stored using
* the Surface structure. The SOLID_ELM type is used for capped surfaces of
* projection and revolution. This is a 3d only element type, it has no 2d representation
* and can not be added to a 2d model.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          SolidHandler : SurfaceOrSolidHandler
{
    DEFINE_T_SUPER(SurfaceOrSolidHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SolidHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void _Draw (ElementHandleCR, ViewContextR) override;

public:

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // SolidHandler
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
