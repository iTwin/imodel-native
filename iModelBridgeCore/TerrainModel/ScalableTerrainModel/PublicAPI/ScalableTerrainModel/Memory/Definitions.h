/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Memory/Definitions.h $
|    $RCSfile: Definitions.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/19 13:51:11 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>

#ifdef MEMORY_DLLE
    #error "Export name conflict with another definition of the same name"
#endif

#ifdef __BENTLEYSTM_BUILD__ //BENTLEY_MRDTM_MEMORY_EXPORTS
    #define MEMORY_DLLE __declspec(dllexport)
#else
    #define MEMORY_DLLE __declspec(dllimport)
#endif


#ifndef BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE
    #define BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE namespace Bentley { namespace MrDTM { namespace Memory {
    #define END_BENTLEY_MRDTM_MEMORY_NAMESPACE   }}}
    #define USING_NAMESPACE_BENTLEY_MRDTM_MEMORY using namespace Bentley::MrDTM::Memory;

#endif //!BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE


BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE

// TDORAY: Restrict that to only some names
using namespace Foundations; // Make Memory synonymous to Foundations

END_BENTLEY_MRDTM_MEMORY_NAMESPACE


