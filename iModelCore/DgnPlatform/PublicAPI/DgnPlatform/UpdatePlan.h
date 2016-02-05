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
    bool ClearAborted() {return m_aborted = false;}
    bool WasAborted()  {return m_aborted;}
    bool SetAborted() {return m_aborted = true;}
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

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct ProgressiveTask : RefCounted<NonCopyableClass>
{
    enum class Completion {Finished=0, Aborted=1, Failed=2};
    enum class WantShow : bool {No=0, Yes=1};
    virtual Completion _DoProgressive(struct SceneContext& context, WantShow& showFrame) = 0;  // if this returns Finished, it is removed from the viewport
};

/*=================================================================================**//**
* The types of events that cause ViewContext operations to abort early.
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
    struct Query
    {
        uint32_t    m_maxTime = 2000;    // maximum time query should run (milliseconds)
        double      m_minPixelSize = 50;
        double      m_frustumScale = 1.25;
        bool        m_onlyAlwaysDrawn = false;
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
    double      m_timeout = 0; // seconds
    Query       m_query;
    AbortFlags  m_abortFlags;

public:
    double GetTargetFramesPerSecond() const {return m_targetFPS;}
    void SetTargetFramesPerSecond(double fps) {m_targetFPS = fps;}
    Query& GetQueryR() {return m_query;}
    Query const& GetQuery() const {return m_query;}
    AbortFlags const& GetAbortFlags() const {return m_abortFlags;}
    AbortFlags& GetAbortFlagsR() {return m_abortFlags;}
    void SetTimeout(double seconds) {m_timeout=seconds;}
    double GetTimeout() const {return m_timeout;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct DynamicUpdatePlan : UpdatePlan
    {
    DynamicUpdatePlan() {m_abortFlags.SetStopEvents(StopEvents::ForQuickUpdate);}
    };


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct DgnQueryQueue
{
    //! Executes a query on a separate thread to load elements for a QueryModel
    struct Task : RefCounted<NonCopyableClass>
    {
        DgnQueryViewR m_view;
        DgnViewportCR m_vp;
        UpdatePlan::Query const& m_plan;

    public:
        Task(DgnQueryViewR view, DgnViewportCR vp, UpdatePlan::Query const& plan) : m_view(view), m_vp(vp), m_plan(plan) {}
        void Process();
        uint32_t GetDelayAfter() {return m_plan.GetDelayAfter();}
        bool IsForView(DgnQueryViewR view) const {return &m_view == &view;}
    };

    typedef RefCountedPtr<Task> TaskPtr;

private:
    enum class State { Active, TerminateRequested, Terminated };

    DgnDbR              m_db;
    BeConditionVariable m_cv;
    std::deque<TaskPtr> m_pending;
    TaskPtr             m_active;
    State               m_state;
    bool WaitForWork();
    void Process();
    THREAD_MAIN_DECL Main(void* arg);

public:
    DgnQueryQueue(DgnDbR db);

    void Terminate();

    //! Enqueue a request to execute the query for a QueryModel
    DGNPLATFORM_EXPORT void Add(Task& task);

    //! Cancel any pending requests to process the specified QueryView.
    //! @param[in] view The view whose processing is to be canceled
    DGNPLATFORM_EXPORT void RemovePending(DgnQueryViewR view);

    //! Suspends the calling thread until the specified model is in the idle state
    DGNPLATFORM_EXPORT void WaitFor(DgnQueryViewR);

    DGNPLATFORM_EXPORT bool IsIdle() const;
};

END_BENTLEY_DGN_NAMESPACE

