//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMemcpy.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCMemcpy
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCMemcpy.h>

#define WordSize  4

#if defined(_WIN32) && !defined(_M_X64)

void* ImagePP::HFCMemcpy (void* po_pDst, const void* pi_pSrc, size_t pi_Count)
    {
    __asm {
        mov     edi,[po_pDst]
        mov     esi,[pi_pSrc]
        mov     ecx,[pi_Count]

//
// The algorithm for forward moves is to align the destination to a dword
// boundary and so we can move dwords with an aligned destination.  This
// occurs in 3 steps.
//
//   - move x = ((4 - Dest & 3) & 3) bytes
//   - move y = ((L-x) >> 2) dwords
//   - move (L - x - y*4) bytes
//
        test    edi,11b          // destination dword aligned?
        jnz     short byterampup // if we are not dword aligned already, align

        mov     eax,ecx         // byte count
        and     eax,11b         // trailing byte count
        shr     ecx,2           // shift down to dword count
        rep     movsd           // move all of our dwords

        jmp     short TrailingVecs


//
// Simple copy, byte at a time. This could be faster with a jump table and
// split-out optimizations, copying as much as possible a dword/word at a
// time and using MOV with displacements, but such short cases are unlikely
// to be called often (it seems silly to call a function to copy less than
// three dwords).
//
        align   WordSize
        ShortMove:
        rep movsb
        jmp short memcpyLongExit


//
// Code to do optimal memory copies for non-dword-aligned destinations.
//
        align   WordSize
        byterampup:

// The following length check is done for two reasons:
//
//    1. to ensure that the actual move length is greater than any possiale
//       alignment move, and
//
//    2. to skip the multiple move logic for small moves where it would
//       be faster to move the bytes with one instruction.
//
// Leading bytes could be handled faster via split-out optimizations and
// a jump table (as trailing bytes are), at the cost of size.
//
// At this point, ECX is the # of bytes to copy, and EDX is the # of leading
// bytes to copy.
//
        cmp     ecx,12                  // check for reasonable length
        jbe     short ShortMove         // do short move if appropriate
        mov     edx,edi
        neg     edx
        and     edx,11b                 // # of leading bytes
        sub     ecx,edx                 // subtract out leading bytes
        mov     eax,ecx                 // # of bytes remaining after leading
        mov     ecx,edx                 // # of leading bytes
        rep     movsb                   // copy leading bytes
        mov     ecx,eax                 // compute number of dwords to move
        and     eax,11b                 // # of trailing bytes
        shr     ecx,2                   // # of whole dwords
        rep     movsd                   // move whole dwords

        align   WordSize
        TrailingVecs:
        test    eax,eax
        jz      memcpyLongExit
        cmp     eax,3
        jne     short Trail2

//Trail3:
        mov     ax,[esi]
        mov     [edi],ax
        mov     al,[esi+2]
        mov     [edi+2],al
        jmp short memcpyLongExit

        align   WordSize
        Trail2:
        cmp     eax,2
        jne     short Trail1

        mov     ax,[esi]
        mov     [edi],ax
        jmp short memcpyLongExit

        align   WordSize
        Trail1:
        mov     al,[esi]
        mov     [edi],al

        memcpyLongExit:
        }

    return po_pDst;
    }

#endif
