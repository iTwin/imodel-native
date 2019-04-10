/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/UpdatePlan.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <Bentley/BeTimeUtilities.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct CheckStop
{  
private:
    bool m_aborted;

public:
    bool InitAborted(bool val) {return m_aborted = val;}
    bool ClearAborted() {m_aborted = false; return false;}
    bool WasAborted()  {return m_aborted;}
    bool SetAborted() {m_aborted = true; return true;}
    bool AddAbortTest(bool val) {return  m_aborted |= val;}

    CheckStop() {m_aborted=false;}

    //! return true to abort the current operation.
    //! @note Overrides MUST call SetAborted or use AddAbortTest since WasAborted may be directly tested!
    virtual bool _CheckStop() {return m_aborted;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class UpdateAbort : int
{
    None          = 0,
    BadView       = 1,
    Motion        = 2,
    MotionStopped = 3,
    Keystroke     = 4,
    ReachedLimit  = 5,
    MouseWheel    = 6,
    Timeout       = 7,
    Button        = 8,
    Paint         = 9,
    Focus         = 10,
    ModifierKey   = 11,
    Gesture       = 12,
    Command       = 13,
    Sensor        = 14,
    Unknown       = 15
};

/*=================================================================================**//**
* The types of events that cause ViewContext operations to abort early.
* @bsiclass                                                     Keith.Bentley   02/04
+===============+===============+===============+===============+===============+======*/
struct StopEvents
{
    bool m_keystrokes;
    bool m_wheel;
    bool m_button;
    bool m_buttonUp;
    bool m_reset;
    bool m_resetUp;
    bool m_paint;
    bool m_focus;
    bool m_modifierKeyTransition;
    bool m_sensor;
    bool m_abortUpdateRequest;
    bool m_touchMotion;          //  Ignored unless the motion exceeds range.
    bool m_mouseMotion;
    bool m_anyEvent;
    uint32_t m_touchLimit;
    uint32_t m_numTouches;
    BentleyApi::Point2d m_touches[3];

    enum StopMask
        {
        None          = 0,
        OnKeystrokes  = 1<<0,
        OnWheel       = 1<<1,
        OnButton      = 1<<2,
        OnButtonUp    = 1<<3,
        OnReset       = 1<<4,
        OnResetUp     = 1<<5,
        OnPaint       = 1<<6,
        OnFocus       = 1<<7,
        OnModifierKey = 1<<8,
        OnTouch       = 1<<9,
        OnAbortUpdate = 1<<10,
        OnSensor      = 1<<11,   //  GPS, Gyro
        OnMouseMotion = 1<<12,   //  any mouse movement
        AnyEvent      = 1<<13,   //  includes all of the other events plus unknown events

        ForFullUpdate  = OnWheel | OnAbortUpdate | OnReset, // doesn't stop on keystrokes, data buttons, or touch
        ForQuickUpdate = ForFullUpdate | OnKeystrokes | OnButton | OnTouch,
        ForViewTransition = OnWheel | OnReset| OnKeystrokes | OnButton | OnTouch, // don't stop for "updateabort"
        };

    void Clear()
        {
        m_keystrokes = m_wheel = m_button = m_buttonUp = m_reset = m_resetUp = false;
        m_paint = m_focus = m_modifierKeyTransition = m_sensor = m_abortUpdateRequest = m_touchMotion = m_mouseMotion = m_anyEvent = false;
        m_touchLimit = 0;
        }

    StopEvents(int mask)
        {
        if (mask & AnyEvent)
            mask = -1;

        m_keystrokes            = TO_BOOL(mask & OnKeystrokes);
        m_wheel                 = TO_BOOL(mask & OnWheel);
        m_button                = TO_BOOL(mask & OnButton);
        m_buttonUp              = TO_BOOL(mask & OnButtonUp);
        m_reset                 = TO_BOOL(mask & OnReset);
        m_resetUp               = TO_BOOL(mask & OnResetUp);
        m_paint                 = TO_BOOL(mask & OnPaint);
        m_focus                 = TO_BOOL(mask & OnFocus);
        m_modifierKeyTransition = TO_BOOL(mask & OnModifierKey);
        m_sensor                = TO_BOOL(mask & OnSensor);
        m_abortUpdateRequest    = TO_BOOL(mask & OnAbortUpdate);
        m_touchMotion           = TO_BOOL(mask & OnTouch);
        m_mouseMotion           = TO_BOOL(mask & OnMouseMotion);
        m_anyEvent              = TO_BOOL(mask & AnyEvent);

        m_touchLimit = 0;
        }

    void SetTouchLimit(uint32_t limit, uint32_t numTouches, Point2dCP touches);

    // Stop when the ctrl or shift key is pressed or released.
    void SetStopOnModifierKey(bool stop) {m_modifierKeyTransition = stop;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct UpdatePlan
{
    struct AbortFlags
    {
        struct Motion
        {
            int     m_tolerance = 0;
            int     m_total = 0;
            Point2d m_cursorPos;
            void Clear() {m_total=0; m_cursorPos.x = m_cursorPos.y = 0;}

            void AddMotion(int val) {m_total += val;}
            int GetTotalMotion() {return m_total;}
            void SetCursorPos(Point2d pt) {m_cursorPos=pt;}
            void SetTolerance(int val) {m_tolerance=val;}
            int GetTolerance() {return m_tolerance;}
            Point2d GetCursorPos() {return m_cursorPos;}
        };

        StopEvents  m_stopEvents = StopEvents::ForFullUpdate;
        mutable Motion m_motion;

        void SetTouchCheckStopLimit(bool enabled, uint32_t pixels, uint32_t numberTouches, Point2dCP touches);
        void SetStopEvents(StopEvents stopEvents) {m_stopEvents = stopEvents;}
        StopEvents GetStopEvents() const {return m_stopEvents;}
        Motion& GetMotion() const {return m_motion;}
        bool WantMotionAbort() const {return 0 != m_motion.GetTolerance();}
    };

    struct TileOptions
    {
    private:
        BeTimePoint m_deadline;
        double m_scale = 1.0;
        uint32_t m_minDepth = 0;
        uint32_t m_maxDepth = 100;
    public:
        // Optional time-out for generating root tiles (those with no displayable parents). If this time is exceeded, tile generation will halt, producing a partial tile.
        // The partial tile will later be refined until it is complete.
        // Note: if the UpdatePlan has WantWait() == true, the UpdatePlan's GetQuitTime() is used instead.
        bool HasDeadline() const { return m_deadline.IsValid(); }
        BeTimePoint GetDeadline() const { return m_deadline; }
        void SetDeadline(BeTimePoint deadline) { m_deadline = deadline; }
        bool IsTimedOut() const { return HasDeadline() && GetDeadline().IsInPast(); }

        // An optional scale to apply to the computed screen size of each tile. A scale greater than 1.0 causes lower-resolution tiles to be selected. This is useful e.g. when
        // generating thumbnails (which are generally not rendered full-size) or when speed of tile generation is more important than quality.
        double GetScale() const { return m_scale; }
        void SetScale(double scale) { m_scale = scale; }

        // Optional minimum and maximum depths of tiles to select.
        uint32_t GetMinDepth() const { return m_minDepth; }
        uint32_t GetMaxDepth() const { return m_maxDepth; }
        void SetMinDepth(uint32_t depth) { m_minDepth = depth; }
        void SetMaxDepth(uint32_t depth) { m_maxDepth = depth; }
        void SetDepthRange(uint32_t minDepth, uint32_t maxDepth) { SetMinDepth(minDepth); SetMaxDepth(maxDepth); }
        void SetFixedDepth(uint32_t depth) { SetDepthRange(depth, depth); }
        bool IsWithinDepthRange(uint32_t depth) const { BeAssert(GetMinDepth() <= GetMaxDepth()); return depth >= GetMinDepth() && depth <= GetMaxDepth(); }
    };
protected:
    BeTimePoint m_quitTime;
    TileOptions m_tileOptions;
    AbortFlags m_abortFlags;
    double m_frustumScale = 1.0;
    uint32_t m_priority;
    DRange3d m_subRect;
    bool m_hasSubRect = false;
    bool m_wantDecorators = true;
    bool m_wantHilite = true;
    mutable bool m_wantWait = false;
public:
    // Scale applied to the frustum when selecting tiles. This allows selecting tiles outside of the frustum.
    double GetFrustumScale() const {return m_frustumScale;}
    void SetFrustumScale(double scale) {m_frustumScale=scale;}

    // Priority of this update (see Render::Task::Priority)
    uint32_t GetPriority() const {return m_priority;}
    void SetPriority(uint32_t val) {m_priority=val;}

    // Options controlling how tiles are selected and generated.
    TileOptions const& GetTileOptions() const { return m_tileOptions; }
    TileOptions& GetTileOptionsR() { return m_tileOptions; }

    // Conditions under which this update will abort.
    AbortFlags const& GetAbortFlags() const {return m_abortFlags;}
    AbortFlags& GetAbortFlagsR() {return m_abortFlags;}
    void ClearAbortFlags() {m_abortFlags.m_stopEvents = StopEvents::None;}
    void SetAbortFlags(AbortFlags const& flags) {m_abortFlags=flags;}

    // If true, this is a synchronous update which will return either when complete or when quite time is reached. Useful for generating thumbnails.
    bool WantWait() const {return m_wantWait;}
    void SetWait(bool val) const {m_wantWait=val;}

    // Time after which this update should stop.
    bool HasQuitTime() const {return m_quitTime.IsValid();}
    BeTimePoint GetQuitTime() const {return m_quitTime;}
    void SetQuitTime(BeTimePoint end) {m_quitTime = end;}
    bool IsTimedOut() const {return HasQuitTime() && GetQuitTime().IsInPast();}

    // An optional sub-range to update. This is used e.g. to 'heal' a rectangular portion of the screen while preserving the screen contents outside that rectangle.
    bool HasSubRect() const {return m_hasSubRect;}
    DRange3dCR GetSubRect() const {BeAssert(HasSubRect()); return m_subRect;}
    void SetSubRect(DRange3dCR rect) {m_subRect=rect; m_hasSubRect=true;}

    // If true, decorators will be invoked to add their graphics.
    bool WantDecorators() const {return m_wantDecorators;}
    void SetWantDecorators(bool want) {m_wantDecorators=want;}
    
    // If true, selection and hilite will be invoked to add their graphics.
    bool WantHilite() const {return m_wantHilite;}
    void SetWantHilite(bool want) {m_wantHilite=want;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct DynamicUpdatePlan : UpdatePlan
{
    DynamicUpdatePlan() {m_abortFlags.SetStopEvents(StopEvents::ForQuickUpdate);}
};

    DGNPLATFORM_EXPORT void AbortAll();
END_BENTLEY_DGN_NAMESPACE

