//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileExtender.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual ~HRFRasterFileExtender();

    // Returns the file exlusive key
    HFCExclusiveKey& GetKey() const override;

    virtual HFCPtr<HRFRasterFile>& GetOriginalFile() const;
    virtual HFCPtr<HRFRasterFile>  GetExtendedFile() const;

    virtual bool IsCacheExtender() const;

    void Save() override;

    HFCPtr<HMDContext>  GetContext(uint32_t pi_PageIndex) const override;

    void                SetContext(uint32_t           pi_PageIndex,
           const HFCPtr<HMDContext>& pi_rpContext) override;

    bool IsOriginalRasterDataStorage() const override;

    void CancelCreate() override;

protected:
    // Raster File
    HFCPtr<HRFRasterFile>   m_pOriginalFile;

    // Constructor
    HRFRasterFileExtender(const HFCPtr<HRFRasterFile>&  pi_rpOriginal);

private:
    // Disabled methods
    HRFRasterFileExtender(const HRFRasterFileExtender&);
    HRFRasterFileExtender& operator=(const HRFRasterFileExtender&);
    };
END_IMAGEPP_NAMESPACE

