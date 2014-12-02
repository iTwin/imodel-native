/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementGeometry.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "SolidKernel.h"
#include "ViewContext.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

// =======================================================================================
//! Query an element for a geometric representation that is an open or closed curve path.
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ICurvePathQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Return path curves as a bvector of ICurvePrimitive. May be defined by multiple connected curves.
//! @param[in] eh Source element
//! @param[out] curves curve data
//! @return SUCCESS if the curve vector is returned.
DGNPLATFORM_EXPORT BentleyStatus GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves);

//! Wrapper method that just dynamic casts handler to ICurvePathQuery and calls GetCurveVector.
DGNPLATFORM_EXPORT static CurveVectorPtr ElementToCurveVector (ElementHandleCR eh);
};

// =======================================================================================
//! Modify an element that represents an open or closed path.
// @bsiclass                                                      Brien.Bastings  11/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ICurvePathEdit : ICurvePathQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Update element from path curves.
//! @param[in] eeh Source element
//! @param[in] path Path curve data
//! @return SUCCESS if the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetCurveVector (EditElementHandleR eeh, CurveVectorCR path);
};

// =======================================================================================
//! Query an element for a geometric representation that is a bspline surface.
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IBsplineSurfaceQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _GetBsplineSurface (ElementHandleCR source, MSBsplineSurfacePtr& surface) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Copy the surface as an MSBsplineurface. Caller is responsible to free it.
//! @param[in]      source      source element
//! @param[out]     surface     copy of surface
//! @return SUCCESS if the bspline surface is returned.
DGNPLATFORM_EXPORT BentleyStatus GetBsplineSurface (ElementHandleCR source, MSBsplineSurfacePtr& surface);
};

// =======================================================================================
//! Modify an element that represents a bspline surface.
// @bsiclass                                                      Brien.Bastings  11/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IBsplineSurfaceEdit : IBsplineSurfaceQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _SetBsplineSurface (EditElementHandleR eeh, MSBsplineSurfaceCR surface) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Update element from a MSBsplineSurface.
//! @param[in]      eeh         Source element
//! @param[in]      surface     Surface data
//! @return SUCCESS if the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetBsplineSurface (EditElementHandleR eeh, MSBsplineSurfaceCR surface);
};

// =======================================================================================
//! Query an element for a geometric representation that is a surface/solid primitive.
// @bsiclass                                                      Brien.Bastings  03/2012
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISolidPrimitiveQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _GetSolidPrimitive (ElementHandleCR, ISolidPrimitivePtr&) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Test if the element is a solid primitive and if so return it's information.
//! @param[in]      source      Source element
//! @param[out]     primitive   Solid primitive information.
//! @return SUCCESS if returned data is valid.
DGNPLATFORM_EXPORT BentleyStatus GetSolidPrimitive (ElementHandleCR source, ISolidPrimitivePtr& primitive);

//! Wrapper method that just dynamic casts handler to ISolidPrimitiveQuery and calls GetSolidPrimitive.
//! Optionally calls ISolidPrimitive::Simplify to return simplest solid primitive representation.
//! @param[in]      eh      Source element
//! @param[in]      simplify    Return swept profiles as simple cone, sphere, torus, and box primitives when possible.
DGNPLATFORM_EXPORT static ISolidPrimitivePtr ElementToSolidPrimitive (ElementHandleCR eh, bool simplify = true);
};

// =======================================================================================
//! Modify an element that represents a surface/solid primitive.
// @bsiclass                                                      Brien.Bastings  03/2012
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISolidPrimitiveEdit : ISolidPrimitiveQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _SetSolidPrimitive (EditElementHandleR, ISolidPrimitiveCR) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Update element from solid primitive information.
//! @param[in]      eeh         Source element
//! @param[out]     primitive   Solid primitive information.
//! @return SUCCESS if the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive);
};

// =======================================================================================
//! Query an element for a geometric representation that is a polyface mesh.
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IMeshQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _GetMeshData (ElementHandleCR source, PolyfaceHeaderPtr&) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Test if the element is a polyface mesh and if so return the mesh data.
//! @param[in]      source      Source element
//! @param[out]     meshData    Polyface mesh data.
//! @return SUCCESS if returned MeshData is valid.
DGNPLATFORM_EXPORT BentleyStatus GetMeshData (ElementHandleCR source, PolyfaceHeaderPtr& meshData);

//__PUBLISH_SECTION_END__
//! Build an (array of) output polyface from faceted surfaces and solids.
//! @param[in]      source      Source element
//! @param[in]      options     facet options
//! @param[out]     meshData    Array of polyface meshes.
//! @return SUCCESS if returned MeshData is valid.
static DGNPLATFORM_EXPORT  BentleyStatus    ElementToApproximateFacets  (ElementHandleCR source, bvector<PolyfaceHeaderPtr> &meshData, IFacetOptionsP options = NULL);
//__PUBLISH_SECTION_START__
};

// =======================================================================================
//! Modify an element that represents a mesh.
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IMeshEdit : IMeshQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _SetMeshData (EditElementHandleR eeh, PolyfaceQueryR meshData) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Update element from mesh data.
//! @param[in]      eeh         Source element
//! @param[out]     meshData    Polyface mesh data.
//! @return SUCCESS if the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetMeshData (EditElementHandleR eeh, PolyfaceQueryR meshData);
};

// =======================================================================================
//! Query an element for a geometric representation that uses a solid modeling kernel.
//! @remarks Requires host implementation of SolidsKernelAdmin::_RestoreEntityFromMemory.
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IBRepQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _GetBRepDataEntity (ElementHandleCR source, ISolidKernelEntityPtr& entity, bool useCache) = 0;
DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Test if the element is a boundary representation surface/solid that uses a solid kernel.
//! @param[in]      source      Source element
//! @param[out]     entity      Brep data.
//! @param[in]      useCache    Allow cached brep to be returned. To extract/modify the brep pass false to request a copy, for query operations pass true.
//! @return SUCCESS if returned entity is valid.
DGNPLATFORM_EXPORT BentleyStatus GetBRepDataEntity (ElementHandleCR source, ISolidKernelEntityPtr& entity, bool useCache = false);

//! Get the solid to uor scale that is applied to the brep data of elements from the specified dgn cache.
//! @param[in]      dgnCache    Which cache to return the scale for.
//! @return solid kernel to uor scale used by all brep elements from the supplied dgncache.
DGNPLATFORM_EXPORT static double GetSolidKernelToUORScale (DgnModelP dgnCache);
};

// =======================================================================================
//! Modify an element that represents a solid modeling kernel surface or solid.
// @bsiclass                                                      Brien.Bastings  09/2012
// =======================================================================================
struct IBRepEdit : IBRepQuery
{
//__PUBLISH_SECTION_END__
protected:

virtual BentleyStatus _SetBRepDataEntity (EditElementHandleR eeh, ISolidKernelEntityR entity) = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Update element from bRep data.
//! @param[in]      eeh     Source element
//! @param[out]     entity  solid kernel entity data.
//! @return SUCCESS if the element was updated.
DGNPLATFORM_EXPORT BentleyStatus SetBRepDataEntity (EditElementHandleR eeh, ISolidKernelEntityR entity);
};

/// @endGroup

#if defined (NEEDS_WORK_DGNITEM)
// =======================================================================================
//! Query an element if it is a public collection of other elements. A cell is a
//! physical container or group as opposed to a loose collection like a graphic or
//! named group. User defined cells (and those placed as shared cells) will implement 
//! the ICellQuery interface.
//! @see ChildElemIter, ChildEditElemIter
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ICellQuery
{
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual bool _IsNormalCell (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _IsSharedCell (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _IsSharedCellDefinition (ElementHandleCR eh);

DGNPLATFORM_EXPORT virtual bool _IsPointCell (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _IsAnnotation (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _IsAnonymous (ElementHandleCR eh);

DGNPLATFORM_EXPORT virtual BentleyStatus _ExtractScale (DVec3dR scale, ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual BentleyStatus _ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual BentleyStatus _ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Query if the supplied element is a normal type 2 cell.
//! @param[in]      eh          The element to query.
//! @return true if eh is a normal type 2 cell.
DGNPLATFORM_EXPORT bool IsNormalCell (ElementHandleCR eh);

//! Query if the supplied element is a shared cell instance.
//! @param[in]      eh          The element to query.
//! @return true if eh is a shared cell instance.
DGNPLATFORM_EXPORT bool IsSharedCell (ElementHandleCR eh);

//! Query if the supplied element is a shared cell definition.
//! @param[in]      eh          The element to query.
//! @return true if eh is a shared cell definition.
DGNPLATFORM_EXPORT bool IsSharedCellDefinition (ElementHandleCR eh);

//! Return whether the given element is a point cell. A point cell is view independent
//! and will always rotate itself about it's origin and orient itself to the views it
//! is displayed in. Point cells are always a uniform symbology and the origin is the
//! only snappable location.
//! @param[in]      eh          The element to query.
//! @return true if eh is a cell or shared cell that is also a point cell. 
//! @note For a shared cell definition the point cell status is stored so that it
//!       may be propagated to new instances that are created from it.
DGNPLATFORM_EXPORT bool IsPointCell (ElementHandleCR eh);

//! Return whether the cell is an annotation cell.
//! @param[in]      eh          The element to query.
//! @return true if eh is a cell or shared cell that is an annotation cell. 
//! @note This is a property of the shared cell definition, will look up definition if supplied an instance.
DGNPLATFORM_EXPORT bool IsAnnotation (ElementHandleCR eh);

//! Return whether the cell is an orphan or the definition for a supplied instance is an anonymous 
//! definition and does not have a unique name.
//! @param[in]      eh          The element to query.
//! @return true if eh is a cell or shared cell that is anonymous (name doesn't uniquely identify a common set of geometry).
//! @note This is a property of the shared cell definition, will look up definition if supplied an instance.
DGNPLATFORM_EXPORT bool IsAnonymous (ElementHandleCR eh);

//! Return the given cell element's scale bvector.
//! @param[out]     scale       The cell's scale (column scale bvector).
//! @param[in]      eh          The element to query.
//! @return SUCCESS if eh is a cell or shared cell and requested information is valid.
//! @see DisplayHandler::GetOrientation
DGNPLATFORM_EXPORT BentleyStatus ExtractScale (DVec3dR scale, ElementHandleCR eh);

//! Return the given cell element's name. A name is required if the shared cell
//! is not anonymous.
//! @param[out]     cellName    The cell's name.
//! @param[in]      bufferSize  Maximum number of characters to return.
//! @param[in]      eh          The element to query.
//! @return SUCCESS if eh is a cell or shared cell with a name.
//! @DotNetMethodExclude
DGNPLATFORM_EXPORT BentleyStatus ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh);

//! Return the given cell element's description if it has one.
//! @param[out]     descr       The cell's description.
//! @param[in]      bufferSize  Maximum number of characters to return.
//! @param[in]      eh          The element to query.
//! @return SUCCESS if eh is a cell or shared cell with a description.
//! @note This function looks for a description specific to the supplied instance,
//!       typically when placing a shared cell from a user defined cell with a 
//!       description, the description is not propagated to every instance and
//!       the description should be obtained from the shared cell definition.
//! @DotNetMethodExclude
DGNPLATFORM_EXPORT BentleyStatus ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh);
};

// =======================================================================================
//! Query an element for information specific to shared cells.
//! @see ICellQuery
// @bsiclass                                                      Brien.Bastings  06/2009
// =======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISharedCellQuery : ICellQuery
{
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual bool _GetMlineScaleOption (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _GetDimScaleOption (ElementHandleCR eh);
DGNPLATFORM_EXPORT virtual bool _GetDimRotationOption (ElementHandleCR eh);
                                                                    
DGNPLATFORM_EXPORT virtual ElementRefP _GetDefinition (ElementHandleCR eh, DgnProjectR);
DGNPLATFORM_EXPORT virtual BentleyStatus _GetDefinitionID (ElementHandleCR eh, ElementId& elemID);
DGNPLATFORM_EXPORT virtual SCOverride const* _GetSharedCellOverrides (ElementHandleCR eh);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Query the flag that controls the display behavior of Multiline offsets when displayed
//! through a scaled shared cell instance.
//! @param[in]      eh          The element to query.
//! @return true if eh is a shared cell and multiline scaling flag is set.
//! @note This is a property of the shared cell definition, will look up definition if supplied an instance.
DGNPLATFORM_EXPORT bool GetMlineScaleOption (ElementHandleCR eh);

//! Query the flag that controls the display behavior of dimensions when displayed
//! through a scaled shared cell instance.
//! @param[in]      eh          The element to query.
//! @return true if eh is a shared cell and dimension scaling flag is set.
//! @note This is a property of the shared cell definition, will look up definition if supplied an instance.
//! @remarks The shared cell instance can override the behavior specified by the definition.
//! @see GetSharedCellOverrides
DGNPLATFORM_EXPORT bool GetDimScaleOption (ElementHandleCR eh);

//! Query the flag that controls the display behavior of dimensions in nested shared cells with
//! regards to whether to use the instance rotation or parent rotation.
//! @param[in]      eh          The element to query.
//! @return true if eh is a shared cell and dimension rotation flag is set.
//! @note This is a property of the shared cell definition, will look up definition if supplied an instance.
DGNPLATFORM_EXPORT bool GetDimRotationOption (ElementHandleCR eh);

//! Return the shared cell definition for the supplied shared cell instance.
//! @param[in]      eh          A shared cell instance.
//! @param[in]      project     The file for the shared cell instance.
//! @return ElementRefP for shared cell definition or NULL if not found.
//! @remarks Finds definition by name or by definition id for anonymous shared cells.
DGNPLATFORM_EXPORT ElementRefP GetDefinition (ElementHandleCR eh, DgnProjectR project);

//! Return the shared cell definition id for the supplied shared cell instance.
//! @param[in]      eh          A shared cell instance.
//! @param[out]     elemID      The element id of the shared cell definition.
//! @return SUCCESS if a valid shared cell definition dependency exits and elemID is valid.
//! @note Named shared cells may have an explicit dependency to their definition, however
//!       this is not required and normally reserved only for anonymous shared cells where
//!       it is required.
DGNPLATFORM_EXPORT BentleyStatus GetDefinitionId (ElementHandleCR eh, ElementId& elemID);

//! Get the shared cell override values and flags that will be used when visiting
//! the components of the shared cell definition.
//! @param[in]      eh          A shared cell instance.
//! @return Overrides or NULL if element is not a ahared cell.
DGNPLATFORM_EXPORT SCOverride const* GetSharedCellOverrides (ElementHandleCR eh);

//! Find and return a shared cell definition by name.
//! @param[in]      name        The name to search for.
//! @param[in]      project     The file to search in.
//! @return ElementRefP for shared cell definition or NULL if not found.
//! @note Will not return an anonymous shared cell definition.
//! @see GetDefinition
DGNPLATFORM_EXPORT static PersistentElementRefPtr FindDefinitionByName (WCharCP name, DgnProjectR project);
};
#endif

//__PUBLISH_SECTION_END__
struct EXPORT_VTABLE_ATTRIBUTE WireframeGeomUtil
{
static bool CollectRules (DgnExtrusionDetailR, bvector<DSegment3d>&, bvector<bool>&, ViewContextP = NULL);
static bool CollectRules (DgnRotationalSweepDetailR, bvector<DEllipse3d>&, bvector<bool>&, ViewContextP = NULL);
static bool CollectRules (DgnRuledSweepDetailR, bvector<DSegment3d>&, bvector<bool>&, ViewContextP = NULL);

DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves (ISolidPrimitiveCR, bool includeEdges = true, bool includeFaceIso = false);
DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves (MSBsplineSurfaceCR, bool includeEdges = true, bool includeFaceIso = false);
DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves (ISolidKernelEntityCR, bool includeEdges = true, bool includeFaceIso = false);

DGNPLATFORM_EXPORT static void Draw (ISolidPrimitiveCR, ViewContextR, bool includeEdges = true, bool includeFaceIso = true);
DGNPLATFORM_EXPORT static void Draw (MSBsplineSurfaceCR, ViewContextR, bool includeEdges = true, bool includeFaceIso = true);
DGNPLATFORM_EXPORT static void Draw (ISolidKernelEntityCR, ViewContextR, IFaceMaterialAttachmentsCP attachments = NULL, bool includeEdges = true, bool includeFaceIso = true);

DGNPLATFORM_EXPORT static void DrawOutline (CurveVectorCR, IDrawGeomR);
DGNPLATFORM_EXPORT static void DrawOutline2d (CurveVectorCR, IDrawGeomR, double zDepth);
DGNPLATFORM_EXPORT static void DrawStyledCurveVector (CurveVectorCR, ViewContextR, bool is3d = true, GeomRepresentations info = DISPLAY_INFO_Edge);

}; // WireframeGeomUtil

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
