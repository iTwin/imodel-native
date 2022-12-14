/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once



BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
//! The utilities for solar light.
// @bsiclass
//=======================================================================================
struct SolarUtility : public NonCopyableClass
{
public:

    //! Gets solar time range by location and time settings.
    //! @param[out] sunriseP    the time when sun rise.
    //! @param[out] sunsetP     the time when sum set.
    //! @param[in]  year        the year.
    //! @param[in]  month       the month.
    //! @param[in]  day         the day.
    //! @param[in]  dst         the daylight saving time.
    //! @param[in]  gmtOffset   the offset of Gmt.
    //! @param[in]  geoLocation the geographic location.
    //! @return ERROR if sun does not rise/set
    DGNPLATFORM_EXPORT static StatusInt GetSolarTimeRange
    (
    double*             sunriseP,
    double*             sunsetP,
    int                 year,
    int                 month,
    int                 day,
    int                 dst,
    double              gmtOffset,
    GeoPoint2d const&   geoLocation
    );

    //! Returns the solar direction by specified location and date.
    //! @param[in]  year        the year.
    //! @param[in]  month       the month.
    //! @param[in]  day         the day.
    //! @param[in]  hour        the hour.
    //! @param[in]  minute      the minute.
    //! @param[in]  dst         the daylight saving time.
    //! @param[in]  gmtOffset   the offset of Gmt.
    //! @param[in]  geoLocation the geographic location.
    //! @param[out] outAzimuth  the azimuth direction of the solar light
    //! @param[out] outAltitude the angle (in **radians**) of the Sun above the horizon (from 0-90 degrees).
    DGNPLATFORM_EXPORT static DVec3d GetSolarDirection
    (
    int                 year,
    int                 month,
    int                 day,
    int                 hour,
    int                 minute,
    bool                dst,
    double              gmtOffset,
    GeoPoint2d const&   geoLocation,
    DgnGCS const*       gcs,
    double*             outAzimuth = NULL,
    double*             outAltitude = NULL
    ) ;

    //! Gets the azimuth and altitude by specified solar direction.
    //! @param[out] azimuth    the azimuth angle in radians.
    //! @param[out] altitude   the altitude angle in radians.
    //! @param[in]  direction  the solar direction.
    DGNPLATFORM_EXPORT static void DirectionToAzimuthAngles
    (
    double&             azimuth,
    double&             altitude,
    DPoint3dCR          direction,
    DgnGCS const*       gcs
    );

    //! Gets solar direction by specified the azimuth and altitude.
    //! @param[out] direction  the return direction.
    //! @param[in]  azimuth    the azimuth angle in radians.
    //! @param[in]  altitude   the altitude angle in radians.
    DGNPLATFORM_EXPORT static DPoint3dR AzimuthAnglesToDirection
    (
    DPoint3dR           direction,
    double              azimuth,
    double              altitude,
    DgnGCS const*       gcs
    );

    //! Returns the solar direction by specified location and date.
    //! @param[in]  year        the year.
    //! @param[in]  month       the month.
    //! @param[in]  day         the day.
    //! @param[in]  hour        the hour.
    //! @param[in]  minute      the minute.
    //! @param[in]  dst         the daylight saving time.
    //! @param[in]  gmtOffset   the offset of Gmt.
    //! @param[in]  geoLocation the geographic location.
    //! @param[out] outAzimuth  the azimuth direction of the solar light
    //! @param[out] outAltitude the angle (in **radians**) of the Sun above the horizon (from 0-90 degrees).
    static DVec3d GetSolarDirection
    (
    int                 year,
    int                 month,
    int                 day,
    int                 hour,
    int                 minute,
    bool                dst,
    double              gmtOffset,
    GeoPoint2d const&   geoLocation,
    DgnDbR              db,
    double*             outAzimuth = NULL,
    double*             outAltitude = NULL
    ) { return GetSolarDirection(year, month, day, hour, minute, dst, gmtOffset, geoLocation, db.GeoLocation().GetDgnGCS(), outAzimuth, outAltitude); }

    //! Gets the azimuth and altitude by specified solar direction.
    //! @param[out] azimuth    the azimuth angle in radians.
    //! @param[out] altitude   the altitude angle in radians.
    //! @param[in]  direction  the solar direction.
    static void DirectionToAzimuthAngles
    (
    double&             azimuth,
    double&             altitude,
    DPoint3dCR          direction,
    DgnDbR              db
    ) { return DirectionToAzimuthAngles(azimuth, altitude, direction, db.GeoLocation().GetDgnGCS()); }

    //! Gets solar direction by specified the azimuth and altitude.
    //! @param[out] direction  the return direction.
    //! @param[in]  azimuth    the azimuth angle in radians.
    //! @param[in]  altitude   the altitude angle in radians.
    static DPoint3dR AzimuthAnglesToDirection
    (
    DPoint3dR           direction,
    double              azimuth,
    double              altitude,
    DgnDbR              db
    ) { return AzimuthAnglesToDirection(direction, azimuth, altitude, db.GeoLocation().GetDgnGCS()); }

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

