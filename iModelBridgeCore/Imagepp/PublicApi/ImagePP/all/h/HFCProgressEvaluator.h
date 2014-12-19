//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProgressEvaluator.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgressEvaluator
//----------------------------------------------------------------------------
#pragma once


class HFCProgressEvaluator : public HFCShareableObject<HFCProgressEvaluator>
    {
public:

    enum ProgressEvaluatorID
        {
        COMPRESSED_DATA_ESTIMATED_SIZE = 0 //Estimated size of compressed data during an export.
        };

    _HDLLu HFCProgressEvaluator(ProgressEvaluatorID pi_EvaluatorID);
    _HDLLu virtual ~HFCProgressEvaluator();

    _HDLLu ProgressEvaluatorID GetID() const;

    _HDLLu void                SetValue(double pi_Value);
    _HDLLu double                GetValue() const;

protected:

    ProgressEvaluatorID    m_ID;                //Evaluator Identification
    double                m_LastEvaluation;   //Value obtained during the last evaluation

private:

    // Disabled methods
    HFCProgressEvaluator(const HFCProgressEvaluator& pi_rObj);
    HFCProgressEvaluator& operator=(const HFCProgressEvaluator& pi_rObj);
    };


#include "HFCProgressEvaluator.hpp"