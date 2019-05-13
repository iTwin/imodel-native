/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    double m_bytesTotal = 0.0;
    double m_bytesTransfered = 0.0;
    Http::Request::ProgressCallbackCR m_callback;
    bmap<int, double> m_progress;
    BeMutex m_progressMutex;
public:
    MultiProgressCallbackHandler(Http::Request::ProgressCallbackCR callback, double bytesTotal) : m_callback(callback), m_bytesTotal(bytesTotal) {}
    IMODELHUBCLIENT_EXPORT void AddCallback(Http::Request::ProgressCallback& callback);
    void SetFinished() const { if (m_callback) { m_callback(m_bytesTotal, m_bytesTotal); } };
};

END_BENTLEY_IMODELHUB_NAMESPACE
