//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSEditorImplementation.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSEditorImplementation
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicToolImplementation.h"

class HGSGraphicToolAttributes;
class HGSSurfaceImplementation;
class HCDPacket;

class HNOVTABLEINIT HGSEditorImplementation : public HGSGraphicToolImplementation
    {
    HDECLARE_CLASS_ID(1729, HGSGraphicToolImplementation)

public:

    // Primary methods
    HGSEditorImplementation(const HGSGraphicToolAttributes* pi_pAttributes,
                            HGSSurfaceImplementation*       pi_pSurfaceImplementation);

    virtual         ~HGSEditorImplementation();

    // sequential pixel access
    virtual void*   GetFirstPixel   (HUINTX*        po_pPosX,
                                     HUINTX*        po_pPosY) const = 0;

    virtual void*   GetNextPixel    (HUINTX*        po_pPosX,
                                     HUINTX*        po_pPosY) const = 0;

    // sequential run access
    virtual void*   GetFirstRun     (HUINTX*        po_pStartPosX,
                                     HUINTX*        po_pStartPosY,
                                     size_t*        po_pPixelCount,
                                     void*          pi_pTransaction = 0) const = 0;

    virtual void*   GetNextRun      (HUINTX*        po_pStartPosX,
                                     HUINTX*        po_pStartPosY,
                                     size_t*        po_pPixelCount,
                                     void*          pi_pTransaction = 0) const = 0;

    virtual void*   GetRun          (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_PixelCount,
                                     void*          pi_pTransaction = 0) const = 0;

    virtual void    SetRun          (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_PixelCount,
                                     const void*    pi_pRun,
                                     void*          pi_pTransaction = 0) = 0;

    // random pixel access
    virtual void*   GetPixel        (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY) const = 0;

    virtual void    SetPixel        (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY,
                                     const void*    pi_pValue) = 0;

    virtual void    GetPixels       (const HUINTX*  pi_pPositionsX,
                                     const HUINTX*  pi_pPositionsY,
                                     size_t         pi_PixelCount,
                                     void*          po_pBuffer) const = 0;

    virtual void    GetPixels       (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     HSINTX         pi_DeltaX,
                                     HSINTX         pi_DeltaY,
                                     size_t         pi_PixelCount,
                                     void*          po_pBuffer) const = 0;

    // clear method
    virtual void    Clear           (const void*    pi_pValue,
                                     void*          pi_pTransaction = 0) = 0;
    virtual void    ClearRun        (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_PixelCount,
                                     const void*    pi_pValue,
                                     void*          pi_pTransaction = 0) = 0;

    virtual void    MergeRuns       (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_Width,
                                     size_t         pi_Height,
                                     const void*    pi_pRun,
                                     void*          pi_pTransaction = 0) = 0;

protected:


private:

    // private members
    HArrayAutoPtr<int32_t>
    m_pLongPositionsX;
    HArrayAutoPtr<int32_t>
    m_pLongPositionsY;
    uint32_t        m_LongNumberOfPixels;

    HArrayAutoPtr<double>
    m_pDoublePositionsX;
    HArrayAutoPtr<double>
    m_pDoublePositionsY;
    uint32_t        m_DoubleNumberOfPixels;

    // disabled methods
    HGSEditorImplementation(const HGSEditorImplementation& pi_rObj);
    HGSEditorImplementation&
    operator=(const HGSEditorImplementation& pi_rObj);
    };
