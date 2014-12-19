//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRABitmapEditor
//-----------------------------------------------------------------------------

#pragma once

#include "HRAStoredRasterEditor.h"
#include "HGSEditor.h"

class HRARaster;
class HVEShape;
class HRABitmapBase;

class HRABitmapEditor : public HRAStoredRasterEditor
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(1762, HRAStoredRasterEditor)

    // Type used in type Error to specify error type


    // Primary methods

    HRABitmapEditor   (const HFCPtr<HRABitmapBase>& pi_pRaster,
                       const HFCAccessMode      pi_Mode);

    HRABitmapEditor   (const HFCPtr<HRABitmapBase>& pi_pRaster,
                       const HVEShape&          pi_rShape,
                       const HFCAccessMode      pi_Mode);

    HRABitmapEditor   (const HFCPtr<HRABitmapBase>& pi_pRaster,
                       const HGFScanLines&      pi_rScanLines,
                       const HFCAccessMode      pi_Mode);


    virtual         ~HRABitmapEditor  ();

    // Editor control
    /*
            virtual const UInt32
                            GetPositionX(void) const = 0;
            virtual const UInt32
                            GetPositionY(void) const = 0;
            virtual bool   Goto(UInt32 pi_PosX, UInt32 pi_PosY) = 0;
            virtual bool   GotoNextX() = 0;
            virtual bool   GotoNextY() = 0;
            virtual bool   GotoPreviousX() = 0;
            virtual bool   GotoPreviousY() = 0;

            virtual bool   GotoFirstOfFirstRow() = 0;
            virtual bool   GotoFirstOfNextRow() = 0;
            virtual bool   GotoNextOfCurrentRow() = 0;

            virtual bool   GotoNextRunOfCurrentRow() = 0;
            virtual UInt32  CountPixelsToEndOfRun() = 0;
    */
    // Generic Pixel data access
    /*
            virtual const void*
                            GetCompositeValueAt(UInt32 pi_PosX, UInt32 pi_PosY) = 0;
            virtual size_t  GetIndexValueAt    (UInt32 pi_PosX, UInt32 pi_PosY,
                                                bool* po_pValidLocation) = 0;
            virtual const void*
                            GetRawDataAt       (UInt32 po_PosX, UInt32 po_PosY) = 0;
            virtual bool   SetCompositeValueAt(const void *pi_pRawData,
                                                UInt32 pi_PosX,
                                                UInt32 pi_PosY) = 0;
            virtual bool   SetIndexValueAt    (size_t pi_NewIndex,
                                                UInt32 pi_PosX,
                                                UInt32 pi_PosY) = 0;
            virtual bool   SetRawDataAt       (const void *pi_pRawData,
                                                UInt32 pi_PosX,
                                                UInt32 pi_PosY) = 0;

    */

    // get surface editor
    _HDLLg HFCPtr<HGSEditor>
    GetSurfaceEditor();


    // Implementation methods (optimizing CopyFrom)


protected:

private:

    HFCPtr<HGSEditor>
    m_pEditor;

    // private methods
    void            InitObject( const HFCPtr<HGSSurfaceDescriptor>& pi_rpSurDescriptor,
                                const HVEShape*          pi_pShape = 0,
                                const HGFScanLines*      pi_pScanlines = 0);

    // Disable methods
    HRABitmapEditor   (const HRABitmapEditor& pi_rObj);
    HRABitmapEditor& operator=(const HRABitmapEditor& pi_rObj);

    };
