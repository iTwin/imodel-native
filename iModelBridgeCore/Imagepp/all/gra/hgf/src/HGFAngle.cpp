//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFAngle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// All methods are declared inline and are defined in HGFAngle.hpp

#include <ImagePPInternal/hstdcpp.h>


// The class declaration must be the last include file.
#include <Imagepp/all/h/HGFAngle.h>
//-----------------------------------------------------------------------------
// ::ConvertDegMinSecToDeg
// Convert an angle in degree, minute second to an angle in decimal degree
// Returns true if the conversion succeeded, otherwise false
//-----------------------------------------------------------------------------
bool ImagePP::ConvertDegMinSecToDeg(WString& pi_rDegMinSec,
                            double& po_rResultigDegreeValue)
    {
    unsigned short ValueComponentInd   = 0;
    WChar   Sep[] = L":";
    WCharP  pNextToken=NULL;
    WCharP  pToken = BeStringUtilities::Wcstok((WChar*)pi_rDegMinSec.c_str(), Sep, &pNextToken);
    bool   WellFormattedString = false;

    po_rResultigDegreeValue = 0;

    while ((pToken != 0) && (ValueComponentInd < 3))
        {
        switch (ValueComponentInd)
            {
                //Degrees
            case 0 :
                po_rResultigDegreeValue = BeStringUtilities::Wtoi(pToken);
                WellFormattedString = true;
                break;
                //Minutes
            case 1 :
                po_rResultigDegreeValue += (double)(BeStringUtilities::Wtoi(pToken) / 60.0);
                break;
                //Seconds
            case 2 :
                po_rResultigDegreeValue += (double)(BeStringUtilities::Wtof(pToken) / 3600.0);
                break;
            default :
                HASSERT(0); //Should not happen
                break;
            }
        pToken = BeStringUtilities::Wcstok(0, Sep, &pNextToken);
        ValueComponentInd++;
        }

    //There should be no more than 3 components.
    if (pToken != 0)
        {
        WellFormattedString = false;
        }

    return WellFormattedString;
    }