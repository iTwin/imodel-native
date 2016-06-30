/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/TasksTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>

USING_NAMESPACE_BENTLEY_TASKS
struct AsyncTestCheckpoint
    {
    private:
        BeAtomic<bool> m_reached;
        BeAtomic<bool> m_wait;

    public:
        //! Class to help testing async code. 
        //! Controlling thread can wait until checkpoint was reached in other threads that are waiting for continue command
        AsyncTestCheckpoint () :
            m_reached (false),
            m_wait (true)
            {
            }

        //! Call in async code to mark that checkpoint is reached. Will block until Continue() was called.
        void CheckinAndWait ()
            {
            m_reached.store(true);
            while (m_wait);
            }

        //! Call in controlling code to make async code continue from checkpoint.
        void Continue ()
            {
            m_wait.store(false);
            }

        //! Check if checkpoint was reached with CheckinAndWait() method by async code
        bool WasReached ()
            {
            return m_reached;
            }

        //! Wait in controlling code until reached with CheckinAndWait()
        void WaitUntilReached ()
            {
            while (!m_reached);
            }
    };
