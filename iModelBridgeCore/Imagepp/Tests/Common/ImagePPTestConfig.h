//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/Common/ImagePPTestConfig.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    std::list<std::wstring> const & GetSourceList() const { return m_sourceFileList; }

    BeFileNameCR GetSourceDir() const { return m_sourceDir; }   

    BeFileNameCR GetBaselineDir() const { return m_baselineDir; }

    bool ValidateExportDuration() const {return m_validateDuration;}

    uint64_t DuratationThreshold() const { return m_durationThreshold; }

    float   GetToleranceRatio() const { return m_toleranceRatio; }

   private:
        std::list<std::wstring> BuildFileList(BeFileNameCR directory);
            
        BeFileName              m_baselineDir;
        BeFileName              m_sourceDir;
        std::list<std::wstring> m_sourceFileList;

        
        bool                    m_validateDuration;
        float                   m_toleranceRatio;
        uint64_t                m_durationThreshold;
    };
