//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* jonfnc.h                                            aec    20-Dec-1992     */
/*----------------------------------------------------------------------------*/
/* Line segment joining function prototypes.                                  */
/*----------------------------------------------------------------------------*/
#pragma once

#include "aecuti.h"

/*----------------------------------------------------------------------------*/
/* Constants.                                                                 */
/*----------------------------------------------------------------------------*/

#define JON_C_OPNSTR	   0x0         /* polygon is open                     */
#define JON_C_CLSSTR	   0x1         /* polygon is closed                   */
#define JON_C_NODUPLICATE  0x2         /* do not allows duplicate segs.       */
#define JON_C_SLOWMODE     0x4         /* do not use mantissa based hashing   */

#define JON_C_HSHSIZ	    97         /* Internal hash table size (default)  */


/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

struct AECjoinPoint
{
    struct AECjoinPoint *hshlnk;
    DPoint3d p;
    unsigned long lnk;
};

struct AECjoin
{
    struct AECjoinPoint **hsh;
    unsigned long nBuckets;
    int (*usrfnc)(void *,int,long,DPoint3d *);
    int opt;
    void *dat;
    void *mdlDescP;
    struct AECjoinPoint *frepntP;
    DPoint3d *vrtsP;
    unsigned long nvrt;
};




/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

struct AECjoin *aecJoin_create /* <= segment joiner descriptor     */
(
  void *mdlDescP,                      /* => mdl desc. (or NULL)              */
  int (*usrFunctionP)(void *,int,      /* => func to call                     */
      long,DPoint3d *),
  void *userDataP,                     /* => pointer to user data             */
  int opt                              /* => options                          */
);

int aecJoin_flush           /* <= TRUE if error                    */
(
  struct AECjoin *joinP                /* => pointer to joining desc.         */
);

int aecJoin_free            /* <= TRUE if error                    */
(
  struct AECjoin *joinP                /* => pointer to joining desc.         */
);

int aecJoin_cleanup         /* <= TRUE if error                    */
(
  void
);

int aecJoin_add             /* <= TRUE if error                    */
(
  struct AECjoin *joinP,               /* => pointer to joining desc.         */
  DPoint3d *p0,                        /* => first point to add               */
  DPoint3d *p1                         /* => second point to add              */
);

void aecJoin_setOption
(
  struct AECjoin *seg,                 /* => join descriptor                  */
  int opt                              /* => options to set                   */
);

