//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteSegment.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DLiteSegment
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class implements a segment. This is a quite simple class made of two sets of
    coordinate pairs. The segment expressed a graphic object which is a line
    beginning from one of the points and finishing at the other point. The order of
    points is maintained, that is the segment is oriented from the first point to
    the second. The class implements methods to perform some mathematical operations
    upon segments and positions (HGF2DPosition). Such operations include finding a
    crossing point between segments, and ways to obtains the closest point on segment
    from a specified location. This class is an acceleration class which implements
    some of the most common operations applicable to segments. This kind of segments
    does not make any reference to an interpretation coordinate system, nor are the coordinates
    expressed in any units. The coordinates are simply expressed as raw numbers, which must
    be expressed in the same units, thus represent values to quantities of exactly the same
    type and interpretation ratio.

    The class does certainly not provide a complete set of method, thus implementing a very
    limited sets of behaviors, however, this class is not made to be used at large, but
    only for the construction of high performance vectorial components which have
    some relation to segments.

    The class also applies some sort of tolerance application. This application is guaranteed
    to be consistent. This means that any behavior will make use of the same tolerance, but most
    important, this tolerance will be applied in the same fashion. Notice that tolerance application
    is based on the distance from one point to the line a segment is part of. Although, such
    computation is dependant in precision on the slope of the line, implementation of tolerance
    compare operation are therefore dependant on the slope of the segment
    line.
    -----------------------------------------------------------------------------
*/
class HGF2DLiteSegment
    {

public:

    enum CrossState
        {
        CROSS_FOUND,
        PARALLEL,
        NO_CROSS
        };

    enum ExtremityProcessing
        {
        INCLUDE_EXTREMITIES,
        EXCLUDE_EXTREMITIES
        };


    // Primary methods
                            HGF2DLiteSegment();
                            HGF2DLiteSegment(const HGF2DPosition& pi_rStartPoint,
                                             const HGF2DPosition& pi_rEndPoint,
                                             double pi_Tolerance = HGLOBAL_EPSILON);
                            HGF2DLiteSegment(const HGF2DLiteSegment&    pi_rObject);
                            ~HGF2DLiteSegment();

    HGF2DLiteSegment&       operator=(const HGF2DLiteSegment& pi_rObj);
    bool                    operator==(const HGF2DLiteSegment& pi_rObj) const;

    // Setting
    void                    SetStartPoint(const HGF2DPosition& pi_rNewStartPoint);
    void                    SetEndPoint(const HGF2DPosition& pi_rNewEndPoint);
    const HGF2DPosition&    GetStartPoint() const;
    const HGF2DPosition&    GetEndPoint() const;

    // Tolerance
    double                  GetTolerance() const;
    void                    SetTolerance(double pi_Tolerance);

    // Geometric properties and behavior
    HGF2DLiteSegment::CrossState
                            IntersectSegment(const HGF2DLiteSegment& pi_rSegment,
                                             HGF2DPosition* po_pPoint) const;

    HGF2DLiteSegment::CrossState
                            IntersectSegmentExtremityIncluded(const HGF2DLiteSegment& pi_rSegment,
                                      HGF2DPosition* po_pPoint,
                                      bool* po_pCrossesAtExtremity = NULL) const;


    size_t                  ObtainContiguousnessPoints(const HGF2DLiteSegment& pi_rSegment,
                                                       HGF2DPositionCollection* po_pContiguousnessPoints) const;

    bool                    LinksTo(const HGF2DLiteSegment& pi_rSegment) const;
    bool                    Crosses(const HGF2DLiteSegment& pi_rSegment) const;
    bool                    AreContiguous(const HGF2DLiteSegment& pi_rSegment) const;
    bool                    AreAdjacent(const HGF2DLiteSegment& pi_rSegment) const;
    bool                    AreParallel(const HGF2DLiteSegment& pi_rSegment,
                                         double pi_SlopeTolerance = 0.0) const;
    bool                    IsPointOn(const HGF2DPosition& pi_rTestPoint, ExtremityProcessing pi_ExtremityProcessing = INCLUDE_EXTREMITIES) const;
    bool                    ArePointsOnDifferentSides(const HGF2DPosition& pi_rPoint1, const HGF2DPosition& pi_rPoint2) const;

    bool                    IntersectsAtSplitPoint(const HGF2DLiteSegment& pi_rFirstSegment,
                                                   const HGF2DLiteSegment& pi_rSecondSegment) const;


    HGF2DPosition           CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;

    bool                    IsPointOnLineOnSegment(const HGF2DPosition& pi_rTestPoint) const;

    double                  CalculateLength() const;

protected:

private:

    bool                    AreDifficultSegmentsContiguous(const HGF2DLiteSegment& pi_rSegment) const;
    void                    GetDeltas(double* po_pDx, double* po_pDy) const;

    // The start and end point of segment
    HGF2DPosition           m_StartPoint;
    HGF2DPosition           m_EndPoint;

    // Tolerance to apply in operations
    double                  m_Tolerance;


    };

END_IMAGEPP_NAMESPACE
#include "HGF2DLiteSegment.hpp"

