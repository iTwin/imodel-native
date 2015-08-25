//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.cpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.cpp,v $
//:>   $Revision: 1.13 $
//:>       $Date: 2011/06/27 14:53:05 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

   
#include <ScalableMeshPCH.h>


/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/
//#include <toolsubs.h>
/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"


/** -----------------------------------------------------------------------------

    PointOp<> class
    
    This template class provides a way to normalize the point class interface.
    Since the point index has the point type as template argument and all
    point classes have their own individiual interfaces, the spatial index
    makes use of the PointOp<> template class. The default implementation
    fits properly the HGF3DCoord<double> interface. Other point types need simply 
    overload the template class and code the appropriate operation.
    
    ----------------------------------------------------------------------------- 
*/
template <class POINT> class PointWithRelevanceOp
{
public:
    static double GetRelevance(const POINT& point) {return point.GetRelevance();}
    static void   SetRelevance(POINT& point, double relevance) {point.SetRelevance(relevance);}
};

template <> class PointWithRelevanceOp<IDTMFile::Point3d64fM64f>
{
public:
    static double GetRelevance(const IDTMFile::Point3d64fM64f& point) {return point.m;}
    static void   SetRelevance(IDTMFile::Point3d64fM64f& point, double relevance) {point.m = relevance;}
};


template<class POINT, class EXTENT> void ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::InitRelevanceDistribution(ULong32 pi_nbBins, 
                                                                         double pi_minRelevance, 
                                                                         double pi_maxRelevance, 
                                                                         int    pi_nbPasses)
    {          
    HPRECONDITION(pi_minRelevance < pi_maxRelevance);    

    for (int passInd = 0; passInd < pi_nbPasses; passInd++)
        {        
        m_pRelevanceDistributions.push_back(new ScalableMeshRelevanceDistribution<double>(pi_nbBins, 
                                                                                   pi_minRelevance, 
                                                                                   pi_maxRelevance));    
        }
    }
 

template<class POINT, class EXTENT> ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::~ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter()
    {
    for (size_t distInd = 0; distInd < m_pRelevanceDistributions.size(); distInd++)
        {        
        delete m_pRelevanceDistributions[distInd];
        }
    }

bool TestLocalFiltering = TRUE;

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::Filter(
                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                                            std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                                            size_t numSubNodes,
                                                            double viewDependentMetrics[]) const
    { 
    //HPRECONDITION(m_pRelevanceDistributions.size() == 1);
    HDEBUGCODE (size_t preconditionLevel;)
    HDEBUGCODE (preconditionLevel = (numSubNodes == 0 ? 0 : subNodes[0]->GetLevel());)
    HDEBUGCODE (for(size_t indexSubNodes = 0 ; indexSubNodes < numSubNodes ; indexSubNodes++) { HPRECONDITION (preconditionLevel == subNodes[indexSubNodes]->GetLevel());});

    if ((m_resolutionLevelToFilter != -1)  &&  
        (parentNode->GetLevel() != (m_resolutionLevelToFilter - 1)))
        {
        return true;
        }

    //int         status;
    
    //Could be compute during the creation of the index.
    //The level are increasing from the shallowest (i.e. 0) to the deepest.
    size_t deepestLevel = parentNode->GetDepth();          
    size_t level = subNodes[0]->GetLevel();
    size_t nbNodesAtLevel;
    size_t nbNodesAtAndBelowLevel = 0;
    size_t totalNbNodes = 0;
    

    //Compute total number of nodes
    for (size_t levelInd = 0; levelInd <= deepestLevel; levelInd++)
        {        
        nbNodesAtLevel = static_cast<size_t>(pow(4.0, int(levelInd)));

        if  (levelInd >= level)
            {   
            nbNodesAtAndBelowLevel += nbNodesAtLevel;
            }

        totalNbNodes += nbNodesAtLevel;
        }    

    double percentileForLevel = (double)nbNodesAtAndBelowLevel / totalNbNodes * 100;

    if (TestLocalFiltering == TRUE)
        {        
        percentileForLevel += (100 - percentileForLevel) * (m_localImportance);
        }

    double relevancePercentileVal;

    if (m_resolutionLevelToFilter == -1)
        {
        relevancePercentileVal = m_pRelevanceDistributions[0]->GetRelevancePercentileValue(percentileForLevel);
        }
    else
        {
        relevancePercentileVal = m_pRelevanceDistributions[m_resolutionLevelToFilter]->GetRelevancePercentileValue(percentileForLevel);
        }
    
    size_t nbPointsInParentNode = 0;

    for (size_t indexSubNodes = 0 ; indexSubNodes < numSubNodes ; indexSubNodes++)
        {
        nbPointsInParentNode += subNodes[indexSubNodes]->size();
        }   

    nbPointsInParentNode /= 4;

    assert(parentNode->size() == 0);

    parentNode->reserve(nbPointsInParentNode);
                      
    for (size_t indexSubNodes = 0 ; indexSubNodes < numSubNodes ; indexSubNodes++)
        {
        FilterChildNode(parentNode, subNodes[indexSubNodes], relevancePercentileVal); 
        subNodes[indexSubNodes]->m_nodeHeader.m_filtered = true;
        }

    size_t nbPointsInParent = parentNode->size();

    assert((int)nbPointsInParent > (int)nbPointsInParentNode - 10);
                                                                                                                                            
    return true;
    }


#include <vector>
// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h>
#include <iostream>
#include <algorithm>

using namespace std;
 
template<class POINT>
bool MyDataSortPredicate(const POINT& d1, const POINT& d2)
        {
        return d1.GetRelevance() > d2.GetRelevance();
        }
    

template<class POINT, class EXTENT> void ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::FilterChildNode(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                                                                                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > childNode,
                                                                                                                   double                                      pi_percentileValue) const
    {            
    vector<POINT> pointRemainingInChildNode;
    vector<POINT> convexHullPoints;    
    size_t        nbPointsInChildNode = childNode->size();

    nbPointsInChildNode = nbPointsInChildNode;

    size_t maxNbPointsToMoveUp = (size_t)(nbPointsInChildNode / 4.0 * (1 - m_localImportance));
    size_t nbPointsMoveUp      = 0;
                
    for (size_t ptInd = 0; ptInd < childNode->size(); ptInd++)
        {      
        //Convex hull point
        if (PointWithRelevanceOp<POINT>::GetRelevance(childNode->operator[](ptInd)) == DBL_MAX)
            {
            convexHullPoints.push_back(childNode->operator[](ptInd));
            }
        else
            {
            //The case when m_localImportance == 1 could be eventually optimized.
            if ((PointWithRelevanceOp<POINT>::GetRelevance(childNode->operator[](ptInd)) <= pi_percentileValue) || 
                (nbPointsMoveUp >= maxNbPointsToMoveUp) ||
                (m_localImportance == 1))             
                {
                pointRemainingInChildNode.push_back(childNode->operator[](ptInd));            
                }                              
            else
                {           
                //Call the fast pooled store directly to ensure that no split occurred.
                parentNode->push_back(childNode->operator[](ptInd));
                nbPointsMoveUp++;             
                }        
            }
        }

    //Do the local filtering 
    size_t nbPointsToMoveUp = 0; 

    if (TestLocalFiltering == true)
        {
      //  std::sort(pointRemainingInChildNode.begin(), pointRemainingInChildNode.end(), MyDataSortPredicate<POINT>);

        std::random_shuffle(pointRemainingInChildNode.begin(), pointRemainingInChildNode.end());
                             
        nbPointsToMoveUp = (size_t)min((nbPointsInChildNode / 4.0) - nbPointsMoveUp, 
                                       (double)pointRemainingInChildNode.size());
                   
        for (size_t ptInd = 0; ptInd < nbPointsToMoveUp; ptInd++)
            {                                   
            parentNode->push_back(pointRemainingInChildNode.operator[](ptInd));                                    
            }        
        }

    childNode->clear();    

    childNode->push_back(&pointRemainingInChildNode[nbPointsToMoveUp], pointRemainingInChildNode.size() - nbPointsToMoveUp);

    //Filter convex hull points
    size_t NbConvexHullPointsToMoveUp = 0; 

    if (convexHullPoints.size() > 0)
        {
        std::random_shuffle(convexHullPoints.begin(), convexHullPoints.end());
                     
        NbConvexHullPointsToMoveUp = (size_t)(convexHullPoints.size() / 1.5);
                   
        for (size_t ptInd = 0; ptInd < NbConvexHullPointsToMoveUp; ptInd++)
            {                                   
            parentNode->push_back(convexHullPoints.operator[](ptInd));                                    
            }        
        }          
        
    childNode->push_back(&convexHullPoints[NbConvexHullPointsToMoveUp], convexHullPoints.size() - NbConvexHullPointsToMoveUp) ; 

//     size_t childSize = childNode->size();
//     size_t parentSize = parentNode->size();
// 
//     parentSize = parentSize;


    //assert(childNode->size() == pointRemainingInChildNode.size() - NbPointsToMoveUp);   
    }

/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::FilterLeaf (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode, 
                                                                                                               double viewParameters[]) const
{       
    //Nothing to do here.
    return true;  
}

/**----------------------------------------------------------------------------
 Compute object relevance.
-----------------------------------------------------------------------------*/

#define NB_COLS 50
#define NB_ROWS 5
#define NUMBER_CLUSTERS (NB_COLS * NB_ROWS)

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::ComputeObjectRelevance(HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode) 
    {
    static bool        s_initialize = false;
    static double      s_clusterZMeans[NUMBER_CLUSTERS];
    static int         s_clusterPointNb[NUMBER_CLUSTERS];
    static vector<int> s_clusters[NUMBER_CLUSTERS];

    ULong32 levelInd = (ULong32)outputNode->GetLevel();

    if ((m_resolutionLevelToFilter != -1) && 
        (levelInd !=  m_resolutionLevelToFilter))
        {
        return true;
        }

    if (s_initialize == false)
        {
        for (int clusterInd = 0; clusterInd < NUMBER_CLUSTERS; clusterInd++)
            {                
            s_clusters[clusterInd].reserve(50000);
            }

        s_initialize = true;
        }

    //Use the second view depeendent metric as a switch 
    //to determine if the node contains edge points or not.
    outputNode->GetViewDependentMetrics()[1] = 0;

    memset(s_clusterZMeans,   0, sizeof(double) * NUMBER_CLUSTERS);
    memset(s_clusterPointNb, 0, sizeof(int) * NUMBER_CLUSTERS);

    int colInd; 
    int rowInd;
    
    EXTENT extent = outputNode->GetContentExtent();
          
    double binWidth = ExtentOp<EXTENT>::GetWidth(extent) / NB_COLS;
    double binHeight = ExtentOp<EXTENT>::GetHeight(extent) / NB_ROWS;
       
    for (int pointInd = 0; pointInd < (int)outputNode->size(); pointInd++)
        {
        POINT point = outputNode->operator[](pointInd);
                
        colInd = (int)((PointOp<POINT>::GetX(point) - ExtentOp<EXTENT>::GetXMin(extent)) / binWidth);
        colInd = max(0, min(colInd, NB_COLS - 1));
        assert((colInd >= 0) && (colInd < NB_COLS));
        
        rowInd = (int)((PointOp<POINT>::GetY(point) - ExtentOp<EXTENT>::GetYMin(extent)) / binHeight);
        rowInd = max(0, min(rowInd, NB_ROWS - 1));
        assert((rowInd >= 0) && (rowInd < NB_ROWS));
        
        int ind = colInd + (rowInd * NB_COLS);

        s_clusterZMeans[ind] += PointOp<POINT>::GetZ(point);        
        s_clusters[ind][s_clusterPointNb[ind]] = pointInd;                                    
        s_clusterPointNb[ind]++;
        }       

    for (int clusterInd = 0; clusterInd < NUMBER_CLUSTERS; clusterInd++)
        {   
        s_clusterZMeans[clusterInd] /= s_clusterPointNb[clusterInd];
        for (int pointInd = 0; pointInd < s_clusterPointNb[clusterInd]; pointInd++)
            {
            POINT point = outputNode->operator[](s_clusters[clusterInd][pointInd]);
            
            double relevance = fabs(PointOp<POINT>::GetZ(point) - s_clusterZMeans[clusterInd]);

            PointWithRelevanceOp<POINT>::SetRelevance(point, relevance);

            outputNode->modify(s_clusters[clusterInd][pointInd], point);

            if (m_resolutionLevelToFilter == -1)
                {
                m_pRelevanceDistributions[0]->AddRelevance(relevance);
                }
            else
                {
                m_pRelevanceDistributions[m_resolutionLevelToFilter]->AddRelevance(relevance);
                }
            }                
        }

    return true;
    }

/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}

//ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<>

// template class ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<HGF3DFilterCoord<double, double>, HGF2DTemplateExtent<double, HGF3DCoord<double>>>;
template class ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<HGF3DFilterCoord<double, double>, HGF3DExtent<double>>;

template class ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<IDTMFile::Point3d64fM64f, IDTMFile::Extent3d64f>;
