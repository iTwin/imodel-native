//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environment
class GCSSpecificWKTTester : public ::testing::TestWithParam< WString >
    {   
    private: 
        static bool s_initialized;

    public:
        virtual void SetUp() {  };
        virtual void TearDown() {};

        GCSSpecificWKTTester();
        ~GCSSpecificWKTTester() {};
    };