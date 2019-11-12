//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSUnaryTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environment
class GCSUnaryTester : public ::testing::TestWithParam< WString >
    {   
    private: 
        static bool s_initialized;
  //      static GeoCoordinates::BaseGCSPtr s_LL84GCS;

    public:
        virtual void SetUp() {  };
        virtual void TearDown() {};

        GCSUnaryTester();
        ~GCSUnaryTester() {};

    };