/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/Common/PrintfProgressMeter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

BEGIN_BIM_TELEPORTER_NAMESPACE
static const wchar_t s_spinner[] = L" /-\\|";
static const size_t s_spinnerSize = _countof(s_spinner) - 1;

//=======================================================================================
// A quick and dirty progress meter.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
{
    DEFINE_T_SUPER(DgnProgressMeter)

protected:
    Utf8String m_stepName;
    Utf8String m_taskName;
    Utf8String m_lastMessage;
    double m_timeOfLastUpdate;
    double m_timeOfLastSpinnerUpdate;
    uint32_t m_spinCount;

    void UpdateDisplay0 (Utf8StringCR msgLeft)
        {
        m_lastMessage = msgLeft;

        // Display the number of tasks remaining. Not all major tasks have a task count.
        Utf8String tbd;
        if (m_stepsRemaining || m_tasksRemaining)
            tbd = Utf8PrintfString(":%d/%d", m_stepsRemaining, m_tasksRemaining);

        // Display the spinner and the task.
        Utf8PrintfString msg("[%c] %-123.123s %-16.16s", s_spinner[m_spinCount%s_spinnerSize], msgLeft.c_str(), tbd.c_str());
        printf("%s\r", msg.c_str());
        }

    void UpdateDisplay()
        {
        auto now = BeTimeUtilities::QuerySecondsCounter();

        if ((now - m_timeOfLastUpdate) < 1.0)
            return;

        m_timeOfLastUpdate = now;

        UpdateDisplay0(FmtMessage());
        }

    bool HasDescription() const
        {
        return m_taskName.find(':') != Utf8String::npos;
        }

    Utf8String FmtMessage() const
        {
        Utf8String msg(m_stepName);
        msg.append(": ");
        msg.append(m_taskName);
        return msg;
        }

    void ForceNextUpdateToDisplay() {m_timeOfLastUpdate=m_timeOfLastSpinnerUpdate=0;}
    virtual void _Hide() override
        {
        Utf8PrintfString msg("    %-123.123s %-16.16s", "", "");
        printf("%s\r", msg.c_str());
        }

    virtual Abort _ShowProgress() override
        {
        if (m_aborted)
            return ABORT_Yes;

        auto now = BeTimeUtilities::QuerySecondsCounter();

        if ((now - m_timeOfLastSpinnerUpdate) < 0.25) // don't do printf's more than a few times per second -- too slow and not useful
            return ABORT_No;

        m_timeOfLastSpinnerUpdate = now;

        m_spinCount++;

        bool justShowSpinner = false;

        if ((now - m_timeOfLastUpdate) < 0.5)
            justShowSpinner = true;         // don't push out full messages more than a couple times per second -- too slow and not useful
        else
            justShowSpinner = (FmtMessage() == m_lastMessage);

        if (justShowSpinner)
            {
            printf("[%c]\r", s_spinner[m_spinCount%s_spinnerSize]);
            return ABORT_No;
            }

        ForceNextUpdateToDisplay();
        UpdateDisplay();
        return ABORT_No;
        }

    virtual void _SetCurrentStepName (Utf8CP stepName) override
        {
        T_Super::_SetCurrentStepName(stepName); // decrements step count

        if (NULL == stepName)
            {
            m_stepName.clear();
            return;
            }
        if (m_stepName.Equals(stepName))
            return;

        m_stepName = stepName;
        m_taskName.clear();
        m_spinCount = 0;
        ForceNextUpdateToDisplay();
        UpdateDisplay();
        }

    virtual void _SetCurrentTaskName (Utf8CP taskName) override
        {
        T_Super::_SetCurrentTaskName(taskName); // decrements task count

        if (taskName && m_taskName == taskName)
            return;

        m_taskName = taskName ? taskName : "";
        m_spinCount = 0;
        ForceNextUpdateToDisplay();
        UpdateDisplay();
        }

public:
    PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_spinCount(0) {}
};

END_BIM_TELEPORTER_NAMESPACE
