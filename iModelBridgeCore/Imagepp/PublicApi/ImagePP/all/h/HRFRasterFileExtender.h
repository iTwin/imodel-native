//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileExtender.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HRFRasterFileExtender : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(1469, HRFRasterFile)

    _HDLLg /*IppImaging_Needs*/ virtual ~HRFRasterFileExtender();

    // Returns the file exlusive key
    _HDLLg /*IppImaging_Needs*/ virtual HFCExclusiveKey& GetKey() const;

    _HDLLg /*IppImaging_Needs*/ virtual HFCPtr<HRFRasterFile>& GetOriginalFile() const;
    _HDLLg /*IppImaging_Needs*/ virtual HFCPtr<HRFRasterFile>  GetExtendedFile() const;

    _HDLLg /*IppImaging_Needs*/ virtual bool IsCacheExtender() const;

    _HDLLg /*IppImaging_Needs*/ virtual void Save();

    _HDLLg /*IppImaging_Needs*/ virtual HFCPtr<HMDContext>  GetContext(uint32_t pi_PageIndex) const;

    _HDLLg /*IppImaging_Needs*/ virtual void                SetContext(uint32_t           pi_PageIndex,
                                               const HFCPtr<HMDContext>& pi_rpContext);

    _HDLLg /*IppImaging_Needs*/ virtual bool IsOriginalRasterDataStorage() const;

    _HDLLg /*IppImaging_Needs*/ virtual void CancelCreate();

protected:
    // Raster File
    HFCPtr<HRFRasterFile>   m_pOriginalFile;

    // Constructor
    _HDLLg /*IppImaging_Needs*/ HRFRasterFileExtender(const HFCPtr<HRFRasterFile>&  pi_rpOriginal);

private:
    // Disabled methods
    HRFRasterFileExtender(const HRFRasterFileExtender&);
    HRFRasterFileExtender& operator=(const HRFRasterFileExtender&);
    };

