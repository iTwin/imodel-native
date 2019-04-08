//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCacheSequentialBlockEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFCacheSequentialBlockEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
//#include "HFCBuffer.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFCacheSequentialBlockEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFRasterFileCache;

    // Constructor
    HRFCacheSequentialBlockEditor(
        HFCPtr<HRFRasterFile> pi_rpRasterFile,
        uint32_t             pi_Page,
        uint16_t        pi_Resolution,
        HFCAccessMode         pi_AccessMode,
        HRFResolutionEditor*  pi_pSrcResolutionEditor,
        HRFResolutionEditor*  pi_pCacheResolutionEditor);

    virtual         ~HRFCacheSequentialBlockEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

protected:
    HAutoPtr<HRFResolutionEditor>   m_pSrcResolutionEditor;
    HAutoPtr<HRFResolutionEditor>   m_pCacheResolutionEditor;

private:
    // Methods Disabled
    HRFCacheSequentialBlockEditor(const HRFCacheSequentialBlockEditor& pi_rObj);
    HRFCacheSequentialBlockEditor& operator=(const HRFCacheSequentialBlockEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
