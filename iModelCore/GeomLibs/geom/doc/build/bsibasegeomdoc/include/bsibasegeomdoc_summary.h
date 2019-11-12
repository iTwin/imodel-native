/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/**
@DocText
@Group "Overview of the Bentley Base Geometry Library"

<h2>The Bentley Base Geometry Library</h2>

<p>
This document provides a brief overview and specific functional documentation for internal developers who seek to leverage the basic
geometric functionality in Bentley's Base Geometry Library.

<p>
The Bentley Base Geometry Library is independent of MicroStation and has no memory management dependencies or issues.

<h3>Library Functionality</h3>

<p>
The Bentley Base Geometry Library provides functionality for creating, querying and transforming basic 2D and 3D geometric objects ranging
from points, vectors, lines, planes, ellipses and range boxes, to analytic surfaces and homogeneous entities.  Each object is represented by
a C struct and supported by functions prefixed with <code>bsi<i>StructName</i>_</code>.

<h4>Preferred Geometry Types</h4>

<p>Below are the geometry types of the Bentley Base Geometry Library that typically see the most usage.

<p>
<table border=1 cellpadding="5%" cols=2>
<tr>
        <th>Geometry Type</th>
        <th>Description</th>
</tr>
<tr>
        <td>~tDPoint3d</td>
        <td>Cartesian point</td>
</tr>
<tr>
        <td>~tDVec3d</td>
        <td>Cartesian vector</td>
</tr>
<tr>
        <td>~tDSegment3d</td>
        <td>line segment</td>
</tr>
<tr>
        <td>~tDPlane3d</td>
        <td>plane, defined by point and vector</td>
</tr>
<tr>
        <td>~tDRange3d</td>
        <td>world axis-aligned 3D box</td>
</tr>
<tr>
        <td>~tDEllipse3d</td>
        <td>ellipse or elliptical arc
</tr>
<tr>
        <td>~tRotMatrix</td>
        <td>linear transformation, defined by 3x3 array</td>
</tr>
<tr>
        <td>~tTransform</td>
        <td>affine transformation, defined by 3x4 array</td>
</tr>
</table>

<h4>Other Geometry Types</h4>

<P>Below are the less commonly used geometry types in the Bentley Base Geometry Library.

<p>
<table border=1 cellpadding="5%" cols=2>
<tr>
        <th>Geometry Type</th>
        <th>Description</th>
</tr>
<tr>
        <td>~tDPoint2d</td>
        <td>2D Cartesian point</td>
</tr>
<tr>
        <td>~tDRange2d</td>
        <td>world-axis aligned 2D box</td>
</tr>
<tr>
        <td>~tDRay3d</td>
        <td>directed line</td>
</tr>
<tr>
        <td>~tDMatrix3d</td>
        <td>linear transformation, defined by 3 columns</td>
</tr>
<tr>
        <td>~tDTransform3d</td>
        <td>affine transformation, defined by matrix and translation</td>
</tr>
<tr>
        <td>~tDEllipsoid3d</td>
        <td>ellipsoid or ellipsoidal patch</td>
</tr>
<tr>
        <td>~tDCone3d</td>
        <td>cone or conical patch</td>
</tr>
<tr>
        <td>~tDToroid3d</td>
        <td>toroid or toroidal patch</td>
</tr>
<tr>
        <td>~tDDisk3d</td>
        <td>elliptical annulus</td>
</tr>
<tr>
        <td>~tDPoint4d</td>
        <td>homogeneous point, or quaternion</td>
</tr>
<tr>
        <td>~tDSegment4d</td>
        <td>homogeneous line segment</td>
</tr>
<tr>
        <td>~tDConic4d</td>
        <td>homogeneous ellipse or elliptical arc representing a conic section or segment thereof when normalized to 3D
</tr>
<tr>
        <td>~tDMatrix4d</td>
        <td>homogeneous linear transformation (e.g., for perspective view transformations)</td>
</tr>
<tr>
        <td>~tDMap4d</td>
        <td>homogeneous linear transformation and its inverse</td>
</tr>
<tr>
        <td>~tGraphicsPoint</td>
        <td>homogeneous point with accompanying parameter and label information.</td>
</tr>
<tr>
        <td>~tFPoint3d</td>
        <td>single-precision Cartesian point</td>
</tr>
<tr>
        <td>~tFPoint2d</td>
        <td>single-precision 2D Cartesian point</td>
</tr>
<tr>
        <td>~tFRange3d</td>
        <td>single-precision world axis-aligned 3D box</td>
</tr>
<tr>
        <td>~tFRange2d</td>
        <td>single-precision world-axis aligned 2D box</td>
</tr>
</table>

<h3>Library Source</h3>

<p>
The source code for the Bentley Base Geometry Library is housed in the <CODE>geom</CODE> subdirectory of the <CODE>geomlibs</CODE> CVS
module, a separate repository from the rest of the MicroStation source.  There are three subdirectories of <CODE>geomlibs\geom</CODE>:
<CODE>src</CODE>, <CODE>build</CODE> and <CODE>doc</CODE>.

<p>
The C source files are found in <CODE>geomlibs\geom\src</CODE>.  The master header file is
<CODE>geomlibs\geom\src\pubinc\msgeomstructs.h</CODE>.  A core subset of the library also has a C++ API that is not documented here; see the
master header file <CODE>geomlibs\geom\src\pubinc\msgeomstructs.hpp</CODE> and its includes.

<p>
Makefiles are housed in <CODE>geomlibs\geom\build</CODE>.  Each subdirectory builds a different deliverable from the same source.

<p>
This help file can be found at <CODE>geomlibs\geom\doc\bsibasegeomdoc.chm</CODE>.


<h3>Building the Library</h3>

<p>
The Bentley makefile for building <CODE>bsibasegeom.dll</CODE>, which MicroStation uses, is in <CODE>geomlibs\geom\build\bsibasegeom</CODE>.
There is also a Bentley makefile for building <CODE>geom_static.lib</CODE>, a static version of the library, in
<CODE>geomlibs\geom\build\geom_static</CODE>.

<p>
To build one of these deliverables, you first need to checkout the <CODE>geomlibs</CODE>, <CODE>bsitools</CODE> and <CODE>util</CODE>
modules from CVS.  You will actually only need the <CODE>sharedmki</CODE> and <CODE>sharedinc</CODE> subdirectories of the
<CODE>util</CODE> module.  It is best to checkout all three modules directly under the same top-level source directory.  For CVS help, see
the <A href="http://cvsmgt/">CVS User Administration</A> page, or send e-mail to the <a href="mailto:CVS Administrators">CVS
Administrators</a>.

<p>
Next, open up a command shell and execute <CODE>geomlibs\common\geomlibs_env.cmd</CODE>, passing it the pathnames of your top-level
source directory, the desired output directory for objects and deliverables, and the directory of your compiler, all with trailing
backslashes.  This will set up the necessary environment variables, like <CODE>GEOMLIBS_ROOT</CODE> and <CODE>GEOMLIBS_OUT</CODE>.

<p>
Finally, <CODE>bmake</CODE> either <CODE>geomlibs\geom\build\bsibasegeom\bsibasegeom.mke</CODE> or
<CODE>geomlibs\geom\build\geom_static\geom_static.mke</CODE>.
*/

/**
* @doctext
* @group "DPoint3d Barycentric"
* <H2>Barycentric Coordinate Functions</H2>
*
* <P>For a given triangle T with vertices v0, v1, v2, every point q in the plane
* of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
* relative to T:
*
* <PRE>
*   q = b0 * v0 + b1 * v1 + b2 * v2,
*   1 = b0 + b1 + b2.
* </PRE>
*/

/**
* @doctext
* @group "DPoint2d Barycentric"
* <H2>Barycentric Coordinate Functions</H2>
*
* <P>For a given triangle T with vertices v0, v1, v2, every point q in the plane
* of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
* relative to T:
*
* <PRE>
*   q = b0 * v0 + b1 * v1 + b2 * v2,
*   1 = b0 + b1 + b2.
* </PRE>
*/

/**
* @doctext
* @group "Trigonometric Rotations"
* <H2>Trigonometric Rotations</H2>
*
* <P>In linear algebra, several generalized rotations are useful:
* <UL>
* <LI>Givens rotation: one-sided rotation used in QR factorization.
* <LI>Jacobi rotation: two-sided rotation used in symmetric eigensystems.
* <LI>Hyperbolic rotation: one-sided rotation used to preserve signs of characteristic matrices.
* </UL>
*/

/* Transforms: or coordinate frame (local to world transformation, where columns are local axes and origin in world coordinates) */

/* Ellipsoid3d: E by local axes (frame columns) and center (frame translation),
            and min/max longitude angle t &amp; latitude angle p contained in [-pi,pi] &amp; [-pi/2,pi/2], resp.:
            E(t,p) = center + cos(t)*cos(p)*xAxis + sin(t)*cos(p)*yAxis + sin(p)*zAxis */

/* DToroid3d: T by elliptical rail vector0, vector90 &amp; torus axis (frame columns), center (frame translation),
            sectional radius as fraction of rail radius (minorAxisRatio),
            and min/max longitude angle t &amp; latitude angle p both contained in [-pi,pi]:
            T(t,p) = center + (1 + minorAxisRatio*cos(p)) * (cos(t)*vector0 + sin(t)*vector90) + sin(p)*minorAxisRatio*axis */

/* DDisk3d: A (or patch thereof) by vector0 &amp; vector90 (first two frame columns), center (frame translation),
            and min/max radius r &amp; angle t contained in [0,1] and [-pi,pi], resp.:
            A(r,t) = center + r*cos(t)*vector0 + r*sin(t)*vector90 */

/* GraphicsPoint: Various mask values can be used to represent relevant
            points of linestrings, B-spline curves (as B&eacute;zier segments), elliptical arcs, and concatenations of all three. */