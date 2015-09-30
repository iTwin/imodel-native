/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/GCSLibrary.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
// This is internal, because it includes csmap\cs_map.h. That file defines all kinds of macros (such as MAXINT) that conflict with Windows .h files.

#include "ExportMacros.h"

#include    <GeoCoord/BaseGeoCoord.h>
#include    <csmap/cs_map.h>
#include    <string>
#include    <Bentley/bmap.h>
#include    <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

#define     NUM_USER_DATUMS     200
#define     NUM_USER_ELLIPSOIDS 100

/*=================================================================================**//**
* Geographic Coordinate System library.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  Library
{
    virtual ~Library() {;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (WCharCP name) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (CSDefinition& csdef) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            CSInLibrary (WCharCP csName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         GetGUIName () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (uint32_t index) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteCS (BaseGCSP gcsToDelete) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewCS (WString& newName, WCharCP sourceName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceCS (BaseGCSP oldGCS, BaseGCSP newGCS) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetCSName (uint32_t index, WStringR csName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetCSCount () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP         GetLibraryFileName () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP         GetOrganizationFileName () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsUserLibrary () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsReadOnly () = 0;



// Datum methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (WCharCP datumName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (uint32_t index) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteDatum (WCharCP datumName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewDatum (DatumP& newDatum, WCharCP seedName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       AddDatum (const CSDatumDef& newDatumDef) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceDatum (const CSDatumDef& oldDatum, const CSDatumDef& newDatum) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetDatumName (uint32_t index, WStringR datumName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetDatumCount () = 0;


#ifdef NOT_YET
// Geodetic methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSGeodeticTransform*    GetGeodeticTransform (WCharCP geodeticTrasnformName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSGeodeticTransform*    GetGeodeticTransform (UInt32 index) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt               DeleteGeodeticTransform (WCharCP geodeticTransformName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
// virtual StatusInt               CreateNewGeodeticTransform (DatumP& newDatum, WCharCP seedName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt               AddGeodeticTransform (const CSGeodeticTransform& newGeodeticTransformDef) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt               ReplaceGeodeticTransform (const CSGeodeticTransform& oldGeodeticTransform, const CSGeodeticTransform& newGeodeticTransform) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt               GetGeodeticTransformName (UInt32 index, WStringR geodeticTransformName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t                  GetGeodeticTransformCount () = 0;

#endif // NOT_YET

// Ellipsoid methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (WCharCP ellipsoidName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (uint32_t index) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteEllipsoid (WCharCP datumName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewEllipsoid (CSEllipsoidDef*& newEllipsoid, WCharCP seedName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       AddEllipsoid (const CSEllipsoidDef& newEllipsoidDef) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceEllipsoid (const CSEllipsoidDef& oldEllipsoid, const CSEllipsoidDef& newEllipsoid) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetEllipsoidName (uint32_t index, WStringR ellipsoidName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetEllipsoidCount () = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            DatumInLibrary (WCharCP datumName) = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            EllipsoidInLibrary (WCharCP ellipsoidName) = 0;
};

/*=================================================================================**//**
*
* Library Manager class.
*
* This class keeps a Map between keyname and csParams for every coordinate system that we've
*  ever access through the cs_def function. I did that because some operations (searching
*  coordinate systems, and looking for EPSG equivalents) are done fairly often, might
*  require us to look through a large subset of the GCS defintions, and when we do them
*  once, we're likely to do them more than once. So I decided to trade memory for time and
*  keep this map. Note that we always copy cs_csPrm_ structure contents (which is
*  fortunately a flat structure, with no pointers out to other memory) rather than
*  giving out the pointer to cs_csPrm_ structure in our list, to avoid complicated
*  memory management issues.
*
+===============+===============+===============+===============+===============+======*/

struct ParamMapEntry : public CSParameters
    {
    Library*  m_sourceLibrary;
    };

typedef bmap <WString, ParamMapEntry*>      T_CSParamMap;
typedef bvector <Library*>                  T_UserLibraryList;

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning(disable:4251) //get rid of warning so we can use the vector class
#endif
struct  BASEGEOCOORD_EXPORTED   LibraryManager
{
private:
T_CSParamMap            m_csParamMap;
T_UserLibraryList*      m_userLibraryList;
Library*                m_csmapLibrary;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
static LibraryManager*    Instance ();

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSDatumDef that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSDatumDef*             GetDatumDefFromGCS (BaseGCSCR gcs);

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSDatumDef that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSEllipsoidDef*         GetEllipsoidDefFromGCS (BaseGCSCR gcs);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Library*                GetLibrary (size_t index);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                  GetLibraryCount ();

/*__PUBLISH_SECTION_END__*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    RemoveFromCache (WCharCP name);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    RemoveAllUsingDatumFromCache (CharCP datumName);        // WIP_CHAR_OK

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    RemoveAllUsingEllipsoidFromCache (CharCP datumName);    // WIP_CHAR_OK

/*---------------------------------------------------------------------------------**//**
* NOTE: CSParameters passed in must be allocated with CS_malc. If it succeeds in finding
*  the definition, it replaces the contents of existingParameters with the new parameters,
*  avoiding freeing and allocating the coordinate system.
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*           ReplaceCSContents (LibraryP& library, WCharCP name, CSParameters* existingCSParams);

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    AddUserLibrary (WCharCP libraryPath, WCharCP guiName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryP                GetSystemLibrary ();

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*           GetCS (LibraryP& outSourceLibrary, WCharCP name);

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*           GetCS (LibraryP& outSourceLibrary, LibraryP sourceLibrary, CSParameters& source);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryP                FindSourceLibrary (WCharCP csName);

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryManager    ();

static LibraryManager*     s_instance;

};
#ifdef _MSC_VER
#pragma warning (pop)
#endif

} // ends GeoCoordinates namespace

END_BENTLEY_NAMESPACE
