//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADEMRaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRADEMRaster
//-----------------------------------------------------------------------------
// This class is used to alter pixels of DEM raster file
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

BEGIN_IMAGEPP_NAMESPACE

class HRAStoredRaster;
class HRAReferenceToStoredRaster;
class HRASurface;
class HRPDEMFilter;
class HGSMemorySurfaceDescriptor;
class HRPPixelNeighbourhood;


class HRADEMRaster : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRADEMRasterId)

public:

    // Primary methods
    IMAGEPP_EXPORT                 HRADEMRaster();

    IMAGEPP_EXPORT                 HRADEMRaster(const HFCPtr<HRAStoredRaster>& pi_pSource,
                                        double                        pi_PixelSizeX,
                                        double                        pi_PixelSizeY,
                                        HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                                        HFCPtr<HRPDEMFilter> const&    pi_Filter);
    IMAGEPP_EXPORT                 HRADEMRaster(const HFCPtr<HRAReferenceToStoredRaster>& pi_pSource,
                                        double                        pi_PixelSizeX,
                                        double                        pi_PixelSizeY,
                                        HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                                        HFCPtr<HRPDEMFilter> const&    pi_Filter);

    IMAGEPP_EXPORT                 HRADEMRaster(const HRADEMRaster& pi_rObject);

    IMAGEPP_EXPORT virtual         ~HRADEMRaster();


    // Overridden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const override;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role, Byte pi_Id) const override;

    HFCPtr<HRPDEMFilter> GetFilter() const {return m_pFilter;}
protected:

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;


private:
    HFCPtr<HGSMemorySurfaceDescriptor>  ApplyFilter(HFCPtr<HGSMemorySurfaceDescriptor> pSourceSurfaceDesc, HFCPtr<HGF2DTransfoModel> pi_pSourceTransfo) const;

    void HandleBorderCases(HRASurface& pio_destSurface, const HRPPixelNeighbourhood& pi_rNeighborhood) const;

    HFCPtr<HGF2DTransfoModel>           ComputeSourceToDestinationTransfo(const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                                          const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys) const;

    HRAImageOpPtr           m_pFilterOp;
    HFCPtr<HRPDEMFilter>    m_pFilter;
    HFCPtr<HRAStoredRaster> m_pSourceStoredRaster;
    };
END_IMAGEPP_NAMESPACE