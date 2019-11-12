//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAEditorN1
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------

#pragma once

#include "HRAGenEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HGSMemorySurfaceDescriptor;


class HRAEditorN1 : public HRAGenEditor
    {
    HDECLARE_CLASS_ID(HRAEditorId_N1, HRAGenEditor)

public:

    // Primary methods
    HRAEditorN1(HGSMemorySurfaceDescriptor& pi_rDescriptor);
    virtual        ~HRAEditorN1();

    void*   GetRun      (HUINTX             pi_StartPosX,
                                 HUINTX             pi_StartPosY,
                                 size_t             pi_PixelCount,
                                 void*              pi_pTransaction = 0) const override;

    void    SetRun      (HUINTX             pi_StartPosX,
                                 HUINTX             pi_StartPosY,
                                 size_t             pi_PixelCount,
                                 const void*        pi_pRun,
                                 void*              pi_pTransaction = 0) override;


    // sequential pixel access

    // random pixel access
    void*   GetPixel    (HUINTX             pi_PosX,
                                 HUINTX             pi_PosY) const override;
    void    SetPixel    (HUINTX             pi_PosX,
                                 HUINTX             pi_PosY,
                                 const void*        pi_pValue) override;
    void    GetPixels   (const HUINTX*      pi_pPositionsX,
                                 const HUINTX*      pi_pPositionsY,
                                 size_t             pi_PixelCount,
                                 void*              po_pBuffer) const override;

    void    GetPixels   (HUINTX             pi_PosX,
                                 HUINTX             pi_PosY,
                                 HSINTX             pi_DeltaX,
                                 HSINTX             pi_DeltaY,
                                 size_t             pi_PixelCount,
                                 void*              po_pBuffer) const override;

    void*   GetNextPixel() const override;

    // clear
    void    Clear       (const void*        pi_pValue,
                                 void*              pi_pTransaction = 0) override;
    void    ClearRun    (HUINTX             pi_PosX,
                                 HUINTX             pi_PosY,
                                 size_t             pi_PixelCount,
                                 const void*        pi_pValue,
                                 void*              pi_pTransaction = 0) override;

    void    MergeRuns   (HUINTX             pi_StartPosX,
                                 HUINTX             pi_StartPosY,
                                 size_t             pi_Width,
                                 size_t             pi_Height,
                                 const void*        pi_pRun,
                                 void*              pi_pTransaction = 0) override;


    virtual bool    HasData() const override;

protected:

private:

    HFCPtr<HCDPacket>   m_pPacket;

    Byte                m_BitsPerPixel;
    Byte                m_PixelsPerByte;
    size_t              m_BytesPerLine;

    HArrayAutoPtr<Byte> m_pTmpRun;

    uint32_t           m_Width;
    uint32_t           m_Height;
    HUINTX              m_XPosInRaster;
    HUINTX              m_YPosInRaster;

    bool                m_SLO4;


    mutable size_t      m_PixelCount;

    mutable Byte        m_TmpValue;

    Byte                m_Mask;

    // optimization variables
    mutable Byte*       m_pRawData;
    mutable Byte        m_BitIndex;

    // private methods
    void            CopyBits(   Byte*         po_pDstBuffer,
                                Byte          pi_DstBitIndex,
                                const Byte*   pi_pSrcBuffer,
                                Byte          pi_SrcBitIndex,
                                size_t          pi_BitsToCopy) const;

    void            ComputeAddress(HUINTX     pi_PosX,
                                   HUINTX     pi_PosY,
                                   Byte**   po_ppRawData,
                                   Byte*    po_pBitIndex) const;

    void            GetRun      (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY,
                                 size_t         pi_PixelCount,
                                 Byte*        po_pBuffer) const;

    // disabled methods
    HRAEditorN1(const HRAEditorN1& pi_rObj);
    HRAEditorN1&    operator=(const HRAEditorN1& pi_rObj);
    bool             operator==(const HRAEditorN1& pi_rObj) const;
    bool             operator!=(const HRAEditorN1& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
