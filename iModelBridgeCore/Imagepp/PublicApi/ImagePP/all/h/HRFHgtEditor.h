//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFHgtEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFHgtEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFHgtFile;

class HRFHgtLineEditor : public HRFResolutionEditor
    {
    public:
        DEFINE_T_SUPER(HRFResolutionEditor)

            friend class HRFHgtFile;

        virtual ~HRFHgtLineEditor();

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
        HRFHgtLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                         uint32_t              pi_Page,
                         uint16_t       pi_Resolution,
                         HFCAccessMode         pi_AccessMode);
    private:

        HFCPtr<HRFHgtFile>    m_pRasterFile;

        uint32_t                    m_DataOffset;

        // 0 if bits per row not multiple of 8
        uint32_t                    m_ExactBytesPerRow;

        // Methods Disabled
        HRFHgtLineEditor(const HRFHgtLineEditor& pi_rObj);
        HRFHgtLineEditor& operator=(const HRFHgtLineEditor& pi_rObj);
    };

class HRFHgtImageEditor : public HRFResolutionEditor
    {
    public:
        DEFINE_T_SUPER(HRFResolutionEditor)

            friend class HRFHgtFile;

        virtual ~HRFHgtImageEditor();

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
        HRFHgtImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                          uint32_t              pi_Page,
                          uint16_t       pi_Resolution,
                          HFCAccessMode         pi_AccessMode);
    private:

        HFCPtr<HRFHgtFile>    m_pRasterFile;


        // Methods Disabled
        HRFHgtImageEditor(const HRFHgtImageEditor& pi_rObj);
        HRFHgtImageEditor& operator=(const HRFHgtImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


