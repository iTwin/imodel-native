/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/MultiProgressCallbackHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <cmath>
#include "MultiProgressCallbackHandler.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2017
//---------------------------------------------------------------------------------------
void MultiProgressCallbackHandler::AddCallback(Http::Request::ProgressCallback& callback, double percentageOfTotal)
    {
    BeMutexHolder lockProgress(m_progressMutex);
    int currentCount = m_progress.size();

    m_progress.Insert(currentCount, 0.0f);
    callback = [percentageOfTotal, this, currentCount](double bytesTransfered, double bytesTotal)
        {
        BeMutexHolder lockProgress(this->m_progressMutex);
        this->m_progress[currentCount] = bytesTotal > 0 ? percentageOfTotal * (bytesTransfered / bytesTotal) : 0;
        double totalProgress = 0.0f;
        for (auto progressItem : this->m_progress)
            {
            totalProgress += progressItem.second;
            }
            
        if (this->m_callback)
            this->m_callback(totalProgress, 100);
        };
    }

END_BENTLEY_IMODELHUB_NAMESPACE
