//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPChannelType.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPChannelType
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPChannelType.h>



//-----------------------------------------------------------------------------
// Default Constructor.
//-----------------------------------------------------------------------------
HRPChannelType::HRPChannelType()
    {
    m_Role     = HRPChannelType::UNUSED;
    m_DataType = HRPChannelType::VOID_CH;
    m_SizeBits = 0;
    m_Id       = 0;
    }


//-----------------------------------------------------------------------------
// The constructor.  It takes channel info parameters.
//-----------------------------------------------------------------------------
HRPChannelType::HRPChannelType(HRPChannelType::ChannelRole pi_Role,
                               HRPChannelType::DataType    pi_DataType,
                               unsigned short             pi_SizeBits,
                               unsigned short             pi_Id,
                               const double*             pi_pNoDataValue)
    {
    m_Role     = pi_Role;
    m_DataType = pi_DataType;
    m_SizeBits = pi_SizeBits;
    m_Id       = pi_Id;

    if (pi_pNoDataValue != 0)
        {
        m_pNoDataValue = new double;
        *m_pNoDataValue = *pi_pNoDataValue;
        }
    else
        {
        m_pNoDataValue = 0;
        }
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRPChannelType::HRPChannelType(const HRPChannelType& pi_rObj)
    {
    m_Role     = pi_rObj.m_Role;
    m_DataType = pi_rObj.m_DataType;
    m_SizeBits = pi_rObj.m_SizeBits;
    m_Id       = pi_rObj.m_Id;

    if (pi_rObj.m_pNoDataValue != 0)
        {
        if (m_pNoDataValue == 0)
            {
            m_pNoDataValue = new double;
            }
        *m_pNoDataValue = *pi_rObj.m_pNoDataValue;
        }
    else
        {
        m_pNoDataValue = 0;
        }
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HRPChannelType::~HRPChannelType()
    {
    // Nothing to do
    }

//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HRPChannelType& HRPChannelType::operator=(const HRPChannelType& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Role     = pi_rObj.m_Role;
        m_DataType = pi_rObj.m_DataType;
        m_SizeBits = pi_rObj.m_SizeBits;
        m_Id       = pi_rObj.m_Id;

        if (pi_rObj.m_pNoDataValue != 0)
            {
            if (m_pNoDataValue == 0)
                {
                m_pNoDataValue = new double;
                }
            *m_pNoDataValue = *pi_rObj.m_pNoDataValue;
            }
        else
            {
            m_pNoDataValue = 0;
            }
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Returns the Id of this channel.
//-----------------------------------------------------------------------------
unsigned short HRPChannelType::GetId() const
    {
    return m_Id;
    }

//-----------------------------------------------------------------------------
// Returns the type of this channel.
//-----------------------------------------------------------------------------
HRPChannelType::ChannelRole HRPChannelType::GetRole() const
    {
    return m_Role;
    }

//-----------------------------------------------------------------------------
// Returns the data type of this channel.
//-----------------------------------------------------------------------------
HRPChannelType::DataType HRPChannelType::GetDataType() const
    {
    return m_DataType;
    }

//-----------------------------------------------------------------------------
// Returns the size, in bits, of data for this channel.
//-----------------------------------------------------------------------------
unsigned short HRPChannelType::GetSize() const
    {
    return m_SizeBits;
    }

//-----------------------------------------------------------------------------
// Returns true only if both channel types are equal
//-----------------------------------------------------------------------------
bool HRPChannelType::operator==(const HRPChannelType& pi_rObj) const
    {
    return(m_Role     == pi_rObj.m_Role &&
        m_DataType == pi_rObj.m_DataType &&
        m_SizeBits == pi_rObj.m_SizeBits &&
        m_Id       == pi_rObj.m_Id);
    }

//-----------------------------------------------------------------------------
// Returns true only if both channel types are not equal
//-----------------------------------------------------------------------------
bool HRPChannelType::operator!=(const HRPChannelType& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }

//-----------------------------------------------------------------------------
// Returns true only if both channel types are not equal
//-----------------------------------------------------------------------------
const double* HRPChannelType::GetNoDataValue() const
    {
    return m_pNoDataValue.get();
    }

//-----------------------------------------------------------------------------

// Returns true only if both channel types are not equal

//-----------------------------------------------------------------------------

void HRPChannelType::SetNoDataValue (double noDataValue)

    {

    m_pNoDataValue = new double(noDataValue);

    }