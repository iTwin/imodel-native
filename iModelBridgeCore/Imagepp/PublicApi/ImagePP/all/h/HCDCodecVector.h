//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

    size_t  GetSubsetMaxCompressedSize() const override;

    size_t  GetMinimumSubsetSize() const override;
    void    SetSubsetSize(size_t pi_Size) override;

    size_t          GetSubsetPos() const;

    size_t          GetSubsetSize() const;

    void            SetSubsetPos(size_t pi_Pos);

    virtual HCDCodec* Clone() const override = 0;

    // settings

    size_t  GetDataSize() const override;

    virtual    void    SetDataSize(size_t pi_DataSize);


    // others

    bool            IsCodecVector() const override;

    void    Reset() override;


protected:

    // constructors

    HCDCodecVector(const Utf8String&   pi_rCodecName);

    HCDCodecVector(const Utf8String&   pi_rCodecName,
                   size_t           pi_DataSize);

    HCDCodecVector(const HCDCodecVector& pi_rObj);


private:

    size_t          m_DataSize;

    size_t          m_SubsetSize;

    size_t          m_SubsetPos;
    };

END_IMAGEPP_NAMESPACE

#include "HCDCodecVector.hpp"

