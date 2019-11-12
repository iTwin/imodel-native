//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"

BEGIN_IMAGEPP_NAMESPACE

class HRAClearOptions;

/** -----------------------------------------------------------------------------
    This class represents a view of another raster. It is the base for
    logical (non-destructive) transformations on rasters.

    By default, all method implementations work directly with the source
    object, as if there was no wrapper class. Some operations as logically
    disabled instead of being applied to the source.
    -----------------------------------------------------------------------------
*/
class HRAImageView : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAImageViewId)

public:

    // Primary methods

    virtual         ~HRAImageView();

    // Overriden methods

    bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const override;

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    HRARasterIterator*
    CreateIterator (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const override;

    HGF2DExtent
    GetAveragePixelSize () const override;
    void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const override;

    HFCPtr<HVEShape>
    GetEffectiveShape () const override;

    HFCPtr<HRPPixelType>
    GetPixelType() const override;

    bool   HasSinglePixelType() const override;

    void    Move(const HGF2DDisplacement& pi_rDisplacement) override;

    void    Rotate(double               pi_Angle,
                           const HGF2DLocation& pi_rOrigin) override;

    void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin) override;

    void    SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_pCoordSys) override;

    HGF2DExtent
    GetExtent() const override;

    const HVEShape&
    GetShape    () const override;
    void    SetShape    (const HVEShape& pi_rShape) override;

    // Source
    IMAGEPP_EXPORT const HFCPtr<HRARaster>&
    GetSource() const;

    // LookAhead Methods
    bool   HasLookAhead() const override;
    void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false) override;

    //Context Methods
    void               SetContext(const HFCPtr<HMDContext>& pi_rpContext) override;
    HFCPtr<HMDContext> GetContext() override;

    void               InvalidateRaster() override;

protected:

    // Primary methods
    HRAImageView();
    HRAImageView(const HFCPtr<HRARaster>& pi_pSource);
    HRAImageView(const HRAImageView& pi_rImageView);

    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) override;

private:
    
    HFCPtr<HRARaster> m_pSource;
    };

END_IMAGEPP_NAMESPACE
