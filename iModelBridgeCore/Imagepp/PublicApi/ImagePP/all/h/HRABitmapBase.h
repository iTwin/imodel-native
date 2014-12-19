//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapBase.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAStoredRaster.h"

class HRADrawOptions;
class HRABitmapEditor;
class HGSSurfaceDescriptor;
class HCDCodec;
class HCDPacket;

class HRABitmapBase : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1741)

public:
    enum SLO
        {
        // SLOs (Data Orientation)
        UPPER_LEFT_VERTICAL    = 0,
        UPPER_RIGHT_VERTICAL   ,
        LOWER_LEFT_VERTICAL    ,
        LOWER_RIGHT_VERTICAL   ,
        UPPER_LEFT_HORIZONTAL  ,
        UPPER_RIGHT_HORIZONTAL ,
        LOWER_LEFT_HORIZONTAL  ,
        LOWER_RIGHT_HORIZONTAL ,
        };

    _HDLLg virtual         ~HRABitmapBase  ();

    virtual HPMPersistentObject* Clone () const=0;

    virtual HRARaster*
    Clone (HPMObjectStore* pi_pStore,
           HPMPool*        pi_pLog=0) const=0;

    // Inherited from HRARaster
    virtual HRARasterEditor*
    CreateEditor   (HFCAccessMode pi_Mode)=0;

    virtual HRARasterEditor*
    CreateEditor   (const HVEShape& pi_rShape,
                    HFCAccessMode   pi_Mode)=0;

    virtual HRARasterEditor*
    CreateEditor   (const HGFScanLines& pi_rShape,
                    HFCAccessMode       pi_Mode)=0;

    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode)=0;

    virtual HRARasterIterator*
    CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pHistogramOptions,
                                     bool                pi_ForceRecompute = false)=0;


    // Special status...
    //
    // Must call the parent
    virtual void    MakeEmpty()=0;
    virtual bool   IsEmpty() const=0;

    virtual size_t  GetAdditionalSize() const=0;

    // surface descriptor
    _HDLLg virtual HFCPtr<HGSSurfaceDescriptor> GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType = 0,
                                                                     HFCPtr<HRPPixelType>* po_ppOutputPixelType = 0) const=0;

    virtual void    Updated(const HVEShape* pi_pModifiedContent = 0)=0;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions)=0;

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const=0;

    virtual const HFCPtr<HCDCodec>&
    GetCodec() const=0;

    HRABitmapBase::SLO  GetSLO() const;

    void            SetPosInRaster  (HUINTX     pi_PosX,
                                     HUINTX     pi_PosY);
    void            GetPosInRaster  (HUINTX*    po_pPosX,
                                     HUINTX*    po_pPosY) const;

protected:

    HRABitmapBase (HRPPixelType*                    pi_PixelType=NULL,
                   bool                            pi_Resizable = false);
    HRABitmapBase (size_t                           pi_WidthPixels,
                   size_t                           pi_HeightPixels,
                   const HGF2DTransfoModel*         pi_pModelCSp_CSl,
                   const HFCPtr<HGF2DCoordSys>&     pi_rpLogicalCoordSys,
                   const HFCPtr<HRPPixelType>&      pi_rpPixel,
                   uint32_t                         pi_BitsAlignment = 8,
                   SLO                              pi_SLO = UPPER_LEFT_HORIZONTAL,
                   bool                            pi_Resizable = false);

    HRABitmapBase   (const HRABitmapBase& pi_rObj);

    HRABitmapBase&      operator=(const HRABitmapBase& pi_rBitmap);

    virtual HFCPtr<HGSSurfaceDescriptor> CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                                 HFCPtr<HRPPixelType>* po_ppOutputPixelType) const=0;

    SLO             m_SLO;
    uint32_t        m_BitsAlignment;
    HUINTX          m_XPosInRaster;
    HUINTX          m_YPosInRaster;
    };


#include "HRABitmapBase.hpp"
