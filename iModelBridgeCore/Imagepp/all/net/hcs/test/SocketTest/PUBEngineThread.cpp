//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBEngineThread.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBEngineThread
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/PUBEngineThread.h>
#include <Imagepp/all/h/PUBRequest.h>
#include <Imagepp/all/h/PUBRequestDispatcher.h>

#include <Imagepp/all/h/HFCHTTPHeader.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>

static const string s_Root("h:\\Inetpub\\wwwroot");
//static const string s_Root("\\\\Vieux_pot\\Coreresult\\Install_RecentBuild_MSI_2.0.300.4\\MSIServerImager\\Help");

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

static const string s_Server("Server: Sebastien's two fingers in the nose Web Server 1.0");
static const string s_HTMLContent("Content-type: text/html");
static const string s_JpegContent("Content-type: image/jpeg");
static const string s_GifContent("Content-type: image/gif");
static const string s_ContentLength("Content-lenght: ");


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
PUBEngineThread::PUBEngineThread(uint32_t              pi_ID,
                                 PUBRequestDispatcher& pi_rDispatcher)
    : m_ID(pi_ID),
      m_rDispatcher(pi_rDispatcher)
    {
    StartThread();
    }


//-----------------------------------------------------------------------------
// Public
// Destroyer
//-----------------------------------------------------------------------------
PUBEngineThread::~PUBEngineThread()
    {
    StopThread();
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// Private
// Start the route print command
//-----------------------------------------------------------------------------
BOOL StartCommand(const string& pi_rCommand, HANDLE pi_StdoutWr)
    {
#ifdef _WIN32

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    // Set up members of STARTUPINFO structure.
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    siStartInfo.wShowWindow = (unsigned short) WS_DISABLED;
    siStartInfo.hStdOutput = pi_StdoutWr;

    // Create the child process.
    return CreateProcess(NULL,
                         (char*)pi_rCommand.c_str(),
                         NULL,          // process security attributes
                         NULL,          // primary thread security attributes
                         true,          // handles are inherited
                         0,             // creation flags
                         NULL,          // use parent's environment
                         "d:\\php4",          // use parent's current directory
                         &siStartInfo,  // STARTUPINFO pointer
                         &piProcInfo);  // receives PROCESS_INFORMATION

#else

    return false;

#endif
    }


//-----------------------------------------------------------------------------
// Private
// Read contents of file in a string
//-----------------------------------------------------------------------------
void ReadContents(const string& pi_rCommand, string& pi_rContents)
    {
    // Clear the string
    pi_rContents.resize(0);

#ifdef _WIN32

    HANDLE hChildStdoutRd;
    HANDLE hChildStdoutWr;
    HANDLE hChildStdoutRdDup;

    SECURITY_ATTRIBUTES saAttr;
    BOOL fSuccess;

    // Set the bInheritHandle flag so pipe handles are inherited.
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = true;
    saAttr.lpSecurityDescriptor = NULL;

    if (CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
        {
        if (DuplicateHandle(GetCurrentProcess(),
                            hChildStdoutRd,
                            GetCurrentProcess(),
                            &hChildStdoutRdDup ,
                            0,
                            false,
                            DUPLICATE_SAME_ACCESS))
            {
            CloseHandle(hChildStdoutRd);

            // Now create the child process.
            fSuccess = StartCommand(pi_rCommand, hChildStdoutWr);

            CloseHandle(hChildStdoutWr);

            // Read from pipe that is the standard output for child process.
            if (fSuccess)
                {
                DWORD dwRead;
                CHAR chBuf[1024];

                // Read output from the child process
                while (ReadFile( hChildStdoutRdDup, chBuf, 1024, &dwRead, NULL) &&
                       dwRead != 0)
                    {
                    pi_rContents.append(chBuf, dwRead);
                    }
                }

            CloseHandle(hChildStdoutRdDup);
            }
        }
#endif
    }


//-----------------------------------------------------------------------------
// Public
// Engine implementation
//-----------------------------------------------------------------------------
void PUBEngineThread::Go()
    {
    HFCPtr<PUBRequest> pRequest;

    while (CanRun())
        {
        // get a request from the dispatcher
        pRequest = m_rDispatcher.GetRequest(*this, 500);
        if (pRequest != 0)
            {
            try
                {
                string RequestString(pRequest->GetRequestString());
                while ((RequestString[0] == '\\') ||
                       (RequestString[0] == '/') )
                    RequestString.erase(0, 1);

                // build the name of the file to transfert
                wostriostringstreamngstream FileName;
                FileName << s_Root;
                FileName << "\\" << RequestString;

                // get the extension
                string Extension;
                string::size_type DotPos = RequestString.find_last_of(".");
                if (DotPos != string::npos)
                    Extension = RequestString.substr(DotPos + 1);
                ctype<char>().tolower(Extension.begin(), Extension.end());

                // get a ref to the request's HTTPHeader
                HFCHTTPHeader& rHeader = pRequest->GetHTTPHeader();

                // prepare the header depending
                if ((Extension.compare("jpeg") == 0) || (Extension.compare("jpg") == 0))
                    rHeader.AddToHeader(s_JpegContent);
                else if (Extension.compare("gif") == 0)
                    rHeader.AddToHeader(s_GifContent);
                else
                    rHeader.AddToHeader(s_HTMLContent);

                // if the page is PHP, then call the generator
                if (Extension.find("php") != string::npos)
                    {
                    // generate the page
                    string Result;
                    ostringstream CommandLine;
                    CommandLine << "d:\\php4\\php.exe " << FileName.str();
                    ReadContents(CommandLine.str(), Result);

                    // send the header & result
                    ostringstream HTTPHeader;
                    HTTPHeader << "HTTP/1.1 200 OK\r\n";
                    HTTPHeader << s_Server;
                    pRequest->GetConnection().Send((const Byte*)HTTPHeader.str().data(), HTTPHeader.str().size());
                    pRequest->GetCacheEntry().AddData(HTTPHeader.str());
                    pRequest->GetConnection().Send((const Byte*)Result.data(), Result.size());
                    pRequest->GetCacheEntry().AddData(Result);
                    }
                else
                    {
                    // try to open the file
                    HAutoPtr<HFCLocalBinStream> pFile;
                    try
                        {
                        pFile = new HFCLocalBinStream(FileName.str(), HFC_READ_ONLY);
                        }
                    catch(...)
                        {
                        pFile = 0;

                        rHeader.SetStatusCode(HFCHTTPHeader::_404_NOT_FOUND);
                        rHeader.AddToHeader(s_Server);

                        // send the response to the client
                        string HeaderString(rHeader.GetHeader());
                        pRequest->GetConnection().Send((const Byte*)HeaderString.data(), HeaderString.size());
                        }

                    if (pFile != 0)
                        {
                        rHeader.SetStatusCode(HFCHTTPHeader::_200_OK);

                        // add the content-length to the header
                        ostringstream Length;
                        Length << s_ContentLength << (uint32_t)pFile->GetSize();
                        rHeader.AddToHeader(Length.str());

                        // send the header
                        string HeaderString(rHeader.GetHeader());
                        pRequest->GetConnection().Send((const Byte*)HeaderString.data(), HeaderString.size());
                        pRequest->GetCacheEntry().AddData(HeaderString);


                        Byte Buffer[4096];
                        size_t ReadSize;
                        while ((ReadSize = pFile->Read(Buffer, 4096)) > 0)
                            {
                            pRequest->GetConnection().Send(Buffer, ReadSize);
                            pRequest->GetCacheEntry().AddData(Buffer, ReadSize);
                            }
                        }
                    }

                static const string s_Marker("\r\n\r\n");
                pRequest->GetConnection().Send((const Byte*)s_Marker.data(), s_Marker.size());
                pRequest->GetCacheEntry().AddData(s_Marker);


                // release the connection to the pool if the whole operation was succesful
                //pRequest->ReleaseConnectionToPool();
                }
            catch(...)
                {
                }

            // release our reference to the request.
            pRequest = 0;
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// Gets the assigned request dispatcher
//-----------------------------------------------------------------------------
PUBRequestDispatcher& PUBEngineThread::GetRequestDispatcher() const
    {
    return m_rDispatcher;
    }
