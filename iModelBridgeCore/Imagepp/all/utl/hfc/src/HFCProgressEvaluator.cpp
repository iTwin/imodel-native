//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCProgressEvaluator.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgressEvaluator
//----------------------------------------------------------------------------
#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCProgressEvaluator.h>

//----------------------------------------------------------------------------
// HFCProgressEvaluator
// Constructor
//----------------------------------------------------------------------------
HFCProgressEvaluator::HFCProgressEvaluator(ProgressEvaluatorID pi_EvaluatorID)
    {
    m_ID = pi_EvaluatorID;
    }

//----------------------------------------------------------------------------
// ~HFCProgressEvaluator
// Desctructor
//----------------------------------------------------------------------------
HFCProgressEvaluator::~HFCProgressEvaluator()
    {
    }
