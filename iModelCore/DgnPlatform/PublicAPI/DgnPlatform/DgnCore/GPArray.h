/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/GPArray.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Geom/GeomApi.h>

BEGIN_BENTLEY_NAMESPACE

//__PUBLISH_SECTION_END__
typedef struct GPArrayParam*             GPArrayParamP;
typedef struct GPArrayParam const*       GPArrayParamCP;
typedef struct GPArrayParam&             GPArrayParamR;
typedef struct GPArrayParam const&       GPArrayParamCR;
typedef struct GPArrayInterval*          GPArrayIntervalP;
typedef struct GPArrayInterval const*    GPArrayIntervalCP;
typedef struct GPArrayInterval&          GPArrayIntervalR;
typedef struct GPArrayInterval const&    GPArrayIntervalCR;


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      08/2008
+===============+===============+===============+===============+===============+======*/
struct GPArrayParam
{
    double          m_param;
    uint32_t        m_index;

    GPArrayParam () : m_index (0), m_param (0.0) { }
    GPArrayParam    (int index, double param) { m_index = index, m_param = param; }
DGNPLATFORM_EXPORT    GPArrayParam    (double value);
    double  GetValue () const               { return (double) m_index + m_param; }
    void Init (int index, double param) { m_index = index, m_param = param; }
    bool operator < (GPArrayParamCR rhs) const  { return m_index == rhs.m_index ? (m_param < rhs.m_param) : (m_index < rhs.m_index); }
    bool operator > (GPArrayParamCR rhs) const  { return m_index == rhs.m_index ? (m_param > rhs.m_param) : (m_index > rhs.m_index); }
    bool operator == (GPArrayParamCR rhs) const { return m_index == rhs.m_index && m_param == rhs.m_param; }


};  // GPArrayParam



//__PUBLISH_SECTION_START__
enum class GPCurveType
{
    Invalid     = 0,
    LineString  = 1,
    Ellipse     = 2,
    Bezier      = 3,
    BSpline     = 4
};

/*=================================================================================**//**
  Class for collecting and manipulating curve data. Curve geometry is represented as
  an array of homogeneous points along with label data that denotes the underlying 
  curve type, GPCurveType. The GPArray can represent the geometry for open and closed 
  curves as well as regions having an outer solid loop with one or more hole loops.
  @see GPArraySmartP
  @remarks The GPArray is held by a GPArraySmartP, you do not create instances of GPArray.
  @note A simple loop to parse the curve information held by a GPArray:
  
  \code
BentleyStatus   status = SUCCESS;

for (int i=0, count = gpa->GetCount(); i < count && SUCCESS == status; )
    {
    switch (gpa->GetCurveType (i))
        {
        case GPCurveType::LineString:
            {
            bvector<DPoint3d> points;

            if (SUCCESS != (status = gpa->GetLineString (&i, points)))
                break;

            // do something...
            break;
            }

        case GPCurveType::Ellipse:
            {
            DEllipse3d  ellipse;

            if (SUCCESS != (status = gpa->GetEllipse (&i, &ellipse)))
                break;

            // do something...
            break;
            }

        case GPCurveType::Bezier:
        case GPCurveType::BSpline:
            {
            MSBsplineCurve  curve;

            if (SUCCESS != (status = gpa->GetBCurve (&i, &curve)))
                break;

            // do something...

            curve.ReleaseMem ();
            break;
            }

        default:
            {
            i++;
            break;
            }
        }

    if (i && gpa->IsMajorBreak (i-1))
        // do something...potential start of new loop...
    }
  \endcode
  @bsiclass                                                     KeithBentley    08/02
+===============+===============+===============+===============+===============+======*/
struct  GPArray 
//__PUBLISH_SECTION_END__
: public GraphicsPointArray
//__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__

protected:
    GPArray() {}

public:
    enum    Masks
    {
        MASK_ALL                = -1,
        MASK_DEFAULT            = 0x00000000,
        MASK_FILL               = 0x00000001,
        MASK_CURVES             = 0x00000002,
        MASK_HAS_MAJOR_BREAKS   = 0x00000004,
        MASK_STROKED_DATA       = 0x00000008,
    };


    DGNPLATFORM_EXPORT GPArray (BentleyAllocator <GraphicsPoint> allocator);

    DGNPLATFORM_EXPORT bool Equals (GPArray const&) const;

    DGNPLATFORM_EXPORT void Add (GraphicsPoint const* pt);
    DGNPLATFORM_EXPORT void Add (double x, double y, double z, double w, double a, int mask, int userData);
    DGNPLATFORM_EXPORT BentleyStatus ToBCurve (MSBsplineCurveP curve) const;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DGNPLATFORM_EXPORT void Draw (Render::DrawGeomR drawGeom, bool closed, bool filled) const;
#endif

    DGNPLATFORM_EXPORT bool GetDPoint3dArray (DPoint3dP point, int* nGot, int i0, int nreq) const;
    DGNPLATFORM_EXPORT BentleyStatus GetEllipse (int* index, DEllipse3dP elllipse, bool flatten) const;
    DGNPLATFORM_EXPORT BentleyStatus GetConicOrEllipse (int* index, DConic4dP conic, DEllipse3dP elllipse, bool &isEllipse, bool flatten) const;
    DGNPLATFORM_EXPORT BentleyStatus GetAsBCurve (int* index, MSBsplineCurve* curve) const;

    DGNPLATFORM_EXPORT bool IsSingleDEllipse3d (DEllipse3dP ellipse) const;
    DGNPLATFORM_EXPORT bool IsSingleDSegment3d (DSegment3dP segment) const;
    DGNPLATFORM_EXPORT bool IsSingleCurvePath () const; // single linestring, arc or bspline...

    DGNPLATFORM_EXPORT GraphicsPointCP GetConstPtr (int index) const;
    DGNPLATFORM_EXPORT GraphicsPointP GetPtr (int index);
    DGNPLATFORM_EXPORT bool Get (GraphicsPointR, int index) const;
    DGNPLATFORM_EXPORT bool GetNormalized (DPoint3dR xyz, int index) const;
    DGNPLATFORM_EXPORT bool GetPrimitiveFractionPoint (DPoint3dP point, int index, double fraction) const;
    DGNPLATFORM_EXPORT bool GetPrimitiveFractionPointAndTangent (DPoint3dR point, DVec3dR tangent, int index, double fraction) const;
    DGNPLATFORM_EXPORT bool CopyPortionOf (GPArrayCP, GPArrayParamR start, GPArrayParamR end);
                   bool CopyPortionOf (GPArrayCP g, GPArrayParamCR s, GPArrayParamR e) {return CopyPortionOf(g, const_cast<GPArrayParamR>(s), e);}
    DGNPLATFORM_EXPORT int  CountToEndOfLinestring (int start) const;
    DGNPLATFORM_EXPORT bool ParseAt (int* i0, int* i1, DPoint4d* point0, DPoint4d *point1, GPCurveType* curveType, int prev) const;
    DGNPLATFORM_EXPORT bool ParseBefore (int* i0, int* i1, DPoint4d* point0, DPoint4d *point1, GPCurveType* curveType, int prev) const;
    DGNPLATFORM_EXPORT bool ParseAfter (int* i0, int* i1, DPoint4d* point0, DPoint4d *point1, GPCurveType* curveType, int prev) const;
    DGNPLATFORM_EXPORT void SetArrayMask (int mask);
    DGNPLATFORM_EXPORT int  GetArrayMask (int mask) const;
    DGNPLATFORM_EXPORT GPArrayParam GetEndParam () const; 

    DGNPLATFORM_EXPORT double ApproximateLength () const;
    DGNPLATFORM_EXPORT bool ContainsCurves () const;
    DGNPLATFORM_EXPORT void Dump (WCharCP label) const;
    DGNPLATFORM_EXPORT BentleyStatus GetTranslationTo (DVec3dR translation, GPArrayCR otherArray, double tolerance) const;
    DGNPLATFORM_EXPORT static GPCurveType SimplifyCurveType (int detailType);
    DGNPLATFORM_EXPORT bool GetClosestPoint (DPoint3dP closePoint, GPArrayParamP closeParam, DPoint3dCR testPoint, bool xyOnly = true) const;
    DGNPLATFORM_EXPORT void GetPlaneIntersections (GPArrayP intersections, DPlane3dCR plane, bool extend = false) const;
    DGNPLATFORM_EXPORT void GetXYIntersections (GPArrayP thisIntersects, GPArrayP otherIntersects, GPArrayCR otherGPA) const;
    DGNPLATFORM_EXPORT void GetClosestApproachPoints (GPArrayR thisPoints, GPArrayR otherPoints, GPArrayCR otherCurve, double maxDistance, bool xyOnly = true, bool extend = false, double duplicatePointTol = 0.0) const;
    DGNPLATFORM_EXPORT void SortByA ();
    DGNPLATFORM_EXPORT static BentleyStatus EnforceProfileContinuity (GPArrayP *pProfile, int numProfile, double closureTol, bool bBreakAtIntermediatePhysicalClosure, bool bForceFinalClosure);
    // Return a tolerance based on size of data in two arrays.
    DGNPLATFORM_EXPORT static double GetTolerance (GPArrayCP gpa1, GPArrayCP gpa2, double absTol = 1.0e-12, double relTol = 1.0e-10);


    //! Extract a bspline curve from the GPArray.
    //! @param[in]      index       index of first point in curve, on return will be set to first point after the curve data.
    //! @param[out]     curve       extracted bspline curve data.
    //! @remarks Caller needs to free curve by calling ReleaseMem.
    //! @return SUCCESS if returned information is valid.
    DGNPLATFORM_EXPORT BentleyStatus GetBCurveFromBeziers (int *index, MSBsplineCurveP curve) const;


//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT static GPArrayP Grab ();
    DGNPLATFORM_EXPORT void Drop ();

    //! Get the number of points in the array.
    //! @note This is not the number of curves, it's the total number of points for all curves.
    //! @return Graphics point array count.
    DGNPLATFORM_EXPORT int GetCount () const;

    //! Remove all points in the array.
    DGNPLATFORM_EXPORT void Empty ();
    DGNPLATFORM_EXPORT void EmptyAll ();

    //! Compute the total length of the curves in the array.
    //! @return Total curve length.
    DGNPLATFORM_EXPORT double Length () const;

    //! Initialize a range from all the point in the array.
    //! @param[out]     range       Where to store the GPArray range.
    //! @return A pointer to GPArray range.
    DGNPLATFORM_EXPORT DRange3dP GetRange (DRange3dR range) const;

    //! Get a plane which contains the geometry in the array.
    //! @param[out]     plane       Where to store the GPArray plane.
    //! @return true if geometry is planar. If geometry is non-planar the plane is arbitrary but may be close.
    DGNPLATFORM_EXPORT bool GetPlane (DPlane3dR plane) const;

    //! Compute an in/out classification of a single point using only xy coordinates.
    //! @param[in] point point to test.
    //! @return true if point is on or inside the region defined by the gpa (by parity rules).
    DGNPLATFORM_EXPORT bool IsPointInsideXY (DPoint3dR point) const;

    //! Replace the (possibly curved) contents of the array by strokes with weight 1.
    //! @param[in]      tolerance   Stroke tolerance, to be applied only to the xy values.
    DGNPLATFORM_EXPORT void Stroke (double tolerance);

    //! Transform the contents of the GPArray by the supplied transform.
    //! @param[in]      transform   To apply to GPArray contents.
    DGNPLATFORM_EXPORT void Transform (TransformCP transform);

    //! Transform the contents of the GPArray by the supplied homogeneous transform.
    //! @param[in]      transform   To apply to GPArray contents.
    DGNPLATFORM_EXPORT void Transform (DMatrix4dCP transform);

    //! Copies the source GPArray contents and replace the contents of this instance.
    //! @param[in]      source      GPArray to copy.
    //! @return true if copy was successful.
    DGNPLATFORM_EXPORT bool CopyContentsOf (GPArrayCP source);

    //! Appends the source GPArray contents to the contents of this instance.
    //! @param[in]      source      GPArray to append.
    //! @return true if append was successful.
    DGNPLATFORM_EXPORT bool Append (GPArrayCP source);

    //! Add a single point to the GPArray.
    //! @param[in] pt point to add.
    DGNPLATFORM_EXPORT void Add (DPoint3dCP pt);

    //! Add an array of points to the GPArray.
    //! @param[in]      pt          Points to add.
    //! @param[in]      num         Number of points.
    DGNPLATFORM_EXPORT void Add (DPoint3dCP pt, int num);

    //! Add an array of 2d points to the GPArray, z value set to 0.0.
    //! @param[in]      pt          Points to add.
    //! @param[in]      num         Number of points.
    DGNPLATFORM_EXPORT void Add (DPoint2dCP pt, int num);

    //! Add an arc or ellipse to the GPArray.
    //! @param[in]      ellipse     Ellipse to add
    DGNPLATFORM_EXPORT void Add (DEllipse3dCR ellipse);

    //! Add a bspline curve to the GPArray.
    //! @param[in]      curve       Curve to add
    DGNPLATFORM_EXPORT void Add (MSBsplineCurveCR curve);

    //! Denote a break between disconnected line segments.
    DGNPLATFORM_EXPORT void MarkBreak ();

    //! Denote the break between multiple curve loops as in the solid and hole loops of a region.
    DGNPLATFORM_EXPORT void MarkMajorBreak ();

    //! Add all content of curve vector to this GPArray.
    //! @param[in]      curves          Curve vector to add
    //! @param[in]      splinesAsBezier add curve primitives that are MSBSplineCurve as beziers.
    DGNPLATFORM_EXPORT BentleyStatus Add (CurveVectorCR curves, bool splinesAsBezier = false);

    //! Extract a single point from the GPArray.
    //! @param[out]     point       Where to store point
    //! @param[in]      index       Index of point to return.
    //! @return A pointer to the returned point.
    DGNPLATFORM_EXPORT DPoint3dP GetPoint (DPoint3dR point, int index) const;

    //! Extract an array of points from the GPArray.
    //! @param[in]      index       Index of first point in linestring, on return will be set to first point after the linestring data.
    //! @param[out]     points      Extracted points.
    //! @return SUCCESS if returned information is valid.
    DGNPLATFORM_EXPORT BentleyStatus GetLineString (int *index, bvector<DPoint3d>& points) const;

    //! Extract an arc or ellipse from the GPArray.
    //! @param[in]      index       Index of first point in ellipse, on return will be set to first point after the ellipse data.
    //! @param[out]     ellipse     Extracted arc or ellipse data.
    //! @return SUCCESS if returned information is valid.
    DGNPLATFORM_EXPORT BentleyStatus GetEllipse (int* index, DEllipse3dP ellipse) const;

    //! Extract a bspline curve from the GPArray.
    //! @param[in]      index       Index of first point in curve, on return will be set to first point after the curve data.
    //! @param[out]     curve       Extracted bspline curve data.
    //! @remarks Caller needs to free curve by calling ReleaseMem.
    //! @return SUCCESS if returned information is valid.
    DGNPLATFORM_EXPORT BentleyStatus GetBCurve (int *index, MSBsplineCurveP curve) const;

    //! Extract a single curve from the GPArray and return as a ICurvePrimitive.
    //! @param[in] index Index of first point in curve, on return will be set to first point after the curve data.
    ICurvePrimitivePtr GetCurvePrimitive (int& index) const;

    //! Query the gpa for the curve type starting as the supplied index.
    //! @param[in]      index       Index of point that is the start of a curve segment.
    //! @return The type of curve segment.
    DGNPLATFORM_EXPORT GPCurveType GetCurveType (int index) const;

    //! Query if the supplied index represents a break between curve loops in a region.
    //! @param[in]      index              Index of point to test.
    //! @return true if point at index is a major break.
    DGNPLATFORM_EXPORT bool IsMajorBreak (int index) const;

    //! Create CurveVector form of the curves.
    DGNPLATFORM_EXPORT CurveVectorPtr ToCurveVector () const;

}; // GPArray

typedef bvector<GPArrayP>      GPArrayVector;
typedef GPArrayVector&          GPArrayVectorR;

//=======================================================================================
//! Class that manages the lifecycle of a GPArray.
// @bsiclass 
//=======================================================================================
class GPArraySmartP : NonCopyableClass
{
private:
    GPArrayP m_gpa;

public:
    explicit GPArraySmartP (GPArrayP gpa) {m_gpa = gpa;}
    GPArraySmartP () {m_gpa = GPArray::Grab();}
    ~GPArraySmartP () {if (NULL != m_gpa) m_gpa->Drop();}
    GPArrayP operator -> () const {return m_gpa;}
    operator GPArrayP () const {return m_gpa;}
    operator GPArray& () const {return *m_gpa;}
    void ExtractFrom (GPArraySmartP& donor) {if (NULL != m_gpa) { m_gpa->Drop (); } m_gpa = donor.m_gpa; donor.m_gpa = NULL;}
};

//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/20011
+===============+===============+===============+===============+===============+======*/
struct GPArrayInterval 
{
    GPArrayParam        m_start;
    GPArrayParam        m_end;

                                GPArrayInterval (GPArrayCR gpa) : m_end (gpa.GetEndParam()) { }
DGNPLATFORM_EXPORT                  GPArrayInterval (GPArrayParamCR start, GPArrayParamCR end, GPArrayCP gpa);
DGNPLATFORM_EXPORT static bool      Compare (GPArrayIntervalCR interval0, GPArrayIntervalCR interval1);

    size_t                      GetSegmentCount() { return 1 +  m_end.m_index - m_start.m_index; }
    bool                        operator == (GPArrayIntervalCR rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }

DGNPLATFORM_EXPORT static GPArrayInterval  InterpolateSubInterval (GPArrayIntervalCR interval, GPArrayIntervalCR subCurveInterval, GPArrayCR subCurve);

};  

struct GPArrayIntervals: bvector<GPArrayInterval> { };

typedef struct GPArrayIntervals*             GPArrayIntervalsP;
typedef struct GPArrayIntervals const*       GPArrayIntervalsCP;
typedef struct GPArrayIntervals&             GPArrayIntervalsR;
typedef struct GPArrayIntervals const&       GPArrayIntervalsCR;

typedef std::pair <int, double>     T_SegmentLengthPair;

/*=================================================================================**//**
* @bsiclass           
+===============+===============+===============+===============+===============+======*/
struct GPArraySegmentLengths :  std::map<int, double>
{
private:
                            GPArraySegmentLengths ();  // hidden.
    
public:
    DGNPLATFORM_EXPORT           GPArraySegmentLengths (GPArrayCR gpa);
    DGNPLATFORM_EXPORT double    LengthFromParam (GPArrayParamCR param) const;
                  double     IntervalLength (GPArrayIntervalCR interval) const { return LengthFromParam (interval.m_end) - LengthFromParam (interval.m_start); }
                 

};  //  GPArraySegmentLengths

//__PUBLISH_SECTION_START__

END_BENTLEY_NAMESPACE
