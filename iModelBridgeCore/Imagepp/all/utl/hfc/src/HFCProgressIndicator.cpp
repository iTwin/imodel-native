//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCProgressIndicator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgressIndicator, HFCProgressListener, HFCProgressDurationListener
//----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCProgressIndicator.h>

//----------------------------------------------------------------------------

HFCProgressListener::HFCProgressListener()
    {
    }

//----------------------------------------------------------------------------

HFCProgressListener::~HFCProgressListener()
    {
    }


//----------------------------------------------------------------------------

HFCProgressDurationListener::HFCProgressDurationListener()
    : HFCProgressListener()
    {
    m_ToReset           = true;
    m_IsStarted         = false;
    m_ProcessedDuration = 0;
    m_CountIteration    = 0;
    m_BestDuration      = 0;
    m_WorstDuration     = 0;
    }

//----------------------------------------------------------------------------

HFCProgressDurationListener::~HFCProgressDurationListener()
    {
    }

//----------------------------------------------------------------------------

void HFCProgressDurationListener::Progression(
    HFCProgressIndicator* pi_pProgressIndicator,// Indicator
    uint64_t             pi_Processed,         // Total processed items count.
    uint64_t             pi_CountProgression)  // number of items.
    {
    if (m_IsStarted)
        {
        // stop the previous iteration time calculation
        m_finish = clock();

        double  duration = (double)(m_finish - m_start) / CLOCKS_PER_SEC;

        // if this is the first iteration
        if (m_CountIteration == 1)
            {
            m_WorstDuration  = duration;
            m_BestDuration   = duration;
            }
        else
            {
            if (m_WorstDuration < duration)
                m_WorstDuration = duration;

            if (m_BestDuration > duration)
                m_BestDuration = duration;
            }

        m_ProcessedDuration += duration;
        m_IsStarted = false;
        }

    // re-start for the new iteration
    m_start = clock();

    if (m_ToReset)
        {
        m_ProcessedDuration = 0;
        m_ToReset           = false;
        m_CountIteration    = 0;
        }

    ++m_CountIteration;
    m_IsStarted   = true;
    }

//----------------------------------------------------------------------------

HFCProgressIndicator::~HFCProgressIndicator()
    {
    // Deleting listeners still in list
    ListenerList::iterator itr = m_Listeners.begin();
    while (itr != m_Listeners.end())
        {
        delete (*itr);
        ++itr;
        }
    }

