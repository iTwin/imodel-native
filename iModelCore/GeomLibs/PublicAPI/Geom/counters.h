/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Bentley/BeTimeUtilities.h>

//! @file  counters.h utility classes for statistical data accumulation
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
struct SignCounter
{
double m_tolerance;
size_t numPositive;
size_t numNegative;
size_t numZero;
void Clear (){ numPositive = numNegative = numZero = 0;}

size_t NumPositive (){ return numPositive;}
size_t NumNegative (){ return numNegative;}
size_t NumZero (){ return numZero;}

SignCounter () : m_tolerance (0.0) {Clear ();}
SignCounter (double tolerance) : m_tolerance (fabs (tolerance)) {}
void Announce (double value)
    {
    if (value > m_tolerance)
        numPositive++;
    else if (-value > m_tolerance)
        numNegative++;
    else
        numZero++;
    }
};

//! Accumulate given values.  Return mean, standard deviation.
//! @ingroup BentleyGeom_Operations

struct UsageSums
{
double m_sums[3];
double m_min;
double m_max;

void ClearSums ()
    {
    m_sums[0] = m_sums[1] = m_sums[2] = 0;
    m_max = -DBL_MAX;
    m_min = DBL_MAX;
    }

UsageSums ()
    {
    ClearSums ();
    }

void Accumulate (double x)
    {
    m_sums[0] += 1.0;
    m_sums[1] += x;
    m_sums[2] += x * x;
    if (x > m_max)
        m_max = x;
    if (x < m_min)
        m_min = x;
    }

void AccumulateAbsBMinusA (bvector<double> const &dataA, bvector<double> const &dataB)
    {
    for (size_t i = 0; i < dataA.size () && i < dataB.size (); i++)
        {
        Accumulate (fabs (dataB[i] - dataA[i]));
        }
    }
void Accumulate (size_t x) {Accumulate ((double)x);}

void AccumulateWeighted (double x, double weight)
    {
    m_sums[0] += weight;
    m_sums[1] += weight * x;
    m_sums[2] += weight * x * x;
    if (x > m_max)
        m_max = x;
    if (x < m_min)
        m_min = x;
    }

void Accumulate (UsageSums const &other)
    {
    m_sums[0] += other.m_sums[0];
    m_sums[1] += other.m_sums[1];
    m_sums[2] += other.m_sums[2];
    if (other.m_max > m_max)
        m_max = other.m_max;
    if (other.m_min < m_min)
        m_min = other.m_min;
    }

double Min () const { return m_min;}
double Max () const { return m_max;}
double MaxAbs () const
    {
    double a = fabs (m_min);
    double b = fabs (m_max);
    return a > b ? a : b;
    }
double Sum () const {return m_sums[1];}
double Count () const {return m_sums[0];}
double Mean () const { return m_sums[0] > 0 ? m_sums[1] / m_sums[0] : 0.0;}
double MeanSquare () const { return m_sums[0] > 0 ? m_sums[2] / m_sums[0] : 0.0;}
double StandardDeviation () const
    {
    double a = Mean ();
    return sqrt (MeanSquare () - a * a);
    }
};

//! Manage UsageSums with begin+end signals;
struct TimeAccumulator : UsageSums
{
double m_startTime;
double GetTime (){return BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();}
void Reset (){m_startTime = GetTime ();}
void AccumulateAndReset (){double time = GetTime ();Accumulate (time - m_startTime);Reset ();}
void ClearAndReset (){ClearSums (); Reset ();}
};
//! Accumulate counts of true/false values passed to Count ()
//! @ingroup BentleyGeom_Operations

struct BoolCounter
{
private:
size_t m_true;
size_t m_false;
public:

BoolCounter () : m_true (0), m_false (0) {}

//! Clear all counters.
void Clear () {m_true = m_false = 0;}
//! Return the number of true hits.
size_t GetNumTrue (){ return m_true;}
//! Return the number of false hits.
size_t GetNumFalse (){ return m_false;}
//! Increment the counter selected by the value.  Return the value.
bool Count (bool value)
    {
    if (value)
      m_true++;
    else
      m_false++;
    return value;
    }
};

// Record counts of small integers that are announced over time.
// out of bounds data (both left and right) is accumulated as UsageSums.
struct SmallIntegerHistogram
{
private:
bvector<size_t> m_counter;
UsageSums m_leftData;
UsageSums m_rightData;
public:
// Constructor for histograms with buckets for values 0 to maxValue (inclusive).
SmallIntegerHistogram(uint32_t maxValue)
    : m_counter (maxValue+1, 0)
    {
    Clear ();
    }
size_t GetCount (ptrdiff_t value) const
    {
    if (value < 0)
        return (size_t)m_leftData.Count ();

    size_t index = (size_t) value;
    if (index >= m_counter.size ())
        return (size_t)m_rightData.Count ();
    return m_counter[index];
    }

UsageSums GetLeftData () const { return m_leftData;}
UsageSums GetRightData () const { return m_rightData;}
bool HasLeftOrRightData () const { return m_leftData.Count () > 0 || m_rightData.Count () > 0;}

// return the number of valid entries.
// return the low and high valid entry values
size_t GetValidCount (size_t &low, size_t &high) const
    {
    size_t i = 0;
    size_t m = m_counter.size ();
    for (; i < m && m_counter[i] == 0;) { i++;}
    low = high = i;
    size_t n = 0;
    for (; i < m; i++)
        {
        if (m_counter[i] != 0)
            {
            high = i;
            n += m_counter[i];
            }
        }
    return n;
    }
// return sum of all histogram buckets
UsageSums GetSums (bool includeLeft = false, bool includeRight = false) const
    {
    UsageSums sums;
    for (auto value : m_counter)
        sums.Accumulate (value);
    if (includeLeft)
        sums.Accumulate (m_leftData);
    if (includeRight)
        sums.Accumulate (m_rightData);
    return sums;
    }

void Clear ()
    {
    m_leftData.ClearSums ();
    m_rightData.ClearSums ();
    for (auto &n : m_counter)
        n = 0;
    }



void Record (size_t value)
    {
    if (value > m_counter.size ())
        m_rightData.Accumulate (value);
    else
        m_counter[value]++;
    }
// Equivalent to 
void Record (ptrdiff_t value, size_t count)
    {
    if (value < 0)
        m_leftData.AccumulateWeighted ((double)value, (double)count);
    else
        {
        size_t index = value;
        if (index > m_counter.size ())
            m_rightData.AccumulateWeighted ((double)index, (double)count);
        else
            m_counter[index] += count;
        }
    }

};
END_BENTLEY_GEOMETRY_NAMESPACE
