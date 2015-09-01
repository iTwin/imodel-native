//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelType.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPChannelType
//-----------------------------------------------------------------------------
// Channel type definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"


BEGIN_IMAGEPP_NAMESPACE
#define HRPCHANNELTYPE_NB_CHANNEL_ROLES (23)


class HRPChannelType : public HFCShareableObject<HRPChannelType>
    {
    HDECLARE_SEALEDCLASS_ID(HRPChannelTypeId_Base)

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
    IMAGEPP_EXPORT                 HRPChannelType(ChannelRole     pi_Role,
                                          DataType        pi_DataType,
                                          unsigned short pi_SizeBits,
                                          unsigned short pi_Id,
                                          const double* pi_pNoDataValue = 0);

    IMAGEPP_EXPORT                 HRPChannelType(const HRPChannelType& pi_rObj);

    IMAGEPP_EXPORT                ~HRPChannelType();

    IMAGEPP_EXPORT const double*   GetNoDataValue() const;

    IMAGEPP_EXPORT void            SetNoDataValue (double noDataValue);

    HRPChannelType& operator=(const HRPChannelType& pi_rObj);

    bool           operator==(const HRPChannelType& pi_rObj) const;
    bool           operator!=(const HRPChannelType& pi_rObj) const;

    // Query methods

    IMAGEPP_EXPORT ChannelRole      GetRole() const;
    IMAGEPP_EXPORT DataType         GetDataType() const;
    IMAGEPP_EXPORT unsigned short   GetSize() const;
    IMAGEPP_EXPORT unsigned short   GetId() const;

private:

    ChannelRole        m_Role;
    DataType           m_DataType;
    unsigned short     m_SizeBits;
    unsigned short     m_Id;
    HAutoPtr<double>   m_pNoDataValue;
    };
END_IMAGEPP_NAMESPACE

