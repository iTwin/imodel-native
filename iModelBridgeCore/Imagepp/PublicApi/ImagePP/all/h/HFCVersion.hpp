//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCVersion.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCVersion
//-----------------------------------------------------------------------------
// Note: some methods will have multiple exit points for performance reasons.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
inline HFCVersion::HFCVersion()
    {
    m_NumberCount = 0;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HFCVersion::HFCVersion(const WString& pi_rName)
    : m_Name(pi_rName),
      m_Info(L"")
    {
    FREEZE_STL_STRING(m_Name);
    FREEZE_STL_STRING(m_Info);

    // Save the count of numbers
    m_NumberCount = 0;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HFCVersion::HFCVersion(const WString& pi_rName,
                              const WString& pi_rInfo,
                              size_t         pi_NumberCount,
                              ...)
    : m_Name(pi_rName),
      m_Info(pi_rInfo)
    {
    FREEZE_STL_STRING(m_Name);
    FREEZE_STL_STRING(m_Info);

    size_t i;
    int32_t Num;
    va_list NumberList;

    // start the variable argument parsing
    va_start(NumberList, pi_NumberCount);

    // Save the count of numbers
    m_NumberCount = pi_NumberCount;

    // if there is any numbers, extract them from the variable arguments
    if (m_NumberCount > 0)
        {
        m_pNumbers = new uint32_t[m_NumberCount];

        for (i = 0; i < m_NumberCount; i++)
            {
            // get the number
            Num = va_arg(NumberList, int32_t);
            HASSERT(Num >= 0);

            // add it to our array
            m_pNumbers[i] = (uint32_t)Num;
            }
        }

    // stop the variable argument parsing
    va_end(NumberList);
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
inline HFCVersion::HFCVersion(const HFCVersion& pi_rObj)
    {
    *this = pi_rObj;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HFCVersion::~HFCVersion()
    {
    }


//-----------------------------------------------------------------------------
// public
// Copy operator
//-----------------------------------------------------------------------------
inline HFCVersion&
HFCVersion::operator=(const HFCVersion& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Copy the string
        m_Name = pi_rObj.GetName();
        m_Info = pi_rObj.GetInfo();
        FREEZE_STL_STRING(m_Name);
        FREEZE_STL_STRING(m_Info);

        // release the version array
        m_pNumbers = 0;

        // copy the numbers
        m_NumberCount = pi_rObj.m_NumberCount;
        if (m_NumberCount > 0)
            {
            m_pNumbers    = new uint32_t[m_NumberCount];
            memcpy(m_pNumbers, pi_rObj.m_pNumbers, m_NumberCount * sizeof(uint32_t));
            }
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if the numbers are equal.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator==(const HFCVersion& pi_rObj) const
    {
    // The first number that is not equal results in the versions being not
    // equal to each other.
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        if (GetNumber(i) != pi_rObj.GetNumber(i))
            return false;

    // all numbers are the same, so the versions are equal
    return true;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if the numbers are not equal.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator!=(const HFCVersion& pi_rObj) const
    {
    // The first number that is not equal results in the versions being not
    // equal to each other.
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        if (GetNumber(i) != pi_rObj.GetNumber(i))
            return true;

    // all numbers are the same, so the versions are equal
    return false;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if a version is lesser than another version.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator< (const HFCVersion& pi_rObj) const
    {
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        {
        if (GetNumber(i) < pi_rObj.GetNumber(i))
            return true;
        else if (GetNumber(i) > pi_rObj.GetNumber(i))
            return false;
        }

    return false;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if a version is lesser or equal than another version.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator<=(const HFCVersion& pi_rObj) const
    {
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        {
        if (GetNumber(i) < pi_rObj.GetNumber(i))
            return true;
        else if (GetNumber(i) > pi_rObj.GetNumber(i))
            return false;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if a version is greater than another version.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator> (const HFCVersion& pi_rObj) const
    {
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        {
        if (GetNumber(i) < pi_rObj.GetNumber(i))
            return false;
        else if (GetNumber(i) > pi_rObj.GetNumber(i))
            return true;
        }

    return false;
    }


//-----------------------------------------------------------------------------
// public
// Verifies if a version is greater or equal than another version.
//-----------------------------------------------------------------------------
inline bool HFCVersion::operator>=(const HFCVersion& pi_rObj) const
    {
    for (size_t i = 0; i < MAX(m_NumberCount, pi_rObj.m_NumberCount); i++)
        {
        if (GetNumber(i) < pi_rObj.GetNumber(i))
            return false;
        else if (GetNumber(i) > pi_rObj.GetNumber(i))
            return true;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// public
// Returns the name of the version
//-----------------------------------------------------------------------------
inline const WString& HFCVersion::GetName() const
    {
    return m_Name;
    }


//-----------------------------------------------------------------------------
// public
// Returns the information string
//-----------------------------------------------------------------------------
inline const WString& HFCVersion::GetInfo() const
    {
    return m_Info;
    }


//-----------------------------------------------------------------------------
// public
// Returns the number of numbers in the version
//-----------------------------------------------------------------------------
inline size_t HFCVersion::GetNumberCount() const
    {
    return m_NumberCount;
    }


//-----------------------------------------------------------------------------
// public
// Returns the version number.  Anything beyond NumberCount is 0.
//-----------------------------------------------------------------------------
inline uint32_t HFCVersion::GetNumber(size_t pi_Index) const
    {
    if (pi_Index < GetNumberCount())
        return m_pNumbers[pi_Index];
    else
        return 0;
    }
END_IMAGEPP_NAMESPACE