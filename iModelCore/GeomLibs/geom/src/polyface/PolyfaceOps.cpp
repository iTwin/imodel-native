/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Compute a replacement mesh if indicted by the numeric tags.
PolyfaceHeaderPtr  PolyfaceQuery::ComputeAlternatePolyface
(
PolyfaceQueryCR source, 
IFacetOptionsR options
)
    {
    double optionsTolerance = options.GetChordTolerance ();
    auto data = source.GetNumericTagsCP();
    if (data == nullptr || data->IsZero ())
        return nullptr;
    if (data->m_tagA == TaggedNumericData::TagType::SubdivisionSurface)
        {
        auto fixedDepth = data->TagToInt (TaggedNumericData::SubdivisionControlCode::FixedDepth, 1, 4, 0);
        auto absTol = data->TagToDouble(TaggedNumericData::SubdivisionControlCode::AbsoluteTolerance, 0.0, 10.0, 0.0);
        auto rangeRelTol = data->TagToDouble(TaggedNumericData::SubdivisionControlCode::FractionOfRangeBoxTolerance, 0.0, 0.25, 0.0);
        // BoundaryAction default is 0 : hold boundary.
        auto boundaryAction = data->TagToInt (TaggedNumericData::SubdivisionControlCode::BoundaryAction, 0, 1, 0);
        if (absTol <= 0.0 && rangeRelTol <= 0.0)
            {
            if (optionsTolerance <= 0.0)
                {
                absTol = 0.0;
                rangeRelTol = 0.002;
                }
            else
                {
                absTol = optionsTolerance;
                }
            }
        DRange3d range = source.PointRange ();
        double diagonal = range.low.Distance (range.high);
        double tolerance = DoubleOps::ComputeTolerance(diagonal, absTol, rangeRelTol);
        // If nothing came out of that make it a fixed depth ..
        if (tolerance <= 0.0 && fixedDepth <= 0)
            fixedDepth = 3;
        return PolyfaceHeader::CloneSubdivided(source, data->m_tagB, fixedDepth, tolerance, boundaryAction);
        }
    return nullptr;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
