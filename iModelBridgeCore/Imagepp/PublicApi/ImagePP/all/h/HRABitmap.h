//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmap.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRABitmapBase.h"
#include "HCDCodec.h"

class HRADrawOptions;
class HRABitmapEditor;
class HRAClearOptions;
class HGSEditor;
class HGSSurfaceDescriptor;
class HCDPacket;

// ----------------------------------------------------------------------------
//  HRABitmap
// ----------------------------------------------------------------------------
class HRABitmap : public HRABitmapBase
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1002)

public:
    // Primary methods

    _HDLLg              HRABitmap   (HRPPixelType* pi_PixelType=NULL);
    _HDLLg              HRABitmap   (size_t                         pi_WidthPixels,
                                     size_t                         pi_HeightPixels,
                                     const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                                     const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                                     const HFCPtr<HRPPixelType>&    pi_rpPixel,
                                     uint32_t                       pi_BitsAlignment = 8,
                                     SLO                            pi_SLO = UPPER_LEFT_HORIZONTAL,
                                     const HFCPtr<HCDCodec>&        pi_rpCodec = 0);

    _HDLLg                 HRABitmap   (const HRABitmap& pi_rObj);

    _HDLLg virtual         ~HRABitmap  ();

    HRABitmap&      operator=(const HRABitmap& pi_rBitmap);

    virtual HPMPersistentObject* Clone() const;
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

    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms) override;

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pHistogramOptions,
                                     bool                pi_ForceRecompute = false) override;

    // Clear
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

    _HDLLg const HFCPtr<HCDPacket>& GetPacket() const;
    _HDLLg void                     SetPacket(const HFCPtr<HCDPacket>& pi_rPacket);

    // Memory manager
    _HDLLg void         SetPool(HPMPool* pi_pPool);
    _HDLLg HPMPool*     GetPool() const;

    bool           NotifyPaletteChanged (const HMGMessage& pi_rMessage);

protected:

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor>
    CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                            HFCPtr<HRPPixelType>* po_ppOutputPixelType) const override;


private:

    HFCPtr<HGSSurfaceDescriptor>
    m_pSurfaceDescriptor;

    HFCPtr<HCDPacket>
    m_pPacket;

    // Used by the MemoryMgr
    HPMPool*        m_pPool;

    // resampling
    HFCPtr<HGSEditor>       m_pResamplingSurfaceEditor;
    Byte                  m_PixelsInterval;
    HUINTX                  m_CurrentPixelX;
    HUINTX                  m_CurrentPixelY;
    HUINTX                  m_LastPixelX;

    // Methods

    void            DeepCopy(const HRABitmap& pi_rBitmap);
    void            DeepDelete();
    void            StretchWithHGS(HGFMappedSurface*                pio_pSurface,
                                   const HRADrawOptions*            pi_pOptions,
                                   const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const;

    void            WarpWithHGS(HGFMappedSurface*                   pio_pSurface,
                                const HRADrawOptions*               pi_pOptions,
                                const HFCPtr<HGF2DTransfoModel>&    pi_rpTransfoModel) const;

    void            ComputeHistogramRLE      (HRAHistogramOptions* pio_pOptions);
    void            ComputeLightnessHistogram(HRAHistogramOptions* pio_pOptions);
    void            ComputeGrayscaleHistogram(HRAHistogramOptions* pio_pOptions);
    void            ComputeRGBHistogram      (HRAHistogramOptions* pio_pOptions);
    void            ComputeRGBAHistogram     (HRAHistogramOptions* pio_pOptions);

    const void*     GetFirstResamplingPixel(HRABitmapEditor*        pi_pEditor,
                                            Byte                  pi_PixelsInterval);
    const void*     GetNextResamplingPixel();
    void            TerminateResamplingPixel();

    const void*     GetFirstResamplingLine(HRABitmapEditor*     pi_pEditor,
                                           Byte               pi_PixelsInterval,
                                           size_t*              po_pPixelCount);
    const void*     GetNextResamplingLine(size_t* po_pPixelCount);


    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg)
    };

