//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.h $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.h,v $
//:>   $Revision: 1.19 $
//:>       $Date: 2010/12/15 18:23:19 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include "HGF3DFilterCoord.h"


#include "SMMeshIndex.h"
#include "ScalableMeshRelevanceDistribution.h"
#include <ScalableMesh/IScalableMeshSourceCreator.h>

extern bool   GET_HIGHEST_RES;

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBFilter1 : public ISMPointIndexFilter<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const;

};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBProgressiveFilter1 : public ISMPointIndexFilter<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBProgressiveFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBProgressiveFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const;

        virtual bool        IsProgressiveFilter() const;

};

template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIBMeshFilter1 : public ISMMeshIndexFilter<POINT, EXTENT>//public ISMPointIndexFilter<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMeshQuadTreeBCLIBMeshFilter1() {};
        virtual             ~ScalableMeshQuadTreeBCLIBMeshFilter1() {};

        // IHGFPointFilter implementation
        virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const;        
        
        virtual bool        IsProgressiveFilter() const { return false; }        
};

  
template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIB_CGALMeshFilter : public ISMMeshIndexFilter<POINT, EXTENT>
    {
    public:

        // Primary methods
        ScalableMeshQuadTreeBCLIB_CGALMeshFilter() {};
        virtual             ~ScalableMeshQuadTreeBCLIB_CGALMeshFilter() {};

        // IHGFPointFilter implementation
        virtual bool        Filter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
                                   std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                   size_t numSubNodes) const;       
        
        virtual bool        IsProgressiveFilter() const { return false; }        
    };

#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT> class ScalableMeshQuadTreeBCLIB_UserMeshFilter : public ISMMeshIndexFilter<POINT, EXTENT>
    {
    public:

        // Primary methods
        ScalableMeshQuadTreeBCLIB_UserMeshFilter() : m_callback(nullptr){};
        virtual             ~ScalableMeshQuadTreeBCLIB_UserMeshFilter() {};

        // IHGFPointFilter implementation
        virtual bool        Filter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const;

        virtual bool        IsProgressiveFilter() const { return false; }

        void   SetCallback(MeshUserFilterCallback callback)
            {
            m_callback = callback;
            }

    private:
        MeshUserFilterCallback     m_callback;
    };
#endif


