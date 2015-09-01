//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteExtent.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DLiteExtent
//-----------------------------------------------------------------------------
// Two dimensional extent
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DLiteExtent
    {
public:

    // Primary methods
    HGF2DLiteExtent();
    HGF2DLiteExtent(const HGF2DPosition&          pi_rOrigin,
                    const HGF2DPosition&          pi_rCorner);
    HGF2DLiteExtent(double pi_X1,
                    double pi_Y1,
                    double pi_X2,
                    double pi_Y2);
    HGF2DLiteExtent(const HGF2DLiteExtent& pi_rObj);
    virtual            ~HGF2DLiteExtent();
    HGF2DLiteExtent&       operator=(const HGF2DLiteExtent& pi_rObj);

    // Compare methods
    bool              operator==(const HGF2DLiteExtent& pi_rObj) const;
    bool              operator!=(const HGF2DLiteExtent& pi_rObj) const;

    // Compare methods with epsilon
    bool              IsEqualTo(const HGF2DLiteExtent& pi_rObj) const;
    bool              IsEqualTo(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;

    // Information
    bool              IsDefined() const;
    bool              IsPointIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointInnerIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointOutterIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointInnerIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;
    bool              IsPointOutterIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;

    // Coordinate management
    double            GetXMin() const;
    double            GetYMin() const;
    double            GetXMax() const;
    double            GetYMax() const;

    void              SetXMin(double    pi_XMin);
    void              SetYMin(double    pi_YMin);
    void              SetXMax(double    pi_XMax);
    void              SetYMax(double    pi_YMax);

    HGF2DPosition     GetOrigin() const;
    HGF2DPosition     GetCorner() const;

    double            GetWidth() const;
    double            GetHeight() const;

    void              SetOrigin(const HGF2DPosition& pi_rNewOrigin);
    void              SetCorner(const HGF2DPosition& pi_rNewCorner);

    void              Set(double pi_X1,
                          double pi_Y1,
                          double pi_X2,
                          double pi_Y2);


    void              Add (const HGF2DPosition& pi_rLocation);
    void              Add (const HGF2DLiteExtent& pi_rExtent);

    // Union ...
    void              Union (const HGF2DLiteExtent& pi_rObj);
    void              Intersect (const HGF2DLiteExtent& pi_rObj);

    // Overlap determination
    bool              Overlaps(const HGF2DLiteExtent& pi_rObj) const;
    bool              OutterOverlaps(const HGF2DLiteExtent& pi_rObj) const;
    bool              OutterOverlaps(const HGF2DLiteExtent& pi_rObj,
                                      double pi_Epsilon) const;
    bool              InnerOverlaps(const HGF2DLiteExtent& pi_rObj) const;
    bool              InnerOverlaps(const HGF2DLiteExtent& pi_rObj,
                                     double pi_Epsilon) const;


    bool              Contains(const HGF2DLiteExtent& pi_rObj) const;

    bool              InnerContains(const HGF2DLiteExtent& pi_rObj) const;
    bool              InnerContains(const HGF2DLiteExtent& pi_rObj,
                                     double pi_Epsilon) const;

    bool              OuterContains(const HGF2DLiteExtent& pi_rObj) const;
    bool              OuterContains(const HGF2DLiteExtent& pi_rObj,
                                     double pi_Epsilon) const;

    // linker warning LNK4221
    // this method must be added into the .cpp file
    void                DummyMethod() const;


protected:

private:
    // Coordinates of this location
    double         m_XMin;
    double         m_YMin;
    double         m_XMax;
    double         m_YMax;

    HDEBUGCODE(bool m_initializedXMin;)
    HDEBUGCODE(bool m_initializedXMax;)
    HDEBUGCODE(bool m_initializedYMin;)
    HDEBUGCODE(bool m_initializedYMax;)
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DLiteExtent.hpp"
