/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/RoadAlignmentPairEditor.h>

//! Divided alignments must NOT be tangential at their start/end points.
//! We cheat by adding a small segment at the beginning and end of their geometry with a small angle
#define DIVIDED_ALIGNMENT_EDITOR_SMALL_ANGLE 0.1 //! in degrees
#define DIVIDED_ALIGNMENT_EDITOR_SMALL_LENGTH 0.01 //! in m

#if 0 //&&AG WIP REFACTOR AlignmentPairEditor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditor::RoadAlignmentPairEditor (CurveVectorCR vertical, bool inXY)
    {
    DPoint3d start, end;
    vertical.GetStartEnd (start, end);
    CurveVectorPtr hz = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ICurvePrimitivePtr prim = ICurvePrimitive::CreateLine (DSegment3d::From (0.0, 0.0, 0.0, end.x, 0.0, 0.0));
    hz->Add (prim); // fake hz
    CurveVectorPtr vt = vertical.Clone ();
    if (inXY == true)
        {
        DVec3d u = DVec3d::From (1, 0, 0), v = DVec3d::From (0, 0, 1), w = DVec3d::From (0, 1, 0);
        DPoint3d origin {0.0, 0.0, 0.0};
        Transform flipAxes = Transform::FromOriginAndVectors (origin, u, v, w);
        vt->TransformInPlace (flipAxes);
        }
    UpdateCurveVectors(*hz, vt.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditor::RoadAlignmentPairEditor (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment) :
    T_Super (horizontalAlignment, verticalAlignment)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditorPtr RoadAlignmentPairEditor::Create (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment)
    {
    return new RoadAlignmentPairEditor (horizontalAlignment, verticalAlignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditorPtr RoadAlignmentPairEditor::CreateFromSingleCurveVector (CurveVectorCR curveVector)
    {
    RoadAlignmentPairEditorPtr roadAlignment = new RoadAlignmentPairEditor ();
    if (roadAlignment->_GenerateFromSingleVector (curveVector))
        return roadAlignment;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditorPtr RoadAlignmentPairEditor::Create (AlignmentPairCR pair)
    {
    return new RoadAlignmentPairEditor(pair.GetHorizontalCurveVector(), pair.GetVerticalCurveVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RoadAlignmentPairEditorPtr RoadAlignmentPairEditor::CreateVerticalOnly (CurveVectorCR verticalAlignment, bool inXY)
    {
    return new RoadAlignmentPairEditor (verticalAlignment, inXY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoadAlignmentPairEditor::RebuildRampApproachRoadAtStart (bvector<CurveVectorPtr> drapedPart)
    {
    if (drapedPart.size () < 1 || drapedPart.at (0).IsNull ())
        return false;
    DPoint3d start, end;
    CurveVectorPtr drape = drapedPart.at (0);
    drape->GetStartEnd (start, end);
    return InsertStartIntersection (*drape, fabs(end.x-start.x) * 1.5, 0.02);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoadAlignmentPairEditor::RebuildRampApproachRoadAtEnd (bvector<CurveVectorPtr> drapedPart)
    {
    if (drapedPart.size () < 1 || drapedPart.at (0).IsNull ())
        return false;
    DPoint3d start, end;
    CurveVectorPtr drape = drapedPart.at (0);
    drape->GetStartEnd (start, end);
    return InsertEndIntersection (*drape, fabs(end.x - start.x) * 1.5, 0.02);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr RoadAlignmentPairEditor::BuildRampApproachRoadAtStartWithAuxiliary (AlignmentPairR primaryRoad, CurveVectorR newHZ)
    {
    if (nullptr == GetVerticalCurveVector())
        {
        return AlignmentPair::Create(newHZ, nullptr);
        }

    DPoint3d testPt, testPtOnPrimary;
    testPt = GetPointAtWithZ (3.0);
    double testStationOnPrimary = primaryRoad.HorizontalDistanceAlongFromStart (testPt);
    testPtOnPrimary = primaryRoad.GetPointAtWithZ (testStationOnPrimary);

    AlignmentPairPtr newRoad = AlignmentPair::Create (newHZ, nullptr);
    double newLength = newRoad->LengthXY ();
    double oldLength = LengthXY ();
    // get the first primitive, should be an arc
    if (newHZ.size () <= 0) return nullptr;
    ICurvePrimitivePtr arcPrimitive = newHZ.at (0);
    if (arcPrimitive.IsNull ()) return nullptr;

    DPoint3d arcStart, arcEnd;
    arcPrimitive->GetStartEnd (arcStart, arcEnd);

    DPoint3d arcStartOnPrimary, arcEndOnSecondary;
    double arcStartStationOnPrimary = primaryRoad.HorizontalDistanceAlongFromStart(arcStart);
    arcStartOnPrimary = primaryRoad.GetPointAtWithZ (arcStartStationOnPrimary);
    double arcEndStationOnSecondary = HorizontalDistanceAlongFromStart(arcEnd);
    arcEndOnSecondary = GetPointAtWithZ (arcEndStationOnSecondary);
    arcEndStationOnSecondary = newRoad->HorizontalDistanceAlongFromStart(arcEnd);
    // compute starting z
    double startZ = arcStartOnPrimary.z - (arcStartOnPrimary.DistanceXY(arcStart)) * (( testPtOnPrimary.z - testPt.z ) / testPtOnPrimary.DistanceXY (testPt));

    AlignmentPVI startPVI (DPoint3d::From (0.0, 0.0, startZ), 0.0);
    AlignmentPVI nextPVI (DPoint3d::From (arcEndStationOnSecondary, 0.0, arcEndOnSecondary.z), 21.336);

    double delta = newLength - oldLength;
    bvector<AlignmentPVI> pvis = _GetPVIs ();
    bvector<AlignmentPVI> newPvis;
    newPvis.push_back (startPVI);
    newPvis.push_back (nextPVI);

    bvector<AlignmentPVI> tempPvis;
    for (size_t i = 0; i < pvis.size (); i++)
        {
        AlignmentPVI addpvi (DPoint3d::From (0.0, 0.0, 0.0), 0.0);
        addpvi.length = pvis[i].length;
        for (size_t j = 0; j < 3; j++)
            {
            addpvi.poles[j].x = pvis[i].poles[j].x + delta;
            addpvi.poles[j].z = pvis[i].poles[j].z;
            }
        tempPvis.push_back (addpvi);

        }
    for (size_t i = 0; i < tempPvis.size (); i++)
        {
        if (i == tempPvis.size () - 1)
            tempPvis.at (i).poles[PVI].x = newLength;
        if (tempPvis[i].poles[PVI].x > nextPVI.poles[PVI].x + 3.0 &&
            tempPvis[i].poles[PVC].x > nextPVI.poles[PVI].x + 3.0)
            newPvis.push_back (tempPvis[i]);
        }
    if (newPvis.size () > 4)
        _SolvePVI (newPvis[2], nextPVI, newPvis[3]);
    _SolvePVI (newPvis[1], startPVI, newPvis[2]);

    CurveVectorPtr newCurve = _BuildVectorFromPVIS (newPvis);
    if (newCurve.IsValid ())
        {
        return AlignmentPair::Create (*newHZ.Clone().get(), newCurve.get ());
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr RoadAlignmentPairEditor::BuildRampApproachRoadAtEndWithAuxiliary (AlignmentPairR primaryRoad, CurveVectorR newHZ)
    {
    if (nullptr == GetVerticalCurveVector())
        {
        return AlignmentPair::Create(newHZ, nullptr);
        }

    DPoint3d testPt, testPtOnPrimary;
    testPt = GetPointAtWithZ (3.0);
    double testStationOnPrimary = primaryRoad.HorizontalDistanceAlongFromStart(testPt);
    testPtOnPrimary = primaryRoad.GetPointAtWithZ (testStationOnPrimary);

    AlignmentPairPtr newRoad = AlignmentPair::Create (newHZ, nullptr);
    double newLength = newRoad->LengthXY ();
    // get the last primitive, should be an arc
    if (newHZ.size () <= 0) return nullptr;
    ICurvePrimitivePtr arcPrimitive = newHZ.at (newHZ.size()-1);
    if (arcPrimitive.IsNull ()) return nullptr;
    DPoint3d prevStart, prevEnd;
    newHZ.at (newHZ.size () - 2)->GetStartEnd (prevStart, prevEnd);

    DPoint3d arcStart, arcEnd;
    arcPrimitive->GetStartEnd (arcStart, arcEnd);
    if (!arcStart.AlmostEqualXY (prevEnd))
        {
        DPoint3d tpt = arcStart;
        arcStart = arcEnd;
        arcEnd = tpt;
        }

    DPoint3d arcEndOnPrimary, arcStartOnSecondary;
    double arcEndStationOnPrimary = primaryRoad.HorizontalDistanceAlongFromStart(arcEnd);
    arcEndOnPrimary = primaryRoad.GetPointAtWithZ (arcEndStationOnPrimary);
    double arcStartStationOnSecondary = HorizontalDistanceAlongFromStart(arcStart);
    arcStartOnSecondary = GetPointAtWithZ (arcStartStationOnSecondary);
//    arcStartStationOnSecondary = newRoad->HorizontalDistanceAlongFromStart (arcStart);

    // compute ending z
    double endZ = arcEndOnPrimary.z - ( arcEndOnPrimary.DistanceXY (arcEnd) ) * ( ( testPtOnPrimary.z - testPt.z ) / testPtOnPrimary.DistanceXY (testPt) );

    AlignmentPVI endPVI (DPoint3d::From (newLength, 0.0, endZ), 0.0);
    AlignmentPVI prevPVI (DPoint3d::From (arcStartStationOnSecondary, 0.0, arcStartOnSecondary.z), 21.336);

    bvector<AlignmentPVI> pvis = _GetPVIs ();
    bvector<AlignmentPVI> newPvis;

    for (size_t i = 0; i < pvis.size (); i++)
        {
        if (pvis[i].poles[PVI].x < prevPVI.poles[PVI].x - 3.0 &&
            pvis[i].poles[PVT].x < prevPVI.poles[PVI].x - 3.0)
            newPvis.push_back (pvis[i]);
        }
    if (newPvis.size () > 1)
        {
        size_t index = newPvis.size () - 1;
        _SolvePVI (newPvis[index], newPvis[index - 1], prevPVI);
        }
    _SolvePVI (prevPVI, newPvis[newPvis.size () - 1], endPVI);
    newPvis.push_back (prevPVI);
    newPvis.push_back (endPVI);

    CurveVectorPtr newCurve = _BuildVectorFromPVIS (newPvis);
    if (newCurve.IsValid ())
        {
        return AlignmentPair::Create(*newHZ.Clone ().get (), newCurve.get ());
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr RoadAlignmentPairEditor::BuildRampApproachRoadAtStart (AlignmentPairR primaryRoad, CurveVectorR newHZ)
    {
    if (nullptr == GetVerticalCurveVector())
        {
        return AlignmentPair::Create (newHZ, nullptr);
        }

    StationRangeEdit rangeEdit = ComputeHorizontalEditRange (newHZ);
    CurveVectorPtr newVert = ModifyVerticalRange (rangeEdit); // expand if necessary
    if (newVert.IsValid ())
        return AlignmentPair::Create (*newHZ.Clone ().get (), newVert.get ());
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr RoadAlignmentPairEditor::BuildRampApproachRoadAtEnd (AlignmentPairR primaryRoad, CurveVectorR newHZ)
    {
    if (nullptr == GetVerticalCurveVector())
        {
        return AlignmentPair::Create (newHZ, nullptr);
        }

    StationRangeEdit rangeEdit = ComputeHorizontalEditRange (newHZ);
    CurveVectorPtr newVert = ModifyVerticalRange (rangeEdit); // expand if necessary
    if (newVert.IsValid ())
        return AlignmentPair::Create(*newHZ.Clone ().get (), newVert.get ());

    return nullptr;
    }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Divided editor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DividedRoadAlignmentPairEditorPtr DividedRoadAlignmentPairEditor::Create (AlignmentPairEditorP primaryRoad)
    {
    return new DividedRoadAlignmentPairEditor (primaryRoad);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DividedRoadAlignmentPairEditor::DividedRoadAlignmentPairEditor (AlignmentPairEditorP primaryRoad)
    {
    m_primaryRoad = primaryRoad;
    m_smallAngle = DIVIDED_ALIGNMENT_EDITOR_SMALL_ANGLE;
    m_smallDistance = DIVIDED_ALIGNMENT_EDITOR_SMALL_LENGTH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DividedRoadAlignmentPairEditor::InitializeSettings (double fromSta, double toSta)
    {
    m_leftAlignment = nullptr;
    m_rightAlignment = nullptr;

    if (fromSta == toSta) return false;
    m_endSplitStation = toSta;
    m_startSplitStation = fromSta;
    if (fromSta > toSta)
        {
        m_endSplitStation = fromSta;
        m_startSplitStation = toSta;
        }

    if (m_startSplitStation < 0.0 || m_endSplitStation > m_primaryRoad->LengthXY ())
        return false;

    m_hasStartSplit = true;
    if (DoubleOps::AlmostEqual(m_startSplitStation, 0.0))
        m_hasStartSplit = false;

    m_hasEndSplit = true;
    if (DoubleOps::AlmostEqual(m_endSplitStation, m_primaryRoad->LengthXY ()))
        m_hasEndSplit = false;

    m_transitionLength = 50.0; // max at 50 meters
    if (m_endSplitStation - m_startSplitStation < 150.0)
        m_transitionLength = ( m_endSplitStation - m_startSplitStation ) / 3.0;

    m_offsetStartStation = m_startSplitStation;
    if (m_hasStartSplit)
        m_offsetStartStation = m_startSplitStation + m_transitionLength;

    m_offsetEndStation = m_endSplitStation;
    if (m_hasEndSplit)
        m_offsetEndStation = m_endSplitStation - m_transitionLength;

    m_primaryRoad->GetPointAndTangentAt (m_startSplitPt, m_startSplitVec, m_startSplitStation);
    m_primaryRoad->GetPointAndTangentAt (m_endSplitPt, m_endSplitVec, m_endSplitStation);
    m_endSplitVec.Negate ();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr DividedRoadAlignmentPairEditor::AddStartDoubleCurve(CurveVectorP offsetCurve, bool isLeftCurve)
    {
    if (offsetCurve == nullptr)
        return nullptr;
    CurveVectorPtr origCurve = offsetCurve->Clone ();

    DPoint3d start, end;
    offsetCurve->GetStartEnd (start, end);
    DVec3d startTan;
    m_primaryRoad->GetPointAndTangentAt (end, startTan, m_offsetStartStation); // tangent will be same on offset
    startTan.Negate ();

    //! Creates a small segment before the double fillet so we're not tangent at the beginning of the alignment
    DVec3d newStartSplitVec = m_startSplitVec;
    newStartSplitVec.RotateXY(isLeftCurve? m_smallAngle : -m_smallAngle);
    DPoint3d newStartSplitPt = DPoint3d::FromSumOf(m_startSplitPt, newStartSplitVec, m_smallDistance);
    ICurvePrimitivePtr smallLine = ICurvePrimitive::CreateLine(DSegment3d::From(m_startSplitPt, newStartSplitPt));

    double tangentLength = 0.25 * m_transitionLength; 
    CurveVectorPtr doubleFillet = AlignmentPairIntersection::ConstructDoubleFillet(newStartSplitPt, newStartSplitVec, tangentLength, start, startTan, tangentLength);

    // add double fillet to offsetCurve
    CurveVectorPtr returnVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    returnVector->push_back(smallLine);
    for (auto dprim : *doubleFillet)
        returnVector->Add (dprim);
    for (auto prim : *origCurve)
        returnVector->Add (prim);

    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr DividedRoadAlignmentPairEditor::AddEndDoubleCurve (CurveVectorP offsetCurve, bool isLeftCurve)
    {
    if (offsetCurve == nullptr)
        return nullptr;
    CurveVectorPtr origCurve = offsetCurve->Clone ();

    DPoint3d start, end;
    offsetCurve->GetStartEnd (start, end);
    DVec3d endTan;
    m_primaryRoad->GetPointAndTangentAt (start, endTan, m_offsetEndStation); // tangent will be same on offset

    //! Creates a small segment after the double fillet so we're not tangent at the end of the alignment
    DVec3d newEndSplitVec = m_endSplitVec;
    newEndSplitVec.RotateXY(isLeftCurve? -m_smallAngle : m_smallAngle);
    DPoint3d newEndSplitPt = DPoint3d::FromSumOf(m_endSplitPt, newEndSplitVec, m_smallDistance);
    ICurvePrimitivePtr smallLine = ICurvePrimitive::CreateLine(DSegment3d::From(newEndSplitPt, m_endSplitPt));

    double tangentLength = 0.25 * m_transitionLength;
    CurveVectorPtr doubleFillet = AlignmentPairIntersection::ConstructDoubleFillet (end, endTan, tangentLength, newEndSplitPt, newEndSplitVec, tangentLength);
    // add double fillet to end off original
    CurveVectorPtr returnVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for (auto prim : *origCurve)
        returnVector->Add (prim);
    for (auto dprim : *doubleFillet)
        returnVector->Add (dprim);
    returnVector->push_back(smallLine);

    return returnVector;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Alexandre.Gagnon                        08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr DividedRoadAlignmentPairEditor::CreateVerticalAlignment(CurveVectorCR hzAlignment)
    {
    // so now we have the left and right horizontals, need to work out the verticals
    AlignmentPairPtr partialPrimary = m_primaryRoad->GetPartialAlignment(m_startSplitStation, m_endSplitStation);
    AlignmentPairEditorPtr verticalModifier = RoadAlignmentPairEditor::Create(*partialPrimary);
    AlignmentPairPtr tempOff = AlignmentPair::Create(hzAlignment, nullptr);

    StationRangeEdit editRange;
    editRange.preEditRange.startStation = 0.0;
    editRange.preEditRange.endStation = m_endSplitStation - m_startSplitStation;
    editRange.postEditRange.startStation = 0.0;
    editRange.postEditRange.endStation = tempOff->LengthXY();

    return verticalModifier->ModifyVerticalRange(editRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016

       /------------------\
=======                    =============
       \------------------/

+---------------+---------------+---------------+---------------+---------------+------*/
bool DividedRoadAlignmentPairEditor::SplitAlignment(double fromSta, double toSta, double leftOffset, double rightOffset)
    {
    if (!InitializeSettings (fromSta, toSta))
        return false;
    
    // create the offset alignments at 1/2 the spread
    CurveVectorPtr partialHz = m_primaryRoad->GetPartialHorizontalAlignment (m_offsetStartStation, m_offsetEndStation);
    CurveVectorPtr rightHz = GeometryHelper::CloneOffsetCurvesXYNoBSpline(partialHz, rightOffset);
    CurveVectorPtr leftHz = GeometryHelper::CloneOffsetCurvesXYNoBSpline(partialHz, leftOffset);

    // need to use "construct double fillet"
    if (m_hasStartSplit)
        {
        rightHz = AddStartDoubleCurve (rightHz.get(), false);
        leftHz = AddStartDoubleCurve(leftHz.get(), true);
        }
    if (m_hasEndSplit)
        {
        rightHz = AddEndDoubleCurve (rightHz.get(), false);
        leftHz = AddEndDoubleCurve (leftHz.get (), true);
        }

    if (rightHz.IsValid())
        {
        CurveVectorPtr rightVt = CreateVerticalAlignment(*rightHz);
        m_rightAlignment = RoadAlignmentPairEditor::Create(*rightHz, rightVt.get());
        }
    if (leftHz.IsValid())
        {
        CurveVectorPtr leftVt = CreateVerticalAlignment(*leftHz);
        m_leftAlignment = RoadAlignmentPairEditor::Create(*leftHz, leftVt.get());
        }

    return (m_leftAlignment.IsValid() && m_rightAlignment.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr DividedRoadAlignmentPairEditor::RightSplitAlignment ()
    {
    return m_rightAlignment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairEditorPtr DividedRoadAlignmentPairEditor::LeftSplitAlignment ()
    {
    return m_leftAlignment;
    }

#endif
