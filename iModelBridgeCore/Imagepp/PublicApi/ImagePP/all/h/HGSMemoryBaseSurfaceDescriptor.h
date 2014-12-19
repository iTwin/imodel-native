//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemoryBaseSurfaceDescriptor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSMemoryBaseSurfaceDescriptor
//-----------------------------------------------------------------------------
// General class for MemorySurfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceDescriptor.h"
#include "HGFScanlineOrientation.h"

class HRPPixelType;
class HCDCodec;

class HGSMemoryBaseSurfaceDescriptor : public HGSSurfaceDescriptor
    {
    HDECLARE_CLASS_ID(1711, HGSSurfaceDescriptor)

    virtual HGSSurfaceCapabilities*
    GetRequiredSurfaceCapabilities() const override;

    virtual const HFCPtr<HCDCodec>&
    GetCodec() const =0;

    HGFSLO          GetSLO() const;

    uint32_t        GetBytesPerRow() const;

    _HDLLg const HFCPtr<HRPPixelType>&
    GetPixelType() const;

protected:

    _HDLLg          HGSMemoryBaseSurfaceDescriptor(uint32_t                          pi_Width,
                                                   uint32_t                          pi_Height,
                                                   const HFCPtr<HRPPixelType>&       pi_rpPixelType,
                                                   HGFSLO                            pi_SLO,
                                                   uint32_t                          pi_BytesPerRow);

    _HDLLg virtual  ~HGSMemoryBaseSurfaceDescriptor();

    HFCPtr<HRPPixelType>    m_pPixelType;

    HGFSLO                  m_SLO;

    uint32_t                m_BytesPerRow;

    // disabled methods
    HGSMemoryBaseSurfaceDescriptor();
    HGSMemoryBaseSurfaceDescriptor&
    operator=(const HGSMemoryBaseSurfaceDescriptor& pi_rObj);
    bool             operator==(const HGSMemoryBaseSurfaceDescriptor& pi_rObj) const;
    bool             operator!=(const HGSMemoryBaseSurfaceDescriptor& pi_rObj);
    HGSMemoryBaseSurfaceDescriptor(const HGSMemoryBaseSurfaceDescriptor& pi_rObj);
    };

#include "HGSMemoryBaseSurfaceDescriptor.hpp"

