/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


/*
#ifdef flexwiki
:Title: struct Bentley::RotMatrix

Summary: A RotMatrix is a 3x3 matrix.

#endif
*/

BEGIN_BENTLEY_NAMESPACE
struct TransformedDRange3d;
typedef RefCountedPtr<TransformedDRange3d> TransformedDRange3dPtr;

//! Flag variables and query methods to summarize in/out classifications.
struct InOutStates
    {
    //! If false, the implementation has not conclusively classified all points.
    bool m_isFullyClassified;
    //! If true, inside points have been confirmed.
    bool m_hasInsidePoints;
    //! If true, outside points have been confirmed.
    bool m_hasOutsidePoints;
    //! construct with all states false.
    InOutStates (): m_isFullyClassified (false), m_hasInsidePoints(false), m_hasOutsidePoints(false){}
    //! construct with all given states:
    InOutStates(bool isFullyClassified, bool hasInsidePoints, bool hasOutsidePoints) :
        m_isFullyClassified(isFullyClassified),
        m_hasInsidePoints(hasInsidePoints),
        m_hasOutsidePoints(hasOutsidePoints) {}
    //! return a state with all fields true ...
    static InOutStates DefiniteMixed () {return InOutStates (true, true, true); }
    //! return a state with all fields true ...
    static InOutStates DefiniteAllIn() { return InOutStates(true, true, false); }
    //! return a state with all fields true ...
    static InOutStates DefiniteAllOut() { return InOutStates(true, false, true); }
    //! Test if classification is [TTT] or [F??]
    bool IsMixedOrAmbiguous() const
        {
        return m_isFullyClassified ? (m_hasInsidePoints && m_hasOutsidePoints) : true;
        }
    //! Test if classification has definites for both in and out [?TT]
    bool IsMixed() const {return m_hasInsidePoints && m_hasOutsidePoints;}

    bool IsAllInside() const
        {
        return m_isFullyClassified && m_hasInsidePoints && !m_hasOutsidePoints;
        }
    //! Test if classification is [TF?]
    bool IsAllOutside() const
        {
        return m_isFullyClassified && !m_hasInsidePoints && m_hasOutsidePoints;
        }
    bool HasAnyInside() const
        {
        return m_hasInsidePoints;
        }
    //! Execute an inplace "or" between each member bool and its partner in "other"
    void OrInOutPartsInPlace(InOutStates const &other)
        {
        m_hasInsidePoints |= other.m_hasInsidePoints;
        m_hasOutsidePoints |= other.m_hasOutsidePoints;
        }
    //! if pointIsInside is true make m_hasInsidePoints true
    //! if pointIsInside is false make m_hasOutsidePoints true
    void AnnouncePartialClassification(bool pointIsInside)
        {
        if (pointIsInside)
            m_hasInsidePoints = true;
        else
            m_hasOutsidePoints = true;
        }
    };

//!
//! A TransformedDRange3d has (as private members)
//! <ul>
//! <li> A DRange3d
//! <li> A localToWorld Transform to place (rotate, scale, skew) that range in space
//! <li> The worldToLocal inverse transform.
//! <li> The 8 corners of the range after transform.
//! </ul>
//! The goal of the TransformedDRange3d is to anwer clip queries with "quick exits" for obvious cases.
//! <ul>
//! <li> Create the structure (as a ref-counted pointee) with TransformedDRange3d::Create (range, localToWorld)
//! <li> The create method will compute the inverse transform and the 8 points of the transformed range.
//! <li> Subsequently query the range for various containment conditions:
//! <ul> 
//! <li> Classify(clipPlane) return ClipPlaneContainment versus a plane.
//! <li> Classify(convexClipPlaneSet) return ClipPlaneContainment versus a plane.
//! <li> IsAllInside(clipPlane) return true if entirely inside the clip plane
//! <li> IsAllInside(convexClipPlaneSet) return true if entirely inside the convex set
//! </ul>
//! </ul>
struct TransformedDRange3d : public RefCountedBase
    {
    private:
        Transform m_localToWorld;
        Transform m_worldToLocal;
        DRange3d   m_localRange;
        //! Computed by constructor . . .
        DPoint3d m_worldCorners[8];
        // Constructor . . .
        // Unconditionally copy args to members.
        // Caller is responsible
        //  a) validity of transforms
        //  b) non-null range.
        TransformedDRange3d(DRange3dCR range, TransformCR localToWorld, TransformCR worldToLocal);

    public:
        //! Assemble a range and its local frame into a TransformedRange.
        //! Returns invalid Ptr if the transform is singular.
        GEOMDLLIMPEXP static TransformedDRange3dPtr Create(DRange3dCR range, TransformCR localToWorld);

        //! get the 8 world corners
        GEOMDLLIMPEXP void GetWorldCorners(bvector<DPoint3d> &corners) const;

        //! Classify the range with respect to a single clip plane
        GEOMDLLIMPEXP InOutStates Classify(ClipPlaneCR plane) const;

        //! Classify the range with respect to a convex set.
        //! <ul>
        //! <li>If allowQuickAmbiguousReturn is true, the method tests range corners, allowing
        //!    <ul>
        //!    <li> Quick determination of _all_ "all in " cases
        //!    <li> Quick determination of a _subset of_  "all out" cases, specifically
        //!               cases where a range is fully outside some single plane of the clipper 
        //!    <li> Quick determination of cases where the range has at least one corner in
        //!              the clipper and one corner outside the clipper.
        //!    <li> Return as ambiguous in other cases.
        //!    <li> The ambiguous case possibility is when the TransformedDRange3d 
        //!             has all corners outside the clipper, but 
        //!             is rotated so that it goes "by an edge" of the clipper at an angle
        //!             such that no single plane of the clipper determines simple out.
        //!    </ul>
        //!    <li> If allowQuickAmbiguousReturn is false, the ambiguous case is followed by
        //!          a more detailed (and somewhat expensive) polygon clip step.
        //! </ul>
        GEOMDLLIMPEXP InOutStates Classify(ConvexClipPlaneSetCR clipper,
            bool allowQuickAmbiguousReturn) const;

        //! Classify the range with respect to a collection of convexsets.
        GEOMDLLIMPEXP InOutStates Classify(ClipPlaneSetCR clipper) const;

        //! Classify the range with respect to a collection of convexsets.
        //! Return true if
        //!    1) any vertex
        //!    2) any edge
        //!    3) any face
        //!  is inside the clipper.
        //! If none of those each convexi of the clipper must be fully in or out.  Hence
        //!    test any point of the convexi as in or out.
        //! Otherwise return false.
        GEOMDLLIMPEXP bool IsAnyPointInsideClipper(ClipPlaneSetCR clipper) const;
        //! Return true if the range is completely inside the plane.
        //! Any "out" point is quick "false", i.e. "mixed" and "all out" cases are not distinguished
        GEOMDLLIMPEXP bool IsAllInside(ClipPlaneCR plane) const;
        //! Return true if the range is completely inside the plane.
        //! Any "out" point is quick "false", i.e. "mixed" and "all out" cases are not distinguished
        GEOMDLLIMPEXP bool IsAllInside(ConvexClipPlaneSetCR clipper) const;
    };

END_BENTLEY_NAMESPACE
