//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADEMRaster.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRADEMRaster
//-----------------------------------------------------------------------------
// This class is used to alter pixels of DEM raster file
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

class HRAStoredRaster;
class HRAReferenceToStoredRaster;
class HGSSurface;
class HRPDEMFilter;
class HGSMemorySurfaceDescriptor;
class HRPPixelNeighbourhood;

class HRADEMRaster : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1017)

public:

    // Primary methods
    _HDLLg                 HRADEMRaster();

    _HDLLg                 HRADEMRaster(const HFCPtr<HRAStoredRaster>& pi_pSource,
                                        double                        pi_PixelSizeX,
                                        double                        pi_PixelSizeY,
                                        HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                                        HFCPtr<HRPDEMFilter> const&    pi_Filter);
    _HDLLg                 HRADEMRaster(const HFCPtr<HRAReferenceToStoredRaster>& pi_pSource,
                                        double                        pi_PixelSizeX,
                                        double                        pi_PixelSizeY,
                                        HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                                        HFCPtr<HRPDEMFilter> const&    pi_Filter);

    _HDLLg                 HRADEMRaster(const HRADEMRaster& pi_rObject);

    _HDLLg virtual         ~HRADEMRaster();


    // Overridden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const override;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role, Byte pi_Id) const override;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions) override;

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_rpSurface, const HGFDrawOptions* pi_pOptions) const override;


protected:


private:
    HFCPtr<HGSMemorySurfaceDescriptor>  ApplyFilter(HFCPtr<HGSMemorySurfaceDescriptor> pSourceSurfaceDesc, HFCPtr<HGF2DTransfoModel> pi_pSourceTransfo) const;

    void            HandleBorderCases(const HFCPtr<HGSSurface>&    pio_rpSurface,
                                      const HRPPixelNeighbourhood& pi_rNeighborhood) const;

    HFCPtr<HGF2DTransfoModel>           ComputeSourceToDestinationTransfo(const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                                          const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys) const;

    HFCPtr<HRPDEMFilter>    m_pFilter;
    HFCPtr<HRAStoredRaster> m_pSourceStoredRaster;
    };