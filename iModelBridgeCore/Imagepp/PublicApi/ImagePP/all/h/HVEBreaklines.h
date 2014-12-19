//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEBreaklines.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEBreakLines
//-----------------------------------------------------------------------------

#pragma once

#include "HGF3DPoint.h"
#include "HGF2DCoordSys.h"
#include "HVE2DPolySegment.h"
#include "HVE3DPolyLine.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class is used to represent a breakline defect. Such a defect contains a
    defect code and a location where the defect is located.

    The valid codes are:
    0x00000000 - No defect

    0x00002000 - No point nor breakline defined

    0x00000001 - A breakline crosses itself
    0x00000002 - A vertex does not have a minimal length
    0x00000004 - A breakline has two consecutive identical points
    0x00000008 - A breakline does not have at least two non-dentical points
    0x00000010 - A vertex has too long a length
    0x00000020 - The complete length of a breakline exceeds maximum length
    0x00000040 - Two breaklines cross each other.
    0x00000080 - A breakline starts on another breakline inside a vertex (not at a point)
    0x00000100 - Two breaklines are contiguous
    0x00000200 - Two breaklines flirt other than at a vertex point
    0x00000400 - An elevation point is located on a breakline
    0x00000800 - Two breaklines linked at extremity or other vertex point do not have the
                 same elevation at these points
    0x00001000 - Two points are identical (duplicate)


    -----------------------------------------------------------------------------
*/
class HVEBreakLineDefect
    {
public:
    HVEBreakLineDefect();
    HVEBreakLineDefect(long i_DefectCode, const HGF3DCoord<double>& i_rDefectLocation);
    HVEBreakLineDefect(const HVEBreakLineDefect& i_rDefect);
    virtual ~HVEBreakLineDefect();

    HVEBreakLineDefect&
    operator=(const HVEBreakLineDefect& i_rDefect);

    const HGF3DCoord<double>&
    GetLocation() const;

    long    GetCode() const;

private:
    friend class HVEBreakLines;

    HVEBreakLineDefect(long i_DefectCode, const HGF3DCoord<double>& i_rDefectLocation, long i_BreakLineIndex);
    long    GetBreakLineIndex() const;

    long m_Code;
    HGF3DCoord<double> m_Location;
    long m_BreakLineIndex;
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    -----------------------------------------------------------------------------
*/
class HVEBreakLines
    {
public:

    HVEBreakLines();
    HVEBreakLines(const HFCPtr<HGF2DCoordSys>& i_rCoordSys);
    HVEBreakLines(const HFCPtr<HGF2DCoordSys>& i_rCoordSys,
                  double i_Tolerance);

#if (0)
    HVEBreakLines(const KGI3DDTMPointCollection& i_rDTMPoints, double i_Tolerance);
#endif

    HVEBreakLines(const HVEBreakLines& i_rObj);

    ~HVEBreakLines();
    HVEBreakLines&     operator=(const HVEBreakLines& i_rObj);

#if (0)
    bool               AutoValidates() const;
    bool               SetAutoValidate(bool i_AutoValidate) const;

    KGIBreaklineDefect AddPoint(const HGF3DPoint& i_rNewPoint);
    KGIBreaklineDefect AddLine(const HVE3DPolyLine& i_rNewLine);
#else
    void               AddPoint(const HGF3DPoint& i_rNewPoint);
    void               AddLine(const HVE3DPolyLine& i_rNewLine);
#endif

    size_t             GetPointSize() const;
    size_t             GetLineSize() const;

    const HFCPtr<HVE3DPolyLine>&
    GetLine(size_t i_LineIndex) const;
    const HGF3DPoint&
    GetPoint(size_t i_PointIndex) const;



    int                Validate(ostream& o_rErrorStream,
                                string i_Prefix = string(""),
                                long i_ValidationMask = 0x0000FFFF,
                                bool i_Verbose = false) const;

    int                Validate(list<HVEBreakLineDefect>* o_pListOfDefects,
                                long i_ValidationMask = 0x0000FFFF) const;


    const HFCPtr<HGF2DCoordSys>&
    GetCoordSys() const;

    virtual void       PrintState(ostream& o_rOutput) const;

    friend istream& operator>>(istream& pi_InStream, HVEBreakLines& pio_rPoints);

protected:


private:

#if (0)
    HGFBreakLineDefect Validate(const HGF3DPoint& i_rNewPoint) const;
    HGFBreakLineDefect Validate(const HVE3DPolyLine& i_rNewLine) const;
#endif

    double m_Tolerance;
    HFCPtr<HGF2DCoordSys> m_pCoordSys;
    vector<HFCPtr<HVE3DPolyLine> > m_BreakLineList;
    vector<HGF3DPoint, allocator<HGF3DPoint> > m_DTMPoints;

    };



// Stream interaction global operators.
istream& operator>>(istream& pi_InStream, HVEBreakLines& pio_rPoints);

#if (0)
ostream& operator<<(ostream& pi_OutStream, const HVEBreakLines& pi_rPoints);
#endif



#include "HVEBreakLines.hpp"

