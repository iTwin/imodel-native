//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCProgressEvaluator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCProgressEvaluator
//----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCProgressEvaluator.h>

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