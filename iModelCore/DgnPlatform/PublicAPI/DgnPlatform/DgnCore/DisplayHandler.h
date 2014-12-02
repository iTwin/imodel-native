/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "DgnElements.h"


#include <map>
#include "Handler.h"
#include "ElementHandle.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @addtogroup ElementHandler */
/** @beginGroup */

/*=================================================================================**//**
  Base class for handlers that control the display of graphic (visible) elements.
  @bsiclass
+===============+===============+===============+===============+===============+======*/

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
  Base class for handlers that control the display of graphic (visible) elements.

  <h2>Implementing Draw</h2>

  MicroStation will call the #Draw method of DisplayHandler whenever it needs to generate graphics. This will obviously happen
  when the element is drawn in a view, but also happens in other circumstances too. For example:
  \li to generate the element's range
  \li for hiliting and flashing
  \li to "drop" the element to primitive elements
  \li to plot the element
  \li etc.
  <p>
  These varied purposes can all be satisfied by the same #Draw method by means of the \c context argument. In each case, a different
  type of ViewContext is created and passed to #Draw. If necessary, implementations of #Draw can query the ViewContext to
  get information about the context (e.g. by calling ViewContext::GetDrawPurpose.)
  <p>
  Ultimately, for your #Draw to output geometric primitives to make your element visible, you must call <code>context->GetIDrawGeom</code> to get
  the "draw output" interface. The IDrawGeom interface provides the methods for drawing geometry and for setting the material/symbology.
  From within your #Draw method you can draw as much geometry as necessary and whatever you draw will be displayed in the Viewport.
  @note
  Generally, implementations of #Draw for elements that store their symbology in the display header need not worry about
  setting the active ElemDisplayParams, since the context will have already established it before calling #Draw. Only advanced DisplayHandlers
  that have multiple parts with different symbology need to change the ElemDisplayParams.

  <h2>Caching versus Immediate Mode drawing</h2>

  Calling one of the DrawXXX methods of the IDrawGeom interface will result in geometry displayed in the Viewport. This is
  known as \e immediate mode drawing. Immediate mode drawing is usually fast, efficient and sufficient. However, certain types of elements with complicated
  draw methods (e.g. requiring calculations or lookups), or elements with lots of geometry can be slow to draw. For this reason, it is possible to
  have MicroStation retain a "cached" representation of the geometry that will be re-used. Also, certain types of geometry,
  e.g. bspline surfaces, can \b only be drawn by first creating a cache representation. MicroStation will automatically store the cached representation
  and delete it when it is no longer needed (i.e., the element changes or the DgnModel holding the element is deleted.)
  <p>
  The tradeoff for caching is obviously memory versus performance. [N.B. For very simple geometry, caching will use some memory, but
  won't provide any performance improvement. So caching is \b not desirable in all circumstances.] The amount of memory required to store a
  cached display representation obviously varies, but it is generally relatively small. In certain cases, the performance improvements can be
  substantial, particularly with complicated Draw methods.

  To create a cached representation, applications call ViewContext::DrawCached from within their #Draw method.

 E.g.:
\code
void  MyDisplayHandler::Draw (ElementHandleCR elIter, ViewContextP context, DrawingExpense expense)
    {
    context->DrawCached (elIter, stroker, index, PICK_MODE_xxx, DrawExpense::Medium);
    }
\endcode

  Then, all of the \e work for generating the graphics for this element is performed by the "stroker" object that the context calls
  from within DrawCached. Since DrawCached saves the cached representation, it is only necessary for MicroStation to invoke the "stroker"
  code one time for a given element. All subsequent calls to DrawCached for the same element will use the
  previously cached representation.
  @note
  Some applications may wish to cache only a part of their representation, or only cache certain instances of an element type (e.g. only when
  it is complicated.) In this case, the #Draw method can be more sophisticated than the example above. For example, it could draw the "uncached"
  part of the element and/or test the conditions under which caching is necessary and then call DrawCached.

  <h2>Multiple Cached Representations</h2>

  Sometimes DisplayHandlers may desire to display differently under different circumstances (e.g. zoom levels, render modes, or other
  application-defined modes). Of course this can be accomplished by simply taking different paths through the #Draw method in Immediate mode. But,
  it is also possible to store more than one cached representation, <i>even on the same element</i>. This is accomplished via the \c cacheIndex
  argument of DrawCached. Each cache index value stores a different cached representation, and there is no limit to the number of cached
  representations for a single element (other than the memory it takes to store them.)
  <p>
  Additionally, it is sometimes desirable to cache different \e parts of an element separately. In pseudo-code, a #Draw method may look something
  like:
\code
void  MyDisplayHandler::Draw (ElementHandleCR el, ViewContextP context)
    {

    if <should draw immediate mode part>
        {
        IDrawGeomP drawGeom = context->GetIDrawGeom();
        ...
        drawGeom->DrawXXX (...);
        ...
        drawGeom->DrawXXX (...);
        ...
        }
    ...

    if <should draw cached Part 1>
        context->DrawCached (el, cachedPart1Stroker, 0, PICK_MODE_xxx, DrawExpense::Medium);

    if <should draw cached Part 2>
        context->DrawCached (el, cachedPart2Stroker, 1, PICK_MODE_xxx, DrawExpense::Medium);

    ...
    }
\endcode
  where each of the \c cachedPartXStroker objects would draw a different (cached) part of the element. Each part of the cached
  representation is only be created and/or used as it is needed.

  <h2>Symbology Overrides and Caching</h2>

  Cached representations are stored with their natural symbology. However, any <i>Override Symbology</i> (see discussion of Display Parameters at
  ViewContext) will override all of the symbology of a cached representation. In this manner it is possible to display a cached representation
  with different symbology than the symbology with which it was originally created.

  <h2>General Notes</h2>

  \li Any time an element is rewritten to a DgnModel, all of its cached representations are automatically deleted and must be
  regenerated the next time they're needed.

  @bsiclass
+===============+===============+===============+===============+===============+======*/

//__PUBLISH_SECTION_START__

struct EXPORT_VTABLE_ATTRIBUTE DisplayHandler : public Handler
{
    DEFINE_T_SUPER(Handler);
    ELEMENTHANDLER_DECLARE_MEMBERS (DisplayHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:
    virtual ~DisplayHandler() {}

virtual DisplayHandler* _GetDisplayHandler () override {return this;}

DGNPLATFORM_EXPORT virtual void _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;

DGNPLATFORM_EXPORT virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;

//! Query if this handler is suitable for specified purpose.
//! @param[in]      eh          element data. NOTE: THIS IS A POINTER, NOT A REFERENCE, AND MAY BE NULL!!!
//! @param[in]      stype       purpose handler is being considered for.
//! @return true if handler acceptable for purpose, false otherwise.
DGNPLATFORM_EXPORT virtual bool _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

//! Transforms public linkages and then recomputes element range.
DGNPLATFORM_EXPORT virtual StatusInt _OnTransformFinish (EditElementHandleR, TransformInfoCR) override;

//! Applies fence stretch to public linkages and then recomputes element range.
DGNPLATFORM_EXPORT virtual StatusInt _OnFenceStretchFinish (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;

DGNPLATFORM_EXPORT virtual void _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

//! Draw the presentation for this element into a ViewContext. This method is called <i>every time</i> an element's graphical
//! representation is needed by MicroStation. For this reason, implementations should generally be as efficient as possible.
//! If your implementation is complicated, you should consider caching the time consuming work by calling
//! ViewContext::DrawCached for all or parts of the representation.

//! As mentioned in the overview of this section, this method is used for many purposes.
//! The various implementations of ViewContext return different IDrawGeoms that perform different operations as needed.
//! Typically implementations of Draw perform the same tasks, regardless of the reason for invocation. But, if necessary applications can
//! differentiate the various calls to Draw by querying the context (e.g. calling ViewContext::GetDrawPurpose.)
//! @param[in]      el          element to process
//! @param[in]      context     context to use for drawing
//! @note     Implementations always need to call <code>context->GetIDrawGeom()</code> to get an IDrawGeom interface to output graphics.
//!           If your implementation doesn't call any methods in IDrawGeom, your element is not visible.
DGNPLATFORM_EXPORT virtual void _Draw (ElementHandleCR el, ViewContextR context);

//! Draw the geometry for the supplied display path. Allows extended element handlers to participate in flashing component geometry.
//! @return SUCCESS if handled or ERROR to use ViewContext::VisitElemRef.
//! @bsimethod
DGNPLATFORM_EXPORT virtual BentleyStatus _DrawPath (DisplayPathCR, ViewContextR);

DGNPLATFORM_EXPORT virtual void                _DrawFiltered (ElementHandleCR el, ViewContextR context, DPoint3dCP pts, double size);
DGNPLATFORM_EXPORT virtual bool                _FilterLevelOfDetail (ElementHandleCR, ViewContextR);
DGNPLATFORM_EXPORT virtual SnapStatus          _OnSnap (SnapContextP context, int snapPathIndex);
DGNPLATFORM_EXPORT virtual StatusInt           _EvaluateCustomKeypoint (ElementHandleCR elHandle, DPoint3dP outPointP, byte* customKeypointData);
DGNPLATFORM_EXPORT virtual void                _GetPathDescription (ElementHandleCR el, WStringR string, DisplayPathCP path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiter);
DGNPLATFORM_EXPORT virtual bool                _IsRenderable (ElementHandleCR);
DGNPLATFORM_EXPORT virtual bool                _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal);
DGNPLATFORM_EXPORT virtual bool                _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass);
DGNPLATFORM_EXPORT virtual void                _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false);
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle);
DGNPLATFORM_EXPORT virtual void                _GetTransformOrigin (ElementHandleCR, DPoint3dR);
DGNPLATFORM_EXPORT virtual void                _GetSnapOrigin (ElementHandleCR, DPoint3dR);
DGNPLATFORM_EXPORT virtual void                _GetOrientation (ElementHandleCR, RotMatrixR);
DGNPLATFORM_EXPORT virtual IAnnotationHandlerP _GetIAnnotationHandler (ElementHandleCR);
DGNPLATFORM_EXPORT virtual ScanTestResult      _DoScannerTests (ElementHandleCR eh, BitMaskCP levels, UInt32 const* classMask, ViewContextP) override;
DGNPLATFORM_EXPORT virtual ICurvePathQuery*    _CastToCurvePathQuery(); // (new in graphite)

virtual bool _DisplayHandlerDummy1(void*) {return false;}
virtual bool _DisplayHandlerDummy2(void*) {return false;}
virtual bool _DisplayHandlerDummy3(void*) {return false;}

DGNPLATFORM_EXPORT BentleyStatus CalculateDefaultRange (EditElementHandleR elHandle);
DGNPLATFORM_EXPORT StatusInt EvaluateDefaultSnap (ElementHandleCR elHandle, DPoint3dR outPoint, DisplayPathCP dp1, DisplayPathCP dp2, AssocPoint const& assoc, PersistentSnapPathCR snapPath);
//DGNPLATFORM_EXPORT StatusInt GetDefaultCutGraphicsPatternParams (PatternParamsR params, ElementHandleCR eh, DisplayPathCP displayPath, ICutPlaneR cutPlane, ViewContextP); removed in graphite

//! Utility method to query/edit the element thickness property. Should not be called
//! directly, used by EditProperties/QueryProperties implementations.
DGNPLATFORM_EXPORT static void QueryThicknessProperty (ElementHandleCR eh, PropertyContextR context);
DGNPLATFORM_EXPORT static void EditThicknessProperty (EditElementHandleR eeh, PropertyContextR context);

public:

//! Utility method to get the point at the center of this element's current range cube. The range itself is not calculated, it
//! is merely extracted from the element. So it must be valid for the point to be valid.
DGNPLATFORM_EXPORT void GetRangeCenter (ElementHandleCR el, DPoint3dR origin);

//! Draw the presentation for this element into a ViewContext.
DGNPLATFORM_EXPORT void Draw (ElementHandleCR el, ViewContextR context);

//! Draw the geometry for the supplied display path. Allows extended element handlers to participate in flashing component geometry.
//! @return SUCCESS if handled or ERROR to use ViewContext::VisitElemRef.
//! @bsimethod
DGNPLATFORM_EXPORT BentleyStatus DrawPath (DisplayPathCR, ViewContextR);

//DGNPLATFORM_EXPORT StatusInt DrawCut (ElementHandleCR el, ICutPlaneR, ViewContextR context); removed in graphite
//DGNPLATFORM_EXPORT StatusInt GetCutGraphicsPatternParams (PatternParamsR params, ElementHandleCR solidEh, DisplayPathCP displayPath, ICutPlaneR cutPlane, ViewContextP context); removed in graphite

//! Draw a "filtered by level of detail" representation of this element. The element has already been determined to be too small
//!  to be of detailed interest by a previous call to #FilterLevelOfDetail. Presumably the filtered presentation will be faster than
//!  the unfiltered presentation.
//! @param[in]      el          Element to process
//! @param[in]      context     Context to use for drawing
//! @param[in]      pts         Range points from this element that can be used to draw a "squiggle", 4 points for 2d, 8 points for 3d.
//! @param[in]      size        Rough size of pts in view coordinates
//! @remarks The counter-clockwise point order is 0, 1, 3, 2 (and 4, 5, 7, 6 for 3d elements).
DGNPLATFORM_EXPORT void DrawFiltered (ElementHandleCR el, ViewContextR context, DPoint3dCP pts, double size);

//! Test whether this element is large enough to be displayed on "level of detail" criteria for the supplied context.
DGNPLATFORM_EXPORT bool FilterLevelOfDetail (ElementHandleCR el, ViewContextR context);

//! Setup snap path for this element using supplied snap context. This method is
//! called whenever locates are done on this element or its components; this includes both
//! tentative and AccuSnap. Before this method is called the element would first have
//! been asked to draw itself. Display handlers
//! can test for DrawPurpose::Pick to specialize what gets located or to add additional
//! information to describe what each piece drawn represents using
//! context->GetIPickGeom()->SetElemTopology. After drawing, the supplied ISnapContexP will
//! contain a SnapPath that represents "nearest" snap; it is the handler's responsibility
//! to adjust this for the current snap mode, reject the snap, or pass along the
//! responsibility to the next component element in the path.
//! @param[in]      context             Holds current snap information including snap path.
//! @param[in]      snapPathIndex       Which index in snap path represents this handler's elementRef.
//! @return SnapStatus::Success if snap accepted or SnapStatus::NotSnappable if snap not allowed.
DGNPLATFORM_EXPORT SnapStatus OnSnap (SnapContextP context, int snapPathIndex);

//! Evaluate custom keypoint data supplied in OnSnap and return a location.
//! @param[in]      el                  Element that generated the snap.
//! @param[out]     outPointP           Location of snap.
//! @param[in]      customKeypointData  Data supplied by handler for the snap.
//! @return SUCCESS if location returned for customKeypointData or ERROR.
DGNPLATFORM_EXPORT StatusInt EvaluateCustomKeypoint (ElementHandleCR el, DPoint3dP outPointP, byte* customKeypointData);

//! Evaluate a snap
//! @remarks The default implementation of this function calls EvaluateCustomKeypoint for CUSTOM_ASSOC.
//! @param[in]      elHandle            The element that is the target of the snap. In the case of a two-element snap, such as Intersection, elHandle is the first of the two elements involved.
//! @param[out]     outPoint            Computed snap point.
//! @param[in]      dp1                 Path to the first element involved in the snap
//! @param[in]      dp2                 Optional: path to the second element involved in the snap, if any
//! @param[in]      assoc               Basic snap definition data
//! @param[in]      snapPath            The complete snap definition
//! @return SUCCESS if location returned or ERROR.
//! @see EvaluateCustomKeypoint
DGNPLATFORM_EXPORT StatusInt EvaluateSnap (ElementHandleCR elHandle, DPoint3dR outPoint, DisplayPathCP dp1, DisplayPathCP dp2, AssocPoint const& assoc, PersistentSnapPathCR snapPath);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

public:
//  These methods provide a temporary workaround to the Android dynamic_cast problem.
DGNPLATFORM_EXPORT ICurvePathQuery* CastToCurvePathQuery(); // added in  graphite

//! Return a full description of the path to be used to inform the user which element is under consideration.
//! @param[in]      el                  The element
//! @param[out]     string              The string to be filled with the description of the path.
//! @param[in]      path                The DisplayPath to describe.
//! @param[in]      levelStr            A character string describing the level of the element. This can be used to form the description.
//! @param[in]      modelStr            A character string describing the model of the element.
//! @param[in]      groupStr            A character string describing the groups holding the element.
//! @param[in]      delimiterStr        A character string used to delimit the components of the description (examples: ", " or "\n")
DGNPLATFORM_EXPORT void GetPathDescription (ElementHandleCR el, WStringR string, DisplayPathCP path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr);

//! Get the Transform Origin for this element.
//!<p> The concept of an "origin" of a displayable element does not have a mathematical definition. Instead, every
//! displayable element defines what its "origin" means. It is important to realize that the elements' "origins" are
//! not necessarily geometrically significant points. In fact, while in general the origin(s) of an element have some meaning significant
//! to the element, the origin point may not be on or even near the element.
//!<p> There are two general intended purposes for the concept of an origin. First, tools and applications sometimes wish to find the "beginning"
//! of the element, if there is such a point. This is generally the intent of the "origin" snap mode, and applications can find that
//! point by calling the GetSnapOrigin method. The second intended purpose for "origin" is a point that can be used as the invariant point
//! for geometric transformations. That point can be found via this method.
//!<p> For example, MicroStation Arc elements return the start point of the arc for GetSnapOrigin and the center of the arc for GetTransformOrigin.
//! MicroStation elements that define a curve generally return the first point on the curve for GetSnapOrigin and the path centroid for GetTransformOrigin.
//! MicroStation Text elements return the "user" origin for both GetSnapOrigin and GetTransfromOrigin.
//! @param[in]      el                  Element for origin
//! @param[out]     origin              The transform origin.
DGNPLATFORM_EXPORT void GetTransformOrigin (ElementHandleCR el, DPoint3dR origin);

//! Get the snap origin for this element.
//! @param[in]      el              Element for origin
//! @param[out]     origin          The snap origin (the point that the user will discover using snap-mode-origin on this element.)
//! @remarks see discussion of origins in GetTransformOrigin.
DGNPLATFORM_EXPORT void GetSnapOrigin (ElementHandleCR el, DPoint3dR origin);

//! Get an "orientation" RotMatrix for this element. The orientation matrix will be the "natural" orientation of the element. Often this
//! matrix is used in combination with the GetTransformOrigin for relating this element to others or for visible feedback.
//! @note If the element has no natural orientation, it will return the identity matrix.
DGNPLATFORM_EXPORT void GetOrientation (ElementHandleCR el, RotMatrixR orientation);

//! Determine whether this element is renderable (i.e., subject to lighting). This test also determines whether the element
//! is subject to black/white reversal against the view backgound color.
//! @param[in]      el              Element to test
//! @return true if the element is renderable
DGNPLATFORM_EXPORT bool IsRenderable (ElementHandleCR el);

//!Test whether this element is planar. Callers should be aware that the answer may be expensive to determine.
//! @param[in]      el                  Element to test
//! @param[out]     normal              The normal bvector. Only valid if the method returns true. May be NULL.
//! @param[out]     point               A point on the plane. Only valid if the method returns true. May be NULL.
//! @param[in]       inputDefaultNormal  A normal bvector to be used in the case where the element does not define a plane (a line, for example).
DGNPLATFORM_EXPORT bool IsPlanar (ElementHandleCR el, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal);

//! Determine whether this element is visible in the supplied context.
//! @param[in]      el                  Element to test
//! @param[in]      context             Context for visibility testing
//! @param[in]      testRange           If true, test the element for visibility on range criteria. If false, do not reject element on range criteria
//! @param[in]      testLevel           If true, test the element for visibility on level criteria. If false, do not reject element on level criteria
//! @param[in]      testClass           If true, test the element for visibility on element class criteria. If false, do not reject element on class criteria
//! @return true if the element is visible in the context, false if it is invisible.
DGNPLATFORM_EXPORT bool IsVisible (ElementHandleCR el, ViewContextR context, bool testRange, bool testLevel, bool testClass);

//! Drop this element to a group of (simpler) primitive elements.
//! @param[in]      eh                  Element to drop
//! @param[out]     dropGeom            Element agenda holding the result of the drop operation.
//! @param[in]      geometry            Options to determine how drop should be handled
//! @return SUCCESS if element was dropped and dropGeom is valid
DGNPLATFORM_EXPORT StatusInt Drop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry);

//__PUBLISH_SECTION_END__

//! convert a DRange3d into two DPoint2ds.
//! @param[out]     range               Two DPoint2ds, min/max.
//! @param[in]      scanRange           Input scan range
DGNPLATFORM_EXPORT static DPoint2dP GetDPRange (DPoint2dP range, DRange3dCP scanRange);

//! Get the ElemDisplayParams from this element.
//! @param[in]      el                  Element to use
//! @param[out]     displayParams       The ElemDisplayParams extracted from this element.
//! @param[in]      wantMaterials       If false, do not bother to set the material information in displayParams, it is not needed
//! @bsimethod
DGNPLATFORM_EXPORT void GetElemDisplayParams (ElementHandleCR el, ElemDisplayParams& displayParams, bool wantMaterials=false);

//! @return true if this element is a 3d element
static bool Is3dElem (DgnElementCP el) {return el->Is3d();}

//! Recalculate and update the element range member of this element.
//! @param[in,out]      el              Element whose range is validated
//! @return SUCCESS if the range was successfully caclulated and updated. ERROR means that the element is off the design plane.
DGNPLATFORM_EXPORT BentleyStatus ValidateElementRange (EditElementHandleR el);

//! Recalculate and update the element range member of this view independent element. View independent elements have ranges that
//! are large enough to encompass the element from any view orientation.
//! @param[in,out]      el              Element whose range is validated
//! @return SUCCESS if the range was successfully caclulated and updated. ERROR means that the element is off the design plane.
DGNPLATFORM_EXPORT BentleyStatus ValidateViewIndependentElementRange (EditElementHandleR el);

//! Calculate and return this element's range.
//! @param[in]      el                  The element whose range is to be calculated. The input element is NOT updated.
//! @param[out]     range               The calculated range of el.
//! @param[in]      transform           The coordinate system to compute the range in (NULL for active).
//! @return SUCCESS if range is valid, ERROR if the element's range is outside of the design range
DGNPLATFORM_EXPORT BentleyStatus CalcElementRange (ElementHandleCR el, DRange3dR range, TransformCP transform);

//! Query if the element can be an annotation.
//! The element might not be an annotation now. The caller must call IAnnotationHandler methods
//! for further information.
//! @param[in]      eh                  The annotation element that will be rescaled
//! @return An IAnnotationHandler object that provides annotation behavior for this element, or NULL if the element cannot be an annotation.
DGNPLATFORM_EXPORT IAnnotationHandlerP GetIAnnotationHandler (ElementHandleCR eh);

//! Recalculate and update the element range member of this element descriptor. This function merely creates an EditElementHandle
//! on element and then calls the eponymous method on the DisplayHandler.
//! @param[in,out]      element         The element whose range is calculated and updated.
//! @return SUCCESS if range is valid, ERROR if the element's range is outside of the design range
DGNPLATFORM_EXPORT static BentleyStatus ValidateElementRange (MSElementDescrP element);

//! Recalculate and update the element range member of this DgnElement. This function is provided for convenience only and
//! should generally be avoided. It has to allocate an element descriptor, copy the DgnElement to that descriptor, call ValidateElementRange
//! on that descriptor and then copy the element from the descriptor. This is rather expensive, particularly if you already
//! have an MSElementDescr, or better yet an EditElementHandle. In both of those cases, DO NOT CALL THIS FUNCTION! Call the non-static
//! ValidateElementRange with the EditElementHandle or the static ValidateElementRange with the MSElementDescrP.
//! @param[in,out]      element         The element whose range is calculated and updated.
//! @param[in]          modelRef        The model that contains the element
//! @return SUCCESS if range is valid, ERROR if the element's range is outside of the design range
DGNPLATFORM_EXPORT static BentleyStatus ValidateElementRange (DgnElementP element, DgnModelP modelRef);

//__PUBLISH_SECTION_START__

};

//__PUBLISH_SECTION_END__


/*=================================================================================**//**
  This extension should be implemented only by DisplayHandler classes that wish to suppress
  IModel Publishing.
  @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDisplayHandlerXGraphicsPublishExtension : Handler::Extension
{
ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (IDisplayHandlerXGraphicsPublishExtension, DGNPLATFORM_EXPORT)

/*---------------------------------------------------------------------------------**//**
  Return true to suppress publishing XGraphics for this element.
  @param    eh  IN the element to be tested.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _SuppressXGraphicsPublish (ElementHandleCR eh) = 0;
};

/*=================================================================================**//**
  This extension should be implemented only by DisplayHandler classes that wish to suppress
  IModel Publishing.
  @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDisplayHandlerXGraphicsXAttributePublishExtension : Handler::Extension
{
ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (IDisplayHandlerXGraphicsXAttributePublishExtension, DGNPLATFORM_EXPORT)

/*---------------------------------------------------------------------------------**//**
  Return true to suppress removal of XAttribute from the published XGraphics element.
  @param    eh          IN the element to be tested.
  @param    handlerId   IN handler id. 
  @return   true if should be suppressed.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _SuppressXGraphicsXAttributeRemoval (ElementHandleCR eh, XAttributeHandlerId handlerId) = 0;
};  

/*=================================================================================**//**
  This extension should be implemented only by DisplayHandler classes that wish to suppress
  clash detection.
  @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDisplayHandlerClashDetectionExtension : Handler::Extension
{
ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (IDisplayHandlerClashDetectionExtension, DGNPLATFORM_EXPORT)

/*---------------------------------------------------------------------------------**//**
  Return true to suppress clash detection with this element.  This should be a relatively
  inexpensive test and is meant as an optimization  pretest that is applied prior to
  the user supplied suppression rules.  The default implementation returns false.
  @param    eh  IN the element to be tested.
  @param    thisPath the display path for the element.
  @param    otherPath IN if NULL then return true to suppress all clash detection. If
  otherPath is not NULL then return true to suppress clash detection with otherPath only.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _SuppressClashDetection (ElementHandleCR eh, DisplayPathCP thisPath, DisplayPathCP otherPath) = 0;
};

//__PUBLISH_SECTION_START__

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
