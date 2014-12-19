//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAStoredRasterEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAStoredRasterEditor
//-----------------------------------------------------------------------------

#pragma once

#include "HRARasterEditor.h"

class HRARaster;
class HVEShape;
class HRAStoredRaster;

class HRAStoredRasterEditor : public HRARasterEditor
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(1085, HRARasterEditor)
    /*
            // Type used in type Error to specify error type

            typedef struct
            {
                enum
                {
                    INVALID_ROLE,
                    INVALID_INDEX,
                    INVALID_POS,
                    NO_ALPHA
                } m_ErrorType;
            } Error;
    */
    // Primary methods

    HRAStoredRasterEditor   (const HFCPtr<HRAStoredRaster>& pi_pRaster,
                             const HFCAccessMode            pi_Mode);

    virtual         ~HRAStoredRasterEditor  ();
    /*
            // Editor control

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

            // Generic Pixel data access

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


            // Implementation methods (optimizing CopyFrom)



            // Debug function
            virtual void    PrintState(ostream& po_rOutput) const;
    */
protected:

private:

    // Disable methods
    HRAStoredRasterEditor   (const HRAStoredRasterEditor& pi_rObj);
    HRAStoredRasterEditor& operator=(const HRAStoredRasterEditor& pi_rObj);

    };

