//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.h $
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


#include "SMPointIndex.h"
#include <Mtg/MtgStructs.h>




extern bool   GET_HIGHEST_RES;

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/


/** -----------------------------------------------------------------------------
               
    This class implements a default filter for spatial index of points. It takes
    one every four points out of every vectors.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class ScalableMesh2DDelaunayMesher : public ISMPointIndexMesher<POINT, EXTENT>
{

    public:

        // Primary methods
                            ScalableMesh2DDelaunayMesher() {};
        virtual             ~ScalableMesh2DDelaunayMesher() {};

        virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;

        virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;
        
        
    protected:                
    private:
        void LoadAdjacencyData(MTGGraph* graph, POINT* pPoints, size_t size) const;
        void SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const;
        void SelectPointsBasedOnBox(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, EXTENT neighborExt) const;
        size_t UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        size_t UpdateMeshNodeFromGraphs(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<MTGGraph *>& graphs, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        size_t UpdateMeshNodeFromIndexLists(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<vector<int32_t>>& indices, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        void   SimplifyMesh(vector<int32_t>& indices, vector<POINT>& points, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, std::string& s) const;
    };


 template<class POINT, class EXTENT> class ScalableMeshExistingMeshMesher : public ISMPointIndexMesher<POINT, EXTENT>
{

    public:

        // Primary methods
        ScalableMeshExistingMeshMesher() {};
        virtual             ~ScalableMeshExistingMeshMesher() {};

        virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override
            {         
            return true;
            };

        virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override
            {
            return true;
            };
        
    };
    /** -----------------------------------------------------------------------------

    This class implements a default filter for spatial index of points. It takes
    one every four points out of every vectors.
    -----------------------------------------------------------------------------
    */
    template<class POINT, class EXTENT> class ScalableMesh3DDelaunayMesher : public ISMPointIndexMesher<POINT, EXTENT>
        {

        public:

            // Primary methods
            ScalableMesh3DDelaunayMesher (bool tetGen) : m_tetGen (tetGen)
                {
                };
            virtual             ~ScalableMesh3DDelaunayMesher ()
                {
                };

            virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;

            virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;


        protected:
        private:
            void SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const;
            size_t UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;

        private:
            bool m_tetGen;
        };

        void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons);
        void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons, std::function<bool(const size_t i, const bvector<DPoint3d>& element)> choosePolygonInSet, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded);

//#include "ScalableMeshMesher.hpp"
