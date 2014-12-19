//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFResolutionDescriptor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFResolutionDescriptor
//-----------------------------------------------------------------------------
// This class is used to describe the resolutions scaling.
//-----------------------------------------------------------------------------

#pragma once


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
    _HDLLg              HGFResolutionDescriptor( uint32_t pi_MaxWidth,
                                                 uint32_t pi_MaxHeight);

    _HDLLg              HGFResolutionDescriptor( uint32_t pi_Width,
                                                 uint32_t pi_Height,
                                                 uint32_t pi_MinWidth,
                                                 uint32_t pi_MinHeight,
                                                 bool  pi_IsTiled = true,
                                                 uint32_t pi_ResolutionFactor = 2);

    _HDLLg              HGFResolutionDescriptor(const HGFResolutionDescriptor& pi_rObj);

    _HDLLg virtual     ~HGFResolutionDescriptor();

    // Operator
    HGFResolutionDescriptor&
    operator=(const HGFResolutionDescriptor& pi_rObj);

    // Resolution information methods.
    _HDLLg unsigned short CountResolutions() const;

    _HDLLg double       GetResolution   (unsigned short pi_SubImage) const;
    _HDLLg uint32_t    GetWidth        (unsigned short pi_SubImage) const;
    _HDLLg uint32_t    GetHeight       (unsigned short pi_SubImage) const;
    void                GetDescription  (unsigned short pi_SubImage,
                                         double*   po_pResolution,
                                         uint32_t*    po_pWidth,
                                         uint32_t*    po_pHeight)  const;

    _HDLLg /*IppImaging_Needs*/ void AddResolution  (double      pi_Resolution,
                                                     uint32_t   pi_Width,
                                                     uint32_t   pi_Height);

    void                AddResolutionRelativeToBase(double pi_Resolution,
                                                    uint32_t pi_MinWidth,
                                                    uint32_t pi_MinHeight,
                                                    uint32_t pi_ResolutionFactor = 2);

    _HDLLg void        AddResolutionThatSupports(uint32_t pi_Width,
                                                 uint32_t pi_Height,
                                                 uint32_t pi_MinWidth,
                                                 uint32_t pi_MinHeight,
                                                 uint32_t pi_ResolutionFactor = 2);

    void                RemoveResolution(unsigned short pi_SubImage);
    _HDLLg /*IppImaging_Needs*/ void                RemoveResolutions();

private:
    // Members.

    // List of Resolution
    typedef map<double, ResolutionDescriptor*, less<double>, allocator<ResolutionDescriptor*> > HGFListOfResolution;
    HGFListOfResolution m_ListOfResolution;
    };

