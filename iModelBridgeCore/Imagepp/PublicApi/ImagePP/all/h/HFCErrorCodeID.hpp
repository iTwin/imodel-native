//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeID.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public
// Default constructor
//------------------------------------------------------------------------------
inline HFCErrorCodeID::HFCErrorCodeID()
    {
    m_Value = 0;
    }


//------------------------------------------------------------------------------
// Public
// Value constructor
//------------------------------------------------------------------------------
inline HFCErrorCodeID::HFCErrorCodeID(uint32_t pi_Value)
    {
    m_Value = pi_Value;
    }


//-----------------------------------------------------------------------------
// Public
// String constructor
//-----------------------------------------------------------------------------
inline HFCErrorCodeID::HFCErrorCodeID(const WString& pi_rString)
    {
    HPRECONDITION(!pi_rString.empty());
    HDEBUGCODE(int IsDigit = 1);
    HDEBUGCODE(for (WString::size_type Pos = 0; (IsDigit) && (Pos < pi_rString.size()); Pos++))
        HDEBUGCODE(    IsDigit = iswxdigit(pi_rString[Pos]);)
        HPRECONDITION(IsDigit);

//chck UNICODE
#   define tstringstream wistringstream

    tstringstream InputStream(pi_rString.c_str());
    hex(InputStream);
    InputStream >> m_Value;
    }


//------------------------------------------------------------------------------
// Public
// Copy constructor
//------------------------------------------------------------------------------
inline HFCErrorCodeID::HFCErrorCodeID(const HFCErrorCodeID& pi_rObj)
    : m_Value(pi_rObj.m_Value)
    {
    }


//------------------------------------------------------------------------------
// Public
// No destructor for performance reason
//------------------------------------------------------------------------------
//inline HFCErrorCodeID::~HFCErrorCodeID()
//{
//}


//------------------------------------------------------------------------------
// Public
// Assignement operator
//------------------------------------------------------------------------------
inline HFCErrorCodeID& HFCErrorCodeID::operator=(const HFCErrorCodeID& pi_rObj)
    {
    if (this != &pi_rObj)
        m_Value = pi_rObj;

    return (*this);
    }


//------------------------------------------------------------------------------
// Public
// Cast operator
//------------------------------------------------------------------------------
inline HFCErrorCodeID::operator uint32_t()
    {
    return (m_Value);
    }


//------------------------------------------------------------------------------
// Public
// Cast operator
//------------------------------------------------------------------------------
inline HFCErrorCodeID::operator uint32_t() const
    {
    return (m_Value);
    }


//------------------------------------------------------------------------------
// Public
// Ouputs the value to a stream
//------------------------------------------------------------------------------
//chck UNICODE
inline void HFCErrorCodeID::GenerateString(wostringstream&  po_rStream,
                                           uint32_t         pi_DigitCount) const
    {
    // Verify that the value fits in the given number of hexadecimal digits
    HDEBUGCODE(uint32_t Temp = 1);
    HDEBUGCODE(for (uint32_t iii = 0; iii < pi_DigitCount; iii++) Temp = Temp << 4;);
    HPRECONDITION(m_Value < Temp);

    // setup the stream for an hexadecimal value of pi_DigitCount digits
    po_rStream.fill(L'0');
    po_rStream.width(pi_DigitCount);
    po_rStream.precision(pi_DigitCount);
    uppercase(po_rStream);
    hex(po_rStream);

    po_rStream << m_Value;
    }
