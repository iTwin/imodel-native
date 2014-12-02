/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DropGraphics.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

//__PUBLISH_SECTION_END__
#include <DgnPlatform/DgnCore/SimplifyViewDrawGeom.h>
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup GeometryCollectors
/// @beginGroup

typedef RefCountedPtr<DropGeometry> DropGeometryPtr;

/*=================================================================================**//**
* DropGeometry is suplied to DisplayHandler::Drop to allow the handler to produce
* a simplified representation that isn't necessarily just dumb graphics.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DropGeometry : RefCountedBase
{
public:

enum Options
{
OPTION_None             = (0),
OPTION_Text             = (1<<0),
OPTION_Dimensions       = (1<<1),
OPTION_Mlines           = (1<<2),
OPTION_Complex          = (1<<3),
OPTION_LinearSegments   = (1<<4),
OPTION_SharedCells      = (1<<5),
OPTION_Solids           = (1<<6),
OPTION_AppData          = (1<<7),
};

enum Dimensions
{
DIMENSION_Geometry      = 0,
DIMENSION_Segments      = 1,
};

enum SharedCells
{
SHAREDCELL_Geometry             = 0, //! Drop a shared cell instance to geometry.
SHAREDCELL_NormalCell           = 1, //! Drop a shared cell instance to a normal cell expanding nested instances.
SHAREDCELL_NormalCellOneLevel   = 2, //! Drop a shared cell instance to a normal cell preserving nested instances.
};

enum Solids
{
SOLID_Surfaces          = 0,
SOLID_Wireframe         = 1,
};

//__PUBLISH_SECTION_END__
private:

Options         m_options;
Dimensions      m_dimensions;
SharedCells     m_sharedCells;
Solids          m_solids;

public:

DGNPLATFORM_EXPORT          DropGeometry ();
DGNPLATFORM_EXPORT explicit DropGeometry (Options options);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

DGNPLATFORM_EXPORT void        SetOptions (Options options);
DGNPLATFORM_EXPORT Options     GetOptions () const;

DGNPLATFORM_EXPORT void        SetDimensionOptions (Dimensions option);
DGNPLATFORM_EXPORT Dimensions  GetDimensionOptions () const;

DGNPLATFORM_EXPORT void        SetSharedCellOptions (SharedCells option);
DGNPLATFORM_EXPORT SharedCells GetSharedCellOptions () const;

DGNPLATFORM_EXPORT void        SetSolidsOptions (Solids option);
DGNPLATFORM_EXPORT Solids      GetSolidsOptions () const;

//! Create an instance of DropGeometry for setting geometry options.
//! @return A reference counted pointer to a DropGeometry instance.
DGNPLATFORM_EXPORT static DropGeometryPtr Create ();

}; // DropGeometry

typedef RefCountedPtr<DropGraphics> DropGraphicsPtr;

/*=================================================================================**//**
* DropGraphics is used by drop methods to request the output of common "graphics" not
* specific to an element handler like patterns and linestyles.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DropGraphics : RefCountedBase
{
public:

enum Options
{
OPTION_None             = (0),
OPTION_LineStyles       = (1<<0),
OPTION_Patterns         = (1<<1),
//__PUBLISH_SECTION_END__
OPTION_ApplyOverrides   = (1<<2),
OPTION_ApplyRenderMode  = (1<<3)
//__PUBLISH_SECTION_START__
};

enum PatternBoundary
{
BOUNDARY_Include        = 0,
BOUNDARY_Ignore         = 1,
BOUNDARY_Only           = 2,
};

//__PUBLISH_SECTION_END__
private:

Options         m_options;
PatternBoundary m_patternBoundary;
int             m_patternIndex;

public:

DGNPLATFORM_EXPORT          DropGraphics ();
DGNPLATFORM_EXPORT explicit DropGraphics (Options options);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

DGNPLATFORM_EXPORT void            SetOptions (Options options);
DGNPLATFORM_EXPORT Options         GetOptions () const;

DGNPLATFORM_EXPORT void            SetPatternIndex (int index);
DGNPLATFORM_EXPORT int             GetPatternIndex () const;

DGNPLATFORM_EXPORT void            SetPatternBoundary (PatternBoundary boundary);
DGNPLATFORM_EXPORT PatternBoundary GetPatternBoundary () const;

//! Create an instance of DropGraphics for setting graphics options.
//! @return A reference counted pointer to a DropGraphics instance.
DGNPLATFORM_EXPORT static DropGraphicsPtr Create ();

}; // DropGraphics

//__PUBLISH_SECTION_END__
#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Context to drop an element
* @bsiclass                                                     Brien.Bastings  06/05
+===============+===============+===============+===============+===============+======*/
struct DropToElementDrawGeom : SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
protected:

DropGeometry        m_geometry;
DropGraphics        m_graphics;
ElementAgendaP      m_agenda;

bool                m_nonsnappable;
bool                m_preserveClosedCurves;

DGNPLATFORM_EXPORT virtual bool             _ProcessAsBody (bool isCurved) const override;
DGNPLATFORM_EXPORT virtual bool             _DoClipping () const override;
DGNPLATFORM_EXPORT virtual bool             _DoTextGeometry () const override;
DGNPLATFORM_EXPORT virtual bool             _DoSymbolGeometry () const override;
DGNPLATFORM_EXPORT virtual bool             _ClipPreservesRegions () const override;

DGNPLATFORM_EXPORT virtual StatusInt        _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override;
DGNPLATFORM_EXPORT virtual StatusInt        _ProcessSolidPrimitive (ISolidPrimitiveCR primitive) override;
DGNPLATFORM_EXPORT virtual StatusInt        _ProcessSurface (MSBsplineSurfaceCR surface) override;
DGNPLATFORM_EXPORT virtual StatusInt        _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP) override;
DGNPLATFORM_EXPORT virtual StatusInt        _ProcessFacetSet (PolyfaceQueryCR pFacets, bool filled) override;

DGNPLATFORM_EXPORT virtual void             _ProcessAreaPattern (ElementHandleCR thisElm, ViewContext::ClipStencil& boundary, ViewContext::PatternParamSource& source);
DGNPLATFORM_EXPORT virtual void             _DrawTextString (TextStringCR text, double* zDepth) override;

DGNPLATFORM_EXPORT virtual StatusInt        _OnOutputElement (ElementHandleCR eh) override;
DGNPLATFORM_EXPORT virtual bool             _ApplyRenderMode (EditElementHandleR eeh, bool isClosed = false, bool isFilled = false);
DGNPLATFORM_EXPORT virtual void             _ApplyCurrentAreaParams (EditElementHandleR eeh);
DGNPLATFORM_EXPORT virtual void             _ApplyCurrentDisplayParams (EditElementHandleR eeh);
DGNPLATFORM_EXPORT virtual void             _OnElementCreated (EditElementHandleR eeh);

public:

DGNPLATFORM_EXPORT                          DropToElementDrawGeom (ElementAgendaP agenda);
DGNPLATFORM_EXPORT                          ~DropToElementDrawGeom ();
DGNPLATFORM_EXPORT void                     Init (ViewContextP context, DropGeometryCR info, DropGraphicsCR graphicsInfo);

DGNPLATFORM_EXPORT DropGeometryR            GetDropGeometry ();
DGNPLATFORM_EXPORT DropGraphicsR            GetDropGraphics ();
DGNPLATFORM_EXPORT void                     SetElementNonSnappable (bool nonsnappable);
DGNPLATFORM_EXPORT void                     SetPreserveClosedCurves (bool preserveClosed);

DGNPLATFORM_EXPORT ElementAgendaP           GetDropGeometryAgenda ();
DGNPLATFORM_EXPORT void                     OnElementCreated (EditElementHandleR eeh);
DGNPLATFORM_EXPORT void                     ProcessAreaPattern (ElementHandleCR thisElm, ViewContext::ClipStencil& boundary, ViewContext::PatternParamSource& source);
DGNPLATFORM_EXPORT TransformCP              GetCurrTransformToWorld (TransformR tmpTransform);

static DGNPLATFORM_EXPORT StatusInt         DoDrop (ElementHandleCR elHandle, ElementAgendaR agenda, DropGeometryCR info, DropGraphicsCR graphics);

}; // DropToElementDrawGeom
#endif

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2012
+===============+===============+===============+===============+===============+======*/
struct CaptureGraphicsToElement
{
static DGNPLATFORM_EXPORT StatusInt CaptureDisplayPath (ElementAgendaR agenda, DgnModelR dest, DisplayPathCR displayPath, ViewportP vp, DropGeometryCR info, DropGraphicsCR graphics, bool preserveClosedCurves);
static DGNPLATFORM_EXPORT StatusInt CaptureViewGraphics (ElementAgendaR agenda, DgnModelR dest, DgnModelR source, ViewportP vp, DropGeometryCR info, DropGraphicsCR graphics, bool includeRefs, bool preserveClosedCurves, FenceParamsCP fenceParams);
};

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/2010
+===============+===============+===============+===============+===============+======*/
struct DraftingElementSchema
{
//! Create a single element from a CurveVector that represents a point string, open curve, closed curve, or region.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, CurveVectorCR curves, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef);

//! Create a single element from a ICurvePrimitive that represents a point string, open curve, or closed curve.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, ICurvePrimitiveCR curve, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef);

//! Populate an ElementAgenda from an CurveVector that represents a collection of un-related elements (BOUNDARY_TYPE_None).
static DGNPLATFORM_EXPORT BentleyStatus ToElements (ElementAgendaR agenda, CurveVectorCR curves, ElementHandleCP templateEh, bool is3d, DgnModelR modelRef);

//! Create a single element from a solid primtive.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, ISolidPrimitiveCR solid, ElementHandleCP templateEh, DgnModelR modelRef);

//! Create a single element from a bspline surface.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, MSBsplineSurfaceCR surface, ElementHandleCP templateEh, DgnModelR modelRef);

//! Create a single element from polyface data.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, PolyfaceQueryCR meshData, ElementHandleCP templateEh, DgnModelR modelRef);

//! Create a single smart solid or surface element from brep data.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, ISolidKernelEntityCR entityData, ElementHandleCP templateEh, DgnModelR modelRef);

//! Create a single element from geometry reference to a geometry class.
static DGNPLATFORM_EXPORT BentleyStatus ToElement (EditElementHandleR eeh, IGeometryCR geom, ElementHandleCP templateEh, DgnModelR modelRef);

//! Create a group of "simple" elements that represent the graphics of the supplied element.
static DGNPLATFORM_EXPORT BentleyStatus ToDroppedElements (ElementHandleCR eh, ElementAgendaR agenda, DropGraphicsCR graphics);

}; // DraftingElementSchema

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

