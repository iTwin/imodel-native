//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificTransformTester.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environement
class GCSSpecificTransformTester : public ::testing::TestWithParam< WString >
    {   

public:
  virtual void SetUp() {  };
  virtual void TearDown() {};

    GCSSpecificTransformTester();
    ~GCSSpecificTransformTester() {};
    };