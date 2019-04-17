//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExceptionTester
//-----------------------------------------------------------------------------

#pragma once
 
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
struct ImagePPTestConfig : ImagePP::ImageppLib::Host
    {
    ImagePPTestConfig();

    virtual ~ImagePPTestConfig();

    static ImagePPTestConfig& GetConfig();

    // from ImagePP::ImageppLib::Host
    virtual void _RegisterFileFormat() override;

    virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override;

    void SetUp() { /*occurs once during construction*/}

    BeFileNameCR GetSourceDir() const { return m_sourceDir; }   

    BeFileNameCR GetFileFormatOutputDir() const {return m_fileFormatOutputDir;}

    BeFileNameCR GetBaselineDir() const { return m_baselineDir; }

    bool CompareAgainsBaseline() const { return !GetBaselineDir().IsEmpty(); }

    bool DoMd5() const { return m_md5Validation; }

    bool ValidateExportDuration() const {return m_validateDuration;}

    uint64_t GetDuratationThreshold() const { return m_durationThresholdMs; }

    double GetToleranceRatio() const { return m_toleranceRatio; }

   private:          
        BeFileName              m_baselineDir;
        BeFileName              m_sourceDir;
        BeFileName              m_fileFormatOutputDir;

        bool                    m_md5Validation = false;
        bool                    m_validateDuration = false;     
        double                  m_toleranceRatio = 10;          //! Fails when there is more than 10% difference
        uint64_t                m_durationThresholdMs = 5000;   //! Anything under the threshold is not validated
    };
