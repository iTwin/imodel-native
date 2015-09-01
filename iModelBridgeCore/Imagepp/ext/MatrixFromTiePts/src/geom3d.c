/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/geom3d.c $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ SAIG inc.
**  geom3d.c
**
**      This module contains the functions for performing 3D math operations
**
**===========================================================================*/

#include <math.h>

#include "libgeom.h"

/*fh=======================================================================
**  angle
**
**  Alain Robert oct 1991
**=========================================================================*/
double angle(
const DCOORD *pt1,
const DCOORD *pt2)
{
  double delta_x, delta_y;

  delta_x = pt2->x - pt1->x;
  delta_y = pt2->y - pt1->y;

  if (delta_x == 0) {
    /* limit conditions */

    if (delta_y > 0) {

      return(PI / 2);

    }
    else {
      if (delta_y == 0) {

        return(0.0);

      }
      else {
         /* delta_y < 0 */

        return(3 * PI / 2);

      }
    }
  }
  else {
    /* general case */
    if (delta_x < 0.0) {

      return(simple_angle((atan(delta_y / delta_x)) + PI));

    }
    else {

      return(simple_angle(atan(delta_y / delta_x)));

    }
  }



}












