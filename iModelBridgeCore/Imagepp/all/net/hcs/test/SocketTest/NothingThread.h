//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/NothingThread.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCBuffer.h"

class NothingThread : public HFCThread
    {
public:
    NothingThread()
        {
        StartThread();
        }
    ~NothingThread()
        {
        StopThread();
        WaitUntilSignaled();
        }
    virtual void Go()
        {
        while (CanRun())
            {
            for (uint32_t i = 0; CanRun() && (i < ULONG_MAX); ++i)
                HFCThread::Sleep(100);
            }
        }
    };