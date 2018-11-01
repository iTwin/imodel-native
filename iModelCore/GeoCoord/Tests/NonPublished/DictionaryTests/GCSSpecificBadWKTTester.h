//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificBadWKTTester.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environment
class GCSSpecificBadWKTTester : public ::testing::TestWithParam< WString >
    {   
    private: 
        static bool s_initialized;
    public:
        virtual void SetUp() {  };
        virtual void TearDown() {};

        GCSSpecificBadWKTTester();
        ~GCSSpecificBadWKTTester() {};
    };