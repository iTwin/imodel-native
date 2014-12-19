//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/KeyboardThread.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <conio.h>

class KeyboardThread : public HFCThread
    {
public:
    KeyboardThread(char pi_StopKey, char pi_BreakKey)
        {
        m_StopKey = pi_StopKey;
        m_BreakKey = pi_BreakKey;
        StartThread();
        }
    ~KeyboardThread()
        {
        StopThread();
        WaitUntilSignaled();
        }
    virtual void Go()
        {
        char Key = 0;
        while (Key != m_StopKey)
            {
            Key = _getch();
            if (Key == m_BreakKey)
                {
                DebugBreak();
                }
            }
        }
private:
    char m_StopKey;
    char m_BreakKey;
    };