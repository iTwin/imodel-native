//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFAngle.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// All methods are declared inline and are defined in HGFAngle.hpp

#include <ImageppInternal.h>


// The class declaration must be the last include file.
#include <ImagePP/all/h/HGFAngle.h>
//-----------------------------------------------------------------------------
// ::ConvertDegMinSecToDeg
// Convert an angle in degree, minute second to an angle in decimal degree
// Returns true if the conversion succeeded, otherwise false
//-----------------------------------------------------------------------------
bool ImagePP::ConvertDegMinSecToDeg(Utf8String& pi_rDegMinSec,
                            double& po_rResultigDegreeValue)
    {
    uint16_t ValueComponentInd   = 0;
    Utf8Char   Sep[] = ":";
    Utf8P  pNextToken=NULL;
    Utf8P  pToken = BeStringUtilities::Strtok((Utf8Char*)pi_rDegMinSec.c_str(), Sep, &pNextToken);
    bool   WellFormattedString = false;

    po_rResultigDegreeValue = 0;

    while ((pToken != 0) && (ValueComponentInd < 3))
        {
        switch (ValueComponentInd)
            {
                //Degrees
            case 0 :
                po_rResultigDegreeValue = atoi(pToken);
                WellFormattedString = true;
                break;
                //Minutes
            case 1 :
                po_rResultigDegreeValue += (double)(atoi(pToken) / 60.0);
                break;
                //Seconds
            case 2 :
                po_rResultigDegreeValue += (double)(atof(pToken) / 3600.0);
                break;
            default :
                HASSERT(0); //Should not happen
                break;
            }
        pToken = BeStringUtilities::Strtok(0, Sep, &pNextToken);
        ValueComponentInd++;
        }

    //There should be no more than 3 components.
    if (pToken != 0)
        {
        WellFormattedString = false;
        }

    return WellFormattedString;
    }
