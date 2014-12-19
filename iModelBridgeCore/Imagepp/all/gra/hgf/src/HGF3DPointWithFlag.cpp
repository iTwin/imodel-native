//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF3DPointWithFlag.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HGF3DPointWithFlag
//-----------------------------------------------------------------------------

#include "hstdcpp.h"

#include "HGF3DPointWithFlag.h"
#include <strstream>
//-----------------------------------------------------------------------------
//    GLOBAL FUNCTIONS OPERATING ON OBJECT
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    @author Alain Robert

    This static method pumps the DTM points (Breaklines) from the stream

    @param pi_InStream The input stream from which is loaded the DTM points (breaklines)

    @param pi_rPoints The DTM point collection to insert loaded points into

    @return Reference to input stream to be used afterwards.
    -----------------------------------------------------------------------------
*/
istream& operator>>(istream& pi_InStream, HGF3DPointWithFlagCollection& pio_rPoints)
    {
    // Declare input buffer
    char MyInLine[1024];
    double X;
    double Y;
    double Z;
    int UserFlag;


    // While the end of file is not reached
    while (pi_InStream.good())
        {
        // Input the line from stream
        pi_InStream.getline(MyInLine, 1024);

        // Check if it is a comment
        if (MyInLine[0] != '#' && strlen(MyInLine) > 5)
            {
            // It is not a comment ... load data
            istrstream MyLineStream(MyInLine);

            // Input coordinates
            MyLineStream >> X;
            MyLineStream >> Y;
            MyLineStream >> Z;
            MyLineStream >> UserFlag;

            HGF3DPointWithFlag MyPoint(X, Y, Z, UserFlag);

            pio_rPoints.push_back(MyPoint);
            }
        }

    return(pi_InStream);
    }


/** -----------------------------------------------------------------------------
    @author Alain Robert

    This static method pushes the DTM points (Breaklines) to the stream

    @param po_OutStream The output stream to which to write DTM points (breaklines)

    @param pi_rPoints The DTM points to write to stream

    @return Reference to output stream to be used afterwards.
    -----------------------------------------------------------------------------
*/
ostream& operator<<(ostream& pi_OutStream, const HGF3DPointWithFlagCollection& pi_rPoints)
    {

    // Write points
    HGF3DPointWithFlagCollection::const_iterator Itr;

    for (Itr = pi_rPoints.begin() ; Itr != pi_rPoints.end() ; ++Itr)
        {
        pi_OutStream << Itr->GetX() << " "
                     << Itr->GetY() << " "
                     << Itr->GetZ() << " "
                     << Itr->GetUserFlag() << endl;
        }

    return(pi_OutStream);
    }




//-----------------------------------------------------------------------------
//    HGF3DPointWithFlagCollection
//-----------------------------------------------------------------------------



/** -----------------------------------------------------------------------------
    @author Alain Robert

    This method returns the extent of the collection

    @return A lite extent containing the 2D extent of collection
    -----------------------------------------------------------------------------
*/
HGF2DLiteExtent HGF3DPointWithFlagCollection::Get2DLiteExtent() const
    {
    HGF2DLiteExtent MyExtent;

    HGF3DPointWithFlagCollection::const_iterator Itr = begin();

    // Check that there is at least one point
    if (Itr != end())
        {
        double XMin = Itr->GetX();
        double XMax = Itr->GetX();
        double YMin = Itr->GetY();
        double YMax = Itr->GetY();

        // For every other points ....
        for (; Itr!= end() ; ++Itr)
            {
            // Reset extremes
            XMin = min(XMin, Itr->GetX());
            XMax = max(XMax, Itr->GetX());
            YMin = min(YMin, Itr->GetY());
            YMax = max(YMax, Itr->GetY());
            }

        MyExtent = HGF2DLiteExtent(XMin, YMin, XMax, YMax);
        }

    return(MyExtent);
    }


/** -----------------------------------------------------------------------------
    @author Alain Robert

    This method changes the interpretation coordinate system for the DTM points
    @param pi_rpCoordSys The new interpretation coordinate system. The values
                         of the points are converted to this coordinate system
    -----------------------------------------------------------------------------
*/
void HGF3DPointWithFlagCollection::ChangeCoordSys(const HFCPtr<HGF2DCoordSys> pi_rpCoordSys)
    {
    // Obtain the transformation model
    HFCPtr<HGF2DTransfoModel> pModel = m_pCoordSys->GetTransfoModelTo(pi_rpCoordSys);

    // Convert all points
    HGF3DPointWithFlagCollection::iterator Itr;

    for (Itr = begin(); Itr!= end() ; ++Itr)
        {
        m_pCoordSys->ConvertTo(pi_rpCoordSys, &((*Itr)[HGF3DPointWithFlag::X]), &((*Itr)[HGF3DPointWithFlag::Y]));
        }
    }

//-----------------------------------------------------------------------------
//    HGF3DPointWithFlag
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF3DPointWithFlag::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char       DumString[256];
    sprintf(DumString, "Object is a HGF3DPointWithFlag");
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
    sprintf(DumString, "User flag is: %ld", m_UserFlag);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;

    HGF3DCoord<double>::PrintState(po_rOutput);
#endif
    }











