/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NotificationManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include    <Bentley/WString.h>

BEGIN_BENTLEY_DGN_NAMESPACE

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
    Utf8String            m_briefMsg;
    Utf8String            m_detailedMsg;

public:
    NotifyMessageDetails() : m_priority(OutputMessagePriority::None) {}

    //! Construct a NotifyMessageDetails
    //! @param[in]  priority        The priority this message should be accorded by the NotificationManager.
    //! @param[in]  briefMsg        A short message that conveys the simplest explanation of the issue.
    //! @param[in]  detailedMsg     A comprehensive message that explains the issue in detail and potentially offers a solution.
    //! @param[in]  msgType         The type of message.
    //! @param[in]  openAlert       Whether an alert box should be displayed or not, and if so what kind.
    NotifyMessageDetails(OutputMessagePriority priority, Utf8CP briefMsg, Utf8CP detailedMsg=NULL)
        {
        if (briefMsg)
            m_briefMsg.assign(briefMsg);

        if (detailedMsg)
            m_detailedMsg.assign(detailedMsg);

        m_priority      = priority;
        }

    OutputMessagePriority GetPriority() const { return m_priority; } //!< Get the priority value of this NotifyMessageDetails.
    Utf8StringCR GetBriefMsg() const { return m_briefMsg; } //!< Get the brief message for this NotifyMessageDetails.
    Utf8StringCR GetDetailedMsg() const { return m_detailedMsg; } //!< Get the detailed message for this NotifyMessageDetails.

    void SetPriority(OutputMessagePriority priority) { m_priority=priority; } //!< Set the priority value of this NotifyMessageDetails.
    void SetBriefMsg(Utf8CP msg)  { m_briefMsg.AssignOrClear(msg); } //!< Set the brief message for this NotifyMessageDetails.
    void SetDetailedMsg(Utf8CP msg)  { m_detailedMsg.AssignOrClear(msg);} //!< Set the detailed message for this NotifyMessageDetails.
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
public:
    //! Output a message and/or alert to the user.
    //! @param[in] message      The message details.
    //! @return SUCCESS if the message was displayed, ERROR if an invalid priority is specified.
    DGNPLATFORM_EXPORT static StatusInt OutputMessage(NotifyMessageDetails const& message);

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
