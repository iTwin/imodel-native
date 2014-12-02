/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnProgressMeter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Base class for UI progress meters that process multiple steps and tasks within those steps.
//! When destructed, this will automatically remove itself as the host's progress meter (if it was set).
//! @see DgnFileIOLib::Host::GetProgressMeter
// @bsiclass                                                    Keith.Bentley   10/10
//=======================================================================================
struct DgnProgressMeter
    {
    enum Abort {ABORT_No=0, ABORT_Yes=1};

protected:
    Abort   m_aborted;
    UInt32  m_stepsRemaining;
    UInt32  m_tasksRemaining;

    virtual void _AddSteps(UInt32 numSteps)           {m_stepsRemaining += numSteps;}
    virtual void _AddTasks(UInt32 numTasksToAdd)      {m_tasksRemaining += numTasksToAdd;}
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
    DGNPLATFORM_EXPORT ~DgnProgressMeter();

    //! Indicates that additional steps are necessary.
    DGNPLATFORM_EXPORT void AddSteps (UInt32 numSteps);

    //! Indicates that additional tasks within the current step are necessary.
    DGNPLATFORM_EXPORT void AddTasks (UInt32 numTasks);

    //! Sets the current step name. Also indicates that the previous step was complete.
    DGNPLATFORM_EXPORT void SetCurrentStepName(Utf8CP);

    //! Sets the current task name, within the current step. Also indicates that the previous task was complete.
    DGNPLATFORM_EXPORT void SetCurrentTaskName (Utf8CP description);

    //! Called to indicate that progress is being made on the current task.
    //! @return true if the user has asked that the process be aborted. Callers should abort as soon as possible.
    DGNPLATFORM_EXPORT Abort ShowProgress();

    //! Hides the progress meter. The progress meter will still exist, but this can be used to prematurely hide it.
    DGNPLATFORM_EXPORT void Hide();

    //! Turn on the aborted flag. This will cause the process being monitored to abort at the next opportunity.
    void SetAborted() {m_aborted = ABORT_Yes;}
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
