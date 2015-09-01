//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPFilter
//-----------------------------------------------------------------------------
// This class describes the interface for any filter.  Abstract class.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelNeighbourhood.h"

#include "HRPPixelType.h"
#include "HRPPixelBuffer.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HRPPixelBuffer;

class HNOVTABLEINIT HRPFilter : public HFCShareableObject<HRPFilter>
    {
    HDECLARE_BASECLASS_ID(HRPFilterId_Base)

public:

    // Primary methods
    virtual                    ~HRPFilter();


    // Conversion
    virtual void            Convert(HRPPixelBuffer* pi_pInputBuffer,
                                    HRPPixelBuffer* pio_pOutputBuffer) = 0;
    void                    Convert(const HFCPtr<HRPPixelType>& pio_pPixelType);

    // Composing
    IMAGEPP_EXPORT virtual HRPFilter*      ComposeWith(const HRPFilter* pi_pFilter);

    // Neighbourhood
    const HRPPixelNeighbourhood&
    GetNeighbourhood() const;


    // Pixel types
    const HFCPtr<HRPPixelType>&
    GetInputPixelType() const;
    const HFCPtr<HRPPixelType>&
    GetOutputPixelType() const;
    virtual void            SetInputPixelType(
        const HFCPtr<HRPPixelType>& pi_pInputPixelType);
    virtual void            SetOutputPixelType(
        const HFCPtr<HRPPixelType>& pi_pOutputPixelType);

    // Cloning
    virtual HRPFilter* Clone() const = 0;

protected:

    // Primary methods
    HRPFilter();
    HRPFilter(const HRPPixelNeighbourhood& pi_rNeighbourhood);
    HRPFilter(const HRPFilter& pi_rFilter);

    // Operators
    HRPFilter&              operator=(const HRPFilter& pi_rObj);

    // Neighbourhood
    void                    SetNeighbourhood(
        const HRPPixelNeighbourhood& pi_rNeighbourhood);

private:

    HRPPixelNeighbourhood    m_Neighbourhood;

    HFCPtr<HRPPixelType>    m_pInputPixelType;

    HFCPtr<HRPPixelType>    m_pOutputPixelType;
    };
END_IMAGEPP_NAMESPACE

#include "HRPFilter.hpp"

