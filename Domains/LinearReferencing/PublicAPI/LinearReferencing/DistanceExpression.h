/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/DistanceExpression.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

typedef Nullable<double> NullableDouble;

struct DistanceExpression
{
private:
    double m_distanceAlong;
    NullableDouble m_lateralOffset, m_verticalOffset, m_distanceAlongFromReferent;

public:
    DistanceExpression();
    DistanceExpression(double distanceAlong, NullableDouble lateralOffset, NullableDouble verticalOffset, NullableDouble distanceAlongFromReferent);

    LINEARREFERENCING_EXPORT double GetDistanceAlongFromStart() const { return m_distanceAlong; }
    LINEARREFERENCING_EXPORT NullableDouble GetLateralOffsetFromILinearElement() const { return m_lateralOffset; }
    LINEARREFERENCING_EXPORT NullableDouble GetVerticalOffsetFromILinearElement() const { return m_verticalOffset; }
    LINEARREFERENCING_EXPORT NullableDouble GetDistanceAlongFromReferent() const { return m_distanceAlongFromReferent; }
}; // DistanceExpression

END_BENTLEY_LINEARREFERENCING_NAMESPACE