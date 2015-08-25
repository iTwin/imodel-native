//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.h $
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
        size_t UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
};

template<class POINT, class EXTENT> class ScalableMeshAPSSOutOfCoreMesher : public ISMPointIndexMesher<POINT, EXTENT>
{

public:

    // Primary methods
    ScalableMeshAPSSOutOfCoreMesher() {};
    virtual             ~ScalableMeshAPSSOutOfCoreMesher() {};

    virtual bool        Init(const SMMeshIndex<POINT, EXTENT>& pointIndex) override;
    virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;
    virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;


protected:

    size_t m_totalPointCount; 
    size_t m_numberOfLevels;
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

#include "ScalableMeshMesher.hpp"
