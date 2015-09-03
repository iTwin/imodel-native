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
#include "Render.h"

BEGIN_BENTLEY_RENDER_NAMESPACE

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
struct Output : IRefCounted, ViewDraw
{
    friend struct HealContext;
    friend struct IndexedViewport;

    typedef ImageUtilities::RgbImageInfo CapturedImageInfo;

protected:
    virtual void      _SetViewAttributes(ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText) = 0;
    virtual DgnDisplayCoreTypes::DeviceContextP    _GetScreenDC() const = 0;
    virtual StatusInt _AssignDC(DgnDisplayCoreTypes::DeviceContextP) = 0;
    virtual void      _AddLights(bool threeDview, RotMatrixCP rotMatrixP, DgnModelP model) = 0;
    virtual void      _AdjustBrightness(bool useFixedAdaptation, double brightness) = 0;
    virtual uint64_t  _GetLightStamp() = 0;
    virtual void      _DefineFrustum(DPoint3dCR frustPts, double fraction, bool is2d) = 0;
    virtual void      _SetDrawBuffer(DgnDrawBuffer drawBuffer, BSIRectCP subRect) = 0;
    virtual DgnDrawBuffer _GetDrawBuffer() const = 0;
    virtual void      _SetEraseMode(bool newMode) = 0;
    virtual StatusInt _SynchDrawingFromBackingStore() = 0;
    virtual void      _SynchDrawingFromBackingStoreAsynch() = 0;
    virtual StatusInt _SynchScreenFromDrawing() = 0;
    virtual void      _SynchScreenFromDrawingAsynch() = 0;
    virtual bool      _IsScreenDirty(BSIRect*) = 0;
    virtual void      _ShowProgress() = 0;
    virtual bool      _IsBackingStoreValid() const = 0;
    virtual void      _SetBackingStoreValid(bool) = 0;
    virtual bool      _IsAccelerated() const = 0;
    virtual void      _ScreenDirtied(BSIRect const* rect) = 0;
    virtual bool      _EnableZTesting(bool yesNo) = 0;
    virtual bool      _EnableZWriting(bool yesNo) = 0;
    virtual void      _SetProjectDepth(double depth) = 0;
    virtual StatusInt _BeginDraw(bool eraseBefore) = 0;
    virtual void      _EndDraw(QvPaintOptions const&) = 0;
    virtual bool      _IsDrawActive() = 0;
    virtual void      _ShowTransparent() = 0;
    virtual void      _AccumulateDirtyRegion(bool val) = 0;
    virtual void      _ClearHealRegion() = 0;
    virtual void      _SetNeedsHeal(BSIRect const* dirty) = 0;
    virtual void      _HealComplete(bool aborted) = 0;
    virtual bool      _CheckNeedsHeal(BSIRect* rect) = 0;
    virtual void      _BeginDecorating(BSIRect const* rect) = 0;
    virtual void      _BeginOverlayMode() = 0;
    virtual void      _AbortOutstandingOperations() = 0; // Used in multithreaded case.
    virtual void      _SetIdleCallback(bool (*)(CallbackArgP userData), CallbackArgP userData) = 0; // Used in multithreaded case.
    virtual Scene*    _GetScene() = 0; // May return NULL
    virtual void      _SetFlashMode(bool newMode) = 0;
    virtual BentleyStatus _FillImageCaptureBuffer(bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) = 0;
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    virtual int       _GetVisibleTiles(MRImage* mri, size_t bufSize, int* lrc) = 0;
#endif

public:
    void SetViewAttributes(ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText) {_SetViewAttributes(viewFlags, bgColor, usebgTexture, aaLines, aaText);}
    DgnDisplayCoreTypes::DeviceContextP GetScreenDC() const {return _GetScreenDC();}
    StatusInt AssignDC (DgnDisplayCoreTypes::DeviceContextP ctx) {return _AssignDC (ctx);}
    void AddLights(bool threeDview, RotMatrixCP rotMatrixP, DgnModelP model = NULL) {_AddLights(threeDview, rotMatrixP, model);}
    void AdjustBrightness(bool useFixedAdaptation, double brightness){_AdjustBrightness(useFixedAdaptation, brightness);}
    uint64_t GetLightStamp() {return _GetLightStamp();}
    void DefineFrustum(DPoint3dCR frustPts, double fraction, bool is2d) {_DefineFrustum(frustPts, fraction, is2d);}
    void SetDrawBuffer(DgnDrawBuffer drawBuffer, BSIRect const* subRect) {_SetDrawBuffer(drawBuffer, subRect);}
    DgnDrawBuffer GetDrawBuffer() const {return _GetDrawBuffer();}
    void HealComplete(bool aborted) {_HealComplete(aborted);}

    void SetEraseMode(bool newMode) {_SetEraseMode(newMode);}
    StatusInt SynchDrawingFromBackingStore() {return _SynchDrawingFromBackingStore();}
    void SynchDrawingFromBackingStoreAsynch() {_SynchDrawingFromBackingStoreAsynch();}
    StatusInt SynchScreenFromDrawing() {return _SynchScreenFromDrawing();}
    void SynchScreenFromDrawingAsynch() {_SynchScreenFromDrawingAsynch();}
    bool IsScreenDirty(BSIRect* rect) {return _IsScreenDirty(rect);}
    void ShowProgress() {_ShowProgress();}
    bool IsBackingStoreValid() const {return _IsBackingStoreValid();}
    bool IsAccelerated() const {return _IsAccelerated();}
    void ScreenDirtied(BSIRect const* rect) {_ScreenDirtied(rect);}
    void SetProjectDepth(double depth) {_SetProjectDepth(depth);}
    StatusInt BeginDraw(bool eraseBefore) {return _BeginDraw(eraseBefore);}
    void EndDraw(QvPaintOptions const& op){_EndDraw(op);}
    bool IsDrawActive() {return _IsDrawActive();}
    void ShowTransparent() {_ShowTransparent();}
    void AccumulateDirtyRegion(bool val) {_AccumulateDirtyRegion(val);}
    void ClearHealRegion() {_ClearHealRegion();}
    void SetNeedsHeal(BSIRectCP dirty) {_SetNeedsHeal(dirty);}
    void BeginDecorating(BSIRectCP rect) {_BeginDecorating(rect);}
    void BeginOverlayMode() {_BeginOverlayMode();}
    void AbortOutstandingOperations() {_AbortOutstandingOperations();}
    void SetIdleCallback(bool (*callback)(CallbackArgP userData), CallbackArgP userData) {_SetIdleCallback(callback, userData);}
    void SetFlashMode(bool newMode) {_SetFlashMode(newMode);}
    BentleyStatus FillImageCaptureBuffer(bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) {return _FillImageCaptureBuffer(buffer, info, screenBufferRange, outputImageSize, topDown);}
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DGNPLATFORM_EXPORT int GetVisibleTiles(MRImage* mri, size_t bufSize, int* lrc);
#endif

    //! Push a transform and/or a clip plane set
    //! @param[in]          trans           Transform to push. May be NULL.
    //! @param[in]          clipPlaneSet    Clip planes to push. May be NULL.
    //! @see #PopTransClip
    void PushTransClip(TransformCP trans, ClipPlaneSetCP clipPlaneSet = NULL) {_PushTransClip(trans, clipPlaneSet);}

    //! Pop the most recently pushed transform and clipping.
    //! @see #PushTransClip
    void PopTransClip() {_PopTransClip();}

    bool EnableZTesting(bool yesNo) {return _EnableZTesting(yesNo);}
    bool EnableZWriting(bool yesNo) {return _EnableZWriting(yesNo);}
    bool CheckNeedsHeal(BSIRectP rect){return _CheckNeedsHeal(rect);}
};

END_BENTLEY_RENDER_NAMESPACE
