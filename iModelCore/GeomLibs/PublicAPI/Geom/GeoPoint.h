/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Geographic Point
struct GeoPoint
    {
    double              longitude;
    double              latitude;
    double              elevation;

#if defined (__cplusplus)
    //! Initializes a GeoPoint to the specified values
    //! @param longitudeValue       The longitude.
    //! @param latitudeValue        The latitude.
    //! @param elevationValue       The elevation.
    void Init (double longitudeValue, double latitudeValue, double elevationValue) {longitude = longitudeValue; latitude=latitudeValue; elevation = elevationValue;}
#endif
    };

//! Geographic 2D point
struct GeoPoint2d
    {
    double              longitude;
    double              latitude;

#if defined (__cplusplus)
    //! Initializes a GeoPoint to the specified values
    //! @param longitudeValue       The longitude.
    //! @param latitudeValue        The latitude.
    void Init (double longitudeValue, double latitudeValue) {longitude = longitudeValue; latitude = latitudeValue; }

    //! Creates a GeoPoint to the specified values
    //! @param longitudeValue       The longitude.
    //! @param latitudeValue        The latitude.
    static GeoPoint2d From(double longitudeValue, double latitudeValue) {GeoPoint2d pt; pt.longitude = longitudeValue; pt.latitude = latitudeValue; return pt;}


#endif
    };

//! Status values returned by GeoCoordinate System reproject methods, including Handler::_OnGeoCoordinateReprojection method

// NOTE: Some of the values in the ReprojectStatus enum are error codes from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy from it. Check it.
enum ReprojectStatus
    {
    REPROJECT_Success                               = 0,     //!< The element was successfully reprojected.
    REPROJECT_CSMAPERR_OutOfUsefulRange             = 1,     //!< The reprojection contains points that are outside of the useful range of the calculation, warning.
    REPROJECT_CSMAPERR_OutOfMathematicalDomain      = 2,     //!< The reprojection contains points that are outside of the mathematical domain of the calculation, failure.
    REPROJECT_CSMAPERR_DatumConverterNotSet         = 25,    //!< The datum conversion structure could not be set. This may be interpreted as a warning but user should be advised.
    REPROJECT_CSMAPERR_VerticalDatumConversionError = 26,     //!< Some error occured while applying vertical datum elevation correction. This is likely a vertcon or geoid config problem; may be interpreted as a warning
    REPROJECT_CSMAPERR_Error                        = 4096,  //!< The reprojection did not complete because there was a CSMap error.
    REPROJECT_BadArgument                           = 100,   //!< A bad argument passed to a Reproject method.
    REPROJECT_NoChange                              = 101,   //!< The element does not require reprojection.
    REPROJECT_StrokeError                           = 102,   //!< Unable to stroke element for reprojection.
    REPROJECT_DataError                             = 103,   //!< The data we are trying to reproject is incorrect.
    REPROJECT_DontValidateRange                     = 104,   //!< The reprojection worked, don't try to validate range.
    };


END_BENTLEY_GEOMETRY_NAMESPACE

#ifdef BENTLEYGEOMETRY_MakeGeoPoint2dPublic
#ifdef _WIN32
#pragma make_public (Bentley::GeoPoint2d)
#endif
#endif
