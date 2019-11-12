/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "MrDTMCoreDefs.h"
#include "MrDTMQuadTreeBCLIBFilters.h"
#include "MrDTMFilteringOptions.h"


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
|                   Class MrDTMFilteringOptionsBase
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|                   Class MrDTMGlobalFilteringOptions
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMGlobalFilteringOptions::MrDTMGlobalFilteringOptions
+----------------------------------------------------------------------------*/
MrDTMGlobalFilteringOptions::MrDTMGlobalFilteringOptions(RelevanceEvaluationMethod pi_relevanceEvalMethod, 
                                                         UInt8                     pi_localImportance, 
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
|MrDTMFilteringOptionsBase::MrDTMGlobalFilteringOptions
+----------------------------------------------------------------------------*/
MrDTMGlobalFilteringOptions::~MrDTMGlobalFilteringOptions()
    {
    }

/*----------------------------------------------------------------------------+
|MrDTMFilteringOptionsBase::GetRelevanceEvaluation
+----------------------------------------------------------------------------*/
RelevanceEvaluationMethod MrDTMGlobalFilteringOptions::GetRelevanceEvaluationMethod() const
    {
    return m_relevanceEvaluationMethod;
    } 

/*----------------------------------------------------------------------------+
|MrDTMGlobalFilteringOptions::GetLocalImportance
+----------------------------------------------------------------------------*/
UInt8 MrDTMGlobalFilteringOptions::GetLocalImportance() const
    {
    return m_localImportance;
    }

/*----------------------------------------------------------------------------+
|MrDTMGlobalFilteringOptions::GetFilterBoundaryPoints
+----------------------------------------------------------------------------*/
bool MrDTMGlobalFilteringOptions::GetFilterBoundaryPoints() const
    {
    return m_filterBoundaryPoints;
    }

/*----------------------------------------------------------------------------+
|MrDTMGlobalFilteringOptions::GetNbBins
+----------------------------------------------------------------------------*/
int MrDTMGlobalFilteringOptions::GetNbBins() const
    {
    return m_nbBins;
    }

/*----------------------------------------------------------------------------+
|MrDTMGlobalFilteringOptions::GetRelevanceValueLimits
+----------------------------------------------------------------------------*/
void MrDTMGlobalFilteringOptions::GetRelevanceValueLimits(int& po_rRelevanceMinValue, 
                                                          int& po_rRelevanceMaxValue) const
    {
    po_rRelevanceMinValue = m_relevanceMinValue;
    po_rRelevanceMaxValue = m_relevanceMaxValue;
    }    