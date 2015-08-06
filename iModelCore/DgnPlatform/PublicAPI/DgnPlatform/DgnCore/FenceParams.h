/*----------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/FenceParams.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! The fence clip mode controls element acceptance criteria. A clip mode of none returns
//! only elements inside the fence when overlaps aren't requested. A clip mode of 
//! original or copy always returns overlapping elements. The actual clip behavior varies
//! according to the fence operation, ex. delete, move, copy.
// @bsiclass 
//=======================================================================================
enum class FenceClipMode
    {
    None = 0, //!< Inclusion of inside/overlapping elements controlled by overlap mode. No clipping of elements satisfying the fence criteria.
    Original = 1, //!< Include elements that overlap the fence. Tools will modify the original element.
    Copy = 3, //!< Include elements that overlap the fence. Tools will not modify the original element.
    };

//=======================================================================================
//! Class for finding elements that are inside or overlap a volume defined by an
//! extrusion of a planar region profile.
// @bsiclass 
//=======================================================================================
struct FenceParams
{
/*__PUBLISH_SECTION_END__*/
private:

// Inputs
double                  m_focalLength;
Transform               m_transform;
bool                    m_camera;
bool                    m_overlapMode;
double                  m_onTolerance;
double                  m_unusedRemove;
double                  m_zCameraLimit;

DgnViewportP            m_viewport;

FenceClipMode           m_clipMode;
bool                    m_clipOwned;
ClipVectorPtr           m_clip;
DRange3d                m_fenceRange;

LocateSurfacesPref      m_locateInteriors;
bool                    m_ignoreTreatAsElm;
bool                    m_checkScanCriteria;

// Outputs
bool                    m_hasOverlaps;
bvector<double>         m_splitParams;

bool                            CurveClipPlaneIntersect (ClipPrimitiveCR, MSBsplineCurve*, double clipDistance);
bool                            ClipPlaneArcIntersect (ClipPrimitiveCR, double z, DEllipse3dCR);
bool                            ArcIntersect (DPoint2dCP, DEllipse3dCR,  ClipPrimitiveCR);
bool                            AcceptTransformedCurve (MSBsplineCurveP);
bool                            AcceptCameraCurveSegment (MSBsplineCurveP);
bool                            LinearFenceIntersect (ClipPrimitiveCR, DPoint3dCP, size_t numPoints, bool closed);
bool                            ArcFenceIntersect (ClipPrimitiveCR, DEllipse3dCR);
bool                            CurveFenceIntersect (ClipPrimitiveCR, MSBsplineCurveP);

public:

DGNPLATFORM_EXPORT              FenceParams (TransformCP = NULL);
DGNPLATFORM_EXPORT              FenceParams (FenceParamsP);
DGNPLATFORM_EXPORT              ~FenceParams ();

double*                         GetSplitParamsP () {return m_splitParams.empty() ? NULL : &m_splitParams.front();}
DGNPLATFORM_EXPORT size_t       GetNumSplitParams () const;
DGNPLATFORM_EXPORT bool         GetSplitParam (size_t i, double &value) const;
DGNPLATFORM_EXPORT void         SortSplitParams ();

void                            SetHasOverlaps (bool hasOverlaps) {m_hasOverlaps = hasOverlaps;}
bool                            GetCheckScanCriteria () {return m_checkScanCriteria;}
void                            SetCheckScanCriteria (bool check) {m_checkScanCriteria = check;}
LocateSurfacesPref              GetLocateInteriors () {return m_locateInteriors;}
DRange3dCP                      GetFenceRange () {return &m_fenceRange;}

DGNPLATFORM_EXPORT bool         PointInsideClip (DPoint3dCR testPoint);
DGNPLATFORM_EXPORT bool         PointInOtherClips (DPoint3dCR testPoint, ClipPrimitiveCP clip);
DGNPLATFORM_EXPORT ClipVectorP  ExtractClipP ();
DGNPLATFORM_EXPORT void         PushClip (ViewContextP, DgnViewportP vp, bool displayCut=true);
DGNPLATFORM_EXPORT StatusInt    PushClip (ViewContextR, TransformCP transform) const;
DGNPLATFORM_EXPORT void         ClearCurrentClip ();
DGNPLATFORM_EXPORT void         ClearSplitParams ();
DGNPLATFORM_EXPORT void         StoreIntersection (double param);
DGNPLATFORM_EXPORT bool         StoreIntersectionIfInsideClip (double param, DPoint3dP pointP, ClipPrimitiveCP intersectPrimitive);
DGNPLATFORM_EXPORT StatusInt    GetClipToWorldTransform (TransformR clipToWorld, ClipPrimitiveCR clip) const;
DGNPLATFORM_EXPORT bool         IsOutsideClip () const;

DGNPLATFORM_EXPORT bool         AcceptByCurve ();
DGNPLATFORM_EXPORT bool         AcceptCurve (MSBsplineCurveP);
DGNPLATFORM_EXPORT bool         AcceptLineSegments (DPoint3dP,size_t numPoints, bool closed);
DGNPLATFORM_EXPORT bool         AcceptDEllipse3d (DEllipse3dCR);

DGNPLATFORM_EXPORT void         ParseAcceptedElement (DgnElementPtrVec* inside, DgnElementPtrVec* outside, GeometricElementCR);

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
public:

//! Setup the fence parameters from the supplied viewport.
DGNPLATFORM_EXPORT void                 SetViewParams (DgnViewportP);

//! Set fence mode to inside or overlap for when the clip model is none.
DGNPLATFORM_EXPORT void                 SetOverlapMode (bool mode);

//! Set the fence clip mode.
DGNPLATFORM_EXPORT void                 SetClipMode (FenceClipMode);

//! Set the fence boundary by supplying a ClipVector.
DGNPLATFORM_EXPORT void                 SetClip (ClipVectorCR clip);

//! Set the current fence to root transform. This is normally set by calling SetViewParams.
DGNPLATFORM_EXPORT void                 SetTransform (TransformCR);

//! Specify whether overlap testing should detect a fence that is completely contained within a region/surface element.
DGNPLATFORM_EXPORT void                 SetLocateInteriors (LocateSurfacesPref);

//! Setup the fence clip boundary from 2d clipping points. Pass true for blockIfPossible to create CLIPBLOCK instead of CLIPSHAPE when points define a rectangle.
DGNPLATFORM_EXPORT StatusInt            StoreClippingPoints (bool outside, DPoint2dP, int nPoints);

//! Setup the fence clip boundary from a clip vector that represents a planar region or simple extruded volume.
DGNPLATFORM_EXPORT StatusInt            StoreClippingVector (ClipVectorCR clip, bool outside);

//! Return 2d clipping points suitable for calling StoreClippingPoints from 3d input points in the coordinates of the view's root model.
DGNPLATFORM_EXPORT void                 ClippingPointsFromRootPoints (DPoint2dP, DPoint3dP, int numPts, DgnViewportP);

//! Return true if fence was defined by 2d points in a camera view.
DGNPLATFORM_EXPORT bool                 IsCameraOn () const;

//! Return camera focal length, only valid when IsCameraOn returns true.
DGNPLATFORM_EXPORT double               GetFocalLength () const;

//! Return a pointer to the fence's clip boundary.
DGNPLATFORM_EXPORT ClipVectorPtr        GetClipVector () const;

//! Return current fence clip mode.
DGNPLATFORM_EXPORT FenceClipMode        GetClipMode () const;

//! Return current fence viewport.
DGNPLATFORM_EXPORT DgnViewportP         GetViewport () const;

//! Return the target model for fence viewport. Used as destination for copy with clip.
DGNPLATFORM_EXPORT DgnModelP            GetDgnModel () const;

//! Return current fence to root transform.
DGNPLATFORM_EXPORT TransformP           GetTransform ();

// Return true if the supplied point is inside the fence.
DGNPLATFORM_EXPORT bool                 PointInside (DPoint3dCR);

//! Return true if AcceptElement detected an overlap.
DGNPLATFORM_EXPORT bool                 HasOverlaps () const;

//! Return true if fence overlap mode is set.
DGNPLATFORM_EXPORT bool                 AllowOverlaps () const;

//! Return true if the given element satisfies the fence criteria.
DGNPLATFORM_EXPORT bool                 AcceptElement (GeometricElementCR);

//! @param[out] contents The list of elements that satisfied the fence criteria.
//! @return SUCCESS If one or more elements were found that satisfied the fence criteria.
//! @note Populates contents with un-clipped elements that satisfy the fence criteria. Set overlap mode to false and clip mode to none to 
//!       collect only those elements completely inside the fence, otherwise both inside and overlapping elements are collected.
DGNPLATFORM_EXPORT BentleyStatus        GetContents (DgnElementIdSet& contents);

//! Create a new instance of FenceParams.
DGNPLATFORM_EXPORT static FenceParamsP  Create ();

//! Frees a FenceParams instance.
DGNPLATFORM_EXPORT static void          Delete (FenceParamsP);

}; // FenceParams

END_BENTLEY_DGN_NAMESPACE


