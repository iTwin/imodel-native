//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPDFEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFPDFFile;
class PDFEditorWrapper;

class HRFPDFEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPDFFile;

    virtual         ~HRFPDFEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

    protected:
        // See the parent for Pointer to the raster file, to the resolution descriptor
        // and to the capabilities

        // Constructor
        HRFPDFEditor    (HFCPtr<HRFRasterFile>      pi_rpRasterFile,
            uint32_t                               pi_Page,
            double                                  pi_Resolution,
            HFCAccessMode                           pi_AccessMode);


        // this method is called by HRFPDFFile to support LookAhead
        HSTATUS         ReadBlockPDF (uint32_t     pi_MinX,
                                      uint32_t     pi_MinY,
                                      uint32_t     pi_MaxX,
                                      uint32_t     pi_MaxY,
                                      Byte*         po_pData);

    private:

        void            CreateEditorWrapper();

        HAutoPtr<PDFEditorWrapper>       m_pPDFEditorWrapper;

        Byte*                            m_pBuffer;
        uint32_t                        m_BufferSize;
        HFCPtr<HGFTileIDDescriptor>      m_pTileIDDescriptor;

        uint32_t                        m_BlockWidth;
        uint32_t                        m_BlockHeight;
        uint32_t                        m_BytesPerBlockWidth;
        uint32_t                        m_BlockSizeInBytes;
        uint32_t                        m_WrapperCreatorThreadId;

        // optimization


        // Methods Disabled
        HRFPDFEditor(const HRFPDFEditor& pi_rObj);
        HRFPDFEditor& operator=(const HRFPDFEditor& pi_rObj);
    };


class HNOVTABLEINIT PDFEditorWrapper
    {
    public:
        virtual ~PDFEditorWrapper() {};
        virtual void  GetResolutionSize(uint32_t* po_pResWidth,
            uint32_t* po_pResHeight) const = 0;

        virtual bool ReadBlock(uint64_t                  pi_PosX,
                                 uint64_t                  pi_PosY,
                                 const HFCPtr<HMDContext>& pi_rpContext,
                                 Byte*                     po_pData) = 0;

        virtual bool ReadBlock(uint64_t                  pi_MinX,
                                 uint64_t                  pi_MinY,
                                 uint64_t                  pi_MaxX,
                                 uint64_t                  pi_MaxY,
                                 const HFCPtr<HMDContext>& pi_rpContext,
                                 Byte*                     po_pData) = 0;
    };
END_IMAGEPP_NAMESPACE

