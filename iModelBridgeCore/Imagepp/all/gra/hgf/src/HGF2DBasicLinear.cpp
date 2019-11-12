//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGF2DBasicLinear.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>



//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DBasicLinear::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HDUMP0("Object is a HGF2DBasicLinear\n");
    po_rOutput << "Object is a HGF2DBasicLinear" << endl;

    HGF2DLinear::PrintState(po_rOutput);

#endif
    }
