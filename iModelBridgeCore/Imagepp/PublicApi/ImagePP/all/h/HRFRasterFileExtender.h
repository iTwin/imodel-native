//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileExtender.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFRasterFileExtender
//-----------------------------------------------------------------------------
// This class describes the basic interface of a raster file format
//-----------------------------------------------------------------------------

#pragma once

//#include "HFCAccessMode.h"
#include "HFCPtr.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFileExtender : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFRasterFileId_Extender, HRFRasterFile)

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual ~HRFRasterFileExtender();

    // Returns the file exlusive key
    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual HFCExclusiveKey& GetKey() const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual HFCPtr<HRFRasterFile>& GetOriginalFile() const;
    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual HFCPtr<HRFRasterFile>  GetExtendedFile() const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual bool IsCacheExtender() const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual void Save();

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual HFCPtr<HMDContext>  GetContext(uint32_t pi_PageIndex) const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual void                SetContext(uint32_t           pi_PageIndex,
                                               const HFCPtr<HMDContext>& pi_rpContext);

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual bool IsOriginalRasterDataStorage() const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ virtual void CancelCreate();

protected:
    // Raster File
    HFCPtr<HRFRasterFile>   m_pOriginalFile;

    // Constructor
    IMAGEPP_EXPORT /*IppImaging_Needs*/ HRFRasterFileExtender(const HFCPtr<HRFRasterFile>&  pi_rpOriginal);

private:
    // Disabled methods
    HRFRasterFileExtender(const HRFRasterFileExtender&);
    HRFRasterFileExtender& operator=(const HRFRasterFileExtender&);
    };
END_IMAGEPP_NAMESPACE

