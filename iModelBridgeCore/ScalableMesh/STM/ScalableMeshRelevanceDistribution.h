//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshRelevanceDistribution.h $
//:>    $RCSfile: ScalableMeshRelevanceDistribution.h,v $
//:>   $Revision: 1.4 $
//:>       $Date: 2010/08/19 13:45:40 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DCoord.h>

template<class DATATYPE = double> class ScalableMeshRelevanceDistribution
    {
    typedef DATATYPE DataType_T;
    public:
        
        // Primary methods
        ScalableMeshRelevanceDistribution(ULong32   pi_nbBins, 
                                   DataType_T pi_minRelevance, 
                                   DataType_T pi_maxRelevance) 
            {  
            m_pBins        = new uint64_t[pi_nbBins];
            memset(m_pBins.get(), 0, sizeof(uint64_t) * pi_nbBins);

            m_nbRelevanceValues = 0;
            m_nbBins            = pi_nbBins;
            m_minRelevance      = pi_minRelevance;                                
            m_step              = (pi_maxRelevance - m_minRelevance) / pi_nbBins;
            }

        virtual ~ScalableMeshRelevanceDistribution() {}

        void    AddRelevance(DATATYPE pi_relevance) 
            {
            ULong32 ind = static_cast<ULong32>(floor((pi_relevance - m_minRelevance) / m_step));                                                                                               
            if (ind >= m_nbBins)
                {
                ind = m_nbBins - 1;
                }

            m_pBins[ind]++; 
            m_nbRelevanceValues++;
            }

        BENTLEYSTM_EXPORT DataType_T   GetRelevanceMean() const;

        BENTLEYSTM_EXPORT DataType_T   GetRelevanceStdDev() const;


        DataType_T   GetRelevancePercentileValue(double pi_percentage);

        double     GetRelevancePercentile(DATATYPE pi_relevanceValue, 
                                          bool     pi_inPercent);

        DataType_T   GetMinimumRelevance() const {return m_minRelevance;}

        DataType_T   GetMaximumRelevance() const {return m_minRelevance + m_nbBins * m_step;}
                                        
    private:      

        HArrayAutoPtr<uint64_t> m_pBins;
        ULong32               m_nbBins;
        DataType_T            m_minRelevance;        
        DataType_T            m_step;
        uint64_t                m_nbRelevanceValues;
    };