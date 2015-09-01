//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAEditor
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HRATransaction;
class HCDPacket;
class HRAGenEditor;
class HRASurface;
class HGFScanLines;

class HRAEditor
    {
public:

    // Primary methods
    HRAEditor(HRASurface& pi_surface, bool gridMode = false); 
    ~HRAEditor();


    // long version
    // sequential run access
    void*   GetFirstRun     (HUINTX*            po_pStartPosX,
                             HUINTX*            po_pStartPosY,
                             size_t*            po_pPixelCount,
                             void*              pi_pTransaction = 0) const;

    void*   GetNextRun      (HUINTX*            po_pStartPosX,
                             HUINTX*            po_pStartPosY,
                             size_t*            po_pPixelCount,
                             void*              pi_pTransaction = 0) const;

    void*   GetRun          (HUINTX             pi_StartPosX,
                             HUINTX             pi_StartPosY,
                             size_t             pi_PixelCount,
                             void*              pi_pTransaction = 0) const;

    IMAGEPP_EXPORT void    SetRun(HUINTX             pi_StartPosX,
                                 HUINTX             pi_StartPosY,
                                 size_t             pi_PixelCount,
                                 const void*        pi_pRun,
                                 void*              pi_pTransaction = 0);

    // sequential pixel access
    void*   GetFirstPixel   (HUINTX*            po_pPosX,
                             HUINTX*            po_pPosY) const;

    void*   GetNextPixel    (HUINTX*            po_pPosX,
                             HUINTX*            po_pPosY) const;

    // random pixel access
    IMAGEPP_EXPORT void*   GetPixel(HUINTX             pi_PosX,
                                   HUINTX             pi_PosY) const;

    void    GetPixels       (const HUINTX*      pi_pPositionsX,
                             const HUINTX*      pi_pPositionsY,
                             size_t             pi_PixelCount,
                             void*              po_pBuffer) const;

    void    GetPixels       (HUINTX             pi_PositionX,
                             HUINTX             pi_PositionY,
                             HSINTX             pi_DeltaX,
                             HSINTX             pi_DeltaY,
                             size_t             pi_PixelCount,
                             void*              po_pBuffer) const;

    void    SetPixel        (HUINTX             pi_PosX,
                             HUINTX             pi_PosY,
                             const void*        pi_pValue);


    // clear
    void    Clear           (const void*        pi_pValue,
                             void*              pi_pTransaction = 0);
    IMAGEPP_EXPORT void    ClearRun(HUINTX             pi_StartPosX,
                                   HUINTX             pi_StartPosY,
                                   size_t             pi_PixelCount,
                                   const void*        pi_pValue,
                                   void*              pi_pTransaction = 0);


    void    MergeRuns       (HUINTX             pi_StartPosX,
                             HUINTX             pi_StartPosY,
                             size_t             pi_Width,
                             size_t             pi_Height,
                             const void*        pi_pRun,
                             void*              pi_pTransaction = 0);


    //:> Public because HRABlitter needs to access the scanlines.
    //:> Should not be used otherwise.

    const HGFScanLines* GetScanlines() const;

    HRASurface& GetSurface() const;

protected:

private:

    HRASurface& m_surface;
   
    uint32_t        m_Width;
    uint32_t        m_Height;

    HAutoPtr<HRAGenEditor> m_pEditor;

    mutable HAutoPtr<HGFScanLines>  m_pScanLinesSmartPtr;
    mutable HGFScanLines*           m_pScanLines;

    mutable uint32_t m_PosY;
    mutable size_t  m_RunPixelCount;
    mutable uint32_t m_PosX;

    // disabled methods
    HRAEditor(const HRAEditor& pi_rObj);
    HRAEditor&      operator=(const HRAEditor& pi_rObj);
    };


END_IMAGEPP_NAMESPACE