/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//! A DSegment1d is an interval [x0,x1] on the real line.
//! The interval is directed -- x1 may be less than x0.
//! The interval is never null -- as with DSegment3d the endpoints are always present and there is no way to be empty.
//! (Compare to a DRange1d.  Both are defined by two real limit values. DRange1d is deemed EMPTY if its second limit value is below the first.)
struct DSegment1d
{
private:
double m_x0, m_x1;
public:
//! Constructor with distinct endpoints.
GEOMDLLIMPEXP DSegment1d (double x0, double x1);
//! Constructor with single endpoint.
GEOMDLLIMPEXP DSegment1d (double x);

//! Constructor for segment 00.  (Note that this is a single point at 0, not an undefined value)
GEOMDLLIMPEXP DSegment1d () : m_x0(0), m_x1(0){}

//! Change start coordinate.
GEOMDLLIMPEXP void SetStart (double x);
//! Change end coordinate.
GEOMDLLIMPEXP void SetEnd (double x);

//! Return start coordinate.
GEOMDLLIMPEXP double GetStart () const;
//! Return end coordinate.
GEOMDLLIMPEXP double GetEnd () const;

//! return (x-x0) * (x-x1). This is negative "inside" the interval
GEOMDLLIMPEXP double EndPointProduct (double x) const;

//! Test if the EndPointProduct is zero or negative
GEOMDLLIMPEXP bool IsInteriorOrEnd (double x) const;
//! Test if the EndPointProduct is strictly negative
GEOMDLLIMPEXP bool IsStrictInterior (double x) const;

//! Exact equality test.
GEOMDLLIMPEXP bool IsEqual (DSegment1dCR other) const;
//! Exact equality test for reversed coordinates.
GEOMDLLIMPEXP bool IsReversed (DSegment1dCR other) const;

//! Exact equality test for 0.0 to 1.0 interval
GEOMDLLIMPEXP bool Is01 () const;

GEOMDLLIMPEXP bool AlmostEqual (DSegment1d const &other, double tolerance = 0.0) const;

//! Return coordinate at fractional parameter from start to end.
GEOMDLLIMPEXP double FractionToPoint (double fraction) const;
//! Find fractional parameter for specified coordinate.
//! @return false if endpoints are identical.
GEOMDLLIMPEXP bool PointToFraction (double point, double &fraction) const;
//! Return absolute length of the segment.
GEOMDLLIMPEXP double Length () const;
//! Return (signed) distance from start to end
GEOMDLLIMPEXP double Delta () const;

//! Return (signed) distance from end of this segment to start of other.
GEOMDLLIMPEXP double EndToStartDelta (DSegment1dCR other) const;

//! Return absolute distance from end of this segment to start of other.
GEOMDLLIMPEXP double EndToStartDistance (DSegment1dCR other) const;

//! Find overlap of primary with clipper.
//! If empty, return the instance
//! If nonempty, result maintains orientation of primary.  Note that single point intersection is non-empty.
GEOMDLLIMPEXP ValidatedDSegment1d DirectedOverlap (DSegment1dCR other) const;

//! Find overlap of primary with clipper, treating single point intersection as empty.
//! If empty or single point, return false and primary unchanged.
//! If nonempty, result maintains orientation of primary.
GEOMDLLIMPEXP ValidatedDSegment1d NonZeroDirectedOverlap (DSegment1dCR other) const;

//! Find overlap between segmentA and segmentB.
//! Return the overlap as fractions of each.
static GEOMDLLIMPEXP bool NonZeroFractionalDirectedOverlap
(
DSegment1dCR segmentA,  //!< [in] first segment
DSegment1dCR segmentB,  //!< [in] second segment
DSegment1dR fractionA,  //!< [in] overlap expressed as fractional positions in segmentA
DSegment1dR fractionB   //!< [in] overlap expressed as fractional positions in segmentB
);

//! Return a segment with endpoints at fractions of input segment.
GEOMDLLIMPEXP DSegment1d BetweenFractions (double f0, double f1) const;
//! return reverse of the instance
GEOMDLLIMPEXP DSegment1d Reverse () const;
//! return a segment mirrored around 0.5 -- i.e. reverse direction in of 01 fractional coordinates.
GEOMDLLIMPEXP DSegment1d Mirror01 () const;
//! reverse the instance in place
GEOMDLLIMPEXP void ReverseInPlace ();
//! return a copy, adding delta to each end.
GEOMDLLIMPEXP DSegment1d CopyTranslated (double delta) const;

//! return a copy, with each coordinate replaced by its fractional position in the parent.
//! If the parent is zero length, the ValidatedDSegment1d is invalid and has coordinates 00
GEOMDLLIMPEXP ValidatedDSegment1d CopyAsFractionOf (DSegment1dCR parent) const;

};

END_BENTLEY_GEOMETRY_NAMESPACE