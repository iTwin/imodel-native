//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemoryBaseSurfaceDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSMemoryBaseSurfaceDescriptor
//-----------------------------------------------------------------------------
// General class for MemorySurfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceDescriptor.h"
#include "HGFScanlineOrientation.h"

BEGIN_IMAGEPP_NAMESPACE

class HRPPixelType;
class HCDCodec;

class HGSMemoryBaseSurfaceDescriptor : public HGSSurfaceDescriptor
    {
    HDECLARE_CLASS_ID(HGSMemoryId_BaseSurfaceDescriptor, HGSSurfaceDescriptor)

    virtual const HFCPtr<HCDCodec>&
    GetCodec() const =0;

    HGFSLO          GetSLO() const;

    uint32_t        GetBytesPerRow() const;

    const HFCPtr<HRPPixelType>& GetPixelType() const;

protected:

    HGSMemoryBaseSurfaceDescriptor(uint32_t                          pi_Width,
                                   uint32_t                          pi_Height,
                                   const HFCPtr<HRPPixelType>&        pi_rpPixelType,
                                   HGFSLO                             pi_SLO,
                                   uint32_t                          pi_BytesPerRow);

    virtual  ~HGSMemoryBaseSurfaceDescriptor();

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

END_IMAGEPP_NAMESPACE
#include "HGSMemoryBaseSurfaceDescriptor.hpp"

