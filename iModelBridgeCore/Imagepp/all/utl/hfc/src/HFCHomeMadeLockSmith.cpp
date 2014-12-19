//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHomeMadeLockSmith.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHomeMadeLockSmith
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCHomeMadeLockSmith.h>
#include <Imagepp/all/h/HFCHomeMadeLock.h>
#include <Imagepp/all/h/HFCHomeMadeKey.h>


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HFCHomeMadeLockSmith::~HFCHomeMadeLockSmith()
    {
    }


//-----------------------------------------------------------------------------
// Create a new key matching a lock
//-----------------------------------------------------------------------------
HFCSecurityKey*  HFCHomeMadeLockSmith::CreateKey(const HFCSecurityLock& pi_rLock) const
    {
    HFCSecurityKey* pKey = 0;

    if (pi_rLock.IsCompatibleWith(HFCHomeMadeLock::CLASS_ID))
        {
        // Lock is of a known type. Give a key for it...
        pKey = new HFCHomeMadeKey(((HFCHomeMadeLock&)pi_rLock).GetKeyRepresentation());
        }

    return pKey;
    }


//-----------------------------------------------------------------------------
// Create a lock for a key
//-----------------------------------------------------------------------------
HFCSecurityLock* HFCHomeMadeLockSmith::CreateLock(const HFCSecurityKey& pi_rKey) const
    {
    // This is impossible with our algorithm. There is no way to back
    // from a key to a lock...
    HASSERT(false);

    return 0;
    }


//-----------------------------------------------------------------------------
// Create lock and key pair
//-----------------------------------------------------------------------------
bool HFCHomeMadeLockSmith::CreateKeyAndLock(HFCSecurityKey** po_ppKey,
                                             HFCSecurityLock** po_ppLock) const
    {
    HASSERT(m_LockRepresentation.size() > 0);
    HASSERT(po_ppKey != 0);
    HASSERT(po_ppLock != 0);

    *po_ppLock = new HFCHomeMadeLock(m_LockRepresentation);
    *po_ppKey  = new HFCHomeMadeKey(((HFCHomeMadeLock*)*po_ppLock)->GetKeyRepresentation());

    return true;
    }


//-----------------------------------------------------------------------------
// Encode the seed into a lock representation
//-----------------------------------------------------------------------------
void HFCHomeMadeLockSmith::CalculateLockRepresentation()
    {
    WString NumberString;
    NumberString.resize(REPRESENTATION_LENGTH);

    int CurrentPosition = 0;
    WString::size_type SeedPosition = 0;

    // Extract the digits only
    for( ; CurrentPosition < REPRESENTATION_LENGTH && SeedPosition < m_Seed.size() ; SeedPosition++)
        {
        if (iswdigit(m_Seed[SeedPosition]))
            NumberString[CurrentPosition++] = m_Seed[SeedPosition];
        }

    if (CurrentPosition == REPRESENTATION_LENGTH)
        {
        // Extract parts from the 14 digit string as:
        // Part 1 | Part 2 | Part 3 | Part 4 | Part5 | Part6 | Part7
        //   xx       xxx      xx      xx        x      xx      xx
        //
        // Add prime numbers so that 0, 1 and 2 are not easy to decypher...
        unsigned int Part1 = BeStringUtilities::Wtoi(NumberString.substr(0, 2).c_str()) + 13;
        unsigned int Part2 = BeStringUtilities::Wtoi(NumberString.substr(2, 3).c_str()) + 7;
        unsigned int Part3 = BeStringUtilities::Wtoi(NumberString.substr(5, 2).c_str()) + 11;
        unsigned int Part4 = BeStringUtilities::Wtoi(NumberString.substr(7, 2).c_str()) + 3;
        unsigned int Part5 = BeStringUtilities::Wtoi(NumberString.substr(9, 1).c_str()) + 5;
        unsigned int Part6 = BeStringUtilities::Wtoi(NumberString.substr(10, 2).c_str()) + 3;
        unsigned int Part7 = BeStringUtilities::Wtoi(NumberString.substr(12, 2).c_str()) + 13;

        // Add some parts together (create cross-links)
        Part2 += Part4;
        Part3 += Part1;
        Part7 += Part3;
        Part6 += Part2;
        Part1 += Part7;
        Part5 += Part4;
        Part4 += Part5;

        // Multiply by prime numbers
        Part1 *= 23;
        Part2 *= 31;
        Part3 *= 59;
        Part4 *= 11;
        Part5 *= 19;
        Part6 *= 47;
        Part7 *= 71;

        // Stay withing digit width.
        Part1 %= 100;
        Part2 %= 1000;
        Part3 %= 100;
        Part4 %= 100;
        Part5 %= 10;
        Part6 %= 100;
        Part7 %= 100;

        // Clear the rep.
        m_LockRepresentation.resize(REPRESENTATION_LENGTH);

        // Transform the integer results into strings, then place them
        // in the rep. string in the following order
        // Part3 | Part6 | Part1 | Part5 | Part7 | Part4 | Part2
        //  xx      xx       xx      x      xx      xx      xxx
        WChar TempPart[4];
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part3);
        m_LockRepresentation.replace(0, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part6);
        m_LockRepresentation.replace(2, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part1);
        m_LockRepresentation.replace(4, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%01d", Part5);
        m_LockRepresentation.replace(6, 1, TempPart, 1);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part7);
        m_LockRepresentation.replace(7, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part4);
        m_LockRepresentation.replace(9, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%03d", Part2);
        m_LockRepresentation.replace(11, 3, TempPart, 3);
        }
    }
