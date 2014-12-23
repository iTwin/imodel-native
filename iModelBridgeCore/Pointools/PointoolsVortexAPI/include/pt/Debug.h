/*--------------------------------------------------------------------------*/ 
/*  Debug.h																	*/ 
/*	Debugging helper														*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK | All Rights Reserved				*/  
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_DEBUG_H
#define POINTOOLS_DEBUG_H

#ifdef _WIN32
    #include <crtdbg.h>
#endif

#include <pt/debugPrintf.h>
#include <pt/debugAssert.h>

namespace pt {

#ifdef WIN32
    /* Turn off 64-bit warnings		*/ 
    #pragma warning( disable : 4312)
    #pragma warning( disable : 4267)
    #pragma warning( disable : 4311)
#endif


/**
 Useful for debugging purposes.  Note: On windows, 
 this will helpfully return "false" for a stack pointer.
 */
inline bool isValidHeapPointer(const void* x) {
    #ifdef WIN32
        return (_CrtIsValidHeapPointer(x) != 0) && (x != (void*)0xcccccccc) && (x != (void*)0xdeadbeef) && (x != (void*)0xfeeefeee);
    #else
        return x != NULL;
    #endif
}

/**
 Returns true if the pointer is likely to be
 a valid pointer (instead of an arbitrary number). 
 Useful for debugging purposes.
 */
inline bool isValidPointer(const void* x) {
    #ifdef WIN32
        return (_CrtIsValidPointer(x, 0, true) != 0) && (x != (void*)0xcccccccc) && (x != (void*)0xdeadbeef) && (x != (void*)0xfeeefeee);
    #else
        return x != NULL;
    #endif
}

#ifdef WIN32
    #pragma warning( default : 4312)
    #pragma warning( default : 4267)
    #pragma warning( default : 4311)
#endif

}

#endif
