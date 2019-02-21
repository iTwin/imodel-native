//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificWKTInterpretationTester.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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