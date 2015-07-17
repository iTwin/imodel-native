/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/base/bcMem.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define bcMem_malloc malloc
#define bcMem_free free
#define bcMem_realloc realloc
#define bcMem_calloc calloc
template<class c> void bcMem_freeAndClear (c **a) { if (a != 0 && *a != 0) { free (*a); *a = 0; } }
