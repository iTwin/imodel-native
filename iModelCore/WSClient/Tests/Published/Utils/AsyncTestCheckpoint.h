/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/AsyncTestCheckpoint.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

struct AsyncTestCheckpoint
    {
    private:
        BeAtomic<bool> m_reached;
        BeAtomic<bool> m_wait;

    public:
        //! Class to help testing async code. 
        //! Controlling thread can wait until checkpoint was reached in other threads that are waiting for continue command
        AsyncTestCheckpoint() :
            m_reached(false),
            m_wait(true)
            {}

        //! Call in async code to mark that checkpoint is reached. Will block until Continue() was called.
        void CheckinAndWait()
            {
            m_reached = true;
            while (m_wait);
            }

        //! Call in controlling code to make async code continue from checkpoint.
        void Continue()
            {
            m_wait = false;
            }

        //! Check if checkpoint was reached with CheckinAndWait() method by async code
        bool WasReached()
            {
            return m_reached;
            }

        //! Wait in controlling code until reached with CheckinAndWait()
        void WaitUntilReached()
            {
            while (!m_reached);
            }
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
