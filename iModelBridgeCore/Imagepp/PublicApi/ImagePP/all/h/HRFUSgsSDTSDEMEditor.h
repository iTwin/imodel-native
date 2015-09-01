//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsSDTSDEMEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFGdalSupportedFileEditor.h"

#define USGS_SDTS_BACKGROUND_VALUE -32766

BEGIN_IMAGEPP_NAMESPACE

class HRFUSgsSDTSDEMEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFUSgsSDTSDEMFile;

    virtual ~HRFUSgsSDTSDEMEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFUSgsSDTSDEMEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                         uint32_t              pi_Page,
                         unsigned short       pi_Resolution,
                         HFCAccessMode         pi_AccessMode);


private:
    template<class T>
    void ReplacePixelsWithNoDataValue(Byte* pio_pData, T pi_PixelValue, double pi_NoDataValue) const;

    template<class T, class Pred>
    void ReplacePixelsWithNoDataValueIf(Byte* pio_pData, Pred pi_Predicate, double pi_NoDataValue) const;

    // Methods Disabled
    HRFUSgsSDTSDEMEditor(const HRFUSgsSDTSDEMEditor& pi_rObj);
    HRFUSgsSDTSDEMEditor& operator=(const HRFUSgsSDTSDEMEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
