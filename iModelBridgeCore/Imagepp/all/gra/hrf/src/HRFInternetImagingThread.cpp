//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetImagingThread.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingThread
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetImagingThread.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFHTTPConnection.h>
#include <Imagepp/all/h/HRFSocketConnection.h>

#include <Imagepp/all/h/HFCMonitor.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const string s_Marker("\r\n");
static const int32_t s_SleepTime      = 100;
static const size_t g_ReadGrowSize = 1024;
static const size_t g_ReadMaxSize  = 32 * 1024;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetImagingThread::HRFInternetImagingThread(HRFInternetImagingFile& pi_rFile)
    : HFCThread(false),
      m_rFile(pi_rFile),
      m_ReadBuffer(g_ReadGrowSize, g_ReadMaxSize)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetImagingThread::~HRFInternetImagingThread()
    {
    // Reset the run event so that the thread may end
    StopThread();

    // Wait until it actually ends
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// Public
// Add a handler to the thread handlers
//-----------------------------------------------------------------------------
void HRFInternetImagingThread::AddHandler(const HFCPtr<HRFInternetImagingHandler>& pi_rpHandler)
    {
    HPRECONDITION(pi_rpHandler != 0);
    HFCMonitor HandlerMonitor(m_HandlerMonitor);

    m_Handlers.push_back(pi_rpHandler);
    }


//-----------------------------------------------------------------------------
// Public
// Copies the handlers of another handler list.
//-----------------------------------------------------------------------------
void HRFInternetImagingThread::AddHandlers(const HandlerList& pi_rHandlers)
    {
    HFCMonitor HandlerMonitor(m_HandlerMonitor);

    HandlerList::const_iterator Itr(pi_rHandlers.begin());
    while (Itr != pi_rHandlers.end())
        {
        AddHandler(*Itr);
        Itr++;
        }
    }


//-----------------------------------------------------------------------------
// Public
// Clears the handler list
//-----------------------------------------------------------------------------
void HRFInternetImagingThread::ClearHandlers()
    {
    HFCMonitor HandlerMonitor(m_HandlerMonitor);

    m_Handlers.clear();
    }


//-----------------------------------------------------------------------------
// Public
// Clears the read buffer
//-----------------------------------------------------------------------------
void HRFInternetImagingThread::ClearBuffer()
    {
    m_ClearReadBuffer = true;
    }


//-----------------------------------------------------------------------------
// Public
//
// This method will read from the connection one character at a time until a
// handler can identify the label it can handle.  From there, the connection
// will given to the handler which will optimize the handling for the current
// data.
//-----------------------------------------------------------------------------
void HRFInternetImagingThread::Go()
    {
    bool Disconnected = false;
    bool CurrentRequestEnded = true;


    while (CanRun())
        {
        try
            {
            // Lock the file connection
            HFCMonitor ConnectionMonitor(m_rFile.m_ConnectionKey);

            // Verify if the connection exists
            if ((m_rFile.m_pConnection != 0) &&
                (!Disconnected || m_rFile.m_pConnection->IsConnected()) &&
                (m_rFile.m_pConnection->WaitDataAvailable(s_SleepTime) > 0))
                {
                // There is data available so the line is connection
                Disconnected = false;

                // clear the read buffer if needed
                if (m_ClearReadBuffer)
                    {
                    m_ReadBuffer.Clear();
                    m_ClearReadBuffer = false;
                    }

                // read one character in the buffer
                Byte TheByte;
                m_rFile.m_pConnection->Receive(&TheByte, 1);

                // Add the new byte to the buffer
                m_ReadBuffer.AddData(&TheByte, 1);

                // try to find a handler that can handle the actual data
                HFCMonitor HandlerMonitor(m_HandlerMonitor);
                HandlerList::iterator Itr(m_Handlers.begin());
                bool Handled = false;
                while ((!Handled) && (Itr != m_Handlers.end()))
                    {
                    // verify if the data can be handle by the current handler
                    if ((*Itr)->CanHandle(m_ReadBuffer))
                        {
                        // Handle the response and clear the buffer
                        (*Itr)->Handle(m_rFile, m_ReadBuffer, *m_rFile.m_pConnection);
                        m_ReadBuffer.Clear();
                        CurrentRequestEnded = false;
                        Handled = true;
                        }

                    // Proceed to the next handler
                    Itr++;
                    }

                // Check if the current data ends with s_Marker
                // so to invalidate the data in case of a unhandled response.
                if (m_ReadBuffer.GetDataSize() >= s_Marker.size() &&
                    memcmp(m_ReadBuffer.GetData() + (m_ReadBuffer.GetDataSize() - s_Marker.size()), s_Marker.data(), s_Marker.size()) == 0)
                    ClearBuffer();


                if (m_rFile.m_HTTPConnection)
                    {
                    // if we have read the whole answer, tell it to the internet image
                    if (((HRFHTTPConnection*)m_rFile.m_pConnection.get())->RequestEnded() && m_rFile.m_pConnection->WaitDataAvailable(0) == 0)
                        {
                        ClearBuffer();
                        m_rFile.EndOfCurrentRequest();
                        }
                    }
                else
                    {
                    if (((HRFSocketConnection*)m_rFile.m_pConnection.get())->RequestEnded() && m_rFile.m_pConnection->WaitDataAvailable(0) == 0)
                        {
                        m_rFile.EndOfCurrentRequest();
                        ClearBuffer();
                        }
                    }

                // We can release the handlers and the connection here
                // to permit other threads to access it while we end the
                // current processing
                ConnectionMonitor.ReleaseKey();
                HandlerMonitor.ReleaseKey();

                }

            // No connection or no data is available. Wait a little and try again
            else
                {
                // Release the connection before sleeping to give other threads a little chance
                ConnectionMonitor.ReleaseKey();
                HFCThread::Sleep(s_SleepTime);
                }
            }


        // Connection Exception
        catch(HFCInternetConnectionException& ConnectionException)
            {
            // Clear the buffer
            ClearBuffer();

            // There was a disconnected, so we need no read or wait until the
            // connection indicates that it is connected again
            Disconnected = true;

            // repackage as an Internet Imaging Connection Exception
            HRFInternetImagingConnectionException::ErrorType ErrorType;
            switch(ConnectionException.GetErrorType())
                {
                case HFCInternetConnectionException::CANNOT_CONNECT:
                    ErrorType = HRFInternetImagingConnectionException::CANNOT_CONNECT;
                    break;

                case HFCInternetConnectionException::CONNECTION_LOST:
                    ErrorType = HRFInternetImagingConnectionException::CONNECTION_LOST;
                    break;

                default:
                case HFCInternetConnectionException::UNKNOWN:
                    ErrorType = HRFInternetImagingConnectionException::UNKNOWN;
                    break;

                }

            // set the exception
            m_rFile.SetThreadException(HRFInternetImagingConnectionException(m_rFile.GetURL()->GetURL(),
                                                                             ErrorType));
            HFCThread::Sleep(s_SleepTime);
            }


        // Internet Imaging Exception
        catch(HRFInternetImagingException& rInternetImagingException)
            {
            // Clear the buffer
            ClearBuffer();

            m_rFile.SetThreadException(rInternetImagingException);
            HFCThread::Sleep(s_SleepTime);
            }


        // Generic Image++ Exception
        catch(HFCException& Exception)
            {
            // Clear the buffer
            ClearBuffer();

            m_rFile.SetThreadException(Exception);
            HFCThread::Sleep(s_SleepTime);
            }


        // Unknown Exception
        catch(...)
            {
            // Clear the buffer
            ClearBuffer();

            m_rFile.SetThreadException(HFCException());
            HFCThread::Sleep(s_SleepTime);
            }
        }
    }
