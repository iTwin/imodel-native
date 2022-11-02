/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#if !defined(TOTAL_SPECIAL)
#define TOTAL_SPECIAL
#endif

#if !defined(GEOCOORD_ENHANCEMENT)
#define GEOCOORD_ENHANCEMENT
#endif

#include <json/BeJsValue.h>
#include <Geom/GeomApi.h>
#include "BaseGeoDefs.r.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include    <GeoCoord/IGeoTiffKeysList.h>

typedef struct cs_Csprm_             CSParameters;
typedef struct cs_GeodeticTransform_ CSGeodeticTransformDef;

#include "ExportMacros.h"
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>

/** @namespace BentleyApi::GeoCoordinates Geographic Coordinate System classes @see GeoCoordinate */
BEGIN_BENTLEY_NAMESPACE

namespace BeSQLite {
    typedef struct CloudContainer* CloudContainerP;
};

namespace GeoCoordinates {

#define GeoCoordParserStatusBase (0x4000)

enum GeoCoordParseStatus
  {
  GeoCoordParse_GeoCoordNotInitialized = GEOCOORDERR_GeoCoordNotInitialized,
  GeoCoordParse_Success = 0,
  GeoCoordParse_Error = 32768,

  GeoCoordParse_ParseError  = GeoCoordParserStatusBase + 1,
  GeoCoordParse_NoGCS   = GeoCoordParserStatusBase + 2,
  GeoCoordParse_NoDatum = GeoCoordParserStatusBase + 3,
  GeoCoordParse_NoEllipsoid = GeoCoordParserStatusBase + 4,
  GeoCoordParse_NoTransform = GeoCoordParserStatusBase + 5,
  GeoCoordParse_NoRoot = GeoCoordParserStatusBase + 6,
  GeoCoordParse_BadAlias = GeoCoordParserStatusBase + 7,
  GeoCoordParse_NodeNotFound = GeoCoordParserStatusBase + 8,
  GeoCoordParse_NoContent = GeoCoordParserStatusBase + 9,
  GeoCoordParse_BadProjectionMethod = GeoCoordParserStatusBase + 10,
  GeoCoordParse_UnknownProjectionMethod = GeoCoordParserStatusBase + 11,
  GeoCoordParse_BadProjectionParamsSection = GeoCoordParserStatusBase + 12,
  GeoCoordParse_BadProjectionParam = GeoCoordParserStatusBase + 13,
  GeoCoordParse_BadProjection = GeoCoordParserStatusBase + 14,
  GeoCoordParse_UnknownProjectionParam = GeoCoordParserStatusBase + 15,
  GeoCoordParse_InvalidParamForMethod = GeoCoordParserStatusBase + 16,
  GeoCoordParse_BadQuadrant = GeoCoordParserStatusBase + 17,
  GeoCoordParse_BadDomain = GeoCoordParserStatusBase + 18,
  GeoCoordParse_BadEllipsoid = GeoCoordParserStatusBase + 19,
  GeoCoordParse_BadEllipsoidRadius = GeoCoordParserStatusBase + 20,
  GeoCoordParse_BadTransformParam = GeoCoordParserStatusBase + 21,
  GeoCoordParse_BadTransform = GeoCoordParserStatusBase + 22,
  GeoCoordParse_BadTransformPath = GeoCoordParserStatusBase + 23,
  GeoCoordParse_BadTransformParamSection = GeoCoordParserStatusBase + 24,
  GeoCoordParse_BadGridFileDef = GeoCoordParserStatusBase + 25,
  GeoCoordParse_UnknownGridFileFormat = GeoCoordParserStatusBase + 26,
  GeoCoordParse_MREGNotSupported = GeoCoordParserStatusBase + 27,
  GeoCoordParse_MissingPropertyOrParameter = GeoCoordParserStatusBase + 28,
  GeoCoordParse_BadDatum = GeoCoordParserStatusBase + 29,
  GeoCoordParse_BadGCS = GeoCoordParserStatusBase + 30,
  GeoCoordParse_UnknownEllipsoid = GeoCoordParserStatusBase + 31,
  GeoCoordParse_UnknownDatum = GeoCoordParserStatusBase + 32,
  GeoCoordParse_InvalidDefinition = GeoCoordParserStatusBase + 33,
  GeoCoordParse_UnknownTransformMethod = GeoCoordParserStatusBase + 34,
  GeoCoordParse_BadExtension = GeoCoordParserStatusBase + 35,
  GeoCoordParse_BadPrimeMeridian = GeoCoordParserStatusBase + 36,
  GeoCoordParse_BadAxis = GeoCoordParserStatusBase + 37,
  GeoCoordParse_BadVertical = GeoCoordParserStatusBase + 38,
  GeoCoordParse_UnsupportedProjectionParam = GeoCoordParserStatusBase + 39,
  GeoCoordParse_UnknownUnit = GeoCoordParserStatusBase + 40,
  GeoCoordParse_UnsupportedMeridian =  GeoCoordParserStatusBase + 41,
  GeoCoordParse_BadUnit = GeoCoordParserStatusBase + 42,

  };

// NOTE: This was added to meet the DOT requirements for setting the Vertical Datum separately from the Datum.
enum VertDatumCode
    {
    vdcFromDatum    = 0,    // Vertical Datum implied by Datum. This value is interpreted differently depending on the datum
                            // By default it represents the WGS84 ellipsoid vertical datum but if the datum is a
                            // variation of NAD83 (excluding canadian variations)
                            // then the value is interpreted as NAVD88. If the datum is NAD27 then it is interpreted as NGVD29
    vdcNGVD29       = 1,    // Vertical Datum of 1929
    vdcNAVD88       = 2,    // Vertical Datum of 1988.
    vdcGeoid        = 3,    // Other Geoid (indicates GeoidHeight.gdc catalog should be used)
    vdcEllipsoid    = 4,    // WGS84 Ellipsoid vertical datum. Since this value cannot be stored in the DGN file without making the file
                            // somewhat incompatible with v8i products it should only be used if datum is NAD83 or NAD27
                            // This value will be interpreted as the GCS datum if it is coincident with WGS84 or WGS84 otherwise.
    vdcLocalEllipsoid = 5   // This value is to be interpreted as the local GCS datum if this datum is not coincident with WGS84.
                            // In that case the elevation difference with the other datum in case of transformation will be applied.
                            // Use of this value is discouraged.
    };

// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy it. Check it.
enum WGS84ConvertCode
    {
    ConvertType_NONE      =   0,
    ConvertType_MOLO      =   1,
    ConvertType_MREG      =   2,
    ConvertType_BURS      =   3,
    ConvertType_NAD27     =   4,
    ConvertType_NAD83     =   5,
    ConvertType_WGS84     =   6,
    ConvertType_WGS72     =   7,
    ConvertType_HPGN      =   8,
    ConvertType_7PARM     =   9,
    ConvertType_AGD66     =   10,
    ConvertType_3PARM     =   11,
    ConvertType_6PARM     =   12,
    ConvertType_4PARM     =   13,
    ConvertType_AGD84     =   14,
    ConvertType_NZGD4     =   15,
    ConvertType_ATS77     =   16,
    ConvertType_GDA94     =   17,
    ConvertType_NZGD2K    =   18,
    ConvertType_CSRS      =   19,
    ConvertType_TOKYO     =   20,
    ConvertType_RGF93     =   21,
    ConvertType_ED50      =   22,
    ConvertType_DHDN      =   23,
    ConvertType_ETRF89    =   24,
    ConvertType_GEOCTR    =   25,
    ConvertType_CHENYX    =   26,
    ConvertType_GENGRID   =   27,
    ConvertType_MAXVALUE  =   27,       // the maximum allowable value.
    };

// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy it. Check it.
enum class GenConvertCode
    {
    GenConvertType_NONE      =   0x1001, // cs_DTCMTH_NULLX,
    GenConvertType_WGS72     =   0x1002, // cs_DTCMTH_WGS72,
    GenConvertType_3PARM     =   0x2001, // cs_DTCMTH_3PARM,
    GenConvertType_MOLO      =   0x2002, // cs_DTCMTH_MOLOD,
    GenConvertType_GEOCTR    =   0x2004, // cs_DTCMTH_GEOCT,
    GenConvertType_4PARM     =   0x2005, // cs_DTCMTH_4PARM,
    GenConvertType_6PARM     =   0x2006, // cs_DTCMTH_6PARM,
    GenConvertType_BURS      =   0x2007, // cs_DTCMTH_BURSA,
    GenConvertType_7PARM     =   0x2009, // cs_DTCMTH_7PARM,
    GenConvertType_BDKAS     =   0x200A, // cs_DTCMTH_BDKAS,
    GenConvertType_GFILE     =   0x3000, // cs_DTCMTH_GFILE,
    GenConvertType_MREG      =   0x5001, // cs_DTCMTH_MULRG,
    };

/*=================================================================================**//**
* @addtogroup GeoCoordinate

A Geographic Coordinate System describes the way that coordinates on the earth's surface
(which are generally described in degrees of longitude, degrees of latitude, and elevation
above the surface) are transformed to a cartesian coordinate system that can be represented
on an inherently planar medium such as a sheet of paper or a computer screen.
<p>
In general, a Geographic Coordinate System (referred to below as a GeoCoordinate System or GCS)
is fully described by a projection type, the mathematical parameters that customize the projection,
and a datum or ellipsoid.
<p>
The projection type describes the mathematical formula for taking a position in longitude, latitude,
and elevation on (or near) the earth's surface and converting it x, y, and z cartesian coordinates.
<p>
The datum describes the assumptions in force when a particular latitude, longitude measurement
was made, including the shape of the earth and the reference points to which the measurements was relative.
There are a number of well-known datum, almost all of which were originally published by one governmental
body or another.
<p>
In some cases, measurements are relative to an ellipsoid rather than a datum. In that case, the
latitude and longitude are assumed to be mathematically correct on the ellipsoid's surface. Like datums,
there are a number of well-known ellipsoid definitions.
<p>
There are many different Geographic Coordinate Systems in use, and it is always possible to create new
variations, either by inventing a new projection type, varying projection parameters for existing
projection types, or by varying the datum. Often the best choice is to use a GCS that has been established
as a standard for the relevant geographic area, usually by a governmental body. For example, one or more
"State Plane" geographic coordinate systems have been defined by state governments in the United States.
An extensive Coordinate System Library is supplied with Bentley products, and Geographic
Coordinate Systems from that library can be used by looking them up by name - see the appropriate GCS constructors.
<p>
A full discussion of Geographic Coordinate Systems can be found in "Elements of Cartography" by Arthur
H. Robinson, et al. ISBN-10: 0471728055, and in many other books on the subject.
<p>
The GeoCoordinate API makes use of the "CS_MAP" library published by Mentor Software, Inc. for all the
projection calculations and datum conversion algorithms. Mentor also supplies the Coordinate System
Library.
<p>
Here are some concepts and API classes explained.
<p>
There are various models to represent geographic coordinate systems. One of the most common is based
on the ISO 19111 model based or a basis to other models used by various vendors, APIs,
including the EPSG database.
<p>
This model is highly complex and mostly object oriented. It defines concepts such as Coordinate
Reference Systems (CRS) that can be of many types (Projected, Geographic, Geocentric, Vertical, ...) Individual
CRS can be combined together into compound coordinate systems.
CRS depending on their types make used of another CRS or Geodetic Datums, projections,
geodetic transformations, units of measures, axises definitions, area of use and so on.
This model was judged too complex for most purpose. The ISO 19111 model can represent any possible
variation of CRS but the complexity renders it difficult to use in a most specific cases.
<p>
The model adopted by Bentley Systems is derived from the CSMAP Open Source library upon which we are based.
In this model basically 3 major concepts are implemented:
 - Coordinate Reference System
 - Geodetic Datum
 - Geodetic Ellipsoid

A Coordinate Reference System (represented by the BaseGCS class) contains all map projection
parameters. The projections projects a map based on a geodetic datum it is based upon.
The datum itself is based on a model of the shape of the Earth called a Geodetic Ellipsoid.
Originally the datum defined a single geodetic transformation to another geodetic datum WGS84.
These transform definitions are then stored directly in the datum definition mostly based on
a variation of geodetic geocentric transformations (3 parameter, Molodensky, geocentric, 4 or 6 parameters,
Bursa-Wolfe and 7 parameter transformation).

Latest versions of CSMAP introduced, additionally, the concepts of independent Geodetic Transforms stored in another
mostly different concept. This introduction allowed better support for grid shift file
based geodetic transformations that had to be built-in the system prior to this modification.
The latest CSMAP enabled definition of geodetic transformation to other target datum that were not
the common base WGS84 which, in turn, enabled to have a list of multiple geodetic transformations
chained together. This permitted the proper support of many previously unsupported transforms
such as for Slovakia or Switzerland.

In any case, though Bentley System decided to bind more closely the various concepts than CSMAp model is we nevertheless
use the two additional concepts
 - Geodetic Transform
 - Geodetic Path

The full object oriented model is thus:

A CRS is based on a Datum which also uses an Ellipsoid. The datum is the start of a geodetic transform path
comprised of one or more geodetic transforms leading to WGS84.
In theory there can be many geodetic transform paths originating from a datum. These serve either as shorcuts but also
to implement a different path to a designated target that will be different from the combination of
transformations from the first datum to WGS84 then to the second datum.

The BaseGCS class contains more than the horizontal portion of a CRS. It also contains
a vertical datum identifier indicating the vertical basis. It may optionally include an additional
local transformation (usually an Helmert Transformation) that adds a scaling and translation component
to the BaseGCS coordinates.

CRS storage and pre-definition.

Bentley Systems adopted a workflow where the use of pre-defined CRS is encouraged but custom made CRS is supported.
There are two types of pre-defined CRS being the system library provided or the user-defined library stored.
Additionally fully custom defined CRS are also supported enabling, for example, to support workflows where a
DGN file makes reference to a CRS stored in a user-defined library that is not available. The content
of the DGN element storing the CRS definition allows to support the unknown CRS.

Note that, currently, geodetic transforms of any types are supported they cannot yet be stored in a model file (TODO).

IMPORTANT: Libraries (or dictionaries) store only the horizontal portion of a CRS. There is not storage
of either vertical datum or local transform in a library. These are modifiers that are added on top
of the horizontal portion of the projection.

*/

/*
Maintenance notes:

The current section is intended for internal Bentley Systems developpers in charge of maintaining
the GeoCoord library.

CSMAP structure
The GeoCoord library is based on CSMAP, an Open Source project owned by Autodesk.
We rely upon the class structs, definitions, and API of the CSMAP library.
Here is a rapid survey of the CSMAP structures used.

CSMAP uses two sets of classes for every concept. One class contains the definition
where the individual parameters and properties are set. For each definition classes there exist
a more complex sister class where the intermediate values and external data has been gathered
into a mostly pre-computed class ready for immediate use for coordinate transformation.

For example there exist a class cs_CSDef_ (typedefed CSDefinition) containing the raw definition of the CRS including
the projection method used, parameters and keyname of the datum used.
The sister class named cs_Csprm_ (typedefed CSParameters) aggregates all pre-computed geodetic datum and ellipsoid
parameters and provides an interface ready for immediate transformation.

    Concept             Definition Class and alias               Processing ready class and alias        Corresponding GeoCoord class
    CRS              cs_Csdef_ / CSDefinition                    cs_Csprm_ / CSParameters                   BaseGCS
    Datum            cs_Dtdef_ / CSDatumDef                      cs_Datum_ / CSDatum                        Datum
    Ellipsoid        cs_Eldef_ / CSEllipsoidDef                  ** not used **                             Ellipsoid
    Transform        cs_Transform / CSGeodeticTransformDef       cs_GxXForm_ / CSGeodeticTransform          GeodeticTransform
    Transform Path     ** not used **                            cs_Dtcprm_ / CSDatumConvert                GeodeticTransformPath / DatumConverter

Processing ready ellipsoid class is unused as there are few valid operations requiring a single ellipsoid.
The CSMAP Geodetic Path transform definition class is not used. We opted for a simple list of Geodetic Transforms
to represent encapsulated into the GeodeticTransformPath class. For the transform path we do have
two classes mimicking the definition/process ready strategy adopted by CSMAP.
Geodetic Transforms are highly complex and require numerous checks and validations to promote from the
definition to the process ready. This notably include use of external grid shift files that may
be absent, thus the difference between definition and process ready classes.


The process ready class CSParameters contains all defining parameters for both Datum and ellipsoid but not
any geodetic transform external to the datum geocentric transform definition.

The geodetic transforms and path are considered conceptually bound to the datum yet the BaseGCS definitions
and construction do not allow specification of this extended datum concept. For this reason we introduced
two additional path for creating a BaseGCS with an non-datum stored geodetic transform.

The first one implies providing a single geodetic transform definition at the creation of a BaseGCS using

BaseGCSPtr CreateGCS (CSParameters const& csParameters,
                      int32_t coordSysId,
                      CSGeodeticTransformDef* geodeticTransform = nullptr);

This signature is compatible with the user-defined libraries that allow a single geodetic transform
associated to a user-defined datum. Yet this signature does not allow specification of multiple
geodetic transforms as may be required for non-library stored GCS.

The second method requires setting the datum using the Datum class that will contain the geodetic transform definition
or definitions. Using the SetDatum() and GetDatum() will provide access to all datum properties as were already
exposed using the duplicate datum and ellipsoid properties interfaces of BaseGCS.

NOTE: Geodetic Transforms and Geodetic Transform paths are not stored with the datum definition in the system dictionary.
These are instead computed relative to a designated target datum during the creation of the Datum Converter.
This process is highly complex and is mostly performed by a significantly modified version of CSMAP.
A combination of entries located in the GeodeticTransform.dty file, and GeodeticPath.dty plus the geocentric definition
included in the datum are all used to obtain a datum converter between system defined datums or system
datums and geocentric defined user-defined datums. User defined datums based on grid shift file cannot
be inferred during the datum converter creation process and must be provided.
This is the reason these geodetic transforms are stored directly in the datum object.

The same applies to self-contained non stored datum definition that can then have multiple
geodetic transform steps chained together.
*/


#if defined (_MANAGED)
#define MPUBLIC public
#else
#define MPUBLIC
#endif

typedef class BaseGCS*                  BaseGCSP;
typedef class BaseGCS const*            BaseGCSCP;
typedef class BaseGCS&                  BaseGCSR;
typedef class BaseGCS const&            BaseGCSCR;
typedef RefCountedPtr<BaseGCS>          BaseGCSPtr;
typedef RefCountedCPtr<BaseGCS>         BaseGCSCPtr;

typedef class LocalTransformer*         LocalTransformerP;
typedef class LocalTransformer const*   LocalTransformerCP;
typedef RefCountedPtr<LocalTransformer> LocalTransformerPtr;

typedef class DatumConverter*       DatumConverterP;
typedef class Datum const*        DatumCP;
typedef class Datum*              DatumP;

class GridFileDefinition;

/*=================================================================================**//**
*
* Geographic Coordinate System class.
*
* The BaseGCS class allows the conversion of Coordinates in Geographic Latitude/Longitude/Elevation
* to and from a Cartesian coordinate system in x, y, and z. The units of the Cartesian coordinates are determined
* by the units of the projection. Those units are usually a linear measure, but can be Degrees if the
* coordinate system is a "Unity" coordinate system in CSMap terms.<p>
*
* This class is reference counted. Users should use the usual reference counting semantics - if
* a pointer is retained, call the AddRef() method. When the reference is no longer needed, call the Release() method.
* Application code should never call the "delete" operator on an instance of BaseGCS.
* The BaseGCSPtr type is a smart pointer that makes following those rules easy.<p>
*
* The BaseGCS class provides methods for converting from Longitude/Latitude/Elevation to x, y, and z in
* the units of the coordinate system. When an input point is in latitude/longitude/elevation, the longitude in decimal
* degrees is the x component of the DPoint3d, the latitude is the y component, and the elevation is the z component.<p>
*
* There is also a method for converting from Longitude/Latitude/Elevation in one BaseGCS to Long/Latitude/Elevation
* in a target coordinate system. This does the datum conversion that is necessary when the two GCS's utilize different
* datum.
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class EXPORT_VTABLE_ATTRIBUTE BaseGCS : public RefCountedBase {
protected:
    CSParameters* m_csParameters;                   // all coordinate system parameters, gathered for use by the CSMap transformation functions.
    mutable Datum* m_datum;                         // This property contains either the datum loaded from the library or a custom specified
                                                    // datum set using SetDatum(), depending on the value of m_customDatum
    mutable bool m_customDatum;                     // Flag that indicates if the datum is custom set using SetDatum()
    mutable BaseGCSCP m_targetGCS;                  // current target coordinate system.
    mutable bvector<BaseGCSCP> m_listOfPointingGCS; // List of BaseGCS that are using the current BaseGCS as a cached target GCS
    mutable DatumConverterP m_datumConverter;       // datum converter from this Lat/Long to the Lat/Long of m_targetGCS.
    bool m_reprojectElevation;                      // if true, LatLongFromLatLong adjusts elevation values.
    int32_t m_coordSysId;                           // our internal coordinate system ID
    VertDatumCode m_verticalDatum;
    mutable int32_t m_csError;
    bool m_canEdit;
    LocalTransformerPtr m_localTransformer;   // The local transformer converts to or from the GCS Cartesian coordinates to Local Cartesian coordinates.
    mutable Utf8StringP m_nameString; // these are here as "adapters" needed because CS_Map uses all char rather than unicode.
    mutable Utf8StringP m_descriptionString;
    mutable Utf8StringP m_projectionString;
    mutable Utf8StringP m_datumNameString;
    mutable Utf8StringP m_datumDescriptionString;
    mutable Utf8StringP m_ellipsoidNameString;
    mutable Utf8StringP m_ellipsoidDescriptionString;
    mutable IGeoTiffKeysList* m_originalGeoKeys; // A size of 0 indicates that the BaseGCS was not initialized from GeoTiffKeys or has been modified since init.
    mutable Utf8StringP m_originalWKT;              // A NULL or empty string indicates the BaseGCS was not initialized from a Well Known Text or as been modified since init.
    mutable bool m_modified; // Indicates if the BaseGCS has been modified from original definition. At the moment it is only internal.
    mutable short m_foundEPSGCode; // Used to store the EPSG code resulting from a search through dictionary. Value of zero indicates not set or no EPSG code.
    static BaseGCSPtr s_LL84GCS; // used for all static functions that need ECEF
    friend struct GeoTiffKeyInterpreter;
    friend struct GeoTiffKeyCreator;

    void Init();
    void InitHorizontal();

    BASEGEOCOORD_EXPORTED BaseGCS(BaseGCSCR source);
    BASEGEOCOORD_EXPORTED BaseGCS();
    BASEGEOCOORD_EXPORTED BaseGCS(Utf8CP coordinateSystemKeyName);
    BASEGEOCOORD_EXPORTED BaseGCS(CSParameters& csParameters, int32_t coordSysId, CSGeodeticTransformDef const* geodeticTransform = nullptr);
    BASEGEOCOORD_EXPORTED virtual ~BaseGCS();
    DatumConverterP SetupDatumConverterFor(BaseGCSCR destGCS) const;

    /*---------------------------------------------------------------------------------**/ /**
    * USED By another Base GCS only
    * This method is called by a BaseGCS upon which the 'this' BaseGCS is registered.
    * By calling this method, a BaseGCS indicates that the link between BaseGCS established
    * by the m_targetGCS and m_datumConverter members must be severed.
    * Note that only mutable members are modified thus the constness of the method
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED void ClearCache() const;

    /*---------------------------------------------------------------------------------**/ /**
    * Called by another BaseGCS to indicate that this GCS given as a parameter used the
    * present BaseGCS as part of his cache members m_targetGCS.
    * When the value of m_targetGCS is changed the BaseGCS must unregister form the
    * target GCS using UnRegisterIsADestinationOf() prior to setting the member to the
    * new value.
    * Note that only mutable members are modified thus the constness of the method
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED void RegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const;

    /*---------------------------------------------------------------------------------**/ /**
    * Called by another BaseGCS to indicate that this GCS given as a parameter used to be
    * making a references to the present BaseGCS as part of his cache members m_targetGCS.
    * The present call indicates the link is severed and the BaseGCS are not linked anymore.
    * Note that only mutable members are modified thus the constness of the method
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED void UnRegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const;

    /*---------------------------------------------------------------------------------**/ /**
    * Called to indicate the BaseGCS has been modified or not from the original definition.
    * this internal method should be called with true whenever a definition parameter is set
    * and called false whenever the definition is complete from one of the Init method
    * or a call to DefinitionComplete()
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetModified(bool modified);

public:
    BASEGEOCOORD_EXPORTED static void EnableLocalGcsFiles(bool yesNo);

    // Add a list of WorkspaceDbs to find GCS resources.
    // @param dbName the workspace database name containing GCS resources.
    // @param container the CloudContainer holding the dbs (or nullptr)
    // @param priority 0=highest (loaded first)
    BASEGEOCOORD_EXPORTED static bool AddWorkspaceDb(Utf8String dbName, BeSQLite::CloudContainerP container, int priority);

    BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS(CSParameters const& csParameters, int32_t coordSysId);
    BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS(CSParameters const& csParameters, int32_t coordSysId, CSGeodeticTransformDef* geodeticTransform = nullptr);

    /*---------------------------------------------------------------------------------**//**
    * Initialize the Geographic Coordinate System libraries.
    * @param  dataDirectory  IN  The directory that contains the CSMap data files.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED static StatusInt Initialize(Utf8CP dataDirectory);

    BASEGEOCOORD_EXPORTED static void Shutdown();

    /*---------------------------------------------------------------------------------**/ /**
     * Indicates if the Geographic Coordinate System library was initialized.
     * PP performs the
     * initialization if running within that environment. If basegeocoord.dll is used by
     * a standalone program, that program is responsible for initialization.
     * @return true if geocoord has been initialized.
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED static bool IsLibraryInitialized();

    /*---------------------------------------------------------------------------------**/ /**
     * Initialize the Base ECEF GCS that is needed for static functions GetLinearTransformECEF()
     * and CartesianFromECEF()
     * @return true if Base ECEF GCS has been initialized or was already initialized
     * @bsimethod
     +---------------+---------------+---------------+---------------+---------------+------*/
    BASEGEOCOORD_EXPORTED static bool InitializeBaseGcsECEF();

public:

/*---------------------------------------------------------------------------------**//**
* Returns an empty BaseGCSPtr. This factory method is designed
* to be used in conjunction with the initialization methods such as the InitAzimuthalEqualArea
* method.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS ();

/*---------------------------------------------------------------------------------**//**
* Initializes a BaseGCS by looking for the specified key name in the Coordinate System Library.
* If the coordinate system specified is found, then the IsValid method returns true. Otherwise the
* GetError method returns the CS_MAP error code.
* @param    coordinateSystemKeyName    IN  The key name of the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS(Utf8CP coordinateSystemKeyName);

/*---------------------------------------------------------------------------------**//**
* Returns a copy of the object.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS(BaseGCSCR baseGcs);

/*---------------------------------------------------------------------------------**//**
* Allocates an empty baseGCS which will be in the valid but modified state.
* once allocated the m_csParameters structure can be filled and once complete
* DefinitionComplete() be called. This method is intended for manual creation of
* a BaseGCS after CreateGCS() has been called (no parameters).
* Note that vertical datum set is unchanged.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void AllocateClean();

/*---------------------------------------------------------------------------------**//**
* Clears all cache members and memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Clear();

/*---------------------------------------------------------------------------------**/ /**
* This method is only intended to be called when previous conversion call returned warnings
* related to datum conversion (typically grid files missing). If after such a warning
* actions are taken to remedy to the missing files then calling this method will clear
* the conversion cache insuring datum converter is computed anew using newly installed
* data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void ClearConverterCache() const;

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the no-argument contructor to set the BaseGCS to
* an Azimuthal Equal Area projection. Such a projection is the mathematical equivalent of
* placing a flat sheet of paper on the surface of the area at originLongitude, originLatitude,
* projecting the earth's surface onto that paper, and then adding the "falseEasting", "falseNorthing"
* values to yield cartesian coordinates.
* @return   SUCCESS or a CS_MAP error code.
* @param    errorMsg        OUT     if non-NULL, the Utf8String is filled in with the CS_MAP error
*                                   message when an error occurs.
* @param    datumName       IN      The name of the datum used in the GCS, such as "WGS84".
* @param    unitName        IN      The name of the linear unit for the Cartesian coordinates, such as "METER".
* @param    originLongitude IN      The longitude of the tangency point.
* @param    originLatitude  IN      The latitude of the tangency point.
* @param    azimuthAngle    IN      The angle, clockwise from true north in decimal degrees, of the rotation to be applied.
* @param    scale           IN      This argument is ignored. The scale is always 1.0.
* @param    falseEasting    IN      The value to add to each Cartesian X value.
* @param    falseNorthing   IN      The value to add to each Cartesian Y value.
* @param    quadrant        IN      Quadrant for the cartesian coordinate system. If north is up and east is right, pass 1.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitAzimuthalEqualArea
(
Utf8StringP             errorMsg,
Utf8CP                  datumName,
Utf8CP                  unitName,
double                  originLongitude,
double                  originLatitude,
double                  azimuthAngle,
double                  scale,
double                  falseEasting,
double                  falseNorthing,
int                     quadrant
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the no-argument contructor to set the BaseGCS to
* an Transverse Mercator projection.
* @return   SUCCESS or a CS_MAP error code.
* @param    errorMsg        OUT     if non-NULL, the Utf8String is filled in with the CS_MAP error
*                                   message when an error occurs.
* @param    datumName       IN      The name of the datum used in the GCS, such as "WGS84".
* @param    unitName        IN      The name of the linear unit for the Cartesian coordinates, such as "METER".
* @param    originLongitude IN      The longitude of the tangency point.
* @param    originLatitude  IN      The latitude of the tangency point.
* @param    scale           IN      This scale reduction at the origin.
* @param    falseEasting    IN      The value to add to each Cartesian X value.
* @param    falseNorthing   IN      The value to add to each Cartesian Y value.
* @param    quadrant        IN      Quadrant for the cartesian coordinate system. If north is up and east is right, pass 1.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitTransverseMercator
(
Utf8StringP             errorMsg,
Utf8CP                  datumName,
Utf8CP                  unitName,
double                  originLongitude,
double                  originLatitude,
double                  scale,
double                  falseEasting,
double                  falseNorthing,
int                     quadrant
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the no-argument contructor to set the BaseGCS to
* give cartesian angular values. This is often useful for applying datum shifts when
* you have Longitude Latitude values in one datum and you need then in a different datum.
* @return   SUCCESS or a CS_MAP error code.
* @param    errorMsg        OUT     if non-NULL, the Utf8String is filled in with the CS_MAP error
*                                   message when an error occurs.
* @param    datumName       IN      The name of the datum used in the GCS, such as "WGS84".
* @param    ellipsoidName   IN      The name of the ellipsoid used in the GCS, such as "WGS84". This is used only if the datumName is NULL.
* @param    unitName        IN      The name of the linear unit for the Cartesian coordinates, such as "METER".
* @param    originLongitude IN      Allows displacement of the longitude values if a different origin is desired - usually 0.0.
* @param    originLatitude  IN      Allows displacement of the latitude values if a different origin is desired - usually 0.0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitLatLong
(
Utf8StringP             errorMsg,
Utf8CP                  datumName,          // Datum
Utf8CP                  ellipsoidName,      // only if datum is NULL.
Utf8CP                  unitName,           // usually "DEGREE"
double                  originLongitude,    // displacement from Greenwich
double                  originLatitude      // displacement from Greenwich
);

// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy it. Check it.
enum WktFlavor
    {
    wktFlavorOGC        = 1,    // Open Geospatial Consortium flavor
    wktFlavorGeoTiff    = 2,    // GeoTiff flavor.
    wktFlavorESRI       = 3,    // ESRI flavor.
    wktFlavorOracle     = 4,    // Oracle flavor.
    wktFlavorGeoTools   = 5,    // GeoTools flavor
    wktFlavorEPSG       = 6,    // EPSG flavor
    wktFlavorOracle9    = 7,    // Oracle 9 flavor
    wktFlavorAutodesk   = 8,    // Autodesk and default value since CSMAP was bought

    // Note concerning these last entries and specifically wktFlavorUnknown
    // The values for these used to be Unknown = 7 AppAlt = 8 and LclAlt = 9
    // As the latter two were yet unsupported they cannot cause any issue but the
    // Change for the Unknown may lead to a backward compatibility issue for libraries if additional
    // values are added. Make sure these changes occur in between major versions only.
    wktFlavorUnknown    = 9,    // used if the flavor is unknown. InitFromWellKnownText will do its best to figure it out.
    wktFlavorAppAlt     = 10,   // Not yet supported
    wktFlavorLclAlt     = 11,   // Not yet supported
    };

enum RangeTestResult
    {
    RangeTestOk                 = 0,   // The points are within the useful range
    RangeTestOutsideRange       = 1,   // one of more points outside the useful range of the coordinate system.
    RangeTestOutsideMathDomain  = 2,   // one or more points outside the mathematical domain of the coordinate system.
    };

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from a
* "well known text" string.
* @return   SUCCESS or a CS_MAP error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the Utf8String is filled in with the CS_MAP warning or error message.
* @param    wktFlavor       IN      The WKT Flavor. If not known, use wktFlavorUnknown.
* @param    wellKnownText   IN      The Well Known Text specifying the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromWellKnownText
(
StatusInt*              warning,
Utf8StringP             warningErrorMsg,
WktFlavor               wktFlavor,
Utf8CP                  wellKnownText
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from a
* "OSGEO XML" string.
* @return   SUCCESS or a CS_MAP error code.
* @param    autoXML   IN      The XML specifying the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoCoordParseStatus       InitFromOSGEOXML
(
    Utf8CP                 osgeoXML
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from a
* "well known text" string.
* @return   SUCCESS or a CS_MAP error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the Utf8String is filled in with the CS_MAP warning or error message.
* @param    wellKnownText   IN      The Well Known Text specifying the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoCoordParseStatus         InitFromWellKnownText
(
StatusInt*              warning,
Utf8StringP             warningErrorMsg,
Utf8CP                  wellKnownText
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from an
* EPSG coordinate system code. The valid EPSG code ranges are from 20000 through 32767 for projected coordinate systems
* and 4000 through 4199 for geographic (Lat/long) coordinate systems.
* @return   SUCCESS or a CS_MAP error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the Utf8String is filled in with the CS_MAP warning or error message.
* @param    epsgCode        IN      The EPSG code for the desired coordinate system.
* @remarks  Only those EPSG coordinate systems that are in our library will be successfully created.
*           The method first looks for a coordinate system named EPSGnnnnn, where nnnnn is the EPSG code.
*           If that fails, it looks in CS-Map's lookup table to see if the EPSG code appears there.
*           If that fails, it returns an error code and the IsValid method of the coordinate system will return false.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromEPSGCode
(
StatusInt*              warning,
Utf8StringP             warningErrorMsg,
int                     epsgCode
);

/*---------------------------------------------------------------------------------**//**
* Gets the Well Known Text string from a coordinate system definition.
* @return   SUCCESS or a CS_MAP error code.
* @param    wellKnownText     Out     The Well Known Text specifying the coordinate system.
* @param    wktFlavor         IN      The WKT Flavor desired. If not known, use wktFlavorUnknown
* @param    originalIfPresent IN      true indicates that if the BaseGCS originates from a
*                                     WKT fragment then this WKT should be returned. In this
*                                     case the wktFlavor is only used if an original was not present.
*                                     This parameter is optional regardless optional parameters
*                                     are discouraged. For reasons of backward compatibility
*                                     this was considered necessary.
* @param doNotInsertTOWGS84 IN If true indicates that the TOWGS84 clause should not be added.
*                              default is false which indicates to add it if applicable to flavor
*                              and datum transformation.
* @param posVectorRotationSignConvention IN If true indicates that the TOWGS84 rotation signal
*                            convention should follow Position Vector (EPSG:9607) convention.
*                            The default is false to use the Coordinate Frame (EPSG:9606)
*                            convention.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetWellKnownText
(
Utf8StringR             wellKnownText,
WktFlavor               wktFlavor,
bool                    originalIfPresent = false,
bool                    doNotInsertTOWGS84 = false,
bool                    posVectorRotationSignConvention = false
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the COMPD_CS Well Known Text string from a coordinate system definition.
* This compound coordinate system is composed of a PROJCS or GEOCS section followed
* by a VERT_CS section that contains the definition of the vertical datum used.
* @return   SUCCESS or a CS_MAP error code.
* @param    wellKnownText     Out   The Well Known Text specifying the coordinate system.
* @param    wktFlavor         IN    The WKT Flavor desired. If not known, use wktFlavorUnknown
* @param    originalIfPresent IN    true indicates that if the BaseGCS originates from a
*                                   WKT fragment then this WKT should be returned. In this
*                                   case the wktFlavor is only used if an original was not present.
*                                   Note that if the original was not a compound WKT but a plain
*                                   WKT then this original fragment will be used internally
*                                   in the composition of the compound WKT.
*                                   This parameter is optional regardless optional parameters
*                                   are discouraged. For reasons of backward compatibility
*                                   this was considered necessary.
* @param doNotInsertTOWGS84 IN If true indicates that the TOWGS84 clause should not be added.
*                              default is false which indicates to add it if applicable to flavor
*                              and datum transformation.
* @param posVectorRotationSignConvention IN If true indicates that the TOWGS84 rotation signal
*                            convention should follow Position Vector (EPSG:9607) convention.
*                            The default is false to use the Coordinate Frame (EPSG:9606)
*                            convention.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetCompoundCSWellKnownText
(
Utf8StringR             wellKnownText,
WktFlavor               wktFlavor,
bool                    originalIfPresent = false,
bool                    doNotInsertTOWGS84 = false,
bool                    posVectorRotationSignConvention = false
) const;

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from a
* set of GeoTiff Keys. Note that the original list of geotiff keys will be preserved
* inside the BaseGCS object. This mechanism allows to obtain the geotiff keys exactly
* as they were when interpreted for the sake of conformity to a client data standard.
* @return   SUCCESS, a CS_MAP error code, or a GeoCoord error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the Utf8String is filled in with the CS_MAP warning or error message.
* @param    geoTiffKeys     IN      an object implementing the IGeoTiffKeysList interface.
* @param    allowUnitsOverride IN   if true then default units can be overriden by the presence
*                                   a proj linear unit even though the units associated to
*                                   the GCS specifies different units
*                                   This parameter is necessary since many client make use of this mechanism.
*                                   This parameter is optional regardless optional parameters
*                                   are discouraged. For reasons of backward compatibility
*                                   this was considered necessary.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromGeoTiffKeys
(
StatusInt*                  warning,
Utf8StringP                 warningErrorMsg,
GeoCoordinates::IGeoTiffKeysList const*   geoTiffKeys,
bool                    allowUnitsOverride = false
);

/*---------------------------------------------------------------------------------**//**
* Computes a linear transformation over a designated area that approximates the
* transformation between source and given target coordinate system.
*
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Denmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outTransform OUT the transform effective at the point elementOrigin in
*                            source coordinates
* @param    extent       IN  the extent in source GCS coordinate to use to find the transform.
*                            This extent must of course be valid (not empty) but shall also
*                            define an extent no less than 0.01 of the linear units of the input
*                            GCS wide in all dimensions. If the input GCS is longitude/latitude then
*                            the extent will be no less than 0.0000001 (1e-07) degrees for the first
*                            two ordinate and 0.01[Meter] for the elevation (z) ordinate.
* @param    targetGCS    IN  target coordinate system
* @param    maxError     OUT If provided receives the max error observed over the extent
* @param    meanError    OUT If provided receives the mean error observed over the extent
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED  ReprojectStatus     GetLinearTransform
(
    TransformP              outTransform,
    DRange3dCR              extent,
    BaseGCSCR               targetGCS,
    double*                 maxError,
    double*                 meanError
) const;

/*---------------------------------------------------------------------------------**//**
* Computes a linear transformation over a designated area that approximates the
* transformation between ECEF and given target coordinate system.
*
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Denmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outTransform OUT the transform effective at the point elementOrigin in
*                            source coordinates
* @param    extentECEF   IN  the extent in ECEF coordinate to use to find the transform.
*                            This extent must of course be valid (not empty) but shall also
*                            define an extent no less than 0.01[meter] wide in all three
*                            dimensions.
* @param    targetGCS    IN  target coordinate system
* @param    maxError     OUT If provided receives the max error observed over the extent
* @param    meanError    OUT If provided receives the mean error observed over the extent
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED  static ReprojectStatus     GetLinearTransformECEF
(
    TransformP              outTransform,
    DRange3dCR              extentECEF,
    BaseGCSCR               targetGCS,
    double*                 maxError,
    double*                 meanError
);

/*---------------------------------------------------------------------------------**//**
* Computes a linear transformation over a designated area that approximates the
* transformation between this source coordinate system and ECEF.
*
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Denmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outTransform OUT the transform effective at the point elementOrigin in
*                            source coordinates
* @param    extent       IN  the extent in source GCS coordinate to use to find the transform.
*                            This extent must of course be valid (not empty) but shall also
*                            define an extent no less than 0.01 of the linear units of the input
*                            GCS wide in all dimensions. If the input GCS is longitude/latitude then
*                            the extent will be no less than 0.0000001 (1e-07) degrees for the first
*                            two ordinate and 0.01[Meter] for the elevation (z) ordinate.
* @param    maxError     OUT If provided receives the max error observed over the extent
* @param    meanError    OUT If provided receives the mean error observed over the extent
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED  ReprojectStatus     GetLinearTransformToECEF
(
    TransformP              outTransform,
    DRange3dCR              extent,
    double*                 maxError,
    double*                 meanError
) const;

/*---------------------------------------------------------------------------------**//**
* Private - We do not wish to publicise this method yet.
* CartesianFromCartesian - Converts from the cartesian representation of a GCS to
* the cartesian of the target. 3D conversion is applied.
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Danmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outCartesian   OUT Receives the output coordinate.
* @param    inCartesian    IN  The input coordinate.
* @param    targetGCS      IN  target coordinate system
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus  CartesianFromCartesian(DPoint3dR outCartesian, DPoint3dCR inCartesian, BaseGCSCR targetGCS) const;


/*---------------------------------------------------------------------------------**//**
* Private - We do not wish to publicise this method yet.
* CartesianFromECEF - Converts from ECEF to the cartesian of the target. 3D conversion is applied.
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Danmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outCartesian   OUT Receives the output coordinate.
* @param    inECEF         IN  The input coordinate.
* @param    targetGCS      IN  Target coordinate system
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static ReprojectStatus  CartesianFromECEF(DPoint3dR outCartesian, DPoint3dCR inECEF, BaseGCSCR targetGCS);

/*---------------------------------------------------------------------------------**//**
* Private - We do not wish to publicise this method yet.
* ECEFFromCartesian - Converts from source cartesian to the ECEF. 3D conversion is applied.
* @return REPROJECT_Success if the process was fully successful.
*         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
*           was out of the normal useful domain of either coordinate system.
*           This can be interpreted as a warning when the extent is known to extend past the
*           domain of the GCS. This will occur invariably in GCS such as Danmark 34 system
*           that use a non-square domain (polygon domain).
*         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
*           applied due to some configuration file missing. This will not normally affect the
*           result of the present method but should be reported to the user to
*           fix the configuration issue.
*         Any other error is a hard error depending on the value.
*
* @param    outECEF             OUT Receives the output coordinate.
* @param    inCartesian         IN  The input coordinate.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus  ECEFFromCartesian(DPoint3dR outECEF, DPoint3dCR inCartesian) const;

/*---------------------------------------------------------------------------------**//**
* High performance way of changing the BaseGCS to represent a different named coordinate system.
* @param    coordinateSystemKeyName IN      cs name.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFromCSName (Utf8CP coordinateSystemKeyName);


/*---------------------------------------------------------------------------------**//**
* Sets the BaseGCS from the provided Json given.
* This Json may contain all three components of the GCS definition (horizontal, vertical and
* additional transform)
* @param    jsonValue IN      The JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon that will contain the three components of a GCS definition (horizontal,
* vertical and additional transform) from the definition. Note that not all GCS can be fully converted to Json.
* For example all GCS that use the Grad unit will result in an error. Likewise
* Longitude/Latitude coordinate system that do not make use of the Greenwich meridian
* will not be successful. The same holds for obscure projection methods.
* If an error is returned the Json generated will still contain all the useful information it can.
* Name, Description, source, datum and such will still be valid even if the projection is not supported.
* In that case there will simply have an absence of the projection section. Having the keyname still enables to
* use the Json to identify the GCS.
* @param jsonValue IN OUT The JSonValue representing the horizontal, vertical and transform of a GCS.
*                     Previous JsonValue properties are retained unchanged.
* @param expandDatum If true indicates the datum definition must be expanded. By default it is
*                    false.
*                    The expansion is cascaded to the ellipsoid for which definition will be added.
*                    If expansion of datum but not ellipsoid is required then do not expand and add
*                    datum property to Json afterwards using Datum::ToJson().
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         ToJson(BeJsValue jsonValue, bool expandDatum = false) const;

/*---------------------------------------------------------------------------------**//**
* Sets the BaseGCS from the provided Horizontal Json given.
* An horizontal Json only specifies the horizontal portion of the GCS. The vertical portion
* should not be present.
* @param    jsonValue IN      The Horizontal JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromHorizontalJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the definition. Note that not all GCS can be fully converted to Json.
* For example all GCS that use the Grad unit will result in an error. Likewise
* Longitude/Latitude coordinate system that do not make use of the Greenwich meridian
* will not be successful. The same holds for obscure projection methods.
* If an error is returned the Json generated will still contain all the useful information it can.
* Name, Description, source, datum and such will still be valid even if the projection is not supported.
* In that case there will simply have an absence of the projection section. Having the keyname still enables to
* use the Json to identify the GCS.
* @param jsonValue IN OUT The JSonValue representing the GCS.
*                     Previous JsonValue properties are retained unchanged.
* @param expandDatum If true indicates the datum definition must be expanded. By default it is
*                    false.
*                    The expansion is cascaded to the ellipsoid for which definition will be added.
*                    If expansion of datum but not ellipsoid is required then do not expand and add
*                    datum property to Json afterwards using Datum::ToJson().
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt ToHorizontalJson(BeJsValue jsonValue, bool expandDatum = false) const;

/*---------------------------------------------------------------------------------**//**
* Sets the BaseGCS vertical datum from the provided Json.
* An vertical Json only specifies the vertical datum. The horizontal definition is untouched.
* @param    jsonValue IN      The Vertical JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromVerticalJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the vertical datum definition.
* @param jsonValue IN OUT The JSonValue representing the vertical datum of the GCS.
*                     Previous JsonValue properties are retained unchanged.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt ToVerticalJson(BeJsValue jsonValue) const;

/*---------------------------------------------------------------------------------**//**
* Sets the BaseGCS local transform from the provided Json.
* An local transform Json only specifies the optional local transform. The horizontal
* and vertical definition is untouched.
* @param    jsonValue IN      The Local Transform JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromLocalTransformerJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the optional local transfom definition.
* @param jsonValue IN OUT The JSonValue representing the local transform of the GCS.
*                     Previous JsonValue properties are retained unchanged.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         ToLocalTransformerJson(BeJsValue jsonValue) const;

/*---------------------------------------------------------------------------------**//**
* Saves the coordinate system to GeoTiffKeys, if possible.
* set of GeoTiff Keys.
* USE GetGeoTiffKeys() instead ... current name is misleading.
* @return   SUCCESS, a CS_MAP error code, or a GeoCoord error code.
* @param    geoTiffKeys     IN      an object implementing the IGeoTiffKeysList interface.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGeoTiffKeys
(
GeoCoordinates::IGeoTiffKeysList*     geoTiffKeys         // The GeoTiff keys list.
) const;

/*---------------------------------------------------------------------------------**//**
* Saves the coordinate system to GeoTiffKeys, if possible.
* set of GeoTiff Keys.
* @return   SUCCESS, a CS_MAP error code, or a GeoCoord error code.
* @param    geoTiffKeys     IN      an object implementing the IGeoTiffKeysList interface.
* @param    originalsIfPresent IN true indicates that original geokeys should be returned
*                                 if the baseGCS was originally created using geo keys and
*                                 was not modified.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetGeoTiffKeys
(
GeoCoordinates::IGeoTiffKeysList*     geoTiffKeys,         // The GeoTiff keys list.
bool                    originalsIfPresent   // true indicates the original geokeys should be returned

) const;

/*---------------------------------------------------------------------------------**//**
* Reveals whether the coordinate system can be saved to GeoTiffKeys.
* @return   true if the coordinate system can be saved to GeoTiffKeys, false if not.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              CanSaveToGeoTiffKeys () const;

/*---------------------------------------------------------------------------------**//**
* Reveals whether the datum can be saved to GeoTiffKeys.
* @return   true if the datum for the coordinate system can be saved to GeoTiffKeys, false if not.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              CanSaveDatumToGeoTiffKeys () const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSParameters*     GetCSParameters() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED size_t            GetCSParametersSize() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Coordinate System is valid or not.
* @return   True if the Coordinate System is valid, False otherwise. @see #GetError
* @remarks  If the coordinate system is constructed using the coordinate system keyname
*           constructor, and the specified name does not correspond to a coordinate system
*           in the coordinate system library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetErrorMessage (Utf8StringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with the error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static Utf8CP    GetErrorMessage (Utf8StringR errorMsg, StatusInt errorCode);

/*---------------------------------------------------------------------------------**//**
* Determines whether the coordinate system matches the strings passed in.
* @return   A score indicating the quality of the match between the coordinate system and
*           the strings passed in. A positive value indicates a match.
* @param    matchStrings    IN      An array of pointers to strings to test against.
* @param    numMixedCase    IN      The number of mixed case strings in matchStrings.
* @param    numUpperCase    IN      The number of upper case strings in matchStrings.
* @param    anyWord         IN      If true, a positive score results from matching any of the
*                                   words in matchStrings. If false, a positive score results
*                                   only if all words in matchStrings are matched.
* @remarks The match first attempts to match exactly matchStrings[0] through matchStrings[numMixedCase-1].
*          It then attempts to match matchStrings[numMixedCase] through matchStrings[numMixedCase+numUpperCase-1]
*          disregarding case. A higher score results if the case sensitive match succeeds.
*          For score computation, The match strings are regarded to be in decreasing order of importance.
*          The algorithm attempts to match the coordinate system Name, Description, ellipse name, datum
*          name, csmap group, location, source, and EPSG number.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               Matches (char const * const * matchStrings, int numMixedCase, int numUpperCase, bool anyWord) const;


/*---------------------------------------------------------------------------------**//**
* Tests the coordinate system definition for validity.
* @return   True if the Coordinate System parameters are valid, false if not.
* @param    errorList       OUT     A list of validation errors generated by CSMap.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              Validate (T_Utf8StringVector& errorList) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Coordinate System is a standard coordinate system or not.
* @return   True if the Coordinate System originated from the coordinate system library, False otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsStandard() const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System
* @return   The name of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the name of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (Utf8CP name);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Coordinate System.
* @return   The description of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (Utf8CP description);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the projection used by the Coordinate System.
* @return   The name of the projection used by the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetProjection() const;

enum ProjectionCodeValue : unsigned short     {
    pcvInvalid                                      = 0,
    pcvUnity                                        = 1,
    pcvTransverseMercator                           = 3,
    pcvAlbersEqualArea                              = 4,
    pcvHotineObliqueMercator                        = 5,
    pcvMercator                                     = 6,
    pcvLambertEquidistantAzimuthal                  = 7,
    pcvLambertTangential                            = 8,
    pcvAmericanPolyconic                            = 9,
    pcvModifiedPolyconic                            = 10,
    pcvLambertEqualAreaAzimuthal                    = 11,
    pcvEquidistantConic                             = 12,
    pcvMillerCylindrical                            = 13,
    pcvModifiedStereographic                        = 15,
    pcvNewZealandNationalGrid                       = 16,
    pcvSinusoidal                                   = 17,
    pcvOrthographic                                 = 18,
    pcvGnomonic                                     = 19,
    pcvEquidistantCylindrical                       = 20,
    pcvVanderGrinten                                = 21,
    pcvCassini                                      = 22,
    pcvRobinsonCylindrical                          = 23,
    pcvBonne                                        = 24,
    pcvEckertIV                                     = 25,
    pcvEckertVI                                     = 26,
    pcvMollweide                                    = 27,
    pcvGoodeHomolosine                              = 28,
    pcvEqualAreaAuthalicNormal                      = 29,
    pcvEqualAreaAuthalicTransverse                  = 30,
    pcvBipolarObliqueConformalConic                 = 31,
    pcvObliqueCylindricalSwiss                      = 32,
    pcvPolarStereographic                           = 33,
    pcvObliqueStereographic                         = 34,
    pcvSnyderObliqueStereographic                   = 35,
    pcvLambertConformalConicOneParallel             = 36,
    pcvLambertConformalConicTwoParallel             = 37,
    pcvLambertConformalConicBelgian                 = 38,
    pcvLambertConformalConicWisconsin               = 39,
    pcvTransverseMercatorWisconsin                  = 40,
    pcvLambertConformalConicMinnesota               = 41,
    pcvTransverseMercatorMinnesota                  = 42,
    pcvSouthOrientedTransverseMercator              = 43,
    pcvUniversalTransverseMercator                  = 44,
    pcvSnyderTransverseMercator                     = 45,
    pcvGaussKrugerTranverseMercator                 = 46,
    pcvCzechKrovak                                  = 47,
    pcvCzechKrovakObsolete                          = 48,
    pcvMercatorScaleReduction                       = 49,
    pcvObliqueConformalConic                        = 50,
    pcvCzechKrovak95                                = 51,
    pcvCzechKrovak95Obsolete                        = 52,
    pcvPolarStereographicStandardLatitude           = 53,
    pcvTransverseMercatorAffinePostProcess          = 54,
    pcvNonEarth                                     = 55,
    pcvObliqueCylindricalHungary                    = 56,
    pcvTransverseMercatorDenmarkSys34               = 57,
    pcvTransverseMercatorOstn97                     = 58,
    pcvAzimuthalEquidistantElevatedEllipsoid        = 59,
    pcvTransverseMercatorOstn02                     = 60,
    pcvTransverseMercatorDenmarkSys3499             = 61,
    pcvTransverseMercatorKruger                     = 62,
    pcvWinkelTripel                                 = 63,
    pcvNonEarthScaleRotation                        = 64,
    pcvLambertConformalConicAffinePostProcess       = 65,
    pcvTransverseMercatorDenmarkSys3401             = 66,
    pcvEquidistantCylindricalEllipsoid              = 67,
    pcvPlateCarree                                  = 68,
    pcvPopularVisualizationPseudoMercator           = 69,
    pcvLambertMichigan                              = 70,
    pcvCzechKrovakModified                          = 71,
    pcvHotineObliqueMercator1UV                     = (pcvHotineObliqueMercator * 256) + 1,
    pcvHotineObliqueMercator1XY                     = (pcvHotineObliqueMercator * 256) + 2,
    pcvHotineObliqueMercator2UV                     = (pcvHotineObliqueMercator * 256) + 3,
    pcvHotineObliqueMercator2XY                     = (pcvHotineObliqueMercator * 256) + 4,
    pcvRectifiedSkewOrthomorphic                    = (pcvHotineObliqueMercator * 256) + 5,
    pcvRectifiedSkewOrthomorphicCentered            = (pcvHotineObliqueMercator * 256) + 6,
    pcvRectifiedSkewOrthomorphicOrigin              = (pcvHotineObliqueMercator * 256) + 7,
    pcvTotalUniversalTransverseMercator             = 490,
    pcvTotalTransverseMercatorBF                    = 491,
    pcvObliqueMercatorMinnesota                     = 492,
    pcvTransverseMercatorOstn15                     = 493,

    };

/*---------------------------------------------------------------------------------**//**
* Gets CS_Map projection code of the Coordinate System.
* @return   The CS_Map projection code of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ProjectionCodeValue    GetProjectionCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets CS_Map projection code of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetProjectionCode (ProjectionCodeValue projectionCode);

/*---------------------------------------------------------------------------------**//**
* Gets the CSMap group to which the Coordinate System belongs.
* @return   The CSMap group to which the Coordinate System belongs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetGroup (Utf8StringR group) const;

/*---------------------------------------------------------------------------------**//**
* Sets the group of the Coordinate System. The group specified must be the identifier
* of a known group. In all cases the group name may not be more than 23 characters long.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGroup (Utf8CP source);

/*---------------------------------------------------------------------------------**//**
* Gets the location for which the Coordinate System applies.
* @return   The location for which the Coordinate System applies.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetLocation (Utf8StringR location) const;


/*---------------------------------------------------------------------------------**//**
* Gets the source of the Coordinate System.
* @return   The source of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSource (Utf8StringR source) const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (Utf8CP source);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System units.
* @return   Name of source of the Coordinate System units.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetUnits (Utf8StringR units) const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index for the Coordinate System units.
* @return   index into internal table of unit names.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetUnitCode() const;

/*---------------------------------------------------------------------------------**//**
* Finds the EPSG code for the unit used by this coordinate system.
* @return   The EPSG code, or 0 if the unit cannot be found in the CS Map table or the
            there is no EPSG code corresponding to the unit used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGUnitCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Units to those indicated by the unit code. The unit code must come
*  from either GetUnitCode or an index into the array returned by GetUnitNames.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetUnitCode (int code);

/*---------------------------------------------------------------------------------**//**
* Sets the Units to those indicated by the unit keyname. The unit key name must be an exact
* match to one of the csmap dictionary names (case insensitive). Typically the units in
* use are 'METER', 'IFOOT' for internatial foot and 'FOOT' for US Survey foot.
* The only possible unit for a latitude/longitude GCS is 'DEGREE'.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetUnitByKeyname(Utf8CP unitKeyname);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Datum.
* @return   Name of the Coordinate System Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Coordinate System Datum. This index will be from 0 to
* the number of datums in the system library for system defined datum. The code will be
* over 1000000 for user-defined datum stored in a user-library. If the GCS does not use a datum
* but is directly based on the ellipsoid then the code will be Datum::NO_DATUM_CODE (-1).
* For a fully self-contained datum as set using SetDatum() then the code will be Datum::CUSTOM_DATUM_CODE(-2) in
* which case the GCS cannot be stored in any library.
* @return   Index of the Coordinate System Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetDatumCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the internal index of the Datum for the Coordinate System Datum. Must be an
* index into the array returned by the GetDatumNames() static method for system datums.
* It can also be set to Datum::NO_DATUM_CODE (-1) to indicate that the GCS does not use a datum and is ellipsoid
* based after which the SetEllipsoidCode() method must be used to indicate the proper
* ellipsoid.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDatumCode (int datumCode);

/*---------------------------------------------------------------------------------**//**
* Returns the pointer to the internal datum. The datum must not be deallocated.
* The datum returned will be the internal datum set using SetDatum() or the datum
* obtained from the library. If the BaseGCS has no datum and is ellipsoid based then
* NULL is returned
* @return   Pointer to internal datum or NULL if no datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumCP            GetDatum() const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum. If the datum indicates it is stored in a user library then the GCS will
* automatically be changed to this user library.
* If the datum is self contained and unstored then the stored datum code will be set to -2.
* The datum is captured and must not be destroyed by the caller.
* @return   SUCCESS if successful
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDatum (DatumP datum);

/*---------------------------------------------------------------------------------**//**
* Indicates if the GCS uses a custom datum
* @return   true if the GCS uses a custom datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool         HasCustomDatum () const;

/*---------------------------------------------------------------------------------**//**

* Gets the description of the Coordinate System Datum.
* @return   Description of the Coordinate System Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDatumDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the Coordinate System Datum source citation.
* @return   Source citation of the Coordinate System Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDatumSource (Utf8StringR datumSource) const;

/*---------------------------------------------------------------------------------**//**
* Gets the method used to convert longitude/latitude from the Datum of this GCS to the WGS84 datum.
* @return   The convert method.
* @note: If this GCS does not have a Datum, ConvertType_NONE is returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WGS84ConvertCode  GetDatumConvertMethod() const;

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of the Datum of this GCS.
* @param    delta OUT  The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDatumDelta (DPoint3dR delta) const;

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of the Datum of this GCS
* @param    rotation OUT  The rotation angles.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDatumRotation (DPoint3dR rotation) const;

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scaling in parts per million of the Datum of this GCS, if known.
* @return   The datum transformation scale in ppm.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetDatumScale () const;

/*---------------------------------------------------------------------------------**//**
* Utilitarian method.
* The present method will return the grid file definition if the datum is currently based
* on a stored geodetic transform path containing a single transform that must use the Grid File
* method to WGS84 and contain a single grid file definition.
* If any of those conditions are not met ERROR is returned, otherwise the grid file definition
* is returned with a SUCCESS status.
* @param gridFileDef   OUT Reference to the grid file that will receive the current
*                      datum grid file definition.
* @return  SUCCESS is successful or ERROR if any prerequisite condition indicated above is
*          not met.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetDatumGridFile (GridFileDefinition& gridFileDef) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for the Datum of this GCS.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @return true if any of the output valid flags are true.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              DatumParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for the Datum of this GCS.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @param  gridValid     OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that grid file is used.
* @return true if any of the output valid flags are true.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              DatumExtendedParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid, bool& gridValid) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Vertical Datum. It will be either
* NAVD88, NGVD29, Ellipsoid or Geoid. The pointer returned is a pointer to a constant
* string defined in the library.
* NOTE: This method used to be published but hopefully it is not used anywhere
* since it was binding by the implementation to returning either constants
* or string internal to the BaseGCS which is bad practice.
* We will be phasing off the support of this methid that will be replaced eventually.
* @return   Name of the Coordinate System Vertical Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetVerticalDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the BaseGCS is based on a datum that is a variant of NAD27 or not.
* Only continental USA NAD27 variants are considered NAD27
* @return   True if the Datum is a variation of NAD27
* @return   True if the Datum is a variation of NAD27
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool            IsNAD27 () const;
/*---------------------------------------------------------------------------------**//**
* Returns whether the BaseGCS is based on a datum that is a variant of
* NAD83 (Excluding Canadian and non USA variations) or not.
* @return   True if the Datum is a variation of NAD83
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool            IsNAD83 () const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is coincident with WGS84 or not.
* @return   True if the Datum is coincident with WGS84
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              HasWGS84CoincidentDatum () const;

/*---------------------------------------------------------------------------------**//**
* Gets the Vertical Datum Code. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum.
* @return   Member of the VerticalDatum enum indicating Vertical Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED VertDatumCode     GetVerticalDatumCode () const;

/*---------------------------------------------------------------------------------**//**
* Gets the Vertical Datum Code. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum. This method contrary to GetVerticalDatumCode()
* never returns vdcFromDatum but returns the explicit datum code for ellipsoid, NGVD29 or NAVD88.
* @return   Member of the VerticalDatum enum indicating Vertical Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED VertDatumCode     GetNetVerticalDatumCode () const;

/*---------------------------------------------------------------------------------**//**
* Sets the Vertical Datum Code. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetVerticalDatumCode (VertDatumCode);

/*---------------------------------------------------------------------------------**//**
* Sets the Vertical Datum by vertical datum key
* See SetVerticalDatumCode() for details.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt SetVerticalDatumByKey(Utf8CP verticalDatumKey);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Ellipsoid.
* @return   Name of the Coordinate System Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Coordinate System Ellipsoid.
* @return   Index of the Coordinate System Ellipsoid. Ellipsoid:: NO_ELLIPSOID_CODE (-1) is returned
*           if there are no ellipsoid set. If a custom ellipsoid is set the
*           Ellipsoid::CUSTOM_ELLIPSOID_CODE
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEllipsoidCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the internal index of the Coordinate System Ellipsoid. Must be an
*  index into the array returned by the GetEllipsoidNames static method.
* @remarks  The Ellipsoid can only be set if the Datum Code is Ellipsoid::NO_ELLIPSOID_CODE(-1).
*           Otherwise, the Datum determines the Ellipsoid.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEllipsoidCode (int ellipsoidCode);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Coordinate System Ellipsoid.
* @return   The description of the Coordinate System Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the Coordinate System Ellipsoid source citation.
* @return   Source citation of the Coordinate System Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidSource (Utf8StringR ellipsoidSource) const;

#if defined (DGNGEOORD_ONLY)
/*---------------------------------------------------------------------------------**//**
* Gets a name suitable for displaying in a user interface.
* @return   The display name of the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDisplayName (Utf8StringR displayName) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Gets the origin latitude of the Coordinate System.
* @return   The origin latitude of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetOriginLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the origin latitude of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetOriginLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the origin longitude of the Coordinate System.
* @return   The origin longitude of the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetOriginLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the origin longitude of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetOriginLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the false easting of the Coordinate System.
* @return   The value added to all x cartesian coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetFalseEasting() const;

/*---------------------------------------------------------------------------------**//**
* Sets the false easting of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFalseEasting (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the false northing of the Coordinate System.
* @return   The value added to all y cartesian coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetFalseNorthing() const;

/*---------------------------------------------------------------------------------**//**
* Sets the false northing of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFalseNorthing (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the scale reduction of the Coordinate System.
* @return   The scale reduction for the Coordinate System.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScaleReduction() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Scale Reduction of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetScaleReduction (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Coordinate System.
* @return   The polar radius of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Gets the minimum longitude for the Geographic Coordinate System.
* @return   The minimum longitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the minimum longitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMinimumLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the maximum longitude for the Geographic Coordinate System.
* @return   The maximum longitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the maximum longitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMaximumLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the minimum latitude for the Geographic Coordinate System.
* @return   The minimum latitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the minimum latitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMinimumLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the maximum latitude for the Geographic Coordinate System.
* @return   The maximum latitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the maximum latitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMaximumLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the minimum useful longitude for the Geographic Coordinate System.
* @return   The minimum useful longitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MinimumLongitude is not specified in the coordinate system library,
*           a minimum longitude is calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumUsefulLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the maximum useful longitude for the Geographic Coordinate System.
* @return   The maximum useful longitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MaximumLongitude is not specified in the coordinate system library,
*           a maximum longitude is calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumUsefulLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the minimum useful latitude for the Geographic Coordinate System.
* @return   The minimum useful latitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MinimumLatitude is not specified in the coordinate system library,
*           a minimum latitude is calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumUsefulLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the maximum useful latitude for the Geographic coordinate system.
* @return   The maximum useful latitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MaximumLatitude is not specified in the coordinate system library,
*           a maximum latitude is calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumUsefulLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the first standard parallel for Projections that have one.
* @return   The first standard parallel for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetStandardParallel1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the first standard parallel for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetStandardParallel1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the first standard parallel for Projections that have one.
* @return   The first standard parallel for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetStandardParallel2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the second standard parallel for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetStandardParallel2 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the azimuth angle for Projections that have one.
* @return   The azimuth angle for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAzimuth() const;

/*---------------------------------------------------------------------------------**//**
* Sets the azimuth angle for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAzimuth (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central meridian (in degrees) for Projections that have one.
* @return   The central meridian longitude for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralMeridian() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central meridian (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralMeridian (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the eastern meridian (in degrees) for Projections that have one.
* @return   The eastern meridian longitude for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEasternMeridian() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central meridian (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEasternMeridian (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central point longitude (in degrees) for Projections that have one.
* @return   The central point longitude for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralPointLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central point longitude (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralPointLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central point latitude (in degrees) for Projections that have one.
* @return   The central point latitude for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralPointLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central point latitude (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralPointLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the longitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   The longitude of the first point of the central geodesic for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint1Longitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the longitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint1Longitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the latitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   The latitude of the first point of the central geodesic for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint1Latitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the latitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint1Latitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the longitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   The longitude of the second point of the central geodesic for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint2Longitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the longitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint2Longitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the latitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   The latitude of the second point of the central geodesic for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint2Latitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the latitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint2Latitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the geoid separation, in CS units, for Projections that use that parameter.
* @return   The geoid separation, in CS Units for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetGeoidSeparation() const;

/*---------------------------------------------------------------------------------**//**
* Sets the geoid separation, in CS units, for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGeoidSeparation (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the elevation above the geoid, in CS units, for Projections that use that parameter.
* @return   The elevation above the geoid, in CS Units for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetElevationAboveGeoid() const;

/*---------------------------------------------------------------------------------**//**
* Sets the elevation above the geoid, in CS units, for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetElevationAboveGeoid (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the ellipsoid scale factor, used by Lambert Conformal Conic Michigan variation.
* @return   The ellipsoid scale factor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidScaleFactor() const;

/*---------------------------------------------------------------------------------**//**
* Sets the ellisoid scale factor for a Lambert Conformal Conic Michigan variation.
* The value given must be reasonnably close to 1.0.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEllipsoidScaleFactor (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the UTM Zone number (1-60) for the Univeral Transverse Mercator projection.
* @return   The UTM Zone number for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetUTMZone() const;

/*---------------------------------------------------------------------------------**//**
* Sets the UTM Zone number (1-60) for the Universal Transverse Mercator projections.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetUTMZone (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the Hemisphere (1 for north or -1 for south) for the Univeral Transverse Mercator projection.
* @return   The UTM Zone number for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetHemisphere() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Hemisphere (1 for north or -1 for south) for the Univeral Transverse Mercator projection.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetHemisphere (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the Quadrant for Projections that use that parameter.
* @return   The Quadrant for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetQuadrant() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Quadrant for Projections that use that parameter.
* @return   The Quadrant for Projections that use that parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetQuadrant (short value);

/*---------------------------------------------------------------------------------**//**
* Gets the Danish System 34 Region for Danish Sys 34 Projections.
* @return   The Region.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetDanishSys34Region() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Danish System 34 Region for Danish Sys 34 Projections.
* @return   The Region.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDanishSys34Region (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the A0 Affine post-processing parameter for Projections that have one.
* @return   The A0 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA0() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A0 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA0 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the A1 Affine post-processing parameter for Projections that have one.
* @return   The A1 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A1 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the A2 Affine post-processing parameter for Projections that have one.
* @return   The A2 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A2 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA2 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B0 Affine post-processing parameter for Projections that have one.
* @return   The B0 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB0() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B0 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineB0 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B1 Affine post-processing parameter for Projections that have one.
* @return   The B1 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B1 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineB1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B2 Affine post-processing parameter for Projections that have one.
* @return   The B2 Affine post-processing parameter for Projections that have one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B2 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineB2 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the affine parameters for the affine post-processing portion of Transverse Mercator
* with post affine or Lambert Conformal Conic with post affine projections.
*
* @param    A0 OUT  The X translation of the affine transformation
* @param    A1 OUT  The A1 parameter of the rotation/scale/shearing portion of the affine.
* @param    A2 OUT  The A2 parameter of the rotation/scale/shearing portion of the affine.
* @param    B0 OUT  The Y translation of the affine transformation
* @param    B1 OUT  The B1 parameter of the rotation/scale/shearing portion of the affine.
* @param    B2 OUT  The B2 parameter of the rotation/scale/shearing portion of the affine.

* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetAffineParameters (double* A0, double* A1, double* A2, double* B0, double* B1, double* B2) const;

/*---------------------------------------------------------------------------------**//**
* Sets the affine parameters for the affine post-processing portion of Transverse Mercator
* with post affine or Lambert Conformal Conic with post affine projections. The
* transformation must be valid. In order to be valid, the determinant of the matrix formed
* by parameters A1 A2 and B1 B2 must be different than 0.0. To express no rotation, scale
* nor shearing, set A1 and B2 equal to 1.0 and A2 and B1 equal to 0.0.
*
* @param    A0 IN  The X translation of the affine transformation
* @param    A1 IN  The A1 parameter of the rotation/scale/shearing portion of the affine.
* @param    A2 IN  The A2 parameter of the rotation/scale/shearing portion of the affine.
* @param    B0 IN  The Y translation of the affine transformation
* @param    B1 IN  The B1 parameter of the rotation/scale/shearing portion of the affine.
* @param    B2 IN  The B2 parameter of the rotation/scale/shearing portion of the affine.

* @return   SUCCESS or error code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineParameters (double A0, double A1, double A2, double B0, double B1, double B2);

/*---------------------------------------------------------------------------------**//**
* Signals that the caller has finished setting the coordinate system parameters, and that
* the coordinate system internal definition should be initialized with the current parameter set.
* @return   SUCCESS or a CS_MAP error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         DefinitionComplete ();

/*---------------------------------------------------------------------------------**//**
* Gets whether coordinate system is set to be editable.
* @return true if editable
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetCanEdit () const;

/*---------------------------------------------------------------------------------**//**
* Sets whether coordinate system can be editted.
* @return true if editable
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetCanEdit (bool value);


/*---------------------------------------------------------------------------------**//**
* Gets the available Linear Units.
* @return   vector of strings of unit names.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/

BASEGEOCOORD_EXPORTED static T_Utf8StringVector* GetLinearUnitNames ();
/*---------------------------------------------------------------------------------**//**
* Gets all available Units, linear and degree-based.
* @return   vector of strings of unit names.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_Utf8StringVector* GetUnitNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the available Datum Names.
* @return   vector of strings of datum names.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_Utf8StringVector* GetDatumNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the available Ellipsoid Names.
* @return   vector of strings of datum names.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_Utf8StringVector* GetEllipsoidNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the grid scale along a meridian of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the grid scale is to be computed.
* @return   The grid scale along the meridian at the position specified.
* @remarks This is sometimes called the 'h' scale in geo coordinate system literature.
* @remarks The scale along a meridian is equal to the scale along a parallel for conformal projections.
* @see #GetScaleAlongParallel, #GetGridScale
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScaleAlongMeridian
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the grid scale along a parallel of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the grid scale is to be computed.
* @return   The grid scale along the parallel at the position specified.
* @remarks This is sometimes called the 'k' scale in geo coordinate system literature.
* @remarks The scale along a parallel is equal to the scale along a meridian for conformal projections.
* @see #GetScaleAlongMeridian, #GetGridScale
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScaleAlongParallel
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the grid scale of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the grid scale is to be computed.
* @return   The grid scale at the position specified.
* @remarks Non-conformal projections have two different grid scale factors: the scale along a meridian
*          and the scale along a parallel. In the case of azimuthal projections, the two scale factors
*          are along a radial line from the origin and normal to such radial lines, respectively. In
*          these cases, GetGridScale will return the more interesting of the two. For example, in
*          the American Polyconic, the grid scale factor along all parallels is always 1.0;
*          therefore GetGridScale returns the grid scale factor along a meridian for this projection.
* @see #GetScaleAlongMeridian, #GetScaleAlongParallel
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetGridScale
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the convergence angle, in degrees, of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the convergence angle is to be computed.
* @see #GetScaleAlongMeridian, #GetScaleAlongParallel
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetConvergenceAngle
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Computes distance (in the units of this GCS) and starting azimuthal angle (in degrees)
* from one geographic point to another. This distance is the geodetic distance following
* the surface of the ellipsoid of the BaseGCS. It does not take into account the elevation
* of points provided or the fact the GCS can indicate another vertical datum setting.
* @param    distance    OUT     The distance, in units of this GCS, from startPoint to endPoint.
* @param    azimuth     OUT     The initial azimuth, in degrees clockwise from true north,
*                               needed to get from startPoint to endPoint.
* @param    startPoint  IN      The starting point.
* @param    endPoint    IN      The end point.
* @remarks  If either distance or azimuth is not needed, pass NULL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetDistance
(
double      *distance,
double      *azimuth,
GeoPointCR  startPoint,
GeoPointCR  endPoint
) const;

/*---------------------------------------------------------------------------------**//**
* Computes distance (in meters) and starting azimuthal angle (in degrees)
* from one geographic point to another. This distance is the geodetic distance following
* the surface of the ellipsoid of the BaseGCS. It does not take into account the elevation
* of points provided or the fact the GCS can indicate another vertical datum setting.
* @param    distance    OUT     The distance, in meters from startPoint to endPoint.
* @param    azimuth     OUT     The initial azimuth, in degrees clockwise from true north,
*                               needed to get from startPoint to endPoint.
* @param    startPoint  IN      The starting point.
* @param    endPoint    IN      The end point.
* @remarks  If either distance or azimuth is not needed, pass NULL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetDistanceInMeters
(
double      *distance,
double      *azimuth,
GeoPointCR  startPoint,
GeoPointCR  endPoint
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the latitude and longitude of a "center" point in the coordinate system. This
*  might be the origin of longitude/origin of latitude, or perhaps the central meridian
*  and latitude of latitude, depending on the projection in use.
* @param    centerPoint     OUT     The center point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetCenterPoint
(
GeoPointR       centerPoint
) const;

/*---------------------------------------------------------------------------------**//**
* Compares this coordinate system with the argument and returns true if they have equivalent
*  projection, parameters, datum, ellipsoid, and modifiers.
* @param    compareTo IN     The BaseGCS to compare to.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent
(
BaseGCSCR        compareTo
) const;

/*---------------------------------------------------------------------------------**//**
* Compares this coordinate system with the argument and returns true if they are equal.
* This method differs from the IsEquivalent() method as it requires that all fields including
* name, description, source, projection method, unit etc be identical.
* @param    compareTo IN     The BaseGCS to compare to.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEqual
(
BaseGCSCR        compareTo
) const;

/*---------------------------------------------------------------------------------**//**
* If a BaseGCS has been created from a WKT that does not include a vertical datum definition
* the elevation will have changed since we went from not reprojecting elevation by default
* to always reprojecting elevation by default. CorrectVerticalDatumToPreserveLegacyElevation
* attempts to choose a vertical datum that will preserve the same elevation that would have
* been generated if reproject elevation had been switched off i.e. we now have reproject
* elevation switched on and the extent passed will be at the same elevation when reprojected as
* it was when reproject elevation was switched off.
* datum set
* @param[in]    extent      the extent in this CS
* @param[in]    targetGCS   the target CS we are reprojecting to
* @return       ERROR if we were not able to generate a linear transform between this CS
*                   and the target CS, SUCCESS if a new vertical datum was selected or if
*                   the existing vertical datum was not changed - note that if the existing
*                   vertical datum was not changed then:
*                       - it was not possible to choose a new datum that would preserve the
*                           expected legacy elevation
*                       - the existing vertical datum already preserves the expected legacy
*                           elevation
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt CorrectVerticalDatumToPreserveLegacyElevation(const DRange3d& extent, BaseGCSCR targetGCS);

/*---------------------------------------------------------------------------------**//**
* Compares this coordinate system with the argument and returns true if they have equivalent
*  projection, parameters, datum, and ellipsoid.
* @param[in]    compareTo   The BaseGCS to compare to.
* @param[out]   datumDifferent          true if the datum is different.
* @param[out]   csDifferent             true if the coordinate system projection is different.
* @param[out]   verticalDatumDifferent  true if the vertical datum is different.
* @param[out]   localTransformDifferent true if the local transform is different.
* @param[in]    stopFirstDifference If true, the comparison stops when the first difference is encountered.
*                                   Only one of datumDifferent, csDifferent, and modifiersDifferent will be set.
* @return       True if the coordinate systems are identical, false otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              Compare (BaseGCSCR compareTo, bool& datumDifferent, bool& csDifferent, bool& verticalDatumDifferent, bool& localTransformDifferent, bool stopFirstDifference) const;

/*---------------------------------------------------------------------------------**//**
* Returns the mathematical domain of application for GCS. The domain will usually be
* much larger than the user domain yet caller must remember that distortion may result
* outside the user domain.
* This method is intended to provide a strong limit to displaying geospatial objects
* expressed in a BaseGCS. Outside this limit the representation is meaningless and may
* lead to conversion errors and wild points.
* It is assumed that any geospatial object can be fully represented within its own
* interpretation BaseGCS. The usefulness of the present method becomes evident when we must
* reproject a geospatial object expressed in a BaseGCS into another BaseGCS that has
* a mathematical domain smaller than the one of the geospatial object. The proper way to
* deal with domains is to obtain the two mathematical domains of both BaseGCS, intersect
* them together and convert the result in cartesian coordinate of either BaseGCS.
* The result shape is the mathematical limit of the geospatial object when reprojected
* to the other coordinate reference system.
* To illustrate the process let us assume we have a DGN using the New Zealand Grid
* BaseGCS for a map in New  Zealand. As a base map we want to use Bing. Raster Manager
* will compute the mathematical limit of reprojecting Bing into a New Zealand GCS
* resulting in clipping Bing to the New Zealand area only.
*
* @param[out] shape A list of GeoPoint (latitude/longitude) that contains the definition
*             of the shape of the mathematical domain. The shape contains the closing
*             point. This shape is usually rectangular but for some specific projections
*             may contain more points. The shape is always returned even if an error occurs
*             In this case the shape will either contain the user domain definition
*             or a shape containing the whole Earth (excluding the poles)
* @return BSIERROR in case of error or if computations are not implemented for this
*         projection method and BSISUCCESS otherwise. Note that a valid shape is always
*         returned even if an error occurs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt GetMathematicalDomain(bvector<GeoPoint>&    shape) const;

/*---------------------------------------------------------------------------------**//**
* Returns the extent of the mathematical domain as returned by GetMathematicalDomain().
* Although GetMathematicalDomain() usually returns 4 points defining an axis aligned
* bounding box, some projections have more complex domain shapes composed of
* dozens of points. The present method computes the bounding box of this domain.
* The values returned are in degrees.
* It is technically possible for the mininum longitude to be greater than the
* maximum longitude. This occurs if the extent of the domain overlaps the
* boundary between -180 degrees and +180 longitude.
* @param[out] minLongitude The minimum longitude of the domain in degrees.
* @param[out] maxLongitude The maximum longitude of the domain in degrees.
* @param[out] minLatitude  The minimum latitude of the domain in degrees.
* @param[out] maxLatitude  The maximum latitude of the domain in degrees.
*
* @return BSIERROR in case of error or if computations are not implemented for this
*         projection method and BSISUCCESS otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt GetMathematicalDomainExtent
(
double& minLongitude,
double& maxLongitude,
double& minLatitude,
double& maxLatitude
) const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the BaseGCS.
* This format is only useful for dictionary management purposes.
* @param    GCSAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
Utf8StringR GCSAsASC
) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Compares the Datum of this coordinate system with the argument and returns true if they have equivalent
*  datum (including ellipsoid).
* @param    compareTo IN     The BaseGCS to compare to.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              HasEquivalentDatum
(
BaseGCSCR        compareTo
) const;

/*---------------------------------------------------------------------------------**//**
* Finds the EPSG code for this coordinate system.
* @remarks  Sometimes a search through the coordinate system library to find a matching EPSG coordinate system is required.
*           This can be time consuming. To prevent the search, pass true for the "noSearch" argument.
* @return   The EPSG code, or 0 if this coordinate system does not match any EPSG CS known to the system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode
(
bool            noSearch = false
) const;

/*---------------------------------------------------------------------------------**//**
* Finds the EPSG code for the datum used by this coordinate system.
* @remarks  Sometimes a search through the datum library to find a matching EPSG datum is required.
*           This can be time consuming. To prevent the search, pass true for the "noSearch" argument.
* @return   The EPSG code, or 0 if this coordinate system does not match any EPSG CS known to the system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGDatumCode
(
bool            noSearch = false
) const;

/*---------------------------------------------------------------------------------**//**
* Finds the EPSG code for the ellipsoid used by this coordinate system.
* @remarks  Sometimes a search through the ellipsoid library to find a matching EPSG ellipsoid is required.
*           Since there are a limited number of ellipsoids, this is not very time consuming. The "noSearch"
*           argument is included for symmetry with GetEPSGCode and GetEPSGDatumCode.
* @return   The EPSG code, or 0 if this coordinate system does not match any EPSG CS known to the system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGEllipsoidCode
(
bool            noSearch = false
) const;

/*---------------------------------------------------------------------------------**//**
* Returns the EPSG code as stored in the coordinate system definition.
* @return   The EPSG code as stored in the definition. A value of 0 indicates it is undefined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetStoredEPSGCode
(
) const;

/*---------------------------------------------------------------------------------**//**
* Sets the EPSG code in the coordinate system definition.
* @param    value IN     The new EPSG code. Can be 0 to 32768
* @return   SUCCESS is successful
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetStoredEPSGCode
(
short value
);

/*---------------------------------------------------------------------------------**//**
* Indicates if the BaseGCS is deprecated. A deprecated GCS will have the group name set
* to LEGACY which is an alternate way to check.
* @return   true if the GCS is deprecated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsDeprecated
(
) const;

/*---------------------------------------------------------------------------------**//**
* Indicates if the BaseGCS is based on a projection. If not then the GCS is
* Longitude/Latitude based.
* @return   true if the GCS uses a projection (not a null projection or lat/long based).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsProjected
(
) const;


// These Methods are related to the ability to have a local coordinate system that is
// related to the Cartesian coordinate system by the LocalTransformerP

/*---------------------------------------------------------------------------------**//**
* Sets the LocalTransformer object for the GCS.
* @param    transformer     IN  The object that transformer from local cartesian to GCS cartesian coordinates and vice versa.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetLocalTransformer (LocalTransformerP transformer);

/*---------------------------------------------------------------------------------**//**
* Gets the LocalTransformer object for the GCS.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED LocalTransformerP GetLocalTransformer () const;

/*---------------------------------------------------------------------------------**//**
* Converts from Cartesian Coordinates of the GCS to the Cartesian Coordinate system
*   used in the projection.
* @param    outInternalCartesian    OUT The coordinates used in the projection calculation.
* @param    inCartesian             IN  The cartesian system of the GCS.
* @note: This method is rarely needed in client code. The LatLontFromCartesian method uses it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              InternalCartesianFromCartesian
(
DPoint3dR       outInternalCartesian,
DPoint3dCR      inCartesian
) const;

/*---------------------------------------------------------------------------------**//**
* Converts from Cartesian Coordinats of the GCS to the Local Coordinate system.
* @param    outCartesian            OUT The cartesian system of the GCS.
* @param    inInternalCartesian     IN  The coordinates of the projection.
* @note: This method is rarely needed in client code. The CartesianFromLatLong method uses it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              CartesianFromInternalCartesian
(
DPoint3dR       outCartesian,
DPoint3dCR      inInternalCartesian
) const;

/*---------------------------------------------------------------------------------**//**
* Converts from Cartesian Coordinates of the GCS to the Cartesian Coordinate system
*   used in the projection.
* @param    outInternalCartesian    OUT The coordinates used in the projection calculation.
* @param    inCartesian             IN  The cartesian system of the GCS.
* @note: This method is rarely needed in client code. The LatLontFromCartesian method uses it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              InternalCartesianFromCartesian2D
(
DPoint2dR       outInternalCartesian,
DPoint2dCR      inCartesian
) const;

/*---------------------------------------------------------------------------------**//**
* Converts from Cartesian Coordinats of the GCS to the Local Coordinate system.
* @param    outCartesian            OUT The cartesian system of the GCS.
* @param    inInternalCartesian     IN  The coordinates of the projection.
* @note: This method is rarely needed in client code. The CartesianFromLatLong method uses it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              CartesianFromInternalCartesian2D
(
DPoint2dR       outCartesian,
DPoint2dCR      inInternalCartesian
) const;


/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian coordinates of the input Longitude/Latitude/Elevation point.
* @param    outCartesian    OUT     The calculated cartesian coordinates.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   CartesianFromLatLong
(
DPoint3dR       outCartesian,       // <= cartesian coordinates in this GCS
GeoPointCR      inLatLong           // => latitude longitude in this GCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian x and y of the input Longitude/Latitude point. The input elevation is ignored.
* @param    outCartesian    OUT     The calculated cartesian coordinates.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   CartesianFromLatLong2D
(
DPoint2dR       outCartesian,       // <= cartesian coordinates in this GCS
GeoPoint2dCR    inLatLong           // => latitude longitude in this GCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude, latitude, and elevation from cartesian x,y, and z.
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of this GCS.
* @param    inCartesian     IN      The input cartesian coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromCartesian
(
GeoPointR       outLatLong,         // <= latitude longitude in this GCS
DPoint3dCR      inCartesian         // => cartesian coordinates in this GCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude from cartesian x and y. Elevation is unchanged.
* @param    outLatLong      OUT     The calculated longitude and latitude in the datum of this GCS.
* @param    inCartesian     IN      The input cartesian coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromCartesian2D
(
GeoPoint2dR     outLatLong,         // <= latitude longitude in this GCS
DPoint2dCR      inCartesian         // => cartesian coordinates in this GCS
) const;

/*---------------------------------------------------------------------------------**//**
* Returns the scale factor needed to convert to the units of the coordinate system from
*      meters by multiplication.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            UnitsFromMeters
(
) const;

/*---------------------------------------------------------------------------------**//**
* Determines whether the input GeoPoints are within the useful range of the coordinate system.
* @param    points          IN  The points to test.
* @param    numPoints       IN  Number of points to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED RangeTestResult   CheckGeoPointRange
(
GeoPointCR      points,
int             numPoints
) const;

/*---------------------------------------------------------------------------------**//**
* Determines whether the input GeoPoints are within the useful range of the coordinate system.
* @param    points          IN  The points to test.
* @param    numPoints       IN  Number of points to test.
* @remarks  The input points must be in the cartesian units of the coordinate system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED RangeTestResult   CheckCartesianRange
(
DPoint3dCR       points,
int             numPoints
) const;

BASEGEOCOORD_EXPORTED static DgnProjectionTypes     DgnProjectionTypeFromCSDefName (CharCP projectionKeyName);

BASEGEOCOORD_EXPORTED static DgnProjectionTypes     DgnProjectionTypeFromCSMapProjectionCode (BaseGCS::ProjectionCodeValue projectionCode);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumConverterP   SetDatumConverter
(
BaseGCSCR destGCS
);

/*---------------------------------------------------------------------------------**//**
* Sets whether reprojections from this coordinate system sadjust elevations.
* @return  The previous ReprojectElevation setting.
* @remarks The ReprojectElevation setting affects coordinate reprojections performed by the
* #LatLongFromLatLong method. If ReprojectElevation is false, the elevation value is
* unchanged.
* @remarks The default value is false.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              SetReprojectElevation (bool value);

/*---------------------------------------------------------------------------------**//**
* Gets whether reprojections from this coordinate system adjust elevations.
* @return  The ReprojectElevation setting.
* @remarks The ReprojectElevation setting affects coordinate reprojections performed by the
* #LatLongFromLatLong method. If ReprojectElevation is false, the elevation value is unchanged.
* @remarks The default value is false.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetReprojectElevation () const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude in the target GCS, applying the appropriate datum shift.
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of targetGCS.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @param    targetGCS         IN      The Coordinate System corresponding to outLatLong.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromLatLong
(
GeoPointR       outLatLong,
GeoPointCR      inLatLong,
BaseGCSCR       targetGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude in the target GCS, applying the appropriate datum shift.
* @param    outLatLong      OUT     The calculated longitude,latitude in the datum of targetGCS.
* @param    inLatLong       IN      The longitude,latitude in the datum of this GCS.
* @param    targetGCS       IN      The Coordinate System corresponding to outLatLong.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromLatLong2D
(
GeoPoint2dR     outLatLong,
GeoPoint2dCR    inLatLong,
BaseGCSCR       targetGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Converts from Degrees to Radians
* @param    inDegrees       IN      Angular value in degrees.
* @return   Angular value in radians
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static double     RadiansFromDegrees
(
double          inDegrees
);

/*---------------------------------------------------------------------------------**//**
* Converts from Radians to Degrees
* @param    inRadians       IN      Angular value in radians.
* @return   Angular value in degrees
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static double     DegreesFromRadians
(
double          inRadians
);

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude from ECEF coordinate according to GCS current datum.
* This does not compute necessarily the ECEF relative to WGS84.
* This method will not work for all GCS. In order to be able to compute the ECEF coordinates
* relative to the GCS Datum and ellipsoid the elevation must be expressed using a vertical
* datum that can be converted to the Local Ellipsoid vertical datum. For example for all GCS
* based on the NAD27 datum, the XYZ coordinate cannot be computed due to the inability to
* convert to the Clarke66 ellipsoid local elevation. Datum conversions to WGS84 rely
* on grid files which are neutral vertical wise and the Clarke66 Ellipsoid does not have a
* shape similar to the WGS84 ellipsoid.
* For the other GCS that are not vertically coincident with WGS84 the elevation must already
* be based on the local ellipsoid (vdcLocalEllipsoid). The present method will not perform
* vertical change resulting from vertical datum upon the WGS84 ellipsoid (vdcEllipsoid or vdcFromDatum)
* that may also require application of geoid separation.
* Note however that GCS that make use of a Geoid vertical datum and that have datums considered
* vertically coincident to WGS84 then geoid separation value will be applied prior to XYZ computations.
* Example: All based on grid files to WGS84 but that do not have an ellipsoid of same shape
* will result in an error.
* EPSG:3857 and EPSG:900913 will not work because the ellipsoid used in the calculation is
* spherical even though the transformation to WGS84 is vertically neutral.
* BritishNatGrid with vertical datum set to vdcLocalEllipsoid WILL WORK. Because coordinates
* already expressed relative to the OSGB datum based on the Airy30 ellipsoid.
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation.
* @param    inXYZ           IN      The XYZ (ECEF) coordinates of this GCS.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromXYZ
(
GeoPointR       outLatLong,
DPoint3dCR      inXYZ
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the XYZ (ECEF) coordinates from the longitude, latitude and elevation.
* The ECEF coordinates are comuted according to GCS current datum.
* This does not compute necessarily the ECEF relative to WGS84.
* This method will not work for all GCS. In order to be able to compute the ECEF coordinates
* relative to the GCS Datum and ellipsoid the elevation must be expressed using a vertical
* datum that can be converted to the Local Ellipsoid vertical datum. For example for all GCS
* based on the NAD27 datum, the XYZ coordinate cannot be computed due to the inability to
* convert to the Clarke66 ellipsoid local elevation. Datum conversions to WGS84 rely
* on grid files which are neutral vertical wise and the Clarke66 Ellipsoid does not have a
* shape similar to the WGS84 ellipsoid.
* For the other GCS that are not vertically coincident with WGS84 the elevation must already
* be based on the local ellipsoid (vdcLocalEllipsoid). The present method will not perform
* vertical change resulting from vertical datum upon the WGS84 ellipsoid (vdcEllipsoid or vdcFromDatum)
* that may also require application of geoid separation.
* Note however that GCS that make use of a Geoid vertical datum and that have datums considered
* vertically coincident to WGS84 then geoid separation value will be applied prior to XYZ computations.
* Example: All based on grid files to WGS84 but that do not have an ellipsoid of same shape
* will result in an error.
* EPSG:3857 and EPSG:900913 will not work because the ellipsoid used in the calculation is
* spherical even though the transformation to WGS84 is vertically neutral.
* BritishNatGrid with vertical datum set to vdcLocalEllipsoid WILL WORK. Because coordinates
* already expressed relative to the OSGB datum based on the Airy30 ellipsoid.
* @param    outXYZ      OUT     The calculated XYZ (ECEF) coordinates.
* @param    inLatLong   IN      The latitude, longitude and elevation to convert
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   XYZFromLatLong
(
DPoint3dR       outXYZ,
GeoPointCR      inLatLong
) const;

};

enum LocalTransformType
    {
    TRANSFORM_None                  = 0,
    TRANSFORM_Helmert               = 1,
    TRANSFORM_SecondOrderConformal  = 2,
    };

} // ends GeoCoordinates namespace

END_BENTLEY_NAMESPACE

/*=================================================================================**//**
*
* This class has only static methods that map directly to CSMap functions. With the exception
*  of the Initialize method, most have the CS-map name.
*
+===============+===============+===============+===============+===============+======*/
typedef struct cs_Csdef_             CSDefinition;
typedef struct cs_Datum_             CSDatum;
typedef struct cs_Dtdef_             CSDatumDef;
typedef struct cs_Eldef_             CSEllipsoidDef;
typedef struct cs_Dtcprm_            CSDatumConvert;
typedef struct cs_GxXform_           CSGeodeticTransform;
typedef struct cs_Prjprm_            CSParamInfo;
typedef struct cs_Unittab_           CSUnitInfo;
typedef struct cs_Mgrs_              CSMilitaryGrid;
typedef struct cs_GeodeticPath_      CSGeodeticPath;


// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy from it. Check it.
enum CSMapErrors
    {
    CSMAPERR_OutOfMathematicalDomain = 1,
    CSMAPERR_OutOfUsefulRange        = 2,
    };

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

class CSMap
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static int              CS_csGrpEnum (int index,char *grp_name,int name_sz,char *grp_dscr,int dscr_sz);

BASEGEOCOORD_EXPORTED static int              CS_csEnum (int index, char *key_name,int name_sz);

BASEGEOCOORD_EXPORTED static CSParameters*    CScsloc2 (CSDefinition*, CSDatumDef*, CSEllipsoidDef*);

BASEGEOCOORD_EXPORTED static CSParameters*    CScsloc1 (CSDefinition*);

BASEGEOCOORD_EXPORTED static CSParameters*    CS_csloc (const char *key_name);

BASEGEOCOORD_EXPORTED static char*            CS_stncp (char*, CharCP   , int);

BASEGEOCOORD_EXPORTED static void             CS_errmsg (char*, int);

BASEGEOCOORD_EXPORTED static int              CS_cs3ll (const CSParameters*, GeoPointP ll, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static int              CS_ll3cs (const CSParameters*, DPoint3dP xy, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_dtcvt3D (CSDatumConvert*, GeoPointCP ll_in, GeoPointP ll_out);

BASEGEOCOORD_EXPORTED static int              CS_cs2ll (const CSParameters*, GeoPointP ll, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static int              CS_ll2cs (const CSParameters*, DPoint3dP xy, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_dtcvt (CSDatumConvert*, GeoPointCP ll_in, GeoPointP ll_out);

BASEGEOCOORD_EXPORTED static CSDatumConvert*  CS_dtcsu (const CSParameters*, const CSParameters*);
#ifdef GEOCOORD_ENHANCEMENT
BASEGEOCOORD_EXPORTED static CSDatumConvert*  CS_dtcsuDefOnly (const CSParameters*, const CSParameters*);
BASEGEOCOORD_EXPORTED static int              CS_gpdefFrom(CSGeodeticPath **array, int numArray, const char *srcDatum);
BASEGEOCOORD_EXPORTED static int              CS_gxdefFrom(CSGeodeticTransformDef **array, int numArray, const char *srcDatum);
BASEGEOCOORD_EXPORTED static void             CS_dtclsDefOnly (CSDatumConvert*);

#endif

BASEGEOCOORD_EXPORTED static int              CS_gxdefAll (CSGeodeticTransformDef **pDefArray[]);
BASEGEOCOORD_EXPORTED static int              CS_gpdefAll (CSGeodeticPath **pDefArray[]);

BASEGEOCOORD_EXPORTED static void             CS_dtcls (CSDatumConvert*);

BASEGEOCOORD_EXPORTED static double           CS_cscnv (const CSParameters*, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static double           CS_cssch (const CSParameters*, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static double           CS_cssck (const CSParameters*, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static double           CS_csscl (const CSParameters*, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static void             CS_fillIn (CSDefinition*);

BASEGEOCOORD_EXPORTED static int              CSerpt (char *mesg,int size,int err_num);

BASEGEOCOORD_EXPORTED static int              CS_wktToCsEx (CSDefinition *csDef, CSDatumDef *dtDef, CSEllipsoidDef *elDef, GeoCoordinates::BaseGCS::WktFlavor flavor, CharCP wellKnownText);

BASEGEOCOORD_EXPORTED static bool             CS_prjprm (CSParamInfo *info, int projectionCode, int paramNum);

BASEGEOCOORD_EXPORTED static CSEllipsoidDef*  CS_eldef (const char *key_name);

BASEGEOCOORD_EXPORTED static CSDatumDef*      CS_dtdef (const char *key_name);

BASEGEOCOORD_EXPORTED static CSDefinition*    CS_csdef (const char *key_name);

BASEGEOCOORD_EXPORTED static CSGeodeticTransformDef*  CS_gxdef (const char *key_name);

BASEGEOCOORD_EXPORTED static void             CS_free (void *mem);

BASEGEOCOORD_EXPORTED static int              CS_dtEnum (int index, char *key_name, int name_sz);

BASEGEOCOORD_EXPORTED static int              CS_elEnum (int index, char *key_name, int name_sz);

BASEGEOCOORD_EXPORTED static CSDatum*         CS_dtloc (char *key_name);

BASEGEOCOORD_EXPORTED static CSUnitInfo const* GetCSUnitInfo (int unitIndex);

BASEGEOCOORD_EXPORTED static double           CS_llazdd (double eRad, double eSq, GeoPointCP startPoint, GeoPointCP endPoint, double *distance);

BASEGEOCOORD_EXPORTED static int              CS_llchk (const CSParameters*, int numPoints, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_xychk (const CSParameters*, int numPoints, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static CSDatumConvert*  CSdtcsu (const CSDatum* src, const CSDatum* dest);
#ifdef GEOCOORD_ENHANCEMENT
BASEGEOCOORD_EXPORTED static CSDatumConvert*  CSdtcsuDefOnly (const CSDatum* src, const CSDatum* dest);
#endif

BASEGEOCOORD_EXPORTED static CSDatum*         CSdtloc1 (const CSDatumDef* datumDef);

BASEGEOCOORD_EXPORTED static CSMilitaryGrid*  CSnewMgrs (double e_rad, double e_sq, short bessel);

BASEGEOCOORD_EXPORTED static int              CScalcMgrsFromLl (CSMilitaryGrid* mg, char* result, int size, GeoPoint2dP ll, int precision);

BASEGEOCOORD_EXPORTED static int              CScalcLlFromMgrs (CSMilitaryGrid* mg, GeoPoint2dP ll, const char* mgrsString);

BASEGEOCOORD_EXPORTED static void             CSdeleteMgrs (CSMilitaryGrid* mg);

BASEGEOCOORD_EXPORTED static void             CS_llhToXyz (DPoint3dP xyz, GeoPointCP llh, double e_rad, double e_sq);

BASEGEOCOORD_EXPORTED static int              CS_xyzToLlh (GeoPointP llh , DPoint3dCP xyz, double e_rad, double e_sq);

BASEGEOCOORD_EXPORTED static double           CSmrcatPhiFromK (double e_sq,double scl_red);
};

typedef int (*DatumConvert3dFunc) (CSDatumConvert*, GeoPointCP in, GeoPointP out);

class   Datum;
typedef class Datum const&        DatumCR;

struct  VerticalDatumConverter;

/*=================================================================================**//**
* Grid File Direction enum class.
* Defines the direction in which the grid file transforms the source to the target
* datum. The source and target datum are not specified since they are part of the
* englobing Geodetic Transformation the grid file definition belongs or will
* belong to.
* These are the same values as CSMAP
* but we do not want to include csmap include files
+===============+===============+===============+===============+===============+======*/
enum class GridFileDirection
{
    DIRECTION_NONE = 0,
    DIRECTION_DIRECT = 0x46,  // Should have been one but in fact CSMAP returns 0x46
    DIRECTION_INVERSE = 0x49, // Should have been two but in fact CSMAP returns 0x49
};

/*=================================================================================**//**
* Grid File format enum class.
* Defines the format in which the grid file is. These are the same values as CSMAP
* but we do not want to include csmap include files
+===============+===============+===============+===============+===============+======*/
enum class GridFileFormat
{
    FORMAT_NONE   = 0x00,   // cs_DTCFRMT_NONE
    FORMAT_NTv1   = 0x01,   // cs_DTCFRMT_CNTv1    0x01
    FORMAT_NTv2   = 0x02,   // cs_DTCFRMT_CNTv2    0x02
    FORMAT_NADCON = 0x03,   // cs_DTCFRMT_NADCN    0x03
    FORMAT_FRENCH = 0x04,   // cs_DTCFRMT_FRNCH    0x04
    FORMAT_JAPAN  = 0x05,   // cs_DTCFRMT_JAPAN    0x05
    FORMAT_ATS77  = 0x06,   // cs_DTCFRMT_ATS77    0x06
    FORMAT_OSTN97 = 0x07,   // cs_DTCFRMT_OST97    0x07
    FORMAT_OSTN02 = 0x08,   // cs_DTCFRMT_OST02    0x08
    FORMAT_GEOCN  = 0x09,   // cs_DTCFRMT_GEOCN    0x09
    FORMAT_OSTN15 = 0x0A,   // cs_DTCFRMT_OST15    0x0A
};

/*=================================================================================**//**
* Grid File Definition.
* This light class represent a single grid file definition containing the name of the file
* the format type and the direction of conversion. The source and target datum
* are not specified.
+===============+===============+===============+===============+===============+======*/
class GridFileDefinition
{
    private:
        // filename is specified relative to dictionary (ex: ./UK/SomGridFile.gsb)
        Utf8String        m_fileName;
        GridFileFormat    m_format;
        GridFileDirection m_direction;

    public:

/*---------------------------------------------------------------------------------**//**
* CTOR -  Sets the file name, direction and format.
* The filename can have at most 237 characters otherwise it will be truncated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GridFileDefinition(const char* fileName, GridFileFormat format, GridFileDirection direction);

/*---------------------------------------------------------------------------------**//**
* Copy - CTOR
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GridFileDefinition(const GridFileDefinition& obj);

/*---------------------------------------------------------------------------------**//**
* DTOR
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ~GridFileDefinition();

/*---------------------------------------------------------------------------------**//**
* Copy operator
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GridFileDefinition& operator=(const GridFileDefinition& obj);

/*---------------------------------------------------------------------------------**//**
* Returns the internal constant reference to the file name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED        const Utf8String& GetFileName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the file name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED        void SetFileName(const Utf8CP newFileName);

/*---------------------------------------------------------------------------------**//**
* Returns the file format
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED        GridFileFormat GetFormat() const;

/*---------------------------------------------------------------------------------**//**
* Sets the file format
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED         void SetFormat(GridFileFormat newFormat);

/*---------------------------------------------------------------------------------**//**
* Returns the file direction
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED        GridFileDirection GetDirection() const;

/*---------------------------------------------------------------------------------**//**
* Sets the file direction
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED        void SetDirection(GridFileDirection newDirection);
};

typedef class GridFileDefinition const*       GridFileDefinitionCP;

/*=================================================================================**//**
* GeodeticTransform Data Available enum class.
* Defines a status for a geodetic transform to indicate if the geodetic transform
* has all data available to perform the transformation. Typically a geodetic transform will
* have data available unless it is based on a grid file transformation. In that case the
* files may not be available.
* There are three states possible:
* DataAvailable       - Indicates all data was available at the time of creation or the
*                       presence of the data was checked and verified afterward.
* DataUnavailable     - Indicates that the data was not available at the time of the
*                       creation fo the transform. To use this transform the required
*                       must be obtained (typically the grid sift files).
* AvailabilityUnknown - Indicates the availability of the data is unknown. Prior to
*                       using the transform the availability of the data should be verified.
*                       This check cannot be performed by the present class as
*                       sometimes verification requires the fully qualified datums in the process.
*                       Verification can be perfomed by creating a datum converter using
*                       this transform and optionally the source and possibly the target datum.
+===============+===============+===============+===============+===============+======*/
enum class GeodeticTransformDataAvailability
{
    DataAvailable,
    DataUnavailable,
    AvailabilityUnknown
};

class Ellipsoid;
typedef class Ellipsoid*                EllipsoidP;
typedef class Ellipsoid const*          EllipsoidCP;
typedef class Ellipsoid const&          EllipsoidCR;

typedef class GeodeticTransform const*        GeodeticTransformCP;
typedef class GeodeticTransform *             GeodeticTransformP;
typedef class GeodeticTransform&              GeodeticTransformR;
typedef class GeodeticTransform const&        GeodeticTransformCR;

/*=================================================================================**//**
* Geodetic Transformation class.
* This class represent a single step datum transformation based on one transformation
* method and a set of associated parameters.
* A DatumConverter instance defines the whole list of Datum Transformations to convert
* from one datum to the other. Conceptually a DatumConverter contains a list of
* Datum Transformations with application direction for each step.
*
* Note that the definition of a Geodetic Transformation is similar to the definition of
* datum. A datum used to be the container of all and any transformations.
* With the introduction of the Geodetic Transformations and Geodetic Paths by CSMAP
* the transformation stored in the datum is now considered a fallback if no
* other datum transformation can be located in the dictionary.
+===============+===============+===============+===============+===============+======*/
class GeodeticTransform
{
private:
// CSMAP has two structures to represent a geodetic transform. One is the definition
// as stored in the GeodeticTransform.dty (the definition) and another where
// all parameters have been verified, the existence of datum certified and is fully ready for
// performing transformations.
// Note that the ellipsoid parameters (polar radius, equatorial radius) are necessary for
// computing the transformation but csmap makes use instead of the source target keynames of the datums to extract these values.
// which results in a cicular dependency (datum uses transform and transform uses datum)
CSGeodeticTransformDef*     m_geodeticTransformDef; // Geodetic transform definition
GeodeticTransform*          m_fallback;
mutable EllipsoidCP         m_sourceEllipsoid; // The source ellipsoid if specified or null if source datum name specified.
mutable EllipsoidCP         m_targetEllipsoid; // The target ellipsoid if specified or null if target datum name specified.
int32_t                     m_csError;
mutable GeodeticTransformDataAvailability m_dataAvailability; // Indicates if data for the transform is available.

// Cache name and description parameters
mutable Utf8StringP            m_nameString;                   // these are here as "adapters" needed because CS_Map uses all char rather than unicode.
mutable Utf8StringP            m_descriptionString;
mutable Utf8StringP            m_sourceString;
mutable Utf8StringP            m_sourceDatumString;
mutable Utf8StringP            m_destDatumString;


/*---------------------------------------------------------------------------------**//**
* Initializes a new uninitialized instance of the GeodeticTransform class.
* The transform must be initialized with a method such as FromJson() afterwards.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform ();

/*---------------------------------------------------------------------------------**//**
* Initializes a new geodetic transform of type grid file adding the given file
* definition
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform (const GridFileDefinition& gridFileDefinition);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the GeodeticTransform class. Looks only in the system library.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform (Utf8CP keyName);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the GeodeticTransform class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform(CSGeodeticTransform const& geodeticTransform);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the GeodeticTransform class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform (
CSGeodeticTransformDef const& geodeticTransform,
GeodeticTransformDataAvailability dataAvailability = GeodeticTransformDataAvailability::AvailabilityUnknown,
EllipsoidP sourceEllipsoid = nullptr,
EllipsoidP targetEllipsoid = nullptr,
CSGeodeticTransformDef const * fallback = nullptr);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the GeodeticTransform class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransform (
CSGeodeticTransformDef const& geodeticTransform,
CSGeodeticTransformDef const& fallback,
GeodeticTransformDataAvailability dataAvailability = GeodeticTransformDataAvailability::AvailabilityUnknown);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ~GeodeticTransform();

public:
/*---------------------------------------------------------------------------------**//**
* Initializes a new uninitialized instance of the Geodetic Transform class
* @return Uninitialized Geodetic Transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GeodeticTransformP    CreateGeodeticTransform ();

/*---------------------------------------------------------------------------------**//**
* Initializes a new geodetic transform of type grid file adding the given file
* definition
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GeodeticTransformP    CreateGeodeticTransform (const GridFileDefinition& gridFileDefinition);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Geodetic Transform class
* @param keyname IN The keyname of the geodetic transform.
* @return Initialized Geodetic Transform.
* @remarks If keyName is in the GeodeticTransform library and valid, the return
           if either null or a valid geodetic transform.
*          In case of error, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GeodeticTransformP    CreateGeodeticTransform (Utf8CP keyName);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Geodetic Transform class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GeodeticTransformP    CreateGeodeticTransform(CSGeodeticTransform const& geodeticTransform);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Geodetic Transform class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GeodeticTransformP    CreateGeodeticTransform
(CSGeodeticTransformDef const& geodeticTransformDef,
    GeodeticTransformDataAvailability dataAvailability = GeodeticTransformDataAvailability::AvailabilityUnknown,
    EllipsoidP sourceEllipsoid = nullptr,
    EllipsoidP targetEllipsoid = nullptr,
    CSGeodeticTransformDef const * fallback = nullptr);

/*---------------------------------------------------------------------------------**//**
* Indicates if the Geodetic Transform is deprecated. A deprecated Geodetic Transform
* will have the group name set to LEGACY which is an alternate way to check.
* @return   true if the GeodeticTransform is deprecated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsDeprecated() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the geodetic transform is valid or not.
* @return   True if the geodetic transform is valid, False otherwise. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the geodetic transform represents a null transformation.
* This implies that the transform is defined on the NONE convertion type or that
* the parameters of a 3,4,6 or 7 parameter transformation represent a null transformation.
* @return   True if the geodetic transform is valid, False otherwise. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsNullTransform() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetErrorMessage (Utf8StringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the geodetic transformation.
* @return   Name of the geodetic transformation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the geodetic transformation.
* @param name IN  Name of the geodetic transformation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName(Utf8CP name);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Geodetic Transform.
* @return   The description of the Geodetic Transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Geodetic Transform.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Geodetic Transform information
* @return   The source of the Geodetic Transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSource () const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Geodetic Transform
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the source Datum.
* @return   Name of the source Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSourceDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the name of the source Datum.
* @param datumName IN  Name of the source Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSourceDatumName(Utf8CP datumName);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the target Datum.
* @return   Name of the target Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetTargetDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the name of the target Datum.
* @param datumName IN  Name of the target Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetTargetDatumName(Utf8CP datumName);

/*---------------------------------------------------------------------------------**//**
* Returns the source ellipsoid. If one has not been set using SetSourceEllipsoid()
* and the source datum name is set and known then the ellipsoid will be obtained from
* this datum.
* @return   Const pointer to internal source ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP           GetSourceEllipsoid() const;

/*---------------------------------------------------------------------------------**//**
* Sets the source ellipsoid of the transform
* @param sourceEllipsoid IN  A pointer to an Ellipsoid that is given to the transform.
*                    The caller must not free the given ellipsoid. It will be freed
*                    automatically at the destruction of the transform
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSourceEllipsoid(EllipsoidP sourceEllipsoid);

/*---------------------------------------------------------------------------------**//**
* Returns the target ellipsoid. If one has not been set using SetTargetEllipsoid()
* and the target datum name is set and known then the ellipsoid will be obtained from
* this datum.
* @return   Const pointer to internal target ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP           GetTargetEllipsoid() const;

/*---------------------------------------------------------------------------------**//**
* Sets the target ellipsoid of the transform
* @param targetEllipsoid IN  A pointer to an Ellipsoid that is given to the transform.
*                    The caller must not free the given ellipsoid. It will be freed
*                    automatically at the destruction of the transform
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetTargetEllipsoid(EllipsoidP);

/*---------------------------------------------------------------------------------**//**
* Gets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   The convert method.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GenConvertCode    GetConvertMethodCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetConvertMethodCode (GenConvertCode value);

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta OUT  The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetDelta (DPoint3dR delta) const;

/*---------------------------------------------------------------------------------**//**
* Sets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta IN    The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the delta settings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDelta (DPoint3dCR delta);

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation OUT The rotation angles.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetRotation (DPoint3dR rotation) const;

/*---------------------------------------------------------------------------------**//**
* Sets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation IN The rotation angles.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the rotation settings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetRotation (DPoint3dCR rotation) ;

/*---------------------------------------------------------------------------------**//**
* Gets the translation for the Molodenski-Badekas transformation. Do not confuse translation
* with the delta.
* @param    translation OUT  translation value for the Molodenki-Badekas transformation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetTranslation (DPoint3dR translation) const;

/*---------------------------------------------------------------------------------**//**
* Sets the translation for the Molodenski-Badekas transformation. Do not confuse translation
* with the delta.
* @param    translation IN     translation value for the Molodenki-Badekas transformation.
* @return  GEOCOORDERR_ParameterNotUsed if the datum GenConvertMethod is not
*          GenConverttype_BDKAS.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetTranslation (DPoint3dCR translation);

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scaling in parts per million if known.
* @return   The datum transformation scale in ppm. If the transform method does not use
*           a scale then 0 is returned
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScale () const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum transformation scaling in parts per million if known.
* @param scalePPM   IN The scale in parts per million.
* @return  GEOCOORDERR_ParameterNotUsed if the method does not use the scale setting.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetScale (double scalePPM);

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation accuracy in meters.
* @return   The datum transformation accuracy in meters.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAccuracy () const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum transformation accuracy in meters
* @param scalePPM   IN The accuracy in meters.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAccuracy (double accuracyInMeter);

/*---------------------------------------------------------------------------------**//**
* Returns the number of grid file definitions.
* @return   The number of grid file definitions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED size_t               GetGridFileDefinitionCount () const;

/*---------------------------------------------------------------------------------**//**
* Gets the geodetic transformation indexed grid file definition.
* @param index IN the index of the grid file definition to obtain. This index must be
*                 between 0 and the number of grid file definition as returned by
                  GetGridFileDefinitionCount() minus one.
* @return   A copy of the grid file definition.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GridFileDefinition      GetGridFileDefinition (size_t index) const;

/*---------------------------------------------------------------------------------**//**
* Adds the geodetic transformation at the end of the list of grid file definitions.
* There can be at most 50 grid file definitions in a transform
* @param definition IN The new definition to add.
* @return   SUCCESS in case of success or some error code otherwise. The most probable error
*                   may result if the filename of the grid file is too long. The underlying
*                   CSMAP engine limits filenames for grid files to 237 characters maximum.
*                   An error is also reported if there are already 50 grid file definitions in the
*                   transform.
*                   If the filename length exceeds then GEOCOORDERR_BadArg will be returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt      AddGridFileDefinition (const GridFileDefinition& definition);

/*---------------------------------------------------------------------------------**//**
* Gets the geodetic transformation fallback definition.
* A fallback is only applicable for grid file transforms that can specify a 7 parameter
* transform as fallback in the event there is no grid file node at indicated location.
* @param delta OUT Ref to the translation components of the 7 parameters transform.
* @param rotation OUT Ref to the rotation components of the 7 parameters transform.
* @param scalePPM OUT Ref to double for the scale in PPM format.
* @return   true if there is a fallback and false otherwise. It always answers false
*           for transforms other than grid file based.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool GetFallback(DPoint3dR delta, DPoint3dR rotation, double& scalePPM) const;

/*---------------------------------------------------------------------------------**//**
* Sets the geodetic transformation fallback definition.
* A fallback is only applicable for grid file transforms that can specify a 7 parameter
* transform as fallback in the event there is no grid file node at indicated location.
* @param delta IN Ref to the translation components of the 7 parameters transform.
* @param rotation IN Ref to the rotation components of the 7 parameters transform.
* @param scalePPM IN The scale in PPM format.
* @return   SUCCESS if successful or false if the transform method is not grid file based.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt SetFallback(DPoint3dCR delta, DPoint3dCR rotation, double scalePPM);

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Geodetic Transformation, if known.
* @return   The EPSG code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode () const;

/*---------------------------------------------------------------------------------**//**
* Sets the EPSG code for this Geodetic Transformation, if known.
* @param    value IN   The new EPSG code. Can be 0 to 32767 where 0 indicates there are
*                      no EPSG code for this definition
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEPSGCode (short value);

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for this
* Geodetic transformation.
* @param  deltaValid        OUT Returns true if the transform is valid and its
*                               GenConvertCode indicates that the delta parameters are used.
* @param  rotationValid     OUT Returns true if the transform is valid and its
*                               GenConvertCode indicates that the rotation parameters are used.
* @param  scaleValid        OUT Returns true if the transform is valid and its
*                               GenConvertCode indicates that the scale parameter is used.
* @param  translationValid  OUT Returns true if the transform is valid and its
*                               GenConvertCode indicates that the scale parameter is used.
* @param  grid Valid        OUT Returns true if the transform is valid and its
*                               GenConvertCode indicates that the grid files are used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              ParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid, bool& translationValid, bool& gridValid) const;

/*---------------------------------------------------------------------------------**//**
* Returns the data availability flag of the transform. For geocentric transformations
* the returned value will always be data available since no external data is required.
* For grid file base transformation there must be at least one file specified to indicate
* availability.
* @param    recheck IN indicates that checking again for file availability is necessary
*           possibly after files have been intalled.
* @return   a data availability value (available/not available/unknown)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformDataAvailability           GetDataAvailability(bool recheck = false) const;

/*---------------------------------------------------------------------------------**//**
* Sets the data availability flag of the transform. This flag can only be set
* for no geocentric transformation methods otherwise these will be maintained as available.
* @param name IN  New data availability value (available/not available/unknown)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt        SetDataAvailability(GeodeticTransformDataAvailability availability);

/*---------------------------------------------------------------------------------**//**
* Sets the Geodetic Transform from the provided JSon fragment given.
* @param    jsonValue IN      The JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the definition.
* @param jsonValue IN OUT The JSonValue representing the transform.
*                         Previous JsonValue properties are retained unchanged.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED  StatusInt        ToJson(BeJsValue jsonValue) const;

/*---------------------------------------------------------------------------------**//**
* Free this geodetic transformation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

/*---------------------------------------------------------------------------------**//**
* Gets the group name to which the Geodetic Transform belongs.
* @return   The group to which the Geodetic Transform belongs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetGroup (Utf8StringR group) const;

/*---------------------------------------------------------------------------------**//**
* Sets the group name to which the Geodetic Transform belongs.
* @param   The group to which the Geodetic Transform belongs. Note that the actual
*          groups are typically limited in values yet the values LEGACY and NONE are usual.
*          Setting the group to LEGACY indicates the definition is deprecated. Setting NONE
*          indicates the definition is not part of any group.
*          The group name may not be more than 23 characters long.
* @return SUCCESS if the group can be set and an error code otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGroup (Utf8StringCR group);

/*---------------------------------------------------------------------------------**//**
* Checks if  GeodeticTransform is equivalent vertically, meaning that there would
* be no elevation change during transformation.
* @return    True if the geodetic transform is vertically neutral.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsVerticallyNeutral() const;

/*---------------------------------------------------------------------------------**//**
* Compares this GeodeticTransform with the argument and returns true if they are equivalent
* @param    compareTo IN     The Geodetic Transform to compare to.
* @param    looselyCompare IN OPTIONAL If false then the method checks if the
*              geodetic transform is the same including the method used and the accuracy
*              expected; the test is more strict.
*              If true then the method will verify if the two geodetic transform would
*              yield the same result.
*              For example a 7 parameter definition having no rotation and no scale would be
*              considered equivalent to a 3 parameter transform given the delta values
*              were the same.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent
(
GeodeticTransformCR        compareTo,
bool looselyCompare = true
) const;

/*---------------------------------------------------------------------------------**//**
* Reverses the geodetic transfom if possible.
* @return SUCCESS if the transform could be reversed and ERROR otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         Reverse();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformP  Clone() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSGeodeticTransformDef const*          GetCSGeodeticTransformDef() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSGeodeticTransformDef const*          GetFallbackCSGeodeticTransformDef() const;
};

typedef class GeodeticTransformPath *             GeodeticTransformPathP;
typedef class GeodeticTransformPath const*        GeodeticTransformPathCP;
typedef class GeodeticTransformPath&              GeodeticTransformPathR;
typedef class GeodeticTransformPath const&        GeodeticTransformPathCR;

/*=================================================================================**//**
*
* GeodeticTransformPath class.
* This class defines a list of chained geodetic transforms to describe the path of transformation
* from a source datum to another. The class is similar to the DatumConverter class except
* a datum converter requires a list of fully configured and ready to convert components,
* and provides additionally vertical datum conversion.
* A geodetic transform path only provides the definition of the path including the definition
* of each individual transforms. A geodetic transform path has no notion of vertical datum
* and does not provide coordinate conversion methods. In addition the path does not require
* that all resources including grid shift files be present and ready for processing.
*
+===============+===============+===============+===============+===============+======*/
class GeodeticTransformPath
{
private:
bvector<GeodeticTransformP> m_listOfGeodeticTransforms;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeodeticTransformPath ();

/*---------------------------------------------------------------------------------**//**
* Creates a transform path from provided datum convert. The datum convert is NOT
* captured and must be deallocated by the caller.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeodeticTransformPath
(
CSDatumConvert*             datumConvert
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~GeodeticTransformPath();

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    GeodeticTransformPathP        Create ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    GeodeticTransformPathP     Create(
BaseGCSCR        from,
BaseGCSCR        to
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    GeodeticTransformPathP        Create
(
DatumCR         from,
DatumCR         to
);

/*---------------------------------------------------------------------------------**//**
* Create a geodetic path from given dtum up to WGS84 datum. If this path cannot be
* determined then nullptr is returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    GeodeticTransformPathP        CreateToWgs84
(
DatumCR         from
);

/*---------------------------------------------------------------------------------**//**
* Returns a new geodetic path as the concatenation of the two paths given.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    GeodeticTransformPathP        CreateMerged(GeodeticTransformPathCR firstPath, GeodeticTransformPathCR secondPath);

/*---------------------------------------------------------------------------------**//**
* Returns a new geodetic transform as the simplification of path to a geocentric
* transformation. NULL is returned if the path cannot be simplified.
* The returned transformed must be destroyed once it is not needed anymore.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED     GeodeticTransformP        CreateSimplifiedGeocentricTransform() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if the transform path uses grid file based transformation and at least one
* file is missing or of the wrong format.
* @param listOfFiles OUT Receives the list of missing files.
* @param includeFilesToDeprecatedDatums IN OPTIONAL Indicates if the grid files on the path
*         refers to all deprecated datums then the files must be returned or not.
* @return   True if the path has missing grid files, False otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              HasMissingGridFiles(bvector<Utf8String>& listOfFiles, bool includeFilesToDeprecatedDatums = false) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the source Datum.
* @return   Name of the source Datum. This is the source datum of the first geodetic
*           transform
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSourceDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the target Datum.
* @return   Name of the target Datum. This is the target datum of the last
*           transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetTargetDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Returns the number of geodetic transform definitions.
* @return   The number of geodetic transform definitions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED size_t      GetGeodeticTransformCount () const;

/*---------------------------------------------------------------------------------**//**
* Gets the indexed geodetic transformation definition.
* The returned pointer points directly to the internal geodetic transform and
* must not be modified or destroyed
* @param index IN the index of the geodetic transform definition to obtain. This index must be
*                 between 0 and the number of grid file definition as returned by
*                 GetGeodeticTransformationCount() minus one.
* @return   Pointer to internal geodetic transform or null if index is invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformCP      GetGeodeticTransform (size_t index) const;
BASEGEOCOORD_EXPORTED GeodeticTransformP       GetGeodeticTransform (size_t index);

/*---------------------------------------------------------------------------------**//**
* Adds a new Geodetic Transform at the end of path
* The given transform is not duplicated but retained and must not be destroyed.
* @param newtransform IN A pointer to the new transform given to the path.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt      AddGeodeticTransform(GeodeticTransformP newTransform);

/*---------------------------------------------------------------------------------**//**
* Returns whether the geodetic transform represents a null transformation.
* To be a Null transform either all component geodetic transform must be null transforms
* or two non-null transforms must be the inverse of the other (resulting in a null transform
* once combined).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsNullTransform() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if given geodetic transform path is vertically neutral meaning it does not
* imply any vertical change between ellipsoids..
* @return true if the transformation path is vertically neutral.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsVerticallyNeutral() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if given geodetic transform path is equivalent or false otherwise.
* @param compareTo IN the geodetic transform path to compare to
* @param    looselyCompare IN OPTIONAL If false then the method checks if the
*              geodetic transform is the same including the method used and the accuracy
*              expected; the test is more strict.
*              If true then the method will verify if the two geodetic transform would
*              yield the same result.
*              For example a 7 parameter definition having no rotation and no scale would be
*              considered equivalent to a 3 parameter transform given the delta values
*              were the same.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent(GeodeticTransformPathCR compareTo, bool looselyCompare = true) const;

/*---------------------------------------------------------------------------------**//**
* Returns true if given geodetic transform path contains a sub-path to the datum
* whose name is provided. Note that transforms may not have source or target names
* defined in which case the final target name if unset is considered to be WGS84
* @param datumName IN The name of the datum to check for the presence of a path to.
* @return true if a path can be found.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              ContainsTransformPathTo(Utf8CP datumName) const;

/*---------------------------------------------------------------------------------**//**
* Use the method ContainsTranscformPathTo() to determine if it is possible to create
* such path.
* This method create a new Geodetic Transform Path containing the transforms from
* the start right up to the named datum.
* @param datumName IN The name of the datum to create a path to.
* @return A newly allocated path that must be destroyed when not needed anymore or NULL
*         if no transform path could be created to target. Note that the path can
*         be non-NULL but empty indicating the target datum has none transform to source
*         datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformPath*   CreateTransformPathTo(Utf8CP datumName) const;

/*---------------------------------------------------------------------------------**//**
* Reverses the GeodeticTransformPath. The result geodetic path as swapped source and
* target datums. The order and direction of transforms is also reversed.
* The operation may fail if one of the transform is not reversible such as
* Multiple regressions and Molodenski Badekas. If reversal does not work the path
* remains integrally as it was prior to attempt at reversal.
* @return SUCCESS if the path could be reversed. Since some transform methods cannot
*      be reversed it is possible for the path not to be reversible.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt       Reverse();

/*---------------------------------------------------------------------------------**//**
* Sets the Geodetic Transform Path from the provided JSon fragment given.
* @param    jsonValue IN      The JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         FromJson (BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the definition.
* @param jsonValue IN OUT The JSonValue representing the transform.
*                         Previous JsonValue properties are retained unchanged.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED  StatusInt        ToJson(BeJsValue jsonValue) const;

/*---------------------------------------------------------------------------------**//**
* Clone a GeodeticTransformPath. The result geodetic path must be destroyed with Destroy()
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformPathP       Clone() const;

/*---------------------------------------------------------------------------------**//**
* Free this GeodeticTransformPath.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};

typedef class DatumConverter const*        DatumConverterCP;
typedef class DatumConverter&              DatumConverterR;
typedef class DatumConverter const&        DatumConverterCR;

/*=================================================================================**//**
*
* Datum Converter class.
*
+===============+===============+===============+===============+===============+======*/
class DatumConverter
{
friend class MilitaryGridConverter;

private:
CSDatumConvert*             m_datumConvert;
VerticalDatumConverter*     m_verticalDatumConverter;
DatumConvert3dFunc          m_3dDatumConvertFunc;
bool                        m_reprojectElevation;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverter
(
CSDatumConvert*             datumConvert,
VerticalDatumConverter*     verticalDatumConverter
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~DatumConverter();

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    DatumConverterP        Create
(
BaseGCSCR        from,
BaseGCSCR        to
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    DatumConverterP        Create
(
DatumCR         from,
DatumCR         to
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static   DatumConverterP         Create
(
DatumCR       from,
DatumCR       to,
VertDatumCode fromVerticalDatum,
VertDatumCode toVerticalDatum
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   ConvertLatLong3D
(
GeoPointR       outLatLong,
GeoPointCR      inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   ConvertLatLong2D
(
GeoPoint2dR     outLatLong,
GeoPoint2dCR    inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              SetReprojectElevation   // <= returns old value.
(
bool            reprojectElevation
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetReprojectElevation
(
) const;

/*---------------------------------------------------------------------------------**//**
* Returns the number of geodetic transform definitions.
* @return   The number of geodetic transform definitions.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED size_t      GetGeodeticTransformCount () const;

/*---------------------------------------------------------------------------------**//**
* Gets the indexed geodetic transformation definition.
* GeodeticTransform::Destroy() must be called when the Geodetic Transform is not
* needed anymore.
* @param index IN the index of the geodetic transform definition to obtain. This index must be
*                 between 0 and the number of grid file definition as returned by
*                 GetGeodeticTransformationCount() minus one.
* @param directApply OUT OPTIONAL. If given the bool will receive true if the
*                 transformation is meant to be applied direct and false if it should
*                 be inversed.
* @return   A newly alocated GeodeticTransform
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformP      GetGeodeticTransform (size_t index, bool* directApply = NULL) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the datum converter represents a null transformation.
* To be a Null transform either all component geodetic transform must be null transforms
* or two non-null transforms must be the inverse of the other (resulting in a null trsnsform
* once combined. Note that if reproject elevation is set then the vertical reprojection
* must also be null to have a null transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsNullTransform() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if given datum converter is equivalent or false otherwise.
* note that is reprojection of elevation is active and a vertical conveter is present
* they must also be equivalent.
* @param compareTo IN the datum converter to compare to
* @param    looselyCompare IN OPTIONAL If false then the method checks if the
*              geodetic transform is the same including the method used and the accuracy
*              expected; the test is more strict.
*              If true then the method will verify if the two geodetic transform would
*              yield the same result.
*              For example a 7 parameter definition having no rotation and no scale would be
*              considered equivalent to a 3 parameter transform given the delta values
*              were the same.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent(DatumConverterCR compareTo, bool looselyCompare = true) const;

/*---------------------------------------------------------------------------------**//**
* Free this DatumConverter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;
};

/*=================================================================================**//**
* Local Transformer abstract class.
+===============+===============+===============+===============+===============+======*/
class   LocalTransformer : public RefCountedBase {
protected:
// make sure that nobody else can create one of these.
LocalTransformer();
virtual ~LocalTransformer();

public:

/*---------------------------------------------------------------------------------**//**
* Convert the transformer to JSON. If include root is true then a root node using the type
* label will be added then the definition will be inserted in this node. If includeRoot
* is false then the parameters will be directly added.
* @param jsonValue IN/OUT json to which to add the transformer definition.
* @param includeRoot IN If true the root node will be added.
* @return SUCCESS if successful.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   ToJson (BeJsValue jsonValue, bool includeRoot) = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InternalCartesianFromCartesian (DPoint3dR outInternalCartesian, DPoint3dCR inCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    CartesianFromInternalCartesian (DPoint3dR outCartesian, DPoint3dCR inInternalCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InternalCartesianFromCartesian2D (DPoint2dR outInternalCartesian, DPoint2dCR inCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    CartesianFromInternalCartesian2D (DPoint2dR outCartesian, DPoint2dCR inInternalCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Save local transform parameters to memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    SaveParameters (uint16_t& transformType, double parameters[12]) const = 0;

/*---------------------------------------------------------------------------------**//**
* Read local transform parameters from memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    ReadParameters (double parameters[12]) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    IsEquivalent (LocalTransformerCP other) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    GetDescription (Utf8String& description) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual LocalTransformerP   Copy () const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static LocalTransformerP CreateLocalTransformer(LocalTransformType transformType, double parameters[12]);

/*---------------------------------------------------------------------------------**//**
* Creates the local transformer from JSON.
* @param jsonValue IN json from which the transformer definition is obtained.
* @param errorMessage OUT  A developer facing error message (Non internationalizable).
* @return nullptr if the transformer could not be created.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static LocalTransformerP  CreateLocalTransformerFromJson(BeJsConst jsonValue, Utf8String& errorMessage);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static bool IsEquivalent(LocalTransformerPtr const& transformer1, LocalTransformerPtr const& transformer2);
};

/*=================================================================================**//**
* Linear Local Transformer class.
+===============+===============+===============+===============+===============+======*/
class   HelmertLocalTransformer : public LocalTransformer
{
private:
    double  m_a;
    double  m_b;
    double  m_c;
    double  m_d;
    double  m_e;
    double  m_inverseA;
    double  m_inverseB;
    double  m_inverseC;
    double  m_inverseD;
    double  m_inverseE;

    void    ComputeInverse ();

/*---------------------------------------------------------------------------------**//**
* Constructs a linear transformer of the Helmert type.
* The equations are:
* X = a * x - b * y + c
* Y = b * x + a * y + d
* Z = z + e
* Meaning c, d and e are the offsets in x, y and z while a and b are scaling and rotation
* factors where a = scale * cos(rotation) and b = scale * sin(rotation)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer  (double a, double b, double c, double d, double e);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer ();


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static HelmertLocalTransformer* Create(double a, double b, double c, double d, double e);

/*---------------------------------------------------------------------------------**/ /**
* Creates the helmert transform from JSON.
* @param jsonValue IN json from which the helmert definition is obtained.
* @param errorMessage OUT  A developer facing error message (Non internationalizable).
* @return nullptr if the transformer could not be created.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static HelmertLocalTransformer* CreateFromJson(BeJsConst jsonValue, Utf8String& errorMessage);

/*---------------------------------------------------------------------------------**/ /**
* Convert the helmert transform to JSON. If include root is true then a node 'helmert2DWithZOffset'
* will be added then the helmert definition will be inserted in this node. If includeRoot
* is false then the helmert parameters will be directly added.
* @param jsonValue IN/OUT json to which to add the helmert definition.
* @param includeRoot IN If true the 'helmert2DWithZOffset' root node will be added.
* @return SUCCESS if successful.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt ToJson(BeJsValue jsonValue, bool includeRoot) override;

/*---------------------------------------------------------------------------------**/ /**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
* are appropriate for the geographic projection calculations) from the Cartesian
* coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void InternalCartesianFromCartesian(DPoint3dR outInternalCartesian, DPoint3dCR inCartesian) const override;

/*---------------------------------------------------------------------------------**/ /**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void CartesianFromInternalCartesian(DPoint3dR outCartesian, DPoint3dCR inInternalCartesian) const override;

/*---------------------------------------------------------------------------------**/ /**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
* are appropriate for the geographic projection calculations) from the Cartesian
* coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void InternalCartesianFromCartesian2D(DPoint2dR outInternalCartesian, DPoint2dCR inCartesian) const override;

/*---------------------------------------------------------------------------------**/ /**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
* the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void CartesianFromInternalCartesian2D(DPoint2dR outCartesian, DPoint2dCR inInternalCartesian) const override;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void GetDescription(Utf8String& description) const override;

/*---------------------------------------------------------------------------------**/ /**
* Save local transform parameters to memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void SaveParameters(uint16_t& transformType, double parameters[12]) const override;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual bool IsEquivalent(LocalTransformerCP other) const override;

/*---------------------------------------------------------------------------------**/ /**
* Read local transform parameters from memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void ReadParameters(double parameters[12]) override;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual LocalTransformerP Copy() const override;

BASEGEOCOORD_EXPORTED double GetA() const;
BASEGEOCOORD_EXPORTED double GetB() const;
BASEGEOCOORD_EXPORTED double GetC() const;
BASEGEOCOORD_EXPORTED double GetD() const;
BASEGEOCOORD_EXPORTED double GetE() const;

BASEGEOCOORD_EXPORTED void SetA(double val);
BASEGEOCOORD_EXPORTED void SetB(double val);
BASEGEOCOORD_EXPORTED void SetC(double val);
BASEGEOCOORD_EXPORTED void SetD(double val);
BASEGEOCOORD_EXPORTED void SetE(double val);

BASEGEOCOORD_EXPORTED void GetInternalCartesianFromCartesianTransform(TransformR transform);
BASEGEOCOORD_EXPORTED void GetCartesianFromInternalCartesianTransform(TransformR transform);
};

/*=================================================================================**//**
* Unit enumerations. The Geo prefix is to prevent collision with the DgnPlaform UnitBase and UnitSystem.
*  They in fact have the same values and one can be cast to the other without problems.
+===============+===============+===============+===============+===============+======*/
enum class GeoUnitBase
    {
    None    = 0,
    Meter   = 1,
    Degree  = 2,
    };

enum class GeoUnitSystem
    {
    None     = 0,
    English  = 1,
    Metric   = 2,
    USSurvey = 3,
    };

class   UnitEnumerator;

/*=================================================================================**//**
* Unit class.
+===============+===============+===============+===============+===============+======*/
class           Unit
{
friend  UnitEnumerator;
private:
CSUnitInfo const*           m_csUnit;
mutable Utf8StringP         m_nameString;
mutable Utf8StringP         m_pluralNameString;
mutable Utf8StringP         m_abbreviationString;

// constructor is private. Use FindUnit method
Unit (int index);
~Unit ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
BASEGEOCOORD_EXPORTED static Unit const* FindUnit (Utf8CP unitName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP       GetName() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP       GetPluralName() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP       GetAbbreviation() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoUnitSystem GetSystem() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoUnitBase   GetBase() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int           GetEPSGCode() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double        GetConversionFactor() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void          Destroy() const;
};

typedef class Unit const*       UnitCP;
typedef class UnitEnumerator*   UnitEnumeratorP;

/*=================================================================================**//**
* Unit enumeration class.
+===============+===============+===============+===============+===============+======*/
class   UnitEnumerator
{
private:
int     m_currentIndex;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   UnitEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   ~UnitEnumerator ();

/*---------------------------------------------------------------------------------**//**
* Moves to the next unit
* @return   true if successful in moving to the next Unit, false if there are no more.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool          MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Unit.
* @return   the current Unit.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Unit const*        GetCurrent();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void          Destroy() const;

};

typedef class Ellipsoid*                EllipsoidP;
typedef class Ellipsoid const*          EllipsoidCP;
typedef class Ellipsoid const&          EllipsoidCR;
typedef class EllipsoidEnumerator*      EllipsoidEnumeratorP;
/*=================================================================================**//**
Definition of the globe as elliptical distortion of a sphere.
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class Ellipsoid
{
public:
    static const int NO_ELLIPSOID_CODE = -1; // Indicates there is no ellipsoid.
    static const int CUSTOM_ELLIPSOID_CODE = -2; // Indicates the ellipsoid is custom defined (for a self defined datum)
    static const int USER_ELLIPSOID_CODE_OFFSET = 1000000; // The offset for user-defined ellipsoids. Use defined ellipsoids will have a value greater or equal to this value
    static const int ELLIPSOID_CODE_SEPARATOR = 2000000; // Dummy artifact code made for the separator line in UI. Using that code will be ignored.

private:
int32_t                 m_csError;
CSEllipsoidDef         *m_ellipsoidDef;

mutable Utf8StringP     m_nameString;
mutable Utf8StringP     m_descriptionString;

/*---------------------------------------------------------------------------------**/ /**
 * Ellipsoid constructor - internal use only. Creates and empty invalid ellipsoid.
 * The ellipsoid must be initialized afterwards using a method such as FromJson().
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid();

/*---------------------------------------------------------------------------------**/ /**
 * Ellipsoid constructor
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid(Utf8CP keyName);

/*---------------------------------------------------------------------------------**/ /**
 * Ellipsoid constructor - internal use only
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid(EllipsoidCR source);

/*---------------------------------------------------------------------------------**/ /**
 * Ellipsoid constructor - internal use only
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid(const CSEllipsoidDef& ellipsoidDef);

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
~Ellipsoid();

void AllocateClean();
void Clear();

public:
/*---------------------------------------------------------------------------------**//**
* Initializes a new empty non-initialized instance of the ellipsoid class
* @return New un-initialied ellipsoid.
* @remarks The ellipsoid must be initialized afterward, typically with FromJson() or manually
*          defined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static EllipsoidP    CreateEllipsoid();

static EllipsoidCP CreateEllipsoid (EllipsoidCR source);
BASEGEOCOORD_EXPORTED static EllipsoidCP CreateEllipsoid (CSEllipsoidDef const& ellipsoidDef);


public:
/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Ellipsoid class
* @return Initialized Ellipsoid.
* @remarks If keyName is in the Ellipsoid library and valid, the return Ellipsoid's IsValid() returns true.
*          Otherwise, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static EllipsoidCP CreateEllipsoid (Utf8CP keyName);

/*---------------------------------------------------------------------------------**//**
* Returns whether the Ellipsoid is valid or not.
* @return   True if the Ellipsoid is valid, False otherwise. @see #GetError
* @remarks  If the Ellipsoid does not correspond to a Ellipsoid in the Ellipsoid library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool IsValid() const;

/*---------------------------------------------------------------------------------**/ /**
 * Gets the error code associated with constructor failure if IsValid is false.
 * @return   The CSMap error code. @see #GetErrorMessage
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetErrorMessage (Utf8StringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Ellipsoid
* @return   The name of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the key name of the Ellipsoid.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid.
* @return   The description of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Ellipsoid
* @return   The source of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSource (Utf8StringR source) const;

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Ellipsoid in meters.
* @return   The polar radius of the Ellipsoid in meters.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid in meters.
* @return   The equatorial radius of the Ellipsoid in meters.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Ellipsoid, if known.
* @return   The EPSG code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode () const;

/*---------------------------------------------------------------------------------**//**
* Sets the EPSG code in the ellipsoid definition.
* @param    value IN   The new EPSG code. Can be 0 to 32767 where 0 indicates there are
*                      no EPSG code for this definition
* @return   SUCCESS is successful
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEPSGCode
(
short value
);

/*---------------------------------------------------------------------------------**//**
* Compares this ellipsoid with the argument and returns true if they are equivalent
* @param    compareTo IN     The ellipsoid to compare to.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent
(
EllipsoidCR        compareTo
) const;

/*---------------------------------------------------------------------------------**//**
* Returns the ellipsoid definition class
* @return The pointer to the internal ellipsoid definition object. This pointer
*         can be null for invalid ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSEllipsoidDef*   GetCSEllipsoidDef() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if the Ellipsoid name does not exist in the source library or the system library.
* @param  inSystemLibrary IN Set to true if the method returns true and name collision is with the system library.
* @remarks It is only valid to call this after an ellipsoid has been created and before it has been added to the library.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              NameUnique (bool& inSystemLibrary) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidP        Clone () const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Ellipsoid
* @param description IN     The Ellipsoid description
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (Utf8CP description);

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (Utf8CP source);

/*---------------------------------------------------------------------------------**//**
* Sets the ellipsoid from the provided JSon fragment given.
* @param    jsonValue IN      The JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the definition of the ellipsoid.
* @param jsonValue IN OUT The JSonValue representing the ellipsoid.
*                     Previous JsonValue properties are retained unchanged.
* @param shortForm IN default false. Indicates if the short version Json must be generated
*                  The short version will only contain id, polar and equatorial radiuses
*                  without descritive information.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt        ToJson(BeJsValue jsonValue, bool shortForm = false) const;

/*---------------------------------------------------------------------------------**//**
* Gets the group name to which the ellipsoid belongs.
* @return   The group to which the ellipsoid belongs. If this group is LEGAC then the
*           definition is to be considered deprecated.
*           The group name may not be more than 5 characters long thus the truncation of
*           LEGACY in to LEGAC.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetGroup (Utf8StringR group) const;

/*---------------------------------------------------------------------------------**//**
* Sets the group name to which the ellipsoid belongs.
* @param   The group to which the ellipsoid belongs. Note that the actual
*          groups are typically limited in values yet the values LEGAC and NONE are usual.
*          Setting the group to LEGAC indicates the definition is deprecated. Setting NONE
*          indicates the definition is not part of any group.
*          The group name may not be more than 5 characters long thus the truncation of
*          LEGACY in to LEGAC.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGroup (Utf8StringCR group);

/*---------------------------------------------------------------------------------**//**
* Indicates if the Ellipsoid is deprecated. A deprecated Ellipsoid will have the group name set
* to LEGACY which is an alternate way to check.
* @return   true if the Ellipsoid is deprecated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsDeprecated() const;

/*---------------------------------------------------------------------------------**//**
* Sets the polar radius of the Ellipsoid.
* @param value IN  The polar radius of the Ellipsoid in meters. This value must
*                  be positive and be between 100 meters and 70 million
*                  meters (to support mapping on other celestial objects than Earth).
* @return SUCCESS if successful and another value otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPolarRadius (double value);

/*---------------------------------------------------------------------------------**//**
* Sets the equatorial radius of the Ellipsoid.
* @param value IN  The equatorial radius of the Ellipsoid in meters. This value must
*                   be positive and be between 100 meters and 70 million
*                   meters (to support mapping on other celestial objects than Earth).
* @return SUCCESS if successful and another value otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEquatorialRadius (double value);

/*---------------------------------------------------------------------------------**//**
* Sets flattening and eccentricity
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static bool       CalculateParameters (double& flattening, double& eccentricity, double equatorialRadius, double polarRadius);


/*---------------------------------------------------------------------------------**//**
* Free this Group.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the Ellipsoid.
* This format is only useful for dictionary management purposes.
* @param    EllipsoidAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
Utf8StringR EllipsoidAsASC
) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Factory method to create an EllipsoidEnumerator
* @return  An initialized EllipsoidEnumerator
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static EllipsoidEnumeratorP CreateEnumerator();

};

/*=================================================================================**//**
* Ellipsoid enumeration class.
* @ingroup GeoCoordinate
+===============+===============+===============+===============+===============+======*/
class   EllipsoidEnumerator
{
private:
int     m_currentIndex;
Utf8String m_currentEllipsoidName;
friend class Ellipsoid;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~EllipsoidEnumerator ();


public:
/*---------------------------------------------------------------------------------**//**
* Moves to the next Ellipsoid
* @return   true if successful in moving to the next Ellipsoid, false if there are no more.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Ellipsoid.
* @return   the current Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP       GetCurrent();

/*---------------------------------------------------------------------------------**//**
* Free this EllipsoidEnumerator.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};

typedef class DatumEnumerator*    DatumEnumeratorP;


/*=================================================================================**//**
Position and orientation relative to a WGS84 Datum
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class Datum
{
public:
    static const int NO_DATUM_CODE = -1; // Indicates there is no datum.
    static const int CUSTOM_DATUM_CODE = -2; // Indicates the datum is custom defined (either for a self defined datum)

private:

int32_t                     m_csError;

CSDatumDef*                 m_datumDef;

mutable CSDatum*            m_csDatum;    // this is created only if needed.

// The following member are used to store the full ellipsoid of the datum. Either this
// ellipsoid is create when first requested or is explicitly set using SetEllipsoid(). In this last case the
// member m_customEllipsoid will be set to true indicating a self contained ellispoid is used.
mutable Ellipsoid const*    m_ellipsoid;  // this is created only if needed or is set using SetEllipsoid().
mutable bool                m_customEllipsoid;

mutable Utf8StringP         m_nameString;                   // these are here as "adapters" needed because CS_Map uses all char rather than unicode.
mutable Utf8StringP         m_descriptionString;
mutable Utf8StringP         m_ellipsoidNameString;
mutable Utf8StringP         m_ellipsoidDescriptionString;

// The following two members are used to store the geodetic transform path defined.
// m_listOfTransformsToWgs84 contains the cached path containing the full list of transforms required from the current
// datum to the WGS84 datum.
// The property ,_listOfSetTransforms contains the list of explicitely set geodetic transforms for a self-defined datum.
// The target datum of this path may not be WGS84 but some other system library datum. When the geodetic path will be requested
// the path in m_listOfSetTransforms will be completed to generate the content of m_listOfTransformsToWgs84.
// NOTE: If the target datum name of the m_listOfSetTransforms is empty then WGS84 will be assumed.
mutable GeodeticTransformPath*   m_listOfTransformsToWgs84;  // List of transforms to WGS84. This member is only set if the transforms is requested or if set externally.
GeodeticTransformPath*           m_listOfSetTransforms;  // List of transforms explicitely set for the datum. The target of this path must be a system datum

mutable bvector<GeodeticTransformPath const *> m_listOfAdditionalPaths; // List of paths to other datum explicitely defned not passing through WGS84
mutable bool m_listOfAdditionalPathsBuilt;


/*---------------------------------------------------------------------------------**//**
* Creates  a new empty invalid instance of the Datum class.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Datum ();

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class. Looks only in the system library.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (Utf8CP keyName);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class by given epsg code.
* Looks only in the system library.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (int epsgCode);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (CSDatumDef const& datum, CSGeodeticTransformDef const* geodeticTransform = nullptr);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (CSDatum const& datum, CSGeodeticTransformDef const* geodeticTransform);


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~Datum();

void AllocateClean();
void Clear();

public:

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @return Initialized Datum.
* @remarks If keyName is in the Datum library and valid, the return Datum's IsValid() returns true.
*          Otherwise, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatum (Utf8CP keyName);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class by EPSG number
* @return Initialized Datum.
* @remarks If EPSG is in the Datum library and valid, the return Datum's IsValid() returns true.
*          Otherwise, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatumFromEPSGCode (int epsgCode);

/*---------------------------------------------------------------------------------**//**
* Initializes a new empty non-initialized instance of the Datum class
* @return New un-initialied Datum.
* @remarks The datum must be initialized afterward, typically with FromJson() or manually
*          defined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumP    CreateDatum ();

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* The optional geodetic transform is meant for datums stored in user library that make
* use of grid shift file transformations.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatum (CSDatumDef const& datumDef, CSGeodeticTransformDef const* geodeticTransform = nullptr);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class from a CSDatum definition
* most probably originating from a CSParameter for a self-defined datum originating
* from a stored user-defined datum.
* The optional geodetic transform is meant for datums based on the GENGRID method that make
* use of grid shift file transformations.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatum (CSDatum const& datum, CSGeodeticTransformDef const* geodeticTransform = nullptr);

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is valid or not.
* @return   True if the Datum is valid, False otherwise. @see #GetError
* @remarks  If the datum does not correspond to a datum in the datum library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if the datum uses grid file based transformation and at least one
* file is missing or of the wrong format.
* @param listOfFiles OUT Receives the list of missing files.
* @param includeFilesInAdditionalPaths IN OPTIONAL Indicates if the grid files on additional paths
*        are to be returned as well as the grid files on the path to WGS84.
* @param includeFilesToDeprecatedDatums IN OPTIONAL Indicates if the grid files on additional paths
*         to deprecated datums are to be returned.This value is ignored if
*         includeFilesInAdditionalPaths is false.
* @return   True if the path has missing grid files, False otherwise.
* @bsimethod
+ -------------- - +-------------- - +-------------- - +-------------- - +-------------- - +------*/
BASEGEOCOORD_EXPORTED bool              HasMissingGridFiles(bvector<Utf8String>& listOfFiles, bool includeFilesInAdditionalPaths = false, bool includeFilesToDeprecatedDatums = false) const;

/*---------------------------------------------------------------------------------**//**
* Indicates if the Datum is deprecated. A deprecated Datum will have the group name set
* to LEGACY which is an alternate way to check.
* @return   true if the Datum is deprecated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsDeprecated() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is a variant of NAD27 or not. Only continental USA NAD27
* variants are considered NAD27
* @return   True if the Datum is a variation of NAD27
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsNAD27 () const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is a variant of NAD83 (Excluding Canadian
* and non USA variations) or not.
* @return   True if the Datum is a variation of NAD83
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsNAD83 () const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is coincident with WGS84 or not.
* @return   True if the Datum is coincident with WGS84
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsWGS84Coincident () const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetErrorMessage (Utf8StringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Datum
* @return   The name of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the key name of the Datum.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Datum.
* @return   The description of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Datum.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Datum
* @return   The source of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetSource (Utf8StringR source) const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Datum
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (Utf8CP value);

/*---------------------------------------------------------------------------------**//**
* Gets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   The convert method.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WGS84ConvertCode  GetConvertToWGS84MethodCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   BSISUCCESS or an error code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetConvertToWGS84MethodCode (WGS84ConvertCode value);

/*---------------------------------------------------------------------------------**//**
* Inidcates if the datum transformation uses one of the geocentric methods
* (3, 4, 6, 7, molodenski, bursa wolf,...)
* @return   true if the datum transformation uses a geocentric method.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool  UsesAGeocentricMethod() const;

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta OUT  The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDelta (DPoint3dR delta) const;

/*---------------------------------------------------------------------------------**//**
* Sets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta IN    The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the delta settings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDelta (DPoint3dCR delta);

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation OUT The rotation angles.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetRotation (DPoint3dR rotation) const;

/*---------------------------------------------------------------------------------**//**
* Sets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation IN The rotation angles.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the rotation settings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetRotation (DPoint3dCR rotation) ;

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scaling in parts per million if known.
* @return   The datum transformation scale in ppm.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScale () const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum transformation scaling in parts per million if known.
* @param scalePPM   IN The scale in parts per million.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the scale setting.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetScale (double scalePPM);

/*---------------------------------------------------------------------------------**//**
* Utilitarian method. Although the grid file definitions are part of the transform path
* set using SetStoredGeodticTransformPath() method, some common workflow require
* a single transform using a single grid file. The present method is functionally equivalent
* to clearing caches, setting the convert method to GENGRID, removing the stored
* transform path if any then creating a new stored transform path containing a single
* transform to WGS84 based on grid file using a single grid file.
* @param gridFileDef   IN The new grid file definition for the datum transform.
* @return  SUCCESS is successful or ERROR in case of any error.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGridFile (const GridFileDefinition& gridFileDef);

/*---------------------------------------------------------------------------------**//**
* Utilitarian method. Although the grid file definitions are part of the transform path
* set using SetStoredGeodticTransformPath() method, some common workflow require
* a single transform using a single grid file.
* The present method will return the grid file definition if the datum is currently based
* on a stored geodetic transform path containing a single transform that must use the Grid File
* method to WGS84 and contain a single grid file definition.
* If any of those conditions are not met ERROR is returned, otherwise the grid file definition
* is returned with a SUCCESS status.
* @param gridFileDef   OUT Reference to the grid file that will receive the current
*                      datum grid file definition.
* @param strict IN OPTIONAL default true. If false is provided then the method will return the
*                grid file definition of the first transform even if additional transform exists.
*                If true then there must not exist any additional transform.
* @return  SUCCESS is successful or ERROR if any prerequisite condition indicated above is
*          not met.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetGridFile (GridFileDefinition& gridFileDef, bool strict = true) const;

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Datum, if known.
* @return   The EPSG code.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode () const;

/*---------------------------------------------------------------------------------**//**
* Sets the EPSG code in the datum definition.
* @param    value IN   The new EPSG code. Can be 0 to 32767. Value 0 indicates there is
*                      no EPSG code for this definition.
* @return   SUCCESS is successful.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEPSGCode
(
short value
);

/*---------------------------------------------------------------------------------**//**
* Gets the key name of the Ellipsoid used in this Datum
* @return   The key name Ellipsoid of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Ellipsoid used in this Datum
* @return   Index of the Coordinate System Ellipsoid. If not set then Ellipsoid::NO_ELLIPSOID_CODE
*           is returned. If the ellipsoid is custom (for self-defined datums) then
*           Ellipsoid::CUSTOM_ELLIPSOID_CODE is returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEllipsoidCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the internal index of the Ellipsoid used in this Datum
* @return   Index of the Coordinate System Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEllipsoidCode (int ellipsoidCode);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid used in this Datum
* @return   The description of the Ellipsoid of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Ellipsoid used in this Datum
* @return   The source of the Ellipsoid of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetEllipsoidSource (Utf8StringR ellipsoidSource) const;

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Coordinate System.
* @return   The polar radius of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Returns the pointer to the internal ellipsoid used by this Datum
* @return   The Ellipsoid of the Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP       GetEllipsoid() const;

/*---------------------------------------------------------------------------------**//**
* Sets the ellipsoid for the datum. The ellipsoid is captured and must not be
* destroyed by the caller.
* @param newEllipsoid IN   The new Ellipsoid for the Datum.
* @return SUCCESS if ellipsoid was correctly set.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt       SetEllipsoid(EllipsoidP newEllipsoid);

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for this Datum.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              ParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for this Datum.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @param  gridValid     OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that grid file is used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              ExtendedParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid, bool& gridValid) const;

/*---------------------------------------------------------------------------------**//**
* Gets the CSDatum
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSDatum*          GetCSDatum() const;

/*---------------------------------------------------------------------------------**//**
* Gets the CSDatumDef. It is the internal structure. Even if the returned pointer is not to
* a const object it must be treated so (CSMAP API makes poor use of const)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSDatumDef*       GetCSDatumDef(CSGeodeticTransformDef const** geodetictransform) const;

/*---------------------------------------------------------------------------------**//**
* Gets the CSEllipsoidDef of the ellipsoid referenced by datum. Even if the returned pointer is not to
* a const object it must be treated so (CSMAP API makes poor use of const)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSEllipsoidDef*   GetCSEllipsoidDef() const;

/*---------------------------------------------------------------------------------**//**
* Compares this datum with the argument and returns true if they are equivalent
* @param    compareTo IN     The datum to compare to.
* @param    looselyCompare IN OPTIONAL If false then the method checks if the
*              geodetic transform is the same including the method used and the accuracy
*              expected; the test is more strict.
*              If true then the method will verify if the two geodetic transform would
*              yield the same result.
*              For example a 7 parameter definition having no rotation and no scale would be
*              considered equivalent to a 3 parameter transform given the delta values
*              were the same.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent
(
DatumCR        compareTo,
bool looselyCompare = true
) const;

/*---------------------------------------------------------------------------------**//**
* Returns true if the Datum name does not exist in the source library or the system library.
* @param  inSystemLibrary IN Set to true if the method returns true and name collision is with the system library.
* @remarks It is only valid to call this after a datum has been created and before it has been added to the library.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              NameUnique (bool& inSystemLibrary) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumP            Clone () const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum from the provided JSon fragment given.
* @param    jsonValue IN      The JSonValue to obtain the definition from.
* @param    errorMessage OUT  A developer facing error message (Non internationalizable).
* @return   SUCCESS if successful. An error code otherwise. Typical error
*           are GEOCOORDERR_MissingPropertyOrParameter when a required json
            property is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt FromJson(BeJsConst jsonValue, Utf8StringR errorMessage);

/*---------------------------------------------------------------------------------**//**
* Generates a JSon from the definition.
* Note that the Json representation of the datum includes the geodetic transforms
* to WGS84.
* @param value IN OUT The JSonValue representing the datum.
*              Previous JsonValue properties are retained unchanged.
* @param expandEllipsoid By default it is false. If expansion is requested then
*                       the definition of the ellipsoid will be added to the Json.
* @param includeDeprecatedAdditionals By default it is false. If datum contains additional
*                       transform paths to specific datum, these will only be added
*                       to the json if the self datum is deprecated or this parameter
*                       is true. This prevents cluttering needlessly the result json
*                       with additional transforms that will likely not be needed.
*                       Deprecated datums are always given all additional transforms
*                       to insure given two jsons the additional path is present in
*                       either defining the transformation required.
* @return The returns status.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt        ToJson(BeJsValue value, bool expandEllipsoid = false, bool includeDeprecatedAdditionals = false) const;

/*---------------------------------------------------------------------------------**//**
* Gets the access to the internal geodetic path to WGS84. This path is comprised
* of the stored path (see Get/SetStoredGeodeticTransformPath()) followed by
* the required components needed to complete the path to WGS84.
* @return  The const pointer to the geodetic path. It cannot be changed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformPathCP GetGeodeticTransformPathToWGS84() const;

/*---------------------------------------------------------------------------------**//**
* Gets the access to the internal stored geodetic path if any. This path is the stored
* component for user-library stored datums or for self contained datums.
* The pre-defined transforms between system datum are not in the stored path.
* @return  The const pointer to the stored geodetic path. It cannot be changed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeodeticTransformPathCP GetStoredGeodeticTransformPath() const;

/*---------------------------------------------------------------------------------**//**
* Gets the access to the internal stored list of additional geodetic path if any. Theses paths
* indicate shortcuts or deviation from transforming through WGS84. Although rare
* these additional paths are essential for some specific datums.
* @return  The const pointer to the stored geodetic path. It cannot be changed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bvector<GeodeticTransformPathCP> const & GetAdditionalGeodeticTransformPaths() const;

/*---------------------------------------------------------------------------------**//**
* Adds an additional geodetic path if any. Theses paths
* indicate shortcuts or deviation from transforming through WGS84. Although rare
* these additional paths are essential for some specific datums.
* @param newPath IN The new Geodetic path. This path is given and must not be deallocated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt AddAdditionalGeodeticTransformPath(GeodeticTransformPathCP newPath);

/*---------------------------------------------------------------------------------**//**
* Sets the stored geodetic transform path for a user-defined library stored datum
* or a self contained datum. Changing the stored path invalidates the path to
* WGS84.
* @param newPath - Pointer to the new transform. The given path is captured and must not
* be deallocated by caller. The new geodetic transform path contains a list of transforms
* the source datum of the first should be the current datum (it will be automatically be set if needed)
* The target datum of the last transform in the path is the path target datum. It can be specified
* or it can be left unset. If unset then WGS84 is assumed.
* @return  SUCCESS if the new path could be set.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt SetStoredGeodeticTransformPath(GeodeticTransformPathP newPath);

/*---------------------------------------------------------------------------------**//**
* Checks if the datum has a null transform to WGS84. If needed the geodetic transforms
* from the system library will be extracted (given the datum originates from this
* system library). If there are no geodetic transform or they are invalid due to
* missing grid files then the fallback definition is checked.
* @return  true if the datum has a null transform to WGS84.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool      HasNullTransformToWGS84() const;

/*---------------------------------------------------------------------------------**//**
* Free this Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the datum.
* This format is only useful for dictionary management purposes.
* @param    DatumAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
Utf8StringR            DatumAsASC
) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Factory method to create a DatumEnumerator
* @return  An initialized DatumEnumerator
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumEnumeratorP CreateEnumerator();

/*---------------------------------------------------------------------------------**//**
* Gets the group name to which the Datum belongs.
* @return   The group to which the Datum belongs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Utf8CP           GetGroup (Utf8StringR group) const;

/*---------------------------------------------------------------------------------**//**
* Sets the group name to which the Datum belongs.
* @param   The group to which the Datum belongs. Note that the actual
*          groups are typically limited in values yet the values LEGACY and NONE are usual.
*          Setting the group to LEGACY indicates the definition is deprecated. Setting NONE
*          indicates the definition is not part of any group.
*          The group name may not be more than 23 characters long.
* @return  SUCCESS if the group was set and an error otherwise.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGroup (Utf8StringCR group);


};

/*=================================================================================**//**
* Datum enumeration class.
* @ingroup GeoCoordinate
+===============+===============+===============+===============+===============+======*/
class   DatumEnumerator
{
private:
int        m_currentIndex;
Utf8String m_currentDatumName;
friend  class Datum;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DatumEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~DatumEnumerator ();

public:
/*---------------------------------------------------------------------------------**//**
* Moves to the next Datum
* @return   true if successful in moving to the next Datum, false if there are no more.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Datum.
* @return   the current Datum.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumCP           GetCurrent();

/*---------------------------------------------------------------------------------**//**
* Free this DatumEnumerator.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};

/*=================================================================================**//**
* Provides localizaion services to DgnGeoCoord.
+===============+===============+===============+===============+===============+======*/
class   BaseGeoCoordResource
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static Utf8CP    GetLocalizedProjectionName
(
Utf8StringR                     outString,
BaseGCS::ProjectionCodeValue    projectionCode
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static Utf8CP    GetLocalizedProjectionName
(
Utf8StringR                     outString,
DgnProjectionTypes              projectionType
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static Utf8CP    GetLocalizedString
(
Utf8StringR             outString,
DgnGeoCoordStrings      stringNum
);

};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef class MilitaryGridConverter*                  MilitaryGridConverterP;
typedef RefCountedPtr<MilitaryGridConverter>          MilitaryGridConverterPtr;

class   MilitaryGridConverter : public RefCountedBase
{
private:
CSMilitaryGrid*         m_csMgrs;
DatumConverterP         m_toWGS84Converter;
DatumConverterP         m_fromWGS84Converter;

BASEGEOCOORD_EXPORTED MilitaryGridConverter (BaseGCSR baseGCS, bool useBessel, bool useWGS84Datum);
BASEGEOCOORD_EXPORTED virtual   ~MilitaryGridConverter ();

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static MilitaryGridConverterPtr CreateConverter (BaseGCSR baseGCS, bool useBessel, bool useWGS84Datum);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt   LatLongFromMilitaryGrid (GeoPoint2dR latLong, Utf8CP mgString);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt   MilitaryGridFromLatLong (Utf8String& mgString, GeoPoint2dCR latLong, int precision);
};


} // End namespace GeoCoordinates

END_BENTLEY_NAMESPACE
