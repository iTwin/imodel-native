//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoreferenceContext.h $
//:>    $RCSfile: HRFGeoreferenceContext.h,v $
//:>   $Revision: 1.1 $
//:>       $Date: 2011/12/22 14:03:54 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGeoreferenceContext
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPMPersistentObject.h>
#include <Imagepp/all/h/HRFRasterFile.h>


BEGIN_IMAGEPP_NAMESPACE
class HRFGeoreferenceContext : public HFCShareableObject<HRFGeoreferenceContext>
{
    // Class ID for this class.
    HDECLARE_BASECLASS_ID(HRFGeoreferenceContextId_Base)    
    
    public : 
        
        HRFGeoreferenceContext(double pi_defaultRatioToMeterForRaster,
                               double pi_defaultRatioToMeterForSisterFile,
                               bool   pi_useSisterFile,
                               bool   pi_usePCSLinearUnit,
                               bool   pi_interpretAsIntergraphUnit);
        
        virtual ~HRFGeoreferenceContext();

        double GetDefaultRatioToMeterForSisterFile() const;

        bool UpdateGeoreference(uint32_t              pi_page, 
                                HFCPtr<HRFRasterFile>& pio_rasterFile) const;
        
        bool UseSisterFile() const;
                
    private : 

        double m_defaultRatioToMeterForRaster;
        double m_defaultRatioToMeterForSisterFile;
        bool   m_useSisterFile;
        bool   m_usePCSLinearUnit;
        bool   m_interpretAsIntergraphUnit;        
};
END_IMAGEPP_NAMESPACE
