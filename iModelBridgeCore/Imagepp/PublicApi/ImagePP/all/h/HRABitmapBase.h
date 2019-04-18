//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAStoredRaster.h"

struct BitmapSourceNode;

BEGIN_IMAGEPP_NAMESPACE
class HRABitmapEditor;
class HGSSurfaceDescriptor;
class HCDCodec;
class HCDPacket;
class HRABitmapRLE;
class HRATiledRaster;


class HRABitmapBase : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRABitmapId_Base)

public:
    friend BitmapSourceNode;
    friend HRATiledRaster;
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

    virtual HRABitmapEditor* CreateEditor(HFCAccessMode pi_Mode) = 0;

    virtual HRABitmapEditor* CreateEditor(const HVEShape& pi_rShape, HFCAccessMode pi_Mode) = 0;

    virtual HRABitmapEditor* CreateEditor(const HGFScanLines& pi_rShape, HFCAccessMode pi_Mode) = 0;

    virtual HRABitmapEditor* CreateEditorUnShaped(HFCAccessMode pi_Mode) = 0;

    HRARasterIterator*  CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const override;

    
    // Special status...
    //
    // Must call the parent
    virtual void                MakeEmpty()=0;
    virtual bool                IsEmpty() const=0;

    // surface descriptor
    virtual HFCPtr<HGSSurfaceDescriptor> GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType = 0,
                                                              HFCPtr<HRPPixelType>* po_ppOutputPixelType = 0) const=0;

    virtual void                Updated(const HVEShape* pi_pModifiedContent = 0)=0;

    virtual const HFCPtr<HCDCodec>&
                                GetCodec() const=0;

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

    HRABitmapBase&      operator=(const HRABitmapBase& pi_rBitmap) = delete;

    virtual HFCPtr<HGSSurfaceDescriptor> CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                                 HFCPtr<HRPPixelType>* po_ppOutputPixelType) const=0;

    // These are used in the context of a HRATiledRaster. They add per tile context information.
    void GetPosInRaster(HUINTX* po_pPosX, HUINTX* po_pPosY) const;
    void SetPosInRaster(HUINTX pi_PosX, HUINTX pi_PosY);
    void SetTileDataSize(uint32_t width, uint32_t height) { m_dataWidth = width; m_dataHeight = height; }

    uint32_t       m_BitsAlignment;
    HUINTX          m_XPosInRaster;
    HUINTX          m_YPosInRaster;

    // Used during tile sub-resolution generation.
    // TiledRaster creates tiles border with the block size even though the actual data size is less. This is problematic for sampler with neighborhood
    // since they will use this extra data instead of repeating the last pixel.
    uint32_t        m_dataWidth = 0;
    uint32_t        m_dataHeight = 0;
    };

END_IMAGEPP_NAMESPACE
#include "HRABitmapBase.hpp"
