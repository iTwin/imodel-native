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


#include <ImagePP/all/h/HVE2DBasicLinear.h>

HPM_REGISTER_ABSTRACT_CLASS(HVE2DBasicLinear, HVE2DLinear)


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DBasicLinear::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HDUMP0("Object is a HVE2DBasicLinear\n");
    po_rOutput << "Object is a HVE2DBasicLinear" << endl;

    HVE2DLinear::PrintState(po_rOutput);

#endif
    }
