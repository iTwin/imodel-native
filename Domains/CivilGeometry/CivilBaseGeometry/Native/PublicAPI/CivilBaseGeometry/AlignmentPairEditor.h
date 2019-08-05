/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include "AlignmentPair.h"
#include "StationRange.h"

BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

typedef struct AlignmentPI& AlignmentPIR;
typedef struct AlignmentPI const& AlignmentPICR;
typedef struct AlignmentPVI& AlignmentPVIR;
typedef struct AlignmentPVI const& AlignmentPVICR;

enum class Orientation { ORIENTATION_Unknown, ORIENTATION_CW, ORIENTATION_CCW };

//=======================================================================================
// @bsiclass
// Class that keeps all the marker bits used for our alignment classes.
// ECValue doesn't support storing marker bits directly on ICurvePrimitive,
// so we use CurvePrimitiveId and this API instead.
//=======================================================================================
struct AlignmentMarkerBits
{
private:
    static uint32_t GetMarkerBitsFromPrimitive(ICurvePrimitiveCR primitive);
    static void SetMarkerBitsToPrimitive(ICurvePrimitiveR primitive, uint32_t markerBits);

public:
    enum Bit
        {
        BIT_Vertical_IsParabolaLengthByK = ICurvePrimitive::CurvePrimitiveMarkerBit::CURVE_PRIMITIVE_BIT_ApplicationBit0
        };

    CIVILBASEGEOMETRY_EXPORT static void SetMarkerBit(ICurvePrimitiveR primitive, Bit bit, bool value);
    CIVILBASEGEOMETRY_EXPORT static bool GetMarkerBit(ICurvePrimitiveCR primitive, Bit bit);
};

//=======================================================================================
// @bsiclass
// AlignmentPI
// Data holder for different PI types stored using a discriminated union
//=======================================================================================
struct AlignmentPI
{
public:
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
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(NoCurveInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ArcInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(SCSInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(SSInfo)

    // Possible PI types
    enum Type
        {
        TYPE_Uninitialized  = 0,
        TYPE_NoCurve        = 1,
        TYPE_Arc            = 2,
        TYPE_SCS            = 3,
        TYPE_SS             = 4
        };
    
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
    CIVILBASEGEOMETRY_EXPORT AlignmentPI();
    CIVILBASEGEOMETRY_EXPORT void InitNoCurve(DPoint3dCR piPoint);
    CIVILBASEGEOMETRY_EXPORT void InitArc(DPoint3dCR piPoint, double radius);
    CIVILBASEGEOMETRY_EXPORT void InitSCS(DPoint3dCR overallPI, double arcRadius, double spiralLength1, double spiralLength2);
    CIVILBASEGEOMETRY_EXPORT void InitSS(DPoint3dCR overallPI, double spiralLength);
    void InitInvalid(Type piType); //! @private

    // Returns whether this PI is initialized
    bool IsInitialized() const { return TYPE_Uninitialized != m_type; }
    // Returns the PI type
    AlignmentPI::Type GetType() const { return m_type; }

    // Returns the PI location
    // @remarks for SCS and SS types, returns the 'overallPI'
    CIVILBASEGEOMETRY_EXPORT DPoint3dCR GetPILocation() const;
    // Sets the PI location
    // @remarks for SCS and SS types, sets the 'overallPI'
    CIVILBASEGEOMETRY_EXPORT bool SetPILocation(DPoint3dCR piPoint);

    // Returns the length between the start point of the curve and the PI point.
    // @remarks only used for basic validations. This is not a civil concept.
    CIVILBASEGEOMETRY_EXPORT double GetPseudoTangentLength() const;

    // If this PI defines an arc, returns it
    CIVILBASEGEOMETRY_EXPORT bool TryGetArcData(AlignmentPI::Arc& arc) const;
    // If this PI defines a spiral, returns it
    // @remarks the flag 'isFirstSpiral' is ignored if the PI only defines a single spiral
    CIVILBASEGEOMETRY_EXPORT bool TryGetSpiralData(AlignmentPI::Spiral& spiral, bool isFirstSpiral) const;

    NoCurveInfoCP GetNoCurve() const    { return (TYPE_NoCurve == m_type) ? &m_noCurveInfo : nullptr; }
    NoCurveInfoP GetNoCurveP()          { return (TYPE_NoCurve == m_type) ? &m_noCurveInfo : nullptr; }
    ArcInfoCP GetArc() const            { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    ArcInfoP GetArcP()                  { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    SCSInfoCP GetSCS() const            { return (TYPE_SCS == m_type) ? &m_scsInfo : nullptr; }
    SCSInfoP GetSCSP()                  { return (TYPE_SCS == m_type) ? &m_scsInfo : nullptr; }
    SSInfoCP GetSS() const              { return (TYPE_SS == m_type) ? &m_ssInfo : nullptr; }
    SSInfoP GetSSP()                    { return (TYPE_SS == m_type) ? &m_ssInfo : nullptr; }
}; // AlignmentPI


// PVI stuff
enum class ZeroSlopePoints
    {
    SagOnly = 0,
    CrestOnly = 1,
    BothSagAndCrest = 2
    };

enum class VerticalCurveType
    {
    Linear = 0,
    TypeI = 1,
    TypeII = 2,
    TypeIII = 3,
    TypeIV = 4,
    Invalid = -1
    };

//=======================================================================================
// @bsiclass
// AlignmentPVI
// Data holder for different PVI types stored using a discriminated union
//=======================================================================================
struct AlignmentPVI
{
public:
    //=======================================================================================
    // Keeps information about the origins of the PVI
    //=======================================================================================
    struct Provenance : RefCountedBase, NonCopyableClass
    {
    protected:
        CurveVectorCPtr m_curve;
    protected:
        Provenance(CurveVectorCR curve);
    public:
        CIVILBASEGEOMETRY_EXPORT bool ContainsLineString() const;
        CIVILBASEGEOMETRY_EXPORT static RefCountedPtr<Provenance> Create(CurveVectorCR curve);
        CIVILBASEGEOMETRY_EXPORT static RefCountedPtr<Provenance> Create(ICurvePrimitiveCR primitive);
    };
    typedef Provenance const* ProvenanceCP;

    //=======================================================================================
    //=======================================================================================
    struct GradeBreakInfo
    {
    DPoint3d pvi;
    };
    //=======================================================================================
    //=======================================================================================
    struct ArcInfo
    {
    DPoint3d pvi;
    DPoint3d pvc;
    DPoint3d pvt;

    double radius;
    Orientation orientation;
    };
    //=======================================================================================
    //=======================================================================================
    struct ParabolaInfo
    {
    DPoint3d pvi;
    DPoint3d pvc;
    DPoint3d pvt;

    bool isLengthByK;
    double length;
    double kValue; // Whenever the PVI is solved, we keep a copy of the KValue

    // Return the computed K value based on pvc/pvt slopes and current length
    //! @private
    CIVILBASEGEOMETRY_EXPORT double ComputeKValue() const;
    // Return a length computed based on a given K value
    CIVILBASEGEOMETRY_EXPORT double LengthFromK(double kvalue) const;
    };
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(GradeBreakInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ArcInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ParabolaInfo)

    // Possible PVI types
    enum Type
        {
        TYPE_Uninitialized  = 0,
        TYPE_GradeBreak     = 1,
        TYPE_Arc            = 2,
        TYPE_Parabola       = 3
        };
    
private:
    // Holds information about the provenance of the PVI, or invalid
    RefCountedPtr<Provenance> m_provenance;

    // The variable holding the current type
    Type m_type;

    // The union storing the data of this PVI
    union
        {
        GradeBreakInfo m_gradeBreakInfo;    //< If a Grade Break, holds the grade break information
        ArcInfo m_arcInfo;                  //< If an Arc, holds the arc information
        ParabolaInfo m_parabolaInfo;        //< If a Parabola, holds the parabola information
        };

public:
    CIVILBASEGEOMETRY_EXPORT AlignmentPVI();
    CIVILBASEGEOMETRY_EXPORT void InitGradeBreak(DPoint3dCR pviPoint);
    CIVILBASEGEOMETRY_EXPORT void InitArc(DPoint3dCR pviPoint, double radius);
    // Initializes a parabola PVI.
    // @remarks when isLengthByK is true, the third parameter is the K Value, and is the length otherwise
    CIVILBASEGEOMETRY_EXPORT void InitParabola(DPoint3dCR pviPoint, bool isLengthByK = false, double kValueOrLength = 0.0);
    void InitInvalid(Type pviType); //! @private

    // Returns whether this PVI is initialized
    bool IsInitialized() const { return TYPE_Uninitialized != m_type; }

    bool HasProvenance() const { return m_provenance.IsValid(); }
    ProvenanceCP GetProvenance() const { return m_provenance.get(); }
    void SetProvenance(Provenance& provenance) { m_provenance = &provenance; }

    // Returns the PVI type
    AlignmentPVI::Type GetType() const { return m_type; }

    // Returns the x-range this PVI occupies
    CIVILBASEGEOMETRY_EXPORT StationRange GetStationRange() const;
    // Returns the station range from PVC to PVI
    CIVILBASEGEOMETRY_EXPORT StationRange GetStationRangePVCPVI() const;
    // Returns the station range from PVI to PVT
    CIVILBASEGEOMETRY_EXPORT StationRange GetStationRangePVIPVT() const;

    // Returns the PVC location
    // @remarks for GradeBreak, returns the PVI
    CIVILBASEGEOMETRY_EXPORT DPoint3dCR GetPVCLocation() const;
    // Returns the PVI location
    CIVILBASEGEOMETRY_EXPORT DPoint3dCR GetPVILocation() const;
    // Returns the PVT location
    // @remarks for GradeBreak, returns the PVI
    CIVILBASEGEOMETRY_EXPORT DPoint3dCR GetPVTLocation() const;

    CIVILBASEGEOMETRY_EXPORT bool SetPVILocation(DPoint3dCR pviPoint);
    CIVILBASEGEOMETRY_EXPORT static double Slope(DPoint3dCR p0, DPoint3dCR p1);

    /// Determine if this PVI is a crest.  
    /// That is, the incoming grade is positive and the outgoing grade is negative.
    CIVILBASEGEOMETRY_EXPORT bool IsCrest() const;
    /// Determine if this PVI is a sag.
    /// That is, the incoming grade is negative and the outgoing grade is positive.
    CIVILBASEGEOMETRY_EXPORT bool IsSag() const;
    /// Sets highPoint to the high point on this PVI and returns true if this is a crest curve, otherwise returns false.
    CIVILBASEGEOMETRY_EXPORT bool HighDistance(double& highDistance) const;
    /// Sets lowDistance to the distance of the lowest point on the curve and returns true if this is a sag curve, otherwise returns false.
    CIVILBASEGEOMETRY_EXPORT bool LowDistance(double& lowDistance) const;

    GradeBreakInfoCP GetGradeBreak() const  { return (TYPE_GradeBreak == m_type) ? &m_gradeBreakInfo : nullptr; }
    GradeBreakInfoP GetGradeBreakP()        { return (TYPE_GradeBreak == m_type) ? &m_gradeBreakInfo : nullptr; }
    ArcInfoCP GetArc() const                { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    ArcInfoP GetArcP()                      { return (TYPE_Arc == m_type) ? &m_arcInfo : nullptr; }
    ParabolaInfoCP GetParabola() const      { return (TYPE_Parabola == m_type) ? &m_parabolaInfo : nullptr; }
    ParabolaInfoP GetParabolaP()            { return (TYPE_Parabola == m_type) ? &m_parabolaInfo : nullptr; }
}; // AlignmentPVI

//=======================================================================================
// @bsiclass
// Editing capabilities for alignment geometry
//=======================================================================================
struct AlignmentPairEditor : AlignmentPair
{
DEFINE_T_SUPER (AlignmentPair)

private:
    mutable bvector<AlignmentPI> m_cachedPIs;   //< PIs cached on the first call of GetPIs.
    mutable bvector<AlignmentPVI> m_cachedPVIs; //< PVIs cached on the first call of GetPVIs

protected:
    AlignmentPairEditor() {}
    CIVILBASEGEOMETRY_EXPORT AlignmentPairEditor(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment);
    CIVILBASEGEOMETRY_EXPORT virtual AlignmentPairPtr _Clone() const override;
    CIVILBASEGEOMETRY_EXPORT virtual void _UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment) override;
    CIVILBASEGEOMETRY_EXPORT virtual void _UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment) override;

protected:
    //! Compute the PI by intersecting two rays.
    DPoint3d ComputePIFromPointsAndVectors(DPoint3d pointA, DVec3d vectorA, DPoint3d pointB, DVec3d vectorB) const;

    bool LoadArcData(AlignmentPI::Arc& arc, ICurvePrimitiveCR primitiveArc) const;
    bool LoadSpiralData(AlignmentPI::Spiral& spiral, ICurvePrimitiveCR primitiveSpiral) const;

    bool GetLinePI(AlignmentPIR pi, size_t primitiveIdx) const;
    bool GetArcPI(AlignmentPIR pi, size_t primitiveIdx) const;
    bool GetSCSPI(AlignmentPIR pi, size_t primitiveIdx) const;
    bool GetSSPI(AlignmentPIR pi, size_t primitiveIdx) const;

    // Creates an arc primitive
    // @return Arc or invalid primitive
    ICurvePrimitivePtr BuildArc(DPoint3dCR prevPI, DPoint3dCR currPI, DPoint3dCR nextPI, double radius, Orientation orientation) const;
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
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr _BuildCurveVectorFromPIs(bvector<AlignmentPI> const& pis) const;

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
    CIVILBASEGEOMETRY_EXPORT virtual bool _SolvePI(bvector<AlignmentPI>& pis, size_t index) const;

    CIVILBASEGEOMETRY_EXPORT virtual bool _ValidatePIs(bvector<AlignmentPI> const& pis) const;

public:
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr Create(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment);
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr Create(AlignmentPairCR pair);
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr CreateVerticalOnly(CurveVectorCR verticalAlignment);

    // Returns the transform that converts back and forth between XY and XZ
    CIVILBASEGEOMETRY_EXPORT static TransformCR GetFlipYZTransform();

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

public:
    static double _Slope (DPoint3d p1, DPoint3d p2);

    // override from base
    // this overridden version will "force" the vertical to match the exact station end of the HZ length (no fuzz)
    CIVILBASEGEOMETRY_EXPORT virtual AlignmentPairPtr GetPartialAlignment (double startStation, double endStation) const override;

#endif
    //////////// Horizontal Editing ////////////////////////////
    // Editing Tools
    // Allow the move pi, will return invalid if appropriate
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePI(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePI(size_t index, AlignmentPIR inOutPI) const;

    CurveVectorPtr MovePCorPT(size_t index, DPoint3dCR toPt, bool isPC, AlignmentPI* pOutPI) const;
    // Allow the move pc, will return invalid if appropriate
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePC(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    // Allow the move pt, will return invalid if appropriate
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePT(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;

    CurveVectorPtr MoveBSorES(size_t index, DPoint3dCR toPt, bool isBS, AlignmentPI* pOutPI) const;
    // Allow the move beginning of a spiral, will ONLY affect the length, returns null if invalid
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MoveBS(size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;
    // Allow the move end of a spiral, will ONLY affect the length, returns null if invalid
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MoveES (size_t index, DPoint3dCR toPt, AlignmentPI* pOutPI = nullptr) const;

    // Updates the radius of a particular PI or nullptr if the radius is too large
    // @remarks will only work for ARC or SCS types
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr UpdateRadius(size_t index, double radius, AlignmentPI* pOutPI = nullptr) const;
    // Updates the length of both spirals
    // @remarks will only work for SCS or SS types.
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr UpdateSpiralLengths(size_t index, double spiralLength, AlignmentPI* pOutPI = nullptr) const;

    // change SCS PI to an Arc, set the spiral length to 0
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr RemoveSpirals(size_t index, AlignmentPI* pOutPI = nullptr) const;
    // change Arc PI to SCS and add the new spiral length, will return nullptr if the resultant curve doesn't work
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr AddSpirals(size_t index, double spiralLength, AlignmentPI* pOutPI = nullptr) const;

    // insert a pi to a horizontal alignment, this method will insert the pi at a location
    // between the to "nearest" PIs
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr InsertPI(AlignmentPICR pi) const;
    // insert a PI at a given index using the curve information provided
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr InsertPI(AlignmentPICR pi, size_t index) const;
    // delete a pi on a horizontal alignment
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr DeletePI(DPoint3dCR piPoint) const;
    // delete a pi on a horizontal alignment
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr DeletePI(size_t index) const;
    // Get the PI Points
    CIVILBASEGEOMETRY_EXPORT bvector<DPoint3d> GetPIPoints() const;
    // get the AlignmentPIs
    CIVILBASEGEOMETRY_EXPORT bvector<AlignmentPI> GetPIs() const;
    // get a specific AlignmentPI
    CIVILBASEGEOMETRY_EXPORT bool GetPI(AlignmentPIR pi, size_t index) const;



protected:
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct VerticalEditResult
    {
    StationRange modifiedRange;
    CurveVectorPtr vtCurve;
    };
    // Helper to reduce duplication of code
    // Calls _SolvePVI() on adjacent PVIs, _ValidatePVIs() and _BuildCurveVectorFromPVIs().
    VerticalEditResult SolveValidateAndBuild(bvector<AlignmentPVI>& pvis, size_t index, bool isDelete) const;

    bool AreStationsEqual(double station0, double station1) const;
    bool AreStationsEqual(DPoint3dCR p0, DPoint3dCR p1) const;
    // Checks whether the PVI at the given index overlaps the previous or next PVI
    // @remarks only looks for PVI and ignores PVC/PVT
    bool IsPVIOverlap(bvector<AlignmentPVI> const& pvis, size_t index) const;
    // Computes the maximum length a PVI can have and still fit in between adjacent PVIs
    double ComputeMaximumLength(bvector<AlignmentPVI> const& pvis, size_t index) const;

    // Returns the index of the first PVI whose station is greater or equal to the station
    // @remarks returns pvis.size() when all PVIs have smaller stations
    size_t FindNexOrEqualPVIIndex(bvector<AlignmentPVI> const& pvis, double station) const;
    size_t FindEqualPVIIndex(bvector<AlignmentPVI> const& pvis, double station) const;
    bool FindEqualPVCorPVT(size_t& pviIndex, bool& isPVC, bvector<AlignmentPVI> const& pvis, double station) const;
    
    bool LoadVerticalArcData(AlignmentPVIR pi, ICurvePrimitiveCR primitiveArc) const;
    bool LoadVerticalParabolaData(AlignmentPVIR pvi, ICurvePrimitiveCR primitiveParabola) const;

    // Creates an arc primitive
    // @return Arc or invalid primitive
    ICurvePrimitivePtr BuildVerticalArc(AlignmentPVI::ArcInfoCR info) const;
    // Create a parabola primitive (BSpline)
    // @return MSBSpline or invalid primitive
    ICurvePrimitivePtr BuildVerticalParabola(AlignmentPVI::ParabolaInfoCR info) const;

    // Fits the arc PVI given the arc radius and adjacent PVIs
    // Updates the PVI information upon success
    bool SolveArcPVI(bvector<AlignmentPVI>& pis, size_t index) const;
    // Fits the parabola PVI given length and adjacent PVIs
    // Updates the PVI information upon success
    bool SolveParabolaPVI(bvector<AlignmentPVI>& pis, size_t index) const;
    //! Builds a CurveVector off a vector of PVIs
    //! @remarks caller should make sure all PVIs are solved and validated before calling this
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr _BuildCurveVectorFromPVIs(bvector<AlignmentPVI> const& pvis) const;

    CIVILBASEGEOMETRY_EXPORT virtual bool _SolvePVI(bvector<AlignmentPVI>& pvis, size_t index) const;
    CIVILBASEGEOMETRY_EXPORT virtual bool _ValidatePVIs(bvector<AlignmentPVI> const& pvis) const;

public:
    //////////// Vertical Editing /////////////////////////////
    // query for the slope at a given station
    // Note this function was never finished, leaving it in here
    CIVILBASEGEOMETRY_EXPORT virtual double SlopeAtStation(const double& station);

#if 0
    // return a vector of points in X,Z format for high and low points on a profile
    CIVILBASEGEOMETRY_EXPORT virtual bvector<DPoint3d> CrestAndSagPointsXZ(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);
    // return a vector of station values for high and low points
    CIVILBASEGEOMETRY_EXPORT virtual bvector<double> CrestAndSagPointsStation(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);

    CIVILBASEGEOMETRY_EXPORT virtual bool HasSag ();
    // return the maximum grade of the vertical design
    CIVILBASEGEOMETRY_EXPORT virtual double MaximumGradeInPercent ();
    // return the maximum grade change |G1-G2| for the vertical design
    CIVILBASEGEOMETRY_EXPORT virtual double MaximumGradeChangeInPercent ();

#endif
    // Editing Tools
    // Allow the move elevation of a pvi, will return invalid if appropriate
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePVI(size_t index, DPoint3dCR toPt, StationRangeEditP pRangeEdit = nullptr) const;
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MovePVI(size_t index, AlignmentPVIR inOutPVI, StationRangeEditP pRangeEdit = nullptr) const;
    // allow the move of a tangent segment up or down (pass the old center x,z and new center x, z)
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MoveVerticalTangent(DPoint3dCR fromPt, DPoint3dCR toPt, StationRangeEditP pRAngeEdit = nullptr) const;
    // Allow the move of a parabola pvc or pvt based on station. Result may be invalid
    // Updates the length based on the new position of the pvc/pvt and the 'holdLength' flag
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MoveParabolaPVCorPVT(double fromDistanceAlong, double toDistanceAlong, StationRangeEditP pEditRange = nullptr) const;
    // Allow the move of an arc pvc or pvt based on station. Result may be invalid
    // Updates the arc radius based on the new position of the pvc/pvt
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr MoveArcPVCorPVT(double fromDistanceAlong, double toDistanceAlong, StationRangeEditP pEditRange = nullptr) const;
    // Updates the radius of a particular PVI or nullptr if the radius is too large
    // @remarks will only work for ARC types
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr UpdateVerticalRadius(size_t index, double radius, StationRangeEditP pRangeEdit = nullptr) const;
    // insert a pvi to a vertical alignment
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr InsertPVI(AlignmentPVICR pi, StationRangeEditP pRangeEdit = nullptr) const;
    // delete a pvi in a vertical alignment
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr DeletePVI(DPoint3dCR pviPoint, StationRangeEditP pRangeEdit = nullptr) const;
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr DeletePVI(size_t index, StationRangeEditP pRangeEdit = nullptr) const;
    // remove all PVIS between station range, primary use case is the removal of a grade crossing when an intersection changes
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr DeletePVIs(StationRangeCR range, StationRangeEditP pRangeEdit = nullptr) const;
    // force a grade at a station, will hold the previous PVI and modify beyond.
    CIVILBASEGEOMETRY_EXPORT CurveVectorPtr ForceGradeAtStation(double distanceAlongFromStart, double slope, StationRangeEditP pEditRange = nullptr) const;

#if 0
    // enforce pvi's to have some distance, unless no grade change
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr ValidateVerticalData (bool updateInternalCurveVector = false);
    // force a vertical curve or tangent through a point (point in x, z)
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr ForceThroughPoint (DPoint3d pt, StationRangeEditR editRange);

    // Add data for an start intersection
    CIVILBASEGEOMETRY_EXPORT virtual bool InsertStartIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double endSlope, StationRangeEditP editP = nullptr);
    // Add data for an end intersection
    CIVILBASEGEOMETRY_EXPORT virtual bool InsertEndIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double endSlope, StationRangeEditP editP = nullptr);
    // add data for a "crossing" intersection
    CIVILBASEGEOMETRY_EXPORT virtual bool InsertCrossingIntersection (CurveVectorCR drapeVector, double lengthofgradeapproachinmeters, double entranceSlope, double exitSlope, double stationWhereSecondaryIntersectsPrimaryPavement, StationRangeEditP editP = nullptr);

    // based on a horizontal change, the length of the vertical profile must change also
    // try and fudge in a "modified" vertical based on the stations and the new range
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr ModifyVerticalRange (StationRangeEditR editRange, double matchEndStation = -1.0);
    CIVILBASEGEOMETRY_EXPORT virtual CurveVectorPtr ModifyVerticalRange (bvector<StationRangeEdit> editRanges, double matchEndStation = -1.0);
    // return a vector a points in XYZ format for the local high and low points on a roadway
    CIVILBASEGEOMETRY_EXPORT bvector<DPoint3d> CrestAndSagPointsXYZ(ZeroSlopePoints zsType = ZeroSlopePoints::BothSagAndCrest);

    // higher level call to allow full edit, for use with intersection updates
    CIVILBASEGEOMETRY_EXPORT virtual bool MoveEndPIWithVerticalChange (DPoint3d toPt, bool isStart = true, StationRangeEditP stationRangeEditP = nullptr);    

    // return a "reversed" vertical alignment, this allows for "flipping"
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr FlipAlignment ();

    // return a horizontal edit range by comparing the two curve vectors
    CIVILBASEGEOMETRY_EXPORT StationRangeEdit ComputeHorizontalEditRange (CurveVectorCR newHorizontal);
#endif
    CIVILBASEGEOMETRY_EXPORT bvector<AlignmentPVI> GetPVIs() const;
    // get a specific AlignmentPVI
    CIVILBASEGEOMETRY_EXPORT bool GetPVI(AlignmentPVIR pvi, size_t index) const;

#if 0
public:
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr Create (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr Create (AlignmentPair const& roadAlignment);
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr CreateVerticalOnly (CurveVectorCR verticalAlignment, bool inXY = false);

    // special alignment generator which uses a single curve vector to create
    // both a flattened horizontal and a vertical alignment from the z values
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairEditorPtr CreateFromSingleCurveVector (CurveVectorCR curveVector);
#endif
}; // AlignmentPairEditor

END_BENTLEY_CIVILGEOMETRY_NAMESPACE
