/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCreatorWorker.h $
|    $RCSfile: IScalableMeshSourceCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2015/07/15 10:35:02 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshQuery.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshSourceCreatorWorker;
typedef RefCountedPtr<IScalableMeshSourceCreatorWorker>            IScalableMeshSourceCreatorWorkerPtr;


//This is the creator interface to use when providing a series of source files to import data to the Scalable Mesh. All details of indexing, etc are handled
//automatically. At the moment, it is not possible to import data from source files and also manually create nodes in the index.
struct IScalableMeshSourceCreatorWorker /*: public IScalableMeshSourceCreator*/
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        //friend struct                       IScalableMeshSourceCreator;
        struct                              Impl;
        //std::auto_ptr<Impl>                 m_implP;

        explicit                            IScalableMeshSourceCreatorWorker(Impl*                       implP);

        /*__PUBLISH_SECTION_START__*/

    public:
        BENTLEY_SM_IMPORT_EXPORT virtual                 ~IScalableMeshSourceCreatorWorker();

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshSourceCreatorWorkerPtr GetFor(const WChar* filePath,
                                                                                   StatusInt&   status);
/*
        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshSourceCreatorWorker GetFor(const IScalableMeshPtr&     scmPtr,
                                                                                StatusInt&                  status);
*/
        // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

/*
        BENTLEY_SM_IMPORT_EXPORT const IDTMSourceCollection& GetSources() const;
        BENTLEY_SM_IMPORT_EXPORT IDTMSourceCollection&       EditSources();

        BENTLEY_SM_IMPORT_EXPORT void                    SetSourcesDirty();
        BENTLEY_SM_IMPORT_EXPORT bool                    HasDirtySources() const;

        BENTLEY_SM_IMPORT_EXPORT bool                    AreAllSourcesReachable() const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt               UpdateLastModified();

        BENTLEY_SM_IMPORT_EXPORT void                    ResetLastModified();

        BENTLEY_SM_IMPORT_EXPORT void                    SetSourceImportExtent(const DRange2d& ext);

        BENTLEY_SM_IMPORT_EXPORT void                    SetSourceImportPolygon(const DPoint3d* polygon, size_t nPts);

        BENTLEY_SM_IMPORT_EXPORT void                    SetCreationMethod(ScalableMeshCreationMethod creationMethod);
*/

//        BENTLEY_SM_IMPORT_EXPORT void SetUserFilterCallback(MeshUserFilterCallback callback);

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE