//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshRelevanceDistribution.cpp $
//:>    $RCSfile: ScalableMeshRelevanceDistribution.cpp,v $
//:>   $Revision: 1.6 $
//:>       $Date: 2010/11/29 13:15:52 $
//:>     $Author: Daryl.Holmwood $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/
//#include <toolsubs.h>

/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include ScalableMesh header files                                          |
+----------------------------------------------------------------------*/
#include "ScalableMeshRelevanceDistribution.h"

/**----------------------------------------------------------------------------
 Get relevance value for given percentile.
-----------------------------------------------------------------------------*/
template<class DATATYPE> DATATYPE ScalableMeshRelevanceDistribution<DATATYPE>::GetRelevancePercentileValue(double pi_percentage)
    {
    HPRECONDITION(pi_percentage <= 100.0);

    DATATYPE percentileValue = 0;     

    if (pi_percentage == 100)
        {
        //Maximum relevance value
        percentileValue = m_step * m_nbBins + m_minRelevance;
        }
    else
        {
        uint64_t nbValuesForGivenPercentile = static_cast<uint64_t>(m_nbRelevanceValues * pi_percentage / 100);
        uint64_t nbValuesTaken = 0;            

        for (uint32_t binInd = 0; (binInd < m_nbBins); binInd++)
            {                
            if (nbValuesTaken + m_pBins[binInd] > nbValuesForGivenPercentile)
                {
                percentileValue = m_minRelevance + m_step * binInd;

                //Since all the points in the bin are not considered, apply a correction factor 
                //considering that distribution for a bin is uniform.
                percentileValue += m_step / 2 * (nbValuesForGivenPercentile - nbValuesTaken) / m_pBins[binInd];
                            
                //percentileValue += (nbValuesForGivenPercentile - nbValuesTaken) * averageBinRelevance;
     
                break; 
                }

            //percentileValue += m_pBins[binInd] * (m_minRelevance + m_step * binInd + m_step / 2);        
            nbValuesTaken += m_pBins[binInd];
            }    
        }

    return percentileValue;
    }


/**----------------------------------------------------------------------------
 Get percentile value for given relevance.
-----------------------------------------------------------------------------*/
template<class DATATYPE> double ScalableMeshRelevanceDistribution<DATATYPE>::GetRelevancePercentile(DATATYPE pi_relevanceValue, 
                                                                                             bool     pi_inPercent)
    {        
    double   percentile;        

    double   binIndDecimal = fabs((pi_relevanceValue - m_minRelevance) / m_step);
    uint32_t   binIndForRelevance = static_cast<uint32_t>(floor(binIndDecimal));
    uint64_t  nbValuesTaken = 0;     

    assert(binIndDecimal <= m_nbBins);
    
    uint32_t binInd;

    for (binInd = 0; (binInd < binIndForRelevance); binInd++)
        {                
        nbValuesTaken += m_pBins[binInd];
        }    

    double binIndFraction = binIndDecimal - binInd;
    
    if (binIndFraction > 0)
        {
        //We are assuming that bin distribution is uniform.
        nbValuesTaken += static_cast<uint64_t>(m_pBins[binInd] * binIndFraction);
        }

    percentile = (double)nbValuesTaken / m_nbRelevanceValues;

    assert(percentile <= 1.0);

    if (pi_inPercent == true)
        {
        percentile *= 100;
        }
 
     
    return percentile;
    }

template<class DATATYPE> DATATYPE ScalableMeshRelevanceDistribution<DATATYPE>::GetRelevanceMean() const
    {
    DATATYPE mean = 0;
    DATATYPE binValue = m_minRelevance + m_step / 2;

    for (uint32_t binInd = 0; (binInd < m_nbBins); binInd++)
        {    
        mean += m_pBins[binInd] * binValue;

        binValue += m_step;        
        }    

    return mean / m_nbRelevanceValues;
    }

template<class DATATYPE> DATATYPE ScalableMeshRelevanceDistribution<DATATYPE>::GetRelevanceStdDev() const
    {
    DATATYPE mean = GetRelevanceMean();
    DATATYPE stdDev = 0;
    DATATYPE binValue = m_minRelevance + m_step / 2;

    for (uint32_t binInd = 0; (binInd < m_nbBins); binInd++)
        {     
        stdDev += m_pBins[binInd] * pow(binValue - mean, 2);        

        binValue += m_step;        
        }    

    return sqrt(stdDev / m_nbRelevanceValues);    
    }

template class ScalableMeshRelevanceDistribution<double>;
