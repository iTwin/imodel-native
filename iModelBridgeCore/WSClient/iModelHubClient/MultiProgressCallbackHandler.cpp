/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
void MultiProgressCallbackHandler::AddCallback(Http::Request::ProgressCallback& callback)
    {
    BeMutexHolder lockProgress(m_progressMutex);
    int currentCount = m_progress.size();

    m_progress.Insert(currentCount, 0.0);
    callback = [this, currentCount](double bytesTransfered, double bytesTotal)
        {
            {
            BeMutexHolder lockProgress(this->m_progressMutex);
            double difference = bytesTransfered - this->m_progress[currentCount];
            if (difference > 0.0)
                m_bytesTransfered += bytesTransfered - this->m_progress[currentCount];
            this->m_progress[currentCount] = bytesTransfered;
            }

        if (this->m_callback)
            this->m_callback(m_bytesTransfered, m_bytesTotal);
        };
    }

END_BENTLEY_IMODELHUB_NAMESPACE
