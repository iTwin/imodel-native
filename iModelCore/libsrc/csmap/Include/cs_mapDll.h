/*
	The following define the return value possibilities from
	the coordinate conversion functions.

	The first three defines are bit map values.  These are
	returned by the CS_xychk function.

	The second three are simple status values returned by the
	forward and inverse conversion functions.
*/

/* The following typedef's were introduced to CS-MAP to accommodate 64 bit
   architecture.  That is, a long integer in 64 bit compilers tend to be 64
   bits Variables.  Many uses of long within CS-MAP, due to its heritage,
   explicitly require a long to be 32 bits.  Thus, we have the following two
   typedefs which need to produce variable of 32 bits when the library is
   created, and when the library is consumed by an application. */
typedef unsigned ulong32_t;
typedef int long32_t;

/* The following define the specific numeric values returned as status
   by the High Performance API (aka: 'C' interface) conversion functions. */
#define cs_CNVRT_OK    0
#define cs_CNVRT_USFL  1
#define cs_CNVRT_DOMN  2
#define cs_CNVRT_ERR   4096

#define cs_CNVRT_NRML  cs_CNVRT_OK
#define cs_CNVRT_INDF  cs_CNVRT_USFL
#define cs_CNVRT_RNG   cs_CNVRT_DOMN

/* The following define the bits allocated for the return value for the
   High Level Interface (aka: VB interface). The return value is an exclusive
   OR of all appropriate bits. */
#define cs_BASIC_SRCRNG 1	/* Source coordinate is outside the
							   domain of the source coordinate
							   system. */
#define cs_BASIC_DSTRNG 2	/* The intermediate geographic coordinate,
							   after possible datum shifting, is
							   outside the domain of the destination
							   (target) coordinate system. */
#define cs_BASIC_DTCWRN 4	/* The intermediary geographic coordinate
							   produced by the source coordinate system
							   was outside the domain of one or more
							   of the datum shift algorithms required
							   to produce the desired result. */

/* The following are the error control codes which are used to control how the
   datum conversion routines deal with errors.  There is one set for datums,
   i.e. dealing	with unsupported datum changes, and one set for a data block
   which is not found, i.e. required data is not present on the system. */
#define cs_DTCFLG_DAT_F  0              /* Fatal */
#define cs_DTCFLG_DAT_W  1              /* Warning */
#define cs_DTCFLG_DAT_W1 2              /* Warn Once */
#define cs_DTCFLG_DAT_I  3              /* Ignore */

#define cs_DTCFLG_BLK_F 0               /* Fatal */
#define cs_DTCFLG_BLK_W 1               /* Warning */
#define cs_DTCFLG_BLK_1 2               /* Warn, once per block */
#define cs_DTCFLG_BLK_I 3               /* Ignore */

/* Two types of units are currently supported. One of these constants is a
   required argument to the CS_unitlu and CS_unEnum functions. */
#define cs_UTYP_LEN 'L'		/* Linear units. */
#define cs_UTYP_ANG 'R'		/* Angular units. */

/* The following defines are used in the Double/ASCII conversion functions
   CS_atof and CS_ftoa.  CS_atof returns a long which contains the following
   information.  CS_ftoa accepts a long with this information and uses it to
   format the ASCII result.  Thus, results can easily be formatted pretty
   mush the same as the provided input. */
#define cs_ATOF_PRCMSK    ( 0x1F)	/* # of digits, plus one, after the
									   decimal point (zero value reserved
									   as a special flag indicating automatic
									   precision determination on conversion
									   to ASCII. */
#define cs_ATOF_VALLNG  (1L <<  5)	/* Ignored by ftoa. Set by atof if
									   value is valid for longitude */
#define cs_ATOF_VALLAT  (1L <<  6)	/* Ignored by ftoa. Set by atof if
									   value is valid for latitude. */
#define cs_ATOF_MINSEC  (1L <<  7)	/* minutes and seconds were used */
#define cs_ATOF_MINUTE  (1L <<  8)	/* minutes only were used */
#define cs_ATOF_FRACTN  (1L <<  9)	/* a fractional portion was present */
#define cs_ATOF_EXPNT   (1L << 10)	/* value was in 1.0E-01 form */
#define cs_ATOF_OVRFLW  (1L << 11)	/* value is in *.* (overflow) form */
#define cs_ATOF_COMMA   (1L << 12)	/* Commas were present */
#define cs_ATOF_DIRCHR  (1L << 13)	/* A valid directional character was
									   present. */
#define cs_ATOF_XEAST   (1L << 14)	/* True indicates that the value
									   should be considered a longitude,
									   easting, or X value for the
									   purposes of choosing a directional
									   character. */
#define cs_ATOF_MINSEC0 (1L << 15)	/* Include leading zeros on minutes and
									   seconds. */
#define cs_ATOF_DEG0    (1L << 16)	/* Include leading zeros on degrees. */
#define cs_ATOF_0BLNK   (1L << 17)	/* Output a zero value as a null string.
									   On input, set for a null string input. */
#define cs_ATOF_FORCE3	(1L << 18)	/* Forces 3 characters for degrees
									   when deg/min/sec output is used
									   for output. Useful in formatting
									   longitudes. */
#define cs_ATOF_RATIO   (1L << 19)	/* Forces output as a ratio, i.e.
									   1:17,000. Other flags, such as
									   COMA and precision apply to the
									   numeric value. */
/* The following bit specifications are used by CS_atof for detailed error
   reporting input format errors.  Note that the sign bot of the return value
   is set in the event of an error, bits 21 through 26 indicate the nature
   of the error. */
#define cs_ATOF_SECS60  (1L << 20)	/* What was expected to be seconds
									   evaluated to a value >= 60 */
#define cs_ATOF_MINS60  (1L << 21)	/* What was expected to be minutes
									   evaluated to a value >= 60 */
#define cs_ATOF_MLTPNT  (1L << 22)	/* Multiple decimal point indications
									   were present */
#define cs_ATOF_MLTSGN  (1L << 23)	/* Multiple sign indications were
									   present. */
#define cs_ATOF_ERRCMA  (1L << 24)	/* Commas were encountered where
									   unexpected. */
#define cs_ATOF_FMTERR  (1L << 25)	/* General format error. */
#define cs_ATOF_RATERR  (1L << 26)	/* Ratio format error. */
#define cs_ATOF_ERRFLG  (0x80000000L)	/* General error flag. */
/* Bits 27 thru 30 are reserved for future use by CS_MAP developers. */

/* Some commonly used default format specifications for formatting coordinates,
   scale factors, and convergence angles. */
#define cs_ATOF_XXXDFLT (cs_ATOF_COMMA  | 4L)
#define cs_ATOF_YYYDFLT (cs_ATOF_COMMA  | 4L)
#define cs_ATOF_LNGDFLT (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST | cs_ATOF_FORCE3 | 4L)
#define cs_ATOF_LATDFLT (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | 4L)
#define cs_ATOF_SCLDFLT (9L)
#define cs_ATOF_CNVDFLT (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST | 3L)

/* The following values appear in the prj_code element of the cs_Csprm_ structure */
#define cs_PRJCOD_END       0		/* Value not set yet, end of table marker. */
#define cs_PRJCOD_UNITY     1		/* No projection,, indicates a gepgraphic coordinate system. */
#define cs_PRJCOD_TRMER     3		/* General Transverse Mercator */
#define cs_PRJCOD_ALBER     4		/* Albers Equal Area Conic */
#define cs_PRJCOD_OBLQM     5		/* Hotine Oblique Mercator / Rectified Skew Orthomorphic, see below */ 
#define cs_PRJCOD_MRCAT     6		/* Traditional Mercator */
#define cs_PRJCOD_AZMED     7		/* Lambert Equi-Distant Azimuthal */
#define cs_PRJCOD_LMTAN     8		/* Lambert Conformal Conic, Tangential Form */
#define cs_PRJCOD_PLYCN     9		/* American Polyconic */
#define cs_PRJCOD_MODPC    10		/* Lallemand IMW Modified Polyconic */
#define cs_PRJCOD_AZMEA    11		/* Lambert Equal Area Azimuthal */
#define cs_PRJCOD_EDCNC    12		/* Equi-Distant Conic */
#define cs_PRJCOD_MILLR    13		/* Miller */
#define cs_PRJCOD_MSTRO    15		/* Modified Stereographic */
#define cs_PRJCOD_NZLND    16		/* New Zealand National Grid */
#define cs_PRJCOD_SINUS    17		/* Sinusdoidal */
#define cs_PRJCOD_ORTHO    18		/* Orthogrpahic */
#define cs_PRJCOD_GNOMC    19		/* Gnomonic */
#define cs_PRJCOD_EDCYL    20		/* Equi-Distant Cylindrical [DEPRECATED] */
#define cs_PRJCOD_VDGRN    21		/* Vasn der Grinten */
#define cs_PRJCOD_CSINI    22		/* Cassini */
#define cs_PRJCOD_ROBIN    23		/* Robinson */
#define cs_PRJCOD_BONNE    24		/* Bonne Conic */
#define cs_PRJCOD_EKRT4    25		/* Ekert Pseudocylindrical, Number IV */
#define cs_PRJCOD_EKRT6    26		/* Ekert Pseudocylindrical, Number VI */
#define cs_PRJCOD_MOLWD    27		/* Mollweide */
#define cs_PRJCOD_HMLSN    28		/* Goode Homolosine */
#define cs_PRJCOD_NACYL    29		/* Normal Aspect, Equal Area Cylindrical */
#define cs_PRJCOD_TACYL    30		/* Transverse Aspect, Equal Area Cylindrical */
#define cs_PRJCOD_BPCNC    31		/* Bi-Polar Conformal Conic */
#define cs_PRJCOD_SWISS    32		/* Oblique Cylindrical Projection (aka: Rosenmund, Swiss) */
#define cs_PRJCOD_PSTRO    33		/* Polar sterographic */
#define cs_PRJCOD_OSTRO    34		/* Oblique Sterographic */
#define cs_PRJCOD_SSTRO    35		/* Synder's Oblique Sterographic */
#define cs_PRJCOD_LM1SP    36		/* Single standard parallel variation
									   of the Lambert Conformal Conic. */
#define cs_PRJCOD_LM2SP    37		/* Double standard parallel variation
									   of the Lambert Conformal Conic. */
#define cs_PRJCOD_LMBLG    38		/* Belgian variation of the Lambert
									   Conformal Conic Projection. */
#define cs_PRJCOD_WCCSL    39		/* Wisconsin County Coordinate System
									   variation of the Lambert Conformal
									   Conic. */
#define cs_PRJCOD_WCCST    40		/* Wisconsin County Coordinate System
									   variation of the Transverse Mercator
									   projection. */
#define cs_PRJCOD_MNDOTL   41		/* Minnesota Department of Transportation
									   variation of the Lambert Conformal
									   Conic. */
#define cs_PRJCOD_MNDOTT   42		/* Minnesota Department of Transportation
									   variation of the Transverse Mercator
									   projection. */
#define cs_PRJCOD_SOTRM    43		/* South Oriented variation of the
									   Transverse Mercator Projection. */
#define cs_PRJCOD_UTM      44		/* The UTM direct variation of the
									   Transverse Mercator projection. */
#define cs_PRJCOD_TRMRS    45		/* Transverse Mercator precisely per J. P.
										Snyder. */
#define cs_PRJCOD_GAUSSK   46		/* Gauss-Kruger: Transverse Mercator without
									   scale reduction parameter. */
#define cs_PRJCOD_KROVAK   47		/* Czech Krovak, with precise origin. */
#define cs_PRJCOD_KROVK1   48		/* Obsolete, do not reuse.  Retained for
									   compatibility with previous releases
									   only. Czech Krovak, with rounded origin. */
#define cs_PRJCOD_MRCATK   49		/* Standard Mercator with a scale reduction
									   factor instead of a standard parallel. */
#define cs_PRJCOD_KRVK95   51		/* Czech Krovak, with precise origin, includes
									   S-JTSK/95 adjustment. */
#define cs_PRJCOD_KRVK951  52		/* Obsolete, do not reuse.  Retained for
									   compatibility with previous releases
									   only. Czech Krovak, with rounded origin,
									   includes S-JTSK/95 adjustment. */
#define cs_PRJCOD_PSTROSL  53		/* Polar sterographic with standard latitude */
#define cs_PRJCOD_TRMERAF  54		/* Transverse Mercator with affiine post-processor. */
#define cs_PRJCOD_NERTH    55		/* Non-georeferenced coordinate system.  Named
									   Non-Earth by Map Info. */
#define cs_PRJCOD_OBQCYL   56		/* Oblique Cylindrical, a generalized version of the
									   Swiss projection, specifically for Hungary. */
#define cs_PRJCOD_SYS34    57		/* Combination of Transverse Mercator and a polynomial
									   expansion used in Denmark. */
#define cs_PRJCOD_OSTN97   58		/* The Transverse Mercator with specific parameters,
									   with the OSTN97 grid shift tacked on.  This is
									   a combination of a projection and a datum shift. */
#define cs_PRJCOD_AZEDE    59		/* Azimuthal Equi-Distant, Elevated ellipsoid. */
#define cs_PRJCOD_OSTN02   60		/* The Transverse Mercator with specific parameters,
									   with the OSTN02 grid shift tacked on.  This is
									   a combination of a projection and a datum shift. */
#define cs_PRJCOD_SYS34_99 61		/* Combination of Transverse Mercator and polynomial
									   expansion used in Denmark.  Polynominals are of the
									   1999 vintage. */
#define cs_PRJCOD_TRMRKRG  62		/* Variation of the Transverse Mercator which uses the
									   Kruger Formulation. */
#define cs_PRJCOD_WINKL    63		/* Winkel-Tripel projection. */
#define cs_PRJCOD_NRTHSRT  64		/* Nerth with scale and rotation. (NERTH == Non ERTH ala MapInfo) */
#define cs_PRJCOD_LMBRTAF  65		/* Lambert Conformal Conic with affiine post-processor. */
#define cs_PRJCOD_SYS34_01 66		/* Combination of Transverse Mercator and polynomial
									   expansion used in Denmark.  Polynominals are of the
									   1999 vintage, except for Bornholm, which are post
									   1999. */
#define cs_PRJCOD_EDCYLE   67		/* Equidistant Cylindrical, ellipsoid form supported.
									   This variation replaces the original variation which
									   only supported the psherical form of this projection. */
#define cs_PRJCOD_PCARREE  68		/* Plate Carree, standard form.  This is _NOT_ the same
									   as EPSG 9825 - Pseudo Plate Carree. */
#define cs_PRJCOD_MRCATPV  69		/* Psuedo Mercator, Popular Visualization. */
#define cs_PRJCOD_LMMICH   70		/* Lambert Conformal Conic,.Michigan Variation */

#ifdef GEOCOORD_ENHANCEMENT
#define cs_PRJCOD_KROVAKMOD 71      /* Krovak Modified for Czech republic */
#endif

/* Variations of the basic Oblique Mercator concept. */
#define cs_PRJCOD_HOM1UV   ((cs_PRJCOD_OBLQM << 8) + 1)
#define cs_PRJCOD_HOM1XY   ((cs_PRJCOD_OBLQM << 8) + 2)	/* Hotine Oblique Mercator, Single Point Form */
#define cs_PRJCOD_HOM2UV   ((cs_PRJCOD_OBLQM << 8) + 3)
#define cs_PRJCOD_HOM2XY   ((cs_PRJCOD_OBLQM << 8) + 4)	/* Hotine Oblique Mercator, Two Point Form */
#define cs_PRJCOD_RSKEW    ((cs_PRJCOD_OBLQM << 8) + 5)	/* Rectified Skew Orthomorphic */
#define cs_PRJCOD_RSKEWC   ((cs_PRJCOD_OBLQM << 8) + 6)	/* Rectified Skew Orthomorphic, Origin at Center */

/* Bit maps of the following appear in the prj_flag element of the cs_Csprm_ structure. */
#define cs_PRJFLG_SPHERE  (1L <<  0)	/* Sphere supported. */
#define cs_PRJFLG_ELLIPS  (1L <<  1)	/* Ellipsoid supported. */

#define cs_PRJFLG_SCALK   (1L <<  2)	/* Analytical K available */
#define cs_PRJFLG_SCALH   (1L <<  3)	/* Analytical H available */
#define cs_PRJFLG_CNVRG   (1L <<  4)	/* Analytical C available */

#define cs_PRJFLG_CNFRM   (1L <<  5)	/* Conformal */
#define cs_PRJFLG_EAREA   (1L <<  6)	/* Equal area */
#define cs_PRJFLG_EDIST   (1L <<  7)	/* Equal distant, either h or
										   k is always 1. */
#define cs_PRJFLG_AZMTH   (1L <<  8)	/* Azimuthal */
#define cs_PRJFLG_GEOGR   (1L <<  9)	/* Geographic coordinates */

					/* Modifiers to surface type: */
#define cs_PRJFLG_OBLQ    (1L << 10)	/* Oblique */
#define cs_PRJFLG_TRNSV   (1L << 11)	/* Transverse */
#define cs_PRJFLG_PSEUDO  (1L << 12)	/* Psuedo */
#define cs_PRJFLG_INTR    (1L << 13)	/* Interuptible */

					/* Surface Type */
					/* Generally considered: */
#define cs_PRJFLG_CYLND   (1L << 14)	/* Cylindrical */
#define cs_PRJFLG_CONIC   (1L << 15)	/* Conic */
#define cs_PRJFLG_FLAT    (1L << 16)	/* Azimuthal */
#define cs_PRJFLG_OTHER   (1L << 17)	/* Other */

						/* Additional processes */
#define cs_PRJFLG_AFFINE  (1L << 20)	/* Affine transformation post-processer */

						/* Parameter Control */
#define cs_PRJFLG_ORGFLS  (1L << 24)	/* The projection does NOT support
										   a false origin. */
#define cs_PRJFLG_SCLRED  (1L << 25)	/* Scale Reduction is supported. */
#define cs_PRJFLG_ORGLAT  (1L << 26)	/* Projection does not use an
										   origin latitude paremeter. */
#define cs_PRJFLG_ORGLNG  (1L << 27)	/* Projection does not use an
										   origin longitude parameter.
										   In many cases, prj_prm1 is the
										   origin latitude and is labeled
										   central meridian. */

enum EcsMapObjType {	csMapNone = 0,
						csMapFlavorName,				//   1
						csMapParameterKeyName,			//   2
						csMapProjectionKeyName,			//   3
						csMapGeodeticOpMthKeyName,		//   4
						csMapVerticalOpMthKeyName,		//   5
						csMapLinearUnitKeyName,			//   6
						csMapAngularUnitKeyName,		//   7
						csMapPrimeMeridianKeyName,		//   8
						csMapEllipsoidKeyName,			//   9
						csMapGeodeticXfrmKeyName,		//  10
						csMapVerticalXfrmKeyName,		//  11
						csMapDatumKeyName,				//  12
						csMapVerticalDatumKeyName,		//  13
						csMapGeoidKeyName,				//  14
						csMapGeographicCSysKeyName,		//  15
						csMapProjectedCSysKeyName,		//  16
						csMapGeographic3DKeyName,		//  17
						csMapUnknown,					//  18
						csMapUnitKeyName,				// See comments below
						csMapProjGeoCSys				// See comments below
				   };

enum EcsNameFlavor {	csMapFlvrNone      =  0,		// 0x00000000
						csMapFlvrEpsg      =  1,		// 0x00000001
						csMapFlvrEsri      =  2,		// 0x00000002
						csMapFlvrOracle    =  3,		// 0x00000004
						csMapFlvrAutodesk  =  4,		// 0x00000008
						csMapFlvrBentley   =  5,		// 0x00000010
						csMapFlvrSafe      =  6,		// 0x00000020
						csMapFlvrMapInfo   =  7,		// 0x00000040
						csMapFlvrCtmx      =  8,		// 0x00000080
						csMapFlvrCsMap     =  9,		// 0x00000100
						csMapFlvrOGC       = 10,		// 0x00000200
						csMapFlvrOCR       = 11,		// 0x00000400
						csMapFlvrGeoTiff   = 12,		// 0x00000800
						csMapFlvrGeoTools  = 13,		// 0x00001000
						csMapFlvrOracle9   = 14,		// 0x00002000
						csMapFlvrIBM       = 15,		// 0x00004000
						csMapFlvrOem       = 16,		// 0x00008000
						csMapFlvrAnon01    = 17,
						csMapFlvrAnon02    = 18,
						csMapFlvrAnon03    = 19,
						csMapFlvrAnon04    = 20,
						csMapFlvrAnon05    = 21,
						csMapFlvrAnon06    = 22,
						csMapFlvrAnon07    = 23,
						csMapFlvrAnon08    = 24,
						csMapFlvrAnon09    = 25,
						csMapFlvrAnon10    = 26,
						csMapFlvrAnon11    = 27,
						csMapFlvrAnon12    = 28,
						csMapFlvrAnon13    = 29,
						csMapFlvrUser      = 30,
						csMapFlvrLegacy    = 31,
						csMapFlvrUnknown   = 32
				   };

enum EcsMapSt { csMapOk = 0,
                csMapNoName  = 1,
                csMapNoNbr   = 2,
                csMapNoMatch = 4
              };

/* The following structure defines the form of the Ellipsoid Dictionary. */
struct cs_Eldef_
{
	char key_nm [24];	/* Key name, used to identify a specific
				   entry in the table. */
	char group [6];		/* Used to group ellipsoids for selection
				   purposes. */
	char fill [2];		/* Filler */
	double e_rad;		/* Equatorial radius IN METERS. */
	double p_rad;		/* Polar radius IN METERS. */
	double flat;		/* Flattening ratio, e.g. 1.0/297.0 */
	double ecent;		/* Eccentricity. */
	char name [64];		/* Full descriptive name of the
				   datum. */
	char source [64];	/* Description of where the data for this
				   coordinate system came from. */
	short protect;		/* TRUE indicates that this is a
				   distribution definition.  Typically
				   used to require a confirmation before
				   modification. */
	short fill01;		/* Fill out to a multiple of 16 to avoid
				   affects of byte alignment options in
				   many compilers. */
	short fill02;		/* Ditto. */
	short fill03;		/* Ditto. */
	short fill04;		/* Ditto. */
	short fill05;		/* Ditto. */
	short fill06;		/* Ditto. */
	short fill07;		/* Ditto. */
};

/* The following structure defines the form of records in the Datum Dictionary. */
struct cs_Dtdef_
{
	char key_nm [24];	/* Key name, used to locate a specific
						   entry in the dictionary. */
	char ell_knm [24];	/* Ellipsoid key name. Used to access the
						   Ellipsoid Dictionary to obtain the
						   ellipsoid upon which the datum is
						   based. */
	char  group [24];	/* The classification into which this
						   datum falls. */
	char locatn [24];	/* Field by which coordinate systems can
						   be classified by location, for example,
						   world, North America, Central America,
						   Europe, etc.  To be used by selector
						   programs. */
	char cntry_st [48];	/* Up to 24 two character codes which define
						   the countries (or US states) in which
						   this coordinate system is designed to be
						   used.  We use the US postal code
						   abbreviations for states in lower case,
						   and POSC country abbreviations in upper
						   case. This also, is intended for use by
						   a coordinate system selector program. */
	char fill [8];		/* Fill the structure out to a boundary
						   of 8 bytes, for portability
						   considerations. */

	/***************************************************************************
	   The following values were, typically, extracted from DMA Technical Report
	   8350.2-B.  As of RFC-2 (SVN Revision 1905, Sept, 2010), these values are
	   only used in rare cases; and usually less and less usage as maintenance
	   continues.  These values remain for LEGACY purposes only and should be
	   considered DEPRECATED.
	***************************************************************************/
	double delta_X;		/* X component of the vector from the
						   WGS-84 geocenter to the geocenter of
						   this datum. */
	double delta_Y;		/* Y component of the vector from the
						   WGS-84 geocenter to the geocenter of
						   this datum. */
	double delta_Z;		/* Z component of the vector from this
						   WGS-84 geocenter to the geocenter of
						   this datum. */

						/* If a Bursa/Wolfe definition has been
						   made, one of the following values will
						   be non-zero.  If a pure Molodensky
						   definition is provided, the values
						   of the rot_X, rot_Y, rot_Z, and
						   bwscale are all set to hard zero.
						   This is the switch which is used to
						   determine is a Bursa/Wolf conversion
						   has been defined as opposed to a
						   standard Molodensky. */
	double rot_X;		/* Angle from WGS-84 X axis to local
						   geodetic system X axis in arc
						   seconds, east is positive. */
	double rot_Y;		/* Angle from WGS-84 Y axis to local
						   geodetic system Y axis in arc
						   seconds, north is positive. */
	double rot_Z;		/* Angle from WGS-84 Z axis to local
						   geodetic system Z axis in arc
						   seconds, use right hand rule. */
	double bwscale;		/* Scale factor in parts per million.
						   Don't include the base 1.0; we
						   add that at setup time. */
	char name [64];		/* Full descriptive name of the
						   datum. */
	char source [64];	/* Description of where the data for this
						   coordinate system came from. */
	short protect;		/* TRUE indicates that this is a 
						   distribution definition.  Typically
						   requires confirmation for change. */
	short to84_via;		/* Conversion technique, for example:
						   cs_DTCTYP_MOLO
						   cs_DTCTYP_BURS
						   cs_DTCTYP_MREG
						   cs_DTCTYP_NAD27
						   cs_DTCTYP_NAD83
						   cs_DTCTYP_WGS84
						   cs_DTCTYP_WGS72   */
	short fill01;		/* Fill out to a multiple of 16 to avoid
						   affects of byte alignment options in
						   many compilers. */
	short fill02;		/* Ditto. */
	short fill03;		/* Ditto. */
	short fill04;		/* Ditto. */
	short fill05;		/* Ditto. */
	short fill06;		/* Ditto. */
};

/* cs_Csdef_ is the record structure of the coordinate system
   dictionary.
*/

struct cs_Csdef_
{
	char key_nm [24];	/* The name used to identify the
						   coordinate system. */
	char dat_knm [24];	/* The key name of the datum upon which
						   the coordinate system is based. */
	char elp_knm [24];	/* The key name of the ellipsoid upon
						   which the coordinate system is based. */
	char prj_knm [24];	/* The key name of the projection upon
						   which the coordinate system is based,
						   eight characters max. */
	char  group [24];	/* The classification into which this
						   coordinate system falls.  I.e.
						   State Plane 27, State Plane 83,
						   UTM 27, etc. */
	char locatn [24];	/* Field by which coordinate systems can
						   be classified by location, for example,
						   world, North America, Central America,
						   Europe, etc.  To be used by selector
						   programs. NOT CURRENTLY POPULATED  */
	char cntry_st [48];	/* Up to 24 two character codes which define
						   the countries (or US states) in which
						   this coordinate system is designed to be
						   used.  We use the US postal code
						   abbreviations for states in lower case,
						   and POSC country abbreviations in upper
						   case. This also, is intended for use by
						   a coordinate system selector program.
						    NOT CURRENTLY POPULATED   */
	char unit [16];		/* The name of the units of the coordinate
						   system, i.e. the units of the resulting
						   coordinate system. */
	char fill [8];		/* Fill out to a boundary of 8. */
	double prj_prm1;
	double prj_prm2;
	double prj_prm3;
	double prj_prm4;
	double prj_prm5;
	double prj_prm6;
	double prj_prm7;
	double prj_prm8;
	double prj_prm9;
	double prj_prm10;
	double prj_prm11;
	double prj_prm12;
	double prj_prm13;
	double prj_prm14;
	double prj_prm15;
	double prj_prm16;
	double prj_prm17;
	double prj_prm18;
	double prj_prm19;
	double prj_prm20;
	double prj_prm21;
	double prj_prm22;
	double prj_prm23;
	double prj_prm24;
						/* Twenty four projection parameters.
						   The actual contents depend upon the
						   projection.  For example, for
						   Transverse Mercator only prj_prm1
						   is used and it contains the Central
						   Meridian.  Values in degrees as
						   opposed to radians. */
	double org_lng;
	double org_lat;		/* The origin of the projection.  Values
						   are in degrees.  For several
						   projections, parm1 carries the origin
						   longitude (i.e. central meridian). */
	double x_off;		/* The false easting to be applied to keep X
						   coordinates positive.  Values are in the
						   units of the resulting coordinates. */
	double y_off;		/* The false northing to be applied to keep
						   the Y coordinates positive.  Values are in
						   the units of the resulting coordinates. */
	double scl_red;		/* The scale reduction which is used
						   on some projections to distribute
						   the distortion uniformly across
						   the map, else 1.0. */
	double unit_scl;	/* The scale factor required to get
						   from coordinate system units to meters
						   by multiplication.  This factor is used
						   to convert scalars (i.e. text height,
						   elevations, etc.) in the system unit to
						   meters by multiplication.  It is also used
						   to convert scalars from meters to the
						   system unit by division. */
	double map_scl;		/* The scale factor to get to the desired
						   map scale by division (e.g. 24000 for a
						   USGS topo quad map).  This feature of
						   CS_MAP is only used when one is trying
						   to produce inches, millimeters, etc. on
						   an existing map.  In this case, one sets
						   the unit to inches, millimeters, whatever,
						   and sets this value appropriately.  Usually,
						   this value is set to 1.0. */
	double scale;		/* A single scale factor which includes
						   all the unit scale and the map scale
						   factors defined above.  This factor
						   must convert meters to coordinate system
						   units by multiplication.  Therefore, it
						   is essentially:
						         1.0 / (unit_scl * map_scl).
						   This value is used to convert the ellipsoid
						   equatorial radius to system units before
						   all other calculations are made.  This
						   variable exists primarily for historical
						   reasons. */
	double zero [2];	/* Absolute values of X & Y which are
						   smaller than this are to be converted
						   to a hard zero.  Set by the compiler
						   to the system unit equivalent of .01
						   millimeters by the setup function.
						   This feature is included to prevent
						   output such as 2.345E-05 which is
						   usually undesirable. */

						/* The following values are set to zero
						   by the compiler and are an attempt to
						   prepare for future changes. */
	double hgt_lng;		/* Longitude of the elevation point. */
	double hgt_lat;		/* Latitude of the elevation point. */
	double hgt_zz;		/* Elevation of the coordinate system;
						   typically the actual elevation at
						   elevation average point.  This is
						   an orthometric height, i.e. height
						   above the geoid. */
	double geoid_sep;	/* If defined by the user, the height of
						   the geoid above the ellipsoid, also
						   known as the geoid separation, at the
						   elevation point. */

						/* Lat/Longs outside the rectangle
						   established by the following
						   cause a warning message when
						   converted.  If the min is greater
						   than the max, the setup function
						   generates these automatically. */
	double ll_min [2];
	double ll_max [2];
					/* XY's outside the rectangle
					   established by the following
					   cause a warning message when
					   converted.  If the min is
					   greater than the max, the
					   setup function generates these
					   automatically. */
	double xy_min [2];
	double xy_max [2];


	char desc_nm [64];	/* The complete name of the coordinate
						   system. */
	char source [64];	/* Description of where the data for this
						   coordinate system came from. */
	short quad;			/* Quadrant of the Cartesian coordinates.
						   Used to handle coordinate systems in
						   which X increases to the left, etc. */
	short order;		/* Order of the complex series, if any
						   used for this coordinate system.
						   Order is currently computed automatically,
						   so this field is currently ignored. */
	short zones;		/* Number of zones in an interrupted
						   coordinate system definition, such
						   as sinusoidal and Goode homolosine.
						   Currently, the number of zones is
						   automatically calculated and this
						   field is ignored. This, of course,
						   could change. */
	short protect;		/* Set to TRUE is this definition is to
						   be protected from being changed by
						   users. */
	short epsg_qd;		/* In the same form as the quad member of this
						   structure, this element carries quad as
						   specified by the EPSG database, originally
						   populated with values from EPSG 7.05 */
	short srid;			/* The Oracle SRID number if known, else 0. */
	short epsgNbr;		/* EPSG number, if known */
	short wktFlvr;		/* WKT flavor, if derived from WKT, else zero */
};

/*
	The following structure is used internally by CsMapDll.
	It combines the information of a datum definition and an
	ellipsoid definition into one structure. It is one of the
	elements of the cs_Csprm_ structure, defined below, and is
	included here only for that reason.
*/
struct cs_Datum_
{
	char key_nm [24];	/* Key name, used to locate the definition
						   in the Datum Dictionary. */
	char ell_knm [24];	/* Ellipsoid key name. */
	double e_rad;		/* Equatorial radius in meters. */
	double p_rad;		/* Polar radius in meters. */
	double flat;		/* Flattening ratio, e.g. 1.0/297.0 */
	double ecent;		/* Eccentricity (note, not squared). */
	double delta_X;		/* Position of the geocenter relative to
						   WGS-84 in meters. */
	double delta_Y;
	double delta_Z;

						/* If a Bursa/Wolfe definition has been
						   made, one of the following values will
						   be non-zero.  If a pure Molodensky
						   definition is provided, all four
						   values will be exactly zero. */

	double rot_X;		/* Angle from WGS-84 X axis to local
						   geodetic system X axis in arc
						   seconds, east is positive. */
	double rot_Y;		/* Angle from WGS-84 Y axis to local
						   geodetic system Y axis in arc
						   seconds, north is positive. */
	double rot_Z;		/* Angle from WGS-84 Z axis to local
						   geodetic system Z axis in arc
						   seconds, use right hand rule. */
	double bwscale;		/* Scale factor in parts per million.
						   Don't include the base 1.0; we
						   add that at setup time. */
	short to84_via;		/* Code which indicates how to convert
						   from this datum to WGS84. */
	char dt_name [64];	/* Full descriptive name of the
						   datum. */
	char el_name [64];	/* Full descriptive name of the
						   ellipsoid in use. */
};

/*
	The following structure contains everything an upper
	level module needs to know about converting lat/longs
	to a specific coordinate system and vice-versa.  The
	idea here is that the upper level module is completely
	independent of the coordinate system in use.  It
	just needs to call CS_csloc to get one of these
	created and then call CS_ll2cs or CS_cs2ll as
	appropriate.
*/

struct cs_Csprm_
{
	struct cs_Csdef_ csdef; /* The coordinate system definition
							   as obtained from the coordinate
							   system dictionary. */
	struct cs_Datum_ datum; /* The datum definition as obtained
							   from the datum dictionary. */

	/* Information for the following is supplied by the setup function.  If the
	   coordinate system definition carries values for the useful range of the
	   coordinate system, the supplied values are placed here without checking
	   or limitation.  All checks (i.e. CS_xychk and CS_llchk) always check the
	   mathematical domain which is independent of this check.  Therefore,
	   specifying large values can essentially disable this checking feature on
	   a coordinate system by coordinate system basis.

	   If the coordinate system definition does not carry values for these
	   limits, the setup function will calculate some values appropriate for
	   the coordinate system and its underlying projection.

	   The lat/long values are in degrees, and longitude values are relative to
	   the cent_mer element.  The cent_mer element is required to solve the
	   -180 degree longitude crack problem. */

	double cent_mer;	/* The longitude upon which the min and
						   max longitudes are based; in degrees. */
	double min_ll [2];	/* The delta longitude and absolute latitude
						   of the southwestern extent of the useful
						   range of this coordinate system; in
						   degrees. */
	double max_ll [2];	/* The delta longitude and absolute latitude
						   of the north eastern extent of the useful
						   range of this coordinate system; in
						   degrees. */
	double min_xy [2];	/* The minimum X and Y coordinates of the
						   useful range of this coordinate system.
						   Values and units are per coordinate
						   system definition and include the false
						   origin.  For a coordinate system in a
						   quadrant other than the first quad,
						   these values may not be the southwestern
						   extent as would normally be expected. */
	double max_xy [2];	/* The minimum X and Y coordinates of the
						   useful range of this coordinate system. */
	unsigned short prj_code;
						/* The projection code extracted from the
						   projection table for the active
						   projection. */
	unsigned long prj_flags;
						/* The projection flags as extracted from
						   the projection table for the active
						   projection. */
	char proprietary [690];	/* Used internally by CsMapDll. */
};

/*
	CS_dtcsu returns a pointer to a malloc'ed instance of the
	following structure.  This structure includes all of the
	information required to perform datum conversions and is
	the argument required by the CS_dtcvt function.

	This structure should not be free'ed directly, but rather
	by a call to CS_dtcls.  CS_dtcls will free this structure,
	but only after any other resources acquired by the conversion
	have been released.
*/

struct cs_Dtcprm_
{
	char src_nm [24];	/* Name of the source geodetic
						   reference system.  For error
						   reporting purposes only. */
	char dst_nm [24];	/* Key name of the destination
						   geodetic reference system.  For error
						   reporting purposes only. */
	short block_err;	/* A bitmap/value word which indicates
						   how errors encountered in the
						   conversion are to be processed.
						   This value is passed on to many of
						   the conversion routines. */
	char proprietary [2760];/* Used internally by CsMapDll. */
};

/*
	The following structure is returned by CS_csEnumByGroup.
	Essentially, consider this structure to be a brief description
	of a coordinate system.
*/
struct cs_Csgrplst_
{
	struct cs_Csgrplst_ *next;
	char key_nm [24];
	char descr [64];
	char source [64];
	char ref_typ [10];
	char ref_to [24];
	char unit [16];
};

/*
	The following define the numeric codes for each
	of the possible error conditions detected by
	the CsMapDll library.
*/
#define cs_CSDICT       102             /* Open of the Coordinate System
										   Dictionary failed. */
#define cs_CS_NOT_FND   103             /* Coordinate system not found. */
#define cs_NO_MEM       104             /* Insufficient memory. */
#define cs_UNKWN_PROJ   105             /* Unknown projection code
										   encountered in the Coordinate
										   System Dictionary. */
#define cs_CS_BAD_MAGIC 106             /* Magic number (first four bytes)
										   of Coordinate System Dictionary
										   indicate that the file is not
										   a coordinate system dictionary. */
#define cs_IOERR        107             /* Physical I/O error. */
#define cs_DTDICT       109             /* Open of Datum Dictionary failed. */
#define cs_DT_BAD_MAGIC 110             /* Magic number (first four bytes)
										   of Datum Dictionary indicate that
										   the file is not a Datum
										   Dictionary. */
#define cs_DT_NOT_FND   111             /* Datum definition not found in
										   Datum Dictionary. */
#define cs_INV_FILE     113             /* File is corrupted, specifically
										   a read of n bytes which was
										   supposed to produce n bytes,
										   didn't produce n bytes. */
#define cs_TMP_CRT      114             /* Creation of a temporary file
										   failed. */
#define cs_DISK_FULL    115             /* Disk is full, or for some other
										   reason a disk write failed. */
#define cs_UNLINK       116             /* Couldn't unlink (i.e. delete)
										   a file. */
#define cs_RENAME       117             /* Couldn't rename a file. */
#define cs_INV_NAME     118             /* Coordinate System or Datum key
										   name is invalid.  Must start
										   with an alphabetic character. */
#define cs_INV_UNIT     119             /* Invalid unit name. */
#define cs_DTC_NO_FILE  120             /* Couldn't open a required datum
										   conversion file.  Not currently
										   used; new datum conversion software
										   may require such. */
#define cs_DTC_MAGIC    122             /* Magic number (i.e. first four bytes)
										   of datum conversion data file were
										   not correct.  Not currently used.
										   New datum conversion software may
										   require such in the future. */
#define cs_DTC_NO_SETUP 124             /* Datum conversion function was
										   called bu the datum conversion
										   was not properly initialized. */
#define cs_NADCON_ICNT  126             /* The NADCON iteration count (9)
										   failed to resolve the NAD83 to
										   NAD27 conversion within the
										   required tolerance. */
#define cs_NADCON_CONS  127             /* Properly named NADCON database
										   files are inconsistent, e.g.
										   don't cover the same region. */
#define cs_DTC_FILE     128             /* Couldn't open a required datum
										   conversion file. */
#define cs_DTC_DAT_F    129             /* FATAL: Requested datum conversion
										   not supported in its most precise
										   form. */
#define cs_DTC_DAT_W    130             /* WARNING: Optional software would
										   produce more precise datum
										   conversion results. */
#define cs_DTC_BLK_F    131             /* FATAL: Data required to convert
										   specific coordinate is not
										   available on this system. */
#define cs_DTC_BLK_W    132             /* WARNING: Data required to convert
										   specific coordinate is not
										   available on this system. */
#define cs_DTC_BLK_Q    133             /* WARNING: System has reported 20
										   cs_DTC_BLK_W error conditions,
										   additional occurences will be
										   suppressed. */
#define cs_WGS_CNVRG    134             /* The iterative inverse WGS-84
										   calculations failed to converge. */
#define cs_EL_NOT_FND   135             /* Ellipsoid definition not found in
										   Ellipsoid Dictionary. */
#define cs_ELDICT       136             /* Open of Ellipsoid Dictionary
										   failed. */
#define cs_EL_BAD_MAGIC 137             /* Magic number (first four bytes)
										   of Ellipsoid Dictionary indicate
										   that the file is not a Ellipsoid
										   Dictionary. */
#define cs_NAD_NO_DATA  138             /* Couldn't locate and NADCON data
										   when initializing NADCON
										   alogorithm. */
#define cs_ISER         139             /* CS_MAP internal software error. */
#define cs_HPGN_ICNT    140             /* The NADCON iteration count (9)
										   failed to resolve the NAD91 to
										   NAD83 conversion within the
										   required tolerance. */
#define cs_HPGN_CONS    141             /* Properly named HPGN database
										   files are inconsistent, e.g.
										   don't cover the same region. */
#define cs_HPG_NO_DATA  142             /* Couldn't locate and HPGN data
										   when initializing HPGN
										   alogorithm. */
#define cs_HPG_BLK_F    143             /* FATAL: Data required to convert
										   specific HPGN coordinate is not
										   available on this system. */
#define cs_HPG_BLK_W    144             /* WARNING: Data required to convert
										   specific HPGN coordinate is not
										   available on this system. */
#define cs_HPG_BLK_Q    145             /* WARNING: System has reported 10
										   cs_HPG_BLK_W error conditions,
										   additional occurences will be
										   suppressed. */
#define cs_MREG_RANGE   146             /* WARNING: Conversion outside of
										   normal range of Multiple Regression
										   formulas. */
#define cs_MREG_BADMAG  147             /* Invalid magic number on Multiple
								   Regression data file. */
#define cs_NWDT_READ	148		/* Error reading old datum file when
								   updating to new format. */
#define cs_NWDT_WRIT	149		/* Error writing new datum file when
								   updating to new format. */
#define cs_TMPFN_MAXED  150		/* Algorithm to generate a temporary
								   file name is max'ed out. */
#define cs_BSWP_UNKWN   151		/* Byte ordering of this system is
								   unsupported. */
#define cs_NWCS_READ	152		/* Error reading old datum file when
								   updating to new format. */
#define cs_NWCS_WRIT	153		/* Error writing new datum file when
								   updating to new format. */
#define cs_NWEL_READ	154		/* Error reading old datum file when
								   updating to new format. */
#define cs_NWEL_WRIT	155		/* Error writing new datum file when
								   updating to new format. */
#define cs_NO_REFERNCE	156		/* CS_csloc was asked to activate a
								   coordinate system which had
								   neither a cartographic reference
								   (i.e. an ellipsoid) or a geodetic
								   reference (i.e. a datum). */
#define cs_CSGRP_INVKEY 157 		/* Invalid group name provided to
								   group list function. */
#define cs_FL_OPEN      158		/* General file open failure. */
#define cs_SWP_TYPE     159		/* Unknown file type passed to swapper (Cs_swpfl). */
#define cs_ELDEF_INV    160		/* Ellipsoid definition is invalid. */
#define cs_DTDEF_INV    161		/* Datum definition is invalid. */
#define cs_RGN_PNTCNT   162		/* Invalid point count argument to
								   a coordinate checker function. */
#define cs_CNTV2_MULT   163		/* Multiple Version 2 Canadian
								   National Transformation files
								   detected. */
#define cs_CS_PROT      164		/* Distribution coordinate system is
								   protected. */
#define cs_DT_PROT      165		/* Distribution datum definition is
								   protected. */
#define cs_EL_PROT      166		/* Distribution ellipsoid is
								   protected. */
#define cs_CS_UPROT     167		/* User defined  coordinate system is
								   now protected. */
#define cs_DT_UPROT     168		/* User defined datum is now
								   protected. */
#define cs_EL_UPROT     169		/* User defined ellipsoid is now
								   protected. */
#define cs_UNIQUE       170		/* User defined dictionary items
								   require unique character. */
#define cs_A2F_MLTSGN   171		/* Multiple sign indications in
								   numeric value. */
#define cs_A2F_ERRCMA   172		/* Inconsistent use of the comma. */
#define cs_A2F_MLTPNT   173		/* Multiple decimal point indications
								   in numeric value. */
#define cs_A2F_SECS60   174		/* Value greater than 60 encountered
								   where seconds were expected. */
#define cs_A2F_MINS60   175		/* Value greater than 60 encountered
								   where minutes were expected. */
#define cs_A2F_FMTERR   176		/* General format error encountered,
								   too many spaces. */
#define cs_INV_INDX     177		/* Invalid enumeration index. */
#define cs_INV_ARG1     178		/* Pointer argument 1 is invalid. */
#define cs_INV_ARG2     179		/* Pointer argument 2 is invalid. */
#define cs_INV_ARG3     180		/* Pointer argument 3 is invalid. */
#define cs_INV_ARG4     181		/* Pointer argument 4 is invalid. */
#define cs_INV_ARG5     182		/* Pointer argument 5 is invalid. */
#define cs_INV_UTYP     183		/* Invalid unit type. */

#define cs_GEOID_NO_DATA 184	/* Couldn't locate any Geoid data
								   when initializing Geoid
								   alogorithm. */
#define cs_GEOID_FILE	185		/* Error opening a Geoid file. */
#define cs_GEOID_NO_SETUP 186	/* Geoid algorithm has not been set up. */
#define cs_DYMUTM	187			/* Dynamic UTM operation requested on
								   a non UTM coordinate system. */
#define cs_MIF_UNIT	188			/* CS-MAP definition uses a unit not
								   supported by MapInfo. */
#define cs_MIF_ELREF	189		/* Mapinfo doesn't support direct ellipsoid
								   references. */
#define cs_MIF_DATUM	190		/* CS-MAP definition references a datum
								   which is not supported by MapInfo. */
#define cs_MIF_FEATR	191		/* CS-MAP definition uses a features not
								   supported by MapInfo. */
#define cs_MIF_PROJ	192			/* CS-MAP definition uses a projection not
								   supported by MapInfo. */
#define cs_DBL_SPACE	193		/* Double space encountered in key name. */
#define cs_MRT_NTFND	194		/* Multiple Regression file not found. */

#define cs_DTC_FIL_F    195		/* FATAL: Generally available NADCON
								   data file was not found. */
#define cs_DTC_FIL_W    196		/* WARNING: Generally available
								   NADCON data file was not found. */
#define cs_DTC_RNG_F    197		/* FATAL: Data point outside range
								   of NAD27/83 conversion. */
#define cs_DTC_RNG_W    198		/* WARNING: Data point outside range
								   of NAD27/83 conversion. */
#define cs_SPZ_INVALID  199		/* State plane zone number is invalid. */

/* Begin coordinate system definition checker specific stuff. */
#define cs_CSQ_AZM	201
#define cs_CSQ_AZMTH	202
#define cs_CSQ_LAT	203
#define cs_CSQ_LATEQU	204
#define cs_CSQ_LNG	205
#define cs_CSQ_LNGEQU	206
#define cs_CSQ_MAPSCL	207
#define cs_CSQ_MEREQU	208
#define cs_CSQ_MRCAT	209
#define cs_CSQ_MSCOEF	210
#define cs_CSQ_NOREF	211
#define cs_CSQ_NOTNRTH	212
#define cs_CSQ_NRTHLAT	213
#define cs_CSQ_NRTHPNT	214
#define cs_CSQ_ORGLAT	215
#define cs_CSQ_ORGLNG	216
#define cs_CSQ_PLL90	217
#define cs_CSQ_PLLEQU	218
#define cs_CSQ_PLLLRG	219
#define cs_CSQ_PLLREV	220
#define cs_CSQ_PLLZERO	221
#define cs_CSQ_POLDD	222
#define cs_CSQ_POLDUP	223
#define cs_CSQ_POLLAT	224
#define cs_CSQ_POLLNG	225
#define cs_CSQ_QUAD	226
#define cs_CSQ_SCLRED	227
#define cs_CSQ_SOTHLAT	228
#define cs_CSQ_STDLAT	229
#define cs_CSQ_STDLNG	230
#define cs_CSQ_STDPLL	231
#define cs_CSQ_STDSOU	232
#define cs_CSQ_STDWEST	233
#define cs_CSQ_UNIT	234
#define cs_CSQ_INVPRJ	235		/* Invalid projection key name
								   specification */
#define cs_CSQ_INVDTM	236		/* Invalid Datum Name. */
#define cs_CSQ_INVELP	237		/* Invalid ellipsoid name. */
#define cs_CSQ_LLRNG	238		/* Lat/Long range error. */
#define cs_CSQ_RNGORD	239		/* Lat/Long range order. */
#define cs_CSQ_INVQUAD	240		/* Invalid Quadrant specification. */
#define cs_CSQ_GEOMM	241		/* Invalid geographic useful range. */
#define cs_CSQ_CRTMM	242		/* Invalid cartesian useful range. */
#define cs_CSQ_PLLED	243
#define cs_CSQ_PLRLAT	244		/* Org Lat is not polar. */
#define cs_CSQ_USEPLR	245		/* Polr org lat on oblique projection. */
#define cs_CSQ_UTMZON	246		/* Invalid UTM zone number provided. */
#define cs_CSQ_HMISPHR	247		/* Invalid hemisphere specifiction. */		
#define cs_CSQ_USESW	248		/* Az == 90, use Swiss Oblique Mercator */
#define cs_CSQ_MAX15	249		/* Eastern meridian cannot be more than
								   15 degrees away from the central
								   meridian. */		
#define cs_CSQ_OBLQPOLE 250		/* Invalid oblique pole specification. */		
#define cs_CSQ_AFFZERO	251		/* Denominator of affine is zero. */
/* End coordinate system definition checker specific stuff. */

#define cs_DLM_CSIDX_FULL 301		/* Coordindate System index is full. */
#define cs_DLM_DTIDX_FULL 302		/* Datum conversion index is full. */
#define cs_A2F_RATIO	  303		/* Invalid ratio input format. */
#define cs_AGD66_RNG_F	  304		/* FATAL: Data point outside range
									   of AGD66/GDA94 conversion. */
#define cs_AGD66_RNG_W	  305		/* WARNING: Data point outside range
									   of AGD66/GDA94 conversion. */
#define cs_XYZ_ITR		  306		/* Geocentric inverse convergence failure. */
#define cs_MO_CNVRG		  307		/* The iterative inverse Molodensky
									   calculations failed to converge. */
#define cs_BW_CNVRG		  308		/* The iterative inverse Bursa/Wolf
									   calculations failed to converge. */
#define cs_7P_CNVRG		  309		/* The iterative inverse 7 Parameter
									   calculations failed to converge. */
#define cs_3P_CNVRG		  310		/* The iterative inverse 3 Parameter
									   calculations failed to converge. */
#define cs_6P_CNVRG		  311		/* The iterative inverse 6 Parameter
									   calculations failed to converge. */
#define cs_4P_CNVRG		  312		/* The iterative inverse 4 parameter
									   calculations failed to converge. */
#define cs_DTC_PATH		  313		/* Datum file specification format
									   error. */
#define cs_NAD_HPGN		  314		/* Datum file specification format
									   error. */
#define cs_NAD_EXT		  315		/* Datum file specification format
									   error. */
#define cs_NAD_LAS		  316		/* Datum file specification format
									   error. */
#define cs_NAD_LOS		  317		/* Datum file specification format
									   error. */
#define cs_HPGN_NAD		  318		/* Datum file specification format
									   error. */
#define cs_HPGN_EXTA	  319		/* Datum file specification format
									   error. */
#define cs_HPGN_EXTO	  320		/* Datum file specification format
									   error. */
#define cs_HPGN_EXTX	  321		/* Datum file specification format
									   error. */
#define cs_GHGT_GEO		  322		/* Datum file specification format
									   error. */
#define cs_GHGT_EXT		  323		/* Datum file specification format
									   error. */
#define cs_VCON_94		  324		/* Datum file specification format
									   error. */
#define cs_VCON_EXT		  325		/* Datum file specification format
									   error. */
#define cs_DTC_CATFILE	  326		/* Datum catalog file not found, open
									   failed. */
#define cs_DTC_EXT		  327		/* Datum catalog path entries without
									   extension or full path. */
#define cs_AGD84_RNG_F	  328		/* FATAL: Data point outside range
									   of AGD84/GDA94 conversion. */
#define cs_AGD84_RNG_W	  329		/* WARNING: Data point outside range
									   of AGD84/GDA94 conversion. */
#define cs_NZGD49_RNG_F	  330		/* FATAL: Data point outside range
									   of NZGD49/NZGD2K conversion. */
#define cs_NZGD49_RNG_W	  331		/* WARNING: Data point outside range
									   of NZGD49/NZGD2K conversion. */
#define cs_ATS77_RNG_F	  332		/* FATAL: Data point outside range
									   of ATS77/WGS84 conversion. */
#define cs_ATS77_RNG_W	  333		/* WARNING: Data point outside range
									   of ATS77/WGS84 conversion. */
#define cs_NAD_RNG_F      334		/* FATAL: Data point outside range
									   of NAD27/83 conversion. */
#define cs_NAD_RNG_W      335		/* WARNING: Data point outside range
									   of NAD27/83 conversion. */
#define cs_HARN_RNG_F     336		/* FATAL: Data point outside range
									   of HARN/83 conversion. */
#define cs_HARN_RNG_W     337		/* WARNING: Data point outside range
									   of HARN/83 conversion. */
#define cs_MGRS_LL        338		/* Invalid lat/long for MGRS conversion. */
#define cs_MGRS_XY        339		/* Invalid Utm/Ups presented for conversion */
#define cs_MGRS_ZONE      340	       /* Invalid zone number for MGRS inversion */
#define cs_MGRS_UTM       341       /* Invalid UtmUps values */
#define cs_MGRS_STR1      342       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR2      343       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR3      344       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR4      345       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR5      346       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR6      347       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR7      348       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR8      349       /* MGRS string provided was inconsistent */
#define cs_MGRS_STR9      350       /* MGRS string provided was inconsistent */
#define cs_MGRS_STRA      351       /* MGRS string provided was inconsistent */
#define cs_MGRS_STRB      352       /* MGRS string provided was inconsistent */
#define cs_MGRS_STRC      353       /* MGRS string provided was inconsistent */
#define cs_MGRS_NOSET     354       /* High Level MGRS interface not set up. */
#define cs_FLBK_NOSET     355       /* No fallback datum name specified. */
#define cs_FLBK_NTFND     356       /* Fallback datum not found. */
#define cs_FLBK_WRNGT     357       /* Fallback type is invalid. */
#define cs_NOT_NERTH      358       /* Conversion from Nerth to something else, or vice versa */
#define cs_NAD_RNG_A      359       /* Data outside coverage, fallback used successfully */
#define cs_AGD66_RNG_A    360       /* Data outside coverage, fallback used successfully */
#define cs_AGD84_RNG_A    361       /* Data outside coverage, fallback used successfully */
#define cs_NZGD49_RNG_A   362       /* Data outside coverage, fallback used successfully */
#define cs_ATS77_RNG_A    363       /* Data outside coverage, fallback used successfully */
#define cs_HARN_RNG_A     364       /* Data outside coverage, fallback used successfully */
#define cs_CSRS_RNG_F     365       /* FATAL: Data point outside range of CSRS conversion. */
#define cs_CSRS_RNG_W     366       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_CSRS_RNG_A     367       /* WARNING: Data outside coverage, fallback used successfully. */
#define cs_DTC_SOFTMAX    368       /* Ten or more block errors (at different geographical locations) in the same datum. */
#define cs_MREG_RANGEF    369       /* Conversion outside of normal range of Multiple Regression formulas, no fallback specified. */
#define cs_MREG_CNVRG     370       /* The iterative inverse Multiple regression calculation failed to converge. */
#define cs_DTC_SOFTIGNR   371       /* Ten or more block errors found, ignoring the remainder. */
#define cs_GEOID_INIT     373       /* Geoid height object not initiailized. */
#define cs_TOKYO_ICNT     374       /* Japanese inverse datum calculation failed to converge. */
#define cs_TOKYO_RNG_F    375       /* FATAL: Data point outside range of Tokyo conversion. */
#define cs_TOKYO_RNG_W    376       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_TOKYO_RNG_A    377       /* WARNING: Data outside coverage, fallback used successfully. */
#define cs_TOKYO_EXT	  378		/* Datum file specification file extension error. */
#define cs_RGF93_ICNT     379       /* RGF93 inverse datum calculation failed to converge. */
#define cs_RGF93_RNG_F    380       /* FATAL: Data point outside range of RGF93 conversion. */
#define cs_RGF93_RNG_W    381       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_RGF93_RNG_A    382       /* Data outside coverage, fallback used successfully */
#define cs_CSRS27_RNG_F   383       /* FATAL: Data point outside range of CSRS27 conversion. */
#define cs_CSRS27_RNG_W   384       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_CSRS27_RNG_A   385       /* WARNING: Data outside coverage, fallback used successfully. */
#define cs_ED50_RNG_F     386       /* FATAL: Data point outside range of ED50 conversion. */
#define cs_ED50_RNG_W     387       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_ED50_RNG_A     388       /* WARNING: Data outside coverage, fallback used successfully. */
#define cs_UADD_TYPE      389		/* Invalid unit type on unit add. */
#define cs_UADD_DUP	      390		/* Unit to be added already exists. */		
#define cs_UADD_FULL      391		/* on unit add, table is full */
#define cs_UDEL_NONE      392		/* on unit del, not there. */
#define cs_N27A77_RNG_F   393       /* FATAL: Data point outside range of N27A77 conversion. */
#define cs_N27A77_RNG_W   394       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_N27A77_RNG_A   395       /* WARNING: Data outside coverage, fallback used successfully. */
#define cs_THRD_LCK       396		/* Request to obtain lock on restricted resource failed. */
#define cs_GDC_DENSITY    397		/* Suspect density value in GDC file. */
#define cs_LLENUM_NOSU    398		/* Useful Range enumerator has not been setup. */

#define cs_WKT_WRNGTYP    399		/* Wrong type presented to be converted */
#define cs_WKT_NOELLIP    400		/* WKT SPHEROID child object was not present */
#define cs_WKT_NODATUM    401		/* WKT DATUM child object was not present */
#define cs_WKT_NOUNIT     402		/* WKT UNIT child object was not present */
#define cs_WKT_INVUNIT    403		/* WKT UNIT name was not one currently recognized */
#define cs_WKT_NOGEOCS    404		/* No GEOGCS child element was present */
#define cs_WKT_NOGUNIT    405		/* UNIT child element was not present in GEOGCS */
#define cs_WKT_INVGUNIT   406		/* GEOGCS unit name was not recognized */
#define cs_WKT_NOPROJ     407		/* Couldn't locate a PROJECTION element */
#define cs_WKT_INVPROJ    408		/* Value of the PROJECTION element was not recognized */
#define cs_WKT_GEOGCNT    409		/* Did not find two GEOGCS elements in a GEOGTRAN */
#define cs_WKT_NOSRCDT    410		/* Couldn't locate the source datum in a GEOGTRAN */
#define cs_WKT_NOMETH     411		/* Couldn't find a METHOD element in a GEOGTRAN */
#define cs_WKT_MTHERR     412		/* The PARAMETERS and METHOD of a GEOGTRAN were inconsistent */
#define cs_WKT_UKMETH     413		/* Unrecognized method in GEOGTRAN */
#define cs_WKT_WRNGTRG    414		/* Target of GEOGTRAN is not WGS84 */
#define cs_WKT_PRMSUPRT   415		/* Parameter arrangement not supported by CS-MAP. */
#define cs_WKT_UNITMAP    416       /* Don't know WKT equivalent to MSI unit. */
#define cs_WKT_PRJSUPRT   417		/* Projection variation arrangement not supported WKT */
#define cs_WKT_NODTMNM    418		/* Unamed datum specification */
#define cs_WKT_NODTMSPC   419		/* No TOWGS84 specification and name not found in dictionary. */
#define cs_WKT_NOLLBASE   420		/* Reference datum did not produce a geographic coordinate system. */
#define cs_WKT_NODTREF    421		/* WKT does not support direct ellipsoid references. */
#define cs_WLD_NOSOL      422		/* No solution to workd (.wld) file problem. */
#define cs_WKT_BADFORM    423		/* Poorly formed WKT structure. */
#define cs_WKT_DTMAP      424		/* Datum name mapping failed */
#define cs_WKT_FLAVOR     425		/* Couldn't determine flavor of provided WKT string */
#define cs_WKT_INCNSIST   426		/* Parameters provided to CS_cs2WktEx are inconsistent */

#define cs_DHDN_RNG_F     427       /* FATAL: Data point outside range of DHDN conversion. */
#define cs_DHDN_RNG_W     428       /* WARNING: Data outside coverage, coordinates unshifted. */
#define cs_DHDN_RNG_A     429       /* WARNING: Data outside coverage, fallback used successfully. */

#define cs_NMMAP_INIT     430       /* Name mapper has not been initialized. */
#define cs_NMMAP_FAIL1    431       /* Name mapper initialization failed, no file */
#define cs_NMMAP_FAIL2    432       /* Name mapper initialization failed, file format */
#define cs_NMMAP_NONAME   433       /* No record of given name with the specified flavor */
#define cs_NMMAP_NONBR    434       /* No record of given number with the specified flavor */

#define cs_INV_ARG6       435		/* Pointer argument 6 is invalid. */
#define cs_INV_ARG7       436		/* Pointer argument 7 is invalid. */
#define cs_INV_ARG8       437		/* Pointer argument 8 is invalid. */
#define cs_INV_ARG9       438		/* Pointer argument 9 is invalid. */

#define cs_SYS34_NOSRC    439		/* Danish System 34 source code unavailable.  */

#define cs_CHENYX_RNG_F   440       /* FATAL: Data point outside range of CHENYX conversion. */
#define cs_CHENYX_RNG_W   441       /* WARNING: Data outside CHENYX coverage, coordinates unshifted. */
#define cs_CHENYX_RNG_A   442       /* WARNING: Data outside CHENYX coverage, fallback used successfully. */
#define cs_MGRS_GRDSQR    443       /* Invalid grid square position provided */

#define cs_GP_BAD_MAGIC   444		/* Magic number (first four bytes) of Geodetic Path
									   Dictionary indicate that the file is not a Geodetic
									   Path Dictionary. */
#define cs_GPDICT         445		/* Open of the Geodetic Path Dictionary failed. */
#define cs_GP_PROT        446		/* Attempt to change a distribution Geodetic Path dictionary entry. */
#define cs_GP_UPROT       447		/* Attempt to change a protected user Geodetic Path dictionary entry. */
#define cs_GP_NOT_FND     448		/* Geodetic path not found. */
#define cs_GP_NOPATH      449		/* Geodetic path not found. */
#define cs_GEOPATH_DUP    450		/* Duplicate entries in the Geodetic Path dictionary */

#define cs_GX_BAD_MAGIC   451		/* Magic number (first four bytes) of Geodetic Transformation
									   Dictionary indicate that the file is not a Geodetic
									   Path Dictionary. */
#define cs_GXDICT         452		/* Open of the Geodetic Transformation Dictionary failed. */
#define cs_GX_PROT        453		/* Attempt to change a distribution Geodetic Transformation dictionary entry. */
#define cs_GX_UPROT       454		/* Attempt to change a protected user Geodetic Transformation dictionary entry. */
#define cs_GX_NOT_FND     455		/* Geodetic transformation not found. */
#define cs_NULLX_CNVRG	  456		/* The iterative inverse Null Transformation
									   calculation failed to converge. */
#define cs_GEOCT_CNVRG	  457		/* The iterative inverse Geocentric Transformation
									   calculation failed to converge. */
#define cs_MULRG_CNVRG	  458		/* The iterative inverse multiple regression
									   calculations failed to converge. */
#define cs_UNKWN_DTCMTH   459		/* Unknown datum transformation method encountered in the
									   Geodetic Transformation dictionary. */
#define cs_GRD_RNG_FLBK	  460		/* Out of coverage of grid files, fallback used. */
#define cs_GRD_RNG_WRN	  461		/* Out of coverage of grid files, warning only. */
#define cs_GX_TOOMANY	  462		/* Too many transformations required to do datum transformation. */
#define cs_GEOXFRM_DUP	  463		/* Duplicate geodetic transformation definitions. */
#define cs_DT_NOPATH	  464		/* Couldn't locate or construct a path between datums. */
#define cs_ATS77_INV	  465		/* Attempt to perform an inverse calculation on the ATS77 datum. */

#define cs_CT_NOT_FND	  466		/* Category not found. */
#define cs_CT_CS_NOT_IN	  467		/* CS name not contained in category. */
#define cs_CT_PROT		  468		/* CT is (partially) protected. */
#define cs_CT_CS_ADD_DUP  469		/* CT already contains a CS with that name. */
#define cs_CT_DICT		  470		/* CT dictionary open failed. */

#define cs_DICT_INV		  471		/* Dictionary contains at least 1 invalid entry */
#define cs_DICT_DUP_IDS   472		/* Dictionary contains duplicate IDs */

#define cs_ENV_TOOLONG    473		/* String presented for environmental subsitution is too long. */
#define cs_ENV_NOVAR      474		/* A referenced environmental variable did not exist. */
#define cs_ENV_FORMAT     475		/* The format of the string presented fro environmental
									   variable subsitution is improperly formatted. */
									   
#define cs_ERROR_MAX	  cs_ENV_FORMAT

/*
	Now for the function prototypes.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (DLL_16) && !defined (DLL_32) && !defined (DLL_64)
#	define DLL_32
#endif

/* EXP_LVL1 essentially defines the High Level Interface (i.e. VB, Delphi,
   etc).  No pointers are required to use the interface.  Use CS_cnvrt for
   coordinate conversions. */
#ifndef EXP_LVL1
#	if defined (DLL_16)
#		define EXP_LVL1 _pascal
#	elif defined (DLL_32)
#		define EXP_LVL1 __stdcall
#	elif defined (DLL_64)
#		define EXP_LVL1 __declspec(dllimport)
#	else
#		define EXP_LVL1
#	endif
#endif

/* Defining EXP_LVL2 activates several functions which make limited use of
   pointers and, therefore, may NOT be of value in all High Level Language
   environments. */
#ifndef EXP_LVL2
#	if defined (DLL_16)
#		define EXP_LVL1
#	elif defined (DLL_32)
#		define EXP_LVL2 __stdcall
#	elif defined (DLL_64)
#		define EXP_LVL2 __declspec(dllimport)
#	else
#		define EXP_LVL2
#	endif
#endif

/* EXP_LVL3 essentially defines the High Performance Interface (i.e. 'C',
   'C++'.  Ability to use pointers is required.  Please note that several
   functions in this Interface return pointers to memory allocated by the
   DLL from the DLL's own heap memory.   IT IS ABSOLUTELY ESSENTIAL THAT
   ALL SUCH MEMORY BE FREE'd USING THE CS_dllFree() FUNCTION. */
#ifndef EXP_LVL3
#	if defined (DLL_16)
#		define EXP_LVL3
#	elif defined (DLL_32)
#		define EXP_LVL3 __stdcall
#	elif defined (DLL_64)
#		define EXP_LVL3 __declspec(dllimport)
#	else
#		define EXP_LVL3
#		define EXP_DATA
#	endif
#endif

#ifndef EXP_LVL4
#	define EXP_LVL4
#endif

#ifndef EXP_LVL5
#	define EXP_LVL5
#endif

#ifndef EXP_LVL6
#	define EXP_LVL6
#endif

#ifndef EXP_LVL7
#	define EXP_LVL7
#endif

#ifndef EXP_LVL8
#	define EXP_LVL8
#endif

#ifndef EXP_LVL9
#	define EXP_LVL9
#endif

#ifndef EXP_DATA
#	if defined (DLL_16)
#		define EXP_DATA
#	elif defined (DLL_32)
#		define EXP_DATA __declspec(dllimport)
#	elif defined (DLL_64)
#		define EXP_DATA __declspec(dllimport)
#	else
#		define EXP_DATA
#	endif
#endif

/* Due to the heritage of CS_MAP, constants are declared with the Const (note
   upper case 'C') manifest constant.  Thus, declarations of this sort can be
   easily turned on and off using the following define statement. */
#define Const const

/* The High Level Interface
   Please note that data of the string type from the DLL is problematic and
   depends greatly on the environment hosting the DLL.  For these functions,
   a Vb variation is available to support Visual Basic; perhaps it these will
   be useful in other environments.

   The Vb versions of all these functions differ in one basic respect.  That
   is, the Vb versions treat the provided 'result' pointer as a character
   field, rather than a 'C' string.  Thus, the result is NOT null terminated,
   and the result which is returned occupies the entire field provided, left
   justified with space fill on the right.
*/

#if defined (EXP_LVL1) || defined (EXP_LVL2) || defined (EXP_LVL3)
int				EXP_LVL1	CS_altdr (Const char *alt_dir);
long32_t		EXP_LVL1	CS_atof (double *result,Const char *value);
int				EXP_LVL1	CS_azddll (double e_rad,double e_sq,double ll_from [3],double azimuth,double *dist,double ll_to [3]);
double			EXP_LVL1	CS_azsphr (double ll0 [2],double ll1 [2]);
int				EXP_LVL1	CS_cmpDbls (double first,double second);
double			EXP_LVL1	CS_cnvrg (Const char *cs_nam,double ll [2]);
int				EXP_LVL1	CS_cnvrt (Const char *src_cs,Const char *dst_cs,double coord [3]);
int				EXP_LVL1	CS_cnvrt3D (Const char *src_cs,Const char *dst_cs,double coord [3]);
int				EXP_LVL1	CS_csEnum (int index,char *key_name,int size);
void			EXP_LVL1	CS_csfnm (Const char *new_name);
int				EXP_LVL1	CS_csGrpEnum (int index,char *grp_name,int name_sz,char *grp_dscr,int dscr_sz);
int				EXP_LVL1	CS_csIsValid (Const char *key_name);
int				EXP_LVL1	CS_csRangeEnum (int index,char *key_name,int size);
int				EXP_LVL1	CS_csRangeEnumSetup (double longitude,double latitude);
int				EXP_LVL1	CS_dtEnum (int index,char *key_name,int size);
void			EXP_LVL1	CS_dtfnm (Const char *new_name);
int				EXP_LVL1	CS_dtIsValid (Const char *key_name);
int				EXP_LVL1	CS_elEnum (int index,char *key_name,int size);
void			EXP_LVL1	CS_elfnm (Const char *new_name);
int				EXP_LVL1	CS_elIsValid (Const char *key_name);
void			EXP_LVL1	CS_errmsg (char *user_bufr,int buf_size);
void			EXP_LVL1	CS_fast (int fast);
long32_t		EXP_LVL1	CS_ftoa (char *bufr,int size,double value,long frmt);
int				EXP_LVL1	CS_geoctrSetUp (const char* ellipsoid);
int				EXP_LVL1	CS_geoctrGetLlh (double llh [3],double xyz [3]);
int				EXP_LVL1	CS_geoctrGetXyz (double xyz [3],double llh [3]);
double			EXP_LVL1	CS_getCurvatureAt (Const char *csKeyName,double lat);
int				EXP_LVL1	CS_getDataDirectory (char *data_dir,int dir_sz);
int				EXP_LVL1	CS_getDatumOf (Const char *csKeyName,char *datumName,int size);
int				EXP_LVL1	CS_getDescriptionOf (Const char *csKeyName,char *description,int size);
int				EXP_LVL1	CS_getEllipsoidOf (Const char *csKeyName,char *ellipsoidName,int size);
int				EXP_LVL1	CS_getElValues (Const char *elKeyName,double *eRadius,double *eSquared);
int				EXP_LVL1	CS_getReferenceOf (Const char *csKeyName,char *reference,int size);
int				EXP_LVL1	CS_getSourceOf (Const char *csKeyName,char *source,int size);
int				EXP_LVL1	CS_getUnitsOf (Const char *csKeyName,char *unitName,int size);
void			EXP_LVL1	CS_geoidCls (void);
int				EXP_LVL1	CS_geoidHgt (Const double ll_84 [2],double *height);
int				EXP_LVL1	CS_csGrpEnum (int index,char *grp_name,int name_sz,char *grp_dscr,int dscr_sz);
int				EXP_LVL1	CS_isgeo (Const char *cs_nam);
double			EXP_LVL1	CS_llazdd (double e_rad,double e_sq,double ll_from [3],double ll_to [3],double *dist);
int				EXP_LVL1	CS_llFromMgrs (double latLng [2],const char* mgrsString);
int				EXP_LVL1	CS_mgrsSetUp (const char* ellipsoid,short bessel);
int				EXP_LVL1	CS_mgrsFromLl (char* result,double latLng [2],int prec);
int				EXP_LVL1	CS_prjEnum (int index,ulong32_t *prj_flags,char *prj_keynm,int keynm_sz,char *prj_descr,int descr_sz);
void			EXP_LVL1	CS_recvr (void);
double			EXP_LVL1	CS_scale (Const char *cs_nam,double ll [3]);
double			EXP_LVL1	CS_scalh (Const char *cs_nam,double ll [3]);
double			EXP_LVL1	CS_scalk (Const char *cs_nam,double ll [3]);
int				EXP_LVL1	CS_spZoneNbrMap (char *csKeyName,int is83);
int				EXP_LVL1	CS_unEnum (int index,int type,char *un_name,int un_size);
int				EXP_LVL1	CS_unEnumPlural (int index,int type,char *un_name,int un_size);
int				EXP_LVL1	CS_unEnumSystem (int index,int type);
double			EXP_LVL1	CS_unitlu (short type,Const char *name);
#endif

/* The following functions may be useful in the a HIgh Level Language
   environment, but do require the use of pointers. Note that these
   functions copy the definition into a memory location provided by
   the host module and the result MUST NOT BE FREE'd by CS_free(). */
#if defined (EXP_LVL2) || defined (EXP_LVL3)
int				EXP_LVL2	CS_getcs (Const char *cs_nam,struct cs_Csdef_ *cs_ptr);
int				EXP_LVL2	CS_getdt (Const char *dt_nam,struct cs_Dtdef_ *dt_ptr);
int				EXP_LVL2	CS_getel (Const char *el_nam,struct cs_Eldef_ *el_ptr);
int				EXP_LVL2 	CS_csEnumByGroup (int index,Const char *grp_name,struct cs_Csgrplst_ *cs_descr);
#endif

/* The High Performance Interface 

   These function should work, but have not be tested, as yet (3/2015), for DLL
   linkage.  They all pretty much rely on the ability to pass pointers to
   DLL heap memory around, and thus are more difficult to use and, perhaps,
   reduce the robustness of the resulting DLL.  The performance increase is
   actually minimal on modern processors with significant memory caches.  Use
   this interface at your own risk.

   Should you choose to use this interface, make sure you use CS_free to free
   the memory returned by CS_csdef(), CS_csloc(), CS_dtcsu(), CS_dtdef(), and
   CS_eldef ().  Note that CS_audflt), CS_dtdflt(), CS_eldftl(), and CS_ludft(),
   all return a pointer to a static variable in the DLL and all of these
   functions will be soon deprecated. */
#if defined (EXP_LVL3)
char*				EXP_LVL3	CS_audflt (Const char *angUnit);
int					EXP_LVL3	CS_cs2ll (struct cs_Csprm_ *csprm,double ll [3],double xy [3]);
int					EXP_LVL3	CS_cs3ll (struct cs_Csprm_ *csprm,double ll [3],double xy [3]);
double				EXP_LVL3	CS_cscnv (struct cs_Csprm_ *csprm,double ll [3]);
struct cs_Csdef_*	EXP_LVL3	CS_csdef (Const char *cs_nam);
int					EXP_LVL3	CS_csdel (struct cs_Csdef_ *csdef);
struct cs_Csprm_*	EXP_LVL3	CS_csloc (Const char *cs_nam);
double				EXP_LVL3	CS_cssch (struct cs_Csprm_ *csprm,double ll [3]);
double				EXP_LVL3	CS_cssck (struct cs_Csprm_ *csprm,double ll [3]);
double				EXP_LVL3	CS_csscl (struct cs_Csprm_ *csprm,double ll [3]);
void				EXP_LVL3	CS_dtcls (struct cs_Dtcprm_ *dtc_ptr);
struct cs_Dtcprm_*	EXP_LVL3	CS_dtcsu (struct cs_Csprm_ *src_cs,struct cs_Csprm_ *dest_cs,int dat_erf,int blk_erf);
int					EXP_LVL3	CS_dtcvt (struct cs_Dtcprm_ *dtc_ptr,double ll_in [3],double ll_out [3]);
int					EXP_LVL3	CS_dtcvt3D (struct cs_Dtcprm_ *dtc_ptr,Const double ll_in [3],double ll_out [3]);
struct cs_Dtdef_*	EXP_LVL3	CS_dtdef (Const char *dat_nam);
int					EXP_LVL3	CS_dtdel (struct cs_Dtdef_ *dtdef);
char*				EXP_LVL3	CS_dtdflt (Const char *dtKeyName);
struct cs_Eldef_*	EXP_LVL3	CS_eldef (Const char *el_nam);
int					EXP_LVL3	CS_eldel (struct cs_Eldef_ *eldef);
char*				EXP_LVL3	CS_eldflt (Const char *elKeyName);
void				EXP_LVL3	CS_free (void *ptr);
int					EXP_LVL3	CS_ll2cs (struct cs_Csprm_ *csprm,double xy [2],double ll [3]);
int					EXP_LVL3	CS_ll3cs (Const struct cs_Csprm_ *csprm,double xy [3],Const double ll [3]);
int					EXP_LVL3	CS_llchk (struct cs_Csprm_ *csprm,int cnt,double pnts [][3]);
char*				EXP_LVL3	CS_ludflt (Const char *linUnit);
int					EXP_LVL3	CS_xychk (struct cs_Csprm_ *csprm,int cnt,double pnts [][3]);
unsigned long		EXP_LVL3	csMapIdToId (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																	 enum EcsNameFlavor srcFlavor,
																	 unsigned long srcId);
unsigned long		EXP_LVL3	csMapNameToIdC (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																		enum EcsNameFlavor srcFlavor,
																		const char* srcName);
enum EcsMapSt		EXP_LVL3	csMapNameToNameC (enum EcsMapObjType type,char* trgName,
																		  size_t trgSize,
																		  enum EcsNameFlavor trgFlavor,
																		  enum EcsNameFlavor srcFlavor,
																		  const char* srcName);
enum EcsMapSt		EXP_LVL3	csMapIdToNameC (enum EcsMapObjType type,char* trgName,
																		size_t trgSize,
																		enum EcsNameFlavor trgFlavor,
																		enum EcsNameFlavor srcFlavor,
																		unsigned long srcId);
void				EXP_LVL3	csReleaseNameMapper (void);
#endif

#if defined (__VB__)
int				EXP_LVL1	CS_csEnumVb (int index,char *key_name,int size);
int				EXP_LVL1	CS_csRangeEnumVb (int index,char *key_name,int size);
int				EXP_LVL1	CS_dtEnumVb (int index,char *key_name,int size);
int				EXP_LVL1	CS_elEnumVb (int index,char *key_name,int size);
void			EXP_LVL1	CS_errmsgVb (char *user_bufr,int buf_size);
long32_t		EXP_LVL1	CS_ftoaVb (char *bufr,int size,double value,long frmt);
int				EXP_LVL1	CS_getDataDirectoryVb (char *data_dir,int dir_sz);
int				EXP_LVL1	CS_getDatumOfVb (Const char *csKeyName,char *datumName,int size);
int				EXP_LVL1	CS_getDescriptionOfVb (Const char *csKeyName,char *description,int size);
int				EXP_LVL1	CS_getEllipsoidOfVb (Const char *csKeyName,char *ellipsoidName,int size);
int				EXP_LVL1	CS_getReferenceOfVb (Const char *csKeyName,char *reference,int size);
int				EXP_LVL1	CS_getSourceOfVb (Const char *csKeyName,char *source,int size);
int				EXP_LVL1	CS_getUnitsOfVb (Const char *csKeyName,char *unitName,int size);
int				EXP_LVL1	CS_mgrsFromLlVb (char* result,int rslt_size,double latLng [2],int prec);
int				EXP_LVL1	CS_prjEnumVb (int index,ulong32_t *prj_flags,char *prj_keynm,int keynm_sz,char *prj_descr,int descr_sz);
int				EXP_LVL1	CS_unEnumVb (int index,int type,char *un_name,int un_size);
int				EXP_LVL1	CS_unEnumPluralVb (int index,int type,char *un_name,int un_size);

long32_t		EXP_LVL1	CS_mapIdToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																	enum EcsNameFlavor srcFlavor,
																	long32_t srcId);
long32_t		EXP_LVL1	CS_mapNameToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																	  enum EcsNameFlavor srcFlavor,
																	  const char* srcName);
long32_t		EXP_LVL1	CS_mapNameToNameVb (enum EcsMapObjType type,char* trgName,
																		size_t trgSize,
																		enum EcsNameFlavor trgFlavor,
																		enum EcsNameFlavor srcFlavor,
																		const char* srcName);
long32_t		EXP_LVL1	CS_mapIdToNameVb (enum EcsMapObjType type,char* trgName,
																	  size_t trgSize,
																	  enum EcsNameFlavor trgFlavor,
																	  enum EcsNameFlavor srcFlavor,
																	  long32_t srcId);
#endif
#ifdef __cplusplus
}
#endif
