/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshProgressiveQuery.h $
|    $RCSfile: ScalableMeshQuery.h,v $
|   $Revision: 1.20 $
|       $Date: 2012/06/27 14:07:12 $
|     $Author: Chantal.Poulin $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>

#include <ImagePP/all/h/HCPGCoordModel.h>


#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMesh.h>
#include "SMMeshIndex.h"
#include "./ScalableMesh/ScalableMeshGraph.h"
#include "ScalableMeshProgressiveQueryPlanner.h"
#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
#endif
//#include <hash_map>

//Only way found to deactivate warning C4250 since the pragma warning(disable... doesn't work
#pragma warning( push, 0 )


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



//typedef ISMStore::Extent3d64f Extent3dType;
typedef DRange3d Extent3dType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

struct ScalableMeshExtentQuery;
typedef RefCountedPtr<ScalableMeshExtentQuery> ScalableMeshExtentQueryPtr;



class ScalableMeshProgressiveQueryEngine : public virtual IScalableMeshProgressiveQueryEngine                              
    {    
    private:  

        bvector<ScalableMeshCachedDisplayNode<DPoint3d>::Ptr> m_overviewNodes;
        bvector<IScalableMesh*>                               m_smOverviews;
        mutable std::vector<RequestedQuery>                   m_requestedQueries;        
       // IScalableMeshPtr                                      m_scalableMeshPtr;
        IScalableMeshDisplayCacheManagerPtr                   m_displayCacheManagerPtr;
        bset<uint64_t>                                        m_activeClips;
        bool                                                  m_loadTexture;
        bvector<QueryPlanner*>                                m_planners;

        QueryPlanner* GetQueryPlanner(const RequestedQuery& queryObject);

        void UpdatePreloadOverview();
        void PreloadOverview(HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>& node, IScalableMesh* sMesh);
        void StartNewQuery(RequestedQuery& newQuery, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP, const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, IScalableMeshViewDependentMeshQueryParamsPtr queryParam);

        void SortOverviews(bvector<IScalableMeshCachedDisplayNodePtr>& overviews, ISMPointIndexQuery<DPoint3d, Extent3dType>* queryObjectP);
    protected:                                        

        void ComputeOverview();

        //Inherited from ScalableMeshProgressiveQueryEngine        
        virtual void          _SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr);

        virtual void          _GetActiveClips(bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr);

        virtual BentleyStatus _StartQuery(int                                                                      queryId, 
                                          IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam, 
                                          const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes,                                           
                                          bool                                                                     loadTexture,
                                          const bvector<bool>&                                                     clipVisibilities,
                                          IScalableMeshPtr&                                                        smPtr) override; 

        virtual BentleyStatus _GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                int                                                                queryId) const override;        

        virtual BentleyStatus _GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                int                                                                queryId) const override;

        virtual BentleyStatus _GetRequiredTextureTiles(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::SMRasterTile>& rasterTiles,
                                                       int                    queryId) const override;

        virtual BentleyStatus _StopQuery(int queryId) override; 

        virtual bool          _IsQueryComplete(int queryId) override; 

        virtual void _ClearOverviews(IScalableMesh* scalableMeshP) override;

        virtual void _InitScalableMesh(IScalableMeshPtr& scalableMeshPtr) override;
        

    public : 

        static void CancelPreload(ScalableMesh<DPoint3d>* smP);

        static void PreloadData(ScalableMesh<DPoint3d>* smP, bvector<HFCPtr<SMPointIndexNode<DPoint3d, Extent3dType>>>& toLoadNodes, bool cancelLastPreload);
        
        ScalableMeshProgressiveQueryEngine(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture);

        virtual ~ScalableMeshProgressiveQueryEngine();

    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
