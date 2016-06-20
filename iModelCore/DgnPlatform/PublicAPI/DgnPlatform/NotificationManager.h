/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NotificationManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include    <Bentley/WString.h>

BEGIN_BENTLEY_DGN_NAMESPACE

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
    long                  m_msgAttributes;
    Utf8String            m_briefMsg;
    Utf8String            m_detailedMsg;

public:
    NotifyMessageDetails() : m_priority(OutputMessagePriority::None), m_openAlert(OutputMessageAlert::None), m_msgAttributes(0) {}

    //! Construct a NotifyMessageDetails
    //! @param[in]  priority        The priority this message should be accorded by the NotificationManager.
    //! @param[in]  briefMsg        A short message that conveys the simplest explanation of the issue.
    //! @param[in]  detailedMsg     A comprehensive message that explains the issue in detail and potentially offers a solution.
    //! @param[in]  msgAttr         Any combination of the MLTEXTATTR_xxx values from dlogbox.r.h.
    //! @param[in]  openAlert       Whether an alert box should be displayed or not, and if so what kind.
    NotifyMessageDetails(OutputMessagePriority priority, Utf8CP briefMsg, Utf8CP detailedMsg=NULL, long msgAttr=0, OutputMessageAlert openAlert=OutputMessageAlert::None)
        {
        if (briefMsg)
            m_briefMsg.assign(briefMsg);

        if (detailedMsg)
            m_detailedMsg.assign(detailedMsg);

        m_priority      = priority;
        m_openAlert     = openAlert;
        m_msgAttributes = msgAttr;
        }

    OutputMessagePriority GetPriority() const { return m_priority; } //!< Get the priority value of this NotifyMessageDetails.
    OutputMessageAlert GetOpenAlert() const { return m_openAlert; } //!< Get the OpenAlert value of this NotifyMessageDetails
    long GetMsgAttributes() const { return m_msgAttributes; } //!< Get the MsgAttributes value of this NotifyMessageDetails.
    Utf8StringCR GetBriefMsg() const { return m_briefMsg; } //!< Get the brief message for this NotifyMessageDetails.
    Utf8StringCR GetDetailedMsg() const { return m_detailedMsg; } //!< Get the detailed message for this NotifyMessageDetails.

    void SetPriority(OutputMessagePriority priority) { m_priority=priority; } //!< Set the priority value of this NotifyMessageDetails.
    void SetOpenAlert(OutputMessageAlert openAlert) { m_openAlert=openAlert; } //!< Set the OpenAlert value of this NotifyMessageDetails
    void SetMsgAttributes(long attrs) { m_msgAttributes=attrs; } //!< Set the MsgAttributes value of this NotifyMessageDetails.
    void SetBriefMsg(Utf8CP msg)  { m_briefMsg.AssignOrClear(msg); } //!< Set the brief message for this NotifyMessageDetails.
    void SetDetailedMsg(Utf8CP msg)  { m_detailedMsg.AssignOrClear(msg);} //!< Set the detailed message for this NotifyMessageDetails.
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
};

END_BENTLEY_DGN_NAMESPACE
