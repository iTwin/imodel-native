/**
 @file format.h
 
 @maintainer Morgan McGuire, matrix@graphics3d.com
 
 @author  2000-09-09
 @edited  2004-01-03

 Copyright 2000-2003, Morgan McGuire.
 All rights reserved.
 */

#ifndef G3D_FORMAT_H
#define G3D_FORMAT_H

#include <pt/ptunicode.h>

#ifndef _MSC_VER
    #ifndef __cdecl
        #define __cdecl __attribute__((cdecl))
    #endif
#endif

namespace pt {

/**
  Produces a string from arguments of the style of printf.  This avoids
  problems with buffer overflows when using sprintf and makes it easy
  to use the result functionally.  This function is fast when the resulting
  string is under 160 characters (not including terminator) and slower
  when the string is longer.
 */
std::string   __cdecl format(
    const char*                 fmt
    ...);

/**
  Like format, but can be called with the argument list from a ... function.
 */
std::string vformat(
    const char*                 fmt,
    char*		                va_list);
}; // namespace

#endif
