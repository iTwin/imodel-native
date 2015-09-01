//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAIteratorOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAIteratorOptions
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HRARaster;
class HVEShape;
class HGF2DCoordSys;


class HNOVTABLEINIT HRAIteratorOptions
    {
public:

    // Primary methods

    IMAGEPP_EXPORT                  HRAIteratorOptions();

    HRAIteratorOptions(const HRAIteratorOptions& pi_rOptions);

    HRAIteratorOptions(const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys);

    HRAIteratorOptions(const HFCPtr<HVEShape>&      pi_rpRegionToProcess,
                       const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys,
                       bool                        pi_ClipUsingEffectiveShape = true);

    HRAIteratorOptions(const HFCPtr<HVEShape>& pi_rpRegionToProcess,
                       bool                   pi_ClipUsingEffectiveShape = true);

    IMAGEPP_EXPORT                ~HRAIteratorOptions();

    HRAIteratorOptions&
    operator=(const HRAIteratorOptions& pi_rObj);


    // Make the options keep a personal copy of some attributes.
    void            CreatePrivateCopies();

    // Clip using this raster's effective shape if necessary
    void            ClipRegionToRaster(const HFCPtr<HRARaster>& pi_rpRaster);

    // Simply obtain the resulting (clipped) region, without
    // modifying the internal region.
    HFCPtr<HVEShape>
    CalculateClippedRegion(const HFCPtr<HRARaster>& pi_rpRaster) const;

    // Attributes get/set

    const HFCPtr<HVEShape>&
    GetRegionToProcess() const;
    void            SetRegionToProcess(HFCPtr<HVEShape>& pi_rpRegionToProcess);
    bool           IsShaped() const;

    bool           MustClipUsingEffectiveShape() const;
    void            ClipUsingEffectiveShape(bool pi_Clip);

    const HFCPtr<HGF2DCoordSys>&
    GetPhysicalCoordSys() const;
    void            SetPhysicalCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys);

    uint8_t        MaxResolutionStretchingFactor() const;
    void            SetMaxResolutionStretchingFactor(uint8_t pi_Factor);

protected:

private:

    // The region to iterate on
    HFCPtr<HVEShape>            m_pRegionToProcess;

    // Does the iterator have to clip the region to process or not
    bool                       m_ClipUsingEffectiveShape;

    // Physical CS of the data we need
    HFCPtr<HGF2DCoordSys>       m_pPhysicalCoordSys;

    bool                       m_MembersArePrivate;

    uint8_t                     m_MaxResolutionStretchingFactor;
    };


END_IMAGEPP_NAMESPACE