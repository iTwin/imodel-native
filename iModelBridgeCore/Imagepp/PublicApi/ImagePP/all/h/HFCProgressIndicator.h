//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProgressIndicator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgress
//----------------------------------------------------------------------------
#pragma once

#include "HFCProgressEvaluator.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCProgressIndicator;
//----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    @version 1.0

    The HFCProgressListener object could be use when you want the
    feedback of HFCProgressIndicator. You have to connect the listener
    with the indicator to receive the feedback. It is possible to
    connect at the same time your listener on different indicators.
    You can connect or unplug your listener at any time. On each
    iteration processing you will be notify on your redefined virtual
    Progression function. If the indicator iteration processing is
    stopped no feedback will be send to the listener.

    -----------------------------------------------------------------------------
*/
class HFCProgressListener
    {
public:
    IMAGEPP_EXPORT HFCProgressListener();
    IMAGEPP_EXPORT virtual ~HFCProgressListener();


    /** -----------------------------------------------------------------------------
        On each iteration processing you will be notify on this function.
        This function must be redefined in your child object.

        @param pi_pProgressIndicator Specify the indicator that communicates
                                     with the listener.

        @param pi_Processed The current count of processed iteration by the
                            specified indicator.

        @param pi_CountProgression The total iteration to do. The count can be set
                                   to 0. The 0 value indicate that unknown the
                                   number of iteration. In this case we indicate
                                   is a life signal only.

        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator,     // Indicator
                                    uint64_t             pi_Processed,              // Total processed items count.
                                    uint64_t             pi_CountProgression) = 0;  // number of items.
    };

//----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    @version 1.0

    The HFCProgressDurationListener object could be use when you want the
    feedback of HFCProgressIndicator. You have to connect your listener with
    the indicator to receive the feedback. It is possible to connect at the
    same time your listener on different indicators. You can connect or
    unplug your listener at any time. On each iteration processing you will
    be notify on your redefined virtual Progression function. If the
    indicator iteration processing is stopped no feedback will be send to
    the listener. The listener estimates the duration in milliseconds for
    the best, the worst and the average iteration.

    -----------------------------------------------------------------------------
*/
class HFCProgressDurationListener : public HFCProgressListener
    {
public:
    IMAGEPP_EXPORT HFCProgressDurationListener();
    IMAGEPP_EXPORT virtual ~HFCProgressDurationListener();

    IMAGEPP_EXPORT virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator,// Indicator
                                    uint64_t             pi_Processed,         // Total processed items count.
                                    uint64_t             pi_CountProgression); // number of items.

    // best\worst\average iteration duration in milliseconds
    double GetBestDuration() const;
    double GetAverageDuration() const;
    double GetWorstDuration() const;

    // Processed iteration duration in milliseconds
    double GetProcessedDuration() const;

protected:

    // Reset the duration values and iteration count
    void Reset();

    uint64_t m_CountIteration;

    // Iteration Duration
    double m_BestDuration;
    double m_WorstDuration;

    // Total processed items duration.
    double m_ProcessedDuration;

    clock_t m_start;
    clock_t m_finish;

    bool m_ToReset;
    bool m_IsStarted;
    };

//----------------------------------------------------------------------------

/**

    The HFCProgressIndicator object could be use when you have a multiple of
    iterations processing that take a lot of time. The HFCProgressIndicator
    goal is to give a feedback at each progression. The feedback is send to
    the connected listener. When you are not interested to be notified in
    your listener unplug it from the indicator with the @k{RemoveListener}
    functions. After you can reconnect it at any time to receive the
    feedback with the @k{AddListener} functions. The indicator has the
    capabilities to be connected with a multiple of listeners. You can stop
    or restart the iteration processing at any times. If you stop the
    iteration processing no feedback is send to the connected listener.

    @see  HFCProgressListener

*/


class HFCProgressIndicator
    {
public:
    IMAGEPP_EXPORT HFCProgressIndicator();
    IMAGEPP_EXPORT virtual ~HFCProgressIndicator();

    IMAGEPP_EXPORT virtual void StopIteration();

    bool IsLifeSignal() const;

    void  AddListener(HFCProgressListener* pi_pListener);
    void  RemoveListener(HFCProgressListener* pi_pListener);

    void  Restart(uint64_t pi_CountIteration);

    // at each iteration call the Iteration methodes
    // We information the indicator about present object processing
    bool                ContinueIteration();
    bool                IsIterationStopped() const;

    uint64_t GetProcessedIteration() const;

    bool   AddEvaluator(HFCPtr<HFCProgressEvaluator>& pi_prEvaluator);
    bool   RemoveEvaluator(HFCProgressEvaluator::ProgressEvaluatorID pi_EvaluatorID);
    bool    GetEvaluator(HFCProgressEvaluator::ProgressEvaluatorID pi_EvaluatorID,
                          HFCPtr<HFCProgressEvaluator>&               po_prProgressEvaluator);
    bool    HasEvaluator();


protected:
    friend class HFCProgressListener;

    typedef list<HFCProgressListener*>          ListenerList;
    typedef list<HFCPtr<HFCProgressEvaluator> > EvaluatorList;

    EvaluatorList   m_Evaluators;
    ListenerList    m_Listeners;
    uint64_t        m_IterationProcessed;
    uint64_t        m_CountIteration;
    bool            m_Continue;
    uint64_t        m_NumberOfListener;
    };

//----------------------------------------------------------------------------
END_IMAGEPP_NAMESPACE

#include "HFCProgressIndicator.hpp"
