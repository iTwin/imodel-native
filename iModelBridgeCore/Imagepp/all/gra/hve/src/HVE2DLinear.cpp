//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DLinear.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DLinear.h>

HPM_REGISTER_ABSTRACT_CLASS(HVE2DLinear, HVE2DVector)


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Sets the coordinate system of the linear without changing coordinate values
//-----------------------------------------------------------------------------
void HVE2DLinear::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpNewCoordSys)
    {
    // Change the ancester coord sys
    HVE2DVector::SetCoordSysImplementation(pi_rpNewCoordSys);

    // Change the coord system used by start and end point
    m_StartPoint.SetCoordSys(GetCoordSys());
    m_EndPoint.SetCoordSys(GetCoordSys());
    }



//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DLinear::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "BEGIN Dumping a HVE2DLinear object" << endl;
    HDUMP0("BEGIN Dumping a HVE2DLinear object\n");

    // Dump the coordinate system

    // Dump the end points
    char    DumString[256];
    sprintf(DumString, "Start Point : %5.15lf , %5.15lf", m_StartPoint.GetX(), m_StartPoint.GetY());
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");
    sprintf(DumString, "End Point   : %5.15lf , %5.15lf", m_EndPoint.GetX(), m_EndPoint.GetY());
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

    po_rOutput << "END (HVE2DLinear)" << endl;
    HDUMP0("END (HVE2DLinear)\n");
#endif
    }
