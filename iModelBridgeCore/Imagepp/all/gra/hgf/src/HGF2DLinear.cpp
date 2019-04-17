//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGF2DLinear.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DLinear::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "BEGIN Dumping a HGF2DLinear object" << endl;
    HDUMP0("BEGIN Dumping a HGF2DLinear object\n");

    // Dump the coordinate system

    // Dump the end points
    char    DumString[256];
    sprintf(DumString, "Start Point : %5.15lf , %5.15lf", GetStartPoint().GetX(), GetStartPoint().GetY());
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");
    sprintf(DumString, "End Point   : %5.15lf , %5.15lf", GetEndPoint().GetX(), GetEndPoint().GetY());
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

    po_rOutput << "END (HGF2DLinear)" << endl;
    HDUMP0("END (HGF2DLinear)\n");
#endif
    }
