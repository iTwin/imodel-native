//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAEditorRLE1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAEditorRLE1
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HGSMemorySurfaceDescriptor;

class HRAEditorRLE1 : public HRAGenEditor
    {
    HDECLARE_CLASS_ID(HRAEditorId_RLE1, HRAGenEditor)

public:

    // Primary methods
    HRAEditorRLE1(HGSMemorySurfaceDescriptor& pi_rDescriptor);
    virtual        ~HRAEditorRLE1();

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

    unsigned short*        ComputeAddress(HUINTX   pi_PosX,
                                   HUINTX   pi_PosY,
                                   unsigned short* po_pPixelsToSkipInFirstLen,
                                   unsigned short* po_pPixelsToSkipInSecondLen) const;

private:


    uint32_t            m_Width;
    uint32_t            m_Height;

    // State information for GetPixel, GetNextPixel methods
    mutable unsigned short*    m_pCurrentCount;
    mutable unsigned short m_RemainingCount;
    mutable HUINTX      m_CurrentLine;
    mutable unsigned short m_aData[2];


    bool               m_SLO4;

    HFCPtr<HCDPacket>   m_pPacket;
    uint32_t*             m_pLineIndexes;

    // temporary buffer
    HArrayAutoPtr<unsigned short>
    m_pTmpRun;
    HArrayAutoPtr<unsigned short>
    m_pWorkingRun;

    bool               m_Edited;


    // private methods

    bool           IsPixelOn   (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY) const;
    unsigned short*        PrepareToAppendDataInBuffer
    (size_t pi_DataSize);
    void            GetRun      (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY,
                                 size_t         pi_PixelCount,
                                 unsigned short*       po_pBuffer) const;

    // disabled methods
    HRAEditorRLE1();
    HRAEditorRLE1(const HRAEditorRLE1& pi_rObj);
    HRAEditorRLE1&  operator=(const HRAEditorRLE1& pi_rObj);
    };


END_IMAGEPP_NAMESPACE