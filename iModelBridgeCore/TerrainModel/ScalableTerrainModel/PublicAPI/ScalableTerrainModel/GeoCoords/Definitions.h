/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/GeoCoords/Definitions.h $
|    $RCSfile: Definitions.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/07 14:20:42 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>

#include <typeinfo>

#include <Geom/GeomApi.h>

#ifdef GEOCOORDS_DLLE
    #error "Export name conflict with another definition of the same name"
#endif

#ifdef __BENTLEYSTM_BUILD__ //BENTLEY_MRDTM_GEOCOODINATES_EXPORTS
    #define GEOCOORDS_DLLE __declspec(dllexport)
#else
    #define GEOCOORDS_DLLE __declspec(dllimport)
#endif


#ifndef BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
    #define BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE namespace Bentley { namespace MrDTM { namespace GeoCoords {
    #define END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE   }}}
    #define USING_NAMESPACE_BENTLEY_MRDTM_GEOCOORDINATES using namespace Bentley::MrDTM::GeoCoords;
#endif //!BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE


BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE

using namespace Foundations; // Make GeoCoords synonymous to Foundations

END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE


/* 
 * Declare functions and types that are highly used in headers in order to avoid per header inclusions or forward declarations. 
 * Those will often be default parameters.
 */
BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

struct Log;

FOUNDATIONS_DLLE Log&               GetDefaultLog                      ();

END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE