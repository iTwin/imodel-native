//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProgressIndicator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Inline methods for progression indicator classes
//----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------
// Reset the duration values and iteration count
//----------------------------------------------------------------------------
inline void HFCProgressDurationListener::Reset()
    {
    m_ToReset   = true;
    m_IsStarted = false;
    }

/** -----------------------------------------------------------------------------
    The best iteration duration in milliseconds.

    @return Return the best iteration duration in milliseconds.

    @see GetWorstDuration()
    @see GetAverageDuration()
    -----------------------------------------------------------------------------
*/
inline double HFCProgressDurationListener::GetBestDuration() const
    {
    return m_BestDuration;
    }

/** -----------------------------------------------------------------------------
    The average iteration duration in milliseconds.

    @return Return the average iteration duration in milliseconds.

    @see GetWorstDuration()
    @see GetBestDuration()
    -----------------------------------------------------------------------------
*/
inline double HFCProgressDurationListener::GetAverageDuration() const
    {
    double AverageDuration;

    if ((m_CountIteration >= 1) && (m_ProcessedDuration != 0.0))
        AverageDuration = m_ProcessedDuration / (double) (m_CountIteration - 1);
    else
        AverageDuration = 0.0;

    return AverageDuration;
    }

/** -----------------------------------------------------------------------------

    The worst iteration duration in milliseconds.

    @return Return the worst iteration duration in milliseconds.

    @see GetAverageDuration()
    @see GetBestDuration()
    -----------------------------------------------------------------------------
*/
inline double HFCProgressDurationListener::GetWorstDuration() const
    {
    return m_WorstDuration;
    }

/** -----------------------------------------------------------------------------

    The total processed iteration duration in milliseconds.


    @return Return the total processed iteration duration in milliseconds.

    -----------------------------------------------------------------------------
*/
inline double HFCProgressDurationListener::GetProcessedDuration() const
    {
    return m_ProcessedDuration;
    }




//****************************************************************************
//****************************************************************************
// HFCProgressIndicator
//****************************************************************************


/**---------------------------------------------------------------------------
 Constructor for this class. At the construction time the iteration
 processing is not stopped and the count iteration is set to 0. The zero
 value indicates that the number of iteration is unknown. In this case we
 indicate is a life signal only.
----------------------------------------------------------------------------*/
inline HFCProgressIndicator::HFCProgressIndicator()
    {
    Restart(0);
    m_NumberOfListener = 0;
    }

/**---------------------------------------------------------------------------
 Returns the indicator mode.

 @return Returns true if the indicator is life signal type. Returns false if
         the indicator is progression type that knows the iteration count.
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::IsLifeSignal() const
    {
    return (m_CountIteration == 0);
    }

/**---------------------------------------------------------------------------
 Connect a listener to the progress indicator. The specified listener
 will be notified after each progression step.

 @note Your listener must be removed from the indicator before you destroy it.

 @param pi_pListener Pointer to the listener to connect to this indicator.
----------------------------------------------------------------------------*/
inline void HFCProgressIndicator::AddListener(HFCProgressListener* pi_pListener)
    {
    HPRECONDITION(pi_pListener != 0);
    m_Listeners.push_back(pi_pListener);
    ++m_NumberOfListener;
    }

/**---------------------------------------------------------------------------
 Unplug a listener from the progress indicator.

 @param pi_pListener Pointer to the listener to disconnect from this indicator.
                     That listener must be connected to the indicator.
----------------------------------------------------------------------------*/
inline void HFCProgressIndicator::RemoveListener(HFCProgressListener* pi_pListener)
    {
    HPRECONDITION(pi_pListener != 0);
    ListenerList::iterator itr = find(m_Listeners.begin(), m_Listeners.end(), pi_pListener);
    HASSERT(itr != m_Listeners.end());
    m_Listeners.erase(itr);
    m_NumberOfListener--;
    }

/**---------------------------------------------------------------------------
 Restarts the iteration processing that have been previously stopped by a call
 to the @k{Stop} method.

 @param pi_CountIteration Specifies the number of iteration steps to count.
                          Can be set to zero, indicating that the number of
                          iterations is unknown. In this case we indicate
                          a life signal only. The count is returned on
                          notification to the connected listener.



 @see Stop
----------------------------------------------------------------------------*/
inline void HFCProgressIndicator::Restart(uint64_t pi_CountIteration)
    {
    m_IterationProcessed = 0;
    m_CountIteration     = pi_CountIteration;
    m_Continue = true;
    }

/**---------------------------------------------------------------------------
 Stops the iteration processing. After the current iteration processing the
 next iteration will be not processed. To continue the iteration processing
 call the Restart method.

 @see Restart
----------------------------------------------------------------------------*/
inline void HFCProgressIndicator::StopIteration()
    {
    m_Continue = false;
    }

/**---------------------------------------------------------------------------

Informs the indicator that we want to continue the processing of the next
iteration. This method increment the processed count received by the
connected listener. If the iteration is not stopped before the call of
this methods. Each connected listener will be notified about the current
iteration processing.

----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::ContinueIteration()
    {
    if ((m_Continue) && (m_NumberOfListener > 0))
        {
        ListenerList::iterator itr = m_Listeners.begin();
        while (itr != m_Listeners.end())
            {
            (*itr)->Progression(this, m_IterationProcessed, m_CountIteration);
            ++itr;
            }
        m_IterationProcessed++;
        }
    return m_Continue;
    }

/**---------------------------------------------------------------------------
Return the iteration processing state.

@return Return true if the iteration processing is stopped. Return false
        if the iteration is processed.

Example:
@code
UInt32 BlockPosX;
UInt32 BlockPosY;
pExportProgressIndicator->Restart(pDstResDesc->CountBlocks());
for (UInt32 Index = 0 ; ((Index < pDstResDesc->CountBlocks()) &&
                   pExportProgressIndicator->ContinueIteration()) ; Index++)
{
       …
       for (UInt32 Pix = 0; ((Pix < CountPixels()) &&
              !pExportProgressIndicator->IsIterationStopped ()) ; Pix++)
       {
                …
       }
}
@end

@see StopIteration
@see Start
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::IsIterationStopped() const
    {
    return !m_Continue;
    }

/**---------------------------------------------------------------------------
Returns the number of processed iteration.

@return Returns the current iteration count.
----------------------------------------------------------------------------*/
inline uint64_t HFCProgressIndicator::GetProcessedIteration() const
    {
    return m_IterationProcessed;
    }

/**---------------------------------------------------------------------------
Add an evaluator to the progress indicator

@return Returns true if no other evaluator with the same ID had already been
        added.
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::AddEvaluator(HFCPtr<HFCProgressEvaluator>& pi_prEvaluator)
    {
    bool                    IsToBeAdd = true;
    EvaluatorList::iterator EvaluatorIter = m_Evaluators.begin();

    while (EvaluatorIter != m_Evaluators.end())
        {
        if (pi_prEvaluator->GetID() == (*EvaluatorIter)->GetID())
            {
            IsToBeAdd = false;
            break;
            }
        EvaluatorIter++;
        }

    if (IsToBeAdd == true)
        {
        m_Evaluators.push_back(pi_prEvaluator);
        }

    return IsToBeAdd;
    }

/**---------------------------------------------------------------------------
Remove an evaluator attached to the progress indicator

@return Returns true if the evaluator to remove was found.
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::RemoveEvaluator(HFCProgressEvaluator::ProgressEvaluatorID pi_EvaluatorID)
    {
    bool                    IsFound = false;
    EvaluatorList::iterator EvaluatorIter = m_Evaluators.begin();

    while (EvaluatorIter != m_Evaluators.end())
        {
        if (pi_EvaluatorID == (*EvaluatorIter)->GetID())
            {
            IsFound = true;
            m_Evaluators.erase(EvaluatorIter);
            break;
            }
        EvaluatorIter++;
        }
    return IsFound;
    }


/**---------------------------------------------------------------------------
Get an evaluator attached to the progress indicator

@return Returns true if the evaluator to remove was found.
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::GetEvaluator(HFCProgressEvaluator::ProgressEvaluatorID pi_EvaluatorID,
                                                HFCPtr<HFCProgressEvaluator>&              po_prProgressEvaluator)
    {
    bool                    IsFound = false;
    EvaluatorList::iterator EvaluatorIter = m_Evaluators.begin();

    while (EvaluatorIter != m_Evaluators.end())
        {
        if (pi_EvaluatorID == (*EvaluatorIter)->GetID())
            {
            po_prProgressEvaluator = *EvaluatorIter;
            IsFound = true;
            break;
            }
        EvaluatorIter++;
        }

    return IsFound;
    }

/**---------------------------------------------------------------------------
Indicate if the indicator has one or more evaluators.

@return Returns true if the indicator has more than one evaluator.
----------------------------------------------------------------------------*/
inline bool HFCProgressIndicator::HasEvaluator()
    {
    return m_Evaluators.size() > 0;
    }

END_IMAGEPP_NAMESPACE