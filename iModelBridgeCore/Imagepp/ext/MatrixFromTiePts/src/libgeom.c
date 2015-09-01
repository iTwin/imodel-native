/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/libgeom.c $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ SAIG inc.
**  libgeom.c
**
**      This module contains the functions for performing various math
**      operations
**
**  FUNCTIONS
**
**      simple_angle
**
**===========================================================================*/

#include <math.h>

#include "libgeom.h"


/*fh=======================================================================
**
**  simple_angle()
**
**  DESCRIPTION
**
**      The simple_angle() function simply makes sure that an angle given
**      in radians between -PI to 2PI is returned as an angle
**      from 0 to 2PI without changing the effective angle value.
**
**  PARAMETERS
**
**      angl - A double variable that contains the angle to simplify
**
**  RETURN VALUE
**
**      double : an angle from 0 to 2*PI
**
**  SEE ALSO
**
**      angle()
**
**  Alain Robert April 1992 (Original version)
**
**=====================================================================*/
double simple_angle(
double angl)
{
  if (angl < 0.0)
     return(angl + (2 * PI));
  else
     return(angl);
}

