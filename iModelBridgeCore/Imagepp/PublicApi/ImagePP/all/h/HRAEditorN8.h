//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAEditorN8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAEditorN8
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------

#pragma once

#include "HRAGenEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRATransaction;
class HGSMemorySurfaceDescriptor;
class HCDPacket;

class HRAEditorN8 : public HRAGenEditor
    {
    HDECLARE_CLASS_ID(HRAEditorId_N8, HRAGenEditor)

public:

    // Primary methods
    HRAEditorN8(HGSMemorySurfaceDescriptor& pi_rDescriptor);
    virtual        ~HRAEditorN8();

    // sequential pixel access
    virtual void*   GetRun          (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_PixelCount,
                                     void*              pi_pTransaction = 0) const;

    virtual void    SetRun          (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_PixelCount,
                                     const void*        pi_pRun,
                                     void*              pi_pTransaction = 0);

    // random pixel access
    virtual void*   GetPixel        (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY) const;

    virtual void    SetPixel        (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY,
                                     const void*        pi_pValue);

    virtual void    GetPixels       (const HUINTX*      pi_pPositionsX,
                                     const HUINTX*      pi_pPositionsY,
                                     size_t             pi_PixelCount,
                                     void*              po_pBuffer) const;

    virtual void    GetPixels       (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY,
                                     HSINTX             pi_DeltaX,
                                     HSINTX             pi_DeltaY,
                                     size_t             pi_PixelCount,
                                     void*              po_pBuffer) const;

    virtual void*   GetNextPixel    () const;

    // clear
    virtual void    Clear           (const void*        pi_pValue,
                                     void*              pi_pTransaction = 0);
    virtual void    ClearRun        (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY,
                                     size_t             pi_PixelCount,
                                     const void*        pi_pValue,
                                     void*              pi_pTransaction = 0);



    void            MergeRuns       (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_Width,
                                     size_t             pi_Height,
                                     const void*        pi_pRun,
                                     void*              pi_pTransaction = 0);

    virtual bool    HasData() const override;

protected:

    Byte*         ComputeAddress  (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY) const;

private:

    size_t              m_BytesPerPixel;
    size_t              m_BytesPerLine;

    HUINTX              m_XPosInRaster;
    HUINTX              m_YPosInRaster;

    uint32_t            m_Width;
    uint32_t            m_Height;

    bool               m_SLO4;

    HFCPtr<HCDPacket>   m_pPacket;

    mutable Byte*     m_pRawData;
    mutable HUINTX      m_RawDataPosX;
    mutable HUINTX      m_RawDataPosY;

    // disabled methods
    HRAEditorN8(const HRAEditorN8& pi_rObj);
    HRAEditorN8&    operator=(const HRAEditorN8& pi_rObj);
    };

END_IMAGEPP_NAMESPACE