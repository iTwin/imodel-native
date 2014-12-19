//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeFlags.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::HFCErrorCodeFlags()
    {
    m_Flags     = 0;
    }


//-----------------------------------------------------------------------------
// Public
// Value constructor
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::HFCErrorCodeFlags(uint8_t pi_Value)
    {
    HPRECONDITION(pi_Value <= 0xF);

    m_Flags = pi_Value;
    }


//-----------------------------------------------------------------------------
// Public
// String constructor
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::HFCErrorCodeFlags(const WString& pi_rString)
    {
    HPRECONDITION(pi_rString.size() == 1);
    HPRECONDITION(iswxdigit(pi_rString[0]));

    wistringstream InputStream(pi_rString.c_str());
    hex(InputStream);
    int32_t Temp;
    InputStream >> Temp;
    m_Flags = (uint8_t)Temp;
    }


//-----------------------------------------------------------------------------
// Public
// Copy constructor
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::HFCErrorCodeFlags(const HFCErrorCodeFlags& pi_rObj)
    : m_Flags(pi_rObj.m_Flags)
    {
    }


//-----------------------------------------------------------------------------
// Public
// No destructor for performance reason
//-----------------------------------------------------------------------------
//inline HFCErrorCodeFlags::~HFCErrorCodeFlags()
//{
//}


//-----------------------------------------------------------------------------
// Public
// Assignement operator
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags& HFCErrorCodeFlags::operator=(const HFCErrorCodeFlags& pi_rObj)
    {
    if (this != &pi_rObj)
        m_Flags = pi_rObj.m_Flags;

    return (*this);
    }


//-----------------------------------------------------------------------------
// Public
// Cast operator
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::operator uint8_t()
    {
    return (m_Flags);
    }


//-----------------------------------------------------------------------------
// Public
// Cast operator
//-----------------------------------------------------------------------------
inline HFCErrorCodeFlags::operator uint8_t() const
    {
    return (m_Flags);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if the fatal flag is on
//-----------------------------------------------------------------------------
inline bool HFCErrorCodeFlags::IsFatal() const
    {
    return (m_Flags & 0x01);
    }


//-----------------------------------------------------------------------------
// Public
// Ouputs the value to a stream
//-----------------------------------------------------------------------------
//chck UNICODE
inline void HFCErrorCodeFlags::GenerateString(wostringstream& po_rStream) const
    {
    // Verify that the value fits in the given number of hexadecimal digits
    HPRECONDITION(m_Flags <= 0xF);

    // setup the stream for an hexadecimal value of 1 digit
    po_rStream.fill('0');
    po_rStream.width(1);
    po_rStream.precision(1);
    uppercase(po_rStream);
    hex(po_rStream);

    int32_t Temp = m_Flags;
    po_rStream << Temp;
    }
