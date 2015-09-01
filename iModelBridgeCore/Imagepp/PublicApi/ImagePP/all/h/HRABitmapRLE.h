//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapRLE.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRABitmapBase.h"

BEGIN_IMAGEPP_NAMESPACE
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
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRABitmapId_RLE)

public:

    IMAGEPP_EXPORT static HFCPtr<HRABitmapRLE> Create(uint32_t                       pi_WidthPixels,
                                                     uint32_t                       pi_HeightPixels,
                                                     const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                                                     const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                                                     const HFCPtr<HRPPixelType>&    pi_rpPixel,
                                                     bool                           pi_Resizable = false);


    virtual HPMPersistentObject* Clone () const;
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

    // packets and codecs
    virtual const HFCPtr<HCDCodec>&
    GetCodec() const override;

    IMAGEPP_EXPORT const HFCPtr<HCDPacketRLE>& GetPacket() const;
    IMAGEPP_EXPORT       HFCPtr<HCDPacket> GetContiguousBuffer() const;


    bool           NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    virtual HRABitmapRLE* _AsHRABitmapRleP() override;

protected:

    // Primary methods
    HRABitmapRLE(bool                          pi_Resizable = false);
    
    HRABitmapRLE(uint32_t                       pi_WidthPixels,
                 uint32_t                       pi_HeightPixels,
                 const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                 const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                 const HFCPtr<HRPPixelType>&    pi_rpPixel,
                 bool                          pi_Resizable = false);

    HRABitmapRLE& operator=(const HRABitmapRLE& pi_rBitmap);
    HRABitmapRLE   (const HRABitmapRLE& pi_rObj);

    virtual      ~HRABitmapRLE  ();

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) override;        

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

    void            ComputeHistogramRLE      (HRAHistogramOptions* pio_pOptions);

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE