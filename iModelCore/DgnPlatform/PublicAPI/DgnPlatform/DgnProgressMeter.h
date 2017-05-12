/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnProgressMeter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Base class for UI progress meters that process multiple steps and tasks within those steps.
//! @see DgnFileIOLib::Host::GetProgressMeter
// @bsiclass                                                    Keith.Bentley   10/10
//=======================================================================================
struct DgnProgressMeter
    {
    enum Abort {ABORT_No=0, ABORT_Yes=1};

protected:
    Abort   m_aborted;
    uint32_t m_stepsRemaining;
    uint32_t m_tasksRemaining;

    virtual void _AddSteps(uint32_t numSteps)           {m_stepsRemaining += numSteps;}
    virtual void _AddTasks(uint32_t numTasksToAdd)      {m_tasksRemaining += numTasksToAdd;}
    virtual void _SetCurrentStepName(Utf8CP stepName) 
        {
        if (m_stepsRemaining > 0)
            m_stepsRemaining--; 
        m_tasksRemaining = 0;
        }
    virtual void _SetCurrentTaskName(Utf8CP taskName) 
        {
        if (m_tasksRemaining > 0)
            m_tasksRemaining--;
        }
    virtual Abort _ShowProgress() {return m_aborted;}
    virtual void _Hide() {}

public:
    DgnProgressMeter() {m_aborted=ABORT_No; m_stepsRemaining=m_tasksRemaining=0;}

    //! Indicates that additional steps are necessary.
    DGNPLATFORM_EXPORT void AddSteps(uint32_t numSteps);

    //! Indicates that additional tasks within the current step are necessary.
    DGNPLATFORM_EXPORT void AddTasks(uint32_t numTasks);

    //! Sets the current step name. Also indicates that the previous step was complete.
    DGNPLATFORM_EXPORT void SetCurrentStepName(Utf8CP);

    //! Sets the current task name, within the current step. Also indicates that the previous task was complete.
    DGNPLATFORM_EXPORT void SetCurrentTaskName(Utf8CP description);

    //! Called to indicate that progress is being made on the current task.
    //! @return true if the user has asked that the process be aborted. Callers should abort as soon as possible.
    DGNPLATFORM_EXPORT Abort ShowProgress();

    //! Hides the progress meter. The progress meter will still exist, but this can be used to hide it.
    DGNPLATFORM_EXPORT void Hide();

    //! Turn on the aborted flag. This will cause the process being monitored to abort at the next opportunity.
    void SetAborted() {m_aborted = ABORT_Yes;}
    };

//=======================================================================================
// A progress meter for command-line applications.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
{
    DEFINE_T_SUPER(DgnProgressMeter)

protected:
    Utf8String m_stepName;
    Utf8String m_taskName;
    Utf8String m_lastMessage;
    double m_timeOfLastUpdate;
    double m_timeOfLastSpinnerUpdate;
    uint32_t m_spinCount;

    void UpdateDisplay0 (Utf8StringCR msg);
    void UpdateDisplay();
    void PopDescription();
    bool HasDescription() const;
    Utf8String FmtMessage() const;
    void ForceNextUpdateToDisplay() {m_timeOfLastUpdate=m_timeOfLastSpinnerUpdate=0;}
    DGNPLATFORM_EXPORT virtual void _Hide() override;
    DGNPLATFORM_EXPORT virtual Abort _ShowProgress() override;
    DGNPLATFORM_EXPORT virtual void _SetCurrentStepName (Utf8CP stepName) override;
    DGNPLATFORM_EXPORT virtual void _SetCurrentTaskName (Utf8CP taskName) override;

public:
    PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_spinCount(0) {}
};

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      03/17
+===============+===============+===============+===============+===============+======*/
struct NopProgressMeter : DgnProgressMeter
    {
    virtual void _Hide() override {}
    virtual Abort _ShowProgress() override { return Abort::ABORT_No; }
    virtual void _SetCurrentStepName(Utf8CP stepName) override {}
    virtual void _SetCurrentTaskName(Utf8CP taskName) override {}
    };

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
