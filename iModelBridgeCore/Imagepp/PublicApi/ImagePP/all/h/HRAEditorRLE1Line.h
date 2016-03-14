//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAEditorRLE1Line.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAEditorRLE1Line
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HGSMemoryRLESurfaceDescriptor;
class HCDPacketRLE;

class HRAEditorRLE1Line : public HRAGenEditor
    {
    HDECLARE_CLASS_ID(HRAEditorId_RLE1Line, HRAGenEditor)

public:

    // Primary methods
    HRAEditorRLE1Line(HGSMemoryRLESurfaceDescriptor& pi_rDescriptor);
    virtual        ~HRAEditorRLE1Line();

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

    virtual void    MergeRuns       (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_Width,
                                     size_t             pi_Height,
                                     const void*        pi_pRun,
                                     void*              pi_pTransaction = 0);

    virtual bool    HasData() const override;


protected:

    uint16_t*        ComputeAddress(HUINTX   pi_PosX,
                                   HUINTX   pi_PosY,
                                   uint16_t* po_pPixelsToSkipInFirstLen,
                                   uint16_t* po_pPixelsToSkipInSecondLen) const;

private:


    uint32_t            m_Width;
    uint32_t            m_Height;
    HUINTX              m_XPosInRaster;
    HUINTX              m_YPosInRaster;


    // State information for GetPixel, GetNextPixel methods
    mutable uint16_t*    m_pCurrentCount;
    mutable uint16_t m_RemainingCount;
    mutable HUINTX      m_CurrentLine;
    mutable uint16_t m_aData[2];


    bool               m_SLO4;

    HFCPtr<HCDPacketRLE> m_pPacketRLE;

    // temporary buffer
    HArrayAutoPtr<uint16_t>
    m_pTmpRun;
    HArrayAutoPtr<uint16_t>
    m_pWorkingRun;
    HArrayAutoPtr<uint16_t>
    m_pClearRun;

    bool               m_Edited;


    // private methods

    bool           IsPixelOn   (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY) const;

    void            GetRun      (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY,
                                 size_t         pi_PixelCount,
                                 uint16_t*       po_pBuffer) const;


    // disabled methods
    HRAEditorRLE1Line();
    HRAEditorRLE1Line(const HRAEditorRLE1Line& pi_rObj);
    HRAEditorRLE1Line&  operator=(const HRAEditorRLE1Line& pi_rObj);
    };

END_IMAGEPP_NAMESPACE