//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.h $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.h,v $
//:>   $Revision: 1.19 $
//:>       $Date: 2010/12/15 18:23:19 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DCoord.h>


#include "SMPointIndex.h"
#include <Mtg/MtgStructs.h>
#include "ScalableMesh.h"



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

        virtual void        AddClip(bvector<DPoint3d>& clip) override { m_clip = clip; }
        
        
    protected:                
    private:

        void CreateGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT>>& node, const DPoint3d* points, int nbPoints, const long* ptsIndice, int nbPtsIndice) const;
        void SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const;
        void SelectPointsBasedOnBox(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, EXTENT neighborExt) const;
        size_t UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        size_t UpdateMeshNodeFromGraphs(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<MTGGraph *>& graphs, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        size_t UpdateMeshNodeFromIndexLists(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<vector<int32_t>>& indices, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const;
        void   SimplifyMesh(vector<int32_t>& indices, vector<POINT>& points, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, std::string& s) const;

        //Used to restrict meshing to a nonconvex polygonal shape (when importing within a nonconvex polygon)
        bvector<DPoint3d> m_clip;
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
            assert(!"Currently Deactivated");

#ifdef WIP_MESH_IMPORT
            node->GetMetadata();
            node->GetMeshParts();
            bvector<int> newMeshParts;
            bvector<Utf8String> newMeshMetadata;
            int64_t currentTexId = -1;
            int64_t currentElementId = -1;
            if (node->m_meshParts.size() > 0)
                {
                for (size_t i = 0; i < node->m_meshParts.size(); i += 2)
                    {
                    auto metadataString = Utf8String(node->m_meshMetadata[i/2]);
                    Json::Value val;
                    Json::Reader reader;
                    reader.parse(metadataString, val);
                    bvector<int> parts;
                    bvector<int64_t> texId;
                    for (const Json::Value& id : val["texId"])
                        {
                        texId.push_back(id.asInt64());
                        }
                    if (!texId.empty())
                        {
                        if(currentTexId == texId[0] && val["elementId"].asInt64() == currentElementId)
                            {
                            newMeshParts.back() = node->m_meshParts[i+1];
                            continue;
                            }
                        }
                    currentTexId = texId[0];
                    currentElementId = val["elementId"].asInt64();
                    newMeshParts.push_back(node->m_meshParts[i]);
                    newMeshParts.push_back(node->m_meshParts[i+1]);
                    newMeshMetadata.push_back(node->m_meshMetadata[i/2]);

                    }
                }
            node->m_meshParts = newMeshParts;
            node->m_meshMetadata = newMeshMetadata;
            node->StoreMetadata();
            node->StoreMeshParts();
#endif
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

//#include "ScalableMeshMesher.hpp"
