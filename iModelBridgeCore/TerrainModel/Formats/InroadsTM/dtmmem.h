//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmmem.h                                                aec    07-Feb-1994 */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used with functions which allocate and       */
/* deallocate memory for DTM data structures.                                 */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Generate surface data                                                      */
/*----------------------------------------------------------------------------*/

#define GENSRF_PLANAR          0x1     /* planar surface type                 */
#define GENSRF_RANDOM          0x2     /* random surface type                 */
#define GENSRF_NORMAL          0x4     /* bivariate normal                    */
#define GENSRF_COSEXP          0x8     /* cos * exp type                      */
#define GENSRF_PYRMID         0x10     /* pyramidal shape                     */
#define GENSRF_CONIC          0x20     /* conical shape                       */
#define GENSRF_TYPMSK         0x3F     /* type mask                           */
#define GENSRF_SLPEST         0x40     /* eastward sloping                    */
#define GENSRF_SLPWST         0x80     /* westward sloping                    */
#define GENSRF_SLPNRT        0x100     /* northward sloping                   */
#define GENSRF_SLPSTH        0x200     /* southward sloping                   */
#define GENSRF_DIRMSK        0x3C0     /* direction mask                      */
