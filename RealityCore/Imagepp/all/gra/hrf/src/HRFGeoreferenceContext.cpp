//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGeoreferenceContext
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>



#include <ImagePP/all/h/HRFGeoreferenceContext.h>


HRFGeoreferenceContext::HRFGeoreferenceContext(double pi_defaultRatioToMeterForRaster,
                                               double pi_defaultRatioToMeterForSisterFile,
                                               bool   pi_useSisterFile,
                                               bool   pi_usePCSLinearUnit,
                                               bool   pi_interpretAsIntergraphUnit)
{
    m_defaultRatioToMeterForRaster = pi_defaultRatioToMeterForRaster;
    m_defaultRatioToMeterForSisterFile = pi_defaultRatioToMeterForSisterFile;
    m_useSisterFile = pi_useSisterFile;
    m_usePCSLinearUnit = pi_usePCSLinearUnit;
    m_interpretAsIntergraphUnit = pi_interpretAsIntergraphUnit;
}

HRFGeoreferenceContext::~HRFGeoreferenceContext() 
{
}

double HRFGeoreferenceContext::GetDefaultRatioToMeterForSisterFile() const
{
    return m_defaultRatioToMeterForSisterFile;
}

bool HRFGeoreferenceContext::UpdateGeoreference(uint32_t              pi_page, 
                                                HFCPtr<HRFRasterFile>& pio_rasterFile) const
{
    bool isGeoreferenceUpdated = false;

    if (pio_rasterFile->CountPages() > pi_page) 
    {    
        pio_rasterFile->SetDefaultRatioToMeter(m_defaultRatioToMeterForRaster, 
                                               pi_page, 
                                               m_usePCSLinearUnit, 
                                               m_interpretAsIntergraphUnit);

        isGeoreferenceUpdated = true;
    }
    
    return isGeoreferenceUpdated;

}
        
bool HRFGeoreferenceContext::UseSisterFile() const
{
    return m_useSisterFile;
}       
