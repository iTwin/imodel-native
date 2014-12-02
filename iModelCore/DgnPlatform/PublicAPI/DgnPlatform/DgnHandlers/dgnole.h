/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/dgnole.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/OleCellHeaderHandler.h>

#define NO_IODEFS


#define STD_ACCESS  (STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT)
#define NOT_FILLED  0
#define FILLED      1
#define ELM_OLE_SIGNATURE   (45086)     /* Linkage signature "0xB01e" (Bentley Ole) */
#define ___     (0)                     /* place holder for dummy function arguments */
#define DGNOLE_VERSION  2

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#if defined (NEEDS_WORK_DGNITEM)

#pragma pack(1)
typedef struct dwgoleflags
    {
    byte        paperSpace:1;
    byte        unknown1:1;
    byte        notSelectable:1;
    byte        unknown2:1;
    byte        valueIs8:4;
    byte        valueIs55;
    } DwgOleFlags;

typedef struct dwgoleheader
    {
    DwgOleFlags         flags;                  // (+  0 bytes)
    DPoint3d            dwgOutlineShape[4];     // (+  2 bytes) Upper left clockwise points of outline shape,  DWG coordinate space
    HiMetricSizeShort   defaultSize;            // (+ 98 bytes) Object size in 0.01 mm units (i.e. /2540 for inches)
    Int32               inModelSpace;           // (+102 bytes) ?? 1 = model space, 0= paperspace
    Int32               _f2_const_0x100;        // (+106 bytes) ?? 4 bytes unknown, const 0x100
    Int32               objNumber;              // (+110 bytes) Zero based Object number (Ustn's object are 1 based)
    Int32               activateable;           // (+114 bytes) 1 = application known (
    Int16               insertedOrClipped;      // (+118 bytes) 0 = copied from clipboard, 1 = inserted
    Int32               dwAspect;               // (+120 bytes) OLE aspect enumeration
    UInt32              dwOleDataSize;          // (+124 bytes) Length in bytes of following binary stream
    } DwgOleHeader;
#pragma pack()
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
