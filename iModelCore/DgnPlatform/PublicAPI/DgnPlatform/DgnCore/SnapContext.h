/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SnapContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "HitDetail.h"
#include "ViewContext.h"
#include "NullContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Interface to be used when snapping.
//=======================================================================================
struct SnapContext : ViewContext
{
protected:
    NullOutput      m_output;
    SnapDetailP     m_snapPath; // result of the snap
    double          m_snapAperture;
    SnapMode        m_snapMode;
    int             m_snapDivisor;
    virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:
    virtual ~SnapContext () {}

public:
    void SetAdjustedSnapPoint (DPoint3dCR adjustedPt);
    DGNVIEW_EXPORT SnapStatus IntersectDetails (SnapDetailP* snappedPath, HitDetailCP first, HitDetailCP second, DPoint3dCP testPoint, bool allowSelfIntersections);
    DGNVIEW_EXPORT SnapStatus SnapToPath (SnapDetailP* snappedPath, HitDetailCP thisPath, SnapMode snapMode, int snapDivisor, double hotAperture);

    DGNPLATFORM_EXPORT void SetSnapPoint (DPoint3dCR snapPt, bool forceHot);
    DGNPLATFORM_EXPORT static KeypointType GetSnapKeypointType (SnapMode);

    //! Get the snap path created by calling element's draw method, and
    //! representing "nearest" snap.
    //! @return SnapDetailP won't return NULL.
    DGNPLATFORM_EXPORT SnapDetailP GetSnapDetail ();

    //! Get the snap mode that the handler is being asked to adjust the snap path for.
    //! @return SnapMode to use for this snap.
    DGNPLATFORM_EXPORT SnapMode GetSnapMode ();

    //! Get the current snap divisor setting, this is used for SnapMode::NearestKeypoint.
    //! @return current snap divisor.
    DGNPLATFORM_EXPORT int GetSnapDivisor ();

    //! Get the default sprite used for the supplied SnapMode.
    //! @param[in]    mode                  Snap mode to get sprite for.
    //! @return ISpriteP default sprite.
    DGNPLATFORM_EXPORT static ISpriteP GetSnapSprite (SnapMode mode);

    DGNPLATFORM_EXPORT void GetSegmentKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment);

    DGNPLATFORM_EXPORT bool GetParameterKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor);

    //! Define the current snap information for text using default processing.
    DGNPLATFORM_EXPORT SnapStatus DoTextSnap ();

    //! Define the current snap information for this hit using default processing.
    DGNPLATFORM_EXPORT SnapStatus DoDefaultDisplayableSnap ();

    //! Define the current snap information for the path curve primitive using default processing.
    //! @param[in]    mode                  Snap mode to use.
    DGNPLATFORM_EXPORT SnapStatus DoSnapUsingCurve (SnapMode mode);

    //! Define the current snap information for this hit.
    //! @param[in]    mode                  Snap mode used for this snap.
    //! @param[in]    sprite                Sprite to use to decorate snap.
    //! @param[in]    snapPoint             Location for snap in world coordinates.
    //! @param[in]    forceHot              true to make snap active even if cursor is not within locate tolerance of snap location.
    //! @param[in]    isAdjusted            true if snap is not suitable for creating assoc points (pass false if customKeypointData is supplied or snap not overriden).
    //! @param[in]    nBytes                Size in bytes of customKeypointData, or 0 if none.
    //! @param[in]    customKeypointData    Pointer to customKeypointData to save for this snap or NULL.
    DGNPLATFORM_EXPORT void SetSnapInfo (SnapMode mode, ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, bool isAdjusted, int nBytes = 0, Byte* customKeypointData = nullptr);
};

END_BENTLEY_DGN_NAMESPACE
