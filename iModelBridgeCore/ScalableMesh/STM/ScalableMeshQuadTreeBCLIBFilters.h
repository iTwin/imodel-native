//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.h $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.h,v $
//:>   $Revision: 1.19 $
//:>       $Date: 2010/12/15 18:23:19 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include "HGF3DFilterCoord.h"


#include "SMMeshIndex.h"
#include "ScalableMeshRelevanceDistribution.h"



#if (0)
template <> class SpatialOp<HGF3DFilterCoord<double,double>, HGF3DFilterCoord<double,double>, HGF2DTemplateExtent<double, HGF3DCoord<double> > >
{

    public:
    static  HGF2DTemplateExtent<double, HGF3DCoord<double> > GetExtent(const HGF3DFilterCoord<double,double> spatialObject)
    {
    return  HGF2DTemplateExtent<double, HGF3DCoord<double> >(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetX(), spatialObject.GetY());
    }

    static bool IsPointIn2D(const HGF3DFilterCoord<double,double> spatialObject, HGF3DFilterCoord<double,double> pi_rCoord)
    {
    return spatialObject.IsEqualTo2D (pi_rCoord);
    }

    static bool IsSpatialInExtent2D (const HGF3DFilterCoord<double,double>& spatial, const  HGF2DTemplateExtent<double, HGF3DCoord<double> >& extent)
    {
        return extent.IsPointIn (spatial);
    }

};
#endif

template <> class SpatialOp<HGF3DFilterCoord<double,double>, HGF3DFilterCoord<double,double>, HGF3DExtent<double> >
{

    public:
    static  HGF3DExtent<double> GetExtent(const HGF3DFilterCoord<double,double> spatialObject)
    {
    return  HGF3DExtent<double>(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ(), spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ());
    }

    static bool IsPointIn2D(const HGF3DFilterCoord<double,double> spatialObject, HGF3DFilterCoord<double,double> pi_rCoord)
    {
    return spatialObject.IsEqualTo2D (pi_rCoord);
    }

    static bool IsSpatialInExtent2D (const HGF3DFilterCoord<double,double>& spatial, const  HGF3DExtent<double>& extent)
    {
        return extent.IsPointIn2D (spatial);
    }

    static bool IsSpatialInExtent3D (const HGF3DFilterCoord<double,double>& spatial, const  HGF3DExtent<double>& extent)
    {
        return extent.IsPointIn (spatial);
    }

};






extern bool   GET_HIGHEST_RES;

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/





/** -----------------------------------------------------------------------------
               
    This class implements a default filter for spatial index of points. It takes
    one every four points out of every vectors.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBFilterViewDependent : public ISMPointIndexFilter<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBFilterViewDependent() {};
        virtual             ~ScalableMeshQuadTreeBCLIBFilterViewDependent() {};


        virtual bool        GlobalPreFilter (SMPointIndex<POINT, EXTENT>& index)
        {
            // Save the index
            m_index = &index;
            return true;
        };


        virtual bool        FilterLeaf (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;

        virtual bool        IsProgressiveFilter() const;
        
        //By default do nothing        
        virtual bool        ComputeObjectRelevance (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode) {return false;}

    protected:        
        SMPointIndex<POINT, EXTENT>*  m_index;        
};


/** -----------------------------------------------------------------------------
               
    This class implements a simple filter that promotes 1 point out of every four
    points in the sub-nodes. It applies a random algorithnm in the selection of the
    points.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeFilterRandom : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>                   
{                                                                                                                                                       

    public:

        // Primary methods
                            ScalableMeshQuadTreeFilterRandom() {};
        virtual             ~ScalableMeshQuadTreeFilterRandom() {};


        // IHGFPointFilter implementation

        virtual bool        IsProgressiveFilter() const;

        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

};

/** -----------------------------------------------------------------------------
               
    This class implements a simple filter that promotes 1 point out of every four
    points in the sub-nodes. It does not apply a random algorithnm in the selection of the
    points ... it simply takes the first point then the fifth, the ninth and so on.
    As a result if the data follows a clearly identifyable pattern the result filtered
    points can also exhibit some form of pattern.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeFilterNonRandom : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>                   
{                                                                                                                                                       

    public:

        // Primary methods
                            ScalableMeshQuadTreeFilterNonRandom() {};
        virtual             ~ScalableMeshQuadTreeFilterNonRandom() {};


        // IHGFPointFilter implementation

        virtual bool        IsProgressiveFilter() const;

        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

};



template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBFilter1 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBProgressiveFilter1 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBProgressiveFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBProgressiveFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        IsProgressiveFilter() const;



};


template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBFilter2 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBFilter2() {};
        virtual             ~ScalableMeshQuadTreeBCLIBFilter2() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;



};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBProgressiveFilter2 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBProgressiveFilter2() {};
        virtual             ~ScalableMeshQuadTreeBCLIBProgressiveFilter2() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;


        virtual bool        IsProgressiveFilter() const;



};


template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBFilter3 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBFilter3() {};
        virtual             ~ScalableMeshQuadTreeBCLIBFilter3() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;
};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBProgressiveFilter3 : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBProgressiveFilter3() {};
        virtual             ~ScalableMeshQuadTreeBCLIBProgressiveFilter3() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        IsProgressiveFilter() const;
};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter : public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods        
                
                            //MST : There are not much as default values, so the constructor 
                            //      should be removed if this filtering approach is kept.
                            ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter()
                            {

                            }
                            

                            ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter(RelevanceEvaluationMethod pi_relevanceEvaluationMethod, 
                                                                      uint8_t                     pi_localImportance, 
                                                                      bool                      pi_filterBoundaryPoints, 
                                                                      int                       pi_resolutionLevelToFilter = -1) 
                            {
                                HPRECONDITION((pi_localImportance <= 100) && (pi_localImportance >= 0));                                
                                m_filterBoundaryPoints      = pi_filterBoundaryPoints;
                                m_localImportance           = pi_localImportance / 100.0;  
                                m_relevanceEvaluationMethod = pi_relevanceEvaluationMethod;                                
                                m_resolutionLevelToFilter   = pi_resolutionLevelToFilter;
                            }

        virtual             ~ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter();

        // IScalableMeshPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        FilterLeaf (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;


        virtual bool        GlobalPreFilter (SMPointIndex<POINT, EXTENT>& index)
        {
            //bool result(index.ComputeObjectRelevance());      

            ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::GlobalPreFilter(index);
                        
            index.ComputeObjectRelevance();

            //assert(result == true);
                        
            return true;
        };


        virtual bool        ComputeObjectRelevance (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode);
       
        virtual bool        IsProgressiveFilter() const;

        virtual void        InitRelevanceDistribution(ULong32 pi_nbBins, 
                                                      double pi_minRelevance, 
                                                      double pi_maxRelevance, 
                                                      int    pi_nbPasses = 1);

        void SetResolutionLevelToFilter(int pi_resolutionLevelToFilter)
        {
            m_resolutionLevelToFilter = pi_resolutionLevelToFilter;
        }
            
    private:
              
        void FilterChildNode(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                             HFCPtr<SMPointIndexNode<POINT, EXTENT> > childNode,
                             double                                    pi_percentileValue) const;

        //Probably need to template this class for the relevance type.        
        double                                      m_localImportance;
        vector<ScalableMeshRelevanceDistribution<double>*> m_pRelevanceDistributions;
        RelevanceEvaluationMethod                   m_relevanceEvaluationMethod;
        int                                         m_resolutionLevelToFilter;
        bool                                        m_filterBoundaryPoints;        
};

/*
template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBMultipassProgressiveGlobalFilter : public ScalableMeshQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT> 
{

    public:

        // Primary methods        
        
                            ScalableMeshQuadTreeBCLIBMultipassProgressiveGlobalFilter(RelevanceEvaluationMethod pi_relevanceEvaluationMethod = RELEVANCE_EVAL_ELEV_DIFF_TIN) 
                            {
                                m_relevanceEvaluationMethod = pi_relevanceEvaluationMethod;
                            }
        virtual             ~ScalableMeshQuadTreeBCLIBMultipassProgressiveGlobalFilter() {}

        // IScalableMeshPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        FilterLeaf (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;

        virtual bool        ComputeObjectRelevance (HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode, 
                                                    double                                    localToGlobalImportanceRatio);
       
        virtual bool        IsProgressiveFilter() const;

        virtual void        InitRelevanceDistribution(ULong32 pi_nbBins, 
                                                      double pi_minRelevance, 
                                                      double pi_maxRelevance);

    private:

        void FilterChildNode(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                             HFCPtr<SMPointIndexNode<POINT, EXTENT> > childNode,
                             double                                      pi_percentileValue) const;

        //Probably need to template this class for the relevance type.        
        double                                       m_localImportance;
        HAutoPtr<ScalableMeshRelevanceDistribution<double>> m_pRelevanceDistributions;
        RelevanceEvaluationMethod                    m_relevanceEvaluationMethod;

};
*/


template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBMeshFilter1 : public ISMMeshIndexFilter<POINT, EXTENT>//public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBMeshFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBMeshFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;        
        
        virtual bool        IsProgressiveFilter() const { return false; }

        //By default do nothing        
        virtual bool        ComputeObjectRelevance(HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode) { return false; }

};

    template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIB_GarlandMeshFilter : public ISMMeshIndexFilter<POINT, EXTENT>//public ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIB_GarlandMeshFilter() {};
        virtual             ~ScalableMeshQuadTreeBCLIB_GarlandMeshFilter() {};

        // IHGFPointFilter implementation
        virtual bool        Filter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
                                   std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                   size_t numSubNodes,
                                   double viewDependentMetrics[]) const;        
        
        virtual bool        IsProgressiveFilter() const { return false; }

        //By default do nothing        
        virtual bool        ComputeObjectRelevance(HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode) { return false; }
};

#include "ScalableMeshQuadTreeBCLIBFilters.hpp"


