//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAEditor
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSEditorImplementation.h"
#include "HGSMacros.h"

#include "HRAGenEditor.h"
#include "HRPFilter.h"


class HRATransaction;
class HCDPacket;

HGS_DECLARE_GRAPHICTOOL_DLL(_HDLLg, HRAEditor)

class HRAEditor : public HGSEditorImplementation
    {
    HDECLARE_CLASS_ID(1743, HGSEditorImplementation)

    HGS_DECLARE_GRAPHICCAPABILITIES()

public:

    // Primary methods
    HRAEditor(const HGSGraphicToolAttributes*   pi_pAttributes,
              HGSSurfaceImplementation*         pi_pSurfaceImplementation);
    
    virtual        ~HRAEditor();


    // long version
    // sequential run access
    virtual void*   GetFirstRun     (HUINTX*            po_pStartPosX,
                                     HUINTX*            po_pStartPosY,
                                     size_t*            po_pPixelCount,
                                     void*              pi_pTransaction = 0) const;

    virtual void*   GetNextRun      (HUINTX*            po_pStartPosX,
                                     HUINTX*            po_pStartPosY,
                                     size_t*            po_pPixelCount,
                                     void*              pi_pTransaction = 0) const;

    virtual void*   GetRun          (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_PixelCount,
                                     void*              pi_pTransaction = 0) const;

    virtual void    SetRun          (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_PixelCount,
                                     const void*        pi_pRun,
                                     void*              pi_pTransaction = 0);

    // sequential pixel access
    virtual void*   GetFirstPixel   (HUINTX*            po_pPosX,
                                     HUINTX*            po_pPosY) const;

    virtual void*   GetNextPixel    (HUINTX*            po_pPosX,
                                     HUINTX*            po_pPosY) const;

    // random pixel access
    virtual void*   GetPixel        (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY) const;

    virtual void    GetPixels       (const HUINTX*      pi_pPositionsX,
                                     const HUINTX*      pi_pPositionsY,
                                     size_t             pi_PixelCount,
                                     void*              po_pBuffer) const;

    virtual void    GetPixels       (HUINTX             pi_PositionX,
                                     HUINTX             pi_PositionY,
                                     HSINTX             pi_DeltaX,
                                     HSINTX             pi_DeltaY,
                                     size_t             pi_PixelCount,
                                     void*              po_pBuffer) const;

    virtual void    SetPixel        (HUINTX             pi_PosX,
                                     HUINTX             pi_PosY,
                                     const void*        pi_pValue);


    // clear
    virtual void    Clear           (const void*        pi_pValue,
                                     void*              pi_pTransaction = 0);
    virtual void    ClearRun        (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_PixelCount,
                                     const void*        pi_pValue,
                                     void*              pi_pTransaction = 0);


    virtual void    MergeRuns       (HUINTX             pi_StartPosX,
                                     HUINTX             pi_StartPosY,
                                     size_t             pi_Width,
                                     size_t             pi_Height,
                                     const void*        pi_pRun,
                                     void*              pi_pTransaction = 0);


    //:> Public because HRABlitter needs to access the scanlines.
    //:> Should not be used otherwise.

    const HGFScanLines*
    GetScanlines() const;

protected:

private:


    uint32_t        m_Width;
    uint32_t        m_Height;

    HAutoPtr<HRAGenEditor>
    m_pEditor;

    mutable HAutoPtr<HGFScanLines>  m_pScanLinesSmartPtr;
    mutable HGFScanLines*           m_pScanLines;

    mutable uint32_t m_PosY;
    mutable size_t  m_RunPixelCount;
    mutable uint32_t m_PosX;

    // disabled methods
    HRAEditor(const HRAEditor& pi_rObj);
    HRAEditor&      operator=(const HRAEditor& pi_rObj);
    };

