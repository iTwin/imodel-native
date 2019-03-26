/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Definitions.h $
|    $RCSfile: Definitions.h,v $
|   $Revision: 1.13 $
|       $Date: 2011/10/20 18:48:20 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>
#include <ScalableTerrainModel/Memory/Definitions.h>
#include <ScalableTerrainModel/GeoCoords/Definitions.h>

// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h>
#include <vector>
#include <memory>
#include <typeinfo>

#ifdef IMPORT_DLLE
    #error "Export name conflict with another definition of the same name"
#endif

#ifdef __BENTLEYSTM_BUILD__ //BENTLEY_MRDTM_IMPORT_EXPORTS
    #define IMPORT_DLLE __declspec(dllexport)
#else
    #define IMPORT_DLLE __declspec(dllimport)
#endif


#ifndef BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
    #define BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE namespace Bentley { namespace MrDTM { namespace Import {
    #define END_BENTLEY_MRDTM_IMPORT_NAMESPACE   }}}
    #define USING_NAMESPACE_BENTLEY_MRDTM_IMPORT using namespace Bentley::MrDTM::Import;
#endif //!BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE



#define BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE namespace Plugin {
#define END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE END_BENTLEY_MRDTM_IMPORT_NAMESPACE }


#define BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(version) BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE namespace V ## version ## { 
#define END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE }

#define USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN using namespace Bentley::MrDTM::Import::Plugin;
#define USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(version) using namespace Bentley::MrDTM::Import::Plugin::V ## version ## ;



BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
using namespace GeoCoords; // Make synonymous to GeoCoords
using namespace Bentley::MrDTM::Import; // Make synonymous to Import
using namespace Bentley::MrDTM::Import::Plugin; // Make synonymous to Plugin
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE



BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

using namespace Foundations; // Make Import synonymous to Foundations
using namespace Memory; // Make Import synonymous to Memory
using namespace GeoCoords; // Make Import synonymous to GeoCoords


END_BENTLEY_MRDTM_IMPORT_NAMESPACE


/* 
 * Declare functions interfaces that are highly used in headers in order to avoid per header inclusions. 
 * Those will often be default parameters.
 */
BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

struct Log;

FOUNDATIONS_DLLE Log&               GetDefaultLog                      ();

END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE