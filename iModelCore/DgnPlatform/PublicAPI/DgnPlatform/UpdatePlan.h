/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/UpdatePlan.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"

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
    bool ClearAborted() {return m_aborted = false;}
    bool WasAborted()  {return m_aborted;}
    bool SetAborted() {return m_aborted = true;}
    bool AddAbortTest(bool val) {return  m_aborted |= val;}

    CheckStop() {m_aborted=false;}

    //! return true to abort the current operation.
    //! @note Overrides MUST call SetAborted or use AddAbortTest since WasAborted may be directly tested!
    virtual bool _CheckStop() {return m_aborted;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct ProgressiveDisplay : RefCounted<NonCopyableClass>
{
    enum class Completion {Finished=0, Aborted=1, Failed=2};
    enum class WantShow : bool {No=0, Yes=1};
    virtual Completion _Process(ViewContextR, uint32_t batchSize, WantShow& showFrame) = 0;  // if this returns Finished, it is removed from the viewport
};

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   02/04
+===============+===============+===============+===============+===============+======*/
struct StopEvents
    {
    bool    m_keystrokes;
    bool    m_wheel;
    bool    m_button;
    bool    m_buttonUp;
    bool    m_paint;
    bool    m_focus;
    bool    m_modifierKeyTransition;
    bool    m_sensor;
    bool    m_abortUpdateRequest;
    bool    m_touchMotion;          //  Ignored unless the motion exceeds range.
    bool    m_anyEvent;
    uint32_t m_touchLimit;
    uint32_t m_numTouches;
    BentleyApi::Point2d m_touches[3];

    enum StopMask
        {
        None        = 0,
        OnKeystrokes  = 1<<0,
        OnWheel       = 1<<2,
        OnButton      = 1<<3,
        OnPaint       = 1<<4,
        OnFocus       = 1<<5,
        OnModifierKey = 1<<6,
        OnTouch       = 1<<7,
        OnAbortUpdate = 1<<8,
        OnSensor      = 1<<9,   //  GPS, Gyro
        OnButtonUp    = 1<<10,
        AnyEvent      = 1<<11,   //  includes all of the other events plus unknown events

        ForFullUpdate  = OnWheel | OnAbortUpdate,             // doesn't stop on keystrokes, buttons, or touch
        ForQuickUpdate = ForFullUpdate | OnKeystrokes | OnButton | OnTouch,
        };

    void Clear()
        {
        m_keystrokes = m_wheel = m_button = m_paint = m_focus = m_modifierKeyTransition = m_abortUpdateRequest = m_touchMotion = m_anyEvent = false;
        m_touchLimit = 0;
        }

    StopEvents(int mask)
        {
        if (mask & AnyEvent)
            mask = -1;

        m_keystrokes = TO_BOOL(mask & OnKeystrokes);
        m_wheel      = TO_BOOL(mask & OnWheel);
        m_button     = TO_BOOL(mask & OnButton);
        m_buttonUp   = TO_BOOL(mask & OnButtonUp);
        m_paint      = TO_BOOL(mask & OnPaint);
        m_focus      = TO_BOOL(mask & OnFocus);
        m_sensor     = TO_BOOL(mask & OnSensor);
        m_modifierKeyTransition = TO_BOOL(mask & OnModifierKey);
        m_touchMotion = TO_BOOL(mask & OnTouch);
        m_abortUpdateRequest = TO_BOOL(mask & OnAbortUpdate);
        m_anyEvent   = TO_BOOL(mask & AnyEvent);
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
    friend struct ViewSet;

    struct Query
    {
        uint32_t    m_maxTime = 2000;    // maximum time query should run (milliseconds)
        double      m_minPixelSize = 50;
        double      m_frustumScale = 1.25;
        bool        m_wait = false;
        uint32_t    m_minElements = 300;
        uint32_t    m_maxElements = 50000;
        mutable uint32_t m_delayAfter = 0;
        mutable uint32_t m_targetNumElements;

        uint32_t GetTimeout() const {return m_maxTime;}
        uint32_t GetMinElements() const {return m_minElements;}
        uint32_t GetMaxElements() const {return m_maxElements;}
        void SetMinjElements(uint32_t val) {m_minElements = val;}
        void SetMaxElements(uint32_t val) {m_maxElements = val;}
        double GetMinimumSizePixels() const {return m_minPixelSize;}
        void SetMinimumSizePixels(double val) {m_minPixelSize=val;}
        void SetTargetNumElements(uint32_t val) const {m_targetNumElements=val;}
        uint32_t GetTargetNumElements() const {return m_targetNumElements;}
        void SetWait(bool val) {m_wait=val;}
        bool WantWait() const {return m_wait;}
        uint32_t GetDelayAfter() const {return m_delayAfter;}
        void SetDelayAfter (uint32_t val) const {m_delayAfter=val;}
    };

    struct Scene
    {   
        double m_timeout = 0.0; // abort create scene after this time. If 0, no timeout
        double GetTimeout() const {return m_timeout;}
        void SetTimeout(double seconds) {m_timeout=seconds;}
    };

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

    double      m_targetFPS = 20.0; // Frames Per second
    Query       m_query;
    Scene       m_scene;
    AbortFlags  m_abortFlags;

public:
    double GetTargetFramesPerSecond() const {return m_targetFPS;}
    void SetTargetFramesPerSecond(double fps) {m_targetFPS = fps;}
    Query& GetQueryR() {return m_query;}
    Query const& GetQuery() const {return m_query;}
    Scene& GetSceneR() {return m_scene;}
    Scene const& GetScene() const {return m_scene;}
    AbortFlags const& GetAbortFlags() const {return m_abortFlags;}
    AbortFlags& GetAbortFlagsR() {return m_abortFlags;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct DynamicUpdatePlan : UpdatePlan
    {
    DynamicUpdatePlan() {m_abortFlags.SetStopEvents(StopEvents::ForQuickUpdate);}
    };

END_BENTLEY_DGN_NAMESPACE

