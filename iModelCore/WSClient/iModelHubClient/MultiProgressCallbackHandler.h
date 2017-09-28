/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/MultiProgressCallbackHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct MultiProgressCallbackHandler> MultiProgressCallbackHandlerPtr;

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             09/2017
//=======================================================================================
struct MultiProgressCallbackHandler : RefCountedBase
{
private:
    Http::Request::ProgressCallbackCR m_callback;
    bmap<int, double> m_progress;
    BeMutex m_progressMutex;
public:
    MultiProgressCallbackHandler(Http::Request::ProgressCallbackCR callback) : m_callback(callback) {}
    IMODELHUBCLIENT_EXPORT void AddCallback(Http::Request::ProgressCallback& callback, double percentageOfTotal);
    void SetFinished() const { if (m_callback) { m_callback(100.0f, 100.0f); } };
};

END_BENTLEY_IMODELHUB_NAMESPACE
