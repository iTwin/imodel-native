//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSEditor.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSEditor
//-----------------------------------------------------------------------------
// General class for Stretchers.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicTool.h"

class HCDPacket;

class HGSEditor : public HGSGraphicTool
    {
    HDECLARE_CLASS_ID(1730, HGSGraphicTool)

public:

    // Primary methods
    HGSEditor();
    virtual        ~HGSEditor();


    // type
    virtual HCLASS_ID
    GetType() const;

    // sequential pixel access
    void*           GetFirstPixel   (HUINTX*        po_pPosX,
                                     HUINTX*        po_pPosY) const;

    void*           GetNextPixel    (HUINTX*        po_pPosX,
                                     HUINTX*        po_pPosY) const;

    // sequential run access
    void*           GetFirstRun     (HUINTX*        po_pStartPosX,
                                     HUINTX*        po_pStartPosY,
                                     size_t*        po_pPixelCount,
                                     void*          pi_pTransaction = 0) const;

    void*           GetNextRun      (HUINTX*        po_pStartPosX,
                                     HUINTX*        po_pStartPosY,
                                     size_t*        po_pPixelCount,
                                     void*          pi_pTransaction = 0) const;

    void*           GetRun          (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY,
                                     size_t         pi_PixelCount,
                                     void*          pi_pTransaction = 0) const;

    void            SetRun          (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY,
                                     size_t         pi_PixelCount,
                                     const void*    pi_pRun,
                                     void*          pi_pTransaction = 0);

    // random pixel access
    void*           GetPixel        (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY) const;

    void            SetPixel        (HUINTX         pi_PosX,
                                     HUINTX         pi_PosY,
                                     const void*    pi_pValue);

    void            GetPixels       (const HUINTX*  pi_pPositionsX,
                                     const HUINTX*  pi_pPositionsY,
                                     size_t         pi_PixelCount,
                                     void*          po_pBuffer) const;

    void            GetPixels       (HUINTX         pi_StartPositionX,
                                     HUINTX         pi_StartPositionY,
                                     HSINTX         pi_DeltaX,
                                     HSINTX         pi_DeltaY,
                                     size_t         pi_PixelCount,
                                     void*          po_pBuffer) const;

    // clear
    void            Clear           (const void*    pi_pValue,
                                     void*          pi_pTransaction = 0);
    void            ClearRun        (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_PixelCount,
                                     const void*    pi_pValue,
                                     void*          pi_pTransaction = 0);

    void            MergeRuns       (HUINTX         pi_StartPosX,
                                     HUINTX         pi_StartPosY,
                                     size_t         pi_Width,
                                     size_t         pi_Height,
                                     const void*    pi_pRun,
                                     void*          pi_pTransaction = 0);

protected:

private:

    // disabled methods
    HGSEditor(const HGSEditor& pi_rObj);
    HGSEditor&      operator=(const HGSEditor& pi_rObj);
    };

#include "HGSEditor.hpp"



