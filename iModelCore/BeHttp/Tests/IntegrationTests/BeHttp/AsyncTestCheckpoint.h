/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Tests.h"

#include <Bentley/BeTimeUtilities.h>

struct AsyncTestCheckpoint
    {
    private:
        BeAtomic<bool> m_reached;
        BeAtomic<bool> m_wait;

    private:
        void WaitFor(BeAtomic<bool>& variable, bool value, uint64_t failAfterMs)
            {
            uint64_t msStarted = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            while (variable != value)
                {
                BeThreadUtilities::BeSleep(1);
                uint64_t msCurrent = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
                if (failAfterMs != 0 && failAfterMs < msCurrent - msStarted)
                    {
                    ADD_FAILURE() << "Waited for " << failAfterMs << "ms " << " without change";
                    return;
                    }
                }
            }

    public:
        //! Class to help testing async code. 
        //! Controlling thread can wait until checkpoint was reached in other threads that are waiting for continue command
        //! NOTE - can be used only once, it is not meant for reusing
        AsyncTestCheckpoint() :
            m_reached(false),
            m_wait(true)
            {}

        //! Call in async code to mark that checkpoint is reached. Will block until Continue() was called.
        void CheckinAndWait(uint64_t failAfterMs = 0)
            {
            m_reached.store(true);
            WaitFor(m_wait, false, failAfterMs);
            }

        //! Call in async code to mark that checkpoint is reached.
        void Checkin()
            {
            m_reached.store(true);
            }

        //! Call in controlling code to make async code continue from checkpoint.
        void Continue()
            {
            m_wait.store(false);
            }

        //! Check if can continue
        bool CanContinue()
            {
            return !m_wait;
            }

        //! Check if checkpoint was reached with CheckinAndWait() method by async code
        bool WasReached()
            {
            return m_reached;
            }

        //! Wait in controlling code until reached with CheckinAndWait()
        void WaitUntilReached(uint64_t failAfterMs = 0)
            {
            WaitFor(m_reached, true, failAfterMs);
            }
    };
