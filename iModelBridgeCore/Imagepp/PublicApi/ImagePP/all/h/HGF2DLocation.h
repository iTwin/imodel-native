//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLocation.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLocation
//-----------------------------------------------------------------------------
// Locations in two-dimensional coordinate systems.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HGF2DDisplacement.h"
#include "HGF2DPosition.h"


BEGIN_IMAGEPP_NAMESPACE
class HGF2DCoordSys;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class is used to describe locations in two-dimensional coordinate systems.
    It plays the role of an atomic "concrete data type" having no parent and not
    specifically designed to have child versions.

    The location is described by coordinate values labeled X and Y, each name
    corresponding to the dimensions of the coordinate system on which the values
    refer.  These values are expressed in a specific coordinate system described
    by a HGF2DCoordSys object to which a reference is kept in the location object.

    Both X and Y values are kept in the form of a double floating point value.
    The location does not assume that the coordinate system assigned to it is its
    propriety. The location assumes that many objects, of potentially many
    different kinds use this coordinate system. It does however assume that
    a coordinate system will not change after it has been assigned to it.
    The units for the X and Y dimensions will therefore always be the same as
    they were when this coordinate system was assigned. The coordinate system
    interface does not provide any methods for modifying the units, and this
    should remain so.

    This class also defines a class called HGF2DLocationCollection that is
    defined as an STL vector of HGF2DLocation objects:
        typedef vector<HGF2DLocation, allocator<HGF2DLocation> > HGF2DLocationCollection;
    Refer to the STL documentation for details, method description and behavior.

    Example:
    @code
        HFCPtr<HGF2DCoordSys>   pMyMainSystem (new HGF2DCoordSys());

        // An HGF2DStretch is a kind of HGF2DTransfoModel
        HGF2DStretch ImageToWorld(HGF2DDisplacement (10, 12),
                                  2.0, 2.0);

        HFCPtr<HGF2DCoordSys>   pImageCoordSys (new HGF2DCoordSys (ImageToWorld, pMyMainSystem));

        HGF2DLocation       ImageOrigin (0, 0, pImageCoordSys);

        HGF2DLocation       ImageWorld = ImageOrigin;
        HGF2DLocation       APoint (pMyMainSystem);

        // Example of ChangeCoordSys()
        ImageWorld.ChangeCoordSys (pMyMainSystem);

        // Example of ExpressedIN()
        HGF2DLocation        OtherImgWorld(ImageWorld.ExpressedIn(pMyMainSystem));

        // Example of GetCoordSys()
        HFCPtr<HGF2DCoordSys> pRefToSys = ImageOrigin.GetCoordSys();

        // Examples of Gets
        double RawX = ImageOrigin.GetX();
        double RawY = ImageOrigin.GetY();

        // Examples of compare operations
        if (ImageOrigin.IsEqualTo(OtherPoint))
        {
          ...
        }
        ...
        if (ImageOrigin.IsEqualToAutoEpsilon(OtherPoint))
        {
          ...
        }

        // Example of assignment
        HGF2DLocation OtherCoord = ImageOrigin;

        // Examples of Sets
        ImageWorld.Set(ImageOrigin);
        ImageWorld.SetX (12);
        ImageWorld.SetY (12);

        // Examples of operations
        HGF2DDisplacement MyDisplacement(10, 10);
        HGF2DLocation NewLocation = ImageOrigin + MyDisplacement;
        HGF2DDisplacement  MyOtherDisplacement = ImageWorld - ImageOrigin;
        HGF2DLocation OtherNewLocation = ImageOrigin - MyDisplacement;
        ImageOrigin += MyDisplacement;
        ImageOrigin -= MyDisplacement;
    @end

    -----------------------------------------------------------------------------
*/
class HGF2DLocation
    {


public:

    // Primary methods

    IMAGEPP_EXPORT                     HGF2DLocation();
    IMAGEPP_EXPORT                     HGF2DLocation  (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HGF2DLocation  (double pi_X,
                                              double pi_Y,
                                              const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
    IMAGEPP_EXPORT                    HGF2DLocation  (const HGF2DPosition& pi_rPoint,
                                              const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
    IMAGEPP_EXPORT                    HGF2DLocation  (const HGF2DLocation& pi_rObj,
                                              const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    IMAGEPP_EXPORT                    HGF2DLocation  (const HGF2DLocation& pi_rObj);
    IMAGEPP_EXPORT                    ~HGF2DLocation ();

    HGF2DLocation&     operator=(const HGF2DLocation& pi_rObj);

    // Compare operations
    IMAGEPP_EXPORT bool       operator==(const HGF2DLocation& pi_rObj) const;
    bool              operator!=(const HGF2DLocation& pi_rObj) const;
    bool              operator<(const HGF2DLocation& pi_rObj) const;

    // Compare operations with epsilon
    bool              IsEqualTo(const HGF2DLocation& pi_rObj) const;
    bool              IsEqualTo(const HGF2DLocation& pi_rObj, double pi_Epsilon) const;
    bool              IsEqualToAutoEpsilon(const HGF2DLocation& pi_rObj) const;
    bool              IsEqualToSCS(const HGF2DLocation& pi_rObj) const;
    bool              IsEqualToSCS(const HGF2DLocation& pi_rObj, double pi_Epsilon) const;
    bool              IsEqualToAutoEpsilonSCS(const HGF2DLocation& pi_rObj) const;

    // Location management
    double             GetX() const;
    double             GetY() const;
    void               SetX(double pi_NewRawValue);
    void               SetY(double pi_NewRawValue);
    void               Set(const HGF2DLocation& pi_rLocation);

    HGF2DPosition      GetPosition() const;

    IMAGEPP_EXPORT HGF2DLocation   operator+(const HGF2DDisplacement& pi_rOffset) const;
    friend HGF2DLocation   operator+(const HGF2DDisplacement& pi_rOffset,
              const HGF2DLocation& pi_rLocation);
    IMAGEPP_EXPORT HGF2DDisplacement    operator-(const HGF2DLocation& pi_rLocation) const;
    HGF2DLocation      operator-(const HGF2DDisplacement& pi_rOffset) const;

    IMAGEPP_EXPORT HGF2DLocation&        operator+=(const HGF2DDisplacement& pi_rOffset);
    HGF2DLocation&     operator-=(const HGF2DDisplacement& pi_rOffset);

    // Coord system management
    IMAGEPP_EXPORT void               ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
    const HFCPtr<HGF2DCoordSys>&     GetCoordSys() const;
    void               SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpSystem);
    HGF2DLocation      ExpressedIn(const HFCPtr<HGF2DCoordSys>& pi_rpSystem) const;

protected:

private:

    // Coordinates of this location
    double         m_XValue;
    double         m_YValue;

    // Coordinate system used to expressed previous coordinates
    HFCPtr<HGF2DCoordSys>   m_pCoordSys;


    };


typedef vector<HGF2DLocation, allocator<HGF2DLocation> > HGF2DLocationCollection;
END_IMAGEPP_NAMESPACE
#include "HGF2DCoordSys.h"
#include "HGF2DLocation.hpp"
