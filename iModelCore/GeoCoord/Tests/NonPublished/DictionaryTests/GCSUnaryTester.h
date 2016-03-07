//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSUnaryTester.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSUserDomainTester
//-----------------------------------------------------------------------------

#pragma once


 
// Preparation of required environement
class GCSUnaryTester : public ::testing::TestWithParam< WString >
    {   

public:
  virtual void SetUp() {  };
  virtual void TearDown() {};

    GCSUnaryTester();
    ~GCSUnaryTester() {};
    };