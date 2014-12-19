//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelNeighbourhood.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelNeighbourhood
//-----------------------------------------------------------------------------
// This class describes the Neighbourhood of a filter
//-----------------------------------------------------------------------------
#pragma once

class HRPPixelNeighbourhood : public HFCShareableObject<HRPPixelNeighbourhood>
    {
    HDECLARE_BASECLASS_ID(1129)

public:

    // Primary methods
    HRPPixelNeighbourhood();

    _HDLLg                 HRPPixelNeighbourhood(    uint32_t pi_Width,
                                                     uint32_t pi_Height,
                                                     uint32_t pi_XOrigin,
                                                     uint32_t pi_YOrigin);

    _HDLLg                 HRPPixelNeighbourhood(const HRPPixelNeighbourhood& pi_rNeighbourhood);


    _HDLLg virtual         ~HRPPixelNeighbourhood();


    // Settings

    uint32_t          GetHeight() const;

    uint32_t          GetWidth() const;

    uint32_t          GetXOrigin() const;

    uint32_t          GetYOrigin() const;

    bool           IsUnity() const;

    // Operations

    HRPPixelNeighbourhood
    operator+(const HRPPixelNeighbourhood& pi_rNeighbourhood) const;
    _HDLLg HRPPixelNeighbourhood&
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

#include "HRPPixelNeighbourhood.hpp"

