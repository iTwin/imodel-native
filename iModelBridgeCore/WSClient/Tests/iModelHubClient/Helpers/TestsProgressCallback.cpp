/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "TestsProgressCallback.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
Http::Request::ProgressCallback TestsProgressCallback::Get()
    {
    return [&] (double bytesTransfered, double bytesTotal)
        {
        // Workaround for TFS#582017
        if (bytesTransfered == 0.0 && bytesTotal == 0.0 && m_lastProgressBytesTransfered > 0.0)
            {
            // Gets here if connection dies and query is repeated
            EXPECT_EQ(m_progressRetryCount, 0);
            m_progressRetryCount++;
            m_lastProgressBytesTransfered = 0.0;
            m_lastProgressBytesTotal = 0.0;
            }

        EXPECT_GE(bytesTransfered, m_lastProgressBytesTransfered);
        if (bytesTotal > 0.0)
            {
            if (m_lastProgressBytesTotal > 0.0)
                {
                EXPECT_EQ(bytesTotal, m_lastProgressBytesTotal);
                }

            m_lastProgressBytesTotal = bytesTotal;
            }

        m_lastProgressBytesTransfered = bytesTransfered;
        };
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
void TestsProgressCallback::Verify(bool completed)
    {
    if (completed)
        {
        EXPECT_GT(m_lastProgressBytesTotal, 0.0);
        EXPECT_LE(m_lastProgressBytesTransfered, m_lastProgressBytesTotal);
        }
    else
        {
        EXPECT_EQ(m_lastProgressBytesTransfered, 0.0);
        EXPECT_EQ(m_lastProgressBytesTotal, 0.0);
        }
    }
