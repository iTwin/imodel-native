//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAGenEditor
//-----------------------------------------------------------------------------
// General class for Editors.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HRATransaction;
class HCDPacket;

class HNOVTABLEINIT HRAGenEditor
    {
    HDECLARE_BASECLASS_ID(HRAGenEditorId_Base)

public:

    // Primary methods
    HRAGenEditor();

    virtual        ~HRAGenEditor();

    virtual void*   GetRun     (HUINTX          pi_StartPosX,
                                HUINTX          pi_StartPosY,
                                size_t          pi_PixelCount,
                                void*           pi_pTransaction = 0) const = 0;


    virtual void    SetRun     (HUINTX          pi_StartPosX,
                                HUINTX          pi_StartPosY,
                                size_t          pi_PixelCount,
                                const void*     pi_pRun,
                                void*           pi_pTransaction = 0) {
        HASSERT(0);
        };

    // random pixel access
    virtual void*   GetPixel   (HUINTX          pi_PosX,
                                HUINTX          pi_PosY) const = 0;

    virtual void    GetPixels  (const HUINTX*   pi_pPositionsX,
                                const HUINTX*   pi_pPositionsY,
                                size_t          pi_PixelCount,
                                void*           po_pBuffer) const = 0;

    virtual void    GetPixels  (HUINTX          pi_PositionX,
                                HUINTX          pi_PositionY,
                                HSINTX          pi_DeltaX,
                                HSINTX          pi_DeltaY,
                                size_t          pi_PixelCount,
                                void*           po_pBuffer) const = 0;


    virtual void    SetPixel   (HUINTX          pi_PosX,
                                HUINTX          pi_PosY,
                                const void*     pi_pValue) = 0;

    // get pixel
    virtual void*   GetNextPixel() const = 0;

    // clear
    virtual void    Clear       (const void*    pi_pValue,
                                 void*          pi_pTransaction = 0) = 0;
    virtual void    ClearRun    (HUINTX         pi_PosX,
                                 HUINTX         pi_PosY,
                                 size_t         pi_PixelCount,
                                 const void*    pi_pValue,
                                 void*          pi_pTransaction = 0) = 0;

    virtual void    MergeRuns   (HUINTX         pi_StartPosX,
                                 HUINTX         pi_StartPosY,
                                 size_t         pi_Width,
                                 size_t         pi_Height,
                                 const void*    pi_pRun,
                                 void*          pi_pTransaction = 0) = 0;

    virtual bool    HasData() const = 0;


protected:


private:


    // disabled methods
    HRAGenEditor(const HRAGenEditor& pi_rObj);
    HRAGenEditor&    operator=(const HRAGenEditor& pi_rObj);
    };

END_IMAGEPP_NAMESPACE