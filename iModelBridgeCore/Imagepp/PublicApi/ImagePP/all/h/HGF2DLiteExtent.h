//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    IMAGEPPTEST_EXPORT HGF2DLiteExtent();
    IMAGEPPTEST_EXPORT HGF2DLiteExtent(const HGF2DPosition&          pi_rOrigin,
                    const HGF2DPosition&          pi_rCorner);
    IMAGEPPTEST_EXPORT HGF2DLiteExtent(double pi_X1,
                    double pi_Y1,
                    double pi_X2,
                    double pi_Y2);
    IMAGEPPTEST_EXPORT HGF2DLiteExtent(const HGF2DLiteExtent& pi_rObj);
    virtual            ~HGF2DLiteExtent();
    HGF2DLiteExtent&       operator=(const HGF2DLiteExtent& pi_rObj);

    // Compare methods
    IMAGEPPTEST_EXPORT bool              operator==(const HGF2DLiteExtent& pi_rObj) const;
    bool              operator!=(const HGF2DLiteExtent& pi_rObj) const;

    // Compare methods with epsilon
    IMAGEPPTEST_EXPORT bool              IsEqualTo(const HGF2DLiteExtent& pi_rObj) const;
    bool              IsEqualTo(const HGF2DLiteExtent& pi_rObj, double pi_Epsilon) const;

    // Information
    bool              IsDefined() const;
    bool              IsPointIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointInnerIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointOutterIn(const HGF2DPosition& pi_rPoint) const;
    bool              IsPointInnerIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;
    bool              IsPointOutterIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const;

    // Coordinate management
    IMAGEPPTEST_EXPORT double            GetXMin() const;
    IMAGEPPTEST_EXPORT double            GetYMin() const;
    IMAGEPPTEST_EXPORT double            GetXMax() const;
    IMAGEPPTEST_EXPORT double            GetYMax() const;

    IMAGEPPTEST_EXPORT void              SetXMin(double    pi_XMin);
    IMAGEPPTEST_EXPORT void              SetYMin(double    pi_YMin);
    IMAGEPPTEST_EXPORT void              SetXMax(double    pi_XMax);
    IMAGEPPTEST_EXPORT void              SetYMax(double    pi_YMax);

    IMAGEPPTEST_EXPORT HGF2DPosition     GetOrigin() const;
    IMAGEPPTEST_EXPORT HGF2DPosition     GetCorner() const;

    double            GetWidth() const;
    double            GetHeight() const;

    IMAGEPPTEST_EXPORT void              SetOrigin(const HGF2DPosition& pi_rNewOrigin);
    IMAGEPPTEST_EXPORT void              SetCorner(const HGF2DPosition& pi_rNewCorner);

    void              Set(double pi_X1,
                          double pi_Y1,
                          double pi_X2,
                          double pi_Y2);


    void              Add (const HGF2DPosition& pi_rLocation);
    void              Add (const HGF2DLiteExtent& pi_rExtent);

    // Union ...
    void              Union (const HGF2DLiteExtent& pi_rObj);
    IMAGEPPTEST_EXPORT void              Intersect (const HGF2DLiteExtent& pi_rObj);

    // Overlap determination
    IMAGEPPTEST_EXPORT bool              Overlaps(const HGF2DLiteExtent& pi_rObj) const;
    IMAGEPPTEST_EXPORT bool              OutterOverlaps(const HGF2DLiteExtent& pi_rObj) const;
   IMAGEPPTEST_EXPORT  bool              OutterOverlaps(const HGF2DLiteExtent& pi_rObj,
                                      double pi_Epsilon) const;
    IMAGEPPTEST_EXPORT bool              InnerOverlaps(const HGF2DLiteExtent& pi_rObj) const;
    IMAGEPPTEST_EXPORT bool              InnerOverlaps(const HGF2DLiteExtent& pi_rObj,
                                     double pi_Epsilon) const;


    IMAGEPPTEST_EXPORT bool              Contains(const HGF2DLiteExtent& pi_rObj) const;

    IMAGEPPTEST_EXPORT bool              InnerContains(const HGF2DLiteExtent& pi_rObj) const;
    IMAGEPPTEST_EXPORT bool              InnerContains(const HGF2DLiteExtent& pi_rObj,
                                     double pi_Epsilon) const;

    IMAGEPPTEST_EXPORT bool              OuterContains(const HGF2DLiteExtent& pi_rObj) const;
    IMAGEPPTEST_EXPORT bool              OuterContains(const HGF2DLiteExtent& pi_rObj,
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

    bool m_initializedXMin;
    bool m_initializedXMax;
    bool m_initializedYMin;
    bool m_initializedYMax;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DLiteExtent.hpp"
