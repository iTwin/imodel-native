/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#define __msembeddedarrayH__
//!
//! @DocText
//! @Group "Embedded Arrays"
//!
//! <h1> Embedded Arrays</h1>
//! <P><b>msembeddedarray.h</b> is the master include file for the various "rubber array" data types:
//! <ul>
//! <li>EmbeddedIntArray
//! <li>EmbeddedDPoint3dArray
//! </ul>
//!
//! <P>EmbeddedXXXArray's are &quot;rubber&quot; arrays with zero-based integer indexing.  Data can be inserted and removed at positions indicated by an integer index, and memory logic required to allow the size of the array to change are handled by the implementation.      Caller code is able to start with an empty array, add data, and perform its calculations with significantly less concern about memory issues compared to direct use of heap memory via malloc and free.</P>
//!
//! <P>To simplify discussion, this document will talk about EmbeddedDPoint3dArray - a &quot;rubber array&quot; of  DPoint3d structures.   Use of the other array types --- EmbeddedIntArray - follows the same patterns.</P>
//!
//! <P>Embedded arrays are a &quot;gray box&quot; data type.   Most - and even the vast majority of - accesses to the array can be through function calls that hide the underlying implementation, i.e. as a &quot;black box&quot;.  However, there are advanced functions which require the calling programmer to have a general understanding of the fact that the support code is allocating and reallocating contiguous arrays on their behalf.</P>
//!
//! <h2> Creating arrays </h2>
//! <P>The preferred way to get an (empty) array is to &quot;grab&quot; it from a cache maintained by the jmdlEmbeddedXXXArray functions.   The caller should always &quot;drop&quot; the array back into the cache after usage.  The proper balancing of &quot;grab&quot; and &quot;drop&quot; is like normal balancing of &quot;malloc&quot; and &quot;free&quot; - if you don't do it, there are memory leaks.</P>
//!
//! <P>The normal code structure is</P>
//! <pre>
//! // Start by &quot;borrowing&quot; an array from the cache:
//! EmbeddedDPoint3dArray *pMyArray = <B>jmdlEmbeddedDPoint3dArray_grab</B> ();
//! //   .... The borrowed array is empty - <B>jmdlEmbeddedDPoint3dArray_getCount</B> () will return 0.
//! //   .... add and retrieve DPoint3d data as needed here ...
//! //   .... at end, return the borrowed array
//! <B>jmdlEmbeddedDPoint3dArray_drop</B> (pMyArray);
//! </pre>
//!
//! <I><P>Important safety tip: It is strongly recommended that the _grab and _drop for a particular array be physically located in the same lexical code block.   This makes it easy to verify (visually) that grabs and drops are balanced.</P>
//! </I>
//! <P>It is also possible to initialize and free arrays using the function pair</P>
//! <pre>
//!    <B>jmdlEmbeddedDPoint3dArray_new</B> ()
//!    <B>jmdlEmbeddedDPoint3dArray_free</B> ()
//! </pre>
//!
//! <P>Using &quot;<B>jmdlEmbeddedDPoint3dArray_new</B>&quot; and &quot;<B>jmdlEmbeddedDPoint3dArray_free</B>&quot; will incur modest additional overhead because each call will allocate or deallocate memory using the system heap (like malloc).  The grab/drop pair reuses both the fixed sized headers and the variable size blocks, which can provide performance benefits.</P>
//!
//! <h2>Adding data to an embedded array</h2>
//!
//! Use functions in the API to add data to the array.   &quot;index&quot; array indices are <I>0-based</I> in the manner of normal C arrays.</P>
//!
//! <P>1) To add data &quot;at the end of the array&quot;:</P>
//! <pre>
//! DPoint3d mySinglePoint;
//! DPoint3d myArrayOfPoints[100];
//! // initialize mySinglePoint, myArrayOfPoints
//!
//! <B>jmdlEmbeddedDPoint3dArray_addDPoint3d</B> (pMyArray, &amp;mySinglePoint)
//!
//! <B>jmdlEmbeddedDPoint3dArray_addDPoint3dArray</B>
//!        (pMyArray, myArrayOfPoints, numberOfPoints)
//! </pre>
//! <p>Each of these increases the &quot;count&quot; of DPoint3d's in the array.</p>
//!
//! <p>2) To &quot;set&quot; the  DPoint3d at a specific integer &quot;index&quot; in the array, use</P>
//! <P>&#9;<B>jmdlEmbeddedDPoint3dArray_setDPoint3d</B> (pMyArray, &amp;mySinglePoint, index)</P>
//! <P>Because this is a <I>replacement</I> operation, rather than an insert or add, the array count remains unchanged (see exception below)</P>
//!
//! <P>Advanced note: If the index is <I>negative</I>, it is reinterpretted as the index of the &quot;last&quot; DPoint3d, i.e. index &quot;count-1&quot; where count is the number of DPoint3d in the array.</P>
//!
//! <P>Advanced note: If the index is larger than the data range, the array is padded with zeros between the &quot;old&quot; and &quot;new&quot; end of array.  Hence this is an exception to the remark that &quot;set&quot; does not change the count of the array.</P>
//!
//! <P>3) To &quot;insert&quot; a DPoint3d at a specified index, and at the same time move the higher indexed points one postion ahead, use</P>
//! <P>&#9;<B>jmdlEmbeddedDPoint3dArray_insertDPoint3d</B> (pMyArray, &amp;mySinglePoint, index)</P>
//!
//! <P>&nbsp;</P>
//!
//! <h2>Retrieving data from an embedded array </h2>
//!
//! <P>The number of DPoint3d indices which contain data may be queried by</P>
//! <pre>
//! &#9;int count = <B>jmdEmbeddedDPoint3dArray_getCount </B>(pMyArray)
//! </pre>
//!
//! <P>The following functions copy data &quot;from&quot; the array &quot;to&quot; conventional C memory:</P>
//! <pre>
//! // To copy a single point from known index:
//! <B>jmdlEmbeddedDPoint3dArray_getDPoint3d</B> (pMyArray, &amp;mySinglePoint, index)
//! // To copy a contiguous block of points, starting at index:
//! <B>jmdlEmbeddedDPoint3dArray_getDPoint3dArray</B>
//!    (pMyArray, &amp;myArray, &amp;numGot, indiex, maxAllowed)
//! // Note that getDPoint3dArray has two output parameters - data is copied into the myArray
//! //      and numGot indicates how much data was copied.
//! //      This is less than maxAllowed when index+maxAllowed exceeds the array size.
//! </pre>
//!
//! <h2>Advanced access with buffer pointers</h2>
//!
//! <P>The caller program can obtain a pointer to the current &quot;real&quot; C array hidden &quot;behind&quot; the EmbeddedDPoint3dArray header by calling
//! <pre>
//!    DPoint3d *pCurrentBuffer = <B>jmdlEmbeddedDPoint3dArray_getPtr</B> (pMyHeader, index)
//! or
//!    const DPoint3d *pCurrentBuffer = <B>jmdlEmbeddedDPoint3dArray_getConstPtr</B>(pMyHeader, index)
//! </pre>
//!
//! <I><P>Warning:   Any function call which adds, sets, or inserts data to the array may cause the contiguous buffer to be reallocated.  Hence the pointer returned by a call to </I><B>jmdlEmbeddedDPoint3dArray_getPtr</B> <I>or
//! </I><B>jmdlEmbeddedDPoint3dArray_getConstPtr</B> may be invalidated.</P>
//!
//! <h2>Internal structure</h2>
//! <P>EmbeddedArrays are physically composed of two pieces:</P>
//! <UL>
//!    <LI>the fixed sized header (the piece addressed by pMyArray in the examples).</LI>
//!    <LI>a contiguous block of heap memory containing the DPoint3d data.</LI>
//! </UL>
//!
//! <P>The essential service of the EmbeddedArray implementation is to allocate and reallocate the contiguous buffer as often as is needed. </P>
//!
//! <P>Callers that never uses <B>_getPtr</B> or <B>_getConstPtr</B> functions can be largely oblivious to reallocation.   Callers that access the pointers must be careful to refresh their pointers if data is added to the array.</P>
//!
//!
// Temporary - until we can debug "b" classes.

#include <vector>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Temporary - until we can debug "b" classes.

typedef struct _EmbeddedStructArray EmbeddedStructArray;
struct _EmbeddedStructArray
    {
    int		    count;      /* The number of entries in this array.  The the number of entries that  */
                                /* are "in use" is never larger than the capacity (number allocated).    */
    
    int		    nAllocated; /* The current capacity of this array.  That is, the number of items     */
                                /* that can be accomodated without reallocation.                         */
    
    int		    itemSize;   /* The sizeof an individual structure within this array.                 */
    
    char*           pList;      /* The pointer to the buffer that holds the array of structures.         */
    };
END_BENTLEY_GEOMETRY_NAMESPACE


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifndef VBARRAY_SORTFUNCTION_DEFINED
#define VBARRAY_SORTFUNCTION_DEFINED
typedef int (*VBArray_SortFunction)
    (
    const void *,
    const void *
    );
#endif

typedef bvector<DPoint3d>   EmbeddedDPoint3dArray;
typedef bvector<int>        EmbeddedIntArray;

typedef EmbeddedDPoint3dArray * EmbeddedDPoint3dArrayP;
typedef EmbeddedIntArray      * EmbeddedIntArrayP;

typedef EmbeddedDPoint3dArray const * EmbeddedDPoint3dArrayCP;
typedef EmbeddedIntArray      const * EmbeddedIntArrayCP;

typedef EmbeddedDPoint3dArray const * EmbeddedDPoint3dArrayConstP;
typedef EmbeddedIntArray      const * EmbeddedIntArrayConstP;

typedef struct _PtrCacheHeader *PPtrCacheHeader;

#ifndef GraphicsPointArrayDefined
#define GraphicsPointArrayDefined 1
typedef struct GraphicsPointArray *GraphicsPointArrayP;
typedef struct GraphicsPointArray const *GraphicsPointArrayCP;
#endif
#ifdef __cplusplus

typedef struct GraphicsPointArray &GraphicsPointArrayR;
typedef struct GraphicsPointArray const &GraphicsPointArrayCR;
struct GraphicsPointArray
    {
    // Index management for walking the gpa.
    struct Parser
        {
        private:
        size_t  m_i0;
        size_t  m_i1;
        int     m_curveType;
        GraphicsPointArrayCP m_gpa;


        friend struct GraphicsPointArray;
        void Set (size_t i0, size_t i1, int  curveType);

        //  Init with explicit indices ...
        Parser  (GraphicsPointArrayCP gpa, size_t i0, size_t i1, int curveType);

        public:
        //  Initialize in start condtion -- NOT valid for immediate read
        GEOMDLLIMPEXP Parser  (GraphicsPointArrayCP gpa);
        //  Copy from another parser...
        GEOMDLLIMPEXP Parser  (Parser const & other);

        // Constructor equivalent...
        GEOMDLLIMPEXP void Attach  (GraphicsPointArrayCP gpa);
        // Init ala constructor
        // Reset to start condition
        void GEOMDLLIMPEXP   Reset ();

        // Setup at primitive with specific start index.
        //  Return false if not a valid index.
        bool GEOMDLLIMPEXP   MoveToPrimitive (size_t index);
        // Advance to next read position, treating each stick of a line segment
        // as a separately readable primtive.
        //  Return false if no more.
        bool GEOMDLLIMPEXP   MoveToNextPrimitive ();
        // Advance to next read position, in treating linestrings as single unit.
        //  Return false if no more.
        bool GEOMDLLIMPEXP   MoveToNextFragment ();

        // Reset for reverse parse.
        void ResetAtEnd ();
        // Advance to previous read position, treating linestrings as single unit.
        //  Return false if no more.
        bool GEOMDLLIMPEXP   MoveToPreviousFragment ();
        // Advance to previous read position, treating linestrings as multiple pieces.
        //  Return false if no more.
        bool GEOMDLLIMPEXP   MoveToPreviousPrimitive ();
        // Return index for reading
        size_t GEOMDLLIMPEXP GetReadIndex () const;
        // Return index for reading the end of the current primitive or fragment.
        size_t GEOMDLLIMPEXP GetReadIndexTail () const;

        // Return curvetype at ReadIndex.
        int   GEOMDLLIMPEXP  GetCurveType () const;
        // Test if in a valid state for reading
        bool   GEOMDLLIMPEXP IsValid () const;
        // Test if there are more steps ....
        bool GEOMDLLIMPEXP HasMore () const;
        //! Test if indices and curve types match.
        bool GEOMDLLIMPEXP SameIndicesAndCurveType (Parser const &other) const;
        };


public:
    GEOMDLLIMPEXP GraphicsPointArray ();

    bvector<GraphicsPoint> vbArray_hdr;
    int			arrayMask;
    size_t GEOMDLLIMPEXP GetGraphicsPointCount () const;
    size_t GEOMDLLIMPEXP GetLastIndex () const;
    //! Test if i is within the array range.
    bool GEOMDLLIMPEXP IsValidIndex (size_t i) const;
    bool GEOMDLLIMPEXP IsEmpty () const;
    //! Test if i is within the array range and is at the start of a primtive.
    bool GEOMDLLIMPEXP IsPrimitiveIndex (size_t i) const;
    bool GEOMDLLIMPEXP IsLastPrimitive (size_t index) const;
    bool GEOMDLLIMPEXP GetLastPrimitiveIndex (size_t &index) const;

    //! Replace negative index by final index in array; others unchnaged.
    size_t GEOMDLLIMPEXP ResolveIndex (int signedIndex) const;

    void GEOMDLLIMPEXP CopyFrom (GraphicsPointArrayCR source);
    // return the number of bsplines expanded
    int GEOMDLLIMPEXP CopyFromWithBsplinesExpandedToBeziers (GraphicsPointArrayCR source);
    void GEOMDLLIMPEXP CopyReversedFrom (GraphicsPointArrayCR source);
    void GEOMDLLIMPEXP AppendFrom (GraphicsPointArrayCR source);
    // i0,i1 are INCLUSIVE indices, possibly indicating reverse order copying.
    // This is a blind copy -- curve integrity will be lost on reversal.
    void GEOMDLLIMPEXP AppendFrom (GraphicsPointArrayCR source, size_t i0, size_t i1);
    bool GEOMDLLIMPEXP AppendPrimitiveFrom (GraphicsPointArrayCR source, size_t index);
    bool GEOMDLLIMPEXP AppendIntervalFrom (GraphicsPointArrayCR source,
                size_t index0, double fraction0, size_t index1, double fraction1);
    
    bool GEOMDLLIMPEXP GetGraphicsPoint (size_t i, GraphicsPointR gp) const;
    bool GEOMDLLIMPEXP GetDPoint4d (size_t i, DPoint4dR xyzw) const;
    // Normalize the DPoint4d -- false if zero weight.
    bool GEOMDLLIMPEXP GetNormalizedPoint (size_t i, DPoint3dR xyz) const;
    // return true iff entire buffer filled.   false may still have {numGot} points.
    bool GEOMDLLIMPEXP GetDPoint3dArray (size_t index, DPoint3dP buffer, size_t &numGot, size_t max) const;

    //! Get graphics point and confirm curve and point type parts of mask.
    bool GEOMDLLIMPEXP GetCheckedGraphicsPoint (size_t i, GraphicsPointR gp,
                    int curveType, int pointType) const;
    bool GEOMDLLIMPEXP IsCurvePointType (size_t index, int curveType, int PointType) const;
    bool GEOMDLLIMPEXP FindCurvePointTypeAfter (size_t index, int curveType, int PointType, size_t &foundIndex) const;
    bool GEOMDLLIMPEXP FindCurvePointTypeBefore (size_t index, int curveType, int PointType, size_t &foundIndex) const;
    //! Search for a major break.  Return false (with invalid large index) if not found.
    bool GEOMDLLIMPEXP FindMajorBreakAfter (size_t index, size_t &foundIndex) const;

    bool GEOMDLLIMPEXP AddAsCompleteBsplineCurve (MSBsplineCurveCR);
    bool GEOMDLLIMPEXP AddBsplineCurve (MSBsplineCurveCR curve, bool splinesAsBezier);

    bool GEOMDLLIMPEXP GetBezierSpanFromBsplineCurve
        (
        size_t bsplineBaseIndex,
        size_t bezierSelect,
        DPoint4dP poles,
        int &order,
        int maxOrder,
        bool &isNullInterval,
        double &knot0,
        double &knot1
        ) const;

    double GEOMDLLIMPEXP CurveLength () const;
    void GEOMDLLIMPEXP GetRange(DRange3dR range) const;
    int GEOMDLLIMPEXP HighestBezierOrder () const;
    bool GEOMDLLIMPEXP PrimitiveFractionToDPoint3d (size_t primitiveIndex, double fraction, DPoint3dR xyz) const;
    bool GEOMDLLIMPEXP PrimitiveFractionToDPoint3d
        (
        size_t primitiveIndex,
        double fraction,
        DPoint3dR xyz,
        DVec3dR   dXYZ
        ) const;

    bool GEOMDLLIMPEXP PrimitiveFractionToDPoint3d
        (
        size_t primitiveIndex,
        double fraction,
        DPoint3dR xyz,
        DVec3dR   dXYZ,
        DVec3dR   ddXYZ
        ) const;

    double GEOMDLLIMPEXP PrimitiveLength (size_t primitiveIndex) const;
    GEOMDLLIMPEXP bool PrimitiveFractionArrayToDPoint3dDerivatives
        (
        size_t primitiveIndex,
        double *pFractions,
        size_t numFraction,
        DPoint3dP xyz,
        DVec3dP   dXYZ,
        DVec3dP   ddXYZ
        ) const;

    GEOMDLLIMPEXP bool PrimitiveFractionArrayToDPoint4dDerivatives
        (
        size_t primitiveIndex,
        double *pFractions,
        size_t numFraction,
        DPoint4dP   xyzw,
        DPoint4dP   dXYZW,
        DPoint4dP   ddXYZW
        ) const;

    ///! Return a coordinate frame aligned with the primtive
    ///! @param [in] primtiveIndex primtiive to evaluate
    ///! @param [in] fraction fractional position on primitive
    ///! @param [out] transform Coordinate frame with origin on primitive, x vector forward tangent, y vector towards center or curvature.
    ///! @param [in] defaultZ optional vector to resolve line segment with no nearby geometry to resolve ambiguous y and z vectors.
    GEOMDLLIMPEXP bool PrimitiveFractionToFrenetFrame
        (
        size_t           primitiveIndex,
        double           fraction,
        TransformR       transform,
        DVec3dCP        defaultZ = NULL
        ) const;

    ///! Return a coordinate frame aligned with the geometry.
    ///! @param [out] transform Coordinate frame with origin at start, x vector along first primitive, z perpendicular to plane.
    ///! @param [out] localrange data range in the local coordinate system (optional)
    ///! @param [out] maxDeviation max deviation from planarity
    ///! @param [in] defaultZ optional vector to resolve line segment with no nearby geometry to resolve ambiguous y and z vectors.
    GEOMDLLIMPEXP bool GetPlane
        (
        TransformR   transform,
        DRange3dP    localRange = NULL,
        double       *maxDeviation = NULL,
        DVec3dCP     defaultZ = NULL
        ) const;

    ///! Compute the centroid of the curves as a path (thin wire) -- not as an area.
    ///! @param [out] centroid computed centroid.
    ///! @param [out] arcLength computed arc lenght
    ///! @param [in] computeAtFixedZ true to enable xy-plane calculations.
    ///! @param [in] fixedZ z value for computeAtFixedZ
    GEOMDLLIMPEXP bool GetPathCentroid
        (
        DPoint3dR centroid,
        double &arcLength,
        bool   computeAtFixedZ = false,
        double fixedZ = 0.0
        ) const;


    //! Search the knot sequence for the interval containing knotValue.
    //! @param [in] readIndex start of bspline
    //! @param [in] knotValue target knot value.
    //! @param [in] chooseIntervalToRightIfExactHit true if favor left, false to favor right when
    //!            knotValue exactly matches a knot.
    //! @param [out] bezierSelect selector index for use in GetBezierSpaneFromBsplineCurve.
    //! @param [out] bezierFraction fractional position of the knot within this interval.
    //! @remark If knotValue is out of the domain, the nearest end inteval is used. (bezierFraction
    //              will be less than 0 at left, greater than 1 at right)
    //! @return false if not a Bspline
    bool GEOMDLLIMPEXP SearchBsplineInterval (size_t readIndex, double knotValue,
                    bool chooseIntervalToRightIfExactHit,
                    size_t &bezierSelect, double &bezierFraction) const;
    bool GEOMDLLIMPEXP IsBsplineCurve (size_t readIndex) const;
    bool GEOMDLLIMPEXP IsBezierCurve (size_t readIndex, size_t &tailIndex) const;
    bool GEOMDLLIMPEXP IsBsplineCurve (size_t readIndex, size_t &i1, bool checkAll = false) const;
    bool GEOMDLLIMPEXP IsConic (size_t readIndex, size_t &i1) const;
    bool GEOMDLLIMPEXP IsLineSegment (size_t readIndex, size_t &i1) const;
    bool GEOMDLLIMPEXP IsLineString (size_t readIndex, size_t &i1) const;
    bool GEOMDLLIMPEXP ParseBsplineCurveKnotDomain (size_t readIndex,
                    size_t &poleIndex0, size_t &poleIndex1, double &knot0, double &knot1) const;
    bool GEOMDLLIMPEXP ParseBsplineCurveKnotDomain (size_t readIndex,
                    size_t &poleIndex0, size_t &poleIndex1, double &knot0, double &knot1,
                    int &order, size_t &numBezier) const;
    bool GEOMDLLIMPEXP ParseBsplineCurvePoleIndices (size_t readIndex,
                    size_t &i0, size_t &i1) const;
    bool GEOMDLLIMPEXP IsBsplineCurveTail (size_t readIndex, size_t &i0, bool checkAll = false) const;
    bool GEOMDLLIMPEXP GetBsplineCurve (size_t readIndex, MSBsplineCurve &curve) const;

    // Incremental moves.
    // Forward move from end of array forces nextReadIndex to end of array.
    bool GEOMDLLIMPEXP GetNextPrimitiveReadIndex (size_t readIndex, size_t &nextReadIndex) const;
    // Reverse move failure mode leaves previousReadIndex at readIndex.  (size_t can't be negative.  ugh)
    bool GEOMDLLIMPEXP GetPreviousPrimitiveReadIndex (size_t readIndex, size_t &previousReadIndex) const;

    //! Connectivity test at all gaps (both interior and end-to-start).
    bool GEOMDLLIMPEXP IsConnectedAndClosed (double absTol, double relTol) const;
    //! empty the array
    void GEOMDLLIMPEXP Clear ();

    //! Mark the final point as "end of linestring".
    void GEOMDLLIMPEXP MarkBreak ();
    //! Markt the final point as "end of loop"
    void GEOMDLLIMPEXP MarkMajorBreak ();


    //! Test if a point is a linestring break.
    bool GEOMDLLIMPEXP IsBreak (size_t i) const;
    //! Test if a point is a loop break.
    bool GEOMDLLIMPEXP IsMajorBreak (size_t i) const;


    //! Add a fully constructed graphics points.
    void GEOMDLLIMPEXP Add (GraphicsPointCR gp);
    //! Insert a fully constructed graphics points.
    void GEOMDLLIMPEXP Insert (size_t index, GraphicsPointCR gp);

    //! Add a simple point, optionally marking break.
    void GEOMDLLIMPEXP Add (DPoint3dCR point, bool markBreak = false);
    //! Add a line segment, optionally marking break;
    void GEOMDLLIMPEXP Add (DSegment3dCR segment, bool markBreak = false);
    //! Add an ellipse.
    void GEOMDLLIMPEXP Add (DEllipse3dCR ellipse);

    //! Add a single point with mask.
    void GEOMDLLIMPEXP AddMasked (DPoint4dCR xyz, int mask, bool markBreak = false);
    //! Add a single point with mask.
    void GEOMDLLIMPEXP AddMasked (DPoint3dCR xyz, int mask, bool markBreak = false);
    //! Add an array of points, optionally marking final point as break.
    void GEOMDLLIMPEXP AddArray (DPoint3dCP points, size_t numPoints, bool markBreak = false);
    //! Add an array of points, optionally marking final point as break.
    void GEOMDLLIMPEXP AddArray (DPoint2dCP points, size_t numPoints, bool markBreak = false);

    //! Add an array of points, optionally marking final point as break.
    void GEOMDLLIMPEXP Add (bvector<DPoint3d> &points, bool markBreak);
    //! Add a bezier curve
    void GEOMDLLIMPEXP AddBezier (bvector<DPoint3d> &points);
    //! Add a (single) bezier curve (curve order equal {numPoints})
    void GEOMDLLIMPEXP AddBezier (DPoint3dCP points, size_t numPoints);
    //! Add a (single) bezier curve (curve order equal {numPoints})
    void GEOMDLLIMPEXP AddBezier (DPoint4dCP points, size_t numPoints);
    
    //! Multiply all xyzw data
    void GEOMDLLIMPEXP Multiply (DMatrix4dCR transform);
    //! Multiply all xyzw data
    void GEOMDLLIMPEXP Multiply (TransformCR transform);

    //! Read specific types.
    bool GEOMDLLIMPEXP GetDSegment3d (size_t index, size_t &nextReadIndex, DSegment3dR segment) const;
    bool GEOMDLLIMPEXP GetDSegment4d (size_t index, size_t &nextReadIndex, DSegment4dR segment) const;

    bool GEOMDLLIMPEXP GetDEllipse3d (size_t index, size_t &nextReadIndex, DEllipse3dR ellipse) const;
    bool GEOMDLLIMPEXP GetDConic4d   (size_t index, size_t &nextReadIndex, DConic4dR conic) const;
    bool GEOMDLLIMPEXP GetDConic4d   (size_t index, size_t &nextReadIndex, DConic4dR conic, DEllipse3dR ellipse, bool &isSimpleEllipse) const;
    //! Extract bezier curve segment.
    //! (Use IsBezier to get tailIndex)
    //! @param [in] readIndex read index.
    //! @param [in] maxOrder max allowed poles
    //! @param [out] nextReadIndex
    //! @param [out] poles buffer for poles
    //! @param [out] order number of poles
    //! @param [out] a0 {a] field at first pole.
    //! @param [out] a1 {a} field at last pole.
    bool GEOMDLLIMPEXP GetBezier (size_t readIndex, int maxOrder, size_t &nextReadIndex, DPoint4dP poles, int &order, double &a0, double &a1) const;
    bool GEOMDLLIMPEXP GetBezier (size_t readIndex, size_t &nextReadIndex, bvector<DPoint4d> &poles, double &a0, double &a1) const;
    bool GEOMDLLIMPEXP AdvancePrimitveIndex (size_t &index) const;
    void GEOMDLLIMPEXP SetArrayMask (int mask);
    int  GEOMDLLIMPEXP GetArrayMask (int mask) const;

    bool GEOMDLLIMPEXP SetPointMask (size_t index, int mask);
    bool GEOMDLLIMPEXP GetPointMask (size_t index, int mask, int &returnedMask) const;

    bool GEOMDLLIMPEXP ClosestVertex (DPoint3dCR spacePoint, bool xyOnly,
                GraphicsPointR gp, size_t &index) const;


    //! @param [out] perpendicularPointData Data for closest true perpendicular.
    //! @param [out] endPointData Optional endpoint test data.
    //! @param [in] spacePoint base point of distance measurement.
    //! @param [in] xyOnly true to do distance calculations with only xy.
    //! @param [in] extend true to extend lines, arcs.
    bool GEOMDLLIMPEXP ClosestPointOnCurve (
                ProximityDataR          perpendicularPointData,
                ProximityDataP          endPointData,
                DPoint3dCR              point,
                bool                    xyOnly,
                bool                    extend
                );

    //! @description
    //!    Compute points at specified distances along a path.
    //!    Add points to the instance.
    //! @param [in] path subject curve
    //! @param [in] distanceArray array of distances along curve.
    //!             (If start point of path is to be present in the output, distanceArray[0] must be zero.)
    //! @param [in] storeTangent true to have each computed point followed immediately by its tangent vector.
    //!
    void GEOMDLLIMPEXP AddSpacedPoints
        (
        GraphicsPointArrayCR path,
        bvector<double>&istanceArray,
        bool        storeTangent
        );

//! @description
//! @param [out] pointsOnA intersection points on curves in curveA
//! @param [out] pointsOnB intersection points on curves in curveB
//! @param [in] curveA first curve set
//! @param [in] curveB second curve set
//! @param [in] extendLines true to extend lines
//! @param [in] extendConics true to extend conics
    static void GEOMDLLIMPEXP XYIntersections
        (
        GraphicsPointArrayR pointsOnA,
        GraphicsPointArrayR pointsOnB,
        GraphicsPointArrayCR curveA,
        GraphicsPointArrayCR curveB,
        bool extendLines = false,
        bool extendConics = false
        );

//! @description Find points where a discontinuity point (end point or interior sharp angle) of curveA are within tolerance of curveB.
//! @param [out] pointsOnA intersection points on curves in curveA
//! @param [out] pointsOnB intersection points on curves in curveB
//! @param [in] curveA curves - discontinuities
//! @param [in] curveB second curve set
//! @param [in] extendLines true to extend lines
//! @param [in] extendConics true to extend conics
    static void GEOMDLLIMPEXP AddNearContactAtDiscontinuities
        (
        GraphicsPointArrayR pointsOnA,
        GraphicsPointArrayR pointsOnB,
        GraphicsPointArrayCR curveA,
        GraphicsPointArrayCR curveB,
        double tolerance
        );

//! @description Test if two arrays are have identical curve type and point coordinates.
//! @param [in] other second array of comparison.
//! @param [in] xyzAbsTol absolute tolerance for comparisons.
//! @param [in] relTol relative tolerance.
GEOMDLLIMPEXP bool  IsSameGeometryPointByPoint
(
GraphicsPointArrayCR other,
double xyzAbsTol = 0.0,
double relTol = 0.0
) const;

//! "Stroke" curves from the {source} into {this} array.
//! @param [in] chordTolerance target deviation between curve and strokes. (0.0 means ignore)
//! @param [in] angleTolerance target turning angle between strokes. (0.0 means ignore)
//! @param [in] maxEdgelength target stroke length (0.0 means ignore)
//! @param [in] movePolesToCurves true to have all output points exactly on curves.
//!                 (false allows bspline and beziers to be represented by control polygons
//!                 that are very close but not exactly on)
//! @param xyOnly [in] measure deviations in xy plane only.
GEOMDLLIMPEXP void  AddStrokes
(
GraphicsPointArrayCR source,
double chordTolerance = 0.0,
double angleTolerance = 0.25,
double maxEdgeLength = 0.0,
bool   movePolesToCurves = true,
bool   xyOnly = false
);

//! @description Compute a point at specified distance from start, measuring along the true curve.
//! @param [in] target distance distance along curve
//! @param [out] primitiveIndex returned curve primitive index
//! @param [out] fraction fractional parameter along the primitive
//! @param [out] point point on curve.
GEOMDLLIMPEXP bool GetPointAtDistanceFromStart
(
double distance,
size_t &primitiveIndex,
double &fraction,
DPoint3dR point
) const;


//! @description Generate an offset of the lines and curves.
//! @param [in] arcAngle => If the turning angle at a vertex exceeds this angle (radians),
//!                       an arc is created.   Passing a negative angle                   means no arcs.
//! @param [in] chamferAngle => If the turning angle at a vertex is smaller than
//!                       arc angle but larger than chamfer angle, a chamfer is
//!                       created.   This angle is always forced to .99pi or less.
//! @param [in] offsetDistance the offset distance (uniform).  Positive is to
//!                           left of curve relative to the normal.
//! @param [in] normal optional normal to plane of offset measurement.  Default is Z vector.
GEOMDLLIMPEXP void     AddOffsetCurves
(
GraphicsPointArrayCR source,
double offsetDistance,
DVec3dCP planeNormal,
double arcAngle = -1.0,
double chamferAngle = -1.0
);

//! @description compute simple (point) intersections with a plane.
//! @param [in] source candidate intersection geometry.
//! @param [in] plane 
//! @param [in] extend true to extend lines and arcs
GEOMDLLIMPEXP void AddPlaneIntersectionPoints
(
GraphicsPointArrayCR    source,
DPlane3dCR              plane,
bool                    extend = false
);

//! @description compute line segments of intersections with a plane, using parity rules to determine in and out.
//!    Intersection edges are added to the instance array in start-end pairs.
//! @param [in] boundary boundary geometry
//! @param [in] plane 
GEOMDLLIMPEXP void AddPlaneIntersectionEdges
(
GraphicsPointArrayCR    boundary,
DPlane3dCR              plane
);

//! @description Add all content of curveArray to this GraphicsPointArray.
GEOMDLLIMPEXP BentleyStatus AddCurves (CurveVectorCR curveArray, bool splinesAsBezier = false);

//! @description Create CurveVector form of the curves.
GEOMDLLIMPEXP CurveVectorPtr CreateCurveVector () const;

#ifdef abc

//! @description Apply dash lengths to curves in the source.
//! @param [in] source curves to split
//! @param [in] dashLengths array of dash lengths.  Negative length indicates a skip.
//! @param [in] numLength
GEOMDLLIMPEXP void ExpandDashPattern
(
GraphicsPointArrayCR source,
double *pDashLength,
int    numDashLength
);

//! @description Compute cross hatch lines within boundaries.
//! @param [in] boundary boundary geometry
//! @param [in] worldToLocal Transform from world to hatch coordinates.
//!       After transform, every plane parallel to xy at an integer z generates hatch.
//!     (i.e. The inverse transform for a coordinate frame whith X in the hatch direction, vector from 000 to 001 stepping from one hatch line
//!     to the next)
//! @param [in] clipRangePoints optional set of (any number of) (world space) points whose range (after transform by the hatch transform)
//!             is a clip box.
//! @param [in] clipPlanes Optional set of (any number of) (world space) DPoint4d plane coefficients.  The (intersection of) negative
//!             halfspaces is "in".
//! @param [in] selector for crossing analysis
//! <ul><li>0 -- simple parity
//! <li>1 -- only first and last crossings matter (i.e. fill holes)
//! <li>2 -- If 4 or more crossings, only use first and last pairs
//! </ul>
GEOMDLLIMPEXP void AddHatch
(
GraphicsPointArrayCR boundary,
TransformCR worldToLocal,
GraphicsPointArrayCP clipRangePoints = NULL,
GraphicsPointArrayCp clipPlanes = NULL,
int selector
);


#endif
}; // struct GraphicsPointArray
#endif



END_BENTLEY_GEOMETRY_NAMESPACE

#ifdef __cplusplus
#include "memory/capi/embeddeddpoint3darray_capi.h"
#include "memory/capi/embeddedintarray_capi.h"
#endif



