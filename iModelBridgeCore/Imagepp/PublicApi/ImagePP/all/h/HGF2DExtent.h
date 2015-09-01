//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DExtent.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DExtent
//-----------------------------------------------------------------------------
// Two dimensional extent
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HGF2DLocation.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class is used to describe non-oriented rectangular regions defined in
    the coordinate system.  It plays the role of an atomic "concrete data type"
    having no parent and not specifically designed to have child versions.
    The extent is described by coordinate values that are labeled Xmin Ymin Xmax Ymax.
    The (Xmin, Ymin) coordinates describe the position of one corner known as the
    Origin, the opposite corner referred to by Corner being described by (Xmax, Ymax).
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

    @code
        // Example code of usage of an extent
        HFCPtr<HGF2DCoordSys>   pMyMainSystem (new HGF2DCoordSys ());

        // An HGF2DStretchModel is a kind of HGF2DTransfoModel
        HGF2DStretchModel ImageToWorld(HGF2DDisplacement (10, 12, 2.0, 2.0);

        HFCPtr<HGF2DCoordSys>   pImageCoordSys (new HGF2DCoordSys
                                                         (ImageToWorld, pMyMainSystem));

        HGF2DLocation       ImageOrigin (0, 0, pImageCoordSys);

        HGF2DExtent MyWorldExtent (pMyMainSystem);
        HGF2DExtent MyImageExtent (ImageOrigin.GetX(),
                                   ImageOrigin.GetY(),
                                   ImageOrigin.GetX() + 256.0,
                                   ImageOrigin.GetY() + 256.0,
                                   pMyMainSystem);
        HGF2DDisplacement   MyDisplacement(10, 10);
        HGF2DLocation NewLocation = ImageOrigin + MyDisplacement;

        HGF2DExtent MyOtherExtent (ImageOrigin, NewLocation);
        HGF2DExtent DuplicateExtent (MyOtherExtent);

    @end
    -----------------------------------------------------------------------------
*/
class HGF2DExtent
    {
public:

    // Primary methods
    IMAGEPP_EXPORT                    HGF2DExtent();
    IMAGEPP_EXPORT                     HGF2DExtent(const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys);
    IMAGEPP_EXPORT                     HGF2DExtent(const HGF2DLocation&          pi_rOrigin,
                                           const HGF2DLocation&          pi_rCorner);
    IMAGEPP_EXPORT                     HGF2DExtent(double pi_XMin,
                                           double pi_YMin,
                                           double pi_XMax,
                                           double pi_YMax,
                                           const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys);
    HGF2DExtent(string&                      pi_rSerializationString,
                const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                     HGF2DExtent(const HGF2DExtent& pi_rObj);
    IMAGEPP_EXPORT  virtual            ~HGF2DExtent();
    IMAGEPP_EXPORT  HGF2DExtent&       operator=(const HGF2DExtent& pi_rObj);

    // Compare methods
    IMAGEPP_EXPORT  bool               operator==(const HGF2DExtent& pi_rObj) const;
    bool                       operator!=(const HGF2DExtent& pi_rObj) const;

    // Compare methods with epsilon
    IMAGEPP_EXPORT bool              IsEqualTo(const HGF2DExtent& pi_rObj) const;
    IMAGEPP_EXPORT bool              IsEqualTo(const HGF2DExtent& pi_rObj, double pi_Epsilon) const;

    // Information
    bool              IsDefined() const;
    bool              IsPointIn(const HGF2DLocation& pi_rPoint,
                                 bool                pi_ExcludeBoundary = false) const;


    // Coordinate management
    double        GetXMin() const;
    double        GetYMin() const;
    double        GetXMax() const;
    double        GetYMax() const;

    IMAGEPP_EXPORT  void       SetXMin(double    pi_XMin);
    IMAGEPP_EXPORT  void       SetYMin(double    pi_YMin);
    IMAGEPP_EXPORT  void       SetXMax(double    pi_XMax);
    IMAGEPP_EXPORT  void       SetYMax(double    pi_YMax);

    HGF2DLocation      GetOrigin() const;
    HGF2DLocation      GetCorner() const;
    void               SetOrigin(const HGF2DLocation& pi_rNewOrigin);
    void               SetCorner(const HGF2DLocation& pi_rNewCorner);

    HGF2DLocation      GetLowerLeft() const;
    HGF2DLocation      GetLowerRight() const;
    HGF2DLocation      GetUpperLeft() const;
    HGF2DLocation      GetUpperRight() const;

    // Dimension measurement
    double        GetWidth() const;
    double        GetHeight() const;
    double        CalculateArea() const;

    // Coordinate system management
    const HFCPtr<HGF2DCoordSys>&     GetCoordSys () const;
    void                             SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT  void              ChangeCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);


    void               Move(const HGF2DDisplacement& pi_rOffset);
    HGF2DExtent        operator+(const HGF2DDisplacement& pi_rOffset) const;
    HGF2DExtent        operator-(const HGF2DDisplacement& pi_rOffset) const;
    HGF2DExtent&       operator+=(const HGF2DDisplacement& pi_rOffset);
    HGF2DExtent&       operator-=(const HGF2DDisplacement& pi_rOffset);


    IMAGEPP_EXPORT  void               Add (const HGF2DLocation& pi_rLocation);
    IMAGEPP_EXPORT  void               Add (const HGF2DExtent& pi_rExtent);

    // Union ...
    void                      Union (const HGF2DExtent& pi_rObj);
    IMAGEPP_EXPORT  void       Intersect (const HGF2DExtent& pi_rObj);
    void                      Differentiate (const HGF2DExtent& pi_rObj);
    IMAGEPP_EXPORT  bool       DoTheyOverlap (const HGF2DExtent& pi_rObj) const;

    IMAGEPP_EXPORT  bool       OutterOverlaps(const HGF2DExtent& pi_rObj) const;
    bool                      OutterOverlaps(const HGF2DExtent& pi_rObj,
                                             double pi_Epsilon) const;

    bool                      InnerOverlaps(const HGF2DExtent& pi_rObj) const;
    bool                      InnerOverlaps(const HGF2DExtent& pi_rObj,
                                            double pi_Epsilon) const;

    bool                      Contains(const HGF2DExtent& pi_rObj) const;

    IMAGEPP_EXPORT  bool       InnerContains(const HGF2DExtent& pi_rObj) const;
    bool                      InnerContains(const HGF2DExtent& pi_rObj,
                                            double pi_Epsilon) const;

    IMAGEPP_EXPORT  bool       OuterContains(const HGF2DExtent& pi_rObj) const;
    bool                      OuterContains(const HGF2DExtent& pi_rObj,
                                            double pi_Epsilon) const;

    IMAGEPP_EXPORT HGF2DExtent CalculateApproxExtentIn(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    string                    Serialize();

    static uint32_t           GetSerializationStrSize();

    bool              IsPointInnerIn(const HGF2DLocation& pi_rPoint) const;
    bool              IsPointOutterIn(const HGF2DLocation& pi_rPoint) const;
    bool              IsPointInnerIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const;
    bool              IsPointOutterIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const;

protected:

private:
public : // public because we use HPM_DECLARE_TYPE instead of HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,

    // Coordinates of this location
    double         m_XMin;
    double         m_YMin;
    double         m_XMax;
    double         m_YMax;

    bool               m_XDefined;
    bool               m_YDefined;

    // Coordinate system used to expressed previous coordinates
    HFCPtr<HGF2DCoordSys>   m_pCoordSys;

    };
END_IMAGEPP_NAMESPACE

#include "HGF2DExtent.hpp"
