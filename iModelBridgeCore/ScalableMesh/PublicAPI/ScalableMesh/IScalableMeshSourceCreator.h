/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCreator.h $
|    $RCSfile: IScalableMeshSourceCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2015/07/15 10:35:02 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshQuery.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshSourceCreator;
typedef RefCountedPtr<IScalableMeshSourceCreator>            IScalableMeshSourceCreatorPtr;

//Used to provide a callback to the filter function to get user-created LODs. Callback should return true if LOD creation succeeded.
//Callback should set shouldCreateGraph to true if a stitching is needed or the filter function requires the use of the graph. 
//
typedef std::function<bool(bool& shouldCreateGraph, bvector<bvector<DPoint3d>>& newMeshPts, bvector<bvector<int32_t>>& newMeshIndexes, bvector<Utf8String>& newMeshMetadata, const bvector<IScalableMeshMeshPtr>& submeshes, const bvector<Utf8String>& meshMetadata, DRange3d nodeExt)> MeshUserFilterCallback;

//This is the creator interface to use when providing a series of source files to import data to the Scalable Mesh. All details of indexing, etc are handled
//automatically. At the moment, it is not possible to import data from source files and also manually create nodes in the index.
struct IScalableMeshSourceCreator : public IScalableMeshCreator
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        friend struct                       IScalableMeshCreator;
        struct                              Impl;
        //std::auto_ptr<Impl>                 m_implP;

        explicit                            IScalableMeshSourceCreator(Impl*                       implP);

        /*__PUBLISH_SECTION_START__*/

    public:
        BENTLEY_SM_EXPORT virtual                 ~IScalableMeshSourceCreator();

        BENTLEY_SM_EXPORT static IScalableMeshSourceCreatorPtr GetFor(const WChar*              filePath,
                                                                    StatusInt&                status);

        BENTLEY_SM_EXPORT static IScalableMeshSourceCreatorPtr GetFor(const IScalableMeshPtr&     scmPtr,
                                                                    StatusInt&                  status);
        // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEY_SM_EXPORT const IDTMSourceCollection& GetSources() const;
        BENTLEY_SM_EXPORT IDTMSourceCollection&       EditSources();

        BENTLEY_SM_EXPORT void                    SetSourcesDirty();
        BENTLEY_SM_EXPORT bool                    HasDirtySources() const;

        BENTLEY_SM_EXPORT bool                    AreAllSourcesReachable() const;

        BENTLEY_SM_EXPORT StatusInt               UpdateLastModified();

        BENTLEY_SM_EXPORT void                    ResetLastModified();

        BENTLEY_SM_EXPORT void                    SetSourceImportExtent(const DRange2d& ext);

        BENTLEY_SM_EXPORT void                    SetSourceImportPolygon(const DPoint3d* polygon, size_t nPts);

#ifdef SCALABLE_MESH_ATP

        BENTLEY_SM_EXPORT static unsigned __int64 GetNbImportedPoints();

        BENTLEY_SM_EXPORT static double GetImportPointsDuration();

        BENTLEY_SM_EXPORT static double GetLastBalancingDuration();

        BENTLEY_SM_EXPORT static double GetLastMeshingDuration();

        BENTLEY_SM_EXPORT static double GetLastFilteringDuration();

        BENTLEY_SM_EXPORT static double GetLastStitchingDuration();

        BENTLEY_SM_EXPORT        void   ImportRastersTo(const IScalableMeshPtr& scmPtr);
#endif   

        BENTLEY_SM_EXPORT void SetUserFilterCallback(MeshUserFilterCallback callback);


    };

END_BENTLEY_SCALABLEMESH_NAMESPACE