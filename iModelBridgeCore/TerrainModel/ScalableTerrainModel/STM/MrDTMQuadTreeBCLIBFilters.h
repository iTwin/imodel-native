//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/MrDTMQuadTreeBCLIBFilters.h $
//:>    $RCSfile: MrDTMQuadTreeBCLIBFilters.h,v $
//:>   $Revision: 1.19 $
//:>       $Date: 2010/12/15 18:23:19 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include "HGF3DFilterCoord.h"


#include <ImagePP/all/h/HGFPointIndex.h>
#include "MrDTMRelevanceDistribution.h"



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
template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBFilterViewDependent : public IHGFPointIndexFilter<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBFilterViewDependent() {};
        virtual             ~MrDTMQuadTreeBCLIBFilterViewDependent() {};


        virtual bool        GlobalPreFilter (HGFPointIndex<POINT, EXTENT>& index)
        {
            // Save the index
            m_index = &index;
            return true;
        };


        virtual bool        FilterLeaf (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;

        virtual bool        IsProgressiveFilter() const;
        
        //By default do nothing        
        virtual bool        ComputeObjectRelevance (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode) {return false;}

    protected:        
        HGFPointIndex<POINT, EXTENT>*  m_index;        
};


/** -----------------------------------------------------------------------------
               
    This class implements a simple filter that promotes 1 point out of every four
    points in the sub-nodes. It applies a random algorithnm in the selection of the
    points.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class MrDTMQuadTreeFilterRandom : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>                   
{                                                                                                                                                       

    public:

        // Primary methods
                            MrDTMQuadTreeFilterRandom() {};
        virtual             ~MrDTMQuadTreeFilterRandom() {};


        // IHGFPointFilter implementation

        virtual bool        IsProgressiveFilter() const;

        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
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
template<class POINT, class EXTENT> class MrDTMQuadTreeFilterNonRandom : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>                   
{                                                                                                                                                       

    public:

        // Primary methods
                            MrDTMQuadTreeFilterNonRandom() {};
        virtual             ~MrDTMQuadTreeFilterNonRandom() {};


        // IHGFPointFilter implementation

        virtual bool        IsProgressiveFilter() const;

        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

};



template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBFilter1 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBFilter1() {};
        virtual             ~MrDTMQuadTreeBCLIBFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

};

template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBProgressiveFilter1 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBProgressiveFilter1() {};
        virtual             ~MrDTMQuadTreeBCLIBProgressiveFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        IsProgressiveFilter() const;



};


template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBFilter2 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBFilter2() {};
        virtual             ~MrDTMQuadTreeBCLIBFilter2() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;



};

template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBProgressiveFilter2 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBProgressiveFilter2() {};
        virtual             ~MrDTMQuadTreeBCLIBProgressiveFilter2() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;


        virtual bool        IsProgressiveFilter() const;



};


template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBFilter3 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBFilter3() {};
        virtual             ~MrDTMQuadTreeBCLIBFilter3() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;
};

template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBProgressiveFilter3 : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods
                            MrDTMQuadTreeBCLIBProgressiveFilter3() {};
        virtual             ~MrDTMQuadTreeBCLIBProgressiveFilter3() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        IsProgressiveFilter() const;
};

template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBProgressiveGlobalFilter : public MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>
{

    public:

        // Primary methods        
                
                            //MST : There are not much as default values, so the constructor 
                            //      should be removed if this filtering approach is kept.
                            MrDTMQuadTreeBCLIBProgressiveGlobalFilter()
                            {

                            }
                            

                            MrDTMQuadTreeBCLIBProgressiveGlobalFilter(RelevanceEvaluationMethod pi_relevanceEvaluationMethod, 
                                                                      UInt8                     pi_localImportance, 
                                                                      bool                      pi_filterBoundaryPoints, 
                                                                      int                       pi_resolutionLevelToFilter = -1) 
                            {
                                HPRECONDITION((pi_localImportance <= 100) && (pi_localImportance >= 0));                                
                                m_filterBoundaryPoints      = pi_filterBoundaryPoints;
                                m_localImportance           = pi_localImportance / 100.0;  
                                m_relevanceEvaluationMethod = pi_relevanceEvaluationMethod;                                
                                m_resolutionLevelToFilter   = pi_resolutionLevelToFilter;
                            }

        virtual             ~MrDTMQuadTreeBCLIBProgressiveGlobalFilter();

        // IMrDTMPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        FilterLeaf (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;


        virtual bool        GlobalPreFilter (HGFPointIndex<POINT, EXTENT>& index)
        {
            //bool result(index.ComputeObjectRelevance());      

            MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::GlobalPreFilter(index);
                        
            index.ComputeObjectRelevance();

            //assert(result == true);
                        
            return true;
        };


        virtual bool        ComputeObjectRelevance (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode);
       
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
              
        void FilterChildNode(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                             HFCPtr<HGFPointIndexNode<POINT, EXTENT> > childNode,
                             double                                    pi_percentileValue) const;

        //Probably need to template this class for the relevance type.        
        double                                      m_localImportance;
        vector<MrDTMRelevanceDistribution<double>*> m_pRelevanceDistributions;
        RelevanceEvaluationMethod                   m_relevanceEvaluationMethod;
        int                                         m_resolutionLevelToFilter;
        bool                                        m_filterBoundaryPoints;        
};

/*
template<class POINT, class EXTENT> class MrDTMQuadTreeBCLIBMultipassProgressiveGlobalFilter : public MrDTMQuadTreeBCLIBProgressiveGlobalFilter<POINT, EXTENT> 
{

    public:

        // Primary methods        
        
                            MrDTMQuadTreeBCLIBMultipassProgressiveGlobalFilter(RelevanceEvaluationMethod pi_relevanceEvaluationMethod = RELEVANCE_EVAL_ELEV_DIFF_TIN) 
                            {
                                m_relevanceEvaluationMethod = pi_relevanceEvaluationMethod;
                            }
        virtual             ~MrDTMQuadTreeBCLIBMultipassProgressiveGlobalFilter() {}

        // IMrDTMPointFilter implementation
        virtual bool        Filter (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const;

        virtual bool        FilterLeaf (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode, 
                                        double viewParameters[]) const;

        virtual bool        ComputeObjectRelevance (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > outputNode, 
                                                    double                                    localToGlobalImportanceRatio);
       
        virtual bool        IsProgressiveFilter() const;

        virtual void        InitRelevanceDistribution(ULong32 pi_nbBins, 
                                                      double pi_minRelevance, 
                                                      double pi_maxRelevance);

    private:

        void FilterChildNode(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                             HFCPtr<HGFPointIndexNode<POINT, EXTENT> > childNode,
                             double                                      pi_percentileValue) const;

        //Probably need to template this class for the relevance type.        
        double                                       m_localImportance;
        HAutoPtr<MrDTMRelevanceDistribution<double>> m_pRelevanceDistributions;
        RelevanceEvaluationMethod                    m_relevanceEvaluationMethod;

};
*/


#include "MrDTMQuadTreeBCLIBFilters.hpp"
