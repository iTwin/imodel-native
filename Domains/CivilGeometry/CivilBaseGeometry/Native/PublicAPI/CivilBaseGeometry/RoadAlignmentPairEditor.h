/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "AlignmentPairEditor.h"
#include "AlignmentPairIntersection.h"
#include "GeometryHelper.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

#if 0 //&&AG WIP REFACTOR AlignmentPairEditor

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct RoadAlignmentPairEditor : AlignmentPairEditor
{
    DEFINE_T_SUPER(AlignmentPairEditor)

protected:
    ROADRAILALIGNMENT_EXPORT RoadAlignmentPairEditor(CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
    ROADRAILALIGNMENT_EXPORT RoadAlignmentPairEditor(CurveVectorCR vertical, bool inXY);
    ROADRAILALIGNMENT_EXPORT RoadAlignmentPairEditor() { }

public:
    // very high level call to initially set the ramp road in place
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr BuildRampApproachRoadAtStart(AlignmentPairR primaryRoad, CurveVectorR newHZ);
    // very high level call to initially set the ramp road in place
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr BuildRampApproachRoadAtEnd(AlignmentPairR primaryRoad, CurveVectorR newHZ);
    // very high level call to initially set the ramp road in place
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr BuildRampApproachRoadAtStartWithAuxiliary(AlignmentPairR primaryRoad, CurveVectorR newHZ);
    // very high level call to initially set the ramp road in place
    ROADRAILALIGNMENT_EXPORT virtual AlignmentPairPtr BuildRampApproachRoadAtEndWithAuxiliary(AlignmentPairR primaryRoad, CurveVectorR newHZ);
    // very high level call to specifically swap around and rebuild the road horizontal and vertical for ramp approach roads
    ROADRAILALIGNMENT_EXPORT virtual bool RebuildRampApproachRoadAtStart(bvector<CurveVectorPtr> drapedPart);
    // very high level call to specifically swap around and rebuild the road horizontal and vertical for ramp approach roads
    ROADRAILALIGNMENT_EXPORT virtual bool RebuildRampApproachRoadAtEnd(bvector<CurveVectorPtr> drapedPart);

    ROADRAILALIGNMENT_EXPORT static RoadAlignmentPairEditorPtr Create(CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment);
    ROADRAILALIGNMENT_EXPORT static RoadAlignmentPairEditorPtr Create(AlignmentPair const& roadAlignment);
    ROADRAILALIGNMENT_EXPORT static RoadAlignmentPairEditorPtr CreateVerticalOnly(CurveVectorCR verticalAlignment, bool inXY = false);

    // special alignment generator which uses a single curve vector to create
    // both a flattened horizontal and a vertical alignment from the z values
    ROADRAILALIGNMENT_EXPORT static RoadAlignmentPairEditorPtr CreateFromSingleCurveVector(CurveVectorCR curveVector);
}; // RoadAlignmentPairEditor


/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
struct DividedRoadAlignmentPairEditor : NonCopyableClass, RefCountedBase
{
    friend struct AlignmentPairEditor;

    AlignmentPairEditorPtr m_primaryRoad;
    double m_startSplitStation;
    double m_endSplitStation;

    double m_offsetStartStation;
    double m_offsetEndStation;

    DPoint3d m_rightStartPoint;
    DPoint3d m_leftStartPoint;
    DPoint3d m_rightEndPoint;
    DPoint3d m_leftEndPoint;

    DPoint3d m_startSplitPt;
    DPoint3d m_endSplitPt;
    DVec3d m_startSplitVec;
    DVec3d m_endSplitVec;

    bool m_hasStartSplit;
    bool m_hasEndSplit;

    double m_transitionLength;  // for double curve
    double m_smallAngle;        // for double curve. angle for a small segment we'll be putting at the start/end of the offsetted alignments
    double m_smallDistance;     // for double curve. segment length

    AlignmentPairEditorPtr m_rightAlignment;
    AlignmentPairEditorPtr m_leftAlignment;

protected:
    bool InitializeSettings (double fromSta, double toSta);
    CurveVectorPtr AddStartDoubleCurve (CurveVectorP offsetCurve, bool isLeftCurve);
    CurveVectorPtr AddEndDoubleCurve (CurveVectorP offsetCurve, bool isLeftCurve);
        
    //! Creates the vertical alignment given the original alignment and the offsetted curve
    CurveVectorPtr CreateVerticalAlignment(CurveVectorCR hzAlignment);

    DividedRoadAlignmentPairEditor (AlignmentPairEditorP primaryRoad);

public:
    //! Split alignment into multiple alignments, from station to station
    ROADRAILALIGNMENT_EXPORT bool SplitAlignment (double fromSta, double toSta, double leftOffset, double rightOffset);

    ROADRAILALIGNMENT_EXPORT AlignmentPairEditorPtr RightSplitAlignment ();
    ROADRAILALIGNMENT_EXPORT AlignmentPairEditorPtr LeftSplitAlignment ();

public:
    ROADRAILALIGNMENT_EXPORT static DividedRoadAlignmentPairEditorPtr Create (AlignmentPairEditorP primaryRoad);
}; // DividedRoadAlignmentPairEditor

#endif

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE