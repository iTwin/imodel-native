//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSUnaryTester.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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