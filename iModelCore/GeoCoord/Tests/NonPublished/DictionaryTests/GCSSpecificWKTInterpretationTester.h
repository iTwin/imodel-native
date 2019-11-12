//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environment
class GCSSpecificWKTInterpretationTester : public ::testing::TestWithParam< bpair <WString, WString> >
    {   
    private: 
        static bool s_initialized;

    public:
        virtual void SetUp() {  };
        virtual void TearDown() {};

        GCSSpecificWKTInterpretationTester();
        ~GCSSpecificWKTInterpretationTester() {};
    };