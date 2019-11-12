/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_GROUND_DETECTION_NAMESPACE

struct DiscreetHistogram;
typedef RefCountedPtr<DiscreetHistogram> DiscreetHistogramPtr;

/*=================================================================================**//**
* @bsiclass                                                     Mathieu.Marchand 05/2009
+===============+===============+===============+===============+===============+======*/
struct DiscreetHistogram : public RefCountedBase
{
public:
    static DiscreetHistogramPtr Create(double minimum, double maximum, size_t entryCount);

    virtual ~DiscreetHistogram() {}

    void Reset()
        {
        memset(m_entryFrenquencies, 0, sizeof(uint64_t)*m_entryCount);
        }

    size_t  GetEntryCount() const {return m_entryCount;}

    uint64_t  GetCount(size_t index) const
        {
        assert(index < m_entryCount);
        return m_entryFrenquencies[index];
        }

    void    SetCount(size_t index, uint64_t count)
        {
        assert(index < m_entryCount);
        m_entryFrenquencies[index] = count;
        }

    double  GetValueAt(size_t index) const {return m_minimum + (m_step*index);}

    double  GetValueOfNThEntry(uint64_t nThEntry) const
        {
        uint64_t count = 0;
        for (size_t i = 0; i < m_entryCount; ++i)
            {
            count += m_entryFrenquencies[i];
            if (count>=nThEntry)
                return GetValueAt(i);
            }
        return GetValueAt(m_entryCount-1);
        }

    void    IncrementCount(size_t index)
        {
        assert(index < m_entryCount);
        ++(m_entryFrenquencies[index]);
        }

    void    IncrementCountFromValue(double value)
        {
        // Ignore values that are not within the specified range.
        if(value < m_minimum || value > m_maximum)
            return;

        IncrementCount((size_t) ((value - m_minimum)*m_preMulStep));
        }

    uint64_t   GetCountFromValue(double value) const
        {
        // Ignore values that are not within the specified range.
        if(value < m_minimum || value > m_maximum)
            return 0;

        size_t index(static_cast<size_t>((value - m_minimum)*m_preMulStep));
        return m_entryFrenquencies[index];
        }

    double  FindMinValue() const
        {
        for(size_t i=0; i < m_entryCount; ++i)
            {
            if(m_entryFrenquencies[i] != 0)
                return GetValueAt(i);
            }

        return m_minimum;
        }

    double  FindMaxValue() const
        {
        for(size_t i=m_entryCount; m_entryCount; --i)
            {
            if(m_entryFrenquencies[i-1] != 0)
                return GetValueAt(i-1);
            }

        return m_maximum;
        }

    uint64_t  ComputeTotalCount() const
        {
        uint64_t count = 0;
        for(size_t i=0; i < m_entryCount; ++i)
            {
            count += m_entryFrenquencies[i];
            }
        return count;
        }

    void    ConvertTo(double factor)
        {
        m_minimum *= factor;
        m_maximum *= factor;
        m_step    *= factor;
        m_preMulStep *= factor;
        }

    double ComputeMedian() const
        {
        uint64_t TotalCount(ComputeTotalCount());
        double medianValue(0.0);
        if ((TotalCount % 2) == 0) //we have a even number of entries
            {
            uint64_t nThEntry(TotalCount/ 2);
            uint64_t nThEntry2(nThEntry + 1);
            medianValue = (GetValueOfNThEntry(nThEntry) + GetValueOfNThEntry(nThEntry2))/2.0;
            }
        else //we have a odd number of entries
            {
            uint64_t nThEntry((TotalCount + 1)/2);
            medianValue = GetValueOfNThEntry(nThEntry);
            }
        return medianValue;
        }
    //Nearest rank method (https://en.wikipedia.org/wiki/Percentile)
    //* must be [0..100]
    double ComputePercentile(double P) const   
        {
        if (P > 100)
            P = 100;
        if (P < 0)
            P = 0;
        uint64_t N(ComputeTotalCount());
        uint64_t nThEntry((uint64_t)round(P/100.0 * N));
        double persentileValue = GetValueOfNThEntry(nThEntry);
        return persentileValue;
        }
    StatusInt MergeWith(DiscreetHistogram inputHisto)
        {
        if (inputHisto.m_minimum != m_minimum ||
            inputHisto.m_maximum != m_maximum ||
            inputHisto.m_entryCount != m_entryCount)
            return ERROR;//Cannot be merge
        for (size_t i = 0; i < m_entryCount; ++i)
            {
            m_entryFrenquencies[i] += inputHisto.m_entryFrenquencies[i];
            }
        return SUCCESS;
        }

private:
    DiscreetHistogram(double minimum, double maximum, size_t entryCount)
        {
        if (entryCount<1)
            entryCount = 1;
        m_entryCount = entryCount;
        m_entryFrenquencies = new uint64_t[entryCount];
        memset(m_entryFrenquencies, 0, sizeof(uint64_t)*entryCount);
        m_minimum = minimum;
        m_maximum = maximum;
        if (entryCount > 1)
            {
            m_step = (m_maximum - m_minimum) / (entryCount - 1);     // First step is minimum.
            if (m_step>0.0)
                m_preMulStep = 1.0 / m_step;
            else
                m_preMulStep=0;
            }
        else
            {
            m_step = m_preMulStep = 0;
            }
        }

    size_t                  m_entryCount;           // Number of entry in m_entryFrenquencies;
    uint64_t*                 m_entryFrenquencies;
    double                  m_minimum;
    double                  m_maximum;
    double                  m_step;
    double                  m_preMulStep;       // Optimization
};

END_GROUND_DETECTION_NAMESPACE
