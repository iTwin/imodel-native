/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/oldhmrmacro.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <malloc.h>
#include <assert.h>
#include "oldhmrtypes.h"


/*====================================================================
**  Definition de macro
**====================================================================*/
/*
** -----------------------------------------------------------------------
**                    Macros for class implementation
**
**  To use thes macros, define EXPOSED_CLASS for a class implementation
**  based on a staic object.  Define PRIVATE_CLASS for a class implementation
**  based on a dynamically allocated object.
**
**  Macro                 Description
**  --------------------  -----------------------------------------------
**
**  THIS or HTHIS         The pointer to the object which gives access
**                        to its private attributes.
**
**  OBJECT_PRECONDITIONS  Checks the pre-conditions on the object.  Use for
**                        all the methods except the constructor.
**
** -----------------------------------------------------------------------
*/
#if defined(__HMR_USE_HTHIS)
#   if defined(EXPOSED_CLASS)
#       define HTHIS (OBJECT)
#       define HOBJECT_PRECONDITIONS  \
            HPRECONDITION(HTHIS);
#   endif

#   if defined(PRIVATE_CLASS)
#       define CLASS_IMPLEMENTATION
#       define HTHIS (*OBJECT)
#       define HOBJECT_PRECONDITIONS  \
            HPRECONDITION(OBJECT);     \
            HPRECONDITION(HTHIS);
#   endif
#else
#   if defined(EXPOSED_CLASS)
#       define THIS (OBJECT)
#       define HOBJECT_PRECONDITIONS  \
            HPRECONDITION(THIS);
#   endif

#   if defined(PRIVATE_CLASS)
#       define CLASS_IMPLEMENTATION
#       define THIS (*OBJECT)
#       define HOBJECT_PRECONDITIONS  \
            HPRECONDITION(OBJECT);     \
            HPRECONDITION(THIS);
#   endif
#endif

/*
** -----------------------------------------------------------------------
**  HDECLARE_CLASS(name,attr) -
**
** -----------------------------------------------------------------------
*/
#if defined(CLASS_IMPLEMENTATION)
#   define HDECLARE_CLASS(name,attr) typedef attr * name
#else
#   define HDECLARE_CLASS(name,attr) typedef void * name
#endif

/*
** -----------------------------------------------------------------------
**  HPOSTCONDITION(expr) - Assertion on entry conditions
**
**                         See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HPRECONDITION(expr)
#else
#   define HPRECONDITION(expr) HASSERT(expr)
#endif

/*
** -----------------------------------------------------------------------
**  HPOSTCONDITION(expr) - Assertion on exit conditions
**
**                         See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HPOSTCONDITION(expr)
#else
#   define HPOSTCONDITION(expr) HASSERT(expr)
#endif

/*
** -----------------------------------------------------------------------
**  HDEBUGCODE(expr) - Allows to enter a line of code that compiles only
**                     in debug mode
**
**  Example:  HDEBUGCODE(int i=0;);
**            HDEBUGCODE(printf("I: %d\n",i););
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HDEBUGCODE(expr)
#else
#   define HDEBUGCODE(expr) expr
#endif


/*
** -----------------------------------------------------------------------
**  HASSERT(expr) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HASSERT(expr)
#else
#   define HASSERT(expr) BeAssert((int)(expr))
#endif


/*
** --------------------------------------------------------------------------
**  HDOUBLE_EQUAL(v1, v2, precision) -
**
**      if (HDOUBLE_EQUAL (v1, v2, 0.000001))
** --------------------------------------------------------------------------
*/
#define HGLOBAL_EPSILON (1.0E-14)

#define HDOUBLE_EQUAL_EPSILON(v1,v2)  ((v1 <= (v2+HGLOBAL_EPSILON)) && (v1 >= (v2-HGLOBAL_EPSILON)))
#define HDOUBLE_EQUAL(v1,v2,precision)  ((v1 <= (v2+precision)) && (v1 >= (v2-precision)))

#define HDOUBLE_GREATER_OR_EQUAL_EPSILON(v1, v2) (v1 >= (v2-HGLOBAL_EPSILON))
#define HDOUBLE_GREATER_OR_EQUAL(v1, v2, precision) (v1 >= (v2-precision))

#define HDOUBLE_SMALLER_OR_EQUAL_EPSILON(v1, v2) (v1 <= (v2+HGLOBAL_EPSILON))
#define HDOUBLE_SMALLER_OR_EQUAL(v1, v2, precision) (v1 <= (v2+precision))

#define HDOUBLE_GREATER_EPSILON(v1, v2) (v1 > (v2+HGLOBAL_EPSILON))
#define HDOUBLE_GREATER(v1, v2, precision) (v1 > (v2+precision))

#define HDOUBLE_SMALLER_EPSILON(v1, v2) (v1 < (v2-HGLOBAL_EPSILON))
#define HDOUBLE_SMALLER(v1, v2, precision) (v1 < (v2-precision))


#if !defined(round)
#if defined (_WIN32) && (_MSC_VER < 1800)
/* #   define round(a) ((long)((a)<0.0?(a)-0.5:(a)+0.5)) */
#   define round(a) ((long)(HDOUBLE_SMALLER_EPSILON((a),0.0)?(a)-0.5:(a)+0.5))
#endif
#endif

#if !defined(SIGN)
#   define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#endif

#if !defined(SQR)
#   define SQR(a)     ((a) * (a))
#endif

#if !defined(ARG_USED)
#   define ARG_USED(a)           (a=a)
#endif



