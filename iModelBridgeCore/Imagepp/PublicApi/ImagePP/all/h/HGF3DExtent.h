//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DExtent.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : Extent3D
//-----------------------------------------------------------------------------
// Position in three-dimension
//-----------------------------------------------------------------------------

#pragma once

#include "HGF3DCoord.h"
#include "HGF2DExtent.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class is used to describe non-oriented cubic regions defined in
    the space.  It plays the role of an atomic "concrete data type"
    having no parent and not specifically designed to have child versions.
    The extent is described by coordinate values that are labeled Xmin Ymin ZMin Xmax Ymax ZMax.
    The (Xmin, Ymin, ZMin) coordinates describe the position of one corner known as the
    Origin, the opposite corner referred to by Corner being described by (Xmax, Ymax, ZMax).
    The origin coordinates must be of lower or equal value than the opposite corner.


    A HGF2DCoordSys object is used to give concrete sense to coordinate values of
    the extent.

    An HGF2DExtent object can be empty. In this case some coordinates of the extent
    are undefined, and no operations other than those that permit the definition of
    the extent are permitted. The extent can be partially defined when for example
    the X coordinates are defined, but not the Y coordinates. At all times Xmin must
    be smaller or equal to Xmax, and Ymin smaller or equal to Ymax. The methods strongly
    enforce these rules by use of contracts. The specific details of each restrictions
    imposed when the extent is empty are described in each methods.

    -----------------------------------------------------------------------------
*/

template <class DataType = double> class HGF3DExtent
    {
public:
    HGF3DExtent();
    HGF3DExtent(const HGF3DCoord<DataType>&          i_rOrigin,
                const HGF3DCoord<DataType>&          i_rCorner);
    HGF3DExtent(DataType            i_XMin,
                DataType            i_YMin,
                DataType            i_ZMin,
                DataType            i_XMax,
                DataType            i_YMax,
                DataType            i_ZMax);
    HGF3DExtent(const HGF3DCoord<DataType>& i_OriginPoint);
    HGF3DExtent(const HGF3DExtent<DataType>& i_rObject);
    virtual            ~HGF3DExtent();
    HGF3DExtent<DataType>&
    operator=(const HGF3DExtent<DataType>& i_rObject);

    bool               operator==(const HGF3DExtent<DataType>& i_rObject) const;
    bool               operator!=(const HGF3DExtent<DataType>& i_rObject) const;

    bool               Equals(const HGF3DExtent<DataType>& i_rObject, DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON()) const;

    bool               IsPointIn(const HGF3DCoord<DataType>& i_rPoint,
                                 DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON(),
                                 bool     i_ExcludeBoundary = false) const;

    bool               IsPointIn2D(const HGF3DCoord<DataType>& i_rPoint,
                                   DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON(),
                                   bool     i_ExcludeBoundary = false) const;


    DataType           GetXMin() const;
    DataType           GetYMin() const;
    DataType           GetZMin() const;
    DataType           GetXMax() const;
    DataType           GetYMax() const;
    DataType           GetZMax() const;

    void               SetXMin(DataType    i_XMin);
    void               SetYMin(DataType    i_YMin);
    void               SetZMin(DataType    i_ZMin);
    void               SetXMax(DataType    i_XMax);
    void               SetYMax(DataType    i_YMax);
    void               SetZMax(DataType    i_ZMax);

    HGF3DCoord<DataType>
    GetOrigin() const;
    HGF3DCoord<DataType>
    GetCorner() const;
    void               SetOrigin(const HGF3DCoord<DataType>& i_rNewOrigin);
    void               SetCorner(const HGF3DCoord<DataType>& i_rNewCorner);

    void               Set(DataType pi_X1,
                           DataType pi_Y1,
                           DataType pi_Z1,
                           DataType pi_X2,
                           DataType pi_Y2,
                           DataType pi_Z2);

    DataType           GetWidth() const;
    DataType           GetHeight() const;
    DataType           GetThickness() const;
    DataType           CalculateVolume() const;

    void               Add(const HGF3DCoord<DataType>& i_rLocation);
    void               Add(const HGF3DExtent<DataType>& i_rExtent);

    bool               Overlaps(const HGF3DExtent<DataType>& i_rObject) const;

    bool               OuterOverlaps(const HGF3DExtent<DataType>& i_rObject,
                                     DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON()) const;

    bool               InnerOverlaps(const HGF3DExtent& i_rObject,
                                     DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON()) const;

    bool               Contains(const HGF3DExtent<DataType>& i_rObject) const;

    bool               InnerContains(const HGF3DExtent<DataType>& i_rObject,
                                     DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON()) const;

    bool               OuterContains(const HGF3DExtent<DataType>& i_rObject,
                                     DataType i_Tolerance = HNumeric<DataType>::GLOBAL_EPSILON()) const;

protected:
    void            ValidateInvariants() const
        {
        HASSERT(m_XMin <= m_XMax);
        HASSERT(m_YMin <= m_YMax);
        HASSERT(m_ZMin <= m_ZMax);
        };



private:

    // Coordinates of this location
    DataType         m_XMin;
    DataType         m_YMin;
    DataType         m_ZMin;
    DataType         m_XMax;
    DataType         m_YMax;
    DataType         m_ZMax;
    };

END_IMAGEPP_NAMESPACE
#include "HGF3DExtent.hpp"
