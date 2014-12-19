//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHomeMadeLock.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHomeMadeLock
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCHomeMadeLock.h>
#include <Imagepp/all/h/HFCHomeMadeKey.h>


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HFCHomeMadeLock::~HFCHomeMadeLock()
    {
    }


//-----------------------------------------------------------------------------
// Unlock us
//-----------------------------------------------------------------------------
bool HFCHomeMadeLock::UnlockWith(const HFCSecurityKey& pi_rKey)
    {
    bool Result = pi_rKey.IsCompatibleWith(HFCHomeMadeKey::CLASS_ID);

    if (Result)
        {
        Result = CanBeUnlockedBy((HFCHomeMadeKey&)pi_rKey);

        if (Result)
            m_Locked = false;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Lock us
//-----------------------------------------------------------------------------
bool HFCHomeMadeLock::LockWith(const HFCSecurityKey& pi_rKey)
    {
    bool Result = pi_rKey.IsCompatibleWith(HFCHomeMadeKey::CLASS_ID);

    if (Result)
        {
        Result = CanBeUnlockedBy((HFCHomeMadeKey&)pi_rKey);

        if (Result)
            m_Locked = true;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Test is the provided key matches
//-----------------------------------------------------------------------------
bool HFCHomeMadeLock::CanBeUnlockedBy(HFCHomeMadeKey& pi_rKey) const
    {
    // True if keys are equal
    return (GetKeyRepresentation().compare(pi_rKey.GetRepresentation()) == 0);
    }


//-----------------------------------------------------------------------------
// Return representation of key that can unlock us
//-----------------------------------------------------------------------------
WString HFCHomeMadeLock::GetKeyRepresentation() const
    {
    WString Representation;

    WString NumberString;
    NumberString.resize(REPRESENTATION_LENGTH);

    int CurrentPosition = 0;
    WString::size_type RepPosition = 0;

    // Extract the digits only
    for( ; CurrentPosition < REPRESENTATION_LENGTH && RepPosition < m_Representation.size() ; RepPosition++)
        {
        if (iswdigit(m_Representation[RepPosition]))
            NumberString[CurrentPosition++] = m_Representation[RepPosition];
        }

    if (CurrentPosition == REPRESENTATION_LENGTH)
        {
        Representation.resize(REPRESENTATION_LENGTH);

        // Extract three parts from the 10 digit string as:
        // Part 1 | Part 2 | Part 3 | Part4 | Part5 | Part6 | Part7 | Part8
        //    x       xx      xx        x      xx      xx      xxx      x
        //
        // Add prime numbers so that 0, 1 and 2 are not easy to decypher...
        unsigned int Part1 = BeStringUtilities::Wtoi(NumberString.substr(0, 1).c_str()) + 3;
        unsigned int Part2 = BeStringUtilities::Wtoi(NumberString.substr(1, 2).c_str()) + 7;
        unsigned int Part3 = BeStringUtilities::Wtoi(NumberString.substr(3, 2).c_str()) + 19;
        unsigned int Part4 = BeStringUtilities::Wtoi(NumberString.substr(5, 1).c_str()) + 5;
        unsigned int Part5 = BeStringUtilities::Wtoi(NumberString.substr(6, 2).c_str()) + 11;
        unsigned int Part6 = BeStringUtilities::Wtoi(NumberString.substr(8, 2).c_str()) + 31;
        unsigned int Part7 = BeStringUtilities::Wtoi(NumberString.substr(10, 3).c_str()) + 5;
        unsigned int Part8 = BeStringUtilities::Wtoi(NumberString.substr(13, 1).c_str()) + 3;

        // Add some parts together (create cross-links)
        Part1 += Part4;
        Part6 += Part3;
        Part3 += Part8;
        Part2 += Part1;
        Part8 += Part7;
        Part4 += Part6;
        Part5 += Part4;
        Part7 += Part2;

        // Multiply by prime numbers
        Part1 *= 23;
        Part2 *= 31;
        Part3 *= 59;
        Part4 *= 11;
        Part5 *= 19;
        Part6 *= 17;
        Part7 *= 83;
        Part8 *= 61;

        // Stay withing digit width.
        Part1 %= 10;
        Part2 %= 100;
        Part3 %= 100;
        Part4 %= 10;
        Part5 %= 100;
        Part6 %= 100;
        Part7 %= 1000;
        Part8 %= 10;

        // Transform the integer results into strings, then place them
        // in the rep. string in the following order
        // Part3 | Part2 | Part8 | Part1 | Part5 | Part7 | Part4 | Part6
        //  xx      xx       x       x       xx     xxx      x      xx
        WChar TempPart[4];
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part3);
        Representation.replace(0, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part2);
        Representation.replace(2, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%01d", Part8);
        Representation.replace(4, 1, TempPart, 1);
        BeStringUtilities::Snwprintf(TempPart, L"%01d", Part1);
        Representation.replace(5, 1, TempPart, 1);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part5);
        Representation.replace(6, 2, TempPart, 2);
        BeStringUtilities::Snwprintf(TempPart, L"%03d", Part7);
        Representation.replace(8, 3, TempPart, 3);
        BeStringUtilities::Snwprintf(TempPart, L"%01d", Part4);
        Representation.replace(11, 1, TempPart, 1);
        BeStringUtilities::Snwprintf(TempPart, L"%02d", Part6);
        Representation.replace(12, 2, TempPart, 2);
        }

    return Representation;
    }

