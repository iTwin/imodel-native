//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFResolutionDescriptor
//-----------------------------------------------------------------------------
// This class is used to describe the resolutions scaling.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

// Descriptor for a resolution
typedef    struct
    {
    uint32_t Width;
    uint32_t Height;
    } ResolutionDescriptor;


class HGFResolutionDescriptor
    {
public:
    // Primary methods.
    IMAGEPP_EXPORT              HGFResolutionDescriptor( uint32_t pi_MaxWidth,
                                                 uint32_t pi_MaxHeight);

    IMAGEPP_EXPORT              HGFResolutionDescriptor( uint32_t pi_Width,
                                                 uint32_t pi_Height,
                                                 uint32_t pi_MinWidth,
                                                 uint32_t pi_MinHeight,
                                                 bool  pi_IsTiled = true,
                                                 uint32_t pi_ResolutionFactor = 2);

    IMAGEPP_EXPORT              HGFResolutionDescriptor(const HGFResolutionDescriptor& pi_rObj);

    IMAGEPP_EXPORT virtual     ~HGFResolutionDescriptor();

    // Operator
    HGFResolutionDescriptor&
    operator=(const HGFResolutionDescriptor& pi_rObj);

    // Resolution information methods.
    IMAGEPP_EXPORT uint16_t CountResolutions() const;

    IMAGEPP_EXPORT double       GetResolution   (uint16_t pi_SubImage) const;
    IMAGEPP_EXPORT uint32_t    GetWidth        (uint16_t pi_SubImage) const;
    IMAGEPP_EXPORT uint32_t    GetHeight       (uint16_t pi_SubImage) const;
    IMAGEPP_EXPORT void                GetDescription  (uint16_t pi_SubImage,
                                         double*   po_pResolution,
                                         uint32_t*    po_pWidth,
                                         uint32_t*    po_pHeight)  const;

    IMAGEPP_EXPORT void AddResolution  (double      pi_Resolution,
                                                     uint32_t   pi_Width,
                                                     uint32_t   pi_Height);

    void                AddResolutionRelativeToBase(double pi_Resolution,
                                                    uint32_t pi_MinWidth,
                                                    uint32_t pi_MinHeight,
                                                    uint32_t pi_ResolutionFactor = 2);

    IMAGEPP_EXPORT void        AddResolutionThatSupports(uint32_t pi_Width,
                                                 uint32_t pi_Height,
                                                 uint32_t pi_MinWidth,
                                                 uint32_t pi_MinHeight,
                                                 uint32_t pi_ResolutionFactor = 2);

    void                RemoveResolution(uint16_t pi_SubImage);
    IMAGEPP_EXPORT void                RemoveResolutions();

private:
    // Members.

    // List of Resolution
    typedef map<double, ResolutionDescriptor*, less<double>, allocator<std::pair<const double, ResolutionDescriptor*> > > HGFListOfResolution;
    HGFListOfResolution m_ListOfResolution;
    };

END_IMAGEPP_NAMESPACE
