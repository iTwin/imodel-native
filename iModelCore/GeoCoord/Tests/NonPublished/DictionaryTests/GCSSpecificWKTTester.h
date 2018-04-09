//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificWKTTester.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environment
class GCSSpecificWKTTester : public ::testing::TestWithParam< WString >
    {   

public:
  virtual void SetUp() {  };
  virtual void TearDown() {};

    GCSSpecificWKTTester();
    ~GCSSpecificWKTTester() {};
    };