//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DPolyline.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE3DPolyLine
//-----------------------------------------------------------------------------
// This class implements chained multi-segments basic linear
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DPolySegment.h"
#include "HGF3DPoint.h"
#include "HFCPtr.h"
#include "HGF2DExtent.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DCoordSys;
class HVE2DPolySegment;

/** -----------------------------------------------------------------------------

    This class implements a 3D polyline which is a simple chain of 3D points.
    The present implementation is not intended as a full 3D implementation
    but does provide a rich set of 2D operations.
    -----------------------------------------------------------------------------
*/
class HGF3DPolyLine : public HFCShareableObject<HGF3DPolyLine>
    {


public:


    // Primary methods
    HGF3DPolyLine();
#if (0)
    HGF3DPolyLine(const HGF3DPoint& i_rStartPoint,
                  const HGF3DPoint& i_rEndPoint);
    HGF3DPolyLine(const HGF3DPointCollection& i_rListOfPoints);
#endif
    HGF3DPolyLine(const HGF3DPointCollection& i_rListOfPoints,
                  const HFCPtr<HGF2DCoordSys>& i_rpCoordSys);
    HGF3DPolyLine(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys);
    HGF3DPolyLine(const HGF3DPolyLine&   i_rObject);
    virtual            ~HGF3DPolyLine();

    HGF3DPolyLine&  operator=(const HGF3DPolyLine& i_rObj);

    // Setting and extracting
    void               AppendPoint(const HGF3DPoint& i_rNewPoint);
    const HGF3DPoint&  GetPoint(size_t i_Index) const;
    size_t             GetSize() const;

    size_t             Intersect2D(const HGF3DPolyLine& i_rPolyLine,
                                   HGF3DPointCollection* po_pCrossPoints) const;
    bool               Crosses2D(const HGF3DPolyLine& i_rPolyLine) const;
    bool               AutoCrosses2D() const;

    HGF3DPolyLine*     Clone() const;

    bool               ConnectsTo2D(const HGF3DPolyLine& i_rPolyLine) const;
    bool               LinksTo2D(const HGF3DPolyLine& i_rPolyLine) const;

    const HGF3DPoint&  GetStartPoint() const;
    const HGF3DPoint&  GetEndPoint() const;
    bool               IsPointOn2D(const HGF3DPoint& i_rPoint) const;
    bool               IsContiguousTo2D(const HGF3DPolyLine& i_rPolyLine) const;
    size_t             GetContiguousPointsTo2D(const HGF3DPolyLine& i_rPolyLine,
                                               HGF3DPointCollection* po_pCrossPoints) const;


    bool               IsNull() const;
    double             GetTolerance() const;
    void               SetTolerance(double i_Tolerance);
    bool               IsAutoToleranceActive() const;
    void               SetAutoToleranceActive(bool i_AutoToleranceActive);
    double             CalculateLength2D() const;
protected:


private:

    void               ValidateInvariants() const
        {
#if (0)
        KASSERT(m_Points.size() == 0 ||
                (m_Points == m_Points[0] && m_EndPoint == m_Points.back()));
#endif
        }

    void               Create2DPolyLine() const;
    void               ResetTolerance();


    // List of points
    HGF3DPointCollection    m_Points;
    HFCPtr<HGF2DCoordSys>   m_pCoordSys;
    double                  m_Tolerance;
    bool                    m_AutoToleranceActive;

    // Acceleration attributes
    mutable bool                     m_ExtentUpToDate;
    mutable HGF2DExtent              m_Extent;
    mutable HFCPtr<HGF2DPolySegment> m_p2DPolyLine;
    mutable bool                     m_2DPolyLineUpToDate;


    // Private methods
    void               Reserve(unsigned long i_PointsToPreAllocate);
    void               MakeEmpty();

    };

END_IMAGEPP_NAMESPACE
#include "HGF3DPolyLine.hpp"

