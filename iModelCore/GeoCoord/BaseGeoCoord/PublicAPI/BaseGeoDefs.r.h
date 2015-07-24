/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/BaseGeoDefs.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_END__*/

#define CS_ALBER    "AE"
#define CS_AZMEA    "AZMEA"
#define CS_AZEDE    "AZMED-ELEV"
#define CS_AZMED    "AZMED"
#define CS_BONNE    "BONNE"
#define CS_BPCNC    "BIPOLAR"
#define CS_CSINI    "CASSINI"
#define CS_EDCNC    "EDCNC"
#define CS_EDCYL    "EDCYL"
#define CS_EKRT4    "ECKERT4"
#define CS_EKRT6    "ECKERT6"
#define CS_GAUSK    "GAUSSK"
#define CS_GNOMC    "GNOMONIC"
#define CS_HMLSN    "GOODE"
#define CS_HOM1U    "HOM1UV"
#define CS_HOM2U    "HOM2UV"
#define CS_HUEOV    "OBQCYL"
#define CS_KRVK95   "KROVAK95"
#define CS_KRVKG    "KROVKG"
#define CS_KRVKP    "KROVAK"
#define CS_KRVKR    "KROVAK1"
#define CS_LMBRT    "LM"
#define CS_LM1SP    "LM1SP"
#define CS_LMBLG    "LMBLGN"
#define CS_LMTAN    "LMTAN"
#define CS_LMWIS    "LM-WCCS"
#define CS_LMMIN    "LM-MNDOT"
#define CS_MILLR    "MILLER"
#define CS_MODPC    "MODPC"
#define CS_MOLWD    "MOLLWEID"
#define CS_MRCAT    "MRCAT"
#define CS_MRCSR    "MRCATK"
#define CS_MSTRO    "MSTERO"
#define CS_NACYL    "NEACYL"
#define CS_NERTH    "NERTH"
#define CS_NESRT    "NERTH-SRT"
#define CS_NZLND    "NZEALAND"
#define CS_OBLQ1    "HOM1XY"
#define CS_OBLQ2    "HOM2XY"
#define CS_OCCNC    "OCCNC"
#define CS_ORTHO    "ORTHO"
#define CS_OSTRO    "OSTERO"
#define CS_OST02    "OSTN02"
#define CS_OST97    "OSTN97"
#define CS_PLYCN    "PLYCN"
#define CS_PSTRO    "PSTERO"
#define CS_PSTSL    "PSTEROSL"
#define CS_ROBIN    "ROBINSON"
#define CS_RSKEW    "RSKEW"
#define CS_RSKWC    "RSKEWC"
#define CS_RSKWO    "RSKEWO"
#define CS_SINUS    "SINUS"
#define CS_SOTRM    "SOTM"
#define CS_STERO    "OSTEROUS"
#define CS_SWISS    "SWISS"
#define CS_S3499    "SYSTM34-99"
#define CS_SYS34    "SYSTM34"
#define CS_TACYL    "TEACYL"
#define CS_TMWIS    "TM-WCCS"
#define CS_TMMIN    "TM-MNDOT"
#define CS_TRMER    "TM"
#define CS_TMKRG    "TRMRKRG"
#define CS_TRMAF    "TMAF"
#define CS_UNITY    "LL"
#define CS_UTMZN    "UTM"
#define CS_VDGRN    "VDGRNTN"
#define CS_WINKT    "WINKEL"
#define CS_LMBRTAF  "LMAF"

#define CS_UTMZNBF "UTMZN-BF"     // TOTAL_SPECIAL - Transverse Mercator using the Bernard Flaceliere calculation.
#define CS_TRMERBF "TRMER-BF"
#define CS_S3401    "SYSTM34-01"     
#define CS_EDCYLE   "EDCYL-E"
#define CS_PCARREE  "PCARREE"
#define CS_MRCATPV  "MRCAT-PV"
#define CS_MNDOTOBL "OBL-MNDOT"


#define MSGLISTID_GeoCoordErrors        1
#define MSGLISTID_GeoCoordNames         2
#define MSGLISTID_DgnGeoCoordStrings    3

// Coordinate system identifiers. Do NOT renumber these as they are stored in the coordinate system element
// These are the values that are saved in the element. They are not the values that CS_Map uses. See
enum    DgnProjectionTypes 
    {
    COORDSYS_NONE       =    0,
    COORDSYS_ALBER      =    1, // (Cone) Albers' Equal Area Conic
    COORDSYS_AZMEA      =    2, // (Azim) Azimuthal Equal Area
    COORDSYS_AZMED      =    3, // (Azim) Azimuthal Equidistant
    COORDSYS_BONNE      =    4, // (Cone) Bonne
    COORDSYS_BPCNC      =    5, // (Cone) Bipolar Conic
    COORDSYS_CSINI      =    6, // (Cyln) Cassini
    COORDSYS_EDCNC      =    7, // (Cone) Equidistant Conic
    COORDSYS_EDCYL      =    8, // (Cyln) Equidistant Cylindrical (Equirectangular)
    COORDSYS_EKRT4      =    9, // (Cyln) Eckert 4
    COORDSYS_EKRT6      =   10, // (Cyln) Eckert 6
    COORDSYS_GNOMC      =   11, // (Azim) Gnomonic
    COORDSYS_HMLSN      =   12, // (Othr) Homolsine
    COORDSYS_LMBRT      =   13, // (Cone) Lambert Conformal Conic
    COORDSYS_LMTAN      =   14, // (Cone) Lambert Tangential
    COORDSYS_MILLR      =   15, // (Cyln) Miller
    COORDSYS_MODPC      =   16, // (Cone) Modified Polyconic
    COORDSYS_MOLWD      =   17, // (Othr) Mollweide
    COORDSYS_MRCAT      =   18, // (Cyln) Mercator
    COORDSYS_MSTRO      =   19, // (Azim) Modified Stereographic
    COORDSYS_NACYL      =   20, // (Cyln) Normal Equal Area Cylindrical
    COORDSYS_NZLND      =   21, // (Cyln) New Zealand
    COORDSYS_OBLQ1      =   22, // (Cyln) Oblique Mercator 1 Point
    COORDSYS_OBLQ2      =   23, // (Cyln) Oblique Mercator 2 Points
    COORDSYS_ORTHO      =   24, // (Azim) Orthographic
    COORDSYS_PLYCN      =   25, // (Cone) Polyconic
    COORDSYS_ROBIN      =   26, // (Othr) Robinson
    COORDSYS_SINUS      =   27, // (Othr) Sinusoidal
    COORDSYS_STERO      =   28, // (Azim) Stereographic  (Synder)
    COORDSYS_TACYL      =   29, // (Cyln) Transverse Equal Area Cylindrical
    COORDSYS_TRMER      =   30, // (Cyln) Transverse Mercator
    COORDSYS_UNITY      =   31, // (Geog) Geographic Latitude/Longitude
    COORDSYS_VDGRN      =   32, // (Othr) Van Der Grinten

    COORDSYS_UTMZN      =   33, // (Othr) Universal Transverse Mercator
    COORDSYS_LM1SP      =   34, // (Cone) Lambert Conf. Conic 1 std parallel
    COORDSYS_OSTRO      =   35, // (Azim) Oblique Sterographic Projection
    COORDSYS_PSTRO      =   36, // (Azim) Polar Sterographic Projection
    COORDSYS_RSKWC      =   37, // (Cyln) Rectified Skew Orthomorphic Origin=Center
    COORDSYS_RSKEW      =   38, // (Cyln) Rectified Skew Orthomorphic Origin=Intersec
    COORDSYS_SWISS      =   39, // (Cyln) Swiss Oblique Cylindrical Projection
    COORDSYS_LMBLG      =   40, // (Cone) Belgian Lambert Conformal Conic
    COORDSYS_SOTRM      =   41, // (Cyln) South Orientated Trans Merc. (RSA)
    COORDSYS_HOM1U      =   42, // (Cyln) Oblique Mercator 1 Point (unrectified)
    COORDSYS_HOM2U      =   43, // (Cyln) Oblique Mercator 2 Point (unrectified)

    COORDSYS_GAUSK      =   46, // (Cyln) Transverse Mercator with scale red=1.0
    COORDSYS_KRVKP      =   47, // (Cone) Czech Krovak, with precise origin
    COORDSYS_KRVKR      =   48, // (Cone) Czech Krovak, with rounded origin
    COORDSYS_MRCSR      =   49, // (Cyln) Standard Mercator with scale reduc. factor
    COORDSYS_OCCNC      =   50, // (Cone) Oblique conformal conic
    COORDSYS_KRVKG      =   51, // (Cone) Czech Krovak, all params customizable
    COORDSYS_TRMAF      =   52, // (Cyln) Transverse Mercator with affine post proc.
    COORDSYS_PSTSL      =   53, // (Azim) Polar Stereographic with standard latitude
    COORDSYS_NERTH      =   54, // (Othr) Non-earth projection for non-datum ops
    COORDSYS_SPCSL      =   55, // (Vars) State Plane Coordinate System Library

    COORDSYS_HUEOV      =   56, // (Cyln) Hungarian EOV Oblique Cylindrical Projection
    COORDSYS_SYS34      =   57, // (Cyln) Danish System 1934/1945
    COORDSYS_OST97      =   58, // (Cyln) OSTN97 Ordnance Survey 1997 Great Britain
    COORDSYS_OST02      =   59, // (Cyln) OSTN02 Ordnance Survey 2002 Great Britain
    COORDSYS_S3499      =   60, // (Cyln) Danish System 1934/1945 with 1999 polynomials*/
    COORDSYS_AZEDE      =   61, // (Azim) Azimuthal Equi-Distant, Elevated ellipsoid
    COORDSYS_KEYNM      =   62, // (Vars) Keyname specifies entire coordinate system
    COORDSYS_LMMIN      =   63, // (Cone) Minnesota DOT Lambert Conformal Conic
    COORDSYS_LMWIS      =   64, // (Cone) Wisconsin County Lambert Conformal Conic
    COORDSYS_TMMIN      =   65, // (Cyln) Minnesota DOT Transverse Mercator
    COORDSYS_TMWIS      =   66, // (Cyln) Wisconsin County Transverse Mercator
    COORDSYS_RSKWO      =   67, // (Cyln) RSKEW except azimuth applies to projn origin
    COORDSYS_WINKT      =   68, // (Othr) Winkel-Tripel, variable standard latitude
    COORDSYS_TMKRG      =   69, // (Cyln) Transverse Mercator Kruger Formulation
    COORDSYS_NESRT      =   70, // (Othr) Non-earth scale, rotation then translation
    COORDSYS_FAVOR      =   71, // (Othr) Favorites - User Keyname Selections
    COORDSYS_LMBRTAF    =   72, // (Cone) Lambert with affine post proc.

    COORDSYS_UTMZNBF    =   73, // (Othr) Universal Transverse Mercator Zone, using TOTAL's Bernard Flaceliere calculation
    COORDSYS_TRMERBF    =   74, // (Othr) Transverse Mercator using TOTAL's Bernard Flaceliere calculation

    COORDSYS_S3401     =    75, // (Cyln) Danish System 1934/1945 with 2001 polynomials*/
    COORDSYS_EDCYLE    =    76, // (Cyln) Equidistant Cylindrical Ellipsoid variation
    COORDSYS_PCARREE   =    77, // (Cyln) Simple Cylindrical Plate Carree
    COORDSYS_MRCATPV   =    78, // (Cyln) Mercator Popular Visuualization Variation
    COORDSYS_MNDOTOBL  =    79, // (Cyln) Minnesoat DOT Oblique Mercator

    COORDSYS_Geographic =   COORDSYS_UNITY, // Geographic Latitude/Longitude
    COORDSYS_StatePlane =   101,    // (Vars) Obsolete, use COORDSYS_SPCSL
    COORDSYS_UTM        =   102,    // (Cyln) Obsolete, use COORDSYS_UTMZN
    };


/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_NAMESPACE

#if !defined (mdl_resource_compiler) && !defined (mdl_type_resource_generator)
namespace GeoCoordinates {
#endif

// Error codes that we generate, separate from CS_Map error codes. These are all negative, in the range -100000 to -200000.
#define GeoCoordErrorBase       -100000
#define GeoCoordErrorTiffBase   -101000
#define GeoCoordErrorEnd        -110000

enum  BaseGeoCoordErrors
    {
    // General errors
    GEOCOORDERR_InvalidCoordSys                 = (GeoCoordErrorBase  - 1),
    GEOCOORDERR_BadArg                          = (GeoCoordErrorBase  - 2),
    GEOCOORDERR_LibraryReadonly                 = (GeoCoordErrorBase  - 3),
    GEOCOORDERR_IOError                         = (GeoCoordErrorBase  - 4),
    GEOCOORDERR_CoordSysNotFound                = (GeoCoordErrorBase  - 5),
    GEOCOORDERR_CoordSysNoUniqueName            = (GeoCoordErrorBase  - 6),
    GEOCOORDERR_CoordSysIllegalName             = (GeoCoordErrorBase  - 7),
    GEOCOORDERR_InvalidCoordinateCode           = (GeoCoordErrorBase  - 8),
    GEOCOORDERR_InvalidUnitCode                 = (GeoCoordErrorBase  - 9),
    GEOCOORDERR_InvalidDatumCode                = (GeoCoordErrorBase  - 10),
    GEOCOORDERR_CantSetEllipsoid                = (GeoCoordErrorBase  - 11),
    GEOCOORDERR_InvalidEllipsoidCode            = (GeoCoordErrorBase  - 12),
    GEOCOORDERR_ProjectionDoesntUseParameter    = (GeoCoordErrorBase  - 13),
    GEOCOORDERR_NoModelContainsGCS              = (GeoCoordErrorBase  - 14),
    GEOCOORDERR_CoordinateRange                 = (GeoCoordErrorBase  - 15),
    GEOCOORDERR_InvalidAffineParameters         = (GeoCoordErrorBase  - 16),
    GEOCOORDERR_CantSetVerticalDatum            = (GeoCoordErrorBase  - 17),
    GEOCOORDERR_VerticalDatumConversion         = (GeoCoordErrorBase  - 18),
    GEOCOORDERR_StringTooLong                   = (GeoCoordErrorBase  - 19),

    GEOCOORDERR_DatumIllegalName                = (GeoCoordErrorBase  - 30),
    GEOCOORDERR_DatumNotFound                   = (GeoCoordErrorBase  - 31),
    GEOCOORDERR_DatumNoUniqueName               = (GeoCoordErrorBase  - 32),
    GEOCOORDERR_DatumInUse                      = (GeoCoordErrorBase  - 33),
    GEOCOORDERR_MaxUserLibraryDatums            = (GeoCoordErrorBase  - 34),
    GEOCOORDERR_ParameterNotUsed                = (GeoCoordErrorBase  - 35),
    GEOCOORDERR_InvalidDatum                    = (GeoCoordErrorBase  - 36),
    GEOCOORDERR_NotInUserLibrary                = (GeoCoordErrorBase  - 37),

    GEOCOORDERR_EllipsoidIllegalName            = (GeoCoordErrorBase  - 40),
    GEOCOORDERR_EllipsoidNotFound               = (GeoCoordErrorBase  - 41),
    GEOCOORDERR_EllipsoidNoUniqueName           = (GeoCoordErrorBase  - 42),
    GEOCOORDERR_EllipsoidInUse                  = (GeoCoordErrorBase  - 43),
    GEOCOORDERR_MaxUserLibraryEllipsoids        = (GeoCoordErrorBase  - 44),
    GEOCOORDERR_InvalidEllipsoid                = (GeoCoordErrorBase  - 45),

    // GeoTiff read errors:
    GEOCOORDERR_GeocentricNotSupported          = (GeoCoordErrorTiffBase - 1),
    GEOCOORDERR_IncompleteGeoTiffSpec           = (GeoCoordErrorTiffBase - 2),
    GEOCOORDERR_UnexpectedGeoTiffModelType      = (GeoCoordErrorTiffBase - 3),
    GEOCOORDERR_UnexpectedGeoTiffPrimeMeridian  = (GeoCoordErrorTiffBase - 4),
    GEOCOORDERR_UnrecognizedLinearUnit          = (GeoCoordErrorTiffBase - 5),
    GEOCOORDERR_UnrecognizedAngularUnit         = (GeoCoordErrorTiffBase - 6),
    GEOCOORDERR_BadEllipsoidDefinition          = (GeoCoordErrorTiffBase - 7),
    GEOCOORDERR_ProjectionGeoKeyNotSupported    = (GeoCoordErrorTiffBase - 8),
    GEOCOORDERR_CoordTransNotSupported          = (GeoCoordErrorTiffBase - 9),
    GEOCOORDERR_CoordParamNotNeededForTrans     = (GeoCoordErrorTiffBase - 10),
    GEOCOORDERR_ProjectionParamNotSupported     = (GeoCoordErrorTiffBase - 11),
    GEOCOORDERR_CoordSysSpecificationIncomplete = (GeoCoordErrorTiffBase - 12),
    GEOCOORDERR_CoordParamRedundant             = (GeoCoordErrorTiffBase - 13),

    // GeoTiff write errors:
    GEOCOORDERR_CantSaveGCS                     = (GeoCoordErrorTiffBase  - 102),
    GEOCOORDERR_InvalidGeographicEPSGCode       = (GeoCoordErrorTiffBase  - 103),
    };

enum    DgnGeoCoordStrings
    {
    DGNGEOCOORD_Msg_ElementTypeName                         = 1,
    DGNGEOCOORD_Msg_ReprojectingCoordinateData              = 2,
    DGNGEOCOORD_Msg_ReprojectedPointsWithErrors             = 3,
    DGNGEOCOORD_Msg_ReprojectedPointsWithWarnings           = 4,
    DGNGEOCOORD_Msg_ReprojectedPoints                       = 5,
    DGNGEOCOORD_Msg_PointsReprojectedDetail                 = 6,
    DGNGEOCOORD_Msg_DomainErrors                            = 7,
    DGNGEOCOORD_Msg_UsefulRangeErrors                       = 8,
    DGNGEOCOORD_Msg_OtherErrors                             = 9,
    DGNGEOCOORD_Msg_DatumError                              = 10,
    DGNGEOCOORD_Msg_GeoCoordACSType                         = 11,
    DGNGEOCOORD_Msg_NoGeoCoordinateSystem                   = 12,
    DGNGEOCOORD_Msg_PointFromStringRequiresBoth             = 13,
    DGNGEOCOORD_Msg_UnparseableInputAngle                   = 14,
    DGNGEOCOORD_Msg_UnparseableInputElevation               = 15,
    DGNGEOCOORD_Msg_SubstituteLinearTransform               = 16,
    DGNGEOCOORD_Msg_SubstituteLinearTransformDetails        = 17,

    DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesName           = 30,
    DGNGEOCOORD_Msg_MiltaryGridCoordinatesName              = 31,
    DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Name         = 32,
    DGNGEOCOORD_Msg_USNationalGridName                      = 33,

    DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesDescription    = 34,
    DGNGEOCOORD_Msg_MiltaryGridCoordinatesDescription       = 35,
    DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Description  = 36,
    DGNGEOCOORD_Msg_USNationalGridDescription               = 37,

    DGNGEOCOORD_Msg_MilitaryGridACSType                     = 38,

    DGNGEOCOORD_Msg_MilitaryGridNotRelative                 = 40,
    DGNGEOCOORD_Msg_MilitaryGridNotDelta                    = 41,
    DGNGEOCOORD_Msg_CantConvertToMilitaryGrid               = 42, 
    DGNGEOCOORD_Msg_CantConvertFromMilitaryGrid             = 43,

    DGNGEOCOORD_Msg_DatumConvertNotSetErrors                = 44,
    DGNGEOCOORD_Msg_VerticalConvertErrors                   = 45,

    };

#if !defined (mdl_resource_compiler) && !defined (mdl_type_resource_generator)
};

#endif
END_BENTLEY_NAMESPACE
