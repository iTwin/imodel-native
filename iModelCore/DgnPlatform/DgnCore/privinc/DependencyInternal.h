/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/privinc/DependencyInternal.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnCore/DependencyManagerLinkage.h>

///////////////////////////////////////////////////////////////////////////////
/////// Callbacks from file I/O ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Only DgnFile can do things like load a PersistentElementRefP, move a PersistentElementRefP or load a cache. 
// That is why these notification functions are private to graphics, rather than methods on DependencyManager.
void dependency_elementLoaded (ElementRefP, DgnElementCP);
void  dependency_replaceRefWithRef (ElementRefP newRef, ElementRefP oldRef);
void dependency_cacheUnloading (DgnModelP);
void dependency_setInModelLoading (bool inCacheLoading);

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void            dependency_setIgnoreCacheLoaded (bool ignored);

void            dependency_dumpRootDgnModels ();

