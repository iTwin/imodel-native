//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/ClientThread2.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HCSNamedPipeConnection.h"
#include "HFCBuffer.h"

class ClientThread : public HFCThread
    {
public:
    ClientThread(uint32_t      pi_ID,
                 const string& pi_rHost,
                 unsigned short pi_Port,
                 HFCEvent&     pi_rStopEvent)
        : m_ID(pi_ID),
          m_Host(pi_rHost),
          m_Port(pi_Port),
          m_rStopEvent(pi_rStopEvent)
        {
        StartThread();
        }
    ~ClientThread()
        {
        StopThread();
        try
            {
            if (m_pConnection != 0)
                m_pConnection->Disconnect();
            }
        catch(...)
            {
            }
        WaitUntilSignaled();
        }
    virtual void Go()
        {
        ostringstream Host;
        Host << m_Host;
        while (!m_rStopEvent.WaitUntilSignaled(0))
            {
            try
                {
                m_pConnection = new HCSNamedPipeConnection(Host.str());
                if (m_pConnection->Connect(string(""), string(""), 3000))
                    {
                    // send the Request to the server
                    m_pConnection->Send((const Byte*)s_Marker.data(), s_Marker.size());

                    HFCBuffer Buffer(1, 1);
                    while ((m_pConnection->IsConnected()) &&
                           (Buffer.SearchFor((const Byte*)s_Marker.data(), s_Marker.size()) == -1))
                        {
                        // Wait for data to arrive
                        size_t DataAvailable = 1;

                        // read it
                        m_pConnection->Receive(Buffer.PrepareForNewData(DataAvailable), &DataAvailable);
                        Buffer.SetNewDataSize(DataAvailable);
                        }

                    // Verify if the content of the buffer is "hip:..."
                    if (Buffer.GetDataSize() >= s_Response.size())
                        {
                        // convert the available data into a string and lower-case it
                        string Response((const char*)Buffer.GetData(), Buffer.GetDataSize());
                        ctype<char>().tolower(Response.begin(), Response.end());

                        // if the content starts with an OBJ=HIP response, then the server is HIP compatible
                        HFCGenericMonitor<OutputKeyType> Monitor(s_OutputKey);
                        if (Response.compare(s_Response))
                            cout << m_ID << " ";
                        else
                            cout << endl << "Thread " << m_ID << " failed!  Bad response!" << endl;
                        }
                    else
                        {
                        HFCGenericMonitor<OutputKeyType> Monitor(s_OutputKey);
                        cout << endl << "Thread " << m_ID << " failed!  Bad response size!" << endl;
                        }
                    }
                else
                    {
                    HFCGenericMonitor<OutputKeyType> Monitor(s_OutputKey);
                    cout << endl << "Thread " << m_ID << " failed!  Can not connect!" << endl;
                    }

                m_pConnection->Disconnect();
                HFCThread::Sleep(100);
                m_pConnection = 0;
                }
            catch(...)
                {
                HFCGenericMonitor<OutputKeyType> Monitor(s_OutputKey);
                cout << endl << "Thread " << m_ID << " failed!  Exception occured!" << endl;
                }
            }
        }
private:
    uint32_t m_ID;
    string  m_Host;
    unsigned short m_Port;
    HAutoPtr<HFCInternetConnection> m_pConnection;
    HFCEvent& m_rStopEvent;
    };