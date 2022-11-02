/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <PlacementOnEarth/Placement.h>

TEST(PlacementOnEarth, Placement3d)
    {
    DRange3d myRange = DRange3d::From (1,2,3,4,5,6);
    DVec3d diagonal = myRange.high - myRange.low;
    auto alignedRange = ElementAlignedBox3d (myRange);
    Check::ExactDouble (myRange.low.x, alignedRange.GetLeft ());
    Check::ExactDouble(myRange.low.y, alignedRange.GetBottom ());
    Check::ExactDouble(myRange.low.z, alignedRange.GetFront ());
    Check::ExactDouble(myRange.high.x, alignedRange.GetRight ());
    Check::ExactDouble(myRange.high.y, alignedRange.GetTop ());
    Check::ExactDouble(myRange.high.z, alignedRange.GetBack ());

    Check::Near (diagonal.x, alignedRange.GetWidth ());
    Check::Near(diagonal.y, alignedRange.GetHeight());
    Check::Near(diagonal.z, alignedRange.GetDepth());
    auto origin = DPoint3d::From (100, 200, 300);
    Placement3d placementA (origin,
        YawPitchRollAngles::FromDegrees (360.0,0,0),
        alignedRange
        );
    auto transformA = placementA.GetTransform ();
    Check::Near(Transform::From (origin), transformA);

    Json::Value jsonA;
    placementA.ToJson (BeJsValue(jsonA));
    Placement3d placementB;
    Check::False (placementB.IsValid ());
    placementB.FromJson (BeJsValue(jsonA));
    Check::True(placementB.IsValid());

    auto placementC = placementA;
    Check::True (placementC.IsValid ());
    Check::True (placementC.TryApplyTransform(Transform::From (1,1,0)));
    double bigNum = 4.0 * PlacementOnEarth::CircumferenceOfEarth ();
    DRange3d bigRange = DRange3d::From(
        0,0,0, bigNum, bigNum, bigNum);
    auto badBox = ElementAlignedBox3d(DRange3d::NullRange());
    // The ctor flips null range to infinite ... undo it
    std::swap (badBox.low, badBox.high);
    Placement3d placementZ1 (origin,
        YawPitchRollAngles::FromDegrees (1,2,3),
        badBox
        );

    Check::False(placementZ1.IsValid());

    Placement3d placementZ0 (origin,
        YawPitchRollAngles::FromDegrees(1, 2, 3),
        ElementAlignedBox3d(bigRange));
    Check::False(placementZ0.IsValid());

    DRange3d bigRange2 = DRange3d::From(
        bigNum, bigNum, bigNum, bigNum + 5, bigNum, bigNum);
    Placement3d placementZ2(DPoint3d::From (bigNum, 0, 0),
        YawPitchRollAngles::FromDegrees(1, 2, 3),
        ElementAlignedBox3d(bigRange2));
    Check::False(placementZ2.IsValid());
    }

TEST(PlacementOnEarth, Placement2d)
    {
    DRange2d myRange = DRange2d::From(1, 2, 4, 5);
    DVec2d diagonal = DVec2d::FromStartEnd (myRange.low, myRange.high);
    auto alignedRange = ElementAlignedBox2d(myRange.low.x, myRange.low.y, myRange.high.x, myRange.high.y);
    Check::ExactDouble(myRange.low.x, alignedRange.GetLeft());
    Check::ExactDouble(myRange.low.y, alignedRange.GetBottom());
    Check::ExactDouble(myRange.high.x, alignedRange.GetRight());
    Check::ExactDouble(myRange.high.y, alignedRange.GetTop());

    Check::Near(diagonal.x, alignedRange.GetWidth());
    Check::Near(diagonal.y, alignedRange.GetHeight());
    auto origin = DPoint2d::From (100,200);
    Placement2d placementA(origin,
        AngleInDegrees::FromDegrees(360.0),
        alignedRange
    );
    auto transformA = placementA.GetTransform();
    Check::Near(Transform::From(DPoint3d::From (origin)), transformA);

    Json::Value jsonA;
    placementA.ToJson(BeJsValue(jsonA));
    Placement2d placementB;
    Check::False(placementB.IsValid());
    placementB.FromJson(BeJsValue(jsonA));
    Check::True(placementB.IsValid());
    auto placementC = placementA;
    Check::True(placementC.IsValid());
    double bigNum = 4.0 * PlacementOnEarth::CircumferenceOfEarth();
    auto badBox = ElementAlignedBox2d(0,1,0,1);
    Placement2d placementZ1(origin,
        Angle::FromDegrees(20),
        badBox
    );
    auto & boxZ1 = placementZ1.GetElementBoxR();
    // The ctor flips to low..high ... undo it
    boxZ1.low.x = boxZ1.high.x + 1;
    Check::False(placementZ1.IsValid());

    Placement2d placementZ0(origin,
        Angle::FromDegrees(20),
        ElementAlignedBox2d(0, 0, bigNum, bigNum));
    double dx = placementZ0.GetElementBoxR().XLength ();
    Check::True (dx > PlacementOnEarth::CircumferenceOfEarth());
    bool q0 = placementZ0.IsValid ();
    Check::False(q0);

    Placement2d placementZ2(DPoint2d::From(bigNum, 0),
        Angle::FromDegrees(20),
        ElementAlignedBox2d(bigNum, bigNum, bigNum + 5, bigNum));
    Check::False(placementZ2.IsValid());

    }
