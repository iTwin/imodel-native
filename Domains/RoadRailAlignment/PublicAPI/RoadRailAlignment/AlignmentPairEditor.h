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

#define CS_SPI_INFINITY            ( ( double )1.0E+38 )

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlignmentSpiral
{
    AlignmentSpiral ()
    {
    entranceRadius = 0.0;
    exitRadius = 0.0;
    length = 0.0;
    }

    DPoint3d beginSpiralPt;
    DPoint3d endSpiralPt;
    DPoint3d spiralPIPt;

    DVec3d startVector;
    DVec3d endVector;

    double entranceRadius;
    double exitRadius;
    double length;
}; // AlignmentSpiral

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlignmentArc
{
    AlignmentArc ()
    {
    sweep = radius = 0.0;
    clockwise = false;
    }

    DPoint3d startPoint;
    DPoint3d endPoint;
    DPoint3d centerPoint;
    double radius;
    double sweep;
    bool clockwise;
}; // AlignmentArc

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlignmentPI
{
enum HorizontalPIType
    {
    ARC = 0,
    SCS = 1,
    SS = 2,
    NONE = 3
    };

    DPoint3d location;

    AlignmentArc arc;
    AlignmentSpiral spiral1;
    AlignmentSpiral spiral2;

    HorizontalPIType curveType;

AlignmentPI ()
    {
    curveType = NONE;
    }

AlignmentPI (DPoint3d loc) : location (loc)
    {
    curveType = NONE;
    }
}; // AlignmentPI

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

typedef struct AlignmentPI& AlignmentPIR;
typedef struct AlignmentPI const& AlignmentPICR;

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlignmentPairEditor : AlignmentPair
{
DEFINE_T_SUPER (AlignmentPair)

private:
    void _GetValidEditRange (bvector<AlignmentPVI> const& pvis, int index, double * from, double *to);

protected:
    // move help
    ROADRAILALIGNMENT_EXPORT bool _ValidatePIs (bvector<AlignmentPI> const& pis);
    ROADRAILALIGNMENT_EXPORT bool _ValidateMove (size_t index, DPoint3d toPt, bvector<AlignmentPI> const& pis);
        
    ROADRAILALIGNMENT_EXPORT virtual StatusInt _SolveArcPI (size_t index, bvector<AlignmentPI>& pis);
    ROADRAILALIGNMENT_EXPORT virtual StatusInt _SolveSpiralPI (size_t index, bvector<AlignmentPI>& pis);
    ROADRAILALIGNMENT_EXPORT virtual StatusInt _SolveSSPI (size_t index, bvector<AlignmentPI>& pis);
    ROADRAILALIGNMENT_EXPORT virtual StatusInt _SolvePI (size_t index, bvector<AlignmentPI>& pis);
    ROADRAILALIGNMENT_EXPORT virtual StatusInt _LoadSpiralData(ICurvePrimitiveCP primitive, AlignmentSpiral& roadSpiral);
    ROADRAILALIGNMENT_EXPORT bvector<AlignmentPI> _GetPIs();
        
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr _BuildVectorFromPIS(bvector<AlignmentPI> const& pvis);

    double _FullTangentLength (AlignmentPICR pi);
    double _GetDesignLength (DPoint3d newPt, bvector<AlignmentPVI> const& pvis, int index, bool useComputedG = false);

    ICurvePrimitivePtr CreateSpiralPrimitive (AlignmentSpiral const& spiral);                       
    bool AddPrimitivesFromPI (CurveVectorR hvec, const AlignmentPI& pi, DPoint3dR lastPoint);

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
    bvector<AlignmentPVI> _GetPVIs (CurveVectorR vt);
    CurveVectorPtr _BuildVectorFromPVIS (bvector<AlignmentPVI> pvis, double matchLen = -1.0);
    CurveVectorPtr _RemovePVIs (const double& startStation, const double& endStation);
    bool _SolvePVI (AlignmentPVI& pviToSolve, const AlignmentPVI& prevPVI, const AlignmentPVI& nextPVI, bool forceFit = true);
    bool _SolveAllPVIs (bvector<AlignmentPVI>& pvis);
    CurveVectorPtr _ReverseVertical ();

    bool _GenerateFromSingleVector (CurveVectorCR curveVector);

protected:

    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor (CurveVectorCR vertical, bool inXY);
    ROADRAILALIGNMENT_EXPORT AlignmentPairEditor () { }

public:
    static double _Slope (DPoint3d p1, DPoint3d p2);

    // override from base
    // this overridden version will "force" the vertical to match the exact station end of the HZ length (no fuzz)
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr GetPartialAlignment (double startStation, double endStation) const override;

    // check if horizontal is valid
    ROADRAILALIGNMENT_EXPORT virtual bool IsHorizontalValid ();
    //////////// Horizontal Editing ////////////////////////////
    // Editing Tools
    // Allow the move pi, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePI (size_t index, DPoint3d toPt, AlignmentPIR pi, double minRadius = 0.0, bvector<AlignmentPI>* pis = nullptr, AlignmentPI::HorizontalPIType piType = AlignmentPI::ARC);
    // Allow the move pc, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePC (size_t index, DPoint3d toPt, AlignmentPIR pi);
    // Allow the move pt, will return invalid if appropriate
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePT (size_t index, DPoint3d toPt, AlignmentPIR pi);
    // Allow the move beginning of a spiral, will ONLY affect the length, returns null if invalid
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MoveBS (size_t index, DPoint3d toPt, AlignmentPIR pi);
    // Allow the move end of a spiral, will ONLY affect the length, returns null if invalid
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MoveES (size_t index, DPoint3d toPt, AlignmentPIR pi);
    // update the radius of a particular PI index.  Will return nullptr if the radius is too large
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr UpdateRadius (size_t index, double radius, AlignmentPIR pi, bool validate = true);
    // change PI to an arc only, set the spiral length to 0
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr RemoveSpirals (size_t index, AlignmentPIR pi);
    // change PI to SCS and add the new spiral length, will return nullptr if the resultant curve doesn't work
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr AddSpirals (size_t index, double length, AlignmentPIR pi);
    // query the index to see for the PI type
    ROADRAILALIGNMENT_EXPORT AlignmentPI::HorizontalPIType GetHorizontalPIType (size_t index);
    // query for the slope at a given station
    ROADRAILALIGNMENT_EXPORT virtual double SlopeAtStation (const double& station);

    // change a curve length
    //        ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr MovePCorPT (double fromSta, double toSta);
    // insert a pi to a horizontal alignment, this method will insert the pi at a location
    // between the to "nearest" PIs
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr InsertPI (DPoint3d piPt);
    // insert a PI at a given index using the curve information provided
    ROADRAILALIGNMENT_EXPORT CurveVectorPtr InsertPI (size_t index, AlignmentPIR pi);
    // delete a pi on a horizontal alignment
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr DeletePI (DPoint3d pviPt);
    // delete a pi on a horizontal alignment
    ROADRAILALIGNMENT_EXPORT virtual CurveVectorPtr DeletePI (size_t index);
    // get the PI points
    ROADRAILALIGNMENT_EXPORT virtual StatusInt GetPIPoints (bvector<DPoint3d>& pts);
    // get the AlignmentPIs
    ROADRAILALIGNMENT_EXPORT virtual StatusInt GetAlignmentPIs (bvector<AlignmentPI>& pis);
    // get a specific AlignmentPI
    ROADRAILALIGNMENT_EXPORT virtual StatusInt GetAlignmentPI (size_t index, AlignmentPI& pi);


    //////////// Vertical Editing /////////////////////////////
    // check if the alignment has a valid vertical
    ROADRAILALIGNMENT_EXPORT virtual bool IsVerticalValid ();
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
}; // AlignmentPairEditor

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE