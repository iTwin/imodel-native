/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/papatch.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/

/*--------------------------------------------------------------------------------------+
|  included from w32start.cpp. This header contains the entire papatch implementation and
|  has no .cpp or .obj file. This behavior is grandfathered in from papatch.c, which was
|  directly included in w32start.c for good reasons that no one can remember, possibly
|  related to HeapZone. Rather than mess with something that works, we're continuing the
|  tradition.
+--------------------------------------------------------------------------------------*/
#ifdef __papatch_h__
#error Papatch was included twice
#else
#define __papatch_h__
#pragma once
#pragma intrinsic(memcpy)

#include <tchar.h>
#include <DgnPlatform/DgnPlatform.h>

#if defined (STANDALONE) || defined (LMSERVER)
#  ifdef  PAGALLOC_API
#    undef  PAGALLOC_API
#  endif
#  define PAGALLOC_API extern
#  include <pagalloc.fdf>
#  include <pagalloc.h>
#endif /* STANDALONE || LMSERVER */

/*----------------------------------------------------------------------+
|
|   Function Pointer typedefs
|
+----------------------------------------------------------------------*/
typedef void *(*CRuntimeMallocFunc)         (size_t);
typedef void *(*CRuntimeNhMallocFunc)       (size_t,size_t);
typedef void *(*CRuntimeCallocFunc)         (size_t,size_t);
typedef void *(*CRuntimeReallocFunc)        (void*,size_t);
typedef void  (*CRuntimeFreeFunc)           (void*);
typedef int32_t (*CRuntimeMsizeFunc)          (void*);
typedef void *(*CRuntimeExpandFunc)         (void*,size_t);
typedef void  (*PagallocSetFreeHFunc)       (CRuntimeFreeFunc);
typedef void  (*PagallocSetMsizeHFunc)      (CRuntimeMsizeFunc);
typedef void *(*PagallocOperatorNewFunc)    (size_t);
typedef void *(*PagallocOperatorDeleteFunc) (void*);
typedef void *(*PagallocHeapWalkFunc)       (void*);
typedef char *(*PagallocStrdupFunc)         (char const * const);       // WIP_CHAR_OK
typedef wchar_t *(*PagallocWcsdupFunc)      (wchar_t  const * const);

/*----------------------------------------------------------------------+
|
|   Data Structures
|
+----------------------------------------------------------------------*/
// Most functions only need a few dozen bytes but this is much easier than debugging the problems that show up when functions write over each other.
const static int32_t PATCHDATA_PATCHBYTES = 256;

static struct __patchData
    {
    Byte mallocCode[PATCHDATA_PATCHBYTES];
    Byte nhMallocCode[PATCHDATA_PATCHBYTES];
    Byte callocCode[PATCHDATA_PATCHBYTES];
    Byte reallocCode[PATCHDATA_PATCHBYTES];
    Byte freeCode[PATCHDATA_PATCHBYTES];
    Byte msizeCode[PATCHDATA_PATCHBYTES];
    Byte expandCode[PATCHDATA_PATCHBYTES];
    Byte operatorNewCode[PATCHDATA_PATCHBYTES];
    Byte operatorDeleteCode[PATCHDATA_PATCHBYTES];
    Byte heapWalkCode[PATCHDATA_PATCHBYTES];
    Byte strdupCode[PATCHDATA_PATCHBYTES];
    Byte wcsdupCode[PATCHDATA_PATCHBYTES];

    CRuntimeMallocFunc              callCrtMalloc;
    CRuntimeNhMallocFunc            callCrtNhMalloc;
    CRuntimeCallocFunc              callCrtCalloc;
    CRuntimeReallocFunc             callCrtRealloc;
    CRuntimeFreeFunc                callCrtFree;
    CRuntimeMsizeFunc               callCrtMsize;
    CRuntimeExpandFunc              callCrtExpand;
    PagallocOperatorNewFunc     callCrtOperatorNew;
    PagallocOperatorDeleteFunc  callCrtOperatorDelete;
    PagallocHeapWalkFunc            callCrtHeapWalk;
    PagallocStrdupFunc              callCrtStrdup;
    PagallocWcsdupFunc              callCrtWcsdup;
    } *patchData;


static Byte *crtMallocP;
static Byte *crtNhMallocP;
static Byte *crtCallocP;
static Byte *crtReallocP;
static Byte *crtFreeP;
static Byte *crtMsizeP;
static Byte *crtExpandP;
static Byte *crtOperatorNewP, *crtOperatorDeleteP;
static Byte *crtHeapWalkP;
static Byte *crtStrdupP;
static Byte *crtWcsdupP;

static int32_t s_crtBuild;

/*----------------------------------------------------------------------+
|
|   Visual C Runtime Patch Routines - The following code allows
|   of the Visual C runtime DLL memory image to get control when malloc,
|   calloc, realloc, or free are called.  The following table is used
|   to verify that the runtime image can be safely patched, and is also
|   used to help modify instructions that need to be changed on x64,
|   for example a 32-bit call that won't work if we're far from the CRT.
|   Each byte in the 'info' and 'test' arrays correspond to a code byte at the
|   start of the runtime functions we will patch.  If the 'info' array
|   bit 7 is set, then the 'test' array value must match the code
|   before we can patch. On x86, if bit 0 of the 'info' byte is set, then the
|   corresponding code byte needs to be relocated when it is copied. On x64 this
|   type of relocation does not work at large distances, but it will still attempt
|   relocation. For vital functions, we can rewrite the machine code to use instructions
|   that work at large distances. MCP denotes the first byte of an instruction in the
|   format: mov rcx, qword ptr [address]. VCRP denotes the first byte of an instruction
|   in the format: call qword ptr [address]. VC32 denotes the first byte of an
|   instruction in the format: call address. These are used to aid in rewriting the
|   machine code on x64. Note that Bit 7 is set on all of them, so they will be
|   checked for equality. For addresses in these instructions, use VABS, so they are not
|   checked for equality. For the remaining bytes, use VEQU. VOFF is used for short jumps.
|   Code added at runtime will invalidate the old offsets, and VOFF is a marker indicating
|   that the offset needs to be changed. Put this in the byte corresponding to the offset,
|   not the opcode. This is generally the second byte of a two-byte jump.
|
+----------------------------------------------------------------------*/
typedef enum testycontrol
    {
    VOFF   =   0x1040,  // Used for adjusting a short jump to account for code rewriting.
    VMCP   =   0x880,   // "Move to rCx over Pointer" Used for 32-bit relative move-from-pointer instructions accessing CRT data which may need to be made absolute if the CRT is too far away.
    VCRP   =   0x480,   // "Call Relative over Pointer". A lot like VC32
    VC32   =   0x280,   // "Call 32-bit (relative)". Used for 32-bit calls to CRT functions which may need to be made absolute if the CRT is too far away.
    VEQU   =   0x80,    // Exact match
    VABS   =   0x40,    // Absolute address (ignore in match, but copied exactly in patch). Used for "push offset __non_rtti_object::`vftable'+0BC0h (77c11fa8)"
    V___   =   0x01,    // Relocate the address in the patch area. Used for "77C2892B: E8 EA 22 00 00   call free+1AFh (77c2ac1a)" (i.e. relative call)
    } TestControl;


#define PATCHBYTES_MALLOC(build)            (sizeof build##MallocTest)
#define PATCHBYTES_NH_MALLOC(build)         (sizeof build##NhMallocTest)
#define PATCHBYTES_CALLOC(build)            (sizeof build##CallocTest)
#define PATCHBYTES_REALLOC(build)           (sizeof build##ReallocTest)
#define PATCHBYTES_FREE(build)              (sizeof build##FreeTest)
#define PATCHBYTES_MSIZE(build)             (sizeof build##MsizeTest)
#define PATCHBYTES_EXPAND(build)            (sizeof build##ExpandTest)
#define PATCHBYTES_OperatorNew(build)       (sizeof build##OperatorNewTest)
#define PATCHBYTES_OperatorDelete(build)    (sizeof build##OperatorDeleteTest)
#define PATCHBYTES_HEAPWALK(build)          (sizeof build##HeapWalkTest)
#define PATCHBYTES_STRDUP(build)            (sizeof build##StrdupTest)
#define PATCHBYTES_WCSDUP(build)            (sizeof build##WcsdupTest)

#if !defined (PAPATCH_No_MSCVRT_Patch_Tables)
#pragma region Patch Tables
#if defined (_X86_)
#pragma region Visual Studio 9 Patch Table
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 9.0.21022.8   VC 9.0  (21022==0x521E)*/
    TestControl g_b0900521eMallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0900521eMallocTest[_countof(g_b0900521eMallocInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x56, 0x8b, 0x75, 0x08};
    Byte g_b0900521eMallocSave[_countof(g_b0900521eMallocInfo)];
    //_malloc:
    //73B63D3F 8B FF            mov         edi,edi
    //73B63D41 55               push        ebp
    //73B63D42 8B EC            mov         ebp,esp
    //73B63D44 56               push        esi
    //73B63D45 8B 75 08         mov         esi,dword ptr [ebp+8]
    //73B63D48 83 FE E0         cmp         esi,0FFFFFFE0h

    TestControl g_b0900521eCallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eCallocTest[_countof(g_b0900521eCallocInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x51, 0x83, 0x65, 0xfc, 0x00};
    Byte g_b0900521eCallocSave[_countof(g_b0900521eCallocInfo)];
    //_calloc
    //73B63C40 8B FF            mov         edi,edi
    //73B63C42 55               push        ebp
    //73B63C43 8B EC            mov         ebp,esp
    //73B63C45 51               push        ecx
    //73B63C46 83 65 FC 00      and         dword ptr [ebp-4],0

    TestControl g_b0900521eReallocInfo[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0900521eReallocTest[_countof(g_b0900521eReallocInfo)] = { 0x6a, 0x1c, 0x68, 0xe0, 0xd6, 0xb8, 0x73 };
    Byte g_b0900521eReallocSave[_countof(g_b0900521eReallocInfo)];
    //_realloc
    //73B663FC 6A 1C            push        1Ch
    //73B663FE 68 E0 D6 B8 73   push        73B8D6E0h
    //73B66403 E8 D8 6A 00 00   call        73B6CEE0

    TestControl g_b0900521eFreeInfo[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS};
    Byte g_b0900521eFreeTest[_countof(g_b0900521eFreeInfo)] = { 0x6a, 0x18, 0x68, 0x80, 0xd5, 0xb8, 0x73};
    Byte g_b0900521eFreeSave[_countof(g_b0900521eFreeInfo)];
    //_free
    //73B63B4E 6A 18            push        18h
    //73B63B50 68 80 D5 B8 73   push        73B8D580h

    TestControl g_b0900521eMsizeInfo[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0900521eMsizeTest[_countof(g_b0900521eMsizeInfo)] = { 0x6a, 0x1c, 0x68, 0xb8, 0xd6, 0xb8, 0x73};
    Byte g_b0900521eMsizeSave[_countof(g_b0900521eMsizeInfo)];
    //_msize
    //73B66182 6A 1C            push        1Ch
    //73B66184 68 B8 D6 B8 73   push        73B8D6B8h

    TestControl g_b0900521eExpandInfo[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0900521eExpandTest[_countof(g_b0900521eExpandInfo)] = { 0x6a, 0x1c, 0x68, 0x20, 0xd6, 0xb8, 0x73};
    Byte g_b0900521eExpandSave[_countof(g_b0900521eExpandInfo)];
    //_expand
    //73B64334 6A 1C            push        1Ch
    //73B64336 68 20 D6 B8 73   push        73B8D620h

    TestControl g_b0900521eOperatorNewCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0900521eOperatorNewTest[_countof(g_b0900521eOperatorNewCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xec, 0x0c};
    Byte g_b0900521eOperatorNewSave[_countof(g_b0900521eOperatorNewCtrl)];
    //_??2@YAPAXI@Z
    //73B63E99 8B FF            mov         edi,edi
    //73B63E9B 55               push        ebp
    //73B63E9C 8B EC            mov         ebp,esp
    //73B63E9E 83 EC 0C         sub         esp,0Ch
    //73B63EA1 EB 0D            jmp         73B63EB0

    TestControl g_b0900521eOperatorDeleteCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, V___, V___, V___ , V___ };
    Byte g_b0900521eOperatorDeleteTest[_countof(g_b0900521eOperatorDeleteCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x5d, 0xe9, 0x40, 0xfc, 0xff, 0xff };
    Byte g_b0900521eOperatorDeleteSave[_countof(g_b0900521eOperatorDeleteCtrl)];
    //_??3@YAXPAX@Z
    //73B63F03 8B FF            mov         edi,edi
    //73B63F05 55               push        ebp
    //73B63F06 8B EC            mov         ebp,esp
    //73B63F08 5D               pop         ebp
    //73B63F09 E9 40 FC FF FF   jmp         73B63B4E

    TestControl g_b0900521eHeapWalkCtrl[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0900521eHeapWalkTest[_countof(g_b0900521eHeapWalkCtrl)] = { 0x6a, 0x2c, 0x68, 0x98, 0xd6, 0xb8, 0x73 };
    Byte g_b0900521eHeapWalkSave[_countof(g_b0900521eHeapWalkCtrl)];
    //_heapwalk
    //73B66020 6A 2C            push        2Ch
    //73B66022 68 98 D6 B8 73   push        73B8D698h

    TestControl g_b0900521eStrdupCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0900521eStrdupTest[_countof(g_b0900521eStrdupCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x33, 0xdb, 0x39, 0x5d, 0x08};
    Byte g_b0900521eStrdupSave[_countof(g_b0900521eStrdupCtrl)];
    //_strdup
    //73B36A87 8B FF            mov         edi,edi
    //73B36A89 55               push        ebp
    //73B36A8A 8B EC            mov         ebp,esp
    //73B36A8C 53               push        ebx
    //73B36A8D 33 DB            xor         ebx,ebx
    //73B36A8F 39 5D 08         cmp         dword ptr [ebp+8],ebx

    TestControl g_b0900521eWcsdupCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0900521eWcsdupTest[_countof(g_b0900521eWcsdupCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x33, 0xdb, 0x39, 0x5d, 0x08};
    Byte g_b0900521eWcsdupSave[_countof(g_b0900521eWcsdupCtrl)];
    //_wcsdup
    //73B372E8 8B FF            mov         edi,edi
    //73B372EA 55               push        ebp
    //73B372EB 8B EC            mov         ebp,esp
    //73B372ED 53               push        ebx
    //73B372EE 33 DB            xor         ebx,ebx
    //73B372F0 39 5D 08         cmp         dword ptr [ebp+8],ebx
#pragma endregion
#pragma region Visual Studio 10 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 10.0.30319.1   VC 10.0  (30319=766F)*/
    TestControl g_b0A00766fMallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fMallocTest[_countof(g_b0A00766fMallocInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x8b, 0x5d, 0x08, 0x83, 0xfb, 0xe0 };
    Byte g_b0A00766fMallocSave[_countof(g_b0A00766fMallocInfo)];
    //_malloc
    //69440233 8B FF            mov         edi,edi
    //69440235 55               push        ebp
    //69440236 8B EC            mov         ebp,esp
    //69440238 53               push        ebx
    //69440239 8B 5D 08         mov         ebx,dword ptr [ebp+8]
    //6944023C 83 FB E0         cmp         ebx,0FFFFFFE0h

    TestControl g_b0A00766fCallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0A00766fCallocTest[_countof(g_b0A00766fCallocInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec};
    Byte g_b0A00766fCallocSave[_countof(g_b0A00766fCallocInfo)];
    //69440282 8B FF            mov         edi,edi
    //69440284 55               push        ebp
    //69440285 8B EC            mov         ebp,esp
    //69440287 51               push        ecx
    //69440288 83 65 FC 00      and         dword ptr [ebp-4],0

    TestControl g_b0A00766fReallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fReallocTest[_countof(g_b0A00766fReallocInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0A00766fReallocSave[_countof(g_b0A00766fReallocInfo)];
    //69442ADB 8B FF            mov         edi,edi
    //69442ADD 55               push        ebp
    //69442ADE 8B EC            mov         ebp,esp
    //69442AE0 83 7D 08 00      cmp         dword ptr [ebp+8],0
    //69442AE4 0F 84 A3 00 00 00 je          realloc+0Bh (69442B8Dh)

    TestControl g_b0A00766fFreeInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fFreeTest[_countof(g_b0A00766fFreeInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0A00766fFreeSave[_countof(g_b0A00766fFreeInfo)];
    //6944014E 8B FF            mov         edi,edi
    //69440150 55               push        ebp
    //69440151 8B EC            mov         ebp,esp
    //69440153 83 7D 08 00      cmp         dword ptr [ebp+8],0
    //69440157 74 19            je          free+38h (69440172h)

    TestControl g_b0A00766fMsizeInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fMsizeTest[_countof(g_b0A00766fMsizeInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0A00766fMsizeSave[_countof(g_b0A00766fMsizeInfo)];
    //69442231 8B FF            mov         edi,edi
    //69442233 55               push        ebp
    //69442234 8B EC            mov         ebp,esp
    //69442236 83 7D 08 00      cmp         dword ptr [ebp+8],0
    //6944223A 0F 84 FD D0 02 00 je          _msize+0Bh (6946F33Dh)

    TestControl g_b0A00766fExpandInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU  };
    Byte g_b0A00766fExpandTest[_countof(g_b0A00766fExpandInfo)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x51, 0x83, 0x7d, 0x08, 0x00};
    Byte g_b0A00766fExpandSave[_countof(g_b0A00766fExpandInfo)];
    //694B6957 8B FF            mov         edi,edi
    //694B6959 55               push        ebp
    //694B695A 8B EC            mov         ebp,esp
    //694B695C 51               push        ecx
    //694B695D 83 7D 08 00      cmp         dword ptr [ebp+8],0

    TestControl g_b0A00766fOperatorNewCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fOperatorNewTest[_countof(g_b0A00766fOperatorNewCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xec, 0x10, 0xff, 0x75, 0x08 };
    Byte g_b0A00766fOperatorNewSave[_countof(g_b0A00766fOperatorNewCtrl)];
    //694402C1 8B FF            mov         edi,edi
    //694402C3 55               push        ebp
    //694402C4 8B EC            mov         ebp,esp
    //694402C6 83 EC 10         sub         esp,10h
    //694402C9 FF 75 08         push        dword ptr [ebp+8]

    TestControl g_b0A00766fOperatorDeleteCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fOperatorDeleteTest[_countof(g_b0A00766fOperatorDeleteCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x5d};
    Byte g_b0A00766fOperatorDeleteSave[_countof(g_b0A00766fOperatorDeleteCtrl)];
    //69440174 8B FF            mov         edi,edi
    //69440176 55               push        ebp
    //69440177 8B EC            mov         ebp,esp
    //69440179 5D               pop         ebp
    //6944017A EB D2            jmp         free (6944014Eh)

    TestControl g_b0A00766fHeapWalkCtrl[] = { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0A00766fHeapWalkTest[_countof(g_b0A00766fHeapWalkCtrl)] = { 0x6a, 0x2c, 0x68, 0x88, 0x6b, 0x4b, 0x69, 0xe8, 0x0d, 0xa2, 0xf8, 0xff };
    Byte g_b0A00766fHeapWalkSave[_countof(g_b0A00766fHeapWalkCtrl)];
    //694B6A67 6A 2C            push        2Ch
    //694B6A69 68 88 6B 4B 69   push        offset __CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QAE@ABV01@@Z12+60h (694B6B88h)
    //694B6A6E E8 0D A2 F8 FF   call        __SEH_prolog4 (69440C80h)

    TestControl g_b0A00766fStrdupCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fStrdupTest[_countof(g_b0A00766fStrdupCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x33, 0xdb, 0x39, 0x5d, 0x08 };
    Byte g_b0A00766fStrdupSave[_countof(g_b0A00766fStrdupCtrl)];
    //69455B46 8B FF            mov         edi,edi
    //69455B48 55               push        ebp
    //69455B49 8B EC            mov         ebp,esp
    //69455B4B 53               push        ebx
    //69455B4C 33 DB            xor         ebx,ebx
    //69455B4E 39 5D 08         cmp         dword ptr [ebp+8],ebx

    TestControl g_b0A00766fWcsdupCtrl[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fWcsdupTest[_countof(g_b0A00766fWcsdupCtrl)] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x33, 0xdb, 0x39, 0x5d, 0x08 };
    Byte g_b0A00766fWcsdupSave[_countof(g_b0A00766fWcsdupCtrl)];
    //694B1E41 8B FF            mov         edi,edi
    //694B1E43 55               push        ebp
    //694B1E44 8B EC            mov         ebp,esp
    //694B1E46 53               push        ebx
    //694B1E47 33 DB            xor         ebx,ebx
    //694B1E49 39 5D 08         cmp         dword ptr [ebp+8],ebx
#pragma endregion
#pragma region Visual Studio 11 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 11.0.51106.1   VC 11.0  (30319=C7A2)*/
    TestControl g_b0B00C7A2MallocInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2MallocTest[_countof(g_b0B00C7A2MallocInfo)] =   { 0x55, 0x8b, 0xec, 0x56, 0x8b, 0x75, 0x08, 0x83, 0xfe, 0xe0 };
    Byte g_b0B00C7A2MallocSave[_countof(g_b0B00C7A2MallocInfo)];
    //_malloc
    //6459DCCC 55                   push        ebp  
    //6459DCCD 8B EC                mov         ebp,esp  
    //6459DCCF 56                   push        esi  
    //6459DCD0 8B 75 08             mov         esi,dword ptr [ebp+8]  
    //6459DCD3 83 FE E0             cmp         esi,0FFFFFFE0h  

    TestControl g_b0B00C7A2CallocInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2CallocTest[_countof(g_b0B00C7A2CallocInfo)] =   { 0x55, 0x8b, 0xec, 0x51, 0x83, 0x65, 0xfc, 0x00 };
    Byte g_b0B00C7A2CallocSave[_countof(g_b0B00C7A2CallocInfo)];
    //645BFA75 55                   push        ebp  
    //645BFA76 8B EC                mov         ebp,esp  
    //645BFA78 51                   push        ecx  
    //645BFA79 83 65 FC 00          and         dword ptr [ebp-4],0  

    TestControl g_b0B00C7A2ReallocInfo[] =                          { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2ReallocTest[_countof(g_b0B00C7A2ReallocInfo)] = { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0B00C7A2ReallocSave[_countof(g_b0B00C7A2ReallocInfo)];
    //645A21C5 55                   push        ebp  
    //645A21C6 8B EC                mov         ebp,esp  
    //645A21C8 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645A21CC 0F 84 8F D9 01 00    je          _realloc+9 (645BFB61h)  

    TestControl g_b0B00C7A2FreeInfo[] =                             { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2FreeTest[_countof(g_b0B00C7A2FreeInfo)] =       { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0B00C7A2FreeSave[_countof(g_b0B00C7A2FreeInfo)];
    //6459DCA8 55                   push        ebp  
    //6459DCA9 8B EC                mov         ebp,esp  
    //6459DCAB 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //6459DCAF 74 19                je          _free+36h (6459DCCAh)  

    TestControl g_b0B00C7A2MsizeInfo[] =                            { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2MsizeTest[_countof(g_b0B00C7A2MsizeInfo)] =     { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0B00C7A2MsizeSave[_countof(g_b0B00C7A2MsizeInfo)];
    //645A1DCC 55                   push        ebp  
    //645A1DCD 8B EC                mov         ebp,esp  
    //645A1DCF 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645A1DD3 0F 84 BF 85 04 00    je          __msize+9 (645EA398h)  

    TestControl g_b0B00C7A2ExpandInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2ExpandTest[_countof(g_b0B00C7A2ExpandInfo)] =   { 0x55, 0x8b, 0xec, 0x57, 0x8b, 0x7d, 0x08 };
    Byte g_b0B00C7A2ExpandSave[_countof(g_b0B00C7A2ExpandInfo)];
    //6462BB3E 55                   push        ebp  
    //6462BB3F 8B EC                mov         ebp,esp  
    //6462BB41 57                   push        edi  
    //6462BB42 8B 7D 08             mov         edi,dword ptr [ebp+8]  
    //6462BB45 85 FF                test        edi,edi  
    //6462BB47 75 17                jne         __expand+22h (6462BB60h)  

    TestControl g_b0B00C7A2OperatorNewCtrl[] =                              { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2OperatorNewTest[_countof(g_b0B00C7A2OperatorNewCtrl)] = { 0x55, 0x8b, 0xec, 0x83, 0xec, 0x10 };
    Byte g_b0B00C7A2OperatorNewSave[_countof(g_b0B00C7A2OperatorNewCtrl)];
    //6459DD99 55                   push        ebp  
    //6459DD9A 8B EC                mov         ebp,esp  
    //6459DD9C 83 EC 10             sub         esp,10h  
    //6459DD9F FF 75 08             push        dword ptr [ebp+8]  

    TestControl g_b0B00C7A2OperatorDeleteCtrl[] =                                   { VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2OperatorDeleteTest[_countof(g_b0B00C7A2OperatorDeleteCtrl)] =   { 0xeb, 0x96, 0x90, 0x90, 0x90 };
    Byte g_b0B00C7A2OperatorDeleteSave[_countof(g_b0B00C7A2OperatorDeleteCtrl)];
    //6459DD10 EB 96                jmp         _free (6459DCA8h)  
    //6459DD12 90                   nop  
    //6459DD13 90                   nop  
    //6459DD14 90                   nop  

    TestControl g_b0B00C7A2HeapWalkCtrl[] =                             { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0B00C7A2HeapWalkTest[_countof(g_b0B00C7A2HeapWalkCtrl)] =   { 0x6a, 0x2c, 0x68, 0x68, 0xbd, 0x62, 0x64, 0xe8, 0x13, 0x43, 0xf7, 0xff };
    Byte g_b0B00C7A2HeapWalkSave[_countof(g_b0B00C7A2HeapWalkCtrl)];
    //6462BC4B 6A 2C                push        2Ch  
    //6462BC4D 68 68 BD 62 64       push        offset __CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QAE@ABV01@@Z12+5Ch (6462BD68h)  
    //6462BC52 E8 13 43 F7 FF       call        __SEH_prolog4 (6459FF6Ah)  

    TestControl g_b0B00C7A2StrdupCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2StrdupTest[_countof(g_b0B00C7A2StrdupCtrl)] =   { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0B00C7A2StrdupSave[_countof(g_b0B00C7A2StrdupCtrl)];
    //645BC788 55                   push        ebp  
    //645BC789 8B EC                mov         ebp,esp  
    //645BC78B 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645BC78F 0F 84 E1 5F 02 00    je          __strdup+9 (645E2776h)  

    TestControl g_b0B00C7A2WcsdupCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2WcsdupTest[_countof(g_b0B00C7A2WcsdupCtrl)] =   { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0B00C7A2WcsdupSave[_countof(g_b0B00C7A2WcsdupCtrl)];
    //645BB02C 55                   push        ebp  
    //645BB02D 8B EC                mov         ebp,esp  
    //645BB02F 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645BB033 0F 84 70 7A 02 00    je          __wcsdup+9 (645E2AA9h)  

#pragma endregion
#pragma region Visual Studio 12 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 12.0.21005.1   VC 12.0  (21005=520d)*/
    TestControl g_b0C00520DMallocInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DMallocTest[_countof(g_b0C00520DMallocInfo)] =   { 0x55, 0x8b, 0xec, 0x56, 0x8b, 0x75, 0x08, 0x83, 0xfe, 0xe0 };
    Byte g_b0C00520DMallocSave[_countof(g_b0C00520DMallocInfo)];
    //_malloc
    //6459DCCC 55                   push        ebp  
    //6459DCCD 8B EC                mov         ebp,esp  
    //6459DCCF 56                   push        esi  
    //6459DCD0 8B 75 08             mov         esi,dword ptr [ebp+8]  
    //6459DCD3 83 FE E0             cmp         esi,0FFFFFFE0h  

    TestControl g_b0C00520DCallocInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DCallocTest[_countof(g_b0C00520DCallocInfo)] =   { 0x55, 0x8b, 0xec, 0x51, 0x83, 0x65, 0xfc, 0x00 };
    Byte g_b0C00520DCallocSave[_countof(g_b0C00520DCallocInfo)];
    //645BFA75 55                   push        ebp  
    //645BFA76 8B EC                mov         ebp,esp  
    //645BFA78 51                   push        ecx  
    //645BFA79 83 65 FC 00          and         dword ptr [ebp-4],0  

    TestControl g_b0C00520DReallocInfo[] =                          { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DReallocTest[_countof(g_b0C00520DReallocInfo)] = { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0C00520DReallocSave[_countof(g_b0C00520DReallocInfo)];
    //645A21C5 55                   push        ebp  
    //645A21C6 8B EC                mov         ebp,esp  
    //645A21C8 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645A21CC 0F 84 8F D9 01 00    je          _realloc+9 (645BFB61h)  

    TestControl g_b0C00520DFreeInfo[] =                             { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DFreeTest[_countof(g_b0C00520DFreeInfo)] =       { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0C00520DFreeSave[_countof(g_b0C00520DFreeInfo)];
    //6459DCA8 55                   push        ebp  
    //6459DCA9 8B EC                mov         ebp,esp  
    //6459DCAB 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //6459DCAF 74 19                je          _free+36h (6459DCCAh)  

    TestControl g_b0C00520DMsizeInfo[] =                            { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DMsizeTest[_countof(g_b0C00520DMsizeInfo)] =     { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0C00520DMsizeSave[_countof(g_b0C00520DMsizeInfo)];
    //645A1DCC 55                   push        ebp  
    //645A1DCD 8B EC                mov         ebp,esp  
    //645A1DCF 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645A1DD3 0F 84 BF 85 04 00    je          __msize+9 (645EA398h)  

    TestControl g_b0C00520DExpandInfo[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DExpandTest[_countof(g_b0C00520DExpandInfo)] =   { 0x55, 0x8b, 0xec, 0x57, 0x8b, 0x7d, 0x08 };
    Byte g_b0C00520DExpandSave[_countof(g_b0C00520DExpandInfo)];
    //6462BB3E 55                   push        ebp  
    //6462BB3F 8B EC                mov         ebp,esp  
    //6462BB41 57                   push        edi  
    //6462BB42 8B 7D 08             mov         edi,dword ptr [ebp+8]  
    //6462BB45 85 FF                test        edi,edi  
    //6462BB47 75 17                jne         __expand+22h (6462BB60h)  

    TestControl g_b0C00520DOperatorNewCtrl[] =                              { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DOperatorNewTest[_countof(g_b0C00520DOperatorNewCtrl)] = { 0x55, 0x8b, 0xec, 0x83, 0xec, 0x10 };
    Byte g_b0C00520DOperatorNewSave[_countof(g_b0C00520DOperatorNewCtrl)];
    //6459DD99 55                   push        ebp  
    //6459DD9A 8B EC                mov         ebp,esp  
    //6459DD9C 83 EC 10             sub         esp,10h  
    //6459DD9F FF 75 08             push        dword ptr [ebp+8]  

    TestControl g_b0C00520DOperatorDeleteCtrl[] =                                   { VEQU, V___, V___, V___, V___ };
    Byte g_b0C00520DOperatorDeleteTest[_countof(g_b0C00520DOperatorDeleteCtrl)] =   { 0xe9, 0x67, 0xff, 0xff, 0xff };
    Byte g_b0C00520DOperatorDeleteSave[_countof(g_b0C00520DOperatorDeleteCtrl)];
    //6704ED74 E9 67 FF FF FF       jmp         free (6704ECE0h)

    TestControl g_b0C00520DHeapWalkCtrl[] =                             { VEQU, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VABS, VABS, VABS, VABS };
    Byte g_b0C00520DHeapWalkTest[_countof(g_b0C00520DHeapWalkCtrl)] =   { 0x6a, 0x2c, 0x68, 0x68, 0xbd, 0x62, 0x64, 0xe8, 0x13, 0x43, 0xf7, 0xff };
    Byte g_b0C00520DHeapWalkSave[_countof(g_b0C00520DHeapWalkCtrl)];
    //6462BC4B 6A 2C                push        2Ch  
    //6462BC4D 68 68 BD 62 64       push        offset __CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QAE@ABV01@@Z12+5Ch (6462BD68h)  
    //6462BC52 E8 13 43 F7 FF       call        __SEH_prolog4 (6459FF6Ah)  

    TestControl g_b0C00520DStrdupCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DStrdupTest[_countof(g_b0C00520DStrdupCtrl)] =   { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0C00520DStrdupSave[_countof(g_b0C00520DStrdupCtrl)];
    //645BC788 55                   push        ebp  
    //645BC789 8B EC                mov         ebp,esp  
    //645BC78B 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645BC78F 0F 84 E1 5F 02 00    je          __strdup+9 (645E2776h)  

    TestControl g_b0C00520DWcsdupCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DWcsdupTest[_countof(g_b0C00520DWcsdupCtrl)] =   { 0x55, 0x8b, 0xec, 0x83, 0x7d, 0x08, 0x00 };
    Byte g_b0C00520DWcsdupSave[_countof(g_b0C00520DWcsdupCtrl)];
    //645BB02C 55                   push        ebp  
    //645BB02D 8B EC                mov         ebp,esp  
    //645BB02F 83 7D 08 00          cmp         dword ptr [ebp+8],0  
    //645BB033 0F 84 70 7A 02 00    je          __wcsdup+9 (645E2AA9h)  

#pragma endregion
#elif defined (_M_X64)
#pragma region Visual Studio 9 Patch Table
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 9.0.21022.8   VC 9.0  (21022==0x521E)*/
    TestControl g_b0900521eMallocInfo[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eMallocTest[_countof(g_b0900521eMallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20 };
    Byte g_b0900521eMallocSave[_countof(g_b0900521eMallocInfo)];
    //_malloc:
    //000000006ADCC82C 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //000000006ADCC831 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //000000006ADCC836 57                   push        rdi
    //000000006ADCC837 48 83 EC 20          sub         rsp,20h

    TestControl g_b0900521eCallocInfo[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eCallocTest[_countof(g_b0900521eCallocInfo)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x83, 0x64, 0x24, 0x40, 0x00, 0x4c, 0x8d, 0x44, 0x24, 0x40 };
    Byte g_b0900521eCallocSave[_countof(g_b0900521eCallocInfo)];
    //_calloc
    //000000006ADCC7E4 40 53                push        rbx
    //000000006ADCC7E6 48 83 EC 20          sub         rsp,20h
    //000000006ADCC7EA 83 64 24 40 00       and         dword ptr [rsp+40h],0
    //000000006ADCC7EF 4C 8D 44 24 40       lea         r8,[rsp+40h]

    TestControl g_b0900521eReallocInfo[] =                   { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eReallocTest[_countof(g_b0900521eReallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20 };
    Byte g_b0900521eReallocSave[_countof(g_b0900521eReallocInfo)];
    //_realloc
    //000000006ADCD50C 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //000000006ADCD511 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //000000006ADCD516 57                   push        rdi
    //000000006ADCD517 48 83 EC 20          sub         rsp,20h


    TestControl g_b0900521eFreeInfo[] =                       { VEQU, VEQU, VEQU, VEQU, VOFF, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VCRP, VEQU, VABS, VABS ,VABS,
        VABS, VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eFreeTest[_countof(g_b0900521eFreeInfo)] = { 0x48, 0x85, 0xc9, 0x74, 0x37, 0x53, 0x48, 0x83, 0xec, 0x20, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0xdc, 0x73, 0x04, 0x00, 0x33, 0xd2, 0xff, 0x15, 0x44, 0xbb, 0x02,
        0x00, 0x85, 0xc0, 0x75, 0x17, 0xe8, 0x5f, 0xa1, 0xfb, 0xff, 0x48, 0x8b, 0xd8, 0xff, 0x15, 0xe2, 0xb8, 0x02, 0x00, 0x8b, 0xc8, 0xe8, 0xff, 0xa0, 0xfb, 0xff, 0x89, 0x03, 0x48, 0x83, 0xc4, 0x20, 0x5b, 0xc3 };
    Byte g_b0900521eFreeSave[_countof(g_b0900521eFreeInfo)];
    //_free
    //000000006ADCC7A0 48 85 C9             test        rcx,rcx
    //000000006ADCC7A3 74 37                je          000000006ADCC7DC
    //000000006ADCC7A5 53                   push        rbx
    //000000006ADCC7A6 48 83 EC 20          sub         rsp,20h
    //000000006ADCC7AA 4C 8B C1             mov         r8,rcx
    //000000006ADCC7AD 48 8B 0D DC 73 04 00 mov         rcx,qword ptr [6AE13B90h]
    //000000006ADCC7B4 33 D2                xor         edx,edx
    //000000006ADCC7B6 FF 15 44 BB 02 00    call        qword ptr [6ADF8300h]
    //000000006ADCC7BC 85 C0                test        eax,eax
    //000000006ADCC7BE 75 17                jne         000000006ADCC7D7
    //000000006ADCC7C0 E8 5F A1 FB FF       call        000000006AD86924
    //000000006ADCC7C5 48 8B D8             mov         rbx,rax
    //000000006ADCC7C8 FF 15 E2 B8 02 00    call        qword ptr [6ADF80B0h]
    //000000006ADCC7CE 8B C8                mov         ecx,eax
    //000000006ADCC7D0 E8 FF A0 FB FF       call        000000006AD868D4
    //000000006ADCC7D5 89 03                mov         dword ptr [rbx],eax
    //000000006ADCC7D7 48 83 C4 20          add         rsp,20h
    //000000006ADCC7DB 5B                   pop         rbx
    //000000006ADCC7DC C3                   ret

    TestControl g_b0900521eMsizeInfo[] =                        { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,
        VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VOFF, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VCRP,
        VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eMsizeTest[_countof(g_b0900521eMsizeInfo)] = { 0x48, 0x83, 0xec, 0x38, 0x48, 0x85, 0xc9, 0x75, 0x26, 0xe8, 0x4a, 0x96, 0xfb, 0xff, 0x48, 0x83, 0x64, 0x24, 0x20, 0x00, 0x45, 0x33, 0xc9, 0x45, 0x33, 0xc0,
        0x33, 0xd2, 0x33, 0xc9, 0xc7, 0x00, 0x16, 0x00, 0x00, 0x00, 0xe8, 0x5b, 0x7c, 0x00, 0x00, 0x48, 0x83, 0xc8, 0xff, 0xeb, 0x12, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0x8b, 0x68, 0x04, 0x00, 0x33, 0xd2, 0xff,
        0x15, 0x13, 0xb0, 0x02, 0x00, 0x48, 0x83, 0xc4, 0x38, 0xc3 };
    Byte g_b0900521eMsizeSave[_countof(g_b0900521eMsizeInfo)];
    //_msize
    //000000006ADCD2CC 48 83 EC 38          sub         rsp,38h
    //000000006ADCD2D0 48 85 C9             test        rcx,rcx
    //000000006ADCD2D3 75 26                jne         000000006ADCD2FB
    //000000006ADCD2D5 E8 4A 96 FB FF       call        000000006AD86924
    //000000006ADCD2DA 48 83 64 24 20 00    and         qword ptr [rsp+20h],0
    //000000006ADCD2E0 45 33 C9             xor         r9d,r9d
    //000000006ADCD2E3 45 33 C0             xor         r8d,r8d
    //000000006ADCD2E6 33 D2                xor         edx,edx
    //000000006ADCD2E8 33 C9                xor         ecx,ecx
    //000000006ADCD2EA C7 00 16 00 00 00    mov         dword ptr [rax],16h
    //000000006ADCD2F0 E8 5B 7C 00 00       call        000000006ADD4F50
    //000000006ADCD2F5 48 83 C8 FF          or          rax,0FFFFFFFFFFFFFFFFh
    //000000006ADCD2F9 EB 12                jmp         000000006ADCD30D
    //000000006ADCD2FB 4C 8B C1             mov         r8,rcx
    //000000006ADCD2FE 48 8B 0D 8B 68 04 00 mov         rcx,qword ptr [6AE13B90h]
    //000000006ADCD305 33 D2                xor         edx,edx
    //000000006ADCD307 FF 15 13 B0 02 00    call        qword ptr [6ADF8320h]
    //000000006ADCD30D 48 83 C4 38          add         rsp,38h
    //000000006ADCD311 C3                   ret
    //000000006ADCD312 CC                   int         3
    //000000006ADCD313 CC                   int         3
    //000000006ADCD314 CC                   int         3
    //000000006ADCD315 CC                   int         3
    //000000006ADCD316 CC                   int         3
    //000000006ADCD317 CC                   int         3


    TestControl g_b0900521eExpandInfo[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eExpandTest[_countof(g_b0900521eExpandInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20 };
    Byte g_b0900521eExpandSave[_countof(g_b0900521eExpandInfo)];
    //_expand
    //000000006ADCCE3C 48 89 5C 24 10       mov         qword ptr [rsp+10h],rbx
    //000000006ADCCE41 48 89 6C 24 18       mov         qword ptr [rsp+18h],rbp
    //000000006ADCCE46 48 89 74 24 20       mov         qword ptr [rsp+20h],rsi

    TestControl g_b0900521eOperatorNewCtrl[] =                       { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU,
        VEQU, VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0900521eOperatorNewTest[_countof(g_b0900521eOperatorNewCtrl)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x40, 0x48, 0x8b, 0xd9, 0xeb, 0x0f, 0x48, 0x8b, 0xcb, 0xe8, 0x15, 0x47, 0xfb, 0xff, 0x85, 0xc0, 0x74, 0x13, 0x48,
        0x8b, 0xcb, 0xe8, 0xa5, 0xfe, 0xff, 0xff, 0x48, 0x85, 0xc0, 0x74, 0xe7, 0x48, 0x83, 0xc4, 0x40, 0x5b, 0xc3};
    Byte g_b0900521eOperatorNewSave[_countof(g_b0900521eOperatorNewCtrl)];
    //_??2@YAPAXI@Z
    //000000006ADCC968 40 53                push        rbx
    //000000006ADCC96A 48 83 EC 40          sub         rsp,40h
    //000000006ADCC96E 48 8B D9             mov         rbx,rcx
    //000000006ADCC971 EB 0F                jmp         000000006ADCC982
    //000000006ADCC973 48 8B CB             mov         rcx,rbx
    //000000006ADCC976 E8 15 47 FB FF       call        000000006AD81090
    //000000006ADCC97B 85 C0                test        eax,eax
    //000000006ADCC97D 74 13                je          000000006ADCC992
    //000000006ADCC97F 48 8B CB             mov         rcx,rbx
    //000000006ADCC982 E8 A5 FE FF FF       call        000000006ADCC82C
    //000000006ADCC987 48 85 C0             test        rax,rax
    //000000006ADCC98A 74 E7                je          000000006ADCC973
    //000000006ADCC98C 48 83 C4 40          add         rsp,40h
    //000000006ADCC990 5B                   pop         rbx
    //000000006ADCC991 C3                   ret

    TestControl g_b0900521eOperatorDeleteCtrl[] =                          { VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eOperatorDeleteTest[_countof(g_b0900521eOperatorDeleteCtrl)] = { 0xe9, 0x93, 0xfd, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc };
    Byte g_b0900521eOperatorDeleteSave[_countof(g_b0900521eOperatorDeleteCtrl)];
    //_??3@YAXPAX@Z
    //000000006ADCCA08 E9 93 FD FF FF       jmp         000000006ADCC7A0
    //000000006ADCCA0D CC                   int         3
    //000000006ADCCA0E CC                   int         3
    //000000006ADCCA0F CC                   int         3
    //000000006ADCCA10 CC                   int         3
    //000000006ADCCA11 CC                   int         3
    //000000006ADCCA12 CC                   int         3
    //000000006ADCCA13 CC                   int         3

    TestControl g_b0900521eHeapWalkCtrl[] =                    { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eHeapWalkTest[_countof(g_b0900521eHeapWalkCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54 };
    Byte g_b0900521eHeapWalkSave[_countof(g_b0900521eHeapWalkCtrl)];
    //_heapwalk
    //000000006ADCD108 48 89 5C 24 18       mov         qword ptr [rsp+18h],rbx
    //000000006ADCD10D 48 89 4C 24 08       mov         qword ptr [rsp+8],rcx
    //000000006ADCD112 56                   push        rsi
    //000000006ADCD113 57                   push        rdi
    //000000006ADCD114 41 54                push        r12

    TestControl g_b0900521eStrdupCtrl[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eStrdupTest[_countof(g_b0900521eStrdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30 };
    Byte g_b0900521eStrdupSave[_countof(g_b0900521eStrdupCtrl)];
    //_strdup
    //000000006AD97EE0 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //000000006AD97EE5 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //000000006AD97EEA 57                   push        rdi
    //000000006AD97EEB 48 83 EC 30          sub         rsp,30h

    TestControl g_b0900521eWcsdupCtrl[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0900521eWcsdupTest[_countof(g_b0900521eWcsdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30 };
    Byte g_b0900521eWcsdupSave[_countof(g_b0900521eWcsdupCtrl)];
    //_wcsdup
    //000000006AD98A18 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //000000006AD98A1D 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //000000006AD98A22 57                   push        rdi
    //000000006AD98A23 48 83 EC 30          sub         rsp,30h
#pragma endregion
#pragma region Visual Studio 10 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 10.0.30319.1   VC 10.0  (30319=766F)*/
    TestControl g_b0A00766fMallocInfo[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fMallocTest[_countof(g_b0A00766fMallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9 };
    Byte g_b0A00766fMallocSave[_countof(g_b0A00766fMallocInfo)];
    //_malloc
    //0000000056988BEC 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //0000000056988BF1 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //0000000056988BF6 57                   push        rdi
    //0000000056988BF7 48 83 EC 20          sub         rsp,20h
    //0000000056988BFB 48 8B D9             mov         rbx,rcx

    TestControl g_b0A00766fCallocInfo[] = { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fCallocTest[_countof(g_b0A00766fCallocInfo)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x83, 0x64, 0x24, 0x40, 0x00, 0x4c, 0x8d, 0x44, 0x24, 0x40 };
    Byte g_b0A00766fCallocSave[_countof(g_b0A00766fCallocInfo)];
    //_calloc
    //0000000056988E28 40 53                push        rbx
    //0000000056988E2A 48 83 EC 20          sub         rsp,20h
    //0000000056988E2E 83 64 24 40 00       and         dword ptr [rsp+40h],0
    //0000000056988E33 4C 8D 44 24 40       lea         r8,[rsp+40h]

    TestControl g_b0A00766fReallocInfo[] =            { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fReallocTest[_countof(g_b0A00766fReallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xda, 0x48, 0x8b, 0xf9 };
    Byte g_b0A00766fReallocSave[_countof(g_b0A00766fReallocInfo)];
    //_realloc
    //00000000569897D0 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //00000000569897D5 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //00000000569897DA 57                   push        rdi
    //00000000569897DB 48 83 EC 20          sub         rsp,20h
    //00000000569897DF 48 8B DA             mov         rbx,rdx
    //00000000569897E2 48 8B F9             mov         rdi,rcx

    TestControl g_b0A00766fFreeInfo[] =                       { VEQU, VEQU, VEQU, VEQU, VOFF, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS,
     VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0A00766fFreeTest[_countof(g_b0A00766fFreeInfo)] = { 0x48, 0x85, 0xc9, 0x74, 0x37, 0x53, 0x48, 0x83, 0xec, 0x20, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0x14, 0xe7, 0x04, 0x00, 0x33, 0xd2, 0xff, 0x15, 0x5c, 0x77, 0x02, 0x00,
     0x85, 0xc0, 0x75, 0x17, 0xe8, 0xdf, 0xc9, 0xfb, 0xff, 0x48, 0x8b, 0xd8, 0xff, 0x15, 0xda, 0x73, 0x02, 0x00, 0x8b, 0xc8, 0xe8, 0x7f, 0xc9, 0xfb, 0xff, 0x89, 0x03, 0x48, 0x83, 0xc4, 0x20, 0x5b, 0xc3 };
    Byte g_b0A00766fFreeSave[_countof(g_b0A00766fFreeInfo)];
    //0000000056988CA8 48 85 C9             test        rcx,rcx
    //0000000056988CAB 74 37                je          free+3Ch (56988CE4h)
    //0000000056988CAD 53                   push        rbx
    //0000000056988CAE 48 83 EC 20          sub         rsp,20h
    //0000000056988CB2 4C 8B C1             mov         r8,rcx
    //0000000056988CB5 48 8B 0D 14 E7 04 00 mov         rcx,qword ptr [_crtheap (569D73D0h)]
    //0000000056988CBC 33 D2                xor         edx,edx
    //0000000056988CBE FF 15 5C 77 02 00    call        qword ptr [__imp_HeapFree (569B0420h)]
    //0000000056988CC5 85 C0                test        eax,eax
    //0000000056988CC7 75 17                jne         free+37h (5EF48CDFh)
    //0000000056988CCC E8 DF C9 FB FF       call        _errno (5EF056ACh)
    //0000000056988CD0 48 8B D8             mov         rbx,rax
    //0000000056988CD6 FF 15 DA 73 02 00    call        qword ptr [__imp_GetLastError (5EF700B0h)]
    //0000000056988CD8 8B C8                mov         ecx,eax
    //0000000056988CDE E8 7F C9 FB FF       call        _get_errno_from_oserr (5EF0565Ch)
    //0000000056988CE4 89 03                mov         dword ptr [rbx],eax
    //000000005FF18CDF 48 83 C4 20          add         rsp,20h
    //000000005FF18CE3 5B                   pop         rbx
    //000000005FF18CE4 C3                   ret

    TestControl g_b0A00766fMsizeInfo[] =                        {VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU,
        VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU/*, VEQU, VEQU, VEQU, VOFF, VOFF, VOFF, VOFF*/};
    Byte g_b0A00766fMsizeTest[_countof(g_b0A00766fMsizeInfo)] = {0x48, 0x83, 0xec, 0x28, 0x48, 0x85, 0xc9, 0x75, 0x19, 0xe8, 0xc6, 0xc0, 0xfb, 0xff, 0xc7, 0x00, 0x16, 0x00, 0x00, 0x00, 0xe8, 0x33, 0x6e, 0x00, 0x00, 0x48, 0x83,
        0xc8, 0xff, 0x48, 0x83, 0xc4, 0x28, 0xc3, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0xcc, 0xdd, 0x04, 0x00, 0x33, 0xd2, 0x48, 0x83, 0xc4, 0x28/*, 0x48, 0xff, 0x25, 0x47, 0x6e, 0x02, 0x00*/};
    Byte g_b0A00766fMsizeSave[_countof(g_b0A00766fMsizeInfo)];
    //00000000569895D8 48 83 EC 28          sub         rsp,28h
    //00000000569895DC 48 85 C9             test        rcx,rcx
    //00000000569895DF 75 19                jne         _msize+22h (569895FAh)
    //00000000569895E1 E8 C6 C0 FB FF       call        _errno (569456ACh)
    //00000000569895E6 C7 00 16 00 00 00    mov         dword ptr [rax],16h
    //00000000569895EC E8 33 6E 00 00       call        _invalid_parameter_noinfo (56990424h)
    //00000000569895F1 48 83 C8 FF          or          rax,0FFFFFFFFFFFFFFFFh
    //0000000071AD95F5 48 83 C4 28          add         rsp,28h
    //0000000071AD95F9 C3                   ret
    //0000000071AD95FA 4C 8B C1             mov         r8,rcx
    //0000000071AD95FD 48 8B 0D CC DD 04 00 mov         rcx,qword ptr [_crtheap (71B273D0h)]
    //0000000071AD9604 33 D2                xor         edx,edx
    //0000000055709606 48 83 C4 28          add         rsp,28h
    //000000005570960A 48 FF 25 47 6E 02 00 jmp         qword ptr [__imp_HeapSize (55730458h)]


    TestControl g_b0A00766fExpandInfo[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fExpandTest[_countof(g_b0A00766fExpandInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x57, 0x48, 0x83, 0xec, 0x30 };
    Byte g_b0A00766fExpandSave[_countof(g_b0A00766fExpandInfo)];
    //00000000569892E4 48 89 5C 24 10       mov         qword ptr [rsp+10h],rbx
    //00000000569892E9 48 89 6C 24 18       mov         qword ptr [rsp+18h],rbp
    //00000000569892EE 48 89 74 24 20       mov         qword ptr [rsp+20h],rsi
    //00000000569892F3 57                   push        rdi
    //00000000569892F4 48 83 EC 30          sub         rsp,30h
    //00000000569892F8 48 8B DA             mov         rbx,rdx
    //00000000569892FB 48 8B F9             mov         rdi,rcx

    TestControl g_b0A00766fOperatorNewCtrl[] =                       { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,
        VEQU, VEQU, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU  };
    Byte g_b0A00766fOperatorNewTest[_countof(g_b0A00766fOperatorNewCtrl)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x40, 0x48, 0x8b, 0xd9, 0xeb, 0x0f, 0x48, 0x8b, 0xcb, 0xe8, 0xd5, 0x65, 0xfb, 0xff, 0x85, 0xc0, 0x74, 0x13, 0x48, 0x8b,
        0xcb, 0xe8, 0xe1, 0xfe, 0xff, 0xff, 0x48, 0x85, 0xc0, 0x74, 0xe7, 0x48, 0x83, 0xC4, 0x40, 0x5B, 0xc3 };
    Byte g_b0A00766fOperatorNewSave[_countof(g_b0A00766fOperatorNewCtrl)];
    //0000000056BB8CEC 40 53                push        rbx
    //0000000056BB8CEE 48 83 EC 40          sub         rsp,40h
    //0000000056BB8CF2 48 8B D9             mov         rbx,rcx
    //0000000056BB8CF5 EB 0F                jmp         operator new+1Ah (56BB8D06h)
    //0000000056BB8CF7 48 8B CB             mov         rcx,rbx
    //0000000056BB8CFA E8  D5 65 FB FF       call        _callnewh (56B6F2D4h)
    //0000000056BB8CFF 85 C0                test        eax,eax
    //0000000056BB8D01 74 13                je          operator new+2Ah (56BB8D16h)
    //000000005EF48D03 48 8B CB             mov         rcx,rbx
    //000000005EF48D06 E8 E1 FE FF FF       call        malloc (5EF48BECh)
    //000000005EF48D0B 48 85 C0             test        rax,rax
    //000000005EF48D0E 74 E7                je          operator new+0Bh (5EF48CF7h)
    //000000005EF48D10 48 83 C4 40          add         rsp,40h
    //000000005EF48D14 5B                   pop         rbx
    //000000005EF48D15 C3                   ret

    //HACK:  This function is tiny, so we may have to steal reserved space from MS Detours (that's the 0xCCCCCCCCCCCCCC), and pray nobody patches
    //the next function, set_malloc_crt_max_wait
    TestControl g_b0A00766fOperatorDeleteCtrl[] =                          { VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fOperatorDeleteTest[_countof(g_b0A00766fOperatorDeleteCtrl)] = { 0xe9, 0x03, 0x03, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc };
    Byte g_b0A00766fOperatorDeleteSave[_countof(g_b0A00766fOperatorDeleteCtrl)];
    //0000000056BB89A0 E9 03 03 00 00       jmp         free (56BB8CA8h)
    //0000000056BB89A5 CC                   int         3
    //0000000056BB89A6 CC                   int         3
    //0000000056BB89A7 CC                   int         3
    //0000000056BB89A8 CC                   int         3
    //0000000056BB89A9 CC                   int         3
    //0000000056BB89AA CC                   int         3
    //0000000056BB89AB CC                   int         3

    TestControl g_b0A00766fHeapWalkCtrl[] =                    { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fHeapWalkTest[_countof(g_b0A00766fHeapWalkCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x48, 0x83, 0xec, 0x50, 0x48, 0x8b, 0xf9 };
    Byte g_b0A00766fHeapWalkSave[_countof(g_b0A00766fHeapWalkCtrl)];
    //0000000056989488 48 89 5C 24 18       mov         qword ptr [rsp+18h],rbx
    //000000005698948D 48 89 4C 24 08       mov         qword ptr [rsp+8],rcx
    //0000000056989492 56                   push        rsi
    //0000000056989493 57                   push        rdi
    //0000000056989494 41 54                push        r12
    //0000000056989496 48 83 EC 50          sub         rsp,50h
    //000000005698949A 48 8B F9             mov         rdi,rcx

    TestControl g_b0A00766fStrdupCtrl[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fStrdupTest[_countof(g_b0A00766fStrdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0A00766fStrdupSave[_countof(g_b0A00766fStrdupCtrl)];
    //0000000056955FFC 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //0000000056956001 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //0000000056956006 57                   push        rdi
    //0000000056956007 48 83 EC 30          sub         rsp,30h
    //000000005695600B 48 8B D9             mov         rbx,rcx

    TestControl g_b0A00766fWcsdupCtrl[] =                  { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0A00766fWcsdupTest[_countof(g_b0A00766fWcsdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0A00766fWcsdupSave[_countof(g_b0A00766fWcsdupCtrl)];
    //0000000056956A90 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx
    //0000000056956A95 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi
    //0000000056956A9A 57                   push        rdi
    //0000000056956A9B 48 83 EC 30          sub         rsp,30h
    //0000000056956A9F 48 8B D9             mov         rbx,rcx
#pragma endregion
#pragma region Visual Studio 11 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 11.0.51106.1   VC 11.0  (30319=C7A2)*/
    TestControl g_b0B00C7A2MallocInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2MallocTest[_countof(g_b0B00C7A2MallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9 };
    Byte g_b0B00C7A2MallocSave[_countof(g_b0B00C7A2MallocInfo)];
    //_malloc
    //000007FEF054635C 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0546361 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF0546366 57                   push        rdi  
    //000007FEF0546367 48 83 EC 20          sub         rsp,20h  
    //000007FEF054636B 48 8B D9             mov         rbx,rcx  

    TestControl g_b0B00C7A2CallocInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2CallocTest[_countof(g_b0B00C7A2CallocInfo)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x83, 0x64, 0x24, 0x40, 0x00, 0x4c, 0x8d, 0x44, 0x24, 0x40 };
    Byte g_b0B00C7A2CallocSave[_countof(g_b0B00C7A2CallocInfo)];
    //_calloc
    //000007FEF0546660 40 53                push        rbx  
    //000007FEF0546662 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546666 83 64 24 40 00       and         dword ptr [rsp+40h],0  
    //000007FEF054666B 4C 8D 44 24 40       lea         r8,[rsp+40h]  

    TestControl g_b0B00C7A2ReallocInfo[] =                          { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2ReallocTest[_countof(g_b0B00C7A2ReallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xda, 0x48, 0x8b, 0xf9 };
    Byte g_b0B00C7A2ReallocSave[_countof(g_b0B00C7A2ReallocInfo)];
    //_realloc
    //000007FEF0546F94 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0546F99 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF0546F9E 57                   push        rdi  
    //000007FEF0546F9F 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546FA3 48 8B DA             mov         rbx,rdx  
    //000007FEF0546FA6 48 8B F9             mov         rdi,rcx  

    TestControl g_b0B00C7A2FreeInfo[] =                       { VEQU, VEQU, VEQU, VEQU, VOFF, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS,
     VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0B00C7A2FreeTest[_countof(g_b0B00C7A2FreeInfo)] = { 0x48, 0x85, 0xc9, 0x74, 0x37, 0x53, 0x48, 0x83, 0xec, 0x20, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0x14, 0xe7, 0x04, 0x00, 0x33, 0xd2, 0xff, 0x15, 0x5c, 0x77, 0x02, 0x00,
     0x85, 0xc0, 0x75, 0x17, 0xe8, 0xdf, 0xc9, 0xfb, 0xff, 0x48, 0x8b, 0xd8, 0xff, 0x15, 0xda, 0x73, 0x02, 0x00, 0x8b, 0xc8, 0xe8, 0x7f, 0xc9, 0xfb, 0xff, 0x89, 0x03, 0x48, 0x83, 0xc4, 0x20, 0x5b, 0xc3 };
    Byte g_b0B00C7A2FreeSave[_countof(g_b0B00C7A2FreeInfo)];
    //000007FEF054631C 48 85 C9             test        rcx,rcx  
    //000007FEF054631F 74 37                je          free+3Ch (7FEF0546358h)  
    //000007FEF0546321 53                   push        rbx  
    //000007FEF0546322 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546326 4C 8B C1             mov         r8,rcx  
    //000007FEF0546329 48 8B 0D 60 02 06 00 mov         rcx,qword ptr [_crtheap (7FEF05A6590h)]  
    //000007FEF0546330 33 D2                xor         edx,edx  
    //000007FEF0546332 FF 15 A8 D0 02 00    call        qword ptr [__imp_HeapFree (7FEF05733E0h)]  
    //000007FEF0546338 85 C0                test        eax,eax  
    //000007FEF054633A 75 17                jne         free+37h (7FEF0546353h)  
    //000007FEF054633C E8 9F 13 FC FF       call        _errno (7FEF05076E0h)  
    //000007FEF0546341 48 8B D8             mov         rbx,rax  
    //000007FEF0546344 FF 15 E6 CC 02 00    call        qword ptr [__imp_GetLastError (7FEF0573030h)]  
    //000007FEF054634A 8B C8                mov         ecx,eax  
    //000007FEF054634C E8 27 14 FC FF       call        _get_errno_from_oserr (7FEF0507778h)  
    //000007FEF0546351 89 03                mov         dword ptr [rbx],eax  
    //000007FEF0546353 48 83 C4 20          add         rsp,20h  
    //000007FEF0546357 5B                   pop         rbx  
    //000007FEF0546358 C3                   ret  

    TestControl g_b0B00C7A2MsizeInfo[] =                        {VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU,
        VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU/*, VEQU, VEQU, VEQU, VOFF, VOFF, VOFF, VOFF*/};
    Byte g_b0B00C7A2MsizeTest[_countof(g_b0B00C7A2MsizeInfo)] = {0x48, 0x83, 0xec, 0x28, 0x48, 0x85, 0xc9, 0x75, 0x19, 0xe8, 0xc6, 0xc0, 0xfb, 0xff, 0xc7, 0x00, 0x16, 0x00, 0x00, 0x00, 0xe8, 0x33, 0x6e, 0x00, 0x00, 0x48, 0x83,
        0xc8, 0xff, 0x48, 0x83, 0xc4, 0x28, 0xc3, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0xcc, 0xdd, 0x04, 0x00, 0x33, 0xd2, 0x48, 0x83, 0xc4, 0x28/*, 0x48, 0xff, 0x25, 0x47, 0x6e, 0x02, 0x00*/};
    Byte g_b0B00C7A2MsizeSave[_countof(g_b0B00C7A2MsizeInfo)];
    //000007FEF0546DAC 48 83 EC 28          sub         rsp,28h  
    //000007FEF0546DB0 48 85 C9             test        rcx,rcx  
    //000007FEF0546DB3 75 19                jne         _msize+22h (7FEF0546DCEh)  
    //000007FEF0546DB5 E8 26 09 FC FF       call        _errno (7FEF05076E0h)  
    //000007FEF0546DBA C7 00 16 00 00 00    mov         dword ptr [rax],16h  
    //000007FEF0546DC0 E8 B7 66 00 00       call        _invalid_parameter_noinfo (7FEF054D47Ch)  
    //000007FEF0546DC5 48 83 C8 FF          or          rax,0FFFFFFFFFFFFFFFFh  
    //000007FEF0546DC9 48 83 C4 28          add         rsp,28h  
    //000007FEF0546DCD C3                   ret  
    //000007FEF0546DCE 4C 8B C1             mov         r8,rcx  
    //000007FEF0546DD1 48 8B 0D B8 F7 05 00 mov         rcx,qword ptr [_crtheap (7FEF05A6590h)]  
    //000007FEF0546DD8 33 D2                xor         edx,edx  
    //000007FEF0546DDA 48 83 C4 28          add         rsp,28h  
    //000007FEF0546DDE 48 FF 25 1B C6 02 00 jmp         qword ptr [__imp_HeapSize (7FEF0573400h)]  

    TestControl g_b0B00C7A2ExpandInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2ExpandTest[_countof(g_b0B00C7A2ExpandInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x57, 0x48, 0x83, 0xec, 0x30 };
    Byte g_b0B00C7A2ExpandSave[_countof(g_b0B00C7A2ExpandInfo)];
    //000007FEF0546ADC 48 89 5C 24 10       mov         qword ptr [rsp+10h],rbx  
    //000007FEF0546AE1 48 89 6C 24 18       mov         qword ptr [rsp+18h],rbp  
    //000007FEF0546AE6 48 89 74 24 20       mov         qword ptr [rsp+20h],rsi  
    //000007FEF0546AEB 57                   push        rdi  
    //000007FEF0546AEC 48 83 EC 30          sub         rsp,30h  
    //000007FEF0546AF0 48 8B DA             mov         rbx,rdx  
    //000007FEF0546AF3 48 8B F9             mov         rdi,rcx  

    TestControl g_b0B00C7A2OperatorNewCtrl[] =                              { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,
        VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU  };
    Byte g_b0B00C7A2OperatorNewTest[_countof(g_b0B00C7A2OperatorNewCtrl)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x40, 0x48, 0x8b, 0xd9, 0xeb, 0x0f, 0x48, 0x8b, 0xcb, 0xe8, 0x4d, 0xb1, 0xfb, 0xff, 0x85, 0xc0, 0x74, 0x13, 0x48, 0x8b,
        0xcb, 0xe8, 0x95, 0x00, 0x00, 0x00, 0x48, 0x85, 0xc0, 0x74, 0xe7, 0x48, 0x83, 0xC4, 0x40, 0x5B, 0xc3 };
    Byte g_b0B00C7A2OperatorNewSave[_countof(g_b0B00C7A2OperatorNewCtrl)];
    //000007FEF05462A8 40 53                push        rbx  
    //000007FEF05462AA 48 83 EC 40          sub         rsp,40h  
    //000007FEF05462AE 48 8B D9             mov         rbx,rcx  
    //000007FEF05462B1 EB 0F                jmp         operator new+1Ah (7FEF05462C2h)  
    //000007FEF05462B3 48 8B CB             mov         rcx,rbx  
    //000007FEF05462B6 E8 4D B1 FB FF       call        _callnewh (7FEF0501408h)  
    //000007FEF05462BB 85 C0                test        eax,eax  
    //000007FEF05462BD 74 13                je          operator new+2Ah (7FEF05462D2h)  
    //000007FEF05462BF 48 8B CB             mov         rcx,rbx  
    //000007FEF05462C2 E8 95 00 00 00       call        malloc (7FEF054635Ch)  
    //000007FEF05462C7 48 85 C0             test        rax,rax  
    //000007FEF05462CA 74 E7                je          operator new+0Bh (7FEF05462B3h)  
    //000007FEF05462CC 48 83 C4 40          add         rsp,40h  
    //000007FEF05462D0 5B                   pop         rbx  
    //000007FEF05462D1 C3                   ret  


    // VC11 X64 OPERATOR DELETE PATCH:
    //
    // This runtime does not have enough room for us to patch in a jump for 'operator delete'.
    // We need 12 bytes (i.e. enough for mov rax, imm64 / jmp rax) and there are only 8 available
    // in this runtime before the next entry point.  However, there is an unused 12-Byte 
    // section nearby where we an insert a patch to the pagalloc delete function.
    //
    // Before we apply any of our patch functions, we modify the CRT 'operator delete' from
    // this:
    //
    //   operator delete:
    //   000007FEEFB26314 E9 03 00 00 00                 jmp         free (7FEEFB2631Ch) ; CRT implementation is 32-bit relative jump to 'free'
    //
    // To this:
    //
    //   operator delete:
    //   000007FEEFB26314 E9 2F FF FF FF                 jmp         7FEEFB26248h        ; 32-bit relative jump to unused region 
    //
    //   000007FEEFB26248 48 B8 1C 63 B2 EF FE 07 00 00  mov         rax,offset free     ; 64-bit absolute jump to 'free' 
    //   000007FEEFB26252 FF E0                          jmp         rax  
    //
    //
    // From then on, we treat the 12-Byte region as if it is the exported 'operator delete' address.

const static int32_t s_vc11_offset_operatorDeleteExportToJmp64Area = -204;    

    TestControl g_b0B00C7A2OperatorDeleteStubCtrl[] =                                     { VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2OperatorDeleteStubTest[_countof(g_b0B00C7A2OperatorDeleteStubCtrl)] = { 0xe9, 0x03, 0x03, 0x00, 0x00, 0xcc, 0xcc, 0xcc };
    //000007FEF0426314 E9 03 00 00 00       jmp         free (7FEF054631Ch)  
    //000007FEF0426319 CC                   int         3  
    //000007FEF042631A CC                   int         3  
    //000007FEF042631B CC                   int         3  

    // We found 12 bytes available here at the specified offset from the 'operator delete' export
    TestControl g_b0B00C7A2OperatorDeleteCtrl[] =                                 { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2OperatorDeleteTest[_countof(g_b0B00C7A2OperatorDeleteCtrl)] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,0xcc, 0xcc, 0xcc, 0xcc };
    Byte g_b0B00C7A2OperatorDeleteSave[_countof(g_b0B00C7A2OperatorDeleteCtrl)];
    //000007FEF0426248 CC                   int         3  
    //000007FEF0426249 CC                   int         3  
    //000007FEF042624A CC                   int         3  
    //000007FEF042624B CC                   int         3  
    //000007FEF042624C CC                   int         3  
    //000007FEF042624D CC                   int         3  
    //000007FEF042624E CC                   int         3  
    //000007FEF042624F CC                   int         3  
    //000007FEF0426250 CC                   int         3  
    //000007FEF0426251 CC                   int         3  
    //000007FEF0426252 CC                   int         3  
    //000007FEF0426253 CC                   int         3 

    TestControl g_b0B00C7A2HeapWalkCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2HeapWalkTest[_countof(g_b0B00C7A2HeapWalkCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x56, 0x57, 0x41, 0x56, 0x48, 0x83, 0xec, 0x50, 0x48, 0x8b, 0xf9 };
    Byte g_b0B00C7A2HeapWalkSave[_countof(g_b0B00C7A2HeapWalkCtrl)];
    //000007FEF0546C64 48 89 5C 24 18       mov         qword ptr [rsp+18h],rbx  
    //000007FEF0546C69 48 89 4C 24 08       mov         qword ptr [rsp+8],rcx  
    //000007FEF0546C6E 56                   push        rsi  
    //000007FEF0546C6F 57                   push        rdi  
    //000007FEF0546C70 41 56                push        r14  
    //000007FEF0546C72 48 83 EC 50          sub         rsp,50h  
    //000007FEF0546C76 48 8B F9             mov         rdi,rcx  

    TestControl g_b0B00C7A2StrdupCtrl[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2StrdupTest[_countof(g_b0B00C7A2StrdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0B00C7A2StrdupSave[_countof(g_b0B00C7A2StrdupCtrl)];
    //000007FEF05165D4 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF05165D9 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF05165DE 57                   push        rdi  
    //000007FEF05165DF 48 83 EC 30          sub         rsp,30h  
    //000007FEF05165E3 48 8B D9             mov         rbx,rcx  

    TestControl g_b0B00C7A2WcsdupCtrl[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0B00C7A2WcsdupTest[_countof(g_b0B00C7A2WcsdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0B00C7A2WcsdupSave[_countof(g_b0B00C7A2WcsdupCtrl)];
    //000007FEF0517750 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0517755 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF051775A 57                   push        rdi  
    //000007FEF051775B 48 83 EC 30          sub         rsp,30h  
    //000007FEF051775F 48 8B D9             mov         rbx,rcx  
#pragma endregion
#pragma region Visual Studio 12 Patch Table
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* 12.0.21005.1   VC 12.0  (21005=520d)*/
    TestControl g_b0C00520DMallocInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DMallocTest[_countof(g_b0C00520DMallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9 };
    Byte g_b0C00520DMallocSave[_countof(g_b0C00520DMallocInfo)];
    //_malloc
    //000007FEF054635C 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0546361 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF0546366 57                   push        rdi  
    //000007FEF0546367 48 83 EC 20          sub         rsp,20h  
    //000007FEF054636B 48 8B D9             mov         rbx,rcx  

    TestControl g_b0C00520DCallocInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DCallocTest[_countof(g_b0C00520DCallocInfo)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x83, 0x64, 0x24, 0x40, 0x00, 0x4c, 0x8d, 0x44, 0x24, 0x40 };
    Byte g_b0C00520DCallocSave[_countof(g_b0C00520DCallocInfo)];
    //_calloc
    //000007FEF0546660 40 53                push        rbx  
    //000007FEF0546662 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546666 83 64 24 40 00       and         dword ptr [rsp+40h],0  
    //000007FEF054666B 4C 8D 44 24 40       lea         r8,[rsp+40h]  

    TestControl g_b0C00520DReallocInfo[] =                          { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DReallocTest[_countof(g_b0C00520DReallocInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xda, 0x48, 0x8b, 0xf9 };
    Byte g_b0C00520DReallocSave[_countof(g_b0C00520DReallocInfo)];
    //_realloc
    //000007FEF0546F94 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0546F99 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF0546F9E 57                   push        rdi  
    //000007FEF0546F9F 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546FA3 48 8B DA             mov         rbx,rdx  
    //000007FEF0546FA6 48 8B F9             mov         rdi,rcx  

    TestControl g_b0C00520DFreeInfo[] =                       { VEQU, VEQU, VEQU, VEQU, VOFF, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS,
     VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VCRP, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU};
    Byte g_b0C00520DFreeTest[_countof(g_b0C00520DFreeInfo)] = { 0x48, 0x85, 0xc9, 0x74, 0x37, 0x53, 0x48, 0x83, 0xec, 0x20, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0x14, 0xe7, 0x04, 0x00, 0x33, 0xd2, 0xff, 0x15, 0x5c, 0x77, 0x02, 0x00,
     0x85, 0xc0, 0x75, 0x17, 0xe8, 0xdf, 0xc9, 0xfb, 0xff, 0x48, 0x8b, 0xd8, 0xff, 0x15, 0xda, 0x73, 0x02, 0x00, 0x8b, 0xc8, 0xe8, 0x7f, 0xc9, 0xfb, 0xff, 0x89, 0x03, 0x48, 0x83, 0xc4, 0x20, 0x5b, 0xc3 };
    Byte g_b0C00520DFreeSave[_countof(g_b0C00520DFreeInfo)];
    //000007FEF054631C 48 85 C9             test        rcx,rcx  
    //000007FEF054631F 74 37                je          free+3Ch (7FEF0546358h)  
    //000007FEF0546321 53                   push        rbx  
    //000007FEF0546322 48 83 EC 20          sub         rsp,20h  
    //000007FEF0546326 4C 8B C1             mov         r8,rcx  
    //000007FEF0546329 48 8B 0D 60 02 06 00 mov         rcx,qword ptr [_crtheap (7FEF05A6590h)]  
    //000007FEF0546330 33 D2                xor         edx,edx  
    //000007FEF0546332 FF 15 A8 D0 02 00    call        qword ptr [__imp_HeapFree (7FEF05733E0h)]  
    //000007FEF0546338 85 C0                test        eax,eax  
    //000007FEF054633A 75 17                jne         free+37h (7FEF0546353h)  
    //000007FEF054633C E8 9F 13 FC FF       call        _errno (7FEF05076E0h)  
    //000007FEF0546341 48 8B D8             mov         rbx,rax  
    //000007FEF0546344 FF 15 E6 CC 02 00    call        qword ptr [__imp_GetLastError (7FEF0573030h)]  
    //000007FEF054634A 8B C8                mov         ecx,eax  
    //000007FEF054634C E8 27 14 FC FF       call        _get_errno_from_oserr (7FEF0507778h)  
    //000007FEF0546351 89 03                mov         dword ptr [rbx],eax  
    //000007FEF0546353 48 83 C4 20          add         rsp,20h  
    //000007FEF0546357 5B                   pop         rbx  
    //000007FEF0546358 C3                   ret  

    TestControl g_b0C00520DMsizeInfo[] =                        {VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VOFF, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VC32, VABS, VABS, VABS, VABS, VEQU, VEQU,
        VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VMCP, VEQU, VEQU, VABS, VABS, VABS, VABS, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU/*, VEQU, VEQU, VEQU, VOFF, VOFF, VOFF, VOFF*/};
    Byte g_b0C00520DMsizeTest[_countof(g_b0C00520DMsizeInfo)] = {0x48, 0x83, 0xec, 0x28, 0x48, 0x85, 0xc9, 0x75, 0x19, 0xe8, 0xc6, 0xc0, 0xfb, 0xff, 0xc7, 0x00, 0x16, 0x00, 0x00, 0x00, 0xe8, 0x33, 0x6e, 0x00, 0x00, 0x48, 0x83,
        0xc8, 0xff, 0x48, 0x83, 0xc4, 0x28, 0xc3, 0x4c, 0x8b, 0xc1, 0x48, 0x8b, 0x0d, 0xcc, 0xdd, 0x04, 0x00, 0x33, 0xd2, 0x48, 0x83, 0xc4, 0x28/*, 0x48, 0xff, 0x25, 0x47, 0x6e, 0x02, 0x00*/};
    Byte g_b0C00520DMsizeSave[_countof(g_b0C00520DMsizeInfo)];
    //000007FEF0546DAC 48 83 EC 28          sub         rsp,28h  
    //000007FEF0546DB0 48 85 C9             test        rcx,rcx  
    //000007FEF0546DB3 75 19                jne         _msize+22h (7FEF0546DCEh)  
    //000007FEF0546DB5 E8 26 09 FC FF       call        _errno (7FEF05076E0h)  
    //000007FEF0546DBA C7 00 16 00 00 00    mov         dword ptr [rax],16h  
    //000007FEF0546DC0 E8 B7 66 00 00       call        _invalid_parameter_noinfo (7FEF054D47Ch)  
    //000007FEF0546DC5 48 83 C8 FF          or          rax,0FFFFFFFFFFFFFFFFh  
    //000007FEF0546DC9 48 83 C4 28          add         rsp,28h  
    //000007FEF0546DCD C3                   ret  
    //000007FEF0546DCE 4C 8B C1             mov         r8,rcx  
    //000007FEF0546DD1 48 8B 0D B8 F7 05 00 mov         rcx,qword ptr [_crtheap (7FEF05A6590h)]  
    //000007FEF0546DD8 33 D2                xor         edx,edx  
    //000007FEF0546DDA 48 83 C4 28          add         rsp,28h  
    //000007FEF0546DDE 48 FF 25 1B C6 02 00 jmp         qword ptr [__imp_HeapSize (7FEF0573400h)]  

    TestControl g_b0C00520DExpandInfo[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DExpandTest[_countof(g_b0C00520DExpandInfo)] = { 0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x57, 0x48, 0x83, 0xec, 0x30 };
    Byte g_b0C00520DExpandSave[_countof(g_b0C00520DExpandInfo)];
    //000007FEF0546ADC 48 89 5C 24 10       mov         qword ptr [rsp+10h],rbx  
    //000007FEF0546AE1 48 89 6C 24 18       mov         qword ptr [rsp+18h],rbp  
    //000007FEF0546AE6 48 89 74 24 20       mov         qword ptr [rsp+20h],rsi  
    //000007FEF0546AEB 57                   push        rdi  
    //000007FEF0546AEC 48 83 EC 30          sub         rsp,30h  
    //000007FEF0546AF0 48 8B DA             mov         rbx,rdx  
    //000007FEF0546AF3 48 8B F9             mov         rdi,rcx  

    TestControl g_b0C00520DOperatorNewCtrl[] =                              { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,
        VEQU, VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU  };
    Byte g_b0C00520DOperatorNewTest[_countof(g_b0C00520DOperatorNewCtrl)] = { 0x40, 0x53, 0x48, 0x83, 0xec, 0x40, 0x48, 0x8b, 0xd9, 0xeb, 0x0f, 0x48, 0x8b, 0xcb, 0xe8, 0x4d, 0xb1, 0xfb, 0xff, 0x85, 0xc0, 0x74, 0x13, 0x48, 0x8b,
        0xcb, 0xe8, 0x95, 0x00, 0x00, 0x00, 0x48, 0x85, 0xc0, 0x74, 0xe7, 0x48, 0x83, 0xC4, 0x40, 0x5B, 0xc3 };
    Byte g_b0C00520DOperatorNewSave[_countof(g_b0C00520DOperatorNewCtrl)];
    //000007FEF05462A8 40 53                push        rbx  
    //000007FEF05462AA 48 83 EC 40          sub         rsp,40h  
    //000007FEF05462AE 48 8B D9             mov         rbx,rcx  
    //000007FEF05462B1 EB 0F                jmp         operator new+1Ah (7FEF05462C2h)  
    //000007FEF05462B3 48 8B CB             mov         rcx,rbx  
    //000007FEF05462B6 E8 4D B1 FB FF       call        _callnewh (7FEF0501408h)  
    //000007FEF05462BB 85 C0                test        eax,eax  
    //000007FEF05462BD 74 13                je          operator new+2Ah (7FEF05462D2h)  
    //000007FEF05462BF 48 8B CB             mov         rcx,rbx  
    //000007FEF05462C2 E8 95 00 00 00       call        malloc (7FEF054635Ch)  
    //000007FEF05462C7 48 85 C0             test        rax,rax  
    //000007FEF05462CA 74 E7                je          operator new+0Bh (7FEF05462B3h)  
    //000007FEF05462CC 48 83 C4 40          add         rsp,40h  
    //000007FEF05462D0 5B                   pop         rbx  
    //000007FEF05462D1 C3                   ret  


    // VC11 X64 OPERATOR DELETE PATCH:
    //
    // This runtime does not have enough room for us to patch in a jump for 'operator delete'.
    // We need 12 bytes (i.e. enough for mov rax, imm64 / jmp rax) and there are only 8 available
    // in this runtime before the next entry point.  However, there is an unused 12-Byte 
    // section nearby where we an insert a patch to the pagalloc delete function.
    //
    // Before we apply any of our patch functions, we modify the CRT 'operator delete' from
    // this:
    //
    //   operator delete:
    //   000007FEEFB26314 E9 03 00 00 00                 jmp         free (7FEEFB2631Ch) ; CRT implementation is 32-bit relative jump to 'free'
    //
    // To this:
    //
    //   operator delete:
    //   000007FEEFB26314 E9 2F FF FF FF                 jmp         7FEEFB26248h        ; 32-bit relative jump to unused region 
    //
    //   000007FEEFB26248 48 B8 1C 63 B2 EF FE 07 00 00  mov         rax,offset free     ; 64-bit absolute jump to 'free' 
    //   000007FEEFB26252 FF E0                          jmp         rax  
    //
    //
    // From then on, we treat the 12-Byte region as if it is the exported 'operator delete' address.

const static int32_t s_vc12_offset_operatorDeleteExportToJmp64Area = -204;    

    TestControl g_b0C00520DOperatorDeleteStubCtrl[] =                                     { VEQU, V___, V___, V___, V___, VEQU, VEQU, VEQU };
    Byte g_b0C00520DOperatorDeleteStubTest[_countof(g_b0C00520DOperatorDeleteStubCtrl)] = { 0xe9, 0x03, 0x03, 0x00, 0x00, 0xcc, 0xcc, 0xcc };
    //000007FEF0426314 E9 03 00 00 00       jmp         free (7FEF054631Ch)  
    //000007FEF0426319 CC                   int         3  
    //000007FEF042631A CC                   int         3  
    //000007FEF042631B CC                   int         3  

    // We found 12 bytes available here at the specified offset from the 'operator delete' export
    TestControl g_b0C00520DOperatorDeleteCtrl[] =                                 { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU,VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DOperatorDeleteTest[_countof(g_b0C00520DOperatorDeleteCtrl)] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,0xcc, 0xcc, 0xcc, 0xcc };
    Byte g_b0C00520DOperatorDeleteSave[_countof(g_b0C00520DOperatorDeleteCtrl)];
    //000007FEF0426248 CC                   int         3  
    //000007FEF0426249 CC                   int         3  
    //000007FEF042624A CC                   int         3  
    //000007FEF042624B CC                   int         3  
    //000007FEF042624C CC                   int         3  
    //000007FEF042624D CC                   int         3  
    //000007FEF042624E CC                   int         3  
    //000007FEF042624F CC                   int         3  
    //000007FEF0426250 CC                   int         3  
    //000007FEF0426251 CC                   int         3  
    //000007FEF0426252 CC                   int         3  
    //000007FEF0426253 CC                   int         3 

    TestControl g_b0C00520DHeapWalkCtrl[] =                           { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DHeapWalkTest[_countof(g_b0C00520DHeapWalkCtrl)] = { 0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x18, 0x48, 0x89, 0x70, 0x20, 0x48, 0x89, 0x48, 0x08, 0x57, 0x41, 0x56, 0x41, 0x57 };
    Byte g_b0C00520DHeapWalkSave[_countof(g_b0C00520DHeapWalkCtrl)];
    //000007FEF1FD7310 48 8B C4             mov         rax,rsp  
    //000007FEF1FD7313 48 89 58 18          mov         qword ptr [rax+18h],rbx 
    //000007FEF1FD7317 48 89 70 20          mov         qword ptr [rax+20h],rsi  
    //000007FEF1FD731B 48 89 48 08          mov         qword ptr [rax+8],rcx  
    //000007FEF1FD731F 57                   push        rdi  
    //000007FEF1FD7320 41 56                push        r14  
    //000007FEF1FD7322 41 57                push        r15  

    TestControl g_b0C00520DStrdupCtrl[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DStrdupTest[_countof(g_b0C00520DStrdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0C00520DStrdupSave[_countof(g_b0C00520DStrdupCtrl)];
    //000007FEF05165D4 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF05165D9 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF05165DE 57                   push        rdi  
    //000007FEF05165DF 48 83 EC 30          sub         rsp,30h  
    //000007FEF05165E3 48 8B D9             mov         rbx,rcx  

    TestControl g_b0C00520DWcsdupCtrl[] =                         { VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU, VEQU };
    Byte g_b0C00520DWcsdupTest[_countof(g_b0C00520DWcsdupCtrl)] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x8b, 0xd9 };
    Byte g_b0C00520DWcsdupSave[_countof(g_b0C00520DWcsdupCtrl)];
    //000007FEF0517750 48 89 5C 24 08       mov         qword ptr [rsp+8],rbx  
    //000007FEF0517755 48 89 74 24 10       mov         qword ptr [rsp+10h],rsi  
    //000007FEF051775A 57                   push        rdi  
    //000007FEF051775B 48 83 EC 30          sub         rsp,30h  
    //000007FEF051775F 48 8B D9             mov         rbx,rcx  
#pragma endregion
#else
#error No patch tables available for current architecture: see papatch.cpp
#endif
#pragma endregion
#endif // !defined (PAPATCH_No_MSCVRT_Patch_Tables)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    George.Dulchinos                12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Byte * findRun                      // <= run location or NULL
(
Byte    const * const startP,       // => start search from here
Byte    const * const endP,         // => end search here
Byte    value,                      // => value to serach for
int     runLength                   // => length of run
)
    {
    Byte * p = (Byte*) startP;

    while (p < endP)
        {
        int cnt = runLength;
        Byte * beginP = p;

        while (value == *p++)
            {

            if (0 == --cnt)
                return beginP;
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    George.Dulchinos01/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t isFuncPatchable             /* <= true if patchable */
(
Byte const * const crtFuncP,             /* => address of CRT function to patch */
TestControl const * const ctrlTable,            /* => mask table */
Byte const * const testTable,            /* => test table */
int32_t             const tableLen              /* => test/mask table length */
)
    {
    int32_t          i;
    Byte const * p;

    for (p=crtFuncP, i=0; i<tableLen; i++, p++)
        {
        if (VEQU & ctrlTable[i])
            {
            if (testTable[i] != *p)
                return false;
            }
        }
    return true;
    }

#if defined(_X86_)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
//X86-specific implementation of createPatchDataEntry. See createPatchDataEntry for description.
static void createPatchDataEntryX86
(
Byte * const replacedCodeP,       /* <= code to replace that overwritten by 'jmp' */
Byte const  * const crtFuncP,            /* => address of CRT function that will be patched */
TestControl const  * const relocTable,          /* => indicates if embedded addr needs reloc. */
int32_t              const patchBytes           /* => no. bytes to move into replaced code */
)
    {
    int32_t              i;
    int32_t const jumpDelta  = (int32_t)(crtFuncP - replacedCodeP);
    int32_t      jumpDest;
    int32_t const JUMP_INSTR_LEN = 5;
    /* Copy bytes to be replaced; if necessary relocate addresses */
    memcpy (replacedCodeP, crtFuncP, patchBytes);
    for (i=0; i<patchBytes; i++)
        {
        if (relocTable[i] & V___)
            {
            uint32_t * const p = (uint32_t *) (replacedCodeP + i);
            jumpDest = *p;
            jumpDest += jumpDelta;
            *p = jumpDest;
            /* Move forward past the end of the address */
            i += 3;
            }
        }

    /* Now follow copied bytes with a jump back to the c runtime */
    jumpDest = jumpDelta - JUMP_INSTR_LEN;
    /* 0xe9 is a long relative jump in x86*/
    *(replacedCodeP+patchBytes) = 0xe9;
    memcpy (replacedCodeP+patchBytes+1, &jumpDest, sizeof(int32_t));
    }
#endif

#if defined(_M_X64)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
//Take a 32-bit relative call and expand it to a 64-bit absolute call, so if we're nowhere near the CRT we can still call it.
static void expandCall32(Byte* const replacedCodeP, const TestControl* const relocTable, int32_t patchBytes, int32_t* i, ptrdiff_t jumpDelta, int32_t * totalCodeSizeDiff)
    {
    /*Patching strategy:
        The nice thing about function calls is there's no way whatsoever the function can legally care about our rax.
        So, unless Microsoft did something asinine, we can shove our target address in rax and call it, not having to worry
        about blowing up some data. We need to insert a mov rax, imm64 instruction to position the address (0x48 0xb8 addr),
        then a good old call rax (0xff 0xd0) to take us to our happy place. Note: rewriting should be all-or-nothing, otherwise
        it's a pain to figure out what short jumps need to be changed and how.*/
    const int32_t newCodeSize = 12; // 0x48 0xb8 (8 Byte address), 0xff 0xd0 is 12 bytes long
    const int32_t oldCodeSize = 5; // 0xe8 (4 Byte offset) is 5 bytes long
    const int32_t codeSizeDiff = newCodeSize - oldCodeSize;
     Byte * ip = replacedCodeP + *i + *totalCodeSizeDiff; //Pointer to modified instruction
    //Make room for the new code
    memmove(ip+newCodeSize, (void*)(ip+oldCodeSize), patchBytes - (*i+oldCodeSize));
    //Destination is (end address of old instruction) -  (space consumed by other patches) + (offset recorded in op) + (distance to CRT)
    const uintptr_t callDest = (uintptr_t)(ip+oldCodeSize) -  *totalCodeSizeDiff + *((int32_t*)(ip+1)) + jumpDelta ;
    //Write out our machine code
    ip[0] = 0x48; ip[1] = 0xb8; // mov rax,
    *(uintptr_t*)(ip+2) = callDest;// imm64
    ip += 10; //next instruction
    ip[0] =0xff; ip[1] = 0xd0; // call rax
    //Update the total code size difference.
    *totalCodeSizeDiff += codeSizeDiff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
//Take an instruction that calls a function pointer, and rewrite it so it works from a high address.
static void expandCallOverPointer(Byte* const replacedCodeP,const TestControl* const relocTable, int32_t patchBytes, int32_t* i, ptrdiff_t jumpDelta, int32_t * totalCodeSizeDiff)
    {
    //Like with expanding a direct call, we can do whatever we want with rax. In this case, we will do mov rax, addr (0x48, 0xb8 ...), followed by call qword ptr [rax] (0xff 0x10).
    const int32_t newCodeSize = 12; //10 for move, two for call
    const int32_t oldCodeSize = 6; //2 for op, 4 for addr
    const int32_t codeSizeDiff = newCodeSize - oldCodeSize;
    Byte * ip = replacedCodeP + *i + *totalCodeSizeDiff;
    //Make room for the extended instruction.
    memmove(ip+newCodeSize, (void*)(ip+oldCodeSize), patchBytes - (*i+oldCodeSize));
    //Old end address of call instruction - change in code size + offset stored in instruction + change in function location = destination.
    const uintptr_t callDest = (uintptr_t)(ip+oldCodeSize) -  *totalCodeSizeDiff + *((int32_t*)(ip+2)) + jumpDelta ;
    //Write out our machine code
    ip[0] = 0x48; ip[1] = 0xb8; // mov rax,
    *(uintptr_t*)(ip+2) = callDest;// imm64
    ip += 10; //next instruction
    ip[0] =0xff; ip[1] = 0x10; // call qword ptr [rax]
    //Update the total code size difference.
    *totalCodeSizeDiff += codeSizeDiff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
//Take a 32-bit move offset to rcx over pointer and convert it to the equivalent 64-bit code that will work when the CRT is far away
static void expandMovCP(Byte* const replacedCodeP,const TestControl* const relocTable, int32_t patchBytes, int32_t* i, ptrdiff_t jumpDelta, int32_t* totalCodeSizeDiff)
    {
    /*Patching strategy: We know rcx is going to get blown up by mov rcx, ... , so we can store intermediate data there, namely the address
    to jump to. We're going to do a mov rcx, (imm64 computed address), which in machine code is 48 B9 (address). Then we'll do a mov rcx, qword ptr [rcx],  which is 48 8B 09 in machine code*/
    const int32_t newCodeSize =  13; // 0x48 + 0xb9 + 8 bytes + 0x48 + 0x8b +0x09 is 13 bytes.
    const int32_t oldCodeSize =   7;//48 8B 0D + 4 bytes is 7 bytes
    const int codeSizeDiff = newCodeSize - oldCodeSize;
    Byte* ip = replacedCodeP + *i + *totalCodeSizeDiff;
    //Make some room
    memmove(ip+newCodeSize, (void*)(ip+oldCodeSize), patchBytes - (*i+oldCodeSize));
    //Source is our address + distance to CRT + offset stored in op - space consumed by patching
    const uintptr_t moveSrc = oldCodeSize + (uintptr_t)ip + jumpDelta + *((int32_t*)(ip+3)) - *totalCodeSizeDiff;
    //Write our machine code
    ip[0]=0x48; ip[1]=0xb9;ip+=2;      // mov rcx, ...
    *(uintptr_t*)ip=moveSrc; ip+=8;    // mov rcx, sourceAddress
    ip[0]= 0x48; ip[1]=0x8b; ip[2]=0x09;// mov rcx, [rcx]
    //Update the total code size difference.
    *totalCodeSizeDiff += codeSizeDiff; //Number of extra bytes inserted
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
//Find the change in code size cause by patching a given instruction type.
static signed char GetPatchingSizeChange(TestControl t)                    // WIP_CHAR_OK
    {
    /*Explanation of magic numbers: Whenever we expand an instruction,
    the code gets bigger. Here are the changes in size for each operation.
    Given the current setup there's not an especially good place to get these
    from, so look at the expand* functions to see the source of each number.*/
    switch(t)
        {
    case VCRP:
        return 6;
    case VC32:
        return 7;
    case VMCP:
        return 6;
    default:
        return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
/* Adjust a short jump to account for the changes in code size that come with patching */
static void adjustOffsetForCodeResize(Byte* const replacedCodeP,const TestControl* const relocTable, int32_t patchBytes, int32_t* i, int32_t codeSizeDiff)
    {
    //Adjust the index to account for the increase in code size
    int32_t offsetI = *i+ codeSizeDiff;
    signed char offset = replacedCodeP[offsetI];                           // WIP_CHAR_OK
    int32_t iTarget = offsetI+offset + 1; //Index of target instruction in control table.
    if(iTarget <0)
        iTarget=0;
    if(iTarget>patchBytes)
        iTarget=patchBytes;
    signed char sizeDelta=0;                                      // WIP_CHAR_OK
    //Jump is forward
    if(*i < iTarget)
        for(int32_t j = *i;j<iTarget;j++)
            sizeDelta += GetPatchingSizeChange(relocTable[j]);
    //Jump is backward
    else
        for(int32_t j = *i;j>iTarget;j--)
            sizeDelta -= GetPatchingSizeChange(relocTable[j]);
    //Make sure we can still fit the offset in a short jump.
    BeAssert(offset + sizeDelta <=127 && offset + sizeDelta >= -128);
    offset += sizeDelta;
    replacedCodeP[offsetI]=offset;

    }

//X64-specific implementation of createPatchDataEntry. See createPatchDataEntry for description.
static void createPatchDataEntryX64
(
Byte * const replacedCodeP,       /* <= code to replace that overwritten by 'jmp' */
Byte const  * const crtFuncP,            /* => address of CRT function that will be patched */
TestControl const  * const relocTable,          /* => indicates if embedded addr needs reloc. */
int32_t              const patchBytes           /* => no. bytes to move into replaced code */
)
    {
    int32_t     i;
    int32_t codeSizeDiff = 0;
    uintptr_t   jumpDest;
    ptrdiff_t   const jumpDelta  = (crtFuncP - replacedCodeP);

    /* Copy bytes to be replaced. Because we're not guaranteed to be near the CRT, and the copied code
    includes 32-bit relative instructions, we need to rewrite some of the instructions to use 64-bit
    addresses. As we go through, rewrite the necessary instructions, and adjust short jumps to account
    for the change in code size. Also, many of the lesser-used functions have not been made high address-safe,
    so still attempt a relocation for those functions when necessary.*/
    memcpy (replacedCodeP, crtFuncP, patchBytes);
    for (i=0; i<patchBytes; i++)
        {
        if (relocTable[i] == VC32)
            expandCall32(replacedCodeP, relocTable, patchBytes,&i, jumpDelta, &codeSizeDiff);
        else if(relocTable[i] == VCRP)
            expandCallOverPointer(replacedCodeP, relocTable, patchBytes, &i, jumpDelta, &codeSizeDiff);
        else if(relocTable[i] == VMCP)
            expandMovCP(replacedCodeP, relocTable, patchBytes, &i, jumpDelta, &codeSizeDiff);
        else if(relocTable[i] == VOFF)
            adjustOffsetForCodeResize(replacedCodeP, relocTable, patchBytes, &i, codeSizeDiff);
        else if (relocTable[i] == V___)
            {
            uintptr_t * const p = (uintptr_t *) (replacedCodeP + i + codeSizeDiff);
            jumpDest = *p;
            jumpDest += jumpDelta;
            *p = jumpDest;
            /* Move forward past the end of the address */
            i += 3;
            }
        }
    //Jump back to CRT
    jumpDest = (uintptr_t) crtFuncP + patchBytes;
    Byte* jumpCode = replacedCodeP+patchBytes + codeSizeDiff;
    //mov rax, imm64
    jumpCode[0] = 0x48;
    jumpCode[1] = 0xb8;
    //mov rax, jumpDest
    memcpy(jumpCode+2, &jumpDest, sizeof(jumpDest));
    // jmp rax
    jumpCode[10] = 0xff;
    jumpCode[11] = 0xe0;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                  George.Dulchinos01/95
 This is used to create copies of the CRT functions that we're going to patch. This is
 important because when we run into non-pagalloced memory, we need the original msize and
 free to manipulate it. Please note that CRT functions other than msize and free are not
 guaranteed to work on x64 when the CRT is far away, because making this work requires
 non-trivial modifications to machine code at runtime, i.e magic.
+---------------+---------------+---------------+---------------+---------------+------*/
static void createPatchDataEntry
(
Byte * const replacedCodeP,       /* <= code to replace that overwritten by 'jmp' */
Byte const  * const crtFuncP,            /* => address of CRT function that will be patched */
TestControl const  * const relocTable,          /* => indicates if embedded addr needs reloc. */
int32_t              const patchBytes           /* => no. bytes to move into replaced code */
)
    {
#if defined(_X86_)
    createPatchDataEntryX86(replacedCodeP, crtFuncP, relocTable, patchBytes);
#elif defined(_M_X64)
    createPatchDataEntryX64(replacedCodeP, crtFuncP, relocTable, patchBytes);
#else
#error Patching not supported for this architecture, see papatch.c
#endif
    }

#ifdef _X86_
static void patchRuntimeFunctionX86
(
Byte * const crtFuncP,       /* => address of CRT function to patch */
Byte const  * const jumpToFuncP   /* => code to jump to instead */
)
    {
    int32_t         jumpDest;
    DWORD           oldProt;
    DWORD    const  fIsRunningWinNT = !(GetVersion() & 0x80000000);
    DWORD    const  fdwNewProtect   = fIsRunningWinNT ? PAGE_EXECUTE_WRITECOPY :  PAGE_EXECUTE_READWRITE;
    int32_t const JUMP_INSTR_LEN = 5;

    VirtualProtect (crtFuncP, JUMP_INSTR_LEN, fdwNewProtect, &oldProt);
    /* jmp rel imm32*/
    *crtFuncP = 0xe9;
    jumpDest = (int32_t)(jumpToFuncP - crtFuncP - JUMP_INSTR_LEN);
    /* Copy target address */
    memcpy (crtFuncP+1, &jumpDest, sizeof(int32_t));
    VirtualProtect (crtFuncP, JUMP_INSTR_LEN, oldProt, &oldProt);
    FlushInstructionCache (GetCurrentProcess(), crtFuncP, sizeof(int32_t)+1);
    }
#endif

#if defined(_M_X64)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
static void patchRuntimeFunctionX64
(
Byte * const crtFuncP,
Byte const  * const jumpToFuncP
)
    {
    intptr_t jumpDest= (intptr_t)jumpToFuncP;
    DWORD oldProt;
    DWORD   const   fIsRunningWinNT = !(GetVersion() & 0x80000000);
    DWORD   const fdwNewProtect     = fIsRunningWinNT ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
    int32_t const JUMP_INSTR_LEN = 12;

    VirtualProtect (crtFuncP, JUMP_INSTR_LEN, fdwNewProtect, &oldProt);
    /* mov rax, imm64 */
    crtFuncP[0] = 0x48;
    crtFuncP[1] = 0xb8;
    /*Address to put in rax */
    memcpy (crtFuncP+2, &jumpDest, sizeof(intptr_t));
    /*jmp rax */
    crtFuncP[10] = 0xff;
    crtFuncP[11] = 0xe0;
    VirtualProtect (crtFuncP, JUMP_INSTR_LEN, oldProt, &oldProt);
    FlushInstructionCache (GetCurrentProcess(), crtFuncP, JUMP_INSTR_LEN);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    George.Dulchinos01/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void patchRuntimeFunction
(
Byte * const crtFuncP,       /* => address of CRT function to patch */
Byte const  * const jumpToFuncP   /* => code to jump to instead */
)
    {
#ifdef _X86_
    patchRuntimeFunctionX86(crtFuncP, jumpToFuncP);
#elif defined(_M_X64)
    patchRuntimeFunctionX64(crtFuncP, jumpToFuncP);
#else
#error Patching not supported on this platform. Please update patchRuntimeFunction in papatch.cpp
#endif
    }

/*---------------------------------------------------------------------------------**//**
* author Philip McGraw 7/99
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void unpatchRuntimeFunction
(
Byte * const crtFuncP,       /* => address of CRT function to unpatch */
Byte const  * const savedFuncP,     /* => code saved before patch */
int32_t       const size            /* => length of patch */
)
    {
    DWORD           oldProt;
    DWORD const     fIsRunningWinNT = !(GetVersion() & 0x80000000);
    DWORD const     fdwNewProtect = fIsRunningWinNT ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE ;

    /* only unpatch if we've patched and still have function pointers */
    if (crtFuncP  &&  savedFuncP && size)
        {
        VirtualProtect (crtFuncP, size, fdwNewProtect, &oldProt);
        memcpy (crtFuncP, savedFuncP, size);
        VirtualProtect (crtFuncP, size, oldProt, &oldProt);
        FlushInstructionCache (GetCurrentProcess(), crtFuncP, size);
        }
    }

typedef int32_t (__stdcall *tMessageBoxA)(void *hWnd,char *lpText, char *lpCaption, uint32_t uType);  // WIP_CHAR_OK
#ifndef MB_OK
#  define MB_OK 0
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    09/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void paPatch_complain
(
char *debugString                                                                        // WIP_CHAR_OK
)
    {
    HINSTANCE hUser32 = LoadLibrary(_T("user32.dll"));
    tMessageBoxA pMessageBoxA = (tMessageBoxA)GetProcAddress(hUser32, "MessageBoxA");
    char szData[512];                                                                    // WIP_CHAR_OK

    OutputDebugStringA(debugString);
    if (GetEnvironmentVariableA("PAGE_DEBUG_PATCH_BREAK", szData, sizeof szData))
        DebugBreak();
    if (!GetEnvironmentVariableA("PAGE_DEBUG_PATCH_NOALERT", szData, sizeof szData) && pMessageBoxA)
        pMessageBoxA(NULL, debugString, "PAGALLOC Patch Alert", MB_OK);
    if (hUser32)
        FreeLibrary(hUser32);
    }

#if !defined (PAPATCH_No_MSCVRT_Patch_Tables)
/*---------------------------------------------------------------------------------**//**
* author Philip McGraw 7/99
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void paPatch_unpatchCRuntimeMemFuncs
(
void
)
    {
    /* unpatch using saved code */
    switch  (s_crtBuild)
        {
    case 0x0900521e:
        unpatchRuntimeFunction(crtMallocP,                  g_b0900521eMallocSave,                PATCHBYTES_MALLOC        (g_b0900521e) );
        unpatchRuntimeFunction(crtCallocP,                  g_b0900521eCallocSave,                PATCHBYTES_CALLOC        (g_b0900521e) );
        unpatchRuntimeFunction(crtReallocP,         g_b0900521eReallocSave,               PATCHBYTES_REALLOC       (g_b0900521e) );
        unpatchRuntimeFunction(crtFreeP,                    g_b0900521eFreeSave,              PATCHBYTES_FREE              (g_b0900521e) );
        unpatchRuntimeFunction(crtMsizeP,                   g_b0900521eMsizeSave,             PATCHBYTES_MSIZE         (g_b0900521e) );
        unpatchRuntimeFunction(crtExpandP,                  g_b0900521eExpandSave,                PATCHBYTES_EXPAND        (g_b0900521e) );
        unpatchRuntimeFunction(crtOperatorNewP,     g_b0900521eOperatorNewSave,       PATCHBYTES_OperatorNew   (g_b0900521e) );
        unpatchRuntimeFunction(crtOperatorDeleteP,      g_b0900521eOperatorDeleteSave,    PATCHBYTES_OperatorDelete(g_b0900521e) );
        unpatchRuntimeFunction(crtHeapWalkP,        g_b0900521eHeapWalkSave,          PATCHBYTES_HEAPWALK      (g_b0900521e) );
        unpatchRuntimeFunction(crtStrdupP,                  g_b0900521eStrdupSave,                PATCHBYTES_STRDUP        (g_b0900521e) );
        unpatchRuntimeFunction(crtWcsdupP,                  g_b0900521eWcsdupSave,                PATCHBYTES_WCSDUP        (g_b0900521e) );
        break;
    case 0x0A00766f:
        unpatchRuntimeFunction(crtMallocP,                  g_b0A00766fMallocSave,                PATCHBYTES_MALLOC        (g_b0A00766f) );
        unpatchRuntimeFunction(crtCallocP,                  g_b0A00766fCallocSave,                PATCHBYTES_CALLOC        (g_b0A00766f) );
        unpatchRuntimeFunction(crtReallocP,         g_b0A00766fReallocSave,               PATCHBYTES_REALLOC       (g_b0A00766f) );
        unpatchRuntimeFunction(crtFreeP,                    g_b0A00766fFreeSave,              PATCHBYTES_FREE              (g_b0A00766f) );
        unpatchRuntimeFunction(crtMsizeP,                   g_b0A00766fMsizeSave,             PATCHBYTES_MSIZE         (g_b0A00766f) );
        unpatchRuntimeFunction(crtExpandP,                  g_b0A00766fExpandSave,                PATCHBYTES_EXPAND        (g_b0A00766f) );
        unpatchRuntimeFunction(crtOperatorNewP,     g_b0A00766fOperatorNewSave,       PATCHBYTES_OperatorNew   (g_b0A00766f) );
        unpatchRuntimeFunction(crtOperatorDeleteP,      g_b0A00766fOperatorDeleteSave,    PATCHBYTES_OperatorDelete(g_b0A00766f) );
        unpatchRuntimeFunction(crtHeapWalkP,        g_b0A00766fHeapWalkSave,          PATCHBYTES_HEAPWALK      (g_b0A00766f) );
        unpatchRuntimeFunction(crtStrdupP,                  g_b0A00766fStrdupSave,                PATCHBYTES_STRDUP        (g_b0A00766f) );
        unpatchRuntimeFunction(crtWcsdupP,                  g_b0A00766fWcsdupSave,                PATCHBYTES_WCSDUP        (g_b0A00766f) );
    case 0x0B00C7A2:
        unpatchRuntimeFunction(crtMallocP,                  g_b0B00C7A2MallocSave,                  PATCHBYTES_MALLOC        (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtCallocP,                  g_b0B00C7A2CallocSave,                  PATCHBYTES_CALLOC        (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtReallocP,                 g_b0B00C7A2ReallocSave,                 PATCHBYTES_REALLOC       (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtFreeP,                    g_b0B00C7A2FreeSave,                    PATCHBYTES_FREE          (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtMsizeP,                   g_b0B00C7A2MsizeSave,                   PATCHBYTES_MSIZE         (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtExpandP,                  g_b0B00C7A2ExpandSave,                  PATCHBYTES_EXPAND        (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtOperatorNewP,             g_b0B00C7A2OperatorNewSave,             PATCHBYTES_OperatorNew   (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtOperatorDeleteP,          g_b0B00C7A2OperatorDeleteSave,          PATCHBYTES_OperatorDelete(g_b0B00C7A2) );
        unpatchRuntimeFunction(crtHeapWalkP,                g_b0B00C7A2HeapWalkSave,                PATCHBYTES_HEAPWALK      (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtStrdupP,                  g_b0B00C7A2StrdupSave,                  PATCHBYTES_STRDUP        (g_b0B00C7A2) );
        unpatchRuntimeFunction(crtWcsdupP,                  g_b0B00C7A2WcsdupSave,                  PATCHBYTES_WCSDUP        (g_b0B00C7A2) );
    case 0x0C00520D:
        unpatchRuntimeFunction(crtMallocP,                  g_b0C00520DMallocSave,                  PATCHBYTES_MALLOC        (g_b0C00520D) );
        unpatchRuntimeFunction(crtCallocP,                  g_b0C00520DCallocSave,                  PATCHBYTES_CALLOC        (g_b0C00520D) );
        unpatchRuntimeFunction(crtReallocP,                 g_b0C00520DReallocSave,                 PATCHBYTES_REALLOC       (g_b0C00520D) );
        unpatchRuntimeFunction(crtFreeP,                    g_b0C00520DFreeSave,                    PATCHBYTES_FREE          (g_b0C00520D) );
        unpatchRuntimeFunction(crtMsizeP,                   g_b0C00520DMsizeSave,                   PATCHBYTES_MSIZE         (g_b0C00520D) );
        unpatchRuntimeFunction(crtExpandP,                  g_b0C00520DExpandSave,                  PATCHBYTES_EXPAND        (g_b0C00520D) );
        unpatchRuntimeFunction(crtOperatorNewP,             g_b0C00520DOperatorNewSave,             PATCHBYTES_OperatorNew   (g_b0C00520D) );
        unpatchRuntimeFunction(crtOperatorDeleteP,          g_b0C00520DOperatorDeleteSave,          PATCHBYTES_OperatorDelete(g_b0C00520D) );
        unpatchRuntimeFunction(crtHeapWalkP,                g_b0C00520DHeapWalkSave,                PATCHBYTES_HEAPWALK      (g_b0C00520D) );
        unpatchRuntimeFunction(crtStrdupP,                  g_b0C00520DStrdupSave,                  PATCHBYTES_STRDUP        (g_b0C00520D) );
        unpatchRuntimeFunction(crtWcsdupP,                  g_b0C00520DWcsdupSave,                  PATCHBYTES_WCSDUP        (g_b0C00520D) );
    default:
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Performs the initialization needed to call pagalloc (further initialization is done when first using pagalloc)
* @bsimethod                                George.Dulchinos    01/95
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t paPatch_patchCRuntimeMemFuncs
(
void
)
    {
    HINSTANCE                   hCrt = NULL, hTool = NULL;
    Byte *toolMallocP, *toolNhMallocP, *toolCallocP, *toolReallocP, *toolFreeP, *toolMsizeP, *toolExpandP, *toolHeapWalkP, *toolStrdupP, *toolWcsdupP;
    Byte *toolOperatorNewP, *toolOperatorDeleteP;
    PagallocSetFreeHFunc        toolSetFreeHP;
    PagallocSetMsizeHFunc       toolSetMsizeHP;
    //Load the latest C runtime, so we can patch it.
#if defined (MSVCRT_NAME)
    hCrt = LoadLibrary(_T(MSVCRT_NAME));        // your choice
#elif (_MSC_VER == 1500)
    hCrt = GetModuleHandle(_T("msvcr90.dll"));           // VS 2008 (aka Orcas)
#elif (_MSC_VER == 1600)
    hCrt = GetModuleHandle(_T("msvcr100.dll"));          // VS 2010 (aka Dev10)
#else
#  error Unknown C runtime version:  Please examine papatch.c
#endif
    if (!hCrt)
        {
        paPatch_complain("PAGALLOC: attempt to patch CRT failed because MSVCRT.DLL not found.\r\nPAGALLOC: perhaps you are linking with MSVCRTD.DLL by using -MDd?\r\n");
        return false;
        }

    crtMallocP   = (Byte *)GetProcAddress (hCrt, "malloc");

        {
        int32_t i;
        for (i=0, crtNhMallocP = crtMallocP;    (0xe8 != *crtNhMallocP)  &&  (i < 100);   i++, crtNhMallocP++)
            ;   // blank line.  Semicolon only.
        }

        //On VS 9 and 10, __nh_malloc appears to be gone, and the code to find it messes with other functions, so don't use it.
        if(_MSC_VER >= 1500)
            crtNhMallocP = NULL;
        else if(0xe8 == *crtNhMallocP)
                {
                uint32_t *ofsNhMallocP = (uint32_t *)(++crtNhMallocP);
                crtNhMallocP += (4 + (*ofsNhMallocP));
                }
        else crtNhMallocP = NULL;


        crtCallocP          = (Byte *)GetProcAddress (hCrt, "calloc");
        crtReallocP         = (Byte *)GetProcAddress (hCrt, "realloc");
        crtFreeP            = (Byte *)GetProcAddress (hCrt, "free");
        crtMsizeP           = (Byte *)GetProcAddress (hCrt, "_msize");
        crtExpandP          = (Byte *)GetProcAddress (hCrt, "_expand");
#if defined(_X86_)
        crtOperatorNewP     = (Byte *)GetProcAddress (hCrt, "??2@YAPAXI@Z");        // Mangled operator new
        crtOperatorDeleteP  = (Byte *)GetProcAddress (hCrt, "??3@YAXPAX@Z");        // Mangled operator delete
#elif defined(_M_X64)
        crtOperatorNewP     = (Byte *)GetProcAddress (hCrt, "??2@YAPEAX_K@Z");      // Mangled operator new
        crtOperatorDeleteP  = (Byte *)GetProcAddress (hCrt, "??3@YAXPEAX@Z");       // Mangled operator delete
#else
#error Patching support incomplete for this architecture: See papatch.h
#endif
        crtHeapWalkP        = (Byte *)GetProcAddress (hCrt, "_heapwalk");
#if (_MSC_VER >= 1400)
        crtStrdupP          = (Byte *)GetProcAddress (hCrt, "_strdup");             // strdup
#else
        crtStrdupP          = (Byte *)GetProcAddress (hCrt, "_mbsdup");             // strdup
#endif
        crtWcsdupP          = (Byte *)GetProcAddress (hCrt, "_wcsdup");             // _wcsdup


#if defined(_M_X64)
        Byte * vc11OpDeleteJmp64Area = crtOperatorDeleteP + s_vc11_offset_operatorDeleteExportToJmp64Area;  // used for VC11 x64 only 

        Byte * vc12OpDeleteJmp64Area = findRun (crtOperatorDeleteP-0x40000, crtOperatorDeleteP+0x40000, 0xcc, 12);
        
#endif

        /*-----------------------------------------------------------------------------------
        First verify that we can safely patch each mem func. in the runtime.
        If all the routines match, we now know it's safe to patch this DLL.
        Allocate a page to hold the data that is overwritten by the patch followed by a jump to
        to C Runtime code that originally followed that data.  We will jump through here
        to access the original C Runtime functions.
        -----------------------------------------------------------------------------------*/


        if( isFuncPatchable (crtMallocP,        g_b0900521eMallocInfo,          g_b0900521eMallocTest,          sizeof g_b0900521eMallocTest            ) &&
            isFuncPatchable (crtCallocP,        g_b0900521eCallocInfo,          g_b0900521eCallocTest,          sizeof g_b0900521eCallocTest            ) &&
            isFuncPatchable (crtReallocP,       g_b0900521eReallocInfo,         g_b0900521eReallocTest,         sizeof g_b0900521eReallocTest           ) &&
            isFuncPatchable (crtFreeP,          g_b0900521eFreeInfo,            g_b0900521eFreeTest,            sizeof g_b0900521eFreeTest              ) &&
            isFuncPatchable (crtMsizeP,         g_b0900521eMsizeInfo,           g_b0900521eMsizeTest,           sizeof g_b0900521eMsizeTest             ) &&
            isFuncPatchable (crtExpandP,        g_b0900521eExpandInfo,          g_b0900521eExpandTest,          sizeof g_b0900521eExpandTest            ) &&
            isFuncPatchable (crtOperatorNewP,   g_b0900521eOperatorNewCtrl,     g_b0900521eOperatorNewTest,     sizeof g_b0900521eOperatorNewTest       ) &&
            isFuncPatchable (crtOperatorDeleteP,g_b0900521eOperatorDeleteCtrl,  g_b0900521eOperatorDeleteTest,  sizeof g_b0900521eOperatorDeleteTest    ) &&
            isFuncPatchable (crtHeapWalkP,      g_b0900521eHeapWalkCtrl,        g_b0900521eHeapWalkTest,        sizeof g_b0900521eHeapWalkTest          ) &&
            isFuncPatchable (crtStrdupP,        g_b0900521eStrdupCtrl,          g_b0900521eStrdupTest,          sizeof g_b0900521eStrdupTest            ) &&
            isFuncPatchable (crtWcsdupP,        g_b0900521eWcsdupCtrl,          g_b0900521eWcsdupTest,          sizeof g_b0900521eWcsdupTest            )
            )
            {
            s_crtBuild = 0x0900521e;
            OutputDebugString(_T("PAGALLOC: patching MSVCRT build 9.0.21022.8   VC 9.0\r\n"));
            /* save a copy of the originals for unpatching */
            memcpy (g_b0900521eMallocSave,                crtMallocP,             PATCHBYTES_MALLOC        (g_b0900521e) );
            memcpy (g_b0900521eCallocSave,                crtCallocP,             PATCHBYTES_CALLOC        (g_b0900521e) );
            memcpy (g_b0900521eReallocSave,               crtReallocP,        PATCHBYTES_REALLOC       (g_b0900521e) );
            memcpy (g_b0900521eFreeSave,              crtFreeP,           PATCHBYTES_FREE          (g_b0900521e) );
            memcpy (g_b0900521eMsizeSave,             crtMsizeP,          PATCHBYTES_MSIZE             (g_b0900521e) );
            memcpy (g_b0900521eOperatorNewSave,   crtOperatorNewP,    PATCHBYTES_OperatorNew   (g_b0900521e) );
            memcpy (g_b0900521eOperatorDeleteSave,crtOperatorDeleteP, PATCHBYTES_OperatorDelete(g_b0900521e) );
            memcpy (g_b0900521eHeapWalkSave,              crtHeapWalkP,       PATCHBYTES_HEAPWALK      (g_b0900521e) );
            memcpy (g_b0900521eStrdupSave,                crtStrdupP,             PATCHBYTES_STRDUP        (g_b0900521e) );
            memcpy (g_b0900521eWcsdupSave,                crtWcsdupP,             PATCHBYTES_WCSDUP        (g_b0900521e) );


            patchData =(struct __patchData *) VirtualAlloc (NULL, sizeof(*patchData), MEM_COMMIT, PAGE_READWRITE);

            createPatchDataEntry (patchData->mallocCode,           crtMallocP,          g_b0900521eMallocInfo,        PATCHBYTES_MALLOC        (g_b0900521e) );
            createPatchDataEntry (patchData->callocCode,           crtCallocP,          g_b0900521eCallocInfo,        PATCHBYTES_CALLOC        (g_b0900521e) );
            createPatchDataEntry (patchData->reallocCode,          crtReallocP,         g_b0900521eReallocInfo,       PATCHBYTES_REALLOC       (g_b0900521e) );
            createPatchDataEntry (patchData->freeCode,             crtFreeP,            g_b0900521eFreeInfo,          PATCHBYTES_FREE          (g_b0900521e) );
            createPatchDataEntry (patchData->msizeCode,            crtMsizeP,           g_b0900521eMsizeInfo,         PATCHBYTES_MSIZE         (g_b0900521e) );
            createPatchDataEntry (patchData->expandCode,           crtExpandP,          g_b0900521eExpandInfo,        PATCHBYTES_EXPAND        (g_b0900521e) );
            createPatchDataEntry (patchData->operatorNewCode,      crtOperatorNewP,     g_b0900521eOperatorNewCtrl,   PATCHBYTES_OperatorNew   (g_b0900521e) );
            createPatchDataEntry (patchData->operatorDeleteCode,   crtOperatorDeleteP,  g_b0900521eOperatorDeleteCtrl,PATCHBYTES_OperatorDelete(g_b0900521e) );
            createPatchDataEntry (patchData->heapWalkCode,         crtHeapWalkP,        g_b0900521eHeapWalkCtrl,      PATCHBYTES_HEAPWALK      (g_b0900521e) );
            createPatchDataEntry (patchData->strdupCode,           crtStrdupP,          g_b0900521eStrdupCtrl,        PATCHBYTES_STRDUP        (g_b0900521e) );
            createPatchDataEntry (patchData->wcsdupCode,           crtWcsdupP,          g_b0900521eWcsdupCtrl,        PATCHBYTES_WCSDUP        (g_b0900521e) );
            }
        else 
         if(isFuncPatchable (crtMallocP,            g_b0A00766fMallocInfo,          g_b0A00766fMallocTest,          sizeof g_b0A00766fMallocTest        ) &&
            isFuncPatchable (crtCallocP,            g_b0A00766fCallocInfo,          g_b0A00766fCallocTest,          sizeof g_b0A00766fCallocTest        ) &&
            isFuncPatchable (crtReallocP,           g_b0A00766fReallocInfo,         g_b0A00766fReallocTest,         sizeof g_b0A00766fReallocTest       ) &&
            isFuncPatchable (crtFreeP,              g_b0A00766fFreeInfo,            g_b0A00766fFreeTest,            sizeof g_b0A00766fFreeTest          ) &&
            isFuncPatchable (crtMsizeP,             g_b0A00766fMsizeInfo,           g_b0A00766fMsizeTest,           sizeof g_b0A00766fMsizeTest         ) &&
            isFuncPatchable (crtExpandP,            g_b0A00766fExpandInfo,          g_b0A00766fExpandTest,          sizeof g_b0A00766fExpandTest        ) &&
            isFuncPatchable (crtOperatorNewP,       g_b0A00766fOperatorNewCtrl,     g_b0A00766fOperatorNewTest,     sizeof g_b0A00766fOperatorNewTest   ) &&
            isFuncPatchable (crtOperatorDeleteP,    g_b0A00766fOperatorDeleteCtrl,  g_b0A00766fOperatorDeleteTest,  sizeof g_b0A00766fOperatorDeleteTest) &&
            isFuncPatchable (crtHeapWalkP,          g_b0A00766fHeapWalkCtrl,        g_b0A00766fHeapWalkTest,        sizeof g_b0A00766fHeapWalkTest      ) &&
            isFuncPatchable (crtStrdupP,            g_b0A00766fStrdupCtrl,          g_b0A00766fStrdupTest,          sizeof g_b0A00766fStrdupTest        ) &&
            isFuncPatchable (crtWcsdupP,            g_b0A00766fWcsdupCtrl,          g_b0A00766fWcsdupTest,          sizeof g_b0A00766fWcsdupTest        ))
            {
            s_crtBuild = 0x0A00766f;
            OutputDebugString(_T("PAGALLOC: patching MSVCRT build 10.00.30319.1   VC 10.0\r\n"));
            /* save a copy of the originals for unpatching */
            memcpy (g_b0A00766fMallocSave,            crtMallocP,             PATCHBYTES_MALLOC        (g_b0A00766f) );
            memcpy (g_b0A00766fCallocSave,            crtCallocP,             PATCHBYTES_CALLOC        (g_b0A00766f) );
            memcpy (g_b0A00766fReallocSave,           crtReallocP,        PATCHBYTES_REALLOC       (g_b0A00766f) );
            memcpy (g_b0A00766fFreeSave,                  crtFreeP,           PATCHBYTES_FREE          (g_b0A00766f) );
            memcpy (g_b0A00766fMsizeSave,                 crtMsizeP,          PATCHBYTES_MSIZE             (g_b0A00766f) );
            memcpy (g_b0A00766fOperatorNewSave,   crtOperatorNewP,    PATCHBYTES_OperatorNew   (g_b0A00766f) );
            memcpy (g_b0A00766fOperatorDeleteSave,crtOperatorDeleteP, PATCHBYTES_OperatorDelete(g_b0A00766f) );
            memcpy (g_b0A00766fHeapWalkSave,          crtHeapWalkP,       PATCHBYTES_HEAPWALK      (g_b0A00766f) );
            memcpy (g_b0A00766fStrdupSave,            crtStrdupP,             PATCHBYTES_STRDUP        (g_b0A00766f) );
            memcpy (g_b0A00766fWcsdupSave,            crtWcsdupP,             PATCHBYTES_WCSDUP        (g_b0A00766f) );


            patchData =(struct __patchData *) VirtualAlloc (NULL, sizeof(*patchData), MEM_COMMIT, PAGE_READWRITE);

            createPatchDataEntry (patchData->mallocCode,           crtMallocP,          g_b0A00766fMallocInfo,        PATCHBYTES_MALLOC        (g_b0A00766f) );
            createPatchDataEntry (patchData->callocCode,           crtCallocP,          g_b0A00766fCallocInfo,        PATCHBYTES_CALLOC        (g_b0A00766f) );
            createPatchDataEntry (patchData->reallocCode,          crtReallocP,         g_b0A00766fReallocInfo,       PATCHBYTES_REALLOC       (g_b0A00766f) );
            createPatchDataEntry (patchData->freeCode,             crtFreeP,            g_b0A00766fFreeInfo,          PATCHBYTES_FREE          (g_b0A00766f) );
            createPatchDataEntry (patchData->msizeCode,            crtMsizeP,           g_b0A00766fMsizeInfo,         PATCHBYTES_MSIZE         (g_b0A00766f) );
            createPatchDataEntry (patchData->expandCode,           crtExpandP,          g_b0A00766fExpandInfo,        PATCHBYTES_EXPAND        (g_b0A00766f) );
            createPatchDataEntry (patchData->operatorNewCode,      crtOperatorNewP,     g_b0A00766fOperatorNewCtrl,   PATCHBYTES_OperatorNew   (g_b0A00766f) );
            createPatchDataEntry (patchData->operatorDeleteCode,   crtOperatorDeleteP,  g_b0A00766fOperatorDeleteCtrl,PATCHBYTES_OperatorDelete(g_b0A00766f) );
            createPatchDataEntry (patchData->heapWalkCode,         crtHeapWalkP,        g_b0A00766fHeapWalkCtrl,      PATCHBYTES_HEAPWALK      (g_b0A00766f) );
            createPatchDataEntry (patchData->strdupCode,           crtStrdupP,          g_b0A00766fStrdupCtrl,        PATCHBYTES_STRDUP        (g_b0A00766f) );
            createPatchDataEntry (patchData->wcsdupCode,           crtWcsdupP,          g_b0A00766fWcsdupCtrl,        PATCHBYTES_WCSDUP        (g_b0A00766f) );
            }
        else 
         if(isFuncPatchable (crtMallocP,        g_b0B00C7A2MallocInfo,          g_b0B00C7A2MallocTest,          sizeof g_b0B00C7A2MallocTest            ) &&
            isFuncPatchable (crtCallocP,        g_b0B00C7A2CallocInfo,          g_b0B00C7A2CallocTest,          sizeof g_b0B00C7A2CallocTest            ) &&
            isFuncPatchable (crtReallocP,       g_b0B00C7A2ReallocInfo,         g_b0B00C7A2ReallocTest,         sizeof g_b0B00C7A2ReallocTest           ) &&
            isFuncPatchable (crtFreeP,          g_b0B00C7A2FreeInfo,            g_b0B00C7A2FreeTest,            sizeof g_b0B00C7A2FreeTest              ) &&
            isFuncPatchable (crtMsizeP,         g_b0B00C7A2MsizeInfo,           g_b0B00C7A2MsizeTest,           sizeof g_b0B00C7A2MsizeTest             ) &&
            isFuncPatchable (crtExpandP,        g_b0B00C7A2ExpandInfo,          g_b0B00C7A2ExpandTest,          sizeof g_b0B00C7A2ExpandTest            ) &&
            isFuncPatchable (crtOperatorNewP,   g_b0B00C7A2OperatorNewCtrl,     g_b0B00C7A2OperatorNewTest,     sizeof g_b0B00C7A2OperatorNewTest       ) &&
#if defined(_M_X64)
            isFuncPatchable (crtOperatorDeleteP,    g_b0B00C7A2OperatorDeleteStubCtrl,  g_b0B00C7A2OperatorDeleteStubTest,  sizeof g_b0B00C7A2OperatorDeleteStubTest    ) &&
            isFuncPatchable (vc11OpDeleteJmp64Area, g_b0B00C7A2OperatorDeleteCtrl,      g_b0B00C7A2OperatorDeleteTest,      sizeof g_b0B00C7A2OperatorDeleteTest        ) &&
#else
            isFuncPatchable (crtOperatorDeleteP,g_b0B00C7A2OperatorDeleteCtrl,  g_b0B00C7A2OperatorDeleteTest,  sizeof g_b0B00C7A2OperatorDeleteTest    ) &&
#endif
            isFuncPatchable (crtHeapWalkP,      g_b0B00C7A2HeapWalkCtrl,        g_b0B00C7A2HeapWalkTest,        sizeof g_b0B00C7A2HeapWalkTest          ) &&
            isFuncPatchable (crtStrdupP,        g_b0B00C7A2StrdupCtrl,          g_b0B00C7A2StrdupTest,          sizeof g_b0B00C7A2StrdupTest            ) &&
            isFuncPatchable (crtWcsdupP,        g_b0B00C7A2WcsdupCtrl,          g_b0B00C7A2WcsdupTest,          sizeof g_b0B00C7A2WcsdupTest            ) )
            {
            // -----------------------------------------------------------------------------------
            // Collect VC 11 Runtime Patch Data
            // -----------------------------------------------------------------------------------

            s_crtBuild = 0x0A00766f;
            OutputDebugString(_T("PAGALLOC: patching MSVCRT build 11.00.51106.1   VC 11.0\r\n"));

#if defined(_M_X64)
            //
            // NOTE: The CRT implementation of 'operator delete' is simply a jump to 'free'. It is a very small 
            //       function and there is not enough room to patch, so we need to vector it through a jump to a 
            //       64-bit address before we can patch.
            //
            //       See additional notes above regarding patching VC11 x64 Operator Delete
            //
            patchRuntimeFunctionX86 (crtOperatorDeleteP,    vc11OpDeleteJmp64Area);  // inserts jump using 32-bit offset
            patchRuntimeFunctionX64 (vc11OpDeleteJmp64Area, crtFreeP);               // inserts jump using 64-bit address

            crtOperatorDeleteP  = vc11OpDeleteJmp64Area;  
#endif
            /* save a copy of the originals for unpatching */
            memcpy (g_b0B00C7A2MallocSave,          crtMallocP,         PATCHBYTES_MALLOC        (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2CallocSave,          crtCallocP,         PATCHBYTES_CALLOC        (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2ReallocSave,         crtReallocP,        PATCHBYTES_REALLOC       (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2FreeSave,            crtFreeP,           PATCHBYTES_FREE          (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2MsizeSave,           crtMsizeP,          PATCHBYTES_MSIZE         (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2OperatorNewSave,     crtOperatorNewP,    PATCHBYTES_OperatorNew   (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2OperatorDeleteSave,  crtOperatorDeleteP, PATCHBYTES_OperatorDelete(g_b0B00C7A2) );
            memcpy (g_b0B00C7A2HeapWalkSave,        crtHeapWalkP,       PATCHBYTES_HEAPWALK      (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2StrdupSave,          crtStrdupP,         PATCHBYTES_STRDUP        (g_b0B00C7A2) );
            memcpy (g_b0B00C7A2WcsdupSave,          crtWcsdupP,         PATCHBYTES_WCSDUP        (g_b0B00C7A2) );


            patchData =(struct __patchData *) VirtualAlloc (NULL, sizeof(*patchData), MEM_COMMIT, PAGE_READWRITE);

            createPatchDataEntry (patchData->mallocCode,           crtMallocP,          g_b0B00C7A2MallocInfo,        PATCHBYTES_MALLOC        (g_b0B00C7A2) );
            createPatchDataEntry (patchData->callocCode,           crtCallocP,          g_b0B00C7A2CallocInfo,        PATCHBYTES_CALLOC        (g_b0B00C7A2) );
            createPatchDataEntry (patchData->reallocCode,          crtReallocP,         g_b0B00C7A2ReallocInfo,       PATCHBYTES_REALLOC       (g_b0B00C7A2) );
            createPatchDataEntry (patchData->freeCode,             crtFreeP,            g_b0B00C7A2FreeInfo,          PATCHBYTES_FREE          (g_b0B00C7A2) );
            createPatchDataEntry (patchData->msizeCode,            crtMsizeP,           g_b0B00C7A2MsizeInfo,         PATCHBYTES_MSIZE         (g_b0B00C7A2) );
            createPatchDataEntry (patchData->expandCode,           crtExpandP,          g_b0B00C7A2ExpandInfo,        PATCHBYTES_EXPAND        (g_b0B00C7A2) );
            createPatchDataEntry (patchData->operatorNewCode,      crtOperatorNewP,     g_b0B00C7A2OperatorNewCtrl,   PATCHBYTES_OperatorNew   (g_b0B00C7A2) );
            createPatchDataEntry (patchData->operatorDeleteCode,   crtOperatorDeleteP,  g_b0B00C7A2OperatorDeleteCtrl,PATCHBYTES_OperatorDelete(g_b0B00C7A2) );
            createPatchDataEntry (patchData->heapWalkCode,         crtHeapWalkP,        g_b0B00C7A2HeapWalkCtrl,      PATCHBYTES_HEAPWALK      (g_b0B00C7A2) );
            createPatchDataEntry (patchData->strdupCode,           crtStrdupP,          g_b0B00C7A2StrdupCtrl,        PATCHBYTES_STRDUP        (g_b0B00C7A2) );
            createPatchDataEntry (patchData->wcsdupCode,           crtWcsdupP,          g_b0B00C7A2WcsdupCtrl,        PATCHBYTES_WCSDUP        (g_b0B00C7A2) );
            }
        else 
         if(isFuncPatchable (crtMallocP,        g_b0C00520DMallocInfo,          g_b0C00520DMallocTest,          sizeof g_b0C00520DMallocTest            ) &&
            isFuncPatchable (crtCallocP,        g_b0C00520DCallocInfo,          g_b0C00520DCallocTest,          sizeof g_b0C00520DCallocTest            ) &&
            isFuncPatchable (crtReallocP,       g_b0C00520DReallocInfo,         g_b0C00520DReallocTest,         sizeof g_b0C00520DReallocTest           ) &&
            isFuncPatchable (crtFreeP,          g_b0C00520DFreeInfo,            g_b0C00520DFreeTest,            sizeof g_b0C00520DFreeTest              ) &&
            isFuncPatchable (crtMsizeP,         g_b0C00520DMsizeInfo,           g_b0C00520DMsizeTest,           sizeof g_b0C00520DMsizeTest             ) &&
            isFuncPatchable (crtExpandP,        g_b0C00520DExpandInfo,          g_b0C00520DExpandTest,          sizeof g_b0C00520DExpandTest            ) &&
            isFuncPatchable (crtOperatorNewP,   g_b0C00520DOperatorNewCtrl,     g_b0C00520DOperatorNewTest,     sizeof g_b0C00520DOperatorNewTest       ) &&
#if defined(_M_X64)
            isFuncPatchable (crtOperatorDeleteP,    g_b0C00520DOperatorDeleteStubCtrl,  g_b0C00520DOperatorDeleteStubTest,  sizeof g_b0C00520DOperatorDeleteStubTest    ) &&
            isFuncPatchable (vc12OpDeleteJmp64Area, g_b0C00520DOperatorDeleteCtrl,      g_b0C00520DOperatorDeleteTest,      sizeof g_b0C00520DOperatorDeleteTest        ) &&
#else
            isFuncPatchable (crtOperatorDeleteP,g_b0C00520DOperatorDeleteCtrl,  g_b0C00520DOperatorDeleteTest,  sizeof g_b0C00520DOperatorDeleteTest    ) &&
#endif
            isFuncPatchable (crtHeapWalkP,      g_b0C00520DHeapWalkCtrl,        g_b0C00520DHeapWalkTest,        sizeof g_b0C00520DHeapWalkTest          ) &&
            isFuncPatchable (crtStrdupP,        g_b0C00520DStrdupCtrl,          g_b0C00520DStrdupTest,          sizeof g_b0C00520DStrdupTest            ) &&
            isFuncPatchable (crtWcsdupP,        g_b0C00520DWcsdupCtrl,          g_b0C00520DWcsdupTest,          sizeof g_b0C00520DWcsdupTest            ) )
            {
            // -----------------------------------------------------------------------------------
            // Collect VC 12 Runtime Patch Data
            // -----------------------------------------------------------------------------------

            s_crtBuild = 0x0C00520d;
            OutputDebugString(_T("PAGALLOC: patching MSVCRT build 12.00.21005.1   VC 12.0\r\n"));

#if defined(_M_X64)
            //
            // NOTE: The CRT implementation of 'operator delete' is simply a jump to 'free'. It is a very small 
            //       function and there is not enough room to patch, so we need to vector it through a jump to a 
            //       64-bit address before we can patch.
            //
            //       See additional notes above regarding patching VC12 x64 Operator Delete
            //
            patchRuntimeFunctionX86 (crtOperatorDeleteP,    vc12OpDeleteJmp64Area);  // inserts jump using 32-bit offset
            patchRuntimeFunctionX64 (vc12OpDeleteJmp64Area, crtFreeP);               // inserts jump using 64-bit address

            crtOperatorDeleteP  = vc12OpDeleteJmp64Area;  
#endif
            /* save a copy of the originals for unpatching */
            memcpy (g_b0C00520DMallocSave,          crtMallocP,         PATCHBYTES_MALLOC        (g_b0C00520D) );
            memcpy (g_b0C00520DCallocSave,          crtCallocP,         PATCHBYTES_CALLOC        (g_b0C00520D) );
            memcpy (g_b0C00520DReallocSave,         crtReallocP,        PATCHBYTES_REALLOC       (g_b0C00520D) );
            memcpy (g_b0C00520DFreeSave,            crtFreeP,           PATCHBYTES_FREE          (g_b0C00520D) );
            memcpy (g_b0C00520DMsizeSave,           crtMsizeP,          PATCHBYTES_MSIZE         (g_b0C00520D) );
            memcpy (g_b0C00520DOperatorNewSave,     crtOperatorNewP,    PATCHBYTES_OperatorNew   (g_b0C00520D) );
            memcpy (g_b0C00520DOperatorDeleteSave,  crtOperatorDeleteP, PATCHBYTES_OperatorDelete(g_b0C00520D) );
            memcpy (g_b0C00520DHeapWalkSave,        crtHeapWalkP,       PATCHBYTES_HEAPWALK      (g_b0C00520D) );
            memcpy (g_b0C00520DStrdupSave,          crtStrdupP,         PATCHBYTES_STRDUP        (g_b0C00520D) );
            memcpy (g_b0C00520DWcsdupSave,          crtWcsdupP,         PATCHBYTES_WCSDUP        (g_b0C00520D) );


            patchData =(struct __patchData *) VirtualAlloc (NULL, sizeof(*patchData), MEM_COMMIT, PAGE_READWRITE);

            createPatchDataEntry (patchData->mallocCode,           crtMallocP,          g_b0C00520DMallocInfo,        PATCHBYTES_MALLOC        (g_b0C00520D) );
            createPatchDataEntry (patchData->callocCode,           crtCallocP,          g_b0C00520DCallocInfo,        PATCHBYTES_CALLOC        (g_b0C00520D) );
            createPatchDataEntry (patchData->reallocCode,          crtReallocP,         g_b0C00520DReallocInfo,       PATCHBYTES_REALLOC       (g_b0C00520D) );
            createPatchDataEntry (patchData->freeCode,             crtFreeP,            g_b0C00520DFreeInfo,          PATCHBYTES_FREE          (g_b0C00520D) );
            createPatchDataEntry (patchData->msizeCode,            crtMsizeP,           g_b0C00520DMsizeInfo,         PATCHBYTES_MSIZE         (g_b0C00520D) );
            createPatchDataEntry (patchData->expandCode,           crtExpandP,          g_b0C00520DExpandInfo,        PATCHBYTES_EXPAND        (g_b0C00520D) );
            createPatchDataEntry (patchData->operatorNewCode,      crtOperatorNewP,     g_b0C00520DOperatorNewCtrl,   PATCHBYTES_OperatorNew   (g_b0C00520D) );
            createPatchDataEntry (patchData->operatorDeleteCode,   crtOperatorDeleteP,  g_b0C00520DOperatorDeleteCtrl,PATCHBYTES_OperatorDelete(g_b0C00520D) );
            createPatchDataEntry (patchData->heapWalkCode,         crtHeapWalkP,        g_b0C00520DHeapWalkCtrl,      PATCHBYTES_HEAPWALK      (g_b0C00520D) );
            createPatchDataEntry (patchData->strdupCode,           crtStrdupP,          g_b0C00520DStrdupCtrl,        PATCHBYTES_STRDUP        (g_b0C00520D) );
            createPatchDataEntry (patchData->wcsdupCode,           crtWcsdupP,          g_b0C00520DWcsdupCtrl,        PATCHBYTES_WCSDUP        (g_b0C00520D) );
            }
        else
            {
            OutputDebugString(_T("PAGALLOC: only can patch one of the following MSVCRT builds: 9.0.21022.8, 10.00.30319.1, 11.00.51106.1, 12.0.21005.1 \r\n"));
            paPatch_complain("PAGALLOC: attempt to patch CRT failed.\r\nPAGALLOC: unsupported MSVCRT.DLL found.\r\n");
            return false;
            }

        /* Set up pointers of the correct type for each function */
        patchData->callCrtMalloc        = (CRuntimeMallocFunc)          (void *)patchData->mallocCode;
        patchData->callCrtNhMalloc      = (CRuntimeNhMallocFunc)        (void *)patchData->nhMallocCode;
        patchData->callCrtCalloc        = (CRuntimeCallocFunc)          (void *)patchData->callocCode;
        patchData->callCrtRealloc       = (CRuntimeReallocFunc)         (void *)patchData->reallocCode;
        patchData->callCrtFree          = (CRuntimeFreeFunc)            (void *)patchData->freeCode;
        patchData->callCrtMsize         = (CRuntimeMsizeFunc)           (void *)patchData->msizeCode;
        patchData->callCrtExpand        = (CRuntimeExpandFunc)          (void *)patchData->expandCode;
        patchData->callCrtOperatorNew   = (PagallocOperatorNewFunc)     (void *)patchData->operatorNewCode;
        patchData->callCrtOperatorDelete= (PagallocOperatorDeleteFunc)  (void *)patchData->operatorDeleteCode;
        patchData->callCrtHeapWalk      = (PagallocHeapWalkFunc)        (void *)patchData->heapWalkCode;
        patchData->callCrtStrdup        = (PagallocStrdupFunc)          (void *)patchData->strdupCode;
        patchData->callCrtWcsdup        = (PagallocWcsdupFunc)          (void *)patchData->wcsdupCode;

        /* Write-protect this page */
        DWORD oldProt;
        VirtualProtect (patchData, sizeof(*patchData), PAGE_EXECUTE_READ, &oldProt);


#if defined (STANDALONE) || defined (LMSERVER)
        /* if we do not have toolsubs available, resolve these addresses locally */
        OutputDebugString(_T("PAGALLOC: patching MSVCRT using static addresses.\r\n"));
        toolMallocP         = (Byte *)               pagalloc_malloc;
        toolNhMallocP       = (Byte *)               pagalloc_nh_malloc;
        toolCallocP         = (Byte *)               pagalloc_calloc;
        toolReallocP        = (Byte *)               pagalloc_realloc;
        toolFreeP           = (Byte *)               pagalloc_free;
        toolMsizeP          = (Byte *)               pagalloc_msize;
        toolExpandP         = (Byte *)               pagalloc_expand;
        toolOperatorNewP    = (Byte *)               pagalloc_operatorNew;
        toolOperatorDeleteP = (Byte *)               pagalloc_operatorDelete;
        toolHeapWalkP       = (Byte *)               pagalloc_heapWalk;
        toolStrdupP         = (Byte *)               pagalloc_strdup;
        toolWcsdupP         = (Byte *)               pagalloc_wcsdup;
        toolSetFreeHP       = (PagallocSetFreeHFunc) pagalloc_set_free_handler;
        toolSetMsizeHP      = (PagallocSetMsizeHFunc)pagalloc_set_msize_handler;
#else /* !STANDALONE && !LMSERVER */

#ifndef DGNPLATFORM_TOOLS_DLL_NAME // Defined in *policy.mki  -
// TODO: right now anything that includes this needs to define DGNPLATFORM_TOOLS_DLL_NAME if they don't want DgnPlatformTools3.dll in their policy that they are using to compile any *cpp that includes this file.
#define DGNPLATFORM_TOOLS_DLL_NAME "DgnPlatform5.dll"
#endif

        /* if we have toolsubs available, now load it to resolve these addresses */
        hTool = LoadLibrary(_T(DGNPLATFORM_TOOLS_DLL_NAME)); // Defined in *policy.mki
        if (!hTool)
            {
            paPatch_complain("PAGALLOC: attempt to patch CRT failed because " DGNPLATFORM_TOOLS_DLL_NAME " not found.\r\n");
            return false;
            }
        else
            {
            toolMallocP         = (Byte *)           GetProcAddress (hTool, "pagalloc_malloc");
            toolNhMallocP       = (Byte *)           GetProcAddress (hTool, "pagalloc_nh_malloc");
            toolCallocP         = (Byte *)           GetProcAddress (hTool, "pagalloc_calloc");
            toolReallocP        = (Byte *)           GetProcAddress (hTool, "pagalloc_realloc");
            toolFreeP           = (Byte *)           GetProcAddress (hTool, "pagalloc_free");
            toolMsizeP          = (Byte *)           GetProcAddress (hTool, "pagalloc_msize");
            toolExpandP         = (Byte *)           GetProcAddress (hTool, "pagalloc_expand");
            toolOperatorNewP    = (Byte *)           GetProcAddress (hTool, "pagalloc_operatorNew");
            toolOperatorDeleteP = (Byte *)           GetProcAddress (hTool, "pagalloc_operatorDelete");
            toolHeapWalkP       = (Byte *)           GetProcAddress (hTool, "pagalloc_heapWalk");
            toolStrdupP         = (Byte *)           GetProcAddress (hTool, "pagalloc_strdup");
            toolWcsdupP         = (Byte *)           GetProcAddress (hTool, "pagalloc_wcsdup");
            toolSetFreeHP       = (PagallocSetFreeHFunc) GetProcAddress (hTool, "pagalloc_set_free_handler");
            toolSetMsizeHP      = (PagallocSetMsizeHFunc)GetProcAddress (hTool, "pagalloc_set_msize_handler");
            }
#endif /* !STANDALONE && !LMSERVER */

        /* Now patch the runtime image - insert a branch to our replacement functions for each memory allocation function in the runtime. */
        patchRuntimeFunction (crtMallocP,           toolMallocP);
        //Gone on VS 9/10
        if(_MSC_VER < 1500)
            patchRuntimeFunction (crtNhMallocP,     toolNhMallocP);
        patchRuntimeFunction (crtCallocP,           toolCallocP);
        patchRuntimeFunction (crtReallocP,          toolReallocP);
        patchRuntimeFunction (crtFreeP,             toolFreeP);
        patchRuntimeFunction (crtMsizeP,            toolMsizeP);
        patchRuntimeFunction (crtExpandP,           toolExpandP);
        patchRuntimeFunction (crtOperatorNewP,      toolOperatorNewP);
        patchRuntimeFunction (crtOperatorDeleteP,   toolOperatorDeleteP);
        patchRuntimeFunction (crtHeapWalkP,         toolHeapWalkP);
        patchRuntimeFunction (crtStrdupP,           toolStrdupP);
        patchRuntimeFunction (crtWcsdupP,           toolWcsdupP);

        /* set free and msize failure handler functions, used to manipulate non-pagalloced memory. */
        if (toolSetFreeHP)
            (*toolSetFreeHP)(patchData->callCrtFree);

        if (toolSetMsizeHP)
            (*toolSetMsizeHP)(patchData->callCrtMsize);

        return true;
    }
#endif // !defined (PAPATCH_No_MSCVRT_Patch_Tables)
#endif // defined __papatch_h__

