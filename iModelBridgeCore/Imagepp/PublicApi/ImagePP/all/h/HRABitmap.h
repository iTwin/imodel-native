//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmap.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRABitmapBase.h"

BEGIN_IMAGEPP_NAMESPACE
class HRADrawOptions;
class HRABitmapEditor;
class HRAClearOptions;
class HGSSurfaceDescriptor;
class HCDPacket;
//class IHPMMemoryManager;
class HRAEditor;


// ----------------------------------------------------------------------------
//  HRABitmap
// ----------------------------------------------------------------------------
class HRABitmap : public HRABitmapBase
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRABitmapId)

public:
    // Primary methods

    IMAGEPP_EXPORT static HFCPtr<HRABitmap> Create(uint32_t                       pi_WidthPixels,
                                                  uint32_t                       pi_HeightPixels,
                                                  const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                                                  const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                                                  const HFCPtr<HRPPixelType>&    pi_rpPixel,
                                                  uint32_t                       pi_BitsAlignment = 8);

    virtual HPMPersistentObject* Clone() const;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

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

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pHistogramOptions,
                                     bool                pi_ForceRecompute = false) override;

    // Clear
    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    // Special status...
    //
    // Must call the parent
    virtual void    MakeEmpty() override;
    virtual bool    IsEmpty() const override;

    virtual size_t  GetAdditionalSize() const override;

    virtual void    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels) override;

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor>
                    GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType = 0,
                                         HFCPtr<HRPPixelType>* po_ppOutputPixelType = 0) const override;

    virtual void    Updated(const HVEShape* pi_pModifiedContent = 0) override;

    // packets and codecs
    virtual const HFCPtr<HCDCodec>&
                    GetCodec() const override;

    IMAGEPP_EXPORT const HFCPtr<HCDPacket>& GetPacket() const;
    IMAGEPP_EXPORT void                     SetPacket(const HFCPtr<HCDPacket>& pi_rPacket);

    // Memory manager
    IMAGEPP_EXPORT void         SetPool(HPMPool* pi_pPool);
    IMAGEPP_EXPORT HPMPool*     GetPool() const;

    bool            NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    uint32_t GetBitsAlignment() const;

    size_t ComputeBytesPerWidth() const;

    virtual HRABitmap*   _AsHRABitmapP() override;

protected:

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor>
                    CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                            HFCPtr<HRPPixelType>* po_ppOutputPixelType) const override;


private:

    // *** Keep private. We want all HRABitmaps to be HFCPtr.
    HRABitmap   (HRPPixelType* pi_PixelType=NULL);
    HRABitmap   (uint32_t                       pi_WidthPixels,
                 uint32_t                       pi_HeightPixels,
                 const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                 const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                 const HFCPtr<HRPPixelType>&    pi_rpPixel,
                 uint32_t                       pi_BitsAlignment = 8);

    HRABitmap   (const HRABitmap& pi_rObj);

    HRABitmap& operator=(const HRABitmap& pi_rBitmap);

    virtual ~HRABitmap  ();

    HFCPtr<HGSSurfaceDescriptor> m_pSurfaceDescriptor;

    HFCPtr<HCDPacket> m_pPacket;

    // Used by the MemoryMgr
    HPMPool*        m_pPool;

    // resampling
    HRAEditor*              m_pResamplingSurfaceEditor;
    Byte                    m_PixelsInterval;
    HUINTX                  m_CurrentPixelX;
    HUINTX                  m_CurrentPixelY;
    HUINTX                  m_LastPixelX;

    // Methods

    void            DeepCopy(const HRABitmap& pi_rBitmap);
    void            DeepDelete();

    //&&Backlog EN: we need to find a way to move out this histogramm stuff from the HRAs. We should have some kind of draw to analyze the data.
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


    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE