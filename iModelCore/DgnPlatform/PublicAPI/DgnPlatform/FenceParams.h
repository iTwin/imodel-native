/*----------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    None        = 0, //!< Inclusion of inside/overlapping elements controlled by overlap mode. No clipping of elements satisfying the fence criteria.
    Original    = 1, //!< Include elements that overlap the fence. Tools will modify the original element.
    Copy        = 3, //!< Include elements that overlap the fence. Tools will not modify the original element.
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FenceCheckStop
{  
//! return true to abort collecting fence contents.
virtual bool _CheckStopFenceContents() {return false;}
};

//=======================================================================================
//! Class for finding elements that are inside or overlap a volume defined by an
//! extrusion of a planar region profile.
// @bsiclass 
//=======================================================================================
struct FenceParams
{
private:
    bool                    m_overlapMode;
    double                  m_onTolerance;
    DgnViewportP            m_viewport;
    FenceClipMode           m_clipMode;
    ClipVectorPtr           m_clip;
    DRange3d                m_fenceRangeNPC;
    bool                    m_hasOverlaps;

public:
    DGNPLATFORM_EXPORT FenceParams();
    DGNPLATFORM_EXPORT FenceParams(FenceParamsP);
    DGNPLATFORM_EXPORT ~FenceParams();

    void SetHasOverlaps(bool hasOverlaps) {m_hasOverlaps = hasOverlaps;}
    DRange3d GetFenceRangeNPC() {return m_fenceRangeNPC;}

    DGNPLATFORM_EXPORT bool PointInsideClip(DPoint3dCR testPoint);
    DGNPLATFORM_EXPORT bool PointInOtherClips(DPoint3dCR testPoint, ClipPrimitiveCP clip);
    DGNPLATFORM_EXPORT ClipVectorP ExtractClipP ();
    DGNPLATFORM_EXPORT void ClearCurrentClip();
    DGNPLATFORM_EXPORT StatusInt GetClipToWorldTransform(TransformR clipToWorld, ClipPrimitiveCR clip) const;
    DGNPLATFORM_EXPORT bool IsOutsideClip() const;

    DGNPLATFORM_EXPORT bool AcceptByCurve();
    DGNPLATFORM_EXPORT bool AcceptCurve(MSBsplineCurveCR);
    DGNPLATFORM_EXPORT bool AcceptLineSegments(DPoint3dP,size_t numPoints, bool closed);
    DGNPLATFORM_EXPORT bool AcceptDEllipse3d(DEllipse3dCR);

    //! Setup the fence parameters from the supplied viewport.
    DGNPLATFORM_EXPORT void SetViewParams(DgnViewportP);

    //! Set fence mode to inside or overlap for when the clip model is none.
    DGNPLATFORM_EXPORT void SetOverlapMode(bool mode);

    //! Set the fence clip mode.
    DGNPLATFORM_EXPORT void SetClipMode(FenceClipMode);

    //! Set the fence boundary by supplying a ClipVector.
    DGNPLATFORM_EXPORT void SetClip(ClipVectorCR clip);

    //! Setup the fence clip boundary from world coordinate points defining a planar shape.
    DGNPLATFORM_EXPORT StatusInt StoreClippingPoints(DPoint3dCP, size_t nPoints, bool outside);

    //! Setup the fence clip boundary from a clip vector that represents a planar region or simple extruded volume.
    DGNPLATFORM_EXPORT StatusInt StoreClippingVector(ClipVectorCR clip, bool outside);

    //! Return a pointer to the fence's clip boundary.
    DGNPLATFORM_EXPORT ClipVectorPtr GetClipVector() const;

    //! Return current fence clip mode.
    DGNPLATFORM_EXPORT FenceClipMode GetClipMode() const;

    //! Return current fence viewport.
    DGNPLATFORM_EXPORT DgnViewportP GetViewport() const;

    // Return true if the supplied point is inside the fence.
    DGNPLATFORM_EXPORT bool PointInside(DPoint3dCR);

    //! Return true if AcceptElement detected an overlap.
    DGNPLATFORM_EXPORT bool HasOverlaps() const;

    //! Return true if fence overlap mode is set.
    DGNPLATFORM_EXPORT bool AllowOverlaps() const;

    //! Return true if we aren't looking for overlaps or clipping.
    DGNPLATFORM_EXPORT bool IsInsideMode() const;

    //! Return true if the given element satisfies the fence criteria.
    DGNPLATFORM_EXPORT bool AcceptElement(GeometrySourceCR);

    //! @param[out] contents The list of elements that satisfied the fence criteria.
    //! @param[in] checkStop Optional - For checking user events.
    //! @return SUCCESS If one or more elements were found that satisfied the fence criteria.
    //! @note Populates contents with un-clipped elements that satisfy the fence criteria. Set overlap mode to false and clip mode to none to
    //!       collect only those elements completely inside the fence, otherwise both inside and overlapping elements are collected.
    DGNPLATFORM_EXPORT BentleyStatus GetContents(DgnElementIdSet& contents, FenceCheckStop* checkStop = nullptr);

}; // FenceParams

END_BENTLEY_DGN_NAMESPACE


