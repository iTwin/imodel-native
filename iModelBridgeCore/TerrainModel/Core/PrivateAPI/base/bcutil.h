/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/base/bcutil.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

static inline bool bcUtil_equalDouble
(
    double  val1,
    double  val2,
    double  eps
)
{
    return (fabs (val1 - val2) <= eps);
}


static inline double bcMath_distance2d
(
 double x1,
 double y1,
 double x2,
 double y2
 )
{
	double dx, dy;
    double distance;

	dx = x1 - x2;
	dy = y1 - y2;
	distance = sqrt(dx * dx + dy * dy);
    return distance;
}

