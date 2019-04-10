/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/Native/PublicAPI/CivilBaseGeometry/AlignmentPairIntersection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CivilBaseGeometry.h"
#include "AlignmentPair.h"

BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

#define ALIGNMENTPAIRINTERSECTION_TOLERANCE 7.0

//=======================================================================================
// @bsiclass
// base class to handle specialty intersection geometry
//=======================================================================================
struct AlignmentPairIntersection : NonCopyableClass, RefCountedBase
{
protected:
    AlignmentPairPtr m_primaryRoad;
    AlignmentPairPtr m_secondaryRoad;

    double m_primaryRightOffset;
    double m_primaryLeftOffset;
    double m_secondaryRightOffset;
    double m_secondaryLeftOffset;

    bool m_isExit;

protected:
    //! @private
    CIVILBASEGEOMETRY_EXPORT AlignmentPairIntersection(AlignmentPairP primary, AlignmentPairP secondary)
        : m_primaryRoad (primary), m_secondaryRoad (secondary)
        {
        m_primaryLeftOffset = m_primaryRightOffset = m_secondaryLeftOffset = m_secondaryRightOffset = ALIGNMENTPAIRINTERSECTION_TOLERANCE;
        }

    // return (if possible) a DEllipse3d which starts at pointA, with initial tangent vector towards pointB, ends on the line containing pointB and pointC
    //! @private
    static ValidatedDEllipse3d _ArcFromStartShoulderTarget
        (
        DPoint3dCR pointA, // Start point of arc
        DPoint3dCR pointB, // shoulder point
        DPoint3dCR pointC  // target point for outbound tangent
        );

    //! @private
    bool ComputeIntersectionPointImpl(DPoint3dR pt, DPoint3dCP checkPoint, double * primaryStation = nullptr, double * secondaryStation = nullptr, bool allowExtension = true);

protected:
    CIVILBASEGEOMETRY_EXPORT bool ComputeOffsetFillet (const double& offsetDist, const double& radius, bvector<CurveCurve::FilletDetail> &arcs);

public:
    // override primary offset checks with real data
    CIVILBASEGEOMETRY_EXPORT void SetPrimaryOffsets (const double& right, const double& left);
    // override secondary offset checks with real data
    CIVILBASEGEOMETRY_EXPORT void SetSecondaryOffsets (const double& right, const double& left);

    // compute the necessary geometry to create an entrance intersection
    // offsets should be signed
    CIVILBASEGEOMETRY_EXPORT bool ComputePotentialFillets (const double& primaryOffset, const double& secondaryOffset, const double& radius, bvector<CurveCurve::FilletDetail> &arcs);
    // calculate the actual intersection point
    CIVILBASEGEOMETRY_EXPORT bool ComputeIntersectionPoint (DPoint3dR pt, double * primaryStation = nullptr, double * secondaryStation = nullptr, bool allowExtension = true);

    // accessor for primary road
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr PrimaryRoad () { return m_primaryRoad; }
    // accessor for secondary road
    CIVILBASEGEOMETRY_EXPORT AlignmentPairPtr SecondaryRoad () { return m_secondaryRoad; }

    // update the secondary road
    CIVILBASEGEOMETRY_EXPORT void UpdateSecondaryRoad (CurveVectorCR hz, CurveVectorCP vt);

public:
    //! Create a new AlignmentPairIntersectionPtr 
    CIVILBASEGEOMETRY_EXPORT static AlignmentPairIntersectionPtr Create (AlignmentPairCP primary, AlignmentPairCP secondary);

    // Construct two arcs and a line segment such that:
    // start at pointA.
    // first arc tangent is directionA.
    // shoulder point of first arc is distanceA along that tangent.
    // end at pointB
    // second arc tangent direction at pointB is directionB.
    // shoulder point of second arc is distanceB along the tangent.
    // NOTE NOTE NOTE both tangent directions are "inbound" to the arcs -- expect directionA to point "forward towards" pointB and directionB "backwards towards" pointA
    //
    CIVILBASEGEOMETRY_EXPORT static CurveVectorPtr ConstructDoubleFillet (DPoint3dCR pointA,
                    DVec3dCR directionA, double distanceA, DPoint3dCR pointB, DVec3dCR directionB, double distanceB);

}; // AlignmentPairIntersection

struct AlignmentIntersection;
//
// Information related to potential intersection between two alignments
//
struct AlignmentIntersectionInfo
{
    friend AlignmentIntersection;

public:
    enum class ExtensionType { None, PrimaryStart, PrimaryEnd, SecondaryStart, SecondaryEnd };

protected:
    DPoint3d m_point;
    DVec3d m_secondaryTangentAtIntersect;
    ExtensionType m_extensionType;
    double m_extendedAmount;       // if extended, this is the length of the extension
    double m_primaryStation;
    double m_secondaryStation;
    double m_Zdifferential;

public:
    AlignmentIntersectionInfo (): m_extensionType(ExtensionType::None), m_extendedAmount(0.0) { }
    AlignmentIntersectionInfo (const DPoint3d& pt, const DVec3d& secondaryTangent, const ExtensionType& extensionType, const double& primaryStation, const double& secondaryStation, const double& extendedAmount, const double& zdiff)
        : m_point (pt), m_secondaryTangentAtIntersect(secondaryTangent), m_extensionType(extensionType), m_primaryStation (primaryStation), m_secondaryStation (secondaryStation), m_extendedAmount(extendedAmount), m_Zdifferential (zdiff)
        { }

    CIVILBASEGEOMETRY_EXPORT DPoint3d Point () { return m_point; }
    CIVILBASEGEOMETRY_EXPORT DVec3d SecondaryTangentAtIntersect() { return m_secondaryTangentAtIntersect; }
    CIVILBASEGEOMETRY_EXPORT ExtensionType ExtensionType () { return m_extensionType; }
    CIVILBASEGEOMETRY_EXPORT double PrimaryStation () { return m_primaryStation; }
    CIVILBASEGEOMETRY_EXPORT double SecondaryStation () { return m_secondaryStation; }
    CIVILBASEGEOMETRY_EXPORT double ExtendedAmount() { return m_extendedAmount; }
}; // AlignmentIntersectionInfo

//
// This class was developed as the public face of alignment intersection computation
// and should supercede any public apis for AlignmentPairIntersection
//
// We found we needed much more explicit logic for potential intersection, extended intersections, etc.
struct AlignmentIntersection : AlignmentPairIntersection
{
private:
    bool m_computeIntersectionsAtAlignmentEndPoints;

    bool m_computeStations;
    bool m_computeZs;
    bool m_extendSecondaryStart;
    bool m_extendSecondaryEnd;
    bool m_extendPrimaryStart;
    bool m_extendPrimaryEnd;

protected:
    bool _ComputeStations () { return m_computeStations; }
    bool _ComputeZDeltas () { return m_computeZs; }
    bool _ExtendSecondaryStart () { return m_extendSecondaryStart; }
    bool _ExtendSecondaryEnd () { return m_extendSecondaryEnd; }
    bool _ExtendPrimaryStart () { return m_extendPrimaryStart; }
    bool _ExtendPrimaryEnd () { return m_extendPrimaryEnd; }

    size_t _ComputeExplicitIntersections (bvector<AlignmentIntersectionInfo>& potentialIntersections);
    size_t _ComputeExtendedIntersections (bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections);
    size_t _ComputePrimaryEndProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections);
    size_t _ComputeSecondaryEndProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections);
    size_t _ComputePrimaryStartProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections);
    size_t _ComputeSecondaryStartProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections);

    bool ComputeClosestImpl(DPoint3dCR referencePt, const double* maxDistance, AlignmentIntersectionInfoR info);

protected:
    AlignmentIntersection (AlignmentPairP primary, AlignmentPairP secondary)
        : AlignmentPairIntersection (primary, secondary), m_computeStations (true), m_computeZs (false),
        m_extendPrimaryEnd (false), m_extendPrimaryStart (false), m_extendSecondaryEnd (false), m_extendSecondaryStart (false)
        {
        m_computeIntersectionsAtAlignmentEndPoints = true;
        }

public:
    // add computed z values to info
    CIVILBASEGEOMETRY_EXPORT void WantZDifferences (const bool& bVal) { m_computeZs = bVal; }
    // add computed station values to info
    CIVILBASEGEOMETRY_EXPORT void WantStations (const bool& bVal) { m_computeStations = bVal; }
    // allow primary start to be extended
    CIVILBASEGEOMETRY_EXPORT void SetAllowExtendPrimaryStart (const bool& bVal) { m_extendPrimaryStart = bVal; }
    // allow primary end to be extended
    CIVILBASEGEOMETRY_EXPORT void SetAllowExtendPrimaryEnd (const bool& bVal) { m_extendPrimaryEnd = bVal; }
    // allow secondary start to be extended
    CIVILBASEGEOMETRY_EXPORT void SetAllowExtendSecondaryStart (const bool& bVal) { m_extendSecondaryStart = bVal; }
    // allow secondary end to be extended
    CIVILBASEGEOMETRY_EXPORT void SetAllowExtendSecondaryEnd (const bool& bVal) { m_extendSecondaryEnd = bVal; }

    // returns all information related to the two alignments intersecting in the XY plane
    // the integer returned is the number of intersection points found
    CIVILBASEGEOMETRY_EXPORT size_t Compute (bvector<AlignmentIntersectionInfo>& potentialIntersections);

    CIVILBASEGEOMETRY_EXPORT bool ComputeClosest (DPoint3dCR referencePt, AlignmentIntersectionInfoR info);
    CIVILBASEGEOMETRY_EXPORT bool ComputeClosest(DPoint3dCR referencePt, const double maxDistance, AlignmentIntersectionInfoR info);
    // return the intersection that is at the closest station along the given alignment (primary or secondary)
    CIVILBASEGEOMETRY_EXPORT bool ComputeClosestToStation(double referenceStation, bool alongPrimary, AlignmentIntersectionInfoR info);

public:
    CIVILBASEGEOMETRY_EXPORT static AlignmentIntersectionPtr Create (AlignmentPairCP primary, AlignmentPairCP secondary);
}; // AlignmentIntersection


END_BENTLEY_CIVILGEOMETRY_NAMESPACE
