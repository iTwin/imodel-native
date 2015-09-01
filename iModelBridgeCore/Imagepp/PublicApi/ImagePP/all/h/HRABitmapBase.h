//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapBase.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAStoredRaster.h"

BEGIN_IMAGEPP_NAMESPACE
class HRADrawOptions;
class HRABitmapEditor;
class HGSSurfaceDescriptor;
class HCDCodec;
class HCDPacket;
class HRABitmapRLE;

class HRABitmapBase : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRABitmapId_Base)

public:
// SLO support has been deprecated. Only SLO 4 and SLO 6 were supported. Now, all bitmap are SLO 4. 
// If SLO 6 is required, it can be supported by adding a flip transfo model.
// ex: HGF2DStretch flipModel(HGF2DDisplacement(0.0, Height), 1.0, -1.0);
// 
//     enum SLO
//         {
//         // SLOs (Data Orientation)  Only SLO 4 and 6 are supported.
//         //UPPER_LEFT_VERTICAL    = 0,
//         //UPPER_RIGHT_VERTICAL   ,
//         //LOWER_LEFT_VERTICAL    ,
//         //LOWER_RIGHT_VERTICAL   ,
//         UPPER_LEFT_HORIZONTAL  = 4,
//         //UPPER_RIGHT_HORIZONTAL ,
//         LOWER_LEFT_HORIZONTAL  = 6,
//         //LOWER_RIGHT_HORIZONTAL ,
//         };

    virtual HPMPersistentObject* Clone () const=0;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const =0;

    // Inherited from HRARaster
    virtual HRARasterEditor*    CreateEditor   (HFCAccessMode pi_Mode)=0;

    virtual HRARasterEditor*    CreateEditor   (const HVEShape& pi_rShape,
                                                HFCAccessMode   pi_Mode)=0;

    virtual HRARasterEditor*    CreateEditor   (const HGFScanLines& pi_rShape,
                                                HFCAccessMode       pi_Mode)=0;

    virtual HRARasterEditor*    CreateEditorUnShaped (HFCAccessMode pi_Mode)=0;

    virtual HRARasterIterator*  CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual void                ComputeHistogram(HRAHistogramOptions* pio_pHistogramOptions,
                                                 bool                pi_ForceRecompute = false)=0;


    // Special status...
    //
    // Must call the parent
    virtual void                MakeEmpty()=0;
    virtual bool                IsEmpty() const=0;

    virtual size_t              GetAdditionalSize() const=0;

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor> GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType = 0,
                                                              HFCPtr<HRPPixelType>* po_ppOutputPixelType = 0) const=0;

    virtual void                Updated(const HVEShape* pi_pModifiedContent = 0)=0;

    virtual const HFCPtr<HCDCodec>&
                                GetCodec() const=0;

    void                        SetPosInRaster (HUINTX     pi_PosX,
                                                HUINTX     pi_PosY);
    void                        GetPosInRaster (HUINTX*    po_pPosX,
                                                HUINTX*    po_pPosY) const;

    virtual HRABitmap*          _AsHRABitmapP()     { return NULL; }
    virtual HRABitmapRLE*       _AsHRABitmapRleP()  { return NULL; }

protected:

    HRABitmapBase (HRPPixelType*                    pi_PixelType=NULL,
                   bool                            pi_Resizable = false);

    HRABitmapBase (size_t                           pi_WidthPixels,
                   size_t                           pi_HeightPixels,
                   const HGF2DTransfoModel*         pi_pModelCSp_CSl,
                   const HFCPtr<HGF2DCoordSys>&     pi_rpLogicalCoordSys,
                   const HFCPtr<HRPPixelType>&      pi_rpPixel,
                   uint32_t                         pi_BitsAlignment = 8,
                   bool                            pi_Resizable = false);

    HRABitmapBase   (const HRABitmapBase& pi_rObj);

    virtual ~HRABitmapBase  ();

    HRABitmapBase&      operator=(const HRABitmapBase& pi_rBitmap);

    virtual HFCPtr<HGSSurfaceDescriptor> CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                                 HFCPtr<HRPPixelType>* po_ppOutputPixelType) const=0;

    void StretchWithHGS(HGFMappedSurface&                pio_destSurface,
                        HRADrawOptions const&            pi_pOptions,
                        const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const;

    void WarpWithHGS(HGFMappedSurface&                pio_destSurface,
                     HRADrawOptions const&            pi_pOptions,
                     const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const;

    uint32_t       m_BitsAlignment;
    HUINTX          m_XPosInRaster;
    HUINTX          m_YPosInRaster;
    };

END_IMAGEPP_NAMESPACE
#include "HRABitmapBase.hpp"
