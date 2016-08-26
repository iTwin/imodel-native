/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/BaseGeoCoord.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Geom/GeomApi.h>
#include "BaseGeoDefs.r.h"

#include    <GeoCoord/IGeoTiffKeysList.h>

typedef struct cs_Csprm_    CSParameters;

#include "ExportMacros.h"

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>

/** @namespace BentleyApi::GeoCoordinates Geographic Coordinate System classes @see GeoCoordinate */
BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

/*__PUBLISH_SECTION_END__*/
// NOTE: This was added to meet the Caltrans requirements for setting the Vertical Datum separately from the Datum.
/*__PUBLISH_SECTION_START__*/
enum VertDatumCode
    {
    vdcFromDatum    = 0,    // Vertical Datum implied by Datum
    vdcNGVD29       = 1,    // Vertical Datum of 1929
    vdcNAVD88       = 2,    // Vertical Datum of 1988.
    vdcGeoid        = 3     // Other Geoid (indicates GeoidHeight.gdc catalog should be used)
    };

/*__PUBLISH_SECTION_END__*/
// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy it. Check it.
/*__PUBLISH_SECTION_START__*/
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
#ifdef GEOCOORD_ENHANCEMENT
    ConvertType_GENGRID   =   27,      
    ConvertType_MAXVALUE  =   27,       // the maximum allowable value.
#else
    ConvertType_MAXVALUE  =   26,       // the maximum allowable value.
#endif
    };

typedef struct Library*                 LibraryP;


/*=================================================================================**//**
* @addtogroup GeoCoordinate

A Geographic Coordinate System describes the way that coordinates on the earth's surface
(which are generally described in degrees of longitude, degrees of latitude, and elevation
above the surface) are transformed to a cartesian coordinate system that can be represented
on an inherently planar medium such as a sheet of paper or a computer screen.
<p>
In general, a Geographic Coordinate System (referred to below as a GeoCoordinate System or GCS)
is fully described by a projection type, the mathematical parameters that customize thae projection,
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
An extensive Coordinate System Library is supplied with MicroStation and related products, and Geographic
Coordinate Systems from that library can be used by looking them up by name - see the appropriate GCS constructors.
<p>
A full discussion of Geographic Coordinate Systems can be found in "Elements of Cartography" by Arthur
H. Robinson, et al. ISBN-10: 0471728055, and in many other books on the subject.
<p>
The GeoCoordinate API makes use of the "CS_MAP" library published by Mentor Software, Inc. for all the
projection calculations and datum conversion algorithms. Mentor also supplies the Coordinate System
Library.
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
* in a destination coordinate system. This does the datum conversion that is necessary when the two GCS's utilize different
* datum.
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
MPUBLIC class EXPORT_VTABLE_ATTRIBUTE BaseGCS : public RefCountedBase
{
private:
static bool                 s_geoCoordInitialized;  // global that indicates if the geocoord engine was initialized. If not then no GCS can 
                                                            // be created which renders the whole system non-operational
protected:
CSParameters*           	m_csParameters;                 // all coordinate system parameters, gathered for use by the CSMap transformation functions.
mutable BaseGCSCP          	m_destinationGCS;            // current destination coordinate system.
mutable bvector<BaseGCSCP> 	m_listOfPointingGCS;         // List of BaseGCS that are using the current BaseGCS as a cached destination GCS      
mutable DatumConverterP 	m_datumConverter;               // datum converter from this Lat/Long to the Lat/Long of m_destinationGCS.
bool                    	m_reprojectElevation;           // if true, LatLongFromLatLong adjusts elevation values.
int32_t                     m_coordSysId;                   // our internal coordinate system ID
VertDatumCode           	m_verticalDatum;
mutable int32_t             m_csError;
bool                    	m_canEdit;
mutable LibraryP        	m_sourceLibrary;                // The library from which the GCS originated. NULL means system library.
mutable bool            	m_failedToFindSourceLibrary;    // We tried to find the source library, but were unable to do so.
LocalTransformerPtr     	m_localTransformer;             // The local transformer converts to or from the GCS Cartesian coordinates to Local Cartesian coordinates.

mutable WStringP        	m_nameString;                   // these are here as "adapters" needed because CS_Map uses all char rather than unicode.
mutable WStringP        	m_descriptionString;
mutable WStringP        	m_projectionString;
mutable WStringP        	m_datumNameString;
mutable WStringP        	m_datumDescriptionString;
mutable WStringP        	m_ellipsoidNameString;
mutable WStringP        	m_ellipsoidDescriptionString;

mutable bvector<IGeoTiffKeysList::GeoKeyItem> 
                            m_originalGeoKeys;     // A size of 0 indicates that the BaseGCS was not initialized from GeoTiffKeys or has been modified since init.
mutable WStringP            m_originalWKT;         // A NULL or empty string indicates the BaseGCS was not initialized from a Well Known Text or as been modified since init.

mutable bool                m_modified;            // Indicates if the BaseGCS has been modified from original definition. At the moment it is only internal.
/// @cond NODOC
friend struct GeoTiffKeyInterpreter;
friend struct GeoTiffKeyCreator;
/// @endcond

protected:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
void                                    Init();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED                   BaseGCS (BaseGCSCR source);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED                   BaseGCS ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED                   BaseGCS (WCharCP coordinateSystemKeyName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED                   BaseGCS (CSParameters& csParameters, int32_t coordSysId, LibraryP sourceLibrary);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual           ~BaseGCS ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverterP SetupDatumConverterFor (BaseGCSCR destGCS) const;


/*---------------------------------------------------------------------------------**//**
* USED By another Base GCS only
* This method is called by a BaseGCS upon which the 'this' BaseGCS is registered. 
* By calling this method, a BaseGCS indicates that the link between BaseGCS established 
* by the m_destinationGCS and m_datumConverter members must be severed. 
* Note that only mutable members are modified thus the constness of the method
* @bsimethod                                    Alain.Robert                   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void ClearCache() const;

/*---------------------------------------------------------------------------------**//**
* Called by another BaseGCS to indicate that this GCS given as a parameter used the
* present BaseGCS as part of his cache members m_destinationGCS.
* When the value of m_destinationGCS is changed the BaseGCS must unregister form the 
* target GCS using UnRegisterIsADestinationOf() prior to setting the member to the
* new value.
* Note that only mutable members are modified thus the constness of the method
* @bsimethod                                    Alain.Robert                   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void RegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const;

/*---------------------------------------------------------------------------------**//**
* Called by another BaseGCS to indicate that this GCS given as a parameter used to be 
* making a references to the present BaseGCS as part of his cache members m_destinationGCS.
* The present call indicates the link is severed and the BaseGCS are not linked anymore.
* Note that only mutable members are modified thus the constness of the method
* @bsimethod                                    Alain.Robert                   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void UnRegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const;

/*---------------------------------------------------------------------------------**//**
* Called to indicate the BaseGCS has been modified or not from the original definition.
* this internal method should be called with true whenever a definition parameter is set
* and called false whenever the definition is complete from one of the Init method
* or a call to DefinitionComplete()
* @bsimethod                                    Alain.Robert                   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SetModified(bool modified);


public:

BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS (CSParameters const& csParameters, int32_t coordSysId);

BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS (CSParameters const& csParameters, int32_t coordSysId, LibraryP sourceLibrary);

public:
/*---------------------------------------------------------------------------------**//**
* Initialize the Geographic Coordinate System libraries. MicroStation performs the
* initialization if running within that environment. If basegeocoord.dll is used by
* a standalone program, that program is responsible for initialization.
* @param  dataDirectory  IN  The directory that contains the CSMap data files.
* @bsimethod                                                    Barry.Bentley   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static void       Initialize (WCharCP dataDirectory);

/*---------------------------------------------------------------------------------**//**
* Indicates if the Geographic Coordinate System library was initialized.
 MicroStation performs the
* initialization if running within that environment. If basegeocoord.dll is used by
* a standalone program, that program is responsible for initialization.
* @return true if geocoord has been initialized.
* @bsimethod                                                    Alain.Robert   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static bool       IsLibraryInitialized ();

public:

/*---------------------------------------------------------------------------------**//**
* Returns an empty BaseGCSPtr. This factory method is designed
* to be used in conjunction with the initialization methods such as the InitAzimuthalEqualArea
* method.
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS ();

/*---------------------------------------------------------------------------------**//**
* Initializes a BaseGCS by looking for the specified key name in the Coordinate System Library.
* If the coordinate system specified is found, then the IsValid method returns true. Otherwise the
* GetError method returns the CS_MAP error code.
* @param    coordinateSystemKeyName    IN  The key name of the coordinate system.
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS (WCharCP coordinateSystemKeyName);

/*---------------------------------------------------------------------------------**//**
* Returns a copy of the object. 
* @bsimethod                                    Marc.Bedard                     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static BaseGCSPtr CreateGCS (BaseGCSCR baseGcs);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the no-argument contructor to set the BaseGCS to
* an Azimuthal Equal Area projection. Such a projection is the mathematical equivalent of
* placing a flat sheet of paper on the surface of the area at originLongitude, originLatitude,
* projecting the earth's surface onto that paper, and then adding the "falseEasting", "falseNorthing"
* values to yield cartesian coordinates.
* @return   SUCCESS or a CS_MAP error code.
* @param    errorMsg        OUT     if non-NULL, the WString is filled in with the CS_MAP error
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
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitAzimuthalEqualArea
(
WStringP                errorMsg,
WCharCP                 datumName,
WCharCP                 unitName,
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
* @param    errorMsg        OUT     if non-NULL, the WString is filled in with the CS_MAP error
*                                   message when an error occurs.
* @param    datumName       IN      The name of the datum used in the GCS, such as "WGS84".
* @param    unitName        IN      The name of the linear unit for the Cartesian coordinates, such as "METER".
* @param    originLongitude IN      The longitude of the tangency point.
* @param    originLatitude  IN      The latitude of the tangency point.
* @param    scale           IN      This scale reduction at the origin.
* @param    falseEasting    IN      The value to add to each Cartesian X value.
* @param    falseNorthing   IN      The value to add to each Cartesian Y value.
* @param    quadrant        IN      Quadrant for the cartesian coordinate system. If north is up and east is right, pass 1.
* @bsimethod                                                    Alain.Robert   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitTransverseMercator
(
WStringP                errorMsg,
WCharCP                 datumName,
WCharCP                 unitName,
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
* @param    errorMsg        OUT     if non-NULL, the WString is filled in with the CS_MAP error
*                                   message when an error occurs.
* @param    datumName       IN      The name of the datum used in the GCS, such as "WGS84".
* @param    ellipsoidName   IN      The name of the ellipsoid used in the GCS, such as "WGS84". This is used only if the datumName is NULL.
* @param    unitName        IN      The name of the linear unit for the Cartesian coordinates, such as "METER".
* @param    originLongitude IN      Allows displacement of the longitude values if a different origin is desired - usually 0.0.
* @param    originLatitude  IN      Allows displacement of the latitude values if a different origin is desired - usually 0.0.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitLatLong
(
WStringP                errorMsg,
WCharCP                 datumName,          // Datum
WCharCP                 ellipsoidName,      // only if datum is NULL.
WCharCP                 unitName,           // usually "DEGREE"
double                  originLongitude,    // displacement from Greenwich
double                  originLatitude      // displacement from Greenwich
);


/*__PUBLISH_SECTION_END__*/
// NOTE: The values in this enum are copied from cs_map.h. I don't want to users of this .h file to have to include CS_map.h, so I have to copy it. Check it.
/*__PUBLISH_SECTION_START__*/
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

    // Note concerning these last entries and specificaly wktFlavorUnknown
    // The values for these used to be Unknown = 7 AppAlt = 8 and LclAlt = 9
    // As the latter two were yet unsupported they cannot cause any issue but the 
    // Change for the Unknown may lead to a backward compatibily issue for libraries if additional
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
* @param    warningErrorMsg OUT     if non-NULL, the WString is filled in with the CS_MAP warning or error message.
* @param    wktFlavor       IN      The WKT Flavor. If not known, use wktFlavorUnknown.
* @param    wellKnownText   IN      The Well Known Text specifying the coordinate system.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromWellKnownText
(
StatusInt*              warning,
WStringP                warningErrorMsg,
WktFlavor               wktFlavor,
WCharCP                 wellKnownText
);

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from an
* EPSG coordinate system code. The valid EPSG code ranges are from 20000 through 32767 for projected coordinate systems
* and 4000 through 4199 for geographic (Lat/long) coordinate systems.
* @return   SUCCESS or a CS_MAP error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the WString is filled in with the CS_MAP warning or error message.
* @param    epsgCode        IN      The EPSG code for the desired coordinate system.
* @remarks  Only those EPSG coordinate systems that are in our library will be successfully created.
*           The method first looks for a coordinate system named EPSGnnnnn, where nnnnn is the EPSG code.
*           If that fails, it looks in CS-Map's lookup table to see if the EPSG code appears there.
*           If that fails, it returns an error code and the IsValid method of the coordinate system will return false.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromEPSGCode
(
StatusInt*              warning,
WStringP                warningErrorMsg,
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetWellKnownText
(
WStringR                wellKnownText,
WktFlavor               wktFlavor,
bool                    originalIfPresent
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
*                                    
* @bsimethod                                                    Alain.Robert   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetCompoundCSWellKnownText
(
WStringR                wellKnownText,
WktFlavor               wktFlavor,
bool                    originalIfPresent
) const;

/*---------------------------------------------------------------------------------**//**
* Used in conjunction with the CreateGCS factory method to set the BaseGCS from a
* set of GeoTiff Keys. Note that the original list of geotiff keys will be preserved
* inside the BaseGCS object. This mechansim allows to obtain the geotiff keys exactly
* as they were when interpreted for the sake of comformity to a client data standard.
* @return   SUCCESS, a CS_MAP error code, or a GeoCoord error code.
* @param    warning         OUT     if non-NULL, this might reveal a warning even if the return value is SUCCESS.
* @param    warningErrorMsg OUT     if non-NULL, the WString is filled in with the CS_MAP warning or error message.
* @param    geoTiffKeys     IN      an object implementing the IGeoTiffKeysList interface.
* @param    allowUnitsOverride IN   if true then default units can be overriden by the presence
*                                   a proj linear unit even though the units associated to
*                                   the GCS specifies different units
*                                   This parameter is necessary since many client make use of this mechanism.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         InitFromGeoTiffKeys
(
StatusInt*                  warning,
WStringP                    warningErrorMsg,
::IGeoTiffKeysList const*   geoTiffKeys,
bool                        allowUnitsOverride
);

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* High performance way of changing the BaseGCS to represent a different named coordinate system.
* @param    coordinateSystemKeyName IN      cs name.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFromCSName (WCharCP coordinateSystemKeyName);
/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Saves the coordinate system to GeoTiffKeys, if possible.
* set of GeoTiff Keys.
* @return   SUCCESS, a CS_MAP error code, or a GeoCoord error code.
* @param    geoTiffKeys     IN      an object implementing the IGeoTiffKeysList interface.
* @param    originalsIfPresent IN true indicates that original geokeys should be returned
*                                 if the baseGCS was originally created using geo keys and
*                                 was not modified. (addition Alain Robert 11/2015)
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetGeoTiffKeys
(
::IGeoTiffKeysList*     geoTiffKeys,         // The GeoTiff keys list.
bool                    originalsIfPresent   // true indicates the original geokeys should be returned

) const;

/*---------------------------------------------------------------------------------**//**
* Reveals whether the coordinate system can be saved to GeoTiffKeys.
* @return   true if the coordinate system can be saved to GeoTiffKeys, false if not.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              CanSaveToGeoTiffKeys () const;


/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Reveals whether the datum can be saved to GeoTiffKeys.
* @return   true if the datum for the coordinate system can be saved to GeoTiffKeys, false if not.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              CanSaveDatumToGeoTiffKeys () const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSParameters*     GetCSParameters() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED size_t            GetCSParametersSize() const;
/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Returns whether the Coordinate System is valid or not.
* @return   True if the Coordinate System is valid, False otherwise. @see #GetError
* @remarks  If the coordinate system is constructed using the coordinate system keyname
*           constructor, and the specified name does not correspond to a coordinate system
*           in the coordinate system library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetErrorMessage (WStringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with the error code.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static WCharCP    GetErrorMessage (WStringR errorMsg, StatusInt errorCode);

/*__PUBLISH_SECTION_END__*/
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
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               Matches (char const * const * matchStrings, int numMixedCase, int numUpperCase, bool anyWord) const;

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Tests the coordinate system definition for validity.
* @return   True if the Coordinate System parameters are valid, false if not.
* @param    errorList       OUT     A list of validation errors generated by CSMap.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              Validate (T_WStringVector& errorList) const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the Coordinate System is a standard coordinate system or not.
* @return   True if the Coordinate System originated from the coordinate system library, False otherwise.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsStandard() const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System
* @return   The name of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the name of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (WCharCP name);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Coordinate System.
* @return   The description of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (WCharCP description);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the projection used by the Coordinate System.
* @return   The name of the projection used by the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetProjection() const;

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
    };

/*---------------------------------------------------------------------------------**//**
* Gets CS_Map projection code of the Coordinate System.
* @return   The CS_Map projection code of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ProjectionCodeValue    GetProjectionCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets CS_Map projection code of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetProjectionCode (ProjectionCodeValue projectionCode);

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Gets the CSMap group to which the Coordinate System belongs.
* @return   The CSMap group to which the Coordinate System belongs.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetGroup (WStringR group) const;

/*---------------------------------------------------------------------------------**//**
* Sets the group of the Coordinate System. The group specified must be the identifier
* of a known group.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Alain.Robert   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGroup (WCharCP source);

/*---------------------------------------------------------------------------------**//**
* Gets the location for which the Coordinate System applies.
* @return   The location for which the Coordinate System applies.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetLocation (WStringR location) const;

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Coordinate System.
* @return   The source of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetSource (WStringR source) const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Coordinate System
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (WCharCP source);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System units.
* @return   Name of source of the Coordinate System units.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetUnits (WStringR units) const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index for the Coordinate System units.
* @return   index into internal table of unit names.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetUnitCode() const;

/*---------------------------------------------------------------------------------**//**
* Finds the EPSG code for the unit used by this coordinate system.
* @return   The EPSG code, or 0 if the unit cannot be found in the CS Map table or the
            there is no EPSG code corresponding to the unit used.
* @bsimethod                                                    Mathieu St-Pierre   01/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGUnitCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Units to those indicated by the unit code. The unit code must come
*  from either GetUnitCode or an index into the array returned by GetUnitNames.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetUnitCode (int code);

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Datum.
* @return   Name of the Coordinate System Datum.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDatumName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Coordinate System Datum.
* @return   Index of the Coordinate System Datum.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetDatumCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the internal index of the Datum for the Coordinate System Datum. Must be an
*  index into the array returned by the GetDatumNames static method.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDatumCode (int datumCode);

/*---------------------------------------------------------------------------------**//**

* Gets the description of the Coordinate System Datum.
* @return   Description of the Coordinate System Datum.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDatumDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the Coordinate System Datum source citation.
* @return   Source citation of the Coordinate System Datum.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDatumSource (WStringR datumSource) const;

/*---------------------------------------------------------------------------------**//**
* Gets the method used to convert longitude/latitude from the Datum of this GCS to the WGS84 datum.
* @return   The convert method.
* @note: If this GCS does not have a Datum, ConvertType_NONE is returned.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WGS84ConvertCode  GetDatumConvertMethod() const;

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of the Datum of this GCS.
* @param    delta OUT  The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDatumDelta (DPoint3dR delta) const;

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of the Datum of this GCS
* @param    rotation OUT  The rotation angles.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDatumRotation (DPoint3dR rotation) const;

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scaling in parts per million of the Datum of this GCS, if known.
* @return   The datum transformation scale in ppm.
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetDatumScale () const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for the Datum of this GCS.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @return true if any of the output valid flags are true.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              DatumParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Vertical Datum. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum. If the Datum of this GCS is not NAD83 or NAD27, then this
* method returns the same thing as GetDatumName.
* @return   Name of the Coordinate System Vertical Datum.
* @bsimethod                                                    Barry.Bentley   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetVerticalDatumName() const;

/*__PUBLISH_SECTION_END__*/
BASEGEOCOORD_EXPORTED bool            IsNAD27 () const;
BASEGEOCOORD_EXPORTED bool            IsNAD83 () const;
/*__PUBLISH_SECTION_START__*/



/*---------------------------------------------------------------------------------**//**
* Gets the Vertical Datum Code. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum.
* @return   Member of the VerticalDatum enum indicating Vertical Datum.
* @bsimethod                                                    Barry.Bentley   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED VertDatumCode     GetVerticalDatumCode () const;

/*---------------------------------------------------------------------------------**//**
* Sets the Vertical Datum Code. In only a few instances, the
* Vertical Datum can be set separately from the Datum. Currently, when the Datum of this
* GCS is either NAD83 or NAD27, the Vertical Datum can be set to either NAVD88 or NGVD29
* independently of the Datum.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetVerticalDatumCode (VertDatumCode);


/*---------------------------------------------------------------------------------**//**
* Gets the name of the Coordinate System Ellipsoid.
* @return   Name of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Coordinate System Ellipsoid.
* @return   Index of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEllipsoidCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the internal index of the Coordinate System Ellipsoid. Must be an
*  index into the array returned by the GetEllipsoidNames static method.
* @remarks  The Ellipsoid can only be set if the Datum Code is -1 (none).
*           Otherwise, the Datum determines the Ellipsoid.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEllipsoidCode (int ellipsoidCode);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Coordinate System Ellipsoid.
* @return   The description of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the Coordinate System Ellipsoid source citation.
* @return   Source citation of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidSource (WStringR ellipsoidSource) const;

#if defined (DGNGEOORD_ONLY)
/*---------------------------------------------------------------------------------**//**
* Gets a name suitable for displaying in a user interface.
* @return   The display name of the coordinate system.
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDisplayName (WStringR displayName) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Gets the origin latitude of the Coordinate System.
* @return   The origin latitude of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetOriginLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the origin latitude of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetOriginLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the origin longitude of the Coordinate System.
* @return   The origin longitude of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetOriginLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the origin longitude of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetOriginLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the false easting of the Coordinate System.
* @return   The value added to all x cartesian coordinates.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetFalseEasting() const;

/*---------------------------------------------------------------------------------**//**
* Sets the false easting of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFalseEasting (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the false northing of the Coordinate System.
* @return   The value added to all y cartesian coordinates.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetFalseNorthing() const;

/*---------------------------------------------------------------------------------**//**
* Sets the false northing of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetFalseNorthing (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the scale reduction of the Coordinate System.
* @return   The scale reduction for the Coordinate System.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScaleReduction() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Scale Reduction of the Coordinate System.
* @return   SUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetScaleReduction (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Coordinate System.
* @return   The polar radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Gets the minimum longitude for the Geographic Coordinate System.
* @return   The minimum longitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the minimum longitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMinimumLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the maximum longitude for the Geographic Coordinate System.
* @return   The maximum longitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the maximum longitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMaximumLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the minimum latitude for the Geographic Coordinate System.
* @return   The minimum latitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the minimum latitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMinimumLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the maximum latitude for the Geographic Coordinate System.
* @return   The maximum latitude for Geographic (Longitude/Latitude) coordinate systems.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the maximum latitude for the Geographic Coordinate System.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetMaximumLatitude (double value);


/*---------------------------------------------------------------------------------**//**
* Gets the minimum useful longitude for the Geographic Coordinate System.
* @return   The minimum useful longitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MinimumLongitude is not specified in the coordinate system library,
*           a minimum longitude is calculated.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumUsefulLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the maximum useful longitude for the Geographic Coordinate System.
* @return   The maximum useful longitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MaximumLongitude is not specified in the coordinate system library,
*           a maximum longitude is calculated.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumUsefulLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the minimum useful latitude for the Geographic Coordinate System.
* @return   The minimum useful latitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MinimumLatitude is not specified in the coordinate system library,
*           a minimum latitude is calculated.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMinimumUsefulLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Gets the maximum useful latitude for the Geographic coordinate system.
* @return   The maximum useful latitude for Geographic (Longitude/Latitude) coordinate systems.
* @remark   If the value of MaximumLatitude is not specified in the coordinate system library,
*           a maximum latitude is calculated.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetMaximumUsefulLatitude() const;


/*---------------------------------------------------------------------------------**//**
* Gets the first standard parallel for Projections that have one.
* @return   The first standard parallel for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetStandardParallel1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the first standard parallel for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetStandardParallel1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the first standard parallel for Projections that have one.
* @return   The first standard parallel for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetStandardParallel2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the second standard parallel for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetStandardParallel2 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the azimuth angle for Projections that have one.
* @return   The azimuth angle for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAzimuth() const;

/*---------------------------------------------------------------------------------**//**
* Sets the azimuth angle for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAzimuth (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central meridian (in degrees) for Projections that have one.
* @return   The central meridian longitude for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralMeridian() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central meridian (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralMeridian (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the eastern meridian (in degrees) for Projections that have one.
* @return   The eastern meridian longitude for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEasternMeridian() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central meridian (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEasternMeridian (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central point longitude (in degrees) for Projections that have one.
* @return   The central point longitude for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralPointLongitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central point longitude (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralPointLongitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the central point latitude (in degrees) for Projections that have one.
* @return   The central point latitude for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetCentralPointLatitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the central point latitude (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetCentralPointLatitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the longitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   The longitude of the first point of the central geodesic for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint1Longitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the longitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint1Longitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the latitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   The latitude of the first point of the central geodesic for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint1Latitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the latitude of the first point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint1Latitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the longitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   The longitude of the second point of the central geodesic for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint2Longitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the longitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint2Longitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the latitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   The latitude of the second point of the central geodesic for Projections that have one.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPoint2Latitude() const;

/*---------------------------------------------------------------------------------**//**
* Sets the latitude of the second point of the central geodesic (in degrees) for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetPoint2Latitude (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the geoid separation, in CS units, for Projections that use that parameter.
* @return   The geoid separation, in CS Units for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetGeoidSeparation() const;

/*---------------------------------------------------------------------------------**//**
* Sets the geoid separation, in CS units, for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetGeoidSeparation (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the elevation above the geoid, in CS units, for Projections that use that parameter.
* @return   The elevation above the geoid, in CS Units for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetElevationAboveGeoid() const;

/*---------------------------------------------------------------------------------**//**
* Sets the elevation above the geoid, in CS units, for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetElevationAboveGeoid (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the UTM Zone number (1-60) for the Univeral Transverse Mercator projection.
* @return   The UTM Zone number for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetUTMZone() const;

/*---------------------------------------------------------------------------------**//**
* Sets the UTM Zone number (1-60) for the Universal Transverse Mercator projections.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetUTMZone (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the Hemisphere (1 for north or -1 for south) for the Univeral Transverse Mercator projection.
* @return   The UTM Zone number for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetHemisphere() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Hemisphere (1 for north or -1 for south) for the Univeral Transverse Mercator projection.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetHemisphere (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the Quadrant for Projections that use that parameter.
* @return   The Quadrant for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetQuadrant() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Quadrant for Projections that use that parameter.
* @return   The Quadrant for Projections that use that parameter.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetQuadrant (short value);

/*---------------------------------------------------------------------------------**//**
* Gets the Danish System 34 Region for Danish Sys 34 Projections.
* @return   The Region.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetDanishSys34Region() const;

/*---------------------------------------------------------------------------------**//**
* Sets the Danish System 34 Region for Danish Sys 34 Projections.
* @return   The Region.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDanishSys34Region (int value);

/*---------------------------------------------------------------------------------**//**
* Gets the A0 Affine post-processing parameter for Projections that have one.
* @return   The A0 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA0() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A0 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA0 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the A1 Affine post-processing parameter for Projections that have one.
* @return   The A1 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A1 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the A2 Affine post-processing parameter for Projections that have one.
* @return   The A2 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineA2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the A2 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineA2 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B0 Affine post-processing parameter for Projections that have one.
* @return   The B0 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB0() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B0 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineB0 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B1 Affine post-processing parameter for Projections that have one.
* @return   The B1 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB1() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B1 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineB1 (double value);

/*---------------------------------------------------------------------------------**//**
* Gets the B2 Affine post-processing parameter for Projections that have one.
* @return   The B2 Affine post-processing parameter for Projections that have one.
* @bsimethod                                                    Barry.Bentley   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetAffineB2() const;

/*---------------------------------------------------------------------------------**//**
* Sets the B2 Affine post-processing parameter for Projections that have one.
* @return   SUCCESS or error code
* @bsimethod                                                    Barry.Bentley   04/09
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

* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetAffineParameters (double* A0, double* A1, double* A2, double* B0, double* B1, double* B2) const;

/*---------------------------------------------------------------------------------**//**
* Sets the affine parameters for the affine post-processing portion of Transverse Mercator
* with post affine or Lambert Conformal Conic with post affine projections. The
* transformation must be valid. In order to be valid, the determinant of the matrix formed
* by parameters A1 A2 and B1 B2 must be different than 0.0. To express no rotation, scale
* nor shearing, set A1 and B2 equal to 1.0 and A2 and B1 equal to 0.0.
*
*
* @param    A0 IN  The X translation of the affine transformation
* @param    A1 IN  The A1 parameter of the rotation/scale/shearing portion of the affine.
* @param    A2 IN  The A2 parameter of the rotation/scale/shearing portion of the affine.
* @param    B0 IN  The Y translation of the affine transformation
* @param    B1 IN  The B1 parameter of the rotation/scale/shearing portion of the affine.
* @param    B2 IN  The B2 parameter of the rotation/scale/shearing portion of the affine.

* @return   SUCCESS or error code
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetAffineParameters (double A0, double A1, double A2, double B0, double B1, double B2);

/*---------------------------------------------------------------------------------**//**
* Signals that the caller has finished setting the coordinate system parameters, and that
* the coordinate system internal definition should be initialized with the current parameter set.
* @return   SUCCESS or a CS_MAP error code.
* @bsimethod                                    Barry.Bentley                   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         DefinitionComplete ();


/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Gets whether coordinate system is set to be editable.
* @return true if editable
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetCanEdit () const;

/*---------------------------------------------------------------------------------**//**
* Sets whether coordinate system can be editted.
* @return true if editable
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetCanEdit (bool value);


/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Gets the available Linear Units.
* @return   vector of strings of unit names.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/

BASEGEOCOORD_EXPORTED static T_WStringVector* GetLinearUnitNames ();
/*---------------------------------------------------------------------------------**//**
* Gets all available Units, linear and degree-based.
* @return   vector of strings of unit names.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_WStringVector* GetUnitNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the available Datum Names.
* @return   vector of strings of datum names.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_WStringVector* GetDatumNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the available Ellipsoid Names.
* @return   vector of strings of datum names.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static T_WStringVector* GetEllipsoidNames ();

/*---------------------------------------------------------------------------------**//**
* Gets the grid scale along a meridian of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the grid scale is to be computed.
* @return   The grid scale along the meridian at the position specified.
* @remarks This is sometimes called the 'h' scale in geo coordinate system literature.
* @remarks The scale along a meridian is equal to the scale along a parallel for conformal projections.
* @see #GetScaleAlongParallel, #GetGridScale
* @bsimethod                                                    Barry.Bentley   03/07
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
* @bsimethod                                                    Barry.Bentley   03/07
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
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetGridScale
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the convergence angle, in degrees, of the coordinate system at the specified longitude/latitude.
* @param    point           IN      The point at which the convergence angle is to be computed.
* @see #GetScaleAlongMeridian, #GetScaleAlongParallel
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetConvergenceAngle
(
GeoPointCR      point
) const;

/*---------------------------------------------------------------------------------**//**
* Computes distance (in the units of this GCS) and starting azimuthal angle (in degrees)
* from one geographic point to another.
* @param    distance    OUT     The distance, in units of this GCS, from startPoint to endPoint.
* @param    azimuth     OUT     The initial azimuth, in degrees clockwise from true north, needed to get from startPoint to endPoint.
* @param    startPoint  IN      The starting point.
* @param    endPoint    IN      The end point.
* @remarks  If either distance or azimuth is not needed, pass NULL.
* @bsimethod                                    Barry.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetDistance
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
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         GetCenterPoint
(
GeoPointR       centerPoint
) const;

/*---------------------------------------------------------------------------------**//**
* Compares this coordinate system with the argument and returns true if they have equivalent
*  projection, parameters, datum, ellipsoid, and modifiers.
* @param    compareTo IN     The BaseGCS to compare to.
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsEquivalent
(
BaseGCSCR        compareTo
) const;

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
* @bsimethod                                                    Barry.Bentley   06/07
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
* interpretation BaseGCS. The usefulness of the present method becames evident when we must
* reproject a gepospatial object expressed in a BaseGCS into another BaseGCS that has
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
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt GetMathematicalDomain(bvector<GeoPoint>&    shape) const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the BaseGCS.
* This format is only useful for dictionary management purposes.
* @param    GCSAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod                                                    Alain.Robert   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
WStringR GCSAsASC
) const;
#endif



/*---------------------------------------------------------------------------------**//**
* Compares the Datum of this coordinate system with the argument and returns true if they have equivalent
*  datum (including ellipsoid).
* @param    compareTo IN     The BaseGCS to compare to.
* @bsimethod                                                    Barry.Bentley   06/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGEllipsoidCode
(
bool            noSearch = false
) const;

/*__PUBLISH_SECTION_END__*/
// These Methods are related to the ability to have a local coordinate system that is
// related to the Cartesian coordinate system by the LocalTransformerP

/*---------------------------------------------------------------------------------**//**
* Sets the LocalTransformer object for the GCS.
* @param    transformer     IN  The object that transformer from local cartesian to GCS cartesian coordinates and vice versa.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetLocalTransformer (LocalTransformerP transformer);

/*---------------------------------------------------------------------------------**//**
* Gets the LocalTransformer object for the GCS.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED LocalTransformerP GetLocalTransformer () const;

/*---------------------------------------------------------------------------------**//**
* Converts from Cartesian Coordinates of the GCS to the Cartesian Coordinate system
*   used in the projection.
* @param    outInternalCartesian    OUT The coordinates used in the projection calculation.
* @param    inCartesian             IN  The cartesian system of the GCS.
* @note: This method is rarely needed in client code. The LatLontFromCartesian method uses it.
* @bsimethod                                    Barry.Bentley                   01/10
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
* @bsimethod                                    Barry.Bentley                   01/10
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
* @bsimethod                                    Barry.Bentley                   01/10
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
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              CartesianFromInternalCartesian2D
(
DPoint2dR       outCartesian,
DPoint2dCR      inInternalCartesian
) const;

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian coordinates of the input Longitude/Latitude/Elevation point.
* @param    outCartesian    OUT     The calculated cartesian coordinates.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromCartesian2D
(
GeoPoint2dR     outLatLong,         // <= latitude longitude in this GCS
DPoint2dCR      inCartesian         // => cartesian coordinates in this GCS
) const;

/*---------------------------------------------------------------------------------**//**
* Returns the scale factor needed to convert to the units of the coordinate system from
*      meters by multiplication.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            UnitsFromMeters
(
) const;

/*---------------------------------------------------------------------------------**//**
* Determines whether the input GeoPoints are within the useful range of the coordinate system.
* @param    points          IN  The points to test.
* @param    numPoints       IN  Number of points to test.
* @bsimethod                                    Barry.Bentley                   07/08
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
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED RangeTestResult   CheckCartesianRange
(
DPoint3dCR       points,
int             numPoints
) const;

/*__PUBLISH_SECTION_END__*/

BASEGEOCOORD_EXPORTED static DgnProjectionTypes     DgnProjectionTypeFromCSDefName (CharCP projectionKeyName);

BASEGEOCOORD_EXPORTED static DgnProjectionTypes     DgnProjectionTypeFromCSMapProjectionCode (BaseGCS::ProjectionCodeValue projectionCode);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumConverterP   SetDatumConverter
(
BaseGCSCR destGCS
);

/*---------------------------------------------------------------------------------**//**
* Returns the library from which this GCS originated.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED LibraryP          GetSourceLibrary () const;


/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Sets whether reprojections from this coordinate system sadjust elevations.
* @return  The previous ReprojectElevation setting.
* @remarks The ReprojectElevation setting affects coordinate reprojections performed by the
* #LatLongFromLatLong method. If ReprojectElevation is false, the elevation value is
* unchanged.
* @remarks The default value is false.
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              SetReprojectElevation (bool value);

/*---------------------------------------------------------------------------------**//**
* Gets whether reprojections from this coordinate system adjust elevations.
* @return  The ReprojectElevation setting.
* @remarks The ReprojectElevation setting affects coordinate reprojections performed by the
* #LatLongFromLatLong method. If ReprojectElevation is false, the elevation value is unchanged.
* @remarks The default value is false.
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetReprojectElevation () const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude in the destination GCS, applying the appropriate datum shift.
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of destGCS.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @param    destGCS         IN      The Coordinate System corresponding to outLatLong.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromLatLong
(
GeoPointR       outLatLong,
GeoPointCR      inLatLong,
BaseGCSCR       destGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude in the destination GCS, applying the appropriate datum shift.
* @param    outLatLong      OUT     The calculated longitude,latitude in the datum of destGCS.
* @param    inLatLong       IN      The longitude,latitude in the datum of this GCS.
* @param    destGCS         IN      The Coordinate System corresponding to outLatLong.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromLatLong2D
(
GeoPoint2dR     outLatLong,
GeoPoint2dCR    inLatLong,
BaseGCSCR       destGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Converts from Degrees to Radians
* @param    inDegrees       IN      Angular value in degrees.
* @return   Angular value in radians
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static double     RadiansFromDegrees
(
double          inDegrees
);

/*---------------------------------------------------------------------------------**//**
* Converts from Radians to Degrees
* @param    inRadians       IN      Angular value in radians.
* @return   Angular value in degrees
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static double     DegreesFromRadians
(
double          inRadians
);

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude and latitude from ECEF coordinate.
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation.
* @param    inXYZ           IN      The XYZ (ECEF) coordinates of this GCS.
* @bsimethod                                                    Alain.Robert   2016/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   LatLongFromXYZ
(
GeoPointR       outLatLong,
DPoint3dCR      inXYZ
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the XYZ (ECEF) coordinates from the longitude, latitude and elevation.
* @param    outXYZ      OUT     The calculated XYZ (ECEF) coordinates.
* @param    inLatLong   IN      The latitude, longitude and elevation to convert
* @bsimethod                                                    Alain.Robert   2016/08
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

class GroupEnumerator;
class MemberEnumerator;
/*=================================================================================**//**
*
* Geographic coordinate systems Group class.
*
* The Group class represents Geographic Coordinate System Groups as defined in the CS_Map
* library. This grouping is not used by MicroStation, but may be useful by application code.
*
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
MPUBLIC class EXPORT_VTABLE_ATTRIBUTE Group
{
/*__PUBLISH_CLASS_VIRTUAL__*/
public:
/*---------------------------------------------------------------------------------**//**
* Gets the name of the GCS Group
* @return   The name of the GCS Group
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP         GetName();

/*---------------------------------------------------------------------------------**//**
* Gets the description of the GCS Group
* @return   The description of the Coordinate System.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP         GetDescription();

/*---------------------------------------------------------------------------------**//**
* Gets enumerator for members of this group
* @return   The enumerator.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED MemberEnumerator*     GetMemberEnumerator();

/*---------------------------------------------------------------------------------**//**
* Gets enumerator for Groups in the library
* @return   The enumerator.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static GroupEnumerator*   GetGroupEnumerator ();

/*---------------------------------------------------------------------------------**//**
* Free this Group. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

/*__PUBLISH_SECTION_END__*/
/// @cond NODOC
friend class GroupEnumerator;
/// @endcond
private:
    WString     m_name;
    WString     m_description;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
Group (WStringCR name, WStringCR description)
    {
    m_name = name;
    m_description = description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
~Group ();


/*__PUBLISH_SECTION_START__*/
};

/*=================================================================================**//**
* Group Enumerator class.
* @ingroup GeoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
MPUBLIC class EXPORT_VTABLE_ATTRIBUTE GroupEnumerator
{
/*__PUBLISH_CLASS_VIRTUAL__*/
public:
/*---------------------------------------------------------------------------------**//**
* Moves to the next group
* @return   true if successful in moving to the next Group, false if there are no more.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool      MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Group.
* @return   the current GCS Group.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Group*    GetCurrent();

/*---------------------------------------------------------------------------------**//**
* Free this GroupEnumerator. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void      Destroy () const;

/*__PUBLISH_SECTION_END__*/
/// @cond NODOC
friend class Group;
/// @endcond
private:
    int     m_currentIndex;
    Group*  m_currentGroup;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
GroupEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
~GroupEnumerator ();

/*__PUBLISH_SECTION_START__*/
};

/*=================================================================================**//**
* Member Enumerator class.
+===============+===============+===============+===============+===============+======*/
MPUBLIC class EXPORT_VTABLE_ATTRIBUTE MemberEnumerator
{
/*__PUBLISH_CLASS_VIRTUAL__*/
public:
/*---------------------------------------------------------------------------------**//**
* Moves to the GCS group member
* @return   true if successful in moving to the next GCS, false if there are no more.
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool      MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current GCS Name.
* @return   the current member GCS name
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP   GetCurrentGCSName();

/*---------------------------------------------------------------------------------**//**
* Gets the current GCS Description.
* @return   the current member GCS description
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP   GetCurrentGCSDescription();

/*---------------------------------------------------------------------------------**//**
* Free this MemberEnumerator. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void      Destroy () const;

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MemberEnumerator (WCharCP groupName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
~MemberEnumerator ();

private:
    int         m_currentIndex;
    WString     m_currentGCSName;
    WString     m_currentGCSDescription;
    WString     m_groupName;


/*__PUBLISH_SECTION_START__*/
};
/*__PUBLISH_SECTION_END__*/

} // ends GeoCoordinates namespace

END_BENTLEY_NAMESPACE

/*=================================================================================**//**
*
* This class has only static methods that map directly to CSMap functions. With the exception
*  of the Initialize method, most have the CS-map name.
*
+===============+===============+===============+===============+===============+======*/
/*__PUBLISH_SECTION_START__*/
typedef struct cs_Csdef_    CSDefinition;
typedef struct cs_Datum_    CSDatum;
typedef struct cs_Dtdef_    CSDatumDef;
typedef struct cs_Eldef_    CSEllipsoidDef;
typedef struct cs_Csgrplst_ CSGroupList;
typedef struct cs_Dtcprm_   CSDatumConvert;
typedef struct cs_GxXform_  CSGeodeticTransform;
typedef struct cs_Prjprm_   CSParamInfo;
typedef struct cs_Unittab_  CSUnitInfo;
typedef struct cs_Mgrs_     CSMilitaryGrid;
/*__PUBLISH_SECTION_END__*/

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
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static int              CS_csGrpEnum (int index,char *grp_name,int name_sz,char *grp_dscr,int dscr_sz);

BASEGEOCOORD_EXPORTED static int              CS_csEnum (int index, char *key_name,int name_sz);

BASEGEOCOORD_EXPORTED static int              CS_csEnumByGroup (int index, const char *groupName, CSGroupList *cs_descr);

BASEGEOCOORD_EXPORTED static CSParameters*    CScsloc2 (CSDefinition*, CSDatumDef*, CSEllipsoidDef*);

BASEGEOCOORD_EXPORTED static CSParameters*    CScsloc1 (CSDefinition*);

BASEGEOCOORD_EXPORTED static char*            CS_stncp (char*, CharCP   , int);

BASEGEOCOORD_EXPORTED static void             CS_errmsg (char*, int);

BASEGEOCOORD_EXPORTED static int              CS_cs3ll (const CSParameters*, GeoPointP ll, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static int              CS_ll3cs (const CSParameters*, DPoint3dP xy, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_dtcvt3D (CSDatumConvert*, GeoPointCP ll_in, GeoPointP ll_out);

BASEGEOCOORD_EXPORTED static int              CS_cs2ll (const CSParameters*, GeoPointP ll, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static int              CS_ll2cs (const CSParameters*, DPoint3dP xy, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_dtcvt (CSDatumConvert*, GeoPointCP ll_in, GeoPointP ll_out);

BASEGEOCOORD_EXPORTED static CSDatumConvert*  CS_dtcsu (const CSParameters*, const CSParameters*);

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

BASEGEOCOORD_EXPORTED static void             CS_free (void *mem);

BASEGEOCOORD_EXPORTED static int              CS_dtEnum (int index, char *key_name, int name_sz);

BASEGEOCOORD_EXPORTED static int              CS_elEnum (int index, char *key_name, int name_sz);

BASEGEOCOORD_EXPORTED static CSDatum*         CS_dtloc (char *key_name);

BASEGEOCOORD_EXPORTED static CSUnitInfo const* GetCSUnitInfo (int unitIndex);

BASEGEOCOORD_EXPORTED static double           CS_llazdd (double eRad, double eSq, GeoPointCP startPoint, GeoPointCP endPoint, double *distance);

BASEGEOCOORD_EXPORTED static int              CS_llchk (const CSParameters*, int numPoints, GeoPointCP ll);

BASEGEOCOORD_EXPORTED static int              CS_xychk (const CSParameters*, int numPoints, DPoint3dCP xy);

BASEGEOCOORD_EXPORTED static CSDatumConvert*  CSdtcsu (const CSDatum* src, const CSDatum* dest);

BASEGEOCOORD_EXPORTED static CSDatum*         CSdtloc1 (const CSDatumDef* datumDef);

BASEGEOCOORD_EXPORTED static CSMilitaryGrid*  CSnewMgrs (double e_rad, double e_sq, short bessel);

BASEGEOCOORD_EXPORTED static int              CScalcMgrsFromLl (CSMilitaryGrid* mg, char* result, int size, GeoPoint2dP ll, int precision);

BASEGEOCOORD_EXPORTED static int              CScalcLlFromMgrs (CSMilitaryGrid* mg, GeoPoint2dP ll, const char* mgrsString);

BASEGEOCOORD_EXPORTED static void             CSdeleteMgrs (CSMilitaryGrid* mg);

BASEGEOCOORD_EXPORTED static void             CS_llhToXyz (DPoint3dP xyz, GeoPointCP llh, double e_rad, double e_sq);

BASEGEOCOORD_EXPORTED static int              CS_xyzToLlh (GeoPointP llh , DPoint3dCP xyz, double e_rad, double e_sq);

};


typedef int (*DatumConvert3dFunc) (CSDatumConvert*, GeoPointCP in, GeoPointP out);

class   Datum;
typedef class Datum const&        DatumCR;

struct  VerticalDatumConverter;

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
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverter
(
CSDatumConvert*             datumConvert,
VerticalDatumConverter*     verticalDatumConverter
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
~DatumConverter();

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    DatumConverterP        Create
(
BaseGCSCR        from,
BaseGCSCR        to
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static    DatumConverterP        Create
(
DatumCR         from,
DatumCR         to
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   ConvertLatLong3D
(
GeoPointR       outLatLong,
GeoPointCR      inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   ConvertLatLong2D
(
GeoPoint2dR     outLatLong,
GeoPoint2dCR    inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              SetReprojectElevation   // <= returns old value.
(
bool            reprojectElevation
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              GetReprojectElevation
(
) const;

/*---------------------------------------------------------------------------------**//**
* Free this DatumConverter. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};


/*=================================================================================**//**
* Local Transformer abstract class.
+===============+===============+===============+===============+===============+======*/
class   LocalTransformer : public RefCountedBase
{
protected:
// make sure that nobody else can create one of these.
LocalTransformer();
virtual ~LocalTransformer();

public:
/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InternalCartesianFromCartesian (DPoint3dR outInternalCartesian, DPoint3dCR inCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    CartesianFromInternalCartesian (DPoint3dR outCartesian, DPoint3dCR inInternalCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InternalCartesianFromCartesian2D (DPoint2dR outInternalCartesian, DPoint2dCR inCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    CartesianFromInternalCartesian2D (DPoint2dR outCartesian, DPoint2dCR inInternalCartesian) const = 0;

/*---------------------------------------------------------------------------------**//**
* Save local transform parameters to memory.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    SaveParameters (uint16_t& transformType, double parameters[12]) const = 0;

/*---------------------------------------------------------------------------------**//**
* Read local transform parameters from memory.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    ReadParameters (double parameters[12]) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    IsEquivalent (LocalTransformerCP other) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    GetDescription (WString& description) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual LocalTransformerP   Copy () const = 0;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static LocalTransformerP  CreateLocalTransformer (LocalTransformType transformType, double parameters[12]);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static bool               IsEquivalent (LocalTransformerPtr const& transformer1, LocalTransformerPtr const& transformer2);

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
* Constructs a linear transformer that uses performs the
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer  (double a, double b, double c, double d, double e);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer ();


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static HelmertLocalTransformer*   Create (double a, double b, double c, double d, double e);

/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  InternalCartesianFromCartesian (DPoint3dR outInternalCartesian, DPoint3dCR inCartesian) const override;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  CartesianFromInternalCartesian (DPoint3dR outCartesian, DPoint3dCR inInternalCartesian) const override;

/*---------------------------------------------------------------------------------**//**
* Calculates the InternalCartesian coordinates (i.e., the cartesian coordinates that
*  are appropriate for the geographic projection calculations) from the Cartesian
*  coordinates.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  InternalCartesianFromCartesian2D (DPoint2dR outInternalCartesian, DPoint2dCR inCartesian) const override;

/*---------------------------------------------------------------------------------**//**
* Calculates the Cartesian coordinates from the InternalCartesian coordinates (i.e.,
*  the cartesian coordinates that are appropriate for the geographic projection)
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  CartesianFromInternalCartesian2D (DPoint2dR outCartesian, DPoint2dCR inInternalCartesian) const override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  GetDescription (WString& description) const override;

/*---------------------------------------------------------------------------------**//**
* Save local transform parameters to memory.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void  SaveParameters (uint16_t& transformType, double parameters[12]) const override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual bool  IsEquivalent (LocalTransformerCP other) const override;

/*---------------------------------------------------------------------------------**//**
* Read local transform parameters from memory.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual void    ReadParameters (double parameters[12]) override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED virtual LocalTransformerP   Copy () const override;

BASEGEOCOORD_EXPORTED double    GetA () const;
BASEGEOCOORD_EXPORTED double    GetB () const;
BASEGEOCOORD_EXPORTED double    GetC () const;
BASEGEOCOORD_EXPORTED double    GetD () const;
BASEGEOCOORD_EXPORTED double    GetE () const;

BASEGEOCOORD_EXPORTED void      SetA (double val);
BASEGEOCOORD_EXPORTED void      SetB (double val);
BASEGEOCOORD_EXPORTED void      SetC (double val);
BASEGEOCOORD_EXPORTED void      SetD (double val);
BASEGEOCOORD_EXPORTED void      SetE (double val);


BASEGEOCOORD_EXPORTED void      GetInternalCartesianFromCartesianTransform (TransformR transform);
BASEGEOCOORD_EXPORTED void      GetCartesianFromInternalCartesianTransform (TransformR transform);

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
mutable WStringP            m_nameString;
mutable WStringP            m_pluralNameString;
mutable WStringP            m_abbreviationString;

// constructor is private. Use FindUnit method
Unit (int index);
~Unit ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
public:
BASEGEOCOORD_EXPORTED static Unit const* FindUnit (WCharCP unitName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP       GetName() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP       GetPluralName() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP       GetAbbreviation() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoUnitSystem GetSystem() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoUnitBase   GetBase() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int           GetEPSGCode() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double        GetConversionFactor() const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
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
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   UnitEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   ~UnitEnumerator ();

/*---------------------------------------------------------------------------------**//**
* Moves to the next unit
* @return   true if successful in moving to the next Unit, false if there are no more.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool          MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Unit.
* @return   the current Unit.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Unit const*        GetCurrent();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void          Destroy() const;

};

typedef class Unit const*       UnitCP;

typedef class Ellipsoid*                EllipsoidP;
/*__PUBLISH_SECTION_START__*/

typedef class EllipsoidEnumerator*      EllipsoidEnumeratorP;
typedef class Ellipsoid const*          EllipsoidCP;
typedef class Ellipsoid const&          EllipsoidCR;

/*=================================================================================**//**
Definition of the globe as elliptical distortion of a sphere.
* @ingroup GeoCoordinate
* @bsiclass                                                     Barry.Bentley   03/08
+===============+===============+===============+===============+===============+======*/
class   Ellipsoid
{
/*__PUBLISH_SECTION_END__*/
private:
int32_t                 m_csError;
CSEllipsoidDef         *m_ellipsoidDef;
LibraryP                m_sourceLibrary;
mutable WStringP        m_nameString;
mutable WStringP        m_descriptionString;


/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
                                        Ellipsoid (WCharCP keyName, LibraryP sourceLibrary);

/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor - internal use only
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
                                        Ellipsoid (EllipsoidCR source);

/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor - internal use only
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
                                        Ellipsoid (const CSEllipsoidDef &ellipsoidDef, LibraryP sourceLibrary);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
                                        ~Ellipsoid();

public:

static EllipsoidCP CreateEllipsoid (EllipsoidCR source);
static EllipsoidCP CreateEllipsoid (WCharCP keyName, LibraryP sourceLibrary);
BASEGEOCOORD_EXPORTED static EllipsoidCP CreateEllipsoid (CSEllipsoidDef const& ellipsoidDef, LibraryP sourceLibrary);


/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
public:
/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Ellipsoid class
* @return Initialized Ellipsoid.
* @remarks If keyName is in the Ellipsoid library and valid, the return Ellipsoid's IsValid() returns true.
*          Otherwise, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static EllipsoidCP CreateEllipsoid (WCharCP keyName);

/*---------------------------------------------------------------------------------**//**
* Returns whether the Ellipsoid is valid or not.
* @return   True if the Ellipsoid is valid, False otherwise. @see #GetError
* @remarks  If the Ellipsoid does not correspond to a Ellipsoid in the Ellipsoid library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetErrorMessage (WStringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Ellipsoid
* @return   The name of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the key name of the Ellipsoid.
* @return   BSISUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (WCharCP value);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid.
* @return   The description of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Ellipsoid
* @return   The source of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetSource (WStringR source) const;

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Ellipsoid.
* @return   The polar radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Ellipsoid, if known.
* @return   The EPSG code.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode () const;

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Returns true if the Ellipsoid name does not exist in the source library or the system library.
* @param  inSystemLibrary IN Set to true if the method returns true and name collision is with the system library.
* @remarks It is only valid to call this after an ellipsoid has been created and before it has been added to the library.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              NameUnique (bool& inSystemLibrary) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidP        Clone () const;

/*---------------------------------------------------------------------------------**//**
* Replaces this Ellipsoid in the library.
* @remarks This will return BSISUCCESS only if the ellipsoid has its sourceLibrary member set.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         ReplaceInLibrary (EllipsoidP newEllipsoid) const;

/*---------------------------------------------------------------------------------**//**
* Returns the library from which this Ellipsoid originated.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED LibraryP          GetSourceLibrary () const;

/*---------------------------------------------------------------------------------**//**
* Adds this ellipsoid to the library.
* @remarks This will return BSISUCCESS only if the ellipsoid has its sourceLibrary member set.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         AddToLibrary () const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Ellipsoid
* @param description IN     The Ellipsoid description
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (WCharCP description);

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Ellipsoid
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (WCharCP source);

/*---------------------------------------------------------------------------------**//**
* Sets the polar radius of the Ellipsoid.
* @param value IN   The polar radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetPolarRadius (double value);

/*---------------------------------------------------------------------------------**//**
* Sets the equatorial radius of the Ellipsoid.
* @param value IN   The equatorial radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              SetEquatorialRadius (double value);

/*---------------------------------------------------------------------------------**//**
* Sets flattening and eccentricity
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static bool       CalculateParameters (double& flattening, double& eccentricity, double equatorialRadius, double polarRadius);

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Free this Group. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the Ellipsoid.
* This format is only useful for dictionary management purposes.
* @param    EllipsoidAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod                                                    Alain.Robert   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
WStringR EllipsoidAsASC
) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Factory method to create an EllipsoidEnumerator
* @return  An initialized EllipsoidEnumerator
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static EllipsoidEnumeratorP CreateEnumerator();

};

/*=================================================================================**//**
* Ellipsoid enumeration class.
* @ingroup GeoCoordinate
+===============+===============+===============+===============+===============+======*/
class   EllipsoidEnumerator
{
/*__PUBLISH_SECTION_END__*/
private:
int     m_currentIndex;
WString m_currentEllipsoidName;
friend class Ellipsoid;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
~EllipsoidEnumerator ();


/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
public:
/*---------------------------------------------------------------------------------**//**
* Moves to the next Ellipsoid
* @return   true if successful in moving to the next Ellipsoid, false if there are no more.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Ellipsoid.
* @return   the current Ellipsoid.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP       GetCurrent();

/*---------------------------------------------------------------------------------**//**
* Free this EllipsoidEnumerator. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};



typedef class DatumEnumerator*    DatumEnumeratorP;
typedef class Datum const*        DatumCP;
typedef class Datum*              DatumP;

/*=================================================================================**//**
Position and orientation relative to a WGS84 Datum
* @ingroup GeoCoordinate
* @bsiclass                                                     Barry.Bentley   03/08
+===============+===============+===============+===============+===============+======*/
class   Datum
{
/*__PUBLISH_SECTION_END__*/
private:
int32_t                     m_csError;
CSDatumDef*                 m_datumDef;
mutable CSDatum*            m_csDatum;    // this is created only if needed.

LibraryP                    m_sourceLibrary;
mutable Ellipsoid const*    m_ellipsoid;  // this is created only if needed.

mutable WStringP            m_nameString;                   // these are here as "adapters" needed because CS_Map uses all char rather than unicode.
mutable WStringP            m_descriptionString;
mutable WStringP            m_ellipsoidNameString;
mutable WStringP            m_ellipsoidDescriptionString;

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class. Looks only in the system library.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (WCharCP keyName);

/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (CSDatumDef const& datum, LibraryP sourceLibrary);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
~Datum();


/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
public:
/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @return Initialized Datum.
* @remarks If keyName is in the Datum library and valid, the return Datum's IsValid() returns true.
*          Otherwise, the GetError() or GetErrorMessage() methods return the error.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatum (WCharCP keyName);

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Initializes a new instance of the Datum class
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumCP    CreateDatum (CSDatumDef const& datumDef, LibraryP sourceLibrary);

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is valid or not.
* @return   True if the Datum is valid, False otherwise. @see #GetError
* @remarks  If the datum does not correspond to a datum in the datum library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              IsValid() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetError() const;

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetErrorMessage (WStringR errorMsg) const;

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Datum
* @return   The name of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetName() const;

/*---------------------------------------------------------------------------------**//**
* Sets the key name of the Datum.
* @return   BSISUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetName (WCharCP value);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Datum.
* @return   The description of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetDescription() const;

/*---------------------------------------------------------------------------------**//**
* Sets the description of the Datum.
* @return   BSISUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDescription (WCharCP value);

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Datum
* @return   The source of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetSource (WStringR source) const;

/*---------------------------------------------------------------------------------**//**
* Sets the source of the Datum
* @return   BSISUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetSource (WCharCP value);

/*---------------------------------------------------------------------------------**//**
* Gets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   The convert method.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WGS84ConvertCode  GetConvertToWGS84MethodCode() const;

/*---------------------------------------------------------------------------------**//**
* Sets the method used to convert longitude/latitude from this Datum to the WGS84 datum.
* @return   BSISUCCESS or an error code.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetConvertToWGS84MethodCode (WGS84ConvertCode value);

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta OUT  The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetDelta (DPoint3dR delta) const;

/*---------------------------------------------------------------------------------**//**
* Sets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @param    delta IN    The vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the delta settings.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetDelta (DPoint3dCR delta);

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation OUT The rotation angles.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              GetRotation (DPoint3dR rotation) const;

/*---------------------------------------------------------------------------------**//**
* Sets the angles from the WGS84 x, y, and z axes to those of this Datum
* @param    rotation IN The rotation angles.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the rotation settings.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetRotation (DPoint3dCR rotation) ;

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scaling in parts per million if known.
* @return   The datum transformation scale in ppm.
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetScale () const;

/*---------------------------------------------------------------------------------**//**
* Sets the datum transformation scaling in parts per million if known.
* @param scalePPM   IN The scale in parts per million.
* @return  GEOCOORDERR_ParameterNotUsed if the datum ConvertToWGS84Method does not use the scale setting.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetScale (double scalePPM);

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Datum, if known.
* @return   The EPSG code.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEPSGCode () const;

/*---------------------------------------------------------------------------------**//**
* Gets the key name of the Ellipsoid used in this Datum
* @return   The key name Ellipsoid of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidName() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Ellipsoid used in this Datum
* @return   Index of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int               GetEllipsoidCode() const;

/*---------------------------------------------------------------------------------**//**
* Gets the internal index of the Ellipsoid used in this Datum
* @return   Index of the Coordinate System Ellipsoid.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         SetEllipsoidCode (int ellipsoidCode);

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid used in this Datum
* @return   The description of the Ellipsoid of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidDescription() const;

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Ellipsoid used in this Datum
* @return   The source of the Ellipsoid of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED WCharCP           GetEllipsoidSource (WStringR ellipsoidSource) const;

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Coordinate System.
* @return   The polar radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidPolarRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEquatorialRadius() const;

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double            GetEllipsoidEccentricity() const;

/*---------------------------------------------------------------------------------**//**
* Gets the Ellipsoid used in this Datum
* @return   The Ellipsoid of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED EllipsoidCP       GetEllipsoid() const;

/*---------------------------------------------------------------------------------**//**
* Returns whether the delta, rotation, and scale parameters are valid for this Datum.
* @param  deltaValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the delta parameters are used.
* @param  rotationValid OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the rotation parameters are used.
* @param  scaleValid    OUT Returns true if the datum is valid and its WGS84ConvertCode indicates that the scale parameter is used.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              ParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const;


/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Gets the CSDatum
* @bsimethod                                                    Barry.Bentley   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED CSDatum*          GetCSDatum() const;

/*---------------------------------------------------------------------------------**//**
* Returns true if the Datum name does not exist in the source library or the system library.
* @param  inSystemLibrary IN Set to true if the method returns true and name collision is with the system library.
* @remarks It is only valid to call this after a datum has been created and before it has been added to the library.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              NameUnique (bool& inSystemLibrary) const;

/*---------------------------------------------------------------------------------**//**
* Returns the library from which this Datum originated.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED LibraryP          GetSourceLibrary () const;

/*---------------------------------------------------------------------------------**//**
* Adds this datum to the library.
* @remarks This will return BSISUCCESS only if the datum has its sourceLibrary member set.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         AddToLibrary () const;

/*---------------------------------------------------------------------------------**//**
* Replaces this datum in the library.
* @remarks This will return BSISUCCESS only if the datum has its sourceLibrary member set.
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         ReplaceInLibrary (DatumP newDatum) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumP            Clone () const;

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
* Free this Datum. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* Creates a string that contains the CSMAP ASC format text definition of the datum.
* This format is only useful for dictionary management purposes.
* @param    DatumAsASC OUT Reference to string that receives the text ASC desctiption of GCS
* @bsimethod                                                    Alain.Robert   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt         OutputAsASC
(
WStringR            DatumAsASC
) const;
#endif

/*---------------------------------------------------------------------------------**//**
* Factory method to create a DatumEnumerator
* @return  An initialized DatumEnumerator
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static DatumEnumeratorP CreateEnumerator();

};

/*=================================================================================**//**
* Datum enumeration class.
* @ingroup GeoCoordinate
+===============+===============+===============+===============+===============+======*/
class   DatumEnumerator
{
/*__PUBLISH_SECTION_END__*/
private:
int     m_currentIndex;
WString m_currentDatumName;
friend  class Datum;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumEnumerator ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
~DatumEnumerator ();

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
public:
/*---------------------------------------------------------------------------------**//**
* Moves to the next Datum
* @return   true if successful in moving to the next Datum, false if there are no more.
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool              MoveNext();

/*---------------------------------------------------------------------------------**//**
* Gets the current Datum.
* @return   the current Datum.
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED DatumCP           GetCurrent();

/*---------------------------------------------------------------------------------**//**
* Free this DatumEnumerator. 
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void              Destroy() const;

};

/*__PUBLISH_SECTION_END__*/

/*=================================================================================**//**
* Provides localizaion services to DgnGeoCoord.
+===============+===============+===============+===============+===============+======*/
class   BaseGeoCoordResource
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static WCharCP    GetLocalizedProjectionName
(
WStringR                        outString,
BaseGCS::ProjectionCodeValue    projectionCode
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static WCharCP    GetLocalizedProjectionName
(
WStringR                        outString,
DgnProjectionTypes              projectionType
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static WCharCP    GetLocalizedString
(
WStringR                outString,
DgnGeoCoordStrings      stringNum
);

};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   06/10
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
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED static MilitaryGridConverterPtr CreateConverter (BaseGCSR baseGCS, bool useBessel, bool useWGS84Datum);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt   LatLongFromMilitaryGrid (GeoPoint2dR latLong, WCharCP mgString);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt   MilitaryGridFromLatLong (WString& mgString, GeoPoint2dCR latLong, int precision);
};

/*=================================================================================**//**
* Geographic Coordinate System selector class. This class provides a user interface for
* selecting a GCS from the available GCS library or libraries. The user interface control is
* implemented as a .net WinForm, and using this class, unlike all others in this header file,
* utilizes Bentley.GeoCoord.dll and a number of supporting .net assemblies. It requires the
* calling program to link with Bentley.GeoCoord.lib. The host program must therefore be
* prepared for the fact that the .net runtime will be utilized when this class is used.
+===============+===============+===============+===============+===============+======*/
#if defined (__BASEMANAGEDGCS_BUILD__)
#pragma managed
#endif
class   GCSSelectFromLibrary
{
public:
/*---------------------------------------------------------------------------------**//**
* Opens the modal WinForms-based Geographic Coordinate System Selector dialog.
* @param    initialGCS  IN  The GCS that should be selected when the dialog opens, or NULL.
* @param    dialogTitle IN  The title of the modal selector dialog.
* @param    favoritesFileList   IN  A semi-colon separated string that contains the names of the favorites files.
*           This is stored in the configuratoin variable MS_MS_GEOCOORDINATE_FAVORITESFILES in MicroStation, but
*           this method cannot depend on any MicroStation capability.
* @return   the selected BaseGCSP, or NULL if the user Cancelled the selection.
* @remark   The calling program must link with Bentley.Geocoord.lib, and be aware that the CLR will be started (if not already)
*           When this method is called, and it will have to be able to locate the Bentley.Geocoord.dll assembly and associated assemblies.
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEMANAGEDGCS_EXPORTED static BaseGCSP   OpenSelectorDialog (BaseGCSP initialGCS, WCharCP dialogTitle, WCharCP favoritesFileList);
};
#if defined (__BASEMANAGEDGCS_BUILD__)
#pragma unmanaged
#endif

/*__PUBLISH_SECTION_START__*/

} // ends GeoCoordinates namespace
END_BENTLEY_NAMESPACE
