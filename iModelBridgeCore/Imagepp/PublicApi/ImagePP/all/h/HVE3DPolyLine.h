//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DPolyLine.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE3DPolyLine
//-----------------------------------------------------------------------------
// This class implements chained multi-segments basic linear
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HGF2DCoordSys;
class HVE2DPolySegment;
END_IMAGEPP_NAMESPACE

#include "HVE2DPolySegment.h"
#include "HGF3DPoint.h"
#include "HGF3DExtent.h"
#include "HFCPtr.h"
#include "HGF2DExtent.h"

/** -----------------------------------------------------------------------------

    This class implements a 3D polyline which is a simple chain of 3D points.
    The present implementation is not intended as a full 3D implementation
    but does provide a rich set of 2D operations.
    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE3DPolyLine : public HFCShareableObject<HVE3DPolyLine>
    {


public:


    // Primary methods
    HVE3DPolyLine(size_t i_rCapacity = 0);
#if (0)
    HVE3DPolyLine(const HGF3DPoint& i_rStartPoint,
                  const HGF3DPoint& i_rEndPoint);
    HVE3DPolyLine(const HGF3DPointCollection& i_rListOfPoints);
#endif
    HVE3DPolyLine(const HGF3DPointCollection& i_rListOfPoints,
                  const HFCPtr<HGF2DCoordSys>& i_rpCoordSys);
    HVE3DPolyLine(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys,
                  size_t i_rCapacity = 0);
    HVE3DPolyLine(const HVE3DPolyLine&   i_rObject);
    virtual            ~HVE3DPolyLine();

    HVE3DPolyLine&  operator=(const HVE3DPolyLine& i_rObj);

    IMAGEPP_EXPORT const HGF3DExtent<double>&
    GetExtent() const;
    // Setting and extracting
    void               AppendPoint(const HGF3DPoint& i_rNewPoint);
    const HGF3DPoint&  GetPoint(size_t i_Index) const;
    size_t             GetSize() const;

    size_t             Intersect2D(const HVE3DPolyLine& i_rPolyLine,
                                   HGF3DPointCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT bool Crosses2D(const HVE3DPolyLine& i_rPolyLine) const;
    bool               AutoCrosses2D() const;

    HVE3DPolyLine*     Clone() const;

    IMAGEPP_EXPORT bool ConnectsTo2D(const HVE3DPolyLine& i_rPolyLine) const;
    IMAGEPP_EXPORT bool LinksTo2D(const HVE3DPolyLine& i_rPolyLine) const;

    const HGF3DPoint&  GetStartPoint() const;
    const HGF3DPoint&  GetEndPoint() const;
    bool               IsPointOn2D(const HGF3DPoint& i_rPoint) const;
    bool               IsContiguousTo2D(const HVE3DPolyLine& i_rPolyLine) const;
    size_t             GetContiguousPointsTo2D(const HVE3DPolyLine& i_rPolyLine,
                                               HGF3DPointCollection* po_pCrossPoints) const;


    bool               IsNull() const;
    double             GetTolerance() const;
    void               SetTolerance(double i_Tolerance);
    bool               IsAutoToleranceActive() const;
    void               SetAutoToleranceActive(bool i_AutoToleranceActive);
    double             CalculateLength2D() const;

    void                  SetCoordSys(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys);
    HFCPtr<HGF2DCoordSys> GetCoordSys();
protected:



    void               ValidateInvariants() const
        {
#if (0)
        KASSERT(m_Points.size() == 0 ||
                (m_Points == m_Points[0] && m_EndPoint == m_Points.back()));
#endif
        }

private:

    void                      Create2DPolyLine() const;
    void                      UpdateInternalExtent() const;
    IMAGEPP_EXPORT void        ResetTolerance();


    // List of points
    HGF3DPointCollection    m_Points;
    HFCPtr<HGF2DCoordSys>   m_pCoordSys;
    double                  m_Tolerance;
    bool                    m_AutoToleranceActive;

    // Acceleration attributes
    mutable bool                     m_ExtentUpToDate;
    mutable HGF3DExtent<double>      m_Extent;
    mutable HFCPtr<HVE2DPolySegment> m_p2DPolyLine;
    mutable bool                     m_2DPolyLineUpToDate;


    // Private methods
    void               Reserve(unsigned long i_PointsToPreAllocate);
    void               MakeEmpty();
    };
END_IMAGEPP_NAMESPACE


#include "HVE3DPolyLine.hpp"

