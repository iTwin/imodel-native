/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentPairEditor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "AlignmentPair.h"
#include "GeometryHelper.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
// AlignmentPI
// Data holder for different PI types stored using a discrimated union
//=======================================================================================
struct AlignmentPI
{
public:
    enum Orientation { ORIENTATION_Unknown, ORIENTATION_CW, ORIENTATION_CCW };

    //=======================================================================================
    // @bsiclass
    // PI definition of an Arc
    //=======================================================================================
    struct Arc
    {
    DPoint3d startPoint;
    DPoint3d endPoint;
    DPoint3d piPoint;
    DPoint3d centerPoint;
    double radius;
    Orientation orientation;
    };
    //=======================================================================================
    // @bsiclass
    // PI definition of a Spiral
    //=======================================================================================
    struct Spiral
    {
    DPoint3d startPoint;
    DPoint3d endPoint;
    DPoint3d piPoint;
    double startRadius;
    double endRadius;
    double length;

    DVec3d startVector;
    DVec3d endVector;
    };

public:
    //=======================================================================================
    //=======================================================================================
    struct NoCurveInfo
    {
    DPoint3d piPoint;
    };
    //=======================================================================================
    //=======================================================================================
    struct ArcInfo
    {
    Arc arc;
    };
    //=======================================================================================
    //=======================================================================================
    struct SCSInfo
    {
    DPoint3d overallPI;
    Spiral spiral1;
    Arc arc;
    Spiral spiral2;
    };
    //=======================================================================================
    //=======================================================================================
    struct SSInfo
    {
    DPoint3d overallPI;
    Spiral spiral1;
    Spiral spiral2;
    };
    DEFINE_POINTER_SUFFIX_TYPEDEFS(NoCurveInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ArcInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(SCSInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(SSInfo)

    // Possible PI types
    enum Type { TYPE_Uninitialized, TYPE_NoCurve, TYPE_Arc, TYPE_SCS, TYPE_SS };

private:
    // The variable holding the current type
    Type m_type;

    // The union storing the data of this PI
    union
        {
        NoCurveInfo m_noCurveInfo;          //< If not a curve (simple PI), holds the NoCurve information
        ArcInfo m_arcInfo;                  //< If an Arc, holds the arc information
        SCSInfo m_scsInfo;                  //< If this PI is a Spiral-Curve-Spiral, holds the SCS information
        SSInfo m_ssInfo;                    //< If this PI is a Spiral-Spiral, holds the SS information
        };

public:
    ROADRAILALIGNMENT_EXPORT AlignmentPI();
    ROADRAILALIGNMENT_EXPORT void InitNoCurve(DPoint3dCR piPoint);
    ROADRAILALIGNMENT_EXPORT void InitArc(DPoint3dCR piPoint, double radius);
    ROADRAILALIGNMENT_EXPORT void InitSCS(DPoint3dCR overallPI, double arcRadius, double spiralLength1, double spiralLength2);
    ROADRAILALIGNMENT_EXPORT void InitSS(DPoint3dCR overallPI, double spiralLength);
    void InitInvalid(Type piType); //! @private

    // Returns whether this PI is initialized
    bool IsInitialized() const { return TYPE_Uninitialized != m_type; }
    // Returns the PI type
    AlignmentPI::Type GetType() const { return m_type; }

    // Returns the PI location
    // @remarks for SCS and SS types, returns the 'overallPI'
    ROADRAILALIGNMENT_EXPORT DPoint3d GetPILocation() const;
    // Sets the PI location
    // @remarks for SCS and SS types, sets the 'overallPI'
    ROADRAILALIGNMENT_EXPORT bool SetPILocation(DPoint3dCR piPoint);

    // Returns the length between the start point of the curve and the PI point.
    // @remarks only used for basic validations. This is not a civil concept.
    ROADRAILALIGNMENT_EXPORT double GetPseudoTangentLength() const;

    NoCurveInfoCP GetNoCurve() const    { return (TYPE_NoCurve == m_type) ? &m_noCurveInfo : nullptr; }
    NoCurveInfoP GetNoCurveP()          { return (TYPE_NoCurve == m_type) ? &m_noCurveInfo : nullptr; }
    ArcInfoCP GetArc() const            { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    ArcInfoP GetArcP()                  { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    SCSInfoCP GetSCS() const            { return (TYPE_SCS == m_type) ? &m_scsInfo : nullptr; }
    SCSInfoP GetSCSP()                  { return (TYPE_SCS == m_type) ? &m_scsInfo : nullptr; }
    SSInfoCP GetSS() const              { return (TYPE_SS == m_type) ? &m_ssInfo : nullptr; }
    SSInfoP GetSSP()                    { return (TYPE_SS == m_type) ? &m_ssInfo : nullptr; }
}; // AlignmentPI

typedef AlignmentPI& AlignmentPIR;
typedef AlignmentPI const& AlignmentPICR;



enum class ZeroSlopePoints
{
    SagOnly = 0,
    CrestOnly = 1,
    BothSagAndCrest = 2
};

typedef enum VerticalCurveIndex
{
    PVC = 0,
    PVI = 1,
    PVT = 2
} VC;

enum class VerticalCurveType
{
    Linear = 0,
    TypeI = 1,
    TypeII = 2,
    TypeIII = 3,
    TypeIV = 4,
    Invalid = -1
};

enum class PVIType
{
    VerticalCurve = 0,
    PointCollection = 1
};

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlignmentPVI
{
    bvector<DPoint3d> poles;
    double length;

    ROADRAILALIGNMENT_EXPORT AlignmentPVI(DPoint3d loc, double len) : length(len)
        {
        DPoint3d pvc = loc;
        DPoint3d pvt = loc;
        poles.push_back(pvc);
        poles.push_back(loc);
        poles.push_back(pvt);
        }

    ROADRAILALIGNMENT_EXPORT AlignmentPVI (DPoint3d loc, DPoint3d pvc, DPoint3d pvt, double len) : length (len)
        {
        poles.push_back (pvc);
        poles.push_back (loc);
        poles.push_back (pvt);
        }

    // for a computed AlignmentPVI, return the K value
    ROADRAILALIGNMENT_EXPORT double KValue () const;
    // return a length computed based on a given k value
    ROADRAILALIGNMENT_EXPORT double LengthFromK (const double& kvalue);
};

//=======================================================================================
// @bsiclass
// Editing capabilities for alignment geometry
//=======================================================================================
struct AlignmentPairEditor : AlignmentPair
{
DEFINE_T_SUPER (AlignmentPair)

private:
    mutable bvector<AlignmentPI> m_cachedPIs; //< PIs cached on the first call of GetPIs.

protected:
    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor();
    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr _Clone() const override;
    ROADRAILALIGNMENT_EXPORT virtual void _UpdateHorizontalCurveVector(CurveVectorCR horizontalAlignment) override;
    ROADRAILALIGNMENT_EXPORT virtual void _UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment) override;     //&&AG WIP deal with cached PVIs

protected:
    //! Compute the PI by intersecting two rays.
    DPoint3d ComputePIFromPointsAndVectors(DPoint3d pointA, DVec3d vectorA, DPoint3d pointB, DVec3d vectorB) const;

    bool LoadArcData(AlignmentPI::Arc& arc, ICurvePrimitiveCR primitiveArc) const;
    bool LoadSpiralData(AlignmentPI::Spiral& spiral, ICurvePrimitiveCR primitiveSpiral) const;

    bool GetLinePI(AlignmentPIR pi, size_t index) const;
    bool GetArcPI(AlignmentPIR pi, size_t index) const;
    bool GetSCSPI(AlignmentPIR pi, size_t index) const;
    bool GetSSPI(AlignmentPIR pi, size_t index) const;

    // Creates an arc primitive
    // @return Arc or invalid primitive
    ICurvePrimitivePtr BuildArc(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, AlignmentPI::Orientation orientation) const;
    ICurvePrimitivePtr BuildArc(AlignmentPI::ArcInfoCR info) const;
    // Creates a SCS curve
    // @return CurveVector with 3 primitives or invalid curve
    CurveVectorPtr BuildSCSCurve(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, double spiralLength1, double spiralLength2) const;
    CurveVectorPtr BuildSCSCurve(AlignmentPI::SCSInfoCR info) const;
    // Creates a symmetric SS curve
    // @return CurveVector with 2 primitives and a radius, or an invalid curve and a negative radius
    CurveVectorPtr BuildSSCurve(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double spiralLength) const;
    CurveVectorPtr BuildSSCurve(AlignmentPI::SSInfoCR info) const;
    //! Builds a CurveVector off a vector of PIs
    //! @remarks caller should make sure all PIs are solved and validated before calling this
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr _BuildCurveVectorFromPIs(bvector<AlignmentPI> const& pis) const;

    // Fits the PI using current PI location and arc radius and adjacent PIs
    // Updates the PI information upon success
    bool SolveArcPI(bvector<AlignmentPI>& pis, size_t index) const;
    // Fits the PI using the current overall PI location, arc radius, spiral lengths and adjacent PIs
    // Updates the PI information upon success
    bool SolveSCSPI(bvector<AlignmentPI>& pis, size_t index) const;
    // Fits the PI using the curent overall PI location, spiral length and adjacent PIs
    // Updates the PI information upon success
    bool SolveSSPI(bvector<AlignmentPI>& pis, size_t index) const;
    //! Solve a PI based on its type
    ROADRAILALIGNMENT_EXPORT virtual bool _SolvePI(bvector<AlignmentPI>& pis, size_t index) const;

    ROADRAILALIGNMENT_EXPORT virtual bool _ValidatePIs(bvector<AlignmentPI> const& pis) const;

public:
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr Create(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment);
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr Create(AlignmentPairCR pair);

#if 0
private:
    void _GetValidEditRange(bvector<AlignmentPVI> const& pvis, int index, double * from, double *to);

protected:
        
        
    double _GetDesignLength (DPoint3d newPt, bvector<AlignmentPVI> const& pvis, int index, bool useComputedG = false);

    double _PreviousVerticalCurveStation (bvector<AlignmentPVI> const & pvis, const double& fromStation, VC vc = PVC);
    double _NextVerticalCurveStation (bvector<AlignmentPVI> const & pvis, const double& fromStation, VC vc = PVT);
    DPoint3d _GetZeroSlopePoint (MSBsplineCurveCP vcurve, double runningLength);
    double _VcurveZeroSlopeDistance (MSBsplineCurveCP vcurve);
    VerticalCurveType _GetVerticalCurveType (MSBsplineCurveCP vcurve);
    VerticalCurveType _GetVerticalCurveType (double G1, double G2);
    bool _ComputeVerticalCurveByThroughPoint (DPoint3d pt, AlignmentPVI pvi, double * resultantLength);
    bool _StationCompare (DPoint3d x, DPoint3d x1);
    bool _StationCompare (double x, double x1);
    bvector<AlignmentPVI> _GetPVIs ();
    bvector<AlignmentPVI> _GetPVIs (CurveVectorCR vt);
    CurveVectorPtr _BuildVectorFromPVIS (bvector<AlignmentPVI> pvis, double matchLen = -1.0);
    CurveVectorPtr _RemovePVIs (const double& startStation, const double& endStation);
    bool _SolvePVI (AlignmentPVI& pviToSolve, const AlignmentPVI& prevPVI, const AlignmentPVI& nextPVI, bool forceFit = true);
    bool _SolveAllPVIs (bvector<AlignmentPVI>& pvis);
    CurveVectorPtr _ReverseVertical ();

    bool _GenerateFromSingleVector (CurveVectorCR curveVector);

protected:

    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor (CurveVectorCR vertical, bool inXY);

public:
    static double _Slope (DPoint3d p1, DPoint3d p2);

    // override from base
    // this overridden version will "force" the vertical to match the exact station end of the HZ length (no fuzz)
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr GetPartialAlignment (double startStation, double endStation) const override;

#endif
    //////////// Horizontal Editing ////////////////////////////
    // Editing Tools
    // Allow the move pi, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MovePI(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MovePI(size_t index, AlignmentPIR inOutPI) const;

    CurveVectorPtr MovePCorPT(size_t index, DPoint3dCR toPt, bool isPC, AlignmentPI* pOutPI) const;
    // Allow the move pc, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MovePC(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    // Allow the move pt, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MovePT(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;

    CurveVectorPtr MoveBSorES(size_t index, DPoint3dCR toPt, bool isBS, AlignmentPI* pOutPI) const;
    // Allow the move beginning of a spiral, will ONLY affect the length, returns null if invalid
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MoveBS(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    // Allow the move end of a spiral, will ONLY affect the length, returns null if invalid
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr MoveES (size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;

    // update the radius of a particular PI index.  Will return nullptr if the radius is too large
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr UpdateRadius(size_t index, double radius, AlignmentPI* pOutPI = nullptr, bool validate = true);
    // change SCS PI to an Arc, set the spiral length to 0
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr RemoveSpirals(size_t index, AlignmentPI* pOutPI = nullptr) const;
    // change Arc PI to SCS and add the new spiral length, will return nullptr if the resultant curve doesn't work
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr AddSpirals(size_t index, double spiralLength, AlignmentPI* pOutPI = nullptr) const;

    // query for the slope at a given station
    ROADRAILALIGNMENT_EXPORT virtual double SlopeAtStation (const double& station);

    // insert a pi to a horizontal alignment, this method will insert the pi at a location
    // between the to "nearest" PIs
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr InsertPI(AlignmentPICR pi) const;
    // insert a PI at a given index using the curve information provided
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr InsertPI(AlignmentPICR pi, size_t index) const;
    // delete a pi on a horizontal alignment
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr DeletePI(DPoint3dCR piPoint) const;
    // delete a pi on a horizontal alignment
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr DeletePI(size_t index) const;
    // Get the PI Points
    ROADRAILALIGNMENT_EXPORT bvector<DPoint3d> GetPIPoints() const;
    // get the AlignmentPIs
    ROADRAILALIGNMENT_EXPORT bvector<AlignmentPI> GetPIs() const;
    // get a specific AlignmentPI
    ROADRAILALIGNMENT_EXPORT bool GetPI(AlignmentPIR pi, size_t index) const;


#if 0
    //////////// Vertical Editing /////////////////////////////
    // return a vector of points in X,Z format for high and low points on a profile
    ROADRAILALIGNMENT_EXPORT virtual bvector<DPoint3d> CrestAndSagPointsXZ(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);
    // return a vector of station values for high and low points
    ROADRAILALIGNMENT_EXPORT virtual bvector<double> CrestAndSagPointsStation(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);

    ROADRAILALIGNMENT_EXPORT virtual bool HasSag ();
    // return the maximum grade of the vertical design
    ROADRAILALIGNMENT_EXPORT virtual double MaximumGradeInPercent ();
    // return the maximum grade change |G1-G2| for the vertical design
    ROADRAILALIGNMENT_EXPORT virtual double MaximumGradeChangeInPercent ();

    // Editing Tools
    // Allow the move elevation of a pvi, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePVI (DPoint3d fromPt, DPoint3d toPt, StationRangeEditR editRange);
    // allow the move of a tangent segment up or down (pass the old center x,z and new center x, z)
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MoveVerticalTangent (DPoint3d fromPt, DPoint3d toPt, StationRangeEditR editRange);
    // Allow the move of a pvc or pvt based on station.  Will return invalid if the result
    // produces any overlapping curve or a 0 or negative length curve
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePVCorPVT (double fromSta, double toSta, StationRangeEditR editRange);
    // insert a pvi to a vertical alignment
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr InsertPVI (DPoint3d pviPt, double lengthOfCurve, StationRangeEditR editRange);
    // delete a pvi in a vertical alignment
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr DeletePVI (DPoint3d pviPt, StationRangeEditR editRange);
    // enforce pvi's to have some distance, unless no grade change
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr ValidateVerticalData (bool updateInternalCurveVector = false);
    // force a vertical curve or tangent through a point (point in x, z)
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr ForceThroughPoint (DPoint3d pt, StationRangeEditR editRange);
    // force a grade at a station, will hold the previous PVI and modify beyond.
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr ForceGradeAtStation (const double& station, const double& slopeAbsolute, StationRangeEditR editRange);
    // remove all PVIS between station range, primary use case is the removal of a grade crossing when an intersection changes
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr RemovePVIs (const double& fromSta, const double& toSta, StationRangeEditR editRange);

    // Add data for an start intersection
    ROADRAILALIGNMENT_EXPORT virtual bool InsertStartIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double endSlope, StationRangeEditP editP = nullptr);
    // Add data for an end intersection
    ROADRAILALIGNMENT_EXPORT virtual bool InsertEndIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double endSlope, StationRangeEditP editP = nullptr);
    // add data for a "crossing" intersection
    ROADRAILALIGNMENT_EXPORT virtual bool InsertCrossingIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double entranceSlope, double exitSlope, double stationWhereSecondaryIntersectsPrimaryPavement, StationRangeEditP editP = nullptr);

    // based on a horizontal change, the length of the vertical profile must change also
    // try and fudge in a "modified" vertical based on the stations and the new range
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr ModifyVerticalRange (StationRangeEditR editRange, double matchEndStation = -1.0);
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr ModifyVerticalRange (bvector<StationRangeEdit> editRanges, double matchEndStation = -1.0);
    // return a vector a points in XYZ format for the local high and low points on a roadway
    ROADRAILALIGNMENT_EXPORT bvector<DPoint3d> CrestAndSagPointsXYZ(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);

    // higher level call to allow full edit, for use with intersection updates
    ROADRAILALIGNMENT_EXPORT virtual bool MoveEndPIWithVerticalChange (DPoint3d toPt, bool isStart = true, StationRangeEditP stationRangeEditP = nullptr);    

    // return a "reversed" vertical alignment, this allows for "flipping"
    ROADRAILALIGNMENT_EXPORT AlignmentPairPtr FlipAlignment ();

    // return a horizontal edit range by comparing the two curve vectors
    ROADRAILALIGNMENT_EXPORT StationRangeEdit ComputeHorizontalEditRange (CurveVectorCR newHorizontal);



public:
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr Create (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr Create (AlignmentPair const& roadAlignment);
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr CreateVerticalOnly (CurveVectorCR verticalAlignment, bool inXY = false);

    // special alignment generator which uses a single curve vector to create
    // both a flattened horizontal and a vertical alignment from the z values
    ROADRAILALIGNMENT_EXPORT static AlignmentPairEditorPtr CreateFromSingleCurveVector (CurveVectorCR curveVector);
#endif
}; // AlignmentPairEditor

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE