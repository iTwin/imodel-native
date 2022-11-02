/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

bool TaggedNumericData::AreSameStructureAndGeometry(TaggedNumericData const *dataA, TaggedNumericData const *dataB, double tolerance)
    {
    bool isNullA = dataA == nullptr || dataA->IsZero ();
    bool isNullB = dataB == nullptr || dataB->IsZero ();
    if (isNullA && isNullB)
        return true;
    if (isNullA || isNullB)
        return false;
    // both are non-null ....
    return AreSameStructureAndGeometry (*dataA, *dataB, tolerance);
    }

bool TaggedNumericData::AreSameStructureAndGeometry(TaggedNumericData const &dataA, TaggedNumericData const &dataB, double tolerance)
    {
    // both are nonnull . . .
    if (dataA.m_tagA != dataB.m_tagA)
        return false;
    if (dataA.m_tagB != dataB.m_tagB)
        return false;
    // compare all sizes before venturing into full array comparisons . ..
    if (dataA.m_intData.size () != dataB.m_intData.size ())
        return false;
    if (dataA.m_doubleData.size () != dataB.m_doubleData.size ())
        return false;

    for (size_t i = 0, n = dataA.m_intData.size(); i < n; i++)
        if (dataA.m_intData[i] != dataB.m_intData[i])
            return false;

    for (size_t i = 0, n = dataA.m_doubleData.size(); i < n; i++)
        if (!DoubleOps::AlmostEqual (dataA.m_doubleData[i], dataB.m_doubleData[i]))
            return false;

    return true;
    }

bool TaggedNumericData::IsZero() const
    {
    return m_tagA == 0
        && m_tagB == 0
        && m_intData.size () == 0
        && m_doubleData.size () == 0;
    }
void TaggedNumericData::PushInts(int value0, int value1)
    {
    m_intData.push_back (value0);
    m_intData.push_back(value1);
    }

void TaggedNumericData::PushIndexedDouble(int value0, double doubleValue)
    {
    size_t index = m_doubleData.size ();
    m_doubleData.push_back(doubleValue);
    PushInts (value0, (int)index);
    }

// Search PAIRED ints for tag and value.
int32_t TaggedNumericData::TagToInt(int32_t targetTag, int32_t minValue, int32_t maxValue, int32_t defaultValue) const
    {
    // m_intData has pairs (index,value)
    for (uint32_t i0 = 0; i0 + 1 < m_intData.size(); i0 += 2)
        {
        if (m_intData[i0] == targetTag)
            {
            int32_t value = m_intData[i0 + 1];
            if (value >= maxValue)
                return maxValue;
            if (value <= minValue)
                return minValue;
            else
                return value;
            }
        }
    return defaultValue;
    }
// Search PAIRED ints for tag and value.
double TaggedNumericData::TagToDouble(int32_t targetTag, double minValue, double maxValue, double defaultValue) const
    {
    // m_intData has pairs (index,value)
    for (uint32_t i0 = 0; i0 + 1 < m_intData.size(); i0 += 2)
        {
        if (m_intData[i0] == targetTag)
            {
            int32_t index = m_intData[i0 + 1];
            if (index < 0 || index >= m_doubleData.size())
                return defaultValue;
            double value = m_doubleData[index];
            if (value >= maxValue)
                return maxValue;
            if (value <= minValue)
                return minValue;
            else
                return value;
            }
        }
    return defaultValue;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
