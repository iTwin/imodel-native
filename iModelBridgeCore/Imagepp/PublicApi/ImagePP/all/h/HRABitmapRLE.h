//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapRLE.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRABitmapBase.h"

class HRADrawOptions;
class HRABitmapEditor;
class HCDPacketRLE;
class HGSSurfaceDescriptor;
class HRAClearOptions;

// ----------------------------------------------------------------------------
//  HRABitmapRLE
// ----------------------------------------------------------------------------
class HRABitmapRLE : public HRABitmapBase
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1744)

public:

    // Primary methods
    HRABitmapRLE(bool                          pi_Resizable = false);

    _HDLLg              HRABitmapRLE(uint32_t                       pi_WidthPixels,
                                     uint32_t                       pi_HeightPixels,
                                     const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                                     const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                                     const HFCPtr<HRPPixelType>&    pi_rpPixel,
                                     SLO                            pi_SLO = UPPER_LEFT_HORIZONTAL,
                                     bool                          pi_Resizable = false);

    _HDLLg              HRABitmapRLE   (const HRABitmapRLE& pi_rObj);

    _HDLLg virtual      ~HRABitmapRLE  ();

    HRABitmapRLE&      operator=(const HRABitmapRLE& pi_rBitmap);

    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    // Inherited from HRARaster
    virtual HRARasterEditor*
    CreateEditor   (HFCAccessMode pi_Mode) override;

    virtual HRARasterEditor*
    CreateEditor   (const HVEShape& pi_rShape,
                    HFCAccessMode   pi_Mode) override;

    virtual HRARasterEditor*
    CreateEditor   (const HGFScanLines& pi_rShape,
                    HFCAccessMode       pi_Mode) override;

    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode) override;

    virtual unsigned short GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms) override;

    virtual void        ComputeHistogram(HRAHistogramOptions* pio_pHistogramOptions,
                                         bool                pi_ForceRecompute = false) override;


    virtual void Clear() override;
    virtual void Clear(const HRAClearOptions& pi_rOptions) override;

    // Special status...
    //
    // Must call the parent
    virtual void    MakeEmpty() override;
    virtual bool   IsEmpty() const override;

    virtual size_t  GetAdditionalSize() const override;

    virtual void    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels) override;

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor>
    GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType = 0,
                         HFCPtr<HRPPixelType>* po_ppOutputPixelType = 0) const override;
    virtual void    Updated(const HVEShape* pi_pModifiedContent = 0) override;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions) override;

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const override;

    // packets and codecs
    virtual const HFCPtr<HCDCodec>&
    GetCodec() const override;

    _HDLLg const HFCPtr<HCDPacketRLE>& GetPacket() const;

    bool           NotifyPaletteChanged (const HMGMessage& pi_rMessage);

protected:

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor>
    CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                            HFCPtr<HRPPixelType>* po_ppOutputPixelType) const override;


private:
    mutable HFCPtr<HGSSurfaceDescriptor> m_pSurfaceDescriptor;

    HFCPtr<HCDPacketRLE> m_pPacketRLE;

    // Methods
    void            DeepCopy(const HRABitmapRLE& pi_rBitmap);
    void            DeepDelete();
    void            StretchWithHGS(HGFMappedSurface*                pio_pSurface,
                                   const HRADrawOptions*            pi_pOptions,
                                   const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const;

    void            WarpWithHGS(HGFMappedSurface*                   pio_pSurface,
                                const HRADrawOptions*               pi_pOptions,
                                const HFCPtr<HGF2DTransfoModel>&    pi_rpTransfoModel) const;

    void            ComputeHistogramRLE      (HRAHistogramOptions* pio_pOptions);

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg)
    };

