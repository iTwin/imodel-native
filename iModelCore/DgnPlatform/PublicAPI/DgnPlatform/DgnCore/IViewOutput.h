/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IViewOutput.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "ImageUtilities.h"
#include "IViewDraw.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Selects the output buffer for IViewDraw methods.
// @bsiclass
//=======================================================================================
enum class DgnDrawBuffer
{
    None         = 0,                    //!< Do not draw to any buffer.
    Screen       = 1,                    //!< The visible buffer.
    Dynamic      = 2,                    //!< Offscreen, usually implemented in hardware as the "back buffer" of a double-buffered context
    BackingStore = 4,                    //!< Non-drawable offscreen buffer. Holds a copy of the most recent scene.
    Drawing      = 8,                    //!< The offscreen drawable buffer.
};

/*=================================================================================**//**
* Draw modes for displaying information in viewports.
* @bsistruct
+===============+===============+===============+===============+===============+======*/
enum class DgnDrawMode
{
    Normal    = 0,
    Erase     = 1,
    Hilite    = 2,
    TempDraw  = 3,
    Flash     = 11,
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct QvPaintOptions
{
private:
    DgnDrawBuffer m_buffer;
    DgnDrawMode   m_drawMode;
    BSIRectCP     m_subRect;
    bool          m_eraseBefore;
    bool          m_synchDrawingFromBs;
    bool          m_synchToScreen;
    bool          m_drawDecorations;
    bool          m_accumulateDirty;
    bool          m_showTransparent;
    bool          m_progressiveDisplay;
    mutable bool  m_lockVp;

public:
    QvPaintOptions(DgnDrawBuffer buffer=DgnDrawBuffer::None, BSIRectCP subRect=NULL) : m_buffer(buffer), m_subRect(subRect)
        {
        m_drawMode = DgnDrawMode::Normal;
        m_eraseBefore = true;
        m_synchDrawingFromBs = true;
        m_accumulateDirty = true;
        m_drawDecorations = false;
        m_synchToScreen = false;
        m_showTransparent = true;
        m_progressiveDisplay = false;
        m_lockVp = false;
        }
    DgnDrawBuffer GetDrawBuffer() const{return m_buffer;}
    DgnDrawMode GetDrawMode() const {return m_drawMode;}
    BSIRectCP GetSubRect() const {return m_subRect;}
    bool WantEraseBefore() const {return m_eraseBefore;}
    bool WantSynchFromBackingStore() const {return m_synchDrawingFromBs;}
    bool WantAccumulateDirty() const {return m_accumulateDirty;}
    bool WantLockVp() const {return m_lockVp;}
    bool WantSynchToScreen() const {return m_synchToScreen;}
    bool WantDrawDecorations() const {return m_drawDecorations;}
    bool WantShowTransparent() const {return m_showTransparent;}
    bool IsProgressiveDisplay() const {return m_progressiveDisplay;}
    void SetDrawBuffer(DgnDrawBuffer buffer) {m_buffer = buffer;}
    void SetDrawMode(DgnDrawMode mode) {m_drawMode = mode;}
    void SetSubRect(BSIRectCP rect) {m_subRect = rect;}
    void SetEraseBefore(bool val) {m_eraseBefore = val;}
    void SetSynchFromBackingStore(bool val) {m_synchDrawingFromBs = val;}
    void SetAccumulateDirty(bool val) {m_accumulateDirty = val;}
    void SetLockVp(bool val) const {m_lockVp = val;}
    void SetSynchToScreen(bool val) {m_synchToScreen = val;}
    void SetDrawDecorations(bool val) {m_drawDecorations = val;}
    void SetShowTransparent(bool val) {m_showTransparent = val;}
    void SetProgressiveDisplay(bool val) {m_progressiveDisplay = val;}
    bool IsHiliteMode() const {return (m_drawMode == DgnDrawMode::Hilite);}
    bool IsEraseMode() const {return (m_drawMode == DgnDrawMode::Erase);}
};


#define NPC_MIN         0.0
#define NPC_MAX         1.0

enum class AntiAliasPref
{
    Detect   = 0,
    On       = 1,
    Off      = 2,
};

enum class DrawExportFlags
{
    UseDefault          = 0,
    ClipToFrustum       = 1,
    LinesAsPolys        = 2,
    DeferTransparent    = 4,
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IViewOutput : IRefCounted, IViewDraw
{
//__PUBLISH_SECTION_END__

    friend struct HealContext;
    friend struct IndexedViewport;

//__PUBLISH_SECTION_START__
    typedef ImageUtilities::RgbImageInfo CapturedImageInfo;
//__PUBLISH_SECTION_END__

protected:
    virtual void      _SetViewAttributes (ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText) = 0;
    virtual DgnDisplayCoreTypes::DeviceContextP    _GetScreenDC () const = 0;
    virtual StatusInt _AssignDC (DgnDisplayCoreTypes::DeviceContextP) = 0;
    virtual void      _AddLights (bool    threeDview, RotMatrixCP rotMatrixP, DgnModelP model) = 0;
    virtual void      _AdjustBrightness (bool useFixedAdaptation, double brightness) = 0;
    virtual uint64_t  _GetLightStamp () = 0;
    virtual void      _DefineFrustum (DPoint3dCR frustPts, double fraction, bool is2d) = 0;
    virtual void      _SetDrawBuffer (DgnDrawBuffer drawBuffer, BSIRectCP subRect) = 0;
    virtual DgnDrawBuffer _GetDrawBuffer () const = 0;
    virtual void      _SetEraseMode (bool newMode) = 0;
    virtual StatusInt _SynchDrawingFromBackingStore () = 0;
    virtual void      _SynchDrawingFromBackingStoreAsynch () = 0;
    virtual StatusInt _SynchScreenFromDrawing () = 0;
    virtual void      _SynchScreenFromDrawingAsynch () = 0;
    virtual bool      _IsScreenDirty (BSIRect*) = 0;
    virtual void      _ShowProgress () = 0;
    virtual bool      _IsBackingStoreValid () const =0;
    virtual void      _SetBackingStoreValid (bool) = 0;
    virtual bool      _IsAccelerated () const = 0;
    virtual void      _ScreenDirtied (BSIRect const* rect) = 0;
    virtual bool      _EnableZTesting (bool yesNo) = 0;
    virtual bool      _EnableZWriting (bool yesNo) = 0;
    virtual void      _SetProjectDepth (double depth) = 0;
    virtual StatusInt _BeginDraw (bool eraseBefore) = 0;
    virtual void      _EndDraw (QvPaintOptions const&) = 0;
    virtual StatusInt _BeginDrawCapture () = 0;
    virtual StatusInt _EndDrawCapture () = 0;
    virtual bool      _HaveCapture () const = 0;
    virtual void      _ResetCapture () = 0;
    virtual StatusInt _DisplayCaptured (ViewFlags flags, DPoint2dCP origin, DPoint2dCP extent, int (*stopProc)()) = 0;
    virtual bool      _IsDrawActive () = 0;
    virtual void      _ShowTransparent () = 0;
    virtual void      _AccumulateDirtyRegion(bool val) = 0;
    virtual void      _ClearHealRegion () = 0;
    virtual void      _SetNeedsHeal (BSIRect const* dirty) = 0;
    virtual void      _HealComplete (bool aborted) = 0;
    virtual bool      _CheckNeedsHeal (BSIRect* rect) = 0;
    virtual void      _BeginDecorating (BSIRect const* rect) = 0;
    virtual void      _BeginOverlayMode () = 0;
    virtual bool      _LocateQvElem (QvElem*, DPoint2dCR borePt, double radius, DPoint3dR hitPt, DVec3dP hitNormal, int (*stopProc)(CallbackArgP), CallbackArgP arg) = 0;
    virtual void      _AbortOutstandingOperations () = 0; // Used in multithreaded case.
    virtual void      _SetIdleCallback (bool (*)(CallbackArgP userData), CallbackArgP userData) = 0; // Used in multithreaded case.
    virtual QvView*   _GetQvView () const = 0; // May return NULL
    virtual void      _SetFlashMode (bool newMode) = 0;
    virtual BentleyStatus _FillImageCaptureBuffer (bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) = 0;
    virtual int       _GetVisibleTiles(QvMRImageP mri, size_t bufSize, int* lrc) = 0;

public:
    virtual uint32_t AddRef() const = 0;
    virtual uint32_t Release() const = 0;

    DGNPLATFORM_EXPORT void SetViewAttributes (ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText);
    DGNPLATFORM_EXPORT DgnDisplayCoreTypes::DeviceContextP GetScreenDC () const;
    DGNPLATFORM_EXPORT StatusInt AssignDC (DgnDisplayCoreTypes::DeviceContextP);
    DGNPLATFORM_EXPORT void AddLights (bool threeDview, RotMatrixCP rotMatrixP, DgnModelP model = NULL);
    DGNPLATFORM_EXPORT void AdjustBrightness (bool useFixedAdaptation, double brightness);
    DGNPLATFORM_EXPORT uint64_t GetLightStamp ();
    DGNPLATFORM_EXPORT void DefineFrustum (DPoint3dCR frustPts, double fraction, bool is2d);
    DGNPLATFORM_EXPORT void SetDrawBuffer (DgnDrawBuffer drawBuffer, BSIRect const* subRect);
    DGNPLATFORM_EXPORT DgnDrawBuffer GetDrawBuffer () const;
    DGNPLATFORM_EXPORT void SetEraseMode (bool newMode);
    DGNPLATFORM_EXPORT static QvCache* GetTempElementCache ();
    DGNPLATFORM_EXPORT static void DeleteCacheElement (QvElem*);
    DGNPLATFORM_EXPORT StatusInt SynchDrawingFromBackingStore ();
    DGNPLATFORM_EXPORT void SynchDrawingFromBackingStoreAsynch ();
    DGNPLATFORM_EXPORT StatusInt SynchScreenFromDrawing ();
    DGNPLATFORM_EXPORT void SynchScreenFromDrawingAsynch ();
    DGNPLATFORM_EXPORT bool IsScreenDirty (BSIRect*);
    DGNPLATFORM_EXPORT void ShowProgress ();
    DGNPLATFORM_EXPORT bool IsBackingStoreValid () const;
    DGNPLATFORM_EXPORT bool IsAccelerated () const;
    DGNPLATFORM_EXPORT void ScreenDirtied (BSIRect const* rect);
    DGNPLATFORM_EXPORT void SetProjectDepth (double depth);
    DGNPLATFORM_EXPORT StatusInt BeginDraw (bool eraseBefore);
    DGNPLATFORM_EXPORT void EndDraw (QvPaintOptions const&);
    DGNPLATFORM_EXPORT StatusInt BeginDrawCapture ();
    DGNPLATFORM_EXPORT StatusInt EndDrawCapture ();
    DGNPLATFORM_EXPORT bool HaveCapture () const;
    DGNPLATFORM_EXPORT void ResetCapture ();
    DGNPLATFORM_EXPORT StatusInt DisplayCaptured (ViewFlags flags, DPoint2dCP origin, DPoint2dCP extent, int (*stopProc)());
    DGNPLATFORM_EXPORT bool IsDrawActive ();
    DGNPLATFORM_EXPORT void ShowTransparent ();
    DGNPLATFORM_EXPORT void AccumulateDirtyRegion(bool val);
    DGNPLATFORM_EXPORT void ClearHealRegion ();
    DGNPLATFORM_EXPORT void SetNeedsHeal (BSIRectCP dirty);
    DGNPLATFORM_EXPORT void HealComplete (bool aborted);
    DGNPLATFORM_EXPORT void BeginDecorating (BSIRectCP rect);
    DGNPLATFORM_EXPORT void BeginOverlayMode ();
    DGNPLATFORM_EXPORT bool LocateQvElem(QvElem*, DPoint2dCR borePt, double radius, DPoint3dR hitPt, DVec3dP hitNormal, int(*stopProc)(CallbackArgP), CallbackArgP arg);
    DGNPLATFORM_EXPORT void AbortOutstandingOperations (); // Used in multithreaded case.
    DGNPLATFORM_EXPORT void SetIdleCallback (bool (*)(CallbackArgP userData), CallbackArgP userData); // Used in multithreaded case.
    DGNPLATFORM_EXPORT QvView* GetQvView () const; // May return NULL
    DGNPLATFORM_EXPORT void SetFlashMode (bool newMode);
    DGNPLATFORM_EXPORT BentleyStatus FillImageCaptureBuffer (bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown);
    DGNPLATFORM_EXPORT int GetVisibleTiles(QvMRImageP mri, size_t bufSize, int* lrc);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Push a transform and/or a clip plane set
    //! @param[in]          trans           Transform to push. May be NULL.
    //! @param[in]          clipPlaneSet    Clip planes to push. May be NULL.
    //! @see #PopTransClip
    //! @bsimethod
    DGNPLATFORM_EXPORT void PushTransClip (TransformCP trans, ClipPlaneSetCP clipPlaneSet = NULL);

    //! Pop the most recently pushed transform and clipping.
    //! @see #PushTransClip
    //! @bsimethod
    DGNPLATFORM_EXPORT void PopTransClip ();

    DGNPLATFORM_EXPORT bool      EnableZTesting (bool yesNo);
    DGNPLATFORM_EXPORT bool      EnableZWriting (bool yesNo);
    DGNPLATFORM_EXPORT bool      CheckNeedsHeal (BSIRectP rect);

}; // IViewOutput

END_BENTLEY_DGNPLATFORM_NAMESPACE

