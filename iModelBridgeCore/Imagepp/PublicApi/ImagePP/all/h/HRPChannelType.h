//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelType.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPChannelType
//-----------------------------------------------------------------------------
// Channel type definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"


#define HRPCHANNELTYPE_NB_CHANNEL_ROLES (23)


class HRPChannelType : public HFCShareableObject<HRPChannelType>
    {
    HDECLARE_SEALEDCLASS_ID(1008)

public:

    // A ChannelType identifies the role of a channel
    typedef enum
        {
        UNUSED = 0,
        RED, GREEN, BLUE,
        GRAY,
        ALPHA,
        CYAN, YELLOW, MAGENTA, BLACK,
        ELEVATION,
        USER,
        PREMULTIPLIED_RED, PREMULTIPLIED_GREEN, PREMULTIPLIED_BLUE,
        Y, CHROMA1, CHROMA2,
        PREMULTIPLIED_Y, PREMULTIPLIED_CHROMA1, PREMULTIPLIED_CHROMA2,
        PREMULTIPLIED_GRAY,
        GRAYWHITE,
        FREE = 255,
        } ChannelRole;

    // A DataType identifies the type of data stored in a channel
    typedef enum
        {
        VOID_CH,
        BOOLEAN_CH,
        INT_CH,    //Unsigned integer
        SINT_CH,   //Signed integer
        FLOAT_CH
        } DataType;

    // Primary methods

    HRPChannelType();
    _HDLLg                 HRPChannelType(ChannelRole     pi_Role,
                                          DataType        pi_DataType,
                                          unsigned short pi_SizeBits,
                                          unsigned short pi_Id,
                                          const double* pi_pNoDataValue = 0);

    _HDLLg                 HRPChannelType(const HRPChannelType& pi_rObj);

    _HDLLg                ~HRPChannelType();

    _HDLLg const double*   GetNoDataValue() const;

    HRPChannelType& operator=(const HRPChannelType& pi_rObj);

    bool           operator==(const HRPChannelType& pi_rObj) const;
    bool           operator!=(const HRPChannelType& pi_rObj) const;

    // Query methods

    _HDLLg ChannelRole      GetRole() const;
    _HDLLg DataType         GetDataType() const;
    _HDLLg unsigned short   GetSize() const;
    _HDLLg unsigned short   GetId() const;

private:

    ChannelRole        m_Role;
    DataType           m_DataType;
    unsigned short    m_SizeBits;
    unsigned short    m_Id;
    HAutoPtr<double> m_pNoDataValue;
    };

