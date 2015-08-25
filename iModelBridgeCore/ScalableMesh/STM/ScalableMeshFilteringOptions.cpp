/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshFilteringOptions.cpp $
|    $RCSfile: ScalableMeshFilteringOptions.cpp,v $
|   $Revision: 1.7 $
|       $Date: 2010/12/15 18:23:17 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "ScalableMeshCoreDefs.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshFilteringOptions.h"


/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/


/*----------------------------------------------------------------------------+
|                   Class ScalableMeshFilteringOptionsBase
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|                   Class ScalableMeshGlobalFilteringOptions
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshGlobalFilteringOptions::ScalableMeshGlobalFilteringOptions
+----------------------------------------------------------------------------*/
ScalableMeshGlobalFilteringOptions::ScalableMeshGlobalFilteringOptions(RelevanceEvaluationMethod pi_relevanceEvalMethod, 
                                                         uint8_t                     pi_localImportance, 
                                                         bool                      pi_filterBoundaryPoints, 
                                                         int                       pi_nbBins, 
                                                         int                       pi_relevanceMinValue, 
                                                         int                       pi_relevanceMaxValue)
: m_relevanceEvaluationMethod(pi_relevanceEvalMethod),   
  m_filterBoundaryPoints(pi_filterBoundaryPoints),
  m_nbBins(pi_nbBins),
  m_relevanceMinValue(pi_relevanceMinValue),
  m_relevanceMaxValue(pi_relevanceMaxValue), 
  m_localImportance(pi_localImportance)
    {        
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFilteringOptionsBase::ScalableMeshGlobalFilteringOptions
+----------------------------------------------------------------------------*/
ScalableMeshGlobalFilteringOptions::~ScalableMeshGlobalFilteringOptions()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFilteringOptionsBase::GetRelevanceEvaluation
+----------------------------------------------------------------------------*/
RelevanceEvaluationMethod ScalableMeshGlobalFilteringOptions::GetRelevanceEvaluationMethod() const
    {
    return m_relevanceEvaluationMethod;
    } 

/*----------------------------------------------------------------------------+
|ScalableMeshGlobalFilteringOptions::GetLocalImportance
+----------------------------------------------------------------------------*/
uint8_t ScalableMeshGlobalFilteringOptions::GetLocalImportance() const
    {
    return m_localImportance;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshGlobalFilteringOptions::GetFilterBoundaryPoints
+----------------------------------------------------------------------------*/
bool ScalableMeshGlobalFilteringOptions::GetFilterBoundaryPoints() const
    {
    return m_filterBoundaryPoints;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshGlobalFilteringOptions::GetNbBins
+----------------------------------------------------------------------------*/
int ScalableMeshGlobalFilteringOptions::GetNbBins() const
    {
    return m_nbBins;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshGlobalFilteringOptions::GetRelevanceValueLimits
+----------------------------------------------------------------------------*/
void ScalableMeshGlobalFilteringOptions::GetRelevanceValueLimits(int& po_rRelevanceMinValue, 
                                                          int& po_rRelevanceMaxValue) const
    {
    po_rRelevanceMinValue = m_relevanceMinValue;
    po_rRelevanceMaxValue = m_relevanceMaxValue;
    }    