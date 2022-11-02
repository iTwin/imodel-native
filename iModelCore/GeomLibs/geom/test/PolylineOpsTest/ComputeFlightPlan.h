/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// Single point for flight path, annotated with properties of the FOLLOWING edge
struct FlightPoint
    {
    DPoint3d m_xyz;     // coordinates of this node (base of edge)
                        // Properties of the OUTGOING edge . . .
    bool    m_isBoundary;   // originally a boundary edge.
    bool    m_isBarrier;    // originally a barrier line
    bool    m_isRule;       // originally one of the parallel rule lines.
    bool    m_isCamera;     // camera "on"
    FlightPoint (DPoint3dCR xyz, bool isBoundary, bool isBarrier, bool isRule, bool isCamera)
        : m_xyz (xyz), m_isBoundary (isBoundary), m_isBarrier (isBarrier), m_isRule (isRule), m_isCamera (isCamera)
        {
        }
    FlightPoint (DPoint3dCR xyz)
        : m_xyz (xyz), m_isBoundary (false), m_isBarrier (false), m_isRule (false), m_isCamera (false)
        {
        }
    static void GetXYZ (bvector<FlightPoint> const &source, bvector<DPoint3d> &dest)
        {
        dest.clear ();
        for (auto &data : source)
            dest.push_back (data.m_xyz);
        }
    };

struct FlightPlanContext
{
bool NodeOrMateHasMask (VuP node, VuMask mask)
    {
    return node->HasMask (mask) || node->EdgeMate ()->HasMask (mask);
    }

// Example of computing a flight plan to cover a single region by repeated passes at specified angle.
void ComputeGridFlightPlanOnGrid
(
    bvector<DPoint3d > const &boundary,     //!< [in] closed boundary for polygon to be covered by grid lines
    bvector<bvector<DPoint3d>>const &holes, //!< [in] 0 or many holes that are not to be crossed.
    DPoint3d gridReferencePoint,            //!< [in] point that the flight grid must pass through.
    double spacing,                         //!< [in] spacing between flight lines.
    Angle gridAngle,                        //!< [in] angle from x to flight lines.
    DPoint3dCR flightStartEnd,              //!< [in] start and end point of flight.
    bvector<FlightPoint> &flightPoints        //!< [out] complete path.
    )
    {
    double myTolerance = 0.001;
    flightPoints.clear ();
    FlightPlanner planner (myTolerance);
    // DEFINE THE PRIMARY BOUNDARY ....
    planner.AddPrimaryBoundary (boundary);
    // DEFINE THE HOLES ....
    for (auto &hole : holes)
        {
        planner.AddBarrierBoundary (hole);
        }
    // DEFINE THE RULE LINES
    planner.AddRuleLines (gridReferencePoint, DVec3d::From (gridAngle.Cos (), gridAngle.Sin (), 0), spacing, true);
    // MERGE IT ALL TOGETHER FOR PLANNING
    planner.Merge (true, true);

    // CLOSEST VERTEX IN THE GRAPH BECOMES START and END OF PATH ...
    VuP seedNode = planner.AnyNodeAtClosestVertex (flightStartEnd);
    bvector<VuP> pathNodes;
    if (seedNode != nullptr)
        {
        planner.PlanPath (seedNode, pathNodes);
        for (auto node : pathNodes)
            {
            VuP mate = node->EdgeMate ();
            UNUSED_VARIABLE(mate);
            flightPoints.push_back (
                FlightPoint
                (
                    node->GetXYZ (),
                    NodeOrMateHasMask (node, planner.m_masks.primaryBoundary),
                    NodeOrMateHasMask (node, planner.m_masks.barrierBoundary),
                    NodeOrMateHasMask (node, planner.m_masks.cameraCandidate),
                    NodeOrMateHasMask (node, planner.m_masks.cameraEdge)
                    )
                );
            }

        if (seedNode->GetXYZ ().DistanceXY (flightStartEnd) > myTolerance)
            {
            // fly from and back to base point ..
            flightPoints.insert (flightPoints.begin (), flightStartEnd);
            flightPoints.push_back (flightStartEnd);
            }
        }
#ifdef CompileForGTestWithSaveGraph
    // (Thisis for debug -- TomC/Sadat do not need it)
    auto baseTransform = Check::GetTransform ();
    SaveGraph (planner.Graph ());
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.primaryBoundary);
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.primaryExterior);
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.barrierBoundary);
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.barrierExterior);
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.cameraCandidate);
    Check::Shift (0, 110, 0);

    SaveGraph (planner.Graph (), planner.m_masks.cameraEdge);
    Check::SetTransform (baseTransform);
    Check::Shift (110,0,0);

#endif
    }

#if defined CompileForMGeometryInterface
bvector<DPoint3d> convertTobVec (std::vector<MPoint2d> &array)
    {
    bvector<DPoint3d> bVec;
    for (int i = 0; i < array.size (); i++)
        {
        MPoint2d point = array.at (i);
        bVec.push_back (DPoint3d::From (point.X (), point.Y ()));
        }
    return bVec;
    }

//Purpose of this is to call the C++ function ComputeFlightPlan with data recieved from an ObjectiveC function. Look at definitions above for reference.
void computeFlightPlanFromBoundaries (
    std::vector<MPoint2d> &boundary,
    std::vector<std::vector<MPoint2d>> &exclusionZoneBoundaries,
    MPoint2d referencePoint,
    double spacing,
    double angle,
    MPoint2d homepoint,
    std::vector<MPoint2d>* pathPoints,
    std::vector<bool>* cameraPoint)
    {
    bvector<bvector<DPoint3d>> boundaries;

    for (int i = 0; i < exclusionZoneBoundaries.size (); i++)
        {
        std::vector<MPoint2d> exBoundary = exclusionZoneBoundaries.at (i);
        boundaries.push_back (convertTobVec (exBoundary));
        }
    bvector<DPoint3d> convexHullBVec = convertTobVec (boundary);

    DPoint3d homepoint3D = DPoint3d::From (homepoint.X (), homepoint.Y ());
    bvector<DPoint3d> flightPath;
    bvector<bool> cameraLines;
    ComputeFlightPath* computeFlightPath = new ComputeFlightPath ();
    computeFlightPath->computeFlightPlan (convexHullBVec, boundaries, homepoint3D, spacing, Angle::FromRadians (angle), homepoint3D, flightPath, cameraLines);
    for (int i = 0; i < flightPath.size (); i++)
        {
        DPoint3d flightPoint = flightPath.at (i);
        pathPoints->push_back (MPoint2d (flightPoint.x, flightPoint.y));
        }

    for (int i = 0; i < cameraLines.size (); i++)
        {
        cameraPoint->push_back (cameraLines.at (i));
        }
    }

#endif
};