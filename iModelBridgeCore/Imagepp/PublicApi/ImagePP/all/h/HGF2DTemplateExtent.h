//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTemplateExtent.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DTemplateExtent
//-----------------------------------------------------------------------------
// Two dimensional extent
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE

template<typename DataType = double, class COORD = HGF2DCoord<DataType> > class HGF2DTemplateExtent
    {
public:

    // Primary methods
    HGF2DTemplateExtent();
    HGF2DTemplateExtent(const COORD&          pi_rOrigin,
                        const COORD&          pi_rCorner);
    HGF2DTemplateExtent(DataType pi_X1,
                        DataType pi_Y1,
                        DataType pi_X2,
                        DataType pi_Y2);
    HGF2DTemplateExtent(const HGF2DTemplateExtent& pi_rObj);
    virtual            ~HGF2DTemplateExtent();
    HGF2DTemplateExtent&       operator=(const HGF2DTemplateExtent& pi_rObj);

    // Compare methods
    bool                operator==(const HGF2DTemplateExtent& pi_rObj) const;
    bool                operator!=(const HGF2DTemplateExtent& pi_rObj) const;

    // Compare methods with epsilon
    int                IsEqualTo(const HGF2DTemplateExtent& pi_rObj) const;
    int                IsEqualTo(const HGF2DTemplateExtent& pi_rObj, DataType pi_Epsilon) const;

    // Information
    bool              IsPointIn(const COORD& pi_rPoint) const;
    bool              IsPointInnerIn(const COORD& pi_rPoint) const;
    bool              IsPointOutterIn(const COORD& pi_rPoint) const;
    bool              IsPointInnerIn(const COORD& pi_rPoint, DataType pi_Tolerance) const;
    bool              IsPointOutterIn(const COORD& pi_rPoint, DataType pi_Tolerance) const;

    // Coordinate management
    DataType           GetXMin() const;
    DataType           GetYMin() const;
    DataType           GetXMax() const;
    DataType           GetYMax() const;

    void               SetXMin(DataType    pi_XMin);
    void               SetYMin(DataType    pi_YMin);
    void               SetXMax(DataType    pi_XMax);
    void               SetYMax(DataType    pi_YMax);

    COORD              GetOrigin() const;
    COORD              GetCorner() const;

    DataType           GetWidth() const;
    DataType           GetHeight() const;

    void               SetOrigin(const COORD& pi_rNewOrigin);
    void               SetCorner(const COORD& pi_rNewCorner);

    void               Set(DataType pi_X1,
                           DataType pi_Y1,
                           DataType pi_X2,
                           DataType pi_Y2);


    void               Add (const COORD& pi_rLocation);
    void               Add (const HGF2DTemplateExtent& pi_rExtent);

    // Union ...
    void               Union (const HGF2DTemplateExtent& pi_rObj);
    void               Intersect (const HGF2DTemplateExtent& pi_rObj);

    // Overlap determination
    bool              Overlaps(const HGF2DTemplateExtent& pi_rObj) const;
    bool              OutterOverlaps(const HGF2DTemplateExtent& pi_rObj) const;
    bool              OutterOverlaps(const HGF2DTemplateExtent& pi_rObj,
                                      DataType pi_Epsilon) const;
    bool              InnerOverlaps(const HGF2DTemplateExtent& pi_rObj) const;
    bool              InnerOverlaps(const HGF2DTemplateExtent& pi_rObj,
                                     DataType pi_Epsilon) const;


    // linker warning LNK4221
    // this method must be added into the .cpp file
    void                DummyMethod() const;


protected:

private:
    // Coordinates of this location
    DataType         m_XMin;
    DataType         m_YMin;
    DataType         m_XMax;
    DataType         m_YMax;

    };

END_IMAGEPP_NAMESPACE

#include "HGF2DTemplateExtent.hpp"

