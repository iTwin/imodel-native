//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificTransformTester.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : GCSSpecificTransformTester
//-----------------------------------------------------------------------------

#pragma once
/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Alain.Robert  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct conversionTest
    {
    WString m_sourceGCS;
    WString m_targetGCS;
    double  m_inputCoordinateX;
    double  m_inputCoordinateY;
    double  m_inputCoordinateZ;
    double  m_outputCoordinateX;
    double  m_outputCoordinateY;
    double  m_outputCoordinateZ;
    bool    m_linearUnit;
    };

 
// Preparation of required environement
class GCSSpecificTransformTester : public ::testing::TestWithParam< conversionTest >
    {   
    private: 
        static bool s_initialized;
    public:
        virtual void SetUp() {  };
        virtual void TearDown() {};

        GCSSpecificTransformTester();
        ~GCSSpecificTransformTester() {};
    };