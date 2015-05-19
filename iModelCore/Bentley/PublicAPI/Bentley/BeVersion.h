/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/BeVersion.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>

BEGIN_BENTLEY_NAMESPACE

#define VERSION_FORMAT "%d.%d.%d.%d"

typedef struct BeVersion& BeVersionR;
typedef const struct BeVersion& BeVersionCR;

//=======================================================================================
//! A 4-digit number that specifies version number
// @bsiclass                                            Benediktas.Lipnickas    08/2013
//=======================================================================================
struct BeVersion
{
private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_sub1;
    uint16_t m_sub2;

public:
    enum Mask : uint64_t
        {
        VERSION_Major      = (((uint64_t) 0xffff) << 48),
        VERSION_Minor      = (((uint64_t) 0xffff) << 32),
        VERSION_Sub1       = (((uint64_t) 0xffff) << 16),
        VERSION_Sub2       = ((uint64_t) 0xffff),
        VERSION_MajorMinor = (VERSION_Major | VERSION_Minor),
        VERSION_MajorMinorSub1 = (VERSION_Major | VERSION_Minor | VERSION_Sub1),
        VERSION_All        = (VERSION_Major | VERSION_Minor | VERSION_Sub1 | VERSION_Sub2),
        };

    BeVersion () : BeVersion (0, 0, 0, 0) { }
    BeVersion (uint16_t major, uint16_t minor) : BeVersion (major, minor, 0, 0) { }
    BeVersion (uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : m_major (major), m_minor (minor), m_sub1 (sub1), m_sub2 (sub2) { }
    BeVersion (int major, int minor) : BeVersion ((uint16_t) major, (uint16_t) minor) { }
    BeVersion (Utf8CP versionStr, Utf8CP format = VERSION_FORMAT) : BeVersion () { FromString (versionStr, format); }

    //! Get the Major version. Typically indicates a software "generation".
    //! When this number changes, older versions will no longer open the database.
    uint16_t GetMajor() const {return m_major;}

    //! Get the Minor version. Typically indicates a software release cycle.
    //! When this number changes, older versions will no longer open the database.
    uint16_t GetMinor() const {return m_minor;}

    //! Get the Sub1 version. Typically means small backwards compatible changes to the schema within a release cycle.
    //! When this number changes, older versions will open the database readonly.
    uint16_t GetSub1() const {return m_sub1;}

    //! Get the Sub2 version. Changes to this number mean new features were added, but older versions may continue to edit
    uint16_t GetSub2() const {return m_sub2;}

    uint64_t GetInt64 (Mask mask) const { return mask & ((((uint64_t) m_major)<<48) | (((uint64_t)m_minor)<<32) | ((uint64_t)m_sub1<<16) | (uint64_t)m_sub2); }
    int CompareTo (BeVersionCR other, Mask mask=VERSION_All) const { return (GetInt64 (mask)==other.GetInt64 (mask)) ? 0 : (GetInt64 (mask) > other.GetInt64 (mask) ? 1 : -1); }

    bool operator == (BeVersionCR rhs) const {return CompareTo (rhs) == 0;}
    bool operator != (BeVersionCR rhs) const {return CompareTo (rhs) != 0;}
    bool operator < (BeVersionCR rhs) const  {return CompareTo (rhs) < 0;}
    bool operator <= (BeVersionCR rhs) const {return CompareTo (rhs) <= 0;}
    bool operator > (BeVersionCR rhs) const  {return CompareTo (rhs) > 0;}
    bool operator >= (BeVersionCR rhs) const {return CompareTo (rhs) >= 0;}
    
    bool IsEmpty() const {return 0 == GetInt64(VERSION_All);}

    Utf8String ToString (Utf8CP format = VERSION_FORMAT) const
        {
        return Utf8PrintfString (format, m_major, m_minor, m_sub1, m_sub2);
        }

    void FromString (Utf8CP versionStr, Utf8CP format = VERSION_FORMAT) 
        {
        int major = 0, minor = 0, sub1 = 0, sub2 = 0;
        BE_STRING_UTILITIES_UTF8_SSCANF (versionStr, format, &major, &minor, &sub1, &sub2);
        m_major = (uint16_t) (0xFFFF & major);
        m_minor = (uint16_t) (0xFFFF & minor);
        m_sub1  = (uint16_t) (0xFFFF & sub1);
        m_sub2  = (uint16_t) (0xFFFF & sub2);
        }
};

END_BENTLEY_NAMESPACE