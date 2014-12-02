/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SnapContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "HitPath.h"
#include "ViewContext.h"
//__PUBLISH_SECTION_END__
#include "NullContext.h"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface to be used when snapping.
//! @bsiclass
//=======================================================================================
struct SnapContext : ViewContext
{
//__PUBLISH_SECTION_END__
protected:

    NullOutput      m_output;
    SnapPathP       m_snapPath; // result of the snap
    double          m_snapAperture;
    SnapMode        m_snapMode;
    int             m_snapDivisor;

    virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:
    virtual ~SnapContext () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

void SetAdjustedSnapPoint (DPoint3dCR adjustedPt);
DGNVIEW_EXPORT SnapStatus IntersectPaths (SnapPathP* snappedPath, HitPathCP first, HitPathCP second, DPoint3dCP testPoint, bool allowSelfIntersections);
DGNVIEW_EXPORT SnapStatus SnapToPath (SnapPathP* snappedPath, HitPathCP thisPath, SnapMode snapMode, int snapDivisor, double hotAperture);

DGNPLATFORM_EXPORT void SetSnapPoint (DPoint3dCR snapPt, bool forceHot);
DGNPLATFORM_EXPORT static KeypointType GetSnapKeypointType (SnapMode);

//! Get the snap path created by calling element's draw method, and
//! representing "nearest" snap.
//! @return SnapPathP won't return NULL.
//! @bsimethod
DGNPLATFORM_EXPORT SnapPathP GetSnapPath ();

//! Get the snap mode that the handler is being asked to adjust the snap path for.
//! @return SnapMode to use for this snap.
//! @bsimethod

DGNPLATFORM_EXPORT SnapMode GetSnapMode ();

//! Get the current snap divisor setting, this is used for SnapMode::NearestKeypoint.
//! @return current snap divisor.
//! @bsimethod
DGNPLATFORM_EXPORT int GetSnapDivisor ();

//! Get the default sprite used for the supplied SnapMode.
//! @param[in]    mode                  Snap mode to get sprite for.
//! @return ISpriteP default sprite.
//! @bsimethod
DGNPLATFORM_EXPORT static ISpriteP GetSnapSprite (SnapMode mode);

//! @bsimethod
DGNPLATFORM_EXPORT void GetSegmentKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment);

//! @bsimethod
DGNPLATFORM_EXPORT bool GetParameterKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor);

//! Check if element at cursor index is snappable.
//! @param[in]    snapPathIndex         Cursor index to test in snap path.
//! @bsimethod
DGNPLATFORM_EXPORT bool IsSnappableElement (int snapPathIndex);

//! Give the next component element in path an opporunity to set the current snap.
//! @param[in]    snapPathIndex         Cursor index to test in snap path.
//! @bsimethod
DGNPLATFORM_EXPORT SnapStatus DoSnapUsingNextInPath (int snapPathIndex);

//! Define the current snap information for text using default processing.
//! @param[in]    snapPathIndex         Cursor index to test in snap path.
//! @bsimethod
DGNPLATFORM_EXPORT SnapStatus DoTextSnap (int snapPathIndex);

//! Define the current snap information for this hit using default processing.
//! @param[in]    snapPathIndex         Cursor index to test in snap path.
//! @bsimethod
DGNPLATFORM_EXPORT SnapStatus DoDefaultDisplayableSnap (int snapPathIndex);

//! Define the current snap information for the path curve primitive using default processing.
//! @param[in]    snapPathIndex         Cursor index to test in snap path.
//! @param[in]    mode                  Snap mode to use.
//! @bsimethod
DGNPLATFORM_EXPORT SnapStatus DoSnapUsingCurve (int snapPathIndex, SnapMode mode);

//! Convert a point from the GeomDetail's local coordinate system (ex. GeomDetail::GetClosestPointLocal) to world coordinates.
DGNPLATFORM_EXPORT void HitLocalToWorld (DPoint3dR);

//! Convert a point from the element's local coordinate system (ex. DisplayHandler::GetTransformOrigin) to world coordinates.
DGNPLATFORM_EXPORT void ElmLocalToWorld (DPoint3dR); // WIP_V10_NO_SHARED_CELLS - Remove with shared cells.

//! Define the current snap information for this hit.
//! @param[in]    cursorIndex           Cursor index to set in snap path, this will be the element called to evaluate any customKeypointData.
//! @param[in]    mode                  Snap mode used for this snap.
//! @param[in]    sprite                Sprite to use to decorate snap.
//! @param[in]    snapPoint             Location for snap in world model coordinates (use SnapContext::HitLocalToWorld or SnapContext::ElmLocalToWorld to convert "local" point).
//! @param[in]    forceHot              true to make snap active even if cursor is not within locate tolerance of snap location.
//! @param[in]    isAdjusted            true if snap is not suitable for creating assoc points (pass false if customKeypointData is supplied or snap not overriden).
//! @param[in]    nBytes                Size in bytes of customKeypointData, or 0 if none.
//! @param[in]    customKeypointData    Pointer to customKeypointData to save for this snap or NULL.
//! @bsimethod
DGNPLATFORM_EXPORT void SetSnapInfo (int cursorIndex, SnapMode mode, ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, bool isAdjusted, int nBytes = 0, byte* customKeypointData = NULL);

}; // SnapContext

END_BENTLEY_DGNPLATFORM_NAMESPACE
