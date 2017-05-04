/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/View/SchedulePlayer.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningViewDefinitions.h"
#include "PlanningViewMessages.h"
#include <DgnView/DgnTool.h>

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! Interface to handle timer events (implemented by schedule player)
//=======================================================================================
struct ISchedulePlayerTimerEventHandler
{
    virtual void _HandleTimerEvent() = 0;
};

//=======================================================================================
//! Application supplied timer used by the schedule player
//=======================================================================================
struct ISchedulePlayerTimer
{ 
private:
    ISchedulePlayerTimerEventHandler * m_handler = nullptr;

protected:
    //! Implement to start the timer on request
    virtual StatusInt _StartTimer(uint32_t millisInterval, bool repeating) = 0;

    //! Implement to terminate the timer on request
    virtual StatusInt _TerminateTimer() = 0;
        
    //! Implementation should call this at every timer event
    void SendTimerEvent() { if (m_handler) m_handler->_HandleTimerEvent(); }

public:
    StatusInt StartTimer(uint32_t millisInterval, bool repeating) { return _StartTimer(millisInterval, repeating); }

    StatusInt TerminateTimer() { return _TerminateTimer(); }

    void SetTimerEventHandler(ISchedulePlayerTimerEventHandler& handler) { m_handler = &handler; }
};

//=======================================================================================
//! Type of event
//=======================================================================================
enum class EventType
    {
    Start = (0x0001 << 0),
    Finish = (0x0001 << 2),
    StartOrFinish = Start | Finish
    };

ENUM_IS_FLAGS(EventType)

//=======================================================================================
//! Represents a single channel in the schedule player
//! @see SchedulePlayer
//! @ingroup PlanningGroup
//=======================================================================================
struct ScheduleChannel : NonCopyableClass
{
    friend struct ScheduleViewController;
    friend struct SchedulePlayer;

    //=======================================================================================
    //! Settings for a specific channel in the player
    //=======================================================================================
    struct Settings
    {
        friend struct ScheduleChannel;
    private:
        PlanId m_planId;
        Utf8String m_baselineLabel;
        Dgn::DgnViewportP m_viewport;
        CameraAnimationId m_cameraAnimationId;
        DateType m_dateType = DateType::Invalid;

    public:
        //! Empty constructor
        Settings() {}

        //! Constructor
        Settings(PlanId planId, DateType dateType, Dgn::DgnViewportR viewport, Utf8CP baselineLabel=nullptr, CameraAnimationId cameraAnimationId = CameraAnimationId()) :
            m_planId(planId), m_dateType(dateType), m_viewport(&viewport), m_cameraAnimationId(cameraAnimationId)
            {
            SetBaselineLabel(baselineLabel);
            }

        //! Copy constructor
        Settings(Settings const& other) { *this = other; }

        //! Assignment operator
        Settings& operator= (Settings const& rhs)
            {
            m_planId = rhs.m_planId;
            m_baselineLabel = rhs.m_baselineLabel;
            m_viewport = rhs.m_viewport;
            m_dateType = rhs.m_dateType;
            return *this;
            }

        //! Get the plan to be played in this channel
        PlanId GetPlanId() const { return m_planId; }

        //! Set the plan to be played in this channel
        void SetPlanId(Planning::PlanId val) { m_planId = val; }

        //! Get the DateType to be played in this channel
        DateType GetDateType() const { return m_dateType; }

        //! Set the DateType to be played in this channel
        void SetDateType(DateType val) { m_dateType = val; }

        //! Get the viewport to be used by this channel
        Dgn::DgnViewportP GetViewport() const { return m_viewport; }

        //! Set the view index to be used by this channel
        void SetViewport(Dgn::DgnViewportP val) { m_viewport = val; }

        //! Get the Baseline to be played in this channel
        Utf8StringCR GetBaselineLabel() const { return m_baselineLabel; }

        //! Set the Baseline to be played in this channel
        void SetBaselineLabel(Utf8CP val) { m_baselineLabel = val ? val : ""; }

        //! Get the Id of the camera animation to be played in this channel
        CameraAnimationId GetCameraAnimationId() const { return m_cameraAnimationId; }

        //! Set the Id of the camera animation to be played in this channel
        void SetCameraAnimationId(CameraAnimationId val) { m_cameraAnimationId = val; }
    };

private:
    SchedulePlayerCR m_player;
    Dgn::DgnDbR m_dgndb;
    Dgn::DgnDbP m_referenceDb = nullptr;
    PlanCPtr m_plan;
    BaselineId m_baselineId;
    Dgn::DgnViewportP m_viewport;
    
    Settings m_channelSettings;

    Utf8String m_elementPlaybackTableNameNoPrefix;
    Utf8String m_elementPlaybackTableName;
    Utf8String m_eventPlaybackTableNameNoPrefix;
    Utf8String m_eventPlaybackTableName;

    mutable BeSQLite::Statement m_elementQueryStmt;
    mutable BeSQLite::Statement m_neverDrawnQueryStmt;
    mutable BeSQLite::Statement m_moveForwardStmt;
    mutable BeSQLite::Statement m_moveBackwardStmt;
    mutable BeSQLite::Statement m_containsEventsStmt;

    int m_backgroundColor;
    int m_backgroundTransparency;
    bool m_backgroundOverridden;

    int64_t m_startTick;
    int64_t m_finishTick;
    int64_t m_currentTick = -1000;

    double m_initialLod;
 
    ScheduleViewControllerPtr m_scheduleViewController;
    Dgn::ViewControllerPtr m_backedupViewController;

    mutable bset<Dgn::DgnClassId> m_elementGroupClassIds;

    mutable CameraAnimationPtr m_cameraAnimation;

    ScheduleChannel(SchedulePlayerCR player);
    
    BentleyStatus Initialize(ScheduleChannel::Settings const& channelSettings);
    void Finalize();

    BentleyStatus InitializeSettings();
    void FinalizeSettings();

    BentleyStatus InitializeStartAndFinishTick();
    BentleyStatus GetStartAndFinishDates(DateTimeR startDate, DateTimeR finishDate, PlanningElementCR planningElement);
    
    BentleyStatus InitializeElementPlaybackTable();
    void InitializeElementPlaybackTableName();
    Utf8StringCR GetEventPlaybackTableName() const { return m_eventPlaybackTableName; }
    void CreateElementPlaybackTable();
    void DestroyElementPlaybackTable();
    BentleyStatus PopulateElementPlaybackTable();
    Dgn::DgnElementIdSet GatherAffectedElements(Dgn::DgnElementId elementId) const;
    void PrepareElementPlaybackTableQueries();
    void FinalizeElementPlaybackTableQueries();
    void DumpElementPlaybackTable();

    BentleyStatus InitializeEventPlaybackTable();
    void InitializeEventPlaybackTableName();
    void CreateEventPlaybackTable();
    void DestroyEventPlaybackTable();
    BentleyStatus PopulateEventPlaybackTable();
    void DumpEventPlaybackTable();
    void PrepareEventPlaybackTableQueries();
    void FinalizeEventPlaybackTableQueries();
    bool ContainsEvents(int64_t startTick, int64_t finishTick) const;

    void PrepareQueries();
    void FinalizeQueries();

    void InitializeBackgroundDisplay();

    BentleyStatus InitializeCameraAnimation() const;
    void SetupCameraAnimationFrame();
    void SetupHiddenElements();

    void InitializeViewController();
    void CreateViewController();
    void ActivateViewController();
    void DeactivateViewController();
    void DestroyViewController();

    // @return false if there aren't any overrides for the element
    bool GetCurrentElementOverrides(bool& hide, int& color, int& transparency, Dgn::DgnElementId elementId) const;
    bool GetCurrentElementOverrides(bool& hide, Dgn::Render::OvrGraphicParams& symbologyOverrides, Dgn::DgnElementId elementId) const;
    void SetupNeverDrawnElements();
    void ClearNeverDrawnElements();

    void UpdateView() const;

    int64_t GetNextEventTick(bool& hasFinished, EventType nextEventType) const;
    int64_t GetPreviousEventTick(bool& hasFinished, EventType previousEventType) const;

    void StepToTick(int64_t toTick);
    void PlayToTick(int64_t toTick);

    void InitializePlayback();
    void FinalizePlayback();

    bset<Dgn::DgnClassId> GatherDerivedDgnClassIds(Dgn::DgnClassId baseClassId) const;

public: 
    ~ScheduleChannel() { Finalize(); }

    //! Get the plan that's being played in the channel
    PlanCP GetPlan() const { return m_plan.get(); }
    
    //! Get the viewport that's being used by the channel
    Dgn::DgnViewportP GetViewport() const { return m_viewport; }

    //! Get the channel settings
    Settings const& GetSettings() const { return m_channelSettings; }
    Settings& GetSettingsR() { return m_channelSettings; }

    //! Get the start tick
    int64_t GetStartTick() const { return m_startTick; }

    //! Get the finish tick
    int64_t GetFinishTick() const { return m_finishTick; }

    //! Get the current tick
    int64_t GetCurrentTick() const { return m_currentTick; }

    //! Get the camera animation for this channel
    CameraAnimationPtr GetCameraAnimation() const { return m_cameraAnimation; }

    //! Options to reload the schedule channel (based on the current settings)
    enum class ReloadOptions
        {
        ElementDisplay,
        CameraAnimation,
        All
        };

    //! Reload the schedule channel (based on the current settings)
    PLANNING_VIEW_EXPORT BentleyStatus Reload(ReloadOptions reloadOptions);

};

//=======================================================================================
//! Utility to play schedules. The player can play multiple schedules in different 
//! viewports simultaneously through ScheduleChannel-s. 
//! @see ScheduleChannel
//! @ingroup PlanningGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SchedulePlayer : Dgn::DgnViewTool, Dgn::IViewMonitor, ISchedulePlayerTimerEventHandler
{
    DEFINE_T_SUPER(Dgn::DgnViewTool)
    friend struct ScheduleChannel;

    //=======================================================================================
    //! Display settings for background elements
    //=======================================================================================
    struct BackgroundDisplaySettings
    {
    protected:
        int m_color = 0xbdbdbd;
        double m_transparency = 0.5;

    public:
        //! Get the color used to show background elements
        int GetColor() const { return m_color; }

        //! Set the color used to show active elements
        void SetColor(int val) { m_color = val; }

        //! Get the transparency used to show background elements (0.0 - Opaque, 1.0 - Transparent)
        double GetTransparency() const { return m_transparency; }

        //! Set the transparency used to show background elements (0.0 - Opaque, 1.0 - Transparent)
        void SetTransparency(double val) { m_transparency = val; }
    };

    //=======================================================================================
    //! Settings to play the schedule
    //=======================================================================================
    struct Settings
    {
        friend struct ScheduleChannel;
        friend struct SchedulePlayer;

    private:
        Dgn::DgnDbR m_dgndb;
        Dgn::DgnDbP m_referenceDb = nullptr;
        ISchedulePlayerTimer& m_timer;
        Duration m_durationPerFrame;
        BackgroundDisplaySettings m_backgroundDisplaySettings;
    
    public:
        //! Constructor
        //! @param[in] dgndb The DgnDb containing the plan and activities
        //! @param[in] timer The timer used to play the schedule
        //! @param[in] referenceDb Optionally specified DgnDb that contains the design elements created, modified or destroyed by the activities. This is 
        //! only used for cases where the activities are in a markup, and the design elements are in a different reference Db. If unspecified or null, it's assumed
        //! that the activities and the design elements are in the same DgnDb. 
        //! @remarks Caller should ensure that the lifetime of the supplied timer outlasts the lifetime of the player itself. 
        Settings(Dgn::DgnDbR dgndb, ISchedulePlayerTimer& timer, Dgn::DgnDbP referenceDb = nullptr) : m_dgndb(dgndb), m_referenceDb(referenceDb), m_timer(timer)
            {
            m_durationPerFrame = Duration(1.0, Duration::Format::Hours);
            }

        //! Copy constructor
        Settings(Settings const& other) : m_dgndb(other.m_dgndb), m_timer(other.m_timer) { *this = other; }

        //! Assignment operator
        Settings& operator= (Settings const& rhs)
            {
            m_referenceDb = rhs.m_referenceDb;
            m_durationPerFrame = rhs.m_durationPerFrame;
            m_backgroundDisplaySettings = rhs.m_backgroundDisplaySettings;
            return *this;
            }

        //! Get the duration between every update (or frame)
        void SetDurationPerFrame(DurationCR durationPerFrame) { m_durationPerFrame = durationPerFrame; }

        //! Get the duration between every update (or frame)
        DurationCR GetDurationPerFrame() const { return m_durationPerFrame; }

        //! Get the settings used to display background elements during the schedule playback
        BackgroundDisplaySettings const& GetBackgroundDisplaySettings() const { return m_backgroundDisplaySettings; }

        //! Set the settings used to display background elements during the schedule playback
        void SetBackgroundDisplaySettings(BackgroundDisplaySettings const& val) { m_backgroundDisplaySettings = val; }
    };

    //=======================================================================================
    //! Iterator over events that happen within a range of dates
    //=======================================================================================
    struct EventIterator : BeSQLite::DbTableIterator
    {
    friend struct SchedulePlayer;

    private:
        int64_t m_eventStartTick;
        int64_t m_eventFinishTick;
        Utf8StringCR m_eventPlaybackTableName;

        //! Iterates all events in the range (startTick, finishTick]
        //! @remarks Includes events exactly at finishTick, but excludes events exactly at startTick
        EventIterator(Dgn::DgnDbCR db, Utf8StringCR eventPlaybackTableName, int64_t eventStartTick, int64_t eventFinishTick) : DbTableIterator((BeSQLite::DbCR) db), 
            m_eventPlaybackTableName(eventPlaybackTableName), m_eventStartTick(eventStartTick), m_eventFinishTick(eventFinishTick)
            {
            }
        
    public:
        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            friend struct EventIterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

        public:
            //! Get the current DateTime
            PLANNING_VIEW_EXPORT DateTime GetDateTime() const;

            //! Returns true if it's a start event
            bool IsStartEvent() const { return (m_sql->GetValueInt(1) > 0); }

            //! Returns true if the event represents a start or finish of an Activity. Returns false if it's a WorkBreakdown
            bool IsActivity() const { return (m_sql->GetValueInt(2) > 0); }

            //! Gets the Id of the current Activity/WorkBreakdown
            Dgn::DgnElementId GetActivityOrWorkBreakdownId() const { return m_sql->GetValueId<Dgn::DgnElementId>(3); }

            //! Gets the label of the current Activity/WorkBreakdown
            Utf8CP GetActivityOrWorkBreakdownLabel() const { return m_sql->GetValueText(4); }

            //! Returns the iterator entry
            Entry const& operator*() const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        PLANNING_VIEW_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_stmt.get(), false); }
    }; // Iterator

    //=======================================================================================
    //! State of the player
    //=======================================================================================
    enum class PlaybackState
    {
        Stopped,
        Paused,
        Playing
    };

    //=======================================================================================
    //! Interface to track schedule events during playback
    //=======================================================================================
    struct EventListener
    {
        virtual ~EventListener() {}

        //! Called anytime events happen - i.e., Activity-s or WorkBreakdown-s have either started or finished
        virtual void _OnScheduleUpdate(SchedulePlayer::EventIterator& eventIterator, ScheduleChannelCR channel) = 0;

        //! Called when the calendar needs to be updated with the current date
        virtual void _OnCalendarUpdate(DateTimeCR currentDate, SchedulePlayerCR player) = 0;

        //! Called only when the player is started, stopped or paused
        //! @deprecated Use _OnPlaybackStateChanged instead. 
        virtual void _OnPlaybackEvent(bool isPlaying) {}

        //! Called when the state of the player has changed - i.e., the player has started playing, or has 
        //! been paused, or has been entirely stopped. 
        virtual void _OnPlaybackStateChanged(PlaybackState playbackState) = 0;
    };

private:
    Dgn::DgnDbR m_dgndb;
    Dgn::DgnDbP m_referenceDb = nullptr;

    bvector<ScheduleChannelP> m_channels;

    Settings m_playerSettings;
    Duration m_durationPerFrame;
    int64_t m_ticksPerFrame;

    int64_t m_startTick = INT64_MAX;
    int64_t m_finishTick = INT64_MIN;

    int64_t m_currentTick = -1000;

    PlaybackState m_currentPlaybackState = PlaybackState::Stopped;

    Dgn::EventHandlerList<EventListener>* m_listeners = nullptr;
    ISchedulePlayerTimer& m_timer;

    Calendar m_calendar;
    
    static int64_t DurationToTicks(DurationCR duration);

    void InitializeStartAndFinishTick();

    void InitializeEventListeners();
    void DestroyEventListeners();

    void RemoveChannel(ScheduleChannelCP channel);
    void RemoveAllChannels();

    void SendUpdateEvents(int64_t fromTick, int64_t toTick);

    int64_t GetNextEventTick(EventType nextEventType) const;
    int64_t GetPreviousEventTick(EventType previousEventType) const;

    void StepToTick(int64_t toTick);
    void PlayToTick(int64_t toTick);

    void InitializePlayback();
    void FinalizePlayback(SchedulePlayer::PlaybackState newPlaybackState);

    /* ISchedulePlayerTimerEventHandler */
    virtual void _HandleTimerEvent() override;

    /* IViewMonitor */
    PLANNING_VIEW_EXPORT virtual void _OnViewClose(Dgn::DgnViewportP viewport) override;
    void ChangeViewController(Dgn::DgnViewportR viewport, Dgn::ViewControllerR viewController) const;

    Settings const& GetSettings() const { return m_playerSettings; }

    SchedulePlayer::PlaybackState GetCurrentPlaybackState() const { return m_currentPlaybackState; }

protected:
    /* ViewTool */
    virtual ToolId _GetToolId() const override { return ToolId(PlanningViewToolIds::GetNameSpace(), PlanningViewToolIds::SchedulePlayer()); }
    PLANNING_VIEW_EXPORT virtual bool _OnDataButtonDown(Dgn::DgnButtonEventCR ev) override;
    PLANNING_VIEW_EXPORT virtual bool _OnResetButtonUp(Dgn::DgnButtonEventCR ev) override;
    PLANNING_VIEW_EXPORT virtual void _OnCleanup() override;
    PLANNING_VIEW_EXPORT virtual void _OnPostInstall() override;

    PLANNING_VIEW_EXPORT SchedulePlayer(SchedulePlayer::Settings const& playerSettings);
    PLANNING_VIEW_EXPORT ~SchedulePlayer();

public:
    //! Create a schedule player
    static SchedulePlayerPtr Create(SchedulePlayer::Settings const& playerSettings) { return new SchedulePlayer(playerSettings); }

    //! Add a channel to the schedule player - a channel plays a specific schedule in a specific viewport. 
    PLANNING_VIEW_EXPORT BentleyStatus AddChannel(ScheduleChannel::Settings const& channelSettings);

    //! Set the duration between every update (or frame)
    PLANNING_VIEW_EXPORT void SetDurationPerFrame(DurationCR durationPerFrame);

    //! Get the duration between every update (or frame)
    DurationCR GetDurationPerFrame() const { return m_durationPerFrame; }

    //! Add element-loaded-from-db event listener.
    PLANNING_VIEW_EXPORT void AddEventListener(SchedulePlayer::EventListener* listener);

    //! Drop element-loaded-from-db event listener.
    PLANNING_VIEW_EXPORT void DropEventListener(SchedulePlayer::EventListener* listener);

    //! Get the calendar used to determine the dates
    CalendarCR GetCalendar() const { return m_calendar; }

    //! Get the date the player will start playing from
    PLANNING_VIEW_EXPORT DateTime GetStartDate() const;

    //! Get the date the player will finish playing at
    PLANNING_VIEW_EXPORT DateTime GetFinishDate() const;

    //! Get the date the player is currently at
    PLANNING_VIEW_EXPORT DateTime GetCurrentDate() const;

    //! Reset the player to the start of the schedule
    PLANNING_VIEW_EXPORT void MoveToStart();

    //! Move the player to the end of the schedule
    PLANNING_VIEW_EXPORT void MoveToFinish();

    //! Move the player to the specified date/time of the schedule
    PLANNING_VIEW_EXPORT void MoveToDate(DateTime toDate);

    //! Step forward to the next step of the schedule
    //! @remarks The next step is the next (earliest) time when the specified type of event happens in any of the channels.
    PLANNING_VIEW_EXPORT void StepForward(EventType nextEventType);

    //! Step backward to the previous step of the schedule
    //! @remarks The previous step is the previous (latest) time when the specified type of event happens in any of the channels.
    PLANNING_VIEW_EXPORT void StepBack(EventType previousEventType);

    //! Play the entire schedule from start
    PLANNING_VIEW_EXPORT void Play();

    //! Stops the schedule player and resets it to the start. 
    PLANNING_VIEW_EXPORT void Stop();

    //! Pauses the schedule player at the current instant. 
    PLANNING_VIEW_EXPORT void Pause();

    //! Get channel by the view index
    //! @remarks The returned pointer should not be stored - the channel can be deleted when
    //! the view closes. 
    PLANNING_VIEW_EXPORT ScheduleChannelP GetChannelByViewport(Dgn::DgnViewportP viewport) const;
    };

END_BENTLEY_PLANNING_NAMESPACE
