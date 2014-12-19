//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequestProcessor.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequestProcessor
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/PUBRequestProcessor.h>
#include <Imagepp/all/h/PUBDumbAnalyzer.h>
#include <Imagepp/all/h/PUBCache.h>
#include <Imagepp/all/h/PUBCacheEntry.h>
#include <Imagepp/all/h/PUBRequest.h>
#include <Imagepp/all/h/PUBRequestDispatcher.h>

#include <Imagepp/all/h/HFCHTTPParser.h>
#include <Imagepp/all/h/HFCHTTPHeader.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const time_t s_ConnectionWaitTimeOut = 200;
static const time_t s_ReadTimeOut           = 5000;
static const string s_Marker("\r\n\r\n");

static const string s_Server("Server: Sebastien's two fingers in the nose Web Server 1.0");


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
PUBRequestProcessor::PUBRequestProcessor(HCSConnectionPool&    pi_rPool,
                                         PUBCache&             pi_rCache,
                                         PUBRequestDispatcher& pi_rDispatcher)
    : HCSRequestProcessor(pi_rPool),
      m_rCache(pi_rCache),
      m_rDispatcher(pi_rDispatcher)
    {
    // Build the dumb analyzer
    m_pAnalyzer = new PUBDumbAnalyzer;

    // Start the thread
    StartThread();
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
PUBRequestProcessor::~PUBRequestProcessor()
    {
    // Stop the thread
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


//-----------------------------------------------------------------------------
// Public
// Thread implementation
//-----------------------------------------------------------------------------
void PUBRequestProcessor::Go()
    {
    HFCInternetConnection*  pConnection;
    bool                   ValidRequest;
    string                  NewRequest;
    string                  StandardRequest;
    HFCPtr<PUBCacheEntry>   pCacheEntry;

    while (CanRun())
        {
        // Get the next connection from the pool
        pConnection = static_cast<HFCInternetConnection*>(GetActiveConnectionFromPool(s_ConnectionWaitTimeOut));
        HPRECONDITION((pConnection == 0) || (pConnection->IsCompatibleWith(HFCInternetConnection::CLASS_ID)));
        if (pConnection != 0)
            {
            // If the request is succesfully processed, the connection will be placed in
            // a request, which is given to
            ValidRequest = false;

            try
                {
                // get the next request from the connection
                NewRequest = GetRequestFromConnection(s_Marker, s_ReadTimeOut);

                HAutoPtr<HFCHTTPHeader> pHeader(new HFCHTTPHeader(1.0));
                pHeader->AddToHeader(s_Server);

                // parse the HTTP request
                HFCHTTPParser Parser(NewRequest);
                if (Parser.Parse())
                    {
                    // verify that this is a GET request
                    if ((Parser.GetMethod() == HFCHTTPParser::GET) ||
                        (Parser.GetMethod() == HFCHTTPParser::POST))
                        {
                        // the request is in fact the search part
                        NewRequest = Parser.GetSearchPart();

                        // Verify if the connection is valid.
                        HFCMonitor AnalyzerMonitor(m_AnalyzerKey);
                        if (m_pAnalyzer->IsQueryValid(NewRequest))
                            {
                            // standardize the query to improve cache hits in the future
                            StandardRequest = m_pAnalyzer->Standardize(NewRequest);
                            AnalyzerMonitor.ReleaseKey();

                            // The request is already in the cache
                            HFCPtr<PUBCacheEntry> pCacheEntry;
                            if ((pCacheEntry = m_rCache.GetEntry(StandardRequest)) != 0)
                                {
                                // Do not wait for the cache entry to become signaled,
                                // this is done by the GetEntry() method of the cache.
                                //pCacheEntry->WaitUntilSignaled();
                                HASSERT(pCacheEntry->GetResponseSize() > 0);

                                // send the cache entry data to the client
                                pConnection->Send(pCacheEntry->GetResponseData(),
                                                  pCacheEntry->GetResponseSize());
                                ValidRequest = true;

                                // replace the connection in the pool
                                ReleaseConnection();

                                // Release the cache entry so that other threads may use
                                // it or delete it.
                                pCacheEntry->Release();
                                pCacheEntry = 0;
                                }

                            // the entry is not already in the cache
                            else
                                {
                                // create the entry in the cache, which will return a reference to
                                // entry in the cache.
                                pCacheEntry = m_rCache.AddEntry(StandardRequest);
                                if (pCacheEntry != 0)
                                    {
                                    // create the new request object
                                    HAutoPtr<PUBRequest>
                                    pRequest(new PUBRequest(time(NULL),
                                                            m_pConnection.release(),
                                                            GetConnectionPool(),
                                                            pCacheEntry,
                                                            StandardRequest,
                                                            pHeader.release()));
                                    HASSERT(pRequest != 0);

                                    // Our job is done.  We can now give the request to the
                                    // request dispatcher.
                                    m_rDispatcher.AddRequest(pRequest.release());
                                    ValidRequest = true;

                                    // Release the cache entry so that other threads may use
                                    // it or delete it.
                                    pCacheEntry->Release();
                                    pCacheEntry = 0;
                                    }

                                // could not create the cache entry.
                                else
                                    {
                                    // inform the client
                                    // TODO
                                    }
                                }
                            }

                        // the connection is not valid.
                        else
                            {
                            // inform the client
                            // TODO
                            }
                        }

                    // could not parse the HTTP request
                    else
                        {
                        pHeader->SetStatusCode(HFCHTTPHeader::_400_BAD_REQUEST);

                        // send the response to the client
                        string HeaderString(pHeader->GetHeader());
                        pConnection->Send((const Byte*)HeaderString.data(), HeaderString.size());
                        }
                    }

                // could not parse the HTTP request
                else
                    {
                    pHeader->SetStatusCode(HFCHTTPHeader::_400_BAD_REQUEST);

                    // send the response to the client
                    string HeaderString(pHeader->GetHeader());
                    pConnection->Send((const Byte*)HeaderString.data(), HeaderString.size());
                    }
                }
            catch(...)
                {
                }

            // Depending on the state of the request and the connection, we give the connection
            // back to the pool.  Otherwise, we destroy the connection
            if (!ValidRequest)
                DestroyConnection();
            }
        }
    }



//-----------------------------------------------------------------------------
// Public
// Changes the configuration of the request processor
//-----------------------------------------------------------------------------
void PUBRequestProcessor::SetConfiguration(const PUBConfiguration& pi_rConfiguration)
    {
    HFCMonitor AnalyzerMonitor(m_AnalyzerKey);

    // the only thing that the processor can configure is the analyzer, so give
    // the configuration object to the analyzer
    m_pAnalyzer->SetConfiguration(pi_rConfiguration);
    }

