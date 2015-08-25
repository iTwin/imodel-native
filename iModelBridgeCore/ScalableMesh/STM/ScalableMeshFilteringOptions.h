/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshFilteringOptions.h $
|    $RCSfile: ScalableMeshFilteringOptions.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:59:47 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshFilteringOptions.h                         (C) Copyright 2009.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
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
#include "ScalableMeshCoreDefs.h"

/*----------------------------------------------------------------------------+
|Class ScalableMeshFilteringOptionsBase
|
| Each different filtering method should have its own filtering option class.
+----------------------------------------------------------------------------*/
class ScalableMeshFilteringOptionsBase
    {
    public:     
        
        BENTLEYSTM_EXPORT ScalableMeshFilteringOptionsBase(){}

        BENTLEYSTM_EXPORT virtual ~ScalableMeshFilteringOptionsBase(){}
    };

/*----------------------------------------------------------------------------+
|Class ScalableMeshGlobalFilteringOptions
|
| New filtering options for the global filtering.
+----------------------------------------------------------------------------*/
class ScalableMeshGlobalFilteringOptions : public ScalableMeshFilteringOptionsBase
    {
    public:     
                      
        BENTLEYSTM_EXPORT ScalableMeshGlobalFilteringOptions(RelevanceEvaluationMethod pi_relevanceEvalMethod, 
                                                uint8_t                     pi_localImportance, 
                                                bool                      pi_filterBoundaryPoints, 
                                                int                       pi_nbBins, 
                                                int                       pi_relevanceMinValue, 
                                                int                       pi_relevanceMaxValue);                        

        BENTLEYSTM_EXPORT virtual ~ScalableMeshGlobalFilteringOptions();

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