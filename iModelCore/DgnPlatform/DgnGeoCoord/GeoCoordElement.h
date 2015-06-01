/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnGeoCoord/GeoCoordElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined (mdl_type_resource_generator) || defined (mdl_resource_compiler)
    #include <Mstn\MicroStation.r.h>
#endif

// Note: In Geographics, the Geo Coordinate type 66 was converted to and from file format in a most unbelievably idiotic and errnoeous way.
//       The net result was that the type 66 had a struct starting at offset 0 consisting of the GeoCoordType66Base structure, stored in
//       packed little endian format, followed by the union in GeoCoordType66Union starting at offset [858] in packed little endian format.
// #define MIZAR_66        0x00b6   // This definition has been superseded by MSGEOCOORD_SIGNATURE in mselems.h
#define GCOORD_66_MAS   0x1000      // Master coordinate system subsignature.
#define GCOORD_66_ALT   0x2000      // Alternate coordinate system subsignature.

#define GCOORD_MINOR_VERSION_460            460
#define GCOORD_MINOR_VERSION_420            420
#define GCOORD_MINOR_VERSION_4000           4000

#define GCOORD_COMPATIBLE_08117_VERSION         2      // this version is usable pre 08.11.09, so we create it when we can.
#define GCOORD_COMPATIBLE_08117_MINOR_VERSION   4000

#define GCOORD_08119_VERSION                    3      // has either local transform or the Datum or Ellipsoid comes from a user library - pre 08.11.09 ignores it.
#define GCOORD_08119_MINOR_VERSION              0

#define OBE_HEMISPHERE_NORTH  0
#define OBE_HEMISPHERE_SOUTH  1

// dimensions within the structures. CANNOT BE CHANGED - STORED IN ELEMENTS.
#define CHRSZ           2
#define NAMSZ           24
#define DSCSZ           64
#define UNTSZ           16
#define DATSZ           32
#define ELLSZ           32
#define NUM_ZONE        8
#define NUM_CMPLX       12  // Size of ABarray of complex numbers.

#define SOURCELIB_USER  (0x2)

typedef struct GeoCoordType66
    {
    int32_t     gcoord66_size;          // Size in Bytes of this structure.  If we find something we do not expect here, we should stop
    int32_t     version;                // Version Number
    uint32_t    deprec_fPos;            // last known position of type 66 element. Not used, and as of 8.9.4 not set. I don't see how this was ever usable.
    int16_t     unused1;                // added to account for abovementioned errors.
    int16_t     unused2;                // added to account for abovementioned errors.

    double      e_rad;                  // Equatorial radius of the reference ellipsoid in meters.
    double      p_rad;                  // Polar radius of the reference ellipsoid in meters.  This value is ignored for
                                        // projections that support only the sphere.  Set this value to be the same as
                                        // e_rad to select the spherical form of projections which support both the ellipsoidal and spherical forms.

                                        // The 7 values below are the Datum definition.  Note, full name and key name are defined below in the character data
                                        // section.  Note, the datum key name given below is used to determine if, and access
                                        // as appropriate, Multiple Regression data files. While these should exist on all
                                        // systems, and be the same on all systems, this is a potential for differences in
                                        // datums shift results on different systems.

    double      delta_X;                // X component of the bvector from the WGS-84 geocenter to the geocenter of this datum, in meters.
    double      delta_Y;                // Y component of the bvector from the WGS-84 geocenter to the geocenter of this datum, in meters.
    double      delta_Z;                // Z component of the bvector from this WGS-84 geocenter to the geocenter of this datum, in meters.

                                        // The following parameters are not always available.  If not known, set to zero.
                                        // This will cause a standard Molodensky transformation and is quite normal.

    double      rot_X;                  // Angle from WGS-84 X axis to local geodetic system X axis in arc seconds, east is positive.
    double      rot_Y;                  // Angle from WGS-84 Y axis to local geodetic system Y axis in arc seconds, north is positive.
    double      rot_Z;                  // Angle from WGS-84 Z axis to local geodetic system Z axis in arc seconds, use right hand rule.
    double      bwscale;                // Bursa-Wolfe Scale factor in parts per million.

                                        // From version 8.9.4 on, we write but never use these fields. They are redundant with data already stored in the design file.
    DPoint3d    deprec_globorg;         // global origin - UORs
    uint32_t    deprec_mastunits;       // master units per design - this is never used and is always 0 as of 8.9.4
    uint32_t    deprec_subpermast;      // sub-units per master unit (unsigned)
    uint32_t    deprec_uorpersub;       // uors per sub unit (unsigned). This isn't necessarily an integer in MicroStation V8, as of 8.9.4 deprecated.

    uint32_t    protect;                // If true then this cannot be changed by end user
    int32_t     projType;               // which member of union proj to use
    int32_t     family;                 // *** OBSOLETE ***  This is now a derived field.  Don't delete the field or else alignment problems
    int32_t     coordsys;               // Actual coordinate system we base on
    char        deprec_mastname[CHRSZ]; // master units name, truncated to two characters only. As of 8.9.4, we set it but never use it.*/
    char        deprec_subname[CHRSZ];  // sub units name, truncated to two characters only. As of 8.9.4, we set it but never use it.
    char        cs_knm[NAMSZ];          // Key name for coordinate system
    char        grp_knm[NAMSZ];         // Key name of projection group (UTMN,SPCS27,etc)
    char        zone_knm[NAMSZ];        // Name of zone within group (egs: 9, AL83-E, CO-C. Used only for UTM and State Plane coordinate systems.
    char        prj_knm[NAMSZ];         // Key name of the projection upon which this coordinate system is based.  Indicates which element of the union below is active.
    char        prj_nm[DSCSZ];          // Complete descriptive name of the projection.
    char        dat_nm[DSCSZ];          // Complete descriptive name of the referenced datum if any.
    char        ell_nm[DSCSZ];          // Complete descriptive name of the referenced  ellipsoid.  There should always be one of these, directly or indirectly
                                        // through a datum reference.  In the case of the Unity projection, a reference ellipsoid still needed as the radius is
                                        // is used to compute scale factors for stuff like text height, etc.
    char        desc_nm [DSCSZ];        // Complete descriptive name of the coordinate system deing defined.
    char        source [DSCSZ];         // The source from which the information used to define this coordinate system was obtained.

                                        // From version 8.9.4 on, we write but never use this field. It is redundant with data already stored in the design file.
    char        deprec_dgnUnitNm[UNTSZ];// 16-character version of the name of the linear unit for dgn master units

    uint32_t    to84_via;               // Conversion technique, one of: cs_DTCTYP_MOLO, cs_DTCTYP_BURSA, cs_DTCTYP_MREG, cs_DTCTYP_NAD27, cs_DTCTYP_NAD83
                                        // cs_DTCTYP_WGS84, cs_DTCTYP_WGS72 cs_DTCTYP_HPGN,  cs_DTCTYP_LCLGRF
    int32_t     minor_version;
    int32_t     attributes;
    int32_t     prj_code;               // CSMap equivalent to projType (different values)
    uint16_t    verticalDatum;          // A member of the the VertDatumCode enumerator. Ignored if verticalDatumValid not set correctly.
    uint16_t    verticalDatumValid;     // This must have value 0x8117 to verify that that verticalDatum is valid. That's because we can't say for that filler was always 0'ed

    uint16_t    dtmOrElpsdFromUserLib;  // If this is equal to SOURCELIB_USER, either the datum of the ellipsoid of this GCS is from a user library. That triggers differ processing.
    uint16_t    localTransformType;     // a member of the LocalTransformType enum - only valid if version >= 
    double      transformParams[12];
    char        filler[156];            // So that subsequent growth will not alter the stored element size
    } GeoCoordType66;

typedef struct _domainLL
    {
    double       min_lng;
    double       min_lat;
    double       max_lng;
    double       max_lat;
    } DomainLL;

typedef struct _domainEN
    {
    double         min_x;
    double         min_y;
    double         max_x;
    double         max_y;
    } DomainEN;

typedef struct _gcDomain
    {
    DomainLL  domLL;
    DomainEN  domEN;
    } GCDomain;

typedef struct _cmplx
    {
    double          real;
    double           img;
    } CMPLX;

typedef struct _zone
    {
    double     cntrl_mer; // Central meridian of the zone.
    double    east_bndry; // Eastern boundary of the zone, longitude.
    double    west_bndry; // Western boundary of the zone, longitude.
    double   north_south; // Sign indicates north of equator, negative
                          // south of the equator.  This may become
                          // more complex in the future.
    } ZONE;

// Each of the following structures is used to carry the definition of a coordinate system based on a specific
// projection.  Many elements of these structure are the same from one projection to the next.  However, each is
// carried separately so each structure represents a complete coordinate system definition.

// Unless otherwise noted, all latitude and longitude values are in degrees relative to Greenwich/Equator where negative values
// represent west longitude and south latitude.  Longitude values are eventually adjusted to be within the range of > -180
// and <= +180.0.  Similarly, all latitudes are eventually adjusted to be in the range of => -90 and <= +90.0.  Azimuth angles are
// always given in degrees east of north (i.e. negative means  west of north) and are eventually adjusted to be in the range.
// of > -90.0 and <= +90.0.

// These are stored as a union in the GeoCoordinate Type 66 element following the (pakced) GeoCoordType66, at offset 858.
// (Deduced from looking at the type 66's from GeoGraphics in the old version.

//    COORDSYS_ALBER  Albers' Equal Area Conic
typedef struct Proj_alber
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lat_1; // Latitude of the first of two standard parallels, the essence of a conic projection.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two, although this is not required.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad; // x/y increases- 1-ri/up 2-le/up 3-le/dw 4-ri/dw
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Alber;

// COORDSYS_AZMEA  Equal Area Azimuthal Projection (Lambert).
typedef struct Proj_azmea
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double            az; // The azimuth of the Y axis relative to true north.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Azmea;

//    Azimuthal Equidistant Projection.
typedef struct Proj_azmed
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double            az; // The azimuth of the Y axis relative to true north.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Azmed;

//    Azimuthal Equidistant Projection with elevated ellipsoid
typedef struct Proj_azede
    {
    double       ellElev;   // Ellipsoid elevation
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double            az; // The azimuth of the Y axis relative to true north.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Azede;

// COORDSYS_BONNE  Bonne Projection.
typedef struct Proj_bonne
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];

    } Proj_Bonne;

// COORDSYS_BPCNC  Bipolar Oblique Conformal Conic projection.
typedef struct Proj_bpcnc
    {
    double         lng_1;   // Longitude of pole A
    double         lat_1;   // Latitude  of pole A
    double         lng_2;   // Longitude of pole B
    double         lat_2;   // Latitude  of pole B
    double       pole_dd;   // Angular distance between the two poles
    double          sp_1;   // Arc dist. fr either pole to 1st std parallel
    double          sp_2;   // Arc dist. fr either pole to 2nd std parallel
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Bpcnc;

// COORDSYS_CSINI  Cassini Projection.
typedef struct Proj_csini
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Csini;

// COORDSYS_EDCNC  Equidistant Conic Projection.
typedef struct Proj_edcnc
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lat_1; // Latitude of the first of two standard parallels.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two, although this is not required.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Edcnc;

// COORDSYS_EDCYL  Equidistant Cylindrical Projection.
typedef struct Proj_edcyl
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       std_lat; // Standard latitude, i.e. latitude of true scale.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Edcyl;

// COORDSYS_EKRT4  Eckert IV projection.
typedef struct Proj_ekrt4
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Ekrt4;

// COORDSYS_EKRT6  Eckert VI projection.
typedef struct Proj_ekrt6
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Ekrt6;

// COORDSYS_GNOMC  Gnomonic Projection.
typedef struct Proj_gnomc
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Gnomc;

// COORDSYS_HMLSN  Homolsine projection.
typedef struct Proj_hmlsn
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    ZONE zone [NUM_ZONE]; // The specifications of each of up to eight different zones.
    ZONE    reserved [4]; // Reserve space if we expand number of zones
    long          nZones; // Number of zones specified.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Hmlsn;

// COORDSYS_HUEOV Hungarian EOV projection.
typedef struct Proj_hueov
    {
    double       org_lng;
    double       org_lat; // Where Y is zero before false origin applied
    double       std_lat; // Normal Parallel determines Gaussian sphere radius
    double       scl_red; // Scale reduction factor
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Hueov;

// COORDSYS_KRVKG       Czech Krovak, with fields for generalized params
// COORDSYS_KRVKP       Czech Krovak, with precise origin
// COORDSYS_KRVKR       Czech Krovak, with rounded origin
typedef struct Proj_krvak
    {
    GCDomain       gcDom;
    double     paper_scl;

    double       std_lat; // Latitude [degree] central standard parallel
    double      pole_lng; // Longitude[degree] of oblique cone pole
    double      pole_lat; // Latitude [degree] of oblique cone pole
    double       org_lng; // Longitude[degree] of origin (where X is zero)
    double       org_lat; // Latitude [degree] of origin (where Y is zero)
    double       scl_red; // Scale reduction factor of lat on oblique cone
    double         x_off; // False Easting in specified units
    double         y_off; // False Northing in specified units

    long         apply95; // if <> 0, Apply S-JTSK/95 x, y offset adjustment
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Krvak;

//  COORDSYS_LMBRT, COORDSYS_LMBLG  Lambert Conformal Conic
typedef struct Proj_lmbrt
    {
    double       org_lng; // Origin longitude of the coordinate system, i.e. where X is zero before false origin is applied.  Central Meridian in many projections.
    double         x_off; // False easting, in the units of the coordinate system.
    double         y_off; // False northing, in the units of the coordinate system.
    GCDomain       gcDom;
    double     paper_scl; // The scale of the map.  E.g. 24000.0 for a 1:24,000 scale map.  This is set to a value other than 1.0 only when the
                          // coordinate system is to be, for example, inches on the map rather than real world numbers.

    double         lat_1; // Latitude of the first of two standard parallels, the essence of the Lambert Conformal Conic projection.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two although this is not really necessary.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad; // Quadrant of the cartesian coordinates. Used to handle coordinate systems in which X increases to the left, etc.  The integer
                          // value indicates the quadrant in which both X and Y are positive.  Zero and one refer to the normal right handed quadrant.
                          // Quadrants are numbered counterclockwise thereafter.
    char unit_nm [UNTSZ]; // Name of the linear unit for the cartesian system.
    char dat_knm [DATSZ]; // Key name of the datum on which this coordinate system is based, else the null string if this coordinate system
                          // is cartographically referenced rather than geodetically referenced.
    char ell_knm [ELLSZ]; // Key name of the ellipsoid on which this coordinate system is based, else the null string is this coordinate system is
                          // geodetically referenced rather than cartographically referenced.
    } Proj_Lmbrt;


// COORDSYS_LMTAN, COORDSYS_LM1SP  Lambert Tangential Projection.
typedef struct Proj_lmtan
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude where Y is zero before x_off
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Lmtan;

//  COORDSYS_LMMIN
typedef struct Proj_lmmin
    {
    double       ellElev; // (prm3) Average elevation above ellipsoid in system units
    double       org_lng; // Origin longitude of the coordinate system, i.e. where X is zeroi before flase origin is applied.  Refered to
                          // as the Central Meridian in many projections.
    double         x_off; // False easting, in the units of the coordinate system.
    double         y_off; // False northing, in the units of the coordinate system.
    GCDomain       gcDom;
    double     paper_scl; // The scale of the map.  E.g. 24000.0 for a 1:24,000 scale map.  This is set to a value other than 1.0 only
                          // when the coordinate system is to be, for example, inches on the map rather than real world numbers.

    double         lat_1; // Latitude of the first of two standard parallels, the essence of the Lambert Conformal Conic projection.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two although this is not really necessary.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad; // Quadrant of the cartesian coordinates. Used to handle coordinate systems in which X increases to the left, etc.  The integer
                          // value indicates the quadrant in which both X and Y are positive.  Zero and one refer to the normal right handed quadrant.
                          // Quadrants are numbered counterclockwise thereafter.
    char unit_nm [UNTSZ]; // Name of the linear unit for the cartesian system.
    char dat_knm [DATSZ]; // Key name of the datum on which this coordinate system is based, else the null string if this coordinate system
                          // is cartographically referenced rather than geodetically referenced.
    char ell_knm [ELLSZ]; // Key name of the ellipsoid on which this coordinate system is based, else the null string is this coordinate system is
                          // geodetically referenced rather than cartographically referenced.
    } Proj_Lmmin;


//  COORDSYS_LMWIS
typedef struct Proj_lmwis
    {
    double      geoidSep; // (prm3) Average geoid in METERS
    double     geoidElev; // (prm4) Average elevation above geoid in system units
    double       org_lng; // Origin longitude of the coordinate system, i.e. where X is zeroi before flase origin is applied.  Refered to
                          // as the Central Meridian in many projections.
    double         x_off; // False easting, in the units of the coordinate system.
    double         y_off; // False northing, in the units of the coordinate system.
    GCDomain       gcDom;
    double     paper_scl; // The scale of the map.  E.g. 24000.0 for a 1:24,000 scale map.  This is set to a value other than 1.0 only
                          // when the coordinate system is to be, for example, inches on the map rather than real world numbers.

    double         lat_1; // Latitude of the first of two standard parallels, the essence of the Lambert Conformal Conic projection.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two although this is not really necessary.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad; // Quadrant of the cartesian coordinates. Used to handle coordinate systems in which X increases to the left, etc.  The integer
                          // value indicates the quadrant in which both X and Y are positive.  Zero and one refer to the normal right handed quadrant.
                          // Quadrants are numbered counterclockwise thereafter.
    char unit_nm [UNTSZ]; // Name of the linear unit for the cartesian system.
    char dat_knm [DATSZ]; // Key name of the datum on which this coordinate system is based, else the null string if this coordinate system
                          // is cartographically referenced rather than geodetically referenced.
    char ell_knm [ELLSZ]; // Key name of the ellipsoid on which this coordinate system is based, else the null string is this coordinate system is
                          // geodetically referenced rather than cartographically referenced.
    } Proj_Lmwis;

// COORDSYS_MRCAT  Standard Mercator Projection.
typedef struct Proj_mrcat
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       std_lat; // Latiude of the standard parallel of the system.  This is usually zero for most uses of the Mercator.  Non-zero values have the
                          // same affect as scale reduction in the Transverse Mercator and other Cylindricals. The sign has no meaning, since use the cosine of this angle.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Mrcat;

// COORDSYS_MRCSR Mercator Projection with scale reduction parameter
typedef struct Proj_mrcsr
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Mrcsr;

// COORDSYS_MILLR  Miller Cylindrical Projection.
typedef struct Proj_millr
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Millr;

// COORDSYS_MODPC  Modified Polyconic
typedef struct Proj_modpc
    {
    double       org_lng; // Central Meridian
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lng_1; // Longitude of the eastern standard meridian. Typically two degrees east of central meridian.
    double         lat_2; // Latitude of the southern standard parallel. This value also serves as origin latitude.
    double         lat_1; // Latitude of the northern standard parallel.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Modpc;

// COORDSYS_MOLWD  Mollweide projection.
typedef struct Proj_molwd
    {
// ToDo: Check to see if org_lng is applicable here. may be included with zone?
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    ZONE zone [NUM_ZONE]; // The specifications of each of up to eight different zones.
    ZONE    reserved [4]; // Reserve space if we expand number of zones
    long          nZones; // Number of zones specified.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Molwd;


// COORDSYS_MSTRO  Modified Stereographic Projection.
typedef struct Proj_mstro
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    CMPLX ABary[NUM_CMPLX];//Coefficients for the complex series expansion, up to 12th order.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Mstro;

// COORDSYS_NACYL  Normal Authalic (Equal Area) Cylindrical Projection.
typedef struct Proj_nacyl
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       std_lat; // Latitude of the standard parallel of the system.  This is usually zero for most uses of the Mercator.  Non-zero values have the
                          // same affect as scale reduction in the Transverse Mercator and other Cylindricals. The sign has no meaning, sicne only the cosine is used.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Nacyl;

// COORDSYS_NERTH  Non-Earth projection
typedef struct Proj_nerth
    {
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ]; // Expected to be null.  Use Ellipsoid
    char ell_knm [ELLSZ];
    } Proj_Nerth;

// COORDSYS_NESRT  Non-Earth projection with scale, rotation then translation
typedef struct Proj_nesrt
    {
    double         x_off;  // X displacement to take place after scale and rotation applied
    double         y_off;  // Y displacement to take place after scale and rotation applied
    GCDomain       gcDom;
    double     paper_scl;  // Unused
    double         scale;  // Scale factor to be applied to coordinates as first step of transform
    double     rotateDeg;  // Degrees counter clockwise.  Appled after scale
    double      x_origin;  // X Origin about which scale, rotate then translation will take place
    double      y_origin;  // Y Origin about which scale, rotate then translation will take place

    long            quad;  // Unused
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];  // Expected to be null.  Use Ellipsoid
    char ell_knm [ELLSZ];
    } Proj_Nesrt;


// COORDSYS_NZLND  New Zealand National Grid.
typedef struct Proj_nzlnd
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Nzlnd;


// COORDSYS_OBLQ1, COORDSYS_RSKWC, COORDSYS_RSKEW , COORDSYS_RSKWO, COORDSYS_HOM1U
// Hotine Oblique Mercator, One point mode.
typedef struct Proj_oblq1
    {
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lng_c; // Longitude of the center of the map.
    double         lat_c; // Latitude  of the center of the map.
    double            az; // The azimuth of the projection.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.
    long       rectified; // TRUE says rectify cartesian coordinates.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Oblq1;

// COORDSYS_OBLQ2, COORDSYS_HOM2U  Hotine Oblique Mercator, Two point mode.
typedef struct Proj_oblq2
    {
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lng_1; // Longitude of the first point.
    double         lat_1; // Latitude of the first point.
    double         lng_2; // Longitude of the second point.
    double         lat_2; // Latitude of the second point.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.
    long       rectified; // TRUE says rectify cartesian coordinates.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Oblq2;

// COORDSYS_MNDOTOBL
// Minnesota DOT Oblique Mercator
typedef struct Proj_mndotobl
    {
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double         lng_c; // Longitude of the center of the map.
    double         lat_c; // Latitude  of the center of the map.
    double            az; // The azimuth of the projection.
    double       ellElev; // (prm4) elevation above ellipsoid in system units
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.
    long       rectified; // TRUE says rectify cartesian coordinates.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Mndotobl;

// COORDSYS_ORTHO  Orthographic Projection.
typedef struct Proj_ortho
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Ortho;

// COORDSYS_PLYCN  American Polyconic Projection.
typedef struct Proj_plycn
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];

    } Proj_Plycn;

// COORDSYS_SINUS  Sinusoidal projection.
typedef struct Proj_sinus
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    ZONE zone [NUM_ZONE]; // The specifications of each of up to eight different zones.
    ZONE    reserved [4]; // Reserve space if we expand number of zones
    long          nZones; // Number of zones specified.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Sinus;

// COORDSYS_STERO, COORDSYS_OSTRO, COORDSYS_PSTRO  Stereographic Projection.
typedef struct Proj_stero
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double            az; // The azimuth of the Y axis relative to true north.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Stero;

// COORDSYS_OST02
// COORDSYS_OST97
typedef struct Proj_ost97
    {
    GCDomain       gcDom;
    double     paper_scl;
    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Ost97;

// COORDSYS_PSTSL, Polar Stereographic with Standard Latitude Projection.
typedef struct Proj_pstsl
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       std_lat; // Standard latitude, i.e. latitude of true scale.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Pstsl;

// COORDSYS_ROBIN  Robinson projection.
typedef struct Proj_robin
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Robin;


typedef struct Proj_swiss
    {
    // COORDSYS_SWISS  The Swiss projection.
    // A double projection where the ellipsoid is projected onto a conformal sphere, ala Gauss, where integration constants are choosen to produce grid
    // scale of 1 at the central point. The pole of the conformal sphere is then tilted such that the equator of the tilted system passes through the
    // central point. The tilted sphere is then projected on to a cylinder ala Mercator.

    double       org_lng;
    double       org_lat; // Where Y is zero before false origin applied
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Swiss;

typedef struct Proj_sys34
    {
    // COORDSYS_S3401  Danish 1934/1945 with KMS 2001 Polynomials
    // COORDSYS_S3499  Danish 1934/1945 with KMS 1999 Polynomials
    // COORDSYS_SYS34  Danish 1934/1945 non-KMS

    long          zoneNo;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    long        variation;   // cs_PRJCOD_SYS34_99 or 0
    } Proj_Sys34;

#if defined (DONTNEED)
//  The following structure exists because state plane selections do not determine the underlying projType until it gets looked up in the dictionary.
//  Unfortunately, I need a place to store information in order to proceed with the lookup. After projtype is determined, it can be copied to that
//  location.   dbb 960618.

typedef struct Proj_statePlane
    {
    long        projType; // which member of union proj to use
    double     paper_scl;
    char zone_knm[NAMSZ];
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_StatePlane;
#endif

// COORDSYS_TACYL  Transverse Authalic (Equal Area) Cylindrical
typedef struct Proj_tacyl
    {
    double       org_lng; // This should be referred to as the central meridian in this projection.
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Tacyl;


// COORDSYS_TMMIN
typedef struct Proj_tmmin
    {
    double       org_lng; // This should be referred to as the central meridian in this projection.
    double       ellElev; // (prm2) Average elevation above ellipsoid in system units
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Tmmin;

// COORDSYS_TMWIS
typedef struct Proj_tmwis
    {
    double       org_lng; // This should be referred to as the central meridian in this projection.
    double      geoidSep; // (prm2) Average geoid in METERS
    double     geoidElev; // (prm3) Average elevation above geoid in system units
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Tmwis;

// COORDSYS_TRMAF  Transverse Mercator with affine processor
typedef struct Proj_trmaf
    {
    double       org_lng; // This should be referred to as the central meridian in this projection.
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    double     affineA0;  // Coefficient for affine transformation
    double     affineA1;  // Coefficient for affine transformation
    double     affineA2;  // Coefficient for affine transformation
    double     affineB0;  // Coefficient for affine transformation
    double     affineB1;  // Coefficient for affine transformation
    double     affineB2;  // Coefficient for affine transformation

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Trmaf;

// COORDSYS_TRMER, COORDSYS_SOTRM, COORDSYS_GAUSK  Transverse Mercator
typedef struct Proj_trmer
    {
    double       org_lng; // This should be referred to as the central meridian in this projection.
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.
    double       scl_red; // The scale reduction applied to the central meridian, also known as the Scale of the Central Meridian.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Trmer;

// COORDSYS_UNITY  Lat/long Geographic
typedef struct Proj_unity
    {
    double       gwo_lng; // The longitude of the location, relative to Greenwich, of the prime meridian of the system being defined.
    double       gwo_lat; // The latitude of the location, relative to the equator, of the prime parallel of the system being defined.  Always zero, is
                          // here for consistency sake.
    DomainEN       domEN;
    char unit_nm [UNTSZ]; // Name of the angular unit in which latitude and longitude are to be reported.
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    double     paper_scl; // Added to version 5.5.1.42 July/97 dbb+no

    DomainLL       domLL;
    double      user_min; // prm1 Must be in units of degrees.  Range user
    double      user_max; // prm2  range user wants (0-360 or -270 - 270.)
    long            quad;
    } Proj_Unity;

// COORDSYS_UTMZN  Universal Transverse Mercator
typedef struct Proj_utmzn
    {
    GCDomain       gcDom;
    double     paper_scl;

    long          zoneNo; // 1-60
    long            quad;
    long      hemisphere; // 0=Northern, 1=Southern (Note: csmap -1=S, 1=N)
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Utmzn;

// COORDSYS_VDGRN  Van Der Grinten projection.
typedef struct Proj_vdgrn
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Vdgrn;

// COORDSYS_WINKT  Winkel-Tripel Projection
typedef struct Proj_winkt
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       std_lat;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Winkt;

//  COORDSYS_LMBRTAF, COORDSYS_LMBLG  Lambert Conformal Conic
typedef struct Proj_lmbrtaf
    {
    double       org_lng; // Origin longitude of the coordinate system, i.e. where X is zero before false origin is applied.  Central Meridian in many projections.
    double         x_off; // False easting, in the units of the coordinate system.
    double         y_off; // False northing, in the units of the coordinate system.
    GCDomain       gcDom;
    double     paper_scl; // The scale of the map.  E.g. 24000.0 for a 1:24,000 scale map.  This is set to a value other than 1.0 only when the
                          // coordinate system is to be, for example, inches on the map rather than real world numbers.

    double         lat_1; // Latitude of the first of two standard parallels, the essence of the Lambert Conformal Conic projection.
    double         lat_2; // Latitude of the second standard parallel, usually the most northerly of the two although this is not really necessary.
    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

                          // This seemingly random order is what cs_map uses. Don't ask me why.
    double     affineA0;  // Coefficient for affine transformation
    double     affineB0;  // Coefficient for affine transformation
    double     affineA1;  // Coefficient for affine transformation
    double     affineA2;  // Coefficient for affine transformation
    double     affineB1;  // Coefficient for affine transformation
    double     affineB2;  // Coefficient for affine transformation

    long            quad; // Quadrant of the cartesian coordinates. Used to handle coordinate systems in which X increases to the left, etc.  The integer
                          // value indicates the quadrant in which both X and Y are positive.  Zero and one refer to the normal right handed quadrant.
                          // Quadrants are numbered counterclockwise thereafter.
    char unit_nm [UNTSZ]; // Name of the linear unit for the cartesian system.
    char dat_knm [DATSZ]; // Key name of the datum on which this coordinate system is based, else the null string if this coordinate system
                          // is cartographically referenced rather than geodetically referenced.
    char ell_knm [ELLSZ]; // Key name of the ellipsoid on which this coordinate system is based, else the null string is this coordinate system is
                          // geodetically referenced rather than cartographically referenced.
    } Proj_Lmbrtaf;

// COORDSYS_PCARREE  Simple Cylindrical / Plate Carree
typedef struct Proj_pcarree
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    double       org_lat; // Origin latitude of the coordinate system, i.e. where Y is zero before false origin is applied.

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Pcarree;

// COORDSYS_MRCATPV  Popular Visualization Pseudo Mercator Projection.
typedef struct Proj_mrcatpv
    {
    double       org_lng;
    double         x_off;
    double         y_off;
    GCDomain       gcDom;
    double     paper_scl;

    long            quad;
    char unit_nm [UNTSZ];
    char dat_knm [DATSZ];
    char ell_knm [ELLSZ];
    } Proj_Mrcatpv;

typedef union ProjectionParams /* Coordinate System specifics */
    {
    Proj_Alber      alber;  /* COORDSYS_ALBER */
    Proj_Azede      azede;  /* COORDSYS_AZMED */
    Proj_Azmea      azmea;  /* COORDSYS_AZMEA */
    Proj_Azmed      azmed;  /* COORDSYS_AZMED */
    Proj_Bonne      bonne;  /* COORDSYS_BONNE */
    Proj_Bpcnc      bpcnc;  /* COORDSYS_BPCNC */
    Proj_Csini      csini;  /* COORDSYS_CSINI */
    Proj_Edcnc      edcnc;  /* COORDSYS_EDCNC */
    Proj_Edcyl      edcyl;  /* COORDSYS_EDCYL */
    Proj_Ekrt4      ekrt4;  /* COORDSYS_EKRT4 */
    Proj_Ekrt6      ekrt6;  /* COORDSYS_EKRT6 */
    Proj_Trmer      gausk;  /* COORDSYS_GAUSK */
    Proj_Gnomc      gnomc;  /* COORDSYS_GNOMC */
    Proj_Hmlsn      hmlsn;  /* COORDSYS_HMLSN */
    Proj_Hueov      hueov;  /* COORDSYS_HUEOV */
    Proj_Krvak      krvkg;  /* COORDSYS_KRVKG */
    Proj_Krvak      krvkp;  /* COORDSYS_KRVKP */
    Proj_Krvak      krvkr;  /* COORDSYS_KRVKR */
    Proj_Lmtan      lm1sp;  /* COORDSYS_LMTAN */
    Proj_Lmbrt      lmbrt;  /* COORDSYS_LMBRT */
    Proj_Lmbrt      lmblg;  /* COORDSYS_LMBRT */
    Proj_Lmtan      lmtan;  /* COORDSYS_LMTAN */
    Proj_Lmmin      lmmin;  /* COORDSYS_LMMIN */
    Proj_Lmwis      lmwis;  /* COORDSYS_LMWIS */
    Proj_Millr      millr;  /* COORDSYS_MILLR */
    Proj_Modpc      modpc;  /* COORDSYS_MODPC */
    Proj_Molwd      molwd;  /* COORDSYS_MOLWD */
    Proj_Mrcat      mrcat;  /* COORDSYS_MRCAT */
    Proj_Mrcsr      mrcsr;  /* COORDSYS_MRCSR */
    Proj_Mstro      mstro;  /* COORDSYS_MSTRO */
    Proj_Nacyl      nacyl;  /* COORDSYS_NACYL */
    Proj_Nerth      nerth;  /* COORDSYS_NERTH */
    Proj_Nesrt      nesrt;  /* COORDSYS_NESRT */
    Proj_Nzlnd      nzlnd;  /* COORDSYS_NZLND */
    Proj_Oblq1      oblq1;  /* COORDSYS_OBLQ1 */
    Proj_Oblq2      oblq2;  /* COORDSYS_OBLQ2 */
    Proj_Ortho      ortho;  /* COORDSYS_ORTHO */
    Proj_Stero      ostro;  /* COORDSYS_STERO */
    Proj_Ost97      ost02;  /* COORDSYS_STERO */
    Proj_Ost97      ost97;  /* COORDSYS_STERO */
    Proj_Plycn      plycn;  /* COORDSYS_PLYCN */
    Proj_Stero      pstro;  /* COORDSYS_PSTRO */
    Proj_Pstsl      pstsl;  /* COORDSYS_PSTSL */
    Proj_Robin      robin;  /* COORDSYS_ROBIN */
    Proj_Oblq1      rskwc;  /* COORDSYS_RSKWC */
    Proj_Oblq1      rskew;  /* COORDSYS_RSKEW */
    Proj_Oblq1      rskwo;  /* COORDSYS_RSKWO */
    Proj_Sinus      sinus;  /* COORDSYS_SINUS */
    Proj_Trmer      sotrm;  /* COORDSYS_SOTRM */
    Proj_Stero      stero;  /* COORDSYS_STERO */
    Proj_Swiss      swiss;  /* COORDSYS_SWISS */
    Proj_Sys34      s3499;  /* COORDSYS_S3499 */
    Proj_Sys34      sys34;  /* COORDSYS_SYS34 */
    Proj_Tacyl      tacyl;  /* COORDSYS_TACYL */
    Proj_Tmmin      tmmin;  /* COORDSYS_TMMIN */
    Proj_Tmwis      tmwis;  /* COORDSYS_TMWIS */
    Proj_Trmaf      trmaf;  /* COORDSYS_TRMAF */
    Proj_Trmer      trmer;  /* COORDSYS_TRMER */
    Proj_Trmer      tmkrg;  /* COORDSYS_TMKRG */
    Proj_Unity      unity;  /* COORDSYS_UNITY */
    Proj_Utmzn      utmzn;  /* COORDSYS_UTMZN */
    Proj_Vdgrn      vdgrn;  /* COORDSYS_VDGRN */
    Proj_Winkt      winkt;  /* COORDSYS_WINKT */
    Proj_Lmbrtaf    lmbrtaf;/* COORDSYS_LMBRTAF */
    Proj_Sys34      s3401;  /* COORDSYS_S3401 */
    Proj_Pcarree    pcarree;  /* COORDSYS_PCARREE */
    Proj_Mrcatpv    mrcatpv;  /* COORDSYS_MRCATPV */
    Proj_Mndotobl   mndotobl; /* COORDSYS_MNDOTOBL */
    double sizer[80];       /* Force a minimum size of the union. */
    } ProjectionParams;



