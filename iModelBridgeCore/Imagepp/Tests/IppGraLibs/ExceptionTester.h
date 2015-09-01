//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/ExceptionTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExceptionTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement

class ExceptionTester : public testing::Test 
    {   

    protected :
    
    ExceptionTester();
    ~ExceptionTester() {};

    HFCPtr<HPANode> m_TestNode;
    };