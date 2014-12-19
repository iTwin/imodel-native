//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/ServerThread2.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HCSRequestProcessor.h"

class ServerThread : public HCSRequestProcessor
    {
public:
    ServerThread(uint32_t           pi_ID,
                 HCSConnectionPool& pi_rPool,
                 HFCEvent&          pi_rStopEvent)
        : HCSRequestProcessor(pi_rPool),
          m_ID(pi_ID),
          m_rStopEvent(pi_rStopEvent)
        {
        StartThread();
        }

    ~ServerThread()
        {
        StopThread();
        try
            {
            DestroyConnection(true);
            }
        catch(...)
            {
            }
        WaitUntilSignaled();
        }

    virtual void Go()
        {
        while (!m_rStopEvent.WaitUntilSignaled(0))
            {
            HFCInternetConnection* pConnection = static_cast<HFCInternetConnection*>(GetConnectionFromPool(100));
            HPRECONDITION((pConnection == 0) || (pConnection->IsCompatibleWith(HFCInternetConnection::CLASS_ID)));

            // If there is a connection, send hello
            if (pConnection != 0)
                {
                try
                    {
                    // send the data and kill the connection
                    try
                        {
                        pConnection->Send((const Byte*)s_Message.data(), s_Message.size());
                        pConnection->Send((const Byte*)s_Marker.data(),  s_Marker.size());

                        HFCGenericMonitor<OutputKeyType> Monitor(s_OutputKey);
                        cout << m_ID << " ";
                        }
                    catch(...)
                        {
                        }
                    }
                catch(...)
                    {
                    }

                // kill the connection
                pConnection->Disconnect();
                HFCThread::Sleep(100);
                DestroyConnection();
                }
            }
        }
private:
    uint32_t  m_ID;
    HFCEvent& m_rStopEvent;
    };