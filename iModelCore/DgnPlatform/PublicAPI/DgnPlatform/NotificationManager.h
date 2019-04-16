/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include    <Bentley/WString.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//! Message Types for NotificationManager::OutputMessage
enum class OutputMessageType
{
    Toast       = 0,
    Pointer     = 1,
    Sticky      = 2,
    InputField  = 3,
    Alert       = 4     //!< Modal
};

//! Relative Position for NotifyMessageDetails::SetPointerTypeDetails
enum class RelativePosition
{
    Left        = 0,
    Top         = 1,
    Right       = 2,
    Bottom      = 3,
    TopLeft     = 4,
    TopRight    = 5,
    BottomLeft  = 6,
    BottomRight = 7
};

//! Reason for ending the activity message via NotificationManager::EndActivityMessage
enum class ActivityMessageEndReason
{
    Completed   = 0,
    Cancelled   = 1
};

//=======================================================================================
//! Specifies the details of a message to be displayed to the user through the NotificationManager.
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct           NotifyMessageDetails
{
//__PUBLISH_SECTION_END__
    friend struct NotificationManager;

//__PUBLISH_SECTION_START__
private:
    OutputMessagePriority m_priority;
    OutputMessageAlert    m_openAlert;
    OutputMessageType     m_msgType;
    Utf8String            m_briefMsg;
    Utf8String            m_detailedMsg;
    int32_t               m_displayTime;

    // OutputMessageType.Pointer type message details
    DgnViewportCP         m_viewport;
    Point2d               m_displayPoint;
    RelativePosition      m_relativePosition;

public:
    NotifyMessageDetails() : m_priority(OutputMessagePriority::None), m_openAlert(OutputMessageAlert::None), m_msgType(OutputMessageType::Toast), m_viewport(nullptr), m_displayTime(3500) {}

    //! Construct a NotifyMessageDetails
    //! @param[in]  priority        The priority this message should be accorded by the NotificationManager.
    //! @param[in]  briefMsg        A short message that conveys the simplest explanation of the issue.
    //! @param[in]  detailedMsg     A comprehensive message that explains the issue in detail and potentially offers a solution.
    //! @param[in]  msgType         The type of message.
    //! @param[in]  openAlert       Whether an alert box should be displayed or not, and if so what kind.
    NotifyMessageDetails(OutputMessagePriority priority, Utf8CP briefMsg, Utf8CP detailedMsg=NULL, OutputMessageType msgType=OutputMessageType::Toast, OutputMessageAlert openAlert=OutputMessageAlert::None)
        {
        if (briefMsg)
            m_briefMsg.assign(briefMsg);

        if (detailedMsg)
            m_detailedMsg.assign(detailedMsg);

        m_priority      = priority;
        m_openAlert     = openAlert;
        m_msgType       = msgType;

        m_displayTime = 3500;   // Default to 3.5 seconds
        m_viewport = nullptr;
        m_displayPoint.Init(0, 0);
        m_relativePosition = RelativePosition::TopRight;
        }

    OutputMessagePriority GetPriority() const { return m_priority; } //!< Get the priority value of this NotifyMessageDetails.
    OutputMessageType GetMsgType() const { return m_msgType; } //!< Get the message type of this NotifyMessageDetails.
    OutputMessageAlert GetOpenAlert() const { return m_openAlert; } //!< Get the OpenAlert value of this NotifyMessageDetails.
    Utf8StringCR GetBriefMsg() const { return m_briefMsg; } //!< Get the brief message for this NotifyMessageDetails.
    Utf8StringCR GetDetailedMsg() const { return m_detailedMsg; } //!< Get the detailed message for this NotifyMessageDetails.
    int32_t GetDisplayTime() const { return m_displayTime; } //!< Get the display time of a Toast or Pointer type message.

    void SetPriority(OutputMessagePriority priority) { m_priority=priority; } //!< Set the priority value of this NotifyMessageDetails.
    void SetMsgType(OutputMessageType msgType) { m_msgType=msgType; } //!< Set the message type of this NotifyMessageDetails.
    void SetOpenAlert(OutputMessageAlert openAlert) { m_openAlert=openAlert; } //!< Set the OpenAlert value of this NotifyMessageDetails.
    void SetBriefMsg(Utf8CP msg)  { m_briefMsg.AssignOrClear(msg); } //!< Set the brief message for this NotifyMessageDetails.
    void SetDetailedMsg(Utf8CP msg)  { m_detailedMsg.AssignOrClear(msg);} //!< Set the detailed message for this NotifyMessageDetails.
    void SetDisplayTime(int32_t displayTime)  { m_displayTime = displayTime;} //!< Set the display time, in milliseconds, for a Toast or Pointer type message.

    //! Set OutputMessageType.Pointer message details.
    //! @param[in]  viewport            Viewport over which to display the Pointer type message.
    //! @param[in]  displayPoint        Point at which to display the Pointer type message.
    //! @param[in]  relativePosition    Position relative to displayPoint at which to display the Pointer type message.
    void SetPointerTypeDetails (DgnViewportCP viewport, Point2dCR displayPoint, RelativePosition relativePosition=RelativePosition::TopRight)
        {
        m_viewport = viewport;
        m_displayPoint = displayPoint;
        m_relativePosition = relativePosition;
        }

    DgnViewportCP GetPointerTypeViewport() const { return m_viewport; } //!< Get the OutputMessageType.Pointer viewport of this NotifyMessageDetails.
    Point2d GetPointerTypeDisplayPoint() const { return m_displayPoint; } //!< Get the OutputMessageType.Pointer display point of this NotifyMessageDetails.
    RelativePosition GetPointerTypeRelativePosition() const { return m_relativePosition; } //!< Get the OutputMessageType.Pointer relative position of this NotifyMessageDetails.
};


//=======================================================================================
//! Specifies the details of an activity message to be displayed to the user.
// @bsiclass                                                      Dan.East      10/17
//=======================================================================================
struct           ActivityMessageDetails : RefCountedBase
{
//__PUBLISH_SECTION_END__
    friend struct NotificationManager;

//__PUBLISH_SECTION_START__
private:
    bool            m_showProgressBar;
    bool            m_showPercentInMessage;
    bool            m_supportsCancellation;
    bool            m_wasCancelled;

public:
    //! Construct a ActivityMessageDetails
    //! @param[in]  showProgressBar         Indicates whether to show the progress bar in the activity message dialog.
    //! @param[in]  showPercentInMessage    Indicates whether to show the percentage complete in the activity message text.
    //! @param[in]  supportsCancellation    Indicates whether to show the Cancel button, giving the user the ability to cancel the operation.
    ActivityMessageDetails(bool showProgressBar, bool showPercentInMessage, bool supportsCancellation)
        {
        m_showProgressBar = showProgressBar;
        m_showPercentInMessage = showPercentInMessage;
        m_supportsCancellation = supportsCancellation;
        m_wasCancelled = false;
        }

    //! Destructor for ActivityMessageDetails
    virtual ~ActivityMessageDetails() {}

    bool GetShowProgressBar() const { return m_showProgressBar; } //!< Gets the property to show the progress bar in the activity message dialog.
    bool GetShowPercentInMessage() const { return m_showPercentInMessage; } //!< Gets the property to show the percentage complete in the activity message text.
    bool GetSupportsCancellation() const { return m_supportsCancellation; } //!< Gets the property to show the Cancel button, giving the user the ability to cancel the operation.

    bool WasCancelled() const { return m_wasCancelled; } //!< Determines if the activity was cancelled.

    void OnActivityCancelled() { m_wasCancelled = true;  _OnActivityCancelled(); } //!< Called from NotificationAdmin when the user cancels the activity.
    void OnActivityCompleted() { m_wasCancelled = false; _OnActivityCompleted(); } //!< Called from NotificationAdmin when the activity completes successfully.

protected:
    //! Implement if cancellation is supported.
    virtual void _OnActivityCancelled() {}

    //! Activity completed successfully.
    virtual void _OnActivityCompleted() {}
};

//=======================================================================================
//! The NotificationManager controls the interaction with the user for prompts, error messages, and alert dialogs.
//! Implementations of the NotificationManager may present the information in different ways. For example, in
//! non-interactive sessions, these messages may be saved to a log file or simply discarded.
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct NotificationManager
{
public:
    enum MessageBoxType
    {
        MESSAGEBOX_TYPE_OkCancel         = -13,
        MESSAGEBOX_TYPE_Ok               = -97,
        MESSAGEBOX_TYPE_LargeOk          = -119,
        MESSAGEBOX_TYPE_MediumAlert      = -112,
        MESSAGEBOX_TYPE_YesNoCancel      = -113,
        MESSAGEBOX_TYPE_YesNo            = -121,
    };

    enum MessageBoxIconType
    {
        MESSAGEBOX_ICON_NoSymbol          = 0,   // Means Don't draw Symbol
        MESSAGEBOX_ICON_Information       = 1,   // Lower Case i
        MESSAGEBOX_ICON_Question          = 2,   // Question Mark
        MESSAGEBOX_ICON_Warning           = 3,   // Exclamation Point
        MESSAGEBOX_ICON_Critical          = 4,   // Stop Sign
    };

    enum MessageBoxValue
    {
        MESSAGEBOX_VALUE_Apply        = 1,
        MESSAGEBOX_VALUE_Reset        = 2,
        MESSAGEBOX_VALUE_Ok           = 3,
        MESSAGEBOX_VALUE_Cancel       = 4,
        MESSAGEBOX_VALUE_Default      = 5,
        MESSAGEBOX_VALUE_Yes          = 6,
        MESSAGEBOX_VALUE_No           = 7,
        MESSAGEBOX_VALUE_Retry        = 8,
        MESSAGEBOX_VALUE_Stop         = 9,
        MESSAGEBOX_VALUE_Help         = 10,
        MESSAGEBOX_VALUE_YesToAll     = 11,
        MESSAGEBOX_VALUE_NoToAll      = 12,
    };

public:
    //! Output a prompt to the user. A 'prompt' is intended to indicate an action the user should take to proceed.
    //! @param[in] prompt       The prompt string.
    DGNPLATFORM_EXPORT static void OutputPrompt(Utf8CP prompt);

    //! Output a message and/or alert to the user.
    //! @param[in] message      The message details.
    //! @return SUCCESS if the message was displayed, ERROR if an invalid priority is specified.
    DGNPLATFORM_EXPORT static StatusInt OutputMessage(NotifyMessageDetails const& message);

    //! Output a MessageBox and wait for response from the user.
    //! @param[in] mbType       The MessageBox type.
    //! @param[in] message      The message to display.
    //! @param[in] icon         The MessageBox icon type.
    //! @return the response from the user.
    DGNPLATFORM_EXPORT static MessageBoxValue OpenMessageBox(MessageBoxType mbType, Utf8CP message, MessageBoxIconType icon);

    //! Set up for activity messages.
    //! @param[in] details      The activity message details.
    //! @return SUCCESS if the message was displayed, ERROR if an invalid priority is specified.
    DGNPLATFORM_EXPORT static StatusInt SetupActivityMessage(ActivityMessageDetails* details);

    //! Output an activity message to the user.
    //! @param[in] messageText      The message text.
    //! @param[in] percentComplete  The percentage of completion.
    //! @return SUCCESS if the message was displayed, ERROR if the message could not be displayed.
    DGNPLATFORM_EXPORT static StatusInt OutputActivityMessage(Utf8CP messageText, int32_t percentComplete);

    //! End an activity message.
    //! @param[in] reason       Reason for the end of the Activity Message.
    //! @return SUCCESS if the message was ended successfully, ERROR if the activitymessage could not be ended.
    DGNPLATFORM_EXPORT static StatusInt EndActivityMessage(ActivityMessageEndReason reason);
};

END_BENTLEY_DGN_NAMESPACE
