/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/vplist.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/ExportMacros.h>

typedef struct vpList VPList;
struct vpList
    {
    int             count;          /* # items added to list so far */
    void          **p;              /* storage */
    int             nAllocated;     /* # item slots allocated so far */
    };

BEGIN_EXTERN_C

DGNPLATFORM_EXPORT VPList *vpList_define (VPList*, int chunkSize);
DGNPLATFORM_EXPORT VPList *vpList_new (void);
DGNPLATFORM_EXPORT VPList *vpList_newCopy (VPList*);
DGNPLATFORM_EXPORT int vpList_growBy (VPList*, int);
DGNPLATFORM_EXPORT VPList *vpList_empty (VPList *);
DGNPLATFORM_EXPORT void vpList_destroy (VPList*);
DGNPLATFORM_EXPORT void *vpList_free (VPList *);
DGNPLATFORM_EXPORT int vpList_n (const VPList*);
DGNPLATFORM_EXPORT void vpList_put (VPList*, void*, int);
DGNPLATFORM_EXPORT int vpList_add (VPList *, void *);
DGNPLATFORM_EXPORT StatusInt vpList_drop (VPList *, const void *);
DGNPLATFORM_EXPORT void vpList_dropItems (VPList*, int i, int n);
DGNPLATFORM_EXPORT void *vpList_get (const VPList *, int);
DGNPLATFORM_EXPORT void *vpList_first (const VPList*);
DGNPLATFORM_EXPORT int vpList_find (const VPList*, const void*);
DGNPLATFORM_EXPORT bool vpList_isFound (const VPList*, const void *);
DGNPLATFORM_EXPORT VPList *vpList_reverse (VPList*);
DGNPLATFORM_EXPORT VPList *vpList_copy (VPList*, const VPList*);
DGNPLATFORM_EXPORT VPList *vpList_dropList (VPList*, const VPList*);
DGNPLATFORM_EXPORT VPList *vpList_appendListUnique (VPList*, const VPList*);

END_EXTERN_C

