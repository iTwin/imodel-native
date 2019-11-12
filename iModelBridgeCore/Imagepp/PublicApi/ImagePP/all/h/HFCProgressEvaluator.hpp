//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Inline methods for HFCProgressEvaluator
//----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------
// GetID
// Return the evaluator's identification number.
//----------------------------------------------------------------------------
inline HFCProgressEvaluator::ProgressEvaluatorID HFCProgressEvaluator::GetID() const
    {
    return m_ID;
    }

//----------------------------------------------------------------------------
// SetValue
// Set the current evaluation.
//----------------------------------------------------------------------------
inline void    HFCProgressEvaluator::SetValue(double pi_Value)
    {
    m_LastEvaluation = pi_Value;
    }

//----------------------------------------------------------------------------
// GetValue
// Get the current evaluation.
//----------------------------------------------------------------------------
inline double HFCProgressEvaluator::GetValue() const
    {
    return m_LastEvaluation;
    }

END_IMAGEPP_NAMESPACE