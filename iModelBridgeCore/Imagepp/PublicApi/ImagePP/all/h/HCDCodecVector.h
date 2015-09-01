//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecVector.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecVector
//-----------------------------------------------------------------------------
// General class for CodecVectors.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodec.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecVector : public HCDCodec
    {
    HDECLARE_CLASS_ID(HCDCodecId_Vector, HCDCodec)

public:

    // destructor

    virtual         ~HCDCodecVector();


    // subset

    virtual size_t  GetSubsetMaxCompressedSize() const;

    virtual size_t  GetMinimumSubsetSize() const;
    virtual void    SetSubsetSize(size_t pi_Size);

    size_t          GetSubsetPos() const;

    size_t          GetSubsetSize() const;

    void            SetSubsetPos(size_t pi_Pos);

    virtual HCDCodec* Clone() const override = 0;

    // settings

    virtual size_t  GetDataSize() const;

    virtual    void    SetDataSize(size_t pi_DataSize);


    // others

    bool            IsCodecVector() const;

    virtual void    Reset();


protected:

    // constructors

    HCDCodecVector(const WString&   pi_rCodecName);

    HCDCodecVector(const WString&   pi_rCodecName,
                   size_t           pi_DataSize);

    HCDCodecVector(const HCDCodecVector& pi_rObj);


private:

    size_t          m_DataSize;

    size_t          m_SubsetSize;

    size_t          m_SubsetPos;
    };

END_IMAGEPP_NAMESPACE

#include "HCDCodecVector.hpp"

