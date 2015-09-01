//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCProgressEvaluator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgressEvaluator
//----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HFCProgressEvaluator : public HFCShareableObject<HFCProgressEvaluator>
    {
public:

    enum ProgressEvaluatorID
        {
        COMPRESSED_DATA_ESTIMATED_SIZE = 0 //Estimated size of compressed data during an export.
        };

    IMAGEPP_EXPORT HFCProgressEvaluator(ProgressEvaluatorID pi_EvaluatorID);
    IMAGEPP_EXPORT virtual ~HFCProgressEvaluator();

    IMAGEPP_EXPORT ProgressEvaluatorID GetID() const;

    IMAGEPP_EXPORT void                SetValue(double pi_Value);
    IMAGEPP_EXPORT double                GetValue() const;

protected:

    ProgressEvaluatorID    m_ID;                //Evaluator Identification
    double                m_LastEvaluation;   //Value obtained during the last evaluation

private:

    // Disabled methods
    HFCProgressEvaluator(const HFCProgressEvaluator& pi_rObj);
    HFCProgressEvaluator& operator=(const HFCProgressEvaluator& pi_rObj);
    };

END_IMAGEPP_NAMESPACE

#include "HFCProgressEvaluator.hpp"