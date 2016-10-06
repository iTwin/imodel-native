//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSRTMEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFSRTMEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFSRTMFile;

class HRFSRTMLineEditor : public HRFResolutionEditor
    {
    public:
        DEFINE_T_SUPER(HRFResolutionEditor)

            friend class HRFSRTMFile;

        virtual ~HRFSRTMLineEditor();

        // Edition by block
        virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                                  uint64_t    pi_PosBlockY,
                                  Byte*       po_pData) override;

        virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                                  uint64_t                pi_PosBlockY,
                                  HFCPtr<HCDPacket>&      po_rpPacket)
            {
            return T_Super::ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
            }


    protected:
        // See the parent for Pointer to the raster file, to the resolution descriptor
        // and to the capabilities

        // Constructor
        HRFSRTMLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                         uint32_t               pi_Page,
                         uint16_t               pi_Resolution,
                         HFCAccessMode          pi_AccessMode);
    private:

        HFCPtr<HRFSRTMFile>    m_pRasterFile;
        uint32_t               m_ExactBytesPerRow;

        // Methods Disabled
        HRFSRTMLineEditor(const HRFSRTMLineEditor& pi_rObj);
        HRFSRTMLineEditor& operator=(const HRFSRTMLineEditor& pi_rObj);
    };

class HRFSRTMImageEditor : public HRFResolutionEditor
    {
    public:
        DEFINE_T_SUPER(HRFResolutionEditor)

            friend class HRFSRTMFile;

        virtual ~HRFSRTMImageEditor();

        // Edition by block
        virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                                  uint64_t    pi_PosBlockY,
                                  Byte*       po_pData);

        virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                                  uint64_t                pi_PosBlockY,
                                  HFCPtr<HCDPacket>&      po_rpPacket)
            {
            return T_Super::ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
            }

    protected:
        // See the parent for Pointer to the raster file, to the resolution descriptor
        // and to the capabilities

        // Constructor
        HRFSRTMImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                          uint32_t              pi_Page,
                          uint16_t              pi_Resolution,
                          HFCAccessMode         pi_AccessMode);
    private:

        HFCPtr<HRFSRTMFile>    m_pRasterFile;


        // Methods Disabled
        HRFSRTMImageEditor(const HRFSRTMImageEditor& pi_rObj);
        HRFSRTMImageEditor& operator=(const HRFSRTMImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


