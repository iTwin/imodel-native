//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHTTPConnection.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHTTPConnection
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCHTTPConnection.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>
#include <Imagepp/all/h/HFCThread.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include <Imagepp/all/h/HFCEvent.h>

#include <Imagepp/all/h/HFCCallbackRegistry.h>
#include <Imagepp/all/h/HFCCallbacks.h>

#if defined (ANDROID) || defined (__APPLE__)
typedef void* HINTERNET;        //DM-Android   Many codes are disable below.

#elif defined (_WIN32)
#include <Winsock2.h>
#include <Winerror.h>
#include <wininet.h>
#endif

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// HTTP connection buffer size
static const size_t s_GrowSize = 1024;
static const size_t s_MaxSize  = 128 * 1024;

//-----------------------------------------------------------------------------
// GetStatusCode
//-----------------------------------------------------------------------------
#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
uint32_t GetStatusCode(HINTERNET pi_hHandle)
    {
    HPRECONDITION(pi_hHandle != NULL);

    DWORD statusCode = 0;
    DWORD len = sizeof(statusCode);

    if (!HttpQueryInfoW(pi_hHandle,
                       HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
                       &statusCode,
                       &len,
                       NULL))
        return 0;
        
    return statusCode;
    }
#endif

//-----------------------------------------------------------------------------
// Internet Session
//-----------------------------------------------------------------------------

class HTTPSession 
    {
    HFC_DECLARE_SINGLETON(HTTPSession)

public:
    //------------------------------------------
    // Constructor - Builds the session
    //------------------------------------------
    HTTPSession()
        {
        m_hSession = NULL;
        }

    //------------------------------------------
    // Destructor - Closes the session handle
    //------------------------------------------
    ~HTTPSession()
        {
        if(NULL != m_hSession)
            {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
            InternetCloseHandle(m_hSession);
#endif
            m_hSession = NULL;
            }
        }

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    //------------------------------------------
    // returns the session
    //------------------------------------------
    HINTERNET GetSession() const
        {
        if (m_hSession == NULL)
            {
            // Create the Internet Session
            m_hSession = InternetOpenW(L"", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

            HASSERT(m_hSession != NULL);
            }

        return (m_hSession);
        }

    //------------------------------------------
    // returns the session
    //------------------------------------------
    bool SetProxy(const WString& pi_rProxyList, const WString& pi_rBypass, DWORD pi_Flags)
        {
        DWORD ErrCode = 0;

        LPCWSTR pProxyList=0;
        if (!pi_rProxyList.empty())
            pProxyList = pi_rProxyList.c_str();

        LPCWSTR pBypassList=0;
        if (!pi_rBypass.empty())
            pBypassList = pi_rBypass.c_str();

        // Check if the Proxy is already setup
        //
        unsigned long        nSizeBuf = 4096;
        uint8_t              szBuf[4096] = { 0 };
        INTERNET_PROXY_INFO* pInfo = (INTERNET_PROXY_INFO*)szBuf;

        // **** Use the 'TCHAR' version of InternetQueryOption(InternetQueryOptionA or InternetQueryOptionW) since INTERNET_PROXY_INFO use TCHAR internally.
        if(!InternetQueryOption(GetSession(), INTERNET_OPTION_PROXY, pInfo, &nSizeBuf))
            {
            ErrCode = GetLastError();
            HASSERT(false);
            }

        // If no proxy name specify and no proxy currently set
        if ((pProxyList == 0) && (pInfo->lpszProxy == 0))
            ErrCode = 0;

        // If proxy names specify and proxy currently set are the same
        else if ((pProxyList != 0) && (pInfo->lpszProxy != 0) && pi_rProxyList.compare(WString(pInfo->lpszProxy,false))==0)
            ErrCode = 0;

        // If no proxy name specify and proxy currently set or
        // If proxy name is specify and there is no current proxy or it is different
        else
            {
            INTERNET_PER_CONN_OPTION_LISTW  List;
            DWORD                           nSize = sizeof(INTERNET_PER_CONN_OPTION_LISTW);

            INTERNET_PER_CONN_OPTIONW  SetOptions[3];

            SetOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
            if(pi_Flags != 0)
                SetOptions[0].Value.dwValue = pi_Flags;
            else if (pProxyList == 0) // No proxy specify
                {
                SetOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;
                pBypassList = 0;    // Ignore the bypass list in this case
                }
            else
                SetOptions[0].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_PROXY;

            WCharP tempProxyList = (WCharP) _alloca ((wcslen(pProxyList)+1)*sizeof(WChar));
            wcscpy (tempProxyList, pProxyList);
            SetOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
            SetOptions[1].Value.pszValue = tempProxyList;

            List.dwOptionCount = 2;
            if (pBypassList != 0)
                {
                WCharP tempByPassList = (WCharP) _alloca ((wcslen(pBypassList)+1)*sizeof(WChar));
                wcscpy (tempByPassList, pBypassList);

                SetOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
                SetOptions[2].Value.pszValue = tempByPassList;
                List.dwOptionCount = 3;
                }

            List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LISTW);
            List.pszConnection = NULL;
            List.dwOptionError = 0;
            List.pOptions = SetOptions;
            if(!InternetSetOptionW(HTTPSession::GetInstance()->GetSession(), INTERNET_OPTION_PER_CONNECTION_OPTION, &List, nSize))
                {
                ErrCode = GetLastError();
                HASSERT(false);
                }

            if (!InternetSetOptionW(HTTPSession::GetInstance()->GetSession(), INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0 ))
                {
                ErrCode = GetLastError();
                HASSERT(false);
                }

            }

        return (ErrCode == 0);
        }
#endif

private:
    mutable HINTERNET m_hSession;
    };

HFC_IMPLEMENT_SINGLETON(HTTPSession)

//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

static const WString s_Device(L"HTTP connection");
static const time_t s_SleepTime = 100;


//-----------------------------------------------------------------------------
// HRFHTTPRequestThread
//
// This thread will perform the OpenURL which will request the data from the
// server
//-----------------------------------------------------------------------------
class ImagePP::HTTPThread : public HFCThread
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HTTPThread(HFCHTTPConnection& pi_rConnection, bool pi_SecureConnection = false);
    ~HTTPThread();

    //--------------------------------------
    // Methods
    //--------------------------------------

    // adds a new request to the thread
    void            AddRequest(HINTERNET pi_hHandle);


    //--------------------------------------
    // HFCThread Overrides
    //--------------------------------------

    virtual void    Go();

private:
    //--------------------------------------
    // Methods
    //--------------------------------------
    // Get the next request in the list
    HINTERNET       GetRequest();

    // Handles a HTTP request from start to finish
    void            HandleRequest(HINTERNET pi_hRequest);

    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Reference to the connection that owns the thread
    HFCHTTPConnection&
    m_rConnection;
    bool           m_SecureConneciton;

    typedef list<HINTERNET> RequestList;

    // Handle to the current request
    HFCInterlockedValue<HINTERNET>
    m_hRequest;

    // List of request to handle
    RequestList     m_Requests;
    HFCExclusiveKey m_RequestKey;
    HFCEvent        m_RequestEvent;
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HFCHTTPConnection implementation
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public / Static
// SetProxy
//-----------------------------------------------------------------------------
bool HFCHTTPConnection::SetProxy(const WString& pi_rProxyList,
                                 const WString& pi_rBypass,
                                 uint32_t       pi_Flags)
    {
    // If the session is already created, we close and reopen a new one.
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
    return HTTPSession::GetInstance()->SetProxy(pi_rProxyList, pi_rBypass, pi_Flags);
#endif
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFCHTTPConnection::HFCHTTPConnection(const WString&     pi_rServer,
                                     const WString&     pi_rUserName,
                                     const WString&     pi_rPassword)
    : HFCInternetConnection(pi_rServer,
                            pi_rUserName,
                            pi_rPassword),
    m_HTTPEvent(true, false),
    m_Buffer(s_GrowSize, s_MaxSize)
    {
    m_hConnection = 0;

    m_pURL = (HFCURLCommonInternet*)HFCURL::Instanciate(pi_rServer);
    m_SecureConnection = m_pURL->IsCompatibleWith(HFCURLHTTPS::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCHTTPConnection::~HFCHTTPConnection()
    {
    // Disconnect the connection
    try
        {
        Disconnect();
        }
    catch(...)
        {
        }
    }



//-----------------------------------------------------------------------------
// static private
// Finds the '&' closest to the middle of the data.
//-----------------------------------------------------------------------------
static size_t sfFindMiddleSeparator(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);
    size_t Result = -1;

    // search in the middle of the data for a '&'
    size_t FwdPos = pi_DataSize / 2;
    size_t RwdPos = pi_DataSize / 2;
    bool  End    = false;
    while ((Result == -1) && (!End))
        {
        // verify if there is a '&' at the current positions
        if (pi_pData[FwdPos] == '&')
            Result = FwdPos;
        else if (pi_pData[RwdPos] == '&')
            Result = RwdPos;

        // update the positions
        if (Result == -1)
            {
            FwdPos++;
            if (FwdPos == pi_DataSize)
                FwdPos = pi_DataSize - 1;
            RwdPos--;
            if (RwdPos == -1)
                RwdPos = 0;

            // check if we have reached the end
            if ((FwdPos == pi_DataSize - 1) &&
                (RwdPos == 0))
                End = true;
            }
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// public
//
// pi_pData should be in UTF-8
//-----------------------------------------------------------------------------
void HFCHTTPConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // If not connected to the server, throw
    if (!IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // format of a HTTP request
    //
    //  urlbase / [searchbase '&'] request

    // format the extention and search identifier (?)
    string Request("/");
    size_t  destinationBuffSize = GetExtention().GetMaxLocaleCharBytes();
    char*  ExtensionMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(ExtensionMBS, GetExtention().c_str(),destinationBuffSize);


    Request += string(ExtensionMBS);
    Request += "?";

    // Add the search part depending if the file was set up
    if (!GetSearchBase().empty())
        {
        Request += GetSearchBase();
        Request += "&";
        }

    // Append the actual data
    Request.append((const char*)pi_pData, pi_DataSize);

    // add the request to the thread.  If it failed, throw.

    HSTATUS Status;
    if ((Status = SendRequest(Request)) != H_SUCCESS)
        {
        if (Status == H_PERMISSION_DENIED)
            {
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::PERMISSION_DENIED);
            }
        else if (Status == H_PROXY_PERMISSION_DENIED)
            {
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::PROXY_PERMISSION_DENIED);
            }
        else
            {
#if defined (ANDROID) || defined (__APPLE__)
 //DM-Android
            if (0)
#elif defined (_WIN32)
            // Get the last error that caused AddRequest to fail
            uint32_t TheLastError = GetLastError();

            // if the last error for the send is ERROR_INSUFFICIENT_BUFFER,
            // separate the request in two parts and resend

            if (TheLastError == ERROR_INSUFFICIENT_BUFFER)
#endif
                {
                size_t Pos = sfFindMiddleSeparator(pi_pData, pi_DataSize);
                if (Pos == (size_t)-1)
                    throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEPARATE_DATA);

                // Send in two parts
                Send(pi_pData, Pos);
                Send(pi_pData + Pos + 1, pi_DataSize - Pos - 1);
                }
            else
                throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_SEND);
            }
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCHTTPConnection::Receive(Byte* po_pData, size_t pi_DataSize)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    size_t ReadSize;

    // Read if there is still data to be read
    while ((ReadSize = pi_DataSize) > 0)
        {
        // Read available data
        Receive(po_pData, &ReadSize);

        // update pointer and data size with the previous read data
        po_pData    += ReadSize;
        pi_DataSize -= ReadSize;
        }
    }


//-----------------------------------------------------------------------------
// public
// Reads data currently available or and until some arrives
//-----------------------------------------------------------------------------
void HFCHTTPConnection::Receive(Byte* po_pData, size_t* pio_pDataSize)
    {
    HPRECONDITION(po_pData       != 0);
    HPRECONDITION(pio_pDataSize  != 0);
    HPRECONDITION(*pio_pDataSize >  0);

    size_t DataAvailable;

    // Wait for data.  Will block until something happens, good or bad.
    if ((DataAvailable = WaitDataAvailable()) > 0)
        {
        // Compute the size of the data to copy
        *pio_pDataSize = MIN(DataAvailable, *pio_pDataSize);

        // Copy the data into the output buffer
        HFCMonitor BufferMonitor(m_BufferKey);
        memcpy(po_pData, m_Buffer.GetData(), *pio_pDataSize);

        // Update the buffer.  If it becomes empty, reset the HTTP event
        m_Buffer.MarkReadData(*pio_pDataSize);
        if (m_Buffer.GetDataSize() == 0)
            m_HTTPEvent.Reset();
        }
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HFCHTTPConnection::Connect(const WString& pi_rUserName,
                                 const WString& pi_rPassword,
                                 time_t         pi_TimeOut)
    {
    HPRECONDITION(m_hConnection == 0);
    HPRECONDITION(pi_TimeOut > 0);
    bool Result = false;

    try
        {
#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
        // change the connection time out of the session
        uint32_t TimeOut = (uint32_t)pi_TimeOut;
        InternetSetOptionW(HTTPSession::GetInstance()->GetSession(), INTERNET_OPTION_CONNECT_TIMEOUT, &TimeOut, sizeof(TimeOut));
        TimeOut = 0;
        InternetSetOptionW(HTTPSession::GetInstance()->GetSession(), INTERNET_OPTION_CONNECT_RETRIES, &TimeOut, sizeof(TimeOut));

        // Create the connection
        int HttpPort;
        if (!m_pURL->GetPort().empty())
            HttpPort = BeStringUtilities::Wtoi(m_pURL->GetPort().c_str());
        else
            HttpPort = (m_SecureConnection ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

        m_hConnection = (ULONG_PTR)InternetConnectW(HTTPSession::GetInstance()->GetSession(),
                                        m_pURL->GetHost().c_str(),
                                        (INTERNET_PORT)(HttpPort),
                                        NULL,
                                        NULL,
                                        INTERNET_SERVICE_HTTP,
                                        0,
                                        0);

        if (m_hConnection == 0)
            throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);
#endif

        // Specify the extension used
        SetExtention(m_SecureConnection? ((const HFCPtr<HFCURLHTTPS>&)m_pURL)->GetPath() : ((const HFCPtr<HFCURLHTTP>&)m_pURL)->GetPath());
        SetConnected(true);

        // Start the socket receive thread
        m_pThread = new HTTPThread(*this, m_SecureConnection);
        Result = true;
        }
    catch(...)
        {
        Disconnect();
        Result = false;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HFCHTTPConnection::ValidateConnect(uint32_t pi_TimeOut)
    {
    bool Result = IsConnected();

    if (!Result)
        Result = Connect(m_UserName,
                         m_Password,
                         pi_TimeOut);

    return Result;
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HFCHTTPConnection::Disconnect()
    {
    // Set the disconnection status to false
    SetConnected(false);

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // close the internet session
    InternetCloseHandle((HINTERNET)m_hConnection);
    m_hConnection = 0;
#endif

    // Stop the HTTP thread
    m_pThread = 0;

    // Signal something for other thread to unblock
    m_HTTPEvent.Signal();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HFCHTTPConnection::WaitDataAvailable()
    {
    // Wait until the HTTP event is signaled
    m_HTTPEvent.WaitUntilSignaled();

    // If the connection is lost, throw up
    if ((m_Buffer.GetDataSize() == 0) && !IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // Read the data size of the buffer
    HFCMonitor BufferMonitor(m_BufferKey);
    return m_Buffer.GetDataSize();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
size_t HFCHTTPConnection::WaitDataAvailable(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);

    // Wait until the HTTP event is signaled
    m_HTTPEvent.WaitUntilSignaled(pi_TimeOut);

    // If the connection is lost, throw up
    if ((m_Buffer.GetDataSize() == 0) && !IsConnected())
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CONNECTION_LOST);

    // Read the data size of the buffer
    HFCMonitor BufferMonitor(m_BufferKey);
    return m_Buffer.GetDataSize();
    }


//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected
// Called by the thread when a single request is over.  Will be overridden be
// inherited classes.
//-----------------------------------------------------------------------------
void HFCHTTPConnection::RequestHasEnded(bool pi_Success)
    {
    }


//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// private
// CreateRequest
//-----------------------------------------------------------------------------
HSTATUS HFCHTTPConnection::SendRequest(const string& pi_rRequest)
    {
    HPRECONDITION(!pi_rRequest.empty());
    HSTATUS Result = H_ERROR;

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    HINTERNET hRequest = NULL;

    // Open the request URL
    // WARNING: Must use the "A" version because the input request is sometime UTF-8 and sometime MultiBytes.
    if ((hRequest = HttpOpenRequestA((HINTERNET)m_hConnection,  // Handle for this Internet Connection
                                     NULL,                      // HTTP VERB, default is GET
                                     pi_rRequest.c_str(),           // the request
                                     NULL,                      // HTTP version, default to 1.0
                                     NULL,                      // Referer, none
                                     NULL,                      // Accept-types, default
                                     INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE | (m_SecureConnection ? INTERNET_FLAG_SECURE : 0),   // flags, none
                                     0)))
        {
        int ErrorCode;
        bool TryAgain = true;
        bool HasStatusProxyAuthReq = false;
        bool HasStatusDenied = false;
        bool CannotConnectFirstTime = true;
        while (TryAgain)
            {
            if (HttpSendRequestW(hRequest, NULL, 0, NULL, 0))
                {
                if ((ErrorCode = GetStatusCode(hRequest)) == HTTP_STATUS_OK)
                    {
                    // add the request to the thread to handle
                    m_pThread->AddRequest(hRequest);
                    Result = H_SUCCESS;
                    TryAgain = false;
                    }
                else if (ErrorCode == HTTP_STATUS_DENIED) // 401
                    {
                    Result = H_PERMISSION_DENIED;
                    TryAgain = false;
                    if (HasStatusProxyAuthReq && !HasStatusDenied)
                        {
                        // we had a proxy authentication, we need to
                        // send again the server authentication
                        if (!GetUserName().empty())
                            {
                            WCharP tempUserName = (WCharP) _alloca (GetUserName().GetMaxLocaleCharBytes());
                            wcscpy (tempUserName, GetUserName().c_str());

                            InternetSetOptionW(hRequest, INTERNET_OPTION_USERNAME, tempUserName, (DWORD)GetUserName().size());
                            TryAgain = true;
                            }

                        if (!GetPassword().empty())
                            {
                            WCharP tempPassword = (WCharP) _alloca (GetPassword().GetMaxLocaleCharBytes());
                            wcscpy (tempPassword, GetPassword().c_str());

                            InternetSetOptionW(hRequest, INTERNET_OPTION_PASSWORD, tempPassword, (DWORD)GetPassword().size());
                            TryAgain = true;
                            }

                        HasStatusDenied = true;
                        }
                    else if (!HasStatusDenied)
                        {
                        // set the server authentication
                        if (!GetUserName().empty())
                            {
                            WCharP tempUserName = (WCharP) _alloca (GetUserName().GetMaxLocaleCharBytes());
                            wcscpy (tempUserName, GetUserName().c_str());
                            InternetSetOptionW(hRequest, INTERNET_OPTION_USERNAME, tempUserName, (DWORD)GetUserName().size());
                            }

                        if (!GetPassword().empty())
                            {
                            WCharP tempPassword = (WCharP) _alloca (GetPassword().GetMaxLocaleCharBytes());
                            wcscpy (tempPassword, GetPassword().c_str());
                            InternetSetOptionW(hRequest, INTERNET_OPTION_PASSWORD, tempPassword, (DWORD)GetPassword().size());
                            }

                        HasStatusDenied = true;
                        TryAgain = true;
                        }
                    }
                else if (ErrorCode == HTTP_STATUS_PROXY_AUTH_REQ)   // 407
                    {
                    Result = H_PROXY_PERMISSION_DENIED;
                    TryAgain = false;

                    if (!HasStatusProxyAuthReq)
                        {
                        // is the first time, try with the login information set in the connection
                        if (!GetProxyUserName().empty())
                            {
                            WCharP tempUserName = (WCharP) _alloca (GetProxyUserName().GetMaxLocaleCharBytes());
                            wcscpy (tempUserName, GetProxyUserName().c_str());
                            InternetSetOptionW(hRequest, INTERNET_OPTION_PROXY_USERNAME, tempUserName, (DWORD)GetProxyUserName().size());
                            TryAgain = true;
                            }

                        if (!GetProxyPassword().empty())
                            {
                            WCharP tempPassword = (WCharP) _alloca (GetProxyPassword().GetMaxLocaleCharBytes());
                            wcscpy (tempPassword, GetProxyPassword().c_str());
                            InternetSetOptionW(hRequest, INTERNET_OPTION_PROXY_PASSWORD, tempPassword, (DWORD)GetProxyPassword().size());
                            TryAgain = true;
                            }

                        HasStatusProxyAuthReq = true;
                        }
                    }
                else
                    {
                    Result = H_ERROR;
                    TryAgain = false;
                    }
                }
            else
                {
                // For some unknown reasons, we receive this error on the first send with do after a proxy setting.
                if (CannotConnectFirstTime && GetLastError() == ERROR_INTERNET_CANNOT_CONNECT)
                    {
                    TryAgain = true;
                    CannotConnectFirstTime = false;
                    }
                else
                    TryAgain = false;
                }
            }

        if (Result != H_SUCCESS)
            {
#ifdef __HMR_DEBUG
            int ErrorCode;
            ErrorCode = GetLastError();
            ErrorCode = GetStatusCode((hRequest));
#endif
            if (hRequest != NULL)
                InternetCloseHandle(hRequest);
            }
        }
    else
        {
        //HDEBUGCODE(int ErrorCode = GetStatusCode((hRequest)));
        if (hRequest != NULL)
            InternetCloseHandle(hRequest);
        }
#endif

    return Result;
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HTTPThread Implementation
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HTTPThread::HTTPThread(HFCHTTPConnection& pi_rConnection, bool pi_SecureConnection)
    : HFCThread(false),
      m_rConnection(pi_rConnection),
      m_RequestEvent(true, false),
      m_SecureConneciton(pi_SecureConnection)
    {
    HPRECONDITION(&pi_rConnection != 0);

#ifdef _WIN32
    m_hRequest = NULL;
#endif

    StartThread();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HTTPThread::~HTTPThread()
    {
#if defined (ANDROID) || defined (__APPLE__)
        //DM-Android
#elif defined (_WIN32)
    // Close the current handle to force any WININET pending operation in
    // the execution of the HTTPThread
        {
        HFCMonitor Monitor(m_hRequest);
        if (m_hRequest != NULL)
            InternetCloseHandle(m_hRequest);
        }
#endif

    // Stop the thread and wait 'til it finishes
    StopThread();
    WaitUntilSignaled();

    // close all the internet handles in the request list
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    while (m_Requests.size() > 0)
        {
        InternetCloseHandle(m_Requests.front());
        m_Requests.pop_front();
        }
#endif
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HTTPThread::AddRequest(HINTERNET pi_hRequest)
    {
    HFCMonitor Monitor(m_RequestKey);
    m_Requests.push_back(pi_hRequest);
    m_RequestEvent.Signal();
    }




//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
HINTERNET HTTPThread::GetRequest()
    {
    HFCMonitor Monitor(m_RequestKey);

    // get the first element & remove from the list
    HINTERNET hResult = m_Requests.front();
    m_Requests.pop_front();

    // if there is no more elements, reset the event
    if (m_Requests.size() == 0)
        m_RequestEvent.Reset();

    HPOSTCONDITION(hResult != NULL);
    return (hResult);
    }
#endif


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
void HTTPThread::HandleRequest(HINTERNET pi_hRequest)
    {
    HPRECONDITION(pi_hRequest != NULL);
    bool     Continue;
    DWORD     DataAvailable;
    Byte*    pBuffer;
    DWORD     BytesRead;
    DWORD     LastError;

    // assign the handle to the current request handle
    m_hRequest = pi_hRequest;

    // Set the time out
    uint32_t TimeOut = m_rConnection.GetTimeOut();
    InternetSetOptionW(m_hRequest, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &TimeOut, sizeof(TimeOut));

    // while there is data
    Continue = true;
    bool ReceivedData = false;
    while (Continue && CanRun())
        {
        // get the amount of data available on the connection
        // When the function succeeds and the number of bytes is 0, the request is over
        if (Continue = ((InternetQueryDataAvailable(m_hRequest, &DataAvailable, 0, 0)) &&
                        (DataAvailable > 0)) )
            {
            ReceivedData = true;
            // Make place for the data in the buffer
            HFCMonitor BufferMonitor(m_rConnection.m_BufferKey);
            pBuffer = m_rConnection.m_Buffer.PrepareForNewData(DataAvailable);

            // read the data
            BytesRead = DataAvailable;
            if (Continue = ((InternetReadFile(m_hRequest, pBuffer, BytesRead, &BytesRead)) &&
                            (BytesRead > 0)))
                {
                // add data and signal its arrival
                m_rConnection.m_Buffer.SetNewDataSize(BytesRead);
                m_rConnection.m_HTTPEvent.Signal();
                }
            }
        else if (Continue)
            HFCThread::Sleep(0);
        }

    // If the stop event is not signaled, process as usual.  Otherwise,
    // end this processing ASAP
    if (CanRun())
        {
        // Get the last error from WININET
        LastError = GetLastError();

        // if the result is true or if the last error is simply
        // a connection reset by the peer, add an end marker and
        // all is fine
        if ((LastError == ERROR_SUCCESS || LastError == ERROR_INTERNET_CONNECTION_RESET) && ReceivedData)
            m_rConnection.RequestHasEnded(true);

        // There was an error on the connection
        else
            {
            m_rConnection.RequestHasEnded(false);
            m_rConnection.SetConnected(false);
            }
        }

    // The connection was stopped by the thread owner
    else
        m_rConnection.SetConnected(false);

    // Signal that something happened
    m_rConnection.m_HTTPEvent.Signal();

    // close the internet handle
    HFCMonitor Monitor(m_hRequest);
    InternetCloseHandle(m_hRequest);
    m_hRequest = NULL;
    }
#endif


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HTTPThread::Go()
    {
    WString Request;
    HFCSynchroContainer SynchroList;

    try
        {
        // prepare for multiple wait
        SynchroList.AddSynchro(&m_RequestEvent);
        SynchroList.AddSynchro(&m_StopEvent);

        // While there is requests to handle and the event wants the
        // thread to run, loop
        while (HFCSynchro::WaitForMultipleObjects(SynchroList, false) == 0)
            {
#if defined (ANDROID) || defined (__APPLE__)
                //DM-Android
#elif defined (_WIN32)
            // get the current request & handle it
            HandleRequest(GetRequest());
#endif

            }
        }
    catch(...)
        {
        }
    }




