/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/oldhmrtypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**  oldhmrtypes.h
**
**  This header file defines the following data types:
**
**      Int8       - Boolean
**      char       - Char
**      unsigned char      - Unsigned char
**      signed char      - Signed char
**      UInt16     - Unsigned short
**      Short     - Signed short
**      UInt32      - Unsigned long
**      Int32      - Signed long
**      UInt32       - Unsigned int
**      Int32       - Signed int
**      float      - Float
**      Int8      - Very small signed integer (-128:127)
**      Byte      - Very small unsigned integer (0:255)
**
**  The following constants are defined as general purpose return
**  code values:
**
**      HSUCCESS                           - Success return code
**      HERROR                             - Error return code
**
**  The following constants are also defined (if not already defined)
**
**      TRUE
**      FALSE
**
**====================================================================*/
#ifndef __OLDHMRTYPES_H__
#define __OLDHMRTYPES_H__

#if !defined(NULL)
#define NULL ((void*)0)
#endif

/*====================================================================
**  Definition de constantes
**====================================================================*/
#define HSUCCESS           0
#define HERROR             1

#if !defined(TRUE)
#   define TRUE 1
#endif

#if !defined(FALSE)
#   define FALSE 0
#endif

#if !defined(Public)
#   define Public
#endif

#if !defined(CLASS_IMPLEMENTATION) && defined(__HMR_DEBUG)
#   define CLASS_IMPLEMENTATION
#endif

#include <Bentley/Bentley.r.h>

#endif /*__OLDHMRTYPES_H__*/
