/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMFilteringOptions.h $
|    $RCSfile: MrDTMFilteringOptions.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:59:47 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMFilteringOptions.h    		  	       (C) Copyright 2009.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

/*----------------------------------------------------------------------+
| Exported classes and definition                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CONSTANT definitions                                                  |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/
#include "MrDTMCoreDefs.h"

/*----------------------------------------------------------------------------+
|Class MrDTMFilteringOptionsBase
|
| Each different filtering method should have its own filtering option class.
+----------------------------------------------------------------------------*/
class MrDTMFilteringOptionsBase
    {
    public:     
        
        BENTLEYSTM_EXPORT MrDTMFilteringOptionsBase(){}

        BENTLEYSTM_EXPORT virtual ~MrDTMFilteringOptionsBase(){}
    };

/*----------------------------------------------------------------------------+
|Class MrDTMGlobalFilteringOptions
|
| New filtering options for the global filtering.
+----------------------------------------------------------------------------*/
class MrDTMGlobalFilteringOptions : public MrDTMFilteringOptionsBase
    {
    public:     
                      
        BENTLEYSTM_EXPORT MrDTMGlobalFilteringOptions(RelevanceEvaluationMethod pi_relevanceEvalMethod, 
                                                uint8_t                     pi_localImportance, 
                                                bool                      pi_filterBoundaryPoints, 
                                                int                       pi_nbBins, 
                                                int                       pi_relevanceMinValue, 
                                                int                       pi_relevanceMaxValue);                        

        BENTLEYSTM_EXPORT virtual ~MrDTMGlobalFilteringOptions();

        RelevanceEvaluationMethod GetRelevanceEvaluationMethod() const;          

        //MST : Local importance value should be typedef.
        uint8_t                     GetLocalImportance() const;          

        bool                      GetFilterBoundaryPoints() const;          

        int                       GetNbBins() const;          

        void                      GetRelevanceValueLimits(int& po_rRelevanceMinValue, 
                                                          int& po_rRelevanceMaxValue) const;          
        
    private : 
        
        uint8_t                     m_localImportance;
        bool                      m_filterBoundaryPoints;
        int                       m_nbBins; 
        int                       m_relevanceMinValue;
        int                       m_relevanceMaxValue;
        RelevanceEvaluationMethod m_relevanceEvaluationMethod;
    };