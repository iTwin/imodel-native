//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRABitmapEditor
//-----------------------------------------------------------------------------

#pragma once

#include "HRARasterEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HVEShape;
class HRABitmapBase;
class HRASurface;
class HRAEditor;
class HGSSurfaceDescriptor;
class HGFScanLines;

class HRABitmapEditor : public HRARasterEditor
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRABitmapId_Editor, HRARasterEditor)

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


    // get surface editor
    IMAGEPP_EXPORT HRAEditor* GetSurfaceEditor();

protected:

private:

    std::unique_ptr<HRAEditor>    m_pEditor;
    std::unique_ptr<HRASurface>   m_pSurface;

    // private methods
    void            InitObject( const HFCPtr<HGSSurfaceDescriptor>& pi_rpSurDescriptor,
                                const HVEShape*          pi_pShape = 0,
                                const HGFScanLines*      pi_pScanlines = 0);

    // Disable methods
    HRABitmapEditor   (const HRABitmapEditor& pi_rObj);
    HRABitmapEditor& operator=(const HRABitmapEditor& pi_rObj);

    };

END_IMAGEPP_NAMESPACE
