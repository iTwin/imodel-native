/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestHeader.h"

BEGIN_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Constructors()
    {
    // Default constructor, invalid
    StationRange range;
    EXPECT_FALSE(range.IsValid());
    EXPECT_EQ_DOUBLE(DBL_MAX, range.startStation);
    EXPECT_EQ_DOUBLE(-DBL_MAX, range.endStation);

    // Constructor with start/end stations
    range = StationRange(0.0, -1.0);
    EXPECT_FALSE(range.IsValid());
    EXPECT_EQ_DOUBLE(0.0, range.startStation);
    EXPECT_EQ_DOUBLE(-1.0, range.endStation);

    range = StationRange(1.0, 1.0);
    EXPECT_TRUE(range.IsValid());
    EXPECT_EQ_DOUBLE(1.0, range.startStation);
    EXPECT_EQ_DOUBLE(1.0, range.endStation);
    
    range = StationRange(13.0, 14.0);
    EXPECT_TRUE(range.IsValid());

    // Copy constructor
    StationRange range2(range);
    EXPECT_EQ_DOUBLE(13.0, range2.startStation);
    EXPECT_EQ_DOUBLE(14.0, range2.endStation);
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Length()
    {
    // Invalid range yields an undefined length (infinity, NAN)?
    EXPECT_EQ(NAN, StationRange().Length());
    
    EXPECT_EQ_DOUBLE(1.0, StationRange(-3.0, -2.0).Length());
    EXPECT_EQ_DOUBLE(28.0, StationRange(-14.0, 14.0).Length());
    EXPECT_EQ_DOUBLE(0.0, StationRange(1.0, 1.0).Length());
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Center()
    {
    EXPECT_EQ(NAN, StationRange().Center());

    EXPECT_EQ_DOUBLE(0.0, StationRange(0.0, 0.0).Center());
    EXPECT_EQ_DOUBLE(-1.0, StationRange(-1.0, -1.0).Center());
    EXPECT_EQ_DOUBLE(-6.0, StationRange(-8.0, -4.0).Center());
    EXPECT_EQ_DOUBLE(10.0, StationRange(5.0, 15.0).Center());
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Contains()
    {
    StationRange range(-10.0, 10.0);
    EXPECT_TRUE(range.IsValid());

    // Check lower/upper bounds
    EXPECT_FALSE(range.ContainsExclusive(-10.0));
    EXPECT_FALSE(range.ContainsExclusive(10.0));
    EXPECT_TRUE(range.ContainsInclusive(-10.0));
    EXPECT_TRUE(range.ContainsInclusive(10.0));

    // Outside
    EXPECT_FALSE(range.ContainsExclusive(-11.0));
    EXPECT_FALSE(range.ContainsInclusive(-11.0));
    EXPECT_FALSE(range.ContainsExclusive(11.0));
    EXPECT_FALSE(range.ContainsInclusive(11.0));

    // Inside
    EXPECT_TRUE(range.ContainsExclusive(0.0));
    EXPECT_TRUE(range.ContainsInclusive(0.0));
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Extend()
    {
    // Extending an uninitialized range should set its stations to the value in parameter
    StationRange range;
    range.Extend(-10.0);
    EXPECT_EQ_DOUBLE(-10.0, range.startStation);
    EXPECT_EQ_DOUBLE(-10.0, range.endStation);

    range = StationRange();
    range.Extend(StationRange(-10.0, 12.0));
    EXPECT_EQ_DOUBLE(-10.0, range.startStation);
    EXPECT_EQ_DOUBLE(12.0, range.endStation);

    // Extend a valid range with another range
    range = StationRange(-10.0, 10.0);
    range.Extend(StationRange(-15.0, -14.0));
    EXPECT_EQ_DOUBLE(-15.0, range.startStation);
    EXPECT_EQ_DOUBLE(10.0, range.endStation);

    range = StationRange(-10.0, 10.0);
    range.Extend(StationRange(-5.0, -15.0));
    EXPECT_EQ_DOUBLE(-15.0, range.startStation);
    EXPECT_EQ_DOUBLE(10.0, range.endStation);

    range = StationRange(-10.0, 10.0);
    range.Extend(StationRange(15.0, 20.0));
    EXPECT_EQ_DOUBLE(-10.0, range.startStation);
    EXPECT_EQ_DOUBLE(20.0, range.endStation);
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_SignedDistance()
    {
    StationRange range;
    EXPECT_EQ(NAN, range.SignedDistance(0.0));

    range = StationRange(0.0, 0.0);
    EXPECT_EQ_DOUBLE(0.0, range.SignedDistance(0.0));
    EXPECT_EQ_DOUBLE(10.0, range.SignedDistance(10.0));
    EXPECT_EQ_DOUBLE(-10.0, range.SignedDistance(-10.0));

    range = StationRange(-10.0, 10.0);
    EXPECT_EQ_DOUBLE(0.0, range.SignedDistance(1.0));
    EXPECT_EQ_DOUBLE(-5.0, range.SignedDistance(-15.0));
    EXPECT_EQ_DOUBLE(5.0, range.SignedDistance(15.0));
    }
//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
void StationRange_Overlaps()
    {
    StationRange range;
    StationRangeOverlapDetail detail = range.Overlaps(range);
    EXPECT_EQ(StationRangeOverlap::InvalidRange, detail.GetOverlap());

    detail = range.Overlaps(StationRange(-10.0, 10.0));
    EXPECT_EQ(StationRangeOverlap::InvalidRange, detail.GetOverlap());

    range = StationRange(-10.0, 10.0);
    detail = range.Overlaps(StationRange());
    EXPECT_EQ(StationRangeOverlap::InvalidRange, detail.GetOverlap());

    // Equal Range
    // |---|
    // |---|
    detail = range.Overlaps(StationRange(-10.0, 10.0));
    EXPECT_EQ(StationRangeOverlap::EqualRange, detail.GetOverlap());
    EXPECT_TRUE(detail.IsEqualRange());
    EXPECT_TRUE(detail.IsEqualOrContainsInclusive());
    EXPECT_TRUE(detail.IsEqualOrEncapsulatedInclusive());

    // IsLeftOf
    // |---|
    //       |---|
    detail = range.Overlaps(StationRange(20.0, 30.0));
    EXPECT_EQ(StationRangeOverlap::IsLeftOf, detail.GetOverlap());
    EXPECT_TRUE(detail.IsNoOverlap());
    EXPECT_TRUE(detail.IsSinglePointOrNoOverlap());

    // IsLeftAdjacentOf
    // |---|
    //     |---|
    detail = range.Overlaps(StationRange(10.0, 15.0));
    EXPECT_EQ(StationRangeOverlap::IsLeftAdjacentOf, detail.GetOverlap());
    EXPECT_TRUE(detail.IsSinglePointOverlap());
    EXPECT_TRUE(detail.IsSinglePointOrNoOverlap());

    // IsRightOf
    //       |---|
    // |---|
    detail = range.Overlaps(StationRange(-30.0, -20.0));
    EXPECT_EQ(StationRangeOverlap::IsRightOf, detail.GetOverlap());
    EXPECT_TRUE(detail.IsNoOverlap());
    EXPECT_TRUE(detail.IsSinglePointOrNoOverlap());

    // IsRightAdjacentOf
    //     |---|
    // |---|
    detail = range.Overlaps(StationRange(-15.0, -10.0));
    EXPECT_EQ(StationRangeOverlap::IsRightAdjacentOf, detail.GetOverlap());
    EXPECT_TRUE(detail.IsSinglePointOverlap());
    EXPECT_TRUE(detail.IsSinglePointOrNoOverlap());

    // IsLeftOverlapOf
    // |---|
    //    |---|
    detail = range.Overlaps(StationRange(5.0, 15.0));
    EXPECT_EQ(StationRangeOverlap::IsLeftOverlapOf, detail.GetOverlap());
    EXPECT_FALSE(detail.IsNoOverlap());
    EXPECT_FALSE(detail.IsSinglePointOverlap());
    EXPECT_FALSE(detail.IsSinglePointOrNoOverlap());

    // IsRightOverlapOf
    //    |---|
    // |---|
    detail = range.Overlaps(StationRange(-15.0, -5.0));
    EXPECT_EQ(StationRangeOverlap::IsRightOverlapOf, detail.GetOverlap());
    EXPECT_FALSE(detail.IsNoOverlap());
    EXPECT_FALSE(detail.IsSinglePointOverlap());
    EXPECT_FALSE(detail.IsSinglePointOrNoOverlap());

    // IsLeftEncapsulatedIn
    // |---|
    // |-------|
    detail = range.Overlaps(StationRange(-10.0, 20.0));
    EXPECT_EQ(StationRangeOverlap::IsLeftEncapsulatedIn, detail.GetOverlap());
    EXPECT_FALSE(detail.IsEncapsulatedExclusive());
    EXPECT_TRUE(detail.IsEncapsulatedInclusive());
    EXPECT_TRUE(detail.IsEqualOrEncapsulatedInclusive());

    // IsRightEncapsulatedIn
    //     |---|
    // |-------|
    detail = range.Overlaps(StationRange(-20.0, 10.0));
    EXPECT_EQ(StationRangeOverlap::IsRightEncapsulatedIn, detail.GetOverlap());
    EXPECT_FALSE(detail.IsEncapsulatedExclusive());
    EXPECT_TRUE(detail.IsEncapsulatedInclusive());
    EXPECT_TRUE(detail.IsEqualOrEncapsulatedInclusive());

    // IsFullyEncapsulatedIn
    //   |---|
    // |-------|
    detail = range.Overlaps(StationRange(-20.0, 20.0));
    EXPECT_EQ(StationRangeOverlap::IsFullyEncapsulatedIn, detail.GetOverlap());
    EXPECT_TRUE(detail.IsEncapsulatedExclusive());
    EXPECT_TRUE(detail.IsEncapsulatedInclusive());
    EXPECT_TRUE(detail.IsEqualOrEncapsulatedInclusive());
    
    // ContainsExclusive
    // |-------|
    //   |---|
    detail = range.Overlaps(StationRange(-5.0, 5.0));
    EXPECT_EQ(StationRangeOverlap::ContainsExclusive, detail.GetOverlap());
    EXPECT_TRUE(detail.ContainsExclusive());
    EXPECT_TRUE(detail.ContainsInclusive());
    EXPECT_TRUE(detail.IsEqualOrContainsInclusive());

    // ContainsInclusive
    // |-------| or |-------| but not  |---|
    // |---|            |---|          |---|
    detail = range.Overlaps(StationRange(-10.0, 5.0));
    EXPECT_EQ(StationRangeOverlap::ContainsInclusive, detail.GetOverlap());
    EXPECT_FALSE(detail.ContainsExclusive());
    EXPECT_TRUE(detail.ContainsInclusive());
    EXPECT_TRUE(detail.IsEqualOrContainsInclusive());

    detail = range.Overlaps(StationRange(-5.0, 10.0));
    EXPECT_EQ(StationRangeOverlap::ContainsInclusive, detail.GetOverlap());
    EXPECT_FALSE(detail.ContainsExclusive());
    EXPECT_TRUE(detail.ContainsInclusive());
    EXPECT_TRUE(detail.IsEqualOrContainsInclusive());
    }
END_UNNAMED_NAMESPACE


//---------------------------------------------------------------------------------------
// @betest                              Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
TEST_F(CivilBaseGeometryTests, StationRangeTests)
    {
    StationRange_Constructors();
    StationRange_Length();
    StationRange_Center();
    StationRange_Contains();
    StationRange_Extend();
    StationRange_SignedDistance();
    StationRange_Overlaps();
    }
