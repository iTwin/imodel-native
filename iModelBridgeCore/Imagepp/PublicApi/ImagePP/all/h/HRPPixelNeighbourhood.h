//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelNeighbourhood.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelNeighbourhood
//-----------------------------------------------------------------------------
// This class describes the Neighbourhood of a filter
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelNeighbourhood : public HFCShareableObject<HRPPixelNeighbourhood>
    {
    HDECLARE_BASECLASS_ID(HRPPixelNeighbourhoodId_Base)

public:

    // Primary methods
    HRPPixelNeighbourhood();

    IMAGEPP_EXPORT                 HRPPixelNeighbourhood(    uint32_t pi_Width,
                                                     uint32_t pi_Height,
                                                     uint32_t pi_XOrigin,
                                                     uint32_t pi_YOrigin);

    IMAGEPP_EXPORT                 HRPPixelNeighbourhood(const HRPPixelNeighbourhood& pi_rNeighbourhood);


    IMAGEPP_EXPORT virtual         ~HRPPixelNeighbourhood();


    // Settings

    uint32_t          GetHeight() const;

    uint32_t          GetWidth() const;

    uint32_t          GetXOrigin() const;

    uint32_t          GetYOrigin() const;

    bool           IsUnity() const;

    // Operations

    HRPPixelNeighbourhood
    operator+(const HRPPixelNeighbourhood& pi_rNeighbourhood) const;
    IMAGEPP_EXPORT HRPPixelNeighbourhood&
    operator=(const HRPPixelNeighbourhood& pi_rObj);

protected:

private:

    uint32_t          m_Width;
    uint32_t          m_Height;

    uint32_t          m_XOrigin;
    uint32_t          m_YOrigin;

    // default values
    bool             operator==(const HRPPixelNeighbourhood& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#include "HRPPixelNeighbourhood.hpp"

