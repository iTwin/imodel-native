/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/dplane3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
Origin and normal vector for a plane.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DPlane3d
{
//! reference point on plane.
DPoint3d origin;
//! vector perpendicular to the plane.  NOT necessarily a unit vector.
DVec3d   normal;

#ifdef __cplusplus

//BEGIN_FROM_METHODS

//!
//! @description Store origin and unnormalized vector.
//! @param [in] x0 x-coordinate of origin point
//! @param [in] y0 y-coordinate of origin point
//! @param [in] z0 z-coordinate of origin point
//! @param [in] ux x-coordinate of normal vector
//! @param [in] uy y-coordinate of normal vector
//! @param [in] uz z-coordinate of normal vector
//! 
//!
static DPlane3d FromOriginAndNormal
(
double          x0,
double          y0,
double          z0,
double          ux,
double          uy,
double          uz
);

//!
//! @description Store origin and unnormalized vector.
//! @param [in] origin origin point
//! @param [in] normal normal vector
//! 
//!
static DPlane3d FromOriginAndNormal
(
DPoint3dCR      origin,
DVec3dCR        normal
);

//!
//! @description 
//! @param [in] normal normal vector.  The vector is normalized inside this method.
//! @param [in] distance (signed) distance from origin to the plane.
//! 
//!
static ValidatedDPlane3d FromNormalAndDistance
(
DVec3dCR        normal,
double          distance
);
//!
//! @description Initialize with first point as origin, normal as unnormalized cross product of vectors
//!   to 2nd and 3rd points.
//! @param [in] origin origin point
//! @param [in] xPoint first point in plane (e.g., x-axis point)
//! @param [in] yPoint second point in plane (e.g., y-axis point)
//! 
//!
static DPlane3d From3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);


//END_FROM_METHODS

/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/
//!
//! @description Store origin and unnormalized vector.
//! @param [in] x0 x-coordinate of origin point
//! @param [in] y0 y-coordinate of origin point
//! @param [in] z0 z-coordinate of origin point
//! @param [in] ux x-coordinate of normal vector
//! @param [in] uy y-coordinate of normal vector
//! @param [in] uz z-coordinate of normal vector
//! 
//!
void InitFromOriginAndNormal
(
double          x0,
double          y0,
double          z0,
double          ux,
double          uy,
double          uz
);

//!
//! @description Store origin and unnormalized vector.
//! @param [in] origin origin point
//! @param [in] normal normal vector
//! 
//!
void InitFromOriginAndNormal
(
DPoint3dCR      origin,
DVec3dCR        normal
);

//!
//! @description Normalize the plane vector.
//! @return true if normal vector has nonzero length.
//! 
//!
bool Normalize ();

//!
//! @description Initialize with first point as origin, normal as unnormalized cross product of vectors
//!   to 2nd and 3rd points.
//! @param [in] origin origin point
//! @param [in] xPoint first point in plane (e.g., x-axis point)
//! @param [in] yPoint second point in plane (e.g., y-axis point)
//! 
//!
void InitFrom3Points
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);

//!
//! @description Extract origin and normal from 4D plane coefficients.
//! @param [in] hPlane 4D plane coefficients
//! @return true if plane has a nonzero normal
//! 
//! 
//!
bool Init (DPoint4dCR hPlane);

//!
//! @description Return the plane as a DPoint4d.
//! @param [out] hPlane 4D plane coefficients
//! 
//! 
//!
void GetDPoint4d (DPoint4dR hPlane) const;

//!
//! @description Convert the implicit plane ax+by+cz=d to origin-normal form, with a unit normal vector.
//! @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
//!       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
//! @param [in] a 4D plane x-coefficient
//! @param [in] b 4D plane y-coefficient
//! @param [in] c 4D plane z-coefficient
//! @param [in] d 4D plane constant coefficient
//! @return true if plane has a nonzero normal
//! 
//! 
//!
bool Init
(
double          a,
double          b,
double          c,
double          d
);

//!
//! @description Convert the plane to implicit coeffcients ax+by+cz=d.
//! @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
//!       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
//! @param [out] coffA 4D plane x-coefficient
//! @param [out] coffB 4D plane y-coefficient
//! @param [out] coffC 4D plane z-coefficient
//! @param [out] coffD 4D plane constant coefficient
//! 
//! 
//!
void GetCoefficients
(
double          &coffA,
double          &coffB,
double          &coffC,
double          &coffD
) const;


//!
//! @description Fill the plane data with zeros.
//! 
//!
void Zero ();

//!
//! @description Test if the numeric entries in the plane are all absolutely zero (no tolerances).
//! @return true if the plane contains only zero coordinates.
//! 
//!
bool IsZero () const;

//!
//! @description Project a (generally off-plane) point onto the plane.
//! @param [out] projection projection of point onto the plane
//! @param [in] point point to project to plane
//! @return true if the plane has a well defined normal.
//! 
//!
bool ProjectPoint
(
DPoint3dR       projection,
DPoint3dCR      point
) const;

//!
//! @description Dot the plane normal with the vector from the plane origin to the point.
//! @remarks If the plane normal is a unit vector, this is the true distance from the
//!       plane to the point.  If not, it is a scaled distance.
//! @param [in] point point for evaluation
//! @return dot product
//! 
//!
double Evaluate (DPoint3dCR point) const;

//!
//! @description Dot the plane normal with the given vector.
//! @remarks If the plane normal is a unit vector, this is the true distance of altitude change due to motion along by this vector.
//!       If not, it is a scaled distance.
//! @param [in] vector vector for evaluation
//! @return dot product
//! 
//!
double EvaluateVector (DVec3dCR vector) const;

//! @return The maximum absolute value of plane evaluation among points in an array.
//! @param [in] points points to search
//! @param [in] n number of points.
double EvaluateMaxAbs (DPoint3dCP points, size_t n) const;

//! @return The maximum absolute value of plane evaluation among points in an array.
//! @param [in] points points to search
double EvaluateMaxAbs (bvector <DPoint3d> const &points) const;

//! @return min and max signed evaluations among points in an array.
//! @param [in] points points to search
//! @param [in] n number of points
//! @param [out] maxIndex index where (signed) max occurs.
//! @param [out] minIndex index where (signed) min occurs.
DRange1d EvaluateRange (DPoint3dCP points, size_t n, size_t   &minIndex, size_t &maxIndex) const;

//! @return min and max signed evaluations among points in an array.
//! @param [in] points points to search
//! @param [out] maxIndex index where (signed) max occurs.
//! @param [out] minIndex index where (signed) min occurs.
DRange1d EvaluateRange (bvector <DPoint3d> const &points, size_t   &minIndex, size_t &maxIndex) const;



//!
//! @description Compute the origin and normal so the plane passes (approximiately) through the array of points.
//! @param [in] points array of points defining the plane
//! @return true if the points define a clear plane (and are close to it!!!)
//!     false if the points are (a) closely clustered, (b) nearly colinear, or (c) not close to a single plane.
//!
bool InitFromArray (bvector<DPoint3d> const &points);
//!
//! @description Compute the origin and normal so the plane passes (approximiately) through the array of points.
//! @param [in] pointArray (pointer to) array of points defining the plane
//! @param [in] numPoint number of points.
//! @return true if the points define a clear plane (and are close to it!!!)
//!     false if the points are (a) closely clustered, (b) nearly colinear, or (c) not close to a single plane.
//!
bool InitFromArray (DPoint3dCP pointArray, int numPoint);

//!
//! @description Compute the origin and normal of some plane that is close to all the points.
//!     The plane will be based on a search for 3 widely separated points.
//!     This method does not try to decide if the maxAbsDistance is small enough so that
//!      the plane can be considered close to all the points -- a true return does not 
//!      mean the points are close to coplanar.
//! @param [in] points array of points defining the plane
//! @param [out] maxAbsDistance largest absolute distance from any point to the plane.
//! @return false if the points are (a) closely clustered or (b) nearly colinear
bool InitFromArray (bvector<DPoint3d> const &points, double &maxAbsDistance);
//!
//! @description Compute the origin and normal of some plane that is close to all the points.
//!     The plane will be based on a search for 3 widely separated points.
//!     This method does not try to decide if the maxAbsDistance is small enough so that
//!      the plane can be considered close to all the points -- a true return does not 
//!      mean the points are close to coplanar.
//! @param [in] pointArray (pointer to) array of points defining the plane
//! @param [in] numPoint number of points.
//! @param [out] maxAbsDistance largest absolute distance from any point to the plane.
//! @return false if the points are (a) closely clustered or (b) nearly colinear
bool InitFromArray (DPoint3dCP pointArray, int numPoint, double &maxAbsDistance);



//! Compute the point of intersection of 3 planes given as normal and distance from origin.
//!<ul>
//!<li>All normals are ASSUMED to be unit length.
//</ul>
//! @return valid point if the three normals are independent.  Otherwise returns origin of planeA, but marked invalid.
static ValidatedDPoint3d Intersect3Planes
(
DVec3dCR unitNormalA,  //!< [in] unit normal to plane A
double distanceA,  //!< [in] distance from origin to plane A (i.e. measured along the direction of normalA)
DVec3dCR unitNormalB,  //!< [in] unit normal to plane B
double distanceB,  //!< [in] distance from origin to plane B (i.e. measured along the direction of normalB)
DVec3dCR unitNormalC,  //!< [in] unit normal to plane C
double distanceC   //!< [in] distance from origin to plane B (i.e. measured along the direction of normalC)
);

//! Compute the point of intersection of 3 planes given as DPlane3d ....
//! @return valid point if the three normals are independent.  Otherwise returns origin of planeA, but marked invalid.
static ValidatedDPoint3d Intersect3Planes
(
DPlane3dCR planeA,  //!< [in] first plane
DPlane3dCR planeB,  //!< [in] first plane
DPlane3dCR planeC  //!< [in] first plane
);

//! Compute a plane, perpendicular to the line between xyzA and xyzB.
//! The voronoiMetric selects the assignment of "bisectors" by one of these rules.
//!<ul>
//!<li>0 is simple bisector
//!<li>1 is split the distance between circles of specified radii.
//!<li>2 is ratio of radii.
//!<li>3 is the power method (https://en.wikipedia.org/wiki/Power_diagram).  This produces the best intersection points !!!
//! Using these split planes "around a vertex" of a Delauney triangulation creates voronoi regions weighted by the radii.
//!</ul>
static ValidatedDPlane3d VoronoiSplitPlane
(
DPoint3dCR xyzA,    //!< [in] start point
double rA,          //!< [in] start weight (radius)
DPoint3dCR xyzB,    //!< [in] end point
double rB,          //!< [in] 
int voronoiMetric   //!< [in] method to place plane origin.  See list above.
);
#endif
};
END_BENTLEY_GEOMETRY_NAMESPACE
