/*--------------------------------------------------------------------------*/ 
/*  Debug.h																	*/ 
/*	Debugging helper														*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK | All Rights Reserved				*/  
/*--------------------------------------------------------------------------*/ 

#pragma once

#ifdef _WIN32
    #include <crtdbg.h>
#endif

#include <pt/debugPrintf.h>
#include <pt/debugAssert.h>

namespace pt {
    
/**
 Useful for debugging purposes.  Note: On windows, 
 this will helpfully return "false" for a stack pointer.
 */
inline bool isValidHeapPointer(const void* x) {
    #ifdef _WIN32
        return (_CrtIsValidHeapPointer(x) != 0) && ((INT_PTR) x != (INT_PTR)0xcccccccc) && ((INT_PTR) x != (INT_PTR) 0xdeadbeef) && ((INT_PTR) x != (INT_PTR) 0xfeeefeee);
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
    #ifdef _WIN32
        return (_CrtIsValidPointer(x, 0, true) != 0) && ((INT_PTR) x != (INT_PTR) 0xcccccccc) && ((INT_PTR) x != (INT_PTR) 0xdeadbeef) && ((INT_PTR) x != (INT_PTR)0xfeeefeee);
    #else
        return x != NULL;
    #endif
}

}

